#include "zCMorphMesh.h"
#include <string>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/alignment.h"
#include "utils/logger.h"
#include "vdfs/fileIndex.h"

#include <fstream>

using namespace ZenLoad;

enum MSID_CHUNK : uint16_t {
  MSID_NONE        = 0,
  MMID_MMB_HEADER  = 0xE020,
  MMID_MMB_ANILIST = 0xE030,
  MMID_UNKNOWN0    = 0xE000,
  MMID_UNKNOWN1    = 0xE010
  };

zCMorphMesh::zCMorphMesh(const std::string& fileName, const VDFS::FileIndex& fileIndex)
{
    std::vector<uint8_t> data;
    fileIndex.getFileData(fileName, data);

    if (data.empty())
        return;  // TODO: Throw an exception or something

    try
    {
        // Create parser from memory
        // FIXME: There is an internal copy of the data here. Optimize!
        ZenLoad::ZenParser parser(data.data(), data.size());

        readObjectData(parser);
    }
    catch (std::exception& e)
    {
        LogError() << e.what();
        return;
    }
}

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCMorphMesh::readObjectData(ZenParser& parser) {
  // Information about a single chunk
  BinaryChunkInfo chunkInfo;

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  bool doneReadingChunks = false;
  while (!doneReadingChunks) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    switch (chunkInfo.id) {
      case MMID_MMB_HEADER: {
        uint32_t version = parser.readBinaryDWord();

        std::string morphProtoName = parser.readLine(true);
        //LogInfo() << "MorphProtoName: " << morphProtoName;

        // Read source-mesh
        m_Mesh.readObjectData(parser);

        // TODO: use these
        std::vector<ZMath::float3> morphPositions(m_Mesh.getVertices().size());
        parser.readBinaryRaw(morphPositions.data(), sizeof(ZMath::float3) * m_Mesh.getVertices().size());
        break;
        }

      case MMID_MMB_ANILIST: {
        doneReadingChunks = true;
        uint16_t aniTotal=0;
        parser.readBinaryRaw(&aniTotal,2);
        aniList.resize(aniTotal);
        for(auto& i:aniList) {
          i.name = parser.readString(false);
          parser.setSeek(parser.getSeek()+22); // unknown data

          uint32_t indexSz=parser.readBinaryDWord();
          i.numFrames = parser.readBinaryDWord();

          i.vertexIndex.resize(indexSz);
          i.samples.resize(i.numFrames*indexSz);
          parser.readBinaryRaw(i.vertexIndex.data(),indexSz*4);
          static_assert(sizeof(i.samples[0])==12,"invalid ani sample size");
          parser.readBinaryRaw(i.samples.data(),i.numFrames*indexSz*12);
          }

        parser.setSeek(chunkEnd);
        break;
        }

      case MMID_UNKNOWN0:
      case MMID_UNKNOWN1: {
        parser.setSeek(chunkEnd);
        break;
        }

      // Morphmeshes don't have a real end-tag. The anilist is the last one, however
      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }
