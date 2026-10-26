# Wall Clock makefile for MSVC nmake (64-bit)
#
# Build with: nmake        Clean with: nmake clean
# Setup.exe:  nmake installer   (needs Inno Setup 6)
#
# Run from a Visual Studio x64 developer command prompt (vcvars64.bat).
# Needs a static release build of wxWidgets 3.2 with the Edge web view
# (see README.md). Point WXDIR at it if it is not in the default place:
#   nmake WXDIR=C:\path\to\wxWidgets

WXDIR = E:\3plibs\wxWidgets64-3.2.4-edge
WXLIBDIR = $(WXDIR)\lib\vc_x64_lib
WEBVIEW2DIR = $(WXDIR)\3rdparty\webview2\build\native

PROJECT = wall_clock

CXXFLAGS = /nologo /EHsc /MT /O2 /W3 /std:c++17 /DNDEBUG \
	/D__WXMSW__ /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS \
	/I$(WXLIBDIR)\mswu /I$(WXDIR)\include

RCFLAGS = /nologo /d__WXMSW__ /d_UNICODE /i$(WXLIBDIR)\mswu /i$(WXDIR)\include

LDFLAGS = /nologo /SUBSYSTEM:WINDOWS /LIBPATH:$(WXLIBDIR) \
	/LIBPATH:$(WEBVIEW2DIR)\x64

LIBS = wxmsw32u_webview.lib wxmsw32u_core.lib wxbase32u.lib \
	wxpng.lib wxzlib.lib wxregexu.lib WebView2LoaderStatic.lib \
	kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib \
	shell32.lib shlwapi.lib ole32.lib oleaut32.lib uuid.lib \
	comctl32.lib advapi32.lib version.lib uxtheme.lib oleacc.lib \
	ws2_32.lib wininet.lib urlmon.lib rpcrt4.lib

all: $(PROJECT).exe

$(PROJECT).obj: $(PROJECT).cpp
	cl $(CXXFLAGS) /c $(PROJECT).cpp

$(PROJECT).res: $(PROJECT).rc icons\wall_clock.ico
	rc $(RCFLAGS) /fo$@ $(PROJECT).rc

$(PROJECT).exe: $(PROJECT).obj $(PROJECT).res
	link $(LDFLAGS) /OUT:$@ $(PROJECT).obj $(PROJECT).res $(LIBS)

# Windows setup.exe in output\
ISCC = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

installer: all
	$(ISCC) /Qp $(PROJECT)_installer.iss

clean:
	-del /q *.exe *.obj *.res *.pdb *.ilk 2>nul
	-rmdir /s /q output 2>nul
