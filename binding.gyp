{
  'targets': [
    {
      'target_name': 'gles2',
      'defines': [
        'VERSION=0.0.1'
      ],
      'variables': {
        'no_brcm_videocore%': '<!(ls /opt/vc/lib 2>&1 >/dev/null | wc -l)'
      },
      'sources': [
          'src/bindings.cc',
          'src/gles2platform.cc',
          'src/interface/webgl.cc'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        '<(module_root_dir)/deps/include',
      ],
      'conditions': [
        [
          'no_brcm_videocore==1', {
            'sources': [
              'src/x11/gles2x11impl.cc'
            ],
            'libraries': ['-lX11', '-lEGL', '-lGLESv2']
          }, {
            'sources': [
              'src/rpi/gles2rpiimpl.cc'
            ],
            'libraries': ['/opt/vc/lib/libGLESv2.so', '/opt/vc/lib/libEGL.so', '/opt/vc/lib/libbcm_host.so'],
            'include_dirs': ['/opt/vc/include']
          }
        ]
      ],

    }
  ]
}
