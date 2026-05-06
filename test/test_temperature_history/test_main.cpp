#include "TemperatureHistory.h"
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_empty_history_returns_zero_temp() {
  TemperatureHistory history;
  TEST_ASSERT_EQUAL_FLOAT(0.0f, history.GetCurrentTemp());
}

void test_empty_history_has_zero_count() {
  TemperatureHistory history;
  TEST_ASSERT_EQUAL(0, history.GetReportCount());
}

void test_report_increments_count() {
  TemperatureHistory history;
  history.Report(TemperatureReport(72.5f, 1000));
  TEST_ASSERT_EQUAL(1, history.GetReportCount());
}

void test_get_current_temp_returns_most_recent() {
  TemperatureHistory history;
  history.Report(TemperatureReport(1000, 72.5f));
  history.Report(TemperatureReport(2000, 80.0f));
  TEST_ASSERT_EQUAL_FLOAT(80.0f, history.GetCurrentTemp());
}

void test_get_report_by_index() {
  TemperatureHistory history;
  history.Report(TemperatureReport(1000, 72.5f));
  history.Report(TemperatureReport(2000, 80.0f));
  TEST_ASSERT_EQUAL_FLOAT(72.5f, history.GetReport(0).GetTemp());
  TEST_ASSERT_EQUAL_FLOAT(80.0f, history.GetReport(1).GetTemp());
}

void test_circular_buffer_fills() {
  TemperatureHistory history;
  for (int i = 0; i < 100; i++) {
    history.Report(TemperatureReport(i, (float)i));
  }
  // Buffer is full — adding one more should drop report 0
  history.Report(TemperatureReport(999, 999.0f));
  TEST_ASSERT_EQUAL(101, history.GetReportCount());
  TEST_ASSERT_EQUAL_FLOAT(1.0f, history.GetReport(0).GetTemp());
  TEST_ASSERT_EQUAL_FLOAT(999.0f, history.GetReport(100).GetTemp());
}

void test_circular_buffer_drops_oldest_when_full() {
  TemperatureHistory history;
  for (int i = 0; i < 500; i++) {
    history.Report(TemperatureReport(i, (float)i));
  }
  // Buffer is full — adding one more should drop report 0
  history.Report(TemperatureReport(999, 999.0f));
  TEST_ASSERT_EQUAL(500, history.GetReportCount());
  TEST_ASSERT_EQUAL_FLOAT(1.0f, history.GetReport(0).GetTemp());
  TEST_ASSERT_EQUAL_FLOAT(999.0f, history.GetReport(499).GetTemp());
}

void test_out_of_bounds_returns_default() {
  TemperatureHistory history;
  TemperatureReport report = history.GetReport(5);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, report.GetTemp());
  TEST_ASSERT_EQUAL(0, report.GetTimestamp());
  TEST_ASSERT_EQUAL(false, report.IsValid());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_empty_history_returns_zero_temp);
  RUN_TEST(test_empty_history_has_zero_count);
  RUN_TEST(test_report_increments_count);
  RUN_TEST(test_get_current_temp_returns_most_recent);
  RUN_TEST(test_get_report_by_index);
  RUN_TEST(test_circular_buffer_drops_oldest_when_full);
  RUN_TEST(test_out_of_bounds_returns_default);
  return UNITY_END();
}
