
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
/* assert */
#include <assert.h>
/* strcmp */
#include <string.h>

#include "storyhtml.h"
#include "storyautomata.h"


#define HTML_FILE_NAME "index.html"

/* ------------------------------------------------------------------------- */

static void
_STORY_writeStateToDisk(_STORY_State_t * state, FILE * html) {

  assert(state);

  fprintf(html, "<div class=\"state\">");  

  fprintf(html, "<div class=\"imagestate\">");
  if (state->image != NULL) {
    fprintf(html, "<img src=\"%s\" />", state->image);
  }
  fprintf(html, "</div>");
  if (state->description != NULL) {
    fprintf(html, "<div class=\"statedesc\"><h class=\"texttype\">Current task</h>: %s</div>", state->description);
  }


  fprintf(html, "</div>");

}

/* ------------------------------------------------------------------------- */

static void
_STORY_writeStoryToDisk(_STORY_Story_t * story,
                        char * divclass, FILE * html) {

  assert(divclass);

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

  assert(divclass);

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
_STORY_writeHTMLToDisk(_STORY_Parameters_t * parameters,
                       _STORY_StoryList_t * stories,
                       _STORY_StoryList_t * activestories,
                       _STORY_Story_t * story,
                       _STORY_State_t * state,
                       _STORY_Telemetry_t * telemetry) {

  FILE * html = NULL;

  assert(parameters);
  assert(stories);
  assert(telemetry);

  if (parameters->html_file != NULL) {
    html = fopen(parameters->html_file, "w");
  } else {
    html = fopen(HTML_FILE_NAME, "w");
  }

  if (html != NULL) {

    fprintf(html, "<html>\n<head>\n<title>Story Mod</title>\n<meta http-equiv=\"refresh\" content=\"%d; URL=index.html\">\n<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\n</head>\n<body>\n", parameters->html_refresh);
    
    fprintf(html, "<div class=\"telemetry\">\n<h class=\"texttype\">Timer</h>: %d <h class=\"texttype\">Position</h>: <h class=\"texttype\">x</h> %f <h class=\"texttype\">y</h> %f <h class=\"texttype\">z</h> %f",
            telemetry->stateruntime,
            telemetry->x, telemetry->y, telemetry->z);
    if ((telemetry->cargo_id != NULL) && (telemetry->cargo != NULL)) {
      fprintf(html, " <h class=\"texttype\">Cargo</h>: %s (%s)",
              telemetry->cargo, telemetry->cargo_id);
    }
    fprintf(html, " <h class=\"texttype\">LBlinker</h>: %d", telemetry->lblinker);
    fprintf(html, " <h class=\"texttype\">RBlinker</h>: %d", telemetry->rblinker);
    fprintf(html, " <h class=\"texttype\">Trailer</h>: %d", telemetry->trailer_connected);
    fprintf(html, "\n</div>\n");
    
    if ((story != NULL) && (state != NULL)) {
      fprintf(html, "<div class=\"currentstatus\">\n");
      _STORY_writeStoryToDisk(story, "currentstory", html);
      _STORY_writeStateToDisk(state, html);
      fprintf(html, "</div>\n");
    }
    
    
    if ((story == NULL) || ((state != NULL) && (_STORY_isFinalState(state) == 1))) {
      
      _STORY_writeStoryListToDisk(activestories, "activestory", html);
      
      if ((activestories == NULL) ||
          (activestories != NULL && activestories->size == 0)) {
        _STORY_writeStoryListToDisk(stories, "allstory", html);
      }
      
    }
    
    fprintf(html, "</body></html>\n");
    
    fclose(html);
    
  }

}


/* ------------------------------------------------------------------------- */
