

sources = files(
      'Config.cpp'
    , 'NomadApp.cpp'
    , 'NomadWin.cpp'
    , 'EditMode.cpp'
    , 'Capture.cpp'
    , 'Shape.cpp'
    , 'CairoShape.cpp'
    , 'TextShape.cpp'
    , 'SvgShape.cpp'
    , 'PenlWindow.cpp'
    , 'ScaleableShape.cpp'
    , 'ImageReader.cpp'
    , 'Preview.cpp'
    , 'ScanDialog.cpp'
    , 'ScanPreview.cpp'
    )

message('x11 found', x11_lib.found())
if x11_lib.found()
    sources += files('X11Capture.cpp')
endif
if get_option('libraw')
    sources += files('RawImageReader.cpp')
endif
message('use sane ', use_sane)
if use_sane
sources += files(
        'SaneScanDialog.cpp'
      , 'SaneScanDevice.cpp'
      , 'SaneScanPreview.cpp'
        )
endif

# both these values are windows (... have not seen msys/cygwin)
#message('build mach', build_machine.system())
#message('targ mach', target_machine.system())
if target_machine.system() == 'windows'
    sources += files(
          'WiaScanDlg.cpp'
        , 'WiaScanPreview.cpp'
        , 'WinCapture.cpp'
        , 'WiaDataCallback.cpp'
        , 'WiaProperty.cpp'
        , 'WiaDevice.cpp'
        , 'WiaScan.cpp'
        , 'wia_lh_i.c'         
        )
endif
