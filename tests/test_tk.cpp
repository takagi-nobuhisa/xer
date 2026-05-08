/**
 * @file tests/test_tk.cpp
 * @brief Tests for Tk-specific helpers.
 */

// XER_TEST_FEATURES: tcltk

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/tk.h>

namespace {

[[nodiscard]] auto to_u8string(int value) -> std::u8string
{
    const auto text = std::to_string(value);
    return std::u8string(
        reinterpret_cast<const char8_t*>(text.data()),
        text.size());
}

[[nodiscard]] auto make_tk_interpreter() -> xer::tk::interpreter
{
    auto interp = xer::tk::interpreter::create();
    xer_assert(interp.has_value());
    xer_assert(interp->valid());

    const auto initialized = xer::tk::init(*interp);
    xer_assert(initialized.has_value());

    return std::move(*interp);
}

void delete_photo(xer::tk::interpreter& interp, const char8_t* name)
{
    std::u8string script = u8"catch {image delete ";
    script += name;
    script += u8"}";
    static_cast<void>(xer::tk::eval(interp, script));
}

[[nodiscard]] auto create_photo(
    xer::tk::interpreter& interp,
    const char8_t* name,
    int width,
    int height) -> xer::tk::photo_image
{
    delete_photo(interp, name);

    std::u8string script = u8"image create photo ";
    script += name;
    script += u8" -width ";
    script += to_u8string(width);
    script += u8" -height ";
    script += to_u8string(height);

    const auto created = xer::tk::eval(interp, script);
    xer_assert(created.has_value());

    auto photo = xer::tk::find_photo(interp, name);
    xer_assert(photo.has_value());
    return *photo;
}

void test_tk_photo_constants_and_type()
{
    xer_assert_eq(xer::tk::photo_composite_overlay, TK_PHOTO_COMPOSITE_OVERLAY);
    xer_assert_eq(xer::tk::photo_composite_set, TK_PHOTO_COMPOSITE_SET);
    static_assert(!std::is_default_constructible_v<xer::tk::photo_image>);
    static_assert(std::is_copy_constructible_v<xer::tk::photo_image>);
}

void test_tk_find_photo_null_name()
{
    auto interp = make_tk_interpreter();

    const auto result = xer::tk::find_photo(interp, nullptr);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_tk_find_photo_missing_name()
{
    auto interp = make_tk_interpreter();

    const auto result = xer::tk::find_photo(interp, u8"xer_missing_photo");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

void test_tk_find_photo_existing_and_size()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_size", 2, 3);

    const auto size = xer::tk::photo_get_size(photo);
    xer_assert_eq(size.width, 2);
    xer_assert_eq(size.height, 3);

    delete_photo(interp, u8"xer_test_photo_size");
}

void test_tk_photo_set_size()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_set_size", 2, 2);

    const auto resized = xer::tk::photo_set_size(interp, photo, 4, 3);
    xer_assert(resized.has_value());

    const auto size = xer::tk::photo_get_size(photo);
    xer_assert_eq(size.width, 4);
    xer_assert_eq(size.height, 3);

    delete_photo(interp, u8"xer_test_photo_set_size");
}

void test_tk_photo_expand()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_expand", 2, 2);

    const auto clipped = xer::tk::photo_expand(interp, photo, 5, 4);
    xer_assert(clipped.has_value());

    const auto explicit_size = xer::tk::photo_get_size(photo);
    xer_assert_eq(explicit_size.width, 2);
    xer_assert_eq(explicit_size.height, 2);

    const auto flexible = xer::tk::photo_set_size(interp, photo, 0, 0);
    xer_assert(flexible.has_value());

    const auto expanded = xer::tk::photo_expand(interp, photo, 5, 4);
    xer_assert(expanded.has_value());

    const auto size = xer::tk::photo_get_size(photo);
    xer_assert_eq(size.width, 5);
    xer_assert_eq(size.height, 4);

    delete_photo(interp, u8"xer_test_photo_expand");
}

void test_tk_photo_blank()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_blank", 3, 2);

    xer::tk::photo_blank(photo);

    const auto size = xer::tk::photo_get_size(photo);
    xer_assert_eq(size.width, 3);
    xer_assert_eq(size.height, 2);

    delete_photo(interp, u8"xer_test_photo_blank");
}

void test_tk_photo_get_image_null_block()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_null_get", 1, 1);
    const auto result = xer::tk::photo_get_image(photo, nullptr);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);

    delete_photo(interp, u8"xer_test_photo_null_get");
}

void test_tk_photo_put_block_null_block()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_null_put", 1, 1);
    const auto result = xer::tk::photo_put_block(interp, photo, nullptr, 0, 0, 1, 1);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);

    delete_photo(interp, u8"xer_test_photo_null_put");
}

void test_tk_photo_put_and_get_block()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_put_get", 2, 2);

    std::array<unsigned char, 16> pixels = {
        10, 20, 30, 255,
        40, 50, 60, 255,
        70, 80, 90, 255,
        100, 110, 120, 255,
    };

    xer::tk::photo_image_block block{};
    block.pixelPtr = pixels.data();
    block.width = 2;
    block.height = 2;
    block.pitch = 8;
    block.pixelSize = 4;
    block.offset[0] = 0;
    block.offset[1] = 1;
    block.offset[2] = 2;
    block.offset[3] = 3;

    const auto put = xer::tk::photo_put_block(
        interp,
        photo,
        &block,
        0,
        0,
        2,
        2,
        xer::tk::photo_composite_set);
    xer_assert(put.has_value());

    xer::tk::photo_image_block fetched{};
    const auto got = xer::tk::photo_get_image(photo, &fetched);
    xer_assert(got.has_value());
    xer_assert_eq(fetched.width, 2);
    xer_assert_eq(fetched.height, 2);

    const auto* const first = fetched.pixelPtr;
    xer_assert(first != nullptr);
    xer_assert_eq(first[fetched.offset[0]], 10);
    xer_assert_eq(first[fetched.offset[1]], 20);
    xer_assert_eq(first[fetched.offset[2]], 30);

    const auto* const last =
        fetched.pixelPtr + fetched.pitch + fetched.pixelSize;
    xer_assert_eq(last[fetched.offset[0]], 100);
    xer_assert_eq(last[fetched.offset[1]], 110);
    xer_assert_eq(last[fetched.offset[2]], 120);

    delete_photo(interp, u8"xer_test_photo_put_get");
}

void test_tk_photo_put_zoomed_block()
{
    auto interp = make_tk_interpreter();

    const auto photo = create_photo(interp, u8"xer_test_photo_zoomed", 2, 2);

    std::array<unsigned char, 4> pixels = {200, 100, 50, 255};

    xer::tk::photo_image_block block{};
    block.pixelPtr = pixels.data();
    block.width = 1;
    block.height = 1;
    block.pitch = 4;
    block.pixelSize = 4;
    block.offset[0] = 0;
    block.offset[1] = 1;
    block.offset[2] = 2;
    block.offset[3] = 3;

    const auto put = xer::tk::photo_put_zoomed_block(
        interp,
        photo,
        &block,
        0,
        0,
        1,
        1,
        2,
        2,
        1,
        1,
        xer::tk::photo_composite_set);
    xer_assert(put.has_value());

    xer::tk::photo_image_block fetched{};
    const auto got = xer::tk::photo_get_image(photo, &fetched);
    xer_assert(got.has_value());
    xer_assert_eq(fetched.width, 2);
    xer_assert_eq(fetched.height, 2);

    const auto* const first = fetched.pixelPtr;
    xer_assert(first != nullptr);
    xer_assert_eq(first[fetched.offset[0]], 200);
    xer_assert_eq(first[fetched.offset[1]], 100);
    xer_assert_eq(first[fetched.offset[2]], 50);

    delete_photo(interp, u8"xer_test_photo_zoomed");
}

} // namespace

auto main() -> int
{
    test_tk_photo_constants_and_type();
    test_tk_find_photo_null_name();
    test_tk_find_photo_missing_name();
    test_tk_find_photo_existing_and_size();
    test_tk_photo_set_size();
    test_tk_photo_expand();
    test_tk_photo_blank();
    test_tk_photo_get_image_null_block();
    test_tk_photo_put_block_null_block();
    test_tk_photo_put_and_get_block();
    test_tk_photo_put_zoomed_block();

    return 0;
}
