# v0.5 美术重绘 · Agents Window Plan 规格包

> **用途**：在 Cursor **Agents Window → Plan 模式**里直接喂本目录，批量生成图片，再手动（或后续脚本）落入 `project/assets/art/`。  
> **不依赖 Figma MCP**：本轮以「可生成、可切分、可进引擎」为交付；Figma 仅作可选对照。  
> **数据源**：`project/data/theme.json` + `weapon_rules.json` + `room_chain.json` + 画风锁 `style_bible_board_v1_LOCKED.png` + `cankao/` 星游记气质参考。

---

## 0. Figma MCP 探测结果（2026-07-27）

| 项 | 结果 |
|----|------|
| 目标文件 | https://www.figma.com/file/en5QOmzMHBH4JkAw79C76t · `node-id=15:40` |
| `fileKey` | `en5QOmzMHBH4JkAw79C76t` |
| 探测 | `whoami` / `get_metadata` → **失败**：`Cannot call tool before MCP process client is registered` |
| 历史备注 | `gpt_gemini/ASSET_MANIFEST.md` 已记 Starter 额度用尽、可写 MCP 曾断开 |

**结论**：本轮**不要卡在 Figma**。用本目录 JSON 在 Agents Window 生图；额度恢复后再考虑把成品上传到同文件新建页 `v0.5-art-YYYYMMDD`。

---

## 1. 三天目标（只做美术 + 武器/技能视觉）

| 天 | 焦点 | 产出 |
|----|------|------|
| **Day1** | 风格锁 + **三武器** + **三技能图标**（取消进度条长相） | 网格资产图 + 切分透明 PNG |
| **Day2** | **迷雾森林**关卡组合：explore / 三幕战斗 / whisper / mood / rest / boss | 各房底图 + 前景剪影条 |
| **Day3** | 标题 / 心灯 HUD / 彩虹海预告图；对照清单勾选 | 可替换进 `theme.json` 路径的定稿图 |

原则（与 v0.4 一致）：

- **只收敛范围，不降级单张质量**
- **禁止**裁切 / 抠图 / 描摹原截图里的角色与图标
- 复杂件：**AI 重生** → 网格批产 → 切分 → 去背 → 裁透明边界 → 进引擎

---

## 2. 本目录文件（Plan 模式入口）

| 文件 | 给 Agents 干什么 |
|------|------------------|
| [01_analysis.json](./01_analysis.json) | 元素级分流：矢量 / SVG / AI 位图 |
| [02_layout.json](./02_layout.json) | 构图与分层；技能 HUD 新布局（图标环，非条） |
| [03_asset_manifest.json](./03_asset_manifest.json) | 网格批产清单、输出尺寸、引擎替换路径 |
| [04_prompts_zh.md](./04_prompts_zh.md) | 中文生图提示词（可直接粘贴） |
| [05_acceptance_checklist.md](./05_acceptance_checklist.md) | 对标与验收勾选 |

**Agents Window Plan 推荐开场白**（复制即可）：

```text
请严格按 doc/v0.5-美术重绘-AgentsWindow计划/ 下的
01_analysis.json → 02_layout.json → 03_asset_manifest.json → 04_prompts_zh.md
执行图片生成。不要抠/描摹 cankao 或旧引擎贴图。
复杂资产用网格 sheet 批量生成后切分去背。
技能 HUD 禁止再做成横向进度条；改为图标 + 冷却环/灰化态。
武器仅：脉冲枪 / 旧手枪 / 散光铳。
主题优先：迷雾森林全链；彩虹海只做标题预告一张。
输出放到 gpt_gemini/v0.5_regen/ 下按 manifest 命名。
```

---

## 3. 现状速查（代码真相）

### 3.1 主题（`theme.json`）

- **活跃**：`forest`（迷雾森林）— 本轮全量重绘
- **标题预告**：`rainbow_sea`（彩虹海）— 只重绘 `title` / preview
- 拉伸主题（黄金神庙等）**整块不做**

### 3.2 武器（`weapon_rules.json`，仅这三种）

| id | 显示名 | 气质 |
|----|--------|------|
| `pulse` | 脉冲枪 | 默认；柔光弹、高射速 |
| `pistol` | 旧手枪 | 快准、单发重 |
| `shotgun` | 散光铳 | 近距扇形泼光 |

心境房可选找回：`pistol` / `shotgun`（脉冲为出生默认）。

### 3.3 技能 HUD（当前是三条进度条 — **要改掉**）

`src/ui/heart_hud.cpp` 现用 `draw_ready_bar`：

| 键位/技能 | 现表现 | 目标表现 |
|-----------|--------|----------|
| 冲刺 Dash | 青色横条 | **羽翼/残影图标** + 冷却环或灰化 |
| Q 极地斩 | 冷青横条 | **月弧斩击图标** + 冷却环 |
| E 能量之光 | 暖金横条 | **心灯光环图标** + 冷却环 |

数值逻辑（`*_ready_ratio`）可保留；**视觉语言**从「条」改为「符印/图标」。

### 3.4 房链组合（`room_chain.json` × `theme.bg`）

| 槽位 | 显示名 | 现 bg key | 重绘要点 |
|------|--------|-----------|----------|
| explore | 雾缘·小径 | explore | 无战斗；路径引导；冷绿薄雾 |
| combat_early | 雾缘 | combat_early | 冷绿；可战构图 |
| whisper | 低语 | whisper | 蓝金；秘密角；无怪 |
| combat_mid | 深心 | combat_mid | 蓝金；影更深 |
| mood | 心境 | mood | 武器/心灯二选一舞台 |
| combat_late | 光扉 | combat_late | 暖金逆光门 |
| rest | 光扉·歇脚 | rest | 喘息；篝火感 |
| boss | 心魔 | boss | 暖金+影紫；对称舞台 |

---

## 4. 媒体策略（总表）

| 策略 | 用于 |
|------|------|
| **AI 位图** | 房背景、角色/敌人、武器实体、子弹/粒子、技能符印、卡牌插画、装饰坠子、标题氛围图 |
| **引擎内矢量/程序** | 纯色底板、简单冷却环描边（可用 `draw_arc`）、AutoLayout 式 UI 框（后期代码） |
| **SVG / 简单矢量** | 极简准星、面包屑小圆点（可选） |
| **禁止** | 从 `cankao/` 或旧 PNG **裁切/描摹**角色与图标 |

---

## 5. 风格锁（不可破）

- 板：`project/assets/art/concept/style_bible_board_v1_LOCKED.png`
- 关键词：蓝金主角 · 紫影敌人 · 暖金心灯 · 金边深蓝 UI · 冷绿/暖金/深蓝紫幕调
- 气质参考（只学光色与线，不复制角色）：`cankao/联想截图_*.jpg`（星游记式赛璐璐）
- 叙事语气：劳累入睡 → 一梦 → 醒来；「找回」不是「升级」

---

## 6. 建议输出目录

```
gpt_gemini/v0.5_regen/
  sheets/          # 网格原图
  splits/          # 切分+去背
  rooms/forest/    # 对齐 theme.bg 文件名
  weapons/
  skills/
  hud/
  title/
```

定稿后再拷进 `project/assets/art/...`（覆盖或 `_v2` 再改 `theme.json` 路径）。

---

## 7. 与代码接入的边界（本规格包不做代码）

生图阶段**只交资产**。技能条改图标需改 `heart_hud.cpp` / 贴图加载——另开实现任务。本包只规定**图长什么样、叫什么名、放哪**。
