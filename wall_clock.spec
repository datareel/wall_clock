# RPM spec for Wall Clock on RHEL 9 and 10 (wxGTK from EPEL)
#
# rpmbuild -ta wall_clock-2.2.tar.gz

Name:           wall_clock
Version:        2.2
Release:        1%{?dist}
Summary:        Full screen UTC and local time clock that keeps the screen awake
License:        GPL-3.0-or-later
URL:            https://datareel.com/info_wall_clock.php
Source0:        https://datareel.com/downloads/wall_clock/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  wxGTK-devel
BuildRequires:  pkgconfig(gio-2.0)
# WebKitGTK needs the MIME database to show the local clock page
Requires:       shared-mime-info

%description
Wall Clock shows UTC time on the top half of the screen and local time on
the bottom half in large red seven-segment digits. While the clock window
is open it asks the desktop not to blank or lock the screen, so it suits a
wall display or an operations room monitor that nobody touches.

%prep
%autosetup

%build
%set_build_flags
%make_build -f makefile.linux CXXFLAGS_EXTRA="%{optflags}"

%install
%make_install -f makefile.linux BINDIR=%{_bindir} APPSDIR=%{_datadir}/applications

%files
%license LICENSE
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
* Mon Oct 26 2026 DataReel <dgaerhome@gmail.com> - 2.2-1
- First Linux release: RHEL 9 and 10 with wxGTK and WebKitGTK
