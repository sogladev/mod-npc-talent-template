#include "TemplateNPC.h"
#include "Config.h"
#include "ScriptedGossip.h"
#include "Tokenize.h"
#include "Chat.h"

enum GossipActions
{
    GOSSIP_ACTION_SPACER = 5000, // ---------
    GOSSIP_ACTION_RESET_TALENTS = 5001,
    GOSSIP_ACTION_RESET_PET_TALENTS = 5002,
    GOSSIP_ACTION_RESET_REMOVE_GLYPHS = 5003,
    GOSSIP_ACTION_RESET_REMOVE_EQUIPPED_GEAR = 5004,
};

void sTemplateNPC::LearnPlateMailSpells(Player *player)
{
    switch (player->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
    case CLASS_DEATH_KNIGHT:
        player->learnSpell(PLATE_MAIL);
        break;
    case CLASS_SHAMAN:
    case CLASS_HUNTER:
        player->learnSpell(MAIL);
        break;
    default:
        break;
    }
}

void sTemplateNPC::ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
{
    if (!item)
        return;

    if (!bonusEntry || bonusEntry == 0)
        return;

    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, bonusEntry, 0, 0);
    player->ApplyEnchantment(item, slot, true);
}

void sTemplateNPC::ApplyGlyph(Player* player, uint8 slot, uint32 glyphID)
{
    if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyphID))
    {
        if (uint32 oldGlyph = player->GetGlyph(slot))
        {
            player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(oldGlyph)->SpellId);
            player->SetGlyph(slot, 0, true);
        }
        player->CastSpell(player, gp->SpellId, true);
        player->SetGlyph(slot, glyphID, true);
    }
}

void sTemplateNPC::RemoveAllGlyphs(Player* player)
{
    for (uint8 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
    {
        if (uint32 glyph = player->GetGlyph(i))
        {
            if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))
            {
                if (GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(player->GetGlyphSlot(i)))
                {
                    player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(glyph)->SpellId);
                    player->SetGlyph(i, 0, true);
                    player->SendTalentsInfoData(false); // this is somewhat an in-game glyph realtime update (apply/remove)
                }
            }
        }
    }
}

void sTemplateNPC::LearnTemplateTalents(Player* player)
{
    for (auto const& talentTemplate : talentContainer)
    {
        if (talentTemplate->playerClass == GetClassString(player).c_str() && talentTemplate->playerSpec == sTalentsSpec)
        {
            player->learnSpellHighRank(talentTemplate->talentId);
            player->addTalent(talentTemplate->talentId, player->GetActiveSpecMask(), 0);
        }
    }
    player->InitTalentForLevel();
}

void sTemplateNPC::LearnTemplateGlyphs(Player* player)
{
    for (auto const& glyphTemplate : glyphContainer)
    {
        if (glyphTemplate->playerClass == GetClassString(player).c_str() && glyphTemplate->playerSpec == sTalentsSpec)
        {
            ApplyGlyph(player, glyphTemplate->slot, glyphTemplate->glyph);
        }
    }
    player->SendTalentsInfoData(false);
}

void sTemplateNPC::EquipTemplateGear(Player* player)
{
    // Reverse sort so we equip items from trinket to helm so we avoid issue with meta gems
    std::ranges::sort(gearContainer, std::greater<>());

    for (auto const& gearTemplate : gearContainer)
    {
        if (gearTemplate->playerClass == GetClassString(player).c_str() &&
            gearTemplate->playerSpec == sTalentsSpec &&
            gearTemplate->playerRaceMask & player->getRaceMask())
        {
            // Equip the item and apply enchants and gems
            if (Item* item = player->EquipNewItem(gearTemplate->pos, gearTemplate->itemEntry, true))
            {
                ApplyBonus(player, item, PERM_ENCHANTMENT_SLOT, gearTemplate->enchant);
                ApplyBonus(player, item, BONUS_ENCHANTMENT_SLOT, gearTemplate->bonusEnchant);
                ApplyBonus(player, item, PRISMATIC_ENCHANTMENT_SLOT, gearTemplate->prismaticEnchant);
                ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_2, gearTemplate->socket2);
                ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT_3, gearTemplate->socket3);
                ApplyBonus(player, item, SOCK_ENCHANTMENT_SLOT, gearTemplate->socket1);
            }
        }
    }
}

void sTemplateNPC::LoadTalentsContainer()
{
    for (auto* talent : talentContainer)
        delete talent;
    talentContainer.clear();

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query("SELECT `playerClass`, `playerSpec`, `talentId` FROM `template_npc_talents`");

    if (!result)
    {
        LOG_WARN("sql.sql", ">> TEMPLATE NPC: Loaded 0 talent templates. DB table `template_npc_talents` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        TalentTemplate *pTalent = new TalentTemplate;

        pTalent->playerClass = fields[0].Get<std::string>();
        pTalent->playerSpec = fields[1].Get<std::string>();
        pTalent->talentId = fields[2].Get<uint32>();

        talentContainer.push_back(pTalent);
        ++count;
    } while (result->NextRow());
    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} talent templates in {} ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadGlyphsContainer()
{
    for (auto* glyph : glyphContainer)
        delete glyph;
    glyphContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT `playerClass`, `playerSpec`, `slot`, `glyph` FROM `template_npc_glyphs`");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        LOG_WARN("sql.sql", ">> TEMPLATE NPC: Loaded 0 glyph templates. DB table `template_npc_glyphs` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        GlyphTemplate* glyph = new GlyphTemplate;

        glyph->playerClass = fields[0].Get<std::string>();
        glyph->playerSpec = fields[1].Get<std::string>();
        glyph->slot = fields[2].Get<uint8>();
        glyph->glyph = fields[3].Get<uint32>();

        glyphContainer.push_back(glyph);
        ++count;
    } while (result->NextRow());

    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} glyph templates in {} ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadGearContainer()
{
    for (auto* gear : gearContainer)
        delete gear;
    gearContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT `playerClass`, `playerSpec`, `playerRaceMask`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant` FROM `template_npc_gear`");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        LOG_INFO("module", ">> TEMPLATE NPC: Loaded 0 gear templates. DB table `template_npc_gear` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        GearTemplate* item = new GearTemplate;

        item->playerClass = fields[0].Get<std::string>();
        item->playerSpec = fields[1].Get<std::string>();
        item->playerRaceMask = fields[2].Get<uint32>();
        item->pos = fields[3].Get<uint8>();
        item->itemEntry = fields[4].Get<uint32>();
        item->enchant = fields[5].Get<uint32>();
        item->socket1 = fields[6].Get<uint32>();
        item->socket2 = fields[7].Get<uint32>();
        item->socket3 = fields[8].Get<uint32>();
        item->bonusEnchant = fields[9].Get<uint32>();
        item->prismaticEnchant = fields[10].Get<uint32>();

        gearContainer.push_back(item);
        ++count;
    } while (result->NextRow());
    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} gear templates in {} ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void sTemplateNPC::LoadIndexContainer()
{
    for (auto* index : indexContainer)
        delete index;
    indexContainer.clear();

    QueryResult result = CharacterDatabase.Query("SELECT `playerClass`, `playerSpec`, `gossipAction`, `gossipText`, `gearMask`, `minLevel`, `maxLevel` FROM `template_npc_index` ORDER BY `gossipAction`;");

    uint32 oldMSTime = getMSTime();
    uint32 count = 0;

    if (!result)
    {
        LOG_INFO("module", ">> TEMPLATE NPC: Loaded 0 index templates. DB table `template_npc_index` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        IndexTemplate* pIndex = new IndexTemplate;

        pIndex->playerClass = fields[0].Get<std::string>();
        pIndex->playerSpec = fields[1].Get<std::string>();
        pIndex->gossipAction = fields[2].Get<uint32>();
        pIndex->gossipText = fields[3].Get<std::string>();
        pIndex->gearMask = static_cast<TemplateFlag>(fields[4].Get<uint32>());
        pIndex->minLevel = fields[5].Get<uint32>();
        pIndex->maxLevel = fields[6].Get<uint32>();

        indexContainer.push_back(pIndex);
        ++count;
    } while (result->NextRow());
    LOG_INFO("module", ">> TEMPLATE NPC: Loaded {} index templates in {} ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

std::string sTemplateNPC::GetClassString(Player* player)
{
    return EnumUtils::ToTitle(Classes(player->getClass()));
}

bool sTemplateNPC::OverwriteTemplate(Player* player, std::string& playerSpecStr)
{
    // Delete old talent, glyph, and gear ,templates before extracting new ones
    CharacterDatabase.Execute("DELETE FROM `template_npc_talents` WHERE `playerClass`='{}' AND `playerSpec`='{}'", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.Execute("DELETE FROM `template_npc_glyphs` WHERE `playerClass`='{}' AND `playerSpec`='{}'", GetClassString(player).c_str(), playerSpecStr.c_str());
    CharacterDatabase.Execute("DELETE FROM `template_npc_gear` WHERE `playerClass`='{}' AND `playerSpec`='{}' AND `playerRaceMask` & {}", GetClassString(player).c_str(), playerSpecStr.c_str(), player->getRaceMask());
    CharacterDatabase.Execute("DELETE FROM `template_npc_index` WHERE `playerClass`='{}' AND `playerSpec`='{}'", GetClassString(player).c_str(), playerSpecStr.c_str());
    return false;
}

void sTemplateNPC::ExtractGearTemplateToDB(Player* player, std::string& playerSpecStr)
{
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* equippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (equippedItem)
        {
            CharacterDatabase.Execute("INSERT INTO `template_npc_gear` (`playerClass`, `playerSpec`, `playerRaceMask`, `pos`, `itemEntry`, `enchant`, `socket1`, `socket2`, `socket3`, `bonusEnchant`, `prismaticEnchant`) VALUES ('{}', '{}', {}, {}, {}, {}, {}, {}, {}, {}, {});", GetClassString(player).c_str(), playerSpecStr.c_str(), player->getRaceMask(), equippedItem->GetSlot(), equippedItem->GetEntry(), equippedItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2), equippedItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3), equippedItem->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT), equippedItem->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
        }
    }
}

void sTemplateNPC::InsertIndexEntryToDB(Player* player, std::string& playerSpecStr)
{
    CharacterDatabase.Execute(
        "INSERT INTO `template_npc_index` (`playerClass`, `playerSpec`, `gossipAction`, `gossipText`, `gearMask`, `minLevel`, `maxLevel`) VALUES ('{}', '{}', {}, '{}', {}, {}, {});",
        GetClassString(player).c_str(), playerSpecStr.c_str(), 9999, "|cff00ff00|TInterface\\\\icons\\\\Trade_Engineering:30:30|t|r Update this gossip text in the index!", 1, player->GetLevel(), player->GetLevel()
    );
}

void sTemplateNPC::ExtractTalentTemplateToDB(Player* player, std::string& playerSpecStr)
{
    QueryResult result = CharacterDatabase.Query("SELECT `spell` FROM `character_talent` WHERE `guid`={} AND `specMask`&{}", player->GetGUID().GetCounter(), player->GetActiveSpecMask());

    if (!result)
    {
        return;
    }
    else if (player->GetFreeTalentPoints() > 0)
    {
        player->GetSession()->SendAreaTriggerMessage("You have unspent talent points. Please spend all your talent points and re-extract the template.");
        return;
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 spell = fields[0].Get<uint32>();

            CharacterDatabase.Execute("INSERT INTO `template_npc_talents` (`playerClass`, `playerSpec`, `talentId`) VALUES ('{}', '{}', {})", GetClassString(player).c_str(), playerSpecStr.c_str(), spell);
        } while (result->NextRow());
    }
}

void sTemplateNPC::ExtractGlyphsTemplateToDB(Player* player, std::string& playerSpecStr)
{
    QueryResult result = CharacterDatabase.Query("SELECT `glyph1`, `glyph2`, `glyph3`, `glyph4`, `glyph5`, `glyph6` FROM `character_glyphs` WHERE `guid`={} AND `talentGroup`={}", player->GetGUID().GetCounter(), player->GetActiveSpec());

    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
    {
        if (!result)
        {
            player->GetSession()->SendAreaTriggerMessage("Get glyphs and re-extract the template!");
            continue;
        }

        Field* fields = result->Fetch();
        uint32 glyph1 = fields[0].Get<uint32>();
        uint32 glyph2 = fields[1].Get<uint32>();
        uint32 glyph3 = fields[2].Get<uint32>();
        uint32 glyph4 = fields[3].Get<uint32>();
        uint32 glyph5 = fields[4].Get<uint32>();
        uint32 glyph6 = fields[5].Get<uint32>();

        uint32 storedGlyph;

        switch (slot)
        {
        case 0:
            storedGlyph = glyph1;
            break;
        case 1:
            storedGlyph = glyph2;
            break;
        case 2:
            storedGlyph = glyph3;
            break;
        case 3:
            storedGlyph = glyph4;
            break;
        case 4:
            storedGlyph = glyph5;
            break;
        case 5:
            storedGlyph = glyph6;
            break;
        default:
            break;
        }

        CharacterDatabase.Execute("INSERT INTO `template_npc_glyphs` (`playerClass`, `playerSpec`, `slot`, `glyph`) VALUES ('{}', '{}', {}, {});", GetClassString(player).c_str(), playerSpecStr.c_str(), slot, storedGlyph);
    }
}

bool sTemplateNPC::CanEquipTemplate(Player* player, std::string& playerSpecStr)
{
    QueryResult result = CharacterDatabase.Query("SELECT `playerClass`, `playerSpec`, `playerRaceMask` FROM `template_npc_gear` WHERE `playerClass`='{}' AND `playerSpec`='{}' AND `playerRaceMask`&{}", GetClassString(player).c_str(), playerSpecStr.c_str(), player->getRaceMask());

    if (!result)
        return false;
    return true;
}

class TemplateNPC : public CreatureScript
{
public:
    TemplateNPC() : CreatureScript("TemplateNPC") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {
        for (auto const& indexTemplate : sTemplateNpcMgr->indexContainer)
        {
            if (indexTemplate->playerClass == sTemplateNpcMgr->GetClassString(player).c_str() &&
                (indexTemplate->minLevel <= player->GetLevel() && player->GetLevel() <= indexTemplate->maxLevel))
            {
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, indexTemplate->gossipText, GOSSIP_SENDER_MAIN, indexTemplate->gossipAction);
            }
        }
        // Extra gossip
        if (sTemplateNpcMgr->enableResetTalents || sTemplateNpcMgr->enableRemoveAllGlyphs || sTemplateNpcMgr->enableDestroyEquippedGear)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "----------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SPACER);
        if (sTemplateNpcMgr->enableResetTalents)
        {
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\Trade_Engineering:30:30|t|r Reset Talents", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_TALENTS);
            if (player->getClass() == CLASS_HUNTER)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_hunter_beasttaming:30:30|t|r Reset Pet Talents", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PET_TALENTS);
        }
        if (sTemplateNpcMgr->enableRemoveAllGlyphs)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\Spell_ChargeNegative:30|t|r Remove all glyphs", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_REMOVE_GLYPHS);
        if (sTemplateNpcMgr->enableDestroyEquippedGear)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_vehicle_launchplayer:30|t|r Destroy my equipped gear", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_REMOVE_EQUIPPED_GEAR);
        SendGossipMenuFor(player, creature->GetEntry(), creature->GetGUID());
        return true;
    }

    static void EquipFullTemplateGear(Player* player, std::string& playerSpecStr) // Merge
    {
        if (sTemplateNpcMgr->CanEquipTemplate(player, playerSpecStr) == false)
        {
            player->GetSession()->SendAreaTriggerMessage("There's no templates for %s specialization yet.", playerSpecStr.c_str());
            return;
        }

        // Don't let players to use Template feature while wearing some gear
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                if (haveItemEquipped)
                {
                    player->GetSession()->SendAreaTriggerMessage("You need to remove all your equipped items in order to use this feature!");
                    CloseGossipMenuFor(player);
                    return;
                }
            }
        }

        // Don't let players to use Template feature after spending some talent points
        if (player->GetFreeTalentPoints() < 71)
        {
            player->GetSession()->SendAreaTriggerMessage("You have already spent some talent points. You need to reset your talents first!");
            CloseGossipMenuFor(player);
            return;
        }

        player->_RemoveAllItemMods();
        sTemplateNpcMgr->LearnTemplateTalents(player);
        sTemplateNpcMgr->LearnTemplateGlyphs(player);
        sTemplateNpcMgr->EquipTemplateGear(player);
        sTemplateNpcMgr->LearnPlateMailSpells(player);

        // update warr talent
        player->UpdateTitansGrip();

        LearnWeaponSkills(player);

        player->GetSession()->SendAreaTriggerMessage("Successfully equipped %s %s template!", playerSpecStr.c_str(), sTemplateNpcMgr->GetClassString(player).c_str());

        if (player->getPowerType() == POWER_MANA)
            player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

        player->SetHealth(player->GetMaxHealth());

        // Learn Riding/Flying
        if (player->HasSpell(SPELL_Artisan_Riding) || player->HasSpell(SPELL_Cold_Weather_Flying) || player->HasSpell(SPELL_Amani_War_Bear) || player->HasSpell(SPELL_Teach_Learn_Talent_Specialization_Switches) || player->HasSpell(SPELL_Learn_a_Second_Talent_Specialization))
            return;

        // Cast spells that teach dual spec
        // Both are also ImplicitTarget self and must be cast by player
        player->CastSpell(player, SPELL_Teach_Learn_Talent_Specialization_Switches, player->GetGUID());
        player->CastSpell(player, SPELL_Learn_a_Second_Talent_Specialization, player->GetGUID());

        player->learnSpell(SPELL_Artisan_Riding);
        player->learnSpell(SPELL_Cold_Weather_Flying);
        player->learnSpell(SPELL_Amani_War_Bear);
    }

    static void LearnOnlyTalentsAndGlyphs(Player* player, std::string& playerSpecStr) // Merge
    {
        if (sTemplateNpcMgr->CanEquipTemplate(player, playerSpecStr) == false)
        {
            player->GetSession()->SendAreaTriggerMessage("There's no templates for %s specialization yet.", playerSpecStr.c_str());
            return;
        }

        // Don't let players to use Template feature after spending some talent points
        if (player->GetFreeTalentPoints() < 71)
        {
            player->GetSession()->SendAreaTriggerMessage("You have already spent some talent points. You need to reset your talents first!");
            CloseGossipMenuFor(player);
            return;
        }

        sTemplateNpcMgr->LearnTemplateTalents(player);
        sTemplateNpcMgr->LearnTemplateGlyphs(player);
        sTemplateNpcMgr->LearnPlateMailSpells(player);

        LearnWeaponSkills(player);

        player->GetSession()->SendAreaTriggerMessage("Successfuly learned talent spec %s!", playerSpecStr.c_str());

        // Learn Riding/Flying
        if (player->HasSpell(SPELL_Artisan_Riding) ||
            player->HasSpell(SPELL_Cold_Weather_Flying) ||
            player->HasSpell(SPELL_Amani_War_Bear) ||
            player->HasSpell(SPELL_Teach_Learn_Talent_Specialization_Switches) || player->HasSpell(SPELL_Learn_a_Second_Talent_Specialization))
            return;

        // Cast spells that teach dual spec
        // Both are also ImplicitTarget self and must be cast by player
        player->CastSpell(player, SPELL_Teach_Learn_Talent_Specialization_Switches, player->GetGUID());
        player->CastSpell(player, SPELL_Learn_a_Second_Talent_Specialization, player->GetGUID());

        player->learnSpell(SPELL_Artisan_Riding);
        player->learnSpell(SPELL_Cold_Weather_Flying);
        player->learnSpell(SPELL_Amani_War_Bear);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();

        if (!player || !creature)
            return false;

        for (auto const& indexTemplate : sTemplateNpcMgr->indexContainer)
        {
            if (indexTemplate->gossipAction == uiAction)
            {
                sTemplateNpcMgr->sTalentsSpec = indexTemplate->playerSpec;
                if (indexTemplate->gearMask == TEMPLATE_APPLY_GEAR_AND_TALENTS)
                    EquipFullTemplateGear(player, sTemplateNpcMgr->sTalentsSpec);
                else if (indexTemplate->gearMask == TEMPLATE_APPLY_TALENTS)
                    LearnOnlyTalentsAndGlyphs(player, sTemplateNpcMgr->sTalentsSpec);
                CloseGossipMenuFor(player);
                break;
            }
        }

        // Extra gossip
        switch (uiAction)
        {
            case GOSSIP_ACTION_SPACER:
                // return to OnGossipHello menu, otherwise it will freeze every menu
                OnGossipHello(player, creature);
                break;

            case GOSSIP_ACTION_RESET_REMOVE_GLYPHS:
                sTemplateNpcMgr->RemoveAllGlyphs(player);
                player->GetSession()->SendAreaTriggerMessage("Your glyphs have been removed.");
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
                    player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                }
                player->SaveToDB(false, false);
                player->GetSession()->SendAreaTriggerMessage("Your equipped gear has been destroyed.");
                CloseGossipMenuFor(player);
                break;

            default:
                break;
        }

        player->UpdateSkillsForLevel();

        return true;
    }
};

using namespace Acore::ChatCommands;
class TemplateNPC_command : public CommandScript
{
public:
    TemplateNPC_command() : CommandScript("TemplateNPC_command") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable templateCommandTable =
        {
            { "reload", HandleReloadTemplateNPCCommand, SEC_ADMINISTRATOR, Console::No },
            { "create", HandleCreateClassSpecItemSetCommand, SEC_ADMINISTRATOR, Console::No },
            // { "copy", HandleCopyCommand, SEC_ADMINISTRATOR, Console::No },
            // {"copy", SEC_ADMINISTRATOR, false, &HandleCopyCommand, "Copies your target's gear onto your character. example: `.template copy`"},
        };

        static ChatCommandTable commandTable =
        {
            { "templatenpc",  templateCommandTable},
        };

        return commandTable;
    }

	static bool HandleCreateClassSpecItemSetCommand(ChatHandler *handler, std::string_view name)
    {
		Player* player = handler->GetSession()->GetPlayer();
        player->SaveToDB(false, false);
		sTemplateNpcMgr->sTalentsSpec = name;
		sTemplateNpcMgr->OverwriteTemplate(player, sTemplateNpcMgr->sTalentsSpec);
		sTemplateNpcMgr->ExtractGearTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
		sTemplateNpcMgr->ExtractTalentTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
		sTemplateNpcMgr->ExtractGlyphsTemplateToDB(player, sTemplateNpcMgr->sTalentsSpec);
		sTemplateNpcMgr->InsertIndexEntryToDB(player, sTemplateNpcMgr->sTalentsSpec);
        player->GetSession()->SendAreaTriggerMessage("Template successfully created!");
        ChatHandler(player->GetSession()).PSendSysMessage("Template skeleton for \"{}\" successfully created! You can `.templatenpc reload` to test your template. WARNING: Templates should be exported and edited to `.sql`. See documentation for more info.", name);
		return true;
	}

    static bool HandleReloadTemplateNPCCommand(ChatHandler *handler)
    {
        LOG_INFO("module", "Reloading templates for Template NPC table...");
        sTemplateNpcMgr->LoadTalentsContainer();
        sTemplateNpcMgr->LoadGlyphsContainer();
        sTemplateNpcMgr->LoadGearContainer();
        sTemplateNpcMgr->LoadIndexContainer();
        handler->SendGlobalGMSysMessage("Template NPC templates reloaded.");
        return true;
    }
};

class TemplateNPC_World : public WorldScript
{
public:
    TemplateNPC_World() : WorldScript("TemplateNPC_World") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sTemplateNpcMgr->enableResetTalents = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableResetTalents", false);
        sTemplateNpcMgr->enableRemoveAllGlyphs = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableRemoveAllGlyphs", true);
        sTemplateNpcMgr->enableDestroyEquippedGear = sConfigMgr->GetOption<bool>("NpcTalentTemplate.EnableDestroyEquippedGear", true);
    }

    void OnStartup() override
    {
        // Load templates for Template NPC #1
        LOG_INFO("module", "== TEMPLATE NPC ==");
        LOG_INFO("module", "Loading Template Talents...");
        sTemplateNpcMgr->LoadTalentsContainer();

        // Load templates for Template NPC #2
        LOG_INFO("module", "Loading Template Glyphs...");
        sTemplateNpcMgr->LoadGlyphsContainer();

        // Load templates for Template NPC #3
        LOG_INFO("module", "Loading Template Gear...");
        sTemplateNpcMgr->LoadGearContainer();

        // Load templates for Template NPC #4
        LOG_INFO("module", "Loading Template Index...");
        sTemplateNpcMgr->LoadIndexContainer();

        LOG_INFO("module", "== END TEMPLATE NPC ==");
    }
};

void AddSC_TemplateNPC()
{
    new TemplateNPC();
    new TemplateNPC_command();
    new TemplateNPC_World();
}
