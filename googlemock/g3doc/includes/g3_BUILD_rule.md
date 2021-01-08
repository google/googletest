## BUILD Rule

Add *one* of the following to your `deps`:

```build
"//testing/base/public:gunit",
"//testing/base/public:gtest_main",
```

Add this to your `.cc` file:

```cpp
#include "gmock/gmock.h"
```

Unless noted, *all functions and classes* are defined in the `::testing`
namespace.
