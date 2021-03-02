#pragma once
#include <map>
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"

namespace ZenLoad
{
    class zCWayNet
    {
    public:
        /**
         * Reads a single waypoint
         */
        static zCWaypointData readWaypoint(ZenParser& parser);

        /**
         * Reads this object from an internal zen
         */
        static void readObjectData(zCWayNetData& info, ZenParser& parser);

    private:
    };

}  // namespace ZenLoad
