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

#include "LFG.h"
#include "LFGGroupData.h"

namespace lfg
{

LfgGroupData::LfgGroupData(): _state(LFG_STATE_NONE), _oldState(LFG_STATE_NONE),
    _leader(), _dungeon(0)
{ }

LfgGroupData::~LfgGroupData()
{ }

bool LfgGroupData::IsLfgGroup()
{
    return _oldState != LFG_STATE_NONE;
}

void LfgGroupData::SetState(LfgState state)
{
    switch (state)
    {
        case LFG_STATE_NONE:
            _dungeon = 0;
        case LFG_STATE_FINISHED_DUNGEON:
        case LFG_STATE_DUNGEON:
            _oldState = state;
            // No break on purpose
        default:
            _state = state;
    }
}

void LfgGroupData::RestoreState()
{
    _state = _oldState;
}

void LfgGroupData::AddPlayer(ObjectGuid guid)
{
    _players.insert(guid);
}

uint8 LfgGroupData::RemovePlayer(ObjectGuid guid)
{
    GuidSet::iterator it = _players.find(guid);
    if (it != _players.end())
        _players.erase(it);
    return uint8(_players.size());
}

void LfgGroupData::RemoveAllPlayers()
{
    _players.clear();
}

void LfgGroupData::SetLeader(ObjectGuid guid)
{
    _leader = guid;
}

void LfgGroupData::SetAutoFill(bool autoFill)
{
    _autoFill = autoFill;
}

void LfgGroupData::SetDungeon(uint32 dungeon)
{
    _dungeon = dungeon;
}

LfgState LfgGroupData::GetState() const
{
    return _state;
}

LfgState LfgGroupData::GetOldState() const
{
    return _oldState;
}

GuidSet const& LfgGroupData::GetPlayers() const
{
    return _players;
}

uint8 LfgGroupData::GetPlayerCount() const
{
    return _players.size();
}

ObjectGuid LfgGroupData::GetLeader() const
{
    return _leader;
}

uint32 LfgGroupData::GetDungeon(bool asId /* = true */) const
{
    if (asId)
        return (_dungeon & 0xFFFF);
    else
        return _dungeon;
}

bool LfgGroupData::IsAutoFill() const
{
    return _autoFill;
}

} // namespace lfg
