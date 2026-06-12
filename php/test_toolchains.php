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
        'cxxflags' => ['-stdlib=libc++'],
        'ldflags' => ['-stdlib=libc++'],
    ],
];
