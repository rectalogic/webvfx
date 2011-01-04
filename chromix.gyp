# Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'chromix',
      'type': '<(library)',
      'include_dirs': [
        '.',
      ],
      'dependencies': [
        '<(DEPTH)/third_party/WebKit/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/WebCore/WebCore.gyp/WebCore.gyp:webcore',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'conditions': [
        ['OS == "linux"', {
          'dependencies': [
            '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
            '<(DEPTH)/gfx/gfx.gyp:gfx',
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
        }],
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
        'chromix/ChromixExtension.h',
        'chromix/ChromixExtension.cc',
        'chromix/Delegate.h',
        'chromix/Delegate.cc',
        'chromix/ImageMap.h',
        'chromix/ImageMap.cc',
        'chromix/Loader.h',
        'chromix/Loader.cc',
        'chromix/MixRender.h',
        'chromix/MixRender.cc',
        'chromix/ScriptingSupport.h',
        'chromix/ScriptingSupport.cc',
        'chromix/WebKitClientImpl.h',
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
