<!-- xer-reference-source-sha256: 967dbf2e25e7330f9e585616f7d8a2f992100eebdb09f4563786ed7fa1ad7a89 -->

# `<xer/complex.h>`

## 目的

`<xer/complex.h>` は、xer の軽量な複素数数学ヘルパーを提供します。

このヘッダーには、`<xer/math.h>` の一部の関数に対応する複素数版が含まれます。実数のみを扱うコードが複素数機能をインクルードせずに済むように、`<xer/math.h>` とは分離されています。

---

## 提供される関数

### `cquadratic`

```cpp
template<std::floating_point T>
auto cquadratic(T a, T b, T c)
    -> xer::result<std::array<std::complex<T>, 2>>;
```

次の二次方程式を解きます。

```text
a * x * x + b * x + c == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、重複度を含む 2 個の複素根が格納されます。

### `ccubic`

```cpp
template<std::floating_point T>
auto ccubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::complex<T>, 3>>;
```

次の三次方程式を解きます。

```text
a * x * x * x + b * x * x + c * x + d == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、重複度を含む 3 個の複素根が格納されます。

---

## 設計メモ

`c` 接頭辞は複素根版を意味します。たとえば、`cquadratic` は `quadratic` に対応する複素根版です。

現在の関数は実係数を受け取り、複素根を返します。実用上の必要が出てきた場合は、複素係数への対応を後で検討できます。
