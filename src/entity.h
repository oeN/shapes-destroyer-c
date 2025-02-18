#pragma once

#include "components.h"
#include "memory.h"

typedef struct Entity {
  int id;
  int totalComponents;
} entity;

typedef enum EntityTag { //
  NONE = 0,
  PLAYER,
  ENEMY,
  BULLET
} EntityTag;

typedef struct EntitiesByTag {
  u16 count;
  u32 entityIds[512];
} entities_by_tag;

typedef struct EntityManager entity_manager, *entity_manager_ptr;
// FIXME: find a way to use the memory_arena and have dynamically sized arrays
struct EntityManager {
  int totalEntities;
  // array of Entities, max 512 entities
  entity entities[512];
  // just for convenience and test it out keep the pointer of the
  // game arena also here
  memory_arena *gameArena;

  // an array of entities_by_tag where the key is the EntityTag
  entities_by_tag entitiesByTag[10];

  // array of array of Component
  // the index of the first array is the entity id
  // for now each entity can have 512 components
  Component components[512][10];
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
