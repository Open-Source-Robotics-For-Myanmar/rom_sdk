# $\color{#00c853}{\text{rom sdk}}$

ROM Robotics အတွက် C++17 SDK။ Google C++ Style Guide ကို အခြေခံထားသည်။

## $\color{red}{\text{လိုအပ်ချက်}}$

- C++17 compiler (GCC သို့မဟုတ် Clang)
- CMake 3.16 နှင့်အထက်
- GoogleTest (test build လုပ်မှသာ လို)။ Ubuntu/Debian တွင်:

```bash
sudo apt install cmake g++ libgtest-dev
```

## $\color{red}{\text{Build}}$

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Default က shared library (`.so`) ထုတ်ပါတယ်။ Test မလိုလျှင် `-DROM_BUILD_TESTS=OFF`၊ static library (`.a`) လိုလျှင် `-DBUILD_SHARED_LIBS=OFF` ထည့်ပါ။

## $\color{red}{\text{Test}}$

```bash
ctest --test-dir build --output-on-failure
```

## $\color{red}{\text{Install နှင့် Uninstall}}$

### $\color{cyan}{\text{Release package (binary)}}$

`release/vX.Y.Z` tag တင်လိုက်လျှင် GitHub Actions က `CMakeLists.txt` version နှင့် စစ်ဆေးပြီး x86_64 (`amd64`) နှင့် AArch64 (`arm64`) အတွက် build/test လုပ်ကာ `.deb` package နှင့် SHA-256 checksum ပါသော GitHub Release ကို ထုတ်ပေးသည်။ Package များကို Ubuntu 24.04 သို့မဟုတ် `libc6` 2.39 နှင့် `libstdc++6` 13 နှင့်အထက်ရှိသော ကိုက်ညီသည့် distro များတွင် သုံးပါ။ [Releases](https://github.com/Open-Source-Robotics-For-Myanmar/rom_sdk/releases) မှ ကိုယ့်စက်၏ architecture နဲ့ ကိုက်ညီသော `rom-sdk_VERSION_amd64.deb` သို့မဟုတ် `rom-sdk_VERSION_arm64.deb` နှင့် သက်ဆိုင်ရာ `.sha256` ဖိုင်ကို download လုပ်ပါ။

```bash
sha256sum -c rom-sdk_VERSION_amd64.deb.sha256  # AArch64 တွင် amd64 ကို arm64 ဖြင့် အစားထိုးပါ
sudo apt install ./rom-sdk_VERSION_amd64.deb
```

`VERSION` ကို Release version ဖြင့် အစားထိုးပါ။ Install လုပ်ပြီးနောက် `-DCMAKE_PREFIX_PATH=/opt/rom` ဖြင့် SDK ကိုသုံးနိုင်သည်။ ဖယ်ရှားရန် `sudo apt remove rom-sdk` ကို သုံးပါ။ Source script နဲ့ binary package ကို တစ်ပြိုင်နက် မတပ်ဆင်ပါနှင့်။ Source install မှ package သို့ ပြောင်းမည်ဆိုလျှင် package မတပ်မီ `./uninstall_rom_sdk.sh` လုပ်ပါ။

### $\color{cyan}{\text{Script နဲ့ (အကြံပြု)}}$

Repo root က script ၂ ခုက build dir ဆောက်တာကနေ ဖျက်တာအထိ အကုန်လုပ်ပေးပါတယ်။ `sudo` ကို လိုတဲ့အဆင့်မှာပဲ တောင်းပါတယ်။

```bash
./install_rom_sdk.sh          # configure → build → test → /opt/rom ထဲ install → ldconfig
./uninstall_rom_sdk.sh        # /opt/rom နဲ့ /etc/ld.so.conf.d/rom.conf ကို ဖျက်
```

- `install_rom_sdk.sh` options: `--skip-tests`၊ `--build-type Debug`၊ `--jobs 4`၊ `--help`
- Install ပြီးသားရှိရင် `install_rom_sdk.sh` က အဟောင်းကို အရင် uninstall လုပ်ပါတယ်
- Build dir ကို `/tmp/rom_sdk_build.XXXXXX` မှာ ဆောက်ပြီး script ပြီးတာနဲ့ (fail ဖြစ်ရင်လည်း) ဖျက်ပါတယ်
- Install manifest ကို `/opt/rom/share/rom/install_manifest.txt` မှာ သိမ်းထားလို့ build dir မရှိတော့လည်း uninstall လုပ်လို့ရပါတယ်
- `DESTDIR=/tmp/stage ./install_rom_sdk.sh` ဆိုရင် sudo မလိုဘဲ `/tmp/stage/opt/rom` ထဲ စမ်း install လုပ်ပါတယ် (uninstall မှာလည်း `DESTDIR` တူတူပေးပါ)

### $\color{cyan}{\text{CMake နဲ့ ကိုယ်တိုင်}}$

Install prefix ကို `CMakeLists.txt` ထဲမှာ `/opt/rom` လို့ အသေသတ်မှတ်ထားပါတယ်။ `-DCMAKE_INSTALL_PREFIX` ပေးလည်း မပြောင်းပါ။

```bash
sudo cmake --install build                 # /opt/rom ထဲ install
cd build && sudo make uninstall            # /opt/rom ကို ပြန်ဖျက်
```

### $\color{cyan}{\text{Install layout}}$

```text
/opt/rom/
├── include/rom/...                     # public header (.h) များ၊ rom/ layout အတိုင်း
│   ├── tools/CoutTag.h, tools/InStream.h ... # rom::tools::cout / cin
│   ├── robotics/control/pid/PidController.h
│   └── robotics/guidance/png/PngGuidance.h
└── lib/
    ├── librom_tools.so*                # rom::tools
    ├── librom_robotics.so -> librom_robotics.so.0
    ├── librom_robotics.so.0 -> librom_robotics.so.0.1.0
    ├── librom_robotics.so.0.1.0
    └── cmake/rom/                      # find_package(rom) အတွက်
/opt/rom/share/rom/install_manifest.txt # script နဲ့ install မှ ပါ (uninstall အတွက်)
```

- `.cc`၊ `*Test.cc`၊ `internal/` header များကို install မလုပ်ပါ
- `/opt/rom/lib` က loader ရဲ့ default path မဟုတ်လို့ CMake နဲ့ ကိုယ်တိုင် install လုပ်ရင် အောက်ပါအတိုင်း တစ်ကြိမ် ထည့်ပေးရပါမယ် (script က အလိုလို လုပ်ပေးပါတယ်):

```bash
echo /opt/rom/lib | sudo tee /etc/ld.so.conf.d/rom.conf && sudo ldconfig
```

- User project ကို configure လုပ်တဲ့အခါ `-DCMAKE_PREFIX_PATH=/opt/rom` ပေးပါ

### $\color{cyan}{\text{Uninstall က ဘာတွေဖျက်လဲ}}$

- Manifest ထဲမှာ မှတ်ထားတဲ့ file အားလုံးကို ဖျက်ပါတယ်
- `/opt/rom` အောက်က directory အလွတ်တွေနဲ့ `/opt/rom` ကိုယ်တိုင်ကို ဖျက်ပါတယ်။ `/opt` ကိုတော့ မဖျက်ပါ
- `uninstall_rom_sdk.sh` က `/etc/ld.so.conf.d/rom.conf` ကိုပါ ဖျက်ပြီး `ldconfig` run ပါတယ်။ `make uninstall` ကတော့ မဖျက်ပါ
- `make uninstall` က `build/install_manifest.txt` ကို သုံးလို့ `build/` ကို ဖျက်ပြီးရင် လုပ်လို့မရတော့ပါ။ Script ကတော့ `/opt/rom/share/rom/install_manifest.txt` ကို သုံးပါတယ်

## $\color{red}{\text{SDK ကို သုံးပုံ}}$

Install ပြီးနောက် တခြား project တစ်ခုက `find_package(rom)` နှင့် သုံးနိုင်သည်။

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_app LANGUAGES CXX)

find_package(rom 0.1 REQUIRED)

add_executable(my_app main.cc)
target_link_libraries(my_app PRIVATE rom::tools rom::robotics)
```

| CMake target | Library | Namespace / ပါဝင်တာ |
|---|---|---|
| `rom::tools` | `librom_tools.so` | `rom::tools::cout`, `cin`, `Color`, `Style` |
| `rom::robotics` | `librom_robotics.so` | `rom::robotics::...` (`PidController`, `PngGuidance`) |

```cpp
#include <rom/tools/CoutTag.h>
#include <rom/tools/InStream.h>
#include <rom/robotics/control/pid/PidController.h>

namespace pid = rom::robotics::control::pid;
namespace tools = rom::tools;

int main() {
  pid::PidController controller;
  controller.set_gains(1.0, 0.0, 0.1);
  const double command = controller.Update(/*error=*/0.5, /*dt_s=*/0.01);

  tools::cout << tools::bold(tools::Color::Green) << "command: " << command;
  return 0;
}
```

### $\color{cyan}{\text{tools cout နဲ့ tools cin}}$

`rom::tools::cout` က statement တစ်ခု = timestamp ပါတဲ့ line တစ်ကြောင်းပါ။ `rom::tools::reset` နဲ့ `"\n"` ကို အလိုလို ထည့်ပေးလို့ ကိုယ်တိုင် မရေးရပါ။

```cpp
namespace tools = rom::tools;

tools::cout << "score: " << score;               // [1718352012.345678901] score: 42
tools::cout << tools::Color::Red << "error";        // Normal (ANSI 30-37)
tools::cout << tools::Color::DarkRed << "dark";     // Dark < Normal < Bright
tools::cout << tools::bg(tools::Color::BrightBlue) << "background";
tools::cout << tools::color256(202) << "orange";
tools::cout << tools::rgb(255, 105, 180) << "pink";

int value = 0;
tools::cin >> value;
std::string line;
tools::cin.getline(line);
```

## $\color{red}{\text{CPU အလိုက် build (နောင်)}}$

`ROM_TARGET_CPU` option (`intel` | `rockchip`) ကို နောင်တွင် ထည့်သွင်းမည်။ Target: Intel x86_64 နှင့် Rockchip AArch64။

## $\color{red}{\text{Markdown ဖိုင်များ}}$

- [./README.md](./README.md) - Project အကျဉ်းနှင့် build instruction
- [./AGENTS.md](./AGENTS.md) - C++17၊ CMake၊ naming၊ directory စည်းမျဉ်း
- [./AGENTS_for_markdown.md](./AGENTS_for_markdown.md) - Markdown ရေးသားပုံ စည်းမျဉ်း
- [./AGENTS_to_create_sdk.md](./AGENTS_to_create_sdk.md) - SDK API ဒီဇိုင်း၊ အသုံးပြုပုံ၊ build/install လမ်းညွှန်
# rom_sdk
# rom_sdk
