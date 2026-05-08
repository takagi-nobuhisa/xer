/**
 * @file xer/bits/tk.h
 * @brief Tk-side implementation for XER Tcl/Tk integration.
 */

#pragma once

#ifndef XER_BITS_TK_H_INCLUDED_
#define XER_BITS_TK_H_INCLUDED_

#include <expected>

#include <tk.h>

#include <xer/bits/common.h>
#include <xer/bits/tcl.h>
#include <xer/error.h>

namespace xer::tk {

using event_flag_t = int;
using photo_composite_rule_t = int;
using photo_image_block = Tk_PhotoImageBlock;

inline constexpr event_flag_t event_all = TCL_ALL_EVENTS;
inline constexpr event_flag_t event_window = TCL_WINDOW_EVENTS;
inline constexpr event_flag_t event_file = TCL_FILE_EVENTS;
inline constexpr event_flag_t event_timer = TCL_TIMER_EVENTS;
inline constexpr event_flag_t event_idle = TCL_IDLE_EVENTS;
inline constexpr event_flag_t event_dont_wait = TCL_DONT_WAIT;

inline constexpr photo_composite_rule_t photo_composite_overlay =
    TK_PHOTO_COMPOSITE_OVERLAY;
inline constexpr photo_composite_rule_t photo_composite_set =
    TK_PHOTO_COMPOSITE_SET;

class photo_image;

/**
 * @brief Returns the native Tk photo handle.
 *
 * This is an escape hatch for direct Tk C API use. The returned handle is
 * borrowed and follows the lifetime rules of the Tcl/Tk photo image.
 */
[[nodiscard]] inline auto to_native_handle(photo_image image) noexcept
    -> Tk_PhotoHandle;

/**
 * @brief Finds an existing Tk photo image by name.
 */
[[nodiscard]] inline auto find_photo(interpreter& interp, const char8_t* name) noexcept
    -> result<photo_image, error_detail>;

/**
 * @brief Non-owning handle for an existing Tk photo image.
 *
 * A `photo_image` value is created only by `find_photo`, and therefore does not
 * represent a null handle. It does not own the underlying Tk photo image. The
 * caller must ensure that the Tcl/Tk photo image outlives this handle.
 */
class photo_image {
public:
    photo_image() = delete;

private:
    explicit photo_image(Tk_PhotoHandle handle) noexcept
        : handle_(handle)
    {
    }

    Tk_PhotoHandle handle_;

    friend auto find_photo(interpreter& interp, const char8_t* name) noexcept
        -> result<photo_image, error_detail>;
    friend auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;
};

[[nodiscard]] inline auto to_native_handle(photo_image image) noexcept
    -> Tk_PhotoHandle
{
    return image.handle_;
}

struct photo_size {
    int width = 0;
    int height = 0;
};

/**
 * @brief Initializes both Tcl and Tk for an interpreter.
 */
[[nodiscard]] inline auto init(interpreter& interp) -> result<void, error_detail>
{
    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    int code = Tcl_Init(raw);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    code = Tk_Init(raw);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Finds an existing Tk photo image by name.
 */
[[nodiscard]] inline auto find_photo(interpreter& interp, const char8_t* name) noexcept
    -> result<photo_image, error_detail>
{
    if (name == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tk_PhotoHandle const handle = Tk_FindPhoto(
        raw,
        reinterpret_cast<const char*>(name));

    if (handle == nullptr) {
        return std::unexpected(detail::make_error(error_t::not_found));
    }

    return photo_image(handle);
}

/**
 * @brief Gets the current size of a Tk photo image.
 */
[[nodiscard]] inline auto photo_get_size(photo_image image) noexcept -> photo_size
{
    photo_size size;
    Tk_PhotoGetSize(to_native_handle(image), &size.width, &size.height);
    return size;
}

/**
 * @brief Blanks the entire Tk photo image.
 */
inline auto photo_blank(photo_image image) noexcept -> void
{
    Tk_PhotoBlank(to_native_handle(image));
}

/**
 * @brief Expands a Tk photo image to at least the specified size.
 */
[[nodiscard]] inline auto photo_expand(
    interpreter& interp,
    photo_image image,
    int width,
    int height) noexcept -> result<void, error_detail>
{
    if (width < 0 || height < 0) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tk_PhotoExpand(raw, to_native_handle(image), width, height);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Sets the explicit size of a Tk photo image.
 */
[[nodiscard]] inline auto photo_set_size(
    interpreter& interp,
    photo_image image,
    int width,
    int height) noexcept -> result<void, error_detail>
{
    if (width < 0 || height < 0) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tk_PhotoSetSize(raw, to_native_handle(image), width, height);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Gets the internal image block descriptor of a Tk photo image.
 */
[[nodiscard]] inline auto photo_get_image(
    photo_image image,
    photo_image_block* block) noexcept -> result<void, error_detail>
{
    if (block == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tk_PhotoGetImage(to_native_handle(image), block);
    if (code == 0) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Puts an image block into a Tk photo image.
 */
[[nodiscard]] inline auto photo_put_block(
    interpreter& interp,
    photo_image image,
    photo_image_block* block,
    int x,
    int y,
    int width,
    int height,
    photo_composite_rule_t rule = photo_composite_set) noexcept
    -> result<void, error_detail>
{
    if (block == nullptr || x < 0 || y < 0 || width < 0 || height < 0) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tk_PhotoPutBlock(
        raw,
        to_native_handle(image),
        block,
        x,
        y,
        width,
        height,
        rule);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Puts a zoomed or subsampled image block into a Tk photo image.
 */
[[nodiscard]] inline auto photo_put_zoomed_block(
    interpreter& interp,
    photo_image image,
    photo_image_block* block,
    int x,
    int y,
    int width,
    int height,
    int zoom_x,
    int zoom_y,
    int subsample_x,
    int subsample_y,
    photo_composite_rule_t rule = photo_composite_set) noexcept
    -> result<void, error_detail>
{
    if (block == nullptr || x < 0 || y < 0 || width < 0 || height < 0 ||
        zoom_x <= 0 || zoom_y <= 0 || subsample_x <= 0 || subsample_y <= 0) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tk_PhotoPutZoomedBlock(
        raw,
        to_native_handle(image),
        block,
        x,
        y,
        width,
        height,
        zoom_x,
        zoom_y,
        subsample_x,
        subsample_y,
        rule);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Runs the Tk main event loop.
 */
inline auto main_loop() -> void
{
    Tk_MainLoop();
}

/**
 * @brief Processes one Tcl/Tk event.
 */
inline auto do_one_event(event_flag_t flags = event_all) -> int
{
    return Tcl_DoOneEvent(flags);
}

} // namespace xer::tk

#endif /* XER_BITS_TK_H_INCLUDED_ */
