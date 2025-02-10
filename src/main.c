#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define STEP_RATE_IN_MILLISECONDS 17

#pragma region Components
typedef struct Vec2
{
  float x;
  float y;
} Vec2;
typedef Vec2 Position;
typedef Vec2 Velocity;

typedef struct Shape
{
  float radius;
  int pointCount;
} Shape;
#pragma endregion

typedef enum ComponentType
{
  CVEC2 = 1,
  CPOSITION = 1 << 1,
  CVELOCITY = 1 << 2,
  CSHAPE = 1 << 3
} ComponentType;

typedef struct Component
{
  ComponentType type;
  union
  {
    Position *pos;
    Velocity *velocity;
    Shape *shape;
  };
} Component;

typedef struct Entity
{
  int id;
  int totalComponents;
  int archetype;
  Component **components;
} Entity;

typedef struct EntityManager
{
  int totalEntities;
  Entity **entities;
} EntityManager;

typedef struct AppState
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  EntityManager entityManager;
  int addEntityEachNFrames;
  int remainingFrames;
  Uint64 last_step;
} AppState;

#pragma region Declarations
void printEntities(EntityManager *em);
void addComponent(Entity *entity, Component *c);
// void spawnEntity(EntityManager *em);
void spawnEntity(EntityManager *em, bool addComponents);
void moveSystem(EntityManager *em);
Component *findComponent(Entity *entity, ComponentType componentType);
void renderRectSystem(EntityManager *em, AppState *as);
#pragma endregion

void increaseArray(void **array, size_t elementSize, int *currentSize)
{
  *currentSize += 1;
  *array = SDL_realloc(*array, (*currentSize) * elementSize);
  if (*array == NULL)
  {
    SDL_Log("Failed to reallocate memory");
  }
}

void addEntity(EntityManager *entityManager)
{
  Entity *entity = SDL_malloc(sizeof(Entity));
  entity->id = entityManager->totalEntities;
  entity->totalComponents = 0;
  entity->components = NULL;
  entity->archetype = 0;

  increaseArray(&entityManager->entities, sizeof(Entity *), &entityManager->totalEntities);

  SDL_Log("Adding entity with ID %d - C(%d) - %p\n", entity->id, entity->totalComponents, entity);
  entityManager->entities[entity->id] = entity;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
    return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;

  *appstate = as;

  // spawn a single entity
  spawnEntity(&as->entityManager, true);

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", 1280, 720, 0, &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

SDL_AppResult handle_key_event(SDL_Scancode key_code)
{
  switch (key_code)
  {
  case SDL_SCANCODE_Q:
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  switch (event->type)
  {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    return handle_key_event(event->key.scancode);
  }
  return SDL_APP_CONTINUE;
}

void printEntities(EntityManager *em)
{
  if (em->totalEntities <= 0)
    return;
  printf("------\n");
  for (int i = 0; i < em->totalEntities; i++)
  {
    Entity *e = em->entities[i];
    printf("Entity ID(%d) - C(%d) - addr(%p) - archetype(%d)\n", e->id, e->totalComponents, e, e->archetype);

    for (int j = 0; j < e->totalComponents; ++j)
    {
      Component *c = e->components[j];
      printf("Component type %d\n", c->type);
      if (c->type == CPOSITION)
      {
        c->pos->x++;
        printf("Entity position (%f, %f)\n", c->pos->x, c->pos->y);
        /*Component *anotherC = e->componentsMap[CPOSITION].value;*/
        /*printf("Entity position from map (%f, %f)\n", anotherC->pos.x, anotherC->pos.y);*/
      }
    }
  }
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  AppState *as = (AppState *)appstate;
  const Uint64 now = SDL_GetTicks();

  while ((now - as->last_step) >= STEP_RATE_IN_MILLISECONDS)
  {
    moveSystem(&as->entityManager);
    as->last_step += STEP_RATE_IN_MILLISECONDS;
  }

  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
  SDL_RenderClear(as->renderer);

  renderRectSystem(&as->entityManager, as);

  SDL_RenderPresent(as->renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  if (appstate != NULL)
  {
    AppState *as = (AppState *)appstate;
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    SDL_free(as);
  }
}

void addComponent(Entity *entity, Component *c)
{
  increaseArray(&entity->components, sizeof(Component *), &entity->totalComponents);
  entity->components[entity->totalComponents - 1] = c;
  entity->archetype |= c->type;
}

void spawnEntity(EntityManager *em, bool addComponents)
{
  if (em->totalEntities >= 10)
    return;

  addEntity(em);
  // every 10th entity add a position component
  if (addComponents)
  {
    Entity *e = em->entities[em->totalEntities - 1];

    Position *pos = SDL_malloc(sizeof(Position));
    pos->x = 10.0;
    pos->y = 20.0;

    Component *c = SDL_malloc(sizeof(Component));
    c->type = CPOSITION;
    c->pos = pos;

    addComponent(e, c);

    Velocity *vel = SDL_malloc(sizeof(Velocity));
    vel->x = 1.0;
    vel->y = 0.0;

    Component *c2 = SDL_malloc(sizeof(Component));
    c2->type = CVELOCITY;
    c2->velocity = vel;

    addComponent(e, c2);
  }
}

void moveSystem(EntityManager *em)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];
    unsigned int toFind = CPOSITION | CVELOCITY;

    if (e->totalComponents <= 0)
      continue;

    if (!((e->archetype & toFind) == toFind))
      continue;

    Component *c = findComponent(e, CPOSITION);
    Position *pos = c->pos;
    c = findComponent(e, CVELOCITY);
    Velocity *vel = c->velocity;

    if (!pos)
    {
      SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Position not found\n");
      continue;
    }
    if (!vel)
    {
      SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Velocity not found\n");
      continue;
    }

    pos->x += vel->x;
    pos->y += vel->y;

    // SDL_Log("Entity ID (%d) current position (%f, %f)", e->id, pos->x, pos->y);
  }
}

void renderRectSystem(EntityManager *em, AppState *as)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];
    unsigned int toFind = CPOSITION | CVELOCITY;

    if (e->totalComponents <= 0)
      continue;

    if (!((e->archetype & toFind) == toFind))
      continue;

    Component *c = findComponent(e, CPOSITION);
    const Position *pos = c->pos;
    c = findComponent(e, CVELOCITY);
    const Velocity *vel = c->velocity;

    if (!pos)
    {
      SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Position not found\n");
      continue;
    }
    if (!vel)
    {
      SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Velocity not found\n");
      continue;
    }

    SDL_FRect r = {
        .x = pos->x,
        .y = pos->y,
        .w = 50.0,
        .h = 50.0,
    };
    SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, 255);
    SDL_RenderRect(as->renderer, &r);

    // SDL_Log("Rendering rect at (%f, %f)", pos->x, pos->y);
  }
}

Component *findComponent(Entity *entity, ComponentType componentType)
{
  // TODO: improve me, it currently perform just a linear search
  for (int i = 0; i < entity->totalComponents; ++i)
  {
    Component *c = entity->components[i];
    if (c->type == componentType)
      return c;
  }

  return NULL;
}