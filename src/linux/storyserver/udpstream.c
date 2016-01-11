
/**************************************************************************************
 * STORYMOD
 *
 * Website: https://sites.google.com/site/storymodsite
 * GitHub: https://github.com/lcudenne/storymod
 * Author: Loic Cudennec <loic@cudennec.fr>
 *
 * This program is a story server for Euro Truck Simulator 2 and
 * American Truck Simulator. It reads a driving trace from either a
 * local file or a UDP stream connected to the simulator and
 * periodically updates a local HTML file to display the current story
 * state to the player. Stories are loaded from a given directory.
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


/* ------------------------------------------------------------------------- */

/* malloc */
#include <stdlib.h>
/* sprintf */
#include <stdio.h>

/* udp */
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

/* ------------------------------------------------------------------------- */

#include "udpstream.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */




/* ------------------------------------------------------------------------- */

void
_UDP_registerClient(char * hostip, unsigned int hostport,
                    char * remoteip, unsigned int remoteport) {

  struct sockaddr_in regsi_other;
  int regudpsocket = 0; 

  char * datagram = NULL;
  
  _UT_ASSERT(hostip);
  _UT_ASSERT(remoteip);

  regudpsocket =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset((char *) &(regsi_other), 0, sizeof((regsi_other)));
  regsi_other.sin_family = AF_INET;
  regsi_other.sin_port = htons(remoteport);
  inet_aton(remoteip, &(regsi_other.sin_addr));

  datagram = malloc(sizeof(char) * 512);
  _UT_ASSERT(datagram);
  
  sprintf(datagram, "%s %d ", hostip, remoteport);
  fprintf(stdout, "Contacting client %s:%d with host %s:%d\n",
          remoteip, remoteport, hostip, hostport);
  
  sendto(regudpsocket, datagram, strlen(datagram), 0,
         (struct sockaddr *) &(regsi_other), sizeof(regsi_other));

  close(regudpsocket);

  free(datagram);
}

/* ------------------------------------------------------------------------- */

struct sockaddr_in si_me;
struct sockaddr_in si_other;
int udpsocket = 0; 
int slen = 0;

/* ------------------------------------------------------------------------- */


void
_UDP_bindSocket(unsigned int hostport) {

  slen = sizeof(si_other);

  udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(hostport);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(udpsocket, (struct sockaddr *) &si_me, sizeof(si_me));

}

void
_UDP_getNextDatagram(char * datagram, unsigned int size) {

  _UT_ASSERT(datagram);

  recvfrom(udpsocket, datagram, sizeof(char) * size,
           0, (struct sockaddr *) &si_other, (socklen_t *) &slen);

}


void
_UDP_closeSocket() {

   close(udpsocket);

}

/* ------------------------------------------------------------------------- */
