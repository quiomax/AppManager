blueprint_compiler = find_program('blueprint-compiler', required: true)

blueprints = custom_target('blueprints',
  input: files('ui/window.blp', 'ui/details_dialog.blp'),
  output: '.',
  command: [blueprint_compiler, 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)

resources = gnome.compile_resources('resources',
  'resources.gresource.xml',
  dependencies: blueprints,
  source_dir: '.',
)
