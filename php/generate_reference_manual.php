<?php

declare(strict_types=1);

/**
 * Generate docs/xer_reference_manual_en.md from:
 *
 * - xer/version.h
 * - docs/public_headers.md
 * - docs/bits/header_*.md
 *
 * Assumptions:
 * - This script is executed from the php/ directory.
 * - The English reference manual is the canonical generated output.
 * - The initial version is intentionally simple: it concatenates
 *   per-header documentation fragments in public-header order.
 */

final class ReferenceManualGenerator
{
    private string $projectRoot;
    private string $docsDir;
    private string $bitsDir;
    private string $versionHeaderPath;
    private string $publicHeadersPath;
    private string $outputPath;

    public function __construct(string $projectRoot)
    {
        $this->projectRoot = $this->normalizePath($projectRoot);
        $this->docsDir = $this->projectRoot . '/docs';
        $this->bitsDir = $this->docsDir . '/bits';
        $this->versionHeaderPath = $this->projectRoot . '/xer/version.h';
        $this->publicHeadersPath = $this->docsDir . '/public_headers.md';
        $this->outputPath = $this->docsDir . '/xer_reference_manual_en.md';
    }

    public function run(): int
    {
        try {
            $version = $this->readVersionString($this->versionHeaderPath);
            $publicHeaders = $this->readPublicHeaders($this->publicHeadersPath);
            $headerFragments = $this->loadHeaderFragments($publicHeaders);

            $markdown = $this->buildReferenceManual($version, $publicHeaders, $headerFragments);

            $this->writeFile($this->outputPath, $markdown);

            fwrite(STDOUT, "Generated: {$this->outputPath}\n");
            return 0;
        } catch (RuntimeException $e) {
            fwrite(STDERR, "Error: {$e->getMessage()}\n");
            return 1;
        }
    }

    private function normalizePath(string $path): string
    {
        $normalized = str_replace('\\', '/', $path);
        return rtrim($normalized, '/');
    }

    private function readFile(string $path): string
    {
        if (!is_file($path)) {
            throw new RuntimeException("File not found: {$path}");
        }

        $content = file_get_contents($path);
        if ($content === false) {
            throw new RuntimeException("Failed to read file: {$path}");
        }

        return str_replace("\r\n", "\n", $content);
    }

    private function writeFile(string $path, string $content): void
    {
        $result = file_put_contents($path, $content);
        if ($result === false) {
            throw new RuntimeException("Failed to write file: {$path}");
        }
    }

    private function readVersionString(string $path): string
    {
        $content = $this->readFile($path);

        if (preg_match('/#define\s+XER_VERSION_STRING\s+"([^"]+)"/', $content, $matches) === 1) {
            return $matches[1];
        }

        throw new RuntimeException("XER_VERSION_STRING was not found in {$path}");
    }

    /**
     * @return list<string>
     */
    private function readPublicHeaders(string $path): array
    {
        $content = $this->readFile($path);

        // 1. Prefer a fenced block inside the "Final List of Public Headers" section.
        $headers = $this->tryReadPublicHeadersFromFinalSection($content);
        if ($headers !== []) {
            return $headers;
        }

        // 2. Otherwise, search all fenced code blocks and use the first one that
        //    looks like a clean public-header list.
        $headers = $this->tryReadPublicHeadersFromAnyCodeBlock($content);
        if ($headers !== []) {
            return $headers;
        }

        // 3. As a last resort, scan the whole document for xer/*.h lines.
        $headers = $this->tryReadPublicHeadersFromWholeDocument($content);
        if ($headers !== []) {
            return $headers;
        }

        throw new RuntimeException(
            "Could not find any usable public-header list in {$path}"
        );
    }

    /**
     * @return list<string>
     */
    private function tryReadPublicHeadersFromFinalSection(string $content): array
    {
        $sectionPattern = '/^##\s+Final List of Public Headers\b(.*?)(?=^##\s|\z)/ms';
        if (preg_match($sectionPattern, $content, $sectionMatches) !== 1) {
            return [];
        }

        $section = $sectionMatches[1];

        $blockPattern = '/```[A-Za-z0-9_-]*\n(.*?)\n```/s';
        if (preg_match_all($blockPattern, $section, $blockMatches) !== false) {
            foreach ($blockMatches[1] as $block) {
                $headers = $this->extractHeaderLines($block);
                if ($headers !== []) {
                    return $headers;
                }
            }
        }

        return [];
    }

    /**
     * @return list<string>
     */
    private function tryReadPublicHeadersFromAnyCodeBlock(string $content): array
    {
        $blockPattern = '/```[A-Za-z0-9_-]*\n(.*?)\n```/s';
        if (preg_match_all($blockPattern, $content, $blockMatches) === false) {
            return [];
        }

        foreach ($blockMatches[1] as $block) {
            $headers = $this->extractHeaderLines($block);
            if ($headers !== []) {
                return $headers;
            }
        }

        return [];
    }

    /**
     * @return list<string>
     */
    private function tryReadPublicHeadersFromWholeDocument(string $content): array
    {
        return $this->extractHeaderLines($content);
    }

    /**
     * Extract lines that look like:
     *   xer/error.h
     *   xer/assert.h
     *
     * @return list<string>
     */
    private function extractHeaderLines(string $text): array
    {
        $lines = preg_split('/\n/', trim($text));
        if ($lines === false) {
            return [];
        }

        $headers = [];

        foreach ($lines as $line) {
            $line = trim($line);

            if ($line === '') {
                continue;
            }

            // Remove common markdown list markers or inline-code wrappers if present.
            $line = preg_replace('/^[-*+]\s+/', '', $line);
            $line = trim((string)$line, " \t`");

            if (preg_match('#^xer/([A-Za-z0-9_]+)\.h$#', $line) === 1) {
                $headers[] = $line;
            }
        }

        $headers = array_values(array_unique($headers));

        if ($headers === []) {
            return [];
        }

        return $headers;
    }

    /**
     * @param list<string> $publicHeaders
     * @return array<string, string>
     */
    private function loadHeaderFragments(array $publicHeaders): array
    {
        $fragments = [];

        foreach ($publicHeaders as $headerPath) {
            $baseName = basename($headerPath, '.h');
            $fragmentPath = $this->bitsDir . '/header_' . $baseName . '.md';

            if (!is_file($fragmentPath)) {
                throw new RuntimeException(
                    "Header fragment is missing for {$headerPath}: {$fragmentPath}"
                );
            }

            $fragment = trim($this->readFile($fragmentPath));
            if ($fragment === '') {
                throw new RuntimeException("Header fragment is empty: {$fragmentPath}");
            }

            $fragments[$headerPath] = $fragment;
        }

        return $fragments;
    }

    /**
     * @param list<string> $publicHeaders
     * @param array<string, string> $headerFragments
     */
    private function buildReferenceManual(
        string $version,
        array $publicHeaders,
        array $headerFragments
    ): string {
        $parts = [];

        $parts[] = '# XER Reference Manual';
        $parts[] = '';
        $parts[] = 'Target version: **v' . $version . '**';
        $parts[] = '';
        $parts[] = '> This file is generated by `php/generate_reference_manual.php`.';
        $parts[] = '> Do not edit this file directly unless you intentionally want to overwrite generated output.';
        $parts[] = '';
        $parts[] = '## Overview';
        $parts[] = '';
        $parts[] = 'This reference manual is generated from public-header documentation fragments under `docs/bits/`.';
        $parts[] = 'The current initial generator intentionally keeps the process simple:';
        $parts[] = '';
        $parts[] = '- it reads the public header order from `docs/public_headers.md`';
        $parts[] = '- it reads the version string from `xer/version.h`';
        $parts[] = '- it concatenates `docs/bits/header_*.md` in public-header order';
        $parts[] = '';
        $parts[] = 'Later, this generator may also incorporate examples, source-level API extraction, and additional reusable documentation snippets.';
        $parts[] = '';
        $parts[] = '## Public Headers';
        $parts[] = '';

        foreach ($publicHeaders as $headerPath) {
            $parts[] = '- `' . $headerPath . '`';
        }

        $parts[] = '';
        $parts[] = '---';
        $parts[] = '';

        foreach ($publicHeaders as $index => $headerPath) {
            $fragment = $headerFragments[$headerPath] ?? null;
            if ($fragment === null) {
                throw new RuntimeException("Internal error: fragment not loaded for {$headerPath}");
            }

            if ($index > 0) {
                $parts[] = '';
                $parts[] = '---';
                $parts[] = '';
            }

            $parts[] = $fragment;
        }

        $parts[] = '';

        return implode("\n", $parts);
    }
}

$scriptDir = __DIR__;
$projectRoot = dirname($scriptDir);

$generator = new ReferenceManualGenerator($projectRoot);
exit($generator->run());
