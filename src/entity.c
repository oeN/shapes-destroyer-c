#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <SDL3/SDL.h>

#include "base.h"
#include "constants.h"
#include "entity.h"

void EntityManager_init(entity_manager *self) {
  self->totalEntities = 0;
  for (int i = 0; i < 10; i++) {
    self->entitiesByTag[i] = (entities_by_tag){0};
    // there are problems while increasing array with the current memory
    // arena implementation, in the exercise tab I'm going deeper with the
    // handmade hero memory_arena but I could also try on my own first,
    // let's see tomorrow morning
    self->entitiesByTag[i].count = 0;
    for (int c = 0; c < 512; c++)
      self->entitiesByTag[i].entityIds[c] = 0;
  }
}

Component *addComponent(entity_manager *em, entity *entity,
                        component_name componentName, void *value) {
  int entityId = entity->id;

  em->components[entityId][entity->totalComponents] = (Component){
      .hash = hash(componentName), //
      .value = value               //
  };
  Component *c = &(em->components[entityId][entity->totalComponents]);

  entity->totalComponents++;

  return c;
}

Component *addComponentToCurrentPlayer(entity_manager *em,
                                       component_name componentName,
                                       void *value) {
  entity *player = getPlayer(em);
  if (!player)
    return NULL;

  return addComponent(em, player, componentName, value);
}

// TODO: make it return a full component and add another function
// that returns just the value
Component *findComponent(entity_manager *em, entity *entity,
                         component_name componentName) {
  int entityId = entity->id;
  unsigned long hashedName = hash(componentName);
  Component *foundComponent = NULL;

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i) {
    Component *c = &em->components[entityId][i];
    if (c->hash == hashedName) {
      foundComponent = c;
      break;
    }
  }

  if (!foundComponent)
    return NULL;

  return foundComponent;
}

void *getComponentValue(entity_manager *em, entity *entity,
                        component_name componentName) {
  int entityId = entity->id;

  unsigned long hashedName = hash(componentName);
  Component *foundComponent = NULL;

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i) {
    Component *c = &em->components[entityId][i];
    if (c->hash == hashedName) {
      foundComponent = c;
      break;
    }
  }

  if (!foundComponent)
    return NULL;

  return foundComponent->value;
}

entity *getEntity(entity_manager *entityManager, int entityId) {
  // printf("getEntity: retrieving ID %d - current entities %d\n", entityId,
  //       entityManager->totalEntities);
  if (!(entityId < entityManager->totalEntities))
    return NULL;

  // printf("getEntity: returning ID %d - address %p\n", entityId,
  //       &entityManager->entities[entityId]);
  return &entityManager->entities[entityId];
}

void addTaggedEntity(entity_manager *entityManager, u32 entityId, u8 tag) {
  if (!tag)
    return;
  entities_by_tag *tagContainer = &(entityManager->entitiesByTag[tag]);
  if (!tagContainer)
    SDL_Log("THERE IS NO TAG CONTAINER");
  tagContainer->entityIds[tagContainer->count] = entityId;
  tagContainer->count++;
}

void addEntity(entity_manager *entityManager, new_entity_params params) {
  int entityId = entityManager->totalEntities;

  entityManager->entities[entityId] =
      (entity){.id = entityId, .totalComponents = 0};
  entity *entity = &entityManager->entities[entityId];
  entity->id = entityId;
  entityManager->totalEntities++;

  addTaggedEntity(entityManager, entity->id, params.tag);
  // always add the entity to the ALL tag
  addTaggedEntity(entityManager, entity->id, ALL);
}

void spawnEntity(entity_manager *em, bool addComponents) {
  new_entity_params params = {};
  if (em->totalEntities == 0)
    params.tag = PLAYER;
  else
    params.tag = ENEMY;

  addEntity(em, params);

  if (addComponents) {
    entity *e = getEntity(em, em->totalEntities - 1);

    Position *pos = pushStruct(em->gameArena, Position);
    pos->x = randomClamped(1, SCREEN_WIDTH);
    pos->y = randomClamped(1, SCREEN_HEIGHT);
    addComponent(em, e, "Position", pos);

    Velocity *vel = pushStruct(em->gameArena, Velocity);
    if (params.tag == PLAYER) {
      vel->x = 0;
      vel->y = 0;
    } else {
      vel->x = randomClamped(5, 20);
      vel->y = randomClamped(5, 20);
    }
    addComponent(em, e, "Velocity", vel);

    Color *color = pushStruct(em->gameArena, Color);
    color->r = randomClamped(0, 255);
    color->g = randomClamped(0, 255);
    color->b = randomClamped(0, 255);
    color->a = 255;
    addComponent(em, e, "Color", color);

    Shape *shape = pushStruct(em->gameArena, Shape);
    shape->pointCount = randomClamped(3, 8);
    shape->radius = randomClamped(20, 30);
    addComponent(em, e, "Shape", shape);
  }
}

int **getEntitiesByTag(entity_manager *em, u8 tag) {
  // add guard clause to make sure the tag is one of the EntityTag
  return em->entitiesByTag[tag].entityIds;
}

entity *getEntityByTag(entity_manager *em, u8 tag, int position) {
  // printf("getEntityByTag\n");
  entities_by_tag *tagContainer = &(em->entitiesByTag[tag]);
  // printf("getEntityByTag: tagContainer %p - count %d\n", tagContainer,
  //       tagContainer->count);
  if (position >= tagContainer->count)
    return NULL;

  int entityId = tagContainer->entityIds[position];
  // printf("getEntityByTag: searching entity with ID %d\n", entityId);
  return getEntity(em, entityId);
}

// I use this in a while loop but I don't know if it's needed or the
// getEntityByTag can do both things
entity *setEntityByTag(entity **entity, entity_manager *em, u8 tag,
                       int position) {
  *entity = getEntityByTag(em, tag, position);
  return *entity;
}

entity *getPlayer(entity_manager *em) { return getEntityByTag(em, PLAYER, 0); }

void removeComponent(entity_manager *em, Component *c, int entityId) {}
