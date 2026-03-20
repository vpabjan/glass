# Glass

**Glass** is an X11 environment written in C centered around its core window manager, *glass*, composed of small desktop components while remaining externally composable and extensible. Glass may both be used as a desktop itself or one may simply extend an existing desktop with its components. It is designed to be simple, configurable and fast. As for compilation Glass is rich in security flags and compiles on -O3.

Check out the [Glass wiki](https://github.com/vpabjan/glass/wiki) for details and [release](RELEASE.md) or [release history](https://github.com/vpabjan/glass/releases) for updates.

---

<img width="960" height="540" alt="Screenshot_20260204_131856" src="https://github.com/user-attachments/assets/d0e6742c-d6da-4024-b685-f5f5515073ec" />

---

## Components

### glass

*glass* is the core window manager of the Glass environment. It links against Xlib. *glass* provides: 


- Core window management: move, resize and close.
- Viewports: multiple spaces for the user to work on.
- Viewport window management: move windows between viewports, *free* windows to make them appear on every viewport.
- Window modes: float, fullscreen, *mono* (leaves space for a bar) and *free*.
- Configuration: set custom keybinds, add shortcuts, configure displays, set padding, etc.
- Live config reloading: with a set keybind the user may reload the config at any moment with virtually no overhead.
- Compositor support: extensible with external compositors such as [picom](https://github.com/yshui/picom) (recommended).
- Bar, panel & widget support: extensible with external widgets such as [polybar](https://github.com/polybar/polybar) (recommended).
- Wallpaper support: set any wallpaper using *glassbg* or an external tool such as [feh](https://github.com/derf/feh).
- Other desktop extensions: *glassdesktop* or live wallpapers.

### glassbg

*glassbg* is a tool for changing wallpapers. It provides scale, tile and center options for the given image.

### glassdesktop

*glassdesktop* is an experimental desktop layer providing a simple, spatial interface for managing files, folders, and commands. It operates on a grid-based layout with a movable view, allowing icons to exist in a persistent coordinate space rather than a fixed screen position. It is designed to remain minimal and independent, integrating with glass without being tightly coupled to it.

---

## Showcase

https://github.com/user-attachments/assets/11bcdf36-5fc2-4043-9e00-aa9a198d0ec5

This setup uses the glass window manager with polybar, feh and glassdesktop.

---

## Philosophy

Glass aims to be stable, efficient, simple, performant, and highly configurable.

*glass* is explicitly viewport-focused. As a result, window minimization is not supported and will not be added by design. Likewise, window decorations (except borders) are intentionally omitted. A user skilled enough to use Glass is expected to distinguish their windows and remember the small, essential set of keybindings.

Glass favors clarity, explicit control, and minimal abstraction over convenience features.

### But Why?

No other window manager could *just* do it right.

---

## ~~Installation~~

This section has been removed as per the existance of the [Glass wiki](https://github.com/vpabjan/glass/wiki). Refer to the wiki for installation and configuration steps. You can find the installation manual on the page [Installing Glass](https://github.com/vpabjan/glass/wiki/Installing-Glass).

---

## Testing
| Operating system | Architecture | Compiles? | Runs? | Notes |
|------------------|--------------|-----------|-------|-------|
| Alpine Linux     | i686         | ✔✔        | ✔✔    | requires imlib2-dev, libx11-dev |
| Alpine Linux     | x86_64       | ✔✔        | ✔✔    | requires imlib2-dev, libx11-dev | 
| Arch Linux       | x86_64       | ✔✔        | ✔✔    | |
| Arch Linux 32    | x686         | ✔✔        | ✔✔    | | 
| FreeBSD          | x86_64       | ✔✔        | ✘     | |

### Legend

| Sign | Compiling | Running |
| ---- | -------   | ------ |
| ✔✔   | Compiles successfully with no tweaks.                                 | Runs without issues or tweaks. |
| ✔    | Compiles successfully with tweaks.                                    | Runs but with stability or compatibility issues. |
| ✘    | Does not compile and tweaks have not been tested.                     | Does not run by default and tweaks have not been tested. |
| ✘✘   | Does not compile with tweaks.                                         | Does not run after further testing. |
| ✘✘✘  | Compiling and testing dropped                                         ||


---


## Contributing

Keep changes simple, explicit, and aligned with the project’s philosophy.

---

## License

Glass is licensed under the [GNU General Public License v3.0](LICENSE).
