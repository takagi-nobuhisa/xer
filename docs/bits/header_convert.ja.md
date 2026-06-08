<!-- xer-reference-source-sha256: c07630b8520ed7e0714205213f577a9c71a74711a6f4449ad13b5174011fefc7 -->

# `<xer/convert.h>`

## 目的

`<xer/convert.h>` は、汎用変換関数 `xer::to<T>` を提供します。

C++ のキャスト式とは異なり、`xer::to<T>` はテキストの解析、値の整形、数値範囲の検証、一部の xer 値型の正規化を行うことがあり、失敗する可能性があります。そのため、戻り値は `xer::result<T>` です。

---

## 主な役割

このヘッダーは、要求された変換が単純な C++ キャストを超える場合のために、実用的な変換の入口を1つ提供します。

```cpp
template <class To, class From>
auto to(From&& from) -> xer::result<To>;
```

典型的な使い方は次のとおりです。

```cpp
auto n = xer::to<int>(u8"123");
auto s = xer::to<std::u8string>(123);
auto p = xer::to<xer::path>(u8"dir\\file.txt");
```

---

## 基本規則

`xer::to<T>` は、次の規則を順に適用します。

1. 変換元と変換先の値型が同じ場合は、その値をそのまま返す。
2. 数値型同士の変換では、変換前に `xer::in_range` で範囲確認を行う。
3. `signed char` と `unsigned char` は整数型として扱う。
4. `char` は数値型ではなく文字型として扱う。
5. 明示的なエンコーディングを持つ文字列は、解析、整形、または文字コード変換の対象にできる。
6. 曖昧なナロー `char` 文字列はテキストとして解釈しない。
7. 非対応または安全でない変換は `error_t::invalid_argument` を返す。

---

## 数値変換

```cpp
auto a = xer::to<int>(u8"123");
auto b = xer::to<unsigned char>(u8"255");
auto c = xer::to<double>(u8"3.5");
```

文字列から数値への変換では、次の文字型に基づく、明示的にエンコードされた文字列を受け付けます。

```cpp
char8_t
char16_t
char32_t
wchar_t
```

例:

```cpp
xer::to<int>(u8"123");
xer::to<int>(u"123");
xer::to<int>(U"123");
xer::to<int>(L"123");
```

数値変換では、入力全体が消費されなければなりません。たとえば、`u8"123x"` は拒否されます。

数値型同士の変換では、変換元の値が変換先の型で表現可能な範囲に収まるかどうかを確認します。

```cpp
xer::to<unsigned char>(255); // 成功
xer::to<unsigned char>(256); // error_t::range
xer::to<unsigned>(-1);       // error_t::range
```

---

## 文字変換

`char` は文字型として扱います。

```cpp
auto s = xer::to<std::u8string>('A'); // u8"A"
auto c = xer::to<char>(u8"A");       // 'A'
```

`char` は数値としては扱いません。

```cpp
xer::to<int>('A'); // error_t::invalid_argument
```

`signed char` と `unsigned char` は数値です。

```cpp
xer::to<int>(static_cast<signed char>(-5));
xer::to<int>(static_cast<unsigned char>(250));
```

---

## テキスト変換

次の所有権を持つ変換先文字列型に対応します。

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

例:

```cpp
auto u8  = xer::to<std::u8string>(123);
auto u16 = xer::to<std::u16string>(u8"あ");
auto u32 = xer::to<std::u32string>(u8"あ");
auto w   = xer::to<std::wstring>(u8"あ");
```

`wchar_t` 文字列は、xer が対象とするプラットフォームに従って扱います。

- 16ビットの `wchar_t` は UTF-16 として扱う。
- 32ビットの `wchar_t` は UTF-32 として扱う。

---

## ナロー `char` 文字列

`char` 文字列は、`xer::to<T>` では意図的にテキストとして解釈しません。

```cpp
xer::to<int>("123");                 // error_t::invalid_argument
xer::to<std::u8string>("abc");       // error_t::invalid_argument
xer::to<xer::path>("dir/file.txt");  // error_t::invalid_argument
```

これは、プラットフォーム依存のナロー文字列を UTF-8、ロケール依存テキスト、またはネイティブパステキストとして暗黙に扱うことを避けるためです。

かわりに、明示的にエンコードされた文字列を使います。

```cpp
xer::to<int>(u8"123");
xer::to<std::u8string>(u8"abc");
xer::to<xer::path>(u8"dir/file.txt");
```

---

## パス変換

`xer::path` は、明示的にエンコードされたテキストから作成できます。

```cpp
auto p1 = xer::to<xer::path>(u8"dir\\file.txt");
auto p2 = xer::to<xer::path>(u"dir\\file.txt");
auto p3 = xer::to<xer::path>(U"dir\\file.txt");
auto p4 = xer::to<xer::path>(L"dir\\file.txt");
```

結果の `xer::path` は正規化済み UTF-8 テキストを保持し、通常の `xer::path` コンストラクタを通じてバックスラッシュをスラッシュに正規化します。

---

## 例

```cpp
#include <xer/convert.h>
#include <xer/path.h>
#include <xer/stdio.h>

int main()
{
    auto count = xer::to<int>(u8"42");
    auto path = xer::to<xer::path>(u8"data\\out.txt");
    auto text = xer::to<std::u8string>(3.5);

    if (!count || !path || !text) {
        return 1;
    }

    xer::printf(u8"count = %@\n", *count);
    xer::printf(u8"path = %s\n", path->str().data());
    xer::printf(u8"text = %s\n", text->c_str());
    return 0;
}
```
