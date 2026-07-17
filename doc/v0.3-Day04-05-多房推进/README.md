# v0.3 Day04–05 多房推进 — 可穿过陷阱 + 三房切换

> **冲刺任务**：[15天冲刺计划-v0.3垂直切片](../15天冲刺计划-v0.3垂直切片.md) · Day 4–5  
> **分支**：`sprint/v0.3`（已编译通过，待本地走测）  
> **依赖**：Day 1–3 打击感；`level1/2/3.tscn` 占位场景  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

| 能力 | 实现 |
|------|------|
| **三房推进** | 房1清怪 → 自动加载房2（保留血量）→ 房3 → Victory 结算 |
| **可穿过陷阱** | 绿 `DamageZone` / 红 `DeathPit` 改为 `Area2D`，走进去才扣血 |
| **围墙** | 四面 `Boundaries`（Walls 层），玩家不能出活动区 |
| **布局差异** | 三关障碍/刷怪点/背景 tint 不同 |

---

## 陷阱改动（红/绿）

| 以前 | 现在 |
|------|------|
| `StaticBody2D` + 玩家 mask 含 16/32 | `Area2D`，玩家 **不挡路** |
| 撞上去才扣血（也挡路） | `overlaps_body` 检测进入 |
| 玩家 `collision_mask = 58` | `collision_mask = 10`（Walls + NPCs） |

- **绿**：扣 1 心（受无敌帧保护）  
- **红**：即死（bypass 无敌）  
- 离开后再进入会再次触发（与旧「首次接触」语义一致）

---

## 多房流程

```text
Main::load_room(0)
  → Level1 清怪 → room_cleared(0) → load_room(1, hearts)
  → Level2 清怪 → room_cleared(1) → load_room(2, hearts)
  → Level3 清怪 → Victory（叙事）
  → R 键 → run_restart → load_room(0)
Defeat 时 R → 仅重置当前房
```

---

## 三关布局摘要

| 关卡 | 敌人 | 灰墙 | 绿陷阱 | 红陷阱 | 备注 |
|------|------|------|--------|--------|------|
| level1 | 1 | 1 | 1 | 1 | 保留紫 PhysicsBox（弹反演示） |
| level2 | 2 | 3 | 2 | 1 | 偏冷色背景 |
| level3 | 3 | 3 | 2 | 1 | 偏暖色；中央通道 + 下方即死区 |

可在 Godot 里继续微调位置；**勿**把碰撞子节点单独拖离父节点。

---

## 涉及文件

| 路径 | 说明 |
|------|------|
| `src/main.cpp/hpp` | 房间加载、`room_cleared` / `run_restart` |
| `src/entity/level.cpp/hpp` | 房号、清怪切房 vs Victory、血量传递 |
| `src/entity/character/player.cpp/hpp` | Area2D 陷阱检测 |
| `src/entity/character/character.*` | `apply_hearts` 公开接口 |
| `src/ui/main_dialog.*` / `heart_hud.cpp` | 切房后重绑 Level / Player |
| `src/core/constants.hpp` | 路径、`player_mask`、`room_count` |
| `project/scenes/levels/level{1,2,3}.tscn` | 布局 + Area2D + Boundaries |
| `project/scenes/characters/player.tscn` | mask = 10 |

---

## 验收对照

| 验收项 | 状态 |
|--------|------|
| 房1清怪自动进房2 | ✅ 代码已接（待走测） |
| 房2→房3→通关叙事 | ✅ |
| 切房保留血量 | ✅ |
| 绿/红可穿过并扣血 | ✅ |
| 灰墙 / 围墙挡路 | ✅ |
| 三关布局可区分 | ✅ |
| 构建通过 | ✅ |

**Day 6 待做**：第二种敌人。

---

## Git

| 项 | 说明 |
|----|------|
| 建议 tag | `day-05`（走测通过后） |
| 前缀 | `feat(day4-5):` |
