// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- //
// C++ Source Code File Name: wall_clock.cpp
// C++ Compiler Used: MSVC, wxWidgets 3.2 with the Edge web view
//                    GCC, wxWidgets 3.2 GTK with the WebKitGTK web view
// ----------------------------------------------------------- //
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Wall Clock: a full screen clock display for Windows and Linux.
//
// The clock face is a web page (html/index.html next to the program)
// shown in a wxWebView window. While the window is open the program
// keeps the display on and stops the screen saver and the idle screen
// lock from starting, so the clock stays visible on an unattended
// display.
//
// Usage: wall_clock[.exe] [--fullscreen] [URL]
//
// Settings: wall_clock.ini next to the program can set the page to show
// (url=) and full screen start (fullscreen=). On Linux a personal
// ~/.config/wall_clock/wall_clock.ini wins over the one next to the
// program. The command line wins over the INI files.
//
// F11 or a double click toggles full screen, Esc leaves full screen,
// F5 reloads the page.
//
// ----------------------------------------------------------- //

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/cmdline.h"
#include "wx/fileconf.h"
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "wx/webview.h"

#if !wxUSE_WEBVIEW
#error "Wall Clock needs wxWidgets built with wxUSE_WEBVIEW"
#endif

#if defined(__WXGTK__)
#include <gio/gio.h>
#include <stdlib.h>
#include <unistd.h>
#elif !defined(__WXMSW__)
#error "Wall Clock is a Windows or Linux (wxGTK) program"
#endif

#define WALL_CLOCK_VERSION "2.4"

// Name the page uses to send messages to the program
static const char *SCRIPT_HANDLER = "wallclock";

class WallClockFrame : public wxFrame
{
public:
  WallClockFrame(const wxString &url, bool fullscreen);

private:
  void ToggleFullScreen();
  void OnScriptMessage(wxWebViewEvent &evt);
  void OnLoadError(wxWebViewEvent &evt);
  void OnLoaded(wxWebViewEvent &evt);
  void OnActivate(wxActivateEvent &evt);
  void OnCharHook(wxKeyEvent &evt);

  wxWebView *m_browser;
  wxString m_url;
};

class WallClockApp : public wxApp
{
public:
  virtual bool OnInit() override;
  virtual int OnExit() override;
  virtual void OnInitCmdLine(wxCmdLineParser &parser) override;
  virtual bool OnCmdLineParsed(wxCmdLineParser &parser) override;

private:
  void ReadSettings();
  void KeepScreenAwake();
  void AllowScreenSleep();

  wxString m_url;
  bool m_fullscreen = false;
  bool m_fullscreen_set = false;
  wxString m_timezone;
  bool m_timezone_set = false;
#if defined(__WXGTK__)
  GDBusConnection *m_bus = nullptr;
  const char *m_inhibit_service = nullptr;
  guint32 m_inhibit_cookie = 0;
#endif
};

wxIMPLEMENT_APP(WallClockApp);

// The program's folder, which holds html/index.html and wall_clock.ini
static wxString ProgramDir()
{
  return wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
}

// Default page: html/index.html in the program's folder
static wxString DefaultClockURL()
{
  wxFileName page(ProgramDir(), "index.html");
  page.AppendDir("html");
  return wxFileName::FileNameToURL(page);
}

// A time zone name the clock page accepts: an IANA name such as
// America/New_York or Etc/GMT+5, or empty for the computer's time zone
static bool ValidTimeZone(const wxString &zone)
{
  if(zone.length() > 64) return false;
  for(wxUniChar c : zone) {
    if(!(wxIsalnum(c) || c == '/' || c == '_' || c == '-' || c == '+'))
      return false;
  }
  return true;
}

// The settings file the program writes: ~/.config/wall_clock/wall_clock.ini
// on Linux, wall_clock.ini in the program's folder on Windows (the setup
// program lets users change it)
static wxString UserSettingsFile()
{
#if defined(__WXGTK__)
  wxString config_home;
  if(!wxGetEnv("XDG_CONFIG_HOME", &config_home) || config_home.empty())
    config_home = wxGetHomeDir() + "/.config";
  return config_home + "/wall_clock/wall_clock.ini";
#else
  return wxFileName(ProgramDir(), "wall_clock.ini").GetFullPath();
#endif
}

// Save the bottom panel's time zone as timezone= in the settings file
static void SaveTimeZone(const wxString &zone)
{
  wxFileName file(UserSettingsFile());
  if(!file.DirExists() && !file.Mkdir(0755, wxPATH_MKDIR_FULL)) {
    wxLogWarning("Cannot save the time zone: cannot make %s", file.GetPath());
    return;
  }
  wxFileConfig config(wxEmptyString, wxEmptyString, file.GetFullPath(),
                      wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
  config.Write("/clock/timezone", zone);
  if(!config.Flush())
    wxLogWarning("Cannot save the time zone in %s", file.GetFullPath());
}

// The alarm list (JSON from the clock page), [alarms] list= in the settings
// files. The user's file is read first on Linux, as for the [clock] settings.
static wxString ReadAlarms()
{
  wxArrayString files;
  files.Add(UserSettingsFile());
#if defined(__WXGTK__)
  files.Add(wxFileName(ProgramDir(), "wall_clock.ini").GetFullPath());
#endif
  for(const wxString &file : files) {
    if(!wxFileName::FileExists(file)) continue;
    wxFileConfig config(wxEmptyString, wxEmptyString, file, wxEmptyString,
                        wxCONFIG_USE_LOCAL_FILE);
    if(config.HasEntry("/alarms/list"))
      return config.Read("/alarms/list", "[]");
  }
  return "[]";
}

static void SaveAlarms(const wxString &json)
{
  wxFileName file(UserSettingsFile());
  if(!file.DirExists() && !file.Mkdir(0755, wxPATH_MKDIR_FULL)) {
    wxLogWarning("Cannot save the alarms: cannot make %s", file.GetPath());
    return;
  }
  wxFileConfig config(wxEmptyString, wxEmptyString, file.GetFullPath(),
                      wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
  config.Write("/alarms/list", json);
  if(!config.Flush())
    wxLogWarning("Cannot save the alarms in %s", file.GetFullPath());
}

// Quote text as a JavaScript string literal
static wxString JSString(const wxString &text)
{
  wxString out = "'";
  for(wxUniChar c : text) {
    if(c == '\\' || c == '\'') out << '\\' << c;
    else if(c == '\n') out << "\\n";
    else if(c == '\r') out << "\\r";
    else if(c == '<') out << "\\x3c";
    else if(c.GetValue() == 0x2028 || c.GetValue() == 0x2029)
      out << wxString::Format("\\u%04x", (unsigned)c.GetValue());
    else out << c;
  }
  return out + "'";
}

// Turn a URL or a file path into a URL. A relative path is relative to
// the program's folder.
static wxString ToURL(const wxString &location)
{
  if(location.Contains("://") || location.StartsWith("about:"))
    return location;
  wxFileName file(location);
  file.MakeAbsolute(ProgramDir());
  return wxFileName::FileNameToURL(file);
}

// Read wall_clock.ini:
//
//   [clock]
//   url=https://example.com/clock/
//   fullscreen=1
//   timezone=America/Chicago
//
// On Linux ~/.config/wall_clock/wall_clock.ini is read first, then the
// one in the program's folder. The first setting found is used, and
// settings given on the command line are kept.
void WallClockApp::ReadSettings()
{
  wxArrayString files;
#if defined(__WXGTK__)
  wxString config_home;
  if(!wxGetEnv("XDG_CONFIG_HOME", &config_home) || config_home.empty())
    config_home = wxGetHomeDir() + "/.config";
  files.Add(config_home + "/wall_clock/wall_clock.ini");
#endif
  files.Add(wxFileName(ProgramDir(), "wall_clock.ini").GetFullPath());

  for(const wxString &file : files) {
    if(!wxFileName::FileExists(file)) continue;

    wxFileConfig config(wxEmptyString, wxEmptyString, file, wxEmptyString,
                        wxCONFIG_USE_LOCAL_FILE);
    config.SetPath("/clock");

    wxString url = config.Read("url", wxEmptyString).Strip(wxString::both);
    if(m_url.empty() && !url.empty()) m_url = url;
    if(!m_fullscreen_set && config.HasEntry("fullscreen")) {
      m_fullscreen = config.ReadBool("fullscreen", false);
      m_fullscreen_set = true;
    }
    if(!m_timezone_set && config.HasEntry("timezone")) {
      m_timezone = config.Read("timezone", wxEmptyString).Strip(wxString::both);
      m_timezone_set = true;
    }
  }
  if(!ValidTimeZone(m_timezone)) {
    wxLogWarning("Ignoring the time zone \"%s\" in wall_clock.ini", m_timezone);
    m_timezone.clear();
  }
}

void WallClockApp::OnInitCmdLine(wxCmdLineParser &parser)
{
  wxApp::OnInitCmdLine(parser);
  parser.AddSwitch("f", "fullscreen", "start in full screen mode");
  parser.AddOption("z", "timezone",
                   "time zone of the bottom panel, such as America/Chicago");
  parser.AddParam("URL of the clock page (default: html/index.html)",
                  wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool WallClockApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
  if(!wxApp::OnCmdLineParsed(parser)) return false;
  m_fullscreen = parser.Found("f");
  m_fullscreen_set = m_fullscreen;
  if(parser.GetParamCount() > 0) m_url = parser.GetParam(0);
  if(parser.Found("z", &m_timezone)) m_timezone_set = true;
  return true;
}

#if defined(__WXGTK__)
// WebKitGTK's DMA-BUF renderer fails with the NVIDIA driver: "Error 71
// (Protocol error) dispatching to Wayland display" on Wayland and "Failed
// to create GBM buffer" on X11. Turn that renderer off when the NVIDIA
// driver is loaded, before the web view starts. A value already set in
// the environment is kept.
static void AvoidNvidiaDmabuf()
{
  if(access("/proc/driver/nvidia/version", F_OK) != 0) return;
  setenv("WEBKIT_DISABLE_DMABUF_RENDERER", "1", 0);
}
#endif

bool WallClockApp::OnInit()
{
#if defined(__WXGTK__)
  AvoidNvidiaDmabuf();
#endif
  // Also names the WebView2 data folder under %LOCALAPPDATA%
  SetAppName("WallClock");
  SetAppDisplayName("Wall Clock");

  if(!wxApp::OnInit()) return false;
  ReadSettings();
  if(m_url.empty()) {
    // The built-in page reads the bottom panel's time zone from ?tz=
    m_url = DefaultClockURL();
    if(!m_timezone.empty()) {
      wxString tz(m_timezone);
      tz.Replace("+", "%2B");
      m_url += "?tz=" + tz;
    }
  }
  else {
    m_url = ToURL(m_url);
  }

  KeepScreenAwake();

  WallClockFrame *frame = new WallClockFrame(m_url, m_fullscreen);
  frame->Show();
  return true;
}

int WallClockApp::OnExit()
{
  AllowScreenSleep();
  return wxApp::OnExit();
}

#if defined(__WXGTK__)
// Idle inhibit services on the session bus, tried in order. GNOME's
// session manager takes flags (4 = suspend, 8 = idle); the freedesktop
// screen saver interface is the one KDE, Xfce and others provide.
struct InhibitService {
  const char *name, *path, *iface, *inhibit, *uninhibit;
  bool gnome;
};
static const InhibitService INHIBIT_SERVICES[] = {
  { "org.gnome.SessionManager", "/org/gnome/SessionManager",
    "org.gnome.SessionManager", "Inhibit", "Uninhibit", true },
  { "org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver",
    "org.freedesktop.ScreenSaver", "Inhibit", "UnInhibit", false }
};
#endif

// Keep the display on and the system awake, which also keeps the
// screen saver and the idle screen lock from starting. Undone in
// AllowScreenSleep(), and by the system when the program ends.
void WallClockApp::KeepScreenAwake()
{
#if defined(__WXMSW__)
  SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED |
                          ES_SYSTEM_REQUIRED);
#elif defined(__WXGTK__)
  GError *error = nullptr;
  m_bus = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
  if(!m_bus) {
    wxLogWarning("Cannot keep the screen awake: no session bus (%s)",
                 error->message);
    g_error_free(error);
    return;
  }
  for(const InhibitService &s : INHIBIT_SERVICES) {
    GVariant *args = s.gnome ?
      g_variant_new("(susu)", "wall_clock", 0, "Showing the wall clock",
                    4u | 8u) :
      g_variant_new("(ss)", "wall_clock", "Showing the wall clock");
    GVariant *reply = g_dbus_connection_call_sync(
      m_bus, s.name, s.path, s.iface, s.inhibit, args,
      G_VARIANT_TYPE("(u)"), G_DBUS_CALL_FLAGS_NONE, 5000, nullptr, &error);
    if(reply) {
      g_variant_get(reply, "(u)", &m_inhibit_cookie);
      g_variant_unref(reply);
      m_inhibit_service = s.name;
      return;
    }
    wxLogDebug("%s inhibit failed: %s", s.name, error->message);
    g_clear_error(&error);
  }
  wxLogWarning("Cannot keep the screen awake: the desktop has no idle "
               "inhibit service");
#endif
}

// Allow normal power saving and screen locking again
void WallClockApp::AllowScreenSleep()
{
#if defined(__WXMSW__)
  SetThreadExecutionState(ES_CONTINUOUS);
#elif defined(__WXGTK__)
  if(!m_bus) return;
  for(const InhibitService &s : INHIBIT_SERVICES) {
    if(!m_inhibit_service || strcmp(s.name, m_inhibit_service) != 0)
      continue;
    GVariant *reply = g_dbus_connection_call_sync(
      m_bus, s.name, s.path, s.iface, s.uninhibit,
      g_variant_new("(u)", m_inhibit_cookie), nullptr,
      G_DBUS_CALL_FLAGS_NONE, 5000, nullptr, nullptr);
    if(reply) g_variant_unref(reply);
  }
  m_inhibit_service = nullptr;
  g_clear_object(&m_bus);
#endif
}

WallClockFrame::WallClockFrame(const wxString &url, bool fullscreen)
  : wxFrame(nullptr, wxID_ANY, "Wall Clock"), m_url(url)
{
#if defined(__WXMSW__)
  SetIcon(wxICON(wall_clock));
#else
  wxFileName icon(ProgramDir(), "wall_clock.png");
  icon.AppendDir("icons");
  if(icon.FileExists()) {
    if(!wxImage::FindHandler(wxBITMAP_TYPE_PNG))
      wxImage::AddHandler(new wxPNGHandler);
    SetIcon(wxIcon(icon.GetFullPath(), wxBITMAP_TYPE_PNG));
  }
#endif
  SetBackgroundColour(*wxBLACK);

  // Prefer the Edge (WebView2) back end. Fall back to the default one,
  // Internet Explorer, if the WebView2 runtime is not installed. On
  // Linux the default is WebKitGTK.
  wxString backend = wxWebViewBackendDefault;
#if wxUSE_WEBVIEW_EDGE
  if(wxWebView::IsBackendAvailable(wxWebViewBackendEdge))
    backend = wxWebViewBackendEdge;
#endif

  m_browser = wxWebView::New(this, wxID_ANY, url, wxDefaultPosition,
                             wxDefaultSize, backend);
  if(!m_browser) {
    wxLogFatalError("Cannot create the web view for the clock page");
  }
  m_browser->EnableContextMenu(false);
  m_browser->EnableAccessToDevTools(false);
  if(!m_browser->AddScriptMessageHandler(SCRIPT_HANDLER)) {
    // Keys still work through OnCharHook() when the frame has focus
    wxLogDebug("Cannot add the script message handler");
  }

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(m_browser, wxSizerFlags(1).Expand());
  SetSizer(sizer);
  SetSize(FromDIP(wxSize(1100, 820)));
  Centre();

  m_browser->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                  &WallClockFrame::OnScriptMessage, this);
  m_browser->Bind(wxEVT_WEBVIEW_ERROR, &WallClockFrame::OnLoadError, this);
  m_browser->Bind(wxEVT_WEBVIEW_LOADED, &WallClockFrame::OnLoaded, this);
  Bind(wxEVT_ACTIVATE, &WallClockFrame::OnActivate, this);
  Bind(wxEVT_CHAR_HOOK, &WallClockFrame::OnCharHook, this);

  if(fullscreen) ShowFullScreen(true);
}

void WallClockFrame::ToggleFullScreen()
{
  ShowFullScreen(!IsFullScreen());
}

void WallClockFrame::OnScriptMessage(wxWebViewEvent &evt)
{
  const wxString msg = evt.GetString();
  if(msg == "fullscreen") {
    ToggleFullScreen();
  }
  else if(msg == "exitfullscreen") {
    if(IsFullScreen()) ShowFullScreen(false);
  }
  else if(msg == "getalarms") {
    m_browser->RunScriptAsync("window.wallclockAlarms && window.wallclockAlarms(" +
                              JSString(ReadAlarms()) + ")");
  }
  else if(msg.StartsWith("alarms:")) {
    const wxString json = msg.Mid(7);
    if(json.length() < 32768 && json.StartsWith("[")) SaveAlarms(json);
  }
  else if(msg == "alarm") {
    // An alarm is ringing: beep and bring the clock to the front
    wxBell();
    if(IsIconized()) Iconize(false);
    Raise();
    RequestUserAttention(wxUSER_ATTENTION_ERROR);
  }
  else if(msg.StartsWith("timezone:")) {
    const wxString zone = msg.Mid(9);
    if(ValidTimeZone(zone)) SaveTimeZone(zone);
  }
}

// Give the keyboard to the page, so its keys (A for alarms, Z for the
// time zone) work without clicking first. On Wayland the web view does
// not always take the focus when the window is activated.
void WallClockFrame::OnLoaded(wxWebViewEvent &evt)
{
  m_browser->SetFocus();
  evt.Skip();
}

void WallClockFrame::OnActivate(wxActivateEvent &evt)
{
  if(evt.GetActive()) {
    CallAfter([this]() { if(m_browser) m_browser->SetFocus(); });
  }
  evt.Skip();
}

void WallClockFrame::OnLoadError(wxWebViewEvent &evt)
{
  // Only report a failure to load the clock page itself
  if(evt.GetURL() != m_url) return;
  wxLogError("Cannot load the clock page %s\n\n%s",
             evt.GetURL(), evt.GetString());
}

void WallClockFrame::OnCharHook(wxKeyEvent &evt)
{
  switch(evt.GetKeyCode()) {
    case WXK_F11:
      ToggleFullScreen();
      return;
    case WXK_ESCAPE:
      if(IsFullScreen()) {
        ShowFullScreen(false);
        return;
      }
      break;
    case WXK_F5:
      m_browser->Reload();
      return;
  }
  evt.Skip();
}
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
