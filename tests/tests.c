#include <stdio.h>

#include "game_engine.h"
#include "systems.h"
#include "unity.h"

game_engine *ge;
int count = 0;

void setUp(void) { ge = initGameEngine(Megabytes(10)); }
void tearDown(void) { ge->destroy(ge); }

void fakeSystem(system_params *params) {
  count++;
  printf("fake system\n");
}

void test_add_a_system() {
  ge->addSystem(ge, UPDATE, fakeSystem);
  TEST_ASSERT_EQUAL(ge->systemsCount, 1);
}

void test_update_calls_the_systems() {
  ge->addSystem(ge, UPDATE, fakeSystem);
  ge->update(ge);
  TEST_ASSERT_EQUAL(count, 1);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_add_a_system);
  RUN_TEST(test_update_calls_the_systems);
  printf("Size of pointer %zu\n", sizeof(void *));
  printf("Size of u8 %zu\n", sizeof(u64));
  return UNITY_END();
}
