#### Mock(able) Files {#MockableFile}

Don't use Mockable Files except to simulate errors on File. For general testing
of File, see go/file-testing.

google3/file/testing/mockablefile/mockablefile.h defines a `MockableFile` class.
It wraps an arbitrary `File` object and makes its virtual methods "mockable",
meaning that by default they'll delegate to the underlying `File` while you have
the option to call `ON_CALL` or `EXPECT_CALL` to set expectations on them and/or
change their behavior. This gives you the best part of both a mock and a real
object:

*   The methods all have a working, default implementation. This can greatly
    reduce the amount of work needed to set up the mock object.
*   By setting expectations on the methods using `EXPECT_CALL`, you can easily
    test how your code uses the `File`.
*   By changing the methods' behavior (using `ON_CALL`), you can easily simulate
    file errors and thus test your error handling code.

`mockablefile.h` contains copious comments on the usage, and
[`mockablefile_test.cc`](http://google3/file/testing/mockablefile/mockablefile_test.cc)
in the same directory contains some complete examples. Here's one of them,
showing how to simulate `Write()` errors:

```cpp
#include "file/base/path.h"
#include "file/testing/mockablefile/mockablefile.h"

using ::file::testing::MockableFile;
using ::testing::_;
using ::testing::DoDefault;
using ::testing::Return;

// This test demonstrates using MockableFile to test code that handles
// File operation errors.  We want to test that WriteToFile() returns
// false when there is a Write() failure.  It's hard to cause such an
// error using a real File object, but easy to make MockableFile
// simulate it.
TEST(SampleTest, SimulateFileError) {
  // Creates a mockable_file object from a real File object.  The real
  // file is a local file in this example, but can also be any other
  // type of File.
  MockableFile* const mockable_file = new MockableFile(
      File::Create(file::JoinPath(FLAGS_test_tmpdir, "/test"), "w"));

  // Tells the mockable file to start failing from the second Write()
  // operation on.
  EXPECT_CALL(*mockable_file, Write)
      // By default, calls are delegated to the real File object.
      .WillOnce(DoDefault())
      // Simulates a write error from the second time on.
      .WillRepeatedly(Return(util::UnknownError("message")));

  // Exercises the code we want to test, letting it talk to the
  // MockableFile object instead of a real one.
  EXPECT_FALSE(WriteToFile(mockable_file));
}
```

`mockablefile.h` also defines a `MockableFileSystem` class that allows you to
register mockable files in the file system under the `/mockable` mount point,
which can then be opened by your code by name. Since `MockableFile` can wrap
**any** type of file, this means you can inject **any** type of file into the
file system for testing. For example, `google3/file/memfile/memfile.h` defines a
convenient in-memory file type `MutableStringFile`. Now, you can wrap a
`MutableStringFile` in a `MockableFile` and inject it using `MockableFileSystem`
in order to test error handling of File operations that want to open a file
themselves.

```cpp
#include "file/memfile/memfile.h" // you also need to depend on //file/memfile:memfile in your BUILD file
#include "file/testing/mockablefile/mockablefile.h"

using ::file::testing::MockableFile;
using ::file::testing::MockableFileSystem;
using ::testing::_;
using ::testing::DoDefault;
using ::testing::Return;

// This test demonstrates registering a MockableFile with (a.k.a.
// injecting it into) the file system and opening it by name later.
// We want to test that WriteToFileByName() returns false when there
// is a Write() failure.
TEST(SampleTest, RegisterMockableFile) {
  // Creates a mockable_file from a MutableStringFile.
  MockableFile* const mockable_file = new MockableFile(
      MutableStringFile("/foo/bar", new string,
                        TAKE_OWNERSHIP, DO_NOT_ALLOW_MMAP));

  // Creates a mockable file system so that we can inject
  // mockable_file into it.
  MockableFileSystem fs;

  // Injects mockable_file as "/mockable/foo/bar".
  const string kPath = "/mockable/foo/bar";
  EXPECT_CALL(fs, CreateFile(kPath, "w", _, _, _))
      .WillOnce(Return(mockable_file));

  // Tells the mockable file to start failing from the second Write()
  // operation on.
  EXPECT_CALL(*mockable_file, Write)
      // By default, calls are delegated to the real File object.
      .WillOnce(DoDefault())
      // Simulates a write error from the second time on.
      .WillRepeatedly(Return(util::error::UNKNOWN));

  // Exercises the code we want to test, letting it talk to the
  // MockableFile object instead of a real one.
  EXPECT_FALSE(WriteToFileByName(kPath));
}
```

#### Mock Network System Calls

Gary Morain (gmorain@) implemented mock network system calls such that you can
use gMock to control their behavior when testing code that invokes network
system calls. You can find the code here:

*   google3/net/util/network_system_call_interface.h - the interface.
*   google3/net/util/network_system_call.h - the real implementation.
*   google3/net/util/network_system_call_mock.h - the mock implementation.
*   google3/net/util/network_system_call_unittest.cc - the unit test and demo.

#### Mock Bigtable

Please see the
[Svala](https://sites.google.com/a/google.com/te-zrh/tools--technologies/gmock-bigtable)
project for a gMock-based Bigtable implementation.

#### Add Yours Here

Don't be shy! If you've created a mock class using gMock and think it would be
useful to other Googlers, write an entry about it on this wiki page so that
people can learn about it.
