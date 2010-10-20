{
  'targets': [
    {
      'target_name': 'mltchromix',
      'type': 'shared_library',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'chromix.gyp:chromix',
      ],
      'cflags': [
        '<!@(pkg-config --cflags mlt-framework)',
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
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
            'OTHER_LDFLAGS': [
              '<!@(pkg-config --libs mlt-framework)',
            ],
          },
        }], # OS=="mac"
      ],
      'sources': [
        'mlt/factory.cc',
        'mlt/filter_chromix.cc',
      ],
    },
  ]
}
