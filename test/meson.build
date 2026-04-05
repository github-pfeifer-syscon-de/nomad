

raw_deps = [glibmm2_deps, gtkmm3_deps, libraw_deps, thread_deps]

incTest = include_directories(
      '..'
     ,'../src')

rawTest = executable('raw_test'
    , 'raw_test.cpp'
    , include_directories : incTest
    , dependencies: raw_deps)
test('raw_test', rawTest)

if sane_lib.found()
sane_deps = [sane_deps, genericimg_deps]

saneTest = executable('sane_test'
      , files('sane_test.cpp'
        , '../src/SaneScanDevice.cpp')
      , include_directories : incTest
      , dependencies: sane_deps)
# use as manual test
#test('sane_test', saneTest)
endif