#include "rom/robotics/guidance/png/PngGuidance.h"

#include <array>
#include <cmath>

#include <gtest/gtest.h>

namespace rom::robotics::guidance::png {
namespace {

double Norm(const PngGuidance::Vector3& v) {
  return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

TEST(PngGuidanceTest, DefaultParams) {
  PngGuidance png;
  EXPECT_EQ(png.navigation_constant(), 3.0);
  EXPECT_EQ(png.max_acceleration_m_s2(), 30.0);
}

TEST(PngGuidanceTest, Setters) {
  PngGuidance png;
  png.set_navigation_constant(4.0);
  png.set_max_acceleration_m_s2(50.0);
  EXPECT_EQ(png.navigation_constant(), 4.0);
  EXPECT_EQ(png.max_acceleration_m_s2(), 50.0);
}

TEST(PngGuidanceTest, HeadOnClosingHasNoLateralAcceleration) {
  PngGuidance png;
  // Pursuer flies +x toward a target directly ahead moving -x. LOS does not
  // rotate, so the acceleration command is ~zero.
  PngGuidance::State pursuer{{0.0, 0.0, 0.0}, {10.0, 0.0, 0.0}};
  PngGuidance::State target{{100.0, 0.0, 0.0}, {-5.0, 0.0, 0.0}};
  const PngGuidance::Vector3 a = png.ComputeAcceleration(pursuer, target);
  EXPECT_NEAR(Norm(a), 0.0, 1e-9);
}

TEST(PngGuidanceTest, CrossingTargetProducesLateralAcceleration) {
  PngGuidance png;
  PngGuidance::State pursuer{{0.0, 0.0, 0.0}, {10.0, 0.0, 0.0}};
  PngGuidance::State target{{100.0, 0.0, 0.0}, {0.0, 5.0, 0.0}};
  const PngGuidance::Vector3 a = png.ComputeAcceleration(pursuer, target);
  EXPECT_GT(Norm(a), 0.0);
}

TEST(PngGuidanceTest, AccelerationIsSaturated) {
  PngGuidance png;
  png.set_navigation_constant(50.0);
  png.set_max_acceleration_m_s2(10.0);
  PngGuidance::State pursuer{{0.0, 0.0, 0.0}, {100.0, 0.0, 0.0}};
  PngGuidance::State target{{50.0, 0.0, 0.0}, {0.0, 80.0, 0.0}};
  const PngGuidance::Vector3 a = png.ComputeAcceleration(pursuer, target);
  EXPECT_LE(Norm(a), 10.0 + 1e-6);
}

TEST(PngGuidanceTest, ZeroRangeReturnsZero) {
  PngGuidance png;
  PngGuidance::State pursuer{{1.0, 2.0, 3.0}, {1.0, 0.0, 0.0}};
  PngGuidance::State target{{1.0, 2.0, 3.0}, {0.0, 1.0, 0.0}};
  const PngGuidance::Vector3 a = png.ComputeAcceleration(pursuer, target);
  EXPECT_NEAR(Norm(a), 0.0, 1e-9);
}

}  // namespace
}  // namespace rom::robotics::guidance::png
