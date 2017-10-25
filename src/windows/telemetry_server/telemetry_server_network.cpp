
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



/* fprintf */
#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* assert */
#include <assert.h>

#include <winsock2.h>
#include <sstream>
#include <string>

#include "telemetry_server_network.h"

using namespace std;

#define UDP_DATAGRAM_SIZE 512

/***********************************************************************************/

char * remote_server_ip = NULL;
unsigned int remote_server_port = 0;

/***********************************************************************************/

#define DEBUG_MODE 1
FILE *udp_log_file = NULL;


void udp_log_line(const char *const text, ...)
{

#ifdef DEBUG_MODE

	if (!udp_log_file) {
		return;
	}
	va_list args;
	va_start(args, text);
	vfprintf(udp_log_file, text, args);
	fprintf(udp_log_file, "\n");
	va_end(args);

#endif

}



/***********************************************************************************/

SOCKET udpsocket;
struct sockaddr_in si_other;

void
initSendUDP() {
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		udp_log_line("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if ((udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		udp_log_line("Could not create socket : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(remote_server_port);
	si_other.sin_addr.S_un.S_addr = inet_addr(remote_server_ip);


}


void
closeSendUDP() {

	closesocket(udpsocket);
	WSACleanup();

}



/***********************************************************************************/

SOCKET getip_udpsocket;
struct sockaddr_in getip_si_other;

void
initGetIP() {

	WSADATA wsa;
	u_long iMode = 1;

	udp_log_line("initGetIP");

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		exit(EXIT_FAILURE);
	}
	if ((getip_udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		exit(EXIT_FAILURE);
	}

	memset((char *)&getip_si_other, 0, sizeof(getip_si_other));
	getip_si_other.sin_family = AF_INET;
	getip_si_other.sin_port = htons(CLIENT_PORT);
	getip_si_other.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(getip_udpsocket, (struct sockaddr *)&getip_si_other, sizeof(getip_si_other));

	ioctlsocket(getip_udpsocket, FIONBIO, &iMode);

}

void
closeGetIP() {


	closesocket(getip_udpsocket);
	WSACleanup();


}

void
getIP() {

	struct sockaddr_in siserver;

	int recv_len = sizeof(siserver);
	char * datagram = NULL;

	datagram = (char *)malloc(sizeof(char) * UDP_DATAGRAM_SIZE);
	assert(datagram);

	memset(datagram, '\0', UDP_DATAGRAM_SIZE);

	recvfrom(getip_udpsocket, datagram, UDP_DATAGRAM_SIZE, 0, (struct sockaddr *) &siserver, &recv_len);

	datagram[UDP_DATAGRAM_SIZE - 1] = '\0';

	if (datagram[0] != '\0') {
		string sdatagram(datagram);
		istringstream iss(sdatagram);
		string subip;
		string subport;
		iss >> subip >> subport;
		remote_server_ip = (char *) subip.c_str();
		remote_server_port = atoi(subport.c_str());
	
		udp_log_line("received connection from %s contacting port %d", remote_server_ip, remote_server_port);

		closeGetIP();
		initSendUDP();
		sendClientVersionToUDP();
	}

	free(datagram);

}

unsigned int
isConnected() {

	unsigned int res = 0;

	if (remote_server_ip != NULL) { res = 1; }

	return res;
}


/***********************************************************************************/

void
sendClientVersionToUDP() {

	char * datagram = NULL;

	if (isConnected() == 1) {

		datagram = (char *)malloc(sizeof(char) * UDP_DATAGRAM_SIZE);
		assert(datagram);

		sprintf(datagram, "%d %d %d ", DATAGRAM_TYPE_CLIENT_VERSION, CLIENT_VERSION_MAJ, CLIENT_VERSION_MIN);

		udp_log_line("sending client version %s to %s:%d", datagram, remote_server_ip, remote_server_port);

		sendto(udpsocket, datagram, strlen(datagram), 0, (struct sockaddr *) &si_other, sizeof(si_other));

		free(datagram);
	}

}


void
sendPositionToUDP(float x, float y, float z, float speed) {

	char * datagram = NULL;

	if (isConnected() == 1) {

		datagram = (char *)malloc(sizeof(char) * UDP_DATAGRAM_SIZE);
		assert(datagram);

		sprintf(datagram, "%d %f %f %f %f ", DATAGRAM_TYPE_POSITION, x, y, z, speed);

		sendto(udpsocket, datagram, strlen(datagram), 0, (struct sockaddr *) &si_other, sizeof(si_other));

		free(datagram);

	}
	else {

		getIP();

	}

}

void
sendConfigToUDP(unsigned int type, char * value) {

	char * datagram = NULL;

	if (isConnected() == 1) {

		datagram = (char *)malloc(sizeof(char) * UDP_DATAGRAM_SIZE);
		assert(datagram);

		sprintf(datagram, "%d %s ", type, value);

		sendto(udpsocket, datagram, strlen(datagram), 0, (struct sockaddr *) &si_other, sizeof(si_other));

		free(datagram);

	}
	else {

		getIP();

	}

}

/***********************************************************************************/

void
initNetwork() {

	udp_log_file = fopen("telemetry_position_udp.log", "wt");

	udp_log_line("initNetwork");

	initGetIP();

}

void
closeNetwork() {

	if (remote_server_ip == NULL) {
		closeGetIP();
	}
	else {
		closeSendUDP();
	}

	udp_log_line("closeNetwork");

	fclose(udp_log_file);

}


/***********************************************************************************/
