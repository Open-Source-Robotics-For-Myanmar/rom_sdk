#include "rom/robotics/control/pid/PidController.h"

#include <gtest/gtest.h>

namespace rom::robotics::control::pid {
namespace {

TEST(PidControllerTest, DefaultsToZeroGainsAndNoLimits) {
  PidController pid;
  EXPECT_EQ(pid.kp(), 0.0);
  EXPECT_EQ(pid.ki(), 0.0);
  EXPECT_EQ(pid.kd(), 0.0);
  EXPECT_EQ(pid.limits().output_min, PidController::kNoLowerLimit);
  EXPECT_EQ(pid.limits().output_max, PidController::kNoUpperLimit);
  EXPECT_NEAR(pid.Update(1.0, 0.1), 0.0, 1e-9);
}

TEST(PidControllerTest, ProportionalTermScalesError) {
  PidController pid;
  pid.set_kp(2.0);
  EXPECT_NEAR(pid.Update(3.0, 0.1), 6.0, 1e-9);
}

TEST(PidControllerTest, IntegralAccumulatesOverTime) {
  PidController pid;
  pid.set_ki(1.0);
  EXPECT_NEAR(pid.Update(2.0, 0.5), 1.0, 1e-9);  // integral = 2 * 0.5
  EXPECT_NEAR(pid.Update(2.0, 0.5), 2.0, 1e-9);  // integral = 1 + 1
}

TEST(PidControllerTest, DerivativeRespondsToErrorChange) {
  PidController pid;
  pid.set_kd(1.0);
  EXPECT_NEAR(pid.Update(0.0, 0.1), 0.0, 1e-9);   // no previous error yet
  EXPECT_NEAR(pid.Update(1.0, 0.1), 10.0, 1e-9);  // (1 - 0) / 0.1
}

TEST(PidControllerTest, OutputIsSaturated) {
  PidController pid;
  pid.set_kp(100.0);
  pid.set_output_min(-5.0);
  pid.set_output_max(5.0);
  EXPECT_NEAR(pid.Update(1.0, 0.1), 5.0, 1e-9);
  EXPECT_NEAR(pid.Update(-1.0, 0.1), -5.0, 1e-9);
}

TEST(PidControllerTest, IntegralIsClamped) {
  PidController pid;
  pid.set_ki(1.0);
  pid.set_integral_max(1.0);
  pid.Update(10.0, 1.0);  // integral would be 10, clamped to 1
  EXPECT_NEAR(pid.integral(), 1.0, 1e-9);
}

TEST(PidControllerTest, NonPositiveDtReturnsZero) {
  PidController pid;
  pid.set_kp(1.0);
  EXPECT_EQ(pid.Update(5.0, 0.0), 0.0);
  EXPECT_EQ(pid.Update(5.0, -0.1), 0.0);
}

TEST(PidControllerTest, ResetClearsHistory) {
  PidController pid;
  pid.set_ki(1.0);
  pid.Update(2.0, 1.0);
  EXPECT_NEAR(pid.integral(), 2.0, 1e-9);
  pid.Reset();
  EXPECT_EQ(pid.integral(), 0.0);
  EXPECT_EQ(pid.previous_error(), 0.0);
}

}  // namespace
}  // namespace rom::robotics::control::pid
