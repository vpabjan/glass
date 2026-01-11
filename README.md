# Glass WM

**Glass** is a lightweight X11 window manager written in C using Xlib.\
It is designed to be simple, fast, and highly configurable by recompiling the source.

---

## Features

- Minimal and lightweight
- Easy keybind customization
- Workspace management (1-9)
- Basic window manipulation: move, resize, close
- Compatible with compositors, bars, and wallpapers

---

## Installation

Clone the repository:

```bash
git clone https://github.com/yourusername/glass.git
cd glass
./build.sh
```

Then start it from your `.xinitrc`:

```bash
exec /path/to/compiled/glass
```

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

