<?php

declare(strict_types=1);

/**
 * Generate the English and Japanese reference manuals from:
 *
 * - xer/version.h
 * - docs/public_headers.md
 * - docs/bits/header_*.md
 * - docs/bits/header_*.ja.md when available and up to date
 * - docs/bits/*.md files included from fragments with XER_INCLUDE directives
 *
 * Assumptions:
 * - This script is executed from the php/ directory.
 */
final class ReferenceManualGenerator
{
    private const TRANSLATION_SOURCE_HASH_PATTERN = '/<!--\s*xer-reference-source-sha256:\s*([a-f0-9]{64})\s*-->/i';

    private string $projectRoot;
    private string $docsDir;
    private string $bitsDir;
    private string $versionHeaderPath;
    private string $publicHeadersPath;
    private string $englishOutputPath;
    private string $japaneseOutputPath;

    public function __construct(string $projectRoot)
    {
        $this->projectRoot = $this->normalizePath($projectRoot);
        $this->docsDir = $this->projectRoot . '/docs';
        $this->bitsDir = $this->docsDir . '/bits';
        $this->versionHeaderPath = $this->projectRoot . '/xer/version.h';
        $this->publicHeadersPath = $this->docsDir . '/public_headers.md';
        $this->englishOutputPath = $this->docsDir . '/xer_reference_manual_en.md';
        $this->japaneseOutputPath = $this->docsDir . '/xer_reference_manual_ja.md';
    }

    public function run(): int
    {
        try {
            $version = $this->readVersionString($this->versionHeaderPath);
            $publicHeaders = $this->readPublicHeaders($this->publicHeadersPath);

            $englishFragments = $this->loadHeaderFragments($publicHeaders, 'en');
            $japaneseFragments = $this->loadHeaderFragments($publicHeaders, 'ja');

            $englishMarkdown = $this->buildReferenceManual(
                $version,
                $publicHeaders,
                $englishFragments,
                'en'
            );
            $japaneseMarkdown = $this->buildReferenceManual(
                $version,
                $publicHeaders,
                $japaneseFragments,
                'ja'
            );

            $this->writeFile($this->englishOutputPath, $englishMarkdown);
            $this->writeFile($this->japaneseOutputPath, $japaneseMarkdown);

            fwrite(STDOUT, "Generated: {$this->englishOutputPath}\n");
            fwrite(STDOUT, "Generated: {$this->japaneseOutputPath}\n");
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

        $headers = $this->tryReadPublicHeadersFromFinalSection($content);
        if ($headers !== []) {
            return $headers;
        }

        $headers = $this->tryReadPublicHeadersFromAnyCodeBlock($content);
        if ($headers !== []) {
            return $headers;
        }

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
    private function loadHeaderFragments(array $publicHeaders, string $language): array
    {
        $fragments = [];

        foreach ($publicHeaders as $headerPath) {
            $baseName = basename($headerPath, '.h');
            $englishPath = $this->bitsDir . '/header_' . $baseName . '.md';

            if (!is_file($englishPath)) {
                throw new RuntimeException(
                    "Header fragment is missing for {$headerPath}: {$englishPath}"
                );
            }

            $fragmentInfo = $this->selectLocalizedFragment($englishPath, $language);
            $fragment = trim($this->expandIncludes(
                $this->stripTranslationSourceHashMarker($this->readFile($fragmentInfo['path'])),
                $fragmentInfo['path'],
                $language
            ));

            if ($fragment === '') {
                throw new RuntimeException("Header fragment is empty: {$fragmentInfo['path']}");
            }

            if ($fragmentInfo['untranslated']) {
                $fragment = $this->makeUntranslatedNotice($headerPath, $fragmentInfo['reason'])
                    . "\n\n" . $fragment;
            }

            $fragments[$headerPath] = $this->normalizeXerBranding($fragment);
        }

        return $fragments;
    }

    /**
     * @return array{path: string, untranslated: bool, reason: string}
     */
    private function selectLocalizedFragment(string $englishPath, string $language): array
    {
        if ($language === 'en') {
            return [
                'path' => $englishPath,
                'untranslated' => false,
                'reason' => '',
            ];
        }

        if ($language !== 'ja') {
            throw new RuntimeException("Unsupported reference-manual language: {$language}");
        }

        $japanesePath = $this->localizedPath($englishPath, 'ja');
        if (!is_file($japanesePath)) {
            return [
                'path' => $englishPath,
                'untranslated' => true,
                'reason' => 'Japanese fragment is missing.',
            ];
        }

        $expectedHash = $this->calculateEnglishSourceHash($englishPath);
        $actualHash = $this->readTranslationSourceHash($japanesePath);

        if ($actualHash === null) {
            return [
                'path' => $englishPath,
                'untranslated' => true,
                'reason' => 'Japanese fragment does not declare the English source hash.',
            ];
        }

        if (!hash_equals($expectedHash, $actualHash)) {
            return [
                'path' => $englishPath,
                'untranslated' => true,
                'reason' => 'Japanese fragment was translated from a different English source hash.',
            ];
        }

        return [
            'path' => $japanesePath,
            'untranslated' => false,
            'reason' => '',
        ];
    }

    private function localizedPath(string $path, string $language): string
    {
        if ($language === 'en') {
            return $path;
        }

        return preg_replace('/\.md\z/', '.' . $language . '.md', $path)
            ?? throw new RuntimeException("Failed to make localized path: {$path}");
    }

    private function makeUntranslatedNotice(string $headerPath, string $reason): string
    {
        return implode("\n", [
            '> **未訳:** この節の日本語版はまだ最新ではありません。',
            '> そのため、暫定的に英語版の内容を掲載しています。',
            '> ',
            '> Header: `' . $headerPath . '`',
            '> Reason: ' . $reason,
        ]);
    }

    /**
     * Expands documentation include directives in a Markdown fragment.
     *
     * The directive format is:
     *
     *     <!-- XER_INCLUDE: file_name.md -->
     *
     * The included file is resolved relative to docs/bits. This keeps
     * header fragments small while allowing detailed subdocuments to be
     * maintained separately and still included in the generated reference
     * manual.
     *
     * For Japanese generation, an include such as `foo.md` uses `foo.ja.md`
     * when that file exists and declares the matching English source hash.
     * Otherwise the English include is used. Unlike header fragments, include-level fallback
     * does not add a separate notice because a header-level notice already
     * identifies untranslated or stale top-level sections.
     *
     * @param list<string> $stack
     */
    private function expandIncludes(
        string $content,
        string $sourcePath,
        string $language,
        array $stack = []
    ): string {
        $pattern = '/<!--\s*XER_INCLUDE:\s*([A-Za-z0-9_.-]+)\s*-->/';

        return preg_replace_callback(
            $pattern,
            function (array $matches) use ($sourcePath, $language, $stack): string {
                $fileName = $matches[1];

                if (str_contains($fileName, '/') || str_contains($fileName, '\\')) {
                    throw new RuntimeException(
                        "Invalid include file name in {$sourcePath}: {$fileName}"
                    );
                }

                $englishIncludePath = $this->bitsDir . '/' . $fileName;
                $includeInfo = $this->selectLocalizedInclude($englishIncludePath, $language);
                $includePath = $includeInfo['path'];

                if (in_array($includePath, $stack, true)) {
                    throw new RuntimeException(
                        "Recursive documentation include detected: {$includePath}"
                    );
                }

                $included = trim($this->stripTranslationSourceHashMarker($this->readFile($includePath)));
                if ($included === '') {
                    throw new RuntimeException("Included documentation fragment is empty: {$includePath}");
                }

                return $this->expandIncludes($included, $includePath, $language, [...$stack, $includePath]);
            },
            $content
        ) ?? throw new RuntimeException("Failed to expand documentation includes in {$sourcePath}");
    }

    /**
     * @return array{path: string}
     */
    private function selectLocalizedInclude(string $englishPath, string $language): array
    {
        if (!is_file($englishPath)) {
            throw new RuntimeException("Included documentation fragment is missing: {$englishPath}");
        }

        if ($language === 'en') {
            return ['path' => $englishPath];
        }

        if ($language !== 'ja') {
            throw new RuntimeException("Unsupported reference-manual language: {$language}");
        }

        $japanesePath = $this->localizedPath($englishPath, 'ja');
        if (!is_file($japanesePath)) {
            return ['path' => $englishPath];
        }

        $expectedHash = $this->calculateEnglishSourceHash($englishPath);
        $actualHash = $this->readTranslationSourceHash($japanesePath);
        if ($actualHash === null || !hash_equals($expectedHash, $actualHash)) {
            return ['path' => $englishPath];
        }

        return ['path' => $japanesePath];
    }

    private function calculateEnglishSourceHash(string $englishPath): string
    {
        $expanded = trim($this->expandIncludes(
            $this->stripTranslationSourceHashMarker($this->readFile($englishPath)),
            $englishPath,
            'en'
        ));

        return hash('sha256', str_replace("\r\n", "\n", $expanded));
    }

    private function readTranslationSourceHash(string $path): ?string
    {
        $content = $this->readFile($path);
        if (preg_match(self::TRANSLATION_SOURCE_HASH_PATTERN, $content, $matches) !== 1) {
            return null;
        }

        return strtolower($matches[1]);
    }

    private function stripTranslationSourceHashMarker(string $content): string
    {
        return preg_replace(self::TRANSLATION_SOURCE_HASH_PATTERN, '', $content)
            ?? throw new RuntimeException('Failed to strip translation source hash marker.');
    }

    /**
     * @param list<string> $publicHeaders
     * @param array<string, string> $headerFragments
     */
    private function buildReferenceManual(
        string $version,
        array $publicHeaders,
        array $headerFragments,
        string $language
    ): string {
        $parts = [];

        if ($language === 'en') {
            $parts[] = '# xer C++ Utility Library Reference Manual';
            $parts[] = '';
            $parts[] = 'Target version: **v' . $version . '**';
        } elseif ($language === 'ja') {
            $parts[] = '# xer C++ Utility Library リファレンスマニュアル';
            $parts[] = '';
            $parts[] = '対象バージョン: **v' . $version . '**';
        } else {
            throw new RuntimeException("Unsupported reference-manual language: {$language}");
        }

        foreach ($publicHeaders as $headerPath) {
            $fragment = $headerFragments[$headerPath] ?? null;
            if ($fragment === null) {
                throw new RuntimeException("Internal error: fragment not loaded for {$headerPath}");
            }

            $parts[] = '';
            $parts[] = '---';
            $parts[] = '';
            $parts[] = $fragment;
        }

        $parts[] = '';

        return implode("\n", $parts);
    }

    private function normalizeXerBranding(string $markdown): string
    {
        $result = '';
        $length = strlen($markdown);
        $offset = 0;

        while ($offset < $length) {
            $fencePos = strpos($markdown, '```', $offset);
            if ($fencePos === false) {
                $result .= $this->normalizeXerBrandingOutsideCode(substr($markdown, $offset));
                break;
            }

            $result .= $this->normalizeXerBrandingOutsideCode(substr($markdown, $offset, $fencePos - $offset));

            $lineEnd = strpos($markdown, "\n", $fencePos);
            if ($lineEnd === false) {
                $result .= substr($markdown, $fencePos);
                break;
            }

            $closingPos = strpos($markdown, "\n```", $lineEnd + 1);
            if ($closingPos === false) {
                $result .= substr($markdown, $fencePos);
                break;
            }

            $closingEnd = strpos($markdown, "\n", $closingPos + 1);
            if ($closingEnd === false) {
                $result .= substr($markdown, $fencePos);
                break;
            }

            $result .= substr($markdown, $fencePos, $closingEnd - $fencePos + 1);
            $offset = $closingEnd + 1;
        }

        return $result;
    }

    private function normalizeXerBrandingOutsideCode(string $markdown): string
    {
        $parts = preg_split('/(`[^`]*`)/', $markdown, -1, PREG_SPLIT_DELIM_CAPTURE);
        if ($parts === false) {
            return $markdown;
        }

        foreach ($parts as $index => $part) {
            if ($index % 2 === 1) {
                continue;
            }

            $parts[$index] = str_replace('XER', 'xer', $part);
        }

        return implode('', $parts);
    }
}

$scriptDir = __DIR__;
$projectRoot = dirname($scriptDir);

$generator = new ReferenceManualGenerator($projectRoot);
exit($generator->run());
