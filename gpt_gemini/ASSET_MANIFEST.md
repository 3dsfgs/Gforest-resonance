# gpt_gemini 资产清单

> 生成时间：2026-07-21  
> 最后整理：2026-07-21（画风 **已定稿**；CG/锚点/敌人/UI 已切进引擎目录）  
> 用途：概念图 / 已拆分 / 待入 Figma 或 Godot 场景  
> 命名规则：`类别_内容_版本.png`

---

## 定稿结论

- **风格板锁定**：`concept/style_bible_board_v1.png`  
  引擎副本：`project/assets/art/concept/style_bible_board_v1_LOCKED.png`
- 关键词：蓝金主角、紫影敌人、暖金心灯、金边深蓝 UI、冷绿/暖金/深蓝紫色板
- 重复锚点 `anchor_bedroom_night_day_split_v2_dup.png` **已删除**

---

## 目录结构

```
gpt_gemini/
├── concept/     # 风格板、环境三联概念 + splits/
├── anchor/      # 房间锚点 + splits/（夜/日）
├── character/   # 主角 + splits/（头像/立绘/背视/手臂/俯视战斗）
├── enemy/       # 敌人 + splits/
├── room/        # 梦房背景 / 关卡概念
├── cg/          # 开场 / 结尾分镜 + splits/op|ed
├── wip/         # 测试图，不进引擎
└── _original/   # 原始 ChatGPT/Gemini 文件名备份
```

---

## 源文件状态

| 新文件名 | 状态 |
|----------|------|
| `concept/style_bible_board_v1.png` | ✅ **画风定稿** |
| `concept/concept_env_triptych_forest_cosmic_temple_v1.png` | 参考（远期主题） |
| `anchor/anchor_bedroom_night_day_split_v1.png` | ✅ 已切夜/日 |
| `character/character_protagonist_concept_sheet_v1.png` | ✅ 已拆 splits/ |
| `character/character_protagonist_isometric_back_v1.png` | ✅ 已并入 splits/back + topdown |
| `character/splits/protagonist_portrait_v1.png` | 头像 |
| `character/splits/protagonist_fullbody_front_v1.png` | 立绘正面（裁切参考） |
| `character/splits/protagonist_back_reference_v1.png` | 背视图参考 |
| `character/splits/protagonist_arm_glove_reference_v1.png` | 右臂/手套参考（左臂需镜像） |
| `character/splits/protagonist_topdown_combat_v1.png` | 俯视战斗姿态 |
| `enemy/enemy_shadow_wisp_purple_v1.png` | ✅ 已抠透明底 |
| `room/room_forest_*` / `room_mood_*` | ✅ 已拷贝进引擎 |
| `room/room_cosmic_*` | 远期参考，未进首版 |
| `cg/cg_op_sleep_to_dream_4panel_v1.png` | ✅ 已切 4 格 |
| `cg/cg_ed_wake_up_3panel_v1.png` | ✅ 已切 3 格 |
| `wip/wip_sprite_test_red_checker_v1.png` | 废弃候选 |

---

## 已入引擎（`project/assets/art/`）

```
anchor/
  bedroom_night_v1.png
  bedroom_day_v1.png
cg/op/
  op_01_desk_exhaustion.png
  op_02_phone_clack.png
  op_03_golden_mist.png
  op_04_dream_knight.png
cg/ed/
  ed_01_alarm_7am.png
  ed_02_standing_ready.png
  ed_03_leaving_door.png
enemies/
  shadow_wisp_purple_v1.png
rooms/forest/
  act1_cold_green_v1.png
  act2_blue_gold_v1.png
  act3_warm_gold_v1.png
  ruins_isometric_tileable_v1.png
rooms/mood/
  weapon_choice_v1.png
ui/
  heart/heart_lamp_v1.png
  buttons/btn_star_sword_next_cluster_v1.png
  palette_strips_v1.png
concept/
  style_bible_board_v1_LOCKED.png
  style_slices/   # 风格板分区参考切片
```

## 已接入 Godot 场景（2026-07-21）

- `scenes/levels/level1.tscn` → 冷绿森林
- `scenes/levels/level2.tscn` → 蓝金森林
- `scenes/levels/level3.tscn` → 暖金森林
- `scenes/characters/player.tscn` → 蓝金俯视主角
- `scenes/characters/enemy.tscn` / `enemy_brute.tscn` → 紫影敌人
- `scenes/prefabs/heart_lamp_hud.tscn` → 心灯 HUD（已挂入 `main_dialog.tscn`）
- `scenes/prefabs/bedroom_anchor.tscn` → 卧室夜 / 日双态
- `scenes/prefabs/weapon_choice_room.tscn` → 心境武器选择房
- `scenes/prefabs/story_sequence.tscn` → 通用图像叙事播放器
- `scenes/story/op_sleep_to_dream.tscn` → OP 4 格
- `scenes/story/ed_wake_up.tscn` → ED 3 格

---

## Figma（v0.4 HUD — 梦境心灯）

- 文件：[v0.4 HUD — 梦境心灯](https://www.figma.com/design/en5QOmzMHBH4JkAw79C76t)
- Variables：`Dream HUD`（金 / 深蓝 / 冷绿 / 影紫）

### 页面结构（Starter 限 3 Page）

| Page | 内容 |
|------|------|
| `01 HUD States` | 两屏 HUD + Components Library Section |
| `02 Character Assets` | 主角拆件网格 + `Character / TopDown Rig` 组件 |
| `03 Story CG` | OP 4 格 / ED 3 格 / 锚点夜·日 |

### HUD 两态（已拆分，不再混屏）

| 画板 | Node | 背景 | 有 | 无 |
|------|------|------|----|----|
| **战斗态** `Combat HUD / 1920x1080` | `15:43` | act2 蓝金森林 | 心灯、徽章、俯视 Player Rig（Body+双臂参考层）、独白、武器芯片、低语框 | 门选三卡 |
| **门选态** `Door Select HUD / 1920x1080` | `15:75` | act1 冷绿森林 | 心灯、徽章、门选三卡、低语框、继续 | 战斗 Player Rig |

- 旧混合屏已归档：`_Archive / Mixed HUD (deprecated)`
- 组件库 Section：`Components Library`（Heart Lamp / Whisper Box / Gold Button / Room Badge / Door Choice）

### 主角 Rig 说明（Figma + 本地一致）

- **战斗态正确用法**：只用 `protagonist_topdown_combat_v1.png` 完整俯视剪影（透明底），**禁止**叠侧视手臂设定图
- **同视角拆层**（`character/splits/`）：
  - `protagonist_topdown_body_v1.png` — Body（挖掉右臂）
  - `protagonist_topdown_arm_weapon_v1.png` — Arm+Weapon（同俯视源）
  - `protagonist_arm_glove_reference_v1.png` — **仅设定参考**（侧视，不进战斗 HUD）
- **正确预览**（Figma 写入 MCP 断开时的对照稿）：
  - `character/previews/combat_hud_correct_preview_v1.png`
  - `character/previews/door_select_hud_correct_preview_v1.png`
  - `character/previews/character_layer_catalog_v1.png`
- 修复脚本：`gpt_gemini/_figma_fix_combat_rig.js`（重连官方 Figma MCP 后执行）

> 当前 Cursor 仅剩只读 `user-figma-developer-mcp`；可写的 `plugin-figma-figma` / `use_figma` 已断开（此前 Starter 额度用尽）。重连后即可把 Combat `#15:43` 的灰块手臂 Rig 换成上述俯视层。

### 脚本

- `gpt_gemini/_split_character_assets.py` → `character/splits/`
- `gpt_gemini/_upload_figma_assets.py` + `_figma_upload_hashes.json`

## 下一步（按优先级）

1. **P0 实现**：真爱心 / 心灯 HUD（对照 Figma 战斗态 `15:43` + `ui/heart/heart_lamp_v1.png`）
2. **Figma 微调**：Player Rig 手臂位置、门选态是否加 portrait/back 剪影
3. **接入叙事**：`story.json` 挂上 op_01–04 / ed_01–03
4. **主角 sprite 精修**：独立左臂像素层 / 四向 walk（需重绘或再生成）
5. **游戏名**：画风已定，可开始起名

---

## 备注

- 心灯 / 按钮目前是风格板裁切件，边缘带羊皮纸底；进 HUD 前建议再抠一次透明底。
- 主角手臂拆分目前是**设定图裁切 + 镜像参考**，不是骨骼级拆件；若要做挥剑动画需单独出图层。
- 敌人 sprite 已去黑底，但仍偏大（约 1024 高），入关卡需缩放。
- 星海系列作「单元剧第二夜」储备。
