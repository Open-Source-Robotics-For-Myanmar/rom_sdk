#ifndef ROM_ROBOTICS_CONTROL_PID_PIDCONTROLLER_H_
#define ROM_ROBOTICS_CONTROL_PID_PIDCONTROLLER_H_

#include <limits>
#include <array>

namespace rom::robotics::control::pid 
{

  // Discrete-time PID controller.
  // Not thread-safe; use one instance per control loop.
  class PidController 
  {
  public:
    static constexpr double kNoLowerLimit = std::numeric_limits<double>::lowest();
    static constexpr double kNoUpperLimit = std::numeric_limits<double>::max();

    struct Gains 
    {
      double kp = 0.0;
      double ki = 0.0;
      double kd = 0.0;
    };

    struct Limits 
    {
      double output_min = kNoLowerLimit;
      double output_max = kNoUpperLimit;
      double integral_min = kNoLowerLimit;
      double integral_max = kNoUpperLimit;
    };

    // Starts with zero gains and no limits. Configure with the setters.
    PidController() = default;

    // Returns the control output for `error`. `dt_s` must be > 0.
    double Update(double error, double dt_s);

    // Clears the integral and derivative history.
    void Reset();

    double kp() const { return gains_.kp; }
    void set_kp(double kp) { gains_.kp = kp; }

    double ki() const { return gains_.ki; }
    void set_ki(double ki) { gains_.ki = ki; }

    double kd() const { return gains_.kd; }
    void set_kd(double kd) { gains_.kd = kd; }

    std::array<double, 3> gains() const { return {gains_.kp, gains_.ki, gains_.kd}; }
    void set_gains(double kp, double ki, double kd) {
      gains_.kp = kp;
      gains_.ki = ki;
      gains_.kd = kd;
    }

    void set_output_min(double output_min) { limits_.output_min = output_min; }
    void set_output_max(double output_max) { limits_.output_max = output_max; }

    void set_integral_min(double integral_min) { limits_.integral_min = integral_min; }
    void set_integral_max(double integral_max) { limits_.integral_max = integral_max; }

    const Limits& limits() const { return limits_; }
    void set_limits(const Limits& limits) { limits_ = limits; }

    double integral() const { return integral_; }
    double previous_error() const { return previous_error_; }

  private:
    Gains gains_;
    Limits limits_;
    double integral_ = 0.0;
    double previous_error_ = 0.0;
    bool has_previous_error_ = false;
  };

}  // namespace rom::robotics::control::pid

#endif  // ROM_ROBOTICS_CONTROL_PID_PIDCONTROLLER_H_
