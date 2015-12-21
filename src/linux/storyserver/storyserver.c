
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
/* assert */
#include <assert.h>
/* strlen */
#include <string.h>
/* fprintf */
#include <stdio.h>

/* ------------------------------------------------------------------------- */

#include "storystruct.h"
#include "storylibxml2.h"
#include "storyautomata.h"
#include "storyposixload.h"
#include "storyhtml.h"
#include "storydot.h"

#include "udpstream.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */

#define DATAGRAM_TYPE_POSITION 0
#define DATAGRAM_TYPE_CARGO 1
#define DATAGRAM_TYPE_LBLINKER 2
#define DATAGRAM_TYPE_RBLINKER 3
#define DATAGRAM_TYPE_TRAILER_CONNECTED 4

/* ------------------------------------------------------------------------- */

void
_STORY_updateTelemetry(_STORY_Telemetry_t * telemetry,
                       char * datagram, unsigned int size) {

  char * token = NULL;
  unsigned int index = 0;
  unsigned int rsize = size;

  assert(telemetry);
  assert(datagram);

  _UT_getNextToken(datagram, rsize, &token);

  switch (atoi(token)) {
  case DATAGRAM_TYPE_POSITION:
    index = strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->x = atof(token);
    index += strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->y = atof(token);
    index += strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->z = atof(token);
    free(token);
    token = NULL;
    break;
  case DATAGRAM_TYPE_CARGO:
    index = strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    if (telemetry->cargo_id != NULL) {
      free(telemetry->cargo_id);
      telemetry->cargo_id = NULL;
    }
    telemetry->cargo_id = token;
    index += strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    if (telemetry->cargo != NULL) {
      free(telemetry->cargo);
      telemetry->cargo = NULL;
    }
    telemetry->cargo = token;
    break;
  case DATAGRAM_TYPE_LBLINKER:
    index = strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->lblinker = atoi(token);
    free(token);
    token = NULL;
    break;
  case DATAGRAM_TYPE_RBLINKER:
    index = strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->rblinker = atoi(token);
    free(token);
    token = NULL;
    break;
  case DATAGRAM_TYPE_TRAILER_CONNECTED:
    index = strlen(token) + 1;
    rsize = rsize - (strlen(token) + 1);
    free(token);
    token = NULL;
    _UT_getNextToken(&(datagram[index]), rsize, &token);
    telemetry->trailer_connected = atoi(token);
    free(token);
    token = NULL;
    break;
  default:
    fprintf(stdout, "Unknown datagram type (%s)\n", token);
    free(token);
    token = NULL;
  }


}
                       



/* ------------------------------------------------------------------------- */


static void
storymod(_STORY_Parameters_t * parameters) {

  _STORY_Context_t * context = NULL;

  char * datagram = NULL;
  char * datagramptr = NULL;

  context = _STORY_newContext();
  context->parameters = parameters;

  if (context->parameters->story_dir != NULL) {
    _STORY_loadStoryListFromDir(context->stories, context->parameters->story_dir);
  }

  _STORY_writeHTMLToDisk(context->parameters, context->stories, context->stories,
                         NULL, NULL, context->telemetry);

  datagram = malloc(sizeof(char) * DATAGRAM_SIZE);
  assert(datagram);
  datagramptr = datagram;

  if (context->parameters->local_trace == NULL) {
    _UDP_registerClient(context->parameters->server_ip, context->parameters->server_port,
                        context->parameters->client_ip, CLIENT_PORT);
    _UDP_bindSocket(8888);
  } else {
    _STORY_openDrivingTrace(context->parameters->local_trace);
  }
  if (context->parameters->dump_trace != NULL) {
    _STORY_openDumpDrivingTrace(context->parameters->dump_trace);
  }

  while (datagram) {

    if (context->parameters->local_trace == NULL) {  
      _UDP_getNextDatagram(datagram, DATAGRAM_SIZE);
    } else {
      _STORY_getNextDrivingTrace(&datagram, DATAGRAM_SIZE);
    }

    if (datagram) {
      
      if (context->parameters->dump_trace != NULL) {
        _STORY_dumpDrivingTrace(datagram);
      }
      
      _STORY_updateTelemetry(context->telemetry, datagram, DATAGRAM_SIZE);
      
      _STORY_storyAutomata(context);
      
    }

  }

  if (context->parameters->local_trace == NULL) {  
    _UDP_closeSocket();
  } else {
    _STORY_closeDrivingTrace();
  }
  if (context->parameters->dump_trace != NULL) {
    _STORY_closeDumpDrivingTrace();
  }

  free(datagramptr);

  _STORY_freeContext(&context);

}



/* ------------------------------------------------------------------------- */

static void
help(int argc, char ** argv)
{

  fprintf(stdout, "Usage: %s [Options]\nOptions:\n", argv[0]);
  fprintf(stdout, "%s", "--ip\t\t<IP> <IP>\tIP of this server (how this machine can be reached by the simulator) and IP of the machine running the simulator.\n");
  fprintf(stdout, "%s", "--port\t\t<PORT>\t\tUDP port of this server (this machine port the simulator has to contact).\n");
  fprintf(stdout, "%s", "--story\t\t<PATH>\t\tPath to the directory containing story files. The program will recursively explore sub-directories.\n");
  fprintf(stdout, "%s", "--trace\t\t<FILE>\t\tPath to a driving trace file. The story server will read this file as input.\n");
  fprintf(stdout, "%s", "--dump\t\t<FILE>\t\tPath to a driving trace file. The story server will write this file as output.\n");
  fprintf(stdout, "%s", "--html\t\t<FILE>\t\tPath to a HTML file. The story server will periodically write this file as output.\n");
  fprintf(stdout, "%s", "--html-refresh\t<DELAY>\t\tSet the delay in seconds to refresh the HTML file.\n");
  fprintf(stdout, "%s", "--story-dot\t<FILE>\t\tPath to a story file. The server will write the corresponding DOT file.\n");
  fprintf(stdout, "Examples:\n%s --ip 192.168.1.13 192.168.1.37 --port 8888 --story ../stories/\n%s --story-dot ../stories/parking/parking.xml\n", argv[0], argv[0]);
  
}

/* ------------------------------------------------------------------------- */


int
main(int argc, char ** argv)
{
  _STORY_Parameters_t * parameters = NULL;
  _STORY_Story_t * story = NULL;
  unsigned int i = 0;

  parameters = _STORY_newParameters();

  i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "--ip") == 0) {
      i++;
      parameters->server_ip = argv[i];
      i++;
      parameters->client_ip = argv[i];
    } else if (strcmp(argv[i], "--port") == 0) {
      i++;
      parameters->server_port = atoi(argv[i]);
    } else if ((strcmp(argv[i], "--story") == 0) || (strcmp(argv[i], "-s") == 0)) {
      i++;
      parameters->story_dir = argv[i];
    } else if (strcmp(argv[i], "--trace") == 0) {
      i++;
      parameters->local_trace = argv[i];
    } else if ((strcmp(argv[i], "--dump") == 0) || (strcmp(argv[i], "-d") == 0)) {
      i++;
      parameters->dump_trace = argv[i];
    } else if (strcmp(argv[i], "--html") == 0) {
      i++;
      parameters->html_file = argv[i];
    } else if ((strcmp(argv[i], "--html-refresh") == 0) || (strcmp(argv[i], "-r") == 0)) {
      i++;
      parameters->html_refresh = atoi(argv[i]);
    } else if (strcmp(argv[i], "--story-dot") == 0) {
      i++;
      parameters->story_dot = argv[i];
    } else if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
      help(argc, argv);
    }
    i++;
  }

  if (parameters->story_dir != NULL) {
    storymod(parameters);
  }
  if (parameters->story_dot != NULL) {
    story = _STORY_loadStoryFromFile(parameters->story_dot, "", 0);
    _STORY_storyToDot(parameters, story);
    _STORY_freeStory(&story);
  }

  _STORY_freeParameters(&parameters);

  _LIBXML2_xmlTerminate();

  return 0;
}


/* ------------------------------------------------------------------------- */
