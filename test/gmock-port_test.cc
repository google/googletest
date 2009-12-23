// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: vladl@google.com (Vlad Losev)

// Google Mock - a framework for writing C++ mock classes.
//
// This file tests the internal cross-platform support utilities.

#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

// NOTE: if this file is left without tests for some reason, put a dummy
// test here to make references to symbols in the gtest library and avoid
// 'undefined symbol' linker errors in gmock_main:
//
// TEST(DummyTest, Dummy) {}

namespace testing {
namespace internal {
// Needed to avoid name collisions in gmock_all_test.cc.
namespace gmock_port_test {

class Base {
 public:
  // Copy constructor and assignment operator do exactly what we need, so we
  // use them.
  Base() : member_(0) {}
  explicit Base(int n) : member_(n) {}
  virtual ~Base() {}
  int member() { return member_; }

 private:
  int member_;
};

class Derived : public Base {
 public:
  explicit Derived(int n) : Base(n) {}
};

TEST(ImplicitCastTest, ConvertsPointers) {
  Derived derived(0);
  EXPECT_TRUE(&derived == ::testing::internal::implicit_cast<Base*>(&derived));
}

TEST(ImplicitCastTest, CanUseInheritance) {
  Derived derived(1);
  Base base = ::testing::internal::implicit_cast<Base>(derived);
  EXPECT_EQ(derived.member(), base.member());
}

class Castable {
 public:
  Castable(bool* converted) : converted_(converted) {}
  operator Base() {
    *converted_ = true;
    return Base();
  }

 private:
  bool* converted_;
};

TEST(ImplicitCastTest, CanUseNonConstCastOperator) {
  bool converted = false;
  Castable castable(&converted);
  Base base = ::testing::internal::implicit_cast<Base>(castable);
  EXPECT_TRUE(converted);
}

class ConstCastable {
 public:
  ConstCastable(bool* converted) : converted_(converted) {}
  operator Base() const {
    *converted_ = true;
    return Base();
  }

 private:
  bool* converted_;
};

TEST(ImplicitCastTest, CanUseConstCastOperatorOnConstValues) {
  bool converted = false;
  const ConstCastable const_castable(&converted);
  Base base = ::testing::internal::implicit_cast<Base>(const_castable);
  EXPECT_TRUE(converted);
}

class ConstAndNonConstCastable {
 public:
  ConstAndNonConstCastable(bool* converted, bool* const_converted)
      : converted_(converted), const_converted_(const_converted) {}
  operator Base() {
    *converted_ = true;
    return Base();
  }
  operator Base() const {
    *const_converted_ = true;
    return Base();
  }

 private:
  bool* converted_;
  bool* const_converted_;
};

TEST(ImplicitCastTest, CanSelectBetweenConstAndNonConstCasrAppropriately) {
  bool converted = false;
  bool const_converted = false;
  ConstAndNonConstCastable castable(&converted, &const_converted);
  Base base = ::testing::internal::implicit_cast<Base>(castable);
  EXPECT_TRUE(converted);
  EXPECT_FALSE(const_converted);

  converted = false;
  const_converted = false;
  const ConstAndNonConstCastable const_castable(&converted, &const_converted);
  base = ::testing::internal::implicit_cast<Base>(const_castable);
  EXPECT_FALSE(converted);
  EXPECT_TRUE(const_converted);
}

class To {
 public:
  To(bool* converted) { *converted = true; }  // NOLINT
};

TEST(ImplicitCastTest, CanUseImplicitConstructor) {
  bool converted = false;
  To to = ::testing::internal::implicit_cast<To>(&converted);
  EXPECT_TRUE(converted);
}

}  // namespace gmock_port_test
}  // namespace internal
}  // namespace testing
