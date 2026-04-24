/**
 * @file tests/test_ctype_width.cpp
 * @brief Tests for fullwidth and halfwidth character classification.
 */

#include <xer/assert.h>
#include <xer/ctype.h>

namespace {

void test_fullwidth_kana() {
    xer_assert(xer::isctype(U'ア', xer::ctype_id::fullwidth_kana));
    xer_assert(xer::isctype(U'カ', xer::ctype_id::fullwidth_kana));
    xer_assert(xer::isctype(U'ガ', xer::ctype_id::fullwidth_kana));
    xer_assert(xer::isctype(U'パ', xer::ctype_id::fullwidth_kana));
    xer_assert(xer::isctype(U'ヴ', xer::ctype_id::fullwidth_kana));
    xer_assert(xer::isctype(U'ー', xer::ctype_id::fullwidth_kana));

    xer_assert_not(xer::isctype(U'ｱ', xer::ctype_id::fullwidth_kana));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::fullwidth_kana));
    xer_assert_not(xer::isctype(U'あ', xer::ctype_id::fullwidth_kana));
}

void test_halfwidth_kana() {
    xer_assert(xer::isctype(U'ｱ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ｶ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ﾊ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ｳ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ｰ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ﾞ', xer::ctype_id::halfwidth_kana));
    xer_assert(xer::isctype(U'ﾟ', xer::ctype_id::halfwidth_kana));

    xer_assert_not(xer::isctype(U'ア', xer::ctype_id::halfwidth_kana));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::halfwidth_kana));
    xer_assert_not(xer::isctype(U'あ', xer::ctype_id::halfwidth_kana));
}

void test_fullwidth_digit() {
    xer_assert(xer::isctype(U'０', xer::ctype_id::fullwidth_digit));
    xer_assert(xer::isctype(U'５', xer::ctype_id::fullwidth_digit));
    xer_assert(xer::isctype(U'９', xer::ctype_id::fullwidth_digit));

    xer_assert_not(xer::isctype(U'0', xer::ctype_id::fullwidth_digit));
    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_digit));
    xer_assert_not(xer::isctype(U'！', xer::ctype_id::fullwidth_digit));
}

void test_halfwidth_digit() {
    xer_assert(xer::isctype(U'0', xer::ctype_id::halfwidth_digit));
    xer_assert(xer::isctype(U'5', xer::ctype_id::halfwidth_digit));
    xer_assert(xer::isctype(U'9', xer::ctype_id::halfwidth_digit));

    xer_assert_not(xer::isctype(U'０', xer::ctype_id::halfwidth_digit));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::halfwidth_digit));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::halfwidth_digit));
}

void test_fullwidth_alpha() {
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_alpha));
    xer_assert(xer::isctype(U'Ｚ', xer::ctype_id::fullwidth_alpha));
    xer_assert(xer::isctype(U'ａ', xer::ctype_id::fullwidth_alpha));
    xer_assert(xer::isctype(U'ｚ', xer::ctype_id::fullwidth_alpha));

    xer_assert_not(xer::isctype(U'A', xer::ctype_id::fullwidth_alpha));
    xer_assert_not(xer::isctype(U'１', xer::ctype_id::fullwidth_alpha));
    xer_assert_not(xer::isctype(U'！', xer::ctype_id::fullwidth_alpha));
}

void test_halfwidth_alpha() {
    xer_assert(xer::isctype(U'A', xer::ctype_id::halfwidth_alpha));
    xer_assert(xer::isctype(U'Z', xer::ctype_id::halfwidth_alpha));
    xer_assert(xer::isctype(U'a', xer::ctype_id::halfwidth_alpha));
    xer_assert(xer::isctype(U'z', xer::ctype_id::halfwidth_alpha));

    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::halfwidth_alpha));
    xer_assert_not(xer::isctype(U'1', xer::ctype_id::halfwidth_alpha));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::halfwidth_alpha));
}

void test_fullwidth_punct() {
    xer_assert(xer::isctype(U'！', xer::ctype_id::fullwidth_punct));
    xer_assert(xer::isctype(U'？', xer::ctype_id::fullwidth_punct));
    xer_assert(xer::isctype(U'＠', xer::ctype_id::fullwidth_punct));
    xer_assert(xer::isctype(U'［', xer::ctype_id::fullwidth_punct));
    xer_assert(xer::isctype(U'］', xer::ctype_id::fullwidth_punct));

    xer_assert_not(xer::isctype(U'!', xer::ctype_id::fullwidth_punct));
    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_punct));
    xer_assert_not(xer::isctype(U'１', xer::ctype_id::fullwidth_punct));
    xer_assert_not(xer::isctype(U'　', xer::ctype_id::fullwidth_punct));
}

void test_halfwidth_punct() {
    xer_assert(xer::isctype(U'!', xer::ctype_id::halfwidth_punct));
    xer_assert(xer::isctype(U'?', xer::ctype_id::halfwidth_punct));
    xer_assert(xer::isctype(U'@', xer::ctype_id::halfwidth_punct));
    xer_assert(xer::isctype(U'[', xer::ctype_id::halfwidth_punct));
    xer_assert(xer::isctype(U']', xer::ctype_id::halfwidth_punct));

    xer_assert_not(xer::isctype(U'！', xer::ctype_id::halfwidth_punct));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::halfwidth_punct));
    xer_assert_not(xer::isctype(U'1', xer::ctype_id::halfwidth_punct));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::halfwidth_punct));
}

void test_fullwidth_space() {
    xer_assert(xer::isctype(U'　', xer::ctype_id::fullwidth_space));

    xer_assert_not(xer::isctype(U' ', xer::ctype_id::fullwidth_space));
    xer_assert_not(xer::isctype(U'\t', xer::ctype_id::fullwidth_space));
    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_space));
}

void test_halfwidth_space() {
    xer_assert(xer::isctype(U' ', xer::ctype_id::halfwidth_space));

    xer_assert_not(xer::isctype(U'　', xer::ctype_id::halfwidth_space));
    xer_assert_not(xer::isctype(U'\t', xer::ctype_id::halfwidth_space));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::halfwidth_space));
}

void test_fullwidth_graph() {
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_graph));
    xer_assert(xer::isctype(U'１', xer::ctype_id::fullwidth_graph));
    xer_assert(xer::isctype(U'！', xer::ctype_id::fullwidth_graph));
    xer_assert(xer::isctype(U'ア', xer::ctype_id::fullwidth_graph));
    xer_assert(xer::isctype(U'ガ', xer::ctype_id::fullwidth_graph));

    xer_assert_not(xer::isctype(U'　', xer::ctype_id::fullwidth_graph));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::fullwidth_graph));
    xer_assert_not(xer::isctype(U'1', xer::ctype_id::fullwidth_graph));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::fullwidth_graph));
    xer_assert_not(xer::isctype(U'ｱ', xer::ctype_id::fullwidth_graph));
}

void test_halfwidth_graph() {
    xer_assert(xer::isctype(U'A', xer::ctype_id::halfwidth_graph));
    xer_assert(xer::isctype(U'1', xer::ctype_id::halfwidth_graph));
    xer_assert(xer::isctype(U'!', xer::ctype_id::halfwidth_graph));
    xer_assert(xer::isctype(U'ｱ', xer::ctype_id::halfwidth_graph));
    xer_assert(xer::isctype(U'ﾞ', xer::ctype_id::halfwidth_graph));

    xer_assert_not(xer::isctype(U' ', xer::ctype_id::halfwidth_graph));
    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::halfwidth_graph));
    xer_assert_not(xer::isctype(U'１', xer::ctype_id::halfwidth_graph));
    xer_assert_not(xer::isctype(U'！', xer::ctype_id::halfwidth_graph));
    xer_assert_not(xer::isctype(U'ア', xer::ctype_id::halfwidth_graph));
}

void test_fullwidth_print() {
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::fullwidth_print));
    xer_assert(xer::isctype(U'１', xer::ctype_id::fullwidth_print));
    xer_assert(xer::isctype(U'！', xer::ctype_id::fullwidth_print));
    xer_assert(xer::isctype(U'　', xer::ctype_id::fullwidth_print));
    xer_assert(xer::isctype(U'ア', xer::ctype_id::fullwidth_print));
    xer_assert(xer::isctype(U'ガ', xer::ctype_id::fullwidth_print));

    xer_assert_not(xer::isctype(U'A', xer::ctype_id::fullwidth_print));
    xer_assert_not(xer::isctype(U'1', xer::ctype_id::fullwidth_print));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::fullwidth_print));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::fullwidth_print));
    xer_assert_not(xer::isctype(U'ｱ', xer::ctype_id::fullwidth_print));
}

void test_halfwidth_print() {
    xer_assert(xer::isctype(U'A', xer::ctype_id::halfwidth_print));
    xer_assert(xer::isctype(U'1', xer::ctype_id::halfwidth_print));
    xer_assert(xer::isctype(U'!', xer::ctype_id::halfwidth_print));
    xer_assert(xer::isctype(U' ', xer::ctype_id::halfwidth_print));
    xer_assert(xer::isctype(U'ｱ', xer::ctype_id::halfwidth_print));
    xer_assert(xer::isctype(U'ﾞ', xer::ctype_id::halfwidth_print));

    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::halfwidth_print));
    xer_assert_not(xer::isctype(U'１', xer::ctype_id::halfwidth_print));
    xer_assert_not(xer::isctype(U'！', xer::ctype_id::halfwidth_print));
    xer_assert_not(xer::isctype(U'　', xer::ctype_id::halfwidth_print));
    xer_assert_not(xer::isctype(U'ア', xer::ctype_id::halfwidth_print));
}

void test_fullwidth_and_halfwidth() {
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::fullwidth));
    xer_assert(xer::isctype(U'１', xer::ctype_id::fullwidth));
    xer_assert(xer::isctype(U'！', xer::ctype_id::fullwidth));
    xer_assert(xer::isctype(U'　', xer::ctype_id::fullwidth));
    xer_assert(xer::isctype(U'ア', xer::ctype_id::fullwidth));
    xer_assert(xer::isctype(U'ガ', xer::ctype_id::fullwidth));

    xer_assert(xer::isctype(U'A', xer::ctype_id::halfwidth));
    xer_assert(xer::isctype(U'1', xer::ctype_id::halfwidth));
    xer_assert(xer::isctype(U'!', xer::ctype_id::halfwidth));
    xer_assert(xer::isctype(U' ', xer::ctype_id::halfwidth));
    xer_assert(xer::isctype(U'ｱ', xer::ctype_id::halfwidth));
    xer_assert(xer::isctype(U'ﾞ', xer::ctype_id::halfwidth));

    xer_assert_not(xer::isctype(U'A', xer::ctype_id::fullwidth));
    xer_assert_not(xer::isctype(U'1', xer::ctype_id::fullwidth));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::fullwidth));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::fullwidth));
    xer_assert_not(xer::isctype(U'ｱ', xer::ctype_id::fullwidth));

    xer_assert_not(xer::isctype(U'Ａ', xer::ctype_id::halfwidth));
    xer_assert_not(xer::isctype(U'１', xer::ctype_id::halfwidth));
    xer_assert_not(xer::isctype(U'！', xer::ctype_id::halfwidth));
    xer_assert_not(xer::isctype(U'　', xer::ctype_id::halfwidth));
    xer_assert_not(xer::isctype(U'ア', xer::ctype_id::halfwidth));
}

} // namespace

auto main() -> int {
    test_fullwidth_kana();
    test_halfwidth_kana();

    test_fullwidth_digit();
    test_halfwidth_digit();

    test_fullwidth_alpha();
    test_halfwidth_alpha();

    test_fullwidth_punct();
    test_halfwidth_punct();

    test_fullwidth_space();
    test_halfwidth_space();

    test_fullwidth_graph();
    test_halfwidth_graph();

    test_fullwidth_print();
    test_halfwidth_print();

    test_fullwidth_and_halfwidth();

    return 0;
}
