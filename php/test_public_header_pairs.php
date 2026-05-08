<?php
declare(strict_types=1);

/**
 * @file php/test_public_header_pairs.php
 * @brief Compiles ordered pairs of public headers under xer/.
 *
 * Usage:
 *   php test_public_header_pairs.php
 *   php test_public_header_pairs.php --all
 *   php test_public_header_pairs.php --compiler=g++
 *   php test_public_header_pairs.php --std=c++23
 *   php test_public_header_pairs.php --jobs=8
 *   php test_public_header_pairs.php --build-id=msys2-ucrt64
 *   php test_public_header_pairs.php --keep-going=0
 *   php test_public_header_pairs.php --verbose=0
 *   php test_public_header_pairs.php --tcltk-cflags="-I/usr/include/tcl -I/usr/include/tk"
 *   php test_public_header_pairs.php --skip-unavailable-features=0
 *
 * By default, this script runs only pairs that include public headers modified
 * after the last successful run. Use --all to run every ordered pair.
 */

final class Config
{
    public string $projectRoot;
    public string $phpDir;
    public string $xerDir;
    public string $buildId;
    public string $buildRoot;
    public string $buildDir;
    public string $stateFile;
    public string $compiler = 'g++';
    public string $cppStd = 'c++23';
    public bool $keepGoing = true;
    public bool $verbose = true;
    public bool $incremental = true;
    public int $jobs = 1;
    public ?string $tcltkCflags = null;
    public bool $skipUnavailableFeatures = true;
}

final class FeatureConfig
{
    public bool $tcltkAvailable = false;
    public string $tcltkCflags = '';
    public string $tcltkDetectionNote = '';
}

/**
 * @param array<int, string> $argv
 */
function main(array $argv): int
{
    $config = build_config($argv);
    $runStartedAt = time();

    validate_environment($config);
    ensure_directory($config->buildRoot);
    ensure_directory($config->buildDir);

    $features = detect_features($config);

    $headers = find_public_headers($config->xerDir);
    if (count($headers) < 2) {
        fwrite(STDERR, "error: fewer than 2 public headers were found under {$config->xerDir}\n");
        return 1;
    }

    clear_directory($config->buildDir);

    $lastSuccessfulRunAt = $config->incremental
        ? read_last_successful_run_at($config->stateFile)
        : null;

    $changedHeaders = [];
    if ($config->incremental && $lastSuccessfulRunAt !== null) {
        $changedHeaders = find_headers_modified_after(
            $headers,
            $config->projectRoot,
            $lastSuccessfulRunAt
        );
    }

    $allPairs = build_ordered_pairs($headers);
    $pairs = $config->incremental
        ? filter_pairs_by_headers($allPairs, $changedHeaders)
        : $allPairs;

    if ($config->incremental && $lastSuccessfulRunAt === null) {
        $pairs = $allPairs;
    }

    $total = count($pairs);

    echo "Project root : {$config->projectRoot}\n";
    echo "Header dir   : {$config->xerDir}\n";
    echo "Build ID     : {$config->buildId}\n";
    echo "Build root   : {$config->buildRoot}\n";
    echo "Build dir    : {$config->buildDir}\n";
    echo "State file   : {$config->stateFile}\n";
    echo "Compiler     : {$config->compiler}\n";
    echo "C++ standard : {$config->cppStd}\n";
    echo "Headers      : " . count($headers) . "\n";
    echo "Mode         : " . ($config->incremental ? 'incremental' : 'all') . "\n";
    echo "Tcl/Tk       : " . ($features->tcltkAvailable ? 'available' : 'unavailable') . "\n";
    if ($features->tcltkAvailable && $features->tcltkCflags !== '') {
        echo "Tcl/Tk cflags: {$features->tcltkCflags}\n";
    }
    if ($features->tcltkDetectionNote !== '') {
        echo "Tcl/Tk detect: {$features->tcltkDetectionNote}\n";
    }
    if ($config->incremental) {
        if ($lastSuccessfulRunAt === null) {
            echo "Last success : none\n";
            echo "Changed hdrs : all headers, because no previous successful run was recorded\n";
        } else {
            echo "Last success : " . format_timestamp($lastSuccessfulRunAt) . "\n";
            echo "Changed hdrs : " . count($changedHeaders) . "\n";
            foreach ($changedHeaders as $header) {
                echo "  - {$header}\n";
            }
        }
    }
    echo "Pair tests   : {$total}\n";
    echo "Jobs         : {$config->jobs}\n\n";

    if ($total === 0) {
        echo "No public headers were modified after the last successful run.\n";
        write_last_successful_run_at($config->stateFile, $runStartedAt);
        return 0;
    }

    $passed = 0;
    $failed = 0;
    $skipped = 0;
    $failures = [];
    $skips = [];
    $stopScheduling = false;

    /** @var array<int, array{index:int, total:int, header1:string, header2:string, features:array<int, string>}> $queue */
    $queue = [];
    $index = 0;
    foreach ($pairs as [$header1, $header2]) {
        ++$index;
        $queue[] = [
            'index' => $index,
            'total' => $total,
            'header1' => $header1,
            'header2' => $header2,
            'features' => detect_pair_features($header1, $header2),
        ];
    }

    /** @var array<int, array{
     *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
     *   process: resource,
     *   pipes: array<int, resource>,
     *   command: string,
     *   source: string,
     *   object: string,
     *   log: string
     * }> $running */
    $running = [];

    while (!$stopScheduling || count($running) > 0) {
        while (
            !$stopScheduling &&
            count($running) < $config->jobs &&
            count($queue) > 0
        ) {
            $meta = array_shift($queue);
            if ($meta === null) {
                break;
            }

            $skipReason = get_unavailable_feature_reason($config, $features, $meta['features']);
            if ($skipReason !== null) {
                ++$skipped;
                $skips[] = [
                    'header1' => $meta['header1'],
                    'header2' => $meta['header2'],
                    'reason' => $skipReason,
                ];
                if ($config->verbose) {
                    $label = sprintf(
                        '[%3d/%3d] %-40s + %-40s',
                        $meta['index'],
                        $meta['total'],
                        $meta['header1'],
                        $meta['header2']
                    );
                    echo $label . " : SKIP ({$skipReason})\n";
                }
                continue;
            }

            $task = start_compile_task($config, $features, $meta);
            $running[] = $task;
        }

        if (count($running) === 0) {
            if (count($queue) === 0) {
                break;
            }
            continue;
        }

        $completed = collect_completed_tasks($running);

        if (count($completed) === 0) {
            usleep(10000);
            continue;
        }

        foreach ($completed as $result) {
            $label = sprintf(
                '[%3d/%3d] %-40s + %-40s',
                $result['meta']['index'],
                $result['meta']['total'],
                $result['meta']['header1'],
                $result['meta']['header2']
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
                'header1' => $result['meta']['header1'],
                'header2' => $result['meta']['header2'],
                'source' => $result['source'],
                'log' => $result['log'],
                'command' => $result['command'],
                'exit_code' => $result['exit_code'],
            ];

            echo $label . " : NG\n";
            echo "  source : {$result['source']}\n";
            echo "  log    : {$result['log']}\n";

            if (!$config->keepGoing) {
                $stopScheduling = true;
            }
        }

        if ($stopScheduling && !$config->keepGoing) {
            while (count($queue) > 0) {
                array_shift($queue);
            }
        }
    }

    echo "\n";
    echo "Passed : {$passed}\n";
    echo "Failed : {$failed}\n";
    echo "Skipped: {$skipped}\n";
    echo "Total  : " . ($passed + $failed + $skipped) . "\n";

    if (count($skips) > 0 && $config->verbose) {
        echo "\nSkipped pairs:\n";
        foreach ($skips as $skip) {
            echo "- {$skip['header1']} + {$skip['header2']} ({$skip['reason']})\n";
        }
    }

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

    write_last_successful_run_at($config->stateFile, $runStartedAt);
    return 0;
}

/**
 * @param array<int, string> $argv
 */
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
    $config->buildId = detect_default_build_id();
    $config->buildRoot = $phpDir . DIRECTORY_SEPARATOR . 'build' . DIRECTORY_SEPARATOR . $config->buildId;
    $config->buildDir = $config->buildRoot . DIRECTORY_SEPARATOR . 'header_pair_compile';
    $config->stateFile = $config->buildRoot . DIRECTORY_SEPARATOR . 'header_pair_compile_state.json';
    $config->jobs = detect_default_jobs();

    foreach (array_slice($argv, 1) as $arg) {
        if ($arg === '--all') {
            $config->incremental = false;
            continue;
        }
        if ($arg === '--incremental') {
            $config->incremental = true;
            continue;
        }
        if (str_starts_with($arg, '--incremental=')) {
            $value = substr($arg, strlen('--incremental='));
            $config->incremental = parse_bool_option($value);
            continue;
        }
        if (str_starts_with($arg, '--build-id=')) {
            $config->buildId = sanitize_build_id(substr($arg, strlen('--build-id=')));
            $config->buildRoot = $config->phpDir . DIRECTORY_SEPARATOR . 'build' . DIRECTORY_SEPARATOR . $config->buildId;
            $config->buildDir = $config->buildRoot . DIRECTORY_SEPARATOR . 'header_pair_compile';
            $config->stateFile = $config->buildRoot . DIRECTORY_SEPARATOR . 'header_pair_compile_state.json';
            continue;
        }
        if (str_starts_with($arg, '--compiler=')) {
            $config->compiler = substr($arg, strlen('--compiler='));
            continue;
        }
        if (str_starts_with($arg, '--std=')) {
            $config->cppStd = substr($arg, strlen('--std='));
            continue;
        }
        if (str_starts_with($arg, '--jobs=')) {
            $value = substr($arg, strlen('--jobs='));
            $config->jobs = parse_positive_int_option($value, 'jobs');
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
        if (str_starts_with($arg, '--tcltk-cflags=')) {
            $config->tcltkCflags = substr($arg, strlen('--tcltk-cflags='));
            continue;
        }
        if (str_starts_with($arg, '--skip-unavailable-features=')) {
            $value = substr($arg, strlen('--skip-unavailable-features='));
            $config->skipUnavailableFeatures = parse_bool_option($value);
            continue;
        }

        throw new InvalidArgumentException("unknown option: {$arg}");
    }

    return $config;
}


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
        throw new InvalidArgumentException('build id must not be empty');
    }

    return $value;
}

function parse_bool_option(string $value): bool
{
    return match (strtolower($value)) {
        '1', 'true', 'yes', 'on' => true,
        '0', 'false', 'no', 'off' => false,
        default => throw new InvalidArgumentException("invalid boolean option value: {$value}"),
    };
}

function parse_positive_int_option(string $value, string $name): int
{
    if (!preg_match('/^[1-9][0-9]*$/', $value)) {
        throw new InvalidArgumentException("invalid {$name} value: {$value}");
    }

    return (int) $value;
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

    if (!function_exists('proc_open')) {
        throw new RuntimeException('proc_open is not available');
    }

    if ($config->jobs < 1) {
        throw new RuntimeException("invalid jobs value: {$config->jobs}");
    }
}

function find_executable(string $name): ?string
{
    $command = host_is_windows()
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

function detect_default_jobs(): int
{
    $count = detect_cpu_count();
    return max(1, $count);
}

function detect_cpu_count(): int
{
    $value = getenv('NUMBER_OF_PROCESSORS');
    if (is_string($value) && preg_match('/^[1-9][0-9]*$/', $value)) {
        return (int) $value;
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
        if ($exitCode !== 0 || count($output) === 0) {
            continue;
        }

        $value = trim($output[0]);
        if (preg_match('/^[1-9][0-9]*$/', $value)) {
            return (int) $value;
        }
    }

    return 1;
}

function detect_features(Config $config): FeatureConfig
{
    $features = new FeatureConfig();

    $manual = $config->tcltkCflags;
    if ($manual !== null) {
        $features->tcltkAvailable = can_compile_tcltk_probe($config, $manual);
        $features->tcltkCflags = $manual;
        $features->tcltkDetectionNote = $features->tcltkAvailable
            ? 'manual --tcltk-cflags'
            : 'manual --tcltk-cflags did not compile';
        return $features;
    }

    foreach (detect_tcltk_cflag_candidates() as $candidate) {
        if (can_compile_tcltk_probe($config, $candidate['cflags'])) {
            $features->tcltkAvailable = true;
            $features->tcltkCflags = $candidate['cflags'];
            $features->tcltkDetectionNote = $candidate['note'];
            return $features;
        }
    }

    $features->tcltkAvailable = false;
    $features->tcltkDetectionNote = 'no usable Tcl/Tk include configuration was found';
    return $features;
}

/**
 * @return array<int, array{cflags:string, note:string}>
 */
function detect_tcltk_cflag_candidates(): array
{
    $candidates = [];
    $seen = [];

    $add = static function (string $cflags, string $note) use (&$candidates, &$seen): void {
        $normalized = trim($cflags);
        $key = $normalized;
        if (isset($seen[$key])) {
            return;
        }
        $seen[$key] = true;
        $candidates[] = [
            'cflags' => $normalized,
            'note' => $note,
        ];
    };

    foreach (query_pkg_config_cflags() as $entry) {
        $add($entry['cflags'], $entry['note']);
    }

    // MSYS2 and many non-Debian environments install Tcl/Tk headers in the
    // compiler's normal include path, such as /mingw64/include or /ucrt64/include.
    // Therefore the empty candidate must be tested before Debian-specific paths.
    $add('', 'compiler default include paths');

    // Debian-family distributions commonly require an explicit Tcl/Tk include
    // directory. Do not add these paths on MSYS2, because /usr/include/tk is not
    // the intended location there and can cause a false skip.
    if (is_debian_like() && !is_msys2()) {
        $debianCandidates = [
            '-I/usr/include/tcl -I/usr/include/tk',
            '-I/usr/include/tcl',
            '-I/usr/include/tk',
        ];
        foreach ($debianCandidates as $cflags) {
            $add($cflags, 'Debian-family Tcl/Tk include path');
        }
    }

    return $candidates;
}

/**
 * @return array<int, array{cflags:string, note:string}>
 */
function query_pkg_config_cflags(): array
{
    if (find_executable('pkg-config') === null) {
        return [];
    }

    $queries = [
        'tcl tk',
        'tcl',
        'tk',
    ];

    $results = [];
    foreach ($queries as $query) {
        $command = 'pkg-config --cflags ' . $query . ' 2>/dev/null';
        $output = [];
        $exitCode = 0;
        exec($command, $output, $exitCode);
        if ($exitCode !== 0) {
            continue;
        }

        $cflags = trim(implode(' ', array_map('trim', $output)));
        $results[] = [
            'cflags' => $cflags,
            'note' => "pkg-config --cflags {$query}",
        ];
    }

    return $results;
}

function can_compile_tcltk_probe(Config $config, string $cflags): bool
{
    $probeDir = $config->buildRoot . DIRECTORY_SEPARATOR . 'feature_probe';
    ensure_directory($probeDir);

    $sourcePath = $probeDir . DIRECTORY_SEPARATOR . 'tcltk_probe.cpp';
    $objectPath = $probeDir . DIRECTORY_SEPARATOR . 'tcltk_probe.o';
    $logPath = $probeDir . DIRECTORY_SEPARATOR . 'tcltk_probe.log';

    $source = <<<'CPP'
#include <tcl.h>
#include <tk.h>

auto main() -> int
{
    return 0;
}
CPP;

    if (file_put_contents($sourcePath, $source) === false) {
        throw new RuntimeException("failed to write Tcl/Tk probe source: {$sourcePath}");
    }

    $parts = [
        escapeshellarg($config->compiler),
        '-std=' . escapeshellarg($config->cppStd),
    ];
    if (trim($cflags) !== '') {
        $parts[] = trim($cflags);
    }
    $parts[] = '-c';
    $parts[] = escapeshellarg($sourcePath);
    $parts[] = '-o';
    $parts[] = escapeshellarg($objectPath);

    $command = implode(' ', $parts);

    $output = [];
    $exitCode = 0;
    exec($command . ' 2>&1', $output, $exitCode);

    $log = "Command: {$command}" . PHP_EOL . PHP_EOL . implode(PHP_EOL, $output);
    if (!str_ends_with($log, PHP_EOL)) {
        $log .= PHP_EOL;
    }
    file_put_contents($logPath, $log);

    return $exitCode === 0;
}

/**
 * @param array<int, string> $features
 */
function get_unavailable_feature_reason(
    Config $config,
    FeatureConfig $featureConfig,
    array $features
): ?string {
    if (!in_array('tcltk', $features, true)) {
        return null;
    }

    if ($featureConfig->tcltkAvailable) {
        return null;
    }

    if (!$config->skipUnavailableFeatures) {
        return null;
    }

    return 'Tcl/Tk headers are unavailable';
}

/**
 * @return array<int, string>
 */
function detect_pair_features(string $header1, string $header2): array
{
    $features = [];
    if ($header1 === 'xer/tk.h' || $header2 === 'xer/tk.h') {
        $features[] = 'tcltk';
    }
    return $features;
}

/**
 * @return array<int, string>
 */
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

/**
 * @param array<int, string> $headers
 * @return array<int, array{0:string, 1:string}>
 */
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

/**
 * @param array<int, string> $headers
 * @return array<int, string>
 */
function find_headers_modified_after(array $headers, string $projectRoot, int $timestamp): array
{
    $changed = [];

    foreach ($headers as $header) {
        $path = $projectRoot . DIRECTORY_SEPARATOR . str_replace('/', DIRECTORY_SEPARATOR, $header);
        $mtime = filemtime($path);
        if ($mtime === false) {
            throw new RuntimeException("failed to read file modification time: {$path}");
        }

        if ($mtime > $timestamp) {
            $changed[] = $header;
        }
    }

    return $changed;
}

/**
 * @param array<int, array{0:string, 1:string}> $pairs
 * @param array<int, string> $headers
 * @return array<int, array{0:string, 1:string}>
 */
function filter_pairs_by_headers(array $pairs, array $headers): array
{
    if (count($headers) === 0) {
        return [];
    }

    $set = array_fill_keys($headers, true);
    $filtered = [];

    foreach ($pairs as $pair) {
        if (isset($set[$pair[0]]) || isset($set[$pair[1]])) {
            $filtered[] = $pair;
        }
    }

    return $filtered;
}

function read_last_successful_run_at(string $stateFile): ?int
{
    if (!is_file($stateFile)) {
        return null;
    }

    $json = file_get_contents($stateFile);
    if ($json === false || $json === '') {
        return null;
    }

    $data = json_decode($json, true);
    if (!is_array($data)) {
        return null;
    }

    $value = $data['last_successful_run_at'] ?? null;
    if (!is_int($value)) {
        return null;
    }

    return $value >= 0 ? $value : null;
}

function write_last_successful_run_at(string $stateFile, int $timestamp): void
{
    $directory = dirname($stateFile);
    ensure_directory($directory);

    $data = [
        'last_successful_run_at' => $timestamp,
        'last_successful_run_at_text' => format_timestamp($timestamp),
    ];

    $json = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    if ($json === false) {
        throw new RuntimeException('failed to encode state file JSON');
    }

    if (file_put_contents($stateFile, $json . PHP_EOL) === false) {
        throw new RuntimeException("failed to write state file: {$stateFile}");
    }
}

function format_timestamp(int $timestamp): string
{
    return date('Y-m-d H:i:s', $timestamp);
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

auto main() -> int
{
    return 0;
}

CPP;
}

/**
 * @param array{index:int, total:int, header1:string, header2:string, features:array<int, string>} $meta
 * @return array{
 *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
 *   process: resource,
 *   pipes: array<int, resource>,
 *   command: string,
 *   source: string,
 *   object: string,
 *   log: string
 * }
 */
function start_compile_task(Config $config, FeatureConfig $features, array $meta): array
{
    $header1 = $meta['header1'];
    $header2 = $meta['header2'];

    $testName = make_test_name($header1, $header2);
    $sourcePath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.cpp';
    $objectPath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.o';
    $logPath = $config->buildDir . DIRECTORY_SEPARATOR . $testName . '.log';

    $source = make_source_code($header1, $header2);
    file_put_contents($sourcePath, $source);

    $command = build_compile_command(
        compiler: $config->compiler,
        cppStd: $config->cppStd,
        projectRoot: $config->projectRoot,
        sourcePath: $sourcePath,
        objectPath: $objectPath,
        extraCflags: get_pair_extra_cflags($features, $meta['features'])
    );

    $descriptorSpec = [
        0 => ['pipe', 'r'],
        1 => ['pipe', 'w'],
        2 => ['pipe', 'w'],
    ];

    $pipes = [];
    $process = proc_open($command, $descriptorSpec, $pipes, $config->projectRoot);

    if (!is_resource($process)) {
        throw new RuntimeException("failed to start compiler process: {$command}");
    }

    fclose($pipes[0]);
    stream_set_blocking($pipes[1], false);
    stream_set_blocking($pipes[2], false);

    return [
        'meta' => $meta,
        'process' => $process,
        'pipes' => $pipes,
        'command' => $command,
        'source' => $sourcePath,
        'object' => $objectPath,
        'log' => $logPath,
    ];
}

/**
 * @param array<int, string> $features
 */
function get_pair_extra_cflags(FeatureConfig $featureConfig, array $features): string
{
    if (in_array('tcltk', $features, true)) {
        return $featureConfig->tcltkCflags;
    }
    return '';
}

function build_compile_command(
    string $compiler,
    string $cppStd,
    string $projectRoot,
    string $sourcePath,
    string $objectPath,
    string $extraCflags = ''
): string {
    $commandParts = [
        escapeshellarg($compiler),
        '-std=' . escapeshellarg($cppStd),
        '-I',
        escapeshellarg($projectRoot),
    ];

    if (trim($extraCflags) !== '') {
        $commandParts[] = trim($extraCflags);
    }

    $commandParts[] = '-c';
    $commandParts[] = escapeshellarg($sourcePath);
    $commandParts[] = '-o';
    $commandParts[] = escapeshellarg($objectPath);

    return implode(' ', $commandParts);
}

/**
 * @param array<int, array{
 *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
 *   process: resource,
 *   pipes: array<int, resource>,
 *   command: string,
 *   source: string,
 *   object: string,
 *   log: string
 * }> $running
 * @return array<int, array{
 *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
 *   success: bool,
 *   exit_code: int,
 *   command: string,
 *   source: string,
 *   log: string
 * }>
 */
function collect_completed_tasks(array &$running): array
{
    $completed = [];
    $remaining = [];

    foreach ($running as $task) {
        $status = proc_get_status($task['process']);
        if ($status === false) {
            $completed[] = finish_compile_task($task, 1);
            continue;
        }

        if ($status['running']) {
            $remaining[] = $task;
            continue;
        }

        $completed[] = finish_compile_task($task, $status['exitcode']);
    }

    $running = $remaining;
    return $completed;
}

/**
 * @param array{
 *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
 *   process: resource,
 *   pipes: array<int, resource>,
 *   command: string,
 *   source: string,
 *   object: string,
 *   log: string
 * } $task
 * @return array{
 *   meta: array{index:int, total:int, header1:string, header2:string, features:array<int, string>},
 *   success: bool,
 *   exit_code: int,
 *   command: string,
 *   source: string,
 *   log: string
 * }
 */
function finish_compile_task(array $task, int $exitCode): array
{
    $stdout = stream_get_contents($task['pipes'][1]);
    $stderr = stream_get_contents($task['pipes'][2]);

    fclose($task['pipes'][1]);
    fclose($task['pipes'][2]);

    $closeCode = proc_close($task['process']);
    if ($closeCode >= 0) {
        $exitCode = $closeCode;
    }

    $log = '';
    $log .= "Command: {$task['command']}" . PHP_EOL . PHP_EOL;
    if ($stdout !== false && $stdout !== '') {
        $log .= $stdout;
    }
    if ($stderr !== false && $stderr !== '') {
        $log .= $stderr;
    }
    if ($log !== '' && !str_ends_with($log, PHP_EOL)) {
        $log .= PHP_EOL;
    }

    file_put_contents($task['log'], $log);

    return [
        'meta' => $task['meta'],
        'success' => $exitCode === 0,
        'exit_code' => $exitCode,
        'command' => $task['command'],
        'source' => $task['source'],
        'log' => $task['log'],
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

function host_is_windows(): bool
{
    return DIRECTORY_SEPARATOR === '\\';
}

function is_msys2(): bool
{
    $msystem = getenv('MSYSTEM');
    if (is_string($msystem) && $msystem !== '') {
        return true;
    }

    $ostype = getenv('OSTYPE');
    if (is_string($ostype) && stripos($ostype, 'msys') !== false) {
        return true;
    }

    $uname = php_uname('s');
    return stripos($uname, 'MSYS') !== false || stripos($uname, 'MINGW') !== false;
}

function is_debian_like(): bool
{
    if (is_file('/etc/debian_version')) {
        return true;
    }

    if (!is_file('/etc/os-release')) {
        return false;
    }

    $text = file_get_contents('/etc/os-release');
    if (!is_string($text)) {
        return false;
    }

    return stripos($text, 'ID=debian') !== false
        || stripos($text, 'ID=ubuntu') !== false
        || stripos($text, 'ID_LIKE=debian') !== false;
}

try {
    exit(main($argv));
} catch (Throwable $e) {
    fwrite(STDERR, 'error: ' . $e->getMessage() . PHP_EOL);
    exit(2);
}
