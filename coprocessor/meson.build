project(
    'coprocessor',
    'c',
    default_options: [
        'c_std=gnu11',
        'b_lto=true',
        'buildtype=minsize',
    ]
)

if not meson.is_cross_build()
    error('Glycon coprocessor should be cross-compiled: meson glycon/coprocessor build --cross-file glycon/coprocessor/avr-atmega2560-cross.ini')
endif

sources = [
    'src/main.c',
    'src/serial.c',
    'src/pinout.c',
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
