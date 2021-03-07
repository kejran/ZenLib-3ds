#pragma once
#include <map>
#include "zTypes.h"

namespace VDFS
{
    class FileIndex;
}

namespace Daedalus
{
     class ZString;
}

namespace ZenLoad
{
    class ZenParser;
    class zCCSLib
    {
    public:
        /**
		 * @brief Loads the file from the given VDF-Archive
		 */
        zCCSLib() = default;
        zCCSLib(const std::string& fileName, const VDFS::FileIndex& fileIndex);
        zCCSLib(ZenParser &parser);

        /**
        * Reads this object from an internal zen
        */
        void readObjectData(ZenParser& parser);

        /**
         * @return the message of the given name
         */
        const oCMsgConversationData& getMessageByName(const Daedalus::ZString& name);

        /**
         * @return true if the message was found
         */
        bool messageExists(const Daedalus::ZString& name) const;

        const zCCSLibData& getData() { return m_Data; }

    private:
        /**
         * Loaded data straight from the file
         */
        zCCSLibData m_Data;

        /**
         * Message indices by their names
         */
        std::map<std::string, size_t> m_MessagesByName;
    };
}  // namespace ZenLoad
