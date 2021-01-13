### Useful Matchers Defined Outside of gMock

#### std::tuple

**deps:** `"//util/tuple:matchers"` <br>
`#include "util/tuple/matchers.h"` <br>
In namespace `util::tuple::testing`:

<a name="table21"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `Tuple(m0, m1, ..., mn)` </td>
    <td> `argument` is a `std::tuple` const reference with `n + 1` elements, where the i-th element matches `std::get*(argument)` </td>
  </tr>
  <tr>
    <td> `FieldPairsAre(m0, m1, ..., mn)` </td>
    <td> matches a pair (2-tuple) of tuples where matcher `mi` matches the i-th fields of the tuples; usually for use inside `Pointwise()` or `UnorderedPointwise()` </td>
  </tr>
</table>

#### Web

**deps:** `"//webutil/url:test_utils"` <br>
`#include "webutil/url/test_utils.h"`

<a name="table23"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `webutil_url::testing::HasUrlArg(key)` </td>
    <td> `argument` is a URL string that has a query argument whose name is `key`. E.g. `http://foo.com/bar?key=value` </td>
  </tr>
  <tr>
    <td> `webutil_url::testing::HasUrlArg(key, m)` </td>
    <td> `argument` is a URL string that has a query argument whose name is `key` and whose value matches `m`. </td>
  </tr>
  <tr>
    <td> `webutil_url::testing::HasUrlPathParam(key)` </td>
    <td> `argument` is a URL string that has a path parameter whose name is `key`. E.g. `http://foo.com/bar;key=value` </td>
  </tr>
  <tr>
    <td> `webutil_url::testing::HasUrlPathParam(key, m)` </td>
    <td> `argument` is a URL string that has a path parameter whose name is `key` and whose value matches `m`. </td>
  </tr>
</table>

**deps:** `"//third_party/jsoncpp:testing"` <br>
`#include "third_party/jsoncpp/testing.h"`

<a name="table24"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `Json::testing::EqualsJson(json)` </td>
    <td> `argument` is a string that represents the same Json value as the `json` string does. </td>
  </tr>
</table>

#### Encoding

**deps:** `"//testing/lib/util/coding:varint"` <br>
`#include "testing/lib/util/coding/varint.h"`

<a name="table25"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `testing_lib_util_coding::EncodesVarint64(n)` </td>
    <td> `argument` is a string that represents a Varint encoding of `n`, a `uint64` value. </td>
  </tr>
</table>

#### XPath

**deps:** `"//template/prototemplate/testing:xpath_matcher"` <br>
`#include "template/prototemplate/testing/xpath_matcher.h"`

<a name="table26"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `prototemplate::testing::MatchesXPath(str)` </td>
    <td> `argument` is a well-formed HTML/XML string that matches the given [XPath](http://www.w3.org/TR/xpath/#contents) expression. </td>
  </tr>
</table>

#### Flume

**deps:** `"//pipeline/flume/contrib:matchers"` <br>
`#include "pipeline/flume/contrib/matchers.h"`

<a name="table27"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `flume::testing::Kv(key_matcher, value_matcher)` </td>
    <td> `argument` is a `KV` where the key matches `key_matcher` and the value matches `value_matcher`. </td>
  </tr>
</table>

#### i18n strings

**deps:** `"///i18n/testing/public:expecti18n"` <br>
`#include "google3/i18n/testing/public/expecti18n.h"`

<a  name="table28"></a>
<table border="1" cellspacing="0" cellpadding="1">
  <tr>
    <td> `i18n_testing::I18nEq(utf8)` </td>
    <td> `argument` is a `absl::string_view` whose content matches `utf8` allowing for locale data changes.
             In case it does not match, the error message contains both a readable version of both strings and the list of
            decoded codepoints.</td>
  </tr>
</table>
