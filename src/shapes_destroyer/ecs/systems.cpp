#include "systems.h"
#include "../game_engine/game_engine.h"
#include <math.h>

#include "../constants.h"
#include "../mymath.h"
#include "../types.h"
#include "entity.h"
#include "memory.h"

// NOTE: do we really want to use guard clause in the systems?
// I want them to break if I'm not passing something to it, if needed we can
// implement the checks later on

void generateAudio(system_params *params) {
  wayne_audio_buffer *Buffer = params->AudioBuffer;

  if (!Buffer) {
    // we don't have audio
    // TODO: diagnostic
    return;
  }

  int SampleCount = Buffer->BufferSize / Buffer->BytesPerSample;
  float *SampleOut = (float *)Buffer->Data;
  local_persist float tSine;

  for (int i = 0; i < SampleCount; i++) {
    float SineValue = sinf(tSine);

    float SampleValue = (float)(SineValue + Buffer->ToneVolume);

    *SampleOut++ = SampleValue;
    *SampleOut++ = SampleValue;

    tSine += 2.0f * PI * 1.0f / (float)Buffer->WavePeriod;
  }
}

void spawnEntities(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->entityManager)
    return;
#endif

  entity_manager *em = params->entityManager;

  // for now the first entity is always the player and the other are all "ENEMY"
  // even if the currently do nothing
  for (int i = 0; i < 30; i++) {
    spawnEntity(em, true);
  }
}

void renderWeirdGradient(system_params *params) {
  game_offscreen_buffer *Buffer = params->backBuffer;

  local_persist uint16 BlueOffset = 0;
  local_persist uint16 GreenOffset = 0;

  uint8 MaxAccelleration = 5;

  // for now loop through all the controllers in order to see the platform code
  // works
  for (int ControllerIndex = 0; ControllerIndex < MAX_N_CONTROLLERS;
       ControllerIndex++) {
    wayne_controller_input Controller =
        Wayne_getController((wayne_t *)params->GameEngine, ControllerIndex);

    if (!Controller.IsActive)
      continue;

    GreenOffset += MaxAccelleration * Controller.StickX;
    BlueOffset += MaxAccelleration * Controller.StickY;

    if (Controller.ButtonSouth.isDown || Controller.MoveDown.isDown)
      GreenOffset += 2;

    if (Controller.ButtonNorth.isDown || Controller.MoveUp.isDown)
      GreenOffset -= 2;

    if (Controller.ButtonEast.isDown || Controller.MoveRight.isDown)
      BlueOffset += 2;

    if (Controller.ButtonWest.isDown || Controller.MoveLeft.isDown)
      BlueOffset -= 2;
  }

  uint8 *Row = (uint8 *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer->Width; ++X) {
      uint8 Blue = (X + BlueOffset);
      uint8 Green = (Y + GreenOffset);
      /*uint8 Blue = 0;*/
      /*uint8 Green = 0;*/
      uint8 Red = 0;
      uint8 Alpha = 255;

      *Pixel++ = ((Alpha << 24) | ((Red << 16) | ((Green << 8) | Blue)));
    }

    Row += Buffer->Pitch;
  }
}

void renderPlayerSystem(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->backBuffer)
    return;
  if (!params->entityManager)
    return;
#endif

  entity_manager *em = params->entityManager;
  game_offscreen_buffer *backBuffer = params->backBuffer;

  entity *e = getPlayer(em);
  if (!e)
    return;

  if (e->totalComponents <= 0)
    return;

  const Position *pos = getComponentValue(em, e, "Position", Position);
  if (!pos)
    return;

  const Velocity *vel = getComponentValue(em, e, "Velocity", Velocity);
  if (!vel)
    return;

  const Shape *shape = getComponentValue(em, e, "Shape", Shape);
  if (!shape)
    return;

  const Color *color = getComponentValue(em, e, "Color", Color);
  if (!color)
    return;

  // TODO: render the player as a red rectangle by manually setting the pixel
  // data for it
}

void renderShapeSystem(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->backBuffer)
    return;
  if (!params->tempArena)
    return;
  if (!params->entityManager)
    return;
#endif

  game_offscreen_buffer *backBuffer = params->backBuffer;

  entity_manager *em = params->entityManager;
  int current_position = 0;
  entity *e = 0;

  while (setEntityByTag(&e, em, ENEMY, current_position)) {
    // we already set the e variable we can increase the position immediately
    // and do not forget about it later
    current_position++;

    if (e->totalComponents <= 0)
      continue;

    const Position *pos = getComponentValue(em, e, "Position", Position);
    if (!pos)
      continue;

    const Velocity *vel = getComponentValue(em, e, "Velocity", Velocity);
    if (!vel)
      continue;

    const Shape *shape = getComponentValue(em, e, "Shape", Shape);
    if (!shape)
      continue;

    const Color *color = getComponentValue(em, e, "Color", Color);
    if (!color)
      continue;

    float unitAngle = 360.0f / shape->pointCount;

    vec2 *points =
        pushSizeTimes(params->tempArena, vec2, shape->pointCount + 1);
    int pointsIndex = 0;

    // TODO: probably this whole loop can be moved into a function, it could
    // be useful at least for this game to have a function that given an angle
    // and a center returns a list of vertices where to draw the lines
    float currentAngle = unitAngle;
    vec2 *prev = NULL;

    while (currentAngle <= 360.0f) {
      points[pointsIndex] = (vec2){};
      vec2 *current = &points[pointsIndex];

      { // angle vector goes out of scope immediately, find a better way?
        vec2 angleVector = vec2FromAngle(currentAngle);
        current->x = angleVector.x;
        current->y = angleVector.y;
      }

      extendVec2(current, shape->radius);
      addToVec2(current, pos);

      prev = current;
      currentAngle += unitAngle;
      pointsIndex++;
    }
    points[pointsIndex] = points[0];

    // TODO: render the lines with the righ color by setting the pixels in the
    // back buffer backBuffer
  }
}

void moveSystem(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->entityManager)
    return;
#endif

  entity_manager *em = params->entityManager;
  int current_position = 0;
  entity *e = 0;

  while (setEntityByTag(&e, em, ALL, current_position)) {
    current_position++;

    if (e->totalComponents <= 0)
      continue;

    Position *pos = getComponentValue(em, e, "Position", Position);
    if (!pos)
      continue;

    const Position *vel = getComponentValue(em, e, "Velocity", Velocity);
    if (!vel)
      continue;

    pos->x += vel->x;
    pos->y += vel->y;
  }
}

void keepInBoundsSystem(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->entityManger)
    return;
#endif

  entity_manager *em = params->entityManager;
  if (!em)
    return;

  for (int i = 0; i < em->totalEntities; ++i) {
    entity *e = getEntity(em, i);

    if (e->totalComponents <= 0)
      continue;

    Position *pos = getComponentValue(em, e, "Position", Position);
    if (!pos)
      continue;

    Velocity *vel = getComponentValue(em, e, "Velocity", Velocity);
    if (!vel)
      continue;

    if (pos->x > SCREEN_WIDTH || pos->x < 0)
      vel->x *= -1;

    if (pos->y > SCREEN_HEIGHT || pos->y < 0)
      vel->y *= -1;
  }
}

void handlePlayerInput(system_params *params) {
#if SYSTEM_GUARDS
  if (!params->entityManager)
    return;
#endif

  entity_manager *em = params->entityManager;

  entity *player = getPlayer(em);
  if (!player)
    return;

  Velocity *vel = getComponentValue(em, player, "Velocity", Velocity);
  if (!vel)
    return;

  float acceleration = 5.0;

  u32 count = 0;
  // TODO: loop through the Action entities and apply only the one that belongs
  // to the current player
#if 0
  while (current) {
    if (!current->value)
      continue;
    /*action *currentAction = getNodeValue(current, action);*/
    action *currentAction = (action *)current->value;

    switch (currentAction->kind) {
    case ACTION_UP:
      if (currentAction->state == ACTION_STATE_START)
        vel->y -= acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->y += acceleration;

      break;

    case ACTION_DOWN:
      if (currentAction->state == ACTION_STATE_START)
        vel->y += acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->y -= acceleration;

      break;

    case ACTION_LEFT:
      if (currentAction->state == ACTION_STATE_START)
        vel->x -= acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->x += acceleration;

      break;

    case ACTION_RIGHT:
      if (currentAction->state == ACTION_STATE_START)
        vel->x += acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->x -= acceleration;

      break;

    default:
      break;
    }

    current = popFromLinkedList(actionQueue);
  }
#endif
}
