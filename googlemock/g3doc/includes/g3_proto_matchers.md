#### Protocol Buffer Matchers {#ProtoMatchers}

(go/protomatchers)

In the following, `argument` can be either a protocol buffer (version 1 or
version 2) or a pointer to it, and `proto` can be either a protocol buffer or a
human-readable ASCII string representing it (e.g. `"foo: 5"`). If you need help
writing the ASCII string, read go/textformat.

<a name="table15"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `EqualsInitializedProto(proto)` </td>
    <td> `argument` is fully-initialized and equal to `proto`. </td>
  </tr>
  <tr>
    <td> `EqualsProto(proto)` </td>
    <td> `argument` is equal to `proto`. Can also be used as a multi-argument matcher; see below. </td>
  </tr>
  <tr>
    <td> `EquivToInitializedProto(proto)` </td>
    <td> `argument` is fully-initialized and equivalent to `proto`. </td>
  </tr>
  <tr>
    <td> `EquivToProto(proto)` </td>
    <td> `argument` is equivalent to `proto`. Can also be used as a multi-argument matcher; see below. </td>
  </tr>
  <tr>
    <td> `IsInitializedProto()` </td>
    <td> `argument` is a fully-initialized protocol buffer. </td>
  </tr>
</table>

Both Equiv and Equal matchers checks that two protocol buffers have identical
values, however only Equal matchers ensure that the protocol buffers fields were
set the same way (explicitly or through their default value).

When these matchers are given a string parameter, they *optionally* accept the
type of the protocol buffer as a template argument, e.g.
`EqualsProto<MyPB>("bar: 'xyz'")`.

The following *protocol buffer matcher transformers* in namespace
`::testing::proto` change the behavior of a matcher:

<a name="table16"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `Approximately(proto_matcher)` </td>
    <td> The same as `proto_matcher` except that it compares floating-point fields approximately. </td>
  </tr>
  <tr>
    <td> `Approximately(proto_matcher, margin)` </td>
    <td> The same as `Approximately(proto_matcher)` except that two floating-point fields are considered equal if their absolute difference is <= `margin`. </td>
  </tr>
  <tr>
    <td> `Approximately(proto_matcher, margin, fraction)` </td>
    <td> The same as `Approximately(proto_matcher)` except that two floating-point fields are considered equal if their absolute difference is <= `margin` or their fractional difference is <= `fraction`. </td>
  </tr>
  <tr>
    <td> `TreatingNaNsAsEqual(proto_matcher)` </td>
    <td> The same as `proto_matcher` except that two floating-point fields are considered equal if both are NaN, matching the behavior of `NanSensitiveDoubleEq()`. </td>
  </tr>
  <tr>
    <td> `IgnoringRepeatedFieldOrdering(proto_matcher)` </td>
    <td> The same as `proto_matcher` except that it ignores ordering of elements within repeated fields (see `proto2::MessageDifferencer::TreatAsSet()` for more details). </td>
  </tr>
  <tr>
    <td> `IgnoringFieldPaths({"some_field.subfield"}, proto_matcher)` </td>
    <td> The same as `proto_matcher` except that it ignores the value of field `subfield` in field `some_field`. </td>
  </tr>
  <tr>
    <td> `Partially(proto_matcher)` </td>
    <td> The same as `proto_matcher` except that only fields present in the expected protocol buffer are considered. </td>
  </tr>
  <tr>
    <td> `WhenDeserialized(typed_proto_matcher)` </td>
    <td> `argument` is a string in the protocol buffer binary format that can be deserialized to a protocol buffer matching `typed_proto_matcher`. </td>
  </tr>
  <tr>
    <td> `WhenDeserializedAs<PB>(matcher)` </td>
    <td> `argument` is a string in the protocol buffer binary format that can be deserialized to a protocol buffer of type `PB` that matches `matcher`. </td>
  </tr>
  <tr>
    <td> `WhenUnpacked(typed_proto_matcher)` </td>
    <td> `argument` is a `google.protobuf.Any` that can be unpacked into a protocol buffer of the type of `typed_proto_matcher` that matches that matcher. </td>
  </tr>
  <tr>
    <td> `WhenUnpackedTo<PB>(matcher)` </td>
    <td> `argument` is a `google.protobuf.Any` that can be unpacked into a protocol buffer of type `PB` that matches `matcher`. </td>
  </tr>
</table>

where:

*   `proto_matcher` can be any of the `Equals*` and `EquivTo*` protocol buffer
    matchers above,
*   `typed_proto_matcher` can be an `Equals*` or `EquivTo*` protocol buffer
    matcher where the type of the expected protocol buffer is known at run time
    (e.g. `EqualsProto(expected_pb)` or `EqualsProto<MyPB>("bar: 'xyz'")`).
*   `matcher` can be any matcher that can be used to match a `PB` value, e.g.
    `EqualsProto("bar: 'xyz'")`, `Not(EqualsProto(my_pb))`, or
    `Property(&MyPB::foo, Ne(5))`.

`Approximately()`, `Partially()`, and `TreatingNaNsAsEqual()` can be combined,
e.g. `Partially(Approximately(EqualsProto(foo)))`.

Note that `EqualsProto()` and `EquivToProto()` can be used as multi-argument
matchers that match a 2-tuple of protos. The following example shows how to
compare two vectors of protos.

```cpp
vector<MyProto> actual;
vector<MyProto> expected;
...  // Fill vectors with data
EXPECT_THAT(actual, Pointwise(EqualsProto(), expected));
```

Similarly, they can be used to compare a vector of protos against a vector of
strings.

```cpp
vector<MyProto> actual;
...  // Fill 'actual' with data
vector<string> expected {"foo:<bar:1>", "foo:<bar:2>"};
EXPECT_THAT(actual, Pointwise(EqualsProto(), expected));
// Or, concisely:
EXPECT_THAT(actual, Pointwise(EqualsProto(), {"foo:<bar:1>", "foo:<bar:2>"}));
```

<a name="table17"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `EqualsProto()` </td>
    <td> `x.Equals(y)` </td>
  </tr>
  <tr>
    <td> `EquivToProto()` </td>
    <td> `x.Equivalent(y)` </td>
  </tr>
</table>
