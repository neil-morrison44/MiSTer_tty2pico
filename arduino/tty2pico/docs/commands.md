# Commands

## tty2oled Commands

These commands are adapted from `tty2oled` and should be (mostly) compatible:

| Command | Function | Example |
| ------- | -------- | ------- |
| CMDBYE | Show Sorgelig's Cat Icon | `CMDBYE` |
| CMDCLS | Clear and Update the Display | `CMDCLS` |
| CMDCOR | Command to announce core change, will try to display in the following order:<br>`[corename].loop.gif`<br>`[corename].gif`<br>`[corename].png` | `CMDCOR,[corename]`<br>e.g.<br>`CMDCOR,SNES`<br>`CMDCOR,19XX` |
| CMDDOFF | Switch Display off | `CMDDOFF` |
| CMDDON | Power Display on | `CMDDON` |
| CMDROT | Rotate screen relative to starting position (0=none, 1=180°, 2=90°, 3=270°) | `CMDROT,0` for no rotation<br>`CMDROT,1` to flip screen |
| CMDSAVER | Disable or Enable the ScreenSaver (currently only toggle) | `CMDSAVER` |
| CMDSHTEMP | Alias to `CMDSHSYSHW` since that screen displays the CPU temp | `CMDSHTEMP` |
| CMDSHSYSHW | Show tty2pico system information | `CMDSHSYSHW` |
| CMDSNAM | Show actual loaded Corename | `CMDSNAM` |
| CMDSORG | Show Startup screen, also an alias to `CMDSHSYSHW` | `CMDSORG` |
| CMDSWSAVER | Switch screensaver on or off | `CMDSWSAVER,0` to disable<br>`CMDSWSAVER,1` to enable |
| CMDTEST | Show system info and some test graphics | `CMDTEST` |
| CMDTXT | Send text to the display | `CMDTXT,Can you see this?`<br>Not currently compatible with `tty2oled` command |

# tty2pico Commands

These commands are specific to `tty2pico`:

| Command | Function | Example |
| ------- | -------- | ------- |
| CMDSHOW | Show an image from the active storage device | `CMDSHOW,/logos/pattern.loop.gif` |
