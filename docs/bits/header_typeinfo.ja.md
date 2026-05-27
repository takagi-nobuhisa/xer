<!-- xer-reference-source-sha256: fdd7bd71d881d69fdb368ea6db99f6d3031f4322cc104f4d228d4d24d9fcef4b -->
# `<xer/typeinfo.h>`

## 目的

`<xer/typeinfo.h>` は、xer の軽量な型情報ヘルパーを提供します。

主な目的は、診断、トレース、その他の開発向け出力で型名を使いやすくすることです。
このヘッダーは、標準C++の型情報を、xer のUTF-8指向のテキストモデルに合う形でラップします。

---

## 主な要素

少なくとも、`<xer/typeinfo.h>` は次の要素を提供します。

```cpp
class xer::type_info;

#define xer_typeid(...)
```

正確な実装では、`xer::type_info` を比較したり順序付きコンテナで使ったりできるように、内部で `std::type_index` を使う場合があります。

---

## `xer::type_info`

`xer::type_info` は、C++の実行時型情報に対する軽量なラッパーです。

少なくとも次の操作を提供します。

```cpp
auto raw_name() const noexcept -> const char*;
auto name() const -> std::u8string;
auto index() const noexcept -> std::type_index;
auto operator==(const type_info& rhs) const noexcept -> bool;
auto operator!=(const type_info& rhs) const noexcept -> bool;
auto operator<(const type_info& rhs) const noexcept -> bool;
```

### `name()`

`name()` は、人間が読む診断用のUTF-8型名を返します。

GCCでは、実装が提供する型名をデマングルする場合があります。
デマングルに失敗した場合は、実装が提供する生の名前をそのまま返す場合があります。

返される名前は表示と診断を目的としたものであり、安定したシリアライズやABI非依存の比較に使うためのものではありません。

---

## `xer_typeid`

`xer_typeid(...)` は、`typeid` が受け付けるものと同種のオペランドから `xer::type_info` オブジェクトを作成します。

これは可変長マクロなので、カンマを含むテンプレート型オペランドも自然に渡せます。

例:

```cpp
const auto a = xer_typeid(int);
const auto b = xer_typeid(std::pair<int, long>);
const auto c = xer_typeid(value);
```

---

## 設計上の役割

このヘッダーは、主に診断とトレースの基盤です。

特に、コードで次のものを表示できるようにします。

- トレース対象オブジェクトの型
- 実装が提供する型名またはデマングル済み型名
- ルックアップテーブル用の順序付き型キー

---

## 他のヘッダーとの関係

`<xer/typeinfo.h>` は、トレースなどの将来的な診断機能と関係します。

また、型名をUTF-8テキストとして出力する場合には、`<xer/stdio.h>` の書式付き出力機能と組み合わせても有用です。

---

## ドキュメント上の注意

型名は本質的に実装依存です。
したがって、ドキュメントでは `name()` を、安定したプログラム上の識別子ではなく、診断表示用の機能として説明するべきです。

---

## 例

```cpp
#include <xer/stdio.h>
#include <xer/typeinfo.h>

#include <utility>

auto main() -> int
{
    const auto info = xer_typeid(std::pair<int, long>);

    if (!xer::puts(info.name()).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

* `header_stdio.md`
