#include <SDL3/SDL.h>

#include "base.h"
#include "entity.h"


void addComponent(Entity *entity, Component *c)
{

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
  increaseArray(&entity->components, sizeof(Component *), &entity->totalComponents);
#pragma clang diagnostic pop

  entity->components[entity->totalComponents - 1] = c;
}

void *findComponent(Entity *entity, unsigned char *componentName)
{
  unsigned long hashedNam = hash(componentName);

  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i)
  {
    Component *c = entity->components[i];
    if (c->hash == hashedNam)
      return c->value;
  }

  return NULL;
}

void addEntity(EntityManager *entityManager)
{
  Entity *entity = SDL_malloc(sizeof(Entity));
  entity->id = entityManager->totalEntities;
  entity->totalComponents = 0;
  entity->components = NULL;


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
  increaseArray(&entityManager->entities, sizeof(Entity *), &entityManager->totalEntities);
#pragma clang diagnostic pop

  entityManager->entities[entity->id] = entity;
}

void spawnEntity(EntityManager *em, bool addComponents)
{
  addEntity(em);
  // every 10th entity add a position component
  if (addComponents)
  {
    Entity *e = em->entities[em->totalEntities - 1];

    Position *pos = SDL_malloc(sizeof(Position));
    pos->x = randomClamped(1, 1280);
    pos->y = randomClamped(1, 720);

    Component *c = SDL_malloc(sizeof(Component));
    c->hash = hash("Position");
    c->value = pos;

    addComponent(e, c);

    Velocity *vel = SDL_malloc(sizeof(Velocity));
    vel->x = randomClamped(5, 20);
    vel->y = randomClamped(5, 20);

    Component *c2 = SDL_malloc(sizeof(Component));
    c2->hash = hash("Velocity");
    c2->value = vel;

    addComponent(e, c2);
  }
}
