# $\color{#00c853}{\text{ROM Color အသုံးပြုပုံ}}$

`src/ct_follower_rl/` ရှိ လက်ရှိ executable source များတွင် `rom_color::` ကို **အရောင် ၁၁ မျိုး၊ log နေရာ ၃၃ ခု**၌ သုံးထားသည်။ အောက်တွင် အရောင်၊ function၊ log အဓိပ္ပာယ်နှင့် ဖွင့်ရန်လိုသော flag များကို တွဲပြထားသည်။

## $\color{red}{\text{အခြေခံအသုံးပြုပုံ}}$

[src/main.cc](./src/main.cc#L7)၊ [src/vision_png_control.cc](./src/vision_png_control.cc#L8) နှင့် [src/px4_multirotor_follower.cc](./src/px4_multirotor_follower.cc#L11) တို့သည် external `rom` library ကို အောက်ပါအတိုင်း သုံးသည်။

```cpp
#include <rom/tools/CoutTag.h>

inline auto& rom_cout = rom::tools::cout;
using rom_color = rom::tools::Color;

rom_cout << rom_color::Red << "message";
```

- `rom_color` သည် `rom::tools::Color` ၏ alias ဖြစ်သည်။ `rom_cout` သည် `rom::tools::cout` ကို ရည်ညွှန်းသည်။
- `rom_color::Red` ကဲ့သို့ အရောင်ကို အရင်ထည့်ပြီး message နှင့် value များကို `<<` ဖြင့် ဆက်ရေးသည်။
- Statement အဆုံး `;` တွင် timestamp ပါသော line ကို ထုတ်ပြီး color reset နှင့် newline ကို library က အလိုအလျောက်လုပ်သည်။
- လက်ရှိ dependency သည် [CMakeLists.txt](./CMakeLists.txt#L31) ရှိ `rom` / `rom::tools` ဖြစ်သည်။ [to_delete_rom.hpp](./include/ct_follower_rl/rom/to_delete_rom.hpp) ရှိ အဟောင်း `rom::cout` example များကို အောက်ပါ runtime စာရင်းတွင် မထည့်ထားပါ။

> အောက်ပါ အရောင်စာသားများသည် ဖတ်ရလွယ်ရန် နမူနာပြထားခြင်းဖြစ်သည်။ Normal နှင့် Bright အရောင်အတိအကျသည် terminal theme အလိုက် ကွာနိုင်သည်။ Dark များသည် library ၏ fixed RGB ကို သုံးသည်။ `Default` သည် အဖြူဟု မသတ်မှတ်ဘဲ terminal ၏ မူလ foreground ဖြစ်သည်။

## $\color{red}{\text{အရောင်နှင့် Function တွဲဖက်စာရင်း}}$

### $\color{cyan}{\text{Guidance နှင့် Control}}$

Source: [src/vision_png_control.cc](./src/vision_png_control.cc)။ Function များသည် `VisionPNGGuidance` ၏ member function များဖြစ်သည်။

အောက်ပါ table ၏ log များတွင် **`USE_ROM_STYLE_LOG` လိုသည်**။ ခြွင်းချက်မှာ `Vz_PID_DEBUG` / `Yaw_PID_DEBUG` ဖြင့်သာ guard လုပ်ထားသော PID log များဖြစ်ပြီး table တွင် သီးသန့်ဖော်ပြထားသည်။ Flag column ရှိ `—` သည် ထပ်ဆောင်း flag မလိုဟု ဆိုလိုသည်။

| အရောင် | Color | Function | Log / အသုံးပြုပုံ | ထပ်ဆောင်း flag / အခြေအနေ |
|---|---|---|---|---|
| $\color{#b58900}{\text{အဝါ}}$ | `rom_color::Yellow` | [on_detection()](./src/vision_png_control.cc#L69) | `→ Not detected from Tracker` — tracker မှ target မတွေ့ခြင်း၊ `width == -1` | `DETECTION_DEBUG` |
| $\color{#006400}{\text{အစိမ်းရင့်}}$ | `rom_color::DarkGreen` | [on_detection()](./src/vision_png_control.cc#L69) | `→ Target visually detected` — valid target detection | `DETECTION_DEBUG` |
| $\color{#006400}{\text{အစိမ်းရင့်}}$ | `rom_color::DarkGreen` | [on_detection()](./src/vision_png_control.cc#L69) | `→ Target visually detected, entering INTERCEPT` — `SEARCHING` မှ `INTERCEPT` သို့ဝင်ခြင်း | `DETECTION_DEBUG` + `FIRST_INTERCEPT_DEBUG` |
| $\color{#00cc66}{\text{အစိမ်းတောက်}}$ | `rom_color::BrightGreen` | [on_detection()](./src/vision_png_control.cc#L69) | `→ Target reappeared, resuming INTERCEPT` — `TRACK_LOST` မှ ပြန်တွေ့ခြင်း | `DETECTION_DEBUG` + `REINTERCEPT_DEBUG` |
| $\color{#ff5555}{\text{အနီတောက်}}$ | `rom_color::BrightRed` | [update_horizontal_approach_shadow()](./src/vision_png_control.cc#L254) | `[APPROACH TRANSITION]` — stop latch ပြောင်းချိန် reason၊ bbox၊ filtered width၊ resume elapsed ကိုပြခြင်း | `stop_latched` ပြောင်းသည့်အခါ |
| $\color{#00008b}{\text{အပြာရင့်}}$ | `rom_color::DarkBlue` | [guidance_tick()](./src/vision_png_control.cc#L405) | `[PNG init]` — ကနဦး LOS angle နှင့် velocity magnitude | `PNG_DEBUG` |
| $\color{#00008b}{\text{အပြာရင့်}}$ | `rom_color::DarkBlue` | [guidance_tick()](./src/vision_png_control.cc#L405) | `[PNG]` — image error၊ LOS၊ LOS ပြောင်းလဲမှုနှင့် desired velocity angle | `PNG_DEBUG`၊ LOS ပြောင်းလဲမှု threshold ကျော်ချိန် |
| $\color{#008b8b}{\text{စိမ်းပြာရင့်}}$ | `rom_color::DarkCyan` | [handle_searching()](./src/vision_png_control.cc#L612) | `[ Searching ]` — searching handler အလုပ်လုပ်နေခြင်း | — |
| $\color{red}{\text{အနီ}}$ | `rom_color::Red` | [handle_searching()](./src/vision_png_control.cc#L612) | `[SEARCHING] Waiting for vision...` — detection width နှင့် search yaw rate | `TARGET_LOST_DEBUG` |
| မူလ foreground | `rom_color::Default` | [handle_intercept()](./src/vision_png_control.cc#L637) | `[INTERCEPT/COAST]` — ခဏပျောက်ချိန် cached velocity ဖြင့် ဆက်သွားခြင်း | `TARGET_LOST_DEBUG`၊ `0 < lost_frames < coast_thresh` |
| $\color{#ff5555}{\text{အနီတောက်}}$ | `rom_color::BrightRed` | [handle_intercept()](./src/vision_png_control.cc#L637) | `[png, correction]` နှင့် `[png] final_vz_cmd` — PNG/PID vertical correction နှင့် clamp မလုပ်မီ blend ရလဒ်၊ **log ၂ ခု** | **`Vz_PID_DEBUG` သာလိုသည်** |
| $\color{#ff55ff}{\text{ပန်းခရမ်းတောက်}}$ | `rom_color::BrightMagenta` | [handle_intercept()](./src/vision_png_control.cc#L637) | `[HYBRID]` — horizontal blend၊ PNG၊ approach နှင့် final velocity | `USE_VX_VY_HORIZONTAL_APPROACH` |
| $\color{#008b8b}{\text{စိမ်းပြာရင့်}}$ | `rom_color::DarkCyan` | [handle_intercept()](./src/vision_png_control.cc#L637) | `[ Calculated ]` — velocity တွက်ပြီး publish မခေါ်မီ marker | — |
| မူလ foreground | `rom_color::Default` | [handle_intercept()](./src/vision_png_control.cc#L637) | `[INTERCEPT]` — PNG velocity၊ yaw rate၊ speed နှင့် image error | `INTERCEPT_DEBUG` |
| $\color{#008b8b}{\text{စိမ်းပြာရင့်}}$ | `rom_color::DarkCyan` | [handle_track_lost()](./src/vision_png_control.cc#L829) | `[ TrackLost ]` — target ပျောက်နေသည့် handler marker | — |
| မူလ foreground | `rom_color::Default` | [handle_track_lost()](./src/vision_png_control.cc#L829) | `[TRACK_LOST]` — velocity သုညနှင့် yaw ဖြင့် ပြန်ရှာနေခြင်း | — |
| $\color{#008b8b}{\text{စိမ်းပြာရင့်}}$ | `rom_color::DarkCyan` | [handle_done()](./src/vision_png_control.cc#L851) | `[ Done ]` — done handler marker | — |
| $\color{magenta}{\text{ပန်းခရမ်း}}$ | `rom_color::Magenta` | [handle_done()](./src/vision_png_control.cc#L851) | `★★★ Intercept complete, hovering ★★★` — ပြီးစီးပြီး hover လုပ်ခြင်း | `INTERCEPT_DEBUG` |
| $\color{#00bcd4}{\text{စိမ်းပြာ}}$ | `rom_color::Cyan` | [publish_velocity_setpoint()](./src/vision_png_control.cc#L867) | `[ PUBLISHED ]` — follower သို့ပို့သော `vx, vy, vz, yaw_rate`၊ compile branch **၂ နေရာ** | `PUBLISHED_DEBUG`။ `USE_VX_VY_HORIZONTAL_APPROACH` ရှိလျှင် requested `vx/vy`၊ မရှိလျှင် `approach_vx_/approach_vy_` ကိုပြသည် |
| $\color{#00008b}{\text{အပြာရင့်}}$ | `rom_color::DarkBlue` | [update_yaw_pid()](./src/vision_png_control.cc#L928) | `[ Yaw PID ]` — `dt`၊ raw PID output နှင့် `d_yaw` | **`Yaw_PID_DEBUG` သာလိုသည်** |
| $\color{#ff5555}{\text{အနီတောက်}}$ | `rom_color::BrightRed` | [vz_inline_pid_control()](./src/vision_png_control.cc#L965) | `[i-term]` — vertical PID ၏ integral contribution | **`Vz_PID_DEBUG` သာလိုသည်** |

### $\color{cyan}{\text{PX4 Follower နှင့် Statistics}}$

Source: [src/px4_multirotor_follower.cc](./src/px4_multirotor_follower.cc)။ အားလုံးတွင် `USE_ROM_STYLE_LOG` လိုပြီး function များသည် `Px4MultirotorFollower` ၏ member များဖြစ်သည်။

| အရောင် | Color | Function | Log / အသုံးပြုပုံ | ထပ်ဆောင်း flag / အခြေအနေ |
|---|---|---|---|---|
| မူလ foreground | `rom_color::Default` | [Px4MultirotorFollower()](./src/px4_multirotor_follower.cc#L20) ရှိ `self_gps_sub_` callback | `[stats] Coordinate frame origin initialized` — ပထမ GPS fix မှ statistics origin သတ်မှတ်ခြင်း | `CSV_LOGGING_DEBUG` |
| မူလ foreground | `rom_color::Default` | [~Px4MultirotorFollower()](./src/px4_multirotor_follower.cc#L154) | `[stats] CSV saved to:` — CSV writer ပိတ်ပြီး path ပြခြင်း | `CSV_LOGGING_DEBUG`၊ writer ဖွင့်ထားခဲ့လျှင် |
| မူလ foreground | `rom_color::Default` | [~Px4MultirotorFollower()](./src/px4_multirotor_follower.cc#L154) | `VisionPNG node shut down` — shutdown | — |
| မူလ foreground | `rom_color::Default` | [prepare()](./src/px4_multirotor_follower.cc#L183) | `→ Take-off complete ... entering SEARCHING` — takeoff ပြီးစီးခြင်း | `TAKEOFF_DEBUG` |
| မူလ foreground | `rom_color::Default` | [prepare()](./src/px4_multirotor_follower.cc#L183) | `[TAKEOFF]` — altitude၊ velocity၊ position validity၊ armed/offboard/hover status | `TAKEOFF_DEBUG`၊ takeoff completion မပြည့်သေးလျှင် |
| $\color{red}{\text{အနီ}}$ | `rom_color::Red` | [record()](./src/px4_multirotor_follower.cc#L292) | `[stats] Could not open CSV file:` — CSV ဖွင့်မရခြင်း၊ error ကို တစ်ကြိမ်သာ log လုပ်သည် | — |
| မူလ foreground | `rom_color::Default` | [record()](./src/px4_multirotor_follower.cc#L292) | `[stats] CSV created:` — CSV အသစ်ဖွင့်ခြင်း | `CSV_LOGGING_DEBUG` |
| $\color{magenta}{\text{ပန်းခရမ်း}}$ | `rom_color::Magenta` | [record()](./src/px4_multirotor_follower.cc#L292) | `★ [stats] HIT!` — true distance က `hit_radius` အောက် ပထမဆုံးဝင်ခြင်း၊ statistics log | `HIT_LOG_DEBUG` |

### $\color{cyan}{\text{Node စတင်ခြင်းနှင့် Error}}$

Source: [src/main.cc](./src/main.cc)။ နှစ်ခုစလုံးတွင် `USE_ROM_STYLE_LOG` လိုသည်။

| အရောင် | Color | Function | Log / အသုံးပြုပုံ |
|---|---|---|---|
| မူလ foreground | `rom_color::Default` | [main()](./src/main.cc#L15) | `VisionPNG node started` — focal length၊ image size၊ PNG gain နှင့် speed |
| $\color{red}{\text{အနီ}}$ | `rom_color::Red` | [main()](./src/main.cc#L15) ၏ `catch` | `error.what()` — startup / runtime exception message |

## $\color{red}{\text{ဖတ်ရှုရာတွင် သတိပြုရန်}}$

- Flag များကို [rom_define_ct_follower_rl.h](./include/ct_follower_rl/rom/rom_define_ct_follower_rl.h) တွင် သတ်မှတ်ထားသည်။ လက်ရှိ `PRODUCTION` သည် comment ဖြစ်နေပြီး development branch တွင် အထက်ပါ debug flag များနှင့် `USE_ROM_STYLE_LOG` ကို define လုပ်ထားသည်။ Comment ထဲက “default FALSE” ထက် တကယ့် `#define` ကို အခြေခံကြည့်ရမည်။
- `PRODUCTION` ဖွင့်လျှင် ထို header ၏ production branch က debug flag များနှင့် `USE_ROM_STYLE_LOG` ကို မသတ်မှတ်ပေးပါ။
- `Red` သည် error သာမက searching status အတွက်ပါ သုံးထားသည်။ `BrightRed` သည် PID / approach transition အတွက်လည်း သုံးသည်။ ထို့ကြောင့် အရောင်ကို log severity ဟု တိုက်ရိုက်မယူရပါ။
- Non-ROM branch ရှိ `\033[...]` အရောင်များသည် `rom_color::` နှင့် အမြဲမတူပါ။ ဤစာရင်းသည် `rom_color::` branch ကိုသာ အခြေခံထားသည်။
- `log_throttled()` / `log_once()` ကို ROM branch တွင် တိုက်ရိုက် `rom_cout` ဖြင့် အစားထိုးထားသောနေရာများရှိသည်။ ထိုနေရာများတွင် non-ROM branch ၏ throttle / once အပြုအမူကို အလိုအလျောက် မရပါ။
- `[INTERCEPT] v_cmd` သည် PNG component ကိုပြသည်။ အမှန်တကယ် follower သို့ပို့သည့် velocity ကိုကြည့်ရန် `[ PUBLISHED ]` ကိုကြည့်ပါ။

References:
- [src/main.cc](./src/main.cc)
- [src/vision_png_control.cc](./src/vision_png_control.cc)
- [src/px4_multirotor_follower.cc](./src/px4_multirotor_follower.cc)
- [include/ct_follower_rl/rom/rom_define_ct_follower_rl.h](./include/ct_follower_rl/rom/rom_define_ct_follower_rl.h)
- [CMakeLists.txt](./CMakeLists.txt)