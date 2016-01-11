
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


/* fopen */
#include <stdio.h>
/* strcmp */
#include <string.h>
/* free */
#include <stdlib.h>

#include "storyhtml.h"
#include "storyautomata.h"
#include "storyposixload.h"


#define HTML_FILE_NAME "index.html"

/* ------------------------------------------------------------------------- */

static void
_STORY_writeStateToDisk(_STORY_State_t * state, FILE * html) {

  _UT_ASSERT(state);

  fprintf(html, "<div class=\"state\">");  

  fprintf(html, "<div class=\"imagestate\">");
  if (state->image != NULL) {
    fprintf(html, "<img src=\"%s\" />", state->image);
  }
  fprintf(html, "</div>");
  if (state->description != NULL) {
    fprintf(html, "<div class=\"statedesc\"><h class=\"texttype\">Current task (visited %d)</h>: %s</div>", state->visited, state->description);
  }


  fprintf(html, "</div>");

}

/* ------------------------------------------------------------------------- */

static void
_STORY_writeStoryToDisk(_STORY_Story_t * story,
                        char * divclass, FILE * html) {

  _UT_ASSERT(divclass);

  if (story != NULL) {
    fprintf(html, "<div class=\"%s\"><div class=\"imagestory\">", divclass);
    if (story->image != NULL) {
      fprintf(html, "<img src=\"%s\" />", story->image);
    }
    fprintf(html, "</div>");
    if ((story->name != NULL) && (strcmp(story->name, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Name</h>: %s<br/>", story->name);
    }
    if (story->distance != 0) {
      fprintf(html, "<h class=\"texttype\">Distance</h>: %f<br/>", story->distance);
    }
    if ((story->author != NULL) && (strcmp(story->author, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Author</h>: %s<br/>", story->author);
    }
    if ((story->meeting != NULL) && (strcmp(story->meeting, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Meeting</h>: %s<br/>", story->meeting);
    }
    if ((story->category != NULL) && (strcmp(story->category, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Category</h>: %s<br/>", story->category);
    }
    if ((story->required != NULL) && (strcmp(story->required, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Required</h>: %s<br/>", story->required);
    }
    if ((story->time != NULL) && (strcmp(story->time, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Time</h>: %s ", story->time);
    }
    if ((story->difficulty != NULL) && (strcmp(story->difficulty, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Difficulty</h>: %s<br/>", story->difficulty);
    } else {
      if ((story->time != NULL) && (strcmp(story->time, "") != 0)) {
        fprintf(html, "<br/>");
      }
    }
    if ((story->description != NULL) && (strcmp(story->description, "") != 0)) {
      fprintf(html, "<h class=\"texttype\">Description</h>: %s<br/>", story->description);
    }
    fprintf(html, "</div>\n");
  }

}


static void
_STORY_writeStoryListToDisk(_STORY_StoryList_t * stories,
                            char * divclass, FILE * html) {

  unsigned int i = 0;

  _UT_ASSERT(divclass);

  if ((stories != NULL) && (stories->size > 0)) {
    fprintf(html, "<div class=\"%slist\">\n", divclass);
    for (i = 0; i < stories->size; i++) {
      _STORY_writeStoryToDisk(stories->tab[i], divclass, html);
    }
    fprintf(html, "</div>\n");
  }

}


/* ------------------------------------------------------------------------- */

void
_STORY_writeHTMLToDisk(_STORY_Context_t * context,
                       _STORY_StoryList_t * activestories) {

  FILE * html = NULL;

  _STORY_Parameters_t * parameters = NULL;
  _STORY_StoryList_t * stories = NULL;
  _STORY_Telemetry_t * telemetry = NULL;

  _STORY_Position_t * position = NULL;
  
  char * css_file = NULL;
  
  _UT_ASSERT(context);
  _UT_ASSERT(context->parameters);
  _UT_ASSERT(context->stories);
  _UT_ASSERT(context->telemetry);

  parameters = context->parameters;
  stories = context->stories;
  telemetry = context->telemetry;

  if (context->positions != NULL) {
    position = _STORY_getPositionFromCoordinates(context->positions,
                                                 telemetry->x, telemetry->y, telemetry->z);
  }
  
  css_file = parameters->css_file;
  if (context->story != NULL) {
    if (context->story->css_file != NULL) {
      css_file = context->story->css_file;
    }
  }
  
  if (parameters->html_file != NULL) {
    html = fopen(parameters->html_file, "w");
  } else {
    html = fopen(HTML_FILE_NAME, "w");
  }

  if (html != NULL) {

    fprintf(html, "<html>\n<head>\n<title>ETS2/ATS Truck Simulator StoryMod Server v%d.%d</title>\n<meta http-equiv=\"refresh\" content=\"%d; URL=index.html\">\n<style media=\"screen\" type=\"text/css\">\n%s\n</style>\n</head>\n<body>\n", SERVER_VERSION_MAJ, SERVER_VERSION_MIN, parameters->html_refresh, css_file);

    fprintf(html, "<div class=\"storymod\">\n");
    
    fprintf(html, "<div class=\"telemetry\">\n");
    fprintf(html, "<h class=\"texttype\">Server</h>: StoryMod Server v%d.%d (%s:%d) ",
            SERVER_VERSION_MAJ, SERVER_VERSION_MIN, parameters->server_ip, parameters->server_port);
    if ((telemetry->client_version_maj != 0) || (telemetry->client_version_min != 0)) {
      fprintf(html, "<h class=\"texttype\">Client</h>: StoryMod Client v%d.%d (%s:%d)",
              telemetry->client_version_maj, telemetry->client_version_min,
              parameters->client_ip, CLIENT_PORT);
    }
    fprintf(html, "<br/>\n<h class=\"texttype\">StoryTimer</h>: %d <h class=\"texttype\">StateTimer</h>: %d <h class=\"texttype\">ProgTimer</h>: %d",
            telemetry->storytimer, telemetry->statetimer, telemetry->progtimer);

    if (position != NULL) {
      fprintf(html, " <h class=\"texttype\">Position</h>: %s", position->name);
    }

    fprintf(html, "<br/><h class=\"texttype\">Coordinates</h>: <h class=\"texttype\">x</h> %f <h class=\"texttype\">y</h> %f <h class=\"texttype\">z</h> %f <br /><h class=\"texttype\">speed</h> %f <h class=\"texttype\">min</h> %f <h class=\"texttype\">max</h> %f<br/>",
            telemetry->x, telemetry->y, telemetry->z,
            telemetry->speed, telemetry->speed_min, telemetry->speed_max);

    fprintf(html, " <h class=\"texttype\">Engine</h>: %d", telemetry->engine_enabled);
    fprintf(html, " <h class=\"texttype\">PBrake</h>: %d", telemetry->parking_brake);
    fprintf(html, " <h class=\"texttype\">LBlinker</h>: %d", telemetry->lblinker);
    fprintf(html, " <h class=\"texttype\">RBlinker</h>: %d", telemetry->rblinker);
    fprintf(html, " <h class=\"texttype\">Trailer</h>: %d", telemetry->trailer_connected);
    
    if ((telemetry->cargo_id != NULL) && (telemetry->cargo != NULL)) {
      fprintf(html, " <h class=\"texttype\">Cargo</h>: %s (%s)",
              telemetry->cargo, telemetry->cargo_id);
    }

    fprintf(html, "\n</div>\n");
    
    if ((context->story != NULL) && (context->state != NULL)) {
      fprintf(html, "<div class=\"currentstatus\">\n");
      _STORY_writeStoryToDisk(context->story, "currentstory", html);
      _STORY_writeStateToDisk(context->state, html);
      fprintf(html, "</div>\n");
    }
    
    
    if ((context->story == NULL) || ((context->state != NULL) && (_STORY_isFinalState(context->state) == 1))) {
      
      _STORY_writeStoryListToDisk(activestories, "activestory", html);
      
      if ((activestories == NULL) ||
          (activestories != NULL && activestories->size == 0)) {
        _STORY_writeStoryListToDisk(stories, "allstory", html);
      }
      
    }

    fprintf(html, "\n</div>\n");
    
    fprintf(html, "</body></html>\n");
    
    fclose(html);
    
  }

}





/* ------------------------------------------------------------------------- */
