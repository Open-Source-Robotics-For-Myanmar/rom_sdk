- bc_mlp_v2_debiased.hpp ကို rom::intellgience::model::rl::bc_mlp_v4_ppo အဖြစ် အသုံးပြုနိုင်သော script နမူနာရေးပြပါ။ ရေးပြီးရင် bc_mlp_v4_ppo.py  ရေးပြီး test လုပ်ပါ။ 
- sdk တစ်ခုလုံးကိုလည်း build လုပ်ကြည့်ပြီး rom::intellgience::model::rl::bc_mlp_v4_ppo အတွက် how to use ကို bc_mlp_v2_debiased.md မှာ ဆက်လက် ထည့်သွင်းရေးသားပါ။

# $\color{#00c853}{\text{BcMlpV4Ppo How to Use Guide}}$

`90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v2_debiased.hpp` ကို ROM SDK ၏ namespace `rom::intelligence::model::rl::bc_mlp_v4_ppo` အောက်ရှိ `BcMlpV4Ppo` class နှင့် `CausalFeatureObserver` အဖြစ် `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h` နှင့် `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.cc` တို့တွင် ထည့်သွင်းထားပါသည်။

## $\color{red}{\text{Architecture Flow}}$

```mermaid
flowchart LR
    A["Raw Sensor and Image Input"] --> B["CausalFeatureObserver::Update (4-sample window)"]
    B --> C["13D Normalized Feature Vector (float)"]
    C --> D["ForwardPhysical (13 -> 32 -> 16 -> 4 MLP)"]
    D --> E["LimitPhysicalAction (vx, vy, vz, yaw_rate)"]

    classDef inputCls fill:#1e88e5,stroke:#0d47a1,color:#ffffff;
    classDef procCls fill:#43a047,stroke:#1b5e20,color:#ffffff;
    classDef outCls fill:#fb8c00,stroke:#e65100,color:#ffffff;

    class A inputCls;
    class B,C,D procCls;
    class E outCls;
```

## $\color{red}{\text{SDK File Layout}}$

- Header: `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h`
- Source: `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.cc`
- Unit Test: `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4PpoTest.cc`
- Python Verification Script: `90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v4_ppo.py`
- CMake Target: `rom::intelligence` (`librom_intelligence.so`)

## $\color{red}{\text{CMake ဖြင့် ချိတ်ဆက်အသုံးပြုပုံ}}$

External project တွင် `find_package(rom)` ဖြင့် `rom::intelligence` နှင့် `rom::tools` ကို link လုပ်နိုင်ပါသည် -

```cmake
cmake_minimum_required(VERSION 3.16...3.28)
project(rl_tracker_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(rom 0.1 REQUIRED)

add_executable(rl_tracker_demo main.cc)
target_link_libraries(rl_tracker_demo PRIVATE rom::intelligence rom::tools)
```

Configure နှင့် build ပြုလုပ်ရန် command:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/rom
cmake --build build -j
```

## $\color{red}{\text{C++ အသုံးပြုပုံ နမူနာ}}$

### $\color{cyan}{\text{နည်းလမ်း ၁ - BcMlpV4Ppo Class ဖြင့် တိုက်ရိုက်သုံးခြင်း}}$

`BcMlpV4Ppo::Step` သည် 4-sample causal feature history စုဆောင်းခြင်း၊ MLP forward pass တွက်ချက်ခြင်းနှင့် physical action limit ကန့်သတ်ခြင်းတို့ကို တစ်ပေါင်းတည်း လုပ်ဆောင်ပေးပါသည်။

```cpp
#include <array>
#include <rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h>
#include <rom/tools/CoutTag.h>

namespace rl = rom::intelligence::model::rl::bc_mlp_v4_ppo;
namespace tools = rom::tools;

int main()
{
  rl::BcMlpV4Ppo policy;
  policy.set_limits({/*speed_limit=*/8.0, /*vz_limit=*/3.0, /*yaw_rate_limit=*/1.8});

  rl::BcMlpV4Ppo::ActionVector action{};

  // 50 Hz (dt = 0.02 s) ဖြင့် အနည်းဆုံး 4 ကြိမ် (0.00s, 0.02s, 0.04s, 0.06s) ထည့်သွင်းရန်လိုသည်
  for (int step = 0; step < 4; ++step)
  {
    const double time_s = step * 0.02;
    const bool ready = policy.Step(
        time_s,
        /*valid=*/true,
        /*ex=*/15.0,
        /*ey=*/-8.0,
        /*los_v=*/0.12,
        /*los_z=*/0.35,
        /*width=*/120.0,
        /*vx_ned=*/3.5,
        /*vy_ned=*/-1.0,
        /*vz_ned=*/-0.2,
        action);

    if (ready)
    {
      tools::cout << tools::Color::Green
                  << "vx_ned=" << action[0]
                  << " vy_ned=" << action[1]
                  << " vz_ned=" << action[2]
                  << " yaw_rate=" << action[3];
    }
  }
  return 0;
}
```

### $\color{cyan}{\text{နည်းလမ်း ၂ - CausalFeatureObserver နှင့် ForwardPhysical ကို ခွဲသုံးခြင်း}}$

Feature vector (13 dimensions) ကို သီးခြားစစ်ဆေးလိုပါက `CausalFeatureObserver`၊ `ForwardPhysical` နှင့် `LimitPhysicalAction` တို့ကို အဆင့်လိုက် ခေါ်သုံးနိုင်ပါသည် -

```cpp
#include <array>
#include <rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h>

namespace rl = rom::intelligence::model::rl::bc_mlp_v4_ppo;

void RunStep()
{
  rl::CausalFeatureObserver observer;
  std::array<float, rl::kInputDim> features{};

  observer.Update(0.00, true, 25.0, -15.0, 0.12, 0.35, 110.0, 3.5, -1.2, -0.4, features);
  observer.Update(0.02, true, 22.0, -12.0, 0.13, 0.36, 114.0, 3.6, -1.1, -0.3, features);
  observer.Update(0.04, true, 19.0, -10.0, 0.14, 0.37, 118.0, 3.7, -1.0, -0.2, features);

  if (observer.Update(0.06, true, 15.0, -8.0, 0.15, 0.38, 122.0, 3.8, -0.9, -0.1, features))
  {
    const auto raw_action = rl::ForwardPhysical(features);
    const auto cmd_action = rl::LimitPhysicalAction(raw_action, 8.0, 3.0, 1.8);
    (void)cmd_action;
  }
}
```

## $\color{red}{\text{Build နှင့် Test စစ်ဆေးပုံ}}$

SDK တစ်ခုလုံးကို build/test လုပ်ရန်နှင့် Python equivalence script ကို run ရန် -

```bash
# 1. SDK build & GoogleTest unit tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure

# 2. Python vs C++ (bc_mlp_v2_debiased.hpp vs BcMlpV4Ppo) verification
python3 90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v4_ppo.py
```

References:
- `90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v2_debiased.hpp`
- `90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v4_ppo.py`
- `90_cpp_to_rom_sdk/99_ppo_cpp_mlp_model_to_rom_sdk/bc_mlp_v2_debiased.md`
- `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.h`
- `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.cc`
- `rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4PpoTest.cc`
- `CMakeLists.txt`
