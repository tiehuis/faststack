# This is linux-specific right now so fail early.
compiler = meson.get_compiler('c')

if not compiler.has_header('linux/input.h')
   error('missing linux/input.h header!')
endif

terminal_src = files(['main.c', 'frontend.c', 'glyph.c'])
terminal_deps = []
terminal_defines = ['-DFS_USE_TERMINAL']
terminal_inc = []
