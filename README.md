# TrafficMonitor LocalFix

[English](README_en-us.md) | [上游项目](https://github.com/zhongyang219/TrafficMonitor) | [下载修复版](https://github.com/xjxnx/TrafficMonitor-LocalFix/releases/tag/v1.8.6.1-localfix.1)

基于 TrafficMonitor 的硬件温度监控修复分支，保留网速、CPU/内存占用、任务栏显示和皮肤功能，修复硬件采集、退出线程和网卡变化时的一批稳定性问题。

当前版本：**1.8.6.1 / Local Fix 2026-11-06**。这是社区修复试用版，基于上游提交 `930f17533d6098989ebad62f210aa97d75ef174b`，并非上游作者的正式发行版。

## 下载和使用

从 [Release 页面](https://github.com/xjxnx/TrafficMonitor-LocalFix/releases/tag/v1.8.6.1-localfix.1) 下载：

- `TrafficMonitor-LocalFix-v1.8.6.1-localfix.1-windows-x64.zip`：Windows x64 便携包，解压即可使用。
- `TrafficMonitor-LocalFix-v1.8.6.1-localfix.1-source.zip`：对应完整源码。
- `SHA256SUMS.txt` 和 `build-info.json`：下载校验值与构建来源。

1. 退出已经运行的 TrafficMonitor。
2. 将便携包完整解压到可写文件夹，运行 `TrafficMonitor.exe`，保留旁边的 DLL 和配置文件。
3. CPU 温度需要另行安装 [官方 PawnIO](https://github.com/namazso/PawnIO.Setup/releases/tag/2.2.0)。安装后重新启动 TrafficMonitor；如果仍显示 `-- °C`，尝试以管理员身份运行。
4. 需要开机启动时，可在程序设置中启用。替换现有安装前，请备份自己的配置和历史流量数据。

发布包使用通用默认配置，不包含使用者的网卡名称、流量记录、错误日志或本机安装路径。PawnIO 安装程序单独从官方渠道下载，本项目不捆绑系统驱动安装器。

## 主要修复

- 硬件库更新至 LibreHardwareMonitor **0.9.6**，移除旧 WinRing0 后端，程序包不携带 `.sys` 文件。
- 硬件初始化、重新配置和关闭归到同一采集线程；使用 Windows 事件唤醒，改善退出和设置时的阻塞问题。
- 修复网卡表指针初始化、扩容失败处理、刷新同步及接口数量变化检测。
- 处理缺失或无效传感器、空频率列表、CPU 总负载选择和重复硬盘名称。
- 硬件错误写入日志，减少反复弹出阻塞对话框；传感器设置列表不再等待忙碌的采集线程。

## 验证范围

已在 Windows 10 x64、Intel i5-10400、NVIDIA RTX 3060 上完成构建和回归。安装官方 PawnIO 2.2.0 后，15 次 CPU 温度采集为 **48.1–50.4°C**，CPU 频率约 4.006GHz，显卡温度约 50–51°C。

线程、网卡缓冲区、模拟传感器和多轮硬件启停测试已通过。完整任务栏交互和 Windows 11 兼容性尚未完成验证，也没有证据证明所有历史崩溃原因均已消除。详细范围见 [LOCAL_FIX.md](LOCAL_FIX.md)。

## 编译

建议使用 Visual Studio 2022，安装 C++ 桌面开发、MFC、C++/CLI、Windows SDK 和 .NET Framework 4.7.2 开发工具。在 x64 Developer PowerShell 中运行：

```powershell
msbuild TrafficMonitor.sln /m /p:Configuration=Release /p:Platform=x64
.\tests\run-tests.ps1
py -3 .\tools\package-release.py --output-dir .\release-artifacts
```

编译输出为 `Bin/x64/Release`。实机温度测试需要支持的硬件和相应驱动；`hardware_smoke` 会输出传感器实际值。保留的上游 Lite CI 通过 GitHub Actions 手动触发。

## 来源和许可证

原项目：[zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)。原始说明保留在 [README_UPSTREAM.md](README_UPSTREAM.md)。

保留上游 [LICENSE](LICENSE)、[LICENSE_CN](LICENSE_CN) 和第三方 [许可证与声明](licenses)。LibreHardwareMonitor 使用官方 v0.9.6 二进制；其源码及 PawnIO 模块来源见 [LOCAL_FIX.md](LOCAL_FIX.md)。
