# $\color{#00c853}{\text{Markdown ရေးသားမှု စည်းမျဉ်းများ}}$

## $\color{red}{\text{ရေးသားပုံ}}$

- မြန်မာလိုရေးပါ။ နည်းပညာအမည်၊ command၊ ဖိုင်လမ်းကြောင်းကို English အတိုင်းထားပါ။ တိုတိုရှင်းရှင်း ရေးပါ။
- ဖြေချက်တိုင်းမှာ ဖိုင်လမ်းကြောင်း (repo-relative) ထည့်ပါ။ အဆုံးမှာ `References:` ပါပါ။

## $\color{red}{\text{Markdown စည်းမျဉ်း}}$

- ခေါင်းစဉ်အဆင့်အလိုက် အရောင်ကို LaTeX နဲ့ ပေးပါ (ခေါင်းစဉ်စာသားကို `\text{...}` ထဲထည့်):
  - `#` → အစိမ်း: `# $\color{#00c853}{\text{ခေါင်းစဉ်}}$`
  - `##` → အနီ: `## $\color{red}{\text{ခေါင်းစဉ်}}$`
  - `###` → cyan: `### $\color{cyan}{\text{ခေါင်းစဉ်}}$`
- မေးခွန်းခေါင်းစဉ် (`## Qn. ...`) က `##` ဖြစ်လို့ အနီပါ: `## $\color{red}{\text{Q1. မေးခွန်း}}$`
- ခေါင်းစဉ်ထဲမှာ `$`၊ `_`၊ `%`၊ `#`၊ `&`၊ `{`၊ `}`၊ `\` တွေ မသုံးပါနဲ့ (LaTeX ကွဲနိုင်လို့)။ code နာမည်တွေ လိုရင် ခေါင်းစဉ်အောက်က စာကြောင်းမှာ ရေးပါ။
- `####` နဲ့ အောက်ဆင့်တွေကို ပုံမှန်ထားပါ။
- Diagram ကို Mermaid နဲ့ ရေးပြီး `classDef` နဲ့ အရောင်ခွဲပါ။ ရေးပြီးရင် `mermaid.parse` နဲ့ syntax စစ်ပါ။ Mermaid label ထဲ ဖိုင်လမ်းကြောင်းနဲ့ အထူးအက္ခရာကို quote ထဲထည့်ပါ။
- သက်ဆိုင်ရာ `md_lists.md` မှာ Markdown ဖိုင်များကို repo root ကနေ relative path နဲ့ စာရင်းပြုစုပါ။
- Markdown ဖိုင်အသစ်ဖန်တီးခြင်း၊ ရွှေ့ခြင်း၊ အမည်ပြောင်းခြင်း၊ ဖျက်ခြင်းတို့ ပြုလုပ်ပါက သက်ဆိုင်ရာ `md_lists.md` စာရင်းကို စစ်ဆေးပြီး update လုပ်ပါ။
- အသုံးပြုသူက ဖိုင်ကို ကြိုဖွင့်ထားရင် အဲဒီဖိုင်ထဲ ဖြည့်ပါ။ planning/notes ဖိုင်တွေကို ကိုယ်ဘာသာ မထည့်ပါနဲ့။


- markdown file များတွင် bash, python scripts များပါလာပါ virtual env တည်ဆောက်ခြင်း , gui , cli , test script များဆိုရင် အောက်ပါအတိုင်း ခွဲခြားထားပေးပါ။
```bash
# create or source virtual env
blabla
```
```bash
# GUI
python3 blabla.py
```
```bash
# CLI
python3 blabla.py --no-gui
```
```bash
python test_blabla.py
```

- mermaid diagram colour သက်မှတ်ပုံ
  - class က အပြာ
    - constructor က အပြာနု
    - member function တွေက အစိမ်းနု
    - override method တွေက အနီ
  - structure ကို gray

- markdown file ဖြစ်သော *.md တို့တွင် timestamp ပါသော markdown file များကို ထည့်သွင်း ရေတွက်ခြင်း md_lists.md ထဲထည့်ခြင်း မပြုလုပ်ရပါ။