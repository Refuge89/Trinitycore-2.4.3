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
#include "Log.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"

void WorldSession::HandleLfgJoinOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFG_JOIN %s", GetPlayerInfo().c_str());

    sLFGMgr->SetAutoJoin(GetPlayer()->GetGUID(), true);

    sLFGMgr->JoinLfg(GetPlayer(), lfg::LFG_MODE_GROUP);
}

void WorldSession::HandleLfgLeaveOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFG_LEAVE %s", GetPlayerInfo().c_str());

    sLFGMgr->SetAutoJoin(GetPlayer()->GetGUID(), false);
}

void WorldSession::HandleLfgSetAutoFillOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFM_SET_AUTOFILL %s", GetPlayerInfo().c_str());

    sLFGMgr->SetAutoFill(GetPlayer()->GetGUID(), true);

    sLFGMgr->JoinLfg(GetPlayer(), lfg::LFG_MODE_MORE);
}

void WorldSession::HandleLfgClearAutoFillOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFM_CLEAR_AUTOFILL %s", GetPlayerInfo().c_str());

    sLFGMgr->SetAutoFill(GetPlayer()->GetGUID(), false);
}

void WorldSession::HandleClearLfgOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_CLEAR_LOOKING_FOR_GROUP %s", GetPlayerInfo().c_str());

    sLFGMgr->SetState(GetPlayer()->GetGUID(), lfg::LFG_STATE_NONE);
}

void WorldSession::HandleClearLfmOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_CLEAR_LOOKING_FOR_MORE %s", GetPlayerInfo().c_str());

    if (Group* group = GetPlayer()->GetGroup())
        if (group->GetLeaderGUID() == _player->GetGUID())
            sLFGMgr->SetState(group->GetGUID(), lfg::LFG_STATE_NONE);
}

void WorldSession::HandleLfgSetLfm(WorldPacket& recvData)
{
    uint32 dungeon;
    recvData >> dungeon;

    TC_LOG_DEBUG("lfg", "CMSG_SET_LOOKING_FOR_MORE %s", GetPlayerInfo().c_str());

    if (Group* group = GetPlayer()->GetGroup())
    {
        if (group->GetLeaderGUID() == _player->GetGUID())
        {
            sLFGMgr->SetDungeon(group->GetGUID(), dungeon);
            sLFGMgr->SetState(group->GetGUID(), lfg::LFG_STATE_QUEUED);
        }
    }

    sLFGMgr->JoinLfg(GetPlayer(), lfg::LFG_MODE_MORE);

    uint32 entry = (dungeon & 0xFFFF);
    uint32 type = ((dungeon >> 24) & 0xFFFF);
    sLFGMgr->SendLfgInfo(GetPlayer(), type, entry, lfg::LFG_MODE_MORE);
}

void WorldSession::HandleLfgSetLfg(WorldPacket& recvData)
{
    uint32 slot, dungeon;
    recvData >> slot >> dungeon;

    TC_LOG_DEBUG("lfg", "CMSG_SET_LOOKING_FOR_GROUP %s", GetPlayerInfo().c_str());

    if (slot >= MAX_LOOKING_FOR_GROUP_SLOT)
        return;

    sLFGMgr->SetSelectedDungeons(GetPlayer()->GetGUID(), slot, dungeon);
    sLFGMgr->SetState(GetPlayer()->GetGUID(), lfg::LFG_STATE_QUEUED);
    sLFGMgr->JoinLfg(GetPlayer(), lfg::LFG_MODE_GROUP);

    uint32 entry = (dungeon & 0xFFFF);
    uint32 type = ((dungeon >> 24) & 0xFFFF);
    sLFGMgr->SendLfgInfo(GetPlayer(), type, entry, lfg::LFG_MODE_GROUP);
}

void WorldSession::HandleLfgSetCommentOpcode(WorldPacket& recvData)
{
    std::string comment;
    recvData >> comment;

    TC_LOG_DEBUG("lfg", "CMSG_LFG_SET_COMMENT %s comment: %s", GetPlayerInfo().c_str(), comment.c_str());

    sLFGMgr->SetComment(GetPlayer()->GetGUID(), comment);
}

void WorldSession::HandleLfgInfoOpcode(WorldPacket& recvData)
{
    uint32 type, entry, unk;
    recvData >> type >> entry >> unk;

    TC_LOG_DEBUG("lfg", "MSG_LOOKING_FOR_GROUP %s type: %u entry: %u", GetPlayerInfo().c_str(), type, entry);

    sLFGMgr->HandleLfgInfo(GetPlayer(), type, entry);
}

void WorldSession::SendLfgDisabled()
{
    TC_LOG_DEBUG("lfg", "SMSG_LFG_DISABLED %s", GetPlayerInfo().c_str());
    WorldPacket data(SMSG_LFG_DISABLED, 0);
    SendPacket(&data);
}
