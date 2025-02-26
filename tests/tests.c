#if 0
#include <stdio.h>

#include "game_engine.h"
#include "systems.h"
#include "unity.h"

game_engine *ge;
int count = 0;

// TODO: add more extensive tests and de-couple the SDL to test your code better

void setUp(void) { ge = bootstrapGameEngine(Megabytes(10)); }
void tearDown(void) { ge->destroy(ge); }

void fakeSystem(system_params *params) {
  count++;
  printf("fake system\n");
}

void test_add_a_system() {
  ge->addSystem(ge, GAME_ENGINE_UPDATE, fakeSystem);
  TEST_ASSERT_EQUAL(ge->systemsCount, 1);
}

void test_update_calls_the_systems() {
  ge->addSystem(ge, GAME_ENGINE_UPDATE, fakeSystem);
  ge->update(ge);
  TEST_ASSERT_EQUAL(count, 1);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_add_a_system);
  RUN_TEST(test_update_calls_the_systems);
  return UNITY_END();
}
#endif