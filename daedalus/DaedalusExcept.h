#pragma once

#include <vector>
#include <string>
#include <stdexcept>

namespace Daedalus {

class DaedalusExcept : public std::runtime_error {
  public:
    DaedalusExcept(const char* why,std::vector<std::string>&& callstack);

    std::vector<std::string> callstack;
  };

class BadMath : public DaedalusExcept {
  public:
    BadMath(std::vector<std::string>&& callstack);
  };

class InconsistentState : public DaedalusExcept {
  public:
    InconsistentState(std::vector<std::string>&& callstack);
  };

class InvalidCall : public DaedalusExcept {
  public:
    InvalidCall(std::vector<std::string>&& callstack);
  };
}

