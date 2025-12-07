#ifndef TALENT_FUNCTIONS_H
#define TALENT_FUNCTIONS_H

#include "Player.h"
#include "ScriptMgr.h"
#include <unordered_map>

#define MODULE_STRING "npc-talent-template"

namespace NPCTalentTemplate {

enum Misc
{
    TEMPLATE_NPC_FACTION_ASHTONGUE_DEATHSWORN = 1012,
    TEMPLATE_NPC_REP_AMOUNT_EXALTED = 42000,
};

enum Strings
{
    ERROR_MUST_REMOVE_EQUIPPED = 1,
    ERROR_MUST_RESET_TALENTS = 2,
    SUCCESS_EQUIPPED_TEMPLATE = 3,
    SUCCESS_DESTROYED_EQUIPPED_GEAR = 4,
    SUCCESS_REMOVED_GLYPHS = 5,
    SUCCESS_COPIED = 6,
    // Extract
    ERROR_EXTRACT_MUST_SPEND_ALL_TALENT_POINTS = 7,
    ERROR_EXTRACT_GET_GLYPHS = 8,
    SUCCESS_EXTRACT = 9,
    SUCCESS_EXTRACT_INFO = 10,
    // Reload
    FEEDBACK_RELOADING = 11,
    SUCCESS_RELOADED = 12,
    // Kit commands
    USAGE_KIT_CREATE = 13,
    ERROR_INVALID_KITID = 14,
    ERROR_FAILED_CREATE_KIT = 15,
    SUCCESS_KIT_DELETED = 16,
    ERROR_INVALID_KIT_TYPE = 17,
    FEEDBACK_NO_KITS = 18,
    FEEDBACK_KITS_HEADER = 19,
    USAGE_KIT_DELETE = 20,
    SUCCESS_KIT_CREATED = 21,
    FEEDBACK_KIT_ROW = 22,
};

enum TemplateNpcSpells
{
    SPELL_BIG_BATTLE_BEAR = 51412, // Default mount if no config
    SPELL_ARTISAN_RIDING = 34091,
    SPELL_COLD_WEATHER_FLYING = 54197,
    SPELL_LEARN_A_SECOND_TALENT_SPECIALIZATION = 63624,
    SPELL_MASTER_HAMMERSMITH = 17040,
    SPELL_TEACH_LEARN_TALENT_SPECIALIZATION_SWITCHES = 63680,
    // Armor proficiency
    SPELL_PLATE_MAIL = 750,
    SPELL_MAIL = 8737
};

enum WeaponProficiencies
{
    BLOCK = 107,
    BOWS = 264,
    CROSSBOWS = 5011,
    DAGGERS = 1180,
    FIST_WEAPONS = 15590,
    GUNS = 266,
    ONE_H_AXES = 196,
    ONE_H_MACES = 198,
    ONE_H_SWORDS = 201,
    POLEARMS = 200,
    SHOOT = 5019,
    STAVES = 227,
    TWO_H_AXES = 197,
    TWO_H_MACES = 199,
    TWO_H_SWORDS = 202,
    WANDS = 5009,
    THROW_WAR = 2567
};

struct TalentEntry
{
    uint32 kitId;
    uint32 talentId;
};

struct GlyphEntry
{
    uint32 kitId;
    uint8 slot;
    uint32 glyph;
};

struct GearEntry
{
    uint32 kitId;
    uint32 playerRaceMask;
    uint8 pos;
    uint32 itemEntry;
    uint32 enchant;
    uint32 socket1;
    uint32 socket2;
    uint32 socket3;
    uint32 bonusEnchant;
    uint32 prismaticEnchant;
};

struct Kit
{
    uint32 id;
    std::string kitType; // 'gear','talent','glyph'
    std::string name;
    std::string playerClass;
    std::string playerSpec;
};


// (Gossip actions defined later)

enum TemplateFlags
{
    TEMPLATE_APPLY_GEAR = 0x1,
    TEMPLATE_APPLY_GLYPHS = 0x2,
    TEMPLATE_APPLY_TALENTS = 0x4,

    TEMPLATE_APPLY_ALL = TEMPLATE_APPLY_GEAR | TEMPLATE_APPLY_TALENTS | TEMPLATE_APPLY_GLYPHS,
};

enum TemplateCategory
{
    TEMPLATE_CATEGORY_PVE = 0,
    TEMPLATE_CATEGORY_PVP,
    TEMPLATE_CATEGORY_VIP,
    TEMPLATE_CATEGORY_UTILITY
};

enum GossipActions {
    GOSSIP_ACTION_CATEGORY_PVE = 1'000,
    GOSSIP_ACTION_CATEGORY_PVP = 1'001,
    GOSSIP_ACTION_CATEGORY_VIP = 1'002,
    GOSSIP_ACTION_SPACER = 1'003, // ---------
    GOSSIP_ACTION_RESET_TALENTS = 1'004,
    GOSSIP_ACTION_RESET_PET_TALENTS = 1'005,
    GOSSIP_ACTION_RESET_REMOVE_GLYPHS = 1'006,
    GOSSIP_ACTION_RESET_REMOVE_EQUIPPED_GEAR = 1'007,
    // Ranges to encode templateId in actionId = base + templateId
    GOSSIP_ACTION_SELECT_TEMPLATE_BASE = 10'000,
    GOSSIP_ACTION_APPLY_ALL_BASE = 20'000,
    GOSSIP_ACTION_APPLY_GEAR_BASE = 30'000,
    GOSSIP_ACTION_APPLY_TALENTS_BASE = 40'000,
    GOSSIP_ACTION_APPLY_GLYPHS_BASE = 50'000,
};

typedef std::unordered_map<uint32, std::vector<GearEntry*>> GearKitContainer;
typedef std::unordered_map<uint32, std::vector<TalentEntry*>> TalentKitContainer;
typedef std::unordered_map<uint32, std::vector<GlyphEntry*>> GlyphKitContainer;

struct TemplateEntry
{
    uint32 id;
    std::string playerClass;
    uint32 mask;
    std::string category;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 gearKitId;
    uint32 talentKitId;
    uint32 glyphKitId;
    std::string lockRequirement;
    std::string templateName;
    std::string icon;
    bool enabled;
    uint32 menuOrder;
};

typedef std::vector<TemplateEntry*> TemplateContainer;

class sTemplateNPC
{
public:
    static sTemplateNPC* instance()
    {
        static auto* instance = new sTemplateNPC();
        return instance;
    }
    void LoadKits();
    void LoadTemplates();

    void EquipTemplateGearByKit(Player* player, uint32 kitId);
    void LearnTemplateTalentsByKit(Player* player, uint32 kitId);
    void LearnTemplateGlyphsByKit(Player* player, uint32 kitId);

    static bool PreApplyChecks(Player* player, TemplateEntry* templateEntry, TemplateFlags parts);
    void ApplyGearPart(Player* player, TemplateEntry* templateEntry);
    void ApplyTalentsPart(Player* player, TemplateEntry* templateEntry);
    void ApplyGlyphsPart(Player* player, TemplateEntry* templateEntry);
    void PostApplyUpdates(Player* player, TemplateEntry* templateEntry, TemplateFlags parts) const;

    static bool OverwriteKit(uint32 kitId);
    static void ExtractGearKitToDB(Player* player, uint32 kitId);
    static void ExtractTalentKitToDB(Player* player, uint32 kitId);
    static void ExtractGlyphKitToDB(Player* player, uint32 kitId);
    static uint32 InsertKitEntryToDB(Player* player, uint32 kitId, const std::string& kitType, const std::string& name);

    void EquipTemplateGear(Player* player, const std::string& sGear);
    void LearnTemplateTalents(Player* player, const std::string& sTalents);
    void LearnTemplateGlyphs(Player* player, const std::string& sGlyphs);
    void ApplyTemplate(Player* player, TemplateEntry* templateEntry);

    GearKitContainer gearKits;
    GlyphKitContainer glyphKits;
    TalentKitContainer talentKits;
    TemplateContainer templateContainer;

    bool enableResetTalents{};
    bool enableRemoveAllGlyphs{};
    bool enableDestroyEquippedGear{};
    uint32 allianceMount{};
    uint32 hordeMount{};
};

std::string GetClassString(Player* player);
void LearnPlateMailSpells(Player *player);
void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry);
void ApplyGlyph(Player* player, uint8 slot, uint32 glyphID);
void RemoveAllGlyphs(Player* player);

bool IsWearingAnyGear(Player* player);
bool HasSpentTalentPoints(Player* player);
void SatisfyExtraGearRequirements(Player* player, const std::string& sGear);
void LearnWeaponSkills(Player* player);

} // namespace NPCTalentTemplate

#define sTemplateNpcMgr NPCTalentTemplate::sTemplateNPC::instance()
#endif