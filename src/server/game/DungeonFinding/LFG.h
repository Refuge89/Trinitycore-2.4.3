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

#ifndef _LFG_H
#define _LFG_H

#include "Define.h"
#include <set>
#include <string>

namespace lfg
{

#define MAX_LOOKING_FOR_GROUP_SLOT 3

enum LfgState
{
    LFG_STATE_NONE,                                 // Not using LFG / LFR
    //LFG_STATE_ROLECHECK,                          // Rolecheck active
    LFG_STATE_QUEUED = 2,                                      // Queued
    //LFG_STATE_PROPOSAL,                                    // Proposal active
    //LFG_STATE_BOOT,                                      // Vote kick active
    LFG_STATE_DUNGEON = 5,                                 // In LFG Group, in a Dungeon
    LFG_STATE_FINISHED_DUNGEON,                            // In LFG Group, in a finished Dungeon
    LFG_STATE_RAIDBROWSER                                  // Using Raid finder
};

typedef std::set<uint32> LfgDungeonSet;

TC_GAME_API std::string ConcatenateDungeons(LfgDungeonSet const& dungeons);
TC_GAME_API std::string GetStateString(LfgState state);

} // namespace lfg

#endif
