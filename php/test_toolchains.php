<?php

declare(strict_types=1);

return [
    'gcc' => [
        'compiler' => 'g++',
        'cxxflags' => [],
        'ldflags' => [],
    ],

    'clang' => [
        'compiler' => 'clang++',
        'cxxflags' => [
            '-stdlib=libc++',
        ],
        'ldflags' => [
            '-stdlib=libc++',
        ],
    ],

    'clang64' => [
        'compiler' => 'clang++',
        'cxxflags' => [
            '-D__USE_MINGW_ANSI_STDIO=1',
        ],
        'ldflags' => [],
    ],
];
