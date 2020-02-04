#pragma once
#include "parserImpl.h"
#include "zenParser.h"

namespace ZenLoad
{
    /**
      * @brief Templated function-calls to read the right type of data
      *		  and convert it to a string for the given type
      */
    template <typename T>
    inline void read(ZenParser& /*p*/, T& /*outData*/, const char* /*exName*/);

    template <>
    inline void read<float>(ZenParser& p, float& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(float), ParserImpl::ZVT_FLOAT);
    }

    template <>
    inline void read<bool>(ZenParser& p, bool& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(bool), ParserImpl::ZVT_BOOL);
    }

    template <>
    inline void read<uint32_t>(ZenParser& p, uint32_t& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(uint32_t), ParserImpl::ZVT_INT);
    }

    template <>
    inline void read<int32_t>(ZenParser& p, int32_t& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(int32_t), ParserImpl::ZVT_INT);
    }

    template <>
    inline void read<uint16_t>(ZenParser& p, uint16_t& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(uint16_t), ParserImpl::ZVT_WORD);
    }

    template <>
    inline void read<uint8_t>(ZenParser& p, uint8_t& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(uint16_t), ParserImpl::ZVT_BYTE);
    }

    template <>
    inline void read<ZMath::float2>(ZenParser& p, ZMath::float2& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData.x, sizeof(float), ParserImpl::ZVT_FLOAT);
        p.getImpl()->readEntry(exName, &outData.y, sizeof(float), ParserImpl::ZVT_FLOAT);
    }

    template <>
    inline void read<ZMath::float3>(ZenParser& p, ZMath::float3& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, sizeof(float) * 3, ParserImpl::ZVT_VEC3);
        /*outStr = "[" + std::to_string(outData.x) 
			+ ", " + std::to_string(outData.y)
			+ ", " + std::to_string(outData.z) + "]";*/
    }

    template <>
    inline void read<ZMath::float4>(ZenParser& p, ZMath::float4& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData.x, sizeof(float), ParserImpl::ZVT_FLOAT);
        p.getImpl()->readEntry(exName, &outData.y, sizeof(float), ParserImpl::ZVT_FLOAT);
        p.getImpl()->readEntry(exName, &outData.z, sizeof(float), ParserImpl::ZVT_FLOAT);
        p.getImpl()->readEntry(exName, &outData.w, sizeof(float), ParserImpl::ZVT_FLOAT);
        /*outStr = "[" + std::to_string(outData.x) 
			+ ", " + std::to_string(outData.y)
			+ ", " + std::to_string(outData.z)
			+ ", " + std::to_string(outData.w) + "]";*/
    }

    template <>
    inline void read<ZMath::Matrix>(ZenParser& p, ZMath::Matrix& outData, const char* exName)
    {
        float m[16];
        p.getImpl()->readEntry(exName, m, sizeof(float) * 16, ParserImpl::ZVT_RAW_FLOAT);
        outData = m;

        /*outStr = "[";
		for(size_t i = 0; i < 16; i++)
		{
			outStr += std::to_string(m[i]);

			// Only add "," when not at the last value
			if(i != 15)
				outStr += ", ";
		}
		outStr += "]";*/
    }

    template <>
    inline void read<std::string>(ZenParser& p, std::string& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, &outData, 0, ParserImpl::ZVT_STRING);
    }

    template <>
    inline void read<Daedalus::ZString>(ZenParser& p, Daedalus::ZString& outData, const char* exName)
    {
        std::string val;
        p.getImpl()->readEntry(exName, &val, 0, ParserImpl::ZVT_STRING);
        outData = Daedalus::ZString(std::move(val));
    }

    template <typename... T>
    static void ReadObjectProperties(ZenParser& ZenParser, std::unordered_map<std::string, std::string>& rval, std::pair<const char*, T*>... d)
    {
        auto fn = [&ZenParser](auto pair) {
            // Read the given datatype from the file
            read<typename std::remove_pointer<decltype(pair.second)>::type>(ZenParser, *pair.second, pair.first);
        };

        auto x = {(fn(d), 0)...};
        (void)x;
    }

    template <typename S>
    static std::pair<const char*, S*> Prop(const char* t, S& s)
    {
        return std::make_pair(t, &s);
    }
}  // namespace ZenLoad
