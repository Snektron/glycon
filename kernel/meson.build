scas = find_program('scas')

kernel_bin = custom_target(
    'kernel',
    input: 'src/main.asm',
    output: 'kernel.bin',
    build_by_default: true,
    command: [scas, '@INPUT@', '-o', '@OUTPUT@'],
)
