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

LFGDungeonData::LFGDungeonData() : id(0), name(), map(0), type(0), expansion(0), group(0), minlevel(0),
    maxlevel(0), difficulty(REGULAR_DIFFICULTY), seasonal(false), x(0.0f), y(0.0f), z(0.0f), o(0.0f)
{
}

LFGDungeonData::LFGDungeonData(LFGDungeonEntry const* dbc) : id(dbc->ID), name(dbc->name[0]),
    type(dbc->type), expansion(uint8(dbc->expansion)),
    minlevel(uint8(dbc->minlevel)), maxlevel(uint8(dbc->maxlevel)),
    x(0.0f), y(0.0f), z(0.0f), o(0.0f)
{
}

LFGMgr::LFGMgr(): m_QueueTimer(0), m_lfgProposalId(1),
    m_options(sWorld->getIntConfig(CONFIG_LFG_OPTIONSMASK))
{
}

LFGMgr::~LFGMgr()
{
}

void LFGMgr::_LoadFromDB(Field* fields, ObjectGuid guid)
{
    if (!fields)
        return;

    if (!guid.IsGroup())
        return;

    SetLeader(guid, ObjectGuid(HighGuid::Player, fields[0].GetUInt32()));

    uint32 dungeon = fields[17].GetUInt32();
    uint8 state = fields[18].GetUInt8();

    if (!dungeon || !state)
        return;

    SetDungeon(guid, dungeon);

    switch (state)
    {
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
            SetState(guid, (LfgState)state);
            break;
        default:
            break;
    }
}

void LFGMgr::_SaveToDB(ObjectGuid guid, uint32 db_guid)
{
    if (!guid.IsGroup())
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_LFG_DATA);
    stmt->setUInt32(0, db_guid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_LFG_DATA);
    stmt->setUInt32(0, db_guid);
    stmt->setUInt32(1, GetDungeon(guid));
    stmt->setUInt32(2, GetState(guid));
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

LFGDungeonData const* LFGMgr::GetLFGDungeon(uint32 id)
{
    LFGDungeonContainer::const_iterator itr = LfgDungeonStore.find(id);
    if (itr != LfgDungeonStore.end())
        return &(itr->second);

    return nullptr;
}

LFGMgr* LFGMgr::instance()
{
    static LFGMgr instance;
    return &instance;
}

void LFGMgr::Update(uint32 diff)
{
    if (!isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    time_t currTime = GameTime::GetGameTime();

    // Remove obsolete proposals
    for (LfgProposalContainer::iterator it = ProposalsStore.begin(); it != ProposalsStore.end();)
    {
        LfgProposalContainer::iterator itRemove = it++;
        if (itRemove->second.cancelTime < currTime)
            RemoveProposal(itRemove, LFG_UPDATETYPE_PROPOSAL_FAILED);
    }

    uint32 lastProposalId = m_lfgProposalId;

    if (lastProposalId != m_lfgProposalId)
    {
        // FIXME lastProposalId ? lastProposalId +1 ?
        for (LfgProposalContainer::const_iterator itProposal = ProposalsStore.find(m_lfgProposalId); itProposal != ProposalsStore.end(); ++itProposal)
        {
            uint32 proposalId = itProposal->first;
            LfgProposal& proposal = ProposalsStore[proposalId];

            ObjectGuid guid;
            for (LfgProposalPlayerContainer::const_iterator itPlayers = proposal.players.begin(); itPlayers != proposal.players.end(); ++itPlayers)
            {
                guid = itPlayers->first;
                SetState(guid, LFG_STATE_PROPOSAL);
                if (ObjectGuid gguid = GetGroup(guid))
                    SetState(gguid, LFG_STATE_PROPOSAL);
            }

            if (proposal.state == LFG_PROPOSAL_SUCCESS)
                UpdateProposal(proposalId, guid, true);
        }
    }

    m_QueueTimer += diff;
}

/**
    Adds the player/group to lfg queue. If player is in a group then it is the leader
    of the group tying to join the group. Join conditions are checked before adding
    to the new queue.

   @param[in]     player Player trying to join (or leader of group trying to join)
   @param[in]     dungeons Dungeons the player/group is applying for
   @param[in]     comment Player selected comment
*/
void LFGMgr::JoinLfg(Player* player)
{
    if (!player || !player->GetSession())
        return;
}

/**
    Leaves Dungeon System. Player/Group is removed from queue, rolechecks, proposals
    or votekicks. Player or group needs to be not NULL and using Dungeon System

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
        case LFG_STATE_PROPOSAL:
        {
            // Remove from Proposals
            LfgProposalContainer::iterator it = ProposalsStore.begin();
            ObjectGuid pguid = gguid == guid ? GetLeader(gguid) : guid;
            while (it != ProposalsStore.end())
            {
                LfgProposalPlayerContainer::iterator itPlayer = it->second.players.find(pguid);
                if (itPlayer != it->second.players.end())
                {
                    // Mark the player/leader of group who left as didn't accept the proposal
                    itPlayer->second.accept = LFG_ANSWER_DENY;
                    break;
                }
                ++it;
            }

            // Remove from queue - if proposal is found, RemoveProposal will call RemoveFromQueue
            if (it != ProposalsStore.end())
                RemoveProposal(it, LFG_UPDATETYPE_PROPOSAL_DECLINED);
            break;
        }
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

/**
   Given a list of dungeons remove the dungeons players have restrictions.

   @param[in, out] dungeons Dungeons to check restrictions
   @param[in]     players Set of players to check their dungeon restrictions
   @param[out]    lockMap Map of players Lock status info of given dungeons (Empty if dungeons is not empty)
*/
void LFGMgr::GetCompatibleDungeons(LfgDungeonSet& dungeons, GuidSet const& players, LfgLockPartyMap& lockMap, bool isContinue)
{
    lockMap.clear();

    std::map<uint32, uint32> lockedDungeons;

    for (GuidSet::const_iterator it = players.begin(); it != players.end() && !dungeons.empty(); ++it)
    {
        ObjectGuid guid = (*it);
        LfgLockMap const& cachedLockMap = GetLockedDungeons(guid);
        Player* player = ObjectAccessor::FindConnectedPlayer(guid);
        for (LfgLockMap::const_iterator it2 = cachedLockMap.begin(); it2 != cachedLockMap.end() && !dungeons.empty(); ++it2)
        {
            uint32 dungeonId = (it2->first & 0x00FFFFFF); // Compare dungeon ids
            LfgDungeonSet::iterator itDungeon = dungeons.find(dungeonId);
            if (itDungeon != dungeons.end())
            {
                bool eraseDungeon = true;

                // Don't remove the dungeon if team members are trying to continue a locked instance
                if (it2->second == LFG_LOCKSTATUS_RAID_LOCKED && isContinue)
                {
                    LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId);
                    ASSERT(dungeon);
                    ASSERT(player);
                    if (InstancePlayerBind* playerBind = player->GetBoundInstance(dungeon->map, Difficulty(dungeon->difficulty)))
                    {
                        if (InstanceSave* playerSave = playerBind->save)
                        {
                            uint32 dungeonInstanceId = playerSave->GetInstanceId();
                            auto itLockedDungeon = lockedDungeons.find(dungeonId);
                            if (itLockedDungeon == lockedDungeons.end() || itLockedDungeon->second == dungeonInstanceId)
                                eraseDungeon = false;

                            lockedDungeons[dungeonId] = dungeonInstanceId;
                        }
                    }
                }

                if (eraseDungeon)
                    dungeons.erase(itDungeon);

                lockMap[guid][dungeonId] = it2->second;
            }
        }
    }
    if (!dungeons.empty())
        lockMap.clear();
}

/**
   Makes a new group given a proposal
   @param[in]     proposal Proposal to get info from
*/
void LFGMgr::MakeNewGroup(LfgProposal const& proposal)
{
    GuidList players;
    GuidList playersToTeleport;

    for (LfgProposalPlayerContainer::const_iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        ObjectGuid guid = it->first;
        if (guid == proposal.leader)
            players.push_front(guid);
        else
            players.push_back(guid);

        if (proposal.isNew || GetGroup(guid) != proposal.group)
            playersToTeleport.push_back(guid);
    }

    // Set the dungeon difficulty
    LFGDungeonData const* dungeon = GetLFGDungeon(proposal.dungeonId);
    ASSERT(dungeon);

    Group* grp = proposal.group ? sGroupMgr->GetGroupByGUID(proposal.group.GetCounter()) : nullptr;
    for (GuidList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        ObjectGuid pguid = (*it);
        Player* player = ObjectAccessor::FindConnectedPlayer(pguid);
        if (!player)
            continue;

        Group* group = player->GetGroup();
        if (group && group != grp)
            group->RemoveMember(player->GetGUID());

        if (!grp)
        {
            grp = new Group();
            grp->ConvertToLFG();
            grp->Create(player);
            ObjectGuid gguid = grp->GetGUID();
            SetState(gguid, LFG_STATE_PROPOSAL);
            sGroupMgr->AddGroup(grp);
        }
        else if (group != grp)
            grp->AddMember(player);

        // Add the cooldown spell if queued for a random dungeon
        if (dungeon->type == LFG_TYPE_RANDOM)
            player->CastSpell(player, LFG_SPELL_DUNGEON_COOLDOWN, false);
    }

    ASSERT(grp);
    grp->SetDungeonDifficulty(Difficulty(dungeon->difficulty));
    ObjectGuid gguid = grp->GetGUID();
    SetDungeon(gguid, dungeon->Entry());
    SetState(gguid, LFG_STATE_DUNGEON);

    _SaveToDB(gguid, grp->GetDbStoreId());

    // Teleport Player
    for (GuidList::const_iterator it = playersToTeleport.begin(); it != playersToTeleport.end(); ++it)
        if (Player* player = ObjectAccessor::FindPlayer(*it))
            TeleportPlayer(player, false);

    // Update group info
    grp->SendUpdate();
}

uint32 LFGMgr::AddProposal(LfgProposal& proposal)
{
    proposal.id = ++m_lfgProposalId;
    ProposalsStore[m_lfgProposalId] = proposal;
    return m_lfgProposalId;
}

/**
   Update Proposal info with player answer

   @param[in]     proposalId Proposal id to be updated
   @param[in]     guid Player guid to update answer
   @param[in]     accept Player answer
*/
void LFGMgr::UpdateProposal(uint32 proposalId, ObjectGuid guid, bool accept)
{
    // Check if the proposal exists
    LfgProposalContainer::iterator itProposal = ProposalsStore.find(proposalId);
    if (itProposal == ProposalsStore.end())
        return;

    LfgProposal& proposal = itProposal->second;

    // Check if proposal have the current player
    LfgProposalPlayerContainer::iterator itProposalPlayer = proposal.players.find(guid);
    if (itProposalPlayer == proposal.players.end())
        return;

    LfgProposalPlayer& player = itProposalPlayer->second;
    player.accept = LfgAnswer(accept);

    TC_LOG_DEBUG("lfg.proposal.update", "%s, Proposal %u, Selection: %u", guid.ToString().c_str(), proposalId, accept);
    if (!accept)
    {
        RemoveProposal(itProposal, LFG_UPDATETYPE_PROPOSAL_DECLINED);
        return;
    }

    // check if all have answered and reorder players (leader first)
    bool allAnswered = true;
    for (LfgProposalPlayerContainer::const_iterator itPlayers = proposal.players.begin(); itPlayers != proposal.players.end(); ++itPlayers)
        if (itPlayers->second.accept != LFG_ANSWER_AGREE)   // No answer (-1) or not accepted (0)
            allAnswered = false;

    if (!allAnswered)
        return;

    proposal.state = LFG_PROPOSAL_SUCCESS;
    MakeNewGroup(proposal);
    ProposalsStore.erase(itProposal);
}

/**
   Remove a proposal from the pool, remove the group that didn't accept (if needed) and readd the other members to the queue

   @param[in]     itProposal Iterator to the proposal to remove
   @param[in]     type Type of removal (LFG_UPDATETYPE_PROPOSAL_FAILED, LFG_UPDATETYPE_PROPOSAL_DECLINED)
*/
void LFGMgr::RemoveProposal(LfgProposalContainer::iterator itProposal, LfgUpdateType type)
{
    LfgProposal& proposal = itProposal->second;
    proposal.state = LFG_PROPOSAL_FAILED;

    TC_LOG_DEBUG("lfg.proposal.remove", "Proposal %u, state FAILED, UpdateType %u", itProposal->first, type);
    // Mark all people that didn't answered as no accept
    if (type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        for (LfgProposalPlayerContainer::iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
            if (it->second.accept == LFG_ANSWER_PENDING)
                it->second.accept = LFG_ANSWER_DENY;

    // Mark players/groups to be removed
    GuidSet toRemove;
    for (LfgProposalPlayerContainer::iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        if (it->second.accept == LFG_ANSWER_AGREE)
            continue;

        ObjectGuid guid = it->second.group ? it->second.group : it->first;
        // Player didn't accept or still pending when no secs left
        if (it->second.accept == LFG_ANSWER_DENY || type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        {
            it->second.accept = LFG_ANSWER_DENY;
            toRemove.insert(guid);
        }
    }

    // Notify players
    for (LfgProposalPlayerContainer::const_iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        ObjectGuid guid = it->first;
        ObjectGuid gguid = it->second.group ? it->second.group : guid;

        if (toRemove.find(gguid) != toRemove.end())         // Didn't accept or in same group that someone that didn't accept
        {
            RestoreState(guid, "Proposal Fail (didn't accepted or in group with someone that didn't accept");
            if (gguid != guid)
                RestoreState(it->second.group, "Proposal Fail (someone in group didn't accepted)");
        }
        else
        {
            TC_LOG_DEBUG("lfg.proposal.remove", "Readding %s to queue.", guid.ToString().c_str());
            SetState(guid, LFG_STATE_QUEUED);
            if (gguid != guid)
                SetState(gguid, LFG_STATE_QUEUED);
        }
    }

    ProposalsStore.erase(itProposal);
}

/**
   Teleports the player in or out the dungeon

   @param[in]     player Player to teleport
   @param[in]     out Teleport out (true) or in (false)
   @param[in]     fromOpcode Function called from opcode handlers? (Default false)
*/
void LFGMgr::TeleportPlayer(Player* player, bool out, bool fromOpcode /*= false*/)
{
    LFGDungeonData const* dungeon = nullptr;
    Group* group = player->GetGroup();

    if (group && group->isLFGGroup())
        dungeon = GetLFGDungeon(GetDungeon(group->GetGUID()));

    if (!dungeon)
    {
        TC_LOG_DEBUG("lfg.teleport", "Player %s not in group/lfggroup or dungeon not found!",
            player->GetName().c_str());
        return;
    }

    if (out)
    {
        TC_LOG_DEBUG("lfg.teleport", "Player %s is being teleported out. Current Map %u - Expected Map %u",
            player->GetName().c_str(), player->GetMapId(), uint32(dungeon->map));
        if (player->GetMapId() == uint32(dungeon->map))
            player->TeleportToBGEntryPoint();

        return;
    }

    LfgTeleportError error = LFG_TELEPORTERROR_OK;

    if (!player->IsAlive())
        error = LFG_TELEPORTERROR_PLAYER_DEAD;
    else if (player->IsFalling() || player->HasUnitState(UNIT_STATE_JUMPING))
        error = LFG_TELEPORTERROR_FALLING;
    else if (player->IsMirrorTimerActive(FATIGUE_TIMER))
        error = LFG_TELEPORTERROR_FATIGUE;
    else if (player->GetCharmGUID())
        error = LFG_TELEPORTERROR_CHARMING;
    else if (player->HasAura(9454)) // check Freeze debuff
        error = LFG_TELEPORTERROR_INVALID_LOCATION;
    else if (player->GetMapId() != uint32(dungeon->map))  // Do not teleport players in dungeon to the entrance
    {
        uint32 mapid = dungeon->map;
        float x = dungeon->x;
        float y = dungeon->y;
        float z = dungeon->z;
        float orientation = dungeon->o;

        if (!fromOpcode)
        {
            // Select a player inside to be teleported to
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr && !mapid; itr = itr->next())
            {
                Player* plrg = itr->GetSource();
                if (plrg && plrg != player && plrg->GetMapId() == uint32(dungeon->map))
                {
                    mapid = plrg->GetMapId();
                    x = plrg->GetPositionX();
                    y = plrg->GetPositionY();
                    z = plrg->GetPositionZ();
                    orientation = plrg->GetOrientation();
                    break;
                }
            }
        }

        if (!player->GetMap()->IsDungeon())
            player->SetBattlegroundEntryPoint();

        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }

        if (!player->TeleportTo(mapid, x, y, z, orientation))
            error = LFG_TELEPORTERROR_INVALID_LOCATION;
    }
    else
        error = LFG_TELEPORTERROR_INVALID_LOCATION;

    TC_LOG_DEBUG("lfg.teleport", "Player %s is being teleported in to map %u "
        "(x: %f, y: %f, z: %f) Result: %u", player->GetName().c_str(), dungeon->map,
        dungeon->x, dungeon->y, dungeon->z, error);
}

/**
   Finish a dungeon and give reward, if any.

   @param[in]     guid Group guid
   @param[in]     dungeonId Dungeonid
*/
void LFGMgr::FinishDungeon(ObjectGuid gguid, const uint32 dungeonId, Map const* currMap)
{
    uint32 gDungeonId = GetDungeon(gguid);
    if (gDungeonId != dungeonId)
    {
        TC_LOG_DEBUG("lfg.dungeon.finish", "Group %s finished dungeon %u but queued for %u", gguid.ToString().c_str(), dungeonId, gDungeonId);
        return;
    }

    if (GetState(gguid) == LFG_STATE_FINISHED_DUNGEON) // Shouldn't happen. Do not reward multiple times
    {
        TC_LOG_DEBUG("lfg.dungeon.finish", "Group: %s already rewarded", gguid.ToString().c_str());
        return;
    }

    SetState(gguid, LFG_STATE_FINISHED_DUNGEON);

    GuidSet const& players = GetPlayers(gguid);
    for (GuidSet::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        ObjectGuid guid = (*it);
        if (GetState(guid) == LFG_STATE_FINISHED_DUNGEON)
        {
            TC_LOG_DEBUG("lfg.dungeon.finish", "Group: %s, Player: %s already rewarded", gguid.ToString().c_str(), guid.ToString().c_str());
            continue;
        }

        uint32 rDungeonId = 0;
        LfgDungeonSet const& dungeons = GetSelectedDungeons(guid);
        if (!dungeons.empty())
            rDungeonId = (*dungeons.begin());

        SetState(guid, LFG_STATE_FINISHED_DUNGEON);

        // Give rewards only if its a random dungeon
        LFGDungeonData const* dungeon = GetLFGDungeon(rDungeonId);

        if (!dungeon || (dungeon->type != LFG_TYPE_RANDOM && !dungeon->seasonal))
        {
            TC_LOG_DEBUG("lfg.dungeon.finish", "Group: %s, Player: %s dungeon %u is not random or seasonal", gguid.ToString().c_str(), guid.ToString().c_str(), rDungeonId);
            continue;
        }

        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player || player->FindMap() != currMap)
        {
            TC_LOG_DEBUG("lfg.dungeon.finish", "Group: %s, Player: %s not found in world", gguid.ToString().c_str(), guid.ToString().c_str());
            continue;
        }

        LFGDungeonData const* dungeonDone = GetLFGDungeon(dungeonId);
        uint32 mapId = dungeonDone ? uint32(dungeonDone->map) : 0;

        if (player->GetMapId() != mapId)
        {
            TC_LOG_DEBUG("lfg.dungeon.finish", "Group: %s, Player: %s is in map %u and should be in %u to get reward", gguid.ToString().c_str(), guid.ToString().c_str(), player->GetMapId(), mapId);
            continue;
        }
    }
}

// --------------------------------------------------------------------------//
// Auxiliar Functions
// --------------------------------------------------------------------------//

/**
   Get the dungeon list that can be done given a random dungeon entry.

   @param[in]     randomdungeon Random dungeon id (if value = 0 will return all dungeons)
   @returns Set of dungeons that can be done.
*/
LfgDungeonSet const& LFGMgr::GetDungeonsByRandom(uint32 randomdungeon)
{
    LFGDungeonData const* dungeon = GetLFGDungeon(randomdungeon);
    uint32 group = dungeon ? dungeon->group : 0;
    return CachedDungeonMapStore[group];
}

/**
   Given a Dungeon id returns the dungeon Type

   @param[in]     dungeon dungeon id
   @returns Dungeon type
*/
LfgType LFGMgr::GetDungeonType(uint32 dungeonId)
{
    LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId);
    if (!dungeon)
        return LFG_TYPE_NONE;

    return LfgType(dungeon->type);
}

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

bool LFGMgr::IsVoteKickActive(ObjectGuid gguid)
{
    ASSERT(gguid.IsGroup());

    bool active = GroupsStore[gguid].IsVoteKickActive();
    TC_LOG_TRACE("lfg.data.group.votekick.get", "Group: %s, Active: %d", gguid.ToString().c_str(), active);

    return active;
}

uint32 LFGMgr::GetDungeon(ObjectGuid guid, bool asId /*= true */)
{
    uint32 dungeon = GroupsStore[guid].GetDungeon(asId);
    TC_LOG_TRACE("lfg.data.group.dungeon.get", "Group: %s, asId: %u, Dungeon: %u", guid.ToString().c_str(), asId, dungeon);
    return dungeon;
}

uint32 LFGMgr::GetDungeonMapId(ObjectGuid guid)
{
    uint32 dungeonId = GroupsStore[guid].GetDungeon(true);
    uint32 mapId = 0;
    if (dungeonId)
        if (LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId))
            mapId = dungeon->map;

    TC_LOG_TRACE("lfg.data.group.dungeon.map", "Group: %s, MapId: %u (DungeonId: %u)", guid.ToString().c_str(), mapId, dungeonId);

    return mapId;
}

const std::string& LFGMgr::GetComment(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.comment.get", "Player: %s, Comment: %s", guid.ToString().c_str(), PlayersStore[guid].GetComment().c_str());
    return PlayersStore[guid].GetComment();
}

LfgDungeonSet const& LFGMgr::GetSelectedDungeons(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.dungeons.selected.get", "Player: %s, Selected Dungeons: %s", guid.ToString().c_str(), ConcatenateDungeons(PlayersStore[guid].GetSelectedDungeons()).c_str());
    return PlayersStore[guid].GetSelectedDungeons();
}

LfgLockMap const LFGMgr::GetLockedDungeons(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.dungeons.locked.get", "Player: %s, LockedDungeons.", guid.ToString().c_str());
    LfgLockMap lock;
    Player* player = ObjectAccessor::FindConnectedPlayer(guid);
    if (!player)
    {
        TC_LOG_WARN("lfg.data.player.dungeons.locked.get", "Player: %s not ingame while retrieving his LockedDungeons.", guid.ToString().c_str());
        return lock;
    }

    uint8 level = player->getLevel();
    uint8 expansion = player->GetSession()->Expansion();
    LfgDungeonSet const& dungeons = GetDungeonsByRandom(0);
    bool denyJoin = !player->GetSession()->HasPermission(rbac::RBAC_PERM_JOIN_DUNGEON_FINDER);

    for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end(); ++it)
    {
        LFGDungeonData const* dungeon = GetLFGDungeon(*it);
        if (!dungeon) // should never happen - We provide a list from sLFGDungeonStore
            continue;

        uint32 lockData = 0;
        if (denyJoin)
            lockData = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (dungeon->expansion > expansion)
            lockData = LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
        else if (DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, dungeon->map, player))
            lockData = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (DisableMgr::IsDisabledFor(DISABLE_TYPE_LFG_MAP, dungeon->map, player))
            lockData = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (dungeon->difficulty > DUNGEON_DIFFICULTY_NORMAL && player->GetBoundInstance(dungeon->map, Difficulty(dungeon->difficulty)))
            lockData = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (dungeon->minlevel > level)
            lockData = LFG_LOCKSTATUS_TOO_LOW_LEVEL;
        else if (dungeon->maxlevel < level)
            lockData = LFG_LOCKSTATUS_TOO_HIGH_LEVEL;
        else if (dungeon->seasonal && !IsSeasonActive(dungeon->id))
            lockData = LFG_LOCKSTATUS_NOT_IN_SEASON;
        else if (AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(dungeon->map, Difficulty(dungeon->difficulty)))
        {
            if (ar->item_level && player->GetAverageItemLevel() < ar->item_level)
                lockData = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
            else if (player->GetTeam() == ALLIANCE && ar->quest_A && !player->GetQuestRewardStatus(ar->quest_A))
                lockData = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else if (player->GetTeam() == HORDE && ar->quest_H && !player->GetQuestRewardStatus(ar->quest_H))
                lockData = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else
                if (ar->item)
                {
                    if (!player->HasItemCount(ar->item) && (!ar->item2 || !player->HasItemCount(ar->item2)))
                        lockData = LFG_LOCKSTATUS_MISSING_ITEM;
                }
                else if (ar->item2 && !player->HasItemCount(ar->item2))
                    lockData = LFG_LOCKSTATUS_MISSING_ITEM;
        }

        /* @todo VoA closed if WG is not under team control (LFG_LOCKSTATUS_RAID_LOCKED)
        lockData = LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE;
        lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_LOW_LEVEL;
        lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_HIGH_LEVEL;
        */

        if (lockData)
            lock[dungeon->Entry()] = lockData;
    }

    return lock;
}

uint8 LFGMgr::GetKicksLeft(ObjectGuid guid)
{
    uint8 kicks = GroupsStore[guid].GetKicksLeft();
    TC_LOG_TRACE("lfg.data.group.kickleft.get", "Group: %s, Kicks left: %u", guid.ToString().c_str(), kicks);
    return kicks;
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

void LFGMgr::SetVoteKick(ObjectGuid gguid, bool active)
{
    ASSERT(gguid.IsGroup());

    LfgGroupData& data = GroupsStore[gguid];
    TC_LOG_TRACE("lfg.data.group.votekick.set", "Group: %s, New state: %d, Previous: %d",
        gguid.ToString().c_str(), active, data.IsVoteKickActive());

    data.SetVoteKick(active);
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

void LFGMgr::SetSelectedDungeons(ObjectGuid guid, LfgDungeonSet const& dungeons)
{
    TC_LOG_TRACE("lfg.data.player.dungeon.selected.set", "Player: %s, Dungeons: %s", guid.ToString().c_str(), ConcatenateDungeons(dungeons).c_str());
    PlayersStore[guid].SetSelectedDungeons(dungeons);
}

void LFGMgr::DecreaseKicksLeft(ObjectGuid guid)
{
    GroupsStore[guid].DecreaseKicksLeft();
    TC_LOG_TRACE("lfg.data.group.kicksleft.decrease", "Group: %s, Kicks: %u", guid.ToString().c_str(), GroupsStore[guid].GetKicksLeft());
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

    LfgState state = GetState(guid);
    // If group is being formed after proposal success do nothing more
    GuidSet const& players = it->second.GetPlayers();
    for (ObjectGuid playerGUID : players)
    {
        SetGroup(playerGUID, ObjectGuid::Empty);
        if (state != LFG_STATE_PROPOSAL)
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

bool LFGMgr::AllQueued(GuidList const& check)
{
    if (check.empty())
        return false;

    for (GuidList::const_iterator it = check.begin(); it != check.end(); ++it)
    {
        LfgState state = GetState(*it);
        if (state != LFG_STATE_QUEUED)
        {
            if (state != LFG_STATE_PROPOSAL)
                TC_LOG_DEBUG("lfg.allqueued", "Unexpected state found while trying to form new group. Guid: %s, State: %s", (*it).ToString().c_str(), GetStateString(state).c_str());

            return false;
        }
    }
    return true;
}

bool LFGMgr::isOptionEnabled(uint32 option)
{
    return (m_options & option) != 0;
}

uint32 LFGMgr::GetOptions()
{
    return m_options;
}

void LFGMgr::SetOptions(uint32 options)
{
    m_options = options;
}

bool LFGMgr::IsSeasonActive(uint32 dungeonId)
{
    switch (dungeonId)
    {
        case 285: // The Headless Horseman
            return IsHolidayActive(HOLIDAY_HALLOWS_END);
        case 286: // The Frost Lord Ahune
            return IsHolidayActive(HOLIDAY_FIRE_FESTIVAL);
        case 287: // Coren Direbrew
            return IsHolidayActive(HOLIDAY_BREWFEST);
        case 288: // The Crown Chemical Co.
            return IsHolidayActive(HOLIDAY_LOVE_IS_IN_THE_AIR);
    }
    return false;
}

void LFGMgr::SetupGroupMember(ObjectGuid guid, ObjectGuid gguid)
{
    LfgDungeonSet dungeons;
    dungeons.insert(GetDungeon(gguid));
    SetSelectedDungeons(guid, dungeons);
    SetState(guid, GetState(gguid));
    SetGroup(guid, gguid);
    AddPlayerToGroup(gguid, guid);
}

bool LFGMgr::selectedRandomLfgDungeon(ObjectGuid guid)
{
    if (GetState(guid) != LFG_STATE_NONE)
    {
        LfgDungeonSet const& dungeons = GetSelectedDungeons(guid);
        if (!dungeons.empty())
        {
             LFGDungeonData const* dungeon = GetLFGDungeon(*dungeons.begin());
             if (dungeon && (dungeon->type == LFG_TYPE_RANDOM || dungeon->seasonal))
                 return true;
        }
    }

    return false;
}

bool LFGMgr::inLfgDungeonMap(ObjectGuid guid, uint32 map, Difficulty difficulty)
{
    if (!guid.IsGroup())
        guid = GetGroup(guid);

    if (uint32 dungeonId = GetDungeon(guid, true))
        if (LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId))
            if (uint32(dungeon->map) == map && dungeon->difficulty == difficulty)
                return true;

    return false;
}

uint32 LFGMgr::GetLFGDungeonEntry(uint32 id)
{
    if (id)
        if (LFGDungeonData const* dungeon = GetLFGDungeon(id))
            return dungeon->Entry();

    return 0;
}

LfgDungeonSet LFGMgr::GetRandomAndSeasonalDungeons(uint8 level, uint8 expansion)
{
    LfgDungeonSet randomDungeons;
    for (lfg::LFGDungeonContainer::const_iterator itr = LfgDungeonStore.begin(); itr != LfgDungeonStore.end(); ++itr)
    {
        lfg::LFGDungeonData const& dungeon = itr->second;
        if ((dungeon.type == lfg::LFG_TYPE_RANDOM || (dungeon.seasonal && sLFGMgr->IsSeasonActive(dungeon.id)))
            && dungeon.expansion <= expansion && dungeon.minlevel <= level && level <= dungeon.maxlevel)
            randomDungeons.insert(dungeon.Entry());
    }
    return randomDungeons;
}

} // namespace lfg
