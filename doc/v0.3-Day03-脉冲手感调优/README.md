# v0.3 Day03 脉冲手感调优

> **冲刺任务**：[15天冲刺计划-v0.3垂直切片](../15天冲刺计划-v0.3垂直切片.md) · Day 3  
> **分支**：`sprint/v0.3`  
> **依赖**：Day 1–2 打击感；P0-B03 脉冲与反弹自伤  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

Day 3 目标：把「射得爽」做成可调参数，并补齐子弹观感与撞墙反弹可控性。

| 项 | 内容 |
|----|------|
| **可调参数** | 射速、冲量、TTL、射程、散射角、物理弹跳/摩擦 → `constants.hpp` |
| **子弹美术** | `bulletsand1.png`（Kenney tanks_redux） |
| **反弹策略** | 最多撞墙 **1 次**；再撞墙销毁；反弹后恢复飞行速度并对齐朝向 |
| **散射** | 发射角 ±`projectile_spread_radians`（默认约 0.04 rad） |

---

## 反弹行为（当前）

1. 发射：沿枪口朝向（可加散射）施加冲量，记录 `m_flight_speed`  
2. **第一次**撞墙 / PhysicsBox：物理材质反弹 → 下一物理帧把速度大小恢复为 `m_flight_speed`，`rotation = velocity.angle()`；开启玩家碰撞（反弹自伤）  
3. **第二次**撞墙：直接 `queue_free`  
4. 全程 `_integrate_forces` 让刚体朝向跟随速度，避免贴图「横着飞」

---

## 涉及文件

| 类型 | 路径 | 说明 |
|------|------|------|
| 修改 | `src/core/constants.hpp` | Day3 脉冲/反弹常量 |
| 修改 | `src/entity/projectile/projectile.hpp/.cpp` | 限弹 1 次、朝向跟随、反弹后速度对齐 |
| 修改 | `src/entity/projectile/projectile_spawner.hpp` | 射速默认接 `projectile_fire_rate` |
| 修改 | `src/entity/level.cpp` | 发射散射 |
| 修改 | `project/scenes/projectiles/bullet.tscn` | 贴图 → `bulletsand1.png` |

---

## 常量一览（默认值）

| 名称 | 值 | 用途 |
|------|-----|------|
| `projectile_fire_rate` | `10.0` | 发/秒 |
| `projectile_impulse` | `1500` | 发射冲量 |
| `projectile_time_to_live` | `2.5` | 存活秒 |
| `projectile_max_travel` | `1000` | 最大飞行距离（像素） |
| `projectile_spread_radians` | `0.04` | 散射半宽；`0` 关散射 |
| `projectile_physics_bounce` | `0.9` | 物理材质弹性 |
| `projectile_physics_friction` | `0.05` | 物理材质摩擦 |
| `projectile_max_wall_bounces` | `1` | 最多反弹次数 |
| `projectile_muzzle_forward_offset` | `12` | 沿射击方向再推出枪口的距离 |
| `ricochet_self_damage_radius` | `120` | 反弹后自伤判定半径 |

---

## 验收对照（Day 3）

| 验收项 | 状态 |
|--------|------|
| 射速/弹速等可在常量集中调节 | ✅ |
| 子弹大贴图（`bulletsand1_outline`） | ✅ 保留尺寸，不缩小 |
| 大贴图从枪口射出不嵌身体 | ✅ Sprite/碰撞前移 + `FiringPoint` + `muzzle_forward_offset` |
| 撞墙最多弹 1 次，再撞消失 | ✅ |
| 反弹后朝向与速度方向一致 | ✅ |
| 反弹后速度不因弹性系数明显变肉 | ✅ 恢复 `m_flight_speed` |
| 轻微散射可感知（可关） | ✅ |
| 命中/击杀/受击反馈仍完整 | ✅（沿用 Day1–2） |

**调参**：改 `constants.hpp` 后重编译 GDExtension；仅换贴图可只重载 Godot。

---

## Git

| 项 | 说明 |
|----|------|
| 建议 tag | `day-03` |
| 提交前缀 | `feat(day3):` |
