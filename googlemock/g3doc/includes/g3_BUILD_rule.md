### BUILD Rule

Add *one* of the following to your `deps`:

```build
"//testing/base/public:gunit",
"//testing/base/public:gunit_main",
```

Add this to your `.cc` file:

```cpp
#include "testing/base/public/gmock.h"
```

Unless noted, *all functions and classes* are defined in the `::testing`
namespace.

TODO: gmock_gen.py doesn't use MOCK_METHOD - update or deprecate.

You can automate this using NealNorwitz's mock generator - run
`/google/src/head/depot/google3/third_party/googletest/googlemock/scripts/generator/gmock_gen.py
*your.h ClassName*` to generate a mock for the given base class (if no class
name is given, all classes in the file are emitted).
