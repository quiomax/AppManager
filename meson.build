project('muha-app-manager', 'c',
  version: '0.1.0',
  default_options: ['warning_level=3', 'c_std=gnu11'],
)

i18n = import('i18n')
gnome = import('gnome')

deps = [
  dependency('gtk4', version: '>= 4.10.0'),
  dependency('libadwaita-1', version: '>= 1.4.0'),
]

subdir('data')
subdir('src')
