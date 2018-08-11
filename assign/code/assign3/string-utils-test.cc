#include <assert.h>
#include "string-utils.h"

void testStartsWith() {
  assert(startsWith("abc", "ab"));
  assert(startsWith("abc", "abc"));
  assert(startsWith("abc", ""));
  assert(!startsWith("ab", "abc"));
  assert(!startsWith("ab", "xy"));

  char const * test = "abc";
  assert(startsWith(test, "ab"));
  cout << "[testStartsWith] :: \t PASSED" << endl;
}

void testToLowerCase() {
  assert(toLowerCase("") == "");
  assert(toLowerCase("a") == "a");
  assert(toLowerCase("A") == "a");
  assert(toLowerCase("abc123") == "abc123");
  assert(toLowerCase("ABC123") == "abc123");
  assert(toLowerCase("UPPER_CASE") == "upper_case");
  cout << "[testToLowerCase] ::  \t PASSED" << endl;
}

void testRtrim() {
  assert(rtrim("abc") == "abc");
  assert(rtrim(" abc") == " abc");
  assert(rtrim("abc ") == "abc");
  assert(rtrim(" abc ") == " abc");
  assert(rtrim("  abc   ") == "  abc");
  assert(rtrim("  ") == "");
  assert(rtrim("   a  b c  ") == "   a  b c");
  assert(rtrim(" \r\nasd asd\n\r\n") == " \r\nasd asd");
  cout << "[testRtrim] ::  \t PASSED" << endl;
}

void testTrim() {
  assert(trim("abc") == "abc");
  assert(trim(" abc") == "abc");
  assert(trim("abc ") == "abc");
  assert(trim(" abc ") == "abc");
  assert(trim("  abc   ") == "abc");
  assert(trim("  ") == "");
  assert(trim("   a  b c  ") == "a  b c");
  assert(trim(" \r\nasd asd\n\r\n") == "asd asd");
  cout << "[testTrim] ::  \t\t PASSED" << endl;
}

void testJoin() {
  assert(join({}, "") == "");
  assert(join({}, "-") == "");
  assert(join({"abc"}, "-") == "abc");
  assert(join({"abc", "abc"}, "-") == "abc-abc");
  assert(join({"abc", "abc"}, "---") == "abc---abc");
  cout << "[testJoin] ::  \t\t PASSED" << endl;
}

int main(int argc, char *argv[]) {
  testStartsWith();
  testToLowerCase();
  testRtrim();
  testTrim();
  testJoin();
}