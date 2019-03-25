//
// Created by andre on 13.05.16.
//

#include "DaedalusStdlib.h"
#include "DATFile.h"
#include "DaedalusVM.h"
#include <utils/logger.h>

#define REGISTER(cname, var)           datFile.registerMember(#cname "." #var, &cname::var, false);
#define REGISTER_IF_EXISTS(cname, var) datFile.registerMember(#cname "." #var, &cname::var, true );

void Daedalus::registerGothicEngineClasses(DaedalusVM& vm) {
  using namespace GEngineClasses;
  auto& datFile = vm.getDATFile();

  // the vm of the GOTHIC.DAT does not contain C_Menu and MENU.DAT's vm does not contain C_NPC
  // so we need to register only class members of existing classes
  auto classExists = [&vm](const char* className) {
    bool exists = vm.getDATFile().hasSymbolName(className);
    return exists && (vm.getDATFile().getSymbolByName(className).properties.elemProps.type==EParType_Class);
    };

  if(classExists("C_Npc")) {
    REGISTER(C_Npc, id);
    REGISTER(C_Npc, name);
    REGISTER(C_Npc, slot);
    REGISTER_IF_EXISTS(C_Npc, effect);
    REGISTER(C_Npc, npcType);
    REGISTER(C_Npc, flags);
    REGISTER(C_Npc, attribute);
    REGISTER_IF_EXISTS(C_Npc, hitChance);
    REGISTER(C_Npc, protection);
    REGISTER(C_Npc, damage);
    REGISTER(C_Npc, damagetype);
    REGISTER(C_Npc, guild);
    REGISTER(C_Npc, level);
    REGISTER(C_Npc, mission);
    REGISTER(C_Npc, fight_tactic);
    REGISTER(C_Npc, weapon);
    REGISTER(C_Npc, voice);
    REGISTER(C_Npc, voicePitch);
    REGISTER(C_Npc, bodymass);
    REGISTER(C_Npc, daily_routine);
    REGISTER(C_Npc, start_aistate);
    REGISTER(C_Npc, spawnPoint);
    REGISTER(C_Npc, spawnDelay);
    REGISTER(C_Npc, senses);
    REGISTER(C_Npc, senses_range);
    REGISTER(C_Npc, aivar);
    REGISTER(C_Npc, wp);
    REGISTER(C_Npc, exp);
    REGISTER(C_Npc, exp_next);
    REGISTER(C_Npc, lp);
    REGISTER_IF_EXISTS(C_Npc, bodyStateInterruptableOverride);
    REGISTER_IF_EXISTS(C_Npc, noFocus);
    }

  if(classExists("C_Focus")) {
    REGISTER(C_Focus, npc_longrange);
    REGISTER(C_Focus, npc_range1);
    REGISTER(C_Focus, npc_range2);
    REGISTER(C_Focus, npc_azi);
    REGISTER(C_Focus, npc_elevdo);
    REGISTER(C_Focus, npc_elevup);
    REGISTER(C_Focus, npc_prio);
    REGISTER(C_Focus, item_range1);
    REGISTER(C_Focus, item_range2);
    REGISTER(C_Focus, item_azi);
    REGISTER(C_Focus, item_elevdo);
    REGISTER(C_Focus, item_elevup);
    REGISTER(C_Focus, item_prio);
    REGISTER(C_Focus, mob_range1);
    REGISTER(C_Focus, mob_range2);
    REGISTER(C_Focus, mob_azi);
    REGISTER(C_Focus, mob_elevdo);
    REGISTER(C_Focus, mob_elevup);
    REGISTER(C_Focus, mob_prio);
    }

  if(classExists("C_Info")) {
    REGISTER(C_Info, npc);
    REGISTER(C_Info, nr);
    REGISTER(C_Info, important);
    REGISTER(C_Info, condition);
    REGISTER(C_Info, information);
    REGISTER(C_Info, description);
    REGISTER(C_Info, trade);
    REGISTER(C_Info, permanent);
    }

  if(classExists("C_ItemReact")) {
    REGISTER(C_ItemReact, npc);
    REGISTER(C_ItemReact, trade_item);
    REGISTER(C_ItemReact, trade_amount);
    REGISTER(C_ItemReact, requested_cat);
    REGISTER(C_ItemReact, requested_item);
    REGISTER(C_ItemReact, requested_amount);
    REGISTER(C_ItemReact, reaction);
    }

  if(classExists("C_Item")) {
    REGISTER(C_Item, id);
    REGISTER(C_Item, name);
    REGISTER(C_Item, nameID);
    REGISTER(C_Item, hp);
    REGISTER(C_Item, hp_max);
    REGISTER(C_Item, mainflag);
    REGISTER(C_Item, flags);
    REGISTER(C_Item, weight);
    REGISTER(C_Item, value);
    REGISTER(C_Item, damageType);
    REGISTER(C_Item, damageTotal);
    REGISTER(C_Item, damage);
    REGISTER(C_Item, wear);
    REGISTER(C_Item, protection);
    REGISTER(C_Item, nutrition);
    REGISTER(C_Item, cond_atr);
    REGISTER(C_Item, cond_value);
    REGISTER(C_Item, change_atr);
    REGISTER(C_Item, change_value);
    REGISTER(C_Item, magic);
    REGISTER(C_Item, on_equip);
    REGISTER(C_Item, on_unequip);
    REGISTER(C_Item, on_state);
    REGISTER(C_Item, owner);
    REGISTER(C_Item, ownerGuild);
    REGISTER(C_Item, disguiseGuild);
    REGISTER(C_Item, visual);
    REGISTER(C_Item, visual_change);
    REGISTER_IF_EXISTS(C_Item, effect);
    REGISTER(C_Item, visual_skin);
    REGISTER(C_Item, scemeName);
    REGISTER(C_Item, material);
    REGISTER(C_Item, munition);
    REGISTER(C_Item, spell);
    REGISTER(C_Item, range);
    REGISTER(C_Item, mag_circle);
    REGISTER(C_Item, description);
    REGISTER(C_Item, text);
    REGISTER(C_Item, count);
    REGISTER_IF_EXISTS(C_Item, inv_zbias);
    REGISTER_IF_EXISTS(C_Item, inv_rotx);
    REGISTER_IF_EXISTS(C_Item, inv_roty);
    REGISTER_IF_EXISTS(C_Item, inv_rotz);
    REGISTER_IF_EXISTS(C_Item, inv_animate);
    }

  if(classExists("C_Spell")) {
    REGISTER_IF_EXISTS(C_Spell, time_per_mana);
    REGISTER_IF_EXISTS(C_Spell, damage_per_level);
    REGISTER_IF_EXISTS(C_Spell, damageType);
    REGISTER_IF_EXISTS(C_Spell, spellType);
    REGISTER_IF_EXISTS(C_Spell, canTurnDuringInvest);
    REGISTER_IF_EXISTS(C_Spell, canChangeTargetDuringInvest);
    REGISTER_IF_EXISTS(C_Spell, isMultiEffect);
    REGISTER_IF_EXISTS(C_Spell, targetCollectAlgo);
    REGISTER_IF_EXISTS(C_Spell, targetCollectType);
    REGISTER_IF_EXISTS(C_Spell, targetCollectRange);
    REGISTER_IF_EXISTS(C_Spell, targetCollectAzi);
    REGISTER_IF_EXISTS(C_Spell, targetCollectElev);
    };

  if(classExists("C_Mission")) {
    REGISTER(C_Mission, name);
    REGISTER(C_Mission, description);
    REGISTER(C_Mission, duration);
    REGISTER(C_Mission, important);
    REGISTER(C_Mission, offerConditions);
    REGISTER(C_Mission, offer);
    REGISTER(C_Mission, successConditions);
    REGISTER(C_Mission, success);
    REGISTER(C_Mission, failureConditions);
    REGISTER(C_Mission, failure);
    REGISTER(C_Mission, obsoleteConditions);
    REGISTER(C_Mission, obsolete);
    REGISTER(C_Mission, running);
    }

  if(classExists("C_Menu")) {
    REGISTER(C_Menu, backPic);
    REGISTER(C_Menu, backWorld);
    REGISTER(C_Menu, posx);
    REGISTER(C_Menu, posy);
    REGISTER(C_Menu, dimx);
    REGISTER(C_Menu, dimy);
    REGISTER(C_Menu, alpha);
    REGISTER(C_Menu, musicTheme);
    REGISTER(C_Menu, eventTimerMSec);
    REGISTER(C_Menu, items);
    REGISTER(C_Menu, flags);
    REGISTER(C_Menu, defaultOutGame);
    REGISTER(C_Menu, defaultInGame);
    }

  if(classExists("C_Menu_Item")) {
    REGISTER(C_Menu_Item, fontName);
    REGISTER(C_Menu_Item, text);
    REGISTER(C_Menu_Item, backPic);
    REGISTER(C_Menu_Item, alphaMode);
    REGISTER(C_Menu_Item, alpha);
    REGISTER(C_Menu_Item, type);
    REGISTER(C_Menu_Item, onSelAction);
    REGISTER(C_Menu_Item, onSelAction_S);
    REGISTER(C_Menu_Item, onChgSetOption);
    REGISTER(C_Menu_Item, onChgSetOptionSection);
    REGISTER(C_Menu_Item, onEventAction);
    REGISTER(C_Menu_Item, posx);
    REGISTER(C_Menu_Item, posy);
    REGISTER(C_Menu_Item, dimx);
    REGISTER(C_Menu_Item, dimy);
    REGISTER(C_Menu_Item, sizeStartScale);
    REGISTER(C_Menu_Item, flags);
    REGISTER(C_Menu_Item, openDelayTime);
    REGISTER(C_Menu_Item, openDuration);
    REGISTER(C_Menu_Item, userfloat);
    REGISTER(C_Menu_Item, userString);
    REGISTER(C_Menu_Item, frameSizeX);
    REGISTER(C_Menu_Item, frameSizeY);
    REGISTER_IF_EXISTS(C_Menu_Item, hideIfOptionSectionSet);
    REGISTER_IF_EXISTS(C_Menu_Item, hideIfOptionSet);
    REGISTER_IF_EXISTS(C_Menu_Item, hideOnValue);
    }

  if(classExists("C_SFX")) {
    REGISTER(C_SFX, file);
    REGISTER(C_SFX, pitchOff);
    REGISTER(C_SFX, pitchVar);
    REGISTER(C_SFX, vol);
    REGISTER(C_SFX, loop);
    REGISTER(C_SFX, loopStartOffset);
    REGISTER(C_SFX, loopEndOffset);
    REGISTER(C_SFX, reverbLevel);
    REGISTER(C_SFX, pfxName);
    }

  if(classExists("C_ParticleFX")) {
    REGISTER(C_ParticleFX, ppsValue);
    REGISTER(C_ParticleFX, ppsScaleKeys_S);
    REGISTER(C_ParticleFX, ppsIsLooping);
    REGISTER(C_ParticleFX, ppsIsSmooth);
    REGISTER(C_ParticleFX, ppsFPS);
    REGISTER(C_ParticleFX, ppsCreateEm_S);
    REGISTER(C_ParticleFX, ppsCreateEmDelay);
    REGISTER(C_ParticleFX, shpType_S);
    REGISTER(C_ParticleFX, shpFOR_S);
    REGISTER(C_ParticleFX, shpOffsetVec_S);
    REGISTER(C_ParticleFX, shpDistribType_S);
    REGISTER(C_ParticleFX, shpDistribWalkSpeed);
    REGISTER(C_ParticleFX, shpIsVolume);
    REGISTER(C_ParticleFX, shpDim_S);
    REGISTER(C_ParticleFX, shpMesh_S);
    REGISTER(C_ParticleFX, shpMeshRender_B);
    REGISTER(C_ParticleFX, shpScaleKeys_S);
    REGISTER(C_ParticleFX, shpScaleIsLooping);
    REGISTER(C_ParticleFX, shpScaleIsSmooth);
    REGISTER(C_ParticleFX, shpScaleFPS);
    REGISTER(C_ParticleFX, dirMode_S);
    REGISTER(C_ParticleFX, dirFOR_S);
    REGISTER(C_ParticleFX, dirModeTargetFOR_S);
    REGISTER(C_ParticleFX, dirModeTargetPos_S);
    REGISTER(C_ParticleFX, dirAngleHead);
    REGISTER(C_ParticleFX, dirAngleHeadVar);
    REGISTER(C_ParticleFX, dirAngleElev);
    REGISTER(C_ParticleFX, dirAngleElevVar);
    REGISTER(C_ParticleFX, velAvg);
    REGISTER(C_ParticleFX, velVar);
    REGISTER(C_ParticleFX, lspPartAvg);
    REGISTER(C_ParticleFX, lspPartVar);
    REGISTER(C_ParticleFX, flyGravity_S);
    REGISTER(C_ParticleFX, flyCollDet_B);
    REGISTER(C_ParticleFX, visName_S);
    REGISTER(C_ParticleFX, visOrientation_S);
    REGISTER(C_ParticleFX, visTexIsQuadPoly);
    REGISTER(C_ParticleFX, visTexAniFPS);
    REGISTER(C_ParticleFX, visTexAniIsLooping);
    REGISTER(C_ParticleFX, visTexColorStart_S);
    REGISTER(C_ParticleFX, visTexColorEnd_S);
    REGISTER(C_ParticleFX, visSizeStart_S);
    REGISTER(C_ParticleFX, visSizeEndScale);
    REGISTER(C_ParticleFX, visAlphaFunc_S);
    REGISTER(C_ParticleFX, visAlphaStart);
    REGISTER(C_ParticleFX, visAlphaEnd);
    REGISTER(C_ParticleFX, trlFadeSpeed);
    REGISTER(C_ParticleFX, trlTexture_S);
    REGISTER(C_ParticleFX, trlWidth);
    REGISTER(C_ParticleFX, mrkFadeSpeed);
    REGISTER(C_ParticleFX, mrkTexture_S);
    REGISTER(C_ParticleFX, mrkSize);
    REGISTER_IF_EXISTS(C_ParticleFX, flockMode);
    REGISTER_IF_EXISTS(C_ParticleFX, flockStrength);
    REGISTER_IF_EXISTS(C_ParticleFX, useEmittersFOR);
    REGISTER_IF_EXISTS(C_ParticleFX, timeStartEnd_S);
    REGISTER_IF_EXISTS(C_ParticleFX, m_bIsAmbientPFX);
    }

  if(classExists("C_MusicTheme")) {
    REGISTER(C_MusicTheme, file);
    REGISTER(C_MusicTheme, vol);
    REGISTER(C_MusicTheme, loop);
    REGISTER(C_MusicTheme, reverbMix);
    REGISTER(C_MusicTheme, reverbTime);
    REGISTER(C_MusicTheme, transType);
    REGISTER(C_MusicTheme, transSubType);
    }

  if(classExists("C_GilValues")) {
    REGISTER(C_GilValues, water_depth_knee);
    REGISTER(C_GilValues, water_depth_chest);
    REGISTER(C_GilValues, jumpup_height);
    REGISTER(C_GilValues, swim_time);
    REGISTER(C_GilValues, dive_time);
    REGISTER(C_GilValues, step_height);
    REGISTER(C_GilValues, jumplow_height);
    REGISTER(C_GilValues, jumpmid_height);
    REGISTER(C_GilValues, slide_angle);
    REGISTER(C_GilValues, slide_angle2);
    REGISTER(C_GilValues, disable_autoroll);
    REGISTER(C_GilValues, surface_align);
    REGISTER(C_GilValues, climb_heading_angle);
    REGISTER(C_GilValues, climb_horiz_angle);
    REGISTER(C_GilValues, climb_ground_angle);
    REGISTER(C_GilValues, fight_range_base);
    REGISTER(C_GilValues, fight_range_fist);
    REGISTER(C_GilValues, fight_range_g);
    REGISTER(C_GilValues, fight_range_1hs);
    REGISTER(C_GilValues, fight_range_1ha);
    REGISTER(C_GilValues, fight_range_2hs);
    REGISTER(C_GilValues, fight_range_2ha);
    REGISTER(C_GilValues, falldown_height);
    REGISTER(C_GilValues, falldown_damage);
    REGISTER(C_GilValues, blood_disabled);
    REGISTER(C_GilValues, blood_max_distance);
    REGISTER(C_GilValues, blood_amount);
    REGISTER(C_GilValues, blood_flow);
    REGISTER(C_GilValues, blood_emitter);
    REGISTER(C_GilValues, blood_texture);
    REGISTER(C_GilValues, turn_speed);
    }

  if(classExists("C_FightAI")) {
    REGISTER(C_FightAI, move);
    }
  }
