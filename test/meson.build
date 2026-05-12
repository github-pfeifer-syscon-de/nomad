

raw_deps = [glibmm2_deps, gtkmm3_deps, libraw_deps, thread_deps]

incTest = include_directories(
      '..'
     ,'../src')

if get_option('libraw')
rawTest = executable('raw_test'
    , 'raw_test.cpp'
    , include_directories : incTest
    , dependencies: raw_deps)
# manual test needs reference to raw see source
#test('raw_test', rawTest)
endif

sane_test_deps = [sane_deps, genericimg_deps]

if sane_lib.found()
saneTest = executable('sane_test'
      , files('sane_test.cpp'
      , '../src/SaneScanDevice.cpp'
      )
      , include_directories : incTest
      , dependencies: sane_test_deps)
# use as manual test
#test('sane_test', saneTest)
endif
