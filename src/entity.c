#include <SDL3/SDL.h>

#include "base.h"
#include "entity.h"


Component* addComponent(EntityManager *em, Entity *entity, ComponentName componentName, void *value)
{
  int entityId = entity->id;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
  increaseArray(&em->components[entityId], sizeof(Component *), &entity->totalComponents);
#pragma clang diagnostic pop

  Component *c = SDL_calloc(1, sizeof(Component));
  c->hash = hash(componentName);
  c->value = value;

  em->components[entityId][entity->totalComponents - 1] = c;
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
void *findComponent(EntityManager *em, Entity *entity, ComponentName componentName)
{
  int entityId = entity->id;
  unsigned long hashedName = hash(componentName);
  Component *foundComponent = NULL;

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i)
  {
    Component *c = em->components[entityId][i];
    if (c->hash == hashedName) {
      foundComponent = c;
      break;
    }
  }

  if (!foundComponent)
    return NULL;

  return foundComponent->value;
}

void addEntity(EntityManager *entityManager)
{
  Entity *entity = SDL_calloc(1, sizeof(Entity));
  entity->id = entityManager->totalEntities;
  entity->totalComponents = 0;

  // is needed to increase the array of components but we don't need to keep
  // track of it, since the totalEntities counter will suffice
  int totalEntities = entityManager->totalEntities;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
  increaseArray(&entityManager->entities, sizeof(Entity *), &entityManager->totalEntities);
  increaseArray(&entityManager->components, sizeof(Component **),&totalEntities);
#pragma clang diagnostic pop

  entityManager->entities[entity->id] = entity;
  entityManager->components[entity->id] = NULL;
}

void spawnEntity(EntityManager *em, bool addComponents)
{
  addEntity(em);
  if (addComponents)
  {
    Entity *e = em->entities[em->totalEntities - 1];

    Position *pos = SDL_malloc(sizeof(Position));
    pos->x = randomClamped(1, 1280);
    pos->y = randomClamped(1, 720);
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
  // FIXME: for now we assume the first entity the player
  // implement a proper way to retrieve the player
  if (em->totalEntities == 0) return NULL;

  return em->entities[0];
}

void removeComponent(EntityManager *em, Component *c, int entityId)
{
}
