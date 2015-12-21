# storymod
Story Mod for ETS2 and ATS Truck Simulators
https://sites.google.com/site/storymodsite/

src/linux/storyserver
=====================
This program is a story server for Euro Truck Simulator 2 and
American Truck Simulator. It reads a driving trace from either a
local file or a UDP stream connected to the simulator and
periodically updates a local HTML file to display the current story
state to the player. Stories are loaded from a given directory.

src/windows/telemetry_server
============================
This program is a telemetry UDP stream client for Euro Truck Simulator 2 and
American Truck Simulator. It waits for a storymod server connection and streams
the truck state and position.
