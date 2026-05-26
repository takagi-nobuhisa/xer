#include <xer/assert.h>
#include <xer/ja.h>

namespace {

void test_name_kanji_class()
{
    xer_assert(xer::ja::is_kyouiku_kanji(U'日'));
    xer_assert(xer::ja::is_jouyou_kanji(U'日'));
    xer_assert(xer::ja::is_name_kanji(U'日'));
    xer_assert(xer::ja::name_kanji_class_of(U'日')
        == xer::ja::name_kanji_class::kyouiku);

    xer_assert_not(xer::ja::is_kyouiku_kanji(U'鬱'));
    xer_assert(xer::ja::is_jouyou_kanji(U'鬱'));
    xer_assert(xer::ja::is_name_kanji(U'鬱'));
    xer_assert(xer::ja::name_kanji_class_of(U'鬱')
        == xer::ja::name_kanji_class::jouyou);

    xer_assert(xer::ja::is_name_kanji(U'凜'));
    xer_assert_not(xer::ja::is_jouyou_kanji(U'凜'));
    xer_assert(xer::ja::name_kanji_class_of(U'凜')
        == xer::ja::name_kanji_class::name);

    xer_assert_not(xer::ja::is_name_kanji(U'龠'));
    xer_assert(xer::ja::name_kanji_class_of(U'龠')
        == xer::ja::name_kanji_class::none);
}

void test_jis_kanji_level()
{
    xer_assert(xer::ja::jis_kanji_level_of(U'亜')
        == xer::ja::jis_kanji_level::level_1);
    xer_assert(xer::ja::is_jis_level_1_kanji(U'亜'));

    xer_assert(xer::ja::jis_kanji_level_of(U'弌')
        == xer::ja::jis_kanji_level::level_2);
    xer_assert(xer::ja::is_jis_level_2_kanji(U'弌'));

    xer_assert(xer::ja::jis_kanji_level_of(U'俱')
        == xer::ja::jis_kanji_level::level_3);
    xer_assert(xer::ja::is_jis_level_3_kanji(U'俱'));

    xer_assert(xer::ja::jis_kanji_level_of(U'㐆')
        == xer::ja::jis_kanji_level::level_4);
    xer_assert(xer::ja::is_jis_level_4_kanji(U'㐆'));

    xer_assert(xer::ja::jis_kanji_level_of(U'あ')
        == xer::ja::jis_kanji_level::none);
}

} // namespace

auto main() -> int
{
    test_name_kanji_class();
    test_jis_kanji_level();

    return 0;
}
