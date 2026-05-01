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
 *   cxxflags:list<string>,
 *   ldflags:list<string>,
 *   jobs:int,
 *   tcltk_cflags:list<string>,
 *   tcltk_libs:list<string>,
 *   targets:list<string>
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
    $jobs = detect_default_jobs();
    $targets = [];

    foreach (array_slice($argv, 1) as $arg) {
        if (str_starts_with($arg, '--cxx=')) {
            $cxx = substr($arg, strlen('--cxx='));
            if (trim($cxx) === '') {
                fail('Invalid --cxx value.');
            }
            continue;
        }

        if (str_starts_with($arg, '--cxxflags=')) {
            $cxxflags = split_command_fragment(substr($arg, strlen('--cxxflags=')));
            continue;
        }

        if (str_starts_with($arg, '--ldflags=')) {
            $ldflags = split_command_fragment(substr($arg, strlen('--ldflags=')));
            continue;
        }

        if (str_starts_with($arg, '--jobs=')) {
            $jobs = parse_jobs_option(substr($arg, strlen('--jobs=')));
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

        $targets[] = $arg;
    }

    return [
        'cxx' => $cxx,
        'cxxflags' => $cxxflags,
        'ldflags' => $ldflags,
        'jobs' => $jobs,
        'tcltk_cflags' => $tcltkCflags,
        'tcltk_libs' => $tcltkLibs,
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
 * @param array{cflags:list<string>, libs:list<string>, source:string} $candidate
 * @return array{available:bool, output:string, command_line:string}
 */
function probe_tcltk_candidate(array $options, array $candidate, string $scriptDir): array
{
    $probeDir = normalize_path($scriptDir . '/build/probe');
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
 * @return array<string, array<string, mixed>>
 */
function detect_feature_options(array $options, string $scriptDir): array
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
    ];

    $lastOutput = '';
    $lastCommandLine = '';
    foreach (tcltk_option_candidates($options) as $candidate) {
        $probe = probe_tcltk_candidate($options, $candidate, $scriptDir);
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
function build_tasks(array $targets, string $scriptDir, string $projectRoot, array $featureOptions): array
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
        $workBaseDir = normalize_path($buildDir . '/work');

        ensure_directory($buildDir);
        ensure_directory($exeDir);
        ensure_directory($logDir);
        ensure_directory($workBaseDir);

        $targetTotals[$target] = count($sourceFiles);

        foreach ($sourceFiles as $sourceFile) {
            $baseName = pathinfo($sourceFile, PATHINFO_FILENAME);
            $executable = normalize_path($exeDir . '/' . $baseName);
            $compileLog = normalize_path($logDir . '/' . $baseName . '.compile.log');
            $runLog = normalize_path($logDir . '/' . $baseName . '.run.log');
            $workDir = normalize_path($workBaseDir . '/' . $baseName);
            $features = detect_source_features($sourceFile);

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
                'relative_compile_log' => normalize_path('./build/' . $target . '/log/' . $baseName . '.compile.log'),
                'relative_run_log' => normalize_path('./build/' . $target . '/log/' . $baseName . '.run.log'),
                'relative_work_dir' => normalize_path('./build/' . $target . '/work/' . $baseName),
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
 * @return array<string, array{total:int, skipped:int, compile_success:int, run_success:int}>
 */
function initialize_target_stats(array $targetTotals): array
{
    $stats = [];

    foreach ($targetTotals as $target => $total) {
        $stats[$target] = [
            'total' => $total,
            'skipped' => 0,
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
 * @param array<string, array{total:int, skipped:int, compile_success:int, run_success:int}> $targetStats
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
 * @param array<string, array{total:int, skipped:int, compile_success:int, run_success:int}> $targetStats
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
        echo 'Compile success : ' . $stats['compile_success'] . PHP_EOL;
        echo 'Run success     : ' . $stats['run_success'] . PHP_EOL;
        echo 'Compile failed  : ' . ($active - $stats['compile_success']) . PHP_EOL;
        echo 'Run failed      : ' . ($stats['compile_success'] - $stats['run_success']) . PHP_EOL;
        echo PHP_EOL;
    }
}

/**
 * @param array<string, array{total:int, skipped:int, compile_success:int, run_success:int}> $targetStats
 * @param list<string> $targets
 * @param list<array{target:string, name:string, stage:string}> $failedPrograms
 * @param list<array{target:string, name:string, reason:string}> $skippedPrograms
 * @return int
 */
function print_overall_summary(array $targetStats, array $targets, array $failedPrograms, array $skippedPrograms): int
{
    $grandTotalCount = 0;
    $grandSkippedCount = 0;
    $grandCompileSuccessCount = 0;
    $grandRunSuccessCount = 0;

    foreach ($targetStats as $stats) {
        $grandTotalCount += $stats['total'];
        $grandSkippedCount += $stats['skipped'];
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
    echo 'Compile success : ' . $grandCompileSuccessCount . PHP_EOL;
    echo 'Run success     : ' . $grandRunSuccessCount . PHP_EOL;
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
 * @param array<string, array{total:int, skipped:int, compile_success:int, run_success:int}> $targetStats
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
$options = [
    'cxx' => $parsed['cxx'],
    'cxxflags' => $parsed['cxxflags'],
    'ldflags' => $parsed['ldflags'],
    'tcltk_cflags' => $parsed['tcltk_cflags'],
    'tcltk_libs' => $parsed['tcltk_libs'],
];
$jobs = $parsed['jobs'];
$targets = $parsed['targets'];

if (find_executable($options['cxx']) === null) {
    fail($options['cxx'] . ' was not found in PATH.');
}

foreach ($targets as $target) {
    if (!is_supported_target($target)) {
        fail('Unsupported target directory: ' . $target);
    }
}

$featureOptions = detect_feature_options($options, $scriptDir);
$taskBundle = build_tasks($targets, $scriptDir, $projectRoot, $featureOptions);
$tasks = $taskBundle['tasks'];
$targetTotals = $taskBundle['target_totals'];

$targetStats = initialize_target_stats($targetTotals);
$failedPrograms = [];
$skippedPrograms = [];

echo '############################################################' . PHP_EOL;
echo 'Parallel test runner' . PHP_EOL;
echo '############################################################' . PHP_EOL;
echo 'Compiler        : ' . $options['cxx'] . PHP_EOL;
echo 'Targets         : ' . implode(', ', $targets) . PHP_EOL;
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
