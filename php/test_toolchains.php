<?php

declare(strict_types=1);

return [
    'gcc' => [
        'compiler' => 'g++',
        'style' => 'gcc',
        'cxxflags' => [],
        'ldflags' => [],
    ],

    'clang' => [
        'compiler' => 'clang++',
        'style' => 'gcc',
        'cxxflags' => [
            '-stdlib=libc++',
        ],
        'ldflags' => [
            '-stdlib=libc++',
        ],
    ],

    'clang64' => [
        'compiler' => 'clang++',
        'style' => 'gcc',
        'cxxflags' => [
            '-D__USE_MINGW_ANSI_STDIO=1',
        ],
        'ldflags' => [],
    ],

    'clang-cl' => [
        'compiler' => 'clang-cl',
        'style' => 'clang-cl',
        'cxxflags' => [],
        'ldflags' => [],
        'platform' => 'windows-msvc',
    ],

    'msvc' => [
        'compiler' => 'cl',
        'style' => 'msvc',
        'cxxflags' => [],
        'ldflags' => [],
        'platform' => 'windows-msvc',
        'build_id' => 'windows-msvc-cl',
    ],
];
