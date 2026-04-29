#!/usr/bin/env php
<?php

declare(strict_types=1);

const UTF8_BOM = "\xEF\xBB\xBF";

/**
 * Normalize UTF-8 BOM and line endings for text files under a directory.
 *
 * Usage:
 *   php normalize_text_files.php [options]
 *
 * Options:
 *   --root=DIR          Root directory. Default: current directory.
 *   --bom=add          Add UTF-8 BOM when missing.
 *   --bom=remove       Remove UTF-8 BOM when present.
 *   --bom=keep         Do not change BOM. Default.
 *   --newline=lf       Normalize CRLF and CR to LF. Default.
 *   --newline=keep     Do not change line endings.
 *   --dry-run          Show changes without writing files.
 *   --verbose          Show skipped and unchanged files.
 *   --help             Show this help.
 *
 * Notes:
 *   - This script treats only UTF-8 BOM as BOM.
 *   - Files that appear to be binary are skipped.
 *   - Symbolic links are skipped.
 *   - Common generated or repository-control directories are skipped.
 */
function main(array $argv): int
{
    $options = parse_options($argv);

    if ($options['help']) {
        print_help();
        return 0;
    }

    $root = realpath($options['root']);
    if ($root === false || !is_dir($root)) {
        fwrite(STDERR, "error: root directory does not exist: {$options['root']}\n");
        return 2;
    }

    $stats = [
        'checked' => 0,
        'changed' => 0,
        'unchanged' => 0,
        'skipped' => 0,
        'errors' => 0,
    ];

    foreach (iterate_files($root) as $path) {
        process_file($path, $options, $stats);
    }

    echo "checked:   {$stats['checked']}\n";
    echo "changed:   {$stats['changed']}\n";
    echo "unchanged: {$stats['unchanged']}\n";
    echo "skipped:   {$stats['skipped']}\n";
    echo "errors:    {$stats['errors']}\n";

    return $stats['errors'] === 0 ? 0 : 1;
}

function parse_options(array $argv): array
{
    $options = [
        'root' => '.',
        'bom' => 'keep',
        'newline' => 'lf',
        'dry_run' => false,
        'verbose' => false,
        'help' => false,
    ];

    for ($i = 1; $i < count($argv); ++$i) {
        $arg = $argv[$i];

        if ($arg === '--help' || $arg === '-h') {
            $options['help'] = true;
            continue;
        }

        if ($arg === '--dry-run') {
            $options['dry_run'] = true;
            continue;
        }

        if ($arg === '--verbose') {
            $options['verbose'] = true;
            continue;
        }

        if (str_starts_with($arg, '--root=')) {
            $options['root'] = substr($arg, strlen('--root='));
            continue;
        }

        if (str_starts_with($arg, '--bom=')) {
            $value = substr($arg, strlen('--bom='));
            if (!in_array($value, ['add', 'remove', 'keep'], true)) {
                fwrite(STDERR, "error: invalid --bom value: {$value}\n");
                exit(2);
            }
            $options['bom'] = $value;
            continue;
        }

        if (str_starts_with($arg, '--newline=')) {
            $value = substr($arg, strlen('--newline='));
            if (!in_array($value, ['lf', 'keep'], true)) {
                fwrite(STDERR, "error: invalid --newline value: {$value}\n");
                exit(2);
            }
            $options['newline'] = $value;
            continue;
        }

        fwrite(STDERR, "error: unknown option: {$arg}\n");
        exit(2);
    }

    return $options;
}

function print_help(): void
{
    echo <<<'HELP'
Normalize UTF-8 BOM and line endings for text files.

Usage:
  php normalize_text_files.php [options]

Options:
  --root=DIR          Root directory. Default: current directory.
  --bom=add          Add UTF-8 BOM when missing.
  --bom=remove       Remove UTF-8 BOM when present.
  --bom=keep         Do not change BOM. Default.
  --newline=lf       Normalize CRLF and CR to LF. Default.
  --newline=keep     Do not change line endings.
  --dry-run          Show changes without writing files.
  --verbose          Show skipped and unchanged files.
  --help             Show this help.

Examples:
  php normalize_text_files.php --bom=add
  php normalize_text_files.php --bom=remove
  php normalize_text_files.php --bom=add --dry-run --verbose
  php normalize_text_files.php --root=.. --bom=remove --newline=lf

HELP;
}

/**
 * @return Generator<int, string>
 */
function iterate_files(string $root): Generator
{
    $directory = new RecursiveDirectoryIterator(
        $root,
        FilesystemIterator::SKIP_DOTS | FilesystemIterator::CURRENT_AS_FILEINFO
    );

    $filter = new RecursiveCallbackFilterIterator(
        $directory,
        static function (SplFileInfo $current): bool {
            $name = $current->getFilename();

            if ($current->isLink()) {
                return false;
            }

            if ($current->isDir()) {
                return !in_array($name, excluded_directory_names(), true);
            }

            return $current->isFile();
        }
    );

    $iterator = new RecursiveIteratorIterator($filter);

    foreach ($iterator as $file) {
        if ($file instanceof SplFileInfo) {
            yield $file->getPathname();
        }
    }
}

function excluded_directory_names(): array
{
    return [
        '.git',
        '.hg',
        '.svn',
        '.idea',
        '.vscode',
        'node_modules',
        'vendor',
        'build',
        'cmake-build-debug',
        'cmake-build-release',
    ];
}

function process_file(string $path, array $options, array &$stats): void
{
    ++$stats['checked'];

    $content = @file_get_contents($path);
    if ($content === false) {
        ++$stats['errors'];
        fwrite(STDERR, "error: failed to read: {$path}\n");
        return;
    }

    if (is_probably_binary($content)) {
        ++$stats['skipped'];
        if ($options['verbose']) {
            echo "skip binary: {$path}\n";
        }
        return;
    }

    $original = $content;
    $changes = [];

    if ($options['bom'] === 'add') {
        if (!has_utf8_bom($content)) {
            $content = UTF8_BOM . $content;
            $changes[] = 'bom:add';
        }
    } elseif ($options['bom'] === 'remove') {
        if (has_utf8_bom($content)) {
            $content = substr($content, strlen(UTF8_BOM));
            $changes[] = 'bom:remove';
        }
    }

    if ($options['newline'] === 'lf') {
        $normalized = normalize_lf($content);
        if ($normalized !== $content) {
            $content = $normalized;
            $changes[] = 'newline:lf';
        }
    }

    if ($content === $original) {
        ++$stats['unchanged'];
        if ($options['verbose']) {
            echo "unchanged: {$path}\n";
        }
        return;
    }

    ++$stats['changed'];
    echo ($options['dry_run'] ? 'would update: ' : 'update: ') . $path;
    echo ' [' . implode(', ', $changes) . "]\n";

    if ($options['dry_run']) {
        return;
    }

    if (!write_file_atomically($path, $content)) {
        ++$stats['errors'];
        fwrite(STDERR, "error: failed to write: {$path}\n");
    }
}

function has_utf8_bom(string $content): bool
{
    return str_starts_with($content, UTF8_BOM);
}

function normalize_lf(string $content): string
{
    $content = str_replace("\r\n", "\n", $content);
    return str_replace("\r", "\n", $content);
}

function is_probably_binary(string $content): bool
{
    if ($content === '') {
        return false;
    }

    $sample = substr($content, 0, 8192);

    return str_contains($sample, "\0");
}

function write_file_atomically(string $path, string $content): bool
{
    $dir = dirname($path);
    $base = basename($path);
    $tmp = tempnam($dir, ".{$base}.");

    if ($tmp === false) {
        return false;
    }

    $mode = @fileperms($path);

    $ok = @file_put_contents($tmp, $content, LOCK_EX) !== false;

    if ($ok && $mode !== false) {
        @chmod($tmp, $mode & 0777);
    }

    if ($ok) {
        $ok = @rename($tmp, $path);
    }

    if (!$ok) {
        @unlink($tmp);
    }

    return $ok;
}

exit(main($argv));
