<div align="center">

# 🏆 EU5 Patcher

### 无条件开启成就

[中文](README_CN.md) | [English](README.md)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)]()
[![Game](https://img.shields.io/badge/game-Europa%20Universalis%20V-orange.svg)]()

</div>

---

## 📖 简介

关于是否应该强制**未修改的铁人模式**才能解锁成就的争论已经持续多年。虽然《十字军之王3》和《群星》采取了对玩家更友好的方式，但《欧陆风云5》遗憾地倒退了一步。

因此，我制作了这个补丁，它可以：
- 在非铁人模式下开启部分成就。
- 在铁人模式下开启**所有成就**，同时允许你像非铁人模式一样自由**存档和读档**。

| 模式        | Mod   | 设置    | 控制台               | 存档 & 读档          | 成就状态           |
| ----------- | ----- | ------- | -------------------- | -------------------- | ------------------ |
| 非铁人模式   | ✅ 任意 | ✅ 任意  | ✅ 开启               | ✅ 开启               | ⚠️ 部分            |
| **铁人模式** | ✅ 任意 | ✅ 任意  | ✅ <ins>**开启**</ins> | ✅ <ins>**开启**</ins> | ✅ 全部            |


> [!NOTE]
> 部分成就在非铁人模式下可能无法触发。因此我们**推荐**使用铁人模式配合此补丁，这样既能获得完整成就体验，又能保留存档/读档功能。

<div align="center">
<img src="figures/Effect.png" alt="Achievement Effect" width="600"/>
</div>

---
## 🚀 如何使用
> [!TIP]
> 每次游戏更新后，都需要重新对 `eu5.exe` 进行打补丁。

### 🐍 方法 1: Python 脚本

你可以从任意位置运行脚本。

```bash
python patch.py
```

### ⚙️ 方法 2: 源码编译 (C++)

```bash
# 1. 编译源代码
cl /std:c++17 /O2 /EHsc patch.cpp

# 2. 运行生成的 patch.exe
patch.exe
```

### ⚠️ 方法 3: 预编译 EXE

> [!WARNING]
> 运行未知来源的可执行文件存在风险。请仅在信任来源的情况下继续。

1. 从 [📦 Releases 页面](https://github.com/UFOdestiny/EU5-Patcher/releases/) 下载 `patch.exe`。
2. 直接运行 `patch.exe`（支持在任意位置运行）。

---

## ✅ 打补丁后

如果一切顺利，你会看到提示：
```
EU5 is successfully patched.
```

> [!TIP]
> 设置菜单中的奖杯和铁人图标最初可能显示为**红色**。只需开始游戏，它们就会变成**绿色**。

---

## 📚 文档

- [技术教程](tutorial.md) - 关于补丁原理的详细说明。

## 🙌 致谢

本项目主要用于学习和技能提升。灵感来源于：
- [Enabling Achievements in Stellaris With Mods (All game versions) [SRE]](https://steamcommunity.com/sharedfiles/filedetails/?id=2460079052)
