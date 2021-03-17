#pragma once
#include <vector>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "utils/mathlib.h"

namespace VDFS
{
class FileIndex;
}

namespace ZenLoad
{
class ZenParser;
class zCMorphMesh {
  public:
    struct Animation {
      std::string                name;
      int32_t                    layer=0;
      std::vector<uint32_t>      vertexIndex;
      std::vector<ZMath::float3> samples;
      uint32_t                   numFrames=0;
      };

    zCMorphMesh() = default;

    /**
     * @brief Loads the mesh from the given VDF-Archive
     */
    zCMorphMesh(const std::string& fileName, const VDFS::FileIndex& fileIndex);

    /**
     * @brief Reads the mesh-object from the given binary stream
     * @param fromZen Whether this mesh is supposed to be read from a zenfile. In this case, information about the binary chunk is also read.
     */
    void readObjectData(ZenParser& parser);

    /**
     * @return Internal zCProgMeshProto of this soft skin. The soft-skin only displaces the vertices found in the ProgMesh.
     */
    const zCProgMeshProto& getMesh() const { return m_Mesh; }

    std::vector<Animation> aniList;

  private:
    /**
     * @brief Internal zCProgMeshProto of this soft skin. The soft-skin only displaces the vertices found in the ProgMesh.
     */
    zCProgMeshProto m_Mesh;
  };
}  // namespace ZenLoad
