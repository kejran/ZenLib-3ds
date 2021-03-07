#include "parserImplBinSafe.h"
#include <utils/logger.h>

using namespace ZenLoad;

ZenLoad::ParserImplBinSafe::ParserImplBinSafe(ZenParser* parser)
    : ParserImpl(parser)
{
}

/**
* @brief Read the start of a chunk. [...]
*/
bool ParserImplBinSafe::readChunkStart(ZenParser::ChunkHeader& header)
{
    size_t seek = m_pParser->getSeek();
    EZenValueType type;
    size_t size;

    // Check if this actually will be a string
    readTypeAndSizeBinSafe(type, size);
    m_pParser->setSeek(seek);

    if(type != ZVT_STRING)
      return false;  // Next property isn't a string or the end

    // Read chunk-header
    char        vobDescStk[256] = {};
    std::string vobDescHeap;
    const char* vobDesc = vobDescStk;

    if(!readString(vobDescStk,sizeof(vobDescStk))) {
      vobDescHeap = readString();
      vobDesc     = vobDescHeap.c_str();
      }
    if(parseHeader(header,vobDesc,std::strlen(vobDesc)))
      return true;
    m_pParser->setSeek(seek);
    return false;
}

/**
* @brief Read the end of a chunk. []
*/
bool ParserImplBinSafe::readChunkEnd() {
  size_t seek = m_pParser->getSeek();
  EZenValueType type;
  size_t size;

  // Check if this actually will be a string
  readTypeAndSizeBinSafe(type, size);
  m_pParser->setSeek(seek);

  if(type != ZVT_STRING)
    return false;  // Next property isn't a string or the end

  char l[3] = {};
  if(!readString(l,3) || l[0]!='[' || l[1]!=']') {
    m_pParser->setSeek(seek);  // Next property isn't a string or the end
    return false;
    }
  return true;
  }

/**
* @brief Read the implementation specific header and stores it in the parsers main-header struct
*/
void ParserImplBinSafe::readImplHeader()
{
    // Bin-Safe archiver has its own version stored again for some reason
    m_pParser->m_Header.binSafeHeader.bsVersion = m_pParser->readBinaryDWord();

    // Read object count
    m_pParser->m_Header.objectCount = m_pParser->readBinaryDWord();

    // Read offset of where to find the global hash-table of all objects
    m_pParser->m_Header.binSafeHeader.bsHashTableOffset = m_pParser->readBinaryDWord();

    // Read hashtable
    size_t s = m_pParser->m_Seek;
    m_pParser->m_Seek = m_pParser->m_Header.binSafeHeader.bsHashTableOffset;

    struct BinSafeEntry
    {
        std::string key;
        uint16_t insertionIndex;
    };

    std::unordered_map<uint32_t, std::vector<BinSafeEntry>> map;
    uint32_t htSize = m_pParser->readBinaryDWord();
    for (uint32_t i = 0; i < htSize; i++)
    {
        uint16_t keyLen = m_pParser->readBinaryWord();
        uint16_t insIdx = m_pParser->readBinaryWord();
        uint32_t hashValue = m_pParser->readBinaryDWord();

        // Read the keys data from the archive
        std::vector<uint8_t> keyData(keyLen);
        m_pParser->readBinaryRaw(keyData.data(), keyLen);

        // Put the source key together with the insertion index into the map
        //std::string key(keyData.begin(), keyData.end());
        //map[hashValue].push_back( {key, insIdx} );
    }

    // Restore old position
    m_pParser->m_Seek = s;
}

/**
* @brief Reads a string
*/
std::string ParserImplBinSafe::readString()
{
    EZenValueType type;
    size_t size;

    // These values start with a small header
    readTypeAndSizeBinSafe(type, size);

    if (type != ZVT_STRING)
        throw std::runtime_error("Expected string-type");

    std::string str;
    str.resize(size);
    m_pParser->readBinaryRaw(&str[0], size);

    // Skip potential hash-value at the end of the string
    EZenValueType t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    if (t != ZVT_HASH)
        m_pParser->m_Seek -= sizeof(uint8_t);
    else
        m_pParser->m_Seek += sizeof(uint32_t);

    return str;
  }

bool ParserImplBinSafe::readString(char* buf, size_t bufSize)
{
  EZenValueType type;
  size_t size;

  // These values start with a small header
  readTypeAndSizeBinSafe(type, size);

  if(type!=ZVT_STRING)
    throw std::runtime_error("Expected string-type");

  if(size>=bufSize)
    return false;

  m_pParser->readBinaryRaw(buf, size);
  buf[size] = '\0';

  // Skip potential hash-value at the end of the string
  EZenValueType t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
  if (t != ZVT_HASH)
      m_pParser->m_Seek -= sizeof(uint8_t);
  else
      m_pParser->m_Seek += sizeof(uint32_t);
  return true;
}

/**
* @brief reads the small header in front of datatypes
*/
void ParserImplBinSafe::readTypeAndSizeBinSafe(EZenValueType& type, size_t& size)
{
    type = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    size = 0;

    switch (type)
    {
        case ZVT_VEC3:
            size = sizeof(float) * 3;
            break;

        case ZVT_BYTE:
            size = sizeof(uint8_t);
            break;

        case ZVT_WORD:
            size = sizeof(uint16_t);
            break;

        case ZVT_RAW:
        case ZVT_RAW_FLOAT:
        case ZVT_STRING:
            size = m_pParser->readBinaryWord();
            break;

        case ZVT_INT:
        case ZVT_ENUM:
        case ZVT_BOOL:
        case ZVT_HASH:
        case ZVT_FLOAT:
        case ZVT_COLOR:
            size = sizeof(uint32_t);  // FIXME: Bool might be only 8 bytes
            break;

        default:
            throw std::runtime_error(std::string("BinSafe: Datatype not implemented"));
    }
}

/**
* @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
*/
void ParserImplBinSafe::readEntry(const char *expectedName, void* target, size_t targetSize, EZenValueType expectedType)
{
    EZenValueType type;
    size_t size;

    // Read type and size of the entry
    readTypeAndSizeBinSafe(type, size);

    // These are the same in size
    if (expectedType == ZVT_INT)
    {
        if (type == ZVT_COLOR)
            type = ZVT_INT;

        if (type == ZVT_ENUM && size == sizeof(uint32_t))
            type = ZVT_INT;  // Enum seems the be there in both 1 and 4 byte?
    }

    if (expectedType != ZVT_BYTE || type != ZVT_ENUM)  // International CSLibs have this
        if (expectedType != ZVT_0 && type != expectedType)
            throw std::runtime_error("Valuetype name does not match expected type. Value:" + std::string(expectedName));

    switch (type)
    {
        case ZVT_0:
            break;
        case ZVT_STRING:
        {
            std::string str;
            str.resize(size);
            m_pParser->readBinaryRaw(&str[0], size);
            *reinterpret_cast<std::string*>(target) = str;
        }
        break;

        case ZVT_HASH:
        case ZVT_INT:
            *reinterpret_cast<uint32_t*>(target) = m_pParser->readBinaryDWord();
            break;
        case ZVT_FLOAT:
            *reinterpret_cast<float*>(target) = m_pParser->readBinaryFloat();
            break;
        case ZVT_BYTE:
            *reinterpret_cast<uint8_t*>(target) = m_pParser->readBinaryByte();
            break;
        case ZVT_WORD:
            *reinterpret_cast<int16_t*>(target) = m_pParser->readBinaryWord();
            break;
        case ZVT_BOOL:
            *reinterpret_cast<bool*>(target) = m_pParser->readBinaryDWord() != 0;
            break;
        case ZVT_VEC3:
            reinterpret_cast<ZMath::float3*>(target)->x = m_pParser->readBinaryFloat();
            reinterpret_cast<ZMath::float3*>(target)->y = m_pParser->readBinaryFloat();
            reinterpret_cast<ZMath::float3*>(target)->z = m_pParser->readBinaryFloat();
            break;

        case ZVT_COLOR:
            reinterpret_cast<uint8_t*>(target)[0] = m_pParser->readBinaryByte();  // FIXME: These are may ordered wrong
            reinterpret_cast<uint8_t*>(target)[1] = m_pParser->readBinaryByte();
            reinterpret_cast<uint8_t*>(target)[2] = m_pParser->readBinaryByte();
            reinterpret_cast<uint8_t*>(target)[3] = m_pParser->readBinaryByte();
            break;

        case ZVT_RAW_FLOAT:
        case ZVT_RAW:
            m_pParser->readBinaryRaw(target, size);
            break;
        case ZVT_10:
            break;
        case ZVT_11:
            break;
        case ZVT_12:
            break;
        case ZVT_13:
            break;
        case ZVT_14:
            break;
        case ZVT_15:
            break;
        case ZVT_ENUM:
            *reinterpret_cast<uint8_t*>(target) = m_pParser->readBinaryDWord();
            break;
    }

    // Skip potential hash-value at the end of the entry
    EZenValueType t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    if (t != ZVT_HASH)
        m_pParser->m_Seek -= sizeof(uint8_t);
    else
        m_pParser->m_Seek += sizeof(uint32_t);
}

/**
* @brief Reads the type of a single entry
*/
void ParserImplBinSafe::readEntryType(EZenValueType& outtype, size_t& size)
{
    readTypeAndSizeBinSafe(outtype, size);
}
