#include "rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h"

#include <array>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

namespace rom::intelligence::model::rl::bc_mlp_v4_ppo
{
  namespace
  {

    TEST(BcMlpV4PpoTest, ForwardZeroObservationProducesExpectedPhysicalActions)
    {
      const std::array<float, kInputDim> zero_obs{};
      const auto out = ForwardPhysical(zero_obs);

      EXPECT_TRUE(std::isfinite(out[0]));
      EXPECT_TRUE(std::isfinite(out[1]));
      EXPECT_TRUE(std::isfinite(out[2]));
      EXPECT_TRUE(std::isfinite(out[3]));

      const auto out_snake = forward_physical(zero_obs);
      for (std::size_t i = 0; i < kOutputDim; ++i)
      {
        EXPECT_NEAR(out[i], out_snake[i], 1e-6f);
      }
    }

    TEST(BcMlpV4PpoTest, CausalFeatureObserverRequiresFourSamples)
    {
      CausalFeatureObserver observer;
      std::array<float, kInputDim> features{};

      EXPECT_FALSE(observer.Update(
          0.00, true, 10.0, -5.0, 0.1, 0.2, 120.0, 2.0, 1.0, -0.5, features));
      EXPECT_FALSE(observer.Update(
          0.02, true, 9.0, -4.0, 0.11, 0.21, 121.0, 2.0, 1.0, -0.5, features));
      EXPECT_FALSE(observer.Update(
          0.04, true, 8.0, -3.0, 0.12, 0.22, 122.0, 2.0, 1.0, -0.5, features));
      EXPECT_TRUE(observer.Update(
          0.06, true, 7.0, -2.0, 0.13, 0.23, 123.0, 2.0, 1.0, -0.5, features));

      EXPECT_NEAR(features[0], static_cast<float>(7.0 / kCx), 1e-6f);
      EXPECT_NEAR(features[1], static_cast<float>(-2.0 / kCy), 1e-6f);
      EXPECT_NEAR(features[9], static_cast<float>(std::sin(0.23)), 1e-6f);
      EXPECT_NEAR(features[10], static_cast<float>(std::cos(0.23)), 1e-6f);
    }

    TEST(BcMlpV4PpoTest, InvalidSampleOrLargeStepDtResetsHistory)
    {
      CausalFeatureObserver observer;
      std::array<float, kInputDim> features{};

      EXPECT_FALSE(observer.Update(
          0.00, true, 0.0, 0.0, 0.0, 0.0, 100.0, 1.0, 0.0, 0.0, features));
      EXPECT_FALSE(observer.Update(
          0.02, true, 0.0, 0.0, 0.0, 0.0, 100.0, 1.0, 0.0, 0.0, features));
      EXPECT_FALSE(observer.Update(
          0.04, false, 0.0, 0.0, 0.0, 0.0, 100.0, 1.0, 0.0, 0.0, features));
      EXPECT_FALSE(observer.Update(
          0.06, true, 0.0, 0.0, 0.0, 0.0, 100.0, 1.0, 0.0, 0.0, features));
    }

    TEST(BcMlpV4PpoTest, LimitPhysicalActionClampsSpeedVzAndYawRate)
    {
      const std::array<float, kOutputDim> raw{6.0f, 8.0f, -5.0f, 3.0f};
      const auto limited = LimitPhysicalAction(raw, 5.0, 3.0, 1.8);

      EXPECT_NEAR(limited[0], 3.0f, 1e-5f);
      EXPECT_NEAR(limited[1], 4.0f, 1e-5f);
      EXPECT_NEAR(limited[2], -3.0f, 1e-5f);
      EXPECT_NEAR(limited[3], 1.8f, 1e-5f);

      const std::array<float, kOutputDim> non_finite{
          std::numeric_limits<float>::quiet_NaN(), 1.0f, 1.0f, 1.0f};
      const auto zeroed = LimitPhysicalAction(non_finite);
      for (float v : zeroed)
      {
        EXPECT_NEAR(v, 0.0f, 1e-6f);
      }
    }

    TEST(BcMlpV4PpoTest, StepRunsObserverForwardAndLimiter)
    {
      BcMlpV4Ppo model;
      BcMlpV4Ppo::ActionVector action{};

      EXPECT_FALSE(model.Step(
          0.00, true, 12.0, -6.0, 0.05, 0.10, 110.0, 3.0, 1.0, -0.2, action));
      EXPECT_FALSE(model.Step(
          0.02, true, 10.0, -5.0, 0.06, 0.11, 112.0, 3.0, 1.0, -0.2, action));
      EXPECT_FALSE(model.Step(
          0.04, true, 8.0, -4.0, 0.07, 0.12, 114.0, 3.0, 1.0, -0.2, action));
      EXPECT_TRUE(model.Step(
          0.06, true, 6.0, -3.0, 0.08, 0.13, 116.0, 3.0, 1.0, -0.2, action));

      const double horiz_speed = std::hypot(action[0], action[1]);
      EXPECT_LE(horiz_speed, model.limits().speed_limit + 1e-5);
      EXPECT_LE(std::abs(action[2]), model.limits().vz_limit + 1e-5);
      EXPECT_LE(std::abs(action[3]), model.limits().yaw_rate_limit + 1e-5);
    }

  }  // namespace
}  // namespace rom::intelligence::model::rl::bc_mlp_v4_ppo
