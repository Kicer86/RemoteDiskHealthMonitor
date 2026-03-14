Name:           rdhm-monitor
Version:        0.2.0
Release:        1%{?dist}
Summary:        Remote Disc Health Monitor - Monitor GUI
License:        GPL-3.0-or-later
URL:            https://github.com/user/hdd-monitor

Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++ >= 10
BuildRequires:  make
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtquickcontrols2-devel
BuildRequires:  avahi-devel

Requires:       qt6-qtbase-gui
Requires:       qt6-qtdeclarative
Requires:       qt6-qtquickcontrols2
Requires:       avahi-libs

%description
Qt6/QML graphical application that discovers RDHM agents on the local
network via mDNS/ZeroConf and displays disk health status in real time.

%prep
%autosetup

%build
%cmake -DBUILD_AGENT=OFF -DBUILD_MONITOR=ON
%cmake_build --target monitor

%install
%cmake_install

%files
%license LICENSE
%{_bindir}/rdhm-monitor
