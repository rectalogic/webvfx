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
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/third_party/WebKit/Source/WebCore/WebCore.gyp/WebCore.gyp:webcore',
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
        '<(DEPTH)/third_party/WebKit/Source/WebCore/WebCore.gyp/WebCore.gyp:webcore',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
        ],
      },
      'sources': [
        'chromix/chromix.h',
        'chromix/chromix.cc',
        'chromix/chromix_extension.h',
        'chromix/chromix_extension.cc',
        'chromix/delegate.h',
        'chromix/delegate.cc',
        'chromix/image_map.h',
        'chromix/image_map.cc',
        'chromix/loader.h',
        'chromix/loader.cc',
        'chromix/mix_render.h',
        'chromix/mix_render.cc',
        'chromix/scripting_support.h',
        'chromix/scripting_support.cc',
        'chromix/webkit_client_impl.h',
      ],
    },
    {
      'target_name': 'chromix_demo',
      'type': 'executable',
      'dependencies': [
        'chromix',
      ],
      'sources': [
        'demo/main.cc',
        'demo/main_mac.mm',
        'demo/main_linux.cc',
      ],
    },
  ]
}
