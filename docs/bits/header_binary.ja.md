<!-- xer-reference-source-sha256: c2065884ec49dea7236237091030fa120d3c3f9912a7f587aeb83996cd009dc7 -->

# `<xer/binary.h>`

## 目的

`<xer/binary.h>` は、小規模なバイナリデータ用ユーティリティ関数を提供します。

現在の対象範囲は意図的に狭くしています。このヘッダーは、固定幅符号なし整数の分割と合成、ビット順反転、xer の128ビット符号なし整数型に対するバイト順反転、単純なチェックサム計算、CRC計算、バイナリから16進文字列への変換、16進文字列からバイナリへの変換、バイト列とファイルに対する実用的なハッシュ計算を扱います。

これらの関数は、入力値を固定幅のバイナリ値として扱います。CPUのネイティブエンディアン設定には依存しません。

---

## 主なエンティティ

最低限、`<xer/binary.h>` は次の機能を提供します。

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};

using std::byteswap;

auto high_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto low_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto make_u16(std::uint8_t high, std::uint8_t low) noexcept -> std::uint16_t;

auto high_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto low_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto make_u32(std::uint16_t high, std::uint16_t low) noexcept -> std::uint32_t;

auto high_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto low_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto make_u64(std::uint32_t high, std::uint32_t low) noexcept -> std::uint64_t;
```

`xer::uint128_t` が利用できる場合、このヘッダーはさらに次を提供します。

```cpp
auto high_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto low_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto make_u128(std::uint64_t high, std::uint64_t low) noexcept -> xer::uint128_t;

auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

このヘッダーはビット順反転も提供します。

```cpp
auto reverse_bits(std::uint8_t value) noexcept -> std::uint8_t;
auto reverse_bits(std::uint16_t value) noexcept -> std::uint16_t;
auto reverse_bits(std::uint32_t value) noexcept -> std::uint32_t;
auto reverse_bits(std::uint64_t value) noexcept -> std::uint64_t;
```

`xer::uint128_t` が利用できる場合は次も提供されます。

```cpp
auto reverse_bits(xer::uint128_t value) noexcept -> xer::uint128_t;
```

単純なチェックサムについて、このヘッダーは8ビット、16ビット、32ビット形式の加算チェックサム、XORチェックサム、および便利な別名を提供します。また、CRC16とCRC32の計算ヘルパーも提供します。

バイナリ/テキスト変換について、このヘッダーは `bin2hex` と `hex2bin` を提供します。実用的なハッシュ計算について、このヘッダーは `md5`、`sha1`、`sha256` を提供します。

---

## 整数の分割と合成

`high_uN` 関数と `low_uN` 関数は、符号なし整数値の上位半分または下位半分を取り出します。

```cpp
auto high = xer::high_u8(std::uint16_t{0x1234}); // 0x12
auto low = xer::low_u8(std::uint16_t{0x1234});  // 0x34
```

`make_uN` 関数は、上位部分と下位部分から、より大きな符号なし整数値を合成します。

```cpp
auto value = xer::make_u16(0x12, 0x34); // 0x1234
```

命名規則は、取り出される部分のサイズを示します。

- `high_u8` と `low_u8` は16ビット値から8ビット部分を取り出します
- `high_u16` と `low_u16` は32ビット値から16ビット部分を取り出します
- `high_u32` と `low_u32` は64ビット値から32ビット部分を取り出します
- `high_u64` と `low_u64` は128ビット値から64ビット部分を取り出します

合成関数は、合成される結果のサイズを示します。

- `make_u16` は2つの8ビット値から16ビット値を合成します
- `make_u32` は2つの16ビット値から32ビット値を合成します
- `make_u64` は2つの32ビット値から64ビット値を合成します
- `make_u128` は2つの64ビット値から128ビット値を合成します

これらの関数は、メモリ上のオブジェクト表現ではなく、整数値に対して動作します。その振る舞いはネイティブエンディアンに依存しません。

---

## `byteswap`

`<xer/binary.h>` は `std::byteswap` を `xer` 名前空間に取り込みます。

```cpp
using std::byteswap;
```

これにより、C++23標準のバイトスワップを、`std::byteswap` が対応している標準の符号なし整数型に対して `xer::byteswap` として利用できます。

`xer::uint128_t` が利用できる場合、xer は128ビットの多重定義も提供します。

```cpp
auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

たとえば、上位半分と下位半分が次の128ビット値があるとします。

```text
0x0011223344556677_8899aabbccddeeff
```

これはバイトスワップによって次になります。

```text
0xffeeddccbbaa9988_7766554433221100
```

128ビット多重定義は、C++23の `std::byteswap` が xer の拡張 `uint128_t` 型を対象にしていないため提供されています。

---

## `reverse_bits`

`reverse_bits` は、固定幅符号なし整数値の全ビットの順序を反転します。

```cpp
auto value = xer::reverse_bits(std::uint8_t{0b0001'0010}); // 0b0100'1000
```

これはバイト順反転とは異なります。

```cpp
std::uint16_t value = 0x1234;

xer::byteswap(value);     // 0x3412
xer::reverse_bits(value); // 0x2c48
```

`reverse_bits` は入力を固定幅の値として扱います。たとえば、8ビット多重定義は正確に8ビットを反転し、32ビット多重定義は正確に32ビットを反転します。

---

## `byte_order`

`byte_order` は、チェックサム計算で連続するバイトを16ビットワードまたは32ビットワードへまとめる方法を指定します。

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};
```

この列挙型は、バイト列を16ビットまたは32ビットワードとして解釈する場合にだけ使われます。

8ビットチェックサムでは、各入力バイトがすでに1つのチェックサム単位なので、バイト順の設定は不要です。

---

## 8ビットチェックサム

8ビットチェックサム関数は、個々のバイトに対して単純なチェックサムを計算します。

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
```

`checksum8` は通常の加算式8ビットチェックサムです。`checksum_add8` と等価です。

`checksum_add8` はすべてのバイトを256を法として加算します。

`checksum_xor8` はすべてのバイトをXORします。

例:

```cpp
const std::array<std::byte, 4> bytes {
    std::byte{0x01},
    std::byte{0x02},
    std::byte{0x03},
    std::byte{0x04},
};

const auto sum = xer::checksum8(std::span<const std::byte>(bytes));      // 0x0a
const auto x = xer::checksum_xor8(std::span<const std::byte>(bytes));   // 0x04
```

---

## 16ビットチェックサム

16ビットチェックサム関数は、入力バイトを16ビットワードへまとめてから、加算チェックサムまたはXORチェックサムを計算します。

```cpp
auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;
```

`checksum16` は通常の加算式16ビットチェックサムです。`checksum_add16` と等価です。

バイト順は、各2バイトをどのようにワードへ変換するかを制御します。

バイト `{0x01, 0x02}` の場合:

- `byte_order::big_endian` はワードを `0x0102` として読み取ります
- `byte_order::little_endian` はワードを `0x0201` として読み取ります

入力のバイト数が奇数の場合、最後に不足しているバイトは `0x00` として扱われます。

たとえば、ビッグエンディアンのグループ化では、`{0x01, 0x02, 0x03}` は次のように扱われます。

```text
0x0102, 0x0300
```

リトルエンディアンのグループ化では、同じ入力は次のように扱われます。

```text
0x0201, 0x0003
```

---

## 32ビットチェックサム

32ビットチェックサム関数は、入力バイトを32ビットワードへまとめてから、加算チェックサムまたはXORチェックサムを計算します。

```cpp
auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

`checksum32` は通常の加算式32ビットチェックサムです。`checksum_add32` と等価です。

バイト `{0x01, 0x02, 0x03, 0x04}` の場合:

- `byte_order::big_endian` はワードを `0x01020304` として読み取ります
- `byte_order::little_endian` はワードを `0x04030201` として読み取ります

入力バイト数が4の倍数ではない場合、最後の不完全なワードはゼロバイトで埋められます。

たとえば、ビッグエンディアンのグループ化では、`{0x01, 0x02, 0x03}` は次のように扱われます。

```text
0x01020300
```

リトルエンディアンのグループ化では、同じ入力は次のように扱われます。

```text
0x00030201
```

---

## チェックサム入力形式

チェックサム関数は4種類の入力形式に対して提供されます。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;

auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto checksum8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_add8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_xor8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_add16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_xor16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_add32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_xor32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto checksum8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_add8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_xor8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_add16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_xor16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_add32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_xor32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

これらの多重定義は、`std::vector<std::byte>`、`std::array<std::uint8_t, N>`、および同様のバイト指向ストレージで役立ちます。

### ファイルパス

ファイル多重定義は、ファイル全体のチェックサムを計算します。

```cpp
auto checksum8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_add8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_xor8(const path& filename) -> xer::result<std::uint8_t>;

auto checksum16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_add16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_xor16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;

auto checksum32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_add32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_xor32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
```

これらの多重定義はファイル全体の内容を読み取ってからチェックサムを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## 加算チェックサムとXORチェックサムの意味

加算チェックサム関数は、各チェックサム単位を加算し、結果の下位ビットだけを保持します。

- `checksum8` と `checksum_add8` は下位8ビットを保持します
- `checksum16` と `checksum_add16` は下位16ビットを保持します
- `checksum32` と `checksum_add32` は下位32ビットを保持します

`checksum8`、`checksum16`、`checksum32` は、加算チェックサム関数の便利な別名です。加算方式であることを呼び出し箇所で明示したい場合は、`checksum_add8`、`checksum_add16`、`checksum_add32` を使ってください。

XORチェックサム関数は、各チェックサム単位をXORし、対応する幅の結果を返します。

これらは意図的に単純なチェックサムです。小規模なバイナリ形式、単純な診断、単純なプロトコルとの互換性に役立ちます。

これらは暗号学的ハッシュではありません。一般的なCRCアルゴリズムとの互換性が必要な場合は、`crc16` または `crc32` を使ってください。


---

## CRC16とCRC32

`crc16` と `crc32` は、バイト列に対して標準的なCRC値を計算します。

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

`crc16` はCRC-16/ARCのパラメータを使います。

- 多項式: `0xa001`
- 初期値: `0x0000`
- 最終XOR: なし
- `"123456789"` に対するチェック値: `0xbb3d`

`crc32` はCRC-32/ISO-HDLCのパラメータを使います。

- 多項式: `0xedb88320`
- 初期値: `0xffffffff`
- 最終XOR: `0xffffffff`
- `"123456789"` に対するチェック値: `0xcbf43926`

これらの関数はバイトに対して動作します。16ビットおよび32ビットの単純チェックサムとは異なり、CRC計算では `byte_order` を使いません。

---

## CRC入力形式

CRC関数は、チェックサム関数と同じ入力形式ポリシーに従います。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto crc16(const void* data, std::size_t size) noexcept -> xer::result<std::uint16_t>;
auto crc32(const void* data, std::size_t size) noexcept -> xer::result<std::uint32_t>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto crc16(InputIt first, InputIt last) -> std::uint16_t;

template<std::input_iterator InputIt>
auto crc32(InputIt first, InputIt last) -> std::uint32_t;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

### ファイルパス

ファイル多重定義は、ファイル全体のCRCを計算します。

```cpp
auto crc16(const path& filename) -> xer::result<std::uint16_t>;
auto crc32(const path& filename) -> xer::result<std::uint32_t>;
```

これらの多重定義はファイル全体の内容を読み取ってからCRCを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## CRCの空入力

空入力は有効です。

空入力に対して:

- `crc16` は `0x0000` を返します
- `crc32` は `0x00000000` を返します

---

## 空入力

空入力は有効です。

空入力に対して、すべてのチェックサム関数はゼロを返します。

---

## 他のヘッダーとの関係

`<xer/binary.h>` は次のヘッダーと併用すると有用です。

- `<xer/bytes.h>`
- `<xer/stdio.h>`
- `<xer/stdint.h>`

おおまかな境界は次のとおりです。

- `<xer/bytes.h>` はテキストまたはバイト風ストレージを明示的なバイトビューまたはバイトベクターへ変換します
- `<xer/binary.h>` は小規模なバイナリ値操作、単純なチェックサム計算、CRC計算、16進変換、ハッシュ計算を行います
- `<xer/stdio.h>` はストリームベースのバイナリI/Oとファイル全体の操作を扱います

---

## 例

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <xer/binary.h>

auto main() -> int
{
    const auto value = xer::make_u16(0x12, 0x34);
    const auto high = xer::high_u8(value);
    const auto low = xer::low_u8(value);

    if (value != 0x1234 || high != 0x12 || low != 0x34) {
        return 1;
    }

    const std::array<std::byte, 4> bytes {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const auto checksum = xer::checksum16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian);

    if (checksum != 0x0406) {
        return 1;
    }

    const auto crc = xer::crc32(std::span<const std::byte>(bytes));
    if (crc != 0xb63cfbcd) {
        return 1;
    }

    return 0;
}
```

---

## バイナリから16進文字列への変換

`bin2hex` はバイナリデータを小文字の16進文字列に変換します。

```cpp
auto bin2hex(std::span<const std::byte> bytes) -> std::u8string;

auto bin2hex(const void* data, std::size_t size) noexcept
    -> xer::result<std::u8string>;

template<std::input_iterator InputIt>
auto bin2hex(InputIt first, InputIt last) -> std::u8string;
```

各入力バイトは2つの小文字16進文字で表されます。空入力は有効で、空文字列を返します。

```cpp
const std::array<std::byte, 3> bytes {
    std::byte{0x12},
    std::byte{0xab},
    std::byte{0x00},
};

auto hex = xer::bin2hex(std::span<const std::byte>(bytes)); // u8"12ab00"
```

ポインタとサイズを受け取る多重定義は、`data == nullptr` かつ `size != 0` のとき `error_t::invalid_argument` で失敗します。`data == nullptr` かつ `size == 0` は空のバイト列として受け入れられます。

イテレータ範囲多重定義は、`std::byte` と、`std::uint8_t` に変換可能なバイト風の整数値を受け入れます。`std::array<std::byte, N>`、`std::vector<std::byte>`、`std::array<std::uint8_t, N>` などのコンテナで役立ちます。

---

## 16進文字列からバイナリへの変換

`hex2bin` は16進文字列をバイナリデータへ変換します。これはPHPの `hex2bin` 関数をモデルにしています。

```cpp
auto hex2bin(std::u8string_view hex) -> xer::result<std::vector<std::byte>>;
```

`hex2bin` は `0` から `9`、`a` から `f`、`A` から `F` の文字を受け入れます。2つの16進文字が1バイトを形成するため、入力長は偶数でなければなりません。

```cpp
auto bytes = xer::hex2bin(u8"12ab00");
```

入力長が奇数の場合、または入力に16進文字以外が含まれる場合、関数は `error_t::invalid_argument` で失敗します。空入力は有効で、空のベクターを返します。

---


## ハッシュ関数

`md5`、`sha1`、`sha256` は、バイト列とファイルのメッセージダイジェストを計算します。

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

返される配列には、生のダイジェストバイトが含まれます。16進文字列ではありません。一般的な16進表現が必要な場合は `bin2hex` を使ってください。

例:

```cpp
const auto text = std::string_view("abc");
const auto bytes = std::as_bytes(std::span(text));

const auto digest = xer::sha256(bytes);
const auto hex = xer::bin2hex(digest.begin(), digest.end());
// hex == u8"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
```

ダイジェストサイズは次のとおりです。

- `md5`: 16バイト、128ビット
- `sha1`: 20バイト、160ビット
- `sha256`: 32バイト、256ビット

MD5とSHA-1は、主に既存のファイル形式、ツール、テストデータとの互換性のために提供されています。暗号学的なセキュリティ機構として使ってはいけません。これら3つの関数の中では、より強い現代的なハッシュが必要な場合はSHA-256が推奨されます。

---

## ハッシュ入力形式

ハッシュ関数は4種類の入力形式に対して提供されます。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto md5(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 16>>;

auto sha1(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 20>>;

auto sha256(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 32>>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto md5(InputIt first, InputIt last) -> std::array<std::byte, 16>;

template<std::input_iterator InputIt>
auto sha1(InputIt first, InputIt last) -> std::array<std::byte, 20>;

template<std::input_iterator InputIt>
auto sha256(InputIt first, InputIt last) -> std::array<std::byte, 32>;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

これらの多重定義は、`std::array<std::byte, N>`、`std::vector<std::byte>`、`std::array<std::uint8_t, N>` などのコンテナで役立ちます。

### ファイルパス

ファイル多重定義は、ファイル全体のハッシュを計算します。

```cpp
auto md5(const xer::path& filename) -> xer::result<std::array<std::byte, 16>>;
auto sha1(const xer::path& filename) -> xer::result<std::array<std::byte, 20>>;
auto sha256(const xer::path& filename) -> xer::result<std::array<std::byte, 32>>;
```

これらの多重定義はファイル全体の内容を読み取ってからハッシュを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## 既知のハッシュテスト値

既知のダイジェスト値には次のものがあります。

| Input | MD5 | SHA-1 | SHA-256 |
| --- | --- | --- | --- |
| empty input | `d41d8cd98f00b204e9800998ecf8427e` | `da39a3ee5e6b4b0d3255bfef95601890afd80709` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `a` | `0cc175b9c0f1b6a831c399e269772661` | `86f7e437faa5a7fce15d1ddcb9eaeaea377667b8` | `ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb` |
| `abc` | `900150983cd24fb0d6963f7d28e17f72` | `a9993e364706816aba3e25717850c26c9cd0d89d` | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |
| `message digest` | `f96b697d7cb7938d525a2f31aaf161d0` | `c12252ceda8be8994d5fa0290a47231c1d16aae3` | `f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650` |
| `abcdefghijklmnopqrstuvwxyz` | `c3fcd3d76192e4007dfb496cca67e13b` | `32d10c7b8cf96570ca04ce37f2a19d84240d3a89` | `71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73` |

---

## ハッシュ関数の空入力

空入力は有効です。

空入力に対して:

- `md5` は `d41d8cd98f00b204e9800998ecf8427e` を返します
- `sha1` は `da39a3ee5e6b4b0d3255bfef95601890afd80709` を返します
- `sha256` は `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` を返します
