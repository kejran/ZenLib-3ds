#pragma once

#include <functional>
#include <algorithm>
#include <cassert>

#include "zCMesh.h"
#include "zTypes.h"
#include "zenParser.h"

namespace ZenLoad
{
    class ZenParser;
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
        static zCBspTreeData readObjectData(ZenParser& parser, zCMesh* mesh)
        {
            zCBspTreeData info;

            // Information about the whole file we are reading here
            BinaryFileInfo fileInfo;
            uint32_t version = 0;
            size_t binFileEnd;  // Ending location of the binary file

            // Read information about the current file. Mainly size is important here.
            parser.readStructure(fileInfo);

            // Calculate ending location and thus, the filesize
            binFileEnd = parser.getSeek() + fileInfo.size;

            // Read the BSP-Tree first
            size_t meshPosition = parser.getSeek();
            zCMesh::skip(parser);
            //mesh->readObjectData(parser);

            // Information about a single chunk
            BinaryChunkInfo chunkInfo;

            bool done = false;
            while (!done && parser.getSeek() < binFileEnd)
            {
                // Read chunk header and calculate position of next chunk
                parser.readStructure(chunkInfo);

                size_t chunkEnd = parser.getSeek() + chunkInfo.length;

                switch (chunkInfo.id) {
                  case EBspChunk::CHUNK_BSP:
                    version = parser.readBinaryWord();
                    info.mode = static_cast<zCBspTreeData::TreeMode>(parser.readBinaryDWord());
                    break;

                  case CHUNK_BSP_POLYLIST: {
                    uint32_t numPolys = parser.readBinaryDWord();
                    info.treePolyIndices.resize(numPolys);
                    parser.readBinaryRaw(info.treePolyIndices.data(),numPolys*sizeof(uint32_t));
                    }
                    break;

                  case CHUNK_BSP_TREE: {
                    uint32_t numNodes = parser.readBinaryDWord();
                    uint32_t numLeafs = parser.readBinaryDWord();

                    if(!numNodes) {
                      parser.setSeek(chunkEnd);  // Skip chunk
                      break;
                      }

                    info.nodes.reserve(numNodes);
                    info.leafIndices.reserve(numLeafs);
                    info.nodes.emplace_back();

                    loadRec(parser,fileInfo,info,0,true);
                    assert(numNodes==info.nodes.size());
                    assert(numLeafs==info.leafIndices.size());
                    }
                    break;

                  case CHUNK_BSP_LEAF_LIGHT:
                    break;

                  case CHUNK_BSP_OUTDOOR_SECTORS: {
                    uint32_t numSectors = parser.readBinaryDWord();

                    // Sectors
                    for(uint32_t i = 0; i < numSectors; i++) {
                      info.sectors.emplace_back();
                      zCSector& sector = info.sectors.back();

                      sector.name = parser.readLine(false);
                      uint32_t numSectorNodes = parser.readBinaryDWord();
                      uint32_t numSectorPortals = parser.readBinaryDWord();

                      // Read Nodes this sector belongs to
                      sector.bspNodeIndices.resize(numSectorNodes);
                      parser.readBinaryRaw(sector.bspNodeIndices.data(),numSectorNodes*sizeof(uint32_t));

                      // Read portals in/out of this sector
                      sector.portalPolygonIndices.resize(numSectorPortals);
                      parser.readBinaryRaw(sector.portalPolygonIndices.data(),numSectorPortals*sizeof(uint32_t));
                      }

                    // Portal-list
                    uint32_t numPortals = parser.readBinaryDWord();
                    info.portalPolyIndices.resize(numPortals);
                    parser.readBinaryRaw(info.portalPolyIndices.data(),numPortals*sizeof(uint32_t));
                    }
                    break;

                  case CHUNK_BSP_END:
                    done = true;
                    break;
                  }
                parser.setSeek(chunkEnd);  // Skip chunk
            }
            (void)version;

            // Now get the list of non-lod polygons to load the worldmesh without them
            std::vector<size_t> nonLodPolys = getNonLodPolygons(info);

            std::sort(nonLodPolys.begin(), nonLodPolys.end());
            nonLodPolys.erase(std::unique(nonLodPolys.begin(), nonLodPolys.end()), nonLodPolys.end());

            // Reset to mesh position
            size_t seek = parser.getSeek();
            parser.setSeek(meshPosition);
            mesh->readObjectData(parser, nonLodPolys);

            // Make access to portals and sectors easier by packing them in better structures
            connectPortals(info, mesh);
            parser.setSeek(binFileEnd);

            return info;
        }

        /**
         * Returns a list of polygon-indices which are not LOD-polyons
         * @param d Loaded BSP-Tree data
         * @return List of indices to polygons, which are not LOD-Polygons. These are the
         *         actual indices of the polygons inside the mesh.
         */
        static std::vector<size_t> getNonLodPolygons(const zCBspTreeData& d)
        {
            std::vector<size_t> r;

            for (size_t nidx : d.leafIndices)
            {
                const zCBspNode& n = d.nodes[nidx];

                for (size_t i = n.treePolyIndex; i < n.treePolyIndex + n.numPolys; i++)
                    r.push_back(d.treePolyIndices[i]);
            }

            return r;
        }

    private:
        static void loadRec(ZenParser& parser,const BinaryFileInfo& fileInfo,zCBspTreeData& info,size_t idx,bool isNode) {
          zCBspNode& n = info.nodes[idx];

          parser.readStructure(n.bbox3dMin);
          parser.readStructure(n.bbox3dMax);

          // Read indices to the polys this contains
          n.treePolyIndex = static_cast<size_t>(parser.readBinaryDWord());
          n.numPolys      = static_cast<size_t>(parser.readBinaryDWord());

          // Only need to load data if this isn't a leaf
          if(isNode) {
              /**
               * Flags:
               * 1: front
               * 2: back
               * 4: front is leaf
               * 8: back is leaf
               */
              enum {
                FLAG_FRONT         = 1,
                FLAG_BACK          = 2,
                FLAG_FRONT_IS_LEAF = 4,
                FLAG_BACK_IS_LEAF  = 8,
                };

              // flags tell if this node got children and whether they are leafs
              uint8_t flags = parser.readBinaryByte();
              parser.readStructure(n.plane.w);
              parser.readStructure(n.plane.x);
              parser.readStructure(n.plane.y);
              parser.readStructure(n.plane.z);

              // G1 has an extra byte here
              if(fileInfo.version==Gothic_18k)
                parser.readBinaryByte();  // Lod-flag

              uint32_t front = zCBspNode::INVALID_NODE;
              uint32_t back  = zCBspNode::INVALID_NODE;


              if((flags & FLAG_FRONT)!=0) {
                front=info.nodes.size();
                info.nodes.emplace_back();
                }
              // Read front node
              if(front != zCBspNode::INVALID_NODE) {
                // Assign index and add actual node
                info.nodes[idx].front = front;
                // Set new nodes parent
                info.nodes[front].parent = idx;

                // If this is a leaf, add it to the leaf-list
                if((flags & FLAG_FRONT_IS_LEAF)!=0)
                  info.leafIndices.push_back(front);

                // Continue to load the tree
                loadRec(parser,fileInfo,info,front,(flags & FLAG_FRONT_IS_LEAF) == 0);
                }

              if((flags & FLAG_BACK)!=0) {
                back=info.nodes.size();
                info.nodes.emplace_back();
                }
              // Read back node
              if(back != zCBspNode::INVALID_NODE) {
                // Assign index and add actual node
                info.nodes[idx].back = back;
                // Set new nodes parent
                info.nodes[back].parent = idx;

                // If this is a leaf, add it to the leaf-list
                if((flags & FLAG_BACK_IS_LEAF) != 0)
                  info.leafIndices.push_back(back);

                // Continue to load the tree
                loadRec(parser,fileInfo,info,back,(flags & FLAG_BACK_IS_LEAF) == 0);
                }
          }
          else
          {
              //LogInfo() << idx << " Leaf!";
          }
        }

        static SectorIndex findSectorIndexByName(zCBspTreeData& info, const std::string& sectorname)
        {
            for(size_t i = 0; i < info.sectors.size(); i++)
            {
                if(info.sectors[i].name == sectorname)
                    return i;
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
        static void connectPortals(zCBspTreeData& info, zCMesh* worldMesh)
        {
            for(const zCMaterialData& m : worldMesh->getMaterials())
            {
                if(isMaterialForPortal(m))
                {
                    std::string from = extractSourceSectorFromMaterialName(m.matName);
                    std::string to   = extractDestSectorFromMaterial(m.matName);

                    info.portals.emplace_back();
                    zCPortal& portal = info.portals.back();

                    portal.frontSectorName = from;
                    portal.backSectorName = to;

                    portal.frontSectorIndex = findSectorIndexByName(info, from);
                    portal.backSectorIndex =  findSectorIndexByName(info, to);

                    //LogInfo() << "Portal material: " << m.matName;
                    //LogInfo() << " - Source: " << extractSourceSectorFromMaterialName(m.matName);
                    //LogInfo() << " - Dest: " << extractDestSectorFromMaterial(m.matName);
                }
                else if(isMaterialForSector(m))
                {
                    std::string to = extractDestSectorFromMaterial(m.matName);

                    info.portals.emplace_back();
                    zCPortal& portal = info.portals.back();

                    // Inner sector portals get the same sector as front and back.
                    // They're named like "S:_dest"
                    portal.backSectorIndex =  findSectorIndexByName(info, to);
                    portal.frontSectorIndex = portal.backSectorIndex;

                    portal.frontSectorName = to;
                    portal.backSectorName = to;

                    //LogInfo() << "Sector material: " << m.matName;
                    //LogInfo() << " - Source: " << extractSourceSectorFromMaterialName(m.matName);
                    //LogInfo() << " - Dest: " << extractDestSectorFromMaterial(m.matName);
                }
            }
        }
    };
}  // namespace ZenLoad
