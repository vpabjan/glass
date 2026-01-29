# Glass WM

**Glass** is a lightweight X11 window manager written in C using Xlib.\
It is designed to be simple, fast, and highly configurable.

---

## Features

- Core window management: move, resize, close, and move windows between workspaces
- Lightweight and minimal (~2.7 MB RAM usage)
- Compatible with status bars such as **polybar** (recommended) or **xfce4-panel**
- Compatible with compositors such as **picom** (recommended)

---

## Philosophy

Glass aims to be stable, efficient, simple, performant, and highly configurable.

It is explicitly workspace-focused. As a result, window minimization is not supported and will not be added by design. Likewise, window decorations are intentionally omitted. A user skilled enough to use Glass is expected to distinguish their windows and remember the small, essential set of keybindings.

Glass favors clarity, explicit control, and minimal abstraction over convenience features.

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

### Compositors, Bars, and Wallpapers

Glass runs a startup script `~/.glass/rc.sh` after the X display is initialized.\
You can use it to launch compositors, panels, or set wallpapers.

**Example ****rc.sh****:**

```bash
#!/bin/bash
# Set wallpaper
feh --bg-scale ~/path/to/wallpaper.jpg &

# Launch panel/bar
polybar &

# Launch compositor
picom &

# Launch notification daemon
dunst &

# Launch polkit agent
lxqt-policykit-agent &

# Set keyboard layout(s), important!
setxkbmap -layout us,cz -variant ,qwerty -option grp:alt_shift_toggle
```

Make sure the script is executable:

```bash
chmod +x ~/.glass/rc.sh
```

*It will already be executable if Glass was installed using `install.sh.`*

---

## Contributing

Glass is intentionally minimal.
If you want to contribute, consider:
- Adding or refining keybindings
- Improving workspace or window handling
- Improving documentation or clarity

Keep changes simple, explicit, and aligned with the project’s philosophy.

---

## License

Glass is licensed under the [GNU General Public License v3.0](LICENSE).

