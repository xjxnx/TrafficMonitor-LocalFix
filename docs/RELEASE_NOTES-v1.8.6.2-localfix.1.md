# TrafficMonitor LocalFix 1.8.6.2

修复显卡驱动恢复后，任务栏 CPU 和显卡温度可能一直显示 `-- °C` 的问题。

- 逐台设备采集。显卡更新失败时，CPU 仍可提供本次采样的温度。
- 连续三次采集失败后，程序会重新打开采集器。故障持续时，两次重试至少间隔一分钟，同一次故障只记录一次。
- 已通过模拟设备故障与恢复测试。在 Windows 10 x64 上使用 i5-10400、RTX 3060 和 PawnIO 2.2.0 连续采样 15 次，CPU 为 46.8–48.3°C，显卡为 50–51°C，已确认任务栏同时显示两项读数。

**下载与升级**

下载 `TrafficMonitor-LocalFix-v1.8.6.2-localfix.1-windows-x64.zip`，完整解压后运行 `TrafficMonitor.exe`。从 1.8.6.1 升级时，先退出程序，保留自己的配置和流量历史，替换 `TrafficMonitor.exe` 与 `OpenHardwareMonitorApi.dll` 即可。可在主程序的文件属性中确认版本为 `1.8.6.2`，“关于”窗口仍沿用原修复分支的日期。

附件还包含对应的源码 ZIP、构建清单 `build-info.json` 和校验文件 `SHA256SUMS.txt`。
