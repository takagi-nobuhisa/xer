<?php
declare(strict_types=1);

/**
 * @file php/package_xer.php
 * @brief Creates a ZIP archive of the xer project tree.
 *
 * Usage:
 *   php package_xer.php
 *   php package_xer.php --output=../xer.zip
 *
 * The archive contains the project root directory itself as xer/.
 * The following directories are excluded:
 *
 * - xer/.git/
 * - xer/php/build/
 * - xer/vcpkg_installed/
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

function absolute_output_path(string $path): string
{
    $path = normalize_path($path);
    if (str_starts_with($path, '/')) {
        return $path;
    }

    return normalize_path(getcwd() . '/' . $path);
}

function ensure_parent_directory(string $path): void
{
    $directory = dirname($path);
    if (is_dir($directory)) {
        return;
    }

    if (!mkdir($directory, 0777, true) && !is_dir($directory)) {
        fail('Failed to create directory: ' . $directory);
    }
}

/**
 * @param array<int, string> $argv
 * @return array{project_root:string, output:string}
 */
function parse_arguments(array $argv): array
{
    $phpDir = absolute_path(__DIR__);
    $projectRoot = absolute_path($phpDir . '/..');
    $output = normalize_path(dirname($projectRoot) . '/xer.zip');

    for ($i = 1; $i < count($argv); ++$i) {
        $arg = $argv[$i];

        if ($arg === '--help' || $arg === '-h') {
            echo "Usage:\n";
            echo "  php package_xer.php\n";
            echo "  php package_xer.php --output=../xer.zip\n";
            exit(EXIT_SUCCESS_CODE);
        }

        if (str_starts_with($arg, '--output=')) {
            $value = substr($arg, strlen('--output='));
            if ($value === '') {
                fail('The --output option requires a path.');
            }
            $output = absolute_output_path($value);
            continue;
        }

        fail('Unknown option: ' . $arg);
    }

    return [
        'project_root' => $projectRoot,
        'output' => $output,
    ];
}

function should_exclude(string $relativePath): bool
{
    $path = trim(normalize_path($relativePath), '/');

    return $path === '.git'
        || str_starts_with($path, '.git/')
        || $path === 'php/build'
        || str_starts_with($path, 'php/build/')
        || $path === 'vcpkg_installed'
        || str_starts_with($path, 'vcpkg_installed/');
}

/**
 * @return array{directories:int, files:int}
 */
function count_package_entries(string $projectRoot, string $output): array
{
    $fileCount = 0;
    $directoryCount = 0;
    $output = normalize_path($output);

    $iterator = new RecursiveIteratorIterator(
        new RecursiveCallbackFilterIterator(
            new RecursiveDirectoryIterator(
                $projectRoot,
                FilesystemIterator::SKIP_DOTS | FilesystemIterator::CURRENT_AS_FILEINFO
            ),
            function (SplFileInfo $current, string $key, RecursiveDirectoryIterator $iterator) use ($projectRoot): bool {
                unset($key, $iterator);

                $path = normalize_path($current->getPathname());
                $relativePath = ltrim(substr($path, strlen($projectRoot)), '/');

                return !should_exclude($relativePath);
            }
        ),
        RecursiveIteratorIterator::SELF_FIRST
    );

    /** @var SplFileInfo $entry */
    foreach ($iterator as $entry) {
        $path = normalize_path($entry->getPathname());
        if ($path === $output) {
            continue;
        }

        $relativePath = ltrim(substr($path, strlen($projectRoot)), '/');
        if ($relativePath === '' || should_exclude($relativePath)) {
            continue;
        }

        if ($entry->isDir()) {
            ++$directoryCount;
        } elseif ($entry->isFile()) {
            ++$fileCount;
        }
    }

    return [
        'directories' => $directoryCount,
        'files' => $fileCount,
    ];
}

function add_entries_with_zip_archive(ZipArchive $zip, string $projectRoot, string $output): void
{
    $baseName = basename($projectRoot);
    $output = normalize_path($output);

    $iterator = new RecursiveIteratorIterator(
        new RecursiveCallbackFilterIterator(
            new RecursiveDirectoryIterator(
                $projectRoot,
                FilesystemIterator::SKIP_DOTS | FilesystemIterator::CURRENT_AS_FILEINFO
            ),
            function (SplFileInfo $current, string $key, RecursiveDirectoryIterator $iterator) use ($projectRoot): bool {
                unset($key, $iterator);

                $path = normalize_path($current->getPathname());
                $relativePath = ltrim(substr($path, strlen($projectRoot)), '/');

                return !should_exclude($relativePath);
            }
        ),
        RecursiveIteratorIterator::SELF_FIRST
    );

    /** @var SplFileInfo $entry */
    foreach ($iterator as $entry) {
        $path = normalize_path($entry->getPathname());
        if ($path === $output) {
            continue;
        }

        $relativePath = ltrim(substr($path, strlen($projectRoot)), '/');
        if ($relativePath === '' || should_exclude($relativePath)) {
            continue;
        }

        $archivePath = normalize_path($baseName . '/' . $relativePath);

        if ($entry->isDir()) {
            if (!$zip->addEmptyDir($archivePath)) {
                $zip->close();
                fail('Failed to add directory to ZIP: ' . $archivePath);
            }
            continue;
        }

        if (!$entry->isFile()) {
            continue;
        }

        if (!$zip->addFile($path, $archivePath)) {
            $zip->close();
            fail('Failed to add file to ZIP: ' . $archivePath);
        }
    }
}

function create_zip_with_zip_archive(string $projectRoot, string $output): void
{
    $zip = new ZipArchive();
    $result = $zip->open($output, ZipArchive::CREATE | ZipArchive::OVERWRITE);
    if ($result !== true) {
        fail('Failed to open ZIP file: ' . $output . ' (code: ' . (string)$result . ')');
    }

    add_entries_with_zip_archive($zip, $projectRoot, $output);

    if (!$zip->close()) {
        fail('Failed to close ZIP file: ' . $output);
    }
}

function find_zip_command(): ?string
{
    $command = trim((string)shell_exec('command -v zip 2>/dev/null'));
    if ($command === '') {
        return null;
    }

    return $command;
}

function create_zip_with_command(string $projectRoot, string $output): void
{
    $zipCommand = find_zip_command();
    if ($zipCommand === null) {
        fail('Neither ZipArchive nor the zip command is available. Please install the PHP zip extension or the zip command.');
    }

    $parentDir = dirname($projectRoot);
    $baseName = basename($projectRoot);

    $command = 'cd ' . escapeshellarg($parentDir)
        . ' && ' . escapeshellarg($zipCommand)
        . ' -rq ' . escapeshellarg($output)
        . ' ' . escapeshellarg($baseName)
        . ' -x ' . escapeshellarg($baseName . '/.git/*')
        . ' -x ' . escapeshellarg($baseName . '/php/build/*')
        . ' -x ' . escapeshellarg($baseName . '/vcpkg_installed/*');

    $lines = [];
    $exitCode = 0;
    exec($command . ' 2>&1', $lines, $exitCode);
    if ($exitCode !== 0) {
        foreach ($lines as $line) {
            fwrite(STDERR, $line . PHP_EOL);
        }
        fail('Failed to create ZIP file with the zip command: ' . $output);
    }
}

function create_zip(string $projectRoot, string $output): int
{
    ensure_parent_directory($output);

    if (file_exists($output) && !unlink($output)) {
        fail('Failed to remove existing ZIP file: ' . $output);
    }

    if (class_exists(ZipArchive::class)) {
        create_zip_with_zip_archive($projectRoot, $output);
        $method = 'ZipArchive';
    } else {
        create_zip_with_command($projectRoot, $output);
        $method = 'zip command';
    }

    $counts = count_package_entries($projectRoot, $output);

    echo 'Project root : ' . $projectRoot . PHP_EOL;
    echo 'Output       : ' . $output . PHP_EOL;
    echo 'Method       : ' . $method . PHP_EOL;
    echo 'Directories  : ' . $counts['directories'] . PHP_EOL;
    echo 'Files        : ' . $counts['files'] . PHP_EOL;

    return EXIT_SUCCESS_CODE;
}

/**
 * @param array<int, string> $argv
 */
function main(array $argv): int
{
    $config = parse_arguments($argv);

    return create_zip($config['project_root'], $config['output']);
}

exit(main($argv));
