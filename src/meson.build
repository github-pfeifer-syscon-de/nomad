

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
    )

message('x11 found', x11_lib.found())
if x11_lib.found()
    sources += files('X11Capture.cpp')
endif
if get_option('libraw')
    sources += files('RawImageReader.cpp')
endif
# both values are windows (as we use win_subsystem: 'windows')
#message('build mach', build_machine.system())
#message('targ mach', target_machine.system())
if target_machine.system() == 'windows'
    sources += files(
          'ScanDlg.cpp'
        , 'ScanPreview.cpp'
        , 'WinCapture.cpp'
        , 'WiaDataCallback.cpp'
        , 'WiaProperty.cpp'
        , 'WiaDevice.cpp'
        , 'WiaScan.cpp'
        , 'wia_lh_i.c'         
        )
endif
