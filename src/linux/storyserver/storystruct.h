
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


#ifndef STORY_STRUCT
#define STORY_STRUCT

/* ----------------------------------------------------------------------------------- */

#define SERVER_VERSION_MAJ 0
#define SERVER_VERSION_MIN 1

/* ----------------------------------------------------------------------------------- */

#define CLIENT_PORT 8888

/* ----------------------------------------------------------------------------------- */

#define DATAGRAM_SIZE 512

/* ------------------------------------------------------------------------- */

#define DATAGRAM_TYPE_POSITION 0
#define DATAGRAM_TYPE_CARGO 1
#define DATAGRAM_TYPE_LBLINKER 2
#define DATAGRAM_TYPE_RBLINKER 3
#define DATAGRAM_TYPE_TRAILER_CONNECTED 4
#define DATAGRAM_TYPE_CLIENT_VERSION 5
#define DATAGRAM_TYPE_ENGINE_ENABLED 6

/* ----------------------------------------------------------------------------------- */


typedef struct _STORY_Context {

  /* application parameters */
  struct _STORY_Parameters * parameters;

  /* simulator telemetry */
  struct _STORY_Telemetry * telemetry;

  /* story list */
  struct _STORY_StoryList * stories;

  /* current state */
  struct _STORY_State * state;

  /* previous state */
  struct _STORY_State * prevstate;

  /* current story */
  struct _STORY_Story * story;

  /* general positions database */
  struct _STORY_PositionList * positions;
  
  /* last html update time */
  unsigned int lasthtmlupdate;

  /* last state update time */
  unsigned int laststatechange;

} _STORY_Context_t;


/* ----------------------------------------------------------------------------------- */


typedef struct _STORY_Parameters {

  /* IP of this server (how this machine can be reached by the simulator) */
  char * server_ip;

  /* UDP port of this server (this machine port the simulator has to contact) */
  unsigned int server_port;

  /* IP of the machine running the simulator */
  char * client_ip;

  /* path to the stories directory */
  char * story_dir;

  /* path to the positions database file */
  char * positions_database;
  
  /* read driving trace from disk (filename or NULL otherwise) */
  char * local_trace;

  /* dump driving trace to disk (filename or NULL otherwise)*/
  char * dump_trace;

  /* html output file */
  char * html_file;

  /* html page refresh time (seconds) */
  unsigned int html_refresh;

  /* css style file */
  char * css_file;
  
  /* story file name to convert in dot */
  char * story_dot;

} _STORY_Parameters_t;


/* ----------------------------------------------------------------------------------- */

#define _STORY_TELEMETRY_PROPERTY_CARGO 0
#define _STORY_TELEMETRY_PROPERTY_CARGO_STR "cargo"
#define _STORY_TELEMETRY_PROPERTY_LBLINKER 1
#define _STORY_TELEMETRY_PROPERTY_LBLINKER_STR "lblinker"
#define _STORY_TELEMETRY_PROPERTY_RBLINKER 2
#define _STORY_TELEMETRY_PROPERTY_RBLINKER_STR "rblinker"
#define _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED 3
#define _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED_STR "trailer_connected"

typedef struct _STORY_Telemetry {

  /* client version */
  unsigned int client_version_maj;
  unsigned int client_version_min;
  
  /* (x,y,z) coordinates */
  float x;
  float y;
  float z;

  /* speed */
  float speed;
  float speed_min;
  float speed_max;
  
  /* current cargo id and name (NULL if no cargo) */
  char * cargo_id;
  char * cargo;

  /* left blinker (O/1) */
  unsigned int lblinker;
  /* right blinker (O/1) */
  unsigned int rblinker;

  /* trailer_connected (O/1) */
  unsigned int trailer_connected;

  /* truck engine enabled (0/1) */
  unsigned int engine_enabled;
  
  /* running time for current state (seconds) */
  unsigned int stateruntime;

} _STORY_Telemetry_t;


/* ----------------------------------------------------------------------------------- */

typedef struct _STORY_Position {

  /* position name. used as a UUID */
  char * name;

  /* position coordinates */
  float x;
  float y;
  float z;

  /* default bounding box for position box condition */
  float boxx;
  float boxy;
  float boxz;

  /* default distance for position distance condition */
  float distance;
  
} _STORY_Position_t;

typedef struct _STORY_PositionList {

  /* size, given as the number of positions */
  unsigned int size;

  /* capacity, given as the number of positions */
  unsigned int capacity;

  _STORY_Position_t ** tab;


} _STORY_PositionList_t;


/* ----------------------------------------------------------------------------------- */

#define _STORY_CONDITION_TYPE_POSITION_BOX_IN 0
#define _STORY_CONDITION_TYPE_POSITION_BOX_IN_STR "position_box_in"
#define _STORY_CONDITION_TYPE_POSITION_BOX_OUT 1
#define _STORY_CONDITION_TYPE_POSITION_BOX_OUT_STR "position_box_out"
#define _STORY_CONDITION_TYPE_TIMER_INF 2
#define _STORY_CONDITION_TYPE_TIMER_INF_STR "timer_inf"
#define _STORY_CONDITION_TYPE_TIMER_SUP 3
#define _STORY_CONDITION_TYPE_TIMER_SUP_STR "timer_sup"
#define _STORY_CONDITION_TYPE_DISTANCE_INF 4
#define _STORY_CONDITION_TYPE_DISTANCE_INF_STR "distance_inf"
#define _STORY_CONDITION_TYPE_DISTANCE_SUP 5
#define _STORY_CONDITION_TYPE_DISTANCE_SUP_STR "distance_sup"
#define _STORY_CONDITION_TYPE_PROPERTY_EQUAL 6
#define _STORY_CONDITION_TYPE_PROPERTY_EQUAL_STR "property_equal"
#define _STORY_CONDITION_TYPE_PROPERTY_DIFF 7
#define _STORY_CONDITION_TYPE_PROPERTY_DIFF_STR "property_different"
#define _STORY_CONDITION_TYPE_VISITED_INF 8
#define _STORY_CONDITION_TYPE_VISITED_INF_STR "visited_inf"
#define _STORY_CONDITION_TYPE_VISITED_SUP 9
#define _STORY_CONDITION_TYPE_VISITED_SUP_STR "visited_sup"
#define _STORY_CONDITION_TYPE_VISITED_EQUAL 10
#define _STORY_CONDITION_TYPE_VISITED_EQUAL_STR "visited_equal"
#define _STORY_CONDITION_TYPE_VISITED_DIFF 11
#define _STORY_CONDITION_TYPE_VISITED_DIFF_STR "visited_different"
#define _STORY_CONDITION_TYPE_SPEED_INF 12
#define _STORY_CONDITION_TYPE_SPEED_INF_STR "speed_inf"
#define _STORY_CONDITION_TYPE_SPEED_SUP 13
#define _STORY_CONDITION_TYPE_SPEED_SUP_STR "speed_sup"
#define _STORY_CONDITION_TYPE_SPEED_MAX_INF 14
#define _STORY_CONDITION_TYPE_SPEED_MAX_INF_STR "speed_max_inf"
#define _STORY_CONDITION_TYPE_SPEED_MAX_SUP 15
#define _STORY_CONDITION_TYPE_SPEED_MAX_SUP_STR "speed_max_sup"
#define _STORY_CONDITION_TYPE_SPEED_MIN_INF 16
#define _STORY_CONDITION_TYPE_SPEED_MIN_INF_STR "speed_min_inf"
#define _STORY_CONDITION_TYPE_SPEED_MIN_SUP 17
#define _STORY_CONDITION_TYPE_SPEED_MIN_SUP_STR "speed_min_sup"
#define _STORY_CONDITION_TYPE_SPEED_RESET 18
#define _STORY_CONDITION_TYPE_SPEED_RESET_STR "speed_reset"



typedef struct _STORY_Condition {

  /* condition type. see above. */
  unsigned int type;

  /* (x,y,z) coordinates and position name. used by:
     _STORY_CONDITION_TYPE_POSITION_BOX_IN
     _STORY_CONDITION_TYPE_POSITION_BOX_OUT
     _STORY_CONDITION_TYPE_DISTANCE_SUP
     _STORY_CONDITION_TYPE_DISTANCE_INF */
  char * name;
  float x;
  float y;
  float z;

  /* (x,y,z) box. used by 
     _STORY_CONDITION_TYPE_POSITION_BOX_IN
     _STORY_CONDITION_TYPE_POSITION_BOX_OUT */
  float boxx;
  float boxy;
  float boxz;

  /* speed (km/h). used by
     _STORY_CONDITION_TYPE_SPEED_INF
     _STORY_CONDITION_TYPE_SPEED_SUP
     _STORY_CONDITION_TYPE_SPEED_MAX_INF
     _STORY_CONDITION_TYPE_SPEED_MAX_SUP
     _STORY_CONDITION_TYPE_SPEED_MIN_INF
     _STORY_CONDITION_TYPE_SPEED_MIN_SUP */
  float speed;
  
  /* timer (seconds). used by:
     _STORY_CONDITION_TYPE_TIMER_INF
     _STORY_CONDITION_TYPE_TIMER_SUP */
  unsigned int timer;

  /* distance. used by:
     _STORY_CONDITION_TYPE_DISTANCE_SUP
     _STORY_CONDITION_TYPE_DISTANCE_INF */
  float distance;

  /* property type (_STORY_TELEMETRY_PROPERTY_*) and value. used by:
     _STORY_CONDITION_TYPE_PROPERTY_EQUAL
     _STORY_CONDITION_TYPE_PROPERTY_DIFF */
  unsigned int property_type;
  char * property_value;

  /* number of times the current state has been visited. used by:
     _STORY_CONDITION_TYPE_VISITED_INF
     _STORY_CONDITION_TYPE_VISITED_SUP
     _STORY_CONDITION_TYPE_VISITED_EQUAL
     _STORY_CONDITION_TYPE_VISITED_DIFF */
  unsigned int visited;

} _STORY_Condition_t;



typedef struct _STORY_ConditionList {

  /* size, given as the number of conditions */
  unsigned int size;

  /* capacity, given as the number of conditions */
  unsigned int capacity;

  _STORY_Condition_t ** tab;


} _STORY_ConditionList_t;


/* ----------------------------------------------------------------------------------- */

#define _STORY_TRANSITION_TYPE_AND 0
#define _STORY_TRANSITION_TYPE_AND_STR "and"
#define _STORY_TRANSITION_TYPE_OR 1
#define _STORY_TRANSITION_TYPE_OR_STR "or"

typedef struct _STORY_Transition {

  _STORY_ConditionList_t * conditions;

  struct _STORY_State * nextstate;

  unsigned int nextstateid;

  unsigned int type;

} _STORY_Transition_t;



typedef struct _STORY_TransitionList {

  /* size, given as the number of transitions */
  unsigned int size;

  /* capacity, given as the number of transitions */
  unsigned int capacity;

  _STORY_Transition_t ** tab;


} _STORY_TransitionList_t;



/* ----------------------------------------------------------------------------------- */


typedef struct _STORY_State {

  /* UUID */
  unsigned int id;

  /* name of the state */
  char * name;

  /* description of the state */
  char * description;

  /* url of the associated image */
  char * image;

  /* transition list */  
  _STORY_TransitionList_t * transitions;

  /* number of times this time has been visited */
  unsigned int visited;

} _STORY_State_t;


typedef struct _STORY_StateList {

  /* size, given as the number of states */
  unsigned int size;

  /* capacity, given as the number of states */
  unsigned int capacity;

  _STORY_State_t ** tab;


} _STORY_StateList_t;



/* ----------------------------------------------------------------------------------- */


typedef struct _STORY_Story {

  /* UUID */
  unsigned int id;

  /* name of the story */
  char * name;

  /* author of the story */
  char * author;

  /* meeting point of the story */
  char * meeting;

  /* category of the story */
  char * category;

  /* time to play the story */
  char * time;

  /* difficulty of the story */
  char * difficulty;

  /* required DLC and MOD list */
  char * required;

  /* description of the story */
  char * description;

  /* url of the associated image */
  char * image;

  /* start state */
  _STORY_State_t * startstate;

  /* state list */
  _STORY_StateList_t * states;

  /* distance */
  float distance;

} _STORY_Story_t;



typedef struct _STORY_StoryList {

  /* size, given as the number of stories */
  unsigned int size;

  /* capacity, given as the number of stories */
  unsigned int capacity;

  _STORY_Story_t ** tab;


} _STORY_StoryList_t;



/* ----------------------------------------------------------------------------------- */



_STORY_Context_t *
_STORY_newContext();

void
_STORY_freeContext(_STORY_Context_t ** context);

/* ----------------------------------------------------------------------------------- */


_STORY_Parameters_t *
_STORY_newParameters();

void
_STORY_freeParameters(_STORY_Parameters_t ** parameters);


/* ----------------------------------------------------------------------------------- */


_STORY_Telemetry_t *
_STORY_newTelemetry();

void
_STORY_freeTelemetry(_STORY_Telemetry_t ** telemetry);

char *
_STORY_toStringTelemetry(_STORY_Telemetry_t * telemetry);

void
_STORY_displayTelemetry(_STORY_Telemetry_t * telemetry);

/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */

_STORY_Position_t *
_STORY_newPosition();

void
_STORY_freePosition(_STORY_Position_t ** position);

_STORY_PositionList_t *
_STORY_newPositionList(unsigned int capacity);

void
_STORY_freePositionList(_STORY_PositionList_t ** positionlist);

void
_STORY_freePositionStructList(_STORY_PositionList_t ** positionlist);

unsigned int
_STORY_addPositionToPositionList(_STORY_Position_t * position,
                                 _STORY_PositionList_t * positionlist);

_STORY_Position_t *
_STORY_getPositionFromName(_STORY_PositionList_t * positionlist, char * name);


/* ----------------------------------------------------------------------------------- */

void
_STORY_positionToCondition(_STORY_Position_t * position,
                           _STORY_Condition_t * condition);

/* ----------------------------------------------------------------------------------- */

_STORY_Condition_t *
_STORY_newCondition(unsigned int type);

void
_STORY_freeCondition(_STORY_Condition_t ** condition);

char *
_STORY_toStringCondition(_STORY_Condition_t * condition);

_STORY_ConditionList_t *
_STORY_newConditionList(unsigned int capacity);

void
_STORY_freeConditionList(_STORY_ConditionList_t ** conditionlist);

unsigned int
_STORY_addConditionToConditionList(_STORY_Condition_t * condition,
                                   _STORY_ConditionList_t * conditionlist);

/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */

_STORY_Transition_t *
_STORY_newTransition(unsigned int nextstateid);

void
_STORY_freeTransition(_STORY_Transition_t ** transition);

char *
_STORY_toStringTransition(_STORY_Transition_t * transition);

_STORY_TransitionList_t *
_STORY_newTransitionList(unsigned int capacity);

void
_STORY_freeTransitionList(_STORY_TransitionList_t ** transitionlist);

unsigned int
_STORY_addTransitionToTransitionList(_STORY_Transition_t * transition,
                                     _STORY_TransitionList_t * transitionlist);

/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */

_STORY_State_t *
_STORY_newState(unsigned int id);

void
_STORY_freeState(_STORY_State_t ** state);

_STORY_StateList_t *
_STORY_newStateList(unsigned int capacity);

void
_STORY_freeStateList(_STORY_StateList_t ** statelist);

void
_STORY_freeStateStructList(_STORY_StateList_t ** statelist);

unsigned int
_STORY_addStateToStateList(_STORY_State_t * state,
                           _STORY_StateList_t * statelist);
_STORY_State_t *
_STORY_getStateFromId(_STORY_StateList_t * statelist, unsigned int id);


/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */

_STORY_Story_t *
_STORY_newStory(unsigned int id);

void
_STORY_freeStory(_STORY_Story_t ** story);

_STORY_StoryList_t *
_STORY_newStoryList(unsigned int capacity);

void
_STORY_freeStoryList(_STORY_StoryList_t ** storylist);

void
_STORY_freeStoryStructList(_STORY_StoryList_t ** storylist);

unsigned int
_STORY_addStoryToStoryList(_STORY_Story_t * story,
                           _STORY_StoryList_t * storylist);

/* ----------------------------------------------------------------------------------- */


#endif
