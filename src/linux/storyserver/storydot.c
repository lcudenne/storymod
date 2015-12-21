
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
/* fprintf */
#include <stdio.h>
/* free */
#include <stdlib.h>

/* ------------------------------------------------------------------------- */

#include "storydot.h"

#include "utils.h"

/* ------------------------------------------------------------------------- */



void
_STORY_storyToDot(_STORY_Parameters_t * parameters, _STORY_Story_t * story) {

  FILE * dot;
  char * filename = NULL;
  char * transitionstr = NULL;

  unsigned int i = 0;
  unsigned int j = 0;

  assert(parameters);
  assert(story);

  filename = _UT_strCpy(filename, parameters->story_dot);
  filename = _UT_strCat(filename, ".dot");

  dot = fopen(filename, "w");
  free(filename);

  if (dot != NULL) {

    fprintf(dot, "digraph %s {\n", story->name);

    for (i = 0; i < story->states->size; i++) {
      fprintf(dot, " state%d [shape=box label=\"%s\"];\n",
              story->states->tab[i]->id,
              story->states->tab[i]->name);
    }

    for (i = 0; i < story->states->size; i++) {
      for (j = 0; j < story->states->tab[i]->transitions->size; j++) {
        transitionstr = _STORY_toStringTransition(story->states->tab[i]->transitions->tab[j]);
        fprintf(dot, " state%d -> state%d [label=\"%s\"];\n",
                story->states->tab[i]->id,
                story->states->tab[i]->transitions->tab[j]->nextstate->id,
                transitionstr);
        free(transitionstr);
      }
    }

    fprintf(dot, "}\n");

    fclose(dot);

  }

}


/* ------------------------------------------------------------------------- */
