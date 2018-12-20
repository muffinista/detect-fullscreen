{
  "targets": [
    {
      "target_name": "Fullscreen",
      "sources": [ "Fullscreen.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      'cflags': [
        '-Wall',
        '-Wparentheses',
        '-Winline',
        '-Wbad-function-cast',
        '-Wdisabled-optimization'
      ],
      'conditions': [
        ['OS == "mac"', {
          'defines': ['IS_MAC'],
          'include_dirs': [
            'System/Library/Frameworks/Carbon.Framework/Headers'
          ],
          'link_settings': {
            'libraries': [
              '-framework Carbon'
            ]
          }
        }],
        ["OS=='win'", {
          'defines': ['IS_WINDOWS']
        }]
      ]
    }
  ]
}
