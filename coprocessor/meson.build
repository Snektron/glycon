sources = [
    'src/main.c',
]

coproc_elf = executable(
    'coproc.elf',
    sources,
)

coproc_ihx = custom_target(
    'coproc',
    input: coproc_elf,
    output: 'coproc.ihx',
    build_by_default: true,
    command: [
        find_program('objcopy'),
        '-O', 'ihex',
        '-R', '.eeprom',
        '@INPUT@',
        '@OUTPUT@',
    ]
)

run_target(
    'flash',
    command: [
        find_program('avrdude'),
        '-p', host_machine.cpu(),
        '-c', meson.get_cross_property('programmer'),
        '-b', meson.get_cross_property('baud_rate'),
        '-D',
        '-U', 'flash:w:@0@:i'.format(coproc_ihx.full_path()),
        '-P', get_option('port')
    ],
    depends: coproc_ihx
)
