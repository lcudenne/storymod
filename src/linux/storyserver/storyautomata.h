
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


#include "storystruct.h"


/* ----------------------------------------------------------------------------------- */

_STORY_StateList_t *
_STORY_getAllNextState(_STORY_State_t * currentstate,
                       _STORY_Telemetry_t * telemetry);

_STORY_TransitionList_t *
_STORY_getAllNextTransitions(_STORY_State_t * currentstate,
                             _STORY_Telemetry_t * telemetry);

_STORY_State_t *
_STORY_getNextState(_STORY_State_t * currentstate,
                    _STORY_Telemetry_t * telemetry);

/* ----------------------------------------------------------------------------------- */

unsigned int
_STORY_isFinalState(_STORY_State_t * state);

/* ----------------------------------------------------------------------------------- */

_STORY_Condition_t *
_STORY_getFirstConditionType(_STORY_ConditionList_t * conditions,
                             unsigned int type);

_STORY_Condition_t *
_STORY_getFirstConditionTypeTransition(_STORY_TransitionList_t * transitions,
                                       unsigned int type);

/* ----------------------------------------------------------------------------------- */


_STORY_StoryList_t *
_STORY_getActiveStoryList(_STORY_Telemetry_t * telemetry,
                          _STORY_StoryList_t * stories);



/* ----------------------------------------------------------------------------------- */

float
_STORY_getDistance(_STORY_Telemetry_t * telemetry,
                   _STORY_Condition_t * condition);

void
_STORY_sortCloseStoryList(_STORY_Telemetry_t * telemetry,
                          _STORY_StoryList_t * stories);

/* ----------------------------------------------------------------------------------- */

void
_STORY_storyAutomata(_STORY_Context_t * context);

/* ----------------------------------------------------------------------------------- */


