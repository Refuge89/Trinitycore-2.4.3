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

#ifndef _LFGPLAYERDATA_H
#define _LFGPLAYERDATA_H

#include "LFG.h"

namespace lfg
{

/**
    Stores all lfg data needed about the player.
*/
class TC_GAME_API LfgPlayerData
{
    public:
        LfgPlayerData();
        ~LfgPlayerData();

        // General
        void SetState(LfgState state);
        void RestoreState();
        void SetTeam(uint8 team);
        void SetGroup(ObjectGuid group);

        // Queue
        void SetAutoJoin(bool autoJoin);
        void SetAutoFill(bool autoFill);
        void SetComment(std::string const& comment);
        void SetSelectedDungeons(uint32 slot, uint32 dungeon);

        // General
        LfgState GetState() const;
        LfgState GetOldState() const;
        uint8 GetTeam() const;
        ObjectGuid GetGroup() const;

        // Queue
        bool IsAutoJoin() const;
        bool IsAutoFill() const;
        std::string const& GetComment() const;
        LfgDungeonSet GetSelectedDungeons() const;
        bool HasDungeonSelected(uint32 dungeon) const;

    private:
        // General
        LfgState _state;
        LfgState _oldState;
        // Player
        uint8 _team;
        ObjectGuid _group;
        // Queue
        bool _autoJoin;
        bool _autoFill;
        std::string _comment;
        uint32 _selectedDungeons[MAX_LOOKING_FOR_GROUP_SLOT];
};

} // namespace lfg

#endif
