env = Environment()

env['CXX'] = 'clang++'

env.Append(CXXFLAGS=[
    '-std=c++2a',
])
env.Append(CCFLAGS= [
    '-Wall',
    '-Wextra',
])

if PROFILE == 'debug':
    env.Append(CCFLAGS=[
        '-g',
        '-Og',
        '-fsanitize=undefined',
        '-fsanitize=address',
    ])
    env.Append(LINKFLAGS=[
        '-fsanitize=undefined',
        '-fsanitize=address',
    ])
else:
    env.Append(CCFLAGS=[
        '-O3',
        '-DNDEBUG',
        '-flto',
    ])
    env.Append(LINKFLAGS=[
        '-flto',
    ])

Export('env')

env.SConscript('src/SConscript', variant_dir='build')
