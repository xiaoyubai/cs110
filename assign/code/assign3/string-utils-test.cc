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
}

void testTrim() {
  assert(trim("abc") == "abc");
  assert(trim(" abc") == "abc");
  assert(trim("abc ") == "abc");
  assert(trim(" abc ") == "abc");
  assert(trim("  abc   ") == "abc");
  assert(trim("  ") == "");
  assert(trim("   a  b c  ") == "a  b c");
}

void testJoin() {
  assert(join({}, "") == "");
  assert(join({}, "-") == "");
  assert(join({"abc"}, "-") == "abc");
  assert(join({"abc", "abc"}, "-") == "abc-abc");
  assert(join({"abc", "abc"}, "---") == "abc---abc");
}

int main(int argc, char *argv[]) {
  testStartsWith();
  testTrim();
  testJoin();
}