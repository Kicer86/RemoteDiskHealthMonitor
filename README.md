
# Remote Disk Health Monitor

Keep an eye on the health of disks in all your machines from one place.

RDHM consists of two programs:

* **rdhm-agent** – a small daemon running on every machine you want to watch.
  It collects SMART data (through `smartctl`), scans kernel logs for
  storage-related errors and serves the results over a simple HTTP API.
  It also announces itself on the local network via mDNS, so in a typical
  home/LAN setup no configuration is needed at all.
* **rdhm-monitor** – a Qt Quick desktop application which discovers agents on
  the network and shows the state of their disks. Agents that cannot be
  discovered (different subnet, VPN etc.) can be added manually. The monitor
  keeps a live connection to each agent, so status changes show up right away.

The agent is written in plain C++ with no Qt dependency and is meant to be
light enough to run on a NAS or a headless server. It works on Linux and
Windows; the monitor builds wherever Qt 6 is available (Linux, Windows and
macOS are covered by CI).

## Building

You will need CMake ≥ 3.16, a C++23 compiler and – for the monitor – Qt 6
(Quick, Gui, Network). On Linux the monitor additionally needs avahi client
libraries for mDNS discovery.

Third party libraries are vendored as submodules, so clone recursively:

```console
git clone --recursive https://github.com/Kicer86/RemoteDiskHealthMonitor.git
cd RemoteDiskHealthMonitor
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Both components are built by default. If you only need one of them (say, the
agent for a server), disable the other with `-DBUILD_AGENT=OFF` or
`-DBUILD_MONITOR=OFF` – this also trims the list of required dependencies
accordingly.

Tests can be run with `ctest --test-dir build`.

## Running the agent

```console
rdhm-agent --name "basement NAS"
```

The name (optional) is what will be shown in the monitor. The agent listens on
port 1630 and needs to be able to run `smartctl`, which usually means root –
on Linux the recommended way is the systemd unit shipped in
`packaging/systemd`. Runtime dependencies on Linux are `smartmontools` and
`util-linux`.

The API is plain HTTP, so the monitor is not the only possible consumer:

```console
curl http://nas:1630/api/v1/status
```

`/api/v1/info`, `/api/v1/disks` and an SSE endpoint at `/api/v1/events` are
also available.

## Packaging

The `packaging/` directory contains files for building Debian, RPM and Arch
packages (separate packages for the agent and the monitor), plus the systemd
service for the agent.

## License

GPLv3, see [LICENSE](LICENSE).
