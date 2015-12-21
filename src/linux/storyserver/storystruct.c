
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
/* assert */
#include <assert.h>
/* fprintf */
#include <stdio.h>


#include "storystruct.h"
#include "utils.h"

/* ----------------------------------------------------------------------------------- */

_STORY_Context_t *
_STORY_newContext() {

  _STORY_Context_t * context = NULL;

  context = malloc(sizeof(_STORY_Context_t ));
  assert(context);

  context->parameters = NULL;
  context->telemetry = _STORY_newTelemetry();
  context->stories = _STORY_newStoryList(1);
  context->state = NULL;
  context->prevstate = NULL;
  context->story = NULL;
  context->lasthtmlupdate = 0;
  context->laststatechange = 0;

  return context;

}

void
_STORY_freeContext(_STORY_Context_t ** context) {

  assert(*context);

  if ((*context)->telemetry) {
    _STORY_freeTelemetry(&((*context)->telemetry));
  }
  if ((*context)->stories) {
    _STORY_freeStoryList(&((*context)->stories));
  }

  free(*context);
  *context = NULL;

}

/* ----------------------------------------------------------------------------------- */

_STORY_Parameters_t *
_STORY_newParameters() {

  _STORY_Parameters_t * parameters = NULL;

  parameters = malloc(sizeof(_STORY_Parameters_t ));
  assert(parameters);

  parameters->story_dir = NULL;
  parameters->local_trace = NULL;
  parameters->dump_trace = NULL;
  parameters->html_file = NULL;
  parameters->html_refresh = 2;
  parameters->story_dot = NULL;

  return parameters;

}

void
_STORY_freeParameters(_STORY_Parameters_t ** parameters) {

  assert(*parameters);

  free(*parameters);
  *parameters = NULL;

}



/* ----------------------------------------------------------------------------------- */

_STORY_Telemetry_t *
_STORY_newTelemetry() {

  _STORY_Telemetry_t * telemetry = NULL;

  telemetry = malloc(sizeof(_STORY_Telemetry_t ));
  assert(telemetry);

  telemetry->x = 0.0;
  telemetry->y = 0.0;
  telemetry->z = 0.0;

  telemetry->cargo_id = NULL;
  telemetry->cargo = NULL;

  telemetry->lblinker = 0;
  telemetry->rblinker = 0;

  telemetry->trailer_connected = 0;

  telemetry->stateruntime = 0;

  return telemetry;

}

void
_STORY_freeTelemetry(_STORY_Telemetry_t ** telemetry) {

  assert(*telemetry);

  if ((*telemetry)->cargo_id) {
    free((*telemetry)->cargo_id);
  }
  if ((*telemetry)->cargo) {
    free((*telemetry)->cargo);
  }

  free(*telemetry);
  *telemetry = NULL;

}

char *
_STORY_toStringTelemetry(_STORY_Telemetry_t * telemetry) {

  char * tostring = NULL;

  assert(telemetry);

  tostring = _UT_strCpy(tostring, "{ \"telemetry\" : { \"position\" : { \"x\" : \"");
  tostring = _UT_strFloat1Cat(tostring, telemetry->x);
  tostring = _UT_strCat(tostring, "\", \"y\" : \"");
  tostring = _UT_strFloat1Cat(tostring, telemetry->y);
  tostring = _UT_strCat(tostring, "\", \"z\" : \"");
  tostring = _UT_strFloat1Cat(tostring, telemetry->z);
  tostring = _UT_strCat(tostring, "\" } } }");

  return tostring;
}

void
_STORY_displayTelemetry(_STORY_Telemetry_t * telemetry) {

  char * tostring = NULL;

  assert(telemetry);

  tostring = _STORY_toStringTelemetry(telemetry);

  fprintf(stdout, "%s                 \r", tostring);

  free(tostring);
}


/* ----------------------------------------------------------------------------------- */

_STORY_Condition_t *
_STORY_newCondition(unsigned int type) {

  _STORY_Condition_t * condition = NULL;

  condition = malloc(sizeof(_STORY_Condition_t ));
  assert(condition);

  condition->type = type;
  condition->name = NULL;
  condition->x = 0.0;
  condition->y = 0.0;
  condition->z = 0.0;
  condition->boxx = 10.0;
  condition->boxy = 1;
  condition->boxz = 100.0;
  condition->timer = 0;
  condition->distance = 0.0;
  condition->property_type = 0;
  condition->property_value = NULL;

  return condition;

}


void
_STORY_freeCondition(_STORY_Condition_t ** condition) {

  assert(*condition);

  if ((*condition)->name != NULL) {
    free((*condition)->name);
  }
  if ((*condition)->property_value != NULL) {
    free((*condition)->property_value);
  }

  free(*condition);
  *condition = NULL;

}


char *
_STORY_toStringCondition(_STORY_Condition_t * condition) {

  char * tostring = NULL;

  assert(condition);

  switch(condition->type) {
  case _STORY_CONDITION_TYPE_POSITION_BOX_IN:
  case _STORY_CONDITION_TYPE_POSITION_BOX_OUT:
    if (condition->type == _STORY_CONDITION_TYPE_POSITION_BOX_IN) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_POSITION_BOX_IN_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_POSITION_BOX_OUT_STR);
    }
    tostring = _UT_strCat(tostring, "({");
    if (condition->name != NULL) {
      tostring = _UT_strCat(tostring, condition->name);
    } else {
      tostring = _UT_strFloat1Cat(tostring, condition->x);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->y);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->z);
    }
    tostring = _UT_strCat(tostring, "}, {");
    tostring = _UT_strFloat1Cat(tostring, condition->boxx);
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strFloat1Cat(tostring, condition->boxy);
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strFloat1Cat(tostring, condition->boxz);
    tostring = _UT_strCat(tostring, "})");
    break;
  case _STORY_CONDITION_TYPE_TIMER_INF:
  case _STORY_CONDITION_TYPE_TIMER_SUP:
    if (condition->type == _STORY_CONDITION_TYPE_TIMER_INF) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_TIMER_INF_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_TIMER_SUP_STR);
    }
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strUnsignedIntCat(tostring, condition->timer);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_DISTANCE_INF:
    if (condition->type == _STORY_CONDITION_TYPE_DISTANCE_INF) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_DISTANCE_INF_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_DISTANCE_SUP_STR);
    }
    tostring = _UT_strCat(tostring, "({");
    if (condition->name != NULL) {
      tostring = _UT_strCat(tostring, condition->name);
    } else {
      tostring = _UT_strFloat1Cat(tostring, condition->x);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->y);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->z);
    }
    tostring = _UT_strCat(tostring, "}, ");
    tostring = _UT_strFloat1Cat(tostring, condition->distance);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_PROPERTY_EQUAL:
  case _STORY_CONDITION_TYPE_PROPERTY_DIFF:
    tostring = _UT_strCpy(tostring, "(");
    switch (condition->property_type) {
    case _STORY_TELEMETRY_PROPERTY_CARGO:
      tostring = _UT_strCpy(tostring, _STORY_TELEMETRY_PROPERTY_CARGO_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_LBLINKER:
      tostring = _UT_strCpy(tostring, _STORY_TELEMETRY_PROPERTY_LBLINKER_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_RBLINKER:
      tostring = _UT_strCpy(tostring, _STORY_TELEMETRY_PROPERTY_RBLINKER_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED:
      tostring = _UT_strCpy(tostring, _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED_STR);
      break;
    default:
      break;
    }
    if (condition->type == _STORY_CONDITION_TYPE_PROPERTY_EQUAL) {
      tostring = _UT_strCat(tostring, " == ");
    } else {
      tostring = _UT_strCat(tostring, " != ");
    }
    tostring = _UT_strCat(tostring, condition->property_value);
    tostring = _UT_strCat(tostring, ")");
    break;
  default:
    break;
  }

  return tostring;

}




_STORY_ConditionList_t *
_STORY_newConditionList(unsigned int capacity) {

  _STORY_ConditionList_t * conditionlist = NULL;

  assert(capacity > 0);

  conditionlist = malloc(sizeof(_STORY_ConditionList_t));
  assert(conditionlist);

  conditionlist->size = 0;
  conditionlist->capacity = capacity;

  conditionlist->tab = NULL;
  conditionlist->tab = malloc(sizeof(_STORY_Condition_t *) * capacity);
  assert(conditionlist->tab);

  return conditionlist;

}


void
_STORY_freeConditionList(_STORY_ConditionList_t ** conditionlist) {

  unsigned int i = 0;

  assert(*conditionlist);

  if (((*conditionlist)->size > 0) && ((*conditionlist)->tab != NULL)) {
    for (i = 0; i < (*conditionlist)->size; i++) {
      if (((*conditionlist)->tab[i]) != NULL) {
	_STORY_freeCondition(&((*conditionlist)->tab[i]));
      }
    }
  }

  free((*conditionlist)->tab);
  free(*conditionlist);

  *conditionlist = NULL;

}

unsigned int
_STORY_addConditionToConditionList(_STORY_Condition_t * condition,
                                   _STORY_ConditionList_t * conditionlist) {

  unsigned int slot = conditionlist->size;

  assert(condition);
  assert(conditionlist);

  if (conditionlist->size == conditionlist->capacity) {
    conditionlist->capacity += conditionlist->capacity;
    conditionlist->tab = realloc(conditionlist->tab, sizeof(_STORY_Condition_t *) * conditionlist->capacity);
  }

  conditionlist->tab[slot] = condition;

  conditionlist->size++;

  return slot;
}


/* ----------------------------------------------------------------------------------- */



/* ----------------------------------------------------------------------------------- */

_STORY_Transition_t *
_STORY_newTransition(unsigned int nextstateid) {

  _STORY_Transition_t * transition = NULL;

  transition = malloc(sizeof(_STORY_Transition_t ));
  assert(transition);

  transition->conditions = _STORY_newConditionList(1);
  transition->nextstate = NULL;
  transition->nextstateid = nextstateid;
  transition->type = _STORY_TRANSITION_TYPE_AND;

  return transition;

}


void
_STORY_freeTransition(_STORY_Transition_t ** transition) {

  assert(*transition);

  _STORY_freeConditionList(&((*transition)->conditions));

  free(*transition);
  *transition = NULL;

}


char *
_STORY_toStringTransition(_STORY_Transition_t * transition) {

  char * tostring = NULL;
  char * conditionstr = NULL;

  unsigned int i = 0;

  assert(transition);

  tostring = _UT_strCpy(tostring, " ");

  for (i = 0; i < transition->conditions->size; i++) {
    conditionstr = _STORY_toStringCondition(transition->conditions->tab[i]);
    tostring = _UT_strCat(tostring, conditionstr);
    free(conditionstr);
    if (i < (transition->conditions->size - 1)) {
      if (transition->type == _STORY_TRANSITION_TYPE_AND) {
        tostring = _UT_strCat(tostring, " &&\n");
      } else if (transition->type == _STORY_TRANSITION_TYPE_OR) {
        tostring = _UT_strCat(tostring, " ||\n");
      }
    }
  }

  return tostring;

}


_STORY_TransitionList_t *
_STORY_newTransitionList(unsigned int capacity) {

  _STORY_TransitionList_t * transitionlist = NULL;

  assert(capacity > 0);

  transitionlist = malloc(sizeof(_STORY_TransitionList_t));
  assert(transitionlist);

  transitionlist->size = 0;
  transitionlist->capacity = capacity;

  transitionlist->tab = NULL;
  transitionlist->tab = malloc(sizeof(_STORY_Transition_t *) * capacity);
  assert(transitionlist->tab);

  return transitionlist;

}


void
_STORY_freeTransitionList(_STORY_TransitionList_t ** transitionlist) {

  unsigned int i = 0;

  assert(*transitionlist);

  if (((*transitionlist)->size > 0) && ((*transitionlist)->tab != NULL)) {
    for (i = 0; i < (*transitionlist)->size; i++) {
      if (((*transitionlist)->tab[i]) != NULL) {
	_STORY_freeTransition(&((*transitionlist)->tab[i]));
      }
    }
  }

  free((*transitionlist)->tab);
  free(*transitionlist);

  *transitionlist = NULL;

}

unsigned int
_STORY_addTransitionToTransitionList(_STORY_Transition_t * transition,
                                     _STORY_TransitionList_t * transitionlist) {

  unsigned int slot = transitionlist->size;

  assert(transition);
  assert(transitionlist);

  if (transitionlist->size == transitionlist->capacity) {
    transitionlist->capacity += transitionlist->capacity;
    transitionlist->tab = realloc(transitionlist->tab, sizeof(_STORY_Transition_t *) * transitionlist->capacity);
  }

  transitionlist->tab[slot] = transition;

  transitionlist->size++;

  return slot;
}




/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */

_STORY_State_t *
_STORY_newState(unsigned int id) {

  _STORY_State_t * state = NULL;

  state = malloc(sizeof(_STORY_State_t ));
  assert(state);

  state->id = id;
  state->name = NULL;
  state->description = NULL;
  state->image = NULL;
  state->transitions = _STORY_newTransitionList(1);

  return state;

}


void
_STORY_freeState(_STORY_State_t ** state) {

  assert(*state);

  if ((*state)->name) {
    free((*state)->name);
    (*state)->name = NULL;
  }
  if ((*state)->description) {
    free((*state)->description);
    (*state)->description = NULL;
  }
  if ((*state)->image) {
    free((*state)->image);
    (*state)->image = NULL;
  }

  _STORY_freeTransitionList(&((*state)->transitions));

  free(*state);
  *state = NULL;

}



_STORY_StateList_t *
_STORY_newStateList(unsigned int capacity) {

  _STORY_StateList_t * statelist = NULL;

  assert(capacity > 0);

  statelist = malloc(sizeof(_STORY_StateList_t));
  assert(statelist);

  statelist->size = 0;
  statelist->capacity = capacity;

  statelist->tab = NULL;
  statelist->tab = malloc(sizeof(_STORY_State_t *) * capacity);
  assert(statelist->tab);

  return statelist;

}


void
_STORY_freeStateList(_STORY_StateList_t ** statelist) {

  unsigned int i = 0;

  assert(*statelist);

  if (((*statelist)->size > 0) && ((*statelist)->tab != NULL)) {
    for (i = 0; i < (*statelist)->size; i++) {
      if (((*statelist)->tab[i]) != NULL) {
	_STORY_freeState(&((*statelist)->tab[i]));
      }
    }
  }

  free((*statelist)->tab);
  free(*statelist);

  *statelist = NULL;

}

void
_STORY_freeStateStructList(_STORY_StateList_t ** statelist) {

  assert(*statelist);

  free((*statelist)->tab);
  free(*statelist);

  *statelist = NULL;

}


unsigned int
_STORY_addStateToStateList(_STORY_State_t * state,
                           _STORY_StateList_t * statelist) {

  unsigned int slot = statelist->size;

  assert(state);
  assert(statelist);

  if (statelist->size == statelist->capacity) {
    statelist->capacity += statelist->capacity;
    statelist->tab = realloc(statelist->tab, sizeof(_STORY_State_t *) * statelist->capacity);
  }

  statelist->tab[slot] = state;

  statelist->size++;

  return slot;
}



_STORY_State_t *
_STORY_getStateFromId(_STORY_StateList_t * statelist, unsigned int id) {

  _STORY_State_t * state = NULL;
  unsigned int i = 0;

  assert(statelist);

  while ((i < statelist->size) && (state == NULL)) {
    if ((statelist->tab[i] != NULL) && (statelist->tab[i]->id == id)) {
      state = statelist->tab[i];
    } else {
      i++;
    }
  }

  return state;
}




/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */

_STORY_Story_t *
_STORY_newStory(unsigned int id) {

  _STORY_Story_t * story = NULL;

  story = malloc(sizeof(_STORY_Story_t ));
  assert(story);

  story->id = id;
  story->name = NULL;
  story->author = NULL;
  story->meeting = NULL;
  story->category = NULL;
  story->time = NULL;
  story->difficulty = NULL;
  story->required = NULL;
  story->description = NULL;
  story->image = NULL;
  story->startstate = NULL;
  story->states = _STORY_newStateList(1);
  story->distance = 0.0;

  return story;

}


void
_STORY_freeStory(_STORY_Story_t ** story) {

  assert(*story);

  if ((*story)->name) {
    free((*story)->name);
    (*story)->name = NULL;
  }
  if ((*story)->author) {
    free((*story)->author);
    (*story)->author = NULL;
  }
  if ((*story)->meeting) {
    free((*story)->meeting);
    (*story)->meeting = NULL;
  }
  if ((*story)->category) {
    free((*story)->category);
    (*story)->category = NULL;
  }
  if ((*story)->time) {
    free((*story)->time);
    (*story)->time = NULL;
  }
  if ((*story)->difficulty) {
    free((*story)->difficulty);
    (*story)->difficulty = NULL;
  }
  if ((*story)->required) {
    free((*story)->required);
    (*story)->required = NULL;
  }
  if ((*story)->description) {
    free((*story)->description);
    (*story)->description = NULL;
  }
  if ((*story)->image) {
    free((*story)->image);
    (*story)->image = NULL;
  }

  _STORY_freeStateList(&((*story)->states));

  free(*story);
  *story = NULL;

}



_STORY_StoryList_t *
_STORY_newStoryList(unsigned int capacity) {

  _STORY_StoryList_t * storylist = NULL;

  assert(capacity > 0);

  storylist = malloc(sizeof(_STORY_StoryList_t));
  assert(storylist);

  storylist->size = 0;
  storylist->capacity = capacity;

  storylist->tab = NULL;
  storylist->tab = malloc(sizeof(_STORY_Story_t *) * capacity);
  assert(storylist->tab);

  return storylist;

}


void
_STORY_freeStoryList(_STORY_StoryList_t ** storylist) {

  unsigned int i = 0;

  assert(*storylist);

  if (((*storylist)->size > 0) && ((*storylist)->tab != NULL)) {
    for (i = 0; i < (*storylist)->size; i++) {
      if (((*storylist)->tab[i]) != NULL) {
	_STORY_freeStory(&((*storylist)->tab[i]));
      }
    }
  }

  free((*storylist)->tab);
  free(*storylist);

  *storylist = NULL;

}

void
_STORY_freeStoryStructList(_STORY_StoryList_t ** storylist) {

  assert(*storylist);

  free((*storylist)->tab);
  free(*storylist);

  *storylist = NULL;

}

unsigned int
_STORY_addStoryToStoryList(_STORY_Story_t * story,
                           _STORY_StoryList_t * storylist) {

  unsigned int slot = storylist->size;

  assert(story);
  assert(storylist);

  if (storylist->size == storylist->capacity) {
    storylist->capacity += storylist->capacity;
    storylist->tab = realloc(storylist->tab, sizeof(_STORY_Story_t *) * storylist->capacity);
  }

  storylist->tab[slot] = story;

  storylist->size++;

  return slot;
}




/* ----------------------------------------------------------------------------------- */


