# xer C++ Utility Library

<p align="center">
  <img src="icon.png" alt="xer C++ Utility Library アイコン" width="160" height="160">
</p>

<p align="center">
  <a href="https://github.com/takagi-nobuhisa/xer">GitHub</a>
  ·
  <a href="https://x.com/xercpplib">X: @xercpplib</a>
  ·
  <a href="https://note.com/xercpplib">note</a>
</p>

[English README / 英語版 README](README.md)

xer は、**C 言語に慣れ親しんだプログラマ向け**に設計している **C++23 のヘッダオンリライブラリ**です。

C 標準ライブラリ由来の分かりやすさを残しつつ、C++23 の型安全性、`std::expected` による明示的なエラー処理、Unicode を意識した文字列処理を取り入れています。

過度に抽象的な C++ らしさよりも、**見て分かること、挙動を予測しやすいこと、C プログラマが扱いやすいこと**を重視しています。

## リンク

- [GitHub](https://github.com/takagi-nobuhisa/xer)
- [X: @xercpplib](https://x.com/xercpplib)
- [note](https://note.com/xercpplib): xer の使い方や、関連する面白いトピックを紹介します。

## 現在の位置付け

xer は開発中のライブラリです。

当初は、C 標準ライブラリのうち実用性の高い部分を、xer の方針に合わせて再設計するところから出発しました。現在は、その考え方と相性のよい周辺領域にも範囲を広げ、`xer::to<T>` による `result` ベースの汎用値変換、構造化データ処理、ZIP アーカイブ操作、固定スキーマのバイナリシリアライズ、実用的な数学・記述統計・軽量なベクトル/幾何補助、SI単位とヤード・ポンド法単位を含む次元安全な物理量、プロセス・ソケット関連、Tcl/Tk 連携、軽量な image/canvas 機能、MeCab を用いた分かち書き・ふりがな・漢数字・点字変換などの日本語処理まで整備を進めています。

現在の主な対象は次のとおりです。

- 文字・文字列処理
- `xer::to<T>` による `result` ベースの汎用値変換
- Unicode コードポイント走査、書記素クラスタ走査、書記素クラスタ単位の文字列操作、実用的な絵文字判定、NFC 正規化
- 入出力とファイルシステム周辺機能
- チェックサム、CRC、16進変換、MD5、SHA-1、SHA-256 などのバイナリデータ補助
- ZIP アーカイブの読み取り、作成、検索、展開
- 生成された `xfer` 構造体を用いる固定スキーマのバイナリシリアライズ
- パス処理
- 算術補助、初等数学、記述統計、軽量なベクトル/幾何補助、複素数解を扱う方程式補助、SI単位とヤード・ポンド法単位を含む次元安全な物理量、数値ユーティリティ型
- 時刻処理
- JSON、INI、TOML の処理
- 固定長送受信や長さ付きメッセージ送受信を含むプロセス・ソケット関連機能、および `xer_print` による軽量な診断出力
- Tcl/Tk 連携
- image/canvas 描画、ビットマップフォント、基本的なピクセル処理、τrad による円弧描画
- 漢数字、ふりがな、MeCab 連携、仮名・ローマ字分かち書き、点字変換などの日本語処理

なお、C 標準ライブラリ、PHP、プラットフォーム API と同じ関数名を用いることがあっても、**完全互換を目的としているわけではありません**。

## 目標

- C プログラマにとって理解しやすいこと
- ヘッダオンリであること
- C++23 を前提とすること
- 挙動が明示的で予測しやすいこと
- ロケール依存を極力避けること
- 実用的な Unicode 対応を行うこと
- 言語固有のテキスト処理は原則として英語と日本語を対象にし、その他の言語・文字体系は必要なユーザーによる拡張に委ねること
- 不必要に凝った抽象化を避けること
- 賢そうに見える API より、分かりやすい API を優先すること

## 対応環境

現時点の正式なコンパイラ対象は次のとおりです。

- GCC 13.3.0 以上
- Ubuntu 上の libc++ を用いる Clang 18.0.0 以上

現在の主なサポート対象およびテスト対象環境は次のとおりです。

- GCC を用いる Ubuntu
- Clang と libc++ を用いる Ubuntu
- GCC を用いる MSYS2 UCRT64
- Clang を用いる MSYS2 CLANG64
- clang-cl を用いる Visual Studio 2026

対象プラットフォームの範囲は次のとおりです。

- Ubuntu による Linux
- MSYS2 UCRT64 および CLANG64 による Windows
- Visual Studio 2026 の clang-cl による Windows

Windows の対象バージョンは、現時点では **Windows 11 以降**です。

サポート対象外の環境は次のとおりです。

- MSYS2 MSYS
- MSYS2 MINGW64

MSYS2 MSYS および MSYS2 MINGW64 は、現在および今後の予定されているテスト対象には含めません。将来あらためて必要性が明確になった場合は、その時点で再検討します。

Ubuntu で Clang を用いたテストを行う場合は、libc++ と libc++abi が必要です。Ubuntu 24.04 の Clang 18 では、事前に次のパッケージをインストールします。

```sh
sudo apt install libc++-18-dev libc++abi-18-dev
```

Visual Studio 2026 の clang-cl で zlib および ICU を利用するテストを行う場合は、vcpkg の manifest mode を使用します。プロジェクトルートに `zlib` と `icu` を依存関係として含む `vcpkg.json` を配置し、`builtin-baseline` を追加したうえで、プロジェクトルートから次を実行します。

```bat
vcpkg install --triplet x64-windows
```

テストスクリプトは、プロジェクトルート配下の `vcpkg_installed\x64-windows` を自動検出します。`run_tests.php` は、存在する場合に `vcpkg_installed\x64-windows\bin` を子プロセスの `PATH` 先頭へ追加するため、テストや examples の実行時にも ICU / zlib の DLL を見つけられます。`vcpkg_installed/` は生成される依存ライブラリ用ディレクトリなので、リポジトリには含めません。

Tcl/Tk のテストには、Magicsplat Tcl、ActiveTcl、IronTcl など、Tcl/Tk プロジェクトサイトから参照できる Windows 向け Tcl/Tk 配布物が必要です。テストスクリプトは、Tcl/Tk 9.0 と 8.6 の両方が見つかった場合は 9.0 を優先しつつ、`%LOCALAPPDATA%\Apps\Tcl90`、`%LOCALAPPDATA%\Apps\Tcl86`、`C:\local\Tcl90`、`C:\local\Tcl86`、`C:\local\IronTcl`、`C:\Tcl`、`C:\Program Files\Tcl` などの代表的なインストール先を自動検出します。任意のインストール先を使う場合は `XER_TEST_TCLTK_ROOT` で指定できます。それでも自動検出できない場合は、テストスクリプトに明示的なインクルードパスとライブラリパスを指定してください。

Visual Studio 2026 の clang-cl は、利用できない任意依存機能を除くテスト対象です。MSVC cl.exe は現時点では対応対象外です。

## 特徴

### 1. ヘッダオンリ

xer は、インクルードだけで利用できる形を目指しています。

### 2. `std::expected` を中心としたエラー処理

通常の失敗は `xer::result` を通じて `std::expected` で表現します。  
一方、内部不変条件違反やバグ検出は、通常エラーとは分けて xer 独自の assert 系で扱います。

### 3. API 層の分離

- 通常 API: `xer`
- 上級者向け低水準 API: `xer::advanced`
- 内部実装: `xer::detail`

### 4. 文字コード方針を明確化

xer で扱う文字エンコーディングは、次の 4 種類に限定しています。

- CP932
- UTF-8
- UTF-16
- UTF-32

通常 API では、

- 文字列は主として UTF-8
- 1 文字は必要に応じて `char32_t`
- 既存環境との互換のために CP932 も扱う

という方針です。

### 5. ロケール非依存を基本とする設計

文字種判定や文字変換などを、ホスト環境のロケール任せにせず、ライブラリの方針として明示的に設計しています。

### 6. `FILE` 系を土台にした入出力

基盤として `iostream` は使いません。  
C 標準ライブラリの `FILE` 系関数を土台にしつつ、公開 API では次のような型として再設計しています。

- `binary_stream`
- `text_stream`

### 7. 数学・幾何と角度方針

xer は、完全な数値計算ライブラリではなく、実用的な公式や軽量な幾何計算を扱う小さな数学補助を提供します。方程式補助、2次元・3次元・4次元の `vec<T, N>`、極座標と直交座標の変換、ベクトル演算、簡単な回転補助などを含みます。

xer の math、matrix、image API で扱う角度は、特に明記しない限り τrad を基本単位とします。この規約では、`1` が1回転、`0.25` が直角、`0.5` が半回転です。周期角を表す API では `cyclic<T>` を使います。ラジアンや度数法とのやり取りには、`from_rad`、`to_rad`、`from_degree`、`to_degree` などの変換関数を使います。

### 8. 実用的な日本語処理部品

xer には、通常のツールやコード例で使いやすい小さな日本語処理部品も含めています。日本語固有の API は `xer::ja` 名前空間に集めます。

- 漢数字変換
- ふりがな整形
- MeCab による形態素解析と分かち書き補助
- MeCab の読みを使った仮名・ローマ字分かち書き
- `xer::braille` に置く点字前置符定数および日英共通・英語寄りの低水準部品
- `xer::ja` に置く日本語仮名点字変換
- MeCab 連携の点字変換

点字関連機能は実用的な部品群です。完全な点訳エンジンを名乗るものではなく、MeCab 連携の変換は外部 MeCab 辞書が返す読みに依存します。

## 公開ヘッダ

現時点の公開ヘッダは次の 46 個です。

- `xer/error.h`
- `xer/assert.h`
- `xer/typeinfo.h`
- `xer/diag.h`
- `xer/scope.h`
- `xer/convert.h`
- `xer/string.h`
- `xer/ctype.h`
- `xer/braille.h`
- `xer/stdlib.h`
- `xer/kansuji.h`
- `xer/mecab.h`
- `xer/furigana.h`
- `xer/ja.h`
- `xer/unicode.h`
- `xer/bytes.h`
- `xer/base64.h`
- `xer/binary.h`
- `xer/zip.h`
- `xer/serialize.h`
- `xer/parse.h`
- `xer/json.h`
- `xer/ini.h`
- `xer/toml.h`
- `xer/stdio.h`
- `xer/iostream.h`
- `xer/path.h`
- `xer/dirent.h`
- `xer/socket.h`
- `xer/tk.h`
- `xer/stdint.h`
- `xer/stdfloat.h`
- `xer/arithmetic.h`
- `xer/math.h`
- `xer/statistics.h`
- `xer/complex.h`
- `xer/cyclic.h`
- `xer/interval.h`
- `xer/color.h`
- `xer/quantity.h`
- `xer/matrix.h`
- `xer/image.h`
- `xer/process.h`
- `xer/cmdline.h`
- `xer/time.h`
- `xer/version.h`

`xer/bits/` 配下は内部実装用ヘッダです。

## リポジトリ構成

```text
xer/
  xer/        公開ヘッダ
  xer/bits/   内部実装ヘッダ
  tests/      テストプログラム
  examples/   コード例
  php/        開発用補助スクリプト
  docs/       設計文書とリファレンス資料
```

## 使用例

```cpp
#include <iostream>
#include <xer/path.h>
#include <xer/arithmetic.h>

int main()
{
    xer::path base(u8"C:/work");
    xer::path file(u8"docs/readme.txt");

    auto joined = base / file;
    auto sum = xer::add(10u, -3);

    std::cout << reinterpret_cast<const char*>(joined.str().data()) << '\n';

    if (sum.has_value()) {
        std::cout << *sum << '\n';
    }
}
```

## 開発用ツールについて

このリポジトリには、開発時専用ツールとして PHP スクリプトを含めています。用途は主に次のとおりです。

- 変換表の生成
- テストケースの生成
- テストプログラムの生成
- コンパイル・実行テストの制御
- テスト結果の集計

これは **xer 自身の開発用**です。  
ライブラリ利用者が xer を使うだけであれば、PHP は不要です。

## 現段階での非目標

少なくとも現段階では、次のものは目標にしていません。

- 完全なロケール対応
- C 標準ライブラリ全ヘッダの再現
- `iostream` ベースの設計
- 各処理系固有挙動との完全一致
- 全コンパイラへの即時対応
- 漢字かな交じり文の読み決定の完全化や、完全な点訳エンジンの提供

また、いくつかのヘッダは独立公開せず、内部実装または既存ヘッダへ吸収しています。数学補助と複素数関連補助は、現在は専用の公開ヘッダを持ちます。

## 文書

設計文書とリファレンス資料は `docs/` 配下にあります。

- [リファレンスマニュアル](docs/xer_reference_manual_ja.md)

たとえば次のような内容を文書化しています。

- プロジェクト骨子
- 文字コード方針
- パス処理方針
- 入出力方針
- 四則演算・比較演算・数学補助方針
- cyclic、角度単位、物理量単位の方針
- MeCab 連携方針
- Tcl/Tk 連携方針
- image/canvas とビットマップフォントの方針
- コーディング規約
- テスト・コード生成方針

## ライセンス

Boost Software License 1.0  
詳細は [LICENSE](LICENSE) を参照してください。

## 名前について

xer は、C 的な流儀に親しんできた X 世代のプログラマ向け C++23 ライブラリとして構想していることに由来します。
