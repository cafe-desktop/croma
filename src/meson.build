cargs = ['-DG_LOG_DOMAIN="croma"']

install_headers([
	'include/boxes.h',
	'ui/gradient.h',
	'include/util.h',
	'include/common.h',
	'ui/preview-widget.h',
	'ui/theme-parser.h',
	'ui/theme.h'
  ],
  subdir : 'croma-1/croma-private'
)

libcroma = shared_library(
  'croma-private',
  sources : [
    'core/async-getprop.c',
    'core/async-getprop.h',
    'core/atomnames.h',
    'core/bell.c',
    'core/bell.h',
    'core/boxes.c',
    'include/boxes.h',
    'compositor/compositor.c',
    'compositor/compositor-private.h',
    'compositor/compositor-xrender.c',
    'compositor/compositor-xrender.h',
    'include/compositor.h',
    'core/constraints.c',
    'core/constraints.h',
    'core/core.c',
    'core/delete.c',
    'core/display.c',
    'core/display-private.h',
    'include/display.h',
    'ui/draw-workspace.c',
    'ui/draw-workspace.h',
    'core/edge-resistance.c',
    'core/edge-resistance.h',
    'core/effects.c',
    'core/effects.h',
    'core/errors.c',
    'include/errors.h',
    'core/eventqueue.c',
    'core/eventqueue.h',
    'core/frame.c',
    'core/frame-private.h',
    'include/frame.h',
    'ui/gradient.c',
    'ui/gradient.h',
    'core/group-private.h',
    'core/group-props.c',
    'core/group-props.h',
    'core/group.c',
    'core/group.h',
    'core/iconcache.c',
    'core/iconcache.h',
    'core/keybindings.c',
    'core/keybindings.h',
    'core/main.c',
    'include/main.h',
    'core/croma-Xatomtype.h',
    'core/place.c',
    'core/place.h',
    'core/prefs.c',
    'include/prefs.h',
    'core/screen.c',
    'core/screen-private.h',
    'include/screen.h',
    'include/types.h',
    'core/session.c',
    'core/session.h',
    'core/stack.c',
    'core/stack.h',
    'core/util.c',
    'include/util.h',
    'core/window-props.c',
    'core/window-props.h',
    'core/window.c',
    'core/window-private.h',
    'include/window.h',
    'core/workspace.c',
    'core/workspace.h',
    'core/xprops.c',
    'include/xprops.h',
    'include/common.h',
    'include/core.h',
    'include/ui.h',
    'ui/fixedtip.c',
    'ui/fixedtip.h',
    'ui/frames.c',
    'ui/frames.h',
    'ui/menu.c',
    'ui/menu.h',
    'ui/metaaccellabel.c',
    'ui/metaaccellabel.h',
    'ui/preview-widget.c',
    'ui/preview-widget.h',
    'ui/resizepopup.c',
    'include/resizepopup.h',
    'ui/tabpopup.c',
    'include/tabpopup.h',
    'ui/tile-preview.c',
    'include/tile-preview.h',
    'ui/theme-parser.c',
    'ui/theme-parser.h',
    'ui/theme.c',
    'ui/theme.h',
    'ui/ui.c',
    'include/all-keybindings.h',
  ],
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    include_directories('include'),
  ],
  dependencies : croma_deps,
  soversion: soversion,
  version: libversion,
  c_args : cargs,
  install: true,
)

executable('croma',
  'core/main.c',
  'include/main.h',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    include_directories('./include'),
    ],
  dependencies : croma_deps,
  link_with : [
    libcroma
    ],
  c_args : cargs,
  install: true,
)
executable('croma-theme-viewer',
  'ui/theme-viewer.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    include_directories('./include'),
    ],
  dependencies : croma_deps,
  link_with : [
    libcroma,
    ],
  install: true,
)

executable('testboxes',
  'include/util.h',
  'core/util.c',
  'include/boxes.h',
  'core/boxes.c',
  'core/testboxes.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    include_directories('./include'),
  ],
  dependencies : croma_deps,
)

executable('testgradient',
  'ui/gradient.h',
  'ui/gradient.c',
  'ui/testgradient.c',
  dependencies : croma_deps,
)

executable('testasyncgetprop',
  'core/async-getprop.h',
  'core/async-getprop.c',
  'core/testasyncgetprop.c',
  dependencies : croma_deps,
)

gnome = import('gnome')
i18n = import('i18n')

i18n.merge_file(
  input: 'croma.desktop.in',
  output: 'croma.desktop',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications')
  )

i18n.merge_file(
  input: 'croma-wm.desktop.in',
  output: 'croma-wm.desktop',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(datadir, 'cafe', 'wm-properties')
  )

compiled_schemas = gnome.compile_schemas()
install_data('org.cafe.croma.gschema.xml',
               install_dir : 'share/glib-2.0/schemas')

xml_files = [
  '50-croma-desktop-key.xml',
  '50-croma-window-key.xml',
  '50-croma-global-key.xml',
  ]

foreach xml : xml_files
  i18n.merge_file(xml,
    input: xml+ '.in',
    output: xml,
    po_dir: '../po',
    data_dirs: '..',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'cafe-control-center', 'keybindings')
    )
endforeach

pkconf = configuration_data()
pkconf.set('prefix', prefix)
pkconf.set('exec_prefix', prefix)
pkconf.set('libdir', libdir)
pkconf.set('includedir', includedir)
pkconf.set('libexecdir', libexecdir)
pkconf.set('VERSION', meson.project_version())

pc = configure_file(
  input : 'libcroma-private.pc.in',
  output : 'libcroma-private.pc',
  configuration : pkconf
)

install_data(pc,
  install_dir: join_paths(get_option('libdir'), 'pkgconfig'),
)
subdir('wm-tester')
subdir('tools')
subdir('themes')
