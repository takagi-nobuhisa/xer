<!-- xer-reference-source-sha256: 23ff3d606dfed5a5f970a9ce1ef1d4869e4835f6cdb6f659f94806ba55878f5b -->

# `<xer/image.h>`


## 目的

`<xer/image.h>` は、軽量な画像およびフレームバッファ機能を提供します。

このヘッダーの初期目的は、本格的な写真編集や完全な画像ファイル処理ではありません。固定サイズキャンバス、VRAM風のエミュレーション、単純な描画、画像処理、および将来的な Tcl/Tk photo image 連携に向けた、小さなフレームバッファ指向レイヤーです。

純粋な画像処理と描画は `<xer/image.h>` に属します。Tcl/Tk photo 連携は `<xer/tk.h>` に属します。

---

## 名前空間

画像関連の型と関数は `xer::image` 名前空間に配置されます。

主要なフレームバッファ所有型は `xer::image::canvas` です。これにより、`xer::image` を画像ストレージ、描画、画像処理の名前空間として使えます。

---

## 主なエンティティ

少なくとも `<xer/image.h>` は次のエンティティを提供します。

```cpp
namespace xer::image {

struct point;
struct pointf;
struct size;
struct sizef;
struct rect;
struct rectf;

struct filter_pixels_error_detail;

enum class bitmap_glyph_width : std::uint8_t;
struct bitmap_font_range;
struct bitmap_font;
struct text_draw_options;

struct pixel;

struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;

template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;

template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;

[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               int x0,
               int y0,
               int x1,
               int y1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               const point& p0,
               const point& p1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              int cx,
              int cy,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              const point& center,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      int cx,
                      int cy,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      const point& center,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

struct filter_pixels_error_detail {
    point first_error_position;
    std::size_t error_count;
};

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;

}
```

---

## 幾何型

このヘッダーは、整数座標用と浮動小数点座標用の小さな幾何型を提供します。

```cpp
struct point {
    int x;
    int y;
};

struct pointf {
    float x;
    float y;
};

struct size {
    int width;
    int height;
};

struct sizef {
    float width;
    float height;
};

struct rect {
    point origin;
    size extent;
};

struct rectf {
    pointf origin;
    sizef extent;
};
```

これらの型は、描画APIでスカラー座標と構造化座標の両方を自然に使うためのものです。

整数型は、ピクセル境界に基づく通常の描画に使います。浮動小数点型は、アンチエイリアス描画のようにピクセル中心やサブピクセル位置を扱うAPIで使います。

---

## フィルターエラー詳細

`filter_pixels_error_detail` は、ユーザー指定フィルターが例外を投げた場合の情報を保持します。

```cpp
struct filter_pixels_error_detail {
    point first_error_position;
    std::size_t error_count;
};
```

`first_error_position` は、フィルター呼び出しが最初に失敗したピクセル位置です。

`error_count` は、フィルター呼び出しに失敗したピクセル総数です。

失敗位置は最初の1件だけを保存します。これにより、失敗ピクセルの巨大なリストを確保せずに、呼び出し側に有用な診断位置を返せます。

---

## 論理ピクセル

`xer::image::pixel` は論理色値を表します。

これは物理フレームバッファのストレージ要素とは同じではありません。物理ストレージ形式は canvas policy によって制御されます。

論理表現は32ビット整数のARGBです。

```text
0xAARRGGBB
```

概念的な形は次のとおりです。

```cpp
struct pixel {
    std::uint32_t argb = 0xff000000u;

    constexpr pixel() noexcept = default;
    constexpr explicit pixel(std::uint32_t value) noexcept;
    constexpr pixel(std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;
    constexpr pixel(std::uint8_t alpha,
                    std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;

    constexpr auto alpha() const noexcept -> std::uint8_t;
    constexpr auto red() const noexcept -> std::uint8_t;
    constexpr auto green() const noexcept -> std::uint8_t;
    constexpr auto blue() const noexcept -> std::uint8_t;

    constexpr auto alpha(std::uint8_t value) noexcept -> void;
    constexpr auto red(std::uint8_t value) noexcept -> void;
    constexpr auto green(std::uint8_t value) noexcept -> void;
    constexpr auto blue(std::uint8_t value) noexcept -> void;
};
```

3引数コンストラクタはRGBを表し、アルファ値を `0xff` に設定します。

4引数コンストラクタの順序はARGBです。

```text
alpha, red, green, blue
```

---

## フレームバッファストレージポリシー

canvas policy は、物理フレームバッファのストレージ形式を制御します。

ポリシーは次を提供します。

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
static constexpr auto encode(pixel value) noexcept -> storage_type;
static constexpr auto set(storage_type& dst, pixel value) noexcept -> void;
```

初期ポリシーは次のとおりです。

```cpp
argb32_policy
rgba32_policy
rgb24_policy
bgr24_policy
```

`argb32_policy` は `0xAARRGGBB` を格納するため、論理 `pixel` 表現と直接一致します。

`rgba32_policy` は `0xRRGGBBAA` を格納します。

`rgb24_policy` と `bgr24_policy` は3つの8ビット成分を格納し、アルファ値は保持しません。これらのポリシーを通じて読み出すと、アルファ値が `0xff` の論理ピクセルが返ります。

---

## `canvas`

主要なキャンバス型は次のとおりです。

```cpp
template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;
```

固定サイズキャンバスが主モデルです。初期用途がVRAMエミュレーションのようなフレームバッファ風の処理だからです。

例:

```cpp
using screen = xer::image::canvas<256, 192>;
using sprite = xer::image::canvas<16, 16>;
using rgba_screen = xer::image::canvas<256, 192, xer::image::rgba32_policy>;
```

動的サイズキャンバスは次の形で表します。

```cpp
canvas<0, 0, Policy>
```

便利な別名は次のとおりです。

```cpp
template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;
```

有効な寸法指定は次の2種類だけです。

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

`canvas<0, 192>` や `canvas<256, 0>` のように片方だけ動的な寸法は無効です。

---

## 公開ピクセルアクセス

公開ピクセルAPIは論理ピクセルを使います。

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto get_pixel(const point& p) const noexcept -> pixel;
auto set_pixel(int x, int y, pixel value) noexcept -> void;
auto set_pixel(const point& p, pixel value) noexcept -> void;
auto set_pixel(int x, int y, pixel value, float coverage) noexcept -> void;
auto set_pixel(const point& p, pixel value, float coverage) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value,
                         float coverage) noexcept -> void;
```

`canvas::at()` は意図的に提供していません。

物理ストレージ要素への参照を返すとフレームバッファレイアウトを露出してしまいます。また、ストレージポリシーがARGBでない場合には不正確です。`pixel` は論理値であり、`Policy::storage_type` は物理値です。

`get_pixel` は、座標がキャンバス内にあることを期待します。

`set_pixel` は符号付き座標を受け取り、座標がキャンバス境界外の場合は何もしません。

coverage付きオーバーロードは、元ピクセルを先ピクセルの上にブレンドします。coverage は `[0.0f, 1.0f]` に丸められます。`0.0f` は先ピクセルを変更しません。`1.0f` は元ピクセルのアルファ値を通常どおり適用します。

`set_pixel_unchecked` は境界チェックを行いません。呼び出し側は `x < width()` かつ `y < height()` を保証しなければなりません。これは、内側の描画ループの外側でクリッピングや境界チェックを済ませたコード向けです。

---

## 基本メンバー関数

`canvas` は基本的なサイズ取得とユーティリティ操作を提供します。

```cpp
auto width() const noexcept -> std::size_t;
auto height() const noexcept -> std::size_t;
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto contains(int x, int y) const noexcept -> bool;
auto contains(const point& p) const noexcept -> bool;
auto fill(pixel value) noexcept -> void;
auto clear() noexcept -> void;
```

`clear()` はキャンバスを不透明な黒で塗りつぶします。

---

## ビットマップフォント型

`<xer/image.h>` は、XBFファイルから読み込んだ等幅ビットマップフォントのコンパクトな実行時表現を定義します。

```cpp
enum class bitmap_glyph_width : std::uint8_t {
    half,
    full,
};

struct bitmap_font_range {
    char32_t first_code_point {};
    char32_t last_code_point {};
    bitmap_glyph_width glyph_width = bitmap_glyph_width::half;
    std::uint64_t bitmap_offset = 0;
};

struct bitmap_font {
    int half_width = 0;
    int full_width = 0;
    int glyph_height = 0;
    std::vector<bitmap_font_range> ranges {};
    std::vector<std::uint8_t> bitmap {};
};

struct text_draw_options {
    int letter_spacing = 0;
    int line_spacing = 0;
};
```

`bitmap_font` は次を格納します。

- 半角セル幅
- 全角セル幅
- フォント全体で共有されるグリフ高さ
- ソート済みで重なりのないUnicodeコードポイント範囲
- パックされた1bppグリフビットマップバイト列

各範囲は半角セルまたは全角セルのいずれかを選択します。幅の種類はUnicodeコードポイントから推測せず、フォントデータに格納します。

`text_draw_options` は `draw_text` の呼び出しごとのレイアウト制御です。

- `letter_spacing` は描画された各グリフセルの後に加算されます
- `line_spacing` は改行処理時に `glyph_height` へ加算されます

負の間隔値も許可され、グリフセルが重なる場合があります。

---

## ビットマップフォント読み込み

```cpp
[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;
```

`bitmap_font_load` はXBFビットマップフォントファイルを読み込み、検証済みの `bitmap_font` を返します。

XBFは xer のコンパクトなバイナリビットマップフォント形式です。XBFは次を格納します。

- リトルエンディアンの数値フィールド
- 等幅の半角・全角グリフセル
- 共通のグリフ高さ
- Unicodeコードポイント範囲
- パックされた1bppビットマップデータ

ローダーは、成功を返す前に、XBFヘッダー、範囲テーブル、ビットマップ範囲、予約フィールド、コードポイント範囲、および関連オフセットを検証します。

### エラー

XBFの解析開始前にファイルI/Oが失敗した場合、`bitmap_font_load` は下位のファイル関連エラーコードを保持し、理由が `parse_error_reason::none` の空の `parse_error_detail` を返します。

XBFバイト列が不正な場合は、`parse_error_detail` とともに `error_t::invalid_argument` を返します。

XBFでは次のように扱います。

- `offset` はバイナリ入力先頭からのバイトオフセット
- `line` は `0`
- `column` は `0`

XBFローダーは、たとえば次の理由を報告する場合があります。

- `parse_error_reason::invalid_magic`
- `parse_error_reason::unsupported_version`
- `parse_error_reason::invalid_header`
- `parse_error_reason::invalid_range`
- `parse_error_reason::invalid_offset`
- `parse_error_reason::truncated_input`

共通の解析詳細モデルとXBF方針については、`header_parse.md` と `policy_bitmap_font.md` を参照してください。

---

## 描画関数

初期の描画関数は単純なフレームバッファヘルパーです。

```cpp
draw_hline
draw_vline
draw_line
draw_line_aa
draw_rect
fill_rect
```

整数描画座標には `std::size_t` ではなく `int` を使います。描画では負の座標をクリッピングできる方が便利なことが多いためです。

描画操作はキャンバス境界でクリッピングされます。対象領域が完全にキャンバス外にある場合は何も描画しません。

クリッピング後、`draw_hline`、`draw_vline`、`fill_rect` はフレームバッファストレージへ直接書き込みます。各ピクセルごとに `set_pixel` を呼びません。これにより、内側ループを座標からオフセットへの繰り返し計算ではなく、単純なポインタまたはストライド加算にできます。

`draw_line` は単純なBresenham風の整数直線アルゴリズムを使います。生成された各点についてキャンバス境界の確認は行いますが、その確認後は `set_pixel_unchecked` で書き込みます。

`draw_line_aa` は浮動小数点のピクセル中心座標を使い、アンチエイリアス付きのカプセル状ストロークを描画します。幅引数のないオーバーロードは1ピクセル幅のアンチエイリアス直線を描きます。幅付きオーバーロードでは、幅引数は色引数の前に置かれます。`pointf` オーバーロードはスカラー座標オーバーロードと等価です。

`draw_line_aa` は `xer::result<void>` を返します。いずれかの座標が有限でない場合、または `width` が有限でないか0以下の場合、`error_t::invalid_argument` を返します。完全にキャンバス外にある直線は成功したno-opです。

`draw_rect` と `fill_rect` のオーバーロードは、`point` と `size` の組、または単一の `rect` を受け取ります。すでに座標値を個別に持っている呼び出し側のために、スカラー座標オーバーロードも残されています。

---

## ビットマップテキスト描画

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             int x,
                             int y,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             const point& origin,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;
```

`draw_text` は、読み込み済みの `bitmap_font` を使ってUTF-8テキストをキャンバスへ描画します。

原点は最初のグリフセルの左上位置です。ベースライン指向の配置は、初期ビットマップフォントAPIには意図的に含めていません。

### レイアウト規則

- 通常のグリフは読み込み済みビットマップデータから描画されます
- グリフを描画した後、ペン位置はグリフセル幅と `letter_spacing` の分だけ進みます
- `\n`、`\r`、`\r\n` は新しい行を開始します
- 改行は x 位置を元の行原点に戻します
- 改行は y 位置を `glyph_height + line_spacing` だけ進めます
- フォントに存在しないコードポイントは、描画せず、ペンも進めずにスキップされます

欠落グリフ規則は意図的に最小限です。初期APIはフォールバック幅を推測したり、`?` を自動的に代用したりしません。

### クリッピング

描画はキャンバス境界でクリッピングされます。グリフはキャンバス外から開始してもよく、見えているセットピクセルだけが書き込まれます。

### エラー

`draw_text` は次を返します。

- `text` が妥当なUTF-8でない場合は `error_t::encoding_error`
- 指定された `bitmap_font` が要求グリフに対して構造的に使用不能な場合は `error_t::invalid_argument`

空のキャンバスまたは空のテキストは成功したno-opです。

---

## 円、楕円、円弧の描画

曲線形状APIは、整数の1ピクセル描画と浮動小数点のアンチエイリアス描画に分かれます。

- 整数APIは `point` またはスカラー `int` 中心座標を使います
- アンチエイリアスAPIは `pointf` またはスカラー `float` 中心座標を使います
- アンチエイリアス輪郭APIは省略可能な `width` を受け取ります
- すべての曲線形状関数は `[[nodiscard]]` なしで `xer::result<void>` を返します

### 円描画

`draw_circle` は、クリッピングされた1ピクセル幅の円輪郭を描画します。`fill_circle` はクリッピングされた円の内部を塗りつぶし、境界も含みます。

`draw_circle_aa` はアンチエイリアス付き輪郭を描画します。幅引数のないオーバーロードは `1.0f` を使います。幅付きオーバーロードは太い円輪郭に対応します。`fill_circle_aa` は外側境界をアンチエイリアスしながら円を塗りつぶします。

半径の扱いは次のとおりです。

- 負の半径は `error_t::invalid_argument` を返します
- 整数半径0は、見えていれば中心ピクセルだけを書き込みます
- アンチエイリアス輪郭の半径0は、直径が `width` に従う丸い点を描画します
- アンチエイリアス塗りつぶしの半径0は中心点を描画します

アンチエイリアス円描画では、中心座標、半径、幅は有限でなければなりません。`width` は0より大きくなければなりません。

### 楕円描画

`draw_ellipse` は、クリッピングされた1ピクセル幅の楕円輪郭を描画します。`fill_ellipse` はクリッピングされた楕円の内部を塗りつぶし、境界も含みます。

`draw_ellipse_aa` はアンチエイリアス付き楕円輪郭を描画します。幅引数のないオーバーロードは `1.0f` を使います。幅付きオーバーロードは太い楕円輪郭に対応します。`fill_ellipse_aa` は外側境界をアンチエイリアスしながら楕円を塗りつぶします。

半径の扱いは次のとおりです。

- いずれかの半径が負なら `error_t::invalid_argument` を返します
- 両方の整数半径が0なら、見えていれば中心ピクセルだけを書き込みます
- 片方の整数半径だけが0なら、縦線または横線へ退化します
- アンチエイリアス楕円でも同じ退化形を扱います

アンチエイリアス楕円描画では、中心座標、半径、幅は有限でなければなりません。`width` は0より大きくなければなりません。

### 円弧描画

円弧APIの角度は τrad 単位で表します。`0` は右方向を指します。正の sweep 角は数学的な意味で反時計回りに進みます。画像の y 座標は下向きに増えるため、点の式は次のようになります。

概念的には次の座標式です。

```text
x = cx + radius * cos(angle * τ)
y = cy - radius * sin(angle * τ)
```

`sweep_angle` が正の場合は反時計回り、負の場合は時計回りに描画します。絶対値が1回転以上の sweep は完全な円として扱われます。sweep が0の場合は開始点を描画します。

整数円弧描画は負の半径と有限でない角度を拒否します。アンチエイリアス円弧描画は、有限でない中心座標、半径、幅も拒否し、`width <= 0.0f` も拒否します。

### 楕円弧描画

楕円弧の角度は円弧と同じ規則に従います。

```text
x = cx + radius_x * cos(angle * τ)
y = cy - radius_y * sin(angle * τ)
```

1回転以上の sweep は完全な楕円として扱われます。sweep が0の場合は開始点を描画します。両方の半径が0の場合は中心点になります。片方の半径だけが0の場合は、角度に基づくパラメータ化を保ったまま、対応する縦線または横線へ退化します。

アンチエイリアス楕円弧は丸い端点を使い、`width` による太線に対応します。

楕円弧描画は負の半径と有限でない角度を拒否します。アンチエイリアス楕円弧描画は、有限でない中心座標、半径、幅も拒否し、`width <= 0.0f` も拒否します。

### クリッピング、ピクセル、返却値

すべての曲線形状描画はキャンバス境界でクリッピングされます。形状が完全にキャンバス外にある場合は成功したno-opです。

整数の円・楕円描画は、指定された論理 `pixel` を直接書き込みます。アンチエイリアス描画は canvas ピクセルAPIを通じて coverage ブレンドを使います。

これらの描画関数は `xer::result<void>` を返しますが、返却値には意図的に `[[nodiscard]]` を付けていません。これにより、描画コードでの呼び出しを軽く保ちつつ、必要な場所では不正引数を扱えます。

---

## 塗りつぶし

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;
```

`flood_fill` は、開始位置を含む4近傍連結領域を置換します。

開始位置の元の論理 `pixel` 値が対象色として使われます。その元の色と論理ARGB値が完全一致する到達可能なすべてのピクセルが `color` に置換されます。

### 連結性

初期実装は4近傍のみを使います。

- 左
- 右
- 上
- 下

斜めに接しているだけでは、2つの領域は連結しているとはみなしません。

### no-opの場合

`flood_fill` は次の場合に成功したno-opです。

- 開始位置がキャンバス外にある
- 置換色が開始位置の元の色と等しい

### 結果

`flood_fill` は `xer::result<void>` を返します。

この操作は再帰探索ではなく、内部の保留位置バッファを使います。そのため、大きな塗りつぶし領域でもコールスタックの深さに依存しません。

---

## 画像処理関数

`mosaic`、`box_blur`、`filter_pixels` はインプレース画像処理操作です。

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;
```

3つの関数はいずれも `area` をキャンバス境界でクリッピングします。空領域や完全にクリップされた領域は成功したno-opです。

`mosaic` は、クリップ後の領域を `block_size` のブロックに分割します。各ブロックは、そのブロック内ピクセルの平均論理ARGB色で置換されます。右端と下端のブロックは、実際にクリップされたサイズを使います。

`box_blur` は `box_size` を平均化カーネルサイズとして扱います。たとえば `size(3, 3)` は各出力ピクセルの周囲に3x3平均を適用します。ソースサンプルは、クリップされた対象領域内の元ピクセルのコピーから取得されるため、要求領域外のピクセルは結果に影響しません。クリップ領域外に出るカーネル部分は無視されます。

偶数のカーネル寸法にも対応します。この場合、余分なサンプルは現在ピクセルの左側または上側に置かれます。

`mosaic` と `box_blur` は、いずれかのサイズ寸法が正でない場合に `error_t::invalid_argument` を返します。

`filter_pixels` は、呼び出し側が指定したピクセル単位フィルターをクリップ後領域に適用します。各ピクセルについて、フィルターは現在の論理 `pixel` 値を受け取り、置換する論理 `pixel` 値を返します。これにより、グレースケール変換、しきい値処理、チャンネル調整、反転などを、効果ごとの専用関数を増やさずに実現できます。

操作はインプレースで、画像全体の一時バッファは確保しません。フィルターがあるピクセルで例外を投げた場合、そのピクセルは変更されず、処理は次のピクセルへ進みます。1つ以上のピクセルで失敗した場合、関数は `filter_pixels_error_detail` 付きの `error_t::user_error` を返します。正常にフィルターされたピクセルは更新されたままです。

グレースケール風の使用例:

```cpp
auto result = xer::image::filter_pixels(
    img,
    xer::image::rect(xer::image::point(0, 0), xer::image::size(16, 16)),
    [](xer::image::pixel p) -> xer::image::pixel {
        const auto gray = static_cast<std::uint8_t>(
            (static_cast<unsigned>(p.red()) +
             static_cast<unsigned>(p.green()) +
             static_cast<unsigned>(p.blue())) / 3u);

        return xer::image::pixel(p.alpha(), gray, gray, gray);
    });
```

---

## Tcl/Tkとの関係

`<xer/image.h>` はTcl/Tkに依存しません。

Tk photo ブリッジ関数は `<xer/tk.h>` に置くべきです。将来的には、Tk photo image block と `xer::image::canvas` または `xer::image::dynamic_canvas` の間の変換を提供する可能性がありますが、純粋な画像ストレージ、描画、画像処理は `<xer/image.h>` に残ります。

---

## 後回しにしている項目

現在の実装では、次の項目を後回しにしています。

- アフィン変換
- ラスタスクロール
- グレースケール変換
- 画像反転
- ファイル形式の読み込みと保存
- 直接的なTk photo変換ヘルパー

これらは、基本的なフレームバッファ型が安定してから追加できます。

---

## 例

```cpp
#include <xer/image.h>
#include <xer/stdio.h>

auto main() -> int
{
    xer::image::canvas<4, 4> img;

    img.clear();

    // This line intentionally starts outside the canvas.
    // xer clips it to the framebuffer boundary.
    xer::image::draw_hline(
        img,
        -2,
        1,
        4,
        xer::image::pixel(0xffu, 0x00u, 0x00u));

    const auto value = img.get_pixel(0, 1);
    return value.argb == 0xffff0000u ? 0 : 1;
}
```

追加の例:

- `examples/example_image_basic.cpp`
- `examples/example_image_geometry_io.cpp`
- `examples/example_image_effects.cpp`
- `examples/example_image_filter_pixels.cpp`
- `examples/example_image_bitmap_text.cpp`
- `examples/example_image_flood_fill.cpp`
- `examples/example_image_circle.cpp`
- `examples/example_image_curves.cpp`

---

## 関連項目

- `header_iostream.md`
- `header_parse.md`
- `policy_image.md`
- `policy_bitmap_font.md`
