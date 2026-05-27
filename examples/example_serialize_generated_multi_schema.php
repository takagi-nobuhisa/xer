<?php

declare(strict_types=1);

return [
    'namespace' => 'xer_example_multi',
    'structs' => [
        'packet_header' => [
            'version' => u16,
            'kind' => u16,
            'sequence' => u32,
        ],
        'sensor_sample' => [
            'id' => u32,
            'name' => s,
            'values' => [f32, v],
            'calibration' => [[s, f64], m],
            'raw' => [u8, [a, 8]],
        ],
    ],
];
