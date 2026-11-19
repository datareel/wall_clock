# RPM spec for Wall Clock on RHEL 8, 9 and 10
#
# RHEL 9 and 10 build with wxGTK from EPEL:
#   rpmbuild -ta wall_clock-<version>.tar.gz
# RHEL 8 (EPEL 8 has only wxWidgets 3.0, too old for the web view) builds a
# private static wxWidgets 3.2 with WebKitGTK first. Put the wxWidgets source
# in ~/rpmbuild/SOURCES and use the spec from the tarball:
#   rpmbuild -bb wall_clock.spec

# Keep the build machine's name and home folder out of the package
%global _buildhost localhost

Name:           wall_clock
Version:        2.3
Release:        1%{?dist}
Summary:        Full screen UTC and local time clock that keeps the screen awake
License:        GPL-3.0-or-later
URL:            https://datareel.com/info_wall_clock.php
Source0:        https://datareel.com/downloads/wall_clock/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  make
%if 0%{?rhel} == 8
%global wxver 3.2.4
Source1:        https://github.com/wxWidgets/wxWidgets/releases/download/v%{wxver}/wxWidgets-%{wxver}.tar.bz2
BuildRequires:  gtk3-devel
BuildRequires:  webkit2gtk3-devel
BuildRequires:  libSM-devel
# wxWidgets is linked in statically; the program itself is the only user
Provides:       bundled(wxWidgets) = %{wxver}
%else
BuildRequires:  wxGTK-devel
%endif
BuildRequires:  pkgconfig(gio-2.0)
# WebKitGTK needs the MIME database to show the local clock page
Requires:       shared-mime-info

%description
Wall Clock shows UTC time on the top half of the screen and local time on
the bottom half in large red seven-segment digits. While the clock window
is open it asks the desktop not to blank or lock the screen, so it suits a
wall display or an operations room monitor that nobody touches.

%prep
%if 0%{?rhel} == 8
%autosetup -a 1
%else
%autosetup
%endif

%build
%set_build_flags
%if 0%{?rhel} == 8
# Source paths in assert messages (__FILE__) start at /builddir, not $HOME
export CFLAGS="$CFLAGS -fmacro-prefix-map=%{_builddir}=/builddir"
export CXXFLAGS="$CXXFLAGS -fmacro-prefix-map=%{_builddir}=/builddir"
mkdir -p wxbuild
cd wxbuild
../wxWidgets-%{wxver}/configure --prefix=$PWD/../wxinst \
  --disable-shared --with-gtk=3 --enable-webview --disable-sys-libs \
  --disable-mediactrl --without-opengl --without-libnotify \
  --disable-secretstore --without-liblzma --without-libmspack
%make_build
make install
cd ..
%make_build -f makefile.linux CXXFLAGS_EXTRA="$CXXFLAGS" \
  WX_CONFIG=$PWD/wxinst/bin/wx-config
%else
%make_build -f makefile.linux CXXFLAGS_EXTRA="%{optflags}"
%endif

%install
%make_install -f makefile.linux BINDIR=%{_bindir} APPSDIR=%{_datadir}/applications

%files
%license LICENSE
%if 0%{?rhel} == 8
%license wxWidgets-%{wxver}/docs/licence.txt
%endif
%doc README.md
%dir /opt/wall_clock
/opt/wall_clock/wall_clock
/opt/wall_clock/html
/opt/wall_clock/icons
/opt/wall_clock/LICENSE
/opt/wall_clock/README.md
%config(noreplace) /opt/wall_clock/wall_clock.ini
%{_bindir}/wall_clock
%{_datadir}/applications/wall_clock.desktop

%changelog
* Wed Nov 18 2026 DataReel Software Development - 2.3-1
- RHEL 8 build with a private static wxWidgets 3.2.4 and WebKitGTK 4.0

* Mon Oct 26 2026 DataReel Software Development - 2.2-1
- First Linux release: RHEL 9 and 10 with wxGTK and WebKitGTK
