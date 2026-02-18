project('nomad', 'c', 'cpp'
    , default_options: ['c_std=c11', 'cpp_std=c++20']
    , license : 'GPL-3.0'
    , meson_version: '>=1.1.0'
    , version: '0.1')

rpcrt4_deps = declare_dependency()
uuid_deps   = declare_dependency()
winole_deps = declare_dependency()
cc = meson.get_compiler('c')
if target_machine.system() == 'windows'
    #  to use vista+ wia-api see wia.h
    add_project_arguments('-D_WIN32_WINNT=0x0600', language: 'cpp')
    rpcrt4_deps    = declare_dependency(dependencies : cc.find_library('rpcrt4', required: true))
    uuid_deps      = declare_dependency(dependencies : cc.find_library('uuid', required: true))
    winole_deps    = declare_dependency(dependencies : cc.find_library('ole32', required: true))
endif

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: 'cpp')
x11_lib        = cc.find_library('X11', required: false)   # , version : '>= 1.6.4'
x11_deps       = declare_dependency(dependencies : x11_lib)   
libharu_deps   = declare_dependency(dependencies : cc.find_library('hpdf', required: false))
conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
conf.set       ('LIBRAW', get_option('libraw'))
conf.set       ('USE_PDF', libharu_deps.found())
configure_file(output : 'nomad_config.h'
               , configuration : conf)

subdir('src')
subdir('res')
sources += app_resources
genericimg_deps = dependency('genericimg', version : '>= 0.4.8')
gtkmm3_deps     = dependency('gtkmm-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
libsvg2_deps    = dependency('librsvg-2.0')
jsonglib1_deps  = dependency('json-glib-1.0')
thread_deps     = dependency('threads')
intel_tbb_deps  = dependency('tbb')     # thread building blocks
libraw_deps = declare_dependency()
if get_option('libraw')
   libraw_deps = dependency('libraw', version : '>= 0.20.0')
endif
subdir('test')

deps = [genericimg_deps
    , gtkmm3_deps
    , glibmm2_deps
    , libsvg2_deps
    , jsonglib1_deps
    , thread_deps
    , intel_tbb_deps
    , libraw_deps
    , libharu_deps
    , x11_deps
    , rpcrt4_deps 
    , uuid_deps
    , winole_deps
    ]

project_targt = executable(meson.project_name()
    , sources, app_resources
    , dependencies: deps
    , install: true
    , win_subsystem: 'windows'  # disable this to see conole output for windows
    )
