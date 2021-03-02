#include "zCVob.h"

#include "zenParserPropRead.h"
#include "parserImpl.h"

using namespace ZenLoad;

#pragma pack(push, 1)
struct packedVobData {
  ZMath::float3 bbox3DWS[2];
  ZMath::float3 positionWS;
  zMAT3 trafoRotWS;

  struct packedBitField {
    uint8_t showVisual : 1;
    uint8_t visualCamAlign : 2;
    uint8_t cdStatic : 1;
    uint8_t cdDyn : 1;
    uint8_t staticVob : 1;
    uint8_t dynShadow : 2;
    uint8_t hasPresetName : 1;
    uint8_t hasVobName : 1;
    uint8_t hasVisualName : 1;
    uint8_t hasRelevantVisualObject : 1;
    uint8_t hasAIObject : 1;
    uint8_t hasEventManObject : 1;
    uint8_t physicsEnabled : 1;
    uint8_t visualAniMode : 2;
    uint8_t zbias : 5;
    uint8_t bAmbient : 1;
    } bitfield;

  float visualAniStrength;
  float vobFarClipZ;
  };
#pragma pack(pop)

static void read_zCDecal(zCVobData &info, ZenParser &parser, WorldVersion version) {
  auto& d  = info.visualChunk.zCDecal;
  auto& rd = *parser.getImpl();
  rd.readEntry("", &d.name);
  rd.readEntry("", &d.decalDim, sizeof(d.decalDim), ParserImpl::ZVT_RAW_FLOAT);
  rd.readEntry("", &d.decalOffset, sizeof(d.decalOffset), ParserImpl::ZVT_RAW_FLOAT);
  rd.readEntry("", &d.decal2Sided);
  rd.readEntry("", &d.decalAlphaFunc);
  rd.readEntry("", &d.decalTexAniFPS);
  if(version != WorldVersion::VERSION_G1_08k) {
    rd.readEntry("", &d.decalAlphaWeight);
    rd.readEntry("", &d.ignoreDayLight);
    }
  }

static void read_Visual(zCVobData &info, ZenParser &parser,
                        WorldVersion version, const ZenParser::ChunkHeader &header) {
  if(header.classname == "zCDecal")
    return read_zCDecal(info,parser,version);
  }

static void read_zCVob(zCVobData &info, ZenParser &parser, WorldVersion version) {
  info.vobType        = zCVobData::VT_zCVob;
  info.rotationMatrix = ZMath::Matrix::CreateIdentity();

  // Read how many vobs this one has as child
  parser.getImpl()->readEntry("", &info.pack, sizeof(info.pack), ZenLoad::ParserImpl::ZVT_INT);

  if(info.pack) {
    packedVobData pd;
    parser.getImpl()->readEntry("", &pd, sizeof(pd), ZenLoad::ParserImpl::ZVT_RAW);

    info.bbox[0] = pd.bbox3DWS[0];
    info.bbox[1] = pd.bbox3DWS[1];
    info.position = pd.positionWS;
    info.rotationMatrix3x3 = pd.trafoRotWS;
    info.rotationMatrix = info.rotationMatrix3x3.toMatrix();
    info.showVisual = pd.bitfield.showVisual;
    info.visualCamAlign = pd.bitfield.visualCamAlign;
    info.cdStatic = pd.bitfield.cdStatic;
    info.cdDyn = pd.bitfield.cdDyn;
    info.staticVob = pd.bitfield.staticVob;
    info.dynamicShadow = pd.bitfield.dynShadow;
    info.physicsEnabled = pd.bitfield.physicsEnabled;
    info.visualAniMode = pd.bitfield.visualAniMode;
    info.zBias = pd.bitfield.zbias;
    info.isAmbient = pd.bitfield.bAmbient;
    info.visualAniModeStrength = pd.visualAniStrength;
    info.vobFarClipScale = pd.vobFarClipZ;

    if(pd.bitfield.hasPresetName)
      parser.getImpl()->readEntry("", &info.presetName, 0, ZenLoad::ParserImpl::ZVT_STRING);

    if(pd.bitfield.hasVobName)
      parser.getImpl()->readEntry("", &info.vobName, 0, ZenLoad::ParserImpl::ZVT_STRING);

    if(pd.bitfield.hasVisualName)
      parser.getImpl()->readEntry("", &info.visual, 0, ZenLoad::ParserImpl::ZVT_STRING);

    if(pd.bitfield.hasRelevantVisualObject) {
      ZenParser::ChunkHeader hdr;
      parser.readChunkStart(hdr);
      read_Visual(info,parser,version,hdr);
      if(!parser.readChunkEnd())
        parser.skipChunk();
      }

    if(pd.bitfield.hasAIObject) {
      // Skip ai-chunk
      ZenParser::ChunkHeader tmph;
      parser.readChunkStart(tmph);
      parser.skipChunk();
      }
    } else {
    ReadObjectProperties(parser, info.properties, Prop("PresetName", info.presetName));

    parser.getImpl()->readEntry("", info.bbox, sizeof(info.bbox), ZenLoad::ParserImpl::ZVT_RAW_FLOAT);
    parser.getImpl()->readEntry("", &info.rotationMatrix3x3, sizeof(info.rotationMatrix3x3), ZenLoad::ParserImpl::ZVT_RAW);

    parser.getImpl()->readEntry("", &info.position, sizeof(float) * 3, ZenLoad::ParserImpl::ZVT_VEC3);

    info.rotationMatrix = info.rotationMatrix3x3.toMatrix();

    if(version != WorldVersion::VERSION_G1_08k) {
      ReadObjectProperties(parser, info.properties,
                           Prop("vobName", info.vobName),
                           Prop("visual", info.visual),
                           Prop("showVisual", info.showVisual),
                           Prop("visualCamAlign", info.visualCamAlign),
                           Prop("visualAniMode", info.visualAniMode),
                           Prop("visualAniModeStrength", info.visualAniModeStrength),
                           Prop("vobFarClipZScale", info.vobFarClipScale),
                           Prop("cdStatic", info.cdStatic),
                           Prop("cdDyn", info.cdDyn),
                           Prop("staticVob", info.staticVob),
                           Prop("dynShadow", info.dynamicShadow),
                           Prop("zBias", info.zBias),
                           Prop("isAmbient", info.isAmbient));  // TODO: References!
      } else {
      ReadObjectProperties(parser, info.properties,
                           Prop("vobName", info.vobName),
                           Prop("visual", info.visual),
                           Prop("showVisual", info.showVisual),
                           Prop("visualCamAlign", info.visualCamAlign),
                           Prop("cdStatic", info.cdStatic),
                           Prop("cdDyn", info.cdDyn),
                           Prop("staticVob", info.staticVob),
                           Prop("dynShadow", info.dynamicShadow),
                           Prop("visualAniMode", info.visualAniMode));  // TODO: References!
      }

    // Visual-chunk
    {
    ZenParser::ChunkHeader visChunk;
    parser.readChunkStart(visChunk);
    ReadObjectProperties(parser, info.properties,
                         Prop("name",             info.visualChunk.zCDecal.name),
                         Prop("decalDim",         info.visualChunk.zCDecal.decalDim),
                         Prop("decalOffset",      info.visualChunk.zCDecal.decalOffset),
                         Prop("decal2Sided",      info.visualChunk.zCDecal.decal2Sided),
                         Prop("decalAlphaFunc",   info.visualChunk.zCDecal.decalAlphaFunc),
                         Prop("decalTexAniFPS",   info.visualChunk.zCDecal.decalTexAniFPS),
                         Prop("decalAlphaWeight", info.visualChunk.zCDecal.decalAlphaWeight),
                         Prop("ignoreDayLight",   info.visualChunk.zCDecal.ignoreDayLight));
    ReadObjectProperties(parser,{
                           prop("name",             info.visualChunk.zCDecal.name),
                           prop("decalDim",         info.visualChunk.zCDecal.decalDim),
                           prop("decalOffset",      info.visualChunk.zCDecal.decalOffset),
                           prop("decal2Sided",      info.visualChunk.zCDecal.decal2Sided),
                           prop("decalAlphaFunc",   info.visualChunk.zCDecal.decalAlphaFunc),
                           prop("decalTexAniFPS",   info.visualChunk.zCDecal.decalTexAniFPS),
                           prop("decalAlphaWeight", info.visualChunk.zCDecal.decalAlphaWeight),
                           prop("ignoreDayLight",   info.visualChunk.zCDecal.ignoreDayLight),
                         });
    parser.skipChunk();
    }

    {
    // Skip ai-chunk
    ZenParser::ChunkHeader tmph;
    parser.readChunkStart(tmph);
    parser.skipChunk();
    }
    }

  // Generate world-matrix
  info.worldMatrix = info.rotationMatrix3x3.toMatrix(info.position);
  }

static void read_zCVobLevelCompo(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobLevelCompo;
  }

static void read_zCVob_oCItem(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCItem;
  rd.readEntry("itemInstance", &info.oCItem.instanceName);
  }

static void read_zCVob_zCVobSpot(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobSpot;
  }

static void read_zCVob_zCVobStartpoint(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobStartpoint;
  }

static void read_zCMessageFilter(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCMessageFilter;
  rd.readEntry("triggerTarget", &info.zCMessageFilter.triggerTarget);
  rd.readEntry("onTrigger",     reinterpret_cast<uint8_t*>(&info.zCMessageFilter.onTrigger));
  rd.readEntry("onUntrigger",   reinterpret_cast<uint8_t*>(&info.zCMessageFilter.onUntrigger));
  }

static void read_zCCodeMaster(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  uint8_t     count=0;

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCCodeMaster;
  rd.readEntry("triggerTarget",        &info.zCCodeMaster.triggerTarget);
  rd.readEntry("orderRelevant",        &info.zCCodeMaster.orderRelevant);
  rd.readEntry("firstFalseIsFailure",  &info.zCCodeMaster.firstFalseIsFailure);
  rd.readEntry("triggerTargetFailure", &info.zCCodeMaster.triggerTargetFailure);
  rd.readEntry("untriggeredCancels",   &info.zCCodeMaster.untriggeredCancels);
  rd.readEntry("", &count);
  info.zCCodeMaster.slaveVobName.resize(count);
  for(auto& i:info.zCCodeMaster.slaveVobName)
    rd.readEntry("slaveVobName", &i);
  }

static void read_zCVob_zCTrigger(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTrigger;
  rd.readEntry("triggerTarget",     &info.zCTrigger.triggerTarget);
  rd.readEntry("flags",             &info.zCTrigger.flags, ZenLoad::ParserImpl::ZVT_RAW);
  rd.readEntry("filterFlags",       &info.zCTrigger.filterFlags, ZenLoad::ParserImpl::ZVT_RAW);
  rd.readEntry("respondToVobName",  &info.zCTrigger.respondToVobName);
  rd.readEntry("numCanBeActivated", &info.zCTrigger.numCanBeActivated);
  rd.readEntry("retriggerWaitSec",  &info.zCTrigger.retriggerWaitSec);
  rd.readEntry("damageThreshold",   &info.zCTrigger.damageThreshold);
  rd.readEntry("fireDelaySec",      &info.zCTrigger.fireDelaySec);
  }

static void read_zCVob_zCTrigger_zCTriggerList(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  uint8_t count=0;
  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTriggerList;

  rd.readEntry("listProcess", &info.zCTriggerList.listProcess);
  rd.readEntry("", &count);
  info.zCTriggerList.list.resize(count);
  for (uint8_t i = 0; i < count; i++) {
    char triggerTarget[64]={};
    char fireDelay    [64]={};
    if(version != WorldVersion::VERSION_G1_08k) {
      std::snprintf(triggerTarget,sizeof(triggerTarget),"slaveVobName%d",int(i));
      } else {
      std::snprintf(triggerTarget,sizeof(triggerTarget),"triggerTarget%d",int(i));
      }
    rd.readEntry(triggerTarget, &info.zCTriggerList.list[i].triggerTarget);
    rd.readEntry(fireDelay,     &info.zCTriggerList.list[i].fireDelay);
    }
  }

static void read_zCVob_zCTrigger_oCTriggerScript(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTriggerScript;
  rd.readEntry("scriptFunc", &info.zCTriggerScript.scriptFunc);
  }

static void read_zCVob_oCMOB(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMOB;
  rd.readEntry("focusName", &info.oCMOB.focusName);
  rd.readEntry("hitpoints", &info.oCMOB.hitpoints);
  rd.readEntry("damage", &info.oCMOB.damage);
  rd.readEntry("moveable", &info.oCMOB.moveable);
  rd.readEntry("takeable", &info.oCMOB.takeable);
  rd.readEntry("focusOverride", &info.oCMOB.focusOverride);
  rd.readEntry("soundMaterial", &info.oCMOB.soundMaterial);
  rd.readEntry("visualDestroyed", &info.oCMOB.visualDestroyed);
  rd.readEntry("owner", &info.oCMOB.owner);
  rd.readEntry("ownerGuild", &info.oCMOB.ownerGuild);
  rd.readEntry("isDestroyed", &info.oCMOB.isDestroyed);
  }

static void read_zCVob_oCMOB_oCMobInter(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobInter;
  rd.readEntry("stateNum", &info.oCMobInter.stateNum);
  rd.readEntry("triggerTarget", &info.oCMobInter.triggerTarget);
  rd.readEntry("useWithItem", &info.oCMobInter.useWithItem);
  rd.readEntry("conditionFunc", &info.oCMobInter.conditionFunc);
  rd.readEntry("onStateFunc", &info.oCMobInter.onStateFunc);
  rd.readEntry("rewind", &info.oCMobInter.rewind);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobBed(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobBed;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobDoor(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobDoor;
  rd.readEntry("locked", &info.oCMobLockable.locked);
  rd.readEntry("keyInstance", &info.oCMobLockable.keyInstance);
  rd.readEntry("pickLockStr", &info.oCMobLockable.pickLockStr);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobFire(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobFire;
  rd.readEntry("fireSlot",        &info.oCMobFire.fireSlot);
  rd.readEntry("fireVobtreeName", &info.oCMobFire.fireVobtreeName);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobLadder(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobLadder;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobSwitch(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobSwitch;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobContainer(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobContainer;
  rd.readEntry("locked", &info.oCMobLockable.locked);
  rd.readEntry("keyInstance", &info.oCMobLockable.keyInstance);
  rd.readEntry("pickLockStr", &info.oCMobLockable.pickLockStr);
  rd.readEntry("contains", &info.oCMobContainer.contains);
  }

static void read_zCVob_zCVobLight(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto&  rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCVobLight;
  rd.readEntry("lightPresetInUse", &info.zCVobLight.lightPresetInUse);
  rd.readEntry("lightType", &info.zCVobLight.lightType);
  rd.readEntry("range", &info.zCVobLight.range);
  rd.readEntry("color", &info.zCVobLight.color);
  rd.readEntry("spotConeAngle", &info.zCVobLight.spotConeAngle);
  rd.readEntry("lightStatic", &info.zCVobLight.lightStatic);
  rd.readEntry("lightQuality", &info.zCVobLight.lightQuality);
  rd.readEntry("lensflareFX", &info.zCVobLight.lensflareFX);

  if(info.zCVobLight.lightStatic)
    return;

  std::string rangeAniScale, colorAniList;
  rd.readEntry("turnedOn",&info.zCVobLight.dynamic.turnedOn);
  rd.readEntry("rangeAniScale",&rangeAniScale);
  rd.readEntry("rangeAniFPS", &info.zCVobLight.dynamic.rangeAniFPS);
  rd.readEntry("rangeAniSmooth", &info.zCVobLight.dynamic.rangeAniSmooth);
  rd.readEntry("colorAniList", &colorAniList);
  rd.readEntry("colorAniFPS", &info.zCVobLight.dynamic.colorAniListFPS);
  rd.readEntry("colorAniSmooth", &info.zCVobLight.dynamic.colorAniSmooth);
  if(version!=WorldVersion::VERSION_G1_08k)
    rd.readEntry("canMove", &info.zCVobLight.dynamic.canMove);

  const char* str = nullptr;
  str  = colorAniList.c_str();
  while(str!=nullptr && *str!='\0') {
    while(*str!='\0') {
      if(*str=='(') {
        ++str;
        break;
        }
      ++str;
      }
    if(*str=='\0')
      break;

    uint32_t color = 0;
    for(int i=0; i<3; ++i) {
      char* next=nullptr;
      long v = std::strtol(str,&next,10);
      if(str==next) {
        break;
        }
      str = next;
      color |= (v << (8*i));
      }

    info.zCVobLight.dynamic.colorAniList.push_back(color);
    while(*str!='\0') {
      if(*str==')') {
        ++str;
        break;
        }
      ++str;
      }
    }

  str = rangeAniScale.c_str();
  while(true) {
    char* next=nullptr;
    float f = std::strtof(str,&next);
    if(str==next)
      break;
    info.zCVobLight.dynamic.rangeAniScale.push_back(f);
    str = next;
    }
  }

static void read_zCVob_zCVobSound(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCVobSound;
  rd.readEntry("sndVolume", &info.zCVobSound.sndVolume);
  rd.readEntry("sndMode", (uint32_t*)&info.zCVobSound.sndType);
  rd.readEntry("sndRandDelay", &info.zCVobSound.sndRandDelay);
  rd.readEntry("sndRandDelayVar", &info.zCVobSound.sndRandDelayVar);
  rd.readEntry("sndStartOn", &info.zCVobSound.sndStartOn);
  rd.readEntry("sndAmbient3D", &info.zCVobSound.sndAmbient3D);
  rd.readEntry("sndObstruction", &info.zCVobSound.sndObstruction);
  rd.readEntry("sndConeAngle", &info.zCVobSound.sndConeAngle);
  rd.readEntry("sndVolType", (uint32_t*)&info.zCVobSound.sndVolType);
  rd.readEntry("sndRadius", &info.zCVobSound.sndRadius);
  rd.readEntry("sndName", &info.zCVobSound.sndName);
  }

static void read_zCVob_zCVobSound_zCVobSoundDaytime(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_zCVobSound(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCVobSoundDaytime;
  rd.readEntry("sndStartTime", &info.zCVobSoundDaytime.sndStartTime);
  rd.readEntry("sndEndTime",   &info.zCVobSoundDaytime.sndEndTime);
  rd.readEntry("sndName2",     &info.zCVobSoundDaytime.sndName2);
  }

static void read_zCVob_oCZoneMusic(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCZoneMusic;
  rd.readEntry("enabled",     &info.oCZoneMusic.enabled);
  rd.readEntry("priority",    &info.oCZoneMusic.priority);
  rd.readEntry("ellipsoid",   &info.oCZoneMusic.ellipsoid);
  rd.readEntry("reverbLevel", &info.oCZoneMusic.reverbLevel);
  rd.readEntry("volumeLevel", &info.oCZoneMusic.volumeLevel);
  rd.readEntry("loop",        &info.oCZoneMusic.loop);
  }

static void read_zCVob_oCZoneMusic_oCZoneMusicDefault(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_oCZoneMusic(info,parser,version);
  info.vobType = zCVobData::VT_oCZoneMusicDefault;
  }

static void read_zCVob_zCTrigger_zCMover(zCVobData &info, ZenParser &parser, WorldVersion version){
  read_zCVob_zCTrigger(info,parser,version);
  uint32_t numKeyframes=0;

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCMover;
  rd.readEntry("moverBehavior",     &reinterpret_cast<uint8_t&>(info.zCMover.moverBehavior));
  rd.readEntry("touchBlockerDamage",&info.zCMover.touchBlockerDamage);
  rd.readEntry("stayOpenTimeSec",   &info.zCMover.stayOpenTimeSec);
  rd.readEntry("moverLocked",       &info.zCMover.moverLocked);
  rd.readEntry("autoLinkEnabled",    &info.zCMover.autoLinkEnable);
  if(version!=WorldVersion::VERSION_G1_08k)
    rd.readEntry("autoRotate",      &info.zCMover.autoRotate);
  rd.readEntry("numKeyframes",      &numKeyframes,sizeof(numKeyframes),ZenLoad::ParserImpl::ZVT_WORD);
  if(numKeyframes>0) {
    rd.readEntry("moveSpeed",     &info.zCMover.moveSpeed);
    rd.readEntry("posLerpType",   &info.zCMover.posLerpType);
    rd.readEntry("speedType",     &info.zCMover.speedType);
    std::vector<zCModelAniSample> fr(numKeyframes);
    rd.readEntry("keyframes", &fr[0],sizeof(zCModelAniSample)*numKeyframes,ZenLoad::ParserImpl::ZVT_RAW);
    info.zCMover.keyframes = std::move(fr);
    }
  rd.readEntry("sfxOpenStart",  &info.zCMover.sfxOpenStart);
  rd.readEntry("sfxOpenEnd",    &info.zCMover.sfxOpenEnd);
  rd.readEntry("sfxMoving",     &info.zCMover.sfxMoving);
  rd.readEntry("sfxCloseStart", &info.zCMover.sfxCloseStart);
  rd.readEntry("sfxCloseEnd",   &info.zCMover.sfxCloseEnd);
  rd.readEntry("sfxLock",       &info.zCMover.sfxLock);
  rd.readEntry("sfxUnlock",     &info.zCMover.sfxUnlock);
  rd.readEntry("sfxUseLocked",  &info.zCMover.sfxUseLocked);
  }

static void read_zCVob_zCTrigger_oCTriggerChangeLevel(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTriggerChangeLevel;
  rd.readEntry("levelName",    &info.oCTriggerChangeLevel.levelName);
  rd.readEntry("startVobName", &info.oCTriggerChangeLevel.startVobName);
  }

static void read_zCVob_zCTrigger_zCTriggerWorldStart(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTriggerWorldStart;
  rd.readEntry("triggerTarget",     &info.oCTriggerWorldStart.triggerTarget);
  rd.readEntry("fireOnlyFirstTime", &info.oCTriggerWorldStart.fireOnlyFirstTime);
  }

static void read_zCVob_zCPFXControler(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCPFXControler;
  rd.readEntry("pfxName",         &info.zCPFXControler.pfxName);
  rd.readEntry("killVobWhenDone", &info.zCPFXControler.killVobWhenDone);
  rd.readEntry("pfxStartOn",      &info.zCPFXControler.pfxStartOn);
  }

static void read_zCVob_oCTouchDamage(zCVobData &info, ZenParser &parser, WorldVersion version) {
  read_zCVob(info,parser,version);
  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTouchDamage;
  rd.readEntry("damage",  &info.oCTouchDamage.damage);

  rd.readEntry("Barrier", &info.oCTouchDamage.touchDamage.barrier);
  rd.readEntry("Blunt",   &info.oCTouchDamage.touchDamage.blunt);
  rd.readEntry("Edge",    &info.oCTouchDamage.touchDamage.edge);
  rd.readEntry("Fire",    &info.oCTouchDamage.touchDamage.fire);
  rd.readEntry("Fly",     &info.oCTouchDamage.touchDamage.fly);
  rd.readEntry("Magic",   &info.oCTouchDamage.touchDamage.magic);
  rd.readEntry("Point",   &info.oCTouchDamage.touchDamage.point);
  rd.readEntry("Fall",    &info.oCTouchDamage.touchDamage.fall);

  rd.readEntry("damageRepeatDelaySec", &info.oCTouchDamage.damageRepeatDelaySec);
  rd.readEntry("damageVolDownScale",   &info.oCTouchDamage.damageVolDownScale);
  rd.readEntry("damageCollType",       &info.oCTouchDamage.damageCollType);
  }

static void readObjectData(zCVobData &info, ZenParser &parser,
                           WorldVersion version, const ZenParser::ChunkHeader &header) {
  if(header.classname == "zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "zCVobLevelCompo:zCVob")
    return read_zCVobLevelCompo(info,parser,version);
  if(header.classname == "oCItem:zCVob")
    return read_zCVob_oCItem(info,parser,version);
  if(header.classname == "zCTrigger:zCVob")
    return read_zCVob_zCTrigger(info,parser,version);
  if(header.classname == "oCMOB:zCVob")
    return read_zCVob_oCMOB(info,parser,version);
  if(header.classname == "oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter(info,parser,version);
  if(header.classname == "oCMobBed:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobBed(info,parser,version);
  if(header.classname == "oCMobFire:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobFire(info,parser,version);
  if(header.classname == "oCMobLadder:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobLadder(info,parser,version);
  if(header.classname == "oCMobSwitch:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobSwitch(info,parser,version);
  if(header.classname == "oCMobWheel:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter(info,parser,version);
  if(header.classname == "oCMobContainer:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobContainer(info,parser,version);
  if(header.classname == "oCMobDoor:oCMobInter:oCMOB:zCVob")
    return read_zCVob_oCMOB_oCMobInter_oCMobDoor(info,parser,version);

  if(header.classname == "zCPFXControler:zCVob")
    return read_zCVob_zCPFXControler(info,parser,version);
  if(header.classname == "zCVobAnimate:zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "zCVobLensFlare:zCVob")
    return read_zCVob(info,parser,version);

  if(header.classname == "zCVobLight:zCVob")
    return read_zCVob_zCVobLight(info,parser,version);

  if(header.classname == "zCVobSpot:zCVob")
    return read_zCVob_zCVobSpot(info,parser,version);
  if(header.classname == "zCVobStartpoint:zCVob")
    return read_zCVob_zCVobStartpoint(info,parser,version);

  if(header.classname == "zCVobSound:zCVob")
    return read_zCVob_zCVobSound(info,parser,version);
  if(header.classname == "zCVobSoundDaytime:zCVobSound:zCVob")
    return read_zCVob_zCVobSound_zCVobSoundDaytime(info,parser,version);

  if(header.classname == "oCZoneMusic:zCVob")
    return read_zCVob_oCZoneMusic(info,parser,version);
  if(header.classname == "oCZoneMusicDefault:oCZoneMusic:zCVob")
    return read_zCVob_oCZoneMusic_oCZoneMusicDefault(info,parser,version);

  if(header.classname == "zCZoneZFog:zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "zCZoneZFogDefault:zCZoneZFog:zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "zCZoneVobFarPlane:zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "zCZoneVobFarPlaneDefault:zCZoneVobFarPlane:zCVob")
    return read_zCVob(info,parser,version);

  if(header.classname == "zCMessageFilter:zCVob")
    return read_zCMessageFilter(info,parser,version);
  if(header.classname == "zCCodeMaster:zCVob")
    return read_zCCodeMaster(info,parser,version);
  if(header.classname == "zCTriggerList:zCTrigger:zCVob")
    return read_zCVob_zCTrigger_zCTriggerList(info,parser,version);
  if(header.classname == "oCTriggerScript:zCTrigger:zCVob")
    return read_zCVob_zCTrigger_oCTriggerScript(info,parser,version);
  if(header.classname == "zCMover:zCTrigger:zCVob")
    return read_zCVob_zCTrigger_zCMover(info,parser,version);
  if(header.classname == "oCTriggerChangeLevel:zCTrigger:zCVob")
    return read_zCVob_zCTrigger_oCTriggerChangeLevel(info,parser,version);
  if(header.classname == "zCTriggerWorldStart:zCVob")
    return read_zCVob_zCTrigger_zCTriggerWorldStart(info,parser,version);
  if(header.classname == "zCCSCamera:zCVob")
    return read_zCVob(info,parser,version);
  if(header.classname == "oCTouchDamage:zCTouchDamage:zCVob")
    return read_zCVob_oCTouchDamage(info,parser,version);
  if(header.classname == "zCEarthquake:zCVob")
    return read_zCVob(info,parser,version);
  //LogInfo() << "skip: \"" << header.classname << "\"";
  }

void zCVob::readObjectData(zCVobData &info, ZenParser &parser,
                           WorldVersion version, const ZenParser::ChunkHeader &header) {
  ::readObjectData(info,parser,version,header);
  if(!parser.readChunkEnd())
    parser.skipChunk();
  }
