# TrafficMonitor LocalFix

[简体中文](README.md) | [Upstream](https://github.com/zhongyang219/TrafficMonitor) | [Download](https://github.com/xjxnx/TrafficMonitor-LocalFix/releases/tag/v1.8.6.1-localfix.1)

A community trial build of TrafficMonitor focused on hardware monitoring, shutdown handling, network table ownership and sensor validation. Based on upstream commit `930f17533d6098989ebad62f210aa97d75ef174b`.

Version: **1.8.6.1 / Local Fix 2026-11-06**. Download the Windows x64 portable ZIP from Releases, extract the entire folder and run `TrafficMonitor.exe`. Keep all accompanying DLLs and configuration files.

CPU temperature requires [official PawnIO](https://github.com/namazso/PawnIO.Setup/releases/tag/2.2.0). Restart TrafficMonitor after installing it; run as administrator if access to the hardware requires it. The application package contains no `.sys` files and does not bundle the driver installer. It ships with generic configuration rather than user logs or machine settings.

LibreHardwareMonitor has been updated to 0.9.6, replacing the old WinRing0 backend. Other fixes cover worker shutdown and synchronization, network buffer resizing and interface changes, invalid sensors, missing clock values, total CPU load, duplicate disk names and blocking error dialogs.

Tested on Windows 10 x64 with an Intel i5-10400 and NVIDIA RTX 3060. After installing PawnIO 2.2.0, all 15 CPU samples returned temperatures between 48.1 and 50.4 degrees Celsius. Windows 11 and complete taskbar interaction have not been validated. See [LOCAL_FIX.md](LOCAL_FIX.md) for limitations and build instructions.

The upstream [LICENSE](LICENSE), [LICENSE_CN](LICENSE_CN), [third-party notices](licenses) and [original English README](README_UPSTREAM_en-us.md) are retained. This is not an official upstream release.
