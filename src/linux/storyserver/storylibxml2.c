
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
/* strcmp */
#include <string.h>
/* xmlDocPtr */
#include <libxml/encoding.h>
/* xmlNodeSetPtr */
#include <libxml/xpath.h>




#include "storylibxml2.h"
#include "storyposixload.h"
#include "utils.h"


/* ----------------------------------------------------------------------------------- */


static xmlDocPtr
_LIBXML2_loadXmlDoc(const char * const filename) {

  xmlDocPtr doc = NULL;

  assert(filename);

  doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);

  return doc;
}


/* ----------------------------------------------------------------------------------- */

static void
_LIBXML2_freeXmlTree(xmlDocPtr doc) {

  xmlFreeDoc(doc);

}


void
_LIBXML2_xmlTerminate() {

  xmlCleanupParser();

}


/* ----------------------------------------------------------------------------------- */


static xmlNodePtr
_LIBXML2_getChild(xmlNodePtr node, const char * const name) {
  xmlNodePtr cur = NULL;

  assert(node);
  assert(name);

  cur = node->children;
  while (cur && strcmp((const char*) cur->name, name))
    cur = cur->next;

  return cur;
}

static xmlNodeSetPtr
_LIBXML2_XPathQueryCtx(xmlDocPtr doc, xmlNodePtr ctxnode,
                       const char * const query) {

  xmlXPathContextPtr xpathctx = NULL;
  xmlXPathObjectPtr xpathobj = NULL;
  xmlNodeSetPtr nodes = NULL;

  assert(query);

  xpathctx = xmlXPathNewContext(doc);

  if (ctxnode)
    xpathctx->node = ctxnode;

  xpathobj = xmlXPathEvalExpression(BAD_CAST query, xpathctx);

  assert(xpathobj);

  nodes = xpathobj->nodesetval;

  /* Do not forget to
     xmlXPathFreeNodeSet(nodes);
  */

  xmlXPathFreeNodeSetList(xpathobj);
  xmlXPathFreeContext(xpathctx);

  return nodes;
}

static unsigned int
_LIBXML2_getUnsignedIntProp(xmlNodePtr node, char * name) {

  unsigned int res = 0;

  xmlChar *agattr = NULL;

  assert(node);
  assert(name);

  agattr = xmlGetProp(node, BAD_CAST name);

  if (agattr)
    res = atol((const char*) agattr);

  xmlFree(agattr);

  return res;
}

static float
_LIBXML2_getFloatProp(xmlNodePtr node, char * name) {

  float res = 0;

  xmlChar *agattr = NULL;

  assert(node);
  assert(name);

  agattr = xmlGetProp(node, BAD_CAST name);

  if (agattr)
    res = atof((const char*) agattr);

  xmlFree(agattr);

  return res;
}


/* ----------------------------------------------------------------------------------- */

static void
_STORY_addXmlAction(_STORY_Context_t * context, xmlDocPtr doc, xmlNodePtr actionnode,
                    _STORY_Transition_t * transition) {

  _STORY_Action_t * action = NULL;
  char * typestr = NULL;
  
  assert(context);
  assert(doc);
  assert(actionnode);
  assert(transition);

  typestr = (char *) xmlGetProp(actionnode, BAD_CAST "type");
  assert(typestr);

  if (strcmp(typestr, _STORY_ACTION_TYPE_SPEED_RESET_STR) == 0) {
    action = _STORY_newAction(_STORY_ACTION_TYPE_SPEED_RESET);
  } else {
    fprintf(stdout, "Unrecognized action type (%s)\n", typestr);
  }

  free(typestr);
  typestr = NULL;
  
  if (action != NULL) {
    _STORY_addActionToActionList(action, transition->actions);
  }
    
}


static void
_STORY_addXmlCondition(_STORY_Context_t * context, xmlDocPtr doc, xmlNodePtr conditionnode,
                       _STORY_Transition_t * transition, _STORY_PositionList_t * storypositions) {

  _STORY_Condition_t * condition = NULL;
  char * typestr = NULL;
  _STORY_Position_t * position = NULL;
  
  xmlNodePtr positionnode = NULL;
  xmlNodePtr distance = NULL;
  xmlNodePtr box = NULL;
  xmlNodePtr timer = NULL;
  xmlNodePtr property = NULL;
  xmlNodePtr visited = NULL;
  xmlNodePtr speed = NULL;

  float value = 0.0;

  char * stateliststr = NULL;
  
  assert(context);
  assert(doc);
  assert(conditionnode);
  assert(transition);

  typestr = (char *) xmlGetProp(conditionnode, BAD_CAST "type");
  assert(typestr);

  if (strcmp(typestr, _STORY_CONDITION_TYPE_POSITION_BOX_IN_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_POSITION_BOX_IN);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_POSITION_BOX_OUT_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_POSITION_BOX_OUT);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_TIMER_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_TIMER_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_TIMER_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_TIMER_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_DISTANCE_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_DISTANCE_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_DISTANCE_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_DISTANCE_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_PROPERTY_EQUAL_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_PROPERTY_EQUAL);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_PROPERTY_DIFF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_PROPERTY_DIFF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_VISITED_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_VISITED_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_VISITED_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_VISITED_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_VISITED_EQUAL_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_VISITED_EQUAL);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_VISITED_DIFF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_VISITED_DIFF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_MAX_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_MAX_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_MAX_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_MAX_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_MIN_INF_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_MIN_INF);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_SPEED_MIN_SUP_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_SPEED_MIN_SUP);
  } else if (strcmp(typestr, _STORY_CONDITION_TYPE_VISITED_LIST_STR) == 0) {
    condition = _STORY_newCondition(_STORY_CONDITION_TYPE_VISITED_LIST);
  } else {
    fprintf(stdout, "Unrecognized condition type (%s)\n", typestr);
  }

  free(typestr);
  typestr = NULL;
  
  if (condition != NULL) {
    _STORY_addConditionToConditionList(condition, transition->conditions);

    positionnode = _LIBXML2_getChild(conditionnode, "POSITION");
    if (positionnode != NULL) {
      condition->name = (char *) xmlGetProp(positionnode, BAD_CAST "name");
      if (condition->name != NULL) {
        position = _STORY_getPositionFromName(context->positions, condition->name);
        if (position != NULL) {
          _STORY_positionToCondition(position, condition);
        }
        if (storypositions != NULL) {
          position = _STORY_getPositionFromName(storypositions, condition->name);
          if (position != NULL) {
            _STORY_positionToCondition(position, condition);
          }
        }
      }
      value = _LIBXML2_getFloatProp(positionnode, "x");
      if (value != 0) {
        condition->x = value;
      }
      value = _LIBXML2_getFloatProp(positionnode, "y");
      if (value != 0) {
        condition->y = value;
      }
      value = _LIBXML2_getFloatProp(positionnode, "z");
      if (value != 0) {
        condition->z = value;
      }
    }
    box = _LIBXML2_getChild(conditionnode, "BOX");
    if (box != NULL) {
      value = _LIBXML2_getFloatProp(box, "x");
      if (value != 0) {
        condition->boxx = value;
      }
      value = _LIBXML2_getFloatProp(box, "y");
      if (value != 0) {
        condition->boxy = value;
      }
      value = _LIBXML2_getFloatProp(box, "z");
      if (value != 0) {
        condition->boxz = value;
      }
    }
    distance = _LIBXML2_getChild(conditionnode, "DISTANCE");
    if (distance != NULL) {
      value = _LIBXML2_getFloatProp(distance, "value");
      if (value != 0) {
        condition->distance = value;
      }
    }
    timer = _LIBXML2_getChild(conditionnode, "TIMER");
    if (timer != NULL) {
      condition->timer = _LIBXML2_getUnsignedIntProp(timer, "value");
    }
    property = _LIBXML2_getChild(conditionnode, "PROPERTY");
    if (property != NULL) {
      typestr = (char *) xmlGetProp(property, BAD_CAST "type");
      assert(typestr);
      if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_CARGO_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_CARGO;
      } else if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_LBLINKER_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_LBLINKER;
      } else if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_RBLINKER_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_RBLINKER;
      } else if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_TRAILER_CONNECTED;
      } else if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_ENGINE_ENABLED_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_ENGINE_ENABLED;
      } else if (strcmp(typestr, _STORY_TELEMETRY_PROPERTY_PARKING_BRAKE_STR) == 0) {
        condition->property_type = _STORY_TELEMETRY_PROPERTY_PARKING_BRAKE;
      }
      free(typestr);
      typestr = NULL;
      condition->property_value = (char *) xmlGetProp(property, BAD_CAST "value");
    }
    visited = _LIBXML2_getChild(conditionnode, "VISITED");
    if (visited != NULL) {
      condition->visited = _LIBXML2_getUnsignedIntProp(visited, "value");
      stateliststr = (char *) xmlGetProp(visited, BAD_CAST "statelist");
      if (stateliststr != NULL) {
        condition->statelistid = _UT_stringToUnsignedIntList(stateliststr);
        free(stateliststr);
        stateliststr = NULL;
      }
    }
    speed = _LIBXML2_getChild(conditionnode, "SPEED");
    if (speed != NULL) {
      condition->speed = _LIBXML2_getUnsignedIntProp(speed, "value");
    }

  }

}


static void
_STORY_addXmlTransition(_STORY_Context_t * context, xmlDocPtr doc, xmlNodePtr transitionnode,
                        _STORY_State_t * state, _STORY_PositionList_t * storypositions) {

  _STORY_Transition_t * transition = NULL;

  unsigned int nextstateid = 0;

  xmlNodeSetPtr conditionsnode = NULL;
  xmlNodeSetPtr actionsnode = NULL;

  char * typestr = NULL;

  unsigned int i = 0;

  assert(context);
  assert(doc);
  assert(transitionnode);
  assert(state);

  nextstateid = _LIBXML2_getUnsignedIntProp(transitionnode, "nextstate");
  transition = _STORY_newTransition(nextstateid);
  assert(transition);
  _STORY_addTransitionToTransitionList(transition, state->transitions);

  typestr = (char *) xmlGetProp(transitionnode, BAD_CAST "type");
  assert(typestr);
  if (strcmp(typestr, _STORY_TRANSITION_TYPE_AND_STR) == 0) {
    transition->type = _STORY_TRANSITION_TYPE_AND;
  } else if (strcmp(typestr, _STORY_TRANSITION_TYPE_OR_STR) == 0) {
    transition->type = _STORY_TRANSITION_TYPE_OR;
  }
  free(typestr);

  conditionsnode = _LIBXML2_XPathQueryCtx(doc, transitionnode,
                                          "CONDITIONS/CONDITION");
  assert(conditionsnode);

  for (i = 0; i < conditionsnode->nodeNr; i++) {
    _STORY_addXmlCondition(context, doc, conditionsnode->nodeTab[i], transition, storypositions);
  }

  xmlXPathFreeNodeSet(conditionsnode);

  actionsnode = _LIBXML2_XPathQueryCtx(doc, transitionnode,
                                       "ACTIONS/ACTION");
  if (actionsnode) {
    for (i = 0; i < actionsnode->nodeNr; i++) {
      _STORY_addXmlAction(context, doc, actionsnode->nodeTab[i], transition);
    }    
    xmlXPathFreeNodeSet(actionsnode);
  }

}


static void
_STORY_addXmlState(_STORY_Context_t * context, xmlDocPtr doc, xmlNodePtr statenode,
                   _STORY_Story_t * story, unsigned int numstartstate,
                   char * dirname, _STORY_PositionList_t * storypositions) {

  _STORY_State_t * state = NULL;
  unsigned int id = 0;
  xmlNodePtr anode = NULL;

  xmlNodeSetPtr transitionsnode = NULL;
  char * imagefile = NULL;

  unsigned int i = 0;

  assert(context);
  assert(doc);
  assert(statenode);
  assert(story);

  id = _LIBXML2_getUnsignedIntProp(statenode, "id");
  state = _STORY_newState(id);
  assert(state);
  _STORY_addStateToStateList(state, story->states);
  if (id == numstartstate) {
    story->startstate = state;
  }

  anode = _LIBXML2_getChild(statenode, "NAME");
  assert(anode);
  state->name = (char *) xmlNodeGetContent(anode);
  anode = NULL;
  anode = _LIBXML2_getChild(statenode, "DESCRIPTION");
  if (anode) {
    state->description = (char *) xmlNodeGetContent(anode);
    anode = NULL;
  }
  anode = _LIBXML2_getChild(statenode, "IMAGE");
  if (anode) {
    imagefile = (char *) xmlNodeGetContent(anode);
    state->image = _UT_strCpy(state->image, dirname);
    state->image = _UT_strCat(state->image, "/");
    state->image = _UT_strCat(state->image, imagefile);
    free(imagefile);
    anode = NULL;
  }

  transitionsnode = _LIBXML2_XPathQueryCtx(doc, statenode,
                                           "TRANSITIONS/TRANSITION");
  assert(transitionsnode);

  for (i = 0; i < transitionsnode->nodeNr; i++) {
    _STORY_addXmlTransition(context, doc, transitionsnode->nodeTab[i], state, storypositions);
  }

  xmlXPathFreeNodeSet(transitionsnode);

}



static unsigned int
_STORY_xmlToStory(_STORY_Context_t * context, xmlDocPtr doc,
                  _STORY_Story_t * story, char * dirname) {

  unsigned int success = 1;

  _STORY_PositionList_t * storypositions = NULL;
  
  xmlNodePtr storymod = NULL;
  xmlNodePtr storynode = NULL;
  xmlNodePtr anode = NULL;

  xmlNodeSetPtr statesnode = NULL;

  char * strstartstate = NULL;
  unsigned int numstartstate = 0;

  char * imagefile = NULL;

  char * csscontent = NULL;
  char * storycssfile = NULL;

  char * positionscontent = NULL;
  char * storypositionsfile = NULL;

  unsigned int i = 0;

  assert(context);
  assert(doc);
  
  storymod = xmlDocGetRootElement(doc);
  assert(storymod);

  if (strcmp((const char *) storymod->name, "STORYMOD") == 0) {

    storynode = _LIBXML2_getChild(storymod, "STORY");
    assert(storynode);
    anode = _LIBXML2_getChild(storynode, "NAME");
    assert(anode);
    story->name = (char *) xmlNodeGetContent(anode);
    anode = NULL;
    anode = _LIBXML2_getChild(storynode, "AUTHOR");
    if (anode) {
      story->author = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "MEETING");
    if (anode) {
      story->meeting = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "CATEGORY");
    if (anode) {
      story->category = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "TIME");
    if (anode) {
      story->time = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "DIFFICULTY");
    if (anode) {
      story->difficulty = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "REQUIRED");
    if (anode) {
      story->required = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "DESCRIPTION");
    if (anode) {
      story->description = (char *) xmlNodeGetContent(anode);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "STYLE");
    if (anode) {
      csscontent = (char *) xmlNodeGetContent(anode);
      if (csscontent != NULL) {
        storycssfile = _UT_strCpy(storycssfile, dirname);
        storycssfile = _UT_strCat(storycssfile, "/");
        storycssfile = _UT_strCat(storycssfile, csscontent);
        story->css_filename = storycssfile;
        free(csscontent);
        fprintf(stdout, "Loading css for story %s (%s)\n", story->name, story->css_filename);
        story->css_file = _STORY_loadFileToChar(story->css_filename);
      }
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "POSITIONS");
    if (anode) {
      positionscontent = (char *) xmlNodeGetContent(anode);
      if (positionscontent != NULL) {
        storypositionsfile = _UT_strCpy(storypositionsfile, dirname);
        storypositionsfile = _UT_strCat(storypositionsfile, "/");
        storypositionsfile = _UT_strCat(storypositionsfile, positionscontent);
        storypositions = _STORY_loadPositionListFromFile(storypositionsfile);
        free(storypositionsfile);
        free(positionscontent);
      }
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "IMAGE");
    if (anode) {
      imagefile = (char *) xmlNodeGetContent(anode);
      story->image = _UT_strCpy(story->image, dirname);
      story->image = _UT_strCat(story->image, "/");
      story->image = _UT_strCat(story->image, imagefile);
      free(imagefile);
      anode = NULL;
    }
    anode = _LIBXML2_getChild(storynode, "STARTSTATE");
    assert(anode);
    strstartstate = (char *) xmlNodeGetContent(anode);
    anode = NULL;
    numstartstate = atoi(strstartstate);
    assert(numstartstate >= 0);
    free(strstartstate);

    statesnode = _LIBXML2_XPathQueryCtx(doc, storynode, "STATES/STATE");
    assert(statesnode);

    for (i = 0; i < statesnode->nodeNr; i++) {
      _STORY_addXmlState(context, doc, statesnode->nodeTab[i],
                         story, numstartstate, dirname, storypositions);
    }

    if (storypositions != NULL) {
      _STORY_freePositionList(&storypositions);
    }
    
    xmlXPathFreeNodeSet(statesnode);

  } else {
    success = 0;
  }

  return success;
}


/* ----------------------------------------------------------------------------------- */


static void
_STORY_resolveConditions(_STORY_Story_t * story) {

  _STORY_State_t * state = NULL;
  _STORY_State_t * tempstate = NULL;
  _STORY_Transition_t * transition = NULL;
  _STORY_Condition_t * condition = NULL;
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int k = 0;
  unsigned int l = 0;
  
  assert(story);

  for (i = 0; i < story->states->size; i++) {
    state = story->states->tab[i];
    for (j = 0; j < state->transitions->size; j++) {
      transition = state->transitions->tab[j];
      transition->nextstate = _STORY_getStateFromId(story->states,
                                                    transition->nextstateid);
      if (transition->nextstate == NULL) {
        fprintf(stdout, "Couldn't find state (%d)\n", transition->nextstateid);
      }
      for (k = 0; k < transition->conditions->size; k++) {
        condition = transition->conditions->tab[k];
        if (condition->type == _STORY_CONDITION_TYPE_VISITED_LIST) {
          if (condition->statelistid != NULL) {
            condition->statelist = _STORY_newStateList(1);
            for (l = 0; l < condition->statelistid->size; l++) {
              tempstate = _STORY_getStateFromId(story->states, condition->statelistid->tab[l]);
              if (tempstate != NULL) {
                _STORY_addStateToStateList(tempstate, condition->statelist);
              } else {
                fprintf(stdout, "Couldn't find state (%d)\n", condition->statelistid->tab[l]);
              }
            }
          }
        }
      }
    }
  }

}


/* ----------------------------------------------------------------------------------- */



_STORY_Story_t *
_STORY_loadStoryFromFile(_STORY_Context_t * context, char * filename,
                         char * dirname, unsigned int id) {

  _STORY_Story_t * story = NULL;

  xmlDocPtr doc = NULL;

  assert(context);
  assert(filename);
  assert(dirname);

  doc = _LIBXML2_loadXmlDoc(filename);

  story = _STORY_newStory(id);

  if (doc != NULL) {

    if (_STORY_xmlToStory(context, doc, story, dirname) == 1) {
      fprintf(stdout, "Loading story %s by %s (%s)\n", story->name, story->author, filename);
      _STORY_resolveConditions(story);
    } else {
      _STORY_freeStory(&story);
    }
    _LIBXML2_freeXmlTree(doc);
  }

  return story;

}



/* ----------------------------------------------------------------------------------- */

static unsigned int
_STORY_xmlToPositionList(xmlDocPtr doc, _STORY_PositionList_t * positionlist) {

  unsigned int success = 1;
  
  xmlNodePtr storymod_positions = NULL;
  xmlNodeSetPtr positionsnode = NULL;
  xmlNodePtr positionnode = NULL;

  _STORY_Position_t * position = NULL;

  unsigned int i = 0;
  
  assert(doc);
  assert(positionlist);

  storymod_positions = xmlDocGetRootElement(doc);
  assert(storymod_positions);

  if (strcmp((const char *) storymod_positions->name, "STORYMOD_POSITIONS") == 0) {
  
    positionsnode = _LIBXML2_XPathQueryCtx(doc, storymod_positions,
                                           "POSITIONS/POSITION");
    assert(positionsnode);
    
    for (i = 0; i < positionsnode->nodeNr; i++) {
      positionnode = positionsnode->nodeTab[i];

      position = _STORY_newPosition();
      position->name = (char *) xmlGetProp(positionnode, BAD_CAST "name");
      position->x = _LIBXML2_getFloatProp(positionnode, "x");
      position->y = _LIBXML2_getFloatProp(positionnode, "y");
      position->z = _LIBXML2_getFloatProp(positionnode, "z");
      position->boxx = _LIBXML2_getFloatProp(positionnode, "boxx");
      position->boxy = _LIBXML2_getFloatProp(positionnode, "boxy");
      position->boxz = _LIBXML2_getFloatProp(positionnode, "boxz");
      position->distance = _LIBXML2_getFloatProp(positionnode, "distance");

      _STORY_addPositionToPositionList(position, positionlist);
    }
    
    xmlXPathFreeNodeSet(positionsnode);

  } else {
    success = 0;
  }

  return success;
}


/* ----------------------------------------------------------------------------------- */


_STORY_PositionList_t *
_STORY_loadPositionListFromFile(char * filename) {

  _STORY_PositionList_t * positionlist = NULL;
  xmlDocPtr doc = NULL;

  assert(filename);

  doc = _LIBXML2_loadXmlDoc(filename);

  positionlist = _STORY_newPositionList(1);
  
  if (doc != NULL) {
    if (_STORY_xmlToPositionList(doc, positionlist) == 1) {
      fprintf(stdout, "Loading %d positions from file (%s)\n", positionlist->size, filename);
    } else {
      _STORY_freePositionList(&positionlist);
    }
    _LIBXML2_freeXmlTree(doc);
  }

  return positionlist;
}

/* ----------------------------------------------------------------------------------- */
