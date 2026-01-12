# Glass WM

**Glass** is a lightweight X11 window manager written in C using Xlib.\
It is designed to be simple, fast, and highly configurable.

---

## Features

- Minimal and lightweight
- Workspace management (1-9)
- Basic window manipulation: move, resize, close
- Compatible with compositors, bars, and wallpapers

---

<img width="960" height="540" alt="A screenshot showing Glass" src="https://github.com/user-attachments/assets/2d158a98-b1ab-4438-a9e2-b6c1c46581ea" />



## Installation

Clone the repository:

```bash
git clone https://github.com/vpabjan/glass.git
cd glass
chmod +x install.sh
./install.sh
```

Then start it using start-glass on a TTY. Glass is **not** installed system-wide, it lives in ~./.glass.

---

## Configuration

### Keybinds

Keybinds are defined in `glass.c`. To change them:

1. Open `glass.c`.
2. Modify the `MOD` key or any `KEY_yourkey` definitions.
3. Recompile using:

```bash
./build.sh
```

**Default keybinds:**

| Action           | Key Combination |
| ---------------- | --------------- |
| Launcher         | mod + d         |
| Terminal         | mod + Return    |
| Move window      | mod + LMB       |
| Resize window    | mod + RMB       |
| Close window     | mod + q         |
| Switch workspace | mod + 1-9       |

---

### Compositors, Bars, and Wallpapers

Glass runs a startup script `~/.glass/rc.sh` after the X display is initialized.\
You can use it to launch compositors, panels, or set wallpapers.

**Example ****rc.sh****:**

```bash
#!/bin/bash

export XDG_CURRENT_DESKTOP=Glass
export XDG_SESSION_DESKTOP=Glass
export DESKTOP_SESSION=glass
export XDG_SESSION_TYPE=x11
export XCURSOR_THEME=Adwaita
export XCURSOR_SIZE=24

# Set wallpaper
feh --bg-scale ~/path/to/wallpaper.jpg &

# Launch panel/bar
polybar &

# Launch compositor
picom &
```

Make sure the script is executable:

```bash
chmod +x ~/.glass/rc.sh
```

---

## Contributing

Glass is meant to be simple and minimal.\
If you want to contribute, you can:

- Add new keybinds
- Improve workspace or window handling
- Enhance documentation

---

## License

Glass is licensed under the [GNU General Public License v3.0](LICENSE).

