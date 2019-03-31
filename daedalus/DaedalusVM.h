#pragma once

#include <functional>
#include <queue>
#include <set>
#include <stack>
#include "DATFile.h"
#include "DaedalusGameState.h"
#include "DaedalusStdlib.h"

namespace Daedalus {
class DaedalusVM {
  public:
    struct Instance final {
      size_t         id=0;
      void*          handle=nullptr;
      EInstanceClass cls=EInstanceClass::IC_Npc;
      };

    DaedalusVM(const std::vector<uint8_t> data);
    DaedalusVM(const uint8_t* pDATFileData, size_t numBytes);

    void    eval(uint32_t pcInit);
    int32_t runFunctionBySymIndex(size_t symIdx, bool clearDataStack = true);

    void registerExternalFunction(const char *symName, const std::function<void(DaedalusVM&)>& fn);
    void registerUnsatisfiedLink (const std::function<void(DaedalusVM&)> &fn);

    void pushInt(uint32_t value);
    void pushInt(int32_t value);
    void pushVar(size_t index, uint32_t arrIdx = 0);
    void pushVar(const char *symName);
    void pushString(const std::string& str);

    void setReturn(int32_t v);
    void setReturn(const std::string& v);
    void setReturn(float f);
    void setReturnVar(int32_t v);

    int32_t      popInt();
    float        popFloat();
    std::string& popString();
    uint32_t     popVar();
    uint32_t     popVar(uint32_t& arrIdx);
    std::string  popString(bool toUpper);

    void setInstance(const char *instSymbol, void *h, EInstanceClass instanceClass);
    void initializeInstance(void* instance, size_t symIdx, EInstanceClass classIdx);

    template<class T>
    void initializeInstance(T& instance, size_t symIdx) {
      //initializeInstance(&instance,symIdx,DaedalusVM::enumFromClass<T>());
      }

    void*                         getCurrentInstanceDataPtr();
    EInstanceClass                getCurrentInstanceClass()  { return m_CurrentInstanceClass; }
    void*                         getCurrentInstanceHandle() { return m_CurrentInstanceHandle; }
    DATFile&                      getDATFile() { return m_DATFile; }
    GameState::DaedalusGameState& getGameState() { return m_GameState; }
    std::vector<std::string>      getCallStack();
    const std::string&            currentCall();

    PARSymbol&                    globalSelf();
    PARSymbol&                    globalOther();
    PARSymbol&                    globalVictim();
    PARSymbol&                    globalItem();

  private:
    template <typename T = int32_t>
    T           popDataValue();
    float       popFloatValue();

    const PARStackOpCode &nextInstruction(size_t& pc);
    void setCurrentInstance(size_t symIdx);

    class CallStackFrame {
    public:
      enum AddressType : uint8_t {
        Address,
        SymbolIndex
        };
      using FunctionInfo = std::pair<size_t, CallStackFrame::AddressType>;

      CallStackFrame(DaedalusVM& vm, int32_t addressOrIndex, AddressType addrType);
      ~CallStackFrame();

      CallStackFrame* const calee         =nullptr;
      const int32_t         addressOrIndex=0;
      const AddressType     addrType      =SymbolIndex;

    private:
      DaedalusVM&     vm;
      };

    DATFile m_DATFile;

    struct Stk {
      Stk(int32_t i):i32(i){}
      Stk(float   f):f(f){}
      Stk(void* inst,int32_t i,size_t id):i32(i),tag(EParOp_PushVar),id(id),inst(inst){}

      union {
        int32_t i32;
        float   f;
        };
      EParOp   tag=EParOp_PushInt;
      uint32_t id =0;
      void*    inst=nullptr;
      };

    struct Stack {
      Stack(std::vector<Stk>& s):storage(s),zero(s.size()){}

      std::vector<Stk> storage;
      size_t           zero=0;
      };

    const std::string &nameFromFunctionInfo(CallStackFrame::FunctionInfo functionInfo);
    [[noreturn]] void terminateScript();

    std::vector<Stk>                                             m_Stack;

    // contains pairs of FunctionInfo, Debugging only
    CallStackFrame*                                              m_CallStack=nullptr;
    std::vector<std::function<void(DaedalusVM&)>>                m_ExternalsByIndex;
    std::function<void(DaedalusVM&)>                             m_OnUnsatisfiedCall;

    size_t                                                       m_CurrentInstance;
    void*                                                        m_CurrentInstanceHandle;
    EInstanceClass                                               m_CurrentInstanceClass;

    size_t                                                       m_SelfId   = size_t(-1);
    size_t                                                       m_OtherId  = size_t(-1);
    size_t                                                       m_VictimId = size_t(-1);
    size_t                                                       m_ItemId   = size_t(-1);

    std::queue<size_t>                                           m_FakeStringSymbols;
    GameState::DaedalusGameState                                 m_GameState;
  };
}  // namespace Daedalus
