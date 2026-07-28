# v0.4 P4 主题扩展框架

> **冲刺任务**：[15天冲刺计划-v0.4打磨与个性化](../15天冲刺计划-v0.4打磨与个性化.md) · 阶段 P4  
> **依赖**：P0 / P2 / P3 已完成  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

把「迷雾森林」抽成可配置主题表，换主题原则上只改 JSON + 素材路径。彩虹海本阶段只做标题预告位。

| # | 任务 | 状态 | 备注 |
|---|------|------|------|
| P4-1 | `theme.json` 数据结构 | ✅ | palette / bg / enemy_sprites / bgm / room_names / title_text |
| P4-2 | 森林主题接入 | ✅ | 标题、BGM、房链、过场 tagline、关卡底图 |
| P4-2b | 彩虹海标题预告 | ✅ | `title_preview_theme`；有图换底，无图显示预告字 |

---

## 怎么换主题（验收口径）

1. 改 [`project/data/theme.json`](../../project/data/theme.json) 的 `active_theme`
2. 补齐该主题的 `bg` / `bgm` / `room_chain` 路径
3. **不改**玩法 C++ / 房型逻辑

彩虹海预告：把 `rainbow_sea_preview.png` 放到 `project/assets/art/title/`（见同目录说明）；标题会自动换预告底图。

---

## 走测清单

| 项 | 怎么测 |
|----|--------|
| 标题文案 | 启动后标题为「林间回响」，tagline 来自 theme；下方有彩虹海预告字 |
| BGM | 标题曲仍能播（路径来自 theme.forest.bgm.title） |
| 房链 | 仍是 6 房梦房链；过场短句与 theme.room_names 一致 |
| 关卡底图 | 进房后 BackgroundTexture 被 theme 覆盖（冷绿→蓝金→暖金） |
| 换主题骨架 | 改 `active_theme` 为不存在的 id 会警告；改回 `forest` 可玩 |

---

## 明确未做

- 彩虹海可玩切片 / 解锁条件逻辑
- 敌人贴图运行时换肤（路径已进 JSON，场景仍用现有 ExtResource）
- 多份 room_chain 实装（字段已留 `room_chain`）
