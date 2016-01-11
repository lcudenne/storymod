
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
/* strcmp */
#include <string.h>
/* fabs */
#include <math.h>


#include "storystruct.h"


/* ----------------------------------------------------------------------------------- */




_STORY_Context_t *
_STORY_newContext() {

  _STORY_Context_t * context = NULL;

  context = malloc(sizeof(_STORY_Context_t ));
  _UT_ASSERT(context);

  context->parameters = _STORY_newParameters();
  context->telemetry = _STORY_newTelemetry();
  context->stories = _STORY_newStoryList(1);
  context->state = NULL;
  context->prevstate = NULL;
  context->story = NULL;
  context->positions = NULL;
  context->time = 0;
  context->htmlupdatetime = 0;
  context->storystarttime = 0;
  context->statestarttime = 0;
  context->progstarttime = 0;
  
  return context;

}

void
_STORY_freeContext(_STORY_Context_t ** context) {

  _UT_ASSERT(*context);

  if ((*context)->parameters) {
    _STORY_freeParameters(&((*context)->parameters));
  }
  if ((*context)->telemetry) {
    _STORY_freeTelemetry(&((*context)->telemetry));
  }
  if ((*context)->stories) {
    _STORY_freeStoryList(&((*context)->stories));
  }
  if ((*context)->positions) {
    _STORY_freePositionList(&((*context)->positions));
  }

  free(*context);
  *context = NULL;

}

/* ----------------------------------------------------------------------------------- */

_STORY_Parameters_t *
_STORY_newParameters() {

  _STORY_Parameters_t * parameters = NULL;

  parameters = malloc(sizeof(_STORY_Parameters_t ));
  _UT_ASSERT(parameters);

  parameters->server_ip = "localhost";
  parameters->server_port = SERVER_PORT;
  parameters->client_ip = "localhost";
  parameters->story_dir = NULL;
  parameters->positions_database = "positions_database.xml";
  parameters->local_trace = NULL;
  parameters->dump_trace = NULL;
  parameters->html_file = NULL;
  parameters->html_refresh = 2;
  parameters->css_filename = "style.css";
  parameters->css_file = NULL;
  parameters->story_dot = NULL;

  return parameters;

}

void
_STORY_freeParameters(_STORY_Parameters_t ** parameters) {

  _UT_ASSERT(*parameters);

  if ((*parameters)->css_file) {
    free((*parameters)->css_file);
  }
  
  free(*parameters);
  *parameters = NULL;

}



/* ----------------------------------------------------------------------------------- */

_STORY_Telemetry_t *
_STORY_newTelemetry() {

  _STORY_Telemetry_t * telemetry = NULL;

  telemetry = malloc(sizeof(_STORY_Telemetry_t ));
  _UT_ASSERT(telemetry);

  telemetry->client_version_maj = 0;
  telemetry->client_version_min = 0;
  
  telemetry->x = 0.0;
  telemetry->y = 0.0;
  telemetry->z = 0.0;
  
  telemetry->speed = 0.0;
  telemetry->speed_min = 0.0;
  telemetry->speed_max = 0.0;
  
  telemetry->cargo_id = NULL;
  telemetry->cargo = NULL;

  telemetry->lblinker = 0;
  telemetry->rblinker = 0;

  telemetry->trailer_connected = 0;

  telemetry->engine_enabled = 0;

  telemetry->parking_brake = 0;

  telemetry->storytimer = 0;
  telemetry->statetimer = 0;
  telemetry->progtimer = 0;
  
  return telemetry;

}

void
_STORY_freeTelemetry(_STORY_Telemetry_t ** telemetry) {

  _UT_ASSERT(*telemetry);

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

  _UT_ASSERT(telemetry);

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

  _UT_ASSERT(telemetry);

  tostring = _STORY_toStringTelemetry(telemetry);

  fprintf(stdout, "%s                 \r", tostring);

  free(tostring);
}


/* ----------------------------------------------------------------------------------- */

_STORY_Position_t *
_STORY_newPosition() {

  _STORY_Position_t * position = NULL;

  position = malloc(sizeof(_STORY_Position_t ));
  _UT_ASSERT(position);

  position->name = NULL;
  position->x = 0.0;
  position->y = 0.0;
  position->z = 0.0;
  position->boxx = 0.0;
  position->boxy = 0.0;
  position->boxz = 0.0;
  position->distance = 0.0;

  return position;

}


void
_STORY_freePosition(_STORY_Position_t ** position) {

  _UT_ASSERT(*position);

  if ((*position)->name) {
    free((*position)->name);
    (*position)->name = NULL;
  }

  free(*position);
  *position = NULL;

}



_STORY_PositionList_t *
_STORY_newPositionList(unsigned int capacity) {

  _STORY_PositionList_t * positionlist = NULL;

  _UT_ASSERT(capacity > 0);

  positionlist = malloc(sizeof(_STORY_PositionList_t));
  _UT_ASSERT(positionlist);

  positionlist->size = 0;
  positionlist->capacity = capacity;

  positionlist->tab = NULL;
  positionlist->tab = malloc(sizeof(_STORY_Position_t *) * capacity);
  _UT_ASSERT(positionlist->tab);

  return positionlist;

}


void
_STORY_freePositionList(_STORY_PositionList_t ** positionlist) {

  unsigned int i = 0;

  _UT_ASSERT(*positionlist);

  if (((*positionlist)->size > 0) && ((*positionlist)->tab != NULL)) {
    for (i = 0; i < (*positionlist)->size; i++) {
      if (((*positionlist)->tab[i]) != NULL) {
	_STORY_freePosition(&((*positionlist)->tab[i]));
      }
    }
  }

  free((*positionlist)->tab);
  free(*positionlist);

  *positionlist = NULL;

}

void
_STORY_freePositionStructList(_STORY_PositionList_t ** positionlist) {

  _UT_ASSERT(*positionlist);

  free((*positionlist)->tab);
  free(*positionlist);

  *positionlist = NULL;

}


unsigned int
_STORY_addPositionToPositionList(_STORY_Position_t * position,
                           _STORY_PositionList_t * positionlist) {

  unsigned int slot = positionlist->size;

  _UT_ASSERT(position);
  _UT_ASSERT(positionlist);

  if (positionlist->size == positionlist->capacity) {
    positionlist->capacity += positionlist->capacity;
    positionlist->tab = realloc(positionlist->tab,
                                sizeof(_STORY_Position_t *) * positionlist->capacity);
  }

  positionlist->tab[slot] = position;

  positionlist->size++;

  return slot;
}



_STORY_Position_t *
_STORY_getPositionFromName(_STORY_PositionList_t * positionlist, char * name) {

  _STORY_Position_t * position = NULL;
  unsigned int i = 0;

  _UT_ASSERT(name);

  _UT_ASSERT(positionlist);

  while ((i < positionlist->size) && (position == NULL)) {
    if ((positionlist->tab[i] != NULL) && (strcmp(positionlist->tab[i]->name, name) == 0)) {
      position = positionlist->tab[i];
    } else {
      i++;
    }
  }

  return position;
}

_STORY_Position_t *
_STORY_getPositionFromCoordinates(_STORY_PositionList_t * positionlist,
                                  float x, float y, float z) {

  _STORY_Position_t * position = NULL;
  unsigned int i = 0;

  _UT_ASSERT(positionlist);

  while ((i < positionlist->size) && (position == NULL)) {
    if ((positionlist->tab[i] != NULL) &&
        (fabs(x - positionlist->tab[i]->x) <= _STORY_POSITION_BOXX_DEFAULT) &&
        (fabs(y - positionlist->tab[i]->y) <= _STORY_POSITION_BOXY_DEFAULT) &&
        (fabs(z - positionlist->tab[i]->z) <= _STORY_POSITION_BOXZ_DEFAULT)) {
      position = positionlist->tab[i];
    } else {
      i++;
    }
  }

  return position;
}


/* ----------------------------------------------------------------------------------- */

_STORY_Action_t *
_STORY_newAction(unsigned int type) {

  _STORY_Action_t * action = NULL;

  action = malloc(sizeof(_STORY_Action_t ));
  _UT_ASSERT(action);

  action->type = type;
  
  return action;

}


void
_STORY_freeAction(_STORY_Action_t ** action) {

  _UT_ASSERT(*action);

  free(*action);
  *action = NULL;

}



_STORY_ActionList_t *
_STORY_newActionList(unsigned int capacity) {

  _STORY_ActionList_t * actionlist = NULL;

  _UT_ASSERT(capacity > 0);

  actionlist = malloc(sizeof(_STORY_ActionList_t));
  _UT_ASSERT(actionlist);

  actionlist->size = 0;
  actionlist->capacity = capacity;

  actionlist->tab = NULL;
  actionlist->tab = malloc(sizeof(_STORY_Action_t *) * capacity);
  _UT_ASSERT(actionlist->tab);

  return actionlist;

}


void
_STORY_freeActionList(_STORY_ActionList_t ** actionlist) {

  unsigned int i = 0;

  _UT_ASSERT(*actionlist);

  if (((*actionlist)->size > 0) && ((*actionlist)->tab != NULL)) {
    for (i = 0; i < (*actionlist)->size; i++) {
      if (((*actionlist)->tab[i]) != NULL) {
	_STORY_freeAction(&((*actionlist)->tab[i]));
      }
    }
  }

  free((*actionlist)->tab);
  free(*actionlist);

  *actionlist = NULL;

}

void
_STORY_freeActionStructList(_STORY_ActionList_t ** actionlist) {

  _UT_ASSERT(*actionlist);

  free((*actionlist)->tab);
  free(*actionlist);

  *actionlist = NULL;

}


unsigned int
_STORY_addActionToActionList(_STORY_Action_t * action,
                           _STORY_ActionList_t * actionlist) {

  unsigned int slot = actionlist->size;

  _UT_ASSERT(action);
  _UT_ASSERT(actionlist);

  if (actionlist->size == actionlist->capacity) {
    actionlist->capacity += actionlist->capacity;
    actionlist->tab = realloc(actionlist->tab,
                                sizeof(_STORY_Action_t *) * actionlist->capacity);
  }

  actionlist->tab[slot] = action;

  actionlist->size++;

  return slot;
}



/* ----------------------------------------------------------------------------------- */

void
_STORY_positionToCondition(_STORY_Position_t * position,
                           _STORY_Condition_t * condition) {
  _UT_ASSERT(position);
  _UT_ASSERT(condition);

  if (position->x != 0) {
    condition->x = position->x;
  }
  if (position->y != 0) {
    condition->y = position->y;
  }
  if (position->z != 0) {
    condition->z = position->z;
  }
  if (position->boxx != 0) {
    condition->boxx = position->boxx;
  }
  if (position->boxy != 0) {
    condition->boxy = position->boxy;
  }
  if (position->boxz != 0) {
    condition->boxz = position->boxz;
  }
  if (position->distance != 0) {
    condition->distance = position->distance;
  }

}


/* ----------------------------------------------------------------------------------- */

_STORY_Condition_t *
_STORY_newCondition(unsigned int type) {

  _STORY_Condition_t * condition = NULL;

  condition = malloc(sizeof(_STORY_Condition_t ));
  _UT_ASSERT(condition);

  condition->type = type;
  condition->name = NULL;
  condition->x = 0.0;
  condition->y = 0.0;
  condition->z = 0.0;
  condition->boxx = _STORY_POSITION_BOXX_DEFAULT;
  condition->boxy = _STORY_POSITION_BOXY_DEFAULT;
  condition->boxz = _STORY_POSITION_BOXZ_DEFAULT;
  condition->speed = 0.0;
  condition->timer = 0;
  condition->distance = 0.0;
  condition->property_type = 0;
  condition->property_value = NULL;
  condition->visited = 0;
  condition->statelist = NULL;
  condition->statelistid = NULL;
  
  return condition;

}


void
_STORY_freeCondition(_STORY_Condition_t ** condition) {

  _UT_ASSERT(*condition);

  if ((*condition)->name != NULL) {
    free((*condition)->name);
  }
  if ((*condition)->property_value != NULL) {
    free((*condition)->property_value);
  }
  if ((*condition)->statelist != NULL) {
    _STORY_freeStateStructList(&((*condition)->statelist));
  }
  if ((*condition)->statelistid != NULL) {
    _UT_freeUnsignedIntList(&((*condition)->statelistid));
  }

  free(*condition);
  *condition = NULL;

}


char *
_STORY_toStringCondition(_STORY_Condition_t * condition) {

  char * tostring = NULL;

  _UT_ASSERT(condition);

  switch(condition->type) {
  case _STORY_CONDITION_TYPE_POSITION_BOX_IN:
  case _STORY_CONDITION_TYPE_POSITION_BOX_OUT:
    if (condition->type == _STORY_CONDITION_TYPE_POSITION_BOX_IN) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_POSITION_BOX_IN_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_POSITION_BOX_OUT_STR);
    }
    tostring = _UT_strCat(tostring, "(");
    if (condition->name != NULL) {
      tostring = _UT_strCat(tostring, condition->name);
    } else {
      tostring = _UT_strCat(tostring, "{");
      tostring = _UT_strFloat1Cat(tostring, condition->x);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->y);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->z);
      tostring = _UT_strCat(tostring, "}");
    }
    tostring = _UT_strCat(tostring, ", {");
    tostring = _UT_strFloat1Cat(tostring, condition->boxx);
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strFloat1Cat(tostring, condition->boxy);
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strFloat1Cat(tostring, condition->boxz);
    tostring = _UT_strCat(tostring, "})");
    break;
  case _STORY_CONDITION_TYPE_STORY_TIMER_INF:
  case _STORY_CONDITION_TYPE_STORY_TIMER_SUP:
  case _STORY_CONDITION_TYPE_STATE_TIMER_INF:
  case _STORY_CONDITION_TYPE_STATE_TIMER_SUP:
  case _STORY_CONDITION_TYPE_PROG_TIMER_INF:
  case _STORY_CONDITION_TYPE_PROG_TIMER_SUP:
    switch (condition->type) {
    case _STORY_CONDITION_TYPE_STORY_TIMER_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_STORY_TIMER_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_STORY_TIMER_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_STORY_TIMER_SUP_STR);
      break;
    case _STORY_CONDITION_TYPE_STATE_TIMER_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_STATE_TIMER_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_STATE_TIMER_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_STATE_TIMER_SUP_STR);
      break;
    case _STORY_CONDITION_TYPE_PROG_TIMER_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_PROG_TIMER_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_PROG_TIMER_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_PROG_TIMER_SUP_STR);
      break;
    }
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strUnsignedIntCat(tostring, condition->timer);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_DISTANCE_INF:
  case _STORY_CONDITION_TYPE_DISTANCE_SUP:
    if (condition->type == _STORY_CONDITION_TYPE_DISTANCE_INF) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_DISTANCE_INF_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_DISTANCE_SUP_STR);
    }
    tostring = _UT_strCat(tostring, "(");
    if (condition->name != NULL) {
      tostring = _UT_strCat(tostring, condition->name);
    } else {
      tostring = _UT_strCat(tostring, "{");
      tostring = _UT_strFloat1Cat(tostring, condition->x);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->y);
      tostring = _UT_strCat(tostring, ", ");
      tostring = _UT_strFloat1Cat(tostring, condition->z);
      tostring = _UT_strCat(tostring, "}");
    }
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strFloat1Cat(tostring, condition->distance);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_PROPERTY_EQUAL:
  case _STORY_CONDITION_TYPE_PROPERTY_DIFF:
    if (condition->type == _STORY_CONDITION_TYPE_PROPERTY_EQUAL) {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_PROPERTY_EQUAL_STR);
    } else {
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_PROPERTY_DIFF_STR);
    }
    tostring = _UT_strCat(tostring, "(");
    switch (condition->property_type) {
    case _STORY_TELEMETRY_PROPERTY_CARGO:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_CARGO_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_LBLINKER:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_LBLINKER_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_RBLINKER:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_RBLINKER_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_ENGINE_ENABLED:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_ENGINE_ENABLED_STR);
      break;
    case _STORY_TELEMETRY_PROPERTY_PARKING_BRAKE:
      tostring = _UT_strCat(tostring, _STORY_TELEMETRY_PROPERTY_PARKING_BRAKE_STR);
      break;
    default:
      break;
    }
    tostring = _UT_strCat(tostring, ", ");
    tostring = _UT_strCat(tostring, condition->property_value);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_VISITED_INF:
  case _STORY_CONDITION_TYPE_VISITED_SUP:
  case _STORY_CONDITION_TYPE_VISITED_EQUAL:
  case _STORY_CONDITION_TYPE_VISITED_DIFF:
    switch (condition->type) {
    case _STORY_CONDITION_TYPE_VISITED_INF:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_VISITED_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_VISITED_SUP:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_VISITED_SUP_STR);
      break;
    case _STORY_CONDITION_TYPE_VISITED_EQUAL:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_VISITED_EQUAL_STR);
      break;
    case _STORY_CONDITION_TYPE_VISITED_DIFF:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_VISITED_DIFF_STR);
      break;
    }
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strUnsignedIntCat(tostring, condition->visited);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_VISITED_LIST:
  case _STORY_CONDITION_TYPE_NOT_VISITED_LIST:
    switch (condition->type) {
    case _STORY_CONDITION_TYPE_VISITED_LIST:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_VISITED_LIST_STR);
      break;
    case _STORY_CONDITION_TYPE_NOT_VISITED_LIST:
          tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_NOT_VISITED_LIST_STR);
      break;
    }
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strUnsignedIntListCat(tostring, condition->statelistid);
    tostring = _UT_strCat(tostring, ")");
    break;
  case _STORY_CONDITION_TYPE_AT_LEAST_VISITED_LIST:
    tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_AT_LEAST_VISITED_LIST_STR);
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strUnsignedIntCat(tostring, condition->visited);
    tostring = _UT_strCat(tostring, ", {");
    tostring = _UT_strUnsignedIntListCat(tostring, condition->statelistid);
    tostring = _UT_strCat(tostring, "})");
    break;
  case _STORY_CONDITION_TYPE_SPEED_INF:
  case _STORY_CONDITION_TYPE_SPEED_SUP:
  case _STORY_CONDITION_TYPE_SPEED_MAX_INF:
  case _STORY_CONDITION_TYPE_SPEED_MAX_SUP:
  case _STORY_CONDITION_TYPE_SPEED_MIN_INF:
  case _STORY_CONDITION_TYPE_SPEED_MIN_SUP:
    switch (condition->type) {
    case _STORY_CONDITION_TYPE_SPEED_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_SPEED_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_SUP_STR);
      break;
    case _STORY_CONDITION_TYPE_SPEED_MAX_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_MAX_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_SPEED_MAX_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_MAX_SUP_STR);
      break;
    case _STORY_CONDITION_TYPE_SPEED_MIN_INF:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_MIN_INF_STR);
      break;
    case _STORY_CONDITION_TYPE_SPEED_MIN_SUP:
      tostring = _UT_strCpy(tostring, _STORY_CONDITION_TYPE_SPEED_MIN_SUP_STR);
      break;
    }
    tostring = _UT_strCat(tostring, "(");
    tostring = _UT_strFloat1Cat(tostring, condition->speed);
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

  _UT_ASSERT(capacity > 0);

  conditionlist = malloc(sizeof(_STORY_ConditionList_t));
  _UT_ASSERT(conditionlist);

  conditionlist->size = 0;
  conditionlist->capacity = capacity;

  conditionlist->tab = NULL;
  conditionlist->tab = malloc(sizeof(_STORY_Condition_t *) * capacity);
  _UT_ASSERT(conditionlist->tab);

  return conditionlist;

}


void
_STORY_freeConditionList(_STORY_ConditionList_t ** conditionlist) {

  unsigned int i = 0;

  _UT_ASSERT(*conditionlist);

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

  _UT_ASSERT(condition);
  _UT_ASSERT(conditionlist);

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
  _UT_ASSERT(transition);

  transition->conditions = _STORY_newConditionList(1);
  transition->actions = _STORY_newActionList(1);
  transition->nextstate = NULL;
  transition->nextstateid = nextstateid;
  transition->type = _STORY_TRANSITION_TYPE_AND;

  return transition;

}


void
_STORY_freeTransition(_STORY_Transition_t ** transition) {

  _UT_ASSERT(*transition);

  _STORY_freeConditionList(&((*transition)->conditions));
  _STORY_freeActionList(&((*transition)->actions));

  free(*transition);
  *transition = NULL;

}


char *
_STORY_toStringTransition(_STORY_Transition_t * transition) {

  char * tostring = NULL;
  char * conditionstr = NULL;

  unsigned int i = 0;

  _UT_ASSERT(transition);

  tostring = _UT_strCpy(tostring, " ");

  for (i = 0; i < transition->conditions->size; i++) {
    conditionstr = _STORY_toStringCondition(transition->conditions->tab[i]);
    tostring = _UT_strCat(tostring, conditionstr);
    free(conditionstr);
    if (i < (transition->conditions->size - 1)) {
      if (transition->type == _STORY_TRANSITION_TYPE_AND) {
        tostring = _UT_strCat(tostring, " AND\n");
      } else if (transition->type == _STORY_TRANSITION_TYPE_OR) {
        tostring = _UT_strCat(tostring, " OR\n");
      }
    }
  }

  return tostring;

}


_STORY_TransitionList_t *
_STORY_newTransitionList(unsigned int capacity) {

  _STORY_TransitionList_t * transitionlist = NULL;

  _UT_ASSERT(capacity > 0);

  transitionlist = malloc(sizeof(_STORY_TransitionList_t));
  _UT_ASSERT(transitionlist);

  transitionlist->size = 0;
  transitionlist->capacity = capacity;

  transitionlist->tab = NULL;
  transitionlist->tab = malloc(sizeof(_STORY_Transition_t *) * capacity);
  _UT_ASSERT(transitionlist->tab);

  return transitionlist;

}


void
_STORY_freeTransitionList(_STORY_TransitionList_t ** transitionlist) {

  unsigned int i = 0;

  _UT_ASSERT(*transitionlist);

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

void
_STORY_freeTransitionStructList(_STORY_TransitionList_t ** transitionlist) {

  _UT_ASSERT(*transitionlist);

  free((*transitionlist)->tab);
  free(*transitionlist);

  *transitionlist = NULL;

}

unsigned int
_STORY_addTransitionToTransitionList(_STORY_Transition_t * transition,
                                     _STORY_TransitionList_t * transitionlist) {

  unsigned int slot = transitionlist->size;

  _UT_ASSERT(transition);
  _UT_ASSERT(transitionlist);

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
  _UT_ASSERT(state);

  state->id = id;
  state->name = NULL;
  state->description = NULL;
  state->image = NULL;
  state->transitions = _STORY_newTransitionList(1);
  state->visited = 0;

  return state;

}


void
_STORY_freeState(_STORY_State_t ** state) {

  _UT_ASSERT(*state);

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

  _UT_ASSERT(capacity > 0);

  statelist = malloc(sizeof(_STORY_StateList_t));
  _UT_ASSERT(statelist);

  statelist->size = 0;
  statelist->capacity = capacity;

  statelist->tab = NULL;
  statelist->tab = malloc(sizeof(_STORY_State_t *) * capacity);
  _UT_ASSERT(statelist->tab);

  return statelist;

}


void
_STORY_freeStateList(_STORY_StateList_t ** statelist) {

  unsigned int i = 0;

  _UT_ASSERT(*statelist);

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

  _UT_ASSERT(*statelist);

  free((*statelist)->tab);
  free(*statelist);

  *statelist = NULL;

}


unsigned int
_STORY_addStateToStateList(_STORY_State_t * state,
                           _STORY_StateList_t * statelist) {

  unsigned int slot = statelist->size;

  _UT_ASSERT(state);
  _UT_ASSERT(statelist);

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

  _UT_ASSERT(statelist);

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
  _UT_ASSERT(story);

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
  story->css_filename = NULL;
  story->css_file = NULL;
  story->startstate = NULL;
  story->states = _STORY_newStateList(1);
  story->distance = 0.0;

  return story;

}


void
_STORY_freeStory(_STORY_Story_t ** story) {

  _UT_ASSERT(*story);

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
  if ((*story)->css_filename) {
    free((*story)->css_filename);
    (*story)->css_filename = NULL;
  }
  if ((*story)->css_file) {
    free((*story)->css_file);
    (*story)->css_file = NULL;
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

  _UT_ASSERT(capacity > 0);

  storylist = malloc(sizeof(_STORY_StoryList_t));
  _UT_ASSERT(storylist);

  storylist->size = 0;
  storylist->capacity = capacity;

  storylist->tab = NULL;
  storylist->tab = malloc(sizeof(_STORY_Story_t *) * capacity);
  _UT_ASSERT(storylist->tab);

  return storylist;

}


void
_STORY_freeStoryList(_STORY_StoryList_t ** storylist) {

  unsigned int i = 0;

  _UT_ASSERT(*storylist);

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

  _UT_ASSERT(*storylist);

  free((*storylist)->tab);
  free(*storylist);

  *storylist = NULL;

}

unsigned int
_STORY_addStoryToStoryList(_STORY_Story_t * story,
                           _STORY_StoryList_t * storylist) {

  unsigned int slot = storylist->size;

  _UT_ASSERT(story);
  _UT_ASSERT(storylist);

  if (storylist->size == storylist->capacity) {
    storylist->capacity += storylist->capacity;
    storylist->tab = realloc(storylist->tab, sizeof(_STORY_Story_t *) * storylist->capacity);
  }

  storylist->tab[slot] = story;

  storylist->size++;

  return slot;
}




/* ----------------------------------------------------------------------------------- */


