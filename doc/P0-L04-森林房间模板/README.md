# P0-L04 森林房间模板 — 实现文档

> **任务**：P0-L04（见 [P0-v0.2-视觉与性能开发任务](../任务/P0-v0.2-视觉与性能开发任务.md)）  
> **策略转向**：由「大地图 + 视差滚动」改为 **《元气骑士》式单房模板**（静态底图 + 锁镜头 + 森林氛围）  
> **依赖**：P0-L01 ✅，P0-L07 ✅  
> **实现时间**：2026-07  
> **关联**：[P0-L01 单关卡场景搭建](../P0-L01-单关卡场景搭建/README.md) · [走测与美术交接](./走测与美术交接.md) · [源码与函数说明](./源码与函数说明.md)

---

## 概述

在 L01 单房 2000×1500 玩法尺寸不变的前提下，将 `level1.tscn` 升级为 **Room Template v0（第一间森林空地）**：

- 三层 parallax 静态贴图（远景地面 / 雾 / 近景框）
- `RoomDecor`：墙带、门框、四角火把点光（占位）
- 相机锁边（元气骑士式当前房取景）
- `Level` 挂于 `MainSubViewport`（与 HUD 分层）

> **视觉统一与占位替换**不在 L04 范围，见 [走测与美术交接](./走测与美术交接.md) 及 P0-L06。

---

## 1. 设计决策

### 为何放弃大地图视差

| 原方案 | 问题 |
|--------|------|
| 全图 `ParallaxBackground` + 多层平铺 | 非无缝 AI 素材拼接穿帮；地面与视差层差速错位 |
| 2000×1500 单图拉伸 + 滚动雾层 | 镜头扫全图时露边、硬切 |

### 新方案（Room Template v0）

借鉴 **元气骑士** 的空间结构，保留本项目的 **治愈节奏**：

- **一串有呼吸感的房间**，而非无限滚动大地图
- 每房：**静态地板** + **雾 overlay** + **近景框** + **四面墙碰撞** + **环境装饰占位** + **刷点**
- **相机限制在房间内**（`Camera2D` limit 对齐 `playable_width/height`）
- 森林气质靠房主题美术 + 雾层；P1 再补 Shader 丁达尔光

> `level1.tscn` 在 v0.2 即第一间森林房；P1 扩展为多房连通与清怪开门。

---

## 2. 涉及文件

| 类型 | 路径 | 说明 |
|------|------|------|
| 场景 | `project/scenes/levels/level1.tscn` | Room Template v0 节点树 |
| 资源 | `project/assets/parallax/parallax_far.png` | 远景地面（1254×1254，拉伸至 2000×1500） |
| 资源 | `project/assets/parallax/parallax_mid.png` | 静态雾 overlay（1024×1024，alpha≈0.38） |
| 资源 | `project/assets/parallax/parallax_near.png` | 近景框（1024×1024，alpha≈0.72） |
| 常量 | `src/core/constants.hpp` | 房间尺寸、节点名、`path::room_assets`、缩放系数 |
| 逻辑 | `src/entity/level.cpp` | `apply_room_camera_limits()` |
| 入口 | `src/main.cpp` | `Level` 挂到 `MainSubViewport` |
| 角色 | `src/entity/character/character.cpp` | `PlayerCamera::make_current()` |

---

## 3. 场景节点（Room Template v0）

```
Level1 (Level)
├── Ground              ColorRect 森林绿 fallback
├── GroundSprite        parallax_far 远景地面
├── FogOverlay          parallax_mid 静态雾
├── NearOverlay         parallax_near 近景框
├── DirectionalLight2D  环境定向光（偏暗森林色调）
├── RoomDecor
│   ├── WallVisuals     ColorRect 墙带 + LightOccluder2D（上墙）
│   ├── Doors           北/南/西/东门框占位（ColorRect）
│   └── Torches         四角火把：prototype 纹理 + PointLight2D
├── Boundaries          四面 StaticBody2D（与 L01 碰撞几何一致）
├── SpawnPoint
├── EnemySpawn1
└── DebugZones          L01 调试区（默认仍可见，L06 隐藏）
```

### 绘制顺序（z_index）

| z | 节点 |
|---|------|
| -30 | `Ground` |
| -20 | `GroundSprite` |
| -10 | `FogOverlay` |
| -5 | `NearOverlay` |
| 1 | `RoomDecor/WallVisuals`（墙带） |
| 0+ | 玩家 / 敌人 / 子弹 / `DebugZones` |

背景三层设 `light_mask = 0`，不参与 2D 光照；实体与火把光默认参与光照层。

---

## 4. 房间尺寸与贴图缩放

| 常量 | 值 | 说明 |
|------|-----|------|
| `level::playable_width` | 2000 | 房内可活动宽度 |
| `level::playable_height` | 1500 | 房内可活动高度 |
| `level::half_playable_width` | 1000 | 相机 limit ±x |
| `level::half_playable_height` | 750 | 相机 limit ±y |
| `level::ground_texture_size` | 1254 | `parallax_far` 原图边长 |
| `level::fog_texture_size` | 1024 | `parallax_mid` 原图边长 |
| `level::near_texture_size` | 1024 | `parallax_near` 原图边长 |
| `level::ground_sprite_scale_x/y` | 2000/1254, 1500/1254 | 非等比拉伸至活动区 |
| `level::fog_overlay_alpha` | 0.38 | 雾层 modulate.a |
| `level::near_overlay_alpha` | 0.72 | 近景 modulate.a |
| `level::door_opening_width` | 220 | 门框开口宽度（与墙带留空对齐） |
| `level::wall_visual_depth` | 72 | 墙带视觉厚度 |

围墙内缘与 L01 相同：x ∈ [-1000, 1000]，y ∈ [-750, 750]。

---

## 5. 相机与入口

### 相机锁边

`Level::_ready` 在玩家生成后调用 `apply_room_camera_limits()`：

- 查找玩家子节点 `PlayerCamera`（`name::level::player_camera`）
- 设置 `limit_left/right/top/bottom` 为 ±半宽/半高
- `limit_smoothed = true`
- `Character::_ready` 中 `m_camera->make_current()`

### Main 挂载

`Main` 构造时将 `Level` 加入 `MainSubViewport`（`name::dialog::game_sub_viewport`），而非 `MainCanvasLayer` 直接子节点，保证游戏画面与 HUD（`HudCanvasLayer`）分层。

---

## 6. 子任务完成状态

| ID | 内容 | 状态 |
|----|------|------|
| L04-1 | `parallax_far.png` 拉伸至活动区 | ✅ |
| L04-2 | `GroundSprite` + `Ground` fallback | ✅ |
| L04-3 | `FogOverlay` 静态雾层 | ✅ |
| L04-4 | `apply_room_camera_limits()` | ✅ |
| L04-5 | `constants.hpp` 登记节点名与 `room_assets` | ✅ |
| L04-6 | 实体 z-index 在雾层之上 | ✅ |
| L04-7 | 手动测试：四角贴边、玩法回归 | ⏳ 待走测 |

### 本次额外实现（超出原 L04 清单，仍为占位）

| 内容 | 说明 |
|------|------|
| `NearOverlay` | 近景 parallax 静态框 |
| `RoomDecor` | 墙带、四门洞框、四角火把点光 |
| `DirectionalLight2D` 调暗 | energy 0.45，突出火把/玩家光 |

---

## 7. 验收（L04 DoD）

- [ ] 房内地面完整、移动时贴图不滑脱
- [ ] 雾层为静态 overlay，不随移动错位
- [ ] 相机在四角贴边时停在墙内缘，不露出 void
- [ ] 玩家/敌人在雾层之上，玩法与 v0.1 一致
- [ ] 1080p 下帧率与 v0.1 相当

> 当前截图已能辨认森林房间骨架，但**占位装饰与多风格素材混搭**属预期现象，由 L06 解决（见 [走测与美术交接](./走测与美术交接.md)）。

---

## 8. 后续

| 阶段 | 内容 | 文档 |
|------|------|------|
| v0.2 下一步 | 占位美术统一、隐藏调试区 | [P0-L06](../任务/P0-v0.2-视觉与性能开发任务.md#5-阶段二占位美术l06) |
| P1 初 | 抽出 `room_*.tscn`；`Level` 管理当前房 | — |
| P1 中 | 清敌开门；相邻房切换 | — |
| P1 氛围 | 雾 Shader 替代 `FogOverlay` | — |

---

## 9. 文档索引

| 文档 | 用途 |
|------|------|
| [源码与函数说明](./源码与函数说明.md) | 常量、函数、场景字段逐项说明 |
| [走测与美术交接](./走测与美术交接.md) | 已知视觉问题、L06 优化清单 |
| [P0-v0.2 视觉与性能开发任务](../任务/P0-v0.2-视觉与性能开发任务.md) | v0.2 总路线图 |
| [P0-L01](../P0-L01-单关卡场景搭建/README.md) | L01 场景基线 |
