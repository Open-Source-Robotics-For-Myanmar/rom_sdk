#include "rom/robotics/control/pid/PidController.h"

#include <algorithm>

namespace rom::robotics::control::pid {

namespace {

// Clamps `value` to [low, high]. With the default sentinel bounds this
// returns `value` unchanged, so an unconfigured controller stays unbounded.
double Clamp(double value, double low, double high) {
  return std::min(std::max(value, low), high);
}

}  // namespace

double PidController::Update(double error, double dt_s) {
  if (dt_s <= 0.0) {
    return 0.0;
  }

  integral_ += error * dt_s;
  integral_ = Clamp(integral_, limits_.integral_min, limits_.integral_max);

  double derivative = 0.0;
  if (has_previous_error_) {
    derivative = (error - previous_error_) / dt_s;
  }
  previous_error_ = error;
  has_previous_error_ = true;

  const double output =
      gains_.kp * error + gains_.ki * integral_ + gains_.kd * derivative;
  return Clamp(output, limits_.output_min, limits_.output_max);
}

void PidController::Reset() {
  integral_ = 0.0;
  previous_error_ = 0.0;
  has_previous_error_ = false;
}

}  // namespace rom::robotics::control::pid
