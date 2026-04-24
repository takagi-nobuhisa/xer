/**
 * @file xer/stdio.h
 * @brief Public stdio-like API header.
 */

#pragma once

#ifndef XER_STDIO_H_INCLUDED_
#define XER_STDIO_H_INCLUDED_

#include <xer/path.h>

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/stream_position.h>
#include <xer/bits/standard_streams.h>

#include <xer/bits/fopen.h>
#include <xer/bits/fclose.h>
#include <xer/bits/fflush.h>
#include <xer/bits/tmpfile.h>
#include <xer/bits/file_entry.h>

#include <xer/bits/binary_stream_io.h>
#include <xer/bits/text_stream_io.h>
#include <xer/bits/stream_position_io.h>
#include <xer/bits/rewind.h>

#include <xer/bits/feof.h>
#include <xer/bits/ferror.h>
#include <xer/bits/clearerr.h>

#include <xer/bits/to_native_handle.h>

#include <xer/bits/printf.h>
#include <xer/bits/scanf.h>

#include <xer/bits/csv.h>

#endif /* XER_STDIO_H_INCLUDED_ */
