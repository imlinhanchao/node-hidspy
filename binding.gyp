{
  "targets": [
    {
      "target_name": "hidspy",
      "sources": [
        "src/usbspy.h",
        "src/usbspy.cpp"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      'library_dirs': ['./lib/'],
      'conditions': [
        ['OS=="win"',
          {
            'sources': [
              "src/usbspy_win.cpp"
            ],
            'include_dirs+': []
          }
        ]
      ]
    }
  ]
}
