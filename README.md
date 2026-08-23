<div align="center">

<img src=".github/images/AppBanner.jpeg" alt="MonkeySounds Banner" width="100%"/>

# MonkeySounds

**Add satisfying keyboard and mouse sounds to every click and keystroke on Windows.**

[![Build](https://github.com/NSTechBytes/MonkeySounds/actions/workflows/build.yml/badge.svg)](https://github.com/NSTechBytes/MonkeySounds/actions/workflows/build.yml)
[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](LICENSE.txt)
[![GitHub release](https://img.shields.io/github/v/release/NSTechBytes/MonkeySounds)](https://github.com/NSTechBytes/MonkeySounds/releases/latest)

[Download](#installation) · [Sound Profiles](#sound-profiles) · [Create a Profile](#creating-your-own-profile) · [Profile JSON Reference](#profilejson-reference) · [Discord](https://discord.gg/fZejMxtMhf) · [Support on Patreon](https://www.patreon.com/c/nstechbytes)

</div>

https://res.cloudinary.com/xkzylop3/video/upload/v1787455349/monkeysounds_demo.mp4

<div align="center">

<br/>

[![Watch Demo](https://img.shields.io/badge/▶%20Watch%20Demo-Click%20to%20Play-blue?style=for-the-badge&logo=cloudinary)](https://res.cloudinary.com/xkzylop3/video/upload/v1787455349/monkeysounds_demo.mp4)

</div>

---

## What is MonkeySounds?

MonkeySounds is a lightweight Windows app that plays realistic mechanical keyboard and mouse click sounds as you type and click — just like the popular browser extension, but system-wide for **every** application on your PC.

- Runs silently in the system tray
- Zero performance impact — uses a low-latency audio engine
- Ships with **16 keyboard profiles** (Cherry MX, Holy Panda, Alpaca, and more)
- Fully customizable — create your own profiles with any audio files you like
- Portable or installed — your choice

---

## Requirements

| | |
|---|---|
| **OS** | Windows 10 or Windows 11 (64-bit) |
| **Runtime** | None — the EXE is fully self-contained |

---

## Installation

### Option 1 — Installer (recommended for most users)

1. Go to the [Releases page](https://github.com/NSTechBytes/MonkeySounds/releases/latest)
2. Download `MonkeySounds_Setup_vX.X.X.X.exe`
3. Run it and follow the setup wizard

During installation you will be asked to choose a mode:

| Mode | Description |
|---|---|
| **Standard** | Installs to `Program Files`, creates Start Menu & Desktop shortcuts, adds an uninstaller to Windows Settings |
| **Portable** | Copies all files to a folder you choose — no registry entries, no shortcuts. Ideal for USB drives or shared PCs |

### Option 2 — Portable ZIP

If you prefer not to run an installer at all:

1. Download `MonkeySounds-dist` from the latest [Actions artifact](https://github.com/NSTechBytes/MonkeySounds/actions)
2. Extract the folder anywhere you like
3. Double-click `MonkeySounds.exe`

> **Tip:** In portable mode MonkeySounds automatically detects that `settings.json` lives next to the EXE and stores all data in that same folder instead of `%APPDATA%`.

---

## First Launch

When MonkeySounds starts it **hides to the system tray** automatically.

Look for the monkey icon near the clock in your taskbar:

| Action | Result |
|---|---|
| **Double-click** the tray icon | Opens the main window |
| **Right-click** the tray icon | Quick menu: mute keyboard, mute mouse, open settings, exit |

The app starts playing sounds immediately — just open any text editor and start typing!

---

## Main Window

### Sounds Tab

This is where you control everything about how the sounds work.

<details>
<summary><strong>Keyboard Sounds section</strong></summary>

| Control | What it does |
|---|---|
| **Enable Keyboard Sounds** checkbox | Turns keyboard sounds on or off |
| **+ New Profile...** | Opens the Profile Wizard to create a new sound profile from scratch |
| **Preset** dropdown | Switch between installed keyboard profiles |
| **★** (star button) | Mark/unmark the current profile as a favourite — favourites appear at the top of the list |
| **ⓘ** (info button) | Shows the profile name, author, description and number of sounds |
| **▶** (play button) | Plays a quick preview of the current profile |
| **Export** | Saves the current profile as a `.zip` file you can share |
| **Import ZIP...** | Loads a profile from a `.zip` file someone shared with you |
| **Volume** slider | Sets how loud the keyboard sounds are (0–100%) |

</details>

<details>
<summary><strong>Mouse Sounds section</strong></summary>

The Mouse Sounds section works exactly the same way as Keyboard Sounds, but controls the sounds played when you click your mouse buttons.

</details>

### Settings Tab

| Option | Description |
|---|---|
| **Current Version** | Shows the installed version of MonkeySounds |
| **Check for Updates** | Opens the GitHub releases page in your browser |
| **Auto-start on Windows startup** | MonkeySounds will launch automatically when you log in |
| **Show notification on startup** | Shows a tray balloon when the app starts in the background |

### About Tab

Shows the app logo, version info, and links to GitHub, Discord, and Patreon.

---

## Sound Profiles

MonkeySounds ships with the following profiles out of the box:

### Keyboard Profiles

| Profile | Based on |
|---|---|
| `alpaca` | Alpaca V2 switches |
| `apex-pro-tkl-v2` | SteelSeries Apex Pro TKL |
| `banana-split` | Banana Split switches |
| `gateron-black-ink` | Gateron Black Ink switches |
| `gateron-red-ink` | Gateron Red Ink switches |
| `holy-panda` | Holy Panda switches |
| `ios` | iPhone / iOS keyboard |
| `logitech-g915-tkl-brown` | Logitech G915 TKL (Brown) |
| `mx-black` | Cherry MX Black |
| `mx-blue` | Cherry MX Blue |
| `mx-brown` | Cherry MX Brown |
| `mx-speed-silver` | Cherry MX Speed Silver |
| `nk-cream` | NovelKeys Cream switches |
| `opera-gx` | Opera GX browser keyboard |
| `telios-v2` | Telios V2 switches |
| `typewriter` | Vintage typewriter |

### Mouse Profiles

| Profile | Based on |
|---|---|
| `g502x-wireless` | Logitech G502 X Wireless |

---

## Creating Your Own Profile

MonkeySounds includes a built-in **Profile Wizard** that walks you through creating a custom profile step by step — no JSON editing required.

Click **+ New Profile...** in either the Keyboard or Mouse section to open it.

<div align="center">
<img src=".github/images/ProfileWizard/1.png" alt="Profile Wizard Step 1" width="48%"/>
<img src=".github/images/ProfileWizard/2.png" alt="Profile Wizard Step 2" width="48%"/>
</div>

<div align="center">
<img src=".github/images/ProfileWizard/3.png" alt="Profile Wizard Step 3" width="48%"/>
<img src=".github/images/ProfileWizard/4.png" alt="Profile Wizard Step 4" width="48%"/>
</div>

### What the wizard asks for

1. **Profile name, author, description** — so you can identify it later
2. **Default sounds** — audio files (`.wav`, `.ogg`, `.mp3`) played for any key/button not specifically mapped. You can add multiple files and MonkeySounds will randomly pick one each time for variety
3. **Specific bindings** *(optional)* — assign different sounds to individual keys (e.g. a louder thock for `Space` and `Enter`)
4. **Options** — activate the profile immediately after creation, and optionally export it as a `.zip` for sharing

---

## profile.json Reference

Every profile is a folder containing a `profile.json` file and your audio files. You can create or edit `profile.json` by hand in any text editor (Notepad, VS Code, etc.) for full control.

### Folder structure

```
MyProfile/
├── profile.json
├── press_key1.mp3
├── press_key2.mp3
├── release_key.mp3
├── press_space.mp3
└── press_enter.mp3
```

### Full keyboard profile.json

```json
{
  "profile": {
    "name": "My Profile",
    "author": "YourName",
    "description": "A custom keyboard sound profile"
  },
  "keys": {
    "default": [
      "s1",
      "s2"
    ],
    "other": [
      {
        "keys": ["space"],
        "sound": "s_space"
      },
      {
        "keys": ["enter", "return"],
        "sound": "s_enter"
      },
      {
        "keys": ["backspace", "delete"],
        "sound": "s_back"
      }
    ]
  },
  "sources": [
    {
      "id": "s1",
      "source": {
        "press": "press_key1.mp3",
        "release": "release_key.mp3"
      }
    },
    {
      "id": "s2",
      "source": {
        "press": "press_key2.mp3",
        "release": "release_key.mp3"
      }
    },
    {
      "id": "s_space",
      "source": {
        "press": "press_space.mp3"
      }
    },
    {
      "id": "s_enter",
      "source": {
        "press": "press_enter.mp3"
      }
    },
    {
      "id": "s_back",
      "source": {
        "press": "press_back.mp3"
      }
    }
  ]
}
```

### Full mouse profile.json

```json
{
  "profile": {
    "name": "My Mouse",
    "author": "YourName",
    "description": "Custom mouse click sounds",
    "device": "mouse"
  },
  "buttons": {
    "default": [
      "click_left"
    ],
    "other": [
      {
        "buttons": ["left"],
        "sound": "click_left"
      },
      {
        "buttons": ["right"],
        "sound": "click_right"
      },
      {
        "buttons": ["middle"],
        "sound": "click_middle"
      }
    ]
  },
  "sources": [
    {
      "id": "click_left",
      "source": {
        "press": "left_down.wav",
        "release": "left_up.wav"
      }
    },
    {
      "id": "click_right",
      "source": {
        "press": "right_down.wav",
        "release": "right_up.wav"
      }
    },
    {
      "id": "click_middle",
      "source": {
        "press": "middle.wav"
      }
    }
  ]
}
```

### Field explanations

| Field | Type | Description |
|---|---|---|
| `profile.name` | string | Profile display name shown in the preset dropdown |
| `profile.author` | string | Author name shown in the info dialog |
| `profile.description` | string | Short description shown in the info dialog |
| `profile.device` | string | `"keyboard"` (default) or `"mouse"` |
| `keys.default` | array of source IDs | Sound(s) played for any key not listed in `other`. Multiple IDs = random pick each press |
| `keys.other` | array of rules | Per-key overrides. Each rule has a `keys` array and a `sound` source ID |
| `buttons.default` | array of source IDs | Default sound(s) for mouse buttons not listed in `other` |
| `buttons.other` | array of rules | Per-button overrides. Each rule has a `buttons` array and a `sound` source ID |
| `sources` | array | Defines each sound. Each entry has an `id` and a `source` with `press` and optional `release` file paths |

> **Tip:** The `release` field is optional. If omitted, no sound plays when the key is released.  
> **Tip:** Audio files can be `.wav`, `.mp3`, or `.ogg`. Paths are relative to the profile folder.  
> **Tip:** To add variation, list multiple source IDs in `default` — MonkeySounds picks one at random each time.

---

## Key Aliases

When writing `keys.other` rules in `profile.json`, use these names in the `keys` array. MonkeySounds recognizes multiple aliases for each key — use whichever you prefer.

### Regular Keys

| Key | Aliases you can use |
|---|---|
| Letters | `a` – `z` (lowercase) |
| Numbers (top row) | `0` – `9` |

### Special Keys

| Key | Aliases |
|---|---|
| Space | `space` |
| Enter / Return | `enter`, `return` |
| Backspace | `backspace`, `back`, `delete` |
| Delete (Del) | `delete`, `del` |
| Tab | `tab` |
| Escape | `escape`, `esc` |
| Caps Lock | `capslock`, `caps_lock`, `caps` |

### Modifier Keys

| Key | Aliases |
|---|---|
| Left Shift | `shift_l`, `lshift`, `shift` |
| Right Shift | `shift_r`, `rshift`, `shift` |
| Either Shift | `shift` |
| Left Ctrl | `ctrl_l`, `lctrl`, `ctrl`, `control` |
| Right Ctrl | `ctrl_r`, `rctrl`, `ctrl`, `control` |
| Either Ctrl | `ctrl`, `control` |
| Left Alt | `alt_l`, `lalt`, `alt` |
| Right Alt / AltGr | `alt_r`, `ralt`, `alt_gr`, `alt` |
| Either Alt | `alt`, `alt_gr` |
| Windows / Super key | `cmd`, `win`, `windows`, `meta`, `super` |
| Context Menu | `menu`, `apps`, `context_menu` |

### Navigation Keys

| Key | Aliases |
|---|---|
| Insert | `insert`, `ins` |
| Home | `home` |
| End | `end` |
| Page Up | `page_up`, `pageup`, `pgup` |
| Page Down | `page_down`, `pagedown`, `pgdn` |
| Left Arrow | `left`, `arrow_left`, `leftarrow` |
| Right Arrow | `right`, `arrow_right`, `rightarrow` |
| Up Arrow | `up`, `arrow_up`, `uparrow` |
| Down Arrow | `down`, `arrow_down`, `downarrow` |

### Function Keys

| Key | Alias |
|---|---|
| F1 – F24 | `f1`, `f2`, … `f24` |

### Numpad Keys

| Key | Alias |
|---|---|
| Numpad 0 – 9 | `numpad0`, `numpad1`, … `numpad9` |
| Num Lock | `numlock`, `num_lock` |

### System Keys

| Key | Aliases |
|---|---|
| Print Screen | `printscreen`, `print_screen`, `prtscn` |
| Scroll Lock | `scrolllock`, `scroll_lock` |
| Pause / Break | `pause` |

---

### Mouse Button Aliases

Use these in `buttons.other` rules in a mouse `profile.json`:

| Button | Aliases |
|---|---|
| Left click | `left`, `click_left`, `primary`, `lclick`, `button1` |
| Right click | `right`, `click_right`, `secondary`, `rclick`, `button2` |
| Middle click | `middle`, `click_middle`, `wheel`, `mclick`, `button3` |

---

## Sharing profiles

- **Export**: click **Export** next to any preset to save it as a `.zip`
- **Import**: click **Import ZIP...** and pick the `.zip` — MonkeySounds extracts it and loads it automatically

The ZIP just needs to contain the profile folder (with `profile.json` at its root).

---

## Where are my settings stored?

| Mode | Location |
|---|---|
| **Standard install** | `%APPDATA%\MonkeySounds\` |
| **Portable** | Same folder as `MonkeySounds.exe` |

Custom profiles you import are saved inside `Sounds\Keyboard\Custom\` or `Sounds\Mouse\Custom\` under the data directory.

---

## Uninstalling

**Standard install:** Go to *Windows Settings → Apps → MonkeySounds → Uninstall*

During uninstall you will be asked:
> *"Also remove all user data (settings, custom sound profiles) from %APPDATA%\MonkeySounds"*

- Leave it **unchecked** to keep your settings and custom profiles
- Check it to do a clean removal of everything

**Portable:** Just delete the folder.

---

## Building from Source

### Requirements

- Visual Studio 2022 (with the **Desktop development with C++** workload)
- Windows SDK 10.0

### Steps

```powershell
git clone https://github.com/NSTechBytes/MonkeySounds.git
cd MonkeySounds

# Debug build
.\Build.ps1

# Release build (also creates dist\ folder)
.\Build.ps1 -Configuration Release -Platform x64
```

The compiled EXE ends up in `x64\Release\MonkeySounds.exe`.
The `dist\` folder contains the EXE and `Sounds\` folder ready for distribution.

---

## Contributing

Pull requests and profile submissions are welcome!

- **Bug reports / feature requests** → [Open an issue](https://github.com/NSTechBytes/MonkeySounds/issues)
- **New sound profiles** → submit a PR adding your profile folder under `Sounds\Keyboard\` or `Sounds\Mouse\`

---

## Community & Support

| | |
|---|---|
| 💬 **Discord** | [discord.gg/fZejMxtMhf](https://discord.gg/fZejMxtMhf) |
| ❤️ **Patreon** | [patreon.com/c/nstechbytes](https://www.patreon.com/c/nstechbytes) |
| 🐙 **GitHub** | [NSTechBytes/MonkeySounds](https://github.com/NSTechBytes/MonkeySounds) |

---

## License

MonkeySounds is released under the **GNU General Public License v2.0**.
See [LICENSE.txt](LICENSE.txt) for the full text.

© 2026 MonkeySounds — NSTechBytes
