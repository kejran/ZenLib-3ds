#pragma once
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"

namespace ZenLoad
{
class zCVob {
  struct packedVobData;
  enum {
    VERSION_G1_08k = 12289,
    VERSION_G26fix = 0  // TODO
    };

  public:
    /**
      * Reads this object from an internal zen
      */
    static void readObjectData(zCVobData& info, ZenParser& parser,
                               const ZenParser::ChunkHeader& header, ZenParser::FileVersion version);
  };
}  // namespace ZenLoad
