<?php

declare(strict_types=1);

/**
 * @file generate_xfer_struct.php
 * @brief Generate simple C++ structs and xfer functions from a PHP schema array.
 *
 * This script assumes that it is executed in the php directory or from the
 * project root. The generated C++ source is written as UTF-8 with BOM.
 */

const EXIT_SUCCESS_CODE = 0;
const EXIT_FAILURE_CODE = 1;

const XER_GENERATED_BOM = "\xEF\xBB\xBF";

// Short type tokens used by schema files.
defined('b') || define('b', 'b');

defined('u8') || define('u8', 'u8');
defined('u16') || define('u16', 'u16');
defined('u32') || define('u32', 'u32');
defined('u64') || define('u64', 'u64');

defined('i8') || define('i8', 'i8');
defined('i16') || define('i16', 'i16');
defined('i32') || define('i32', 'i32');
defined('i64') || define('i64', 'i64');

defined('f32') || define('f32', 'f32');
defined('f64') || define('f64', 'f64');

defined('s') || define('s', 's');
defined('bin') || define('bin', 'bin');

defined('a') || define('a', 'a');
defined('v') || define('v', 'v');
defined('m') || define('m', 'm');

/**
 * @return never
 */
function fail(string $message): never
{
    fwrite(STDERR, $message . PHP_EOL);
    exit(EXIT_FAILURE_CODE);
}

/**
 * @return void
 */
function print_usage(): void
{
    $usage = <<<'TEXT'
Usage:
  php generate_xfer_struct.php <schema.php> <output.hpp> [options]

Options:
  --struct=<name>          Struct name for a schema file that returns a field
                           map. When the schema contains a "structs" map, this
                           filters generation to one struct.
  --namespace=<name>       C++ namespace, for example "demo" or "demo::wire".
                           This overrides a schema-level "namespace" entry.
  --generated-at=<text>    Override the generated-at schema version string.
                           The default is the current time in ISO 8601 format.
  --help                   Show this help.

Schema examples:

  return [
      'id'      => u32,
      'name'    => s,
      'values'  => [f64, v],
      'fixed'   => [u32, [a, 16]],
      'scores'  => [[s, f64], m],
  ];

  return [
      'namespace' => 'demo',
      'struct' => 'sample',
      'fields' => [
          'id' => u32,
          'payload' => bin,
      ],
  ];

  return [
      'namespace' => 'demo',
      'structs' => [
          'sample_header' => [
              'id' => u32,
              'name' => s,
          ],
          'sample_packet' => [
              'header' => bin,
              'values' => [f64, v],
          ],
      ],
  ];

Type tokens:
  b
  u8 u16 u32 u64
  i8 i16 i32 i64
  f32 f64
  s
  bin

Container forms:
  [T, v]          -> std::vector<T>
  [T, [a, N]]     -> std::array<T, N>
  [[K, V], m]     -> std::map<K, V>
TEXT;

    fwrite(STDOUT, $usage . PHP_EOL);
}

/**
 * @return string
 */
function normalize_path(string $path): string
{
    return str_replace('\\', '/', $path);
}

/**
 * @return bool
 */
function is_cpp_identifier(string $name): bool
{
    static $keywords = [
        'alignas' => true, 'alignof' => true, 'and' => true, 'and_eq' => true,
        'asm' => true, 'auto' => true, 'bitand' => true, 'bitor' => true,
        'bool' => true, 'break' => true, 'case' => true, 'catch' => true,
        'char' => true, 'char8_t' => true, 'char16_t' => true, 'char32_t' => true,
        'class' => true, 'compl' => true, 'concept' => true, 'const' => true,
        'consteval' => true, 'constexpr' => true, 'constinit' => true,
        'const_cast' => true, 'continue' => true, 'co_await' => true,
        'co_return' => true, 'co_yield' => true, 'decltype' => true,
        'default' => true, 'delete' => true, 'do' => true, 'double' => true,
        'dynamic_cast' => true, 'else' => true, 'enum' => true, 'explicit' => true,
        'export' => true, 'extern' => true, 'false' => true, 'float' => true,
        'for' => true, 'friend' => true, 'goto' => true, 'if' => true,
        'inline' => true, 'int' => true, 'long' => true, 'mutable' => true,
        'namespace' => true, 'new' => true, 'noexcept' => true, 'not' => true,
        'not_eq' => true, 'nullptr' => true, 'operator' => true, 'or' => true,
        'or_eq' => true, 'private' => true, 'protected' => true, 'public' => true,
        'register' => true, 'reinterpret_cast' => true, 'requires' => true,
        'return' => true, 'short' => true, 'signed' => true, 'sizeof' => true,
        'static' => true, 'static_assert' => true, 'static_cast' => true,
        'struct' => true, 'switch' => true, 'template' => true, 'this' => true,
        'thread_local' => true, 'throw' => true, 'true' => true, 'try' => true,
        'typedef' => true, 'typeid' => true, 'typename' => true, 'union' => true,
        'unsigned' => true, 'using' => true, 'virtual' => true, 'void' => true,
        'volatile' => true, 'wchar_t' => true, 'while' => true, 'xor' => true,
        'xor_eq' => true,
    ];

    return preg_match('/^[A-Za-z_][A-Za-z0-9_]*$/', $name) === 1
        && !isset($keywords[$name]);
}

/**
 * @return void
 */
function require_cpp_identifier(string $name, string $label): void
{
    if (!is_cpp_identifier($name)) {
        fail('Invalid C++ identifier for ' . $label . ': ' . $name);
    }
}

/**
 * @return void
 */
function require_cpp_namespace(string $namespace): void
{
    if ($namespace === '') {
        return;
    }

    foreach (explode('::', $namespace) as $part) {
        require_cpp_identifier($part, 'namespace part');
    }
}

/**
 * @param array<int|string, mixed> $array
 * @return bool
 */
function is_list_array(array $array): bool
{
    return array_keys($array) === range(0, count($array) - 1);
}

/**
 * @param array<string, mixed> $array
 * @param list<string> $allowedKeys
 * @return void
 */
function require_only_keys(array $array, array $allowedKeys, string $label): void
{
    $allowed = array_fill_keys($allowedKeys, true);
    foreach (array_keys($array) as $key) {
        if (!is_string($key) || !isset($allowed[$key])) {
            fail('Unknown ' . $label . ' entry: ' . (string)$key);
        }
    }
}

/**
 * @param mixed $type
 * @return string
 */
function describe_type(mixed $type): string
{
    if (is_string($type)) {
        return $type;
    }

    if (is_array($type)) {
        return json_encode($type, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE) ?: 'array';
    }

    return get_debug_type($type);
}

/**
 * @param mixed $type
 * @return string
 */
function cpp_type(mixed $type): string
{
    if (is_string($type)) {
        return match ($type) {
            b => 'bool',
            u8 => 'std::uint8_t',
            u16 => 'std::uint16_t',
            u32 => 'std::uint32_t',
            u64 => 'std::uint64_t',
            i8 => 'std::int8_t',
            i16 => 'std::int16_t',
            i32 => 'std::int32_t',
            i64 => 'std::int64_t',
            f32 => 'float',
            f64 => 'double',
            s => 'std::u8string',
            bin => 'std::vector<std::byte>',
            default => fail('Unknown type token: ' . $type),
        };
    }

    if (!is_array($type) || !is_list_array($type)) {
        fail('Invalid type expression: ' . describe_type($type));
    }

    if (count($type) !== 2) {
        fail('Invalid type expression: ' . describe_type($type));
    }

    [$base, $modifier] = $type;

    if ($modifier === v) {
        return 'std::vector<' . cpp_type($base) . '>';
    }

    if ($modifier === m) {
        if (!is_array($base) || !is_list_array($base) || count($base) !== 2) {
            fail('std::map requires [[K, V], m]: ' . describe_type($type));
        }

        return 'std::map<' . cpp_type($base[0]) . ', ' . cpp_type($base[1]) . '>';
    }

    if (is_array($modifier) && is_list_array($modifier) && count($modifier) === 2 && $modifier[0] === a) {
        $size = $modifier[1];
        if (!is_int($size) || $size < 0) {
            fail('std::array size must be a non-negative integer: ' . describe_type($type));
        }

        return 'std::array<' . cpp_type($base) . ', ' . (string)$size . '>';
    }

    fail('Unknown type modifier: ' . describe_type($modifier));
}

/**
 * @param mixed $type
 * @param array<string, bool> $includes
 * @return void
 */
function collect_includes(mixed $type, array &$includes): void
{
    if (is_string($type)) {
        match ($type) {
            b, f32, f64 => null,
            u8, u16, u32, u64, i8, i16, i32, i64 => $includes['<cstdint>'] = true,
            s => $includes['<string>'] = true,
            bin => [$includes['<cstddef>'] = true, $includes['<vector>'] = true],
            default => null,
        };
        return;
    }

    if (!is_array($type) || !is_list_array($type) || count($type) !== 2) {
        return;
    }

    [$base, $modifier] = $type;
    if ($modifier === v) {
        $includes['<vector>'] = true;
        collect_includes($base, $includes);
        return;
    }

    if ($modifier === m) {
        $includes['<map>'] = true;
        if (is_array($base) && is_list_array($base) && count($base) === 2) {
            collect_includes($base[0], $includes);
            collect_includes($base[1], $includes);
        }
        return;
    }

    if (is_array($modifier) && is_list_array($modifier) && count($modifier) === 2 && $modifier[0] === a) {
        $includes['<array>'] = true;
        collect_includes($base, $includes);
    }
}

/**
 * @param array<string, mixed> $fields
 * @return void
 */
function validate_fields(array $fields, string $structName): void
{
    foreach ($fields as $fieldName => $type) {
        if (!is_string($fieldName)) {
            fail('Field names must be strings in struct ' . $structName);
        }
        require_cpp_identifier($fieldName, 'field name');
        cpp_type($type);
    }
}

/**
 * @param array<string, mixed> $schema
 * @return array{namespace:string, structs:array<string, array<string, mixed>>}
 */
function normalize_schema(array $schema, ?string $structOption, ?string $namespaceOption): array
{
    $namespace = $namespaceOption ?? '';
    if ($namespace === '' && isset($schema['namespace'])) {
        if (!is_string($schema['namespace'])) {
            fail('Schema entry "namespace" must be a string.');
        }
        $namespace = $schema['namespace'];
    }
    require_cpp_namespace($namespace);

    $structs = [];

    if (isset($schema['structs'])) {
        require_only_keys($schema, ['namespace', 'structs'], 'schema');

        if (!is_array($schema['structs'])) {
            fail('Schema entry "structs" must be an array.');
        }

        foreach ($schema['structs'] as $structName => $definition) {
            if (!is_string($structName)) {
                fail('Struct names must be strings.');
            }
            if ($structOption !== null && $structName !== $structOption) {
                continue;
            }
            require_cpp_identifier($structName, 'struct name');

            if (is_array($definition) && isset($definition['fields'])) {
                require_only_keys($definition, ['fields'], 'struct ' . $structName);
                if (!is_array($definition['fields'])) {
                    fail('Struct entry "fields" must be an array: ' . $structName);
                }
                $fields = $definition['fields'];
            } elseif (is_array($definition)) {
                $fields = $definition;
            } else {
                fail('Struct definition must be an array: ' . $structName);
            }

            validate_fields($fields, $structName);
            $structs[$structName] = $fields;
        }

        if ($structOption !== null && $structs === []) {
            fail('Struct not found in schema: ' . $structOption);
        }

        return ['namespace' => $namespace, 'structs' => $structs];
    }

    if (isset($schema['fields'])) {
        require_only_keys($schema, ['namespace', 'struct', 'fields'], 'schema');

        if (!is_array($schema['fields'])) {
            fail('Schema entry "fields" must be an array.');
        }

        $structName = $structOption;
        if ($structName === null && isset($schema['struct'])) {
            if (!is_string($schema['struct'])) {
                fail('Schema entry "struct" must be a string.');
            }
            $structName = $schema['struct'];
        }
        if ($structName === null) {
            fail('Struct name is required. Use --struct=<name> or schema entry "struct".');
        }

        require_cpp_identifier($structName, 'struct name');
        validate_fields($schema['fields'], $structName);
        $structs[$structName] = $schema['fields'];
        return ['namespace' => $namespace, 'structs' => $structs];
    }

    if ($structOption === null) {
        fail('Struct name is required for a plain field map. Use --struct=<name>.');
    }

    require_cpp_identifier($structOption, 'struct name');
    validate_fields($schema, $structOption);
    $structs[$structOption] = $schema;
    return ['namespace' => $namespace, 'structs' => $structs];
}

/**
 * @param array<string, mixed> $fields
 * @return string
 */
function generate_struct(string $structName, array $fields, string $generatedAt): string
{
    $out = '';
    $out .= 'struct ' . $structName . " {\n";
    foreach ($fields as $fieldName => $type) {
        $out .= '    ' . cpp_type($type) . ' ' . $fieldName . "{};\n";
    }
    $out .= "};\n\n";

    $out .= 'inline constexpr char ' . $structName . "_xfer_schema_generated_at[] = \""
        . addcslashes($generatedAt, "\\\"") . "\";\n\n";

    $out .= "template<class Archive>\n";
    $out .= 'auto xfer(Archive& ar, ' . $structName . "& value) -> xer::result<void>\n";
    $out .= "{\n";
    foreach ($fields as $fieldName => $_type) {
        $out .= '    if (auto r = ar(value.' . $fieldName . "); !r) {\n";
        $out .= "        return r;\n";
        $out .= "    }\n";
    }
    $out .= "\n";
    $out .= "    return {};\n";
    $out .= "}\n";

    return $out;
}

/**
 * @param array<string, array<string, mixed>> $structs
 * @return list<string>
 */
function required_includes(array $structs): array
{
    $includes = [];
    foreach ($structs as $fields) {
        foreach ($fields as $type) {
            collect_includes($type, $includes);
        }
    }

    $order = ['<array>', '<cstddef>', '<cstdint>', '<map>', '<string>', '<vector>'];
    $result = [];
    foreach ($order as $header) {
        if (isset($includes[$header])) {
            $result[] = $header;
        }
    }

    return $result;
}

/**
 * @param array{namespace:string, structs:array<string, array<string, mixed>>} $normalized
 * @return string
 */
function generate_header(array $normalized, string $schemaPath, string $generatedAt): string
{
    $out = '';
    $out .= "/**\n";
    $out .= " * @file\n";
    $out .= " * @brief Generated XER binary transfer structs.\n";
    $out .= " *\n";
    $out .= " * Generated by php/generate_xfer_struct.php.\n";
    $out .= " * Source schema: " . normalize_path($schemaPath) . "\n";
    $out .= " * Schema version: " . $generatedAt . "\n";
    $out .= " */\n\n";
    $out .= "#pragma once\n\n";

    foreach (required_includes($normalized['structs']) as $header) {
        $out .= '#include ' . $header . "\n";
    }
    if (required_includes($normalized['structs']) !== []) {
        $out .= "\n";
    }
    $out .= "#include <xer/serialize.h>\n\n";

    $namespace = $normalized['namespace'];
    if ($namespace !== '') {
        $out .= 'namespace ' . $namespace . " {\n\n";
    }

    $chunks = [];
    foreach ($normalized['structs'] as $structName => $fields) {
        $chunks[] = generate_struct($structName, $fields, $generatedAt);
    }
    $out .= implode("\n\n", $chunks);

    if ($namespace !== '') {
        $out .= "\n} // namespace " . $namespace . "\n";
    }

    return $out;
}

/**
 * @param list<string> $argv
 * @return array{schema:string, output:string, struct:?string, namespace:?string, generated_at:?string}
 */
function parse_arguments(array $argv): array
{
    $schema = null;
    $output = null;
    $struct = null;
    $namespace = null;
    $generatedAt = null;

    for ($i = 1; $i < count($argv); ++$i) {
        $arg = $argv[$i];

        if ($arg === '--help' || $arg === '-h') {
            print_usage();
            exit(EXIT_SUCCESS_CODE);
        }

        if (str_starts_with($arg, '--struct=')) {
            $struct = substr($arg, strlen('--struct='));
            continue;
        }

        if (str_starts_with($arg, '--namespace=')) {
            $namespace = substr($arg, strlen('--namespace='));
            continue;
        }

        if (str_starts_with($arg, '--generated-at=')) {
            $generatedAt = substr($arg, strlen('--generated-at='));
            continue;
        }

        if (str_starts_with($arg, '--')) {
            fail('Unknown option: ' . $arg);
        }

        if ($schema === null) {
            $schema = $arg;
            continue;
        }

        if ($output === null) {
            $output = $arg;
            continue;
        }

        fail('Unexpected argument: ' . $arg);
    }

    if ($schema === null || $output === null) {
        print_usage();
        exit(EXIT_FAILURE_CODE);
    }

    return [
        'schema' => $schema,
        'output' => $output,
        'struct' => $struct,
        'namespace' => $namespace,
        'generated_at' => $generatedAt,
    ];
}

$args = parse_arguments($argv);

$schemaPath = $args['schema'];
if (!is_file($schemaPath)) {
    fail('Schema file not found: ' . $schemaPath);
}

$schema = require $schemaPath;
if (!is_array($schema)) {
    fail('Schema file must return an array: ' . $schemaPath);
}

$normalized = normalize_schema($schema, $args['struct'], $args['namespace']);
if ($normalized['structs'] === []) {
    fail('No structs to generate.');
}

$generatedAt = $args['generated_at'] ?? (new DateTimeImmutable('now'))->format(DateTimeInterface::ATOM);
if ($generatedAt === '') {
    fail('Generated-at string must not be empty.');
}

$header = generate_header($normalized, $schemaPath, $generatedAt);

$outputPath = $args['output'];
$outputDirectory = dirname($outputPath);
if ($outputDirectory !== '' && $outputDirectory !== '.' && !is_dir($outputDirectory)) {
    if (!mkdir($outputDirectory, 0777, true) && !is_dir($outputDirectory)) {
        fail('Failed to create output directory: ' . $outputDirectory);
    }
}

if (file_put_contents($outputPath, XER_GENERATED_BOM . $header) === false) {
    fail('Failed to write output file: ' . $outputPath);
}

fwrite(STDOUT, 'Generated: ' . normalize_path($outputPath) . PHP_EOL);
