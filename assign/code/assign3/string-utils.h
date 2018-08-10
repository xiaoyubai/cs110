#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static string join(const vector<string>& strs, const string& delim) {
  string res;
  if (strs.empty()) return "";
  res += strs[0];
  for (size_t i=1; i<strs.size(); i++) {
    res += delim;
    res += strs[i];
  }
  return res;
}

static bool startsWith(const string& a, const string& b) {
  if (a.length() < b.length()) return false;
  for (size_t i=0; i<b.length(); i++)
    if (a[i] != b[i])
      return false;
  return true;
}

static string toLowerCase(const string& s) {
  string res;
  for (char c: s) res += tolower(c);
  return res;
}

static string rtrim(const string& s) {
  size_t end;
  for (end=s.length()-1; end>=0; end--) {
    if (s[end] != ' ') break;
  }
  size_t len = end+1;
  string res = s.substr(0, len);
  return res;
}

static const string trim(const string& s) {
  size_t start, end, len;
  for (start=0; start<s.length(); start++) {
    if (s[start] != ' ') break;
  }
  for (end=s.length()-1; end>=0; end--) {
    if (s[end] != ' ') break;
  }
  len = end-start+1;
  string res = s.substr(start, len);
  return res;
}
