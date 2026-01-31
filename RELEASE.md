# Version 0.0.1

+ configuration with ~/.glass/glass.conf
+ sloppy focus
+ shell build & install scripts
+ basic window management
+ viewports
+ ewmh
+ window cycling
+ many more...

# Version 0.0.2

+ added displays
+ added fullscreen
+ stability improvements on viewport switching
+ drag window across viewports
+ disable rc.sh thru config

---

# Version 0.0.3

- workspaces
+ viewports
+ upon switching viewports, the last window is now remembered and reverted to when switching back
+ issues with focus being uncertain and buggy on viewport switching until user intervation by change of focus manually
+ border behavior fixed
+ other bug fixes

---

# Version 0.0.4

+ added nogaps option for displays, added primary option for displays
+ when switching to an empty viewport the mouse is now centered to the primary display if warp_pointer is enabled and there is a display flagged as primary
+ log_windows option added to config. enable or disable window resolution and position being logged.
+ ensured no reliance on bash

## TODO

+ improve panel handling. this will be done with an eventual include/panel.c
