# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

## mod-npc-talent-template

- Latest build status with azerothcore:

[![Build Status](https://github.com/azerothcore/mod-npc-talent-template/workflows/core-build/badge.svg?branch=master&event=push)](https://github.com/azerothcore/mod-npc-talent-template)

## Description
This mod introduces an NPC that allows players to instantly apply pre-configured character templates that gear up, gem, set talents, and apply glyphs for any class. Ideal for quick character optimization for various roles and scenarios.

* templates for level 80 PvP S6 and level 70 PvE T6
* race-specific gear options (e.g., humans receive no PvP trinket)
* administrator command for creating custom templates. Templates are managed directly through the database and config. No C++ edits needed

![](https://i.ibb.co/27WPR5j/Wo-WScrn-Shot-021219-000220.jpg)

Video Showcase:
https://streamable.com/yxv1m

## Creating custom gear sets
This process can only be done by an administrator and is error-prone. Creating gear sets should be done on a local development server and exporting sets from the DB to a `.sql` is recommended.

1. Setup a character with the desired gear set, gems, enchants, talents and glyphs.
2. `.templatenpc kit create <gear|talent|glyph> [kitId] <name>` creates a kit entry for the supplied kit type. The `name` is required and may contain spaces (wrap in quotes if your shell splits arguments). If `kitId` is omitted, an ID is auto-generated and printed back. Example: `.templatenpc kit create gear "Warrior Fury 80 PvP S6"` or `.templatenpc kit create gear 100024 "Warrior Fury 80 PvP S6"`.
3. `.templatenpc kit delete <kitId>` removes a kit from the DB for the given kit id.
4. `.templatenpc kit list [<gear|talent|glyph>] [<Class>]` lists all stored kits with their IDs and details; optional filters are kit type and class (e.g., `Gear` or `Warrior`).
3. export to `.sql`
4. some manual changes are needed

The following entries are created in the `acore_characters` db:
a. `mod_npc_talent_template_gear`: gear for your class, spec and race. When exporting, modify the raceMask as needed. When creating spec set for multiple races, create a template for one race, export to sql then create template for a new race. Modify the raceMask as needed
b. `mod_npc_talent_template_talents`: no modifying needed
c. `mod_npc_talent_template_glyphs`: no modifying needed
d. `mod_npc_talent_template_index`: modify gossipText, actionId needs to be unique per spec. Gossip options are displayed according to their ID with the lowest ID at the top. You can override a column to re-use talents or glyphs or gear from another template. For example, talents and glyphs from `Restoration80PvP` can be re-used

`.templatenpc reload` to reload changes. Test with dropping `acore_characters.mod_npc_talent_template*` tables and updates from character db as needed
```sql
DELETE FROM acore_characters.updates WHERE name='npc_talent_template_data_1_80_pvp_s6.sql';
DELETE FROM acore_characters.updates WHERE name='npc_talent_template_data_2_70_pve_t6.sql';
DELETE FROM acore_characters.updates WHERE name='npc_talent_template.sql';
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_index;
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_gear;
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_glyphs;
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_talents;
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_templates;
DROP TABLE IF EXISTS acore_characters.mod_npc_talent_template_kits;
```
