{
  'targets': [
    {
      'target_name': 'chromix',
      'type': 'executable',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '<(DEPTH)/app/app.gyp:app_base',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/gpu/gpu.gyp:gles2_c_lib',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
        '<(DEPTH)/webkit/support/webkit_support.gyp:appcache',
        '<(DEPTH)/webkit/support/webkit_support.gyp:glue',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'sources': [
        'main.cc',
        'main_mac.mm',
        'main_linux.cc',
        'MixKit.h',
        'MixKitPrivate.h',
        'MixKit.cc',
        'MixRender.h',
        'MixRender.cc',
        'MixRenderLoader.h',
        'MixRenderLoader.cc',
        'MixParameterMap.h',
        'MixParameterMap.cc',
        'Parameter.h',
        'Parameter.cc',
        'WebKitClientImpl.h',
        'ChromixExtension.h',
        'ChromixExtension.cc',
      ],
    },
  ]
}
