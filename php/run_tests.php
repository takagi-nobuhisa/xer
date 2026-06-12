<?php

declare(strict_types=1);

/**
 * @file run_tests.php
 * @brief Compile and run C++ programs under specified target directories with optional parallel execution.
 *
 * This script assumes that it is executed in the php directory.
 *
 * Supported target directories:
 *
 * - tests
 * - examples
 *
 * Examples:
 *
 *   php run_tests.php
 *   php run_tests.php tests
 *   php run_tests.php test
 *   php run_tests.php examples
 *   php run_tests.php example
 *   php run_tests.php tests examples
 *   php run_tests.php --jobs=8
 *   php run_tests.php --jobs=8 tests examples
 *   php run_tests.php --all
 *   php run_tests.php --clean-cache
 *   php run_tests.php --tc=gcc
 *   php run_tests.php --tc=clang
 *   php run_tests.php --tc=all
 *   php run_tests.php --compiler=clang++
 *   php run_tests.php --build-id=msys2-ucrt64
 *   php run_tests.php tests/test_string.cpp
 *   php run_tests.php test_string.cpp
 *   php run_tests.php --tcltk-cflags="-I/usr/include/tcl"
 *   php run_tests.php --tcltk-libs="-ltk -ltcl"
 *
 * Source-level feature metadata may be written in a C++ source file.
 *
 *   // XER_TEST_FEATURES: socket tcltk
 *
 * The runner also detects some features from public header includes.
 */

const EXIT_SUCCESS_CODE = 0;
const EXIT_FAILURE_CODE = 1;

/**
 * @return never
 */
function fail(string $message, int $exitCode = EXIT_FAILURE_CODE): never
{
    fwrite(STDERR, $message . PHP_EOL);
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
function shell_quote_path(string $path): string
{
    return escapeshellarg($path);
}

/**
 * @return void
 */
function ensure_directory(string $directory): void
{
    if (is_dir($directory)) {
        return;
    }

    if (!mkdir($directory, 0777, true) && !is_dir($directory)) {
        fail('Failed to create directory: ' . $directory);
    }
}

/**
 * @return void
 */
function remove_tree(string $path): void
{
    if (!file_exists($path)) {
        return;
    }

    if (is_file($path) || is_link($path)) {
        if (!unlink($path)) {
            fail('Failed to remove file: ' . $path);
        }
        return;
    }

    $entries = scandir($path);
    if ($entries === false) {
        fail('Failed to scan directory: ' . $path);
    }

    foreach ($entries as $entry) {
        if ($entry === '.' || $entry === '..') {
            continue;
        }

        remove_tree(normalize_path($path . '/' . $entry));
    }

    if (!rmdir($path)) {
        fail('Failed to remove directory: ' . $path);
    }
}

/**
 * @return void
 */
function recreate_directory(string $directory): void
{
    remove_tree($directory);
    ensure_directory($directory);
}

/**
 * @return void
 */
function write_text_file(string $path, string $content): void
{
    $result = file_put_contents($path, $content);
    if ($result === false) {
        fail('Failed to write file: ' . $path);
    }
}

/**
 * @return string
 */
function format_result_label(bool $ok): string
{
    return $ok ? 'OK' : 'NG';
}

/**
 * @param list<string> $targets
 * @return list<string>
 */
function normalize_targets(array $targets): array
{
    if ($targets === []) {
        return ['tests'];
    }

    $normalized = [];

    foreach ($targets as $target) {
        $value = trim($target);
        if ($value === '') {
            continue;
        }

        $value = normalize_path($value);
        $value = trim($value, '/');

        if ($value === '') {
            continue;
        }

        $value = match ($value) {
            'test' => 'tests',
            'example' => 'examples',
            default => $value,
        };

        $normalized[] = $value;
    }

    if ($normalized === []) {
        return ['tests'];
    }

    $normalized = array_values(array_unique($normalized, SORT_STRING));
    sort($normalized, SORT_STRING);

    return $normalized;
}

/**
 * @return bool
 */
function is_supported_target(string $target): bool
{
    return in_array($target, ['tests', 'examples'], true);
}

/**
 * @return list<string>
 */
function find_cpp_sources(string $sourceDir): array
{
    $pattern = $sourceDir . '/*.cpp';
    $files = glob($pattern);
    if ($files === false) {
        return [];
    }

    $files = array_map('normalize_path', $files);
    sort($files, SORT_STRING);
    return array_values($files);
}

/**
 * @return bool
 */
function is_windows(): bool
{
    return DIRECTORY_SEPARATOR === '\\' || PHP_OS_FAMILY === 'Windows';
}

/**
 * @return bool
 */
function is_windows_target(): bool
{
    return is_windows() || getenv('MSYSTEM') !== false;
}

/**
 * @return string|null
 */
function detect_msys2_prefix(): ?string
{
    $prefix = getenv('MINGW_PREFIX');
    if (is_string($prefix) && trim($prefix) !== '') {
        return str_replace('\\', '/', rtrim(trim($prefix), '/\\'));
    }

    $msystem = getenv('MSYSTEM');
    if (!is_string($msystem)) {
        return null;
    }

    return match (strtoupper(trim($msystem))) {
        'UCRT64' => '/ucrt64',
        'CLANG64' => '/clang64',
        'CLANGARM64' => '/clangarm64',
        'MINGW64' => '/mingw64',
        'MINGW32' => '/mingw32',
        default => null,
    };
}

/**
 * @return int
 */
function detect_default_jobs(): int
{
    $count = detect_cpu_count();
    return max(1, $count);
}

/**
 * @return int
 */
function detect_cpu_count(): int
{
    if (is_windows()) {
        $value = getenv('NUMBER_OF_PROCESSORS');
        if (is_string($value) && preg_match('/^[1-9][0-9]*$/', $value)) {
            return (int) $value;
        }

        return 1;
    }

    $commands = [
        'getconf _NPROCESSORS_ONLN 2>/dev/null',
        'nproc 2>/dev/null',
        'sysctl -n hw.ncpu 2>/dev/null',
    ];

    foreach ($commands as $command) {
        $output = [];
        $exitCode = 0;
        exec($command, $output, $exitCode);

        if ($exitCode !== 0 || $output === []) {
            continue;
        }

        $value = trim($output[0]);
        if (preg_match('/^[1-9][0-9]*$/', $value)) {
            return (int) $value;
        }
    }

    return 1;
}

/**
 * @return int
 */

function detect_default_build_id(): string
{
    $env = getenv('XER_TEST_BUILD_ID');
    if (is_string($env) && trim($env) !== '') {
        return sanitize_build_id($env);
    }

    $parts = [];

    $msystem = getenv('MSYSTEM');
    if (is_string($msystem) && trim($msystem) !== '') {
        $parts[] = 'msys2';
        $parts[] = strtolower(trim($msystem));
    } else {
        $parts[] = strtolower(PHP_OS_FAMILY);

        if (PHP_OS_FAMILY === 'Linux') {
            $linuxId = detect_linux_id();
            if ($linuxId !== '') {
                $parts[] = $linuxId;
            }
        }
    }

    $arch = getenv('MSYSTEM_CARCH');
    if (!is_string($arch) || trim($arch) === '') {
        $arch = php_uname('m');
    }
    if (is_string($arch) && trim($arch) !== '') {
        $parts[] = strtolower(trim($arch));
    }

    return sanitize_build_id(implode('-', $parts));
}

function detect_linux_id(): string
{
    $path = '/etc/os-release';
    if (!is_file($path)) {
        return '';
    }

    $lines = file($path, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    if ($lines === false) {
        return '';
    }

    foreach ($lines as $line) {
        if (!str_starts_with($line, 'ID=')) {
            continue;
        }

        $value = substr($line, 3);
        $value = trim($value, " \t\r\n\"'");
        return strtolower($value);
    }

    return '';
}

function sanitize_build_id(string $value): string
{
    $value = strtolower(trim($value));
    $value = preg_replace('/[^a-z0-9._-]+/', '-', $value);
    $value = trim($value ?? '', '.-_');

    if ($value === '') {
        fail('Build ID must not be empty.');
    }

    return $value;
}

function looks_like_cpp_source_selector(string $value): bool
{
    return str_ends_with(strtolower(normalize_path($value)), '.cpp');
}

/**
 * @param list<string> $selectors
 * @return list<string>
 */
function normalize_source_selectors(array $selectors): array
{
    $normalized = [];

    foreach ($selectors as $selector) {
        $value = trim(normalize_path($selector));
        if ($value === '') {
            continue;
        }

        $value = preg_replace('#^\./#', '', $value);
        if ($value === null || $value === '') {
            continue;
        }

        $normalized[] = $value;
    }

    $normalized = array_values(array_unique($normalized, SORT_STRING));
    sort($normalized, SORT_STRING);
    return $normalized;
}

/**
 * @param list<string> $sourceFiles
 * @param list<string> $selectors
 * @return list<string>
 */
function filter_source_files_by_selectors(array $sourceFiles, string $target, string $projectRoot, array $selectors): array
{
    if ($selectors === []) {
        return $sourceFiles;
    }

    $result = [];

    foreach ($sourceFiles as $sourceFile) {
        $absolute = normalize_path($sourceFile);
        $relative = normalize_path(substr($absolute, strlen(normalize_path($projectRoot)) + 1));
        $targetRelative = normalize_path($target . '/' . basename($sourceFile));
        $baseName = basename($sourceFile);

        foreach ($selectors as $selector) {
            $normalizedSelector = normalize_path($selector);
            $trimmedSelector = ltrim(preg_replace('#^(\.\./)+#', '', $normalizedSelector) ?? $normalizedSelector, '/');

            if (
                $normalizedSelector === $absolute ||
                $trimmedSelector === $relative ||
                $trimmedSelector === $targetRelative ||
                $trimmedSelector === $baseName
            ) {
                $result[] = $sourceFile;
                break;
            }
        }
    }

    return array_values(array_unique($result, SORT_STRING));
}

function parse_jobs_option(string $value): int
{
    if (!preg_match('/^[1-9][0-9]*$/', $value)) {
        fail('Invalid --jobs value: ' . $value);
    }

    return (int) $value;
}

/**
 * @return list<string>
 */
function split_command_fragment(string $value): array
{
    $value = trim($value);
    if ($value === '') {
        return [];
    }

    $tokens = str_getcsv($value, ' ', '"', '\\');
    if ($tokens === false) {
        return [];
    }

    $result = [];
    foreach ($tokens as $token) {
        $token = trim($token);
        if ($token !== '') {
            $result[] = $token;
        }
    }

    return $result;
}

/**
 * @param list<string> $argv
 * @return array{
 *   cxx:string,
 *   all:bool,
 *   clean_cache:bool,
 *   cxxflags:list<string>,
 *   ldflags:list<string>,
 *   jobs:int,
 *   build_id:string,
 *   toolchain:string,
 *   compiler_explicit:bool,
 *   cxxflags_explicit:bool,
 *   ldflags_explicit:bool,
 *   tcltk_cflags:list<string>,
 *   tcltk_libs:list<string>,
 *   icu_cflags:list<string>,
 *   icu_libs:list<string>,
 *   targets:list<string>,
 *   source_selectors:list<string>
 * }
 */
function parse_arguments(array $argv): array
{
    $cxx = getenv('XER_TEST_CXX');
    if (!is_string($cxx) || trim($cxx) === '') {
        $cxx = 'g++';
    }

    $cxxflags = split_command_fragment(getenv('XER_TEST_CXXFLAGS') ?: '');
    $ldflags = split_command_fragment(getenv('XER_TEST_LDFLAGS') ?: '');
    $tcltkCflags = split_command_fragment(getenv('XER_TEST_TCLTK_CFLAGS') ?: '');
    $tcltkLibs = split_command_fragment(getenv('XER_TEST_TCLTK_LIBS') ?: '');
    $icuCflags = split_command_fragment(getenv('XER_TEST_ICU_CFLAGS') ?: '');
    $icuLibs = split_command_fragment(getenv('XER_TEST_ICU_LIBS') ?: '');
    $jobs = detect_default_jobs();
    $buildId = detect_default_build_id();
    $toolchain = getenv('XER_TEST_TOOLCHAIN');
    if (!is_string($toolchain) || trim($toolchain) === '') {
        $toolchain = 'gcc';
    }
    $compilerExplicit = false;
    $cxxflagsExplicit = false;
    $ldflagsExplicit = false;
    $targets = [];
    $sourceSelectors = [];
    $all = false;
    $cleanCache = false;

    foreach (array_slice($argv, 1) as $arg) {
        if ($arg === '--all') {
            $all = true;
            continue;
        }

        if ($arg === '--clean-cache') {
            $cleanCache = true;
            continue;
        }

        if (str_starts_with($arg, '--tc=')) {
            $toolchain = substr($arg, strlen('--tc='));
            if (trim($toolchain) === '') {
                fail('Invalid --tc value.');
            }
            continue;
        }

        if (str_starts_with($arg, '--toolchain=')) {
            $toolchain = substr($arg, strlen('--toolchain='));
            if (trim($toolchain) === '') {
                fail('Invalid --toolchain value.');
            }
            continue;
        }

        if (str_starts_with($arg, '--compiler=')) {
            $cxx = substr($arg, strlen('--compiler='));
            if (trim($cxx) === '') {
                fail('Invalid --compiler value.');
            }
            $compilerExplicit = true;
            continue;
        }

        if (str_starts_with($arg, '--cxx=')) {
            $cxx = substr($arg, strlen('--cxx='));
            if (trim($cxx) === '') {
                fail('Invalid --cxx value.');
            }
            $compilerExplicit = true;
            continue;
        }

        if (str_starts_with($arg, '--cxxflags=')) {
            $cxxflags = split_command_fragment(substr($arg, strlen('--cxxflags=')));
            $cxxflagsExplicit = true;
            continue;
        }

        if (str_starts_with($arg, '--ldflags=')) {
            $ldflags = split_command_fragment(substr($arg, strlen('--ldflags=')));
            $ldflagsExplicit = true;
            continue;
        }

        if (str_starts_with($arg, '--jobs=')) {
            $jobs = parse_jobs_option(substr($arg, strlen('--jobs=')));
            continue;
        }

        if (str_starts_with($arg, '--build-id=')) {
            $buildId = sanitize_build_id(substr($arg, strlen('--build-id=')));
            continue;
        }

        if (str_starts_with($arg, '--tcltk-cflags=')) {
            $tcltkCflags = split_command_fragment(substr($arg, strlen('--tcltk-cflags=')));
            continue;
        }

        if (str_starts_with($arg, '--tcltk-libs=')) {
            $tcltkLibs = split_command_fragment(substr($arg, strlen('--tcltk-libs=')));
            continue;
        }

        if (str_starts_with($arg, '--icu-cflags=')) {
            $icuCflags = split_command_fragment(substr($arg, strlen('--icu-cflags=')));
            continue;
        }

        if (str_starts_with($arg, '--icu-libs=')) {
            $icuLibs = split_command_fragment(substr($arg, strlen('--icu-libs=')));
            continue;
        }

        if (looks_like_cpp_source_selector($arg)) {
            $sourceSelectors[] = $arg;
            continue;
        }

        $targets[] = $arg;
    }

    return [
        'cxx' => $cxx,
        'all' => $all,
        'clean_cache' => $cleanCache,
        'cxxflags' => $cxxflags,
        'ldflags' => $ldflags,
        'jobs' => $jobs,
        'build_id' => $buildId,
        'toolchain' => $toolchain,
        'compiler_explicit' => $compilerExplicit,
        'cxxflags_explicit' => $cxxflagsExplicit,
        'ldflags_explicit' => $ldflagsExplicit,
        'tcltk_cflags' => $tcltkCflags,
        'tcltk_libs' => $tcltkLibs,
        'icu_cflags' => $icuCflags,
        'icu_libs' => $icuLibs,
        'targets' => normalize_targets($targets),
        'source_selectors' => normalize_source_selectors($sourceSelectors),
    ];
}


/**
 * @return array<string, array<string, mixed>>
 */
function load_test_toolchains(string $scriptDir): array
{
    $path = normalize_path($scriptDir . '/test_toolchains.php');
    if (!is_file($path)) {
        return [
            'gcc' => [
                'compiler' => 'g++',
                'cxxflags' => [],
                'ldflags' => [],
            ],
        ];
    }

    $toolchains = require $path;
    if (!is_array($toolchains)) {
        fail('Invalid toolchain configuration: ' . $path);
    }

    foreach ($toolchains as $name => $config) {
        if (!is_string($name) || trim($name) === '') {
            fail('Invalid toolchain name in: ' . $path);
        }
        if (!is_array($config)) {
            fail('Invalid toolchain entry: ' . $name);
        }
        if (!isset($config['compiler']) || !is_string($config['compiler']) || trim($config['compiler']) === '') {
            fail('Toolchain compiler is missing: ' . $name);
        }
    }

    return $toolchains;
}

/**
 * @param array<string, mixed> $parsed
 * @param array<string, mixed> $config
 * @return array<string, mixed>
 */
function apply_toolchain_config(array $parsed, array $config): array
{
    if (($parsed['compiler_explicit'] ?? false) !== true) {
        $parsed['cxx'] = (string) $config['compiler'];
    }

    if (($parsed['cxxflags_explicit'] ?? false) !== true) {
        $parsed['cxxflags'] = array_values($config['cxxflags'] ?? []);
    }

    if (($parsed['ldflags_explicit'] ?? false) !== true) {
        $parsed['ldflags'] = array_values($config['ldflags'] ?? []);
    }

    return $parsed;
}

/**
 * @param list<string> $argv
 * @param list<string> $toolchainNames
 * @return never
 */
function run_all_toolchains(array $argv, array $toolchainNames): never
{
    $php = PHP_BINARY;
    $script = $argv[0] ?? 'run_tests.php';
    $baseArgs = [];

    foreach (array_slice($argv, 1) as $arg) {
        if ($arg === '--tc=all' || $arg === '--toolchain=all') {
            continue;
        }
        if (str_starts_with($arg, '--tc=') || str_starts_with($arg, '--toolchain=')) {
            continue;
        }
        $baseArgs[] = $arg;
    }

    $overallExitCode = EXIT_SUCCESS_CODE;

    foreach ($toolchainNames as $toolchainName) {
        echo PHP_EOL;
        echo '############################################################' . PHP_EOL;
        echo 'Toolchain: ' . $toolchainName . PHP_EOL;
        echo '############################################################' . PHP_EOL;

        $command = array_merge([$php, $script, '--tc=' . $toolchainName], $baseArgs);
        passthru(format_command_line($command), $exitCode);

        if ($exitCode !== EXIT_SUCCESS_CODE) {
            $overallExitCode = EXIT_FAILURE_CODE;
        }
    }

    exit($overallExitCode);
}

/**
 * @return string|null
 */
function find_executable(string $name): ?string
{
    $command = is_windows()
        ? 'where ' . escapeshellarg($name)
        : 'command -v ' . escapeshellarg($name);

    $output = [];
    $exitCode = 0;
    exec($command, $output, $exitCode);

    if ($exitCode !== 0 || $output === []) {
        return null;
    }

    $path = trim($output[0]);
    return $path === '' ? null : $path;
}

/**
 * @param list<string> $command
 * @return string
 */
function format_command_line(array $command): string
{
    $parts = array_map('shell_quote_path', $command);
    return implode(' ', $parts);
}

/**
 * @param list<string> $command
 * @return array{output:string, exit_code:int, command_line:string}
 */
function run_command(array $command, string $cwd): array
{
    $commandLine = format_command_line($command);
    $descriptorSpec = [
        0 => ['pipe', 'r'],
        1 => ['pipe', 'w'],
        2 => ['pipe', 'w'],
    ];

    $pipes = [];
    $process = proc_open($commandLine, $descriptorSpec, $pipes, $cwd);
    if (!is_resource($process)) {
        return [
            'output' => 'Failed to start process: ' . $commandLine,
            'exit_code' => 1,
            'command_line' => $commandLine,
        ];
    }

    fclose($pipes[0]);
    $stdout = stream_get_contents($pipes[1]);
    $stderr = stream_get_contents($pipes[2]);
    fclose($pipes[1]);
    fclose($pipes[2]);
    $exitCode = proc_close($process);

    $parts = [];
    if ($stdout !== false && $stdout !== '') {
        $parts[] = rtrim($stdout, "\r\n");
    }
    if ($stderr !== false && $stderr !== '') {
        $parts[] = rtrim($stderr, "\r\n");
    }

    return [
        'output' => implode(PHP_EOL, array_filter($parts, static fn(string $value): bool => $value !== '')),
        'exit_code' => $exitCode,
        'command_line' => $commandLine,
    ];
}

/**
 * @param resource $process
 * @param array<int, resource> $pipes
 * @return array{output:string, exit_code:int}
 */
function finalize_process($process, array $pipes): array
{
    $stdout = stream_get_contents($pipes[1]);
    $stderr = stream_get_contents($pipes[2]);

    fclose($pipes[1]);
    fclose($pipes[2]);

    $exitCode = proc_close($process);

    $parts = [];

    if ($stdout !== false && $stdout !== '') {
        $parts[] = rtrim($stdout, "\r\n");
    }

    if ($stderr !== false && $stderr !== '') {
        $parts[] = rtrim($stderr, "\r\n");
    }

    $output = implode(PHP_EOL, array_filter($parts, static fn(string $value): bool => $value !== ''));

    return [
        'output' => $output,
        'exit_code' => $exitCode,
    ];
}

/**
 * @param list<string> $command
 * @return array{process:resource, pipes:array<int, resource>, command_line:string}
 */
function start_process(array $command, string $cwd): array
{
    $commandLine = format_command_line($command);

    $descriptorSpec = [
        0 => ['pipe', 'r'],
        1 => ['pipe', 'w'],
        2 => ['pipe', 'w'],
    ];

    $pipes = [];
    $process = proc_open($commandLine, $descriptorSpec, $pipes, $cwd);

    if (!is_resource($process)) {
        fail('Failed to start process: ' . $commandLine);
    }

    fclose($pipes[0]);
    stream_set_blocking($pipes[1], false);
    stream_set_blocking($pipes[2], false);

    return [
        'process' => $process,
        'pipes' => $pipes,
        'command_line' => $commandLine,
    ];
}

/**
 * @return string
 */
function read_source_file(string $sourceFile): string
{
    $content = file_get_contents($sourceFile);
    if ($content === false) {
        fail('Failed to read source file: ' . $sourceFile);
    }

    return $content;
}

/**
 * @return list<string>
 */
function parse_feature_metadata(string $content): array
{
    $features = [];

    if (preg_match_all('/^\s*\/\/\s*XER_TEST_FEATURES\s*:\s*(.+)$/mi', $content, $matches)) {
        foreach ($matches[1] as $line) {
            foreach (preg_split('/[\s,]+/', trim($line)) ?: [] as $feature) {
                $feature = strtolower(trim($feature));
                if ($feature !== '') {
                    $features[] = $feature;
                }
            }
        }
    }

    return $features;
}

/**
 * @return list<string>
 */
function detect_source_features(string $sourceFile): array
{
    $content = read_source_file($sourceFile);
    $features = parse_feature_metadata($content);

    if (preg_match('/#\s*include\s*[<"]xer\/socket\.h[>"]/', $content)) {
        $features[] = 'socket';
    }

    if (preg_match('/#\s*include\s*[<"]xer\/(tk|gui)\.h[>"]/', $content)) {
        $features[] = 'tcltk';
    }

    if (preg_match('/#\s*include\s*[<"]xer\/unicode\.h[>"]/', $content)) {
        $features[] = 'icu';
    }

    if (preg_match('/#\s*include\s*[<"]xer\/zip\.h[>"]/', $content)) {
        $features[] = 'zip';
    }

    $features = array_values(array_unique($features, SORT_STRING));
    sort($features, SORT_STRING);

    return $features;
}

/**
 * @param list<string> $features
 * @return string
 */
function format_features(array $features): string
{
    return $features === [] ? '-' : implode(', ', $features);
}

/**
 * @param array<string, mixed> $options
 * @return list<array{cflags:list<string>, libs:list<string>, source:string}>
 */
function tcltk_option_candidates(array $options): array
{
    $candidates = [];

    if ($options['tcltk_cflags'] !== [] || $options['tcltk_libs'] !== []) {
        $candidates[] = [
            'cflags' => $options['tcltk_cflags'],
            'libs' => $options['tcltk_libs'],
            'source' => 'command line or environment',
        ];
    }

    if (find_executable('pkg-config') !== null) {
        foreach (['tk', 'tk8.6', 'tk86'] as $package) {
            $output = [];
            $exitCode = 0;
            exec('pkg-config --cflags --libs ' . escapeshellarg($package) . ' 2>/dev/null', $output, $exitCode);
            if ($exitCode === 0 && $output !== []) {
                $tokens = split_command_fragment(implode(' ', $output));
                $cflags = [];
                $libs = [];
                foreach ($tokens as $token) {
                    if (str_starts_with($token, '-l') || str_starts_with($token, '-L') || str_starts_with($token, '-Wl,')) {
                        $libs[] = $token;
                    } else {
                        $cflags[] = $token;
                    }
                }
                $candidates[] = [
                    'cflags' => $cflags,
                    'libs' => $libs,
                    'source' => 'pkg-config ' . $package,
                ];
            }
        }
    }

    if (is_windows_target()) {
        $candidates[] = [
            'cflags' => [],
            'libs' => ['-ltk', '-ltcl'],
            'source' => 'default Windows/MSYS2 Tcl/Tk libraries',
        ];
    } else {
        foreach (['/usr/include/tcl', '/usr/include/tcl8.6', '/usr/include/tk', '/usr/include/tk8.6'] as $includeDir) {
            if (is_dir($includeDir)) {
                $candidates[] = [
                    'cflags' => ['-I' . $includeDir],
                    'libs' => ['-ltk', '-ltcl'],
                    'source' => 'default Unix Tcl/Tk libraries with ' . $includeDir,
                ];
            }
        }

        $candidates[] = [
            'cflags' => [],
            'libs' => ['-ltk', '-ltcl'],
            'source' => 'default Unix Tcl/Tk libraries',
        ];
    }

    $unique = [];
    $result = [];
    foreach ($candidates as $candidate) {
        $key = implode("\0", $candidate['cflags']) . "\1" . implode("\0", $candidate['libs']);
        if (isset($unique[$key])) {
            continue;
        }
        $unique[$key] = true;
        $result[] = $candidate;
    }

    return $result;
}

/**
 * @param array<string, mixed> $options
 * @return list<array{cflags:list<string>, libs:list<string>, source:string}>
 */
function icu_option_candidates(array $options): array
{
    $candidates = [];

    if ($options['icu_cflags'] !== [] || $options['icu_libs'] !== []) {
        $candidates[] = [
            'cflags' => $options['icu_cflags'],
            'libs' => $options['icu_libs'],
            'source' => 'command line or environment',
        ];
    }

    if (find_executable('pkg-config') !== null) {
        foreach (['icu-uc'] as $package) {
            $output = [];
            $exitCode = 0;
            exec('pkg-config --cflags --libs ' . escapeshellarg($package) . ' 2>/dev/null', $output, $exitCode);
            if ($exitCode === 0 && $output !== []) {
                $tokens = split_command_fragment(implode(' ', $output));
                $cflags = [];
                $libs = [];
                foreach ($tokens as $token) {
                    if (str_starts_with($token, '-l') || str_starts_with($token, '-L') || str_starts_with($token, '-Wl,')) {
                        $libs[] = $token;
                    } else {
                        $cflags[] = $token;
                    }
                }
                $candidates[] = [
                    'cflags' => $cflags,
                    'libs' => $libs,
                    'source' => 'pkg-config ' . $package,
                ];
            }
        }
    }

    $msys2Prefix = detect_msys2_prefix();
    if ($msys2Prefix !== null) {
        $candidates[] = [
            'cflags' => ['-I' . $msys2Prefix . '/include'],
            'libs' => ['-L' . $msys2Prefix . '/lib', '-licuuc', '-licudt'],
            'source' => 'MSYS2 ICU libraries under ' . $msys2Prefix,
        ];
    }

    if (is_windows_target()) {
        $candidates[] = [
            'cflags' => [],
            'libs' => ['-licuuc', '-licudt'],
            'source' => 'default Windows/MSYS2 ICU libraries',
        ];
        $candidates[] = [
            'cflags' => [],
            'libs' => ['-licuuc', '-licudata'],
            'source' => 'default Windows/MSYS2 ICU libraries with icudata fallback',
        ];
    } else {
        $candidates[] = [
            'cflags' => [],
            'libs' => ['-licuuc', '-licudata'],
            'source' => 'default Unix ICU libraries',
        ];
    }

    $unique = [];
    $result = [];
    foreach ($candidates as $candidate) {
        $key = implode("\0", $candidate['cflags']) . "\1" . implode("\0", $candidate['libs']);
        if (isset($unique[$key])) {
            continue;
        }
        $unique[$key] = true;
        $result[] = $candidate;
    }

    return $result;
}


/**
 * @param array<string, mixed> $options
 * @return list<array{cflags:list<string>, libs:list<string>, source:string}>
 */
function zlib_option_candidates(array $options): array
{
    $candidates = [];

    if (find_executable('pkg-config') !== null) {
        $output = [];
        $exitCode = 0;
        exec('pkg-config --cflags --libs zlib 2>/dev/null', $output, $exitCode);
        if ($exitCode === 0 && $output !== []) {
            $tokens = split_command_fragment(implode(' ', $output));
            $cflags = [];
            $libs = [];
            foreach ($tokens as $token) {
                if (str_starts_with($token, '-l') || str_starts_with($token, '-L') || str_starts_with($token, '-Wl,')) {
                    $libs[] = $token;
                } else {
                    $cflags[] = $token;
                }
            }
            $candidates[] = [
                'cflags' => $cflags,
                'libs' => $libs,
                'source' => 'pkg-config zlib',
            ];
        }
    }

    $msys2Prefix = detect_msys2_prefix();
    if ($msys2Prefix !== null) {
        $candidates[] = [
            'cflags' => ['-I' . $msys2Prefix . '/include'],
            'libs' => ['-L' . $msys2Prefix . '/lib', '-lz'],
            'source' => 'MSYS2 zlib under ' . $msys2Prefix,
        ];
    }

    $candidates[] = [
        'cflags' => [],
        'libs' => ['-lz'],
        'source' => 'default zlib library',
    ];

    $unique = [];
    $result = [];
    foreach ($candidates as $candidate) {
        $key = implode("\0", $candidate['cflags']) . "\1" . implode("\0", $candidate['libs']);
        if (isset($unique[$key])) {
            continue;
        }
        $unique[$key] = true;
        $result[] = $candidate;
    }

    return $result;
}

/**
 * @param array<string, mixed> $options
 * @param array{cflags:list<string>, libs:list<string>, source:string} $candidate
 * @return array{available:bool, output:string, command_line:string}
 */
function probe_tcltk_candidate(array $options, array $candidate, string $scriptDir, string $buildRoot): array
{
    $probeDir = normalize_path($buildRoot . '/probe');
    ensure_directory($probeDir);

    $sourceFile = normalize_path($probeDir . '/tcltk_probe.cpp');
    $executable = normalize_path($probeDir . '/tcltk_probe');
    if (is_windows()) {
        $executable .= '.exe';
    }

    write_text_file(
        $sourceFile,
        "#include <tcl.h>\n#include <tk.h>\nauto main() -> int { return 0; }\n"
    );

    $command = array_merge(
        [$options['cxx'], '-std=gnu++23'],
        $options['cxxflags'],
        $candidate['cflags'],
        [$sourceFile, '-o', $executable],
        $candidate['libs'],
        $options['ldflags']
    );

    $result = run_command($command, $scriptDir);

    return [
        'available' => $result['exit_code'] === 0,
        'output' => $result['output'],
        'command_line' => $result['command_line'],
    ];
}

/**
 * @param array<string, mixed> $options
 * @param array{cflags:list<string>, libs:list<string>, source:string} $candidate
 * @return array{available:bool, output:string, command_line:string}
 */
function probe_icu_candidate(array $options, array $candidate, string $scriptDir, string $buildRoot): array
{
    $probeDir = normalize_path($buildRoot . '/probe');
    ensure_directory($probeDir);

    $sourceFile = normalize_path($probeDir . '/icu_probe.cpp');
    $executable = normalize_path($probeDir . '/icu_probe');
    if (is_windows()) {
        $executable .= '.exe';
    }

    write_text_file(
        $sourceFile,
        "#include <unicode/utypes.h>\n#include <unicode/ustring.h>\n#include <unicode/unorm2.h>\nauto main() -> int { UErrorCode status = U_ZERO_ERROR; return unorm2_getNFCInstance(&status) == nullptr; }\n"
    );

    $command = array_merge(
        [$options['cxx'], '-std=gnu++23'],
        $options['cxxflags'],
        $candidate['cflags'],
        [$sourceFile, '-o', $executable],
        $candidate['libs'],
        $options['ldflags']
    );

    $result = run_command($command, $scriptDir);

    return [
        'available' => $result['exit_code'] === 0,
        'output' => $result['output'],
        'command_line' => $result['command_line'],
    ];
}


/**
 * @param array<string, mixed> $options
 * @param array{cflags:list<string>, libs:list<string>, source:string} $candidate
 * @return array{available:bool, output:string, command_line:string}
 */
function probe_zlib_candidate(array $options, array $candidate, string $scriptDir, string $buildRoot): array
{
    $probeDir = normalize_path($buildRoot . '/probe');
    ensure_directory($probeDir);

    $sourceFile = normalize_path($probeDir . '/zlib_probe.cpp');
    $executable = normalize_path($probeDir . '/zlib_probe');
    if (is_windows()) {
        $executable .= '.exe';
    }

    write_text_file(
        $sourceFile,
        "#include <zlib.h>\nauto main() -> int { return zlibVersion() == nullptr; }\n"
    );

    $command = array_merge(
        [$options['cxx'], '-std=gnu++23'],
        $options['cxxflags'],
        $candidate['cflags'],
        [$sourceFile, '-o', $executable],
        $candidate['libs'],
        $options['ldflags']
    );

    $result = run_command($command, $scriptDir);

    return [
        'available' => $result['exit_code'] === 0,
        'output' => $result['output'],
        'command_line' => $result['command_line'],
    ];
}

/**
 * @param array<string, mixed> $options
 * @return array<string, array<string, mixed>>
 */
function detect_feature_options(array $options, string $scriptDir, string $buildRoot): array
{
    $features = [
        'socket' => [
            'available' => true,
            'cflags' => [],
            'libs' => is_windows_target() ? ['-lws2_32'] : [],
            'reason' => '',
            'source' => 'platform default',
        ],
        'tcltk' => [
            'available' => false,
            'cflags' => [],
            'libs' => [],
            'reason' => 'Tcl/Tk development files were not found.',
            'source' => '',
        ],
        'icu' => [
            'available' => false,
            'cflags' => [],
            'libs' => [],
            'reason' => 'ICU development files were not found.',
            'source' => '',
        ],
        'zip' => [
            'available' => false,
            'cflags' => [],
            'libs' => [],
            'reason' => 'zlib development files were not found.',
            'source' => '',
        ],
    ];

    $lastOutput = '';
    $lastCommandLine = '';
    foreach (tcltk_option_candidates($options) as $candidate) {
        $probe = probe_tcltk_candidate($options, $candidate, $scriptDir, $buildRoot);
        $lastOutput = $probe['output'];
        $lastCommandLine = $probe['command_line'];

        if (!$probe['available']) {
            continue;
        }

        $features['tcltk'] = [
            'available' => true,
            'cflags' => $candidate['cflags'],
            'libs' => $candidate['libs'],
            'reason' => '',
            'source' => $candidate['source'],
        ];
        break;
    }

    if (!$features['tcltk']['available'] && ($lastCommandLine !== '' || $lastOutput !== '')) {
        $features['tcltk']['reason'] = "Tcl/Tk probe failed.\n" . $lastCommandLine;
        if ($lastOutput !== '') {
            $features['tcltk']['reason'] .= "\n" . $lastOutput;
        }
    }

    $lastOutput = '';
    $lastCommandLine = '';
    foreach (icu_option_candidates($options) as $candidate) {
        $probe = probe_icu_candidate($options, $candidate, $scriptDir, $buildRoot);
        $lastOutput = $probe['output'];
        $lastCommandLine = $probe['command_line'];

        if (!$probe['available']) {
            continue;
        }

        $features['icu'] = [
            'available' => true,
            'cflags' => $candidate['cflags'],
            'libs' => $candidate['libs'],
            'reason' => '',
            'source' => $candidate['source'],
        ];
        break;
    }

    if (!$features['icu']['available'] && ($lastCommandLine !== '' || $lastOutput !== '')) {
        $features['icu']['reason'] = "ICU probe failed.\n" . $lastCommandLine;
        if ($lastOutput !== '') {
            $features['icu']['reason'] .= "\n" . $lastOutput;
        }
    }


    $lastOutput = '';
    $lastCommandLine = '';
    foreach (zlib_option_candidates($options) as $candidate) {
        $probe = probe_zlib_candidate($options, $candidate, $scriptDir, $buildRoot);
        $lastOutput = $probe['output'];
        $lastCommandLine = $probe['command_line'];

        if (!$probe['available']) {
            continue;
        }

        $features['zip'] = [
            'available' => true,
            'cflags' => $candidate['cflags'],
            'libs' => $candidate['libs'],
            'reason' => '',
            'source' => $candidate['source'],
        ];
        break;
    }

    if (!$features['zip']['available'] && ($lastCommandLine !== '' || $lastOutput !== '')) {
        $features['zip']['reason'] = "zlib probe failed.\n" . $lastCommandLine;
        if ($lastOutput !== '') {
            $features['zip']['reason'] .= "\n" . $lastOutput;
        }
    }

    return $features;
}

/**
 * @param list<string> $features
 * @param array<string, array<string, mixed>> $featureOptions
 * @return string|null
 */
function get_skip_reason(array $features, array $featureOptions): ?string
{
    foreach ($features as $feature) {
        if (!isset($featureOptions[$feature])) {
            return 'Unknown feature requirement: ' . $feature;
        }

        if (!$featureOptions[$feature]['available']) {
            return $featureOptions[$feature]['reason'];
        }
    }

    return null;
}

/**
 * @param list<string> $features
 * @param array<string, array<string, mixed>> $featureOptions
 * @return list<string>
 */
function collect_feature_cflags(array $features, array $featureOptions): array
{
    $flags = [];
    foreach ($features as $feature) {
        if (isset($featureOptions[$feature])) {
            array_push($flags, ...$featureOptions[$feature]['cflags']);
        }
    }
    return array_values(array_unique($flags, SORT_STRING));
}

/**
 * @param list<string> $features
 * @param array<string, array<string, mixed>> $featureOptions
 * @return list<string>
 */
function collect_feature_libs(array $features, array $featureOptions): array
{
    $libs = [];
    foreach ($features as $feature) {
        if (isset($featureOptions[$feature])) {
            array_push($libs, ...$featureOptions[$feature]['libs']);
        }
    }
    return array_values(array_unique($libs, SORT_STRING));
}

/**
 * @param array<string, mixed> $task
 * @param array<string, mixed> $options
 * @return list<string>
 */
function build_compile_command(array $task, array $options): array
{
    return array_merge(
        [$options['cxx'], '-std=gnu++23'],
        $options['cxxflags'],
        ['-I..'],
        $task['feature_cflags'],
        [$task['source_file'], '-o', $task['executable']],
        $task['feature_libs'],
        $options['ldflags']
    );
}


/**
 * @param list<string> $command
 * @return string
 */
function command_signature(array $command): string
{
    return hash('sha256', implode("\0", $command));
}

/**
 * @return array<string, mixed>|null
 */
function read_json_file(string $path): ?array
{
    if (!is_file($path)) {
        return null;
    }

    $content = file_get_contents($path);
    if ($content === false || $content === '') {
        return null;
    }

    $decoded = json_decode($content, true);
    return is_array($decoded) ? $decoded : null;
}

/**
 * @param array<string, mixed> $data
 * @return void
 */
function write_json_file(string $path, array $data): void
{
    $content = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    if ($content === false) {
        fail('Failed to encode JSON: ' . $path);
    }

    write_text_file($path, $content . PHP_EOL);
}

/**
 * @param list<string> $paths
 * @return int
 */
function latest_mtime(array $paths): int
{
    $latest = 0;

    foreach ($paths as $path) {
        if (!file_exists($path)) {
            continue;
        }

        $mtime = filemtime($path);
        if ($mtime !== false && $mtime > $latest) {
            $latest = $mtime;
        }
    }

    return $latest;
}

/**
 * @return list<string>
 */
function find_header_files(string $projectRoot): array
{
    $headers = [];
    $root = normalize_path($projectRoot . '/xer');

    if (!is_dir($root)) {
        return [];
    }

    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($root, FilesystemIterator::SKIP_DOTS)
    );

    foreach ($iterator as $entry) {
        if (!$entry->isFile()) {
            continue;
        }

        $path = normalize_path($entry->getPathname());
        $extension = strtolower(pathinfo($path, PATHINFO_EXTENSION));
        if (in_array($extension, ['h', 'hpp', 'ipp'], true)) {
            $headers[] = $path;
        }
    }

    sort($headers, SORT_STRING);
    return array_values($headers);
}

/**
 * @param array<string, mixed> $task
 * @param array<string, mixed> $options
 * @return bool
 */
function can_reuse_compile_result(array $task, array $options): bool
{
    if (!is_file($task['executable']) || !is_file($task['compile_log'])) {
        return false;
    }

    $cache = read_json_file($task['cache_file']);
    if ($cache === null) {
        return false;
    }

    $command = build_compile_command($task, $options);
    if (($cache['compile_signature'] ?? '') !== command_signature($command)) {
        return false;
    }

    if (($cache['compile_success'] ?? false) !== true) {
        return false;
    }

    $exeMtime = filemtime($task['executable']);
    if ($exeMtime === false) {
        return false;
    }

    return $exeMtime >= (int) $task['dependency_mtime'];
}

/**
 * @param array<string, mixed> $task
 * @return bool
 */
function can_reuse_run_result(array $task): bool
{
    if (!is_file($task['run_log'])) {
        return false;
    }

    $cache = read_json_file($task['cache_file']);
    if ($cache === null) {
        return false;
    }

    if (($cache['run_signature'] ?? '') !== command_signature([$task['executable']])) {
        return false;
    }

    if (($cache['run_success'] ?? false) !== true) {
        return false;
    }

    $runLogMtime = filemtime($task['run_log']);
    $exeMtime = filemtime($task['executable']);
    if ($runLogMtime === false || $exeMtime === false) {
        return false;
    }

    return $runLogMtime >= $exeMtime;
}

/**
 * @param array<string, mixed> $task
 * @param array<string, mixed> $options
 * @param list<string> $command
 * @return void
 */
function update_compile_cache(array $task, array $options, array $command, bool $success): void
{
    $cache = read_json_file($task['cache_file']) ?? [];
    $cache['source_file'] = $task['source_file'];
    $cache['executable'] = $task['executable'];
    $cache['dependency_mtime'] = $task['dependency_mtime'];
    $cache['compile_signature'] = command_signature($command);
    $cache['compile_success'] = $success;
    if (!$success) {
        $cache['run_success'] = false;
    }
    $cache['cxx'] = $options['cxx'];
    $cache['updated_at'] = date(DATE_ATOM);
    write_json_file($task['cache_file'], $cache);
}

/**
 * @param array<string, mixed> $task
 * @return void
 */
function update_run_cache(array $task, bool $success): void
{
    $cache = read_json_file($task['cache_file']) ?? [];
    $cache['run_signature'] = command_signature([$task['executable']]);
    $cache['run_success'] = $success;
    $cache['updated_at'] = date(DATE_ATOM);
    write_json_file($task['cache_file'], $cache);
}

/**
 * @param array<string, mixed> $task
 * @param array<string, mixed> $options
 * @return array{process:resource, pipes:array<int, resource>, command_line:string, stage:string}
 */
function start_compile_process(array $task, array $options, string $scriptDir): array
{
    $started = start_process(build_compile_command($task, $options), $scriptDir);
    $started['stage'] = 'compile';
    return $started;
}

/**
 * @param array<string, mixed> $task
 * @return array{process:resource, pipes:array<int, resource>, command_line:string, stage:string}
 */
function start_run_process(array $task): array
{
    recreate_directory($task['work_dir']);

    $command = [$task['executable']];

    $started = start_process($command, $task['work_dir']);
    $started['stage'] = 'run';
    return $started;
}

/**
 * @param list<string> $targets
 * @param array<string, array<string, mixed>> $featureOptions
 * @return array{
 *   tasks:list<array<string, mixed>>,
 *   target_totals:array<string, int>
 * }
 */
function build_tasks(array $targets, string $buildRoot, string $projectRoot, array $featureOptions, array $sourceSelectors): array
{
    $tasks = [];
    $targetTotals = [];
    $headerFiles = find_header_files($projectRoot);
    $headerMtime = latest_mtime($headerFiles);

    foreach ($targets as $target) {
        $sourceDir = normalize_path($projectRoot . '/' . $target);

        if (!is_dir($sourceDir)) {
            fail('Target directory not found: ' . $sourceDir);
        }

        $sourceFiles = find_cpp_sources($sourceDir);
        $sourceFiles = filter_source_files_by_selectors($sourceFiles, $target, $projectRoot, $sourceSelectors);
        if ($sourceFiles === []) {
            $message = $sourceSelectors === []
                ? 'No C++ source files were found in: ' . $sourceDir
                : 'No selected C++ source files were found for target: ' . $target;
            fail($message);
        }

        $buildDir = normalize_path($buildRoot . '/' . $target);
        $relativeBuildRoot = normalize_path('build/' . basename(dirname($buildRoot)) . '/' . basename($buildRoot));
        $exeDir = normalize_path($buildDir . '/bin');
        $logDir = normalize_path($buildDir . '/log');
        $workBaseDir = normalize_path($buildDir . '/work');
        $cacheDir = normalize_path($buildDir . '/cache');

        ensure_directory($buildDir);
        ensure_directory($exeDir);
        ensure_directory($logDir);
        ensure_directory($workBaseDir);
        ensure_directory($cacheDir);

        $targetTotals[$target] = count($sourceFiles);

        foreach ($sourceFiles as $sourceFile) {
            $baseName = pathinfo($sourceFile, PATHINFO_FILENAME);
            $executable = normalize_path($exeDir . '/' . $baseName);
            $compileLog = normalize_path($logDir . '/' . $baseName . '.compile.log');
            $runLog = normalize_path($logDir . '/' . $baseName . '.run.log');
            $workDir = normalize_path($workBaseDir . '/' . $baseName);
            $cacheFile = normalize_path($cacheDir . '/' . $baseName . '.json');
            $features = detect_source_features($sourceFile);
            $dependencyMtime = max((int) (filemtime($sourceFile) ?: 0), $headerMtime);

            if (is_windows()) {
                $executable .= '.exe';
            }

            $tasks[] = [
                'target' => $target,
                'name' => $baseName,
                'source_file' => $sourceFile,
                'relative_source' => normalize_path('../' . $target . '/' . basename($sourceFile)),
                'executable' => $executable,
                'compile_log' => $compileLog,
                'run_log' => $runLog,
                'work_dir' => $workDir,
                'cache_file' => $cacheFile,
                'dependency_mtime' => $dependencyMtime,
                'relative_compile_log' => normalize_path('./' . $relativeBuildRoot . '/' . $target . '/log/' . $baseName . '.compile.log'),
                'relative_run_log' => normalize_path('./' . $relativeBuildRoot . '/' . $target . '/log/' . $baseName . '.run.log'),
                'relative_work_dir' => normalize_path('./' . $relativeBuildRoot . '/' . $target . '/work/' . $baseName),
                'features' => $features,
                'feature_cflags' => collect_feature_cflags($features, $featureOptions),
                'feature_libs' => collect_feature_libs($features, $featureOptions),
                'skip_reason' => get_skip_reason($features, $featureOptions),
            ];
        }
    }

    return [
        'tasks' => $tasks,
        'target_totals' => $targetTotals,
    ];
}

/**
 * @param array<string, int> $targetTotals
 * @return array<string, array{total:int, skipped:int, up_to_date:int, compile_success:int, run_success:int}>
 */
function initialize_target_stats(array $targetTotals): array
{
    $stats = [];

    foreach ($targetTotals as $target => $total) {
        $stats[$target] = [
            'total' => $total,
            'skipped' => 0,
            'up_to_date' => 0,
            'compile_success' => 0,
            'run_success' => 0,
        ];
    }

    return $stats;
}

/**
 * @param array<string, array<string, mixed>> $featureOptions
 * @return void
 */
function print_feature_summary(array $featureOptions): void
{
    echo 'Feature options' . PHP_EOL;
    echo '------------------------------------------------------------' . PHP_EOL;

    foreach ($featureOptions as $feature => $info) {
        echo $feature . ' : ' . ($info['available'] ? 'available' : 'unavailable') . PHP_EOL;
        if ($info['source'] !== '') {
            echo '  source : ' . $info['source'] . PHP_EOL;
        }
        if ($info['cflags'] !== []) {
            echo '  cflags : ' . implode(' ', $info['cflags']) . PHP_EOL;
        }
        if ($info['libs'] !== []) {
            echo '  libs   : ' . implode(' ', $info['libs']) . PHP_EOL;
        }
    }

    echo PHP_EOL;
}

/**
 * @param array<string, array{total:int, skipped:int, up_to_date:int, compile_success:int, run_success:int}> $targetStats
 * @return void
 */
function print_target_headers(array $targetStats): void
{
    foreach (array_keys($targetStats) as $target) {
        echo '############################################################' . PHP_EOL;
        echo 'Target: ' . $target . PHP_EOL;
        echo '############################################################' . PHP_EOL;
    }
}

/**
 * @param array<string, mixed> $task
 * @return void
 */
function print_task_header(array $task): void
{
    echo '============================================================' . PHP_EOL;
    echo '[TARGET] ' . $task['target'] . PHP_EOL;
    echo '[NAME  ] ' . $task['name'] . PHP_EOL;
    echo '[SRC   ] ' . $task['relative_source'] . PHP_EOL;
    echo '[FEAT  ] ' . format_features($task['features']) . PHP_EOL;
}

/**
 * @param array<string, mixed> $task
 * @return void
 */
function print_skip_result(array $task): void
{
    echo '[SKIP  ] ' . str_replace(PHP_EOL, ' ', (string) $task['skip_reason']) . PHP_EOL;
}

/**
 * @param array<string, mixed> $task
 * @return void
 */
function print_compile_result(array $task, bool $compileOk): void
{
    echo '[COMP  ] ' . format_result_label($compileOk) . PHP_EOL;
    echo '[CLOG  ] ' . $task['relative_compile_log'] . PHP_EOL;
}

/**
 * @param array<string, mixed> $task
 * @return void
 */
function print_run_result(array $task, bool $runOk): void
{
    echo '[RUN   ] ' . format_result_label($runOk) . PHP_EOL;
    echo '[RLOG  ] ' . $task['relative_run_log'] . PHP_EOL;
    echo '[WDIR  ] ' . $task['relative_work_dir'] . PHP_EOL;
}


/**
 * @param array<string, mixed> $task
 * @return void
 */
function print_up_to_date_result(array $task): void
{
    echo '[CACHE ] up-to-date' . PHP_EOL;
    echo '[CLOG  ] ' . $task['relative_compile_log'] . PHP_EOL;
    echo '[RLOG  ] ' . $task['relative_run_log'] . PHP_EOL;
    echo '[WDIR  ] ' . $task['relative_work_dir'] . PHP_EOL;
}

/**
 * @param array<string, array{total:int, skipped:int, up_to_date:int, compile_success:int, run_success:int}> $targetStats
 * @return void
 */
function print_target_summaries(array $targetStats): void
{
    foreach ($targetStats as $target => $stats) {
        $active = $stats['total'] - $stats['skipped'];

        echo '============================================================' . PHP_EOL;
        echo 'Target summary: ' . $target . PHP_EOL;
        echo '============================================================' . PHP_EOL;
        echo 'Total programs  : ' . $stats['total'] . PHP_EOL;
        echo 'Skipped         : ' . $stats['skipped'] . PHP_EOL;
        echo 'Active programs : ' . $active . PHP_EOL;
        echo 'Up-to-date      : ' . $stats['up_to_date'] . PHP_EOL;
        echo 'Compile success : ' . $stats['compile_success'] . PHP_EOL;        echo 'Run success     : ' . $stats['run_success'] . PHP_EOL;
        echo 'Compile failed  : ' . ($active - $stats['compile_success']) . PHP_EOL;
        echo 'Run failed      : ' . ($stats['compile_success'] - $stats['run_success']) . PHP_EOL;
        echo PHP_EOL;
    }
}

/**
 * @param array<string, array{total:int, skipped:int, up_to_date:int, compile_success:int, run_success:int}> $targetStats
 * @param list<string> $targets
 * @param list<array{target:string, name:string, stage:string}> $failedPrograms
 * @param list<array{target:string, name:string, reason:string}> $skippedPrograms
 * @return int
 */
function print_overall_summary(array $targetStats, array $targets, array $failedPrograms, array $skippedPrograms): int
{
    $grandTotalCount = 0;
    $grandSkippedCount = 0;
    $grandUpToDateCount = 0;
    $grandCompileSuccessCount = 0;
    $grandRunSuccessCount = 0;

    foreach ($targetStats as $stats) {
        $grandTotalCount += $stats['total'];
        $grandSkippedCount += $stats['skipped'];
        $grandUpToDateCount += $stats['up_to_date'];
        $grandCompileSuccessCount += $stats['compile_success'];
        $grandRunSuccessCount += $stats['run_success'];
    }

    $grandActiveCount = $grandTotalCount - $grandSkippedCount;

    echo '############################################################' . PHP_EOL;
    echo 'Overall summary' . PHP_EOL;
    echo '############################################################' . PHP_EOL;
    echo 'Targets         : ' . implode(', ', $targets) . PHP_EOL;
    echo 'Total programs  : ' . $grandTotalCount . PHP_EOL;
    echo 'Skipped         : ' . $grandSkippedCount . PHP_EOL;
    echo 'Active programs : ' . $grandActiveCount . PHP_EOL;
    echo 'Up-to-date      : ' . $grandUpToDateCount . PHP_EOL;
    echo 'Compile success : ' . $grandCompileSuccessCount . PHP_EOL;    echo 'Run success     : ' . $grandRunSuccessCount . PHP_EOL;
    echo 'Compile failed  : ' . ($grandActiveCount - $grandCompileSuccessCount) . PHP_EOL;
    echo 'Run failed      : ' . ($grandCompileSuccessCount - $grandRunSuccessCount) . PHP_EOL;

    if ($skippedPrograms !== []) {
        echo PHP_EOL;
        echo 'Skipped programs' . PHP_EOL;
        echo '------------------------------------------------------------' . PHP_EOL;
        foreach ($skippedPrograms as $skippedProgram) {
            echo $skippedProgram['target']
                . '/'
                . $skippedProgram['name']
                . ' ['
                . $skippedProgram['reason']
                . ']'
                . PHP_EOL;
        }
    }

    if ($failedPrograms !== []) {
        echo PHP_EOL;
        echo 'Failed programs' . PHP_EOL;
        echo '------------------------------------------------------------' . PHP_EOL;
        foreach ($failedPrograms as $failedProgram) {
            echo $failedProgram['target']
                . '/'
                . $failedProgram['name']
                . ' ['
                . $failedProgram['stage']
                . ']'
                . PHP_EOL;
        }
        return EXIT_FAILURE_CODE;
    }

    return EXIT_SUCCESS_CODE;
}

/**
 * @param list<array<string, mixed>> $tasks
 * @param array<string, array{total:int, skipped:int, up_to_date:int, compile_success:int, run_success:int}> $targetStats
 * @param list<array{target:string, name:string, stage:string}> $failedPrograms
 * @param list<array{target:string, name:string, reason:string}> $skippedPrograms
 * @param array<string, mixed> $options
 * @return array{
 *   target_stats:array<string, array{total:int, skipped:int, compile_success:int, run_success:int}>,
 *   failed_programs:list<array{target:string, name:string, stage:string}>,
 *   skipped_programs:list<array{target:string, name:string, reason:string}>
 * }
 */
function run_tasks_in_parallel(
    array $tasks,
    array $targetStats,
    array $failedPrograms,
    array $skippedPrograms,
    int $jobs,
    string $scriptDir,
    array $options
): array {
    $pending = array_values($tasks);

    /** @var list<array{
     *   task:array<string, mixed>,
     *   stage:string,
     *   process:resource,
     *   pipes:array<int, resource>,
     *   command_line:string
     * }> $running */
    $running = [];

    while ($pending !== [] || $running !== []) {
        while ($pending !== [] && count($running) < $jobs) {
            /** @var array<string, mixed> $task */
            $task = array_shift($pending);

            if ($task['skip_reason'] !== null) {
                print_task_header($task);
                print_skip_result($task);
                ++$targetStats[$task['target']]['skipped'];
                $skippedPrograms[] = [
                    'target' => $task['target'],
                    'name' => $task['name'],
                    'reason' => strtok((string) $task['skip_reason'], "\n") ?: 'skipped',
                ];
                continue;
            }

            if (($options['all'] ?? false) !== true && can_reuse_compile_result($task, $options)) {
                ++$targetStats[$task['target']]['compile_success'];

                if (can_reuse_run_result($task)) {
                    print_task_header($task);
                    print_up_to_date_result($task);
                    ++$targetStats[$task['target']]['run_success'];
                    ++$targetStats[$task['target']]['up_to_date'];
                    continue;
                }

                $started = start_run_process($task);
                $running[] = [
                    'task' => $task,
                    'stage' => $started['stage'],
                    'process' => $started['process'],
                    'pipes' => $started['pipes'],
                    'command_line' => $started['command_line'],
                ];
                continue;
            }

            $started = start_compile_process($task, $options, $scriptDir);

            $running[] = [
                'task' => $task,
                'stage' => $started['stage'],
                'process' => $started['process'],
                'pipes' => $started['pipes'],
                'command_line' => $started['command_line'],
            ];
        }

        if ($running === []) {
            break;
        }

        $remaining = [];
        $hadCompletion = false;

        foreach ($running as $entry) {
            $status = proc_get_status($entry['process']);
            if ($status === false) {
                $result = finalize_process($entry['process'], $entry['pipes']);
                $result['exit_code'] = 1;
            } elseif ($status['running']) {
                $remaining[] = $entry;
                continue;
            } else {
                $result = finalize_process($entry['process'], $entry['pipes']);
            }

            $hadCompletion = true;
            $task = $entry['task'];

            if ($entry['stage'] === 'compile') {
                $compileOutput = 'Command: ' . $entry['command_line'];
                $compileOutput .= PHP_EOL . 'Working directory: ' . $scriptDir;
                if ($result['output'] !== '') {
                    $compileOutput .= PHP_EOL . $result['output'];
                }
                write_text_file($task['compile_log'], $compileOutput);
                $compileOk = ($result['exit_code'] === 0);
                update_compile_cache($task, $options, build_compile_command($task, $options), $compileOk);

                print_task_header($task);
                print_compile_result($task, $compileOk);

                if (!$compileOk) {
                    $failedPrograms[] = [
                        'target' => $task['target'],
                        'name' => $task['name'],
                        'stage' => 'compile',
                    ];
                    continue;
                }

                ++$targetStats[$task['target']]['compile_success'];

                $started = start_run_process($task);
                $remaining[] = [
                    'task' => $task,
                    'stage' => $started['stage'],
                    'process' => $started['process'],
                    'pipes' => $started['pipes'],
                    'command_line' => $started['command_line'],
                ];
                continue;
            }

            $runOutput = 'Command: ' . $entry['command_line'];
            $runOutput .= PHP_EOL . 'Working directory: ' . $task['work_dir'];
            if ($result['output'] !== '') {
                $runOutput .= PHP_EOL . $result['output'];
            }
            write_text_file($task['run_log'], $runOutput);
            $runOk = ($result['exit_code'] === 0);
            update_run_cache($task, $runOk);

            print_run_result($task, $runOk);

            if (!$runOk) {
                $failedPrograms[] = [
                    'target' => $task['target'],
                    'name' => $task['name'],
                    'stage' => 'run',
                ];
                continue;
            }

            ++$targetStats[$task['target']]['run_success'];
        }

        $running = $remaining;

        if (!$hadCompletion) {
            usleep(10000);
        }
    }

    return [
        'target_stats' => $targetStats,
        'failed_programs' => $failedPrograms,
        'skipped_programs' => $skippedPrograms,
    ];
}

$scriptDir = normalize_path(__DIR__);
$projectRoot = normalize_path(dirname($scriptDir));

if (normalize_path(getcwd() ?: '') !== $scriptDir) {
    fail('This script must be executed in the php directory.');
}

if (!function_exists('proc_open')) {
    fail('proc_open is not available.');
}

$parsed = parse_arguments($argv);
$toolchains = load_test_toolchains($scriptDir);
$selectedToolchain = (string) $parsed['toolchain'];

if ($selectedToolchain === 'all') {
    run_all_toolchains($argv, array_keys($toolchains));
}

if (!isset($toolchains[$selectedToolchain])) {
    fail('Unknown toolchain: ' . $selectedToolchain);
}

$parsed = apply_toolchain_config($parsed, $toolchains[$selectedToolchain]);

$buildId = $parsed['build_id'];
$buildRoot = normalize_path($scriptDir . '/build/' . $buildId . '/' . $selectedToolchain);
if ($parsed['clean_cache']) {
    remove_tree($buildRoot);
}
ensure_directory($buildRoot);
$options = [
    'cxx' => $parsed['cxx'],
    'cxxflags' => $parsed['cxxflags'],
    'ldflags' => $parsed['ldflags'],
    'tcltk_cflags' => $parsed['tcltk_cflags'],
    'tcltk_libs' => $parsed['tcltk_libs'],
    'icu_cflags' => $parsed['icu_cflags'],
    'icu_libs' => $parsed['icu_libs'],
    'all' => $parsed['all'],
];
$jobs = $parsed['jobs'];
$targets = $parsed['targets'];
$sourceSelectors = $parsed['source_selectors'];

if (find_executable($options['cxx']) === null) {
    fail($options['cxx'] . ' was not found in PATH.');
}

foreach ($targets as $target) {
    if (!is_supported_target($target)) {
        fail('Unsupported target directory: ' . $target);
    }
}

$featureOptions = detect_feature_options($options, $scriptDir, $buildRoot);
$taskBundle = build_tasks($targets, $buildRoot, $projectRoot, $featureOptions, $sourceSelectors);
$tasks = $taskBundle['tasks'];
$targetTotals = $taskBundle['target_totals'];

$targetStats = initialize_target_stats($targetTotals);
$failedPrograms = [];
$skippedPrograms = [];

echo '############################################################' . PHP_EOL;
echo 'Parallel test runner' . PHP_EOL;
echo '############################################################' . PHP_EOL;
echo 'Toolchain       : ' . $selectedToolchain . PHP_EOL;
echo 'Compiler        : ' . $options['cxx'] . PHP_EOL;
echo 'Build ID        : ' . $buildId . PHP_EOL;
echo 'Build root      : ' . $buildRoot . PHP_EOL;
echo 'Mode            : ' . ($options['all'] ? 'all' : 'incremental') . PHP_EOL;
echo 'Targets         : ' . implode(', ', $targets) . PHP_EOL;
if ($sourceSelectors !== []) {
    echo 'Source filters  : ' . implode(', ', $sourceSelectors) . PHP_EOL;
}
echo 'Jobs            : ' . $jobs . PHP_EOL;
echo 'Total programs  : ' . count($tasks) . PHP_EOL;
echo PHP_EOL;

print_feature_summary($featureOptions);
print_target_headers($targetStats);

$result = run_tasks_in_parallel(
    $tasks,
    $targetStats,
    $failedPrograms,
    $skippedPrograms,
    $jobs,
    $scriptDir,
    $options
);

$targetStats = $result['target_stats'];
$failedPrograms = $result['failed_programs'];
$skippedPrograms = $result['skipped_programs'];

print_target_summaries($targetStats);

exit(print_overall_summary($targetStats, $targets, $failedPrograms, $skippedPrograms));
