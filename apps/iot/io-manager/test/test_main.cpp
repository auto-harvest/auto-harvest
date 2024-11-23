#include <Arduino.h>
#include <unity.h>
#include 
// Function to be tested
int add(int a, int b) {
    return a + b;
}

void test_addition() {
    TEST_ASSERT_EQUAL(2, add(1, 1));
    TEST_ASSERT_EQUAL(0, add(-1, 1));
    TEST_ASSERT_EQUAL(-2, add(-1, -1));
}

void setup() {
    // NOTE!!! Wait for >2 secs if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // Start Unity test framework

    RUN_TEST(test_addition);

    UNITY_END(); // Stop Unity test framework
}

void loop() {
    // Empty loop
}