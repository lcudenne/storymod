
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


/* NULL */
#include <stdlib.h>
/* fprintf */
#include <stdio.h>
/* readdir */
#include <dirent.h>
/* strstr */
#include <string.h>
/* nanosleep */
#include <time.h>

#include "storyposixload.h"
#include "storylibxml2.h"


/* ------------------------------------------------------------------------- */

void
_STORY_loadStoryListFromDir(_STORY_Context_t * context, char * dirname) {

  _STORY_StoryList_t * stories = NULL;
  _STORY_Story_t * story = NULL;
  
  unsigned int i = 0;

  DIR * d = NULL;
  struct dirent * dir;
  char * fileext = ".xml";
  size_t fileextsize = strlen(fileext);
  size_t dnamesize = 0;

  char * fullpath = NULL;

  _UT_ASSERT(context);
  _UT_ASSERT(context->stories);
  _UT_ASSERT(context->parameters);
  _UT_ASSERT(dirname);
  
  stories = context->stories;
  
  d = opendir(dirname);
  if (d != NULL) {
    while ((dir = readdir(d)) != NULL) {
      fullpath = _UT_strCpy(fullpath, dirname);
      fullpath = _UT_strCat(fullpath, "/");
      fullpath = _UT_strCat(fullpath, dir->d_name);
      if (dir->d_type == DT_DIR) {
        if ((strcmp(dir->d_name, ".") != 0) &&
            (strcmp(dir->d_name, "..") != 0)) {
          _STORY_loadStoryListFromDir(context, fullpath);
        }
      } else if (dir->d_type == DT_REG) {
        dnamesize = strlen(dir->d_name);
        if (fileextsize <= dnamesize) {
          if (strncmp(dir->d_name + dnamesize - fileextsize, fileext, fileextsize) == 0) {
            story = _STORY_loadStoryFromFile(context, fullpath, dirname, i);
            if (story != NULL) {
              _STORY_addStoryToStoryList(story, stories);
              i++;
            }
          }
        }
      }
      free(fullpath);
      fullpath = NULL;
    }
    closedir(d);
  }

}




/* ------------------------------------------------------------------------- */
/* This is for debugging purpose */

FILE * _STORY_trace = NULL;


void
_STORY_openDrivingTrace(char * filename) {

  _UT_ASSERT(filename);

  if (_STORY_trace == NULL) {
    _STORY_trace = fopen(filename, "r");
    _UT_ASSERT(_STORY_trace);
  }

}

void
_STORY_closeDrivingTrace() {
  if (_STORY_trace != NULL) {
    fclose(_STORY_trace);
    _STORY_trace = NULL;
  }
}

void
_STORY_getNextDrivingTrace(char ** buffer, unsigned int size) {

  struct timespec ts;

  _UT_ASSERT(*buffer);

  ts.tv_sec = 0;
  ts.tv_nsec = 10000000;

  nanosleep(&ts, NULL);

  if (_STORY_trace != NULL) {
    *buffer = fgets(*buffer, size, _STORY_trace);
  }

}



/* ------------------------------------------------------------------------- */
/* This is for debugging purpose */

FILE * _STORY_dump = NULL;

void
_STORY_openDumpDrivingTrace(char * filename) {

  _UT_ASSERT(filename);

  if (_STORY_dump == NULL) {
    _STORY_dump = fopen(filename, "w");
    _UT_ASSERT(_STORY_dump);
  }

}

void
_STORY_dumpDrivingTrace(char * datagram) {

  if (strchr(datagram, '\n') != NULL) {
    fprintf(_STORY_dump, "%s", datagram);
  } else {
    fprintf(_STORY_dump, "%s\n", datagram);
  }

}


void
_STORY_closeDumpDrivingTrace() {
  if (_STORY_dump != NULL) {
    fclose(_STORY_dump);
    _STORY_dump = NULL;
  }
}







/* ------------------------------------------------------------------------- */

char *
_STORY_loadFileToChar(char * filename) {

  FILE * fp = NULL;
  long lSize = 0;
  char * buffer = NULL;

  _UT_ASSERT(filename);
  
  fp = fopen (filename, "rb");

  if (fp != NULL) {
    fseek(fp ,0L , SEEK_END);
    lSize = ftell(fp) ;
    rewind(fp);

    buffer = calloc(1, lSize+1);
    _UT_ASSERT(buffer);

    fread(buffer ,lSize,1 ,fp);
    fclose(fp);
  }

  return buffer;
}

/* ------------------------------------------------------------------------- */
