<?php
declare(strict_types=1);

/**
 * @file php/test_public_header_pairs.php
 * @brief Compiles ordered pairs of public headers under xer/.
 *
 * Usage:
 *   php test_public_header_pairs.php
 *   php test_public_header_pairs.php --compiler=g++
 *   php test_public_header_pairs.php --std=c++23
 *   php test_public_header_pairs.php --keep-going=0
 */

final class Config
{
    public string $projectRoot;
    public string $phpDir;
    public string $xerDir;
    public string $buildDir;
    public string $compiler = 'g++';
    public string $cppStd = 'c++23';
    public bool $keepGoing = true;
    public bool $verbose = true;
}

function main(array $argv): int
{
    $config = build_config($argv);

    validate_environment($config);

    $headers = find_public_headers($config->xerDir);
    if (count($headers) < 2) {
        fwrite(STDERR, "error: fewer than 2 public headers were found under {$config->xerDir}\n");
        return 1;
    }

    ensure_directory($config->buildDir);
    clear_directory($config->buildDir);

    $pairs = build_ordered_pairs($headers);

    $total = count($pairs);
    $passed = 0;
    $failed = 0;
    $failures = [];

    echo "Project root : {$config->projectRoot}\n";
    echo "Header dir   : {$config->xerDir}\n";
    echo "Build dir    : {$config->buildDir}\n";
    echo "Compiler     : {$config->compiler}\n";
    echo "C++ standard : {$config->cppStd}\n";
    echo "Headers      : " . count($headers) . "\n";
    echo "Pair tests   : {$total}\n\n";

    $index = 0;
    foreach ($pairs as [$header1, $header2]) {
        ++$index;

        $testName = make_test_name($header1, $header2);
        $sourcePath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.cpp';
        $objectPath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.o';
        $logPath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.log';

        $source = make_source_code($header1, $header2);
        file_put_contents($sourcePath, $source);

        $result = compile_source(
            compiler: $config->compiler,
            cppStd: $config->cppStd,
            projectRoot: $config->projectRoot,
            sourcePath: $sourcePath,
            objectPath: $objectPath,
            logPath: $logPath
        );

        $label = sprintf(
            '[%3d/%3d] %-40s + %-40s',
            $index,
            $total,
            $header1,
            $header2
        );

        if ($result['success']) {
            ++$passed;
            if ($config->verbose) {
                echo $label . " : OK\n";
            }
            continue;
        }

        ++$failed;
        $failures[] = [
            'header1' => $header1,
            'header2' => $header2,
            'source' => $sourcePath,
            'log' => $logPath,
            'command' => $result['command'],
            'exit_code' => $result['exit_code'],
        ];

        echo $label . " : NG\n";
        echo "  source : {$sourcePath}\n";
        echo "  log    : {$logPath}\n";

        if (!$config->keepGoing) {
            break;
        }
    }

    echo "\n";
    echo "Passed: {$passed}\n";
    echo "Failed: {$failed}\n";
    echo "Total : " . ($passed + $failed) . "\n";

    if ($failed > 0) {
        echo "\nFailed pairs:\n";
        foreach ($failures as $failure) {
            echo "- {$failure['header1']} + {$failure['header2']}\n";
            echo "  source   : {$failure['source']}\n";
            echo "  log      : {$failure['log']}\n";
            echo "  command  : {$failure['command']}\n";
            echo "  exit code: {$failure['exit_code']}\n";
        }
        return 1;
    }

    return 0;
}

function build_config(array $argv): Config
{
    $config = new Config();

    $phpDir = realpath(__DIR__);
    if ($phpDir === false) {
        throw new RuntimeException('failed to resolve __DIR__');
    }

    $projectRoot = realpath($phpDir . DIRECTORY_SEPARATOR . '..');
    if ($projectRoot === false) {
        throw new RuntimeException('failed to resolve project root');
    }

    $config->phpDir = $phpDir;
    $config->projectRoot = $projectRoot;
    $config->xerDir = $projectRoot . DIRECTORY_SEPARATOR . 'xer';
    $config->buildDir = $phpDir . DIRECTORY_SEPARATOR . 'build' . DIRECTORY_SEPARATOR . 'header_pair_compile';

    foreach (array_slice($argv, 1) as $arg) {
        if (str_starts_with($arg, '--compiler=')) {
            $config->compiler = substr($arg, strlen('--compiler='));
            continue;
        }
        if (str_starts_with($arg, '--std=')) {
            $config->cppStd = substr($arg, strlen('--std='));
            continue;
        }
        if (str_starts_with($arg, '--keep-going=')) {
            $value = substr($arg, strlen('--keep-going='));
            $config->keepGoing = parse_bool_option($value);
            continue;
        }
        if (str_starts_with($arg, '--verbose=')) {
            $value = substr($arg, strlen('--verbose='));
            $config->verbose = parse_bool_option($value);
            continue;
        }

        throw new InvalidArgumentException("unknown option: {$arg}");
    }

    return $config;
}

function parse_bool_option(string $value): bool
{
    return match (strtolower($value)) {
        '1', 'true', 'yes', 'on' => true,
        '0', 'false', 'no', 'off' => false,
        default => throw new InvalidArgumentException("invalid boolean option value: {$value}"),
    };
}

function validate_environment(Config $config): void
{
    if (!is_dir($config->xerDir)) {
        throw new RuntimeException("xer directory not found: {$config->xerDir}");
    }

    $compilerPath = find_executable($config->compiler);
    if ($compilerPath === null) {
        throw new RuntimeException("compiler not found in PATH: {$config->compiler}");
    }
}

function find_executable(string $name): ?string
{
    $command = is_windows()
        ? 'where ' . escapeshellarg($name)
        : 'command -v ' . escapeshellarg($name);

    $output = [];
    $exitCode = 0;
    exec($command, $output, $exitCode);

    if ($exitCode !== 0 || count($output) === 0) {
        return null;
    }

    $path = trim($output[0]);
    return $path !== '' ? $path : null;
}

function find_public_headers(string $xerDir): array
{
    $entries = scandir($xerDir);
    if ($entries === false) {
        throw new RuntimeException("failed to scan directory: {$xerDir}");
    }

    $headers = [];
    foreach ($entries as $entry) {
        if ($entry === '.' || $entry === '..') {
            continue;
        }

        $path = $xerDir . DIRECTORY_SEPARATOR . $entry;
        if (!is_file($path)) {
            continue;
        }

        if (pathinfo($entry, PATHINFO_EXTENSION) !== 'h') {
            continue;
        }

        $headers[] = 'xer/' . $entry;
    }

    sort($headers, SORT_STRING);
    return $headers;
}

function build_ordered_pairs(array $headers): array
{
    $pairs = [];

    $count = count($headers);
    for ($i = 0; $i < $count; ++$i) {
        for ($j = 0; $j < $count; ++$j) {
            if ($i === $j) {
                continue;
            }
            $pairs[] = [$headers[$i], $headers[$j]];
        }
    }

    return $pairs;
}

function make_test_name(string $header1, string $header2): string
{
    return sanitize_filename($header1) . '__' . sanitize_filename($header2);
}

function sanitize_filename(string $value): string
{
    $value = str_replace(['/', '\\', '.'], '_', $value);
    $value = preg_replace('/[^A-Za-z0-9_]+/', '_', $value);
    if ($value === null || $value === '') {
        throw new RuntimeException('failed to sanitize file name');
    }
    return $value;
}

function make_source_code(string $header1, string $header2): string
{
    return <<<CPP
#include <{$header1}>
#include <{$header2}>

int main()
{
    return 0;
}

CPP;
}

function compile_source(
    string $compiler,
    string $cppStd,
    string $projectRoot,
    string $sourcePath,
    string $objectPath,
    string $logPath
): array {
    $commandParts = [
        escapeshellarg($compiler),
        '-std=' . escapeshellarg($cppStd),
        '-I',
        escapeshellarg($projectRoot),
        '-c',
        escapeshellarg($sourcePath),
        '-o',
        escapeshellarg($objectPath),
    ];

    $command = implode(' ', $commandParts) . ' 2>&1';

    $output = [];
    $exitCode = 0;
    exec($command, $output, $exitCode);

    $log = implode(PHP_EOL, $output);
    if ($log !== '') {
        $log .= PHP_EOL;
    }
    file_put_contents($logPath, $log);

    return [
        'success' => $exitCode === 0,
        'exit_code' => $exitCode,
        'command' => $command,
    ];
}

function ensure_directory(string $path): void
{
    if (is_dir($path)) {
        return;
    }

    if (!mkdir($path, 0777, true) && !is_dir($path)) {
        throw new RuntimeException("failed to create directory: {$path}");
    }
}

function clear_directory(string $path): void
{
    if (!is_dir($path)) {
        return;
    }

    $entries = scandir($path);
    if ($entries === false) {
        throw new RuntimeException("failed to scan directory: {$path}");
    }

    foreach ($entries as $entry) {
        if ($entry === '.' || $entry === '..') {
            continue;
        }

        $fullPath = $path . DIRECTORY_SEPARATOR . $entry;
        if (is_dir($fullPath)) {
            clear_directory($fullPath);
            if (!rmdir($fullPath)) {
                throw new RuntimeException("failed to remove directory: {$fullPath}");
            }
            continue;
        }

        if (!unlink($fullPath)) {
            throw new RuntimeException("failed to remove file: {$fullPath}");
        }
    }
}

function is_windows(): bool
{
    return DIRECTORY_SEPARATOR === '\\';
}

try {
    exit(main($argv));
} catch (Throwable $e) {
    fwrite(STDERR, 'error: ' . $e->getMessage() . PHP_EOL);
    exit(2);
}
