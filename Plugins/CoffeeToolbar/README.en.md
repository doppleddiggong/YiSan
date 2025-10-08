# CoffeeToolbar

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.0-blue.svg)](https://www.unrealengine.com/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/Docs-GitHub%20Pages-lightgrey.svg)](https://doppleddiggong.github.io/CoffeeToolbar/)

🌐 Languages: [한국어](README.ko.md) | [English](README.en.md)

**Version: 1.0.0**

![Coffee Toolbar Icon](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/icon_coffee_cover_type3.png)

`Coffee Toolbar` is a utility toolbar plug-in designed to enhance productivity in the Unreal Engine editor. By launching frequently used actions with a single click, it minimizes repetitive work and speeds up development.

![How it works](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/use_guide.png?raw=true)

## Key Features

* **Run custom console commands** – Execute any number of pre-defined console commands from a configuration file with one button press.
* **Level management tools** – Build every actor in the current level, build lighting, and access other helpers that streamline level management.
* **High-resolution screenshots** – Capture screenshots at preset multipliers (for example, 2× or 4×) and open the output folder instantly.
* **Configuration-driven extensibility** – Customize which features appear in the toolbar and their order through `.ini` files.

## Installation

### 1. Install from GitHub Releases (Recommended)

1. Download the latest `CoffeeToolbar_X.X.X.zip` file from the [GitHub Releases page](https://github.com/doppleddiggong/CoffeeToolbar/releases).
2. Extract the downloaded ZIP file.
3. Copy the extracted `CoffeeToolbar` folder into your Unreal Engine project’s `Plugins` directory (for example, `YourProject/Plugins/`) or into the engine installation directory at `Engine/Plugins` (for example, `C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/`).
4. Restart the Unreal Engine editor.
5. In the editor menu, choose `Edit > Plugins`, search for “CoffeeToolbar,” and enable it.
6. Restart the editor if prompted after enabling the plug-in.

### 2. Manual installation in a project

1. Create a `Plugins` folder in your project directory if it does not already exist.
2. Copy the `CoffeeToolbar` folder into the `Plugins` directory.
3. Restart the project, then go to `Edit > Plugins` and enable `CoffeeToolbar`.

**Path example**: `[YourProjectDirectory]/Plugins/CoffeeToolbar`

### 3. Manual installation in the engine

1. Navigate to the `Engine/Plugins` directory inside your Unreal Engine installation path.
2. Copy the `CoffeeToolbar` folder into the `Engine/Plugins` directory.
3. Restart the engine, then go to `Edit > Plugins` and enable `CoffeeToolbar`.

**Path example**: `C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/CoffeeToolbar`

## How to Use

1. After activation, a new **coffee icon** appears in the main toolbar at the top of the editor.
2. Click the toolbar icon to open a drop-down menu that lists all registered feature buttons.
3. Click any button to execute the corresponding action immediately.

## Configuration Guide

You can configure and customize every aspect of `Coffee Toolbar` through the Unreal Engine **Project Settings** menu.

![Editing settings](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/modify_guide.png?raw=true)

### 1. Access the plug-in settings

In the Unreal Editor, go to **Edit > Project Settings** and select **Coffee Toolbar** under the **Plugins** section in the left-hand menu.

### 2. Default settings

Configure the order of toolbar buttons, their icons, and the commands they execute.

![Default settings](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_defaultSetting.png?raw=true)

### 3. Adding custom commands

In the **Custom Commands** section, add new entries to register console commands or editor functions as toolbar buttons.

![Command settings](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_command.png?raw=true)

### 4. Configuring the Level Selector

Use the **Level Selector** to register frequently used levels and load them from the toolbar with a single click.

**Button overview:**

* **Add Current Level** – Adds the level currently loaded in the editor to the list.
* **Load Selected Level** – Loads the level selected in the list into the editor.
* **Remove Selected Level** – Deletes the selected level from the list.
* **Clear All Levels** – Removes every registered level from the list.

![Level selector settings](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_levelselector.png?raw=true)

### 5. Screenshot capture feature

Use the **Screenshot** tools to capture high-resolution screenshots of the active viewport or open the capture folder instantly.

#### Available actions
- **Capture at 1×** – Capture at the current resolution.
- **Capture at 2×** – Capture at 2× resolution.
- **Capture at 4×** – Capture at 4× resolution.
- **Capture at 8×** – Capture at 8× resolution.
- **Open capture folder** – Open the folder that contains your saved screenshots.

![Screenshot settings](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_screenshot.png?raw=true)

---

## Configuration Details

Use the `Project Settings > Plugins > Coffee Toolbar` menu to fine-tune how the plug-in behaves.

* **Register console commands** – Add or edit the list of custom console commands available through the toolbar.
* **Toggle toolbar features** – Choose which features appear in the toolbar.

All configuration data is stored in the project’s `Config/ToolbarSettings.ini` file.

## License

This plug-in is distributed under the MIT License. See the `LICENSE` file for details.

## Author

**dopple**

- GitHub: [doppleddiggong](https://github.com/doppleddiggong)
- LinkedIn: [Jubaek Bae](https://www.linkedin.com/in/주백-배-4a814527b/)
- Email: ju100.bae@gmail.com
