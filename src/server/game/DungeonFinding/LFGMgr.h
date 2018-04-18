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

#ifndef _LFGMGR_H
#define _LFGMGR_H

#include "Common.h"
#include "DatabaseEnvFwd.h"
#include "LFG.h"
#include "LFGGroupData.h"
#include "LFGPlayerData.h"

class Group;
class Player;
class Quest;
class Map;

namespace lfg
{

enum LfgOptions
{
    LFG_OPTION_ENABLE_DUNGEON_FINDER             = 0x01,
    LFG_OPTION_ENABLE_RAID_BROWSER               = 0x02
};

enum LfgMode
{
    LFG_MODE_GROUP                               = 0,
    LFG_MODE_MORE                                = 1
};

/// Determines the type of instance
enum LfgType
{
    LFG_TYPE_NONE                                = 0,
    LFG_TYPE_DUNGEON                             = 1,
    LFG_TYPE_RAID                                = 2,
    LFG_TYPE_ZONE                                = 4,
    LFG_TYPE_HEROIC                              = 5
};

typedef std::map<ObjectGuid, LfgGroupData> LfgGroupDataContainer;
typedef std::map<ObjectGuid, LfgPlayerData> LfgPlayerDataContainer;

class TC_GAME_API LFGMgr
{
    private:
        LFGMgr();
        ~LFGMgr();

    public:
        static LFGMgr* instance();

        // Multiple files
        /// Get selected dungeons
        LfgDungeonSet GetSelectedDungeons(ObjectGuid guid);
        /// Get current lfg state
        LfgState GetState(ObjectGuid guid);
        /// Get current dungeon
        uint32 GetDungeon(ObjectGuid guid, bool asId = true);

        // cs_lfg
        /// Get current player comment (used for LFR)
        std::string const& GetComment(ObjectGuid gguid);
        /// Gets current lfg options
        uint32 GetOptions();
        /// Sets new lfg options
        void SetOptions(uint32 options);
        /// Checks if given lfg option is enabled
        bool isOptionEnabled(uint32 option);

        // LFGScripts
        /// Get leader of the group (using internal data)
        ObjectGuid GetLeader(ObjectGuid guid);
        /// Sets player team
        void SetTeam(ObjectGuid guid, uint8 team);
        /// Sets player group
        void SetGroup(ObjectGuid guid, ObjectGuid group);
        /// Gets player group
        ObjectGuid GetGroup(ObjectGuid guid);
        /// Sets the leader of the group
        void SetLeader(ObjectGuid gguid, ObjectGuid leader);
        /// Removes saved group data
        void RemoveGroupData(ObjectGuid guid);
        /// Removes a player from a group
        uint8 RemovePlayerFromGroup(ObjectGuid gguid, ObjectGuid guid);
        /// Adds player to group
        void AddPlayerToGroup(ObjectGuid gguid, ObjectGuid guid);

        // LFGHandler
        /// Sets player lfg comment
        void SetComment(ObjectGuid guid, std::string const& comment);
        void SetAutoJoin(ObjectGuid guid, bool autoJoin);
        void SetAutoFill(ObjectGuid guid, bool autoFill);
        void SetDungeon(ObjectGuid guid, uint32 dungeon);
        void SetSelectedDungeons(ObjectGuid guid, uint32 slot, uint32 dungeon);
        void SetState(ObjectGuid guid, LfgState state);
        /// Join Lfg with selected dungeons and comment
        void JoinLfg(Player* player, uint8 lfgMode);
        /// Leaves lfg
        void LeaveLfg(ObjectGuid guid, bool disconnected = false);

        void HandleLfgInfo(Player* player, uint32 type, uint32 entry);
        void SendLfgInfo(Player* player, uint32 type, uint32 entry, uint8 lfgMode);

        /// Get last lfg state (NONE, DUNGEON or FINISHED_DUNGEON)
        LfgState GetOldState(ObjectGuid guid);
        /// Check if given group guid is lfg
        bool IsLfgGroup(ObjectGuid guid);
        /// Gets the player count of given group
        uint8 GetPlayerCount(ObjectGuid guid);
        /// Checks if given players are ignoring each other
        static bool HasIgnore(ObjectGuid guid1, ObjectGuid guid2);

    private:
        uint8 GetTeam(ObjectGuid guid);
        void RestoreState(ObjectGuid guid, char const* debugMsg);
        void ClearState(ObjectGuid guid, char const* debugMsg);
        void RemovePlayerData(ObjectGuid guid);

        GuidSet const& GetPlayers(ObjectGuid guid);

        // General variables
        uint32 _options;                                  ///< Stores config options
        LfgPlayerDataContainer PlayersStore;               ///< Player data
        LfgGroupDataContainer GroupsStore;                 ///< Group data
};

} // namespace lfg

#define sLFGMgr lfg::LFGMgr::instance()
#endif
