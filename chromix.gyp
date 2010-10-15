{
  'targets': [
    {
      'target_name': 'chromix',
      'type': '<(library)',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
        '<(DEPTH)/third_party/WebKit/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'sources': [
        'src/MixKit.h',
        'src/MixKitPrivate.h',
        'src/MixKit.cc',
        'src/MixRender.h',
        'src/MixRender.cc',
        'src/Loader.h',
        'src/Loader.cc',
        'src/ParameterMap.h',
        'src/ParameterMap.cc',
        'src/Parameter.h',
        'src/Parameter.cc',
        'src/ScriptingSupport.h',
        'src/ScriptingSupport.cc',
        'src/WebKitClientImpl.h',
        'src/ChromixExtension.h',
        'src/ChromixExtension.cc',
      ],
    },
    {
      'target_name': 'chromix_test',
      'type': 'executable',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'chromix',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
      ],
      'include_dirs': [
        'src',
      ],
      'sources': [
        'test/main.cc',
        'test/main_mac.mm',
        'test/main_linux.cc',
      ],
    },
  ]
}
