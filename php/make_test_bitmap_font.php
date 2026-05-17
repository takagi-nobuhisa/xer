#!/usr/bin/env php
<?php

declare(strict_types=1);

/**
 * @file make_test_bitmap_font.php
 * @brief Generate a small XBF test font for bitmap-font loader and drawing tests.
 *
 * Usage:
 *   php make_test_bitmap_font.php [output-path]
 *
 * Default output:
 *   ../tests/assets/test_bitmap_font.xbf
 *
 * The generated font contains:
 *   - U+003F - U+0042 as half-width glyphs: ?, @, A, B
 *   - U+3042 - U+3044 as full-width glyphs: あ, ぃ, い
 */

const XBF_MAGIC = 'XBF0';
const XBF_VERSION = 1;
const XBF_HEADER_SIZE = 36;
const XBF_RANGE_ENTRY_SIZE = 24;

const GLYPH_WIDTH_HALF = 8;
const GLYPH_WIDTH_FULL = 16;
const GLYPH_HEIGHT = 8;

const WIDTH_KIND_HALF = 0;
const WIDTH_KIND_FULL = 1;

const EXIT_SUCCESS_CODE = 0;
const EXIT_FAILURE_CODE = 1;
const EXIT_USAGE_CODE = 2;

/**
 * @return never
 */
function fail(string $message, int $exitCode = EXIT_FAILURE_CODE): never
{
    fwrite(STDERR, 'error: ' . $message . PHP_EOL);
    exit($exitCode);
}

/**
 * @return string
 */
function normalize_path(string $path): string
{
    return str_replace('\\', '/', $path);
}

/**
 * @return string
 */
function repository_root(): string
{
    return normalize_path(dirname(__DIR__));
}

/**
 * @return string
 */
function default_output_path(): string
{
    return repository_root() . '/tests/assets/test_bitmap_font.xbf';
}

/**
 * @return string
 */
function output_path_from_arguments(array $argv): string
{
    if (count($argv) > 2) {
        fail(
            "too many arguments\n"
            . 'usage: php make_test_bitmap_font.php [output-path]',
            EXIT_USAGE_CODE
        );
    }

    if (count($argv) === 2) {
        $path = trim($argv[1]);
        if ($path === '') {
            fail('output path must not be empty', EXIT_USAGE_CODE);
        }

        return normalize_path($path);
    }

    return default_output_path();
}

/**
 * @return void
 */
function ensure_parent_directory(string $path): void
{
    $directory = dirname($path);

    if ($directory === '' || $directory === '.' || is_dir($directory)) {
        return;
    }

    if (!mkdir($directory, 0777, true) && !is_dir($directory)) {
        fail('failed to create directory: ' . $directory);
    }
}

/**
 * @return void
 */
function write_binary_file(string $path, string $bytes): void
{
    ensure_parent_directory($path);

    $written = file_put_contents($path, $bytes);
    if ($written === false || $written !== strlen($bytes)) {
        fail('failed to write file: ' . $path);
    }
}

/**
 * @return string
 */
function pack_u16_le(int $value): string
{
    if ($value < 0 || $value > 0xFFFF) {
        fail('uint16 value is out of range: ' . $value);
    }

    return pack('v', $value);
}

/**
 * @return string
 */
function pack_u32_le(int $value): string
{
    if ($value < 0 || $value > 0xFFFFFFFF) {
        fail('uint32 value is out of range: ' . $value);
    }

    return pack('V', $value);
}

/**
 * @return string
 */
function pack_u64_le(int $value): string
{
    if ($value < 0) {
        fail('uint64 value must be non-negative: ' . $value);
    }

    return pack('P', $value);
}

/**
 * @param list<string> $rows
 * @return string
 */
function pack_glyph_rows(array $rows, int $width, int $height, string $glyphName): string
{
    if (count($rows) !== $height) {
        fail(
            sprintf(
                'glyph %s has %d rows; expected %d',
                $glyphName,
                count($rows),
                $height
            )
        );
    }

    $bytes = '';

    foreach ($rows as $rowIndex => $row) {
        if (strlen($row) !== $width) {
            fail(
                sprintf(
                    'glyph %s row %d has width %d; expected %d',
                    $glyphName,
                    $rowIndex,
                    strlen($row),
                    $width
                )
            );
        }

        if (preg_match('/^[01]+$/D', $row) !== 1) {
            fail(
                sprintf(
                    'glyph %s row %d contains characters other than 0 or 1',
                    $glyphName,
                    $rowIndex
                )
            );
        }

        for ($offset = 0; $offset < $width; $offset += 8) {
            $chunk = substr($row, $offset, 8);
            if (strlen($chunk) < 8) {
                $chunk = str_pad($chunk, 8, '0');
            }

            $bytes .= chr(bindec($chunk));
        }
    }

    return $bytes;
}

/**
 * @param array<int, list<string>> $glyphs
 * @return string
 */
function build_range_bitmap(
    array $glyphs,
    int $firstCodePoint,
    int $lastCodePoint,
    int $glyphWidth,
    string $rangeName
): string {
    $bytes = '';

    for ($codePoint = $firstCodePoint; $codePoint <= $lastCodePoint; ++$codePoint) {
        if (!array_key_exists($codePoint, $glyphs)) {
            fail(
                sprintf(
                    'range %s is missing glyph U+%04X',
                    $rangeName,
                    $codePoint
                )
            );
        }

        $bytes .= pack_glyph_rows(
            $glyphs[$codePoint],
            $glyphWidth,
            GLYPH_HEIGHT,
            sprintf('U+%04X', $codePoint)
        );
    }

    return $bytes;
}

/**
 * @return array<int, list<string>>
 */
function half_width_glyphs(): array
{
    return [
        0x003F => [
            '00111100',
            '01000010',
            '00000010',
            '00001100',
            '00010000',
            '00000000',
            '00010000',
            '00000000',
        ],
        0x0040 => [
            '00111100',
            '01000010',
            '01011010',
            '01010110',
            '01011100',
            '01000000',
            '00111100',
            '00000000',
        ],
        0x0041 => [
            '00011000',
            '00100100',
            '01000010',
            '01111110',
            '01000010',
            '01000010',
            '01000010',
            '00000000',
        ],
        0x0042 => [
            '01111100',
            '01000010',
            '01000010',
            '01111100',
            '01000010',
            '01000010',
            '01111100',
            '00000000',
        ],
    ];
}

/**
 * @return array<int, list<string>>
 */
function full_width_glyphs(): array
{
    return [
        0x3042 => [
            '0000011111000000',
            '0000000100000000',
            '0011111111110000',
            '0001000100010000',
            '0010000100100000',
            '0100000101000000',
            '0011111010000000',
            '0000000000000000',
        ],
        0x3043 => [
            '0000000000000000',
            '0000001111000000',
            '0000010000100000',
            '0000000000100000',
            '0000000001000000',
            '0000000010000000',
            '0000000100000000',
            '0000000000000000',
        ],
        0x3044 => [
            '0000010000010000',
            '0000010000010000',
            '0000010000010000',
            '0000010000010000',
            '0000010000100000',
            '0000001001000000',
            '0000000110000000',
            '0000000000000000',
        ],
    ];
}

/**
 * @return string
 */
function build_xbf_bytes(): string
{
    $halfBitmap = build_range_bitmap(
        half_width_glyphs(),
        0x003F,
        0x0042,
        GLYPH_WIDTH_HALF,
        'half-width ASCII test range'
    );

    $fullBitmap = build_range_bitmap(
        full_width_glyphs(),
        0x3042,
        0x3044,
        GLYPH_WIDTH_FULL,
        'full-width hiragana test range'
    );

    $rangeCount = 2;
    $rangeTableOffset = XBF_HEADER_SIZE;
    $bitmapDataOffset = XBF_HEADER_SIZE + XBF_RANGE_ENTRY_SIZE * $rangeCount;

    $header =
        XBF_MAGIC
        . pack_u16_le(XBF_VERSION)
        . pack_u16_le(XBF_HEADER_SIZE)
        . pack_u16_le(GLYPH_WIDTH_HALF)
        . pack_u16_le(GLYPH_WIDTH_FULL)
        . pack_u16_le(GLYPH_HEIGHT)
        . pack_u16_le(0)
        . pack_u32_le($rangeCount)
        . pack_u64_le($rangeTableOffset)
        . pack_u64_le($bitmapDataOffset);

    if (strlen($header) !== XBF_HEADER_SIZE) {
        fail(
            sprintf(
                'internal error: header size is %d; expected %d',
                strlen($header),
                XBF_HEADER_SIZE
            )
        );
    }

    $rangeTable =
        pack_u32_le(0x003F)
        . pack_u32_le(0x0042)
        . chr(WIDTH_KIND_HALF)
        . str_repeat("\x00", 7)
        . pack_u64_le(0)
        . pack_u32_le(0x3042)
        . pack_u32_le(0x3044)
        . chr(WIDTH_KIND_FULL)
        . str_repeat("\x00", 7)
        . pack_u64_le(strlen($halfBitmap));

    if (strlen($rangeTable) !== XBF_RANGE_ENTRY_SIZE * $rangeCount) {
        fail(
            sprintf(
                'internal error: range table size is %d; expected %d',
                strlen($rangeTable),
                XBF_RANGE_ENTRY_SIZE * $rangeCount
            )
        );
    }

    return $header . $rangeTable . $halfBitmap . $fullBitmap;
}

/**
 * @return int
 */
function main(array $argv): int
{
    $outputPath = output_path_from_arguments($argv);
    $xbf = build_xbf_bytes();

    write_binary_file($outputPath, $xbf);

    echo 'Generated: ' . $outputPath . PHP_EOL;
    echo 'Bytes:     ' . strlen($xbf) . PHP_EOL;

    return EXIT_SUCCESS_CODE;
}

exit(main($argv));
