// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- //
// C++ Source Code File Name: wall_clock.cpp
// C++ Compiler Used: MSVC, wxWidgets 3.2 with the Edge web view
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
// Wall Clock: a full screen clock display for Windows.
//
// The clock face is a web page (html\index.html next to the program)
// shown in a wxWebView window. While the window is open the program
// keeps the display on and stops the screen saver and the idle screen
// lock from starting, so the clock stays visible on an unattended
// display.
//
// Usage: wall_clock.exe [--fullscreen] [URL]
//
// Settings: wall_clock.ini next to the program can set the page to show
// (url=) and full screen start (fullscreen=). The command line wins over
// the INI file.
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

#if !defined(__WXMSW__)
#error "Wall Clock is a Windows program"
#endif

#define WALL_CLOCK_VERSION "2.1"

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

  wxString m_url;
  bool m_fullscreen = false;
};

wxIMPLEMENT_APP(WallClockApp);

// The program's folder, which holds html\index.html and wall_clock.ini
static wxString ProgramDir()
{
  return wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
}

// Default page: html\index.html in the program's folder
static wxString DefaultClockURL()
{
  wxFileName page(ProgramDir(), "index.html");
  page.AppendDir("html");
  return wxFileName::FileNameToURL(page);
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

// Read wall_clock.ini in the program's folder:
//
//   [clock]
//   url=https://example.com/clock/
//   fullscreen=1
//
// Settings given on the command line are kept.
void WallClockApp::ReadSettings()
{
  wxFileName ini(ProgramDir(), "wall_clock.ini");
  if(!ini.FileExists()) return;

  wxFileConfig config(wxEmptyString, wxEmptyString, ini.GetFullPath(),
                      wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
  config.SetPath("/clock");

  wxString url = config.Read("url", wxEmptyString).Strip(wxString::both);
  if(m_url.empty() && !url.empty()) m_url = url;
  if(!m_fullscreen) m_fullscreen = config.ReadBool("fullscreen", false);
}

void WallClockApp::OnInitCmdLine(wxCmdLineParser &parser)
{
  wxApp::OnInitCmdLine(parser);
  parser.AddSwitch("f", "fullscreen", "start in full screen mode");
  parser.AddParam("URL of the clock page (default: html\\index.html)",
                  wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool WallClockApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
  if(!wxApp::OnCmdLineParsed(parser)) return false;
  m_fullscreen = parser.Found("f");
  if(parser.GetParamCount() > 0) m_url = parser.GetParam(0);
  return true;
}

bool WallClockApp::OnInit()
{
  // Also names the WebView2 data folder under %LOCALAPPDATA%
  SetAppName("WallClock");
  SetAppDisplayName("Wall Clock");

  if(!wxApp::OnInit()) return false;
  ReadSettings();
  m_url = m_url.empty() ? DefaultClockURL() : ToURL(m_url);

  // Keep the display on and the system awake, which also keeps the
  // screen saver and the idle screen lock from starting. Cleared again
  // in OnExit(), and by Windows when the program ends.
  SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED |
                          ES_SYSTEM_REQUIRED);

  WallClockFrame *frame = new WallClockFrame(m_url, m_fullscreen);
  frame->Show();
  return true;
}

int WallClockApp::OnExit()
{
  // Allow normal power saving and screen locking again
  SetThreadExecutionState(ES_CONTINUOUS);
  return wxApp::OnExit();
}

WallClockFrame::WallClockFrame(const wxString &url, bool fullscreen)
  : wxFrame(nullptr, wxID_ANY, "Wall Clock"), m_url(url)
{
  SetIcon(wxICON(wall_clock));
  SetBackgroundColour(*wxBLACK);

  // Prefer the Edge (WebView2) back end. Fall back to the default one,
  // Internet Explorer, if the WebView2 runtime is not installed.
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
