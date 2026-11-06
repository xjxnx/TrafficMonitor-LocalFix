# TrafficMonitor LocalFix 1.8.6.1

Windows x64 修复试用版，基于上游 TrafficMonitor 提交 `930f17533d6098989ebad62f210aa97d75ef174b`。

## 下载

- `TrafficMonitor-LocalFix-v1.8.6.1-localfix.1-windows-x64.zip`：免安装便携包。完整解压后运行 `TrafficMonitor.exe`，保留所有 DLL。
- `TrafficMonitor-LocalFix-v1.8.6.1-localfix.1-source.zip`：对应源码。
- `SHA256SUMS.txt`、`build-info.json`：校验值与构建来源。

## 修复

- 硬件库升级至 LibreHardwareMonitor 0.9.6，移除旧 WinRing0 后端。
- 改善硬件初始化、退出等待和线程同步。
- 修复网卡缓冲区初始化、扩容失败、刷新同步及接口变化检测。
- 修复无效传感器、CPU 总负载选择、空频率和同名硬盘处理。
- 硬件异常写入日志，减少阻塞弹窗与设置界面的等待。

## CPU 温度

需要单独安装 [官方 PawnIO 2.2.0](https://github.com/namazso/PawnIO.Setup/releases/tag/2.2.0)，安装后重新启动软件；必要时以管理员身份运行。程序 ZIP 不捆绑系统驱动安装器，且不包含 `.sys` 文件。

## 已验证

Windows 10 x64、i5-10400、RTX 3060。线程、网卡、模拟传感器回归及三轮实机硬件启停通过。安装官方 PawnIO 后，15 次 CPU 温度采集为 48.1–50.4°C，显卡温度约 50–51°C。

这是社区修复试用版；完整任务栏交互、Windows 11 和所有历史崩溃原因尚未完成验证。发布包采用通用配置，不包含个人网卡设置、流量记录或错误日志。上游与第三方许可证已保留。
