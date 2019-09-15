#include "DaedalusExcept.h"

using namespace Daedalus;

DaedalusExcept::DaedalusExcept(const char *why, std::vector<std::string> &&callstack)
  :std::runtime_error(why),callstack(std::move(callstack)){
  }

BadMath::BadMath(std::vector<std::string> &&callstack)
  :DaedalusExcept("bad math",std::move(callstack)){
  }

InconsistentState::InconsistentState(std::vector<std::string> &&callstack)
  :DaedalusExcept("inconsistent state",std::move(callstack)){
  }

InvalidCall::InvalidCall(std::vector<std::string> &&callstack)
  :DaedalusExcept("bad function call",std::move(callstack)){
  }
