cargs = ['-DCROMA_ICON_DIR="croma"']

install_data('croma-window-demo.png',
  install_dir: join_paths(pkgdatadir, 'icons')
)

executable('croma-message',
  'croma-message.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_message_deps,
  link_with : [
    libcroma
    ],
  c_args : cargs,
  install: true,
)

executable('croma-window-demo',
  'croma-window-demo.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_window_demo_deps,
  link_with : [
    libcroma
    ],
  c_args : cargs,
  install: true,
)
