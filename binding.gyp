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
        ['OS=="linux" and no_brcm_videocore==0', {
          'sources': [
            'src/rpi/gles2rpiimpl.cc',
            'src/bindings.cc',
            'src/gles2platform.cc',
            'src/interface/webgl.cc'
          ],
          'libraries': ['/opt/vc/lib/libGLESv2.so', '/opt/vc/lib/libEGL.so', '/opt/vc/lib/libbcm_host.so'],
          'include_dirs': ['/opt/vc/include']
        }],
        ['OS=="linux" and no_brcm_videocore==1', {
          'sources': [
            'src/glew/gles2glewimpl.cc',
            'src/bindings.cc',
            'src/gles2platform.cc',
            'src/interface/webgl.cc'
          ],
          'libraries': ['<!@(pkg-config --libs glfw3 glew)']
        }],
        ['OS=="mac"', {
          'sources': [
            'src/glew/gles2glewimpl.cc',
            'src/bindings.cc',
            'src/gles2platform.cc',
            'src/interface/webgl.cc'
          ],
          'include_dirs': [ '<!@(pkg-config glfw3 glew --cflags-only-I | sed s/-I//g)'],
          'libraries': [ '<!@(pkg-config --libs glfw3 glew)', '-framework OpenGL'],
          'library_dirs': ['/usr/local/lib'],
        }],
        ['OS=="win"', {
          'libraries': [
            'glfw3dll.lib',
            'glew32.lib',
            'opengl32.lib'
          ],
          'defines' : [
            'WIN32_LEAN_AND_MEAN',
            'VC_EXTRALEAN'
          ],
          'msvs_settings' : {
            'VCCLCompilerTool' : {
              'AdditionalOptions' : ['/O2','/Oy','/GL','/GF','/Gm-','/EHsc','/MT','/GS','/Gy','/GR-','/Gd']
            },
            'VCLinkerTool' : {
              'AdditionalOptions' : ['/OPT:REF','/OPT:ICF','/LTCG']
            },
          }
        }]
      ]
    }
  ]
}
