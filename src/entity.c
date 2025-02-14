#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <SDL3/SDL.h>

#include "base.h"
#include "entity.h"
#include "constants.h"

void EntityManager_init(pEntityManager self)
{
  self->entitiesByTag = SDL_calloc(BULLET, sizeof(Entity**));
  for (int i = 0; i < BULLET; i++)
  {
    self->entitiesByTag[i] = NULL;
  }
}


Component* addComponent(EntityManager *em, Entity *entity, ComponentName componentName, void *value)
{
  int entityId = entity->id;

  increaseArray(&em->components[entityId], sizeof(Component), &entity->totalComponents);

  Component *c = SDL_calloc(1, sizeof(Component));
  c->hash = hash(componentName);
  c->value = value;

  em->components[entityId][entity->totalComponents - 1] = *c;
  return c;
}

Component *addComponentToCurrentPlayer(EntityManager *em, ComponentName componentName, void *value)
{
  Entity* player = getPlayer(em); 
  if (!player) return NULL;

  return addComponent(em, player, componentName, value);
}

// TODO: make it return a full component and add another function
// that returns just the value
Component *findComponent(EntityManager *em, Entity *entity, ComponentName componentName)
{
  int entityId = entity->id;
  unsigned long hashedName = hash(componentName);
  Component *foundComponent = NULL;

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i)
  {
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

void *getComponentValue(EntityManager *em, Entity *entity, ComponentName componentName)
{
  int entityId = entity->id;
  unsigned long hashedName = hash(componentName);
  Component *foundComponent = NULL;

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i)
  {
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

Entity *getEntity(EntityManager *entityManager, int entityId)
{
  if (!(entityId < entityManager->totalEntities))
    return nullptr;

  return &entityManager->entities[entityId];
}

void addEntity(EntityManager *entityManager, NewEntityParams params)
{
  Entity *entity = SDL_calloc(1, sizeof(Entity));
  entity->id = entityManager->totalEntities;
  entity->totalComponents = 0;

  // is needed to increase the array of components but we don't need to keep
  // track of it, since the totalEntities counter will suffice
  int totalEntities = entityManager->totalEntities;

  increaseArray(&entityManager->entities, sizeof(Entity), &entityManager->totalEntities);
  increaseArray(&entityManager->components, sizeof(Component),&totalEntities);

  // FIXME: this is wrong I should go back to the array of pointer or find another solution
  // to handle memory if I don't want to free each entity by itself
  entityManager->entities[entity->id] = *entity;
  SDL_Log("created entity %p, saved entity %p", entity, &entityManager->entities[entity->id]);
  entityManager->components[entity->id] = NULL;

  if (params.tag) {
    int len = sizeof(entityManager->entitiesByTag[params.tag])/sizeof(Entity*);
    increaseArray(&entityManager->entitiesByTag[params.tag], sizeof(Entity *),&len);
    SDL_Log("Entity for tag %d at position %d - with address %p", params.tag, (len - 2), entity);
    entityManager->entitiesByTag[params.tag][len - 2] = entity;
  }
}

void spawnEntity(EntityManager *em, bool addComponents)
{
  NewEntityParams params = {};
  if (em->totalEntities == 0)
    params.tag = PLAYER;
  addEntity(em, params);

  if (addComponents)
  {
    Entity *e = getEntity(em, em->totalEntities - 1);

    Position *pos = SDL_malloc(sizeof(Position));
    pos->x = randomClamped(1, SCREEN_WIDTH);
    pos->y = randomClamped(1, SCREEN_HEIGHT);
    addComponent(em, e, "Position", pos);

    Velocity *vel = SDL_malloc(sizeof(Velocity));
    vel->x = randomClamped(5, 20);
    vel->y = randomClamped(5, 20);
    addComponent(em, e, "Velocity", vel);

    Color *color = SDL_malloc(sizeof(Color));
    color->r = randomClamped(0, 255);
    color->g = randomClamped(0, 255);
    color->b = randomClamped(0, 255);
    color->a = 255;
    addComponent(em, e, "Color", color);

    Shape *shape = SDL_malloc(sizeof(Shape));
    shape->pointCount = randomClamped(3, 8);
    shape->radius = randomClamped(20, 30);
    addComponent(em, e, "Shape", shape);
  }
}

Entity *getPlayer(EntityManager *em)
{
  // FIXME: find a proper way to return the player
  int len = sizeof(em->entitiesByTag[PLAYER])/sizeof(Entity *);
  Entity* player = em->entitiesByTag[PLAYER][0];
  /*printf("There are %p players\n", player);*/
  return player;
}

void removeComponent(EntityManager *em, Component *c, int entityId)
{
}

