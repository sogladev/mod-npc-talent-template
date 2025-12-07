#include "npc_talent_template.h"

#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "ReputationMgr.h"
#include "ScriptedGossip.h"
#include "SpellMgr.h"
#include <sstream>
#include <cctype>
#include <fmt/format.h>

using namespace NPCTalentTemplate;

void NPCTalentTemplate::LearnWeaponSkills(Player* player)
{
    WeaponProficiencies wepSkills[] = {
        BLOCK, BOWS, CROSSBOWS, DAGGERS, FIST_WEAPONS, GUNS, ONE_H_AXES, ONE_H_MACES,
        ONE_H_SWORDS, POLEARMS, SHOOT, STAVES, TWO_H_AXES, TWO_H_MACES, TWO_H_SWORDS, WANDS, THROW_WAR
    };

    uint32 size = 17;

    for (uint32 i = 0; i < size; ++i)
        if (player->HasSpell(wepSkills[i]))
            continue;

    switch (player->getClass())
    {
        case CLASS_WARRIOR:
            player->learnSpell(THROW_WAR);
            player->learnSpell(TWO_H_SWORDS);
            player->learnSpell(TWO_H_MACES);
            player->learnSpell(TWO_H_AXES);
            player->learnSpell(STAVES);
            player->learnSpell(POLEARMS);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(ONE_H_AXES);
            player->learnSpell(GUNS);
            player->learnSpell(FIST_WEAPONS);
            player->learnSpell(DAGGERS);
            player->learnSpell(CROSSBOWS);
            player->learnSpell(BOWS);
            player->learnSpell(BLOCK);
            break;
        case CLASS_PRIEST:
            player->learnSpell(WANDS);
            player->learnSpell(STAVES);
            player->learnSpell(SHOOT);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(DAGGERS);
            break;
        case CLASS_PALADIN:
            player->learnSpell(TWO_H_SWORDS);
            player->learnSpell(TWO_H_MACES);
            player->learnSpell(TWO_H_AXES);
            player->learnSpell(POLEARMS);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(ONE_H_AXES);
            player->learnSpell(BLOCK);
            break;
        case CLASS_ROGUE:
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(ONE_H_AXES);
            player->learnSpell(GUNS);
            player->learnSpell(FIST_WEAPONS);
            player->learnSpell(DAGGERS);
            player->learnSpell(CROSSBOWS);
            player->learnSpell(BOWS);
            break;
        case CLASS_DEATH_KNIGHT:
            player->learnSpell(TWO_H_SWORDS);
            player->learnSpell(TWO_H_MACES);
            player->learnSpell(TWO_H_AXES);
            player->learnSpell(POLEARMS);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(ONE_H_AXES);
            break;
        case CLASS_MAGE:
            player->learnSpell(WANDS);
            player->learnSpell(STAVES);
            player->learnSpell(SHOOT);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(DAGGERS);
            break;
        case CLASS_SHAMAN:
            player->learnSpell(TWO_H_MACES);
            player->learnSpell(TWO_H_AXES);
            player->learnSpell(STAVES);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(ONE_H_AXES);
            player->learnSpell(FIST_WEAPONS);
            player->learnSpell(DAGGERS);
            player->learnSpell(BLOCK);
            break;
        case CLASS_HUNTER:
            player->learnSpell(THROW_WAR);
            player->learnSpell(TWO_H_SWORDS);
            player->learnSpell(TWO_H_AXES);
            player->learnSpell(STAVES);
            player->learnSpell(POLEARMS);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(ONE_H_AXES);
            player->learnSpell(GUNS);
            player->learnSpell(FIST_WEAPONS);
            player->learnSpell(DAGGERS);
            player->learnSpell(CROSSBOWS);
            player->learnSpell(BOWS);
            break;
        case CLASS_DRUID:
            player->learnSpell(TWO_H_MACES);
            player->learnSpell(STAVES);
            player->learnSpell(POLEARMS);
            player->learnSpell(ONE_H_MACES);
            player->learnSpell(FIST_WEAPONS);
            player->learnSpell(DAGGERS);
            break;
        case CLASS_WARLOCK:
            player->learnSpell(WANDS);
            player->learnSpell(STAVES);
            player->learnSpell(SHOOT);
            player->learnSpell(ONE_H_SWORDS);
            player->learnSpell(DAGGERS);
            break;
        default:
            break;
        }
    player->UpdateSkillsToMaxSkillsForLevel();
}

#define DEFAULT_GOSSIP_ACTION_ENTRY 9999 // default value for gossipAction when creating new template

enum TalentsAndSpells
{
    TALENT_MANGLE = 33917,
    SPELL_MANGLE_CAT = 33876, // Rank 1
    SPELL_MANGLE_BEAR = 33878 // Rank 1
};

void NPCTalentTemplate::LearnPlateMailSpells(Player *player)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            player->learnSpell(SPELL_PLATE_MAIL);
            break;
        case CLASS_SHAMAN:
        case CLASS_HUNTER:
            player->learnSpell(SPELL_MAIL);
            break;
        default:
        break;
    }
}

void NPCTalentTemplate::ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
{
    if (!item)
        return;

    if (!bonusEntry)
        return;

    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, bonusEntry, 0, 0);
    player->ApplyEnchantment(item, slot, true);
}

void NPCTalentTemplate::ApplyGlyph(Player* player, uint8 slot, uint32 glyphID)
{
    if (uint32 oldGlyph = player->GetGlyph(slot))
    {
        player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(oldGlyph)->SpellId);
        player->SetGlyph(slot, 0, true);
    }
    if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyphID))
    {
        player->CastSpell(player, gp->SpellId, true);
        player->SetGlyph(slot, glyphID, true);
    }
}

void NPCTalentTemplate::RemoveAllGlyphs(Player* player)
{
    for (uint8 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
        if (uint32 glyph = player->GetGlyph(i))
            if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))
                if (sGlyphSlotStore.LookupEntry(player->GetGlyphSlot(i)))
                {
                    player->RemoveAurasDueToSpell(gp->SpellId);
                    player->SetGlyph(i, 0, true);
                    player->SendTalentsInfoData(false); // this is somewhat an in-game glyph realtime update (apply/remove)
                }
}

void sTemplateNPC::LoadKits()
{
    // Clear existing kit containers
    for (auto &pair : gearKits)
        for (auto *e : pair.second)
            delete e;
    gearKits.clear();
    for (auto &pair : talentKits)
        for (auto *e : pair.second)
            delete e;
    talentKits.clear();
    for (auto &pair : glyphKits)
        for (auto *e : pair.second)
            delete e;
    glyphKits.clear();

    // Load gear entries grouped by kit_id
    QueryResult result = CharacterDatabase.Query("SELECT `kit_id`, `playerRaceMask`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant` FROM `mod_npc_talent_template_gear` ORDER BY `kit_id`, `pos`;");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            auto* ge = new GearEntry();
            uint32 kitId = f[0].Get<uint32>();
            ge->kitId = kitId;
            ge->playerRaceMask = f[1].Get<uint32>();
            ge->pos = f[2].Get<uint8>();
            ge->itemEntry = f[3].Get<uint32>();
            ge->enchant = f[4].Get<uint32>();
            ge->socket1 = f[5].Get<uint32>();
            ge->socket2 = f[6].Get<uint32>();
            ge->socket3 = f[7].Get<uint32>();
            ge->bonusEnchant = f[8].Get<uint32>();
            ge->prismaticEnchant = f[9].Get<uint32>();
            gearKits[kitId].emplace_back(ge);
        } while (result->NextRow());
    }

    // Load talents
    result = CharacterDatabase.Query("SELECT `kit_id`, `talentId` FROM `mod_npc_talent_template_talents` ORDER BY `kit_id`;");
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            auto* talentEntry = new TalentEntry();
            uint32 kitId = field[0].Get<uint32>();
            talentEntry->kitId = kitId;
            talentEntry->talentId = field[1].Get<uint32>();
            talentKits[kitId].emplace_back(talentEntry);
        } while (result->NextRow());
    }

    // Load glyphs
    result = CharacterDatabase.Query("SELECT `kit_id`, `slot`, `glyph` FROM `mod_npc_talent_template_glyphs` ORDER BY `kit_id`, `slot`;");
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            auto* glyphEntry = new GlyphEntry();
            uint32 kitId = field[0].Get<uint32>();
            glyphEntry->kitId = kitId;
            glyphEntry->slot = field[1].Get<uint8>();
            glyphEntry->glyph = field[2].Get<uint32>();
            glyphKits[kitId].emplace_back(glyphEntry);
        } while (result->NextRow());
    }

    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} gear kits, {} talent kits, {} glyph kits.", gearKits.size(), talentKits.size(), glyphKits.size());
}

void sTemplateNPC::LoadTemplates()
{
    for (auto* templateEntry : templateContainer)
        delete templateEntry;
    templateContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT `id`, `playerClass`, `mask`, `category`, `minLevel`, `maxLevel`, `gear_kit_id`, `talent_kit_id`, `glyph_kit_id`, `lock_requirement`, `template_name`, `icon`, `enabled` FROM `mod_npc_talent_template_templates`;");
    if (!result)
    {
        LOG_INFO("module", ">> TEMPLATE NPC: Loaded 0 templates. DB table `mod_npc_talent_template_templates` is empty!");
        return;
    }

    do
    {
        Field* field = result->Fetch();
        auto* templateEntry = new TemplateEntry();
        templateEntry->id = field[0].Get<uint32>();
        templateEntry->playerClass = field[1].Get<std::string>();
        templateEntry->mask = field[2].Get<uint32>();
        templateEntry->category = field[3].Get<std::string>();
        templateEntry->minLevel = field[4].Get<uint32>();
        templateEntry->maxLevel = field[5].Get<uint32>();
        templateEntry->gearKitId = field[6].Get<uint32>();
        templateEntry->talentKitId = field[7].Get<uint32>();
        templateEntry->glyphKitId = field[8].Get<uint32>();
        templateEntry->lockRequirement = field[9].Get<std::string>();
        templateEntry->templateName = field[10].Get<std::string>();
        templateEntry->icon = field[11].Get<std::string>();
        templateEntry->enabled = field[12].Get<uint8>();
        templateContainer.emplace_back(templateEntry);
    } while (result->NextRow());

    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} templates.", templateContainer.size());
}

void sTemplateNPC::EquipTemplateGearByKit(Player* player, uint32 kitId)
{
    auto it = gearKits.find(kitId);
    if (it == gearKits.end())
        return;
    // Equip items from kit rows; assume rows are ordered by pos
    for (auto const& ge : it->second)
    {
        if (!(ge->playerRaceMask & player->getRaceMask()))
            continue;
        if (Item* item = player->EquipNewItem(ge->pos, ge->itemEntry, true))
        {
            ApplyBonus(player, item, PERM_ENCHANTMENT_SLOT, ge->enchant);
            ApplyBonus(player, item, BONUS_ENCHANTMENT_SLOT, ge->bonusEnchant);
            ApplyBonus(player, item, PRISMATIC_ENCHANTMENT_SLOT, ge->prismaticEnchant);
            ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_2, ge->socket2);
            ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_3, ge->socket3);
            ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT, ge->socket1);
        }
    }
}

void sTemplateNPC::LearnTemplateTalentsByKit(Player* player, uint32 kitId)
{
    auto it = talentKits.find(kitId);
    if (it == talentKits.end())
        return;
    for (auto const& te : it->second)
    {
        player->learnSpellHighRank(te->talentId);
        player->addTalent(te->talentId, player->GetActiveSpecMask(), 0);
        if (te->talentId == TALENT_MANGLE)
        {
            player->CastSpell(player, TALENT_MANGLE, true);
            auto LearnHighestRankForLevel = [player](uint32 baseRankId)
            {
                for (uint32 id = baseRankId; id; id = sSpellMgr->GetNextSpellInChain(id))
                {
                    const SpellInfo* info = sSpellMgr->GetSpellInfo(id);
                    if (!info || info->BaseLevel > player->GetLevel())
                        break;
                    player->learnSpell(id);
                }
            };
            LearnHighestRankForLevel(SPELL_MANGLE_CAT);
            LearnHighestRankForLevel(SPELL_MANGLE_BEAR);
        }
    }
    player->InitTalentForLevel();
}

void sTemplateNPC::LearnTemplateGlyphsByKit(Player* player, uint32 kitId)
{
    auto it = glyphKits.find(kitId);
    if (it == glyphKits.end())
        return;
    for (auto const& ge : it->second)
    {
        ApplyGlyph(player, ge->slot, ge->glyph);
    }
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::ApplyTemplate(Player* player, TemplateEntry* templateEntry)
{
    if (!player || !templateEntry)
        return;
    auto templateFlags = static_cast<TemplateFlags>(templateEntry->mask);

    if (!PreApplyChecks(player, templateEntry, templateFlags))
        return;

    if ((templateFlags & TEMPLATE_APPLY_GEAR) && templateEntry->gearKitId)
        ApplyGearPart(player, templateEntry);

    if ((templateFlags & TEMPLATE_APPLY_GLYPHS) && templateEntry->glyphKitId)
        ApplyGlyphsPart(player, templateEntry);

    if ((templateFlags & TEMPLATE_APPLY_TALENTS) && templateEntry->talentKitId)
        ApplyTalentsPart(player, templateEntry);

    if (templateFlags & (TEMPLATE_APPLY_TALENTS | TEMPLATE_APPLY_GEAR))
        player->UpdateTitansGrip();

    LearnWeaponSkills(player);
    player->SetHealth(player->GetMaxHealth());
    if (player->getPowerType() == POWER_MANA)
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

    if (!player->HasSpell(SPELL_ARTISAN_RIDING))
        player->learnSpell(SPELL_ARTISAN_RIDING);
    if (!player->HasSpell(SPELL_COLD_WEATHER_FLYING))
        player->learnSpell(SPELL_COLD_WEATHER_FLYING);
    if (player->GetTeamId() == TEAM_HORDE && !player->HasSpell(hordeMount))
        player->learnSpell(hordeMount);
    else if (player->GetTeamId() == TEAM_ALLIANCE && !player->HasSpell(allianceMount))
        player->learnSpell(allianceMount);

    if (!player->HasSpell(SPELL_TEACH_LEARN_TALENT_SPECIALIZATION_SWITCHES))
        player->CastSpell(player, SPELL_TEACH_LEARN_TALENT_SPECIALIZATION_SWITCHES, player->GetGUID());
    if (!player->HasSpell(SPELL_LEARN_A_SECOND_TALENT_SPECIALIZATION))
        player->CastSpell(player, SPELL_LEARN_A_SECOND_TALENT_SPECIALIZATION, player->GetGUID());

    // Notify
    PostApplyUpdates(player, templateEntry, templateFlags);
}

bool sTemplateNPC::PreApplyChecks(Player* player, TemplateEntry* templateEntry, TemplateFlags parts)
{
    if (!player || !templateEntry)
        return false;

    if ((parts & TEMPLATE_APPLY_GEAR) && IsWearingAnyGear(player))
    {
        player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, ERROR_MUST_REMOVE_EQUIPPED)->c_str());
        ChatHandler(player->GetSession()).PSendModuleSysMessage(MODULE_STRING, ERROR_MUST_REMOVE_EQUIPPED);
        return false;
    }

    if ((parts & TEMPLATE_APPLY_TALENTS) && HasSpentTalentPoints(player))
    {
        player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, ERROR_MUST_RESET_TALENTS)->c_str());
        ChatHandler(player->GetSession()).PSendModuleSysMessage(MODULE_STRING, ERROR_MUST_RESET_TALENTS);
        return false;
    }
    return true;
}

void sTemplateNPC::ApplyGearPart(Player* player, TemplateEntry* templateEntry)
{
    player->_RemoveAllItemMods();
    LearnPlateMailSpells(player);
    // restore the original hack-based extra requirements
    SatisfyExtraGearRequirements(player, templateEntry->templateName);
    if (templateEntry->gearKitId)
        EquipTemplateGearByKit(player, templateEntry->gearKitId);
}

void sTemplateNPC::ApplyGlyphsPart(Player* player, TemplateEntry* templateEntry)
{
    if (templateEntry->glyphKitId)
        LearnTemplateGlyphsByKit(player, templateEntry->glyphKitId);
}

void sTemplateNPC::ApplyTalentsPart(Player* player, TemplateEntry* templateEntry)
{
    if (templateEntry->talentKitId)
        LearnTemplateTalentsByKit(player, templateEntry->talentKitId);
    player->UpdateTitansGrip();
}

void sTemplateNPC::PostApplyUpdates(Player* player, TemplateEntry* templateEntry, TemplateFlags parts) const
{
    if (parts & (TEMPLATE_APPLY_TALENTS | TEMPLATE_APPLY_GEAR))
        player->UpdateTitansGrip();

    LearnWeaponSkills(player);
    player->SetHealth(player->GetMaxHealth());
    if (player->getPowerType() == POWER_MANA)
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

    if (!player->HasSpell(SPELL_ARTISAN_RIDING))
        player->learnSpell(SPELL_ARTISAN_RIDING);
    if (!player->HasSpell(SPELL_COLD_WEATHER_FLYING))
        player->learnSpell(SPELL_COLD_WEATHER_FLYING);
    if (player->GetTeamId() == TEAM_HORDE && !player->HasSpell(hordeMount))
        player->learnSpell(hordeMount);
    else if (player->GetTeamId() == TEAM_ALLIANCE && !player->HasSpell(allianceMount))
        player->learnSpell(allianceMount);

    if (!player->HasSpell(SPELL_TEACH_LEARN_TALENT_SPECIALIZATION_SWITCHES))
        player->CastSpell(player, SPELL_TEACH_LEARN_TALENT_SPECIALIZATION_SWITCHES, player->GetGUID());
    if (!player->HasSpell(SPELL_LEARN_A_SECOND_TALENT_SPECIALIZATION))
        player->CastSpell(player, SPELL_LEARN_A_SECOND_TALENT_SPECIALIZATION, player->GetGUID());

    player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, SUCCESS_EQUIPPED_TEMPLATE)->c_str(), GetClassString(player), templateEntry->templateName.c_str());
}

std::string NPCTalentTemplate::GetClassString(Player* player)
{
    return EnumUtils::ToTitle(Classes(player->getClass()));
}

// Legacy template create extractor removed - use kit-based extracts

// End legacy create API removal

bool sTemplateNPC::OverwriteKit(uint32 kitId)
{
    CharacterDatabase.Execute("DELETE FROM `mod_npc_talent_template_gear` WHERE `kit_id`={}", kitId);
    CharacterDatabase.Execute("DELETE FROM `mod_npc_talent_template_talents` WHERE `kit_id`={}", kitId);
    CharacterDatabase.Execute("DELETE FROM `mod_npc_talent_template_glyphs` WHERE `kit_id`={}", kitId);
    CharacterDatabase.Execute("DELETE FROM `mod_npc_talent_template_kits` WHERE `id`={}", kitId);
    return true;
}

void sTemplateNPC::ExtractGearKitToDB(Player* player, uint32 kitId)
{
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            CharacterDatabase.Execute(
                "INSERT INTO `mod_npc_talent_template_gear` (`kit_id`, `playerRaceMask`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
                kitId, player->getRaceMask(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3), equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT)
            );
}

void sTemplateNPC::ExtractTalentKitToDB(Player* player, uint32 kitId)
{
    QueryResult result = CharacterDatabase.Query("SELECT `spell` FROM `character_talent` WHERE `guid`={} AND `specMask`&{}", player->GetGUID().GetCounter(), player->GetActiveSpecMask());
    if (!result)
        return;

    if (player->GetFreeTalentPoints() > 0)
    {
        player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, ERROR_EXTRACT_MUST_SPEND_ALL_TALENT_POINTS)->c_str());
        ChatHandler(player->GetSession()).PSendModuleSysMessage(MODULE_STRING, ERROR_EXTRACT_MUST_SPEND_ALL_TALENT_POINTS);
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint32 spell = fields[0].Get<uint32>();
        CharacterDatabase.Execute("INSERT INTO `mod_npc_talent_template_talents` (`kit_id`, `talentId`) VALUES ({}, {})", kitId, spell);
    } while (result->NextRow());
}

void sTemplateNPC::ExtractGlyphKitToDB(Player* player, uint32 kitId)
{
    QueryResult result = CharacterDatabase.Query("SELECT `glyph1`, `glyph2`, `glyph3`, `glyph4`, `glyph5`, `glyph6` FROM `character_glyphs` WHERE `guid`={} AND `talentGroup`={}", player->GetGUID().GetCounter(), player->GetActiveSpec());
    if (!result)
    {
        player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, ERROR_EXTRACT_GET_GLYPHS)->c_str());
        ChatHandler(player->GetSession()).PSendModuleSysMessage(MODULE_STRING, ERROR_EXTRACT_GET_GLYPHS);
        return;
    }

    Field* fields = result->Fetch();
    uint32 glyph1 = fields[0].Get<uint32>();
    uint32 glyph2 = fields[1].Get<uint32>();
    uint32 glyph3 = fields[2].Get<uint32>();
    uint32 glyph4 = fields[3].Get<uint32>();
    uint32 glyph5 = fields[4].Get<uint32>();
    uint32 glyph6 = fields[5].Get<uint32>();

    uint32 storedGlyphs[MAX_GLYPH_SLOT_INDEX] = { glyph1, glyph2, glyph3, glyph4, glyph5, glyph6 };
    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
        CharacterDatabase.Execute("INSERT INTO `mod_npc_talent_template_glyphs` (`kit_id`, `slot`, `glyph`) VALUES ({}, {}, {});", kitId, slot, storedGlyphs[slot]);
}

uint32 sTemplateNPC::InsertKitEntryToDB(Player* player, uint32 kitId, const std::string& kitType, const std::string& name)
{
    if (kitId != 0)
    {
        CharacterDatabase.Execute("INSERT INTO `mod_npc_talent_template_kits` (`id`, `kit_type`, `playerClass`, `name`) VALUES ({}, '{}', '{}', '{}');", kitId, kitType.c_str(), GetClassString(player).c_str(), name.c_str());
        return kitId;
    }

    CharacterDatabase.Execute("INSERT INTO `mod_npc_talent_template_kits` (`kit_type`, `playerClass`, `name`) VALUES ('{}', '{}', '{}');", kitType.c_str(), GetClassString(player).c_str(), name.c_str());
    QueryResult result = CharacterDatabase.Query("SELECT `id` FROM `mod_npc_talent_template_kits` WHERE `kit_type`='{}' AND `playerClass`='{}' AND `name`='{}' LIMIT 1;", kitType.c_str(), GetClassString(player).c_str(), name.c_str());
    if (!result)
        return 0;
    Field* f = result->Fetch();
    uint32 assignedId = f[0].Get<uint32>();
    return assignedId;
}

bool NPCTalentTemplate::HasSpentTalentPoints(Player* player)
{
    return static_cast<bool>(player->GetFreeTalentPoints() != player->CalculateTalentsPoints());
}

bool NPCTalentTemplate::IsWearingAnyGear(Player* player)
{
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            return true;
    return false;
}

// Reintroduced hack to satisfy extra requirements for some kits
void NPCTalentTemplate::SatisfyExtraGearRequirements(Player* player, const std::string& sGear)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_SHAMAN:
            if (!player->HasSpell(SPELL_MASTER_HAMMERSMITH) && (sGear == "Fury70PvET6" || sGear == "Enhancement70PvET6"))
                player->learnSpell(SPELL_MASTER_HAMMERSMITH);
            break;
        case CLASS_DRUID:
            if ((sGear == "Restoration70PvET6") && (player->GetReputationRank(TEMPLATE_NPC_FACTION_ASHTONGUE_DEATHSWORN) < REP_EXALTED))
                player->GetReputationMgr().SetReputation(sFactionStore.LookupEntry(TEMPLATE_NPC_FACTION_ASHTONGUE_DEATHSWORN), TEMPLATE_NPC_REP_AMOUNT_EXALTED);
            break;
        default:
            break;
    }
}

class npc_talent_template : public CreatureScript
{
public:
    npc_talent_template() : CreatureScript("npc_talent_template") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        // Show category choices
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "PvE Templates", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CATEGORY_PVE);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "PvP Templates", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CATEGORY_PVP);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "VIP Templates", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CATEGORY_VIP);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Utility", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SPACER);

        // Extra actions
        if (sTemplateNpcMgr->enableResetTalents || sTemplateNpcMgr->enableRemoveAllGlyphs || sTemplateNpcMgr->enableDestroyEquippedGear)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "----------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SPACER);

        if (sTemplateNpcMgr->enableResetTalents)
        {
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\Trade_Engineering:30:30|t|r Reset Talents", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_TALENTS, "Are you sure you want to reset your talents?", 0, false);
            if (player->getClass() == CLASS_HUNTER)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_hunter_beasttaming:30:30|t|r Reset Pet Talents", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PET_TALENTS, "Are you sure you want to reset your pet's talents?", 0, false);
        }

        if (sTemplateNpcMgr->enableRemoveAllGlyphs)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\Spell_ChargeNegative:30|t|r Remove all glyphs", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_REMOVE_GLYPHS, "Are you sure you want to remove all your glyphs?", 0, false);

        if (sTemplateNpcMgr->enableDestroyEquippedGear)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_vehicle_launchplayer:30|t|r Destroy my equipped gear", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_REMOVE_EQUIPPED_GEAR, "Are you sure you want to destroy all your equipped gear?", 0, false);

        SendGossipMenuFor(player, creature->GetEntry(), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        if (!player || !creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        auto FindTemplateById = [&](uint32 id)-> TemplateEntry*
        {
            for (auto *templateEntry : sTemplateNpcMgr->templateContainer)
                if (templateEntry->id == id)
                    return templateEntry;
            return nullptr;
        };

        auto IsUnlocked = [&](TemplateEntry* templateEntry)->bool
        {
            return templateEntry->lockRequirement.empty();
        };

        // Category selection
        if (uiAction == GOSSIP_ACTION_CATEGORY_PVE || uiAction == GOSSIP_ACTION_CATEGORY_PVP || uiAction == GOSSIP_ACTION_CATEGORY_VIP)
        {
            std::string requestedCategory = (uiAction == GOSSIP_ACTION_CATEGORY_PVE) ? "pve" : (uiAction == GOSSIP_ACTION_CATEGORY_PVP) ? "pvp" : "vip";
            for (TemplateEntry* templateEntry : sTemplateNpcMgr->templateContainer)
            {
                if (templateEntry->playerClass != GetClassString(player))
                    continue;
                if (player->GetLevel() < templateEntry->minLevel || player->GetLevel() > templateEntry->maxLevel)
                    continue;
                if (templateEntry->category != requestedCategory)
                    continue;
                if (!templateEntry->enabled)
                    continue;

                std::string text = templateEntry->templateName;
                if (!IsUnlocked(templateEntry))
                    text += " (locked)";
                if (IsUnlocked(templateEntry))
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, text, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SELECT_TEMPLATE_BASE + templateEntry->id);
                else
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, text, GOSSIP_SENDER_MAIN, 0);
            }
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SPACER);
            SendGossipMenuFor(player, creature->GetEntry(), creature->GetGUID());
            return true;
        }

        // Template selected -> show apply options
        if (uiAction >= GOSSIP_ACTION_SELECT_TEMPLATE_BASE && uiAction < GOSSIP_ACTION_APPLY_ALL_BASE)
        {
            uint32 id = uiAction - GOSSIP_ACTION_SELECT_TEMPLATE_BASE;
            TemplateEntry* templateEntry = FindTemplateById(id);
            if (!templateEntry) { CloseGossipMenuFor(player); return false; }

            if (templateEntry->mask & TEMPLATE_APPLY_ALL)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Apply All (Gear + Glyphs + Talents)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_APPLY_ALL_BASE + templateEntry->id);
            if (templateEntry->mask & TEMPLATE_APPLY_GEAR)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Apply Gear Only", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_APPLY_GEAR_BASE + templateEntry->id);
            if (templateEntry->mask & TEMPLATE_APPLY_TALENTS)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Apply Talents Only", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_APPLY_TALENTS_BASE + templateEntry->id);
            if (templateEntry->mask & TEMPLATE_APPLY_GLYPHS)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Apply Glyphs Only", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_APPLY_GLYPHS_BASE + templateEntry->id);

            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SPACER);
            SendGossipMenuFor(player, creature->GetEntry(), creature->GetGUID());
            return true;
        }

        // Apply action handling (gear/glyphs/talents/all)
        if (uiAction >= GOSSIP_ACTION_APPLY_ALL_BASE && uiAction <= GOSSIP_ACTION_APPLY_GLYPHS_BASE + 0xFFFF)
        {
            // Reuse logic from ApplyTemplate for the selected template
            uint32 id = 0;
            TemplateEntry* templateEntry = nullptr;

            if (uiAction >= GOSSIP_ACTION_APPLY_ALL_BASE && uiAction < GOSSIP_ACTION_APPLY_GEAR_BASE)
            {
                id = uiAction - GOSSIP_ACTION_APPLY_ALL_BASE;
                templateEntry = FindTemplateById(id);
                if (templateEntry)
                {
                    sTemplateNpcMgr->ApplyTemplate(player, templateEntry);
                    CloseGossipMenuFor(player);
                    return true;
                }
            }

            if (uiAction >= GOSSIP_ACTION_APPLY_GEAR_BASE && uiAction < GOSSIP_ACTION_APPLY_TALENTS_BASE)
            {
                id = uiAction - GOSSIP_ACTION_APPLY_GEAR_BASE;
                templateEntry = FindTemplateById(id);
                if (!templateEntry) { CloseGossipMenuFor(player); return false; }
                // Gear-only apply using manager helper to avoid duplication of logic
                if (!sTemplateNpcMgr->PreApplyChecks(player, templateEntry, TEMPLATE_APPLY_GEAR))
                {
                    CloseGossipMenuFor(player);
                    return true;
                }
                sTemplateNpcMgr->ApplyGearPart(player, templateEntry);
                sTemplateNpcMgr->PostApplyUpdates(player, templateEntry, TEMPLATE_APPLY_GEAR);
                CloseGossipMenuFor(player);
                return true;
            }

            if (uiAction >= GOSSIP_ACTION_APPLY_TALENTS_BASE && uiAction < GOSSIP_ACTION_APPLY_GLYPHS_BASE)
            {
                id = uiAction - GOSSIP_ACTION_APPLY_TALENTS_BASE;
                templateEntry = FindTemplateById(id);
                if (!templateEntry) { CloseGossipMenuFor(player); return false; }
                // Talent-only apply using manager helper
                if (!sTemplateNpcMgr->PreApplyChecks(player, templateEntry, TEMPLATE_APPLY_TALENTS))
                {
                    CloseGossipMenuFor(player);
                    return true;
                }
                if (templateEntry->talentKitId) sTemplateNpcMgr->ApplyTalentsPart(player, templateEntry);
                sTemplateNpcMgr->PostApplyUpdates(player, templateEntry, TEMPLATE_APPLY_TALENTS);
                CloseGossipMenuFor(player);
                return true;
            }

            if (uiAction >= GOSSIP_ACTION_APPLY_GLYPHS_BASE && uiAction < GOSSIP_ACTION_APPLY_GLYPHS_BASE + 0x100000)
            {
                id = uiAction - GOSSIP_ACTION_APPLY_GLYPHS_BASE;
                templateEntry = FindTemplateById(id);
                if (!templateEntry) { CloseGossipMenuFor(player); return false; }
                if (!sTemplateNpcMgr->PreApplyChecks(player, templateEntry, TEMPLATE_APPLY_GLYPHS))
                {
                    CloseGossipMenuFor(player);
                    return true;
                }
                if (templateEntry->glyphKitId) sTemplateNpcMgr->ApplyGlyphsPart(player, templateEntry);
                sTemplateNpcMgr->PostApplyUpdates(player, templateEntry, TEMPLATE_APPLY_GLYPHS);
                CloseGossipMenuFor(player);
                return true;
            }
        }

        // Other actions: resets etc.
        switch (uiAction)
        {
            case GOSSIP_ACTION_SPACER:
                // return to main menu
                OnGossipHello(player, creature);
                break;

            case GOSSIP_ACTION_RESET_REMOVE_GLYPHS:
                RemoveAllGlyphs(player);
                player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, SUCCESS_REMOVED_GLYPHS)->c_str());
                CloseGossipMenuFor(player);
                break;

            case GOSSIP_ACTION_RESET_TALENTS:
                player->resetTalents(true);
                player->SendTalentsInfoData(false);
                player->GetSession()->SendAreaTriggerMessage(LANG_RESET_TALENTS);
                CloseGossipMenuFor(player);
                break;

            case GOSSIP_ACTION_RESET_PET_TALENTS:
                player->ResetPetTalents();
                player->GetSession()->SendAreaTriggerMessage(LANG_RESET_PET_TALENTS);
                CloseGossipMenuFor(player);
                break;

            case GOSSIP_ACTION_RESET_REMOVE_EQUIPPED_GEAR:
                for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
                {
                    if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                        item->SetState(ITEM_CHANGED, player);
                    player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                }
                player->SaveToDB(false, false);
                player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, SUCCESS_DESTROYED_EQUIPPED_GEAR)->c_str());
                CloseGossipMenuFor(player);
                break;

            default:
                CloseGossipMenuFor(player);
                break;
        }

        player->UpdateSkillsForLevel();

        return true;
    }
};

using namespace Acore::ChatCommands;
class npc_talent_template_command : public CommandScript
{
public:
    npc_talent_template_command() : CommandScript("npc_talent_template_command") {}

    [[nodiscard]] ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable kitCommandTable =
        {
            { "create", HandleCreateKitCommand, SEC_ADMINISTRATOR, Console::No },
            { "delete", HandleDeleteKitCommand, SEC_ADMINISTRATOR, Console::No },
            { "list", HandleListKitCommand, SEC_ADMINISTRATOR, Console::No },
        };

        static ChatCommandTable templateCommandTable =
        {
            { "reload", HandleReloadTemplateNPCCommand, SEC_ADMINISTRATOR, Console::No },
            { "kit",  kitCommandTable },
        };

        static ChatCommandTable commandTable =
        {
            { "templatenpc",  templateCommandTable},
        };

        return commandTable;
    }

    // Legacy 'create' command removed: use 'templatenpc kit create' instead

    static bool HandleCreateKitCommand(ChatHandler *handler, std::string_view args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        std::string sArgs = std::string(args);
        std::istringstream iss(sArgs);
        std::string kitType;
        if (!(iss >> kitType))
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::USAGE_KIT_CREATE);
            return false;
        }

        auto trimQuotes = [](std::string& s)
        {
            if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')))
            {
                s = s.substr(1, s.size() - 2);
            }
        };

        std::string token;
        uint32 kitId = 0;
        std::string name;
        if (!(iss >> token))
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::USAGE_KIT_CREATE);
            return false;
        }
        // token may be kitId or start of name
        bool isNumber = true;
        for (char c : token) if (!isdigit((unsigned char)c)) { isNumber = false; break; }
        if (isNumber)
        {
                try { kitId = static_cast<uint32>(std::stoul(token)); } catch(...) { handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_INVALID_KITID); return false; }
            std::getline(iss, name);
            if (!name.empty() && name.front() == ' ') name.erase(0, 1);
        }
        else
        {
            // token starts the name
            std::string rest;
            std::getline(iss, rest);
            name = token + rest;
        }
        if (name.empty())
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::USAGE_KIT_CREATE);
            return false;
        }
        trimQuotes(name);

        // If an explicit kitId was provided we will overwrite its entries; otherwise auto-assign a new kit id

        // Save player before taking items/talents/glyphs
        player->SaveToDB(false, false);

        if (kitType == "gear")
        {
            if (kitId != 0)
            {
                sTemplateNpcMgr->OverwriteKit(kitId);
                sTemplateNpcMgr->ExtractGearKitToDB(player, kitId);
                kitId = sTemplateNpcMgr->InsertKitEntryToDB(player, kitId, "gear", name);
            }
            else
            {
                uint32 newId = sTemplateNpcMgr->InsertKitEntryToDB(player, 0, "gear", name);
                if (newId == 0)
                {
                    handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_FAILED_CREATE_KIT);
                    return false;
                }
                sTemplateNpcMgr->ExtractGearKitToDB(player, newId);
                kitId = newId;
            }
            sTemplateNpcMgr->LoadKits();
            if (auto fmtStr = handler->GetModuleString(MODULE_STRING, Strings::SUCCESS_KIT_CREATED))
                handler->PSendSysMessage(fmt::format(*fmtStr, kitId, "gear", name).c_str());
        }
        else if (kitType == "talent" || kitType == "talents")
        {
            if (kitId != 0)
            {
                sTemplateNpcMgr->OverwriteKit(kitId);
                sTemplateNpcMgr->ExtractTalentKitToDB(player, kitId);
                kitId = sTemplateNpcMgr->InsertKitEntryToDB(player, kitId, "talent", name);
            }
            else
            {
                uint32 newId = sTemplateNpcMgr->InsertKitEntryToDB(player, 0, "talent", name);
                if (newId == 0)
                {
                    handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_FAILED_CREATE_KIT);
                    return false;
                }
                sTemplateNpcMgr->ExtractTalentKitToDB(player, newId);
                kitId = newId;
            }
            sTemplateNpcMgr->LoadKits();
            if (auto fmtStr = handler->GetModuleString(MODULE_STRING, Strings::SUCCESS_KIT_CREATED))
                handler->PSendSysMessage(fmt::format(*fmtStr, kitId, "talent", name).c_str());
        }
        else if (kitType == "glyph" || kitType == "glyphs")
        {
            if (kitId != 0)
            {
                sTemplateNpcMgr->OverwriteKit(kitId);
                sTemplateNpcMgr->ExtractGlyphKitToDB(player, kitId);
                kitId = sTemplateNpcMgr->InsertKitEntryToDB(player, kitId, "glyph", name);
            }
            else
            {
                uint32 newId = sTemplateNpcMgr->InsertKitEntryToDB(player, 0, "glyph", name);
                if (newId == 0)
                {
                    handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_FAILED_CREATE_KIT);
                    return false;
                }
                sTemplateNpcMgr->ExtractGlyphKitToDB(player, newId);
                kitId = newId;
            }
            sTemplateNpcMgr->LoadKits();
            if (auto fmtStr = handler->GetModuleString(MODULE_STRING, Strings::SUCCESS_KIT_CREATED))
                handler->PSendSysMessage(fmt::format(*fmtStr, kitId, "glyph", name).c_str());
        }
        else
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_INVALID_KIT_TYPE);
            return false;
        }

        player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetModuleString(MODULE_STRING, Strings::SUCCESS_EXTRACT)->c_str());
        ChatHandler(player->GetSession()).PSendModuleSysMessage(MODULE_STRING, Strings::SUCCESS_EXTRACT_INFO);
        return true;
    }

    static bool HandleDeleteKitCommand(ChatHandler *handler, std::string_view args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;
        std::string kitIdStr = std::string(args);
        if (kitIdStr.empty())
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::USAGE_KIT_DELETE);
            return false;
        }
        uint32 kitId = 0;
        try { kitId = static_cast<uint32>(std::stoul(kitIdStr)); } catch(...) { handler->PSendModuleSysMessage(MODULE_STRING, Strings::ERROR_INVALID_KITID); return false; }
        sTemplateNpcMgr->OverwriteKit(kitId);
        handler->PSendModuleSysMessage(MODULE_STRING, Strings::SUCCESS_KIT_DELETED);
        return true;
    }

    static bool HandleListKitCommand(ChatHandler *handler, std::string_view args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        std::string sArgs = std::string(args);
        std::istringstream iss(sArgs);
        std::string filter1;
        std::string filter2;
        if (!(iss >> filter1))
        {
            filter1.clear();
        }
        if (iss >> filter2)
        {
            // keep filter2 if present
        }

        // build query
        std::string query = "SELECT `id`, `kit_type`, `playerClass`, `name` FROM `mod_npc_talent_template_kits`";
        bool hasWhere = false;
        if (!filter1.empty())
        {
            // decide whether filter1 is a type or a class
            if (filter1 == "gear" || filter1 == "talent" || filter1 == "glyph")
            {
                query += " WHERE `kit_type`='" + filter1 + "'";
                hasWhere = true;
            }
            else
            {
                query += " WHERE `playerClass`='" + filter1 + "'";
                hasWhere = true;
            }
        }
        if (!filter2.empty())
        {
            if (hasWhere)
                query += " AND `playerClass`='" + filter2 + "'";
            else
                query += " WHERE `playerClass`='" + filter2 + "'";
        }
        query += " ORDER BY `kit_type`, `playerClass`, `id`;";

        QueryResult result = CharacterDatabase.Query(query);
        if (!result)
        {
            handler->PSendModuleSysMessage(MODULE_STRING, Strings::FEEDBACK_NO_KITS);
            return true;
        }
        handler->PSendModuleSysMessage(MODULE_STRING, Strings::FEEDBACK_KITS_HEADER);
        do
        {
            Field* f = result->Fetch();
            uint32 id = f[0].Get<uint32>();
            std::string type = f[1].Get<std::string>();
            std::string pclass = f[2].Get<std::string>();
            std::string name = f[3].Get<std::string>();
            if (auto fmtStr = handler->GetModuleString(MODULE_STRING, Strings::FEEDBACK_KIT_ROW))
                handler->PSendSysMessage(fmt::format(*fmtStr, id, type, pclass, name).c_str());
        } while (result->NextRow());

        return true;
    }

    static bool HandleReloadTemplateNPCCommand(ChatHandler *handler)
    {
        LOG_INFO("module", "Reloading templates for Template NPC table...");
            sTemplateNpcMgr->LoadKits();
            sTemplateNpcMgr->LoadTemplates();
        handler->SendGlobalGMSysMessage(handler->GetModuleString(MODULE_STRING, NPCTalentTemplate::Strings::SUCCESS_RELOADED)->c_str());
        return true;
    }
};

class npc_talent_template_world : public WorldScript
{
public:
    npc_talent_template_world() : WorldScript("npc_talent_template_world", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sTemplateNpcMgr->enableResetTalents = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableResetTalents", true);
        sTemplateNpcMgr->enableRemoveAllGlyphs = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableRemoveAllGlyphs", true);
        sTemplateNpcMgr->enableDestroyEquippedGear = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableDestroyEquippedGear", true);
        sTemplateNpcMgr->allianceMount = sConfigMgr->GetOption<uint32>("NpcTalentTemplate.AllianceMount", SPELL_BIG_BATTLE_BEAR);
        sTemplateNpcMgr->hordeMount = sConfigMgr->GetOption<uint32>("NpcTalentTemplate.HordeMount", SPELL_BIG_BATTLE_BEAR);
    }

    void OnStartup() override
    {
        LOG_INFO("module", "== START NPC TALENT TEMPLATE ==");

        LOG_INFO("module", "Loading Template Kits and Templates...");
        sTemplateNpcMgr->LoadKits();
        sTemplateNpcMgr->LoadTemplates();

        LOG_INFO("module", "== END NPC TALENT TEMPLATE ==");
    }
};

void AddSC_npc_talent_template()
{
    new npc_talent_template();
    new npc_talent_template_command();
    new npc_talent_template_world();
}
