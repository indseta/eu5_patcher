<div align="center">

# 🏆 EU5 Patcher

### Enable Achievements Unconditionally

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)]()
[![Game](https://img.shields.io/badge/game-Europa%20Universalis%20V-orange.svg)]()

</div>

---

## 📖 About

The debate over whether **Ironman mode** should be required to unlock achievements has been going on for years. While *Crusader Kings III* and *Stellaris* took a player-friendly approach, *Europa Universalis V* unfortunately stepped backward.

This patch removes those restrictions — enabling the achievement system in:

| Feature | Status |
|---------|--------|
| 🎮 Non-Ironman mode | ⚠️ Partial (some bugs may block unlocks) |
| 🔧 Modded games | ✅ Enabled |
| ⚙️ Any game settings | ✅ Enabled |

> [!NOTE]
> Some bugs may still prevent achievements from unlocking in non-Ironman mode. If you use Ironman mode, everything works as normal.

<div align="center">
<img src="figures/effect.png" alt="Achievement Effect" width="600"/>
</div>

---
## 🚀 How to Use
> [!TIP]
> You’ll need to patch `eu5.exe` after every game update.

### 🐍 Option 1: Python

```bash
# 1. Navigate to game directory
cd ".../Europa Universalis V/binaries/"

# 2. Run the patch script
python patch.py
```

<details>
<summary>📋 Prerequisites</summary>

- Python 3.10+ installed
- PATH environment variable configured correctly

</details>

### ⚙️ Option 2: C++

```bash
# 1. Compile the source
cl /std:c++17 /O2 /EHsc patch.cpp

# 2. Move patch.exe to game directory and run it
```

### ⚠️ Option 3: Pre-built EXE

> [!WARNING]
> Running unknown executables is risky. Only proceed if you fully trust the source.

1. Download `patch.exe` from the [📦 Releases page](https://github.com/UFOdestiny/EU5-Patcher/releases/tag/exe)
2. Place it in `.../Europa Universalis V/binaries/`
3. Run it

---

## ✅ After Patching

If everything goes well, you'll see:

```
EU5 is successfully patched.
```

> [!TIP]
> The trophy icon may appear **red** in the settings menu at first. Simply start the game, and it will turn **green**.

---

## 📘 Tutorial

<div align="center">
<img src="figures/position.png" alt="Pattern Position" width="600"/>
</div>

*More details coming soon...*

---

## 🙌 Credits

This project was created primarily for learning and skill development. It was inspired by:

- 🔗 [Enabling Achievements in Stellaris With Mods (All game versions) [SRE]](https://steamcommunity.com/sharedfiles/filedetails/?id=2460079052)