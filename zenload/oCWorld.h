#pragma once

#include "zCVob.h"
#include "zCWayNet.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"

namespace ZenLoad
{

class oCWorld
{
public:
  enum class WorldVersionInternal {
    VERSION_G1_08k = 64513,
    VERSION_G26fix = 0
    };

  static size_t readVobTree(ZenParser& parser, std::vector<zCVobData>& target, WorldVersion worldversion);
  /**
    * Reads this object from an internal zen
    */
  static void readObjectData(oCWorldData& info, ZenParser& parser, uint32_t versionInternal, bool forceG2);
};

}  // namespace ZenLoad
