#include "DaedalusDialogManager.h"
#include <algorithm>
#include "DaedalusStdlib.h"
#include "DaedalusVM.h"
#include <utils/logger.h>

using namespace Daedalus;
using namespace GameState;
using namespace ZenLoad;

DaedalusDialogManager::DaedalusDialogManager(Daedalus::DaedalusVM& vm,
                                             const std::string& ou_bin,
                                             const VDFS::FileIndex& vdfsFileIndex,
                                             std::map<size_t, std::set<size_t>>& knownInfos)
    : m_VM(vm)
    , m_MessageLib(ou_bin, vdfsFileIndex)
    , m_KnownNpcInfoSymbolsByNpcSymbols(knownInfos)
{
}

DaedalusDialogManager::DaedalusDialogManager(Daedalus::DaedalusVM& vm,
                                             const std::string& ou_bin,
                                             std::map<size_t, std::set<size_t>>& knownInfos)
    : m_VM(vm)
    , m_MessageLib(ou_bin)
    , m_KnownNpcInfoSymbolsByNpcSymbols(knownInfos)
{
}

void DaedalusDialogManager::setNpcInfoKnown(size_t npcInstance, size_t infoInstance)
{
    //LogInfo() << "He knows! (" << m_VM.getDATFile().getSymbolByIndex(npcInstance).name << " -> " << m_VM.getDATFile().getSymbolByIndex(infoInstance).name << ")!";
    m_KnownNpcInfoSymbolsByNpcSymbols[npcInstance].insert(infoInstance);
}

bool DaedalusDialogManager::doesNpcKnowInfo(size_t npcInstance, size_t infoInstance)
{
    const auto& m = m_KnownNpcInfoSymbolsByNpcSymbols[npcInstance];
    return m.find(infoInstance) != m.end();
}
