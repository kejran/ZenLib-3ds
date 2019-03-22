//
// Created by andre on 09.05.16.
//

#include "DATFile.h"
#include <algorithm>
#include <map>
#include <utils/logger.h>

using namespace ZenLoad;
using namespace Daedalus;

DATFile::DATFile(const uint8_t* pData, size_t numBytes){
  ZenLoad::ZenParser input(pData, numBytes);
  uint8_t version = input.readBinaryByte();
  (void)version;
  readSymTable(input);
  }

void DATFile::readSymTable(ZenLoad::ZenParser& input) {
  uint32_t count = input.readBinaryDWord();
  m_SymTable.symbolsByName.reserve(count);
  m_SymTable.symbols.resize(count);
  m_SymTable.sortTable.resize(count);
  input.readBinaryRaw(m_SymTable.sortTable.data(), sizeof(uint32_t) * count);

  // Read symbols
  for(uint32_t i=0; i<count; i++) {
    PARSymbol s;
    uint32_t named = input.readBinaryDWord();

    if(named) {
      uint8_t b = input.readBinaryByte();
      while(b != 0x0A) {
        if(b != 0xFF)  // FIXME: This happens at INSTANCE_HELP?
          s.name += b;
        b = input.readBinaryByte();
        };
      }

    input.readBinaryRaw(&s.properties, sizeof(s.properties));

    if(0 == (s.properties.elemProps.flags & EParFlag_ClassVar)) {
      switch(s.properties.elemProps.type) {
        case EParType_Float:
          s.floatData.resize(s.properties.elemProps.count);
          input.readBinaryRaw(s.floatData.data(), sizeof(float) * s.floatData.size());
          break;

        case EParType_Int:
          s.intData.resize(s.properties.elemProps.count);
          input.readBinaryRaw(s.intData.data(), sizeof(uint32_t) * s.intData.size());
          break;

        case EParType_String:
          s.strData.resize(s.properties.elemProps.count);
          for(uint32_t j = 0; j < s.properties.elemProps.count; j++) {
            uint8_t b = input.readBinaryByte();
            while (b != 0x0A) {
              if (b != 0xFF)  // FIXME: This happens at INSTANCE_HELP?
                s.strData[j] += b;

              b = input.readBinaryByte();
              };

            auto replace = [](std::string& searchSpace, const std::string& from, const std::string& to) {
              size_t start_pos = 0;
              while((start_pos = searchSpace.find(from, start_pos)) != std::string::npos) {
                searchSpace.replace(start_pos, from.length(), to);
                start_pos += to.length();
                }
              };
            replace(s.strData[j], "\\n", "\n");
            }
          break;

        case EParType_Class:
          s.classOffset = static_cast<int32_t>(input.readBinaryDWord());
          break;

        case EParType_Func:
        case EParType_Prototype:
        case EParType_Instance:
          s.address = static_cast<int32_t>(input.readBinaryDWord());
          if (s.properties.elemProps.flags & EParFlag_External)
            ;
          break;
        }
      }

    s.parent = static_cast<int32_t>(input.readBinaryDWord());

    if (!s.name.empty())
      m_SymTable.symbolsByName.emplace(s.name, i);

    // check for callable object
    if(s.isEParType(EParType_Prototype) ||    // is a Prototype
       (s.isEParType(EParType_Func)           // or a function, which
        && !s.hasEParFlag(EParFlag_ClassVar)  // is no class-member (skip class members of type func)
        && s.hasEParFlag(EParFlag_Const)))    // and is const (skip function params of type func)
      {
      m_SymTable.functionsByAddress.emplace(s.address, i);
      }

    m_SymTable.symbols[i] = std::move(s);
    }

  readStack(input);
  }

void DATFile::readStack(ZenLoad::ZenParser& input) {
  size_t stackSize = input.readBinaryDWord();
  size_t seek      = 0;

  while(input.getSeek()<input.getFileSize()) {
    PARStackOpCode s={};
    s.op = static_cast<EParOp>(input.readBinaryByte());
    seek += sizeof(uint8_t);
    s.opSize = sizeof(uint8_t);

    switch(s.op) {
      case EParOp_Call:
        s.address = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_CallExternal:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_PushInt:
        s.value = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_PushVar:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_PushInstance:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_Jump:
        s.address = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_JumpIf:
        s.address = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_SetInstance:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;

      case EParOp_PushArrayVar:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);

        s.index = input.readBinaryByte();
        s.opSize += sizeof(uint8_t);
        break;
      default:
        break;
      }

    seek += s.opSize;
    code.push_back(s);
    }

  size_t pc=0;
  pcCode.resize(stackSize);
  for(auto& i:code) {
    for(size_t r=0;r<i.opSize;++r)
      pcCode[pc+r] = &i;
    pc+=i.opSize;
    }
  }

namespace Daedalus
{
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Npc>()
    {
        return IC_Npc;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Mission>()
    {
        return IC_Mission;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Info>()
    {
        return IC_Info;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Item>()
    {
        return IC_Item;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_ItemReact>()
    {
        return IC_ItemReact;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Focus>()
    {
        return IC_Focus;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Menu>()
    {
        return IC_Menu;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Menu_Item>()
    {
        return IC_MenuItem;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_SFX>()
    {
        return IC_Sfx;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_ParticleFX>()
    {
        return IC_Pfx;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_MusicTheme>()
    {
        return IC_MusicTheme;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_GilValues>()
    {
        return IC_GilValues;
    };
}  // namespace Daedalus

PARSymbol& DATFile::getSymbolByName(const std::string& symName) {
  std::string n = std::string(symName);
  std::transform(n.begin(), n.end(), n.begin(), ::toupper);
  //assert(hasSymbolName(n));
  if(!hasSymbolName(n)) {
    LogWarn() << "symbol " << symName << " not found";
    }
  return m_SymTable.symbols[m_SymTable.symbolsByName[n]];
  }

bool DATFile::hasSymbolName(const std::string& symName) {
  std::string n = std::string(symName);
  std::transform(n.begin(), n.end(), n.begin(), ::toupper);

  return m_SymTable.symbolsByName.find(n) != m_SymTable.symbolsByName.end();
  }

size_t DATFile::getSymbolIndexByName(const std::string& symName) {
  std::string n = std::string(symName);
  std::transform(n.begin(), n.end(), n.begin(), ::toupper);

  return m_SymTable.symbolsByName[n];
  }

PARSymbol& DATFile::getSymbolByIndex(size_t idx) {
  return m_SymTable.symbols[idx];
  }

size_t DATFile::getFunctionIndexByAddress(size_t address) {
  auto it = m_SymTable.functionsByAddress.find(address);
  if (it != m_SymTable.functionsByAddress.end())
    return it->second;
  else
    return static_cast<size_t>(-1);
  }

const PARStackOpCode& DATFile::getStackOpCode(size_t pc) {
  if(pc<pcCode.size())
    return *pcCode[pc];
  static PARStackOpCode s={};
  s.op=EParOp_Ret;
  return s;
  }

size_t DATFile::addSymbol() {
  m_SymTable.symbols.emplace_back();
  return m_SymTable.symbols.size() - 1;
  }

void DATFile::iterateSymbolsOfClass(const std::string& className, std::function<void(size_t, PARSymbol&)> callback) {
  constexpr auto none = uint32_t{0xFFFFFFFF};
  // First, find the parent-symbol
  size_t baseSym = getSymbolIndexByName(className);

  for(size_t i = 0; i < m_SymTable.symbols.size(); i++) {
    PARSymbol& s = getSymbolByIndex(i);
    if(s.parent == none || s.properties.elemProps.type != EParType_Instance)
      continue;

    if((s.properties.elemProps.flags & Daedalus::EParFlag::EParFlag_Const) == 0)
      continue;  // filters out variables of type C_NPC or C_ITEM

    PARSymbol& p = getSymbolByIndex(s.parent);
    uint32_t pBase = s.parent;

    // In case this is also just a prototype, go deeper one more level
    if(p.properties.elemProps.type == EParType_Prototype && p.parent != none) {
      pBase = p.parent;
      }

    // If the parent-classes match, we found an instance of our class
    if (baseSym == pBase)
      callback(i, s);
    }
  }

namespace Daedalus
{
// getDataContainer specializations
template <>
std::vector<int32_t>& PARSymbol::getDataContainer() {
  return this->intData;
  }

template <>
std::vector<float>& PARSymbol::getDataContainer() {
  return this->floatData;
  }

template <>
std::vector<std::string>& PARSymbol::getDataContainer() {
  return this->strData;
  }
}  // namespace Daedalus

int32_t& PARSymbol::getInt(size_t idx, void* baseAddr) {
  return getValue<int32_t>(idx, baseAddr);
  }

std::string& PARSymbol::getString(size_t idx, void* baseAddr) {
  return getValue<std::string>(idx, baseAddr);
  }

float& PARSymbol::getFloat(size_t idx, void* baseAddr) {
  return getValue<float>(idx, baseAddr);
  }
