#include "rom/robotics/guidance/png/PngGuidance.h"

#include <algorithm>
#include <cmath>

namespace rom::robotics::guidance::png {

namespace {

PngGuidance::Vector3 Subtract(const PngGuidance::Vector3& a,
                              const PngGuidance::Vector3& b) {
  return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

double Dot(const PngGuidance::Vector3& a, const PngGuidance::Vector3& b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

PngGuidance::Vector3 Cross(const PngGuidance::Vector3& a,
                           const PngGuidance::Vector3& b) {
  return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
          a[0] * b[1] - a[1] * b[0]};
}

double Norm(const PngGuidance::Vector3& a) { return std::sqrt(Dot(a, a)); }

}  // namespace

PngGuidance::PngGuidance(const Params& params) : params_(params) {}

// True Proportional Navigation: a = N * Vc * omega, where omega is the
// line-of-sight (LOS) rotation rate vector and Vc is the closing speed.
PngGuidance::Vector3 PngGuidance::ComputeAcceleration(
    const State& pursuer, const State& target) const {
  const Vector3 relative_position = Subtract(target.position_m, pursuer.position_m);
  const Vector3 relative_velocity = Subtract(target.velocity_m_s, pursuer.velocity_m_s);

  const double range_sq = Dot(relative_position, relative_position);
  if (range_sq <= 0.0) {
    return Vector3{};
  }

  // LOS angular rate: omega = (r x v) / |r|^2.
  const Vector3 omega = [&] {
    const Vector3 r_cross_v = Cross(relative_position, relative_velocity);
    return Vector3{r_cross_v[0] / range_sq, r_cross_v[1] / range_sq,
                   r_cross_v[2] / range_sq};
  }();

  // Closing speed Vc = -d(range)/dt = -(r . v) / |r|.
  const double closing_speed = -Dot(relative_position, relative_velocity) / std::sqrt(range_sq);

  // a = N * Vc * (omega x los_unit); here omega already encodes the LOS rate.
  const Vector3 los_unit = {relative_position[0] / std::sqrt(range_sq),
                            relative_position[1] / std::sqrt(range_sq),
                            relative_position[2] / std::sqrt(range_sq)};
  const Vector3 omega_cross_los = Cross(omega, los_unit);

  const double scale = params_.navigation_constant * closing_speed;
  Vector3 acceleration = {scale * omega_cross_los[0], scale * omega_cross_los[1],
                          scale * omega_cross_los[2]};

  const double magnitude = Norm(acceleration);
  if (magnitude > params_.max_acceleration_m_s2 && magnitude > 0.0) {
    const double factor = params_.max_acceleration_m_s2 / magnitude;
    acceleration = {acceleration[0] * factor, acceleration[1] * factor,
                    acceleration[2] * factor};
  }
  return acceleration;
}

}  // namespace rom::robotics::guidance::png
