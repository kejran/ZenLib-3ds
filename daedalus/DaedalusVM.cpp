//
// Created by andre on 09.05.16.
//

#include "DaedalusVM.h"

#include <map>
#include <cassert>
#include <algorithm>

enum {
  NUM_FAKE_STRING_SYMBOLS = 5
  };

using namespace ZenLoad;
using namespace Daedalus;

DaedalusVM::DaedalusVM(const std::vector<uint8_t> data)
  :DaedalusVM(data.data(),data.size()){
  }

DaedalusVM::DaedalusVM(const uint8_t* pDATFileData, size_t numBytes)
    : m_DATFile(pDATFileData, numBytes) {
  m_Stack.reserve(1024);
  // Make fake-strings
  for(size_t i = 0; i<NUM_FAKE_STRING_SYMBOLS; i++) {
    auto symIndex = m_DATFile.addSymbol();
    // make sure there is enough space for 1 string
    m_DATFile.getSymbolByIndex(symIndex).strData.resize(1);
    m_FakeStringSymbols.push(symIndex);
    }

  m_SelfId   = m_DATFile.getSymbolIndexByName("self");
  m_OtherId  = m_DATFile.getSymbolIndexByName("other");
  m_VictimId = m_DATFile.getSymbolIndexByName("victim");
  m_ItemId   = m_DATFile.getSymbolIndexByName("item");
  }

void DaedalusVM::eval(uint32_t PC) {
  int32_t      a=0;
  int32_t      b=0;
  uint32_t     arr=0, arr2=0;
  int32_t*     addr=nullptr;
  std::string *straddr=nullptr;

  while(true) {
    const PARStackOpCode& op = nextInstruction(PC);
    switch (op.op) {
      case EParOp_Add: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a+b);
        break;
        }
      case EParOp_Subract: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a-b);
        break;
        }
      case EParOp_Multiply: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a*b);
        break;
        }
      case EParOp_Divide:{
        a = popDataValue();
        b = popDataValue();
        if(b==0)
          terminateScript();
        pushInt(a/b);
        break;
        }
      case EParOp_Mod: {
        a = popDataValue();
        b = popDataValue();
        if(b==0)
          terminateScript();
        pushInt(a%b);
        break;
        }
      case EParOp_BinOr: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a | b);
        break;
        }
      case EParOp_BinAnd: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a & b);
        break;
        }
      case EParOp_Less: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a < b ? 1 : 0);
        break;
        }
      case EParOp_Greater: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a > b ? 1 : 0);
        break;
        }

      case EParOp_AssignFunc:
      case EParOp_Assign: {
        size_t  a = popVar(arr);
        int32_t b = popDataValue();

        int32_t& aInt = m_DATFile.getSymbolByIndex(a).getInt(arr, getCurrentInstanceDataPtr());
        aInt = b;
        break;
        }
      case EParOp_LogOr:
        a = popDataValue();
        b = popDataValue();
        pushInt((a || b) ? 1 : 0);
        break;
      case EParOp_LogAnd:
        a = popDataValue();
        b = popDataValue();
        pushInt((a && b) ? 1 : 0);
        break;
      case EParOp_ShiftLeft: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a << b);
        break;
        }
      case EParOp_ShiftRight: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a >> b);
        break;
        }
      case EParOp_LessOrEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a <= b ? 1 : 0);
        break;
        }
      case EParOp_Equal: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a == b ? 1 : 0);
        break;
        }
      case EParOp_NotEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a != b ? 1 : 0);
        break;
        }
      case EParOp_GreaterOrEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a >= b ? 1 : 0);
        break;
        }
      case EParOp_AssignAdd:{
        size_t v = popVar(arr);
        addr   = &m_DATFile.getSymbolByIndex(v).getInt(arr, getCurrentInstanceDataPtr());
        *addr += popDataValue();
        break;
        }
      case EParOp_AssignSubtract: {
        size_t v = popVar(arr);
        addr   = &m_DATFile.getSymbolByIndex(v).getInt(arr, getCurrentInstanceDataPtr());
        *addr -= popDataValue();
        break;
        }
      case EParOp_AssignMultiply: {
        size_t v = popVar(arr);
        addr = &m_DATFile.getSymbolByIndex(v).getInt(arr, getCurrentInstanceDataPtr());
        *addr *= popDataValue();
        break;
        }
      case EParOp_AssignDivide: {
        size_t v = popVar(arr);
        addr = &m_DATFile.getSymbolByIndex(v).getInt(arr, getCurrentInstanceDataPtr());
        b = popDataValue();
        if(b==0)
          terminateScript();
        *addr /= b;
        break;
        }
      case EParOp_Plus:
        pushInt(+popDataValue());
        break;
      case EParOp_Minus:
        pushInt(-popDataValue());
        break;
      case EParOp_Not:
        pushInt(!popDataValue());
        break;
      case EParOp_Negate:
        pushInt(~popDataValue());
        break;
      case EParOp_Ret:
        return;
      case EParOp_Call: {
        auto currentInstance = m_Instance;

        {
        CallStackFrame frame(*this, op.address, CallStackFrame::Address);
        eval(size_t(op.address));

        if(frame.hasReturnVal && frame.prevStackGuard==m_Stack.size())
          pushInt(0);
        }

        m_Instance = currentInstance;
        break;
        }

      case EParOp_CallExternal: {
        std::function<void(DaedalusVM&)>* f=nullptr;
        if(size_t(op.symbol)<m_ExternalsByIndex.size()){
          f = &m_ExternalsByIndex[size_t(op.symbol)];
          }

        auto currentInstance = m_Instance;

        if(f!=nullptr && *f) {
          CallStackFrame frame(*this, op.symbol, CallStackFrame::SymbolIndex);
          (*f)(*this);
          } else {
          CallStackFrame frame(*this, op.symbol, CallStackFrame::SymbolIndex);
          m_OnUnsatisfiedCall(*this);
          }

        m_Instance = currentInstance;
        break;
        }

      case EParOp_PushInt:
        pushInt(op.value);
        break;
      case EParOp_PushVar:
        pushVar(size_t(op.symbol));
        break;
      case EParOp_PushInstance:
        pushVar(size_t(op.symbol));
        break;  //TODO: Not sure about this
      case EParOp_AssignString: {
        size_t a = popVar(arr);
        size_t b = popVar(arr2);

        straddr         = &m_DATFile.getSymbolByIndex(a).getString(arr,  getCurrentInstanceDataPtr());
        std::string& s2 =  m_DATFile.getSymbolByIndex(b).getString(arr2, getCurrentInstanceDataPtr());
        *straddr = s2;
        break;
        }

      case EParOp_AssignStringRef:
        LogError() << "EParOp_AssignStringRef not implemented!";
        break;

      case EParOp_AssignFloat: {
        size_t a = popVar(arr);
        float  b = popFloatValue();
        float& aFloat = m_DATFile.getSymbolByIndex(a).getFloat(arr, getCurrentInstanceDataPtr());
        aFloat = b;
        break;
        }

      case EParOp_AssignInstance: {
        size_t a = popVar();
        size_t b = popVar();

        auto& sa = m_DATFile.getSymbolByIndex(a);
        auto& sb = m_DATFile.getSymbolByIndex(b);

        sa.instance = sb.instance;
        break;
        }

      case EParOp_Jump:
        PC = size_t(op.address);
        break;

      case EParOp_JumpIf:
        a = popDataValue();
        if(a==0)
          PC = size_t(op.address);
        break;

      case EParOp_SetInstance:
        setCurrentInstance(size_t(op.symbol));
        break;
      case EParOp_PushArrayVar:
        pushVar(size_t(op.symbol), op.index);
        break;
      }
    }
  }

const PARStackOpCode& DaedalusVM::nextInstruction(size_t& pc) {
  const PARStackOpCode& op = m_DATFile.getStackOpCode(pc);
  pc += op.opSize;
  return op;
  }

void DaedalusVM::registerExternalFunction(const char* symName, const std::function<void(DaedalusVM&)>& fn) {
  if(m_DATFile.hasSymbolName(symName)) {
    size_t s = m_DATFile.getSymbolIndexByName(symName);
    if(m_ExternalsByIndex.size()<=s)
      m_ExternalsByIndex.resize(s+1);
    m_ExternalsByIndex[s] = fn;
    }
  }

void DaedalusVM::registerUnsatisfiedLink(const std::function<void (DaedalusVM &)> &fn) {
  m_OnUnsatisfiedCall = fn;
  }

void DaedalusVM::pushInt(uint32_t value) {
  m_Stack.emplace_back(reinterpret_cast<int32_t&>(value));
  }

void DaedalusVM::pushInt(int32_t value) {
  m_Stack.emplace_back(value);
  }

template <typename T>
T DaedalusVM::popDataValue() {
  // Default behavior of the ZenGin is to pop a 0, if nothing is on the stack.
  if(m_Stack.size()<=m_StackGuard)
    return static_cast<T>(0);

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParOp_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getValue<T>(top.id,top.inst);
    }
  return top.i32;
  }

void DaedalusVM::pushVar(size_t index, uint32_t arrIdx) {
  auto& sym = m_DATFile.getSymbolByIndex(index);(void)sym;
  //int32_t val = sym.getValue<int>(arrIdx, getCurrentInstanceDataPtr());
  auto ptr = getCurrentInstanceDataPtr();
  m_Stack.emplace_back(ptr,index,arrIdx);
  }

uint32_t DaedalusVM::popVar(uint32_t& arrIdx) {
  if(m_Stack.empty()){
    arrIdx=0;
    return 0;
    }

  auto top = m_Stack.back();
  m_Stack.pop_back();
  arrIdx = top.id;
  return uint32_t(top.i32);
  }

std::string DaedalusVM::popString(bool toUpper) {
  uint32_t arr;
  uint32_t idx = popVar(arr);

  std::string s = m_DATFile.getSymbolByIndex(idx).getString(arr, getCurrentInstanceDataPtr());

  if(toUpper)
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);

  return s;
  }

void DaedalusVM::setReturn(int32_t v) {
  pushInt(v);
  }

void DaedalusVM::setReturn(const std::string& v) {
  pushString(v);
  }

void DaedalusVM::setReturn(std::string &&v) {
  pushString(std::move(v));
  }

void DaedalusVM::setReturn(float f) {
  m_Stack.emplace_back(f);
  }

void DaedalusVM::setReturnVar(int32_t v) {
  pushVar(size_t(v));
  }

int32_t DaedalusVM::popInt() {
  if(m_Stack.empty())
    return 0;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParOp_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getValue<int32_t>(top.id,top.inst);
    }
  return top.i32;
  }

float DaedalusVM::popFloat() {
  if(m_Stack.empty())
    return 0;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParOp_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getFloat(top.id,top.inst);
    }
  return top.f;
  }

std::string& DaedalusVM::popString() {
  if(m_Stack.empty()){
    static std::string err;
    return err;
    }

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParOp_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getString(top.id,top.inst);
    }

  static std::string err;
  return err;
  }

float DaedalusVM::popFloatValue() {
  auto top = m_Stack.back();
  m_Stack.pop_back();
  return top.f;
  }

uint32_t DaedalusVM::popVar() {
  uint32_t arr=0;
  return popVar(arr);
  }

void DaedalusVM::pushVar(const char* symName) {
  size_t idx = m_DATFile.getSymbolIndexByName(symName);
  pushVar(idx);
  }

void DaedalusVM::pushString(const std::string& str) {
  size_t symIdx = m_FakeStringSymbols.front();
  Daedalus::PARSymbol& s = m_DATFile.getSymbolByIndex(symIdx);
  m_FakeStringSymbols.push(m_FakeStringSymbols.front());
  m_FakeStringSymbols.pop();

  s.getString(0) = str;

  pushVar(symIdx, 0);
  }

void DaedalusVM::pushString(std::string &&str) {
  size_t symIdx = m_FakeStringSymbols.front();
  Daedalus::PARSymbol& s = m_DATFile.getSymbolByIndex(symIdx);
  m_FakeStringSymbols.push(m_FakeStringSymbols.front());
  m_FakeStringSymbols.pop();

  s.getString(0) = std::move(str);

  pushVar(symIdx, 0);
  }

void DaedalusVM::setInstance(const char* instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass) {
  PARSymbol& s = m_DATFile.getSymbolByName(instSymbol);
  s.instance.set(h,instanceClass);
  }

void DaedalusVM::setInstance(const size_t instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass) {
  PARSymbol& s = m_DATFile.getSymbolByIndex(instSymbol);
  s.instance.set(h,instanceClass);
  }

void DaedalusVM::clearReferences(GEngineClasses::Instance &h) {
  if(h.useCount==0)
    return;

  if(m_Instance.get()==&h)
    m_Instance = nullptr;

  for(size_t i=0;i<m_DATFile.getSymTable().symbols.size();++i){
    auto& s = m_DATFile.getSymbolByIndex(i);
    if(s.instance.get()==&h){
      s.instance = nullptr;
      if(h.useCount==0)
        return;
      }
    }
  }

void DaedalusVM::clearReferences(EInstanceClass h) {
  if(m_Instance.instanceOf(h))
    m_Instance = nullptr;

  for(size_t i=0;i<m_DATFile.getSymTable().symbols.size();++i){
    auto& s = m_DATFile.getSymbolByIndex(i);
    if(s.instance.instanceOf(h)){
      s.instance = nullptr;
      }
    }
  }

void DaedalusVM::initializeInstance(GEngineClasses::Instance &instance, size_t symIdx, EInstanceClass classIdx) {
  PARSymbol& s = m_DATFile.getSymbolByIndex(symIdx);

  // Enter address into instance-symbol
  s.instance.set(&instance,classIdx);

  auto currentInstance = m_Instance;
  setCurrentInstance(symIdx);

  PARSymbol selfCpy;
  // Particle and Menu VM do not have a self symbol
  if(m_SelfId!=size_t(-1)) {
    selfCpy = globalSelf();  // Copy of "self"-symbol
    // Set self
    setInstance(m_SelfId, &instance, classIdx);
    }

  // Place the assigning symbol into the instance
  instance.instanceSymbol = symIdx;

  // Run script code to initialize the object
  runFunctionBySymIndex(symIdx);

  if(m_SelfId!=size_t(-1))
    globalSelf() = selfCpy;

  m_Instance = currentInstance;
  }

void DaedalusVM::setCurrentInstance(size_t symIdx) {
  auto& sym = m_DATFile.getSymbolByIndex(symIdx);
  m_Instance = sym.instance;
  }

GEngineClasses::Instance *DaedalusVM::getCurrentInstanceDataPtr() {
  return m_Instance.get();
  }

std::vector<std::string> DaedalusVM::getCallStack() {
  std::vector<std::string> symbolNames;
  auto frame=m_CallStack;
  while(frame) {
    symbolNames.push_back(nameFromFunctionInfo(std::make_pair(frame->address, CallStackFrame::Address)));
    frame = frame->calee;
    }
  return symbolNames;
  }

const std::string &DaedalusVM::currentCall() {
  if(m_CallStack)
    return nameFromFunctionInfo(std::make_pair(m_CallStack->address, CallStackFrame::Address));

  static std::string n = "<no function>";
  return n;
  }

PARSymbol &DaedalusVM::globalSelf() {
  return m_DATFile.getSymbolByIndex(m_SelfId);
  }

PARSymbol &DaedalusVM::globalOther() {
  return m_DATFile.getSymbolByIndex(m_OtherId);
  }

PARSymbol &DaedalusVM::globalVictim() {
  return m_DATFile.getSymbolByIndex(m_VictimId);
  }

PARSymbol &DaedalusVM::globalItem() {
  return m_DATFile.getSymbolByIndex(m_ItemId);
  }

int32_t DaedalusVM::runFunctionBySymIndex(size_t symIdx, bool clearDataStack) {
  if(symIdx==size_t(-1))
    return 0;

  if(clearDataStack)
    m_Stack.clear();

  CallStackFrame frame(*this, int32_t(symIdx), CallStackFrame::SymbolIndex);
  if(frame.address == 0)
    return -1;

  // Execute the instructions
  eval(frame.address);

  int32_t ret = 0;
  if(frame.hasReturnVal)
    ret = popDataValue();
  return ret;
  }

const std::string& DaedalusVM::nameFromFunctionInfo(DaedalusVM::CallStackFrame::FunctionInfo functionInfo) {
  switch(functionInfo.second) {
    case CallStackFrame::SymbolIndex: {
      auto functionSymbolIndex = functionInfo.first;
      return getDATFile().getSymbolByIndex(functionSymbolIndex).name;
      }
    case CallStackFrame::Address: {
      auto address = functionInfo.first;
      auto functionSymbolIndex = getDATFile().getFunctionIndexByAddress(address);
      if (functionSymbolIndex != static_cast<size_t>(-1))
        return getDATFile().getSymbolByIndex(functionSymbolIndex).name;
        }
      }
  static std::string err;
  err = "unknown function with address: " + std::to_string(functionInfo.first);
  return err;
  }

[[noreturn]]
void DaedalusVM::terminateScript(){
  throw std::logic_error("fatal script error");
  }

DaedalusVM::CallStackFrame::CallStackFrame(DaedalusVM& vm, int32_t addressOrIndex, AddressType addrType)
    : calee(vm.m_CallStack), prevStackGuard(vm.m_StackGuard), vm(vm) {
  auto  symIdx         = addrType==SymbolIndex ? size_t(addressOrIndex) : vm.getDATFile().getFunctionIndexByAddress(address);
  auto& functionSymbol = vm.getDATFile().getSymbolByIndex(symIdx);
  address              = functionSymbol.address;
  if(address == 0)
    return;

  if(vm.m_Stack.size()<functionSymbol.properties.elemProps.count)
    vm.terminateScript();
  vm.m_StackGuard = vm.m_Stack.size() - functionSymbol.properties.elemProps.count;
  hasReturnVal    = functionSymbol.hasEParFlag(EParFlag_Return);

  // entering function
  vm.m_CallStack = this;
  }

DaedalusVM::CallStackFrame::~CallStackFrame() {
  vm.m_CallStack  = calee;
  vm.m_StackGuard = prevStackGuard;
  }
