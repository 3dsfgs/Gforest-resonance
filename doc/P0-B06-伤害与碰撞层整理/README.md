# P0-B06 伤害与碰撞层整理 — 实现文档

> **任务**：P0-B06 伤害与碰撞层整理（见 [P0-MVP最小闭环开发任务](../任务/P0-MVP最小闭环开发任务.md)）  
> **依赖**：P0-B03 脉冲攻击、P0-B04 敌人 AI  
> **完成时间**：2026-07  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

对照任务书碰撞矩阵，核对并固化各场景碰撞层配置；在 `constants.hpp` 增加 `collision` 命名空间，使 `LayerID` 与场景数值一一对应，便于后续技能/陷阱扩展。

B03–B05 联调阶段碰撞关系已基本正确，本次以**文档化 + 显式配置**为主，无玩法逻辑变更。

---

## 碰撞矩阵（目标态）

| 主体 | layer (`LayerID`) | mask 应包含 | `project.godot` 层名 | 场景值 |
|------|-------------------|-------------|----------------------|--------|
| Player | `Player` (1) | Walls, DamageZones, DeathZones, NPCs | `player` | layer 1, mask **58** |
| Enemy | `NPCs` (2) | Walls, Player | `npcs` | layer 2, mask **9** |
| Projectile | `Projectiles` (4) | Walls, NPCs | `projectiles` | layer 4, mask **10**（见下） |
| 围墙 | `Walls` (8) | — | `walls` | layer 8, mask 0 |
| 伤害区 | `DamageZones` (16) | — | `damage_zones` | layer 16, mask 0 |
| 即死区 | `DeathZones` (32) | — | `death_zones` | layer 32, mask 0 |

### B03 反弹扩展

`bullet.tscn` 使用 `collision::projectile_mask_ricochet` = **74**（在基础 mask 10 上额外含 `PhysicsObjects`），以便 DebugZones 内 `PhysicsBox` 可反弹。反弹后 `Projectile::on_body_entered` 运行时追加 `Player` 层，实现近距自伤。

---

## 涉及文件

| 类型 | 路径 | 说明 |
|------|------|------|
| 修改 | `src/core/constants.hpp` | `collision` 命名空间与矩阵注释 |
| 修改 | `project/scenes/characters/player.tscn` | 显式 `collision_layer = 1` |
| 核对 | `project/scenes/characters/enemy.tscn` | layer 2, mask 9 ✓ |
| 核对 | `project/scenes/projectiles/bullet.tscn` | layer 4, mask 74 ✓ |
| 核对 | `project/scenes/levels/level1.tscn` | 围墙/调试区 layer 正确 ✓ |
| 修改 | `src/entity/projectile/projectile.cpp` | 反弹后开 Player mask 改用 `collision::player_layer` |

---

## 常量一览

| 名称 | 值 | 用途 |
|------|-----|------|
| `collision::player_layer` | 1 | 玩家碰撞层 |
| `collision::player_mask` | 58 | 玩家 mask |
| `collision::enemy_layer` | 2 | 敌人碰撞层 |
| `collision::enemy_mask` | 9 | 敌人 mask |
| `collision::projectile_layer` | 4 | 子弹碰撞层 |
| `collision::projectile_mask` | 10 | 子弹基础 mask |
| `collision::projectile_mask_ricochet` | 74 | 含 PhysicsObjects 的子弹 mask |

---

## 伤害路径（与碰撞层关系）

| 伤害来源 | 检测方式 | 依赖层 |
|----------|----------|--------|
| 脉冲命中敌人 | `Projectile::on_body_entered` → `Enemy` | Projectile mask 含 NPCs |
| 敌人接触玩家 | `Player::process_slide_collisions` → `Enemy` | Player mask 含 NPCs |
| 伤害区 / 即死区 | `Player::handle_zone_contact` 读 zone `collision_layer` | Player mask 含 Damage/DeathZones |
| 子弹穿墙 | 物理反弹，不穿透 | Projectile mask 含 Walls |
| 反弹自伤 | 反弹后追加 Player mask + 距离判定 | 运行时 mask 扩展 |

---

## 验收对照

| 验收项 | 状态 |
|--------|------|
| 敌人碰玩家扣心 | ✅ |
| 子弹不打玩家（首发） | ✅ |
| 子弹不穿墙无限飞 | ✅ |
| 碰撞层与 `LayerID` 一致 | ✅ |
| 矩阵文档化于 `constants.hpp` | ✅ |

---

## 后续扩展提示

- 新陷阱：在 `LayerID` 增加层位，于 `project.godot` 注册名称，并更新 `collision` 矩阵表。
- 敌方弹幕：复用 `Projectiles` 层或新增 `EnemyProjectiles` 层，mask 仅含 Player + Walls。
- 友军 NPC：使用 `NPCs` 层时注意不要与敌人共用接触伤害逻辑，可拆子类或加 team 标记。
