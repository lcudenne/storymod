# storymod
Story Mod for ETS2 and ATS Truck Simulators
https://sites.google.com/site/storymodsite/

Euro Truck and American Truck Simulators are based on simple A-to-B load-and-deliver
missions without any narrative elements. This Story Mod aims at providing tools to
build stories on top of the game. The basic idea is to read telemetry data such as
the truck and trailer properties, the player world position, and to run a parallel
story depending on these information. The story is displayed in a regular web browser.

src/windows/telemetry_server
============================
This program is a telemetry UDP stream client for Euro Truck Simulator 2 and
American Truck Simulator. It waits for a storymod server connection and streams
the truck state and position.

Download the telemetry server DLL files (TBA) or build from source with Microsoft Visual Studio.

Put the 32bits DLL into bin/win_x86/plugins/ or the 64bits DLL into bin/win_x64/plugins/
depending on whether you want to play 32 or 64bits.

Get the IP of this machine, you will need it to run the server.

Start the game before the storyserver. Once you are able to drive, start the server.

src/linux/storyserver
=====================
This program is a story server for Euro Truck Simulator 2 and
American Truck Simulator. It reads a driving trace from either a
local file or a UDP stream connected to the simulator and
periodically updates a local HTML file to display the current story
state to the player. Stories are loaded from a given directory.

Build the server (you need libxml2-dev):
chmod u+x build.sh
./build.sh

Start the server (once the ETS2 / ATS simulator is running):
./storyserver --ip <this_machine_ip> <the_simulator_ip> --story /path/to/my/stories/

Open the resulting index.html file within a browser.
