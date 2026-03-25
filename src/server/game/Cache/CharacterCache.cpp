/*
 * This file is part of the NaomiCore Project. Powered by GetRight 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CharacterCache.h"
#include "ArenaTeam.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "MiscPackets.h"
#include "Player.h"
#include "Timer.h"
#include "World.h"
#include "WorldPacket.h"
#include <unordered_map>
#include <shared_mutex>
#include <algorithm>
#include <cctype>

namespace
{
    std::unordered_map<ObjectGuid, CharacterCacheEntry> _characterCacheStore;
    std::unordered_map<std::string, ObjectGuid> _characterCacheByNameStore;
}

CharacterCache::CharacterCache() {}
CharacterCache::~CharacterCache() {}

CharacterCache* CharacterCache::instance()
{
    static CharacterCache instance;
    return &instance;
}

void CharacterCache::NormalizeName(std::string& name) const
{
    if (name.empty())
        return;

    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    name[0] = std::toupper(name[0]);
}

void CharacterCache::LoadCharacterCacheStorage()
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    _characterCacheStore.clear();
    _characterCacheByNameStore.clear();

    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT guid, account, name, gender, race, class, level FROM characters WHERE deleteDate IS NULL");
    if (!result)
    {
        TC_LOG_INFO("server.loading", "No character name data loaded, empty query");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt32());

        AddCharacterCacheEntryInternal(
            guid,
            fields[1].GetUInt32(), // account
            fields[2].GetString(), // name
            fields[3].GetUInt8(),  // gender
            fields[4].GetUInt8(),  // race
            fields[5].GetUInt8(),  // class
            fields[6].GetUInt8()   // level
        );

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded character infos for {} characters in {} ms", _characterCacheStore.size(), GetMSTimeDiffToNow(oldMSTime));
}

void CharacterCache::AddCharacterCacheEntryInternal(ObjectGuid const& guid, uint32 accountId, std::string name, uint8 gender, uint8 race, uint8 playerClass, uint8 level)
{
    NormalizeName(name);

    CharacterCacheEntry& data = _characterCacheStore[guid];
    data.Guid = guid;
    data.Name = name;
    data.AccountId = accountId;
    data.Race = race;
    data.Sex = gender;
    data.Class = playerClass;
    data.Level = level;
    data.GuildId = 0;

    for (uint8 i = 0; i < 3; ++i)
        data.ArenaTeamId[i] = 0;

    _characterCacheByNameStore[name] = guid;
}

void CharacterCache::AddCharacterCacheEntry(ObjectGuid const& guid, uint32 accountId, std::string name, uint8 gender, uint8 race, uint8 playerClass, uint8 level)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    AddCharacterCacheEntryInternal(guid, accountId, name, gender, race, playerClass, level);
}

void CharacterCache::DeleteCharacterCacheEntry(ObjectGuid const& guid, std::string name)
{
    NormalizeName(name);
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    _characterCacheStore.erase(guid);
    _characterCacheByNameStore.erase(name);
}

void CharacterCache::UpdateCharacterData(ObjectGuid const& guid, std::string name, Optional<uint8> gender, Optional<uint8> race)
{
    NormalizeName(name);
    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    std::string oldName = itr->second.Name;

    if (gender) itr->second.Sex = *gender;
    if (race)   itr->second.Race = *race;

    if (oldName != name)
    {
        _characterCacheByNameStore.erase(oldName);
        itr->second.Name = name;
        _characterCacheByNameStore[name] = guid;
    }

    WorldPackets::Misc::InvalidatePlayer packet(guid);
    sWorld->SendGlobalMessage(packet.Write());
}

void CharacterCache::UpdateCharacterLevel(ObjectGuid const& guid, uint8 level)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    if (auto itr = _characterCacheStore.find(guid); itr != _characterCacheStore.end())
        itr->second.Level = level;
}

void CharacterCache::UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    if (auto itr = _characterCacheStore.find(guid); itr != _characterCacheStore.end())
        itr->second.AccountId = accountId;
}

void CharacterCache::UpdateCharacterGuildId(ObjectGuid const& guid, ObjectGuid::LowType guildId)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    if (auto itr = _characterCacheStore.find(guid); itr != _characterCacheStore.end())
        itr->second.GuildId = guildId;
}

void CharacterCache::UpdateCharacterArenaTeamId(ObjectGuid const& guid, uint8 slot, uint32 arenaTeamId)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    if (auto itr = _characterCacheStore.find(guid); itr != _characterCacheStore.end())
    {
        if (slot < 3)
            itr->second.ArenaTeamId[slot] = arenaTeamId;
    }
}

bool CharacterCache::HasCharacterCacheEntry(ObjectGuid const& guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    return _characterCacheStore.find(guid) != _characterCacheStore.end();
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByGuid(ObjectGuid const& guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    return (itr != _characterCacheStore.end()) ? &itr->second : nullptr;
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByName(std::string name) const
{
    NormalizeName(name);
    std::shared_lock<std::shared_mutex> lock(_cacheLock);

    auto itrName = _characterCacheByNameStore.find(name);
    if (itrName != _characterCacheByNameStore.end())
    {
        auto itr = _characterCacheStore.find(itrName->second);
        return (itr != _characterCacheStore.end()) ? &itr->second : nullptr;
    }
    return nullptr;
}

ObjectGuid CharacterCache::GetCharacterGuidByName(std::string name) const
{
    NormalizeName(name);
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheByNameStore.find(name);
    return (itr != _characterCacheByNameStore.end()) ? itr->second : ObjectGuid::Empty;
}

bool CharacterCache::GetCharacterNameByGuid(ObjectGuid guid, std::string& name) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    if (itr != _characterCacheStore.end())
    {
        name = itr->second.Name;
        return true;
    }
    return false;
}

uint32 CharacterCache::GetCharacterTeamByGuid(ObjectGuid guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    if (itr != _characterCacheStore.end())
        return Player::TeamForRace(itr->second.Race);
    return 0;
}

uint32 CharacterCache::GetCharacterAccountIdByGuid(ObjectGuid guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    return (itr != _characterCacheStore.end()) ? itr->second.AccountId : 0;
}

uint32 CharacterCache::GetCharacterAccountIdByName(std::string name) const
{
    if (auto data = GetCharacterCacheByName(name))
        return data->AccountId;
    return 0;
}

uint8 CharacterCache::GetCharacterLevelByGuid(ObjectGuid guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    return (itr != _characterCacheStore.end()) ? itr->second.Level : 0;
}

ObjectGuid::LowType CharacterCache::GetCharacterGuildIdByGuid(ObjectGuid guid) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    return (itr != _characterCacheStore.end()) ? itr->second.GuildId : 0;
}

uint32 CharacterCache::GetCharacterArenaTeamIdByGuid(ObjectGuid guid, uint8 type) const
{
    std::shared_lock<std::shared_mutex> lock(_cacheLock);
    auto itr = _characterCacheStore.find(guid);
    if (itr != _characterCacheStore.end())
    {
        uint8 slot = ArenaTeam::GetSlotByType(type);
        if (slot < 3)
            return itr->second.ArenaTeamId[slot];
    }
    return 0;
}