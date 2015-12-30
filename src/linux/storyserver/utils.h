
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

#ifndef UTILS
#define UTILS

/* ----------------------------------------------------------------------------------- */


typedef struct _UT_UnsignedIntList {

  /* size, given as the number of unsigned int */
  unsigned int size;

  /* capacity, given as the number of unsigned int */
  unsigned int capacity;

  unsigned int * tab;

} _UT_UnsignedIntList_t;

/* ----------------------------------------------------------------------------------- */


_UT_UnsignedIntList_t *
_UT_newUnsignedIntList(unsigned int capacity);

void
_UT_freeUnsignedIntList(_UT_UnsignedIntList_t ** unsignedintlist);

unsigned int
_UT_addUnsignedIntToUnsignedIntList(unsigned int unsignedint,
                                    _UT_UnsignedIntList_t * unsignedintlist);

/* ----------------------------------------------------------------------------------- */

char *
_UT_strCpy(char* dest, const char* src);

/* ----------------------------------------------------------------------------------- */

char *
_UT_strnCat(char * dest, const char * src, unsigned int n);

char *
_UT_strCat(char * dest, const char * src);


char *
_UT_strIntCat(char * dest, int i);

char *
_UT_strUnsignedIntCat(char * dest, unsigned int u);

char *
_UT_strFloat1Cat(char * dest, float f);

/* ----------------------------------------------------------------------------------- */

char *
_UT_strftime();

unsigned int
_UT_getTimeSecond();


/* ----------------------------------------------------------------------------------- */


char
_UT_getNextToken(void * haystack, size_t haystacklen, char ** token);

_UT_UnsignedIntList_t *
_UT_stringToUnsignedIntList(char * pstr);


/* ----------------------------------------------------------------------------------- */


char *
_UT_getPathFromFilePath(char * filename);

/* ----------------------------------------------------------------------------------- */


#endif
