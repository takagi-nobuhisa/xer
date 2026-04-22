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
 *   php run_tests.php examples
 *   php run_tests.php tests examples
 *   php run_tests.php --jobs=8
 *   php run_tests.php --jobs=8 tests examples
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
    return DIRECTORY_SEPARATOR === '\\';
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
function parse_jobs_option(string $value): int
{
    if (!preg_match('/^[1-9][0-9]*$/', $value)) {
        fail('Invalid --jobs value: ' . $value);
    }

    return (int) $value;
}

/**
 * @param list<string> $argv
 * @return array{jobs:int, targets:list<string>}
 */
function parse_arguments(array $argv): array
{
    $jobs = detect_default_jobs();
    $targets = [];

    foreach (array_slice($argv, 1) as $arg) {
        if (str_starts_with($arg, '--jobs=')) {
            $jobs = parse_jobs_option(substr($arg, strlen('--jobs=')));
            continue;
        }

        $targets[] = $arg;
    }

    return [
        'jobs' => $jobs,
        'targets' => normalize_targets($targets),
    ];
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
    $parts = array_map('shell_quote_path', $command);
    $commandLine = implode(' ', $parts);

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
 * @param array<string, mixed> $task
 * @return array{process:resource, pipes:array<int, resource>, command_line:string, stage:string}
 */
function start_compile_process(array $task, string $scriptDir): array
{
    $command = [
        'g++',
        '-std=gnu++23',
        '-I..',
        $task['source_file'],
        '-o',
        $task['executable'],
    ];

    $started = start_process($command, $scriptDir);
    $started['stage'] = 'compile';
    return $started;
}

/**
 * @param array<string, mixed> $task
 * @return array{process:resource, pipes:array<int, resource>, command_line:string, stage:string}
 */
function start_run_process(array $task, string $scriptDir): array
{
    $command = [$task['executable']];

    $started = start_process($command, $scriptDir);
    $started['stage'] = 'run';
    return $started;
}

/**
 * @param list<string> $targets
 * @return array{
 *   tasks:list<array<string, mixed>>,
 *   target_totals:array<string, int>
 * }
 */
function build_tasks(array $targets, string $scriptDir, string $projectRoot): array
{
    $tasks = [];
    $targetTotals = [];

    foreach ($targets as $target) {
        $sourceDir = normalize_path($projectRoot . '/' . $target);

        if (!is_dir($sourceDir)) {
            fail('Target directory not found: ' . $sourceDir);
        }

        $sourceFiles = find_cpp_sources($sourceDir);
        if ($sourceFiles === []) {
            fail('No C++ source files were found in: ' . $sourceDir);
        }

        $buildDir = normalize_path($scriptDir . '/build/' . $target);
        $exeDir = normalize_path($buildDir . '/bin');
        $logDir = normalize_path($buildDir . '/log');

        ensure_directory($buildDir);
        ensure_directory($exeDir);
        ensure_directory($logDir);

        $targetTotals[$target] = count($sourceFiles);

        foreach ($sourceFiles as $sourceFile) {
            $baseName = pathinfo($sourceFile, PATHINFO_FILENAME);
            $executable = normalize_path($exeDir . '/' . $baseName);
            $compileLog = normalize_path($logDir . '/' . $baseName . '.compile.log');
            $runLog = normalize_path($logDir . '/' . $baseName . '.run.log');

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
                'relative_compile_log' => normalize_path('./build/' . $target . '/log/' . $baseName . '.compile.log'),
                'relative_run_log' => normalize_path('./build/' . $target . '/log/' . $baseName . '.run.log'),
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
 * @return array<string, array{total:int, compile_success:int, run_success:int}>
 */
function initialize_target_stats(array $targetTotals): array
{
    $stats = [];

    foreach ($targetTotals as $target => $total) {
        $stats[$target] = [
            'total' => $total,
            'compile_success' => 0,
            'run_success' => 0,
        ];
    }

    return $stats;
}

/**
 * @param array<string, array{total:int, compile_success:int, run_success:int}> $targetStats
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
}

/**
 * @param array<string, array{total:int, compile_success:int, run_success:int}> $targetStats
 * @return void
 */
function print_target_summaries(array $targetStats): void
{
    foreach ($targetStats as $target => $stats) {
        echo '============================================================' . PHP_EOL;
        echo 'Target summary: ' . $target . PHP_EOL;
        echo '============================================================' . PHP_EOL;
        echo 'Total programs  : ' . $stats['total'] . PHP_EOL;
        echo 'Compile success : ' . $stats['compile_success'] . PHP_EOL;
        echo 'Run success     : ' . $stats['run_success'] . PHP_EOL;
        echo 'Compile failed  : ' . ($stats['total'] - $stats['compile_success']) . PHP_EOL;
        echo 'Run failed      : ' . ($stats['compile_success'] - $stats['run_success']) . PHP_EOL;
        echo PHP_EOL;
    }
}

/**
 * @param array<string, array{total:int, compile_success:int, run_success:int}> $targetStats
 * @param list<string> $targets
 * @param list<array{target:string, name:string, stage:string}> $failedPrograms
 * @return int
 */
function print_overall_summary(array $targetStats, array $targets, array $failedPrograms): int
{
    $grandTotalCount = 0;
    $grandCompileSuccessCount = 0;
    $grandRunSuccessCount = 0;

    foreach ($targetStats as $stats) {
        $grandTotalCount += $stats['total'];
        $grandCompileSuccessCount += $stats['compile_success'];
        $grandRunSuccessCount += $stats['run_success'];
    }

    echo '############################################################' . PHP_EOL;
    echo 'Overall summary' . PHP_EOL;
    echo '############################################################' . PHP_EOL;
    echo 'Targets         : ' . implode(', ', $targets) . PHP_EOL;
    echo 'Total programs  : ' . $grandTotalCount . PHP_EOL;
    echo 'Compile success : ' . $grandCompileSuccessCount . PHP_EOL;
    echo 'Run success     : ' . $grandRunSuccessCount . PHP_EOL;
    echo 'Compile failed  : ' . ($grandTotalCount - $grandCompileSuccessCount) . PHP_EOL;
    echo 'Run failed      : ' . ($grandCompileSuccessCount - $grandRunSuccessCount) . PHP_EOL;

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
 * @param array<string, array{total:int, compile_success:int, run_success:int}> $targetStats
 * @param list<array{target:string, name:string, stage:string}> $failedPrograms
 * @return array{target_stats:array<string, array{total:int, compile_success:int, run_success:int}>, failed_programs:list<array{target:string, name:string, stage:string}>}
 */
function run_tasks_in_parallel(
    array $tasks,
    array $targetStats,
    array $failedPrograms,
    int $jobs,
    string $scriptDir
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
            $started = start_compile_process($task, $scriptDir);

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
                write_text_file($task['compile_log'], $result['output']);
                $compileOk = ($result['exit_code'] === 0);

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

                $started = start_run_process($task, $scriptDir);
                $remaining[] = [
                    'task' => $task,
                    'stage' => $started['stage'],
                    'process' => $started['process'],
                    'pipes' => $started['pipes'],
                    'command_line' => $started['command_line'],
                ];
                continue;
            }

            write_text_file($task['run_log'], $result['output']);
            $runOk = ($result['exit_code'] === 0);

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

if (find_executable('g++') === null) {
    fail('g++ was not found in PATH.');
}

$parsed = parse_arguments($argv);
$jobs = $parsed['jobs'];
$targets = $parsed['targets'];

foreach ($targets as $target) {
    if (!is_supported_target($target)) {
        fail('Unsupported target directory: ' . $target);
    }
}

$taskBundle = build_tasks($targets, $scriptDir, $projectRoot);
$tasks = $taskBundle['tasks'];
$targetTotals = $taskBundle['target_totals'];

$targetStats = initialize_target_stats($targetTotals);
$failedPrograms = [];

echo '############################################################' . PHP_EOL;
echo 'Parallel test runner' . PHP_EOL;
echo '############################################################' . PHP_EOL;
echo 'Targets         : ' . implode(', ', $targets) . PHP_EOL;
echo 'Jobs            : ' . $jobs . PHP_EOL;
echo 'Total programs  : ' . count($tasks) . PHP_EOL;
echo PHP_EOL;

print_target_headers($targetStats);

$result = run_tasks_in_parallel(
    $tasks,
    $targetStats,
    $failedPrograms,
    $jobs,
    $scriptDir
);

$targetStats = $result['target_stats'];
$failedPrograms = $result['failed_programs'];

print_target_summaries($targetStats);

exit(print_overall_summary($targetStats, $targets, $failedPrograms));
