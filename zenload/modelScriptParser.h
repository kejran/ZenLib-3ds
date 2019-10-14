#pragma once

#include <string>

#include <zenload/zCModelAni.h>
#include <zenload/zCModelScript.h>

namespace ZenLoad {
  class ZenParser;

  /** Streaming parser for .MDS files.
   */
  class MdsParser {
    public:
      enum Chunk {
        CHUNK_EOF                      = 0,
        CHUNK_ERROR                    = -1,
        CHUNK_MODEL_SCRIPT             = 0xF000,
        CHUNK_MODEL_SCRIPT_END         = 0xFFFF,
        CHUNK_SOURCE                   = 0xF100,
        CHUNK_MODEL                    = 0xF200,
        CHUNK_MODEL_END                = 0xF2FF,
        CHUNK_MESH_AND_TREE            = 0xF300,
        CHUNK_REGISTER_MESH            = 0xF400,
        CHUNK_ANI_ENUM                 = 0xF500,
        // CHUNK_ANI_ENUM_END          = 0xF5FF,
        // CHUNK_ANI_MAX_FPS           = 0xF510,
        CHUNK_ANI                      = 0xF520,
        CHUNK_ANI_ALIAS                = 0xF530,
        CHUNK_ANI_BLEND                = 0xF540,
        // CHUNK_ANI_SYNC              = 0xF550,
        // CHUNK_ANI_BATCH             = 0xF560,
        CHUNK_ANI_COMB                 = 0xF570,
        // CHUNK_ANI_DISABLE           = 0xF580,
        CHUNK_MODEL_TAG                = 0xF590,
        // CHUNK_ANI_EVENTS            = 0xF5A0,
        // CHUNK_ANI_EVENTS_END        = 0xF5AF,
        CHUNK_EVENT_SFX                = 0xF5A1,
        CHUNK_EVENT_SFX_GRND           = 0xF5A2,
        CHUNK_EVENT_TAG                = 0xF5A3,
        CHUNK_EVENT_PFX                = 0xF5A4,
        CHUNK_EVENT_PFX_STOP           = 0xF5A5,
        // CHUNK_EVENT_PFX_GRND        = 0xF5A6,
        // CHUNK_EVENT_SET_MESH        = 0xF5A7,
        // CHUNK_EVENT_SWAP_MESH       = 0xF5A8,
        CHUNK_EVENT_MMSTARTANI      = 0xF5A9,
        // CHUNK_EVENT_CAMTREMOR       = 0xF5AA
        };

      virtual ~MdsParser()=default;

      virtual Chunk parse();

      std::vector<std::string>                  meshesASC;
      zCModelScriptAni                          ani;
      zCModelScriptAniAlias                     alias;
      zCModelScriptAniCombine                   comb;
      std::vector<zCModelScriptEventSfx>        sfx, gfx;
      std::vector<zCModelScriptEventPfx>        pfx;
      std::vector<zCModelScriptEventPfxStop>    pfxStop;
      std::vector<zCModelEvent>                 eventTag;
      std::vector<zCModelEvent>                 modelTag;
      std::vector<zCModelScriptEventMMStartAni> mmStartAni;

    protected:
      virtual Chunk       beginChunk()=0;
      virtual void        endChunk(){}
      virtual void        beginArgs(){}
      virtual void        endArgs(){}

      virtual std::string readStr()=0;
      virtual std::string readKeyword()=0;
      virtual uint32_t    readDWord()=0;
      virtual int32_t     readI32()=0;
      virtual float       readFloat()=0;

      virtual void        readMeshAndTree()=0;
      void                readRegisterMesh();
      void                readAni();
      void                readAniAlias();
      void                readSfx(std::vector<zCModelScriptEventSfx>& out);
      void                readPfx();
      void                readPfxStop();
      void                readEvent(std::vector<zCModelEvent>& out);
      void                readMMStart();
      void                readAniComb();
      EFightMode          readFMode();

      static uint32_t     makeAniFlags(const std::string &flag_str);
      static auto         makeAniDir(const std::string &str) -> EModelScriptAniDir;
    };

  class MdsParserTxt : public MdsParser {
    public:
      MdsParserTxt(ZenParser& zen);

    private:
      ZenParser&  zen;
      std::string buf;

      enum TokType : uint8_t {
        TK_Null,
        TK_Eof,
        TK_Name,
        TK_Num,
        TK_String,
        TK_BracketL,
        TK_BracketR,
        TK_Begin,
        TK_End,
        };

      TokType     nextTok(std::string &buf);
      Chunk       beginChunk() override;
      void        beginArgs() override;
      void        endArgs() override;

      void        implReadItem(TokType tt);

      std::string readStr() override;
      std::string readKeyword() override;
      uint32_t    readDWord() override;
      int32_t     readI32() override;
      float       readFloat() override;

      void        readMeshAndTree() override;
    };

  class MdsParserBin : public MdsParser {
    public:
      MdsParserBin(ZenParser& zen);

    private:
      ZenParser&  zen;
      uint32_t    chunkEnd=0;

      Chunk       beginChunk() override;
      void        endChunk() override;

      std::string readStr() override;
      std::string readKeyword() override;
      uint32_t    readDWord() override;
      int32_t     readI32() override;
      float       readFloat() override;

      void        readMeshAndTree() override;
    };

}  // namespace ZenLoad
