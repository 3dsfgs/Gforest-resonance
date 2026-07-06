# P0-L04 森林房间模板 — 实现文档

> **任务**：P0-L04（见 [P0-v0.2-视觉与性能开发任务](../任务/P0-v0.2-视觉与性能开发任务.md)）  
> **策略转向**：由「大地图 + 视差滚动」改为 **《元气骑士》式单房模板**（静态底图 + 锁镜头 + 森林氛围）  
> **依赖**：P0-L01 ✅，P0-L07 ✅  
> **关联**：[P0-L01 单关卡场景搭建](../P0-L01-单关卡场景搭建/README.md) · [独立游戏项目计划书](../独立游戏项目计划书.md)

---

## 1. 设计决策

### 为何放弃大地图视差

| 原方案 | 问题 |
|--------|------|
| 全图 `ParallaxBackground` + 多层平铺 | 非无缝 AI 素材拼接穿帮；地面与视差层差速错位 |
| 2000×1500 单图拉伸 + 滚动雾层 | 镜头扫全图时露边、硬切 |

### 新方案（Room Template v0）

借鉴 **元气骑士** 的 **空间结构**，保留本项目的 **治愈节奏**（慢叙事、非弹幕海）：

- **一串有呼吸感的房间**，而非一张无限滚动大地图  
- 每房：**静态地板** + **半透明雾 overlay** + **四面墙** + **刷点**  
- **相机限制在房间内**（`Camera2D` limit 对齐 `playable_width/height`）  
- 森林气质靠 **房主题美术 + 雾 overlay**；P1 再补 Shader 丁达尔光  

> `level1.tscn` 在 v0.2 即 **第一间森林空地（Room Template v0）**；P1 扩展为多房连通与清怪开门。

---

## 2. 涉及文件

| 类型 | 路径 | 说明 |
|------|------|------|
| 场景 | `project/scenes/levels/level1.tscn` | Room Template v0（根节点仍为 `Level1`） |
| 资源 | `project/assets/parallax/parallax_far.png` | 房间地面（拉伸至 2000×1500） |
| 资源 | `project/assets/parallax/parallax_mid.png` | 静态雾 overlay（alpha≈0.38） |
| 常量 | `src/core/constants.hpp` | `level::playable_*`、`name::level::fog_overlay` 等 |
| 逻辑 | `src/entity/level.cpp` | `apply_room_camera_limits()` |

---

## 3. 场景节点（Room Template v0）

```
Level1 (Level)                         # C++ 关卡根；v0.2 仅装载单房
├── Ground          ColorRect 2000×1500 森林绿 fallback
├── GroundSprite    单张地面贴图（与活动区等宽等高）
├── FogOverlay      静态半透明雾（非 Parallax）
├── DirectionalLight2D
├── Boundaries      四面墙（与 L01 几何一致）
├── SpawnPoint
├── EnemySpawn1
└── DebugZones
```

### 绘制顺序（z_index）

| z | 节点 |
|---|------|
| -30 | `Ground` |
| -20 | `GroundSprite` |
| -10 | `FogOverlay` |
| 0+ | 玩家 / 敌人 / 子弹 |

---

## 4. 房间尺寸（与 L01 一致）

| 常量 | 值 | 说明 |
|------|-----|------|
| `level::playable_width` | 2000 | 房内可活动宽度 |
| `level::playable_height` | 1500 | 房内可活动高度 |
| `level::half_playable_width` | 1000 | 相机 limit ±x |
| `level::half_playable_height` | 750 | 相机 limit ±y |

围墙内缘与 L01 相同：x ∈ [-1000, 1000]，y ∈ [-750, 750]。

---

## 5. 相机

`Level::_ready` 在玩家生成后调用 `apply_room_camera_limits()`：

- 查找玩家子节点 `PlayerCamera`  
- 设置 `limit_left/right/top/bottom` 为房间半宽/半高  
- `limit_smoothed = true`  

玩家仍在房内自由移动；镜头不超出房间边界（元气骑士式「当前房取景框」）。

---

## 6. 验收（L04 DoD）

- [ ] 房内地面完整、无块状拼接缝  
- [ ] 雾层为静态 overlay，不随移动「滑脱」  
- [ ] 相机在四角贴边时停在墙内缘，不露出 void  
- [ ] 玩家/敌人在雾层之上，玩法与 v0.1 一致  
- [ ] 截图可辨认森林房间氛围（占位美术即可）

---

## 7. 后续（P1 房间制）

| 阶段 | 内容 |
|------|------|
| P1 初 | 抽出 `room_*.tscn`；`Level` 管理当前房实例 |
| P1 中 | 清敌开门；相邻房切换；房间图 |
| P1 氛围周 | 雾 Shader 替代 `FogOverlay` 占位图 |

---

## 8. 文档索引

| 文档 | 变更 |
|------|------|
| [v0.2定义与验收标准](../v0.2定义与验收标准.md) | V1 改为「森林房间氛围」 |
| [独立游戏项目计划书](../独立游戏项目计划书.md) | 关卡结构：房间制 |
| [P0-L01](../P0-L01-单关卡场景搭建/README.md) | L01 场景演进为 Room Template v0 |
