
#include "zCCSLib.h"
#include <algorithm>
#include "parserImpl.h"
#include "zenParser.h"
#include "zenParserPropRead.h"
#include <assert.h>
#include <utils/logger.h>
#include <vdfs/fileIndex.h>

using namespace ZenLoad;

zCCSLib::zCCSLib(const std::string& fileName, const VDFS::FileIndex& fileIndex)
{
    try
    {
        ZenLoad::ZenParser parser(fileName,fileIndex);
        readObjectData(parser);
    }
    catch (std::exception& e)
    {
        LogError() << e.what();
        return;
    }
}

zCCSLib::zCCSLib(ZenParser &parser)
{
  readObjectData(parser);
}

void zCCSLib::readObjectData(ZenParser& parser)
{
    zCCSLibData& info = m_Data;

    parser.readHeader();

    ZenParser::ChunkHeader libHeader;
    parser.readChunkStart(libHeader);

    assert(libHeader.classId == ZenParser::zCCSLib);

    uint32_t numItems = 0;
    parser.getImpl()->readEntry("NumOfItems", numItems);

    LogInfo() << "Reading " << numItems << " blocks";

    for (uint32_t i = 0; i < numItems; i++)
    {
        size_t chEnd = parser.getSeek();
        ZenParser::ChunkHeader blockHeader;
        parser.readChunkStart(blockHeader);
        chEnd += blockHeader.size;

        if(blockHeader.classId!=ZenParser::zCCSBlock)  {
          parser.skipChunk();
          continue;
          }

        zCCSBlockData blk;
        uint32_t      numBlocks = 1;
        float         subBlock0 = 0;
        ReadObjectProperties(parser,
                             Prop("blockName",   blk.blockName),
                             Prop("numOfBlocks", numBlocks),
                             Prop("subBlock0",   subBlock0));

        // Haven't seen different values for these
        assert(numBlocks == 1);
        assert(subBlock0 == 0.f);

        // Read the single atomic block
        {
        ZenParser::ChunkHeader atomicHeader;
        parser.readChunkStart(atomicHeader);
        // Read event-message of atomic block
        {
        ZenParser::ChunkHeader messageHeader;
        parser.readChunkStart(messageHeader);
        ReadObjectProperties(parser,
                             Prop("subType", blk.atomicBlockData.command.subType),
                             Prop("text", blk.atomicBlockData.command.text),
                             Prop("name", blk.atomicBlockData.command.name));

        //LogInfo() << "Read message: " << blk.atomicBlockData.command.name;
        if(!parser.readChunkEnd())
          parser.skipChunk();
        }

        if(!parser.readChunkEnd())
          parser.skipChunk();
        }
        if(!parser.readChunkEnd())
          parser.skipChunk();

        if(blockHeader.size!=0)
          parser.setSeek(chEnd);

        info.blocks.push_back(blk.atomicBlockData);

        auto nameUppered = blk.blockName;
        std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
        //LogInfo() << "message = " << blk.blockName;
        m_MessagesByName[nameUppered] = info.blocks.size() - 1;
    }
}

const oCMsgConversationData& zCCSLib::getMessageByName(const Daedalus::ZString& name)
{
    std::string nameUppered = name.c_str();
    std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
    assert(m_MessagesByName.find(nameUppered) != m_MessagesByName.end());
    size_t idx = m_MessagesByName[nameUppered];
    return m_Data.blocks[idx].command;
}

bool zCCSLib::messageExists(const Daedalus::ZString& name) const
{
    std::string nameUppered = name.c_str();
    std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
    return m_MessagesByName.find(nameUppered) != m_MessagesByName.end();
}
