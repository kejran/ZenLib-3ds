#pragma once

#include <functional>
#include <algorithm>
#include <cassert>

#include "zTypes.h"

namespace ZenLoad
{
    class ZenParser;
    class zCMesh;

    class zCBspTree
    {
    private:
        static void connectPortals(zCBspTreeData& info);

    public:
        enum EVersion
        {
            Gothic_26f = 0,  // TODO
            Gothic_18k = 34144256
        };

        enum EBspChunk
        {
            CHUNK_BSP = 0xC000,
            CHUNK_BSP_POLYLIST = 0xC010,
            CHUNK_BSP_TREE = 0xC040,
            CHUNK_BSP_LEAF_LIGHT = 0xC045,
            CHUNK_BSP_OUTDOOR_SECTORS = 0xC050,
            CHUNK_BSP_END = 0xC0FF,
        };

        /**
        * Reads this object from an internal zen
        */
        static zCBspTreeData readObjectData(ZenParser& parser, zCMesh* mesh);

        /**
         * Returns a list of polygon-indices which are not LOD-polyons
         * @param d Loaded BSP-Tree data
         * @return List of indices to polygons, which are not LOD-Polygons. These are the
         *         actual indices of the polygons inside the mesh.
         */
        static std::vector<size_t> getNonLodPolygons(const zCBspTreeData& d)
        {
            size_t size = 0;
            for(size_t nidx : d.leafIndices)
            {
              const zCBspNode& n = d.nodes[nidx];
              size += n.numPolys;
            }

            std::vector<size_t> r(size);
            size = 0;

            for(size_t nidx : d.leafIndices)
            {
              const zCBspNode& n      = d.nodes[nidx];
              const uint32_t*  treeId = d.treePolyIndices.data()+n.treePolyIndex;
              size_t*          ret    = r.data()+size;
              for(size_t i=0; i<n.numPolys; ++i)
                ret[i] = treeId[i];
              size += n.numPolys;
            }

            return r;
        }

    private:
        static void loadRec(ZenParser& parser,const BinaryFileInfo& fileInfo,zCBspTreeData& info,size_t idx,bool isNode);
        static SectorIndex findSectorIndexByName(zCBspTreeData& info, const std::string& sectorname)
        {
            for(size_t i = 0; i < info.sectors.size(); i++)
            {
                if(info.sectors[i].name == sectorname)
                    return SectorIndex(i);
            }

            return SECTOR_INDEX_INVALID;
        }

        /**
         * Given a material name of "X:abcd_efgh", returns "abcd".
         */
        static std::string extractSourceSectorFromMaterialName(std::string name)
        {
            std::string sectorsOnly = name.substr(name.find_first_of(':') + 1);

            return sectorsOnly.substr(0, sectorsOnly.find_first_of('_'));
        }

        /**
         * Given a material name of "X:abcd_efgh", returns "efgh".
         */
        static std::string extractDestSectorFromMaterial(std::string name)
        {
            std::string sectorsOnly = name.substr(name.find_first_of(':') + 1);

            return sectorsOnly.substr(sectorsOnly.find_first_of('_') + 1);
        }

        static bool isMaterialForPortal(const zCMaterialData& m) {
          return m.matName.find("P:")==0;
          }

        static bool isMaterialForSector(const zCMaterialData& m) {
          return m.matName.find("S:")==0;
          }

        /**
         * Extracts the information given by the various indices inside the BspTree-Structure and packs them
         * into accessible objects.
         * @param info Loaded BSP-Tree data
         * @param worldMesh Loaded world mesh. Needed to access material names, which encode portal information
         */
        static void connectPortals(zCBspTreeData& info, zCMesh* worldMesh);
    };
}  // namespace ZenLoad
