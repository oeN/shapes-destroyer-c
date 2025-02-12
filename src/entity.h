#pragma once

#include "components.h"

typedef struct Entity
{
  int id;
  int totalComponents;
} Entity;

typedef struct EntityManager
{
  int totalEntities;
  // array of pointers to Entity
  Entity **entities;
  // array of array of pointers to Component
  // the index of the first array is the entity id
  // TODO: replace with a linked list
  Component ***components;
} EntityManager;

typedef unsigned char * ComponentName;

Entity *getPlayer(EntityManager *em);

Component *addComponent(EntityManager *em, Entity *entity, ComponentName componentName, void *value);
Component *addComponentToCurrentPlayer(EntityManager *em, ComponentName componentName, void *value);

void removeComponent(EntityManager *em, Component *c, int entityId);

void *findComponent(EntityManager *em, Entity *entity, ComponentName componentName);

void addEntity(EntityManager *em);
void spawnEntity(EntityManager *em, bool addComponents);
