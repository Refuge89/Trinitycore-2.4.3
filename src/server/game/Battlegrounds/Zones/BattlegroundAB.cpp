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

#include "BattlegroundAB.h"
#include "BattlegroundMgr.h"
#include "Creature.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "Util.h"
#include "WorldPacket.h"
#include "WorldSession.h"

void BattlegroundABScore::BuildObjectivesBlock(WorldPacket& data)
{
    data << uint32(2);
    data << uint32(BasesAssaulted);
    data << uint32(BasesDefended);
}

BattlegroundAB::BattlegroundAB()
{
    _isInformedNearVictory = false;
    _buffChange = true;
    BgObjects.resize(BG_AB_OBJECT_MAX);
    BgCreatures.resize(BG_AB_ALL_NODES_COUNT + 5);//+5 for aura triggers

    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        _nodes[i] = 0;
        _prevNodes[i] = 0;
        _nodeTimers[i] = 0;
        _bannerTimers[i].timer = 0;
        _bannerTimers[i].type = 0;
        _bannerTimers[i].teamIndex = 0;
    }

    for (uint8 i = 0; i < BG_TEAMS_COUNT; ++i)
    {
        _lastTick[i] = 0;
        _honorScoreTics[i] = 0;
        _reputationScoreTics[i] = 0;
    }

    _honorTics = 0;
    _reputationTics = 0;
}

BattlegroundAB::~BattlegroundAB() { }

void BattlegroundAB::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        int team_points[BG_TEAMS_COUNT] = { 0, 0 };

        for (int node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        {
            // 3 sec delay to spawn new banner instead previous despawned one
            if (_bannerTimers[node].timer)
            {
                if (_bannerTimers[node].timer > diff)
                    _bannerTimers[node].timer -= diff;
                else
                {
                    _bannerTimers[node].timer = 0;
                    CreateBanner(node, _bannerTimers[node].type, _bannerTimers[node].teamIndex, false);
                }
            }

            // 1-minute to occupy a node from contested state
            if (_nodeTimers[node])
            {
                if (_nodeTimers[node] > diff)
                    _nodeTimers[node] -= diff;
                else
                {
                    _nodeTimers[node] = 0;
                    // Change from contested to occupied !
                    uint8 teamIndex = _nodes[node] - 1;
                    _prevNodes[node] = _nodes[node];
                    _nodes[node] += 2;
                    // burn current contested banner
                    DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex);
                    // create new occupied banner
                    CreateBanner(node, BG_AB_NODE_TYPE_OCCUPIED, teamIndex, true);
                    SendNodeUpdate(node);
                    NodeOccupied(node, (teamIndex == TEAM_ALLIANCE) ? ALLIANCE : HORDE);
                    // Message to chatlog

                    if (teamIndex == TEAM_ALLIANCE)
                    {
                        SendBroadcastText(ABNodes[node].TextAllianceTaken, CHAT_MSG_BG_SYSTEM_ALLIANCE);
                        PlaySoundToAll(BG_AB_SOUND_NODE_CAPTURED_ALLIANCE);
                    }
                    else
                    {
                        SendBroadcastText(ABNodes[node].TextHordeTaken, CHAT_MSG_BG_SYSTEM_HORDE);
                        PlaySoundToAll(BG_AB_SOUND_NODE_CAPTURED_HORDE);
                    }
                }
            }

            for (int team = 0; team < BG_TEAMS_COUNT; ++team)
                if (_nodes[node] == team + BG_AB_NODE_TYPE_OCCUPIED)
                    ++team_points[team];
        }

        // Accumulate points
        for (int team = 0; team < BG_TEAMS_COUNT; ++team)
        {
            int points = team_points[team];
            if (!points)
                continue;

            _lastTick[team] += diff;

            if (_lastTick[team] > BG_AB_TickIntervals[points])
            {
                _lastTick[team] -= BG_AB_TickIntervals[points];
                _teamScores[team] += BG_AB_TickPoints[points];
                _honorScoreTics[team] += BG_AB_TickPoints[points];
                _reputationScoreTics[team] += BG_AB_TickPoints[points];

                if (_reputationScoreTics[team] >= _reputationTics)
                {
                    (team == TEAM_ALLIANCE) ? RewardReputationToTeam(509, 10, ALLIANCE) : RewardReputationToTeam(510, 10, HORDE);
                    _reputationScoreTics[team] -= _reputationTics;
                }

                if (_honorScoreTics[team] >= _honorTics)
                {
                    RewardHonorToTeam(GetBonusHonorFromKill(1), (team == TEAM_ALLIANCE) ? ALLIANCE : HORDE);
                    _honorScoreTics[team] -= _honorTics;
                }

                if (!_isInformedNearVictory && _teamScores[team] > BG_AB_WARNING_NEAR_VICTORY_SCORE)
                {
                    if (team == TEAM_ALLIANCE)
                        SendBroadcastText(BG_AB_TEXT_ALLIANCE_NEAR_VICTORY, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    else
                        SendBroadcastText(BG_AB_TEXT_HORDE_NEAR_VICTORY, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    PlaySoundToAll(BG_AB_SOUND_NEAR_VICTORY);
                    _isInformedNearVictory = true;
                }

                if (_teamScores[team] > BG_AB_MAX_TEAM_SCORE)
                    _teamScores[team] = BG_AB_MAX_TEAM_SCORE;

                if (team == TEAM_ALLIANCE)
                    UpdateWorldState(BG_AB_OP_RESOURCES_ALLY, _teamScores[team]);
                else
                    UpdateWorldState(BG_AB_OP_RESOURCES_HORDE, _teamScores[team]);
            }
        }

        // Test win condition
        if (_teamScores[TEAM_ALLIANCE] >= BG_AB_MAX_TEAM_SCORE)
            EndBattleground(ALLIANCE);
        else if (_teamScores[TEAM_HORDE] >= BG_AB_MAX_TEAM_SCORE)
            EndBattleground(HORDE);
    }
}

void BattlegroundAB::StartingEventCloseDoors()
{
    // despawn banners, auras and buffs
    for (int obj = BG_AB_OBJECT_BANNER_NEUTRAL; obj < BG_AB_DYNAMIC_NODES_COUNT * 8; ++obj)
        SpawnBGObject(obj, RESPAWN_ONE_DAY);

    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT * 3; ++i)
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + i, RESPAWN_ONE_DAY);

    // Starting doors
    DoorClose(BG_AB_OBJECT_GATE_A);
    DoorClose(BG_AB_OBJECT_GATE_H);
    SpawnBGObject(BG_AB_OBJECT_GATE_A, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_AB_OBJECT_GATE_H, RESPAWN_IMMEDIATELY);

    // Starting base spirit guides
    NodeOccupied(BG_AB_SPIRIT_ALIANCE, ALLIANCE);
    NodeOccupied(BG_AB_SPIRIT_HORDE, HORDE);
}

void BattlegroundAB::StartingEventOpenDoors()
{
    // spawn neutral banners
    for (int banner = BG_AB_OBJECT_BANNER_NEUTRAL, i = 0; i < 5; banner += 8, ++i)
        SpawnBGObject(banner, RESPAWN_IMMEDIATELY);

    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        //randomly select buff to spawn
        uint8 buff = urand(0, 2);
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + buff + i * 3, RESPAWN_IMMEDIATELY);
    }

    DoorOpen(BG_AB_OBJECT_GATE_A);
    DoorOpen(BG_AB_OBJECT_GATE_H);
}

void BattlegroundAB::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID().GetCounter()] = new BattlegroundABScore(player->GetGUID());
}

void BattlegroundAB::RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/)
{
}

void BattlegroundAB::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (trigger)
    {
        case 3948:                                          // Arathi Basin Alliance Exit.
            if (player->GetTeam() != ALLIANCE)
                player->GetSession()->SendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                player->LeaveBattleground();
            break;
        case 3949:                                          // Arathi Basin Horde Exit.
            if (player->GetTeam() != HORDE)
                player->GetSession()->SendAreaTriggerMessage("Only The Horde can use that portal");
            else
                player->LeaveBattleground();
            break;
        case 3866:                                          // Stables
        case 3869:                                          // Gold Mine
        case 3867:                                          // Farm
        case 3868:                                          // Lumber Mill
        case 3870:                                          // Black Smith
        case 4020:                                          // Unk1
        case 4021:                                          // Unk2
        case 4674:                                          // Unk3
            //break;
        default:
            Battleground::HandleAreaTrigger(player, trigger);
            break;
    }
}

/*  type: 0-neutral, 1-contested, 3-occupied
    teamIndex: 0-ally, 1-horde                        */
void BattlegroundAB::CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay)
{
    // Just put it into the queue
    if (delay)
    {
        _bannerTimers[node].timer = 2000;
        _bannerTimers[node].type = type;
        _bannerTimers[node].teamIndex = teamIndex;
        return;
    }

    uint8 obj = node * 8 + type + teamIndex;
    SpawnBGObject(obj, RESPAWN_IMMEDIATELY);

    // handle aura with banner
    if (!type)
        return;

    obj = node * 8 + ((type == BG_AB_NODE_TYPE_OCCUPIED) ? (5 + teamIndex) : 7);
    SpawnBGObject(obj, RESPAWN_IMMEDIATELY);
}

void BattlegroundAB::DelBanner(uint8 node, uint8 type, uint8 teamIndex)
{
    uint8 obj = node * 8 + type + teamIndex;
    SpawnBGObject(obj, RESPAWN_ONE_DAY);

    // handle aura with banner
    if (!type)
        return;

    obj = node * 8 + ((type == BG_AB_NODE_TYPE_OCCUPIED) ? (5 + teamIndex) : 7);
    SpawnBGObject(obj, RESPAWN_ONE_DAY);
}

void BattlegroundAB::FillInitialWorldStates(WorldPacket& data)
{
    const uint8 plusArray[] = { 0, 2, 3, 0, 1 };

    // Node icons
    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        data << uint32(BG_AB_OP_NODEICONS[node]) << uint32((_nodes[node] == 0) ? 1 : 0);

    // Node occupied states
    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        for (uint8 i = 1; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
            data << uint32(BG_AB_OP_NODESTATES[node] + plusArray[i]) << uint32((_nodes[node] == i) ? 1 : 0);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
    {
        if (_nodes[node] == BG_AB_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (_nodes[node] == BG_AB_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;
    }

    data << uint32(BG_AB_OP_OCCUPIED_BASES_ALLY) << uint32(ally);
    data << uint32(BG_AB_OP_OCCUPIED_BASES_HORDE) << uint32(horde);

    // Team scores
    data << uint32(BG_AB_OP_RESOURCES_MAX) << uint32(BG_AB_MAX_TEAM_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_WARNING) << uint32(BG_AB_WARNING_NEAR_VICTORY_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_ALLY) << uint32(_teamScores[TEAM_ALLIANCE]);
    data << uint32(BG_AB_OP_RESOURCES_HORDE) << uint32(_teamScores[TEAM_HORDE]);

    // other unknown
    data << uint32(0x745) << uint32(0x2);           // 37 1861 unk
}

void BattlegroundAB::SendNodeUpdate(uint8 node)
{
    // Send node owner state update to refresh map icons on client
    const uint8 plusArray[] = { 0, 2, 3, 0, 1 };

    if (_prevNodes[node])
        UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[_prevNodes[node]], 0);
    else
        UpdateWorldState(BG_AB_OP_NODEICONS[node], 0);

    UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[_nodes[node]], 1);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if (_nodes[i] == BG_AB_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (_nodes[i] == BG_AB_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;
    }

    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_ALLY, ally);
    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_HORDE, horde);
}

void BattlegroundAB::NodeOccupied(uint8 node, Team team)
{
    if (!AddSpiritGuide(node, BG_AB_SpiritGuidePos[node], GetTeamIndexByTeamId(team)))
        TC_LOG_ERROR("bg.battleground", "Failed to spawn spirit guide! point: %u, team: %u, ", node, team);

    if (node >= BG_AB_DYNAMIC_NODES_COUNT)//only dynamic nodes, no start points
        return;

    uint8 capturedNodes = 0;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if (_nodes[i] == GetTeamIndexByTeamId(team) + BG_AB_NODE_TYPE_OCCUPIED && !_nodeTimers[i])
            ++capturedNodes;

    if (capturedNodes >= 5)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_5_BASES, team);
    if (capturedNodes >= 4)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_4_BASES, team);

    Creature* trigger = !BgCreatures[node + 7] ? GetBGCreature(node + 7) : nullptr; // 0-6 spirit guides
    if (!trigger)
        trigger = AddCreature(WORLD_TRIGGER, node + 7, BG_AB_NodePositions[node], GetTeamIndexByTeamId(team));

    //add bonus honor aura trigger creature when node is accupied
    //cast bonus aura (+50% honor in 25yards)
    //aura should only apply to players who have accupied the node, set correct faction for trigger
    if (trigger)
        trigger->SetFaction(team == ALLIANCE ? FACTION_ALLIANCE_GENERIC : FACTION_HORDE_GENERIC);
}

void BattlegroundAB::NodeDeOccupied(uint8 node)
{
    //only dynamic nodes, no start points
    if (node >= BG_AB_DYNAMIC_NODES_COUNT)
        return;

    //remove bonus honor aura trigger creature when node is lost
    DelCreature(node + 7);//NULL checks are in DelCreature! 0-6 spirit guides
    RelocateDeadPlayers(BgCreatures[node]);
    DelCreature(node);
    // buff object isn't despawned
}

/* Invoked if a player used a banner as a gameobject */
void BattlegroundAB::EventPlayerClickedOnFlag(Player* source, GameObject* /*target_obj*/)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 node = BG_AB_NODE_STABLES;
    GameObject* obj = GetBgMap()->GetGameObject(BgObjects[node * 8 + 7]);
    while ((node < BG_AB_DYNAMIC_NODES_COUNT) && ((!obj) || (!source->IsWithinDistInMap(obj, 10))))
    {
        ++node;
        obj = GetBgMap()->GetGameObject(BgObjects[node * 8 + BG_AB_OBJECT_AURA_CONTESTED]);
    }

    // this means our player isn't close to any of banners - maybe cheater ??
    if (node == BG_AB_DYNAMIC_NODES_COUNT)
        return;

    TeamId teamIndex = GetTeamIndexByTeamId(source->GetTeam());

    // Check if player really could use this banner, not cheated
    if (!(_nodes[node] == 0 || teamIndex == _nodes[node] % 2))
        return;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    uint32 sound = 0;
    // If node is neutral, change to contested
    if (_nodes[node] == BG_AB_NODE_TYPE_NEUTRAL)
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        _prevNodes[node] = _nodes[node];
        _nodes[node] = teamIndex + 1;
        // burn current neutral banner
        DelBanner(node, BG_AB_NODE_TYPE_NEUTRAL, 0);
        // create new contested banner
        CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
        SendNodeUpdate(node);
        _nodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

        if (teamIndex == TEAM_ALLIANCE)
            SendBroadcastText(ABNodes[node].TextAllianceClaims, CHAT_MSG_BG_SYSTEM_ALLIANCE, source);
        else
            SendBroadcastText(ABNodes[node].TextHordeClaims, CHAT_MSG_BG_SYSTEM_HORDE, source);

        sound = BG_AB_SOUND_NODE_CLAIMED;
    }
    // If node is contested
    else if ((_nodes[node] == BG_AB_NODE_STATUS_ALLY_CONTESTED) || (_nodes[node] == BG_AB_NODE_STATUS_HORDE_CONTESTED))
    {
        // If last state is NOT occupied, change node to enemy-contested
        if (_prevNodes[node] < BG_AB_NODE_TYPE_OCCUPIED)
        {
            UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
            _prevNodes[node] = _nodes[node];
            _nodes[node] = teamIndex + BG_AB_NODE_TYPE_CONTESTED;
            // burn current contested banner
            DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex);
            // create new contested banner
            CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
            SendNodeUpdate(node);
            _nodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

            if (teamIndex == TEAM_ALLIANCE)
                SendBroadcastText(ABNodes[node].TextAllianceAssaulted, CHAT_MSG_BG_SYSTEM_ALLIANCE, source);
            else
                SendBroadcastText(ABNodes[node].TextHordeAssaulted, CHAT_MSG_BG_SYSTEM_HORDE, source);
        }
        // If contested, change back to occupied
        else
        {
            UpdatePlayerScore(source, SCORE_BASES_DEFENDED, 1);
            _prevNodes[node] = _nodes[node];
            _nodes[node] = teamIndex + BG_AB_NODE_TYPE_OCCUPIED;
            // burn current contested banner
            DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex);
            // create new occupied banner
            CreateBanner(node, BG_AB_NODE_TYPE_OCCUPIED, teamIndex, true);
            SendNodeUpdate(node);
            _nodeTimers[node] = 0;
            NodeOccupied(node, (teamIndex == TEAM_ALLIANCE) ? ALLIANCE : HORDE);

            if (teamIndex == TEAM_ALLIANCE)
                SendBroadcastText(ABNodes[node].TextAllianceDefended, CHAT_MSG_BG_SYSTEM_ALLIANCE, source);
            else
                SendBroadcastText(ABNodes[node].TextHordeDefended, CHAT_MSG_BG_SYSTEM_HORDE, source);
        }
        sound = (teamIndex == TEAM_ALLIANCE) ? BG_AB_SOUND_NODE_ASSAULTED_ALLIANCE : BG_AB_SOUND_NODE_ASSAULTED_HORDE;
    }
    // If node is occupied, change to enemy-contested
    else
    {
        UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
        _prevNodes[node] = _nodes[node];
        _nodes[node] = teamIndex + BG_AB_NODE_TYPE_CONTESTED;
        // burn current occupied banner
        DelBanner(node, BG_AB_NODE_TYPE_OCCUPIED, !teamIndex);
        // create new contested banner
        CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
        SendNodeUpdate(node);
        NodeDeOccupied(node);
        _nodeTimers[node] = BG_AB_FLAG_CAPTURING_TIME;

        if (teamIndex == TEAM_ALLIANCE)
            SendBroadcastText(ABNodes[node].TextAllianceAssaulted, CHAT_MSG_BG_SYSTEM_ALLIANCE, source);
        else
            SendBroadcastText(ABNodes[node].TextHordeAssaulted, CHAT_MSG_BG_SYSTEM_HORDE, source);

        sound = (teamIndex == TEAM_ALLIANCE) ? BG_AB_SOUND_NODE_ASSAULTED_ALLIANCE : BG_AB_SOUND_NODE_ASSAULTED_HORDE;
    }

    // If node is occupied again, send "X has taken the Y" msg.
    if (_nodes[node] >= BG_AB_NODE_TYPE_OCCUPIED)
    {
        if (teamIndex == TEAM_ALLIANCE)
            SendBroadcastText(ABNodes[node].TextAllianceTaken, CHAT_MSG_BG_SYSTEM_ALLIANCE);
        else
            SendBroadcastText(ABNodes[node].TextHordeTaken, CHAT_MSG_BG_SYSTEM_HORDE);
    }
    PlaySoundToAll(sound);
}

uint32 BattlegroundAB::GetPrematureWinner()
{
    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if (_nodes[i] == BG_AB_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (_nodes[i] == BG_AB_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;
    }

    if (ally > horde)
        return ALLIANCE;
    else if (horde > ally)
        return HORDE;

    // If the values are equal, fall back to the original result (based on number of players on each team)
    return Battleground::GetPrematureWinner();
}

bool BattlegroundAB::SetupBattleground()
{
    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if (!AddObject(BG_AB_OBJECT_BANNER_NEUTRAL + 8 * i, BG_AB_OBJECTID_NODE_BANNER_0 + i, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_BANNER_CONT_A + 8 * i, BG_AB_OBJECTID_BANNER_CONT_A, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_BANNER_CONT_H + 8 * i, BG_AB_OBJECTID_BANNER_CONT_H, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_BANNER_ALLY + 8 * i, BG_AB_OBJECTID_BANNER_A, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_BANNER_HORDE + 8 * i, BG_AB_OBJECTID_BANNER_H, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_AURA_ALLY + 8 * i, BG_AB_OBJECTID_AURA_A, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_AURA_HORDE + 8 * i, BG_AB_OBJECTID_AURA_H, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_AURA_CONTESTED + 8 * i, BG_AB_OBJECTID_AURA_C, BG_AB_NodePositions[i], 0, 0, std::sin(BG_AB_NodePositions[i].GetOrientation() / 2), std::cos(BG_AB_NodePositions[i].GetOrientation() / 2), RESPAWN_ONE_DAY))
        {
            TC_LOG_ERROR("sql.sql", "BatteGroundAB: Failed to spawn some object Battleground not created!");
            return false;
        }
    }

    if (!AddObject(BG_AB_OBJECT_GATE_A, BG_AB_OBJECTID_GATE_A, BG_AB_DoorPositions[0][0], BG_AB_DoorPositions[0][1], BG_AB_DoorPositions[0][2], BG_AB_DoorPositions[0][3], BG_AB_DoorPositions[0][4], BG_AB_DoorPositions[0][5], BG_AB_DoorPositions[0][6], BG_AB_DoorPositions[0][7], RESPAWN_IMMEDIATELY)
        || !AddObject(BG_AB_OBJECT_GATE_H, BG_AB_OBJECTID_GATE_H, BG_AB_DoorPositions[1][0], BG_AB_DoorPositions[1][1], BG_AB_DoorPositions[1][2], BG_AB_DoorPositions[1][3], BG_AB_DoorPositions[1][4], BG_AB_DoorPositions[1][5], BG_AB_DoorPositions[1][6], BG_AB_DoorPositions[1][7], RESPAWN_IMMEDIATELY))
    {
        TC_LOG_ERROR("sql.sql", "BatteGroundAB: Failed to spawn door object Battleground not created!");
        return false;
    }

    //buffs
    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if (!AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i, Buff_Entries[0], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, std::sin(BG_AB_BuffPositions[i][3] / 2), std::cos(BG_AB_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 1, Buff_Entries[1], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, std::sin(BG_AB_BuffPositions[i][3] / 2), std::cos(BG_AB_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY)
            || !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 2, Buff_Entries[2], BG_AB_BuffPositions[i][0], BG_AB_BuffPositions[i][1], BG_AB_BuffPositions[i][2], BG_AB_BuffPositions[i][3], 0, 0, std::sin(BG_AB_BuffPositions[i][3] / 2), std::cos(BG_AB_BuffPositions[i][3] / 2), RESPAWN_ONE_DAY))
            TC_LOG_ERROR("sql.sql", "BatteGroundAB: Failed to spawn buff object!");
    }

    return true;
}

void BattlegroundAB::Reset()
{
    //call parent's class reset
    Battleground::Reset();

    _teamScores[TEAM_ALLIANCE] = 0;
    _teamScores[TEAM_HORDE] = 0;
    _lastTick[TEAM_ALLIANCE] = 0;
    _lastTick[TEAM_HORDE] = 0;
    _honorScoreTics[TEAM_ALLIANCE] = 0;
    _honorScoreTics[TEAM_HORDE] = 0;
    _reputationScoreTics[TEAM_ALLIANCE] = 0;
    _reputationScoreTics[TEAM_HORDE] = 0;
    _isInformedNearVictory = false;
    bool isBGWeekend = sBattlegroundMgr->IsBGWeekend(GetTypeID());
    _honorTics = (isBGWeekend) ? BG_AB_ABBGWeekendHonorTicks : BG_AB_NotABBGWeekendHonorTicks;
    _reputationTics = (isBGWeekend) ? BG_AB_ABBGWeekendReputationTicks : BG_AB_NotABBGWeekendReputationTicks;

    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        _nodes[i] = 0;
        _prevNodes[i] = 0;
        _nodeTimers[i] = 0;
        _bannerTimers[i].timer = 0;
    }

    for (uint8 i = 0; i < BG_AB_ALL_NODES_COUNT + 5; ++i)//+5 for aura triggers
        if (BgCreatures[i])
            DelCreature(i);
}

void BattlegroundAB::EndBattleground(uint32 winner)
{
    // Win reward
    if (winner == ALLIANCE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);
    if (winner == HORDE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);
    // Complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);
    RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);

    Battleground::EndBattleground(winner);
}

WorldSafeLocsEntry const* BattlegroundAB::GetClosestGraveyard(Player* player)
{
    TeamId teamIndex = GetTeamIndexByTeamId(player->GetTeam());

    // Is there any occupied node for this team?
    std::vector<uint8> nodes;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if (_nodes[i] == teamIndex + 3)
            nodes.push_back(i);

    WorldSafeLocsEntry const* good_entry = nullptr;
    // If so, select the closest node to place ghost on
    if (!nodes.empty())
    {
        float plr_x = player->GetPositionX();
        float plr_y = player->GetPositionY();

        float mindist = 999999.0f;
        for (uint8 i = 0; i < nodes.size(); ++i)
        {
            WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(BG_AB_GraveyardIds[nodes[i]]);
            if (!entry)
                continue;

            float dist = (entry->x - plr_x) * (entry->x - plr_x) + (entry->y - plr_y) * (entry->y - plr_y);
            if (mindist > dist)
            {
                mindist = dist;
                good_entry = entry;
            }
        }
        nodes.clear();
    }
    // If not, place ghost on starting location
    if (!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry(BG_AB_GraveyardIds[teamIndex + 5]);

    return good_entry;
}

bool BattlegroundAB::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    return true;
}
