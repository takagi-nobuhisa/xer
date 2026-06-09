<?php

declare(strict_types=1);

final class Options
{
    public string $input;
    public string $output;
    public bool $recursive;
    public ?string $jar;
    public ?string $plantuml;
    public bool $verbose;

    public function __construct(
        string $input,
        string $output,
        bool $recursive,
        ?string $jar,
        ?string $plantuml,
        bool $verbose
    ) {
        $this->input = $input;
        $this->output = $output;
        $this->recursive = $recursive;
        $this->jar = $jar;
        $this->plantuml = $plantuml;
        $this->verbose = $verbose;
    }
}

function stderr(string $message): void
{
    fwrite(STDERR, $message . PHP_EOL);
}

function usage(): void
{
    $script = basename(__FILE__);
    echo <<<TXT
Usage:
  php {$script} --input <file-or-directory> --output <directory> [options]

Options:
  --input <path>         Input .puml file or directory.
  --output <path>        Output directory.
  --recursive            Search input directory recursively.
  --jar <path>           Path to plantuml.jar.
  --plantuml <command>   Path to plantuml executable or wrapper script.
  --verbose              Show executed commands and output paths.
  --help                 Show this help.

Notes:
  - Input files use the .puml extension.
  - Output files are always PNG images.
  - This script uses PlantUML pipe mode and writes PNG files by itself.
    It does not depend on PlantUML's -output path handling.
  - Specify either --jar or --plantuml.
  - If neither is specified, this script tries environment variables
    PLANTUML_JAR and PLANTUML_CMD in that order.
  - Directory structure under --input is preserved under --output.

Examples:
  php {$script} --plantuml plantuml --input ../docs/diagrams --output ../docs/images --recursive
  php {$script} --jar /path/to/plantuml.jar --input ../docs/diagrams --output ../docs/images --recursive

TXT;
}

function parse_options(): Options
{
    $opt = getopt('', [
        'input:',
        'output:',
        'recursive',
        'jar:',
        'plantuml:',
        'verbose',
        'help',
    ]);

    if (isset($opt['help'])) {
        usage();
        exit(0);
    }

    $input = isset($opt['input']) ? (string) $opt['input'] : '';
    $output = isset($opt['output']) ? (string) $opt['output'] : '';
    $recursive = isset($opt['recursive']);
    $jar = isset($opt['jar']) ? (string) $opt['jar'] : (getenv('PLANTUML_JAR') !== false ? (string) getenv('PLANTUML_JAR') : null);
    $plantuml = isset($opt['plantuml']) ? (string) $opt['plantuml'] : (getenv('PLANTUML_CMD') !== false ? (string) getenv('PLANTUML_CMD') : null);
    $verbose = isset($opt['verbose']);

    if ($input === '' || $output === '') {
        usage();
        throw new RuntimeException('--input and --output are required.');
    }

    if ($jar !== null && $jar !== '' && $plantuml !== null && $plantuml !== '') {
        throw new RuntimeException('Specify either --jar or --plantuml, not both.');
    }

    if (($jar === null || $jar === '') && ($plantuml === null || $plantuml === '')) {
        throw new RuntimeException('Specify --jar or --plantuml, or set PLANTUML_JAR / PLANTUML_CMD.');
    }

    return new Options(
        $input,
        $output,
        $recursive,
        ($jar === '' ? null : $jar),
        ($plantuml === '' ? null : $plantuml),
        $verbose
    );
}

function normalize_path(string $path): string
{
    return rtrim(str_replace(['/', '\\'], DIRECTORY_SEPARATOR, $path), DIRECTORY_SEPARATOR);
}

function absolute_path(string $path): string
{
    $path = normalize_path($path);

    if ($path === '') {
        throw new RuntimeException('Empty path is not allowed.');
    }

    if (DIRECTORY_SEPARATOR === '\\') {
        if (preg_match('/^[A-Za-z]:\\\\/', $path) === 1 || str_starts_with($path, '\\\\')) {
            return $path;
        }
    } elseif (str_starts_with($path, DIRECTORY_SEPARATOR)) {
        return $path;
    }

    return getcwd() . DIRECTORY_SEPARATOR . $path;
}

function is_puml_file(string $path): bool
{
    return strtolower((string) pathinfo($path, PATHINFO_EXTENSION)) === 'puml';
}

function collect_input_files(Options $options): array
{
    $input = normalize_path($options->input);
    if (!file_exists($input)) {
        throw new RuntimeException('Input path does not exist: ' . $input);
    }

    if (is_file($input)) {
        if (!is_puml_file($input)) {
            throw new RuntimeException('Input file is not a .puml file: ' . $input);
        }
        return [realpath($input) ?: $input];
    }

    if ($options->recursive) {
        $iterator = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($input, FilesystemIterator::SKIP_DOTS));
    } else {
        $iterator = new DirectoryIterator($input);
    }

    $files = [];
    foreach ($iterator as $entry) {
        if ($entry instanceof SplFileInfo && $entry->isFile() && is_puml_file($entry->getPathname())) {
            $files[] = $entry->getRealPath() ?: $entry->getPathname();
        }
    }

    sort($files, SORT_STRING);
    return $files;
}

function ensure_directory(string $path): void
{
    if (is_dir($path)) {
        return;
    }

    if (!mkdir($path, 0777, true) && !is_dir($path)) {
        throw new RuntimeException('Failed to create directory: ' . $path);
    }
}

function relative_path(string $base, string $path): string
{
    $baseReal = realpath(normalize_path($base));
    $pathReal = realpath(normalize_path($path));

    if ($baseReal === false || $pathReal === false) {
        throw new RuntimeException('Failed to resolve path.');
    }

    $baseReal = rtrim(str_replace('\\', '/', $baseReal), '/');
    $pathReal = str_replace('\\', '/', $pathReal);

    if ($pathReal === $baseReal) {
        return '';
    }

    if (str_starts_with($pathReal, $baseReal . '/')) {
        return substr($pathReal, strlen($baseReal) + 1);
    }

    return basename($pathReal);
}

function shell_command(array $args): string
{
    return implode(' ', array_map(static fn(string $arg): string => escapeshellarg($arg), $args));
}

function build_command(Options $options): string
{
    if ($options->jar !== null) {
        return shell_command([
            'java',
            '-Dfile.encoding=UTF-8',
            '-jar',
            $options->jar,
            '-charset',
            'UTF-8',
            '-tpng',
            '-pipe',
        ]);
    }

    return shell_command([
        (string) $options->plantuml,
        '-charset',
        'UTF-8',
        '-tpng',
        '-pipe',
    ]);
}

function is_png_data(string $data): bool
{
    return str_starts_with($data, "\x89PNG\r\n\x1a\n");
}

function render_puml_to_png(Options $options, string $inputFile, string $outputFile): void
{
    $source = file_get_contents($inputFile);
    if ($source === false) {
        throw new RuntimeException('Failed to read input file: ' . $inputFile);
    }

    $command = build_command($options);
    if ($options->verbose) {
        echo $command . ' < ' . $inputFile . ' > ' . $outputFile . PHP_EOL;
    }

    $descriptorSpec = [
        0 => ['pipe', 'r'],
        1 => ['pipe', 'w'],
        2 => ['pipe', 'w'],
    ];

    $process = proc_open($command, $descriptorSpec, $pipes);
    if (!is_resource($process)) {
        throw new RuntimeException('Failed to start PlantUML process.');
    }

    fwrite($pipes[0], $source);
    fclose($pipes[0]);

    $png = stream_get_contents($pipes[1]);
    $stderrOutput = stream_get_contents($pipes[2]);
    fclose($pipes[1]);
    fclose($pipes[2]);

    $exitCode = proc_close($process);
    if ($exitCode !== 0) {
        $message = trim((string) $stderrOutput);
        throw new RuntimeException($message !== '' ? $message : ('PlantUML failed with exit code ' . $exitCode));
    }

    if ($png === false || $png === '') {
        $message = trim((string) $stderrOutput);
        throw new RuntimeException($message !== '' ? $message : 'PlantUML produced no PNG data.');
    }

    if (!is_png_data($png)) {
        $message = trim((string) $stderrOutput);
        if ($message !== '') {
            throw new RuntimeException('PlantUML did not produce PNG data: ' . $message);
        }
        throw new RuntimeException('PlantUML did not produce PNG data.');
    }

    ensure_directory(dirname($outputFile));
    if (file_put_contents($outputFile, $png) === false) {
        throw new RuntimeException('Failed to write output file: ' . $outputFile);
    }
}

function convert_files(Options $options): int
{
    $files = collect_input_files($options);
    if ($files === []) {
        stderr('No .puml files were found.');
        return 0;
    }

    $inputRoot = normalize_path($options->input);
    $outputRoot = absolute_path($options->output);
    ensure_directory($outputRoot);

    $converted = 0;
    foreach ($files as $file) {
        $relativeDir = '';
        if (is_dir($inputRoot)) {
            $relativeDir = relative_path($inputRoot, dirname($file));
        }

        $targetDir = $outputRoot;
        if ($relativeDir !== '') {
            $targetDir .= DIRECTORY_SEPARATOR . str_replace(['/', '\\'], DIRECTORY_SEPARATOR, $relativeDir);
        }

        $outputFile = $targetDir . DIRECTORY_SEPARATOR . pathinfo($file, PATHINFO_FILENAME) . '.png';
        render_puml_to_png($options, $file, $outputFile);

        if (!is_file($outputFile)) {
            throw new RuntimeException('Expected output file was not created: ' . $outputFile);
        }

        echo '[OK] ' . $file . ' -> ' . $outputFile . PHP_EOL;
        ++$converted;
    }

    return $converted;
}

function main(): int
{
    try {
        $options = parse_options();
        $count = convert_files($options);
        echo 'Converted ' . $count . ' file(s).' . PHP_EOL;
        return 0;
    } catch (Throwable $e) {
        stderr('Error: ' . $e->getMessage());
        return 1;
    }
}

exit(main());
