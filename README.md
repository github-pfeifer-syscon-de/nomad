# nomad
Simple capture/scan(win) utility.

Requires genericImg see there for some basic build infos.

Optional dependency libharu to allow exporting pdf.

The desktop file contains entries, that make this app a picture viewer,
if this is not expected remove MimeTypes from res/nomad.desktop.in.

With .configure there is a the option to use --with-libraw
to support basic raw-file viewing.
Since libraw does not supply a list of extensions to recognize
files (i know it's not the smartest way to do this, but it is the fastest)
see RawImageReader.hpp RawImageReader::RAW_EXT you may need to add
extensions for formats you want to handle.