#pragma once
#include <functional>
#include <set>
#include <string>
#include <vector>
#include "DaedalusGameState.h"
#include <zenload/zCCSLib.h>

namespace ZenLoad
{
    struct oCMsgConversationData;
}

namespace VDFS
{
    class FileIndex;
}

namespace Daedalus
{
    class DaedalusVM;

    namespace GameState
    {
        class DaedalusDialogManager
        {
        public:
            DaedalusDialogManager(Daedalus::DaedalusVM& vm,
                                  const std::string& ou_bin,
                                  std::map<size_t, std::set<size_t>>& knownInfos);

            DaedalusDialogManager(Daedalus::DaedalusVM& vm,
                                  const std::string& ou_bin,
                                  const VDFS::FileIndex& vdfsFileIndex,
                                  std::map<size_t, std::set<size_t>>& knownInfos);
            /**
			 * Sets the given info-instance as "known" to the given NPC-Instance
			 */
            void setNpcInfoKnown(size_t npcInstance, size_t infoInstance);

            /**
             * Checks whether the given NPC knows the info passed here
             * @param npcInstance NPC to check
             * @param infoInstance Info to check
             * @return Whether the NPC knows
             */
            bool doesNpcKnowInfo(size_t npcInstance, size_t infoInstance);

            /**
             * @return Map of NPC-Symbols -> All info they know
             */
            const std::map<size_t, std::set<size_t>>& getKnownNPCInformation() { return m_KnownNpcInfoSymbolsByNpcSymbols; };

            /**
             * @return MessageLib
             */
            ZenLoad::zCCSLib& getMessageLib() { return m_MessageLib; }

        private:

            /**
			 * Message library
			 */
            ZenLoad::zCCSLib m_MessageLib;

            /**
			 * Current daedalus VM
			 */
            Daedalus::DaedalusVM& m_VM;

            /**
			 * map of all known infoInstances by npcInstance
			 */
            std::map<size_t, std::set<size_t>>& m_KnownNpcInfoSymbolsByNpcSymbols;
        };
    }  // namespace GameState
}  // namespace Daedalus
