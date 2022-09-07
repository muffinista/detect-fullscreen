{
  "targets": [
    {
      "target_name": "Fullscreen",
      "sources": [ "src/fullscreen.cc" ],
      'cflags': [
        '-Wall',
        '-Wparentheses',
        '-Winline',
        '-Wdisabled-optimization'
      ],
      'conditions': [
        ['OS == "mac"', {
          'sources': [
            'src/mac/fullscreen.mm'
          ],
          'include_dirs': [
            'System/Library/Frameworks/Carbon.Framework/Headers'
          ],
          'link_settings': {
            'libraries': [
              '-framework Carbon',
              '-framework AppKit',
            ]
          },
          'xcode_settings': {
            'OTHER_CFLAGS': [
                '-ObjC++'
            ]
          }
        }],
        ["OS=='win'", {
          'sources': [
            'src/win/fullscreen.cc'
          ]
        }],
        ["OS=='linux'", {
          'sources': [
            'src/unsupported/fullscreen.cc'
          ]
        }]
      ]
    }
  ]
}
