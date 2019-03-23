#pragma once
#include <list>
#include <vector>
#include "DATFile.h"
#include "DaedalusStdlib.h"
#include <utils/staticReferencedAllocator.h>

namespace Daedalus {
    class DaedalusVM;

    namespace GameState {
        struct LogTopic
        {
            enum ELogStatus
            {
                // TODO: Find the actual values for this
                LS_Running = 1,
                LS_Success = 2,
                LS_Failed = 3,
                LS_Obsolete = 4
            };

            enum ESection
            {
                // TODO: Find the actual values for this!
                LT_Mission = 0,
                LT_Note = 1
            };

            std::vector<std::string> entries;

            ELogStatus status;
            ESection section;
        };

        class DaedalusGameState {
          public:
            DaedalusGameState(Daedalus::DaedalusVM& vm);

            struct GameExternals {
              // These will all be executed with the content already created
              std::function<void(GEngineClasses::C_Npc*, std::string)> wld_insertnpc;
              };

            void setGameExternals(const GameExternals& ext) {
              m_GameExternals = ext;
              }

            /**
             * Removes the data behind the given item handle
             * @param item Item to remove
             */
            template <typename C_Class>
            void remove        (C_Class* h) { delete h; }
            void removeItem    (GEngineClasses::C_Item* item);
            void removeNPC     (GEngineClasses::C_Npc*  npc);

            /**
             * Creates a new NPC-Instance, just as Wld_InsertNPC was called from script
             * @param instance Instance-Index of the NPC to create
             * @param waypoint Waypoint to place the newly created npc on
             * @return Handle to the script-instance created
             */
            GEngineClasses::C_Npc* insertNPC(size_t instance, const std::string& waypoint);
            GEngineClasses::C_Npc* insertNPC(const char *instance, const std::string& waypoint);

            /**
             * Creates a new item instance and initializes it (does not position it)
             * @param instance Instance to create
             * @return Handle to the created instance
             */
            GEngineClasses::C_Item*       insertItem(size_t instance);
            GEngineClasses::C_Item*       insertItem(const char *instance);

            GEngineClasses::C_SFX*        insertSFX(size_t instance);
            GEngineClasses::C_SFX*        insertSFX(const char *instance);

            GEngineClasses::C_MusicTheme* insertMusicTheme(size_t instance);
            GEngineClasses::C_MusicTheme* insertMusicTheme(const char *instance);

            /**
             * Creates scripting relevant objects
             */
            template <typename C_Class>
            C_Class*                      create();
            GEngineClasses::C_Npc*        createNPC();
            GEngineClasses::C_Item*       createItem();
            GEngineClasses::C_ItemReact*  createItemReact();
            GEngineClasses::C_Mission*    createMission();
            GEngineClasses::C_Info*       createInfo();
            GEngineClasses::C_Focus*      createFocus();
            GEngineClasses::C_Menu*       createMenu();
            GEngineClasses::C_Menu_Item*  createMenuItem();
            GEngineClasses::C_SFX*        createSfx();
            GEngineClasses::C_ParticleFX* createPfx();
            GEngineClasses::C_MusicTheme* createMusicTheme();
            GEngineClasses::C_GilValues*  createGilValues();

            Daedalus::GEngineClasses::Instance* getByClass(void *h, EInstanceClass instClass);

        private:
            Daedalus::DaedalusVM& m_VM;
            GameExternals         m_GameExternals;
        };
    }  // namespace GameState
}  // namespace Daedalus
