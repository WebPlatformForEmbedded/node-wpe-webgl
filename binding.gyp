{
  'targets': [
    {
      'target_name': 'gles2',
      'defines': [
        'VERSION=0.0.1'
      ],
      'variables': {
        'platform': '<(OS)',
        'no_brcm_videocore%': '<!(ls /opt/vc/lib 2>&1 >/dev/null | wc -l)'
      },
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        '<(module_root_dir)/deps/include',
      ],
      'conditions': [
        [
          'no_brcm_videocore==1', {
            'sources': [
              'src/glew/gles2glewimpl.cc',
              'src/bindings.cc',
              'src/gles2platform.cc',
              'src/interface/webgl.cc'
            ],
            'libraries': ['<!@(pkg-config --libs glfw3 glew)']
          }, {
            'sources': [
              'src/rpi/gles2rpiimpl.cc',
              'src/bindings.cc',
              'src/gles2platform.cc',
              'src/interface/webgl.cc'
            ],
            'libraries': ['/opt/vc/lib/libGLESv2.so', '/opt/vc/lib/libEGL.so', '/opt/vc/lib/libbcm_host.so'],
            'include_dirs': ['/opt/vc/include']
          }
        ]
      ],

    }
  ]
}
