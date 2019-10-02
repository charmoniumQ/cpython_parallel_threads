env = Environment()

env['CXX'] = 'clang++'

env.Append(
    CXXFLAGS=[
        '-std=c++2a',
    ],
    CCFLAGS= [
        '-Wall',
        '-Wextra',
    ],
    LIBS=[
        'boost',
    ],
)

if PROFILE == 'debug':
    env.Append(
        CCFLAGS=[
            '-g',
            '-Og',
            '-fsanitize=undefined',
            '-fsanitize=address',
        ],
        LINKFLAGS=[
            '-fsanitize=undefined',
            '-fsanitize=address',
        ],
    )
else:
    env.Append(
        CCFLAGS=[
            '-O3',
            '-DNDEBUG',
            '-flto',
        ],
        LINKFLAGS=[
            '-flto',
        ],
    )

Export('env')

env.SConscript('src/SConscript', variant_dir='build')
