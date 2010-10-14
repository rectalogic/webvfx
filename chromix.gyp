{
  'targets': [
    {
      'target_name': 'chromix',
      'type': 'executable',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '<(DEPTH)/third_party/WebKit/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'sources': [
        'src/main.cc',
        'src/main_mac.mm',
        'src/main_linux.cc',
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
  ]
}
