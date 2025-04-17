# cmake_policy(['VERSION', '3.6'])
project('jayatana', 'java', 'c',
        meson_version: '>=1.0.0',
        version: '25.04',
        license: ['MIT'],
         default_options : ['c_std=gnu11', 'buildtype=debugoptimized'])

x11 = dependency('x11', required: true)
xkbcommon = dependency('xkbcommon', required : true)
dbusmenu = dependency('dbusmenu-glib-0.4', version : '>=16.04.0')
javac = meson.get_compiler('java')
java = import('java')
jni = dependency('jni', version: '>= 1.8.0', modules: ['jvm', 'awt'], required : true)
glib = dependency('gio-2.0', required : true)

java_ver = javac.version()
version_array = java_ver.split('.')
java_ver = version_array[0]
if java_ver < '2.0'
  java_ver = version_array[1]
endif

data= {
  'CMAKE_INSTALL_FULL_LIBDIR' : join_paths(get_option('prefix'),get_option('libdir')),
  'CMAKE_INSTALL_FULL_DATAROOTDIR' : join_paths(get_option('prefix'),get_option('datadir')),
  'Java_VERSION_MINOR' : java_ver,
  'Java_VERSION_STRING' : javac.version()
}

subdir('java')
subdir('lib')

