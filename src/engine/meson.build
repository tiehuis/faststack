sql_config = [
    '-DSQLITE_THREADSAFE=0',
    '-DSQLITE_DEFAULT_MEMSTATUS=0',
    '-DSQLITE_MAX_ERR_DEPTH=0',
    '-DSQLITE_OMIT_DECLTYPE=1',
    '-DSQLITE_OMIT_PROGRESS_CALLBACK=1',
    '-DSQLITE_OMIT_SHARED_CACHE=1',
    '-DSQLITE_DISABLE_LFS=1',
    '-DSQLITE_DISABLE_FTS3_UNICODE',
    '-DSQLITE_OMIT_DEPRECATED=1',
    '-DSQLITE_OMIT_LOAD_EXTENSION=1',
    '-DSQLITE_OMIT_UTF16=1'
]

add_project_arguments(sql_config, language : 'c')

engine_src = [
    'control.c',
    'dao.c',
    'engine.c',
    'finesse.c',
    'fslibc.c',
    'log.c',
    'option.c',
    'rand.c',
    'rotation.c',
    'sqlite3.c'
]

engine_lib = static_library('fs', engine_src)
engine_inc = include_directories('.')
