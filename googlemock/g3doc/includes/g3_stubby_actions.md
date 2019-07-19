#### Stubby Actions

gMock has the following actions to provide limited support for mocking Stubby
(go/stubby) services. You can use them to return a canned answer from a Stubby
call, which has the signature `void Method(RPC*, const Request*, Response*
response, Closure* done)`. You should consider using Service Mocker
(go/servicemocker) instead if your need is more complex.

<a name="table35"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `BeDone()` </td>
    <td> Calls the `done` closure. </td>
  </tr>
  <tr>
    <td> `FailWith(status)` </td>
    <td> Fails the RPC with the given RPC status code. </td>
  </tr>
  <tr>
    <td> `FailWithUtilStatus(util_status)` </td>
    <td> Fails the RPC with the given util::Status error code. </td>
  </tr>
  <tr>
    <td> `RespondWith(proto)` </td>
    <td> Sets the `response` argument to the given protocol buffer, and calls the `done` closure. </td>
  </tr>
  <tr>
    <td> `RespondWith(proto_string)` </td>
    <td> Sets the `response` argument to the protocol buffer parsed from the given ASCII string, and calls the `done` closure. </td>
  </tr>
</table>
