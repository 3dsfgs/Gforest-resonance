# P0-L06 占位美术规范

> **状态**：v0.2 已接入（行走/施法/受击/死亡动画）  
> **关联**：[P0-v0.2 视觉与性能开发任务](../任务/P0-v0.2-视觉与性能开发任务.md) · [P0-L04 走测与美术交接](../P0-L04-森林房间模板/走测与美术交接.md)

---

## 1. 目录结构

```
project/assets/sprites/
├── player/player_sheet.png          # 玩家动画表
├── enemy/enemy_sheet.png            # 敌人动画表
├── projectile/projectile_sheet.png  # 子弹 / 命中特效
├── environment/environment_tiles.png # 环境条带 + 火把
└── sprite_frames/
    ├── player_frames.tres
    ├── enemy_frames.tres
    ├── projectile_frames.tres
    └── torch_frames.tres
```

房间远景 parallax 仍在 `project/assets/parallax/`（L04）。

---

## 2. 网格与动画切分（176×192）

所有角色/子弹表均为 **1408×768**，按 **176×192** 网格切分（8 列 × 4 行）。勿用 128×128，否则会切到灰底。

### 玩家 `player_sheet.png`

| 行 | 动画名 | 列范围 | 帧数 | 循环 | 速度 |
|----|--------|--------|------|------|------|
| 0 | `idle` | 0–3 | 4 | 是 | 6 |
| 1 | `walk` | 0–5 | 6 | 是 | 10 |
| 2 | `cast` | 0–3 | 4 | 否 | 12 |
| 3 | `hurt` | 0–1 | 2 | 否 | 10 |
| 4 | `death` | 0–3 | 4 | 否 | 8 |

### 敌人 `enemy_sheet.png`

| 行 | 动画名 | 列范围 | 帧数 | 循环 | 速度 |
|----|--------|--------|------|------|------|
| 0 | `idle` | 0–3 | 4 | 是 | 5 |
| 1 | `walk` | 0–3 | 4 | 是 | 8 |
| 3 | `hurt` | 0–1 | 2 | 否 | 10 |
| 4 | `death` | 0–3 | 4 | 否 | 8 |

### 子弹 `projectile_sheet.png`

| 行 | 动画名 | 列范围 | 帧数 | 循环 | 速度 |
|----|--------|--------|------|------|------|
| 0 | `travel` | 0–3 | 4 | 是 | 12 |
| 1 | `impact` | 0–3 | 4 | 否 | 14 |

### 火把 `environment_tiles.png`

| 行 | 动画名 | 列范围 | 帧数 | 循环 | 速度 |
|----|--------|--------|------|------|------|
| 4 | `flicker` | 0–3 | 4 | 是 | 8 |

---

## 3. Godot Import 预设

| 选项 | 值 |
|------|-----|
| Filter | **Nearest** |
| Mipmaps | 关 |
| Compress | Lossless（占位阶段） |

---

## 4. 场景接入

| 场景 | 节点 | 资源 |
|------|------|------|
| `player.tscn` | `PlayerSprite` (`AnimatedSprite2D`) | `player_frames.tres` |
| `enemy.tscn` | `PlayerSprite` | `enemy_frames.tres` |
| `bullet.tscn` | `Bullet` | `projectile_frames.tres` |
| `level1.tscn` | `TorchSprite` ×4 | `torch_frames.tres` |

角色 sprite 在 `PlayerCollisionPoly` 下，`scale = 0.5`；根节点 `scale = 2`，最终显示约 128px。

---

## 5. 代码动画状态机（`Character`）

- **idle / walk**：根据移动输入在 `on_character_movement` 中切换
- **cast**：`on_character_shoot` 时播放，结束后回到 idle/walk
- **hurt**：`take_damage` 存活时播放
- **death**：血量归零时播放，禁用碰撞；敌人死亡动画结束后 `queue_free`

常量见 `src/core/constants.hpp` → `anim::` 与 `path::sprites::`。

---

## 6. AI 出图替换流程

1. 按上文网格重新生成 PNG，保持文件名与路径
2. 若行列布局变化，重新运行切帧脚本或手改 `sprite_frames/*.tres` 中的 `region = Rect2(x, y, 128, 128)`
3. Godot 中 F5 走测：idle/walk/cast、敌人追击、子弹、火把闪烁
4. 确认碰撞未改（B06 矩阵不变）

---

## 7. Godot 内验证（不会调试时按此做）

1. **F5 运行** → 按 **F8** 暂停（或菜单 Debug → Keep Playing 取消勾选后点暂停）
2. 左侧 **Scene** 树展开：`Level1` → `Player` → `PlayerCollisionPoly` → `PlayerSprite`
3. 选中 `PlayerSprite`，右侧 **Inspector**：
   - **Sprite Frames** 应显示 `player_frames.tres`
   - **Animation** 选 `idle`，下方应出现**小德鲁伊预览**（不是灰块）
4. 若预览是灰块：打开 `assets/sprites/player/player_sheet.png`，对照 `sprite_frames/player_frames.tres` 里 `region` 是否对齐角色格
5. **Remote** 标签（运行暂停时 Inspector 旁）：看 `Player` 的 `global_position` 是否在房内；移动时坐标应变化
6. 取消暂停继续玩：WASD 移动应切 `walk`，射击应闪 `cast`

逻辑已在跑（控制台有 `Player location` / `Enemy contact`），验证重点是 **第 3 步预览是否有角色**。

## 8. v0.2 已完成项

- [x] L06-1 目录 `sprites/player|enemy|environment|projectile`
- [x] L06-2/10 玩家/敌人 AnimatedSprite2D + 森林风 sprite
- [x] L06-4 子弹 travel 动画
- [x] L06-5 本文档
- [x] L06-6 `DebugZones` 默认 `visible = false`
- [x] L06-8 火把 `flicker` 动画
- [x] L06-9 墙/门框 ColorRect alpha 弱化
- [x] L06-11 `NearOverlay` alpha 降至 0.5
- [x] L06-13 HeartHud 放大 + 琥珀/绿褐配色（延伸）
- [ ] L06-7 玩法回归（F5 手动走测 M1–M7）
