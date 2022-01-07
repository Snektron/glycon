sources = [
    'src/main.c',
    'src/serial.c',
]

glyco_elf = executable(
    'glyco.elf',
    sources,
    include_directories: [bdbp_inc, include_directories('src')],
    override_options: [
        'buildtype=minsize'
    ]
)

glyco_ihx = custom_target(
    'glyco',
    input: glyco_elf,
    output: 'glyco.ihx',
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
        '-U', 'flash:w:@0@:i'.format(glyco_ihx.full_path()),
        '-P', get_option('port')
    ],
    depends: glyco_ihx,
)
