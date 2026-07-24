# v0.4 P2 玩法深度

> **冲刺任务**：[15天冲刺计划-v0.4打磨与个性化](../15天冲刺计划-v0.4打磨与个性化.md) · 阶段 P2
> **决策**：P1 复杂美术资源整合**整块跳过**（范围收敛，非质量降级）；从玩法深度先动手。
> **依赖**：v0.4 P0 硬伤速修已完成；固定三房线性链可跑
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

P2 目标：把「固定三间战斗房」升级为**梦房链**，并逐步叠武器 / 分波 / Boss / 音频。

| # | 任务 | 借鉴 | 状态 | 备注 |
|---|------|------|------|------|
| P2-6 | 梦房链框架 + 房型（战斗/低语/心境/Boss） | Hades / 元气骑士 / 以撒 | ✅ | **无门选择**首版；预制顺序链 |
| P2-3 | 战斗房分波刷怪（≥2 波） | 以撒 / Nuclear Throne | ✅ | Marker `W2`/`W3` 分组；波间喘息 1.15s |
| P2-1 | `weapon_rules.json` + 拾取切换 | 元气骑士 / Gungeon | ✅ | 脉冲枪默认；心境房唤醒手枪/霰弹 |
| P2-2 | 近战 hitbox | — | ⬜ | **整块推迟**（符合风险表） |
| P2-4 | 心魔 Boss 特殊攻击 | Hades | ✅ | `EnemyBoss` + HeartDemon AI；波1 影从 → 波2 Boss |
| P2-5 | BGM / SFX 按场景与武器分轨 | Hades | ✅ | 标题/探索/战斗/Boss/结尾 + 武器开火音 |

✅ **本轮已完成**：P2-6·3·1·4·5。近战推迟；门选择仍为拉伸。

---

## P2-6 要点

### 新模型：一晚 = 一条梦房链

| 顺序 | 房型 | 场景 | 地名 | 玩法 |
|------|------|------|------|------|
| 1 | 战斗 | `level1.tscn` | 雾缘 | 清怪 |
| 2 | 战斗 | `level2.tscn` | 深心 | 清怪 |
| 3 | **低语** | `whisper_room.tscn`（新） | 低语 | 无敌人；短句独白；按 **E** 继续 |
| 4 | **心境** | `mood_room.tscn`（新） | 心境 | 无敌人；←/→ 选武器，E 唤醒 |
| 5 | 战斗 | `level3.tscn` | 光扉 | 清怪 |
| 6 | **Boss** | `boss_room.tscn`（新） | 心魔 | 波1 影从 → 波2 心魔 Boss；清完 = Victory |

数据在 `project/data/room_chain.json`；开局由 Autoload `DreamRoomChain` 生成配置，C++ `Main::configure_run()` 注入当晚房链。

### 降级边界（与冲刺计划 §8bis.5 一致）

- **门选择**：首版不做，链条固定顺序（可拉伸再加）。
- **房型多样化**：必须保留（治「三房雷同」的关键）→ 已做。
- **美术**：复用现有 level 背景 + 原型占位，不依赖 P1 新美术管线。

---

## 走测中发现并修复的问题

| 问题 | 根因 | 修复 |
|------|------|------|
| 启动 Parser Error：`begin_run()` override 原生类 | Godot 4 不允许 GDScript override GDExtension 原生方法；警告当错误 | 删除 override；新增 `start_dream_run()`，由生日门/暂停菜单调用 |
| 低语/心境房按 E 无反应 | `MainSubViewport`：`handle_input_locally=false` + `gui_disable_input=true`，`_unhandled_input` 收不到键 | 改走全局 `Input`（与 C++ 玩家输入一致） |
| `is_physical_key_just_pressed` 不存在 | Godot 4.7 只有 `is_physical_key_pressed`，无 `*_just_pressed` 键盘 API | 新增 `room_advance_input.gd` 边沿检测 + `interact` 输入动作（E） |

---

## 验收对照（P2-6）

| 验收项 | 状态 |
|--------|------|
| 一晚 6 间房（非固定 3 战斗） | ✅ |
| ≥2 房型（战斗 / 低语 / 心境 / Boss） | ✅ |
| 低语房：E 翻句 → 过场 | ✅ |
| 心境房：E 推进 → 过场 | ✅ |
| 末房（心魔）清完 → Victory / 结语 | ✅ |
| 门选择（Hades 式二选一） | ⬜ 拉伸 |
| 构建通过 | ✅ |

---

## P2-3 分波刷怪

### 规则

- Marker 名含 **`W2` / `W3`** → 第 2 / 3 波；无后缀 → 第 1 波。  
  例：`EnemySpawn1`（波1）、`EnemySpawnW2_1`、`EnemyBruteSpawnW2_1`。
- 清完当前波且预警队列空 → **喘息 1.15s** → 刷下一波（仍带预警圈）。
- 最后一波清完 → `complete_room()`（过场或 Victory）。

### 各房波次（首版）

| 房间 | 波1 | 波2 |
|------|-----|-----|
| 雾缘 level1 | 1 Scout | 2 Scout |
| 深心 level2 | 1 Scout | 1 Scout + 1 Brute |
| 光扉 level3 | 2 Scout | 1 Brute + 1 Scout |
| 心魔 boss | 2 Scout（影从） | **1 HeartDemon Boss** |

### 验收对照（P2-3）

| 验收项 | 状态 |
|--------|------|
| 战斗房 ≥2 波 | ✅ |
| 波间有喘息，非瞬间叠怪 | ✅ |
| 清完全部波次才过场 | ✅ |
| 构建通过 | ✅ |

---

## 你来走测

1. 标题 → 生日门 → 开战（脉冲枪）
2. **房 1**：清第 1 波 → 约 1s 喘息 → 第 2 波 → 过场
3. 房 2 同理（第 2 波含 Brute）
4. **低语**：按 **E** 看完 3 句 → 再按 E 过场
5. **心境**：←/→ 或 1/2 选手枪/霰弹 → **E 唤醒** → 过场
6. 房 5 清怪 → **心魔**：波1 两只影从 → 喘息 → **紫色预警圈**刷出 Boss → 击败 → 结语

Boss 手感自检：体型更大、约 12 心；会环绕射击；周期性**扇形心影爆发**；半血后会**暗影冲撞**。

建议下一件：进入 **P3**（或门选择拉伸）；近战 P2-2 继续推迟。

---

## P2-5 音频分轨

### BGM（`MusicDirector`）

| 轨 | 场景 | 占位曲 |
|----|------|--------|
| 标题 | 标题屏 | `flowing_rocks` |
| 探索 | 低语 / 心境 | `space_cadet` |
| 战斗 | 战斗房 | `mission_plausible` |
| Boss | 心魔房 | `infinite_descent` |
| 结尾 | EndingScreen | `night_at-the-beach` |

过场加载下一房时按 `DreamRoomChain.get_room_kind_at` 切轨。

### SFX

| 类型 | 接入 |
|------|------|
| 武器开火 | `weapon_rules.json` 的 `sfx` 字段；开火时播放 |
| 脉冲 / 手枪 / 霰弹 | 小激光 / 复古激光 / 大激光（Kenney sci_fi） |
| UI | 低语翻页 click；心境换选 / 唤醒 confirm |

### 验收对照（P2-5）

| 验收项 | 状态 |
|--------|------|
| 标题 / 探索 / 战斗 / Boss / 结尾 BGM 可区分 | ✅ |
| 三种远程开火音不同 | ✅ |
| 构建通过 | ✅ |

---

## P2-4 心魔 Boss

### 设计

| 项 | 内容 |
|----|------|
| 实体 | `EnemyBoss`（`enemy_boss.tscn`），紫影放大体型 |
| 血量 / 移速 | 12 心 / 260 |
| AI | `EnemyController::HeartDemon` |
| 普攻 | 中距单发暗影弹 |
| 特殊 | **心影爆发**：对准玩家扇形 5 连射（冷却 ~3.0s） |
| 冲撞 | 全程可用暗影冲撞；≤50% 心冷却更短、触发更远 |
| Marker | `EnemyBossSpawn*`（可加 `W2`） |
| 房结构 | 波1 `EnemySpawn1/2` → 波2 `EnemyBossSpawnW2_1` |

预警圈：Boss 用更大的紫色 telegraph。

### 验收对照（P2-4）

| 验收项 | 状态 |
|--------|------|
| 末房有专用高血量 Boss | ✅ |
| 特殊攻击（扇形爆发）可感知 | ✅ |
| 半血后冲撞（二阶段） | ✅ |
| 构建通过 | ✅ |

---

## P2-1 武器数据 + 心境房唤醒

### 武器（首版 3 种远程；近战推迟）

| id | 名称 | 手感 |
|----|------|------|
| `pulse` | 脉冲枪 | 默认：中射速、伤害 1 |
| `pistol` | 旧手枪 | 较慢射速、伤害 2、更准 |
| `shotgun` | 散光铳 | 慢射速、5 弹丸散射、伤害 1/发 |

数据：`project/data/weapon_rules.json`。  
心境房：←/→ 或 1/2 选择 → **E 唤醒** → 跨房保留（`Main.set_run_weapon`）。

### 验收对照（P2-1）

| 验收项 | 状态 |
|--------|------|
| 数据驱动射速/伤害/散射/弹丸数 | ✅ |
| 心境房可选 ≥2 种并切换 | ✅ |
| 「找回/唤醒」文案 | ✅ |
| 切换后后续战斗房仍生效 | ✅ |
| 近战武器 | ⬜ 推迟（P2-2） |
| 地面掉落物拾取 | ⬜ 拉伸（心境房选择已覆盖决策） |

---

## 涉及文件（摘要）

| 路径 | 作用 |
|------|------|
| `project/data/room_chain.json` | 房链模板 + 房池 |
| `project/data/weapon_rules.json` | 武器数值表 |
| `project/scripts/dream_room_chain.gd` | Autoload：生成当晚配置 |
| `project/scripts/main.gd` | `start_dream_run()` |
| `project/scripts/mood_room_flow.gd` | 心境房选武器 |
| `project/scripts/whisper_room_flow.gd` | 低语房推进 |
| `project/scripts/room_advance_input.gd` | E 键边沿检测 |
| `project/scenes/levels/*.tscn` | 房型；战斗房含 `W2` Marker |
| `src/util/weapon_rules.*` | JSON 加载 + 应用到 Spawner |
| `src/main.*` / `src/entity/level.*` | 房链、分波、武器跨房 |
| `src/entity/projectile/*` | 伤害/多弹丸/冲量 |
| `project/project.godot` | Autoload + `interact` 动作 |
