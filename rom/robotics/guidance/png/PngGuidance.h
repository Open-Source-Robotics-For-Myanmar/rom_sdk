#ifndef ROM_ROBOTICS_GUIDANCE_PNG_PNGGUIDANCE_H_
#define ROM_ROBOTICS_GUIDANCE_PNG_PNGGUIDANCE_H_

#include <array>

namespace rom::robotics::guidance::png {

// Proportional Navigation Guidance (PNG).
// a_cmd = N * Vc * LOS_rate
// Not thread-safe.
class PngGuidance {
 public:
  using Vector3 = std::array<double, 3>;

  // Kinematic state in a common inertial frame (e.g. NED).
  struct State {
    Vector3 position_m{};
    Vector3 velocity_m_s{};
  };

  struct Params {
    double navigation_constant = 3.0;  // N, typically 3 to 5.
    double max_acceleration_m_s2 = 30.0;
  };

  PngGuidance() = default;
  explicit PngGuidance(const Params& params);

  // Returns the lateral acceleration command for `pursuer` to hit `target`.
  Vector3 ComputeAcceleration(const State& pursuer, const State& target) const;

  const Params& params() const { return params_; }
  void set_params(const Params& params) { params_ = params; }

  double navigation_constant() const { return params_.navigation_constant; }
  void set_navigation_constant(double navigation_constant) {
    params_.navigation_constant = navigation_constant;
  }

  double max_acceleration_m_s2() const { return params_.max_acceleration_m_s2; }
  void set_max_acceleration_m_s2(double max_acceleration_m_s2) {
    params_.max_acceleration_m_s2 = max_acceleration_m_s2;
  }

 private:
  Params params_;
};

}  // namespace rom::robotics::guidance::png

#endif  // ROM_ROBOTICS_GUIDANCE_PNG_PNGGUIDANCE_H_
