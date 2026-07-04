# P0-B04 敌人基础 AI — 实现文档

> **任务**：P0-B04 敌人基础 AI（见 [P0-MVP最小闭环开发任务](../任务/P0-MVP最小闭环开发任务.md)）  
> **依赖**：P0-B01 心形血量、P0-L01 单关卡  
> **完成时间**：2026-07  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

本次实现 MVP **M4 敌人**，在 B03 脉冲伤害与 L01 刷点基础上补齐：

- 敌人从 `EnemySpawn*` 标记生成，**直线追击**玩家
- 敌人 **面向** 玩家（与玩家旋转逻辑对称）
- 敌人 **接触** 玩家扣 1 心（B02 无敌帧防连扣）
- 敌人可被 **脉冲** 击杀（复用 B03 `Projectile::on_body_entered`）

---

## 涉及文件一览

| 类型 | 路径 | 说明 |
|------|------|------|
| 修改 | `src/core/constants.hpp` | `enemy_contact_damage_hearts`、`enemy_movement_speed` |
| 修改 | `src/entity/character/enemy.cpp` | 默认心数、移动速度 |
| 修改 | `src/entity/controller/enemy_controller.hpp/cpp` | 追击 + 面向玩家 AI |
| 修改 | `src/entity/character/player.cpp` | 滑动碰撞检测敌人接触扣心 |
| 修改 | `src/entity/level.cpp` | 刷怪时绑定 `EnemyController` |
| 修改 | `project/scenes/characters/enemy.tscn` | mask 含 Player；精灵节点名对齐 |
| 修改 | `project/scenes/characters/player.tscn` | mask 含 NPCs |

---

## 常量一览

| 名称 | 值 | 用途 |
|------|-----|------|
| `combat::enemy_default_hearts` | `3` | 敌人默认心数 |
| `combat::enemy_contact_damage_hearts` | `1` | 接触玩家扣心 |
| `combat::enemy_movement_speed` | `350.0` | 追击速度（低于玩家 500） |

---

## 碰撞层（B04 手工配置，B06 再统一整理）

| 主体 | layer | mask |
|------|-------|------|
| Player | `Player (1)` | `58` = NPCs + Walls + DamageZones + DeathZones |
| Enemy | `NPCs (2)` | `9` = Player + Walls |

---

## 验收对照

| 验收项 | 状态 |
|--------|------|
| 刷点生成敌人并追击 | ✅ |
| 接触扣 1 心，无敌帧不连扣 | ✅ |
| 脉冲击杀后 `queue_free` | ✅ |
| 编译通过 | ✅ |

---

## 后续任务衔接

| 任务 | 关系 |
|------|------|
| P0-B05 + L02 | 监听敌人 `died` 递减计数、胜利判定 |
| P0-B06 | 全场景碰撞矩阵回归 |
