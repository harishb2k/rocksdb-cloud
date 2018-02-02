// Copyright (c) 2017 Rockset

#pragma once

#include <string>

//
// These are inlined methods to deal with pathnames and filenames.

namespace {

// trim from start
static std::string& ltrim(std::string& s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static std::string& rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          s.end());
  return s;
}

// trim from both ends
inline std::string& trim(std::string& s) { return ltrim(rtrim(s)); }

// Extract basename from a full pathname
inline std::string basename(std::string const& pathname) {
  auto pos = pathname.rfind('/');
  if (pos == std::string::npos) {
    return pathname;
  }
  return pathname.substr(pos + 1);
}
inline std::string dirname(std::string const& pathname) {
  auto pos = pathname.rfind('/');
  if (pos == std::string::npos) {
    return "";
  }
  return pathname.substr(0, pos) + "/";
}

// If the last char of the string is the specified character, then return a
// string that has the last character removed.
inline std::string rtrim_if(std::string s, char c) {
  if (s.length() > 0 && s[s.length() - 1] == c) {
    s.erase(s.begin() + s.size() - 1);
  }
  return s;
}
// If the first char of the string is the specified character, then return a
// string that has the first character removed.
inline std::string ltrim_if(std::string s, char c) {
  if (s.length() > 0 && s[0] == c) {
    s.erase(0, 1);
  }
  return s;
}

// Returns true if 'value' has a suffix of 'ending'
inline bool ends_with(std::string const& value, std::string const& ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// pathaname seperator
const std::string pathsep = "/";

// types of rocksdb files
const std::string sst = ".sst";
const std::string ldb = ".ldb";
const std::string log = ".log";

// Is this a sst file, i.e. ends in ".sst" or ".ldb"
inline bool IsSstFile(const std::string& pathname) {
  if (pathname.size() < sst.size()) {
    return false;
  }
  const char* ptr = pathname.c_str() + pathname.size() - sst.size();
  if ((memcmp(ptr, sst.c_str(), sst.size()) == 0) ||
      (memcmp(ptr, ldb.c_str(), ldb.size()) == 0)) {
    return true;
  }
  return false;
}

// A log file has ".log" suffix
inline bool IsWalFile(const std::string& pathname) {
  if (pathname.size() < log.size()) {
    return false;
  }
  const char* ptr = pathname.c_str() + pathname.size() - log.size();
  if (memcmp(ptr, log.c_str(), log.size()) == 0) {
    return true;
  }
  return false;
}

inline bool IsManifestFile(const std::string& pathname) {
  // extract last component of the path
  std::string fname;
  size_t offset = pathname.find_last_of(pathsep);
  if (offset != std::string::npos) {
    fname = pathname.substr(offset + 1, pathname.size());
  } else {
    fname = pathname;
  }
  if (fname.find("MANIFEST") == 0) {
    return true;
  }
  return false;
}

inline bool IsIdentityFile(const std::string& pathname) {
  // extract last component of the path
  std::string fname;
  size_t offset = pathname.find_last_of(pathsep);
  if (offset != std::string::npos) {
    fname = pathname.substr(offset + 1, pathname.size());
  } else {
    fname = pathname;
  }
  if (fname.find("IDENTITY") == 0) {
    return true;
  }
  return false;
}

// A log file has ".log" suffix
inline bool IsLogFile(const std::string& pathname) {
  return IsWalFile(pathname);
}
}
