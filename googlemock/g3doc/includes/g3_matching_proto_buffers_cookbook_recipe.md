#### Matching Protocol Buffers

Many Google APIs pass protocol buffers around. gMock provides some matchers for
protocol buffers. You can use them to specify that an argument must be equal (or
equivalent) to a given protocol buffer.

`EqualsProto(proto_buffer)` matches an argument iff it's equal to
`proto_buffer`, as determined by the `Equals()` method of the argument. The
argument must be a protocol buffer; pointers must be dereferenced.

Sometimes we want to test for equivalence instead of equality, i.e. we want to
use the `Equivalent()` method to compare two protocol buffers. For this we can
use `EquivToProto(proto_buffer)`.

It's worth noting that all of the matchers we mention here make a copy of
`proto_buffer`. This means that you can use a matcher even if the original
protocol buffer used for creating the matcher has been destroyed. Just one less
thing for you to worry about!

Note that `EqualsProto` and `EquivToProto` work for both proto1 and proto2. They
are declared in `gmock.h`, so you do not have to include other files. See
go/protomatchers for more proto buffer matching goodies.

In addition: One application of `Property()` is testing protocol buffers:

<a name="table1"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `Property(&MyProto::has_size, true)` </td>
    <td> Matches `proto` where `proto.has_size()` returns `true`. </td>
  </tr>
  <tr>
    <td> `Property(&MyProto::size, Gt(5))` </td>
    <td> Matches `proto` where `proto.size()` is greater than 5. </td>
  </tr>
</table>
