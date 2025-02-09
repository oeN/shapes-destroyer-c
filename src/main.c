#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#pragma region Components
typedef struct Vec2 {
  float x;
  float y;
} Vec2;

typedef struct Shape {
  float radius;
  int pointCount;
} Shape;
#pragma endregion

typedef struct Entity {
  int id;
  void* components;
} Entity;

typedef struct EntityManager {
  int totalEntities;
  Entity * entities;
} EntityManager;

typedef struct AppState {
  SDL_Window *window;
  SDL_Renderer *renderer;
  EntityManager* entityManager;
} AppState;

void addEntity(EntityManager *entityManager)
{
  Entity *entity = SDL_calloc(1, sizeof(Entity));
  entity->id = entityManager->totalEntities;
  printf("Adding entity with ID %d - %p\n", entity->id, &entity);
  ++entityManager->totalEntities;
  SDL_realloc(entityManager->entities, entityManager->totalEntities);
  entityManager->entities[entity->id] = *entity;
}

void initEntityManager(EntityManager **em)
{
  EntityManager *_em = SDL_calloc(1, sizeof(EntityManager));
  _em->totalEntities = 0;
  _em->entities = (Entity *)SDL_calloc(0, sizeof(Entity*));
  *em = _em;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as) return SDL_APP_FAILURE;

  *appstate = as;
  initEntityManager(&as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);
  addEntity(as->entityManager);

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", 1280, 720, 0, &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

SDL_AppResult handle_key_event(SDL_Scancode key_code)
{
  switch(key_code){
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
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    return handle_key_event(event->key.scancode);
  }
  return SDL_APP_CONTINUE;
}

void printEntities(EntityManager *em)
{
  printf("------\n");
  for (int i =0; i < em->totalEntities; i++) {
    printf("Entity %d - %p\n", em->entities[i].id, &em->entities[i]);
  }
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  AppState * as = (AppState *)appstate;

  printEntities(as->entityManager);
  
  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
  SDL_RenderClear(as->renderer);
  SDL_RenderPresent(as->renderer);
  return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  if (appstate != NULL){
    AppState *as = (AppState *)appstate;
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    SDL_free(as);
  }
}

