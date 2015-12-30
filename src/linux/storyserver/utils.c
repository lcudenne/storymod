
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


/* assert */
#include <assert.h>
/* memcpy */
#include <string.h>
/* malloc */
#include <stdlib.h>
/* fprintf */
#include <stdio.h>
/* gettimeofday */
#include <time.h>
#include <sys/time.h>


/* ----------------------------------------------------------------------------------- */


#include "utils.h"

/**
 * \brief Maximum length, as a string representation
 */
#define _UT_INTEGER_MAX_LEN 10
#define _UT_FLOAT1_MAX_LEN 12







/* ----------------------------------------------------------------------------------- */

_UT_UnsignedIntList_t *
_UT_newUnsignedIntList(unsigned int capacity) {

  _UT_UnsignedIntList_t * unsignedintlist = NULL;

  assert(capacity > 0);

  unsignedintlist = malloc(sizeof(_UT_UnsignedIntList_t));
  assert(unsignedintlist);

  unsignedintlist->size = 0;
  unsignedintlist->capacity = capacity;

  unsignedintlist->tab = NULL;
  unsignedintlist->tab = malloc(sizeof(unsigned int) * capacity);
  assert(unsignedintlist->tab);

  return unsignedintlist;

}


void
_UT_freeUnsignedIntList(_UT_UnsignedIntList_t ** unsignedintlist) {

  assert(*unsignedintlist);

  free((*unsignedintlist)->tab);
  free(*unsignedintlist);

  *unsignedintlist = NULL;

}

unsigned int
_UT_addUnsignedIntToUnsignedIntList(unsigned int unsignedint,
                                    _UT_UnsignedIntList_t * unsignedintlist) {

  unsigned int slot = unsignedintlist->size;

  assert(unsignedintlist);

  if (unsignedintlist->size == unsignedintlist->capacity) {
    unsignedintlist->capacity += unsignedintlist->capacity;
    unsignedintlist->tab = realloc(unsignedintlist->tab, sizeof(unsigned int) * unsignedintlist->capacity);
  }

  unsignedintlist->tab[slot] = unsignedint;

  unsignedintlist->size++;

  return slot;
}

/* ----------------------------------------------------------------------------------- */

char *
_UT_strCpy(char* dest, const char* src) {

  if (src) {

    if((dest) && (dest != src)) {
      dest = realloc(dest, (strlen(src)+1)*sizeof(char));
    } else {
      dest = malloc((strlen(src)+1) * sizeof(char));
    }

    assert(dest);
    strcpy(dest, src);

  }
  
  return dest;
}

/* ----------------------------------------------------------------------------------- */

char *
_UT_strnCat(char * dest, const char * src, unsigned int n) {

  unsigned int srclen = n;

  assert(src);

  if (srclen == 0) {
    srclen = strlen(src);
  }

  if (dest) {
    
    dest = realloc(dest, (strlen(dest) + srclen + 1) * sizeof(char));
    assert(dest);

  } else {

    dest = malloc((srclen + 1) * sizeof(char));
    assert(dest);
    dest[0] = '\0';

  }

  dest = strcat(dest, src);

  return dest;

}


char *
_UT_strCat(char * dest, const char * src) {
  return _UT_strnCat(dest, src, 0);
}



char *
_UT_strIntCat(char * dest, int i) {

  char * intstr = NULL;

  intstr = malloc((_UT_INTEGER_MAX_LEN + 1) * sizeof(char));
  assert(intstr);

  sprintf(intstr, "%d", i);

  dest = _UT_strCat(dest, intstr);

  free(intstr);

  return dest;
}

char *
_UT_strUnsignedIntCat(char * dest, unsigned int u) {

  char * uintstr = NULL;

  uintstr = malloc((_UT_INTEGER_MAX_LEN + 1) * sizeof(char));
  assert(uintstr);

  sprintf(uintstr, "%d", u);

  dest = _UT_strCat(dest, uintstr);

  free(uintstr);

  return dest;
}

char *
_UT_strFloat1Cat(char * dest, float f) {

  char * fstr = NULL;

  fstr = malloc((_UT_FLOAT1_MAX_LEN + 1) * sizeof(char));
  assert(fstr);

  sprintf(fstr, "%.1f", f);

  dest = _UT_strCat(dest, fstr);

  free(fstr);

  return dest;
}



/* ----------------------------------------------------------------------------------- */


#define _UT_MAX_STRFTIME_STRING_LENGTH 64

char *
_UT_strftime() {

  char * date = NULL;
  size_t max = sizeof(char) * _UT_MAX_STRFTIME_STRING_LENGTH;

  time_t myt;
  struct tm * mytm = NULL;

  date = malloc(max);

  myt = time(NULL);

  mytm = localtime(&myt);
  assert(mytm);

  strftime(date, max, "%Y-%m-%dT%H:%M:%S+%Z", mytm);

  return date;

}


/* ----------------------------------------------------------------------------------- */

unsigned int
_UT_getTimeSecond() {

  unsigned int res = 0;

  time_t myt;
  struct tm * mytm = NULL;

  myt = time(NULL);

  mytm = localtime(&myt);
  assert(mytm);

  res = (mytm->tm_mday * 24 * 60 * 60) + (mytm->tm_hour * 60 * 60) +
    (mytm->tm_min * 60) + mytm->tm_sec;

  return res;

}


/* ----------------------------------------------------------------------------------- */

#define _UT_CHAR_SPACE ' '
#define _UT_CHAR_NEW_LINE '\n'


char
_UT_getNextToken(void * haystack, size_t haystacklen, char ** token) {

  char delimiter = _UT_CHAR_SPACE;

  size_t i = 0;
  unsigned int found = 0;

  while ((found != 1) && (i < haystacklen)) {

    delimiter = *((char *) haystack + i);

    if ((delimiter == _UT_CHAR_SPACE) ||
        (delimiter == _UT_CHAR_NEW_LINE)) {
      found = 1;
    } else {
      i += sizeof(char);
    }

  }

  if ((found == 1) && (i > 0)) {
    *token = malloc(i + sizeof(char));
    assert(*token);
    /* /!\ hard copy of the token */
    memcpy(*token, (char *) haystack, i);
    (*token)[i] = '\0';
  }

  return delimiter;

}


_UT_UnsignedIntList_t *
_UT_stringToUnsignedIntList(char * pstr) {

  _UT_UnsignedIntList_t * res;
  char * str = NULL;
  char * tok = NULL;

  assert(pstr);

  str = malloc(sizeof(char) * (strlen(pstr) + 1));
  assert(str);
  strcpy(str, pstr);

  res = _UT_newUnsignedIntList(1);

  tok = strtok(str, " .,;");
  
  while (tok) {
    _UT_addUnsignedIntToUnsignedIntList(atoi(tok), res);
    tok = strtok(NULL, " .,;");
  }

  free(str);

  return res;
}



/* ----------------------------------------------------------------------------------- */

char *
_UT_getPathFromFilePath(char * filename) {

  char * path = NULL;
  char * rchr = NULL;
  size_t i = 0;

  rchr = strrchr(filename, '/');
  
  if (rchr != NULL) {
    i = (rchr - filename);
    path = malloc(sizeof(char) * (i + 1));
    assert(path);
    memcpy(path, filename, i);
    path[i] = '\0';
  }

  return path;
}

/* ----------------------------------------------------------------------------------- */
