{
    'targets': [
        {
            'target_name': 'googletest',
            'type': 'static_library',
            'sources': [
                './googletest/src/gtest-all.cc'
            ],
            'include_dirs': [
                './googletest/include',
                './googletest',
            ],
            'link_settings': {
                'libraries': [
                    '-pthread'
                ],
            },
            'direct_dependent_settings': {
                'include_dirs': [
                    './googletest/include',
                ]
            }
        },
        {
            'target_name': 'googlemock',
            'type': 'static_library',
            'sources': [
                './googlemock/src/gmock-all.cc'
            ],
            'include_dirs': [
                './googlemock/include',
                './googlemock',
            ],
            'link_settings': {
                'libraries': [
                    '-pthread'
                ],
            },
            'direct_dependent_settings': {
                'include_dirs': [
                    './googlemock/include',
                ]
            },
            'dependencies': [
                'googletest'
            ],
            'export_dependent_settings': [
                'googletest'
            ]
        }
    ]
}
