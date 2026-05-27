<!-- xer-reference-source-sha256: 243c22feff71768e96aa1f43491dc13cb628abf41dea6b098741259f0c83d5c6 -->
# `<xer/color.h>`

## 目的

`<xer/color.h>` は、色体系の値型と色変換関数を提供します。

このヘッダーの目的は、軽量な xer らしい形で、実用的な数式ベースの色表現と変換を支援することです。

初期段階でサポートする色体系は次のとおりです。

- RGB
- グレースケール
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

このヘッダーは、完全なカラーマネジメントシステムを目指していません。ICC プロファイル、色順応、分光データ、名前付き色、カラーパレット管理などは扱いません。

---

## 主なエンティティ

`<xer/color.h>` は、少なくとも次のクラステンプレートを提供します。

```cpp
template <std::floating_point T>
struct basic_rgb;

template <std::floating_point T>
struct basic_gray;

template <std::floating_point T>
struct basic_cmy;

template <std::floating_point T>
struct basic_hsv;

template <std::floating_point T>
struct basic_xyz;

template <std::floating_point T>
struct basic_lab;

template <std::floating_point T>
struct basic_luv;
```

通常用途向けに、`float` を使う別名も提供します。

```cpp
using rgb = basic_rgb<float>;
using gray = basic_gray<float>;
using cmy = basic_cmy<float>;
using hsv = basic_hsv<float>;
using xyz = basic_xyz<float>;
using lab = basic_lab<float>;
using luv = basic_luv<float>;
```

公開ヘッダーは次のとおりです。

```cpp
#include <xer/color.h>
```

実装は次の内部ヘッダーで提供されます。

```cpp
#include <xer/bits/color.h>
```

ユーザーコードでは公開ヘッダーをインクルードしてください。

---

## 設計上の役割

`<xer/color.h>` は、一般的な色体系のための小さな値型と変換関数を提供します。

設計は次の考え方に基づきます。

- 色値は公開データメンバーを持つ単純な構造体にする
- 通常の別名は `float` を使う
- 正規化された境界付き成分には `xer::interval<T>` を使う
- 色相には `xer::cyclic<T>` を使う
- XYZ、Lab、Luv のような測色系では生の浮動小数点メンバーを使う
- 変換関数は自由関数にする
- 決定的な算術変換は変換先の値を直接返す

---

## `float` 別名

テンプレート自体は `float`、`double`、`long double` を使えますが、実用的な色処理では通常 `float` を使います。

そのため、通常の別名は `float` を使います。

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

別の精度が必要な場合は、対応するテンプレートを直接使います。

```cpp
xer::basic_rgb<double> color(0.25, 0.5, 0.75);
```

---

## RGB

## `basic_rgb`

```cpp
template <std::floating_point T>
struct basic_rgb {
    using value_type = T;
    using component_type = interval<T>;

    component_type r;
    component_type g;
    component_type b;
};
```

`basic_rgb<T>` は、正規化された赤、緑、青の成分を持つ RGB 色を表します。

各成分は `interval<T>` で表されるため、`[0, 1]` に保たれます。

```cpp
auto color = xer::rgb(1.25f, 0.5f, -0.25f);

// color.r.value() == 1.0f
// color.g.value() == 0.5f
// color.b.value() == 0.0f
```

### sRGB の仮定

公開型名は `rgb` であり、`srgb` ではありません。

ただし、RGB と XYZ の変換では、D65 白色点の sRGB を仮定します。

つまり、次の意味になります。

- `to_xyz(rgb)` は RGB 成分を非線形 sRGB 成分として扱います。
- `to_rgb(xyz)` は非線形 sRGB 成分を生成します。

RGB を XYZ、Lab、Luv と組み合わせて使う場合は、この仮定に注意してください。

### アルファ

アルファは `basic_rgb` には含まれません。

アルファは、主にグラフィックス、合成、画像処理で有用です。色そのものの一般的な成分ではなく、印刷、塗装、照明、測色などの分野では不要です。

将来アルファ対応が必要になった場合は、`basic_rgba<T>` のような別型として提供するのがよいです。`basic_rgb<T>` に混ぜるべきではありません。

---

## グレースケール

## `basic_gray`

```cpp
template <std::floating_point T>
struct basic_gray {
    using value_type = T;
    using component_type = interval<T>;

    component_type y;
};
```

`basic_gray<T>` は、表示指向のグレースケール値を表します。成分は `interval<T>` で表され、`[0, 1]` に保たれます。

`to_luma_gray` は、非線形 sRGB 成分から単純なルーマを直接計算します。`to_luminance_gray` は、sRGB をデコードして相対輝度を求め、それを表示グレースケール値へ再エンコードします。`to_gray` は `to_luma_gray` の別名です。`to_rgb(gray)` はグレースケール成分を RGB の各成分へ複製します。

---

## CMY

## `basic_cmy`

```cpp
template <std::floating_point T>
struct basic_cmy {
    using value_type = T;
    using component_type = interval<T>;

    component_type c;
    component_type m;
    component_type y;
};
```

`basic_cmy<T>` は、単純な正規化 CMY 色を表します。

各成分は `interval<T>` で表されるため、`[0, 1]` に保たれます。

xer の CMY は RGB の単純な補色モデルです。

概念的には次の関係です。

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

および:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

CMYK や印刷用のカラーマネジメントは扱いません。

---

## HSV

## `basic_hsv`

```cpp
template <std::floating_point T>
struct basic_hsv {
    using value_type = T;
    using hue_type = cyclic<T>;
    using component_type = interval<T>;

    hue_type h;
    component_type s;
    component_type v;
};
```

`basic_hsv<T>` は、色相、彩度、明度を持つ HSV 色を表します。

色相 `h` は循環値なので `cyclic<T>` で表されます。彩度 `s` と明度 `v` は `[0, 1]` の境界付き値なので `interval<T>` で表されます。

---

## XYZ

## `basic_xyz`

```cpp
template <std::floating_point T>
struct basic_xyz {
    using value_type = T;

    T x;
    T y;
    T z;
};
```

`basic_xyz<T>` は CIE 1931 XYZ 色を表します。

XYZ は正規化成分型ではなく、測色値として生の浮動小数点値を保持します。

RGB との変換では、sRGB と D65 白色点を仮定します。

---

## Lab

## `basic_lab`

```cpp
template <std::floating_point T>
struct basic_lab {
    using value_type = T;

    T l;
    T a;
    T b;
};
```

`basic_lab<T>` は CIE 1976 L*a*b* 色を表します。

`l` は明度、`a` と `b` は色度成分です。

Lab は、RGB のような `[0, 1]` 成分ではありません。値域は変換元や白色点に依存するため、生の浮動小数点値として表します。

---

## Luv

## `basic_luv`

```cpp
template <std::floating_point T>
struct basic_luv {
    using value_type = T;

    T l;
    T u;
    T v;
};
```

`basic_luv<T>` は CIE 1976 L*u*v* 色を表します。

`l` は明度、`u` と `v` は色度成分です。

Lab と同様に、正規化区間値ではなく生の浮動小数点値として表します。

---

## コンストラクタ

### RGB

RGB は、赤、緑、青の値から構築できます。

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

入力値は `interval<T>` の規則に従って `[0, 1]` にクランプされます。`NaN` や無限大は例外で拒否されます。

### CMY

CMY は、シアン、マゼンタ、イエローの値から構築できます。

```cpp
xer::cmy color(0.0f, 0.5f, 1.0f);
```

### HSV

HSV は、色相、彩度、明度から構築できます。

```cpp
xer::hsv color(0.5f, 1.0f, 0.75f);
```

色相は `cyclic<T>` として扱われ、循環正規化されます。彩度と明度は `interval<T>` として扱われます。

### XYZ、Lab、Luv

XYZ、Lab、Luv は、対応する成分値から構築できます。

これらの測色系では、成分は生の浮動小数点値です。正規化成分ではありません。

---

## 変換関数

変換関数は自由関数として提供されます。

---

## RGB と CMY

RGB と CMY の相互変換は、単純な補色関係に基づきます。

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

および:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

---

## RGB と HSV

RGB と HSV の変換は、一般的な数式に基づきます。

HSV の色相は循環値として扱われ、彩度と明度は `[0, 1]` に保たれます。

グレースケールに近い入力では、色相の意味が弱くなる点に注意してください。

---

## RGB と XYZ

RGB と XYZ の変換では、sRGB と D65 白色点を仮定します。

RGB から XYZ への変換では、非線形 sRGB 成分を線形化してから変換します。

XYZ から RGB への変換では、線形 RGB へ変換した後、sRGB エンコードを行います。結果の RGB 成分は `interval<T>` によって `[0, 1]` に収められます。

---

## XYZ と Lab

XYZ と Lab の相互変換は、CIE 1976 L*a*b* の式に基づきます。

白色点は、RGB/XYZ 変換と整合するように扱われます。

---

## XYZ と Luv

XYZ と Luv の相互変換は、CIE 1976 L*u*v* の式に基づきます。

Luv も測色系であり、成分は正規化区間値ではありません。

---

## 直接変換ポリシー

直接変換関数は、必要に応じて中間色空間を経由しても構いません。

たとえば、RGB から Lab への変換は、概念的には RGB から XYZ、XYZ から Lab への変換を組み合わせたものです。

公開 API として重要なのは、呼び出し側が意図する変換を直接書けることです。

---

## エラーと例外のポリシー

決定的な算術変換は、通常、変換先の値を直接返します。

正規化成分では、`interval<T>` が範囲外値をクランプし、`NaN` や無限大を例外で拒否します。

色変換そのものは、通常の失敗を `xer::result` で返す操作としてではなく、値変換として扱います。

---

## サポートする色体系とサポートしない色体系

### サポート対象

現在サポートする色体系は次のとおりです。

- RGB
- グレースケール
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

### サポート対象外

現在サポートしないものは次のとおりです。

- ICC プロファイル
- CMYK
- HSL
- HWB
- スペクトル色
- 名前付き色
- カラーパレット管理
- デバイス依存の詳細なカラーマネジメント

---

## 後回しにしている項目

次の項目は、初期実装では意図的に後回しにしています。

- アルファ付き色型
- ICC プロファイル対応
- 色順応
- CMYK
- HSL や HWB などの追加色体系
- 色差計算
- 名前付き色
- パレット処理

これらは、必要性が明確になってから別の型や関数として追加するのがよいです。

---

## 他のヘッダーとの関係

`<xer/color.h>` は次のヘッダーやポリシーと関係します。

- `<xer/interval.h>`
- `<xer/cyclic.h>`
- `<xer/arithmetic.h>`
- `policy_color.md`
- `policy_interval.md`

`interval` は正規化成分に使われ、`cyclic` は色相に使われます。

---

## ドキュメント上の注意

このヘッダーを説明するときは、次の点を明確にします。

- 色値は単純な構造体であること
- 正規化成分は `interval<T>` を使うこと
- 色相は `cyclic<T>` を使うこと
- RGB/XYZ 変換では sRGB と D65 を仮定すること
- 完全なカラーマネジメントシステムではないこと

---

## 例

```cpp
#include <xer/color.h>

#include <xer/arithmetic.h>

auto main() -> int
{
    const auto color = xer::rgb(1.25f, 0.5f, -0.25f);

    if (color.r.value() != 1.0f) {
        return 1;
    }

    if (color.g.value() != 0.5f) {
        return 1;
    }

    if (color.b.value() != 0.0f) {
        return 1;
    }

    const auto cmy = xer::to_cmy(color);
    const auto rgb = xer::to_rgb(cmy);

    if (!xer::is_close(rgb.r.value(), 1.0f, 1e-6f)) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

- `policy_color.md`
- `policy_interval.md`
- `header_interval.md`
- `header_cyclic.md`
- `header_arithmetic.md`
