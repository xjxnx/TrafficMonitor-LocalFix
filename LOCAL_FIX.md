# TrafficMonitor LocalFix 1.8.6.1

发布标记：`v1.8.6.1-localfix.1`。日期：2026-11-06。

这是基于 zhongyang219/TrafficMonitor 的社区修复试用分支。上游基线为 `930f17533d6098989ebad62f210aa97d75ef174b`（2026-09-27）。

## 修复内容

- 将硬件库替换为官方 LibreHardwareMonitor 0.9.6 / .NET Framework 4.7.2 构建，补齐运行依赖。程序不再携带或释放原来的 WinRing0 / TrafficMonitor.sys。
- 硬件初始化、启停和采样由同一工作线程执行；使用 Windows 事件替代 10ms 标志轮询。
- 退出时等待工作线程结束，并处理同步窗口消息；在保存配置、流量记录和销毁窗口前停止采集。
- 初始化网卡表指针，限制扩容重试并在失败时保留缓冲区所有权；同步刷新请求和网卡访问，修复接口数量基线。
- 跳过空值、NaN 和无穷值；处理空频率列表及过期频率，使用 CPU Total 获取 CPU 总负载，保留三个及以上同名硬盘。
- 将硬件错误弹窗改为日志；设置界面读取传感器列表时不等待繁忙的采集锁。
- 使用独立实例标识并标明修复版；普通启动使用当前用户权限，需要硬件访问时可管理员运行。

## CPU 温度与 PawnIO

CPU 温度需要 [官方 PawnIO 2.2.0](https://github.com/namazso/PawnIO.Setup/releases/tag/2.2.0)。官方安装包 `PawnIO_setup.exe` 的 SHA-256 为：

```text
1f519a22e47187f70a1379a48ca604981c4fcf694f4e65b734aaa74a9fba3032
```

安装后重新启动 TrafficMonitor。未安装或权限不足时，CPU 温度可能显示 `-- °C`，不应被解释为零度。GPU 温度可能通过厂商接口正常读取。

验证机安装官方组件后，驱动处于 Running 状态且数字签名有效。三轮硬件启用、各五次采样、关闭和重建全部通过；CPU 温度为 48.1–50.4°C，CPU 频率约 4.006GHz，RTX 3060 温度约 50–51°C。

## 测试及限制

- 已在 Windows 10 x64、i5-10400、RTX 3060 上完成 Release x64 构建。
- 原生回归覆盖工作线程唤醒和停止、慢采集、关闭时同步消息、网卡扩容和失败所有权。
- 模拟传感器覆盖空值、NaN/无穷、有效零负载、空或过期频率、CPU 总负载、平均温度和重复硬盘名。
- 硬件库资源中未包含 WinRing0 或 .sys；程序分发包无 .sys 文件。
- 进程启动、路径、版本、哈希和主窗口消息响应已验证。完整界面、任务栏交互和长期稳定性仍需进一步验证；未捕获历史崩溃转储。
- 此发布构建使用 VS2019 v142 和上游已有的 `DISABLE_WINDOWS_WEB_EXPERIENCE_DETECTOR` 兼容宏。Windows 11 小组件包检测未启用，不能视为 Windows 11 已完成回归。

## 编译和打包

建议 Visual Studio 2022，安装 C++ 桌面开发、MFC、C++/CLI、Windows SDK 和 .NET Framework 4.7.2 开发工具。进入 x64 Developer PowerShell：

```powershell
msbuild TrafficMonitor.sln /m /p:Configuration=Release /p:Platform=x64
.\tests\run-tests.ps1
py -3 .\tools\package-release.py --output-dir .\release-artifacts
```

`OpenHardwareMonitorApi` 构建后会复制 `runtime` 目录中的依赖和程序配置。打包脚本从已提交源码及构建产物生成通用便携包、源码 ZIP、构建清单和 SHA-256 校验值，避免混入个人配置或日志。

## 第三方来源

- TrafficMonitor：https://github.com/zhongyang219/TrafficMonitor ，保留原 LICENSE 和 LICENSE_CN。
- LibreHardwareMonitor：https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/v0.9.6 。官方 ZIP SHA-256：`086d9f1b5a99e643edc2cfaaac16051685b551e4c5ac0b32a57c58c0e529c001`。
- PawnIO 驱动：https://github.com/namazso/PawnIO 。安装程序独立从官方发布页下载。
- PawnIO.Modules：https://github.com/namazso/PawnIO.Modules 。模块及其他第三方声明保留在 `licenses` 目录。

LibreHardwareMonitor 二进制未作修改，采用 MPL 2.0；嵌入模块及运行依赖的许可证见第三方声明。
