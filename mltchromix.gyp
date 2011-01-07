# Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'mltchromix',
      'type': 'shared_library',
      'dependencies': [
        'chromix.gyp:chromix',
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          'cflags': [
            '<!@(pkg-config --cflags mlt-framework)',
          ],
          'link_settings': {
            'ldflags': [
              '<!@(pkg-config --libs-only-L --libs-only-other mlt-framework)',
            ],
            'libraries': [
              '<!@(pkg-config --libs-only-l mlt-framework)',
            ],
          },
        }], # OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '<!@(pkg-config --cflags mlt-framework)',
            ],
            'OTHER_LDFLAGS': [
              '<!@(pkg-config --libs mlt-framework)',
            ],
            'DYLIB_INSTALL_NAME_BASE': '<!@(pkg-config --variable=libdir mlt-framework)/mlt',
          },
        }], # OS=="mac"
      ],
      'sources': [
        'mlt/chromix_context.h',
        'mlt/chromix_context_linux.cc',
        'mlt/chromix_context_mac.mm',
        'mlt/chromix_filter.cc',
        'mlt/chromix_producer.cc',
        'mlt/chromix_service.cc',
        'mlt/chromix_service.h',
        'mlt/chromix_task.cc',
        'mlt/chromix_task.h',
        'mlt/chromix_transition.cc',
        'mlt/factory.cc',
      ],
    },
  ]
}
