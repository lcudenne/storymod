
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
_STORY_addXmlCondition(xmlDocPtr doc, xmlNodePtr conditionnode,
                       _STORY_Transition_t * transition) {

  _STORY_Condition_t * condition = NULL;
  char * typestr = NULL;

  xmlNodePtr position = NULL;
  xmlNodePtr distance = NULL;
  xmlNodePtr box = NULL;
  xmlNodePtr timer = NULL;
  xmlNodePtr property = NULL;

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
  } else {
    fprintf(stdout, "Unrecognized condition type (%s)\n", typestr);
  }

  free(typestr);

  if (condition != NULL) {
    _STORY_addConditionToConditionList(condition, transition->conditions);

    position = _LIBXML2_getChild(conditionnode, "POSITION");
    if (position != NULL) {
      condition->name = (char *) xmlGetProp(position, BAD_CAST "name");
      condition->x = _LIBXML2_getFloatProp(position, "x");
      condition->y = _LIBXML2_getFloatProp(position, "y");
      condition->z = _LIBXML2_getFloatProp(position, "z");
    }
    box = _LIBXML2_getChild(conditionnode, "BOX");
    if (box != NULL) {
      condition->boxx = _LIBXML2_getFloatProp(box, "x");
      condition->boxy = _LIBXML2_getFloatProp(box, "y");
      condition->boxz = _LIBXML2_getFloatProp(box, "z");
    }
    distance = _LIBXML2_getChild(conditionnode, "DISTANCE");
    if (distance != NULL) {
      condition->distance = _LIBXML2_getFloatProp(distance, "value");
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
      }
      free(typestr);
      condition->property_value = (char *) xmlGetProp(property, BAD_CAST "value");
    }

  }

}


static void
_STORY_addXmlTransition(xmlDocPtr doc, xmlNodePtr transitionnode,
                        _STORY_State_t * state) {

  _STORY_Transition_t * transition = NULL;

  unsigned int nextstateid = 0;

  xmlNodeSetPtr conditionsnode = NULL;

  char * typestr = NULL;

  unsigned int i = 0;

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
    _STORY_addXmlCondition(doc, conditionsnode->nodeTab[i], transition);
  }

  xmlXPathFreeNodeSet(conditionsnode);
  
}


static void
_STORY_addXmlState(xmlDocPtr doc, xmlNodePtr statenode,
                   _STORY_Story_t * story, unsigned int numstartstate,
                   char * dirname) {

  _STORY_State_t * state = NULL;
  unsigned int id = 0;
  xmlNodePtr anode = NULL;

  xmlNodeSetPtr transitionsnode = NULL;
  char * imagefile = NULL;

  unsigned int i = 0;

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
    _STORY_addXmlTransition(doc, transitionsnode->nodeTab[i], state);
  }

  xmlXPathFreeNodeSet(transitionsnode);

}



static void
_STORY_xmlToStory(xmlDocPtr doc, _STORY_Story_t * story, char * dirname) {

  xmlNodePtr storymod = NULL;
  xmlNodePtr storynode = NULL;
  xmlNodePtr anode = NULL;

  xmlNodeSetPtr statesnode = NULL;

  char * strstartstate = NULL;
  unsigned int numstartstate = 0;

  char * imagefile = NULL;

  unsigned int i = 0;

  storymod = xmlDocGetRootElement(doc);
  assert(storymod);
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
    _STORY_addXmlState(doc, statesnode->nodeTab[i],
                       story, numstartstate, dirname);
  }

  xmlXPathFreeNodeSet(statesnode);

}


/* ----------------------------------------------------------------------------------- */


static void
_STORY_resolveConditions(_STORY_Story_t * story) {

  _STORY_State_t * state = NULL;
  _STORY_Transition_t * transition = NULL;
  unsigned int i = 0;
  unsigned int j = 0;

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
    }
  }

}


/* ----------------------------------------------------------------------------------- */



_STORY_Story_t *
_STORY_loadStoryFromFile(char * filename, char * dirname, unsigned int id) {

  _STORY_Story_t * story = NULL;

  xmlDocPtr doc = NULL;

  assert(filename);
  assert(dirname);

  fprintf(stdout, "Loading story %s\n", filename);

  doc = _LIBXML2_loadXmlDoc(filename);

  story = _STORY_newStory(id);

  if (doc != NULL) {
    _STORY_xmlToStory(doc, story, dirname);
    _LIBXML2_freeXmlTree(doc);
    _STORY_resolveConditions(story);
  }

  return story;

}



/* ----------------------------------------------------------------------------------- */
