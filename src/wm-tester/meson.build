test1 = executable('wm-tester',
  'main.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_deps,
  link_with : [
    libcroma
    ],
)

test2 = executable('test-gravity',
  'test-gravity.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_deps,
  link_with : [
    libcroma
    ],
)

test3 = executable('test-resizing',
  'test-resizing.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_deps,
  link_with : [
    libcroma
    ],
)

test4  = executable('focus-window',
  'focus-window.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_deps,
  link_with : [
    libcroma
    ],
)

test5 = executable('test-size-hints',
  'test-size-hints.c',
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies: croma_deps,
  link_with : [
    libcroma
    ],
)

test('wm-tester', test1)
test('test-gravity',  test2)
test('test-resizing', test3)
test('focus-window',  test4)
test('test-size-hints',  test5)
