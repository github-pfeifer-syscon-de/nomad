# nomad
Simple capture/scan(win) utility.

Requires genericImg see there for some basic build infos.

Optional dependency libharu to allow exporting pdf.

The desktop file contains entries, that make this app a picture viewer,
if this is not expected remove MimeTypes from res/nomad.desktop.in.

With .configure there is a the option to use --with-libraw
to support basic raw-file viewing.
Since libraw does not supply a list of extensions to recognize
files (I know it's not the smartest way to do this, but it is the fastest)
see RawImageReader.hpp RawImageReader::RAW_EXT you may need to add
extensions for formats you want to handle (and are supported by libraw).

## Windows
The scanning interface is just about to get reworked for vista+,
but at moment not working.
As the WIA headers wia_lh.h for vista+ is missing some functions 
here is a workaround if you fell the need to implement more types:
```
pacman -S msys2-w32api-headers
pacman -S msys2-w32api-runtime
pacman -S ${MINGW_PACKAGE_PREFIX}-tools
```
- download the windows SDK to get the signature for missing types from wia_lh.h
- identify the missing types and add those in src/wia_lh.idl
- it is important to add all functions for a type as otherwise calls will not work!
- in src use widl with build.sh to regenerate the header 
