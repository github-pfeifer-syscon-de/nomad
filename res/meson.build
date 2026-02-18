
#
# see https://mesonbuild.com/Gnome-module.html
gnome = import('gnome')

app_resources = gnome.compile_resources(
    'resources', 'nomad.gresources.xml'
    , source_dir: meson.current_source_dir()
)

install_data(
      meson.project_name() + '.png'
    , install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'icons/hicolor/64x64/apps'))

install_data(
      meson.project_name() + '.svg'
    , install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'icons/hicolor/scalable/apps'))

install_data(
      meson.project_name() + '.desktop'
    , install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'applications'))

