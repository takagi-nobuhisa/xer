#!/usr/bin/env php
<?php

declare(strict_types=1);

/**
 * @file convert_bdf_font.php
 * @brief Convert a monospaced BDF bitmap font to the XBF v1 binary format.
 *
 * Usage:
 *   php convert_bdf_font.php input.bdf output.xbf
 *   php convert_bdf_font.php input.bdf output.xbf --half-width=8 --full-width=16
 *
 * Options:
 *   --half-width=N  Explicit half-width XBF cell width.
 *   --full-width=N  Explicit full-width XBF cell width.
 *   --help          Show this help.
 */

const XBF_MAGIC = 'XBF0';
const XBF_VERSION = 1;
const XBF_HEADER_SIZE = 36;
const XBF_RANGE_ENTRY_SIZE = 24;

const XBF_WIDTH_KIND_HALF = 0;
const XBF_WIDTH_KIND_FULL = 1;

const UNICODE_MAX = 0x10FFFF;
const SURROGATE_FIRST = 0xD800;
const SURROGATE_LAST = 0xDFFF;

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
 * @return never
 */
function fail_line(int $lineNumber, string $message): never
{
    fail('line ' . $lineNumber . ': ' . $message);
}

/**
 * @return void
 */
function print_help(): void
{
    echo <<<TEXT
Usage:
  php convert_bdf_font.php input.bdf output.xbf
  php convert_bdf_font.php input.bdf output.xbf --half-width=8 --full-width=16

Options:
  --half-width=N  Explicit half-width XBF cell width.
  --full-width=N  Explicit full-width XBF cell width.
  --help          Show this help.

Notes:
  - Input must be a horizontal BDF 2.1 or 2.2 bitmap font.
  - XBF supports only two glyph cell widths: half-width and full-width.
  - ENCODING -1 glyphs are skipped.
  - Convertible glyphs must use DWIDTH <width> 0.
TEXT;
}

/**
 * @return array{
 *     input: string,
 *     output: string,
 *     half_width: ?int,
 *     full_width: ?int,
 *     help: bool
 * }
 */
function parse_options(array $argv): array
{
    $input = null;
    $output = null;
    $halfWidth = null;
    $fullWidth = null;
    $help = false;

    for ($index = 1; $index < count($argv); ++$index) {
        $arg = $argv[$index];

        if ($arg === '--help' || $arg === '-h') {
            $help = true;
            continue;
        }

        if (str_starts_with($arg, '--half-width=')) {
            if ($halfWidth !== null) {
                fail('duplicate option: --half-width', EXIT_USAGE_CODE);
            }

            $value = substr($arg, strlen('--half-width='));
            $halfWidth = parse_positive_integer_option('--half-width', $value);
            continue;
        }

        if (str_starts_with($arg, '--full-width=')) {
            if ($fullWidth !== null) {
                fail('duplicate option: --full-width', EXIT_USAGE_CODE);
            }

            $value = substr($arg, strlen('--full-width='));
            $fullWidth = parse_positive_integer_option('--full-width', $value);
            continue;
        }

        if (str_starts_with($arg, '--')) {
            fail('unknown option: ' . $arg, EXIT_USAGE_CODE);
        }

        if ($input === null) {
            $input = $arg;
            continue;
        }

        if ($output === null) {
            $output = $arg;
            continue;
        }

        fail('too many positional arguments', EXIT_USAGE_CODE);
    }

    if ($help) {
        return [
            'input' => '',
            'output' => '',
            'half_width' => $halfWidth,
            'full_width' => $fullWidth,
            'help' => true,
        ];
    }

    if ($input === null || $output === null) {
        fail(
            'input and output paths are required; use --help for usage',
            EXIT_USAGE_CODE
        );
    }

    if (($halfWidth === null) !== ($fullWidth === null)) {
        fail(
            '--half-width and --full-width must be specified together',
            EXIT_USAGE_CODE
        );
    }

    if ($halfWidth !== null && $fullWidth !== null && $halfWidth === $fullWidth) {
        fail(
            '--half-width and --full-width must be different values',
            EXIT_USAGE_CODE
        );
    }

    return [
        'input' => $input,
        'output' => $output,
        'half_width' => $halfWidth,
        'full_width' => $fullWidth,
        'help' => false,
    ];
}

/**
 * @return int
 */
function parse_positive_integer_option(string $name, string $value): int
{
    if (preg_match('/^[0-9]+$/D', $value) !== 1) {
        fail($name . ' must be a positive integer', EXIT_USAGE_CODE);
    }

    $number = intval($value, 10);
    if ($number <= 0 || $number > 0xFFFF) {
        fail($name . ' must be in the range 1..65535', EXIT_USAGE_CODE);
    }

    return $number;
}

/**
 * @return list<string>
 */
function read_bdf_lines(string $path): array
{
    $bytes = @file_get_contents($path);
    if ($bytes === false) {
        fail('failed to read input file: ' . $path);
    }

    if (str_starts_with($bytes, "\xEF\xBB\xBF")) {
        $bytes = substr($bytes, 3);
    }

    $lines = preg_split("/\r\n|\n|\r/", $bytes);
    if ($lines === false) {
        fail('failed to split input file into lines: ' . $path);
    }

    return $lines;
}

/**
 * @param list<string> $lines
 * @return array{
 *     version: string,
 *     bbox: array{width: int, height: int, xoff: int, yoff: int},
 *     glyphs: list<array{
 *         name: string,
 *         start_line: int,
 *         encoding: int,
 *         encoding_supplement: ?int,
 *         dwidth_x: int,
 *         dwidth_y: int,
 *         bbx: array{width: int, height: int, xoff: int, yoff: int},
 *         bitmap_rows: list<string>,
 *         bitmap_line: int
 *     }>
 * }
 */
function parse_bdf(array $lines): array
{
    $version = null;
    $bbox = null;
    $glyphs = [];
    $seenStartFont = false;
    $seenEndFont = false;
    $inProperties = false;
    $declaredChars = null;

    for ($index = 0; $index < count($lines); ++$index) {
        $lineNumber = $index + 1;
        $line = trim($lines[$index]);

        if (!$seenStartFont) {
            if ($line === '') {
                continue;
            }

            $match = [];
            if (preg_match('/^STARTFONT\s+([0-9]+\.[0-9]+)$/D', $line, $match) !== 1) {
                fail_line($lineNumber, 'STARTFONT 2.1 or STARTFONT 2.2 is required');
            }

            if ($match[1] !== '2.1' && $match[1] !== '2.2') {
                fail_line($lineNumber, 'unsupported BDF version: ' . $match[1]);
            }

            $version = $match[1];
            $seenStartFont = true;
            continue;
        }

        if ($seenEndFont) {
            if ($line !== '') {
                fail_line($lineNumber, 'unexpected content after ENDFONT');
            }

            continue;
        }

        if ($inProperties) {
            if ($line === 'ENDPROPERTIES') {
                $inProperties = false;
            }

            continue;
        }

        if ($line === '') {
            continue;
        }

        if (str_starts_with($line, 'COMMENT ')) {
            continue;
        }

        if (str_starts_with($line, 'STARTPROPERTIES ')) {
            $inProperties = true;
            continue;
        }

        if (str_starts_with($line, 'FONTBOUNDINGBOX ')) {
            if ($bbox !== null) {
                fail_line($lineNumber, 'duplicate FONTBOUNDINGBOX');
            }

            $bbox = parse_font_bounding_box($line, $lineNumber);
            continue;
        }

        if (str_starts_with($line, 'CHARS ')) {
            if ($declaredChars !== null) {
                fail_line($lineNumber, 'duplicate CHARS declaration');
            }

            $declaredChars = parse_chars_declaration($line, $lineNumber);
            continue;
        }

        if (str_starts_with($line, 'STARTCHAR ')) {
            [$glyph, $endIndex] = parse_glyph($lines, $index);
            $glyphs[] = $glyph;
            $index = $endIndex;
            continue;
        }

        if ($line === 'ENDFONT') {
            $seenEndFont = true;
            continue;
        }

        // Other global BDF fields are accepted and ignored.
    }

    if (!$seenStartFont) {
        fail('STARTFONT was not found');
    }

    if (!$seenEndFont) {
        fail('ENDFONT was not found');
    }

    if ($inProperties) {
        fail('ENDPROPERTIES was not found');
    }

    if ($bbox === null) {
        fail('FONTBOUNDINGBOX was not found');
    }

    if ($bbox['height'] <= 0 || $bbox['height'] > 0xFFFF) {
        fail('FONTBOUNDINGBOX height must be in the range 1..65535');
    }

    if ($declaredChars !== null && $declaredChars !== count($glyphs)) {
        fail(
            sprintf(
                'CHARS declares %d glyphs, but %d STARTCHAR blocks were found',
                $declaredChars,
                count($glyphs)
            )
        );
    }

    return [
        'version' => $version,
        'bbox' => $bbox,
        'glyphs' => $glyphs,
    ];
}

/**
 * @return array{width: int, height: int, xoff: int, yoff: int}
 */
function parse_font_bounding_box(string $line, int $lineNumber): array
{
    $match = [];
    if (
        preg_match(
            '/^FONTBOUNDINGBOX\s+(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)$/D',
            $line,
            $match
        ) !== 1
    ) {
        fail_line($lineNumber, 'invalid FONTBOUNDINGBOX declaration');
    }

    return [
        'width' => intval($match[1], 10),
        'height' => intval($match[2], 10),
        'xoff' => intval($match[3], 10),
        'yoff' => intval($match[4], 10),
    ];
}

/**
 * @return int
 */
function parse_chars_declaration(string $line, int $lineNumber): int
{
    $match = [];
    if (preg_match('/^CHARS\s+([0-9]+)$/D', $line, $match) !== 1) {
        fail_line($lineNumber, 'invalid CHARS declaration');
    }

    return intval($match[1], 10);
}

/**
 * @param list<string> $lines
 * @return array{
 *     0: array{
 *         name: string,
 *         start_line: int,
 *         encoding: int,
 *         encoding_supplement: ?int,
 *         dwidth_x: int,
 *         dwidth_y: int,
 *         bbx: array{width: int, height: int, xoff: int, yoff: int},
 *         bitmap_rows: list<string>,
 *         bitmap_line: int
 *     },
 *     1: int
 * }
 */
function parse_glyph(array $lines, int $startIndex): array
{
    $startLineNumber = $startIndex + 1;
    $startLine = trim($lines[$startIndex]);
    $name = trim(substr($startLine, strlen('STARTCHAR ')));
    if ($name === '') {
        fail_line($startLineNumber, 'STARTCHAR name must not be empty');
    }

    $encoding = null;
    $encodingSupplement = null;
    $dwidthX = null;
    $dwidthY = null;
    $bbx = null;
    $bitmapRows = null;
    $bitmapLine = null;

    for ($index = $startIndex + 1; $index < count($lines); ++$index) {
        $lineNumber = $index + 1;
        $line = trim($lines[$index]);

        if ($bitmapRows !== null) {
            if ($line === 'ENDCHAR') {
                if ($encoding === null) {
                    fail_line($startLineNumber, 'glyph ' . $name . ' is missing ENCODING');
                }

                if ($dwidthX === null || $dwidthY === null) {
                    fail_line($startLineNumber, 'glyph ' . $name . ' is missing DWIDTH');
                }

                if ($bbx === null) {
                    fail_line($startLineNumber, 'glyph ' . $name . ' is missing BBX');
                }

                validate_bitmap_rows($bitmapRows, $bbx, $bitmapLine ?? $lineNumber, $name);

                return [[
                    'name' => $name,
                    'start_line' => $startLineNumber,
                    'encoding' => $encoding,
                    'encoding_supplement' => $encodingSupplement,
                    'dwidth_x' => $dwidthX,
                    'dwidth_y' => $dwidthY,
                    'bbx' => $bbx,
                    'bitmap_rows' => $bitmapRows,
                    'bitmap_line' => $bitmapLine ?? $lineNumber,
                ], $index];
            }

            $bitmapRows[] = $line;
            continue;
        }

        if ($line === 'ENDCHAR') {
            fail_line($lineNumber, 'glyph ' . $name . ' is missing BITMAP');
        }

        if (str_starts_with($line, 'ENCODING ')) {
            if ($encoding !== null) {
                fail_line($lineNumber, 'duplicate ENCODING in glyph ' . $name);
            }

            [$encoding, $encodingSupplement] = parse_encoding($line, $lineNumber, $name);
            continue;
        }

        if (str_starts_with($line, 'DWIDTH ')) {
            if ($dwidthX !== null || $dwidthY !== null) {
                fail_line($lineNumber, 'duplicate DWIDTH in glyph ' . $name);
            }

            [$dwidthX, $dwidthY] = parse_dwidth($line, $lineNumber, $name);
            continue;
        }

        if (str_starts_with($line, 'BBX ')) {
            if ($bbx !== null) {
                fail_line($lineNumber, 'duplicate BBX in glyph ' . $name);
            }

            $bbx = parse_bbx($line, $lineNumber, $name);
            continue;
        }

        if ($line === 'BITMAP') {
            if ($bbx === null) {
                fail_line($lineNumber, 'BBX must appear before BITMAP in glyph ' . $name);
            }

            $bitmapRows = [];
            $bitmapLine = $lineNumber;
            continue;
        }

        // Other per-glyph BDF fields are accepted and ignored.
    }

    fail_line($startLineNumber, 'glyph ' . $name . ' is missing ENDCHAR');
}

/**
 * @return array{0: int, 1: ?int}
 */
function parse_encoding(string $line, int $lineNumber, string $glyphName): array
{
    $match = [];
    if (
        preg_match(
            '/^ENCODING\s+(-?[0-9]+)(?:\s+(-?[0-9]+))?$/D',
            $line,
            $match
        ) !== 1
    ) {
        fail_line($lineNumber, 'invalid ENCODING in glyph ' . $glyphName);
    }

    $encoding = intval($match[1], 10);
    $supplement = array_key_exists(2, $match) && $match[2] !== ''
        ? intval($match[2], 10)
        : null;

    if ($encoding < -1) {
        fail_line($lineNumber, 'ENCODING must be -1 or a non-negative integer in glyph ' . $glyphName);
    }

    if ($encoding === -1) {
        return [$encoding, $supplement];
    }

    if ($supplement !== null) {
        fail_line($lineNumber, 'supplemental ENCODING is only allowed with ENCODING -1 in glyph ' . $glyphName);
    }

    validate_unicode_scalar($encoding, $lineNumber, 'ENCODING in glyph ' . $glyphName);
    return [$encoding, null];
}

/**
 * @return array{0: int, 1: int}
 */
function parse_dwidth(string $line, int $lineNumber, string $glyphName): array
{
    $match = [];
    if (
        preg_match(
            '/^DWIDTH\s+(-?[0-9]+)\s+(-?[0-9]+)$/D',
            $line,
            $match
        ) !== 1
    ) {
        fail_line($lineNumber, 'invalid DWIDTH in glyph ' . $glyphName);
    }

    $x = intval($match[1], 10);
    $y = intval($match[2], 10);

    if ($x <= 0 || $x > 0xFFFF) {
        fail_line($lineNumber, 'DWIDTH x must be in the range 1..65535 in glyph ' . $glyphName);
    }

    if ($y !== 0) {
        fail_line($lineNumber, 'DWIDTH y must be 0 in glyph ' . $glyphName);
    }

    return [$x, $y];
}

/**
 * @return array{width: int, height: int, xoff: int, yoff: int}
 */
function parse_bbx(string $line, int $lineNumber, string $glyphName): array
{
    $match = [];
    if (
        preg_match(
            '/^BBX\s+(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)\s+(-?[0-9]+)$/D',
            $line,
            $match
        ) !== 1
    ) {
        fail_line($lineNumber, 'invalid BBX in glyph ' . $glyphName);
    }

    $width = intval($match[1], 10);
    $height = intval($match[2], 10);

    if ($width < 0 || $height < 0) {
        fail_line($lineNumber, 'BBX width and height must be non-negative in glyph ' . $glyphName);
    }

    return [
        'width' => $width,
        'height' => $height,
        'xoff' => intval($match[3], 10),
        'yoff' => intval($match[4], 10),
    ];
}

/**
 * @param list<string> $rows
 * @param array{width: int, height: int, xoff: int, yoff: int} $bbx
 * @return void
 */
function validate_bitmap_rows(array $rows, array $bbx, int $bitmapLine, string $glyphName): void
{
    if (count($rows) !== $bbx['height']) {
        fail_line(
            $bitmapLine,
            sprintf(
                'BITMAP row count is %d; expected %d in glyph %s',
                count($rows),
                $bbx['height'],
                $glyphName
            )
        );
    }

    $bytesPerRow = intdiv($bbx['width'] + 7, 8);
    $hexLength = $bytesPerRow * 2;
    $unusedBits = $bytesPerRow * 8 - $bbx['width'];

    foreach ($rows as $rowIndex => $row) {
        $lineNumber = $bitmapLine + 1 + $rowIndex;

        if (strlen($row) !== $hexLength) {
            fail_line(
                $lineNumber,
                sprintf(
                    'BITMAP row length is %d; expected %d hex digits in glyph %s',
                    strlen($row),
                    $hexLength,
                    $glyphName
                )
            );
        }

        if ($hexLength > 0 && preg_match('/^[0-9A-Fa-f]+$/D', $row) !== 1) {
            fail_line($lineNumber, 'BITMAP row contains non-hex characters in glyph ' . $glyphName);
        }

        if ($hexLength === 0 && $row !== '') {
            fail_line($lineNumber, 'zero-width BITMAP row must be empty in glyph ' . $glyphName);
        }

        if ($bytesPerRow > 0 && $unusedBits > 0) {
            $bytes = hex2bin($row);
            if ($bytes === false) {
                fail_line($lineNumber, 'failed to decode BITMAP row in glyph ' . $glyphName);
            }

            $lastByte = ord($bytes[$bytesPerRow - 1]);
            $unusedMask = (1 << $unusedBits) - 1;
            if (($lastByte & $unusedMask) !== 0) {
                fail_line($lineNumber, 'unused low-order BITMAP bits must be zero in glyph ' . $glyphName);
            }
        }
    }
}

/**
 * @return void
 */
function validate_unicode_scalar(int $codePoint, int $lineNumber, string $context): void
{
    if ($codePoint < 0 || $codePoint > UNICODE_MAX) {
        fail_line($lineNumber, $context . ' is outside the Unicode scalar range');
    }

    if ($codePoint >= SURROGATE_FIRST && $codePoint <= SURROGATE_LAST) {
        fail_line($lineNumber, $context . ' is in the surrogate range');
    }
}

/**
 * @param list<array{
 *     name: string,
 *     start_line: int,
 *     encoding: int,
 *     encoding_supplement: ?int,
 *     dwidth_x: int,
 *     dwidth_y: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * }> $glyphs
 * @return list<array{
 *     name: string,
 *     start_line: int,
 *     code_point: int,
 *     dwidth_x: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * }>
 */
function collect_convertible_glyphs(array $glyphs): array
{
    $convertible = [];
    $seenCodePoints = [];

    foreach ($glyphs as $glyph) {
        if ($glyph['encoding'] === -1) {
            continue;
        }

        $codePoint = $glyph['encoding'];
        if (array_key_exists((string)$codePoint, $seenCodePoints)) {
            fail_line(
                $glyph['start_line'],
                sprintf('duplicate Unicode code point U+%04X', $codePoint)
            );
        }

        $seenCodePoints[(string)$codePoint] = true;

        $convertible[] = [
            'name' => $glyph['name'],
            'start_line' => $glyph['start_line'],
            'code_point' => $codePoint,
            'dwidth_x' => $glyph['dwidth_x'],
            'bbx' => $glyph['bbx'],
            'bitmap_rows' => $glyph['bitmap_rows'],
            'bitmap_line' => $glyph['bitmap_line'],
        ];
    }

    if ($convertible === []) {
        fail('no glyph with a non-negative ENCODING was found');
    }

    usort(
        $convertible,
        static fn(array $left, array $right): int =>
            $left['code_point'] <=> $right['code_point']
    );

    return $convertible;
}

/**
 * @param list<array{
 *     name: string,
 *     start_line: int,
 *     code_point: int,
 *     dwidth_x: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * }> $glyphs
 * @return array{half_width: int, full_width: int}
 */
function determine_cell_widths(array $glyphs, ?int $explicitHalfWidth, ?int $explicitFullWidth): array
{
    if ($explicitHalfWidth !== null && $explicitFullWidth !== null) {
        foreach ($glyphs as $glyph) {
            if (
                $glyph['dwidth_x'] !== $explicitHalfWidth
                && $glyph['dwidth_x'] !== $explicitFullWidth
            ) {
                fail_line(
                    $glyph['start_line'],
                    sprintf(
                        'DWIDTH x=%d does not match --half-width=%d or --full-width=%d for U+%04X',
                        $glyph['dwidth_x'],
                        $explicitHalfWidth,
                        $explicitFullWidth,
                        $glyph['code_point']
                    )
                );
            }
        }

        return [
            'half_width' => $explicitHalfWidth,
            'full_width' => $explicitFullWidth,
        ];
    }

    $uniqueWidths = [];
    foreach ($glyphs as $glyph) {
        $uniqueWidths[(string)$glyph['dwidth_x']] = $glyph['dwidth_x'];
    }

    $widths = array_values($uniqueWidths);
    sort($widths, SORT_NUMERIC);

    if (count($widths) === 1) {
        $halfWidth = $widths[0];
        $fullWidth = $halfWidth * 2;

        if ($fullWidth > 0xFFFF) {
            fail('inferred full-width cell width exceeds 65535; specify explicit widths instead');
        }

        return [
            'half_width' => $halfWidth,
            'full_width' => $fullWidth,
        ];
    }

    if (count($widths) === 2) {
        return [
            'half_width' => $widths[0],
            'full_width' => $widths[1],
        ];
    }

    fail('more than two distinct DWIDTH x values were found; XBF supports only half-width and full-width cells');
}

/**
 * @param list<array{
 *     name: string,
 *     start_line: int,
 *     code_point: int,
 *     dwidth_x: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * }> $glyphs
 * @param array{width: int, height: int, xoff: int, yoff: int} $fontBoundingBox
 * @param array{half_width: int, full_width: int} $cellWidths
 * @return list<array{
 *     code_point: int,
 *     width_kind: int,
 *     bitmap: string
 * }>
 */
function rasterize_glyph_cells(array $glyphs, array $fontBoundingBox, array $cellWidths): array
{
    $rasterized = [];

    foreach ($glyphs as $glyph) {
        if ($glyph['dwidth_x'] === $cellWidths['half_width']) {
            $widthKind = XBF_WIDTH_KIND_HALF;
            $cellWidth = $cellWidths['half_width'];
        } elseif ($glyph['dwidth_x'] === $cellWidths['full_width']) {
            $widthKind = XBF_WIDTH_KIND_FULL;
            $cellWidth = $cellWidths['full_width'];
        } else {
            fail_line(
                $glyph['start_line'],
                sprintf(
                    'DWIDTH x=%d cannot be classified as half-width or full-width for U+%04X',
                    $glyph['dwidth_x'],
                    $glyph['code_point']
                )
            );
        }

        $destX = $glyph['bbx']['xoff'] - $fontBoundingBox['xoff'];
        $destY =
            $fontBoundingBox['height']
            - ($glyph['bbx']['yoff'] - $fontBoundingBox['yoff'])
            - $glyph['bbx']['height'];

        validate_glyph_placement(
            $glyph,
            $destX,
            $destY,
            $cellWidth,
            $fontBoundingBox['height']
        );

        $rasterized[] = [
            'code_point' => $glyph['code_point'],
            'width_kind' => $widthKind,
            'bitmap' => rasterize_single_glyph(
                $glyph,
                $destX,
                $destY,
                $cellWidth,
                $fontBoundingBox['height']
            ),
        ];
    }

    return $rasterized;
}

/**
 * @param array{
 *     name: string,
 *     start_line: int,
 *     code_point: int,
 *     dwidth_x: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * } $glyph
 * @return void
 */
function validate_glyph_placement(
    array $glyph,
    int $destX,
    int $destY,
    int $cellWidth,
    int $cellHeight
): void {
    if (
        $destX < 0
        || $destY < 0
        || $destX + $glyph['bbx']['width'] > $cellWidth
        || $destY + $glyph['bbx']['height'] > $cellHeight
    ) {
        fail_line(
            $glyph['start_line'],
            sprintf(
                'glyph U+%04X does not fit in the %d x %d XBF cell',
                $glyph['code_point'],
                $cellWidth,
                $cellHeight
            )
        );
    }
}

/**
 * @param array{
 *     name: string,
 *     start_line: int,
 *     code_point: int,
 *     dwidth_x: int,
 *     bbx: array{width: int, height: int, xoff: int, yoff: int},
 *     bitmap_rows: list<string>,
 *     bitmap_line: int
 * } $glyph
 * @return string
 */
function rasterize_single_glyph(
    array $glyph,
    int $destX,
    int $destY,
    int $cellWidth,
    int $cellHeight
): string {
    $cellBytesPerRow = intdiv($cellWidth + 7, 8);
    $cellBytes = array_fill(0, $cellBytesPerRow * $cellHeight, 0);

    $sourceWidth = $glyph['bbx']['width'];
    $sourceRows = $glyph['bitmap_rows'];

    foreach ($sourceRows as $sourceY => $hexRow) {
        $rowBytes = $hexRow === '' ? '' : hex2bin($hexRow);
        if ($rowBytes === false) {
            fail_line(
                $glyph['bitmap_line'] + 1 + $sourceY,
                sprintf('failed to decode BITMAP row for U+%04X', $glyph['code_point'])
            );
        }

        for ($sourceX = 0; $sourceX < $sourceWidth; ++$sourceX) {
            if (!source_bit_is_set($rowBytes, $sourceX)) {
                continue;
            }

            $targetX = $destX + $sourceX;
            $targetY = $destY + $sourceY;
            $targetIndex = $targetY * $cellBytesPerRow + intdiv($targetX, 8);
            $targetMask = 1 << (7 - ($targetX % 8));
            $cellBytes[$targetIndex] |= $targetMask;
        }
    }

    $bytes = '';
    foreach ($cellBytes as $value) {
        $bytes .= chr($value);
    }

    return $bytes;
}

/**
 * @return bool
 */
function source_bit_is_set(string $rowBytes, int $x): bool
{
    $byteIndex = intdiv($x, 8);
    $bitIndex = 7 - ($x % 8);
    $byteValue = ord($rowBytes[$byteIndex]);
    return (($byteValue >> $bitIndex) & 1) !== 0;
}

/**
 * @param list<array{
 *     code_point: int,
 *     width_kind: int,
 *     bitmap: string
 * }> $glyphs
 * @return array{
 *     ranges: list<array{
 *         first_code_point: int,
 *         last_code_point: int,
 *         width_kind: int,
 *         bitmap_offset: int,
 *         bitmap: string
 *     }>,
 *     bitmap_data: string
 * }
 */
function build_ranges(array $glyphs): array
{
    $ranges = [];
    $current = null;
    $bitmapData = '';
    $bitmapOffset = 0;

    foreach ($glyphs as $glyph) {
        if ($current === null) {
            $current = [
                'first_code_point' => $glyph['code_point'],
                'last_code_point' => $glyph['code_point'],
                'width_kind' => $glyph['width_kind'],
                'bitmap_offset' => $bitmapOffset,
                'bitmap' => $glyph['bitmap'],
            ];
            continue;
        }

        $isContiguous = $glyph['code_point'] === $current['last_code_point'] + 1;
        $hasSameWidthKind = $glyph['width_kind'] === $current['width_kind'];

        if ($isContiguous && $hasSameWidthKind) {
            $current['last_code_point'] = $glyph['code_point'];
            $current['bitmap'] .= $glyph['bitmap'];
            continue;
        }

        $ranges[] = $current;
        $bitmapData .= $current['bitmap'];
        $bitmapOffset += strlen($current['bitmap']);

        $current = [
            'first_code_point' => $glyph['code_point'],
            'last_code_point' => $glyph['code_point'],
            'width_kind' => $glyph['width_kind'],
            'bitmap_offset' => $bitmapOffset,
            'bitmap' => $glyph['bitmap'],
        ];
    }

    if ($current !== null) {
        $ranges[] = $current;
        $bitmapData .= $current['bitmap'];
    }

    if (count($ranges) > 0xFFFFFFFF) {
        fail('range count exceeds the XBF v1 limit');
    }

    return [
        'ranges' => $ranges,
        'bitmap_data' => $bitmapData,
    ];
}

/**
 * @param list<array{
 *     first_code_point: int,
 *     last_code_point: int,
 *     width_kind: int,
 *     bitmap_offset: int,
 *     bitmap: string
 * }> $ranges
 * @return string
 */
function build_xbf_bytes(
    array $ranges,
    string $bitmapData,
    int $halfWidth,
    int $fullWidth,
    int $glyphHeight
): string {
    validate_u16($halfWidth, 'half-width cell width');
    validate_u16($fullWidth, 'full-width cell width');
    validate_u16($glyphHeight, 'glyph height');

    $rangeCount = count($ranges);
    $rangeTableOffset = XBF_HEADER_SIZE;
    $bitmapDataOffset = XBF_HEADER_SIZE + XBF_RANGE_ENTRY_SIZE * $rangeCount;

    $header =
        XBF_MAGIC
        . pack_u16_le(XBF_VERSION)
        . pack_u16_le(XBF_HEADER_SIZE)
        . pack_u16_le($halfWidth)
        . pack_u16_le($fullWidth)
        . pack_u16_le($glyphHeight)
        . pack_u16_le(0)
        . pack_u32_le($rangeCount)
        . pack_u64_le($rangeTableOffset)
        . pack_u64_le($bitmapDataOffset);

    if (strlen($header) !== XBF_HEADER_SIZE) {
        fail('internal error: generated XBF header size is invalid');
    }

    $rangeTable = '';
    foreach ($ranges as $range) {
        $rangeTable .=
            pack_u32_le($range['first_code_point'])
            . pack_u32_le($range['last_code_point'])
            . chr($range['width_kind'])
            . str_repeat("\x00", 7)
            . pack_u64_le($range['bitmap_offset']);
    }

    if (strlen($rangeTable) !== XBF_RANGE_ENTRY_SIZE * $rangeCount) {
        fail('internal error: generated XBF range table size is invalid');
    }

    return $header . $rangeTable . $bitmapData;
}

/**
 * @return void
 */
function validate_u16(int $value, string $name): void
{
    if ($value < 0 || $value > 0xFFFF) {
        fail($name . ' is outside the XBF uint16 range');
    }
}

/**
 * @return string
 */
function pack_u16_le(int $value): string
{
    validate_u16($value, 'uint16 value');
    return chr($value & 0xFF) . chr(($value >> 8) & 0xFF);
}

/**
 * @return string
 */
function pack_u32_le(int $value): string
{
    if ($value < 0 || $value > 0xFFFFFFFF) {
        fail('uint32 value is outside the XBF range');
    }

    return
        chr($value & 0xFF)
        . chr(($value >> 8) & 0xFF)
        . chr(($value >> 16) & 0xFF)
        . chr(($value >> 24) & 0xFF);
}

/**
 * @return string
 */
function pack_u64_le(int $value): string
{
    if ($value < 0) {
        fail('uint64 value must be non-negative');
    }

    $bytes = '';
    for ($shift = 0; $shift < 64; $shift += 8) {
        $bytes .= chr(($value >> $shift) & 0xFF);
    }

    return $bytes;
}

/**
 * @return void
 */
function write_binary_file(string $path, string $bytes): void
{
    $directory = dirname($path);
    if ($directory !== '' && $directory !== '.' && !is_dir($directory)) {
        if (!mkdir($directory, 0777, true) && !is_dir($directory)) {
            fail('failed to create output directory: ' . $directory);
        }
    }

    $written = @file_put_contents($path, $bytes);
    if ($written === false || $written !== strlen($bytes)) {
        fail('failed to write output file: ' . $path);
    }
}

/**
 * @return int
 */
function main(array $argv): int
{
    $options = parse_options($argv);

    if ($options['help']) {
        print_help();
        return EXIT_SUCCESS_CODE;
    }

    $lines = read_bdf_lines($options['input']);
    $bdf = parse_bdf($lines);
    $glyphs = collect_convertible_glyphs($bdf['glyphs']);
    $cellWidths = determine_cell_widths(
        $glyphs,
        $options['half_width'],
        $options['full_width']
    );

    $rasterized = rasterize_glyph_cells($glyphs, $bdf['bbox'], $cellWidths);
    $rangeData = build_ranges($rasterized);
    $xbf = build_xbf_bytes(
        $rangeData['ranges'],
        $rangeData['bitmap_data'],
        $cellWidths['half_width'],
        $cellWidths['full_width'],
        $bdf['bbox']['height']
    );

    write_binary_file($options['output'], $xbf);

    echo 'Converted: ' . $options['input'] . ' -> ' . $options['output'] . PHP_EOL;
    echo 'Glyphs:    ' . count($rasterized) . PHP_EOL;
    echo 'Ranges:    ' . count($rangeData['ranges']) . PHP_EOL;
    echo 'Bytes:     ' . strlen($xbf) . PHP_EOL;

    return EXIT_SUCCESS_CODE;
}

exit(main($argv));
