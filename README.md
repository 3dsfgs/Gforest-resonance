# Gforest Resonance（林间回响）

一次没做成完整游戏的独立项目。源码与 **Release 可执行包** 一并开源，当作失败经验的切片留下来。

这不是空仓库示意，也不是成品。它停在 **可玩的 3D 正交灰盒战斗原型**：核心循环已经能打，关卡、叙事、打磨和产品化没有走完。

> 想先上手玩：打开本仓库 **[Releases](../../releases)** 下载打包版本。  
> 想看实现：克隆本仓库源码。  
> 工程骨架来自上游开源项目 **[vorlac/godot-roguelite](https://github.com/vorlac/godot-roguelite)**。

---

## About

**Gforest Resonance** 是用 Godot 4 + C++ GDExtension 做的 3D 正交动作原型。主题是「压力积攒 → 回弹释放」，第一关设想为寂静林缘（Silent Edge）。作者在验证完移动、近战、闪避、格挡、压力与回弹后停止开发，把仓库和 Release 公开，供学习 GDExtension 与复盘独立游戏范围控制。

| | |
|---|---|
| 状态 | 已停止开发（归档切片，非维护中产品） |
| 引擎 | Godot 4.5（子模块锁定）；C++23 GDExtension |
| 视角 | 3D 正交 |
| 可玩范围 | 一张灰盒试炼房 |
| 语言 | 游戏逻辑 C++；场景 Godot；文档中文 |
| 上游 | [godot-roguelite](https://github.com/vorlac/godot-roguelite)（MIT） |
---

## 这是什么

计划中的游戏是 3D 正交视角动作原型，战斗围绕 **Pressure（压力）** 与 **Rebound（回弹）**。第一关文档里叫 **寂静林缘**。

实际落地更窄：一张灰盒战斗房，用来验证移动、攻击、闪避、格挡、压力、回弹。玩法主要在 `src/` 的原生代码里，场景与视觉在 `project/`。

它失败在「没有变成完整游戏」，不失败在「什么都没写」。

### 与上游项目的关系

本仓库从 [godot-roguelite](https://github.com/vorlac/godot-roguelite) 起步（Godot 4 GDExtension C++ 模板：CMake、vcpkg、godot-cpp、调试配置）。本项目在其上做了 2D → 3D（`CharacterBody3D`、正交相机、近战判定）以及压力/回弹循环。

- 上游仓库：https://github.com/vorlac/godot-roguelite
- 上游 Wiki（构建说明仍有参考价值）：https://github.com/vorlac/godot-roguelite/wiki
- 若你只想学干净的 GDExtension 工程脚手架，优先看上游；若你想看模板如何改成 3D 近战循环，看本仓库 `src/entity/`。

---

## 已经做成的部分

### 战斗循环（核心，已可玩）

| 系统 | 状态 | 说明 |
|------|------|------|
| 移动 / 朝向 | 完成 | WASD；鼠标射线打到地面决定朝向；手柄摇杆可用 |
| 近战攻击 | 完成 | 扇形命中、冷却、受击硬直与击退 |
| 闪避 | 完成 | 短无敌帧 + 冷却 |
| 格挡 | 完成 | 格挡成功不受伤，积攒 Pressure |
| 压力 / 回弹 | 完成 | 压力满后范围冲击，并强化下一次攻击 |
| 生命 / 死亡 | 完成 | 玩家与敌人共用角色基类；死亡后需重开场景 |
| 敌人 AI | 完成（占位） | 两名敌人追逐玩家，近身攻击 |
| HUD | 完成（运行时生成） | HP、压力条、存活敌人数、状态提示 |

操作（键鼠）：

- **WASD**：移动
- **鼠标**：瞄准朝向
- **鼠标左键 / J**：攻击
- **Shift / K**：闪避
- **空格 / L**：格挡（按住）
- **E / I**：压力满时回弹

试炼房启动时状态栏会显示 `Silent Edge Test Room`。

### 场景与表现

- `project/scenes/levels/level1.tscn`：灰盒房间（地面、围墙、灯）、物理测试箱
- 玩家 / 敌人：胶囊体占位 + 攻击扇 / 回弹环可视化
- 正交相机由 C++ 在关卡内挂载
- 接入 [Terrain3D](https://github.com/TokisanGames/Terrain3D) 作地形背景（当前 **碰撞关闭**，战斗仍走灰盒地板）
- `project/demo/`：Terrain3D 官方示例，便于对照地形工作流

### 工程

- Godot **4.5** 绑定（`extern/godot-cpp`、`extern/godot-engine`）+ CMake + vcpkg（`fmt` / `spdlog`）
- 游戏逻辑几乎全在原生代码：`GDCLASS`、信号宏、场景预加载、游戏内 Console 单例
- VS / VS Code 调试配置、`compile_commands.json` 导出
- 中文说明：`doc/项目结构基础介绍.md`、`doc/Cursor-环境配置操作清单.md`

### 设计文档（未全部实现）

`doc/` 里还有关卡与玩法方案（阅读顺序见 `doc/README_阅读顺序.md`）。那些文档描述的是 **目标游戏**，不是当前可玩范围。

---

## 明确没做成的部分

- 没有主菜单 / 设置 / 暂停、没有存档、没有完整第一关与叙事
- 没有角色动画、音效、粒子与正式美术；胶囊体 + 灰盒
- Terrain3D 没有真正参与战斗碰撞与导航
- 投射物系统仍留在代码/场景里，当前战斗已改成近战判定
- CMake 库名、Godot 项目显示名仍是模板遗留的 `roguelite` / `Roguelite`
- `project.godot` 的 `features` 写过 `4.7`，子模块锁定的是 **4.5**——从源码构建请以 4.5 为准
- 这不是维护中的产品；不承诺路线图

---

## 仓库结构

```
src/                 C++ GDExtension：Main、Level、Character、Controller、Camera、UI
project/             Godot 工程：场景、Terrain3D、资源、gdextension 输出目录
  scenes/levels/     寂静林缘灰盒房
  scenes/characters/ 玩家 / 敌人占位体
  addons/terrain_3d/ Terrain3D 插件
  demo/              Terrain3D 示例场景
cmake/               CMake 模块（vcpkg、godot-cpp、引擎构建）
extern/              子模块：godot-engine、godot-cpp、vcpkg
doc/                 中文设计与环境说明
```

运行时大致节点关系：

```
Main                         src/main.cpp
└── MainDialog               主视口 + 控制台
    └── SubViewport
        └── Level            正交相机、玩家、敌人、HUD
            ├── Player + PlayerController
            └── Enemy  + EnemyController
```

---

## 玩 Release（推荐大多数人）

从源码编译需要拉引擎、编 godot-cpp 和扩展 DLL，第一次往往以小时计。

1. 打开本仓库 **[Releases](../../releases)**
2. 下载对应平台的打包文件
3. 解压运行

Release 与当前默认分支可能不是同一提交，以发布说明为准。这是「做到这里就停了」的可执行切片，不是官方支持版本。

---

## 从源码构建

**不要用 ZIP 下载。** `extern/` 是 git 子模块，ZIP 里是空的，CMake 会失败。

```bash
git clone --recursive <本仓库 URL>
cd Gforest-resonance
git submodule update --init --recursive
```

前置：Git、Python、SCons、CMake/Ninja、MSVC（Windows 上 Visual Studio 2022 C++ 工具集）。Windows + Cursor 的逐步清单见 `doc/Cursor-环境配置操作清单.md`。通用 CMake 流程仍可参考上游 Wiki：https://github.com/vorlac/godot-roguelite/wiki

概要：

1. CMake Configure（会 bootstrap vcpkg、按需编调试版 Godot、编 godot-cpp）
2. 编译扩展，产出到 `project/bin/`（库名仍为 `roguelite`）
3. 用编好的 Godot 编辑器打开 `project/`，运行主场景 `res://main.tscn`

---

## 为什么开源（失败总结）

1. **范围太大。** 引擎源码进仓库、C++ 全逻辑、3D 地形、完整关卡同时推进，验证玩法的反馈周期被拉得很长。
2. **模板很强，产品很弱。** 构建、绑定、调试、信号宏都比玩法成熟；玩家能感受到的只有灰盒房。
3. **3D 正交 + 原生扩展成本高。** 从 2D 模板迁到 `CharacterBody3D`、相机、近战判定值得留下，但不该和「第一款必须做完的游戏」绑在同一条死线上。
4. **停在可玩原型是合理的止损。** 循环已经能打，再往下是内容生产。没有把半成品藏起来，而是连 Release 一起公开。

---

## 许可与致谢

- 本仓库游戏与扩展代码：以根目录 [LICENSE](./LICENSE) 为准（MIT）
- 工程骨架：[godot-roguelite](https://github.com/vorlac/godot-roguelite)（MIT）— https://github.com/vorlac/godot-roguelite
- Terrain3D：`project/addons/terrain_3d/`（MIT）— https://github.com/TokisanGames/Terrain3D
- 模板遗留 2D 美术/音频：`project/assets/art/`、`project/assets/audio/` 为 CC0，致谢 [Kenney](https://kenney.nl)

Godot 引擎与 godot-cpp 各有其许可证，见对应子模块。

---

## 贡献

仓库按 **归档切片** 开放：修明显崩溃、补文档、把构建说明写清楚都欢迎。不承诺会做成完整游戏，也不承诺跟新版本 Godot。

如果你从这次失败里少走弯路，这份开源就值了。
