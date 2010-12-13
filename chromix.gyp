{
  'targets': [
    {
      'target_name': 'chromix',
      'type': '<(library)',
      'variables': {
        'build_ffmpegsumo': 0,
        'disable_nacl': 1,
        'linux_use_tcmalloc': 1,
        'use_gnome_keyring': 0,
        'use_cups': 0,
        'use_gconf': 0,
        'remoting': 0,
      },
      'include_dirs': [
        '.',
      ],
      'dependencies': [
        '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
        '<(DEPTH)/third_party/WebKit/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
        ],
      },
      'sources': [
        'chromix/Chromix.h',
        'chromix/Chromix.cc',
        'chromix/MixRender.h',
        'chromix/MixRender.cc',
        'chromix/Loader.h',
        'chromix/Loader.cc',
        'chromix/ParameterMap.h',
        'chromix/ParameterMap.cc',
        'chromix/ParameterValue.h',
        'chromix/ParameterValue.cc',
        'chromix/ScriptingSupport.h',
        'chromix/ScriptingSupport.cc',
        'chromix/WebKitClientImpl.h',
        'chromix/ChromixExtension.h',
        'chromix/ChromixExtension.cc',
      ],
    },
    {
      'target_name': 'chromix_test',
      'type': 'executable',
      'dependencies': [
        'chromix',
      ],
      'sources': [
        'test/main.cc',
        'test/main_mac.mm',
        'test/main_linux.cc',
      ],
    },
  ]
}
