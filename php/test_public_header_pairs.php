<?php
declare(strict_types=1);

/**
 * @file php/test_public_header_pairs.php
 * @brief Compiles ordered pairs of public headers under xer/ with optional parallel execution.
 *
 * Usage:
 *   php test_public_header_pairs.php
 *   php test_public_header_pairs.php --compiler=g++
 *   php test_public_header_pairs.php --std=c++23
 *   php test_public_header_pairs.php --jobs=8
 *   php test_public_header_pairs.php --keep-going=0
 *   php test_public_header_pairs.php --verbose=0
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
    public int $jobs = 1;
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

    echo "Project root : {$config->projectRoot}\n";
    echo "Header dir   : {$config->xerDir}\n";
    echo "Build dir    : {$config->buildDir}\n";
    echo "Compiler     : {$config->compiler}\n";
    echo "C++ standard : {$config->cppStd}\n";
    echo "Headers      : " . count($headers) . "\n";
    echo "Pair tests   : {$total}\n";
    echo "Jobs         : {$config->jobs}\n\n";

    $passed = 0;
    $failed = 0;
    $failures = [];
    $stopScheduling = false;

    /** @var array<int, array{index:int, total:int, header1:string, header2:string}> $queue */
    $queue = [];
    $index = 0;
    foreach ($pairs as [$header1, $header2]) {
        ++$index;
        $queue[] = [
            'index' => $index,
            'total' => $total,
            'header1' => $header1,
            'header2' => $header2,
        ];
    }

    /** @var array<int, array{
     *   meta: array{index:int, total:int, header1:string, header2:string},
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

            $task = start_compile_task($config, $meta);
            $running[] = $task;
        }

        if (count($running) === 0) {
            break;
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
    $config->jobs = detect_default_jobs();

    foreach (array_slice($argv, 1) as $arg) {
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

function detect_default_jobs(): int
{
    $count = detect_cpu_count();
    return max(1, $count);
}

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

function start_compile_task(Config $config, array $meta): array
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
        objectPath: $objectPath
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

function build_compile_command(
    string $compiler,
    string $cppStd,
    string $projectRoot,
    string $sourcePath,
    string $objectPath
): string {
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

    return implode(' ', $commandParts);
}

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
