#include "zCVob.h"

#include "zenParserPropRead.h"
#include "parserImpl.h"

using namespace ZenLoad;

struct packedBitField {
  uint8_t showVisual              : 1;
  uint8_t visualCamAlign          : 2;
  uint8_t cdStatic                : 1;
  uint8_t cdDyn                   : 1;
  uint8_t staticVob               : 1;
  uint8_t dynShadow               : 2;

  uint8_t hasPresetName           : 1;
  uint8_t hasVobName              : 1;
  uint8_t hasVisualName           : 1;
  uint8_t hasRelevantVisualObject : 1;
  uint8_t hasAIObject             : 1;
  uint8_t hasEventManObject       : 1;
  uint8_t physicsEnabled          : 1;

  uint8_t visualAniMode           : 2;
  uint8_t zbias                   : 5;
  uint8_t bAmbient                : 1;
  };

#pragma pack(push, 1)
struct packedVobData {
  ZMath::float3  bbox3DWS[2];
  ZMath::float3  positionWS;
  zMAT3          trafoRotWS;

  uint8_t        bits[3];

  float          visualAniStrength;
  float          vobFarClipZ;
  };
#pragma pack(pop)

static void read_zCDecal(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  auto& d  = info.visualChunk.zCDecal;
  auto& rd = *parser.getImpl();
  rd.readEntry("name",           d.name);
  rd.readEntry("decalDim",       d.decalDim);
  rd.readEntry("decalOffset",    d.decalOffset);
  rd.readEntry("decal2Sided",    d.decal2Sided);
  rd.readEntry("decalAlphaFunc", d.decalAlphaFunc);
  rd.readEntry("decalTexAniFPS", d.decalTexAniFPS);
  if(version==ZenParser::FileVersion::Gothic2) {
    rd.readEntry("decalAlphaWeight", d.decalAlphaWeight);
    rd.readEntry("ignoreDayLight",   d.ignoreDayLight);
    }
  }

static void read_Visual(zCVobData &info, ZenParser &parser,
                        const ZenParser::ChunkHeader &header, ZenParser::FileVersion version) {
  if(header.classId==ZenParser::zCProgMeshProto)
    return;
  if(header.classId==ZenParser::zCDecal)
    return read_zCDecal(info,parser,version);
  }

static void read_zCVob(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  info.vobType        = zCVobData::VT_zCVob;
  info.rotationMatrix = ZMath::Matrix::CreateIdentity();

  // Read how many vobs this one has as child
  parser.getImpl()->readEntry("", info.pack);
  bool hasRelevantVisualObject = true;
  bool hasAIObject             = true;

  if(info.pack) {
    packedVobData  pd = {};
    packedBitField bitfield = {};
    parser.getImpl()->readEntry("", &pd, sizeof(pd));
    std::memcpy(&bitfield,&pd.bits[0],3);

    info.bbox[0]               = pd.bbox3DWS[0];
    info.bbox[1]               = pd.bbox3DWS[1];
    info.position              = pd.positionWS;
    info.rotationMatrix3x3     = pd.trafoRotWS;
    info.rotationMatrix        = info.rotationMatrix3x3.toMatrix();
    info.showVisual            = bitfield.showVisual;
    info.visualCamAlign        = bitfield.visualCamAlign;
    info.cdStatic              = bitfield.cdStatic;
    info.cdDyn                 = bitfield.cdDyn;
    info.staticVob             = bitfield.staticVob;
    info.dynamicShadow         = bitfield.dynShadow;
    info.physicsEnabled        = bitfield.physicsEnabled;
    info.visualAniMode         = AnimMode(bitfield.visualAniMode);
    info.zBias                 = bitfield.zbias;
    info.isAmbient             = bitfield.bAmbient;
    info.visualAniModeStrength = pd.visualAniStrength;
    info.vobFarClipScale       = pd.vobFarClipZ;

    if(bitfield.hasPresetName)
      parser.getImpl()->readEntry("", info.presetName);

    if(bitfield.hasVobName)
      parser.getImpl()->readEntry("", info.vobName);

    if(bitfield.hasVisualName)
      parser.getImpl()->readEntry("", info.visual);

    hasRelevantVisualObject = bitfield.hasRelevantVisualObject;
    hasAIObject             = bitfield.hasAIObject;
    } else {
    auto& rd = *parser.getImpl();
    rd.readEntry("presetName", info.presetName);
    rd.readEntry("",           &info.bbox, sizeof(info.bbox));
    rd.readEntry("",           &info.rotationMatrix3x3, sizeof(info.rotationMatrix3x3));
    rd.readEntry("",           info.position);
    info.rotationMatrix = info.rotationMatrix3x3.toMatrix();

    rd.readEntry("vobName",        info.vobName);
    rd.readEntry("visual",         info.visual);
    rd.readEntry("showVisual",     info.showVisual);
    rd.readEntry("visualCamAlign", info.visualCamAlign);

    if(version==ZenParser::FileVersion::Gothic1) {
      rd.readEntry("cdStatic",  info.cdStatic);
      rd.readEntry("cdDyn",     info.cdDyn);
      rd.readEntry("staticVob", info.staticVob);
      rd.readEntry("dynShadow", info.dynamicShadow);
      }

    rd.readEntry("visualAniMode", reinterpret_cast<uint8_t&>(info.visualAniMode));

    if(version==ZenParser::FileVersion::Gothic2) {
      rd.readEntry("visualAniModeStrength", info.visualAniModeStrength);
      rd.readEntry("vobFarClipZScale",      info.vobFarClipScale);
      rd.readEntry("cdStatic",              info.cdStatic);
      rd.readEntry("cdDyn",                 info.cdDyn);
      rd.readEntry("staticVob",             info.staticVob);
      rd.readEntry("dynShadow",             info.dynamicShadow);
      rd.readEntry("zBias",                 info.zBias);
      rd.readEntry("isAmbient",             info.isAmbient);
      }
    }

  // Visual-chunk
  if(hasRelevantVisualObject) {
    ZenParser::ChunkHeader hdr;
    parser.readChunkStart(hdr);
    read_Visual(info,parser,hdr,version);
    if(!parser.readChunkEnd())
      parser.skipChunk();
    }

  if(hasAIObject) {
    // Skip ai-chunk
    ZenParser::ChunkHeader tmph;
    parser.readChunkStart(tmph);
    parser.skipChunk();
    }

  // Generate world-matrix
  info.worldMatrix = info.rotationMatrix3x3.toMatrix(info.position);
  }

static void read_zCVobLevelCompo(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobLevelCompo;
  }

static void read_zCVob_oCItem(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCItem;
  rd.readEntry("itemInstance", info.oCItem.instanceName);
  }

static void read_zCVob_zCVobSpot(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobSpot;
  }

static void read_zCVob_zCVobStartpoint(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobStartpoint;
  }

static void read_zCMessageFilter(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCMessageFilter;
  rd.readEntry("triggerTarget", info.zCMessageFilter.triggerTarget);
  rd.readEntry("onTrigger",     reinterpret_cast<uint8_t&>(info.zCMessageFilter.onTrigger));
  rd.readEntry("onUntrigger",   reinterpret_cast<uint8_t&>(info.zCMessageFilter.onUntrigger));
  }

static void read_zCCodeMaster(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);
  uint8_t     count=0;

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCCodeMaster;
  rd.readEntry("triggerTarget",        info.zCCodeMaster.triggerTarget);
  rd.readEntry("orderRelevant",        info.zCCodeMaster.orderRelevant);
  rd.readEntry("firstFalseIsFailure",  info.zCCodeMaster.firstFalseIsFailure);
  rd.readEntry("triggerTargetFailure", info.zCCodeMaster.triggerTargetFailure);
  rd.readEntry("untriggerCancels",     info.zCCodeMaster.untriggerCancels);
  rd.readEntry("", count);
  info.zCCodeMaster.slaveVobName.resize(count);
  for(size_t i=0; i<info.zCCodeMaster.slaveVobName.size(); ++i) {
    char slaveVobName[64]={};
    std::snprintf(slaveVobName,sizeof(slaveVobName),"slaveVobName%d", int(i));
    rd.readEntry(slaveVobName, info.zCCodeMaster.slaveVobName[i]);
    }
  }

static void read_zCVob_zCTrigger(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTrigger;
  rd.readEntry("triggerTarget",     info.zCTrigger.triggerTarget);
  rd.readEntry("flags",             &info.zCTrigger.flags,       sizeof(info.zCTrigger.flags));
  rd.readEntry("filterFlags",       &info.zCTrigger.filterFlags, sizeof(info.zCTrigger.filterFlags));
  rd.readEntry("respondToVobName",  info.zCTrigger.respondToVobName);
  rd.readEntry("numCanBeActivated", info.zCTrigger.numCanBeActivated);
  rd.readEntry("retriggerWaitSec",  info.zCTrigger.retriggerWaitSec);
  rd.readEntry("damageThreshold",   info.zCTrigger.damageThreshold);
  rd.readEntry("fireDelaySec",      info.zCTrigger.fireDelaySec);
  }

static void read_zCVob_zCTrigger_zCTriggerUntouch(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTriggerUntouch;
  rd.readEntry("triggerTarget", info.zCTriggerUntouch.triggerTarget);
  }

static void read_zCVob_zCTrigger_zCMoverControler(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCMoverControler;
  rd.readEntry("triggerTarget", info.zCMoverControler.triggerTarget);
  rd.readEntry("moverMessage",  reinterpret_cast<uint8_t&>(info.zCMoverControler.moverMessage));
  rd.readEntry("gotoFixedKey",  info.zCMoverControler.gotoFixedKey);
  }

static void read_zCVob_zCTrigger_zCTriggerList(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  uint8_t count=0;
  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTriggerList;

  rd.readEntry("listProcess", info.zCTriggerList.listProcess);
  rd.readEntry("",            count);
  info.zCTriggerList.list.resize(count);
  for (uint8_t i = 0; i < count; i++) {
    char triggerTarget[64]={};
    char fireDelay    [64]={};

    if(parser.getZenHeader().fileType==ZenLoad::ZenParser::FT_ASCII)
      std::snprintf(triggerTarget,sizeof(triggerTarget),"triggerTarget%d",int(i)); else
      std::snprintf(triggerTarget,sizeof(triggerTarget),"slaveVobName%d", int(i));

    rd.readEntry(triggerTarget, info.zCTriggerList.list[i].triggerTarget);
    rd.readEntry(fireDelay,     info.zCTriggerList.list[i].fireDelay);
    }
  }

static void read_zCVob_zCTrigger_oCTriggerScript(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCTriggerScript;
  rd.readEntry("scriptFunc", info.zCTriggerScript.scriptFunc);
  }

static void read_zCVob_oCMOB(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMOB;
  rd.readEntry("focusName",       info.oCMOB.focusName);
  rd.readEntry("hitpoints",       info.oCMOB.hitpoints);
  rd.readEntry("damage",          info.oCMOB.damage);
  rd.readEntry("moveable",        info.oCMOB.moveable);
  rd.readEntry("takeable",        info.oCMOB.takeable);
  rd.readEntry("focusOverride",   info.oCMOB.focusOverride);
  rd.readEntry("soundMaterial",   info.oCMOB.soundMaterial);
  rd.readEntry("visualDestroyed", info.oCMOB.visualDestroyed);
  rd.readEntry("owner",           info.oCMOB.owner);
  rd.readEntry("ownerGuild",      info.oCMOB.ownerGuild);
  rd.readEntry("isDestroyed",     info.oCMOB.isDestroyed);
  }

static void read_zCVob_oCMOB_oCMobInter(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobInter;
  rd.readEntry("stateNum",      info.oCMobInter.stateNum);
  rd.readEntry("triggerTarget", info.oCMobInter.triggerTarget);
  rd.readEntry("useWithItem",   info.oCMobInter.useWithItem);
  rd.readEntry("conditionFunc", info.oCMobInter.conditionFunc);
  rd.readEntry("onStateFunc",   info.oCMobInter.onStateFunc);
  rd.readEntry("rewind",        info.oCMobInter.rewind);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobBed(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobBed;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobDoor(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobDoor;
  rd.readEntry("locked",      info.oCMobLockable.locked);
  rd.readEntry("keyInstance", info.oCMobLockable.keyInstance);
  rd.readEntry("pickLockStr", info.oCMobLockable.pickLockStr);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobFire(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobFire;
  rd.readEntry("fireSlot",        info.oCMobFire.fireSlot);
  rd.readEntry("fireVobtreeName", info.oCMobFire.fireVobtreeName);
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobLadder(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobLadder;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobSwitch(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);
  info.vobType = zCVobData::VT_oCMobSwitch;
  }

static void read_zCVob_oCMOB_oCMobInter_oCMobContainer(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCMOB_oCMobInter(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCMobContainer;
  rd.readEntry("locked",      info.oCMobLockable.locked);
  rd.readEntry("keyInstance", info.oCMobLockable.keyInstance);
  rd.readEntry("pickLockStr", info.oCMobLockable.pickLockStr);
  rd.readEntry("contains",    info.oCMobContainer.contains);
  }

static void read_LightData(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  auto& rd = *parser.getImpl();
  // lightPresetInUse or presetName
  rd.readEntry("",                 info.zCVobLight.lightPresetInUse);
  rd.readEntry("lightType",        info.zCVobLight.lightType);
  rd.readEntry("range",            info.zCVobLight.range);
  rd.readColor("color",            info.zCVobLight.color);
  rd.readEntry("spotConeAngle",    info.zCVobLight.spotConeAngle);
  rd.readEntry("lightStatic",      info.zCVobLight.lightStatic);
  rd.readEntry("lightQuality",     info.zCVobLight.lightQuality);
  rd.readEntry("lensflareFX",      info.zCVobLight.lensflareFX);

  if(info.zCVobLight.lightStatic)
    return;

  std::string rangeAniScale, colorAniList;
  rd.readEntry("turnedOn",       info.zCVobLight.dynamic.turnedOn);
  rd.readEntry("rangeAniScale",  rangeAniScale);
  rd.readEntry("rangeAniFPS",    info.zCVobLight.dynamic.rangeAniFPS);
  rd.readEntry("rangeAniSmooth", info.zCVobLight.dynamic.rangeAniSmooth);
  rd.readEntry("colorAniList",   colorAniList);
  rd.readEntry("colorAniFPS",    info.zCVobLight.dynamic.colorAniListFPS);
  rd.readEntry("colorAniSmooth", info.zCVobLight.dynamic.colorAniSmooth);
  if(version==ZenParser::FileVersion::Gothic2)
    rd.readEntry("canMove", info.zCVobLight.dynamic.canMove);

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

static void read_zCVob_zCVobLight(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);
  info.vobType = zCVobData::VT_zCVobLight;
  read_LightData(info,parser,version);
  }

static void read_zCVob_zCVobLightPreset(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  info.vobType = zCVobData::VT_zCVobLightPreset;
  read_LightData(info,parser,version);
  }

static void read_zCVob_zCVobSound(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCVobSound;
  rd.readEntry("sndVolume",       info.zCVobSound.sndVolume);
  rd.readEntry("sndMode",         reinterpret_cast<uint32_t&>(info.zCVobSound.sndMode));
  rd.readEntry("sndRandDelay",    info.zCVobSound.sndRandDelay);
  rd.readEntry("sndRandDelayVar", info.zCVobSound.sndRandDelayVar);
  rd.readEntry("sndStartOn",      info.zCVobSound.sndStartOn);
  rd.readEntry("sndAmbient3D",    info.zCVobSound.sndAmbient3D);
  rd.readEntry("sndObstruction",  info.zCVobSound.sndObstruction);
  rd.readEntry("sndConeAngle",    info.zCVobSound.sndConeAngle);
  rd.readEntry("sndVolType",      reinterpret_cast<uint32_t&>(info.zCVobSound.sndVolType));
  rd.readEntry("sndRadius",       info.zCVobSound.sndRadius);
  rd.readEntry("sndName",         info.zCVobSound.sndName);
  }

static void read_zCVob_zCVobSound_zCVobSoundDaytime(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_zCVobSound(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCVobSoundDaytime;
  rd.readEntry("sndStartTime", info.zCVobSoundDaytime.sndStartTime);
  rd.readEntry("sndEndTime",   info.zCVobSoundDaytime.sndEndTime);
  rd.readEntry("sndName2",     info.zCVobSoundDaytime.sndName2);
  }

static void read_zCVob_oCZoneMusic(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCZoneMusic;
  rd.readEntry("enabled",     info.oCZoneMusic.enabled);
  rd.readEntry("priority",    info.oCZoneMusic.priority);
  rd.readEntry("ellipsoid",   info.oCZoneMusic.ellipsoid);
  rd.readEntry("reverbLevel", info.oCZoneMusic.reverbLevel);
  rd.readEntry("volumeLevel", info.oCZoneMusic.volumeLevel);
  rd.readEntry("loop",        info.oCZoneMusic.loop);
  }

static void read_zCVob_oCZoneMusic_oCZoneMusicDefault(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_oCZoneMusic(info,parser,version);
  info.vobType = zCVobData::VT_oCZoneMusicDefault;
  }

static void read_zCVob_zCTrigger_zCMover(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version){
  read_zCVob_zCTrigger(info,parser,version);
  uint16_t numKeyframes=0;

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCMover;
  rd.readEntry("moverBehavior",      reinterpret_cast<uint32_t&>(info.zCMover.moverBehavior));
  rd.readEntry("touchBlockerDamage", info.zCMover.touchBlockerDamage);
  rd.readEntry("stayOpenTimeSec",    info.zCMover.stayOpenTimeSec);
  rd.readEntry("moverLocked",        info.zCMover.moverLocked);
  rd.readEntry("autoLinkEnabled",    info.zCMover.autoLinkEnable);
  if(version==ZenParser::FileVersion::Gothic2)
    rd.readEntry("autoRotate", info.zCMover.autoRotate);
  rd.readEntry("numKeyframes", numKeyframes);
  if(numKeyframes>0) {
    rd.readEntry("moveSpeed",     info.zCMover.moveSpeed);
    rd.readEntry("posLerpType",   reinterpret_cast<uint32_t&>(info.zCMover.posLerpType));
    rd.readEntry("speedType",     reinterpret_cast<uint32_t&>(info.zCMover.speedType));
    std::vector<zCModelAniSample> fr(numKeyframes);
    rd.readEntry("keyframes", &fr[0],sizeof(zCModelAniSample)*numKeyframes);
    info.zCMover.keyframes = std::move(fr);
    }
  rd.readEntry("sfxOpenStart",  info.zCMover.sfxOpenStart);
  rd.readEntry("sfxOpenEnd",    info.zCMover.sfxOpenEnd);
  rd.readEntry("sfxMoving",     info.zCMover.sfxMoving);
  rd.readEntry("sfxCloseStart", info.zCMover.sfxCloseStart);
  rd.readEntry("sfxCloseEnd",   info.zCMover.sfxCloseEnd);
  rd.readEntry("sfxLock",       info.zCMover.sfxLock);
  rd.readEntry("sfxUnlock",     info.zCMover.sfxUnlock);
  rd.readEntry("sfxUseLocked",  info.zCMover.sfxUseLocked);
  }

static void read_zCVob_zCTrigger_oCTriggerChangeLevel(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob_zCTrigger(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTriggerChangeLevel;
  rd.readEntry("levelName",    info.oCTriggerChangeLevel.levelName);
  rd.readEntry("startVobName", info.oCTriggerChangeLevel.startVobName);
  }

static void read_zCVob_zCTrigger_zCTriggerWorldStart(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTriggerWorldStart;
  rd.readEntry("triggerTarget",     info.oCTriggerWorldStart.triggerTarget);
  rd.readEntry("fireOnlyFirstTime", info.oCTriggerWorldStart.fireOnlyFirstTime);
  }

static void read_zCVob_zCPFXControler(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_zCPFXControler;
  rd.readEntry("pfxName",         info.zCPFXControler.pfxName);
  rd.readEntry("killVobWhenDone", info.zCPFXControler.killVobWhenDone);
  rd.readEntry("pfxStartOn",      info.zCPFXControler.pfxStartOn);
  }

static void read_zCVob_oCTouchDamage(zCVobData &info, ZenParser &parser, ZenParser::FileVersion version) {
  read_zCVob(info,parser,version);

  auto& rd = *parser.getImpl();
  info.vobType = zCVobData::VT_oCTouchDamage;
  rd.readEntry("damage",  info.oCTouchDamage.damage);

  rd.readEntry("Barrier", info.oCTouchDamage.touchDamage.barrier);
  rd.readEntry("Blunt",   info.oCTouchDamage.touchDamage.blunt);
  rd.readEntry("Edge",    info.oCTouchDamage.touchDamage.edge);
  rd.readEntry("Fire",    info.oCTouchDamage.touchDamage.fire);
  rd.readEntry("Fly",     info.oCTouchDamage.touchDamage.fly);
  rd.readEntry("Magic",   info.oCTouchDamage.touchDamage.magic);
  rd.readEntry("Point",   info.oCTouchDamage.touchDamage.point);
  rd.readEntry("Fall",    info.oCTouchDamage.touchDamage.fall);

  rd.readEntry("damageRepeatDelaySec", info.oCTouchDamage.damageRepeatDelaySec);
  rd.readEntry("damageVolDownScale",   info.oCTouchDamage.damageVolDownScale);
  rd.readEntry("damageCollType",       info.oCTouchDamage.damageCollType);
  }

static void readObjectData(zCVobData &info, ZenParser &parser,
                           const ZenParser::ChunkHeader& header, ZenParser::FileVersion version) {
  switch(header.classId) {
    case ZenParser::zUnknown:
    case ZenParser::zReference:
    case ZenParser::zCProgMeshProto:
    case ZenParser::zCCSLib:
    case ZenParser::zCWorld:
    case ZenParser::zCParticleFX:
    case ZenParser::zCMesh:
    case ZenParser::zCModel:
    case ZenParser::zCMorphMesh:
    case ZenParser::zCAICamera:
      return;
    case ZenParser::zCWayNet:
    case ZenParser::zCWaypoint:
      return;
    case ZenParser::zCCSBlock:
    case ZenParser::zCCSAtomicBlock:
    case ZenParser::oCMsgConversation:
      return;
    case ZenParser::zCVobStair:
    case ZenParser::zCVob:
      return read_zCVob(info,parser,version);
    case ZenParser::zCVobLevelCompo:
      return read_zCVobLevelCompo(info,parser,version);
    case ZenParser::zCDecal:
      return read_zCDecal(info,parser,version);
    case ZenParser::oCItem:
      return read_zCVob_oCItem(info,parser,version);

    case ZenParser::oCMOB:
      return read_zCVob_oCMOB(info,parser,version);
    case ZenParser::oCMobInter:
      return read_zCVob_oCMOB_oCMobInter(info,parser,version);
    case ZenParser::oCMobBed:
      return read_zCVob_oCMOB_oCMobInter_oCMobBed(info,parser,version);
    case ZenParser::oCMobFire:
      return read_zCVob_oCMOB_oCMobInter_oCMobFire(info,parser,version);
    case ZenParser::oCMobLadder:
      return read_zCVob_oCMOB_oCMobInter_oCMobLadder(info,parser,version);
    case ZenParser::oCMobSwitch:
      return read_zCVob_oCMOB_oCMobInter_oCMobSwitch(info,parser,version);
    case ZenParser::oCMobWheel:
      return read_zCVob_oCMOB_oCMobInter(info,parser,version); // TODO
    case ZenParser::oCMobContainer:
      return read_zCVob_oCMOB_oCMobInter_oCMobContainer(info,parser,version);
    case ZenParser::oCMobDoor:
      return read_zCVob_oCMOB_oCMobInter_oCMobDoor(info,parser,version);

    case ZenParser::zCPFXControler:
      return read_zCVob_zCPFXControler(info,parser,version);
    case ZenParser::zCVobAnimate:
      return read_zCVob(info,parser,version);
    case ZenParser::zCVobLensFlare:
      return read_zCVob(info,parser,version);
    case ZenParser::zCVobLight:
      return read_zCVob_zCVobLight(info,parser,version);
    case ZenParser::zCVobLightPreset:
      return read_zCVob_zCVobLightPreset(info,parser,version);

    case ZenParser::zCVobSpot:
      return read_zCVob_zCVobSpot(info,parser,version);
    case ZenParser::zCVobStartpoint:
      return read_zCVob_zCVobStartpoint(info,parser,version);

    case ZenParser::zCVobSound:
      return read_zCVob_zCVobSound(info,parser,version);
    case ZenParser::zCVobSoundDaytime:
      return read_zCVob_zCVobSound_zCVobSoundDaytime(info,parser,version);

    case ZenParser::oCZoneMusic:
      return read_zCVob_oCZoneMusic(info,parser,version);
    case ZenParser::oCZoneMusicDefault:
      return read_zCVob_oCZoneMusic_oCZoneMusicDefault(info,parser,version);

    case ZenParser::zCZoneZFog:
    case ZenParser::zCZoneZFogDefault:
    case ZenParser::zCZoneVobFarPlane:
    case ZenParser::zCZoneVobFarPlaneDefault:
      return read_zCVob(info,parser,version);

    case ZenParser::zCMessageFilter:
      return read_zCMessageFilter(info,parser,version);
    case ZenParser::zCCodeMaster:
      return read_zCCodeMaster(info,parser,version);

    case ZenParser::zCTrigger:
    case ZenParser::oCCSTrigger:
      return read_zCVob_zCTrigger(info,parser,version);
    case ZenParser::zCTriggerList:
      return read_zCVob_zCTrigger_zCTriggerList(info,parser,version);
    case ZenParser::oCTriggerScript:
      return read_zCVob_zCTrigger_oCTriggerScript(info,parser,version);
    case ZenParser::zCMover:
      return read_zCVob_zCTrigger_zCMover(info,parser,version);
    case ZenParser::oCTriggerChangeLevel:
      return read_zCVob_zCTrigger_oCTriggerChangeLevel(info,parser,version);
    case ZenParser::zCTriggerWorldStart:
      return read_zCVob_zCTrigger_zCTriggerWorldStart(info,parser,version);
    case ZenParser::zCTriggerUntouch:
      return read_zCVob_zCTrigger_zCTriggerUntouch(info,parser,version);
    case ZenParser::zCMoverControler:
      return read_zCVob_zCTrigger_zCMoverControler(info,parser,version);

    case ZenParser::zCCSCamera:
      return read_zCVob(info,parser,version);
    case ZenParser::zCCamTrj_KeyFrame:
      return read_zCVob(info,parser,version);
    case ZenParser::oCTouchDamage:
      return read_zCVob_oCTouchDamage(info,parser,version);
    case ZenParser::zCEarthquake:
      return read_zCVob(info,parser,version);

    case ZenParser::zCVobScreenFX:
      return read_zCVob(info,parser,version);
    }

  LogInfo() << "skip zCVob, classId = \"" << header.classId << "\"";
  }

void zCVob::readObjectData(zCVobData &info, ZenParser &parser,
                           const ZenParser::ChunkHeader& header, ZenParser::FileVersion version) {
  ::readObjectData(info,parser,header,version);
  if(!parser.readChunkEnd())
    parser.skipChunk();
  }
