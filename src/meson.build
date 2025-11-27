sources = [
  'main.c',
  'window.c',
  'adb.c',
  'app.c',
  'utils.c',
]

# GResource dosyasını kaynaklara ekle (data/meson.build içinde tanımlanacak)
sources += resources

executable('muha-app-manager',
  sources,
  dependencies: deps,
  include_directories: include_directories('../include'),
  install: true,
)
