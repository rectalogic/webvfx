{
  'targets': [
    {
      'target_name': 'MixKit',
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
        'MixKit.mm',
        'MixManager.cc',
        'MixManager.h',
        'WebKitClientImpl.h',
        'WebViewDelegate.cc',
        'WebViewDelegate.h',
        'ImageExtension.cc',
        'ImageExtension.h',
      ],
    },
  ]
}
