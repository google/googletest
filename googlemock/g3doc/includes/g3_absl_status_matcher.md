### absl::Status

In namespace `testing::status`:

<a name="table22"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `IsOk()` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` whose status is OK. </td>
  </tr>
  <tr>
    <td> `IsOkAndHolds(m)` </td>
    <td>
      `argument` is an `absl::StatusOr<T>` whose status is OK and whose inner value matches matcher `m`.
      See also [`ASSERT_OK_AND_ASSIGN`](http://google3/testing/base/public/gmock_utils/status-matchers.h?q=symbol:ASSERT_OK_AND_ASSIGN).
    </td>
  </tr>
  <tr>
    <td> `StatusHasPayload()` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` whose status is not OK and which has any payload. </td>
  </tr>
  <tr>
    <td> `StatusHasPayload<ProtoType>()` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` whose status is not OK and which has a payload of type `ProtoType`. </td>
  </tr>
  <tr>
    <td> `StatusHasPayload<ProtoType>(m)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` whose status is not OK and which has a payload of type `ProtoType` that matches `m`. </td>
  </tr>
  <tr>
    <td> `StatusIs(s, c, m)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` where: the error space matches `s`, the status code matches `c`, and the error message matches `m`. </td>
  </tr>
  <tr>
    <td> `StatusIs(c, m)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` where: the error space is canonical, the status code matches `c`, and the error message matches `m`. </td>
  </tr>
  <tr>
    <td> `StatusIs(c)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` where: the error space is canonical, and the status code matches `c`. </td>
  </tr>
  <tr>
    <td> `CanonicalStatusIs(c, m)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` where: the canonical status code matches `c`, and the error message matches `m`. </td>
  </tr>
  <tr>
    <td> `CanonicalStatusIs(c)` </td>
    <td> `argument` is a `absl::Status` or `absl::StatusOr<T>` where: the canonical status code matches `c`. </td>
  </tr>
</table>

The two- and one-argument version of StatusIs use util::GetErrorSpaceForEnum to
determine the error space. If the error code matcher is not an enum with an
associated ErrorSpace, then the canonical space will be used.
