#include "parserImplASCII.h"
#include <algorithm>
#include "utils/logger.h"

using namespace ZenLoad;

ParserImplASCII::ParserImplASCII(ZenParser* parser)
    : ParserImpl(parser)
{
}

/**
 * @brief Read the start of a chunk. [...]
 */
bool ParserImplASCII::readChunkStart(ZenParser::ChunkHeader& header)
{
    size_t seek = m_pParser->getSeek();
    // Check chunk-header
    m_pParser->skipSpaces();

    // Early exit if this is a chunk-end or not a chunk header
    if(m_pParser->m_Data[m_pParser->m_Seek] != '[' || m_pParser->m_Data[m_pParser->m_Seek + 1] == ']')
    {
      m_pParser->setSeek(seek);
      return false;
    }

    size_t tmpSeek = m_pParser->m_Seek;
    while (m_pParser->m_Data[tmpSeek] != ']')
    {
        if (m_pParser->m_Data[tmpSeek] == '\r' || m_pParser->m_Data[tmpSeek] == '\n')
            throw std::runtime_error("Invalid vob descriptor");

        ++tmpSeek;
    }

    // Parse chunk-header
    char        vobDescStk[256] = {};
    std::string vobDescHeap;
    const char* vobDesc = vobDescStk;

    if(!readString(vobDescStk,sizeof(vobDescStk))) {
      vobDescHeap = readString();
      vobDesc     = vobDescHeap.c_str();
      }

    if(parseHeader(header,vobDesc,std::strlen(vobDesc))) {
      // Skip the last newline
      m_pParser->skipSpaces();
      return true;
      }
    m_pParser->setSeek(seek);
    return false;
}

/**
 * @brief Read the end of a chunk. []
 */
bool ParserImplASCII::readChunkEnd()
{
    size_t seek = m_pParser->getSeek();

    m_pParser->skipSpaces();
    char l[3] = {};
    if(!readString(l,3) || l[0]!='[' || l[1]!=']') {
      m_pParser->setSeek(seek);  // Next property isn't a string or the end
      return false;
      }

    m_pParser->skipSpaces();
    return true;
}

/**
 * @brief Read the implementation specific header and stores it in the parsers main-header struct
 */
void ParserImplASCII::readImplHeader()
{
    if (!m_pParser->skipString("objects"))
        throw std::runtime_error("Object count missing");

    m_pParser->m_Header.objectCount = m_pParser->readIntASCII();

    if (!m_pParser->skipString("END"))
        throw std::runtime_error("No END in header(2)");

    m_pParser->skipSpaces();
}

/**
 * @brief Reads a string
 */
std::string ParserImplASCII::readString()
{
  return m_pParser->readLine();
}

bool ParserImplASCII::readString(char* buf, size_t size)
{
  return m_pParser->readLine(buf,size);
}

/**
 * @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
 */
void ParserImplASCII::readEntry(const char* expectedName, void* target, size_t targetSize, EZenValueType expectedType)
{
    m_pParser->skipSpaces();

    std::string lineHeap;
    char        lineStk[256] = {};
    char*       line = lineStk;
    if(!readString(lineStk,sizeof(lineStk)))
    {
      lineHeap = m_pParser->readLine();
      line     = lineHeap.size()>0 ? &lineHeap[0] : lineStk;
    }

    size_t lnSize = std::strlen(line);
    if(lnSize>=1) {
      // FIXME?
      if(line[0]=='[')
        return;
      }
    /* Examples:
     * childs0=int:1
     *
     * Syntax:
     * <name>=<type>:<value>
     * */

    const char* name  = line;
    const char* type  = "";
    const char* value = "";

    size_t i=0;
    for(; i<lnSize; ++i) {
      if(line[i]=='=') {
        line[i] = '\0';
        if(i+1<lnSize)
          type = line+i+1;
        break;
        }
      }

    for(; i<lnSize; ++i) {
      if(line[i]==':') {
        line[i] = '\0';
        if(i+1<lnSize)
          value = line+i+1;
        break;
        }
      }

    if(expectedName!=nullptr && expectedName[0]!='\0') {
      for(size_t i=0; ; ++i) {
        char e = expectedName[i];
        char n = name[i];
        if('A'<=e && e<='Z')
          e = char(e-'A'+'a');
        if('A'<=n && n<='Z')
          n = char(n-'A'+'a');
        if(e!=n) {
          throw std::runtime_error(std::string("Value name does not match expected name. Value:") + name + " Expected: " + expectedName);
          }
        if(expectedName[i]=='\0')
          break;
        }
      }

    (void)type;
    switch(expectedType) {
      case ZVT_0:
        break;
      case ZVT_STRING:
        *reinterpret_cast<std::string*>(target) = value;
        break;
      case ZVT_INT:
        *reinterpret_cast<int32_t*>(target) = std::stoi(value);
        break;
      case ZVT_FLOAT:
        *reinterpret_cast<float*>(target) = std::stof(value);
        break;
      case ZVT_BYTE:
        *reinterpret_cast<uint8_t*>(target) = static_cast<uint8_t>(std::stoi(value));
        break;
      case ZVT_WORD:
        *reinterpret_cast<int16_t*>(target) = static_cast<int16_t>(std::stoi(value));
        break;
      case ZVT_BOOL:
        *reinterpret_cast<bool*>(target) = std::stoi(value) != 0;
        break;
      case ZVT_VEC3:
        *reinterpret_cast<ZMath::float3*>(target) = parseVec3(value);
        break;
      case ZVT_COLOR:
        parseU8Vec(value,reinterpret_cast<uint8_t*>(target),targetSize);
        break;
      case ZVT_RAW_FLOAT:
        parseFloatVec(value,reinterpret_cast<float*>(target),targetSize);
        break;
      case ZVT_RAW:
        parseRawVec(value,reinterpret_cast<uint8_t*>(target),targetSize);
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
        *reinterpret_cast<uint8_t*>(target) = std::stoi(value);
        break;
      case ZVT_HASH:
        break;
      }
}

/**
* @brief Reads the type of a single entry
*/
void ParserImplASCII::readEntryType(EZenValueType& outtype, size_t& size)
{
    m_pParser->skipSpaces();
    std::string line = m_pParser->readLine();

    // Special cases for chunk starts/ends
    if (line == "[]" || (line.front() == '[' && line.back() == ']'))
    {
        outtype = ZVT_STRING;
        size = 0;
        return;
    }

    // Split at =, then at :
    // parts should then have 3 elements, name, type and value
    auto parts = Utils::split(line, "=:");

    if (parts.size() < 2)  // Need at least name and type
        throw std::runtime_error("Failed to read property type");

    const std::string& type = parts[1];
    size_t cLoc = line.find_first_of(':') == std::string::npos ? 0 : line.find_first_of(':');

    if (type == "string")
    {
        outtype = ZVT_STRING;
        size = 0;
    }
    else if (type == "int")
    {
        outtype = ZVT_INT;
        size = 0;
    }
    else if (type == "float")
    {
        outtype = ZVT_FLOAT;
        size = 0;
    }
    else if (type == "byte")
    {
        outtype = ZVT_BYTE;
        size = 0;
    }
    else if (type == "word")
    {
        outtype = ZVT_WORD;
        size = 0;
    }
    else if (type == "bool")
    {
        outtype = ZVT_BOOL;
        size = 0;
    }
    else if (type == "vec3")
    {
        outtype = ZVT_VEC3;
        size = 0;
    }
    else if (type == "color")
    {
        outtype = ZVT_COLOR;
        size = 0;
    }
    else if (type == "rawFloat")
    {
        outtype = ZVT_RAW_FLOAT;
        size = 0;
    }
    else if (type == "raw")
    {
        outtype = ZVT_RAW;
        size = 0;
    }
    else if (type == "enum")
    {
        outtype = ZVT_ENUM;
        size = 0;
    }
    else
      throw std::runtime_error("Unknown type");
}

ZMath::float3 ParserImplASCII::parseVec3(const char* line) const
{
  //*reinterpret_cast<ZMath::float3*>(target) = ZMath::float3(std::stof(vparts[0]), std::stof(vparts[1]), std::stof(vparts[2]));
  float ret[3] = {};
  parseFloatVec(line,ret,sizeof(ret));
  return ZMath::float3(ret[0],ret[1],ret[2]);
}

void ParserImplASCII::parseFloatVec(const char* line, float* target, size_t targetSize) const
{
  targetSize /= sizeof(float);
  for(size_t i=0; i<targetSize; ++i) {
    if(*line=='\0')
      break;
    while(std::isspace(*line))
      ++line;
    size_t pos = 0;
    target[i] = std::stof(line,&pos);
    line+=pos;
    }
}

void ParserImplASCII::parseU8Vec(const char* line, uint8_t* target, size_t targetSize) const
{
  for(size_t i=0; i<targetSize; ++i) {
    if(*line=='\0')
      break;
    while(std::isspace(*line))
      ++line;
    size_t pos = 0;
    target[i] = std::stoi(line,&pos);
    line+=pos;
    }
}

void ParserImplASCII::parseRawVec(const char* line, uint8_t* target, size_t targetSize) const
{
  for(size_t i=0; i<targetSize; ++i)
  {
    if(line[0]=='\0' || line[1]=='\0')
      break;
    int c[2] = {};
    for(int r=0; r<2; ++r) {
      if('0'<=line[r] && line[r]<='9')
        c[r] = line[r]-'0';
      else if('a'<=line[r] && line[r]<='f')
        c[r] = line[r]-'a'+10;
      else if('A'<=line[r] && line[r]<='F')
        c[r] = line[r]-'a'+10;
      }
    target[i] = c[0]*16+c[1];
  }
}

