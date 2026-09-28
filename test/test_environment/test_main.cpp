#include <Arduino.h>
#include <unity.h>

void test_esp32_runtime_has_free_heap() {
  TEST_ASSERT_GREATER_THAN(0, ESP.getFreeHeap());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_esp32_runtime_has_free_heap);
  UNITY_END();
}

void loop() {
  if (Serial.available() == 0) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command == "PING") {
    Serial.println("PONG");
  }
}
