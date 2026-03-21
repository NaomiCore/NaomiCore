# NaomiCore
World of Warcraft 3.3.5

----------------  Implemented in the kernel  ----------------
1. Expansion of Gossip Menu functionality (menu of dialogues with NPCs) in the NaomiCore core. It allows you not only to move through pages of text, but also to assign specific actions to dialog buttons directly through the database.

2. - Fixed a bug where conditions from the 'conditions' table were ignored for entries in 'spell_loot_template'.
- Added a call to the Condition Manager in the spell loot processing logic.
- Now loot from spells will correctly respect player level, quests, items, and other requirements set in the database.

3. - Fixed an issue where attack speed was not recalculated when a player's level increased.
- Previously, haste percentage acquired at low levels was incorrectly maintained after leveling up due to missing rating conversion updates.
- Added/Updated 'UpdateAttackSpeed()' call during the level-up process to ensure haste rating to percentage relationship is correctly applied for the new level.

4. Correction of the database - quests and Russian language
