/**
 * @file xer/zip.h
 * @brief Public ZIP archive API header.
 */

#pragma once

#ifndef XER_ZIP_H_INCLUDED_
#define XER_ZIP_H_INCLUDED_

#if defined(__has_include)
#    if __has_include(<zlib.h>)
#        include <zlib.h>
#    else
#        error "xer/zip.h requires zlib development headers."
#    endif
#else
#    include <zlib.h>
#endif

#include <xer/path.h>

#include <xer/bits/zip.h>

#endif /* XER_ZIP_H_INCLUDED_ */
