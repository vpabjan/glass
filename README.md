# Glass

**Glass** is a lightweight X11 floating window manager written in C using Xlib.\
It is designed to be simple, fast, and highly configurable.


Check out the [Glass wiki](https://github.com/vpabjan/glass/wiki) for details and [release](RELEASE.md) for updates.

---

## Features

- Core window management: move, resize, close, and move windows between viewports
- Lightweight and minimal (~2.7 MB RAM usage)
- Compatible with status bars such as [polybar](https://github.com/polybar/polybar) (recommended) or **xfce4-panel**
- Compatible with compositors such as [picom](https://github.com/yshui/picom) (recommended)

---

## Philosophy

Glass aims to be stable, efficient, simple, performant, and highly configurable.

It is explicitly viewport-focused. As a result, window minimization is not supported and will not be added by design. Likewise, window decorations (except borders) are intentionally omitted. A user skilled enough to use Glass is expected to distinguish their windows and remember the small, essential set of keybindings.

Glass favors clarity, explicit control, and minimal abstraction over convenience features.

### But Why?

No other window manager could *just* do it right.

---

<img width="960" height="540" alt="Screenshot_20260204_131856" src="https://github.com/user-attachments/assets/d0e6742c-d6da-4024-b685-f5f5515073ec" />

---


## Installation

This section has been removed as per the existance of the [Glass wiki](https://github.com/vpabjan/glass/wiki). Refer to the wiki for installation and configuration steps. You can find the installation manual on the page [Installing Glass](https://github.com/vpabjan/glass/wiki/Installing-Glass).

---

## Contributing

Glass is intentionally minimal.
If you want to contribute, consider:
- Adding or refining keybindings
- Improving viewport or window handling
- Improving documentation or clarity

Keep changes simple, explicit, and aligned with the project’s philosophy.

---

## License

Glass is licensed under the [GNU General Public License v3.0](LICENSE).
