scas = find_program('scas')

kernel_bin = custom_target(
    'kernel',
    input: 'kernel/main.z80',
    output: 'kernel.bin',
    build_by_default: true,
    command: [scas, '@INPUT@', '-o', '@OUTPUT@'],
)
