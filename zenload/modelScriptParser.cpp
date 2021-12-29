#include <cmath>

#include <algorithm>
#include <sstream>
#include <utils/logger.h>

#include "modelScriptParser.h"
#include "zenParser.h"

using namespace ZenLoad;

MdsParserTxt::MdsParserTxt(ZenParser &zen)
  :zen(zen) {
  buf.reserve(8);
  }

MdsParserTxt::TokType MdsParserTxt::nextTok(std::string& buf) {
  buf.clear();
  bool loop=true;
  while(loop && zen.getRamainBytes()>0) {
    loop = false;
    const char first = zen.readChar();

    if(first==' ' || first=='\n' || first=='\r' || first=='\t') {
      loop = true;
      }
    else if(first==':') {
      // fps semicolon
      loop = true;
      }
    else if(first=='/' && zen.peekChar()=='/') {
      zen.readChar();
      while(zen.getRamainBytes()>0) {
        const char cur = zen.readChar();
        if(cur=='\n')
          break;
        }
      loop = true;
      }
    else if(('a'<=first && first<='z') || ('A'<=first && first<='Z') || first=='_' || first=='*' ) {
      buf.push_back(first);
      while(zen.getRamainBytes()>0) {
        const char cur = zen.peekChar();
        if(('a'<=cur && cur<='z') || ('A'<=cur && cur<='Z') || cur=='*' || cur=='_' || ('0'<=cur && cur<='9')) {
          buf.push_back(zen.readChar());
          } else {
          break;
          }
        }
      return TokType::TK_Name;
      }
    else if('0'<=first && first<='9') {
      bool flt=false;
      buf.push_back(first);
      while(zen.getRamainBytes()>0) {
        const char cur = zen.peekChar();
        if(('0'<=cur && cur<='9') || (cur=='.' && !flt)) {
          buf.push_back(zen.readChar());
          if(cur=='.')
            flt=true;
          } else {
          break;
          }
        }
      return TokType::TK_Num;
      }
    else if(first=='\"') {
      while(zen.getRamainBytes()>0) {
        const char cur = zen.readChar();
        if(cur=='\"')
          break;
        if(cur=='\r' || cur=='\n') {
          // Invalid syntax
          break; // G1 wolf.mds is missing closing " - need to workaround it
          }
        buf.push_back(cur);
        }
      return TokType::TK_String;
      }
    else if(first=='(') {
      return TokType::TK_BracketL;
      }
    else if(first==')') {
      return TokType::TK_BracketR;
      }
    else if(first=='{') {
      return TokType::TK_Begin;
      }
    else if(first=='}') {
      return TokType::TK_End;
      }
    }

  return TokType::TK_Null;
  }

MdsParser::Chunk MdsParserTxt::beginChunk() {
  while(zen.getRamainBytes()>0) {
    const TokType tt = nextTok(buf);
    if(tt==TK_Name) {
      if(buf=="meshAndTree")
        return CHUNK_MESH_AND_TREE;
      if(buf=="registerMesh")
        return CHUNK_REGISTER_MESH;
      if(buf=="ani")
        return CHUNK_ANI;
      if(buf=="aniComb")
        return CHUNK_ANI_COMB;
      if(buf=="aniDisable")
        return CHUNK_ANI_DISABLE;
      if(buf=="aniAlias")
        return CHUNK_ANI_ALIAS;
      if(buf=="*eventSFX")
        return CHUNK_EVENT_SFX;
      if(buf=="*eventSFXGrnd")
        return CHUNK_EVENT_SFX_GRND;
      if(buf=="*eventPFX")
        return CHUNK_EVENT_PFX;
      if(buf=="*eventPFXStop")
        return CHUNK_EVENT_PFX_STOP;
      if(buf=="*eventTag")
        return CHUNK_EVENT_TAG;
      if(buf=="*eventMMStartAni")
        return CHUNK_EVENT_MMSTARTANI;
      }
    }
  return CHUNK_EOF;
  }

void MdsParserTxt::beginArgs() {
  implReadItem(TK_BracketL);
  }

void MdsParserTxt::endArgs() {
  while(zen.getRamainBytes()>0) {
    const TokType tt = nextTok(buf);
    if(tt==TK_BracketR)
      return;
    if(tt==TK_End) {
      zen.setSeek(zen.getSeek()-1); // unget
      break;
      }
    }
  }

void MdsParserTxt::implReadItem(MdsParserTxt::TokType dest) {
  while(zen.getRamainBytes()>0) {
    const TokType tt = nextTok(buf);
    if(tt==dest)
      return;
    if(tt==TK_BracketR || tt==TK_End) {
      zen.setSeek(zen.getSeek()-1); // unget
      break;
      }
    }
  buf.clear();
  }

std::string MdsParserTxt::readStr() {
  implReadItem(TK_String);
  return std::move(buf);
  }

std::string MdsParserTxt::readKeyword() {
  implReadItem(TK_Name);
  return std::move(buf);
  }

uint32_t MdsParserTxt::readDWord() {
  return uint32_t(readI32());
  }

int32_t MdsParserTxt::readI32() {
  implReadItem(TK_Num);
  if(buf.size()>0)
    return std::atol(buf.c_str());
  return 0;
  }

float MdsParserTxt::readFloat() {
  implReadItem(TK_Num);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }

void MdsParserTxt::readMeshAndTree() {
  std::string mesh = readStr();
  std::string hint = readKeyword();

  meshAndThree.m_Name     = mesh;
  meshAndThree.m_Disabled = (hint=="DONT_USE_MESH");
  }


MdsParserBin::MdsParserBin(ZenParser &zen)
  :zen(zen) {
  }

MdsParser::Chunk MdsParserBin::beginChunk() {
  if(zen.getFileSize()<=zen.getSeek()+sizeof(BinaryChunkInfo))
    return MdsParser::CHUNK_EOF;

  BinaryChunkInfo chunk;
  zen.readStructure(chunk);
  chunkEnd = uint32_t(zen.getSeek())+chunk.length;
  return MdsParser::Chunk(chunk.id);
  }

void MdsParserBin::endChunk() {
  zen.setSeek(chunkEnd);
  }

std::string MdsParserBin::readStr() {
  return zen.readLine(false);
  }

std::string MdsParserBin::readKeyword() {
  return zen.readLine(false);
  }

uint32_t MdsParserBin::readDWord() {
  return zen.readBinaryDWord();
  }

int32_t MdsParserBin::readI32() {
  return int32_t(zen.readBinaryDWord());
  }

float MdsParserBin::readFloat() {
  return zen.readBinaryFloat();
  }

void MdsParserBin::readMeshAndTree() {
  const bool dontUseMesh = readDWord()!=0;

  meshAndThree.m_Name     = readStr();
  meshAndThree.m_Disabled = dontUseMesh;
  }


MdsParser::Chunk MdsParser::parse() {
  while(true) {
    const Chunk ch = beginChunk();
    switch(ch) {
      case CHUNK_EOF:
        break;
      case CHUNK_ERROR:
        break;
      case CHUNK_MLID_MODELMESH:
      case CHUNK_MLID_MDM_SOURCE:
      case CHUNK_MLID_MDM_NODEMESHES:
      case CHUNK_MLID_MDM_SOFSKINLIST:
        break;
      case CHUNK_SOURCE:
        break;
      case CHUNK_MODEL_SCRIPT:
        break;
      case CHUNK_MODEL_SCRIPT_END:
        break;
      case CHUNK_MODEL:
        break;
      case CHUNK_MODEL_END:
        break;
      case CHUNK_ANI_ENUM:
        break;
      case CHUNK_ANI_COMB:
        beginArgs();
        readAniComb();
        endArgs();
        break;
      case CHUNK_ANI_DISABLE:
        beginArgs();
        readAniDisable();
        endArgs();
        break;
      case CHUNK_ANI_BLEND:
        beginArgs();
        readAniBlend();
        endArgs();
        break;
      case CHUNK_MESH_AND_TREE:
        readMeshAndTree();
        break;
      case CHUNK_REGISTER_MESH:
        readRegisterMesh();
        break;
      case CHUNK_ANI:
        beginArgs();
        readAni();
        endArgs();
        break;
      case CHUNK_ANI_ALIAS:
        beginArgs();
        readAniAlias();
        endArgs();
        break;
      case CHUNK_EVENT_SFX:
        beginArgs();
        readSfx(sfx);
        endArgs();
        break;
      case CHUNK_EVENT_SFX_GRND:
        beginArgs();
        readSfx(gfx);
        endArgs();
        break;
      case CHUNK_EVENT_PFX:
        beginArgs();
        readPfx();
        endArgs();
        break;
      case CHUNK_EVENT_PFX_STOP:
        beginArgs();
        readPfxStop();
        endArgs();
        break;
      case CHUNK_EVENT_TAG:
        beginArgs();
        readEvent(eventTag);
        endArgs();
        break;
      case CHUNK_MODEL_TAG:
        beginArgs();
        readEvent(modelTag);
        endArgs();
        break;
      case CHUNK_EVENT_MMSTARTANI:
        beginArgs();
        readMMStart();
        endArgs();
        break;
      }
    endChunk();
    return ch;
    }
  }

uint32_t MdsParser::makeAniFlags(const std::string& flag_str) {
  uint32_t flags = 0;
  for(auto ch : flag_str) {
    switch(ch) {
      case 'M':
        flags |= MSB_MOVE_MODEL;
        break;
      case 'R':
        flags |= MSB_ROTATE_MODEL;
        break;
      case 'E':
        flags |= MSB_QUEUE_ANI;
        break;
      case 'F':
        flags |= MSB_FLY;
        break;
      case 'I':
        flags |= MSB_IDLE;
        break;
      }
    }
  return flags;
  }

EModelScriptAniDir MdsParser::makeAniDir(const std::string& str){
  return (!str.empty() && str[0] == 'R') ? MSB_BACKWARD : MSB_FORWARD;
  }

void MdsParser::readRegisterMesh() {
  // Reads "Some_Mesh.ASC"
  std::string mesh = readStr();
  meshesASC.emplace_back(std::move(mesh));
  }

void MdsParser::readAni() {
  ani.m_Name        = readStr();
  ani.m_Layer       = readDWord();
  ani.m_Next        = readStr();
  ani.m_BlendIn     = readFloat();
  ani.m_BlendOut    = readFloat();
  ani.m_Flags       = makeAniFlags(readKeyword());
  ani.m_Asc         = readStr();
  ani.m_Dir         = makeAniDir(readKeyword());
  ani.m_FirstFrame  = readI32();
  ani.m_LastFrame   = readI32();
  ani.m_MaxFps      = readFloat();
  ani.m_Speed       = readFloat();
  ani.m_ColVolScale = readFloat();
  }

void MdsParser::readAniComb() {
  comb.m_Name      = readStr();
  comb.m_Layer     = readDWord();
  comb.m_Next      = readStr();
  comb.m_BlendIn   = readFloat();
  comb.m_BlendOut  = readFloat();
  comb.m_Flags     = makeAniFlags(readKeyword());
  comb.m_Asc       = readStr();
  comb.m_LastFrame = readDWord();
  }

void MdsParser::readAniAlias() {
  alias.m_Name     = readStr();
  alias.m_Layer    = readDWord();
  alias.m_Next     = readStr();
  alias.m_BlendIn  = readFloat();
  alias.m_BlendOut = readFloat();
  alias.m_Flags    = makeAniFlags(readKeyword());
  alias.m_Alias    = readStr();
  alias.m_Dir      = makeAniDir(readKeyword());
  }

void MdsParser::readAniBlend() {
  blend.m_Name     = readStr();
  blend.m_Next     = readStr();
  blend.m_BlendIn  = readFloat();
  blend.m_BlendOut = readFloat();
  }

void MdsParser::readAniDisable() {
  disable.m_Name = readStr();
  }

void MdsParser::readSfx(std::vector<zCModelScriptEventSfx> &out) {
  zCModelScriptEventSfx s;
  s.m_Frame     = readI32();
  s.m_Name      = readStr();
  s.m_Range     = readFloat();
  s.m_EmptySlot = readDWord()!=0;
  out.emplace_back(std::move(s));
  }

void MdsParser::readPfx() {
  zCModelScriptEventPfx p;
  p.m_Frame = readI32();
  p.m_Num   = readI32();
  p.m_Name  = readStr();
  p.m_Pos   = readStr();
  //Like EmptySlot in readSfx encoded in float. No " " around value might be a hint that no string is used
  p.m_isAttached = readDWord()!=0;
  pfx.emplace_back(std::move(p));
  }

void MdsParser::readPfxStop() {
  zCModelScriptEventPfxStop p;
  p.m_Frame = readI32();
  p.m_Num   = readI32();
  pfxStop.emplace_back(p);
  }

void MdsParser::readEvent(std::vector<zCModelEvent> &out) {
  zCModelEvent evt;
  evt.m_Frame = readI32();
  std::string str = readStr();

  // https://worldofplayers.ru/threads/37708/
  if(str=="DEF_CREATE_ITEM")
    evt.m_Def=DEF_CREATE_ITEM;
  else if(str=="DEF_INSERT_ITEM")
    evt.m_Def=DEF_INSERT_ITEM;
  else if(str=="DEF_REMOVE_ITEM")
    evt.m_Def=DEF_REMOVE_ITEM;
  else if(str=="DEF_DESTROY_ITEM")
    evt.m_Def=DEF_DESTROY_ITEM;
  else if(str=="DEF_PLACE_ITEM")
    evt.m_Def=DEF_PLACE_ITEM;
  else if(str=="DEF_EXCHANGE_ITEM")
    evt.m_Def=DEF_EXCHANGE_ITEM;
  else if(str=="DEF_FIGHTMODE")
    evt.m_Def=DEF_FIGHTMODE;
  else if(str=="DEF_PLACE_MUNITION")
    evt.m_Def=DEF_PLACE_MUNITION;
  else if(str=="DEF_REMOVE_MUNITION")
    evt.m_Def=DEF_REMOVE_MUNITION;
  else if(str=="DEF_DRAWSOUND")
    evt.m_Def=DEF_DRAWSOUND;
  else if(str=="DEF_UNDRAWSOUND")
    evt.m_Def=DEF_UNDRAWSOUND;
  else if(str=="DEF_SWAPMESH")
    evt.m_Def=DEF_SWAPMESH;
  else if(str=="DEF_DRAWTORCH")
    evt.m_Def=DEF_DRAWTORCH;
  else if(str=="DEF_INV_TORCH")
    evt.m_Def=DEF_INV_TORCH;
  else if(str=="DEF_DROP_TORCH")
    evt.m_Def=DEF_DROP_TORCH;
  else if(str=="DEF_HIT_LIMB" || str=="HIT_LIMB")
    evt.m_Def=DEF_HIT_LIMB;
  else if(str=="DEF_HIT_DIR")
    evt.m_Def=DEF_HIT_DIR;
  else if(str=="DEF_DAM_MULTIPLY")
    evt.m_Def=DEF_DAM_MULTIPLY;
  else if(str=="DEF_PAR_FRAME")
    evt.m_Def=DEF_PAR_FRAME;
  else if(str=="DEF_OPT_FRAME")
    evt.m_Def=DEF_OPT_FRAME;
  else if(str=="DEF_HIT_END")
    evt.m_Def=DEF_HIT_END;
  else if(str=="DEF_WINDOW")
    evt.m_Def=DEF_WINDOW;
  else
    return;

  switch(evt.m_Def) {
    case DEF_NULL:
    case DEF_LAST:
      break;
    case DEF_CREATE_ITEM:
    case DEF_EXCHANGE_ITEM:
      evt.m_Slot = readStr();
      evt.m_Item = readStr();
      break;
    case DEF_INSERT_ITEM:
    case DEF_PLACE_MUNITION:
      evt.m_Slot = readStr();
      break;
    case DEF_REMOVE_ITEM:
    case DEF_DESTROY_ITEM:
    case DEF_PLACE_ITEM:
    case DEF_REMOVE_MUNITION:
      break;
    case DEF_DRAWSOUND:
    case DEF_UNDRAWSOUND:
      break;
    case DEF_FIGHTMODE:
      evt.m_Fmode = readFMode();
      break;
    case DEF_SWAPMESH:
      evt.m_Slot  = readStr();
      evt.m_Slot2 = readStr();
      break;
    case DEF_DRAWTORCH:
    case DEF_INV_TORCH:
    case DEF_DROP_TORCH:
      break;
    case DEF_HIT_LIMB:
    case DEF_HIT_DIR:
      // TODO
      break;
    case DEF_DAM_MULTIPLY:
    case DEF_PAR_FRAME:
    case DEF_OPT_FRAME:
    case DEF_HIT_END:
    case DEF_WINDOW:{
      str = readStr();
      std::stringstream ss(str);
      while(!ss.eof())
        {
          int frame=0;
          ss >> frame;
          if(!ss.good() && !ss.eof())
            break;
          evt.m_Int.push_back(frame);
        }
      break;
      }
    }

  out.emplace_back(std::move(evt));
  }

void MdsParser::readMMStart() {
  zCModelScriptEventMMStartAni mm;
  mm.m_Frame     = readI32();
  mm.m_Animation = readStr();
  mm.m_Node      = readStr();

  mmStartAni.emplace_back(std::move(mm));
  }

EFightMode MdsParser::readFMode() {
  auto s = readStr();
  if(s=="")
    return FM_NONE;
  if(s=="FIST")
    return FM_FIST;
  if(s=="1H" || s=="1h")
    return FM_1H;
  if(s=="2H" || s=="2h")
    return FM_2H;
  if(s=="BOW")
    return FM_BOW;
  if(s=="CBOW")
    return FM_CBOW;
  if(s=="MAG")
    return FM_MAG;
  return FM_NONE;
  }
