

deps = [glibmm2_deps, gtkmm3_deps, libraw_deps, thread_deps]

# also make config available here
configure_file (output : 'nomad_config.h', configuration : conf)

e = executable('raw_test'
    , 'raw_test.cpp'
    , dependencies: deps)
test('raw_test', e)
