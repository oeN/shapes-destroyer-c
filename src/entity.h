#pragma once

#include "components.h"
#include "memory.h"

// FIXME: find a way to use the memory_arena and have dynamically sized arrays
// theese variables should not exists at all, saved maybe the TAG one
#define MAX_ENTITIES 512
#define MAX_ENTITY_TAGS 10
#define MAX_COMPONENTS_PER_ENTITY 20

typedef struct Entity {
  int id;
  int totalComponents;
} entity;

typedef enum EntityTag { //
  NONE = 0,
  ALL,
  PLAYER,
  ENEMY,
  BULLET
} EntityTag;

typedef struct EntitiesByTag {
  u16 count;
  u32 entityIds[MAX_ENTITIES];
} entities_by_tag;

typedef struct EntityManager entity_manager, *entity_manager_ptr;
struct EntityManager {
  int totalEntities;
  entity entities[MAX_ENTITIES];
  // just for convenience and test it out keep the pointer of the
  // game arena also here
  memory_arena *gameArena;

  // an array of entities_by_tag where the key is the EntityTag
  entities_by_tag entitiesByTag[MAX_ENTITY_TAGS];

  // array of array of Component
  // the index of the first array is the entity id
  Component components[MAX_ENTITIES][MAX_COMPONENTS_PER_ENTITY];
};

typedef unsigned char *component_name;

typedef struct NewEntityParams {
  EntityTag tag;
} new_entity_params;

void EntityManager_init(entity_manager_ptr self);
entity *getPlayer(entity_manager *EntityManager);

Component *addComponent(entity_manager *em, entity *entity,
                        component_name componentName, void *value);
Component *addComponentToCurrentPlayer(entity_manager *em,
                                       component_name componentName,
                                       void *value);

void removeComponent(entity_manager *em, Component *c, int entityId);

Component *findComponent(entity_manager *em, entity *entity,
                         component_name componentName);
void *getComponentValue(entity_manager *em, entity *entity,
                        component_name componentName);

entity *getEntity(entity_manager *em, int entityId);
entity *getEntityByTag(entity_manager *em, u8 tag, int position);
entity *setEntityByTag(entity **entity, entity_manager *em, u8 tag,
                       int position);
int **getEntitiesByTag(entity_manager *em, u8 tag);

void addEntity(entity_manager *em, new_entity_params params);
void spawnEntity(entity_manager *em, bool addComponents);
