/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "WorldSession.h"
#include "CharacterCache.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "Log.h"
#include "MapManager.h"
#include "NPCHandler.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryPackets.h"
#include "UpdateMask.h"
#include "World.h"

void WorldSession::SendNameQueryOpcode(ObjectGuid guid)
{
    Player* player = ObjectAccessor::FindConnectedPlayer(guid);
    CharacterCacheEntry const* nameData = sCharacterCache->GetCharacterCacheByGuid(guid);

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 8 + 1 + 1 + 1 + 1 + 10);
    data << guid.WriteAsPacked();
    if (!nameData)
    {
        SendPacket(&data);
        return;
    }

    data << nameData->Name;                         // played name
    data << uint8(0);                               // realm name - only set for cross realm interaction (such as Battlegrounds)
    data << uint8(nameData->Race);
    data << uint8(nameData->Sex);
    data << uint8(nameData->Class);

    if (DeclinedName const* names = (player ? player->GetDeclinedNames() : nullptr))
    {
        data << uint8(1);                           // Name is declined
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data << names->name[i];
    }
    else
        data << uint8(0);                           // Name is not declined

    SendPacket(&data);
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    recvData >> guid;

    // This is disable by default to prevent lots of console spam
    // TC_LOG_INFO("network", "HandleNameQueryOpcode %u", guid);

    SendNameQueryOpcode(guid);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket & /*recvData*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4 + 4);
    data << uint32(GameTime::GetGameTime());
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - GameTime::GetGameTime());
    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPackets::Query::QueryCreature& query)
{
    if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(query.CreatureID))
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), query.CreatureID);
        if (sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
            SendPacket(&ci->QueryData[static_cast<uint32>(GetSessionDbLocaleIndex())]);
        else
        {
            WorldPacket response = ci->BuildQueryData(GetSessionDbLocaleIndex());
            SendPacket(&response);
        }
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (%s, ENTRY: %u)",
            query.Guid.ToString().c_str(), query.CreatureID);

        WorldPackets::Query::QueryCreatureResponse response;
        response.CreatureID = query.CreatureID;
        SendPacket(response.Write());
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPackets::Query::QueryGameObject& query)
{
    if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(query.GameObjectID))
    {
        if (sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
            SendPacket(&info->QueryData[static_cast<uint32>(GetSessionDbLocaleIndex())]);
        else
        {
            WorldPacket response = info->BuildQueryData(GetSessionDbLocaleIndex());
            SendPacket(&response);
        }
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (%s, ENTRY: %u)",
            query.Guid.ToString().c_str(), query.GameObjectID);

        WorldPackets::Query::QueryGameObjectResponse response;
        response.GameObjectID = query.GameObjectID;
        SendPacket(response.Write());
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket & /*recvData*/)
{
    if (!_player->HasCorpse())
    {
        WorldPacket data(MSG_CORPSE_QUERY, 1);
        data << uint8(0);                                   // corpse not found
        SendPacket(&data);
        return;
    }

    WorldLocation corpseLocation = _player->GetCorpseLocation();
    uint32 corpseMapID = corpseLocation.GetMapId();
    uint32 mapID = corpseLocation.GetMapId();
    float x = corpseLocation.GetPositionX();
    float y = corpseLocation.GetPositionY();
    float z = corpseLocation.GetPositionZ();

    // if corpse at different map
    if (mapID != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapID))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->entrance_map >= 0)
            {
                // if corpse map have entrance
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->entrance_map))
                {
                    mapID = corpseMapEntry->entrance_map;
                    x = corpseMapEntry->entrance_x;
                    y = corpseMapEntry->entrance_y;
                    z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                }
            }
        }
    }

    WorldPacket data(MSG_CORPSE_QUERY, 1 + (5 * 4));
    data << uint8(1);                                       // corpse found
    data << int32(mapID);
    data << float(x);
    data << float(y);
    data << float(z);
    data << int32(corpseMapID);
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recvData)
{
    uint32 textID;
    uint64 guid;

    recvData >> textID;
    TC_LOG_DEBUG("network", "WORLD: CMSG_NPC_TEXT_QUERY TextId: %u", textID);

    recvData >> guid;

    GossipText const* gossip = sObjectMgr->GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);          // guess size
    data << textID;

    if (!gossip)
    {
        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << float(0);
            data << "Greetings $N";
            data << "Greetings $N";
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        std::string text0[MAX_GOSSIP_TEXT_OPTIONS], text1[MAX_GOSSIP_TEXT_OPTIONS];
        LocaleConstant locale = GetSessionDbLocaleIndex();

        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            BroadcastText const* bct = sObjectMgr->GetBroadcastText(gossip->Options[i].BroadcastTextID);
            if (bct)
            {
                text0[i] = bct->GetText(locale, GENDER_MALE, true);
                text1[i] = bct->GetText(locale, GENDER_FEMALE, true);
            }
            else
            {
                text0[i] = gossip->Options[i].Text_0;
                text1[i] = gossip->Options[i].Text_1;
            }

            if (locale != DEFAULT_LOCALE && !bct)
            {
                if (NpcTextLocale const* npcTextLocale = sObjectMgr->GetNpcTextLocale(textID))
                {
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_0[i], locale, text0[i]);
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_1[i], locale, text1[i]);
                }
            }

            data << gossip->Options[i].Probability;

            if (text0[i].empty())
                data << text1[i];
            else
                data << text0[i];

            if (text1[i].empty())
                data << text0[i];
            else
                data << text1[i];

            data << gossip->Options[i].Language;

            for (uint8 j = 0; j < MAX_GOSSIP_TEXT_EMOTES; ++j)
            {
                data << gossip->Options[i].Emotes[j]._Delay;
                data << gossip->Options[i].Emotes[j]._Emote;
            }
        }
    }

    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_PAGE_TEXT_QUERY");

    uint32 pageID;
    recvData >> pageID;
    recvData.read_skip<uint64>();                          // guid

    while (pageID)
    {
        PageText const* pageText = sObjectMgr->GetPageText(pageID);
                                                            // guess size
        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 50);
        data << pageID;

        if (!pageText)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            std::string Text = pageText->Text;

            LocaleConstant localeConstant = GetSessionDbLocaleIndex();
            if (localeConstant != LOCALE_enUS)
                if (PageTextLocale const* pageTextLocale = sObjectMgr->GetPageTextLocale(pageID))
                    ObjectMgr::GetLocaleString(pageTextLocale->Text, localeConstant, Text);

            data << Text;
            data << uint32(pageText->NextPageID);
            pageID = pageText->NextPageID;
        }
        SendPacket(&data);

        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}
