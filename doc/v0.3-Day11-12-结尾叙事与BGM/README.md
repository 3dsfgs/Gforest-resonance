# v0.3 Day11–12 结尾叙事 + BGM 分场景

> **冲刺任务**：[15天冲刺计划-v0.3垂直切片](../15天冲刺计划-v0.3垂直切片.md) · Day 11–12  
> **分支**：`sprint/v0.3`  
> **依赖**：Day 10–11 标题界面  
> **详细说明**：[源码与函数说明.md](./源码与函数说明.md)

---

## 概述

| 能力 | 实现 |
|------|------|
| 通关结语 | 末房 Victory → 全屏 `ending_screen`，**逐行淡入**结语 |
| 回标题 | 结语播完后 **按任意键** → 卸关卡 → 标题 |
| BGM | Autoload `MusicDirector`：标题 / 战斗 / 结尾 三轨切换 |

### BGM 占位曲目（仓库内 Kenney loops）

| 场景 | 文件 |
|------|------|
| 标题 | `music/loops/flowing_rocks.ogg` |
| 战斗 | `music/loops/mission_plausible.ogg` |
| 结尾 | `music/loops/night_at-the-beach.ogg` |

换曲：改 `project/scripts/music_director.gd` 里 `PATHS`。

---

## 流程

```
标题 BGM → 生日 → 进入森林（战斗 BGM）
  → 房1→2→3 清怪
  → Victory → 结语层 + 结尾 BGM
  → 任意键 → 标题 BGM
```

失败仍用 Console 逐字叙事 + **R** 重置当前房（战斗 BGM 不停）。

---

## 验收对照

| 验收项 | 状态 |
|--------|------|
| 通关见慢速结语 | ✅ |
| 结尾有温暖向 BGM | ✅ 占位曲 |
| 标题/战斗/结尾 BGM 可区分 | ✅ |
| 任意键回标题 | ✅ |
| 构建通过 | ✅ |

---

## 你来走测

1. 标题应有环境向 BGM  
2. 进关后 BGM 切换为战斗曲  
3. 打通三房 → 全屏结语逐行浮现 → 提示出现后按键回标题  
4. 死亡按 R 重开当前房，Console 仍有失败文案  

走测 OK 后建议：`git tag day-12`。
