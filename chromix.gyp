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
      'include_dirs': [
        '.',
      ],
      'sources': [
        'chromix/MixKit.h',
        'chromix/MixKitPrivate.h',
        'chromix/MixKit.cc',
        'chromix/MixRender.h',
        'chromix/MixRender.cc',
        'chromix/Loader.h',
        'chromix/Loader.cc',
        'chromix/ParameterMap.h',
        'chromix/ParameterMap.cc',
        'chromix/Parameter.h',
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
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'chromix',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        'test/main.cc',
        'test/main_mac.mm',
        'test/main_linux.cc',
      ],
    },
  ]
}
