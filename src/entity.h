#pragma once

#include "components.h"

typedef struct Entity
{
  int id;
  int totalComponents;
} Entity;

typedef enum EntityTag
{
  NONE = 0,
  PLAYER,
  ENEMY,
  BULLET
} EntityTag;

typedef struct EntityManager EntityManager, *pEntityManager;
struct EntityManager
{
  int totalEntities;
  // array of Entities
  Entity *entities;
  Entity ***entitiesByTag;
  // array of array of Component
  // the index of the first array is the entity id
  Component **components;
};

typedef unsigned char * ComponentName;

typedef struct NewEntityParams
{
  EntityTag tag;
} NewEntityParams;

void EntityManager_init(pEntityManager self);
Entity *getPlayer(EntityManager *em);

Component *addComponent(EntityManager *em, Entity *entity, ComponentName componentName, void *value);
Component *addComponentToCurrentPlayer(EntityManager *em, ComponentName componentName, void *value);

void removeComponent(EntityManager *em, Component *c, int entityId);

Component *findComponent(EntityManager *em, Entity *entity, ComponentName componentName);
void *getComponentValue(EntityManager *em, Entity *entity, ComponentName componentName);

Entity* getEntity(EntityManager *em, int entityId);
void addEntity(EntityManager *em, NewEntityParams params);
void spawnEntity(EntityManager *em, bool addComponents);
