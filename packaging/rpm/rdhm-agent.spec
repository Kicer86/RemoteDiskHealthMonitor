Name:           rdhm-agent
Version:        0.2.0
Release:        1%{?dist}
Summary:        Remote Disc Health Monitor - Agent
License:        GPL-3.0-or-later
URL:            https://github.com/user/hdd-monitor

Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++ >= 10
BuildRequires:  make

Requires:       smartmontools
Requires:       util-linux
Requires:       systemd

%description
Lightweight daemon that collects local disk health metrics using
SMART data, dmesg, and lsblk, and publishes them over an HTTP REST API
with mDNS/DNS-SD service discovery. Designed to run as a systemd service.

%prep
%autosetup

%build
%cmake -DBUILD_MONITOR=OFF -DBUILD_AGENT=ON
%cmake_build --target agent

%install
%cmake_install
install -Dm644 packaging/systemd/rdhm-agent.service \
    %{buildroot}%{_unitdir}/rdhm-agent.service
install -Dm644 packaging/conf/agent.conf \
    %{buildroot}%{_sysconfdir}/rdhm/agent.conf

%post
%systemd_post rdhm-agent.service

%preun
%systemd_preun rdhm-agent.service

%postun
%systemd_postun_with_restart rdhm-agent.service

%files
%license LICENSE
%{_sbindir}/rdhm-agent
%{_unitdir}/rdhm-agent.service
%config(noreplace) %{_sysconfdir}/rdhm/agent.conf

%changelog
* Sat Mar 14 2026 Michał Walenciak <michalwalenciak@gmail.com> - 0.2.0-1
- Initial packaging
