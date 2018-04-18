/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "LFGMgr.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "DisableMgr.h"
#include "GameEventMgr.h"
#include "GameTime.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceSaveMgr.h"
#include "LFGGroupData.h"
#include "LFGPlayerData.h"
#include "LFGScripts.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "World.h"
#include "WorldSession.h"

namespace lfg
{

LFGMgr::LFGMgr(): _options(sWorld->getIntConfig(CONFIG_LFG_OPTIONSMASK))
{
}

LFGMgr::~LFGMgr()
{
}

LFGMgr* LFGMgr::instance()
{
    static LFGMgr instance;
    return &instance;
}

void LFGMgr::JoinLfg(Player* player, uint8 lfgMode)
{
    if (!player || !player->GetSession())
        return;

    ObjectGuid pGuid = player->GetGUID();

    if (lfgMode == LFG_MODE_GROUP)
    {
        if (!PlayersStore[pGuid].IsAutoJoin())
            return;

        if (ObjectGuid gGuid = PlayersStore[pGuid].GetGroup())
            if (GroupsStore[gGuid].GetLeader() != pGuid)
                return;

        LfgGroupDataContainer const& groups = GroupsStore;
        for (LfgGroupDataContainer::const_iterator it = groups.begin(); it != groups.end(); ++it)
        {
            LfgGroupData data = it->second;

            if (data.IsAutoFill() || data.GetState() != LFG_STATE_QUEUED)
                continue;

            if (HasIgnore(data.GetLeader(), pGuid))
                continue;

            uint32 dungeon = data.GetDungeon(false);
            uint32 type = ((dungeon >> 24) & 0xFFFF);

            if (type != LFG_TYPE_DUNGEON || type != LFG_TYPE_HEROIC)
                continue;

            if (!PlayersStore[pGuid].HasDungeonSelected(dungeon))
                continue;

            if (Player* leader = ObjectAccessor::FindConnectedPlayer(data.GetLeader()))
            {
                if (PlayersStore[pGuid].GetTeam() != leader->GetTeam())
                    continue;

                if (Group* group = leader->GetGroup())
                {
                    if (!group->AddMember(player))
                        continue;
                    else
                        return;
                }
            }
        }
    }

    if (lfgMode == LFG_MODE_MORE)
    {
        if (ObjectGuid gGuid = PlayersStore[pGuid].GetGroup())
        {
            if (GroupsStore[gGuid].GetLeader() != pGuid || !GroupsStore[gGuid].IsAutoFill())
                return;

            uint32 dungeon = GroupsStore[gGuid].GetDungeon(false);
            uint32 type = ((dungeon >> 24) & 0xFFFF);

            if (type != LFG_TYPE_DUNGEON || type != LFG_TYPE_HEROIC)
                return;

            LfgPlayerDataContainer const& playerData = PlayersStore;
            for (LfgPlayerDataContainer::const_iterator it = playerData.begin(); it != playerData.end(); ++it)
            {
                if (it->first == pGuid)
                    continue;

                if (HasIgnore(it->first, pGuid))
                    continue;

                LfgPlayerData const& data = it->second;
                if (!data.IsAutoJoin() || data.GetGroup())
                    continue;

                if (data.GetTeam() != player->GetTeam())
                    continue;

                if (!data.HasDungeonSelected(dungeon))
                    continue;

                if (Player* member = ObjectAccessor::FindConnectedPlayer(it->first))
                {
                    if (Group* group = player->GetGroup())
                    {
                        if (!group->AddMember(member))
                            return;
                        else
                            continue;
                    }
                }
            }
        }
    }
}

void LFGMgr::HandleLfgInfo(Player* player, uint32 type, uint32 entry)
{
    if (!player || !player->GetSession())
        return;

    if (player->IsUsingLfg())
        JoinLfg(player, LFG_MODE_GROUP);

    if (ObjectGuid gGuid = PlayersStore[player->GetGUID()].GetGroup())
        if (GroupsStore[gGuid].GetLeader() == player->GetGUID())
            JoinLfg(player, LFG_MODE_MORE);

    SendLfgInfo(player, type, entry, LFG_MODE_GROUP);
}

void LFGMgr::SendLfgInfo(Player* player, uint32 type, uint32 entry, uint8 lfgMode)
{
    uint32 count = 0;

    WorldPacket data(MSG_LOOKING_FOR_GROUP);
    data << uint32(type);
    data << uint32(entry);
    data << uint32(0); // count place holder
    data << uint32(0); // count place holder

    LfgPlayerDataContainer const& playerData = PlayersStore;
    for (LfgPlayerDataContainer::const_iterator it = playerData.begin(); it != playerData.end(); ++it)
    {
        ObjectGuid pGuid = it->first;

        if (PlayersStore[pGuid].GetState() == LFG_STATE_NONE)
            continue;

        if (PlayersStore[pGuid].GetTeam() != player->GetTeam())
            continue;

        if (!PlayersStore[pGuid].HasDungeonSelected((entry | (type << 24))))
            continue;

        Player* plr = ObjectAccessor::FindConnectedPlayer(pGuid);
        if (!plr || !plr->FindMap() || plr->GetSession()->PlayerLoading())
            continue;

        ++count;

        data << pGuid.WriteAsPacked();
        data << uint32(plr->getLevel());
        data << uint32(plr->GetZoneId());
        data << uint8(lfgMode);

        LfgDungeonSet const& dungeons = PlayersStore[pGuid].GetSelectedDungeons();
        for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end(); ++it)
            data << uint32(*it);

        data << PlayersStore[pGuid].GetComment();

        if (ObjectGuid gGuid = PlayersStore[pGuid].GetGroup())
        {
            GuidSet const& players = GetPlayers(gGuid);
            data << uint32(players.size() -1);
            for (ObjectGuid playerGUID : players)
            {
                if (playerGUID != pGuid)
                {
                    if (Player* member = ObjectAccessor::FindPlayer(playerGUID))
                    {
                        data << playerGUID.WriteAsPacked();
                        data << uint32(member->getLevel());
                    }
                }
            }
        }
        else
            data << uint32(0);
    }

    data.put<uint32>(4 + 4, count);
    data.put<uint32>(4 + 4 + 4, count);

    player->GetSession()->SendPacket(&data);
}

/**
    Leaves Dungeon System. Player/Group is removed from queue, proposals
    Player or group needs to be not NULL and using Dungeon System

   @param[in]     guid Player or group guid
*/
void LFGMgr::LeaveLfg(ObjectGuid guid, bool disconnected)
{
    ObjectGuid gguid = guid.IsGroup() ? guid : GetGroup(guid);

    TC_LOG_DEBUG("lfg.leave", "%s left (%s)", guid.ToString().c_str(), guid == gguid ? "group" : "player");

    LfgState state = GetState(guid);
    switch (state)
    {
        case LFG_STATE_QUEUED:
            if (gguid)
            {
                SetState(gguid, LFG_STATE_NONE);
                GuidSet const& players = GetPlayers(gguid);
                for (GuidSet::const_iterator it = players.begin(); it != players.end(); ++it)
                    SetState(*it, LFG_STATE_NONE);
            }
            else
                SetState(guid, LFG_STATE_NONE);
            break;
        case LFG_STATE_NONE:
        case LFG_STATE_RAIDBROWSER:
            break;
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
            if (guid != gguid && !disconnected) // Player
                SetState(guid, LFG_STATE_NONE);
            break;
    }
}

// --------------------------------------------------------------------------//
// Auxiliar Functions
// --------------------------------------------------------------------------//

LfgState LFGMgr::GetState(ObjectGuid guid)
{
    LfgState state;
    if (guid.IsGroup())
    {
        state = GroupsStore[guid].GetState();
        TC_LOG_TRACE("lfg.data.group.state.get", "Group: %s, State: %s", guid.ToString().c_str(), GetStateString(state).c_str());
    }
    else
    {
        state = PlayersStore[guid].GetState();
        TC_LOG_TRACE("lfg.data.player.state.get", "Player: %s, State: %s", guid.ToString().c_str(), GetStateString(state).c_str());
    }

    return state;
}

LfgState LFGMgr::GetOldState(ObjectGuid guid)
{
    LfgState state;
    if (guid.IsGroup())
    {
        state = GroupsStore[guid].GetOldState();
        TC_LOG_TRACE("lfg.data.group.oldstate.get", "Group: %s, Old state: %u", guid.ToString().c_str(), state);
    }
    else
    {
        state = PlayersStore[guid].GetOldState();
        TC_LOG_TRACE("lfg.data.player.oldstate.get", "Player: %s, Old state: %u", guid.ToString().c_str(), state);
    }

    return state;
}

uint32 LFGMgr::GetDungeon(ObjectGuid guid, bool asId /*= true */)
{
    uint32 dungeon = GroupsStore[guid].GetDungeon(asId);
    TC_LOG_TRACE("lfg.data.group.dungeon.get", "Group: %s, asId: %u, Dungeon: %u", guid.ToString().c_str(), asId, dungeon);
    return dungeon;
}

const std::string& LFGMgr::GetComment(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.comment.get", "Player: %s, Comment: %s", guid.ToString().c_str(), PlayersStore[guid].GetComment().c_str());
    return PlayersStore[guid].GetComment();
}

LfgDungeonSet LFGMgr::GetSelectedDungeons(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.dungeons.selected.get", "Player: %s, Selected Dungeons: %s", guid.ToString().c_str(), ConcatenateDungeons(PlayersStore[guid].GetSelectedDungeons()).c_str());
    return PlayersStore[guid].GetSelectedDungeons();
}

void LFGMgr::RestoreState(ObjectGuid guid, char const* debugMsg)
{
    if (guid.IsGroup())
    {
        LfgGroupData& data = GroupsStore[guid];
        TC_LOG_TRACE("lfg.data.group.state.restore", "Group: %s (%s), State: %s, Old state: %s",
            guid.ToString().c_str(), debugMsg, GetStateString(data.GetState()).c_str(),
            GetStateString(data.GetOldState()).c_str());

        data.RestoreState();
    }
    else
    {
        LfgPlayerData& data = PlayersStore[guid];
        TC_LOG_TRACE("lfg.data.player.state.restore", "Player: %s (%s), State: %s, Old state: %s",
            guid.ToString().c_str(), debugMsg, GetStateString(data.GetState()).c_str(),
            GetStateString(data.GetOldState()).c_str());

        data.RestoreState();
    }
}

void LFGMgr::SetState(ObjectGuid guid, LfgState state)
{
    if (guid.IsGroup())
    {
        LfgGroupData& data = GroupsStore[guid];
        TC_LOG_TRACE("lfg.data.group.state.set", "Group: %s, New state: %s, Previous: %s, Old state: %s",
            guid.ToString().c_str(), GetStateString(state).c_str(), GetStateString(data.GetState()).c_str(),
            GetStateString(data.GetOldState()).c_str());

        data.SetState(state);
    }
    else
    {
        LfgPlayerData& data = PlayersStore[guid];
        TC_LOG_TRACE("lfg.data.player.state.set", "Player: %s, New state: %s, Previous: %s, OldState: %s",
            guid.ToString().c_str(), GetStateString(state).c_str(), GetStateString(data.GetState()).c_str(),
            GetStateString(data.GetOldState()).c_str());

        data.SetState(state);
    }
}

void LFGMgr::SetDungeon(ObjectGuid guid, uint32 dungeon)
{
    TC_LOG_TRACE("lfg.data.group.dungeon.set", "Group: %s, Dungeon: %u", guid.ToString().c_str(), dungeon);
    GroupsStore[guid].SetDungeon(dungeon);
}

void LFGMgr::SetComment(ObjectGuid guid, std::string const& comment)
{
    TC_LOG_TRACE("lfg.data.player.comment.set", "Player: %s, Comment: %s", guid.ToString().c_str(), comment.c_str());
    PlayersStore[guid].SetComment(comment);
}

void LFGMgr::SetAutoJoin(ObjectGuid guid, bool autoJoin)
{
    PlayersStore[guid].SetAutoJoin(autoJoin);
}

void LFGMgr::SetAutoFill(ObjectGuid guid, bool autoFill)
{
    PlayersStore[guid].SetAutoFill(autoFill);

    if (ObjectGuid gGuid = GetGroup(guid))
        if (GroupsStore[gGuid].GetLeader() == guid)
            GroupsStore[gGuid].SetAutoFill(autoFill);
}

void LFGMgr::SetSelectedDungeons(ObjectGuid guid, uint32 slot, uint32 dungeon)
{
    TC_LOG_TRACE("lfg.data.player.dungeon.selected.set", "Player: %s, Dungeon: %u", guid.ToString().c_str(), dungeon);
    PlayersStore[guid].SetSelectedDungeons(slot, dungeon);
}

void LFGMgr::RemovePlayerData(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.remove", "Player: %s", guid.ToString().c_str());
    LfgPlayerDataContainer::iterator it = PlayersStore.find(guid);
    if (it != PlayersStore.end())
        PlayersStore.erase(it);
}

void LFGMgr::RemoveGroupData(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.group.remove", "Group: %s", guid.ToString().c_str());
    LfgGroupDataContainer::iterator it = GroupsStore.find(guid);
    if (it == GroupsStore.end())
        return;

    // If group is being formed after proposal success do nothing more
    GuidSet const& players = it->second.GetPlayers();
    for (ObjectGuid playerGUID : players)
    {
        SetGroup(playerGUID, ObjectGuid::Empty);
        SetState(playerGUID, LFG_STATE_NONE);
    }
    GroupsStore.erase(it);
}

uint8 LFGMgr::GetTeam(ObjectGuid guid)
{
    uint8 team = PlayersStore[guid].GetTeam();
    TC_LOG_TRACE("lfg.data.player.team.get", "Player: %s, Team: %u", guid.ToString().c_str(), team);
    return team;
}

uint8 LFGMgr::RemovePlayerFromGroup(ObjectGuid gguid, ObjectGuid guid)
{
    return GroupsStore[gguid].RemovePlayer(guid);
}

void LFGMgr::AddPlayerToGroup(ObjectGuid gguid, ObjectGuid guid)
{
    GroupsStore[gguid].AddPlayer(guid);
}

void LFGMgr::SetLeader(ObjectGuid gguid, ObjectGuid leader)
{
    GroupsStore[gguid].SetLeader(leader);
}

void LFGMgr::SetTeam(ObjectGuid guid, uint8 team)
{
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
        team = 0;

    PlayersStore[guid].SetTeam(team);
}

ObjectGuid LFGMgr::GetGroup(ObjectGuid guid)
{
    return PlayersStore[guid].GetGroup();
}

void LFGMgr::SetGroup(ObjectGuid guid, ObjectGuid group)
{
    PlayersStore[guid].SetGroup(group);
}

GuidSet const& LFGMgr::GetPlayers(ObjectGuid guid)
{
    return GroupsStore[guid].GetPlayers();
}

uint8 LFGMgr::GetPlayerCount(ObjectGuid guid)
{
    return GroupsStore[guid].GetPlayerCount();
}

ObjectGuid LFGMgr::GetLeader(ObjectGuid guid)
{
    return GroupsStore[guid].GetLeader();
}

bool LFGMgr::HasIgnore(ObjectGuid guid1, ObjectGuid guid2)
{
    Player* plr1 = ObjectAccessor::FindConnectedPlayer(guid1);
    Player* plr2 = ObjectAccessor::FindConnectedPlayer(guid2);
    return plr1 && plr2 && (plr1->GetSocial()->HasIgnore(guid2) || plr2->GetSocial()->HasIgnore(guid1));
}

bool LFGMgr::IsLfgGroup(ObjectGuid guid)
{
    return guid && guid.IsGroup() && GroupsStore[guid].IsLfgGroup();
}

bool LFGMgr::isOptionEnabled(uint32 option)
{
    return (_options & option) != 0;
}

uint32 LFGMgr::GetOptions()
{
    return _options;
}

void LFGMgr::SetOptions(uint32 options)
{
    _options = options;
}

} // namespace lfg
