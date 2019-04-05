//
// Created by andre on 24.05.16.
//

#include "DaedalusGameState.h"
#include "DaedalusVM.h"
#include <utils/logger.h>

using namespace ZenLoad;
using namespace Daedalus;
using namespace GameState;

DaedalusGameState::DaedalusGameState(Daedalus::DaedalusVM& vm):m_VM(vm) {
  }

Daedalus::GEngineClasses::Instance* DaedalusGameState::getByClass(void* h,EInstanceClass) {
  return reinterpret_cast<Daedalus::GEngineClasses::Instance*>(h);
  }

template <typename C_Class>
C_Class *DaedalusGameState::create() {
  return new C_Class();
  }

GEngineClasses::C_Item* DaedalusGameState::createItem() {
  return create<GEngineClasses::C_Item>();
  }

GEngineClasses::C_ItemReact* DaedalusGameState::createItemReact() {
  return create<GEngineClasses::C_ItemReact>();
  }

GEngineClasses::C_Mission* DaedalusGameState::createMission() {
  return create<GEngineClasses::C_Mission>();
  }

GEngineClasses::C_Info *DaedalusGameState::createInfo() {
  return create<GEngineClasses::C_Info>();
  }

GEngineClasses::C_Focus *DaedalusGameState::createFocus() {
  return create<GEngineClasses::C_Focus>();
  }

GEngineClasses::C_SFX *DaedalusGameState::createSfx() {
  return create<GEngineClasses::C_SFX>();
  }

GEngineClasses::C_ParticleFX *DaedalusGameState::createPfx() {
  return create<GEngineClasses::C_ParticleFX>();
  }

GEngineClasses::C_Menu *DaedalusGameState::createMenu() {
  return create<GEngineClasses::C_Menu>();
  }

GEngineClasses::C_Menu_Item *DaedalusGameState::createMenuItem() {
  return create<GEngineClasses::C_Menu_Item>();
  }

GEngineClasses::C_MusicTheme *DaedalusGameState::createMusicTheme() {
  return create<GEngineClasses::C_MusicTheme>();
  }

GEngineClasses::C_GilValues *DaedalusGameState::createGilValues() {
  return create<GEngineClasses::C_GilValues>();
  }

GEngineClasses::C_Npc* DaedalusGameState::insertNPC(size_t instance, const std::string& waypoint) {
  auto npc = new GEngineClasses::C_Npc();

  GEngineClasses::C_Npc& npcData = *npc;
  npcData.wp = waypoint;

  // Setup basic class linkage. This is important here, as otherwise the wld_insertnpc-callback won't be
  // able to work properly
  npcData.instanceSymbol = instance;

  PARSymbol& s         = m_VM.getDATFile().getSymbolByIndex(instance);
  s.instanceDataHandle = npc;
  s.instanceDataClass  = IC_Npc;
  m_VM.initializeInstance(npc, instance, IC_Npc);
  return npc;
  }

GEngineClasses::C_Npc* DaedalusGameState::insertNPC(const char* instance, const std::string& waypoint) {
  return insertNPC(m_VM.getDATFile().getSymbolIndexByName(instance), waypoint);
  }

GEngineClasses::C_Item* DaedalusGameState::insertItem(size_t instance) {
  auto h = createItem();
  m_VM.initializeInstance(h, static_cast<size_t>(instance), IC_Item);
  return h;
  }

GEngineClasses::C_Item *DaedalusGameState::insertItem(const char* instance) {
  return insertItem(m_VM.getDATFile().getSymbolIndexByName(instance));
  }

GEngineClasses::C_SFX* DaedalusGameState::insertSFX(size_t instance) {
  auto h = createSfx();
  m_VM.initializeInstance(h, static_cast<size_t>(instance), IC_Sfx);
  return h;
  }

GEngineClasses::C_SFX *DaedalusGameState::insertSFX(const char* instance) {
  return insertSFX(m_VM.getDATFile().getSymbolIndexByName(instance));
  }

GEngineClasses::C_MusicTheme* DaedalusGameState::insertMusicTheme(size_t instance) {
  // Get memory for the item
  auto h = createMusicTheme();
  // Run the script-constructor
  m_VM.initializeInstance(h, static_cast<size_t>(instance), IC_MusicTheme);
  return h;
  }

GEngineClasses::C_MusicTheme *DaedalusGameState::insertMusicTheme(const char* instance) {
  return insertMusicTheme(m_VM.getDATFile().getSymbolIndexByName(instance));
  }
