<!-- xer-reference-source-sha256: a764ff093e95a1ff8188645c9cea67b538492b4de8c6729227c23b511ad2019e -->

# `<xer/tk.h>`

## 目的

`<xer/tk.h>` は、xer の初期 Tcl/Tk 連携層を提供します。

このヘッダーの第一の目的は、C++コードから Tcl インタープリターを作成・制御し、C++の呼び出し可能オブジェクトを Tcl コマンドとして登録し、その基盤を Tk ベースの GUI 機能に利用できるようにすることです。

この設計では、意図的に `Tk_Main` を避けています。`Tk_Main` はプロセス寿命の多くを所有し、main 関数が終了するとプロセス全体を終了させる可能性があります。xer ではその代わりに、インタープリター作成、初期化、スクリプト評価、コマンド登録、変数アクセス、イベントループ補助を通常の C++ API として公開します。

---

## 主なエンティティ

少なくとも、`<xer/tk.h>` は次のエンティティを提供します。

```cpp
namespace xer::tk {

using result_code_t = int;
using eval_flag_t = int;
using var_flag_t = int;
using event_flag_t = int;

inline constexpr result_code_t result_ok;
inline constexpr result_code_t result_error;
inline constexpr result_code_t result_return;
inline constexpr result_code_t result_break;
inline constexpr result_code_t result_continue;

inline constexpr eval_flag_t eval_direct;

inline constexpr var_flag_t var_none;
inline constexpr var_flag_t var_global_only;
inline constexpr var_flag_t var_namespace_only;
inline constexpr var_flag_t var_leave_error_msg;
inline constexpr var_flag_t var_append_value;
inline constexpr var_flag_t var_list_element;

inline constexpr event_flag_t event_all;
inline constexpr event_flag_t event_window;
inline constexpr event_flag_t event_file;
inline constexpr event_flag_t event_timer;
inline constexpr event_flag_t event_idle;
inline constexpr event_flag_t event_dont_wait;

struct error_detail;
class interpreter;
class obj;
class photo_image;

using photo_composite_rule_t = int;
using photo_image_block = Tk_PhotoImageBlock;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

struct photo_size {
    int width;
    int height;
};

auto find_executable() -> xer::result<void, error_detail>;
auto init(interpreter& interp) -> xer::result<void, error_detail>;
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;

auto eval(interpreter& interp, std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;

auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;

auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;

auto photo_get_size(photo_image image) noexcept -> photo_size;
auto photo_blank(photo_image image) noexcept -> void;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

template <class F>
auto main(F&& callback) -> xer::result<void>;

auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;

}
```

Tcl/Tk 層の発展に伴い、対応する呼び出し可能オブジェクトの引数型と戻り値型の正確な集合は拡張される可能性があります。

---

## 名前空間

公開 Tcl/Tk 連携 API はすべて次の名前空間に配置されます。

```cpp
namespace xer::tk
```

初期公開 API では、独立した `xer::tcl` 名前空間は提供しません。Tcl は Tk 連携層で使われるスクリプト基盤として扱います。

将来のバージョンで Tcl 専用の公開ヘッダーを提供する場合は、その時点で独立した名前空間を再検討できます。

---

## 結果コードとフラグ

Tcl は `TCL_OK`、`TCL_ERROR`、`TCL_GLOBAL_ONLY` などの C マクロを通じて結果コードとフラグを公開します。

xer では、通常の利用者がそれらのマクロを直接参照する必要はありません。代わりに、`<xer/tk.h>` は次のような xer 名の定数を提供します。

```cpp
xer::tk::result_ok
xer::tk::result_error
xer::tk::result_break
xer::tk::var_global_only
xer::tk::eval_direct
xer::tk::event_all
```

これらの定数は、Tcl/Tk が使用するネイティブの整数値を保ちながら、公開 API の語彙を `xer::tk` の下に置きます。

---

## エラー詳細

```cpp
struct error_detail {
    result_code_t result_code;
};
```

`error_detail` は、失敗した操作に関連付けられた Tcl/Tk の結果コードを保持します。

`eval` では、保存されるコードは `Tcl_EvalObjEx` が `result_ok` 以外を返したときの戻り値です。null の Tcl 戻り値によって失敗する操作では、`result_error` が使用されます。

現在の Tcl 結果文字列は `error_detail` の内部には保存されません。呼び出し側が現在のインタープリター結果テキストを必要とする場合は、次を呼び出せます。

```cpp
auto get_result(interpreter& interp) -> std::u8string;
```

これにより、エラー詳細を小さく保ち、実際に必要な場合以外は Tcl 結果テキストのコピーを避けられます。

---

## `interpreter`

```cpp
class interpreter;
```

`interpreter` は、`Tcl_Interp*` を包むムーブ専用の RAII ラッパーです。

### 作成

```cpp
auto interpreter::create() -> xer::result<interpreter, error_detail>;
```

`interpreter::create` は、必要であれば Tcl 実行可能ファイル情報を初期化し、その後で新しい Tcl インタープリターを作成します。

通常のコードでは、この静的メンバー関数を通じてインタープリターを作成するべきです。

```cpp
auto interp = xer::tk::interpreter::create();
```

### 寿命

インタープリターは、ネイティブの `Tcl_Interp*` ハンドルを所有します。

`interpreter` オブジェクトが破棄されると、ネイティブインタープリターも削除されます。この型はムーブ専用であり、所有権が明示的に保たれるようにしています。

### 妥当性

```cpp
auto valid() const noexcept -> bool;
```

`valid()` は、ラッパーが現在ネイティブインタープリターハンドルを所有しているかどうかを返します。

---

## 初期化

```cpp
auto init(interpreter& interp) -> xer::result<void, error_detail>;
```

`init` は、指定されたインタープリターに対して Tcl と Tk の両方を初期化します。

この関数は `Tcl_Init` を呼び出し、その後で `Tk_Init` を呼び出します。xer は、`xer::tk` 層では `Tcl_Init` だけを呼び出す公開 API を意図的に提供していません。

呼び出し側が Tcl のみの初期化動作を直接必要とする場合は、`to_native_handle` でネイティブハンドルを取得し、Tcl C API を明示的に呼び出せます。

---

## ネイティブハンドルへの脱出口

```cpp
auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
```

`to_native_handle` は、背後にある `Tcl_Interp*` を返します。

この関数は、Tcl/Tk C API を直接使うための脱出口です。`get` のような通常のアクセサ名には意図的にしていません。ネイティブハンドルを使うと、xer の抽象化の一部を迂回するためです。

`const interpreter&` 用のオーバーロードはありません。Tcl インタープリターハンドルは状態を変更する操作で使われることが多く、const オーバーロードを用意しても意味のある const 安全性は得られません。

---

## Photo Image 補助

`<xer/tk.h>` は、`Tk_FindPhoto`、`Tk_PhotoGetImage`、`Tk_PhotoPutBlock`、および関連するサイズ操作など、Tk photo image API の薄いラッパーを提供します。

これらの補助は、一般的な画像処理層ではなく、意図的に Tk 連携層の一部です。純粋な画像処理とフレームバッファ操作は `<xer/image.h>` に属し、`<xer/tk.h>` は Tk photo image への橋渡しだけを扱います。

### `photo_image`

```cpp
class photo_image;
```

`photo_image` は、既存の Tk photo image に対する非所有ハンドルです。

`photo_image` 値はデフォルト構築できません。これは `find_photo` によってのみ作成されるため、通常の xer コードでは null photo ハンドルを表しません。

この型は背後の Tk image を所有しません。呼び出し側は、その `photo_image` ハンドルが参照する Tcl/Tk photo image の寿命がハンドルより長く続くことを保証しなければなりません。Tcl/Tk 側で image が削除された場合、既存の `photo_image` はネイティブ Tk の寿命規則により無効になります。

ネイティブへの脱出口は次のとおりです。

```cpp
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;
```

### Photo Image の検索

```cpp
auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;
```

`find_photo` は、既存の Tk photo image を名前で検索します。

`name` 引数は、ヌル終端 UTF-8 文字列です。これは、`const char*` の image 名を受け取るネイティブ Tk API に従っています。`eval` と違い、この関数は `std::u8string_view` を受け取りません。ネイティブ API が明示的な長さを持つオブジェクトではなく、ヌル終端文字列を要求するためです。

`name` が null の場合、この関数は `error_t::invalid_argument` で失敗します。指定された名前の image が存在しない、または photo image ではない場合、この関数は `error_t::not_found` で失敗します。

### サイズ操作

```cpp
struct photo_size {
    int width;
    int height;
};

auto photo_get_size(photo_image image) noexcept -> photo_size;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;
```

`photo_get_size` は現在の Tk photo サイズを返します。

`photo_expand` は、photo image を少なくとも要求されたサイズまで拡張します。Tcl/Tk 側で photo image に明示的な `-width` または `-height` が設定されている場合、Tk はその明示的な寸法を拡張せず保持することがあります。

`photo_set_size` は、photo image の明示的なサイズを設定します。両方の寸法に 0 を渡すと、ネイティブ Tk API と同じ方法で明示的なサイズを解除するために使えます。

負の寸法は、Tk を呼び出す前に `error_t::invalid_argument` として拒否されます。

### クリア操作とブロック操作

```cpp
using photo_image_block = Tk_PhotoImageBlock;
using photo_composite_rule_t = int;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

auto photo_blank(photo_image image) noexcept -> void;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;
```

`photo_blank` は photo image を消去します。

`photo_get_image` は、現在の Tk photo image に対する `photo_image_block` 記述子を埋めます。このブロックは Tk が管理するメモリを指すため、Tk の寿命規則に従って扱う必要があります。

`photo_put_block` は、ブロックを photo image に書き込みます。

`photo_put_zoomed_block` は、整数ズームとサブサンプリングを適用しながらブロックを書き込みます。

ブロックポインターは null であってはいけません。座標、幅、高さは負であってはいけません。ズーム係数とサブサンプル係数は正でなければなりません。不正な引数は、Tk を呼び出す前に拒否されます。

### xer 画像機能との関係

photo block 補助は低水準の Tk ラッパーです。必要なときにコードがネイティブ Tk のブロックレイアウトを使えるように、`Tk_PhotoImageBlock` を `photo_image_block` という別名で意図的に公開しています。

Tk photo image と xer の image 型またはフレームバッファ型の間の高水準変換は、これらの補助の上に別途構築するべきです。通常の画像処理アルゴリズム自体は、Tcl/Tk に依存するべきではありません。

---

## Tcl オブジェクト

```cpp
class obj;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
```

`obj` は、`Tcl_Obj*` に対する RAII ラッパーです。ラップされたオブジェクトに対する Tcl 参照を 1 つ所有します。構築とコピーでは Tcl 参照カウントを増やし、破棄では参照カウントを減らし、ムーブでは Tcl 参照カウントを変えずに所有参照を移動します。

`obj` には、`Tcl_Obj*` からの公開コンストラクターはありません。通常のコードでは、`make_string_obj`、`make_int_obj`、`make_list_obj` などのファクトリ関数を通じてオブジェクトを作成するべきです。

`to_native_handle(obj&)` は借用された `Tcl_Obj*` を返し、Tcl 参照カウントを変更しません。返されたポインターを呼び出し側が解放してはいけません。

`make_list_obj` は本物の Tcl リストオブジェクトを作成します。値を手作業で結合した文字列ではなく Tcl リストとして振る舞わせる必要がある場合に有用です。

---

## Tcl 結果の扱い

```cpp
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;
```

`get_result` は、現在の Tcl インタープリター結果を UTF-8 テキストとして返します。内部では、`Tcl_GetObjResult` で結果オブジェクトを取得し、`Tcl_GetStringFromObj` でテキストに変換します。

`reset_result` は、現在のインタープリター結果をクリアします。

---

## スクリプト評価

```cpp
auto eval(interpreter& interp,
          std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;
```

`eval` は、`Tcl_EvalObjEx` を使って Tcl スクリプトテキストを評価します。

スクリプトはまず Tcl オブジェクトに変換されます。成功時には、現在の Tcl 結果が `std::u8string` として返されます。失敗時には、返されるエラー詳細が Tcl 結果コードを保持します。

既定の評価フラグは `eval_direct` です。

---

## 変数アクセス

`<xer/tk.h>` は、Tcl オブジェクト API に基づく変数アクセスを提供します。

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

これらの関数は、`Tcl_ObjSetVar2` と `Tcl_ObjGetVar2` を使用します。

UTF-8 テキスト値に加えて、`set_var` は `xer::tk::obj` も受け取れます。これにより、呼び出し側はリストオブジェクトなどの Tcl オブジェクトを、手作業で結合した文字列に変換せずに代入できます。

配列変数形式も提供されます。

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

既定の変数フラグは `var_global_only` です。

たとえば、Tcl リストは次のように代入できます。

```cpp
auto list = xer::tk::make_list_obj({u8"first value", u8"second"});
if (!list.has_value()) {
    return 1;
}

if (!xer::tk::set_var(interp, u8"argv", *list).has_value()) {
    return 1;
}
```

その後 Tcl コードでは、`llength $argv` や `lindex $argv 0` などの通常のリスト操作を使用できます。

---

## コマンド登録

```cpp
template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;
```

`create_command` は、C++の呼び出し可能オブジェクトを Tcl オブジェクトコマンドとして登録します。

コマンド名は UTF-8 テキストです。呼び出し可能オブジェクトは、関数オブジェクト、関数ポインター、ラムダ式のいずれでも構いません。

簡単な例は次のとおりです。

```cpp
auto created = xer::tk::create_command(
    interp,
    u8"add",
    [](int a, int b) -> int {
        return a + b;
    });
```

登録後、Tcl スクリプトは次のように呼び出せます。

```tcl
add 10 20
```

初期コマンドブリッジは、実用的な一部の引数型と戻り値型をサポートします。

対応するコールバック引数型には次が含まれます。

- `Tcl_Obj*`
- `xer::tk::obj`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`

対応するコールバック戻り値型には次が含まれます。

- `void`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`
- `const char8_t*`
- `Tcl_Obj*`
- `xer::tk::obj`
- `xer::result<T, xer::tk::error_detail>`
- `xer::result<T>` のうち実装が対応するもの
- `xer::tk::result_code_t`

未対応の場合、利用者は `Tcl_Obj*` またはネイティブ Tcl/Tk API にフォールバックできます。

### `std::u8string_view` コールバック引数

C++コマンドコールバックが `std::u8string_view` を受け取る場合、そのビューは対応する `Tcl_Obj` の文字列表現を参照します。
このビューは、コールバックの実行中に限って有効です。

値を保存する、キャプチャする、あとで返す、またはコールバックが戻った後で使用する必要がある場合、コールバックはそれを `std::u8string` などの所有オブジェクトにコピーしなければなりません。

コールバックから `std::u8string_view` を返す場合は話が異なります。xer は、コマンドハンドラーが戻る前に、参照されたテキストを Tcl インタープリター結果へコピーします。
返されるビューは、コマンドハンドラーが Tcl 結果の設定を終えるまで有効であれば十分です。

---

## Tcl/Tk main 補助

```cpp
template <class F>
auto main(F&& callback) -> xer::result<void>;
```

`xer::tk::main` は Tcl/Tk 実行ブロックであり、C++のグローバル `main` 関数を置き換えるものではありません。これはプログラムのメインスレッドから呼び出しても、利用者が管理する別スレッドから呼び出しても構いません。

コールバックは、基本的に次の形を持つべきです。

```cpp
auto callback(xer::tk::interpreter& interp) -> xer::result<void>;
```

この補助は、通常のセットアップ手順を実行します。実行可能ファイルの検出、インタープリター作成、Tcl/Tk 初期化、Tcl 起動変数の設定、コールバック呼び出し、そしてコールバックが成功した場合の `main_loop()` です。イベントループが戻った後、インタープリターは RAII によって破棄されます。

この補助は、Tcl スクリプトが通常の名前を使えるように Tcl 変数を設定します。

- `argc`
- `argv`
- `argv0`
- `env`

`argv` は本物の Tcl リストオブジェクトとして設定されます。`env` は Tcl 配列変数として設定されます。

---

## イベントループ

```cpp
auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;
```

`main_loop` は、`Tk_MainLoop` によって Tk のメインイベントループを実行します。

`do_one_event` は、`Tcl_DoOneEvent` によってイベントを 1 つ処理します。

xer は `Tk_Main` を使用しません。`Tk_Main` はプロセス寿命を制御しすぎるためです。プログラムは、インタープリターを明示的に作成・初期化し、その後で望むスレッド上でイベントループを実行するべきです。

---

## スレッド方針

xer は、利用者が選んだスレッドで Tcl/Tk を実行できるようにすることを目指します。

ただし、インタープリターはスレッドに属するものとして扱います。通常は、同じスレッドで作成、初期化、使用、破棄するべきです。

この規則は、評価、変数アクセス、コマンド登録、そのインタープリターに関連するイベントループ使用、破棄などの通常操作に適用されます。

つまり、xer は Tcl/Tk をメインスレッド以外で実行するという考え方をサポートしますが、1 つのインタープリターを任意のスレッドから自由に呼び出せるようにはしません。

安全なパターンは、ワーカースレッド内でインタープリターを作成し、そのワーカースレッド内だけで使用し、そのスレッドが終了する前に破棄させることです。

xer は現在、スレッド間呼び出し補助、イベント投稿補助、Tcl/Tk 用のスレッド安全なディスパッチキューを提供していません。

---

## 後回しにしている項目

次の項目は、初期 Tcl/Tk 層では意図的に後回しにしています。

* widget ラッパークラス
* `%w` などのコールバック置換値
* スレッド間呼び出し補助
* 完全なイベントディスパッチ抽象化
* TomMath 連携
* Tcl オブジェクト型の完全な網羅
* Tcl 専用の公開ヘッダーと名前空間分離

---

## 例

```cpp
#include <xer/stdio.h>
#include <xer/tk.h>

auto main() -> int
{
    auto interp = xer::tk::interpreter::create();
    if (!interp.has_value()) {
        return 1;
    }

    const auto command = xer::tk::create_command(
        *interp,
        u8"add",
        [](int a, int b) -> int {
            return a + b;
        });
    if (!command.has_value()) {
        return 1;
    }

    const auto result = xer::tk::eval(*interp, u8"add 10 20");
    if (!result.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"%@\n", *result).has_value()) {
        return 1;
    }

    return 0;
}
```

この例では、Tcl インタープリターを作成し、C++の呼び出し可能オブジェクトを Tcl コマンドとして登録しています。GUI widget を使っていないため、Tk は初期化していません。

---

## 関連項目

* `policy_tk.md`
* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_cmdline.md`
