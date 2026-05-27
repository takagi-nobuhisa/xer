<?php

declare(strict_types=1);

return [
    'namespace' => 'xer_example',
    'struct' => 'generated_record',
    'fields' => [
        'id' => u32,
        'name' => s,
        'payload' => bin,
        'scores' => [[s, f64], m],
        'history' => [u16, v],
        'fixed' => [u32, [a, 4]],
    ],
];
