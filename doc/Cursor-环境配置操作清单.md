# godot-roguelite — Cursor 环境配置操作清单

> 参考：[项目 Wiki - CMake Configuration](https://github.com/vorlac/godot-roguelite/wiki#cmake-configuration)

本文档说明如何在 **Windows + Cursor** 下完成本项目的首次构建与调试配置。

---

## 当前环境状态（2026-07-03）

| 项目 | 状态 |
|------|------|
| Git 子模块 | 已初始化 |
| Godot 调试编辑器 | 已构建 |
| vcpkg 依赖 (fmt/spdlog) | 已安装 |
| GDExtension Debug DLL | 已生成：`project/bin/roguelite.windows.64.debug.dll` |
| CMake 构建目录 | `.out/build/msvc-debug` |
| IntelliSense | `compile_commands.json` 已复制到项目根目录 |

**下一步**：在 Cursor 中按 `F5`，选择 **`editor (msvc | windows)`** 首次启动编辑器。

---
---

## 一、项目是什么

| 组件 | 路径 | 说明 |
|------|------|------|
| C++ 游戏逻辑 | `src/` | Player、Level、Projectile 等 GDExtension 代码 |
| Godot 项目 | `project/` | 场景、UI、`roguelite.gdextension` |
| Godot 引擎源码 | `extern/godot-engine` | SCons 构建调试用编辑器（仅首次需编译） |
| C++ 绑定 | `extern/godot-cpp` | godot-cpp，由 CMake 自动编译 |
| 包管理 | `extern/vcpkg` | 管理 `fmt`、`spdlog` 等第三方库 |
| 构建产物 | `project/bin/` | GDExtension `.dll` 输出目录 |

CMake 首次 **Configure** 会自动完成：初始化 submodule → bootstrap vcpkg →（如需）SCons 编译 Godot 调试编辑器 → vcpkg 编译依赖 → 编译 godot-cpp 与本项目 DLL。

---

## 二、前置依赖检查

在 PowerShell 中逐项确认：

```powershell
git --version
python --version
scons --version
```

本机还需已安装：

- **Visual Studio 2022**（含 MSVC C++ 工具集、CMake 工具、Clang 工具附加组件）
- 本仓库当前检测到 VS 安装路径：`D:\vs2022_Professional`

若 `cmake`、`ninja` 不在系统 PATH 中，可使用 VS 自带路径（已在 `.vscode/settings.json` 中配置）：

```
D:\vs2022_Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
D:\vs2022_Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe
```

缺少 `scons` 时：

```powershell
pip install scons
```

---

## 三、克隆与子模块（仅首次）

> 若已 clone 且 `extern/` 下三个子目录均有内容，可跳过。

```powershell
git clone --recursive https://github.com/vorlac/godot-roguelite.git
# 或已 clone 后补全子模块：
git submodule update --init --recursive
```

**不要用 ZIP 下载**，否则 `extern/` 子模块为空，构建必然失败。

---

## 四、Cursor 扩展安装

打开项目根目录后，按提示安装推荐扩展（见 `.vscode/extensions.json`）：

| 扩展 ID | 用途 |
|---------|------|
| `ms-vscode.cpptools` | C/C++ IntelliSense |
| `ms-vscode.cmake-tools` | CMake 配置与构建 |
| `xaver.clang-format` | 保存时自动格式化（可选） |
| `vadimcn.vscode-lldb` | LLDB 调试（可选，MSVC 调试可用 cppvsdbg） |

---

## 五、CMake 配置（Cursor 图形界面）

1. 用 Cursor **打开项目根目录**（含 `CMakeLists.txt` 的文件夹）
2. `Ctrl+Shift+P` → **CMake: Select Configure Preset**
3. 选择 **`MSVC Debug`**（preset 名：`msvc-debug`）
4. `Ctrl+Shift+P` → **CMake: Configure**（或状态栏点 Configure）
5. 等待配置完成（首次可能 10–30 分钟：vcpkg 编译依赖 + godot-cpp）
6. `Ctrl+Shift+P` → **CMake: Build**（或状态栏 Build）

### 构建成功标志

`project/bin/` 下应出现：

```
roguelite.windows.64.debug.dll
```

`project/roguelite.gdextension` 中已指向该文件名。

### 常见问题

| 现象 | 原因与处理 |
|------|-----------|
| DLL 名为 `roguelite.windows.64..dll`（中间缺 debug） | 未使用带 build type 的 preset；重新选 `msvc-debug` 并清缓存后 configure |
| DLL 在 `project/bin/Debug/` 子目录 | 误用 Visual Studio 多配置生成器；应使用 Ninja + preset（见 `CMakePresets.json`） |
| `scons not found` | `pip install scons`，重启终端 |
| `cl` 找不到 | 在 **Developer PowerShell for VS 2022** 中构建，或确保 CMake Tools 使用 VS 工具链 |
| Configure 卡在 Godot 引擎编译 | 首次 SCons 编译引擎耗时很长，属正常现象，仅需一次 |

---

## 六、命令行构建（推荐：一键脚本）

本仓库提供 `scripts/configure-and-build.bat`，会自动加载 VS 开发者环境并执行 configure + build（解决 Cursor 终端中 `cl` 不在 PATH 的问题）。

```bat
:: 首次或需要清缓存时
scripts\configure-and-build.bat clean

:: 日常增量构建
scripts\configure-and-build.bat
```

脚本默认 VS 路径为 `D:\vs2022_Professional`；若你的安装位置不同，请编辑脚本顶部的 `VS_ROOT`。

### 手动命令行（备选）

在 **Developer Command Prompt for VS 2022**（`VsDevCmd.bat` 已加载的环境）中：

```powershell
cd "D:\Makera_Ra\MY_homepage_1\game_demo_2026\godot-roguelite"
cmake --preset msvc-debug
cmake --build .out/build/msvc-debug
```

验证：

```powershell
Test-Path "project\bin\roguelite.windows.64.debug.dll"
```

---

## 七、首次运行与调试

> Wiki 要求：**第一次必须用编辑器模式启动**，让 Godot 处理并缓存资源文件。

### 推荐顺序

1. 先运行 **`editor (msvc | windows)`**（`.vscode/launch.json`）
2. 编辑器成功打开 `project/` 后可关闭
3. 之后可用 **`project (msvc | windows)`** 独立运行游戏

### 启动配置说明

| 配置名 | 作用 |
|--------|------|
| `editor (msvc | windows)` | 以编辑器模式启动 Godot 调试版 |
| `project (msvc | windows)` | 直接运行游戏（需先完成上一步） |
| `editor (lldb)` / `project (lldb)` | 使用 CodeLLDB 调试（跨平台） |

调试器会加载自编译的 `extern/godot-engine/bin/godot.windows.editor.dev.x86_64.exe`，可在 C++ 扩展代码与引擎源码中下断点。

---

## 八、日常开发工作流

```
修改 src/*.cpp
    → CMake: Build（或保存后自动构建，若已开启）
    → 重启 Godot / 重新运行 launch 配置
    → project/bin/roguelite.windows.64.debug.dll 被加载
```

- 新增 C++ 源文件：放入 `src/`，CMake 会通过 `GLOB_RECURSE` 自动检测并触发重新配置
- 新增第三方库：编辑 `vcpkg.json` → Configure 时按 vcpkg 提示更新 `CMakeLists.txt` 的 `find_package` 与 `target_link_libraries`
- 格式化：`Ctrl+Shift+I`（需 clang-format ≥ 16，或按 Wiki 调整 `.clang-format`）

---

## 九、目录与索引说明

`.cursorignore` 已排除 `extern/`、`.out/`、`project/bin/` 等大体积路径，Cursor AI 索引聚焦 `src/` 与 `project/` 游戏代码。查阅引擎 API 时可临时注释对应 ignore 规则。

---

## 十、检查清单（可打印勾选）

- [ ] Git 子模块 `extern/godot-engine`、`extern/godot-cpp`、`extern/vcpkg` 已初始化
- [ ] Python + scons 可用
- [ ] VS 2022 + CMake Tools + Ninja 可用
- [ ] Cursor 推荐扩展已安装
- [ ] CMake Preset 选为 **MSVC Debug**
- [ ] Configure 无报错
- [ ] Build 成功，`project/bin/roguelite.windows.64.debug.dll` 存在
- [ ] 首次以 **editor (msvc | windows)** 启动成功
- [ ] 能以 **project (msvc | windows)** 独立运行

---

## 相关链接

- [项目 Wiki 首页](https://github.com/vorlac/godot-roguelite/wiki)
- [Windows 依赖安装](https://github.com/vorlac/godot-roguelite/wiki/Windows-10-%E2%80%90-How-to-install-all-of-the-required-dependencies)
- [CMake 配置说明](https://github.com/vorlac/godot-roguelite/wiki#cmake-configuration)
- [常见问题](https://github.com/vorlac/godot-roguelite/wiki/Common-issues-troubleshooting)
