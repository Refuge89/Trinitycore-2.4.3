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

#include "LFGPlayerData.h"

namespace lfg
{

LfgPlayerData::LfgPlayerData(): _state(LFG_STATE_NONE), _oldState(LFG_STATE_NONE),
    _team(0), _group(), _comment("")
{
    memset(_selectedDungeons, 0, sizeof(_selectedDungeons));
}

LfgPlayerData::~LfgPlayerData() { }

void LfgPlayerData::SetState(LfgState state)
{
    if (state == LFG_STATE_NONE)
    {
        memset(_selectedDungeons, 0, sizeof(_selectedDungeons));
        _oldState = state;
        _state = state;
    }
}

void LfgPlayerData::RestoreState()
{
    if (_oldState == LFG_STATE_NONE)
        memset(_selectedDungeons, 0, sizeof(_selectedDungeons));

    _state = _oldState;
}

void LfgPlayerData::SetTeam(uint8 team)
{
    _team = team;
}

void LfgPlayerData::SetGroup(ObjectGuid group)
{
    _group = group;
}

void LfgPlayerData::SetAutoJoin(bool autoJoin)
{
    _autoJoin = autoJoin;
}

void LfgPlayerData::SetAutoFill(bool autoFill)
{
    _autoFill = autoFill;
}

void LfgPlayerData::SetComment(std::string const& comment)
{
    _comment = comment;
}

void LfgPlayerData::SetSelectedDungeons(uint32 slot, uint32 dungeon)
{
    _selectedDungeons[slot] = dungeon;
}

LfgState LfgPlayerData::GetState() const
{
    return _state;
}

LfgState LfgPlayerData::GetOldState() const
{
    return _oldState;
}

uint8 LfgPlayerData::GetTeam() const
{
    return _team;
}

ObjectGuid LfgPlayerData::GetGroup() const
{
    return _group;
}

bool LfgPlayerData::IsAutoJoin() const
{
    return _autoJoin;
}

bool LfgPlayerData::IsAutoFill() const
{
    return _autoFill;
}

std::string const& LfgPlayerData::GetComment() const
{
    return _comment;
}

LfgDungeonSet LfgPlayerData::GetSelectedDungeons() const
{
    LfgDungeonSet dungeons;
    for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        dungeons.insert(_selectedDungeons[i]);

    return dungeons;
}

bool LfgPlayerData::HasDungeonSelected(uint32 dungeon) const
{
    for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        if (_selectedDungeons[i] == dungeon)
            return true;

    return false;
}

} // namespace lfg
