#pragma once

#include "components.h"

typedef struct Entity
{
  int id;
  int totalComponents;
  Component **components;
} Entity;

typedef struct EntityManager
{
  int totalEntities;
  Entity **entities;
} EntityManager;

void addComponent(Entity *entity, Component *c);
void *findComponent(Entity *entity, unsigned char *componentName);

void addEntity(EntityManager *em);
void spawnEntity(EntityManager *em, bool addComponents);
