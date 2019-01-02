project('onedrivefs', 'cpp',
        default_options : ['buildtype=release', 'b_pie=true', 'b_lto=true'])

add_project_arguments(['-fno-strict-aliasing',
                       '-fvisibility-inlines-hidden',
                       '-fvisibility=hidden',
                       '-ffunction-sections',
                       '-fdata-sections'], language : 'cpp')

libcurl_dep = dependency('libcurl', version : '>= 7.63')
jsoncpp_dep = dependency('jsoncpp', version : '>= 1.8')
fuse_dep = dependency('fuse', version : '>= 2.9')

src = ['src/appconfig.cpp',
       'src/curl.cpp',
       'src/fuse.cpp',
       'src/graph.cpp',
       'src/main.cpp',
       'src/onedrive.cpp']

vflag = ['-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), 'src/version'),
         '-Wl,-z,now,-z,noexecstack,-z,relro']

executable('onedrivefs', src,
           dependencies : [libcurl_dep, jsoncpp_dep, fuse_dep],
           link_args : vflag)
