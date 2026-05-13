# nomad
Simple capture/scan(win) utility.

Requires genericImg see there for some basic build infos.

Optional dependency libharu to allow exporting pdf.

The desktop file contains entries, that make this app a picture viewer,
if this is not expected remove MimeTypes from res/nomad.desktop.in.

With meson setup there is the option to use -Dlibraw=true
to support basic raw-file viewing.
Since libraw does not supply a list of extensions to recognize
files (I know it's not the smartest way to do this, but it is the fastest)
see RawImageReader.hpp RawImageReader::RAW_EXT you may need to add
extensions for formats you want to handle (and are supported by libraw).

## Dependencies

The following dependencies libs are optional
- libharu for pdf functions
- sane (linux) scanning
- x11 linux screen capture

## Linux

The Sane interface is a work in progress, results may vary... 

The sane interface will be enabled if 
the include file "sane/sane.h" was found on initialisation.
The Sane interface is dynamically linked (at runtime),
if you are interested why it came this way see explanation.
The nomad.conf allows using a section like: 
```
[scan] 
sanePath=/usr/lib/sane
```
to load the sane interface, adapt the path to match your installation 
(expected file: libsane-dll.so.1), if anything looks wrong check console output.
The interface uses a minimal version check, if you feel this 
might be too tight see ~SaneScanDevice.hpp:239 SANE_MIN_VERSION.
Changed function signatures should result in compile errors
(even if they appear a bit lengthly due to the template).

### Explanation

To include the sane interface in meson.build was used: 
```
sane_lib    = cc.find_library('sane-dll', required: false, dirs: '/usr/lib/sane')   
```
The generated build.ninja contains:
```
... 
build nomad: ...
-Wl,-rpath,/usr/lib/sane -Wl,--start-group ... /usr/lib/sane/libsane-dll.so -Wl,--end-group
```
Result: from the build dir the generated executable will run fine.
With meson 1.11? install will remove the RUNPATH from executable 
```
 readelf -d  nomad |grep sane
 0x0000000000000001 (NEEDED)             Shared library: [libsane-dll.so.1]
 0x000000000000001d (RUNPATH)            Library runpath: [/usr/lib/sane]
 readelf -d  pkg/.../usr/bin/nomad |grep sane
 0x0000000000000001 (NEEDED)             Shared library: [libsane-dll.so.1]
```
This breaks program startup, workaround alternatives (now obsolete):
- set LD_LIBRARY_PATH before starting/conf files
- use the binary from build dir (without install "processing")

## Windows

The Wia interface is a work in progress, results may vary...

As the WIA headers wia_lh.h for vista+ is missing some functions 
here is a workaround if you fell the need to implement more types:
```
pacman -S msys2-w32api-headers
pacman -S msys2-w32api-runtime
pacman -S ${MINGW_PACKAGE_PREFIX}-tools
pacman -S ${MINGW_PACKAGE_PREFIX}-tbb
```
- download the windows SDK to get the signature for missing types from wia_lh.h
- identify the missing types and add those in src/wia_lh.idl
- it is important to add all functions for a type as otherwise calls will not work!
- in src use widl with build.sh to regenerate the header 
