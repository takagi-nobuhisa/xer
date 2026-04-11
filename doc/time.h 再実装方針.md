# time.h 再実装方針

## 1. 基本方針

`xer/time.h` は、C 標準ライブラリの `time.h` をそのまま模倣するのではなく、C の流儀の分かりやすさを残しつつ、XER の方針に合わせて C++ ライブラリとして再設計する。

C++ の `std::chrono` は高機能だが、表現が重くなりやすく、C 的な素朴な時間操作を行いたい場面では扱いづらい。  
そのため XER では、`chrono` を公開 API の中心には据えず、C 由来の単純な時間ライブラリとして `time.h` を再構成する。

エラー処理は、XER 全体の方針に合わせて、特殊値ではなく `std::expected` と `error<void>` によって表現する。

---

## 2. time_t の方針

### 2.1 型

`xer::time_t` は算術型とし、`double` を用いる。

```cpp
using time_t = double;
````

### 2.2 意味

`xer::time_t` の単位は秒とする。

* 整数部: 秒
* 小数部: 秒未満

これにより、C 的な感覚でそのまま四則演算しやすくする。

### 2.3 分解能

実用上の分解能はマイクロ秒級を想定する。
ただし、内部表現はあくまで `double` であり、厳密な固定小数点やナノ秒精度を保証するものではない。

---

## 3. エポックの方針

C 標準では `time_t` の基準時刻は処理系定義だが、XER では POSIX に合わせて次を基準時刻とする。

* 1970-01-01 00:00:00 UTC

すなわち、`xer::time_t` の値 `0.0` はこの日時を表す。

---

## 4. 対応範囲

初期実装では、エポック以前の時刻は扱わない。

* `time_t < 0` は非対応
* 1970-01-01 00:00:00 UTC より前の日時は非対応

したがって、負の `time_t` を `gmtime` や `localtime` に渡した場合はエラーとする。
また、`mktime` にエポック以前の日時を与えた場合もエラーとする。

---

## 5. xer::tm の方針

### 5.1 基本構成

`xer::tm` は C の `struct tm` をベースとし、秒未満の情報を保持するために `tm_microsec` を追加する。

```cpp
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};
```

### 5.2 tm_microsec

`tm_microsec` は秒未満のマイクロ秒を表す。

* 範囲: `0` 以上 `999999` 以下

`tm_sec` は通常通り秒を表し、`tm_microsec` がその小数部を補う。

---

## 6. 関数分類

`time.h` の関数は、性質に応じて次のように分類する。

### 6.1 時刻取得

* `time`
* `clock`

### 6.2 分解・逆変換

* `gmtime`
* `localtime`
* `mktime`

### 6.3 差分計算

* `difftime`

### 6.4 文字列化・書式化

* `ctime`
* `strftime`

### 6.5 初期段階では保留または簡略対応

* `asctime`（`ctime` に統合）
* `%f` を含む XER 独自高精度書式
* 高度なロケール依存機能
* タイムゾーン拡張機能

---

## 7. 各関数の方針

## 7.1 time

現在の暦時間を返す。

```cpp
std::expected<time_t, error<void>> time();
```

現実には失敗することは少ないが、失敗時は `std::expected` により表現する。
エラー種別は `error_t::runtime_error` で十分とする。

---

## 7.2 gmtime

UTC の分解時刻へ変換する。

```cpp
std::expected<tm, error<void>> gmtime(time_t value);
```

* `value < 0` の場合はエラー
* 秒未満は `tm_microsec` に格納する

---

## 7.3 localtime

ローカル時刻の分解時刻へ変換する。

```cpp
std::expected<tm, error<void>> localtime(time_t value);
```

* `value < 0` の場合はエラー
* 秒未満は `tm_microsec` に格納する
* ローカル時刻変換に失敗した場合は `error_t::runtime_error`

---

## 7.4 mktime

分解時刻から暦時間へ変換する。

```cpp
std::expected<time_t, error<void>> mktime(const tm& value);
```

* `tm_microsec` を小数部へ反映する
* エポック以前の日時になる場合はエラー
* `tm_microsec` が範囲外の場合はエラー

---

## 7.5 ctime

`asctime` と `ctime` は、XER では `ctime` に統一する。
入力型の違いは多重定義で表現する。

```cpp
std::u8string ctime(time_t value);
std::u8string ctime(const tm& value);
```

### 方針

* `time_t` を受ける版と `tm` を受ける版を用意する
* `tm` は `const tm&` で受け取る
* 戻り値は `std::u8string` とする
* C のような静的内部バッファは使わない

`tm` をポインタで受ける必要はない。
XER は C++ ライブラリとして再設計するため、入力専用であることが明確な `const tm&` を採用する。

---

## 7.6 strftime

書式付きの時刻文字列を生成する。

```cpp
std::expected<std::u8string, error<void>>
strftime(std::u8string_view format, const tm& value);
```

### 方針

* 書式文字列は UTF-8 とする
* 戻り値は `std::u8string`
* エラー時は `std::expected` で返す

### 書式文字列について

書式文字列は ASCII 限定としない。
UTF-8 の固定文字列を含んでよい。

例えば次のような書式を許容する。

```text
%Y年%m月%d日 %H時%M分%S秒
```

この場合、認識対象は `%` とその直後の書式指定子だけであり、それ以外の UTF-8 文字列は固定文字列としてそのまま扱う。

### 初期実装

初期実装では、内部で処理系の `std::strftime` または `wcsftime` を利用してよい。
ただし、XER 独自拡張の `%f` は初期段階では未対応とする。

### ロケール依存

`LC_TIME` に依存する指定子（曜日名、月名、慣用表記など）は、初期実装では処理系依存の結果でよい。
現実には `LC_TIME` を明示的に変更するケースは少ないため、初期段階では過度に厳密化しない。

---

## 8. asctime の扱い

C には `asctime` があるが、XER では独立関数としては設けず、`ctime(const tm&)` に統合する。

つまり、分解時刻を文字列化する機能は `ctime(const tm&)` が担う。

---

## 9. error_t の扱い

`time.h` の初期実装では、エラー種別の細分化は最低限に留める。

### 主に用いるもの

* `error_t::runtime_error`
* `error_t::invalid_argument`

### 用途

* 実行時の時刻取得失敗や変換失敗: `runtime_error`
* 負の `time_t`
* 範囲外の `tm_microsec`
* エポック以前の日時
* 不正な書式指定子

など: `invalid_argument`

---

## 10. 初期実装対象

初期段階で実装対象とする関数は次の通り。

* `time`
* `clock`
* `gmtime`
* `localtime`
* `mktime`
* `difftime`
* `ctime`
* `strftime`

---

## 11. 初期段階で見送るもの

次の要素は初期段階では未対応または簡略対応とする。

* エポック以前の時刻
* `strftime` の `%f`
* `strftime` の高度なロケール制御
* 高度なタイムゾーン機能
* C の静的内部バッファ互換動作
* `asctime` の独立提供

---

## 12. 暫定 API 一覧

現時点の暫定 API は次の通り。

```cpp
using time_t = double;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};

std::expected<time_t, error<void>> time();
std::expected<clock_t, error<void>> clock();

std::expected<tm, error<void>> gmtime(time_t value);
std::expected<tm, error<void>> localtime(time_t value);
std::expected<time_t, error<void>> mktime(const tm& value);

double difftime(time_t left, time_t right);

std::u8string ctime(time_t value);
std::u8string ctime(const tm& value);

std::expected<std::u8string, error<void>>
strftime(std::u8string_view format, const tm& value);
```

---

## 13. 今後の拡張候補

将来的には次のような拡張を検討できる。

* `strftime` への `%f` 追加
* マイクロ秒を含む ISO 8601 形式の強化
* タイムゾーン情報の拡張
* 高精度時刻のより厳密な内部表現
* ロケール依存書式の整理
* 必要に応じたエポック以前の時刻対応

ただし、初期段階ではそこまで拡げず、まずは C 的に分かりやすく使える高精度 `time.h` を成立させることを優先する。
