<?php

declare(strict_types=1);

/**
 * @file php/update_project.php
 * @brief Synchronizes generated project files with project_config.php.
 *
 * Usage:
 *   php update_project.php
 *   php update_project.php --no-reference
 *   php update_project.php --check
 *
 * This script is executed from the php/ directory.
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

function normalize_path(string $path): string
{
    return str_replace('\\', '/', $path);
}

function absolute_path(string $path): string
{
    $real = realpath($path);
    if ($real === false) {
        fail('Path does not exist: ' . $path);
    }

    return normalize_path($real);
}

function read_text_file(string $path): string
{
    $content = file_get_contents($path);
    if ($content === false) {
        fail('Failed to read file: ' . $path);
    }

    return $content;
}

function write_text_file(string $path, string $content): void
{
    if (file_put_contents($path, $content) === false) {
        fail('Failed to write file: ' . $path);
    }
}

/**
 * @param array<int, string> $argv
 * @return array{check:bool, reference:bool}
 */
function parse_arguments(array $argv): array
{
    $check = false;
    $reference = true;

    for ($i = 1; $i < count($argv); ++$i) {
        $arg = $argv[$i];

        if ($arg === '--help' || $arg === '-h') {
            echo "Usage:\n";
            echo "  php update_project.php\n";
            echo "  php update_project.php --no-reference\n";
            echo "  php update_project.php --check\n";
            exit(EXIT_SUCCESS_CODE);
        }

        if ($arg === '--check') {
            $check = true;
            continue;
        }

        if ($arg === '--no-reference') {
            $reference = false;
            continue;
        }

        fail('Unknown option: ' . $arg);
    }

    return [
        'check' => $check,
        'reference' => $reference,
    ];
}

/**
 * @return array{major:int, minor:int, patch:int, suffix:string, string:string, display:string}
 */
function load_version_config(string $configPath): array
{
    $config = require $configPath;
    if (!is_array($config) || !isset($config['version']) || !is_array($config['version'])) {
        fail('Invalid project configuration: missing version settings.');
    }

    $version = $config['version'];
    foreach (['major', 'minor', 'patch'] as $key) {
        if (!isset($version[$key]) || !is_int($version[$key]) || $version[$key] < 0) {
            fail('Invalid version setting: ' . $key);
        }
    }

    $suffix = $version['suffix'] ?? '';
    if (!is_string($suffix)) {
        fail('Invalid version setting: suffix');
    }
    if ($suffix !== '' && !preg_match('/^[A-Za-z0-9._-]+$/', $suffix)) {
        fail('Invalid version suffix: ' . $suffix);
    }

    $versionString = $version['major'] . '.' . $version['minor'] . '.' . $version['patch'] . $suffix;

    return [
        'major' => $version['major'],
        'minor' => $version['minor'],
        'patch' => $version['patch'],
        'suffix' => $suffix,
        'string' => $versionString,
        'display' => 'v' . $versionString,
    ];
}

/**
 * @param array{major:int, minor:int, patch:int, suffix:string, string:string, display:string} $version
 */
function build_version_header(array $version): string
{
    $suffix = addcslashes($version['suffix'], "\\\"");
    $versionString = addcslashes($version['string'], "\\\"");

    return "\xEF\xBB\xBF" . <<<CPP
#ifndef XER_VERSION_H_
#define XER_VERSION_H_

#include <string_view>

/**
 * @file
 * @brief Version information for XER.
 */

/**
 * @brief Major version number of XER.
 */
#define XER_VERSION_MAJOR {$version['major']}

/**
 * @brief Minor version number of XER.
 */
#define XER_VERSION_MINOR {$version['minor']}

/**
 * @brief Patch version number of XER.
 */
#define XER_VERSION_PATCH {$version['patch']}

/**
 * @brief Version suffix string of XER.
 *
 * This macro holds the non-numeric suffix part of the version string.
 */
#define XER_VERSION_SUFFIX "{$suffix}"

/**
 * @brief Full version string of XER.
 *
 * This macro holds the complete version string.
 */
#define XER_VERSION_STRING "{$versionString}"

namespace xer {

/**
 * @brief Major version number of XER.
 */
inline constexpr int version_major = XER_VERSION_MAJOR;

/**
 * @brief Minor version number of XER.
 */
inline constexpr int version_minor = XER_VERSION_MINOR;

/**
 * @brief Patch version number of XER.
 */
inline constexpr int version_patch = XER_VERSION_PATCH;

/**
 * @brief Version suffix string of XER.
 *
 * This constant holds the non-numeric suffix part of the version string.
 */
inline constexpr std::string_view version_suffix = XER_VERSION_SUFFIX;

/**
 * @brief Full version string of XER.
 *
 * This constant holds the complete version string.
 */
inline constexpr std::string_view version_string = XER_VERSION_STRING;

} // namespace xer

#endif
CPP;
}

/**
 * @param array{major:int, minor:int, patch:int, suffix:string, string:string, display:string} $version
 */
function update_reference_translation_terms(string $path, array $version): string
{
    $content = read_text_file($path);
    $updated = preg_replace(
        '/^対象バージョン:\s*\*\*v[^*]+\*\*/m',
        '対象バージョン: **' . $version['display'] . '**',
        $content,
        1,
        $count
    );

    if ($updated === null) {
        fail('Failed to update target version in: ' . $path);
    }
    if ($count !== 1) {
        fail('Target version line was not found in: ' . $path);
    }

    return $updated;
}

function report_file_status(string $path, string $expected, bool $check): bool
{
    $current = is_file($path) ? read_text_file($path) : null;
    $relative = normalize_path($path);

    if ($current === $expected) {
        fwrite(STDOUT, 'Up to date: ' . $relative . PHP_EOL);
        return false;
    }

    if ($check) {
        fwrite(STDOUT, 'Needs update: ' . $relative . PHP_EOL);
        return true;
    }

    write_text_file($path, $expected);
    fwrite(STDOUT, 'Updated: ' . $relative . PHP_EOL);
    return true;
}

function run_reference_manual_generator(string $phpDir): void
{
    $script = $phpDir . '/generate_reference_manual.php';
    if (!is_file($script)) {
        fail('Reference manual generator was not found: ' . $script);
    }

    $command = escapeshellarg(PHP_BINARY) . ' ' . escapeshellarg($script);
    $cwd = getcwd();
    if ($cwd === false) {
        fail('Failed to get current directory.');
    }

    if (!chdir($phpDir)) {
        fail('Failed to change directory: ' . $phpDir);
    }

    passthru($command, $exitCode);

    if (!chdir($cwd)) {
        fail('Failed to restore current directory: ' . $cwd);
    }

    if ($exitCode !== EXIT_SUCCESS_CODE) {
        fail('Reference manual generation failed.');
    }
}

$options = parse_arguments($argv);
$phpDir = absolute_path(__DIR__);
$projectRoot = absolute_path($phpDir . '/..');
$configPath = $phpDir . '/project_config.php';
$versionHeaderPath = $projectRoot . '/xer/version.h';
$translationTermsPath = $projectRoot . '/docs/reference_translation_terms.md';

$version = load_version_config($configPath);
$needsUpdate = false;

$needsUpdate = report_file_status(
    $versionHeaderPath,
    build_version_header($version),
    $options['check']
) || $needsUpdate;

$needsUpdate = report_file_status(
    $translationTermsPath,
    update_reference_translation_terms($translationTermsPath, $version),
    $options['check']
) || $needsUpdate;

if ($options['check']) {
    if ($needsUpdate) {
        fwrite(STDERR, 'Project files are not up to date.' . PHP_EOL);
        exit(EXIT_FAILURE_CODE);
    }

    fwrite(STDOUT, 'Project files are up to date.' . PHP_EOL);
    exit(EXIT_SUCCESS_CODE);
}

if ($options['reference']) {
    run_reference_manual_generator($phpDir);
} else {
    fwrite(STDOUT, "Skipped reference manual generation.\n");
}

fwrite(STDOUT, 'Project update completed for ' . $version['display'] . '.' . PHP_EOL);
