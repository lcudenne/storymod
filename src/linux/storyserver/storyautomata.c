
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
/* fabs */
#include <math.h>
/* fprintf */
#include <stdio.h>
/* strcmp */
#include <string.h>
/* atoi */
#include <stdlib.h>


#include "storyautomata.h"

#include "utils.h"
#include "storyhtml.h"

/* ----------------------------------------------------------------------------------- */

/* time to wait at final state before leaving story (seconds) */
#define TIME_FINAL_STATE 10

/* ----------------------------------------------------------------------------------- */

static unsigned int
_STORY_checkCondition(_STORY_State_t * state,
                      _STORY_Condition_t * condition,
                      _STORY_Telemetry_t * telemetry) {

  unsigned int res = 0;
  unsigned int i = 0;
  unsigned int j = 0;
  
  assert(state);
  assert(condition);
  assert(telemetry);

  switch (condition->type) {
  case _STORY_CONDITION_TYPE_POSITION_BOX_IN:
    if ((fabs(telemetry->x - condition->x) <= condition->boxx) &&
        (fabs(telemetry->y - condition->y) <= condition->boxy) &&
        (fabs(telemetry->z - condition->z) <= condition->boxz)) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_POSITION_BOX_OUT:
    if ((fabs(telemetry->x - condition->x) > condition->boxx) ||
        (fabs(telemetry->y - condition->y) > condition->boxy) ||
        (fabs(telemetry->z - condition->z) > condition->boxz)) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_DISTANCE_INF:
    if (_STORY_getDistance(telemetry, condition) < condition->distance) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_DISTANCE_SUP:
    if (_STORY_getDistance(telemetry, condition) > condition->distance) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_PROPERTY_EQUAL:
  case _STORY_CONDITION_TYPE_PROPERTY_DIFF:
    switch(condition->property_type) {
    case _STORY_TELEMETRY_PROPERTY_CARGO:
      if (((telemetry->cargo_id != NULL) &&
           (strcmp(telemetry->cargo_id, condition->property_value) == 0)) ||
          ((telemetry->cargo != NULL) &&
           (strcmp(telemetry->cargo, condition->property_value) == 0))) {
        res = 1;
      }
      break;
    case _STORY_TELEMETRY_PROPERTY_LBLINKER:
      if (telemetry->lblinker == atoi(condition->property_value)) {
        res = 1;
      }
      break;
    case _STORY_TELEMETRY_PROPERTY_RBLINKER:
      if (telemetry->rblinker == atoi(condition->property_value)) {
        res = 1;
      }
      break;
    case _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED:
      if (telemetry->trailer_connected == atoi(condition->property_value)) {
        res = 1;
      }
      break;
    case _STORY_TELEMETRY_PROPERTY_ENGINE_ENABLED:
      if (telemetry->engine_enabled == atoi(condition->property_value)) {
        res = 1;
      }
      break;
    case _STORY_TELEMETRY_PROPERTY_PARKING_BRAKE:
      if (telemetry->parking_brake == atoi(condition->property_value)) {
        res = 1;
      }
      break;
    default:
      fprintf(stdout, "[STORY] Condition property type (%d) not known\n",
              condition->property_type);
    }
    if (condition->type == _STORY_CONDITION_TYPE_PROPERTY_DIFF) {
      if (res == 1) {
        res = 0;
      } else if (res == 0){
        res = 1;
      }
    }
    break;
  case _STORY_CONDITION_TYPE_VISITED_INF:
    if (condition->visited < state->visited) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_VISITED_SUP:
    if (condition->visited > state->visited) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_VISITED_EQUAL:
    if (condition->visited == state->visited) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_VISITED_DIFF:
    if (condition->visited != state->visited) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_INF:
    if (telemetry->speed < condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_SUP:
    if (telemetry->speed > condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_MAX_INF:
    if (telemetry->speed_max < condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_MAX_SUP:
    if (telemetry->speed_max > condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_MIN_INF:
    if (telemetry->speed_min < condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_SPEED_MIN_SUP:
    if (telemetry->speed_min > condition->speed) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_VISITED_LIST:
    i = 0;
    if (condition->statelist != NULL) {
      res = 1;
      while (i < condition->statelist->size) {
        res *= condition->statelist->tab[i]->visited;
        i++;
      }
    }
    break;
  case _STORY_CONDITION_TYPE_NOT_VISITED_LIST:
    i = 0;
    if (condition->statelist != NULL) {
      res = 0;
      while ((res == 0) && (i < condition->statelist->size)) {
        res += condition->statelist->tab[i]->visited;
        i++;
      }
      if (res == 0) {
        res = 1;
      } else {
        res = 0;
      }
    }
    break;
  case _STORY_CONDITION_TYPE_AT_LEAST_VISITED_LIST:
    i = 0;
    if (condition->statelist != NULL) {
      j = 0;
      while ((j < condition->visited) && (i < condition->statelist->size)) {
        if (condition->statelist->tab[i]->visited > 0) {
          j++;
        }
        i++;
      }
      if (j >= condition->visited) {
        res = 1;
      }
    }
    break;
  case _STORY_CONDITION_TYPE_STORY_TIMER_INF:
    if (telemetry->storytimer < condition->timer) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_STORY_TIMER_SUP:
    if (telemetry->storytimer > condition->timer) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_STATE_TIMER_INF:
    if (telemetry->statetimer < condition->timer) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_STATE_TIMER_SUP:
    if (telemetry->statetimer > condition->timer) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_PROG_TIMER_INF:
    if (telemetry->progtimer < condition->timer) {
      res = 1;
    }
    break;
  case _STORY_CONDITION_TYPE_PROG_TIMER_SUP:
    if (telemetry->progtimer > condition->timer) {
      res = 1;
    }
    break;
  default:
    fprintf(stdout, "[STORY] Condition type not known (%d)\n", condition->type);
    break;
  }

  if (res > 0) {
    res = 1;
  }
  
  return res;
}


static unsigned int
_STORY_checkConditionList(_STORY_State_t * state,
                          _STORY_ConditionList_t * conditions,
                          _STORY_Telemetry_t * telemetry,
                          unsigned int type) {
  
  unsigned int res = 1;
  unsigned int i = 0;

  assert(state);
  assert(conditions);
  assert(telemetry);

  switch (type) {

  case _STORY_TRANSITION_TYPE_AND:  
    res = 1;
    for (i = 0; i < conditions->size; i++) {
      res = res * _STORY_checkCondition(state, conditions->tab[i], telemetry);
    }
    break;

  case _STORY_TRANSITION_TYPE_OR:
    res = 0;
    i = 0;
    while ((i < conditions->size) && (res == 0)) {
      res = _STORY_checkCondition(state, conditions->tab[i], telemetry);
      i++;
    }
    break;

  }

  return res;

}


/* ----------------------------------------------------------------------------------- */

static void
_STORY_performActions(_STORY_Context_t * context, _STORY_ActionList_t * actions) {

  unsigned int i = 0;
  
  assert(context);
  assert(context->telemetry);
  assert(actions);

  for (i = 0; i < actions->size; i++) {
    switch (actions->tab[i]->type) {
    case _STORY_ACTION_TYPE_SPEED_RESET:
      context->telemetry->speed_min = 0;
      context->telemetry->speed_max = 0;
      break;
    case _STORY_ACTION_TYPE_PROG_TIMER_RESET:
      context->progstarttime = context->time;
      break;
    default:
      fprintf(stdout, "[STORY] Action type not known (%d)\n", actions->tab[i]->type);
      break;
    }
  }
  
}


/* ----------------------------------------------------------------------------------- */

_STORY_StateList_t *
_STORY_getAllNextState(_STORY_State_t * currentstate,
                       _STORY_Telemetry_t * telemetry) {

  _STORY_StateList_t * nextstatelist = NULL;
  _STORY_Transition_t * transition = NULL;
  unsigned int i = 0;

  assert(currentstate);
  assert(telemetry);

  nextstatelist = _STORY_newStateList(1);

  for (i = 0; i < currentstate->transitions->size; i++) {
    transition = currentstate->transitions->tab[i];
    if (_STORY_checkConditionList(currentstate, transition->conditions, telemetry, transition->type) == 1) {
      _STORY_addStateToStateList(transition->nextstate, nextstatelist);
    }
  }

  return nextstatelist;

}

_STORY_TransitionList_t *
_STORY_getAllNextTransitions(_STORY_State_t * currentstate,
                             _STORY_Telemetry_t * telemetry) {

  _STORY_TransitionList_t * transitionlist = NULL;
  _STORY_Transition_t * transition = NULL;
  unsigned int i = 0;

  assert(currentstate);
  assert(telemetry);

  transitionlist = _STORY_newTransitionList(1);

  for (i = 0; i < currentstate->transitions->size; i++) {
    transition = currentstate->transitions->tab[i];
    if (_STORY_checkConditionList(currentstate, transition->conditions, telemetry, transition->type) == 1) {
      _STORY_addTransitionToTransitionList(transition, transitionlist);
    }
  }

  return transitionlist;

}


_STORY_State_t *
_STORY_getNextState(_STORY_Context_t * context) {

  _STORY_State_t * currentstate = NULL;  
  _STORY_State_t * nextstate = NULL;
  _STORY_Telemetry_t * telemetry = NULL;
  _STORY_TransitionList_t * transitionlist = NULL;

  unsigned int nsid = 0;

  assert(context);
  assert(context->state);
  assert(context->telemetry);

  currentstate = context->state;
  nextstate = context->state;
  telemetry = context->telemetry;
  
  transitionlist = _STORY_getAllNextTransitions(currentstate, telemetry);

  if (transitionlist->size > 0) {
    nsid = _UT_getTimeSecond() % transitionlist->size;
    _STORY_performActions(context, transitionlist->tab[nsid]->actions);
    nextstate = transitionlist->tab[nsid]->nextstate;
    if (nextstate == NULL) {
      fprintf(stdout, "Error: next state is unreachable\n");
      nextstate = currentstate;
    }
    nextstate->visited++;
  }

  _STORY_freeTransitionStructList(&transitionlist);

  return nextstate;

}


/* ----------------------------------------------------------------------------------- */

unsigned int
_STORY_isFinalState(_STORY_State_t * state) {

  unsigned int res = 0;

  assert(state);

  if (state->transitions->size == 0) {
    res = 1;
  }

  return res;
}


/* ----------------------------------------------------------------------------------- */

_STORY_StoryList_t *
_STORY_getActiveStoryList(_STORY_Telemetry_t * telemetry,
                          _STORY_StoryList_t * stories) {

  _STORY_StoryList_t * activestories = NULL;
  _STORY_StateList_t * statelist = NULL;

  unsigned int i = 0;

  assert(telemetry);
  assert(stories);

  activestories = _STORY_newStoryList(1);
  for (i = 0; i < stories->size; i++) {
    statelist = _STORY_getAllNextState(stories->tab[i]->startstate,
                                       telemetry);
    if (statelist->size > 0) {
      _STORY_addStoryToStoryList(stories->tab[i], activestories);
    }
    _STORY_freeStateStructList(&statelist);
  }

  return activestories;
}

/* ----------------------------------------------------------------------------------- */

float
_STORY_getDistance(_STORY_Telemetry_t * telemetry,
                   _STORY_Condition_t * condition) {
  assert(telemetry);
  assert(condition);
  return (fabs(telemetry->x - condition->x) + fabs(telemetry->y - condition->y));
}

/* ----------------------------------------------------------------------------------- */

_STORY_Condition_t *
_STORY_getFirstConditionType(_STORY_ConditionList_t * conditions,
                             unsigned int type) {

  _STORY_Condition_t * condition = NULL;

  unsigned int i = 0;

  assert(conditions);

  while ((i < conditions->size) && (condition == NULL)) {
    if (conditions->tab[i]->type == type) {
      condition = conditions->tab[i];
    } else {
      i++;
    }
  }

  return condition;

}

_STORY_Condition_t *
_STORY_getFirstConditionTypeTransition(_STORY_TransitionList_t * transitions,
                                       unsigned int type) {

  _STORY_Condition_t * condition = NULL;

  unsigned int i = 0;

  assert(transitions);

  while ((i < transitions->size) && (condition == NULL)) {
    condition = _STORY_getFirstConditionType(transitions->tab[i]->conditions, type);
    i++;
  }

  return condition;

}



/* ----------------------------------------------------------------------------------- */

void
_STORY_sortCloseStoryList(_STORY_Telemetry_t * telemetry,
                          _STORY_StoryList_t * stories) {

  _STORY_Story_t * story = NULL;
  _STORY_Condition_t * condition = NULL;

  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int invert = 1;

  assert(telemetry);
  assert(stories);

  for (i = 0; i < stories->size; i++) {
    story = stories->tab[i];
    if (story->startstate) {
      condition = _STORY_getFirstConditionTypeTransition(story->startstate->transitions,
                                                         _STORY_CONDITION_TYPE_POSITION_BOX_IN);
      if (condition == NULL) {
        condition = _STORY_getFirstConditionTypeTransition(story->startstate->transitions,
                                                           _STORY_CONDITION_TYPE_DISTANCE_INF);
      }
      if (condition != NULL) {
        story->distance = _STORY_getDistance(telemetry, condition);
      } else {
        story->distance = 0;
      }
    }
  }

  if (stories->size == 0) {
    i = 0;
  } else {
    i = stories->size - 1;
  }
  while ((invert == 1) && (i > 0)) {
    invert = 0;
    for (j = 0; j < i; j++) {
      if (stories->tab[j]->distance > stories->tab[j+1]->distance) {
        story = stories->tab[j];
        stories->tab[j] = stories->tab[j+1];
        stories->tab[j+1] = story;
        invert = 1;
      }
    }
    i--;
  }

}

/* ----------------------------------------------------------------------------------- */

static void
_STORY_resetStoryAutomata(_STORY_Story_t * story) {

  unsigned int i = 0;
  
  assert(story);
  
  for (i = 0; i < story->states->size; i++) {
    story->states->tab[i]->visited = 0;
  }
  
}

/* ----------------------------------------------------------------------------------- */


void
_STORY_storyAutomata(_STORY_Context_t * context) {

  _STORY_StoryList_t * activestories = NULL;

  assert(context);
  assert(context->parameters);
  assert(context->telemetry);
  assert(context->stories);

  context->time = _UT_getTimeSecond();
  
  if (context->state != NULL) {
    context->state = _STORY_getNextState(context);
    if (context->state != context->prevstate) {
      _STORY_writeHTMLToDisk(context, activestories);
      context->prevstate = context->state;
      context->statestarttime = context->time;
      fprintf(stdout, "%d %s [%d]%s\n", context->time,
              context->story->name, context->state->id, context->state->name);
    }
  } else {
    activestories = _STORY_getActiveStoryList(context->telemetry, context->stories);
    if (activestories->size > 0) {
      context->story = activestories->tab[0];
      _STORY_resetStoryAutomata(context->story);
      context->state = context->story->startstate;
      context->state->visited = 1;
      context->storystarttime = context->time;
      context->statestarttime = context->time;
      context->progstarttime = context->time;
    }
  }
  
  context->telemetry->storytimer = (context->time - context->storystarttime);
  context->telemetry->statetimer = (context->time - context->statestarttime);
  context->telemetry->progtimer = (context->time - context->progstarttime);
  
  if ((context->time - context->htmlupdatetime) > context->parameters->html_refresh) {
    _STORY_sortCloseStoryList(context->telemetry, context->stories);
    _STORY_writeHTMLToDisk(context, activestories);
    context->htmlupdatetime = context->time;
  }
  if (context->telemetry->statetimer > TIME_FINAL_STATE) {
    if ((context->state != NULL) && (_STORY_isFinalState(context->state) == 1)) {
      context->story = NULL;
      context->state = NULL;
    }
  }

  if (activestories != NULL) {
    _STORY_freeStoryStructList(&activestories);
  }

}





/* ----------------------------------------------------------------------------------- */
