<!-- xer-reference-source-sha256: 4e19876b6cf3642401f5fdbc894f09953f54216607b33088927f25950527ddb0 -->

# `<xer/process.h>`

## 目的

`<xer/process.h>` は子プロセス管理機能を提供します。

初期 API は意図的に小さく、プロセスの直接起動、待機、標準ストリームの接続に重点を置いています。

---

## 主な役割

このヘッダーは次の機能を提供します。

- ムーブ専用のプロセスハンドル
- コマンドシェルを介さない子プロセスの直接起動
- 標準入力、標準出力、標準エラー出力の設定
- `binary_stream` オブジェクトとして公開される任意のパイプ
- プロセスの待機と終了コードの取得

---

## 主な型

```cpp
enum class process_stdio;
struct process_options;
struct process_result;
class process;
struct process_spawn_result;
```

### `process_stdio`

```cpp
inherit
null
pipe
```

- `inherit` は子プロセスのストリームを対応する親プロセスのストリームに接続します。
- `null` は子プロセスのストリームをプラットフォームのヌルデバイスに接続します。
- `pipe` は親側のパイプを作成し、`binary_stream` として表します。

### `process_options`

```cpp
path program;
std::vector<std::u8string> arguments;
process_stdio stdin_mode;
process_stdio stdout_mode;
process_stdio stderr_mode;
```

`arguments` には `argv[0]` を含めません。プログラムパスは別に指定します。

### `process_result`

```cpp
int exit_code;
```

POSIX では、シグナルによる終了は `128 + signal_number` として表されます。

### `process_spawn_result`

```cpp
process proc;
std::optional<binary_stream> stdin_stream;
std::optional<binary_stream> stdout_stream;
std::optional<binary_stream> stderr_stream;
```

任意のストリームは、対応する `process_stdio::pipe` モードが要求された場合にだけ存在します。

---

## 主な関数

```cpp
auto process_spawn(const process_options& options) noexcept -> xer::result<process_spawn_result>;
auto process_wait(process& value) noexcept -> xer::result<process_result>;
```

`process_spawn` は対象プログラムを直接実行し、引数を個別のコマンドライン引数として渡します。
コマンドシェルは起動しません。

`process_wait` は子プロセスを待機し、その終了状態を返します。

---

## プロセスハンドル

`process` はムーブ専用のハンドル型です。

```cpp
auto is_open() const noexcept -> bool;
```

デストラクタはオブジェクトが所有するネイティブハンドルを解放しますが、プロセスの終了は待機しません。
終了コードが必要な場合は、`process_wait` を明示的に呼び出してください。

---

## 注意

- パスには `xer::path` を使い、内部でネイティブパス変換を行います。
- 引数には UTF-8 の `std::u8string` 値を使います。
- Windows では、直接プロセス生成のためのコマンドライン引用処理を内部で行います。
- POSIX では、子プロセスをプラットフォームのプロセス機能で作成し、その後直接実行します。
- パイプストリームはバイナリストリームです。必要であれば、より高水準のテキスト処理を別に重ねられます。
