
/**************************************************************************************
 * STORYMOD
 *
 * Website: https://sites.google.com/site/storymodsite
 * GitHub: https://github.com/lcudenne/storymod
 * Author: Loic Cudennec <loic@cudennec.fr>
 *
 * This program is a telemetry UDP stream client for Euro Truck Simulator 2 and
 * American Truck Simulator. It waits for a storymod server connection and streams
 * the truck state and position.
 *
 **************************************************************************************
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 **************************************************************************************/

#define CLIENT_PORT 8888


#define DATAGRAM_TYPE_POSITION 0
#define DATAGRAM_TYPE_CARGO 1
#define DATAGRAM_TYPE_LBLINKER 2
#define DATAGRAM_TYPE_RBLINKER 3
#define DATAGRAM_TYPE_TRAILER_CONNECTED 4


void
initNetwork();

void
closeNetwork();

void
sendPositionToUDP(float x, float y, float z);

void
sendConfigToUDP(unsigned int type, char * value);
