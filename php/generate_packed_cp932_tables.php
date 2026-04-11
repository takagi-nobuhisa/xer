<?php
declare(strict_types=1);

/**
 * @file generate_packed_cp932_tables.php
 * @brief Generate packed CP932 <-> UTF-16 lookup tables.
 *
 * Output:
 *   - packed_cp932_to_utf16_table : char16_t[65536]
 *   - utf16_to_packed_cp932_table : std::uint16_t[65536]
 *
 * Notes:
 *   - Invalid entry sentinel is 0xFFFF for both tables.
 *   - CP932 packed form:
 *       * 1-byte character: low 8 bits only
 *       * 2-byte character: first byte in low 8 bits, second byte in high 8 bits
 *   - UTF-16 side is BMP only because the table element type is char16_t.
 */

const INVALID_U16 = 0xFFFF;
const TABLE_SIZE = 0x10000;

/**
 * Convert packed CP932 to raw CP932 bytes.
 *
 * @param int $packed Packed CP932 value.
 * @return string Raw CP932 bytes.
 */
function packed_cp932_to_bytes(int $packed): string
{
    if (($packed & 0xFF00) === 0) {
        return chr($packed & 0xFF);
    }

    return chr($packed & 0xFF) . chr(($packed >> 8) & 0xFF);
}

/**
 * Convert raw CP932 bytes to packed CP932.
 *
 * @param string $bytes Raw CP932 bytes.
 * @return int Packed CP932 value, or INVALID_U16 on failure.
 */
function bytes_to_packed_cp932(string $bytes): int
{
    $length = strlen($bytes);

    if ($length === 1) {
        return ord($bytes[0]);
    }

    if ($length === 2) {
        return ord($bytes[0]) | (ord($bytes[1]) << 8);
    }

    return INVALID_U16;
}

/**
 * Convert packed CP932 to UTF-16 code unit.
 *
 * Returns INVALID_U16 on failure or if the byte sequence does not represent
 * exactly one BMP code point.
 *
 * @param int $packed Packed CP932 value.
 * @return int UTF-16 code unit, or INVALID_U16 on failure.
 */
function convert_packed_cp932_to_utf16(int $packed): int
{
    $cp932 = packed_cp932_to_bytes($packed);
    $utf16le = @iconv('CP932', 'UTF-16LE', $cp932);

    if ($utf16le === false) {
        return INVALID_U16;
    }

    if (strlen($utf16le) !== 2) {
        return INVALID_U16;
    }

    $unpacked = unpack('v', $utf16le);
    if ($unpacked === false) {
        return INVALID_U16;
    }

    return $unpacked[1];
}

/**
 * Convert UTF-16 code unit to packed CP932.
 *
 * Returns INVALID_U16 on failure.
 * Surrogates are rejected because a single char16_t cannot represent them
 * as a standalone Unicode scalar value.
 *
 * @param int $u16 UTF-16 code unit.
 * @return int Packed CP932 value, or INVALID_U16 on failure.
 */
function convert_utf16_to_packed_cp932(int $u16): int
{
    if ($u16 >= 0xD800 && $u16 <= 0xDFFF) {
        return INVALID_U16;
    }

    $utf16le = pack('v', $u16);
    $cp932 = @iconv('UTF-16LE', 'CP932', $utf16le);

    if ($cp932 === false) {
        return INVALID_U16;
    }

    $length = strlen($cp932);
    if ($length !== 1 && $length !== 2) {
        return INVALID_U16;
    }

    $roundtrip = @iconv('CP932', 'UTF-16LE', $cp932);
    if ($roundtrip === false || $roundtrip !== $utf16le) {
        return INVALID_U16;
    }

    return bytes_to_packed_cp932($cp932);
}

/**
 * Format one C++ array element as hexadecimal literal.
 *
 * @param int $value 16-bit value.
 * @return string Formatted hexadecimal literal.
 */
function format_hex16(int $value): string
{
    return sprintf('0x%04X', $value & 0xFFFF);
}

/**
 * Emit one C++ array.
 *
 * @param string $type C++ element type.
 * @param string $name C++ variable name.
 * @param array<int, int> $values Array values.
 * @return void
 */
function emit_array(string $type, string $name, array $values): void
{
    echo "inline constexpr {$type} {$name}[65536] = {\n";

    $count = count($values);
    for ($i = 0; $i < $count; $i += 8) {
        echo '    ';
        $chunk = array_slice($values, $i, 8);
        $formatted = array_map(
            static fn(int $v): string => format_hex16($v),
            $chunk
        );
        echo implode(', ', $formatted);

        if ($i + 8 < $count) {
            echo ',';
        }

        echo "\n";
    }

    echo "};\n";
}

/**
 * Build packed_cp932_to_utf16_table.
 *
 * @return array<int, int>
 */
function build_packed_cp932_to_utf16_table(): array
{
    $table = [];

    for ($packed = 0; $packed < TABLE_SIZE; ++$packed) {
        $table[] = convert_packed_cp932_to_utf16($packed);
    }

    return $table;
}

/**
 * Build utf16_to_packed_cp932_table.
 *
 * @return array<int, int>
 */
function build_utf16_to_packed_cp932_table(): array
{
    $table = [];

    for ($u16 = 0; $u16 < TABLE_SIZE; ++$u16) {
        $table[] = convert_utf16_to_packed_cp932($u16);
    }

    return $table;
}

$packedCp932ToUtf16Table = build_packed_cp932_to_utf16_table();
$utf16ToPackedCp932Table = build_utf16_to_packed_cp932_table();

echo "/**\n";
echo " * @file xer/bits/packed_cp932_tables.h\n";
echo " * @brief Packed CP932 <-> UTF-16 lookup tables.\n";
echo " */\n\n";
echo "#pragma once\n\n";
echo "#ifndef XER_BITS_PACKED_CP932_TABLES_H_INCLUDED_\n";
echo "#define XER_BITS_PACKED_CP932_TABLES_H_INCLUDED_\n\n";
echo "#include <cstdint>\n\n";
echo "namespace xer::detail {\n\n";

emit_array('char16_t', 'packed_cp932_to_utf16_table', $packedCp932ToUtf16Table);
echo "\n";
emit_array('std::uint16_t', 'utf16_to_packed_cp932_table', $utf16ToPackedCp932Table);

echo "\n} // namespace xer::detail\n\n";
echo "#endif /* XER_BITS_PACKED_CP932_TABLES_H_INCLUDED_ */\n";
