# v0.3 Day07–08 冲刺/闪避技能

> **冲刺任务**：[15天冲刺计划-v0.3垂直切片](../15天冲刺计划-v0.3垂直切片.md) · Day 7–8  
> **分支**：`sprint/v0.3`  
> **依赖**：Day 6 第二种敌人（远程弹可躲）  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

玩家主动位移技能：**冲刺/闪避**。

| 项目 | 内容 |
|------|------|
| 按键 | **空格** / **Shift** / 手柄 LB（Joypad 9） |
| 方向 | 最近一次 WASD 方向；静止时沿瞄准朝向 |
| 位移 | 约 **300px** 猛冲（可调常量） |
| 无敌 | 冲刺起约 **0.22s** 短无敌（可躲弹/穿怪） |
| 冷却 | **0.9s**；左上 HUD 心形下方有冷却条 |
| 音效 | 占位 `laser1.ogg`（可换成专用冲刺音） |

冲刺期间碰撞 mask 临时只留墙体，可**穿过敌人**；墙仍挡住。

---

## 手感常量（走测请调这里）

全部在 `src/core/constants.hpp` → `combat::`：

| 常量 | 默认 | 含义 |
|------|------|------|
| `dash_duration` | 0.16 | 冲刺持续（秒） |
| `dash_speed_mult` | 3.8 | 相对移速倍率 |
| `dash_cooldown` | 0.9 | 冷却 |
| `dash_invincibility_duration` | 0.22 | 短无敌 |
| `sfx_player_dash_volume_db` | -2 | 音量 |

距离粗算：`movement_speed × dash_speed_mult × dash_duration` ≈ `500 × 3.8 × 0.16` ≈ **304px**。

---

## 验收对照

| 验收项 | 状态 |
|--------|------|
| Shift/空格能冲刺 | ✅ 构建通过，待你走测 |
| 冲刺中短无敌（能躲远程弹） | ✅ |
| 冷却反馈（HUD 条） | ✅ 心形下方青色条 |
| 有音效 | ✅ 占位 laser1 |
| 手感顺滑 | 🧑 请本地微调常量 |
| 构建通过 | ✅ |

---

## 涉及文件

| 路径 | 说明 |
|------|------|
| `project/project.godot` | 新增 `dash` 输入 |
| `src/core/constants.hpp` | 冲刺常量 + 音效路径 |
| `src/util/input.hpp` | `action::dash` |
| `src/entity/controller/player_controller.cpp` | 检测 dash → 发信号 |
| `src/entity/character/character.*` | `start_invincibility(duration)`；`on_character_movement` 虚化 |
| `src/entity/character/player.*` | 冲刺状态机 / 穿怪 / 短无敌 |
| `src/util/combat_feedback.*` | `play_player_dash` |
| `src/ui/heart_hud.*` | 冷却条绘制 |

---

## 你来走测

1. 重新打开/热加载 Godot（确保读到新 dll + InputMap）
2. WASD 移动中按 **空格** 或 **Shift**：应猛冲一段，HUD 条变空再回满
3. 房 2/3 对侦察远程弹：冲刺穿越弹道应不掉血
4. 站立不动按冲刺：应朝瞄准方向冲
5. 觉得太远/太近/冷却太长 → 改常量后重建

**可选**：把 `path::audio::player_dash` 换成你下的专用冲刺音效。

走测 OK 后建议：`git tag day-07` / `day-08`（或合并打 `day-08`）。
