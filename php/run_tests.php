<?php

declare(strict_types=1);

/**
 * @file run_tests.php
 * @brief Compile and run all test programs under ../tests.
 *
 * This script assumes that it is executed in the php directory.
 * Each test source file is compiled by the following fixed command:
 *
 *   g++ -std=gnu++23 -I.. <source file>
 *
 * The output executable and logs are written under ./build/tests.
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
 * @return list<string>
 */
function find_test_sources(string $testsDir): array
{
    $pattern = $testsDir . '/*.cpp';
    $files = glob($pattern);
    if ($files === false) {
        return [];
    }

    $files = array_map('normalize_path', $files);
    sort($files, SORT_STRING);
    return array_values($files);
}

/**
 * @return string
 */
function shell_quote_path(string $path): string
{
    return escapeshellarg($path);
}

/**
 * @param list<string> $command
 * @return array{output:string, exit_code:int}
 */
function run_command(array $command): array
{
    $parts = array_map('shell_quote_path', $command);
    $commandLine = implode(' ', $parts) . ' 2>&1';

    $outputLines = [];
    $exitCode = 0;
    exec($commandLine, $outputLines, $exitCode);

    return [
        'output' => implode(PHP_EOL, $outputLines),
        'exit_code' => $exitCode,
    ];
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

$scriptDir = normalize_path(__DIR__);
$projectRoot = normalize_path(dirname($scriptDir));
$testsDir = normalize_path($projectRoot . '/tests');
$buildDir = normalize_path($scriptDir . '/build/tests');
$exeDir = normalize_path($buildDir . '/bin');
$logDir = normalize_path($buildDir . '/log');

if (normalize_path(getcwd() ?: '') !== $scriptDir) {
    fail('This script must be executed in the php directory.');
}

if (!is_dir($testsDir)) {
    fail('Tests directory not found: ' . $testsDir);
}

$testSources = find_test_sources($testsDir);
if ($testSources === []) {
    fail('No test source files were found in: ' . $testsDir);
}

ensure_directory($buildDir);
ensure_directory($exeDir);
ensure_directory($logDir);

$totalCount = 0;
$compileSuccessCount = 0;
$runSuccessCount = 0;
$failedTests = [];

foreach ($testSources as $sourceFile) {
    ++$totalCount;

    $baseName = pathinfo($sourceFile, PATHINFO_FILENAME);
    $executable = normalize_path($exeDir . '/' . $baseName);
    $compileLog = normalize_path($logDir . '/' . $baseName . '.compile.log');
    $runLog = normalize_path($logDir . '/' . $baseName . '.run.log');

    if (DIRECTORY_SEPARATOR === '\\') {
        $executable .= '.exe';
    }

    $relativeSource = normalize_path('../tests/' . basename($sourceFile));

    echo '============================================================' . PHP_EOL;
    echo '[TEST] ' . $baseName . PHP_EOL;
    echo '[SRC ] ' . $relativeSource . PHP_EOL;

    $compileCommand = [
        'g++',
        '-std=gnu++23',
        '-I..',
        $sourceFile,
        '-o',
        $executable,
    ];

    $compileResult = run_command($compileCommand);
    write_text_file($compileLog, $compileResult['output']);

    $compileOk = ($compileResult['exit_code'] === 0);
    echo '[COMP] ' . format_result_label($compileOk) . PHP_EOL;
    echo '[CLOG] ' . normalize_path('./build/tests/log/' . $baseName . '.compile.log') . PHP_EOL;

    if (!$compileOk) {
        $failedTests[] = [
            'name' => $baseName,
            'stage' => 'compile',
        ];
        continue;
    }

    ++$compileSuccessCount;

    $runCommand = [$executable];
    $runResult = run_command($runCommand);
    write_text_file($runLog, $runResult['output']);

    $runOk = ($runResult['exit_code'] === 0);
    echo '[RUN ] ' . format_result_label($runOk) . PHP_EOL;
    echo '[RLOG] ' . normalize_path('./build/tests/log/' . $baseName . '.run.log') . PHP_EOL;

    if (!$runOk) {
        $failedTests[] = [
            'name' => $baseName,
            'stage' => 'run',
        ];
        continue;
    }

    ++$runSuccessCount;
}

echo '============================================================' . PHP_EOL;
echo 'Summary' . PHP_EOL;
echo '============================================================' . PHP_EOL;
echo 'Total tests      : ' . $totalCount . PHP_EOL;
echo 'Compile success  : ' . $compileSuccessCount . PHP_EOL;
echo 'Run success      : ' . $runSuccessCount . PHP_EOL;
echo 'Compile failed   : ' . ($totalCount - $compileSuccessCount) . PHP_EOL;
echo 'Run failed       : ' . ($compileSuccessCount - $runSuccessCount) . PHP_EOL;

if ($failedTests !== []) {
    echo PHP_EOL;
    echo 'Failed tests' . PHP_EOL;
    echo '------------------------------------------------------------' . PHP_EOL;
    foreach ($failedTests as $failedTest) {
        echo $failedTest['name'] . ' [' . $failedTest['stage'] . ']' . PHP_EOL;
    }
    exit(EXIT_FAILURE_CODE);
}

exit(EXIT_SUCCESS_CODE);
