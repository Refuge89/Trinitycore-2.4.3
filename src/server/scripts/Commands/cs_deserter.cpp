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

/**
* @file cs_deserter.cpp
* @brief .deserter related commands
*
* This file contains the CommandScripts for all deserter sub-commands
*/

#include "ScriptMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "RBAC.h"
#include "SpellAuras.h"

enum Spells
{
    BG_SPELL_DESERTER = 26013
};

class deserter_commandscript : public CommandScript
{
public:
    deserter_commandscript() : CommandScript("deserter_commandscript") { }

    /**
    * @brief Returns the command structure for the system.
    */

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> deserterBGCommandTable =
        {
            { "add",      rbac::RBAC_PERM_COMMAND_DESERTER_BG_ADD,    false, &HandleDeserterAdd,    "" },
            { "remove",   rbac::RBAC_PERM_COMMAND_DESERTER_BG_REMOVE, false, &HandleDeserterRemove, "" },
        };

        static std::vector<ChatCommand> deserterCommandTable =
        {
            { "bg",       rbac::RBAC_PERM_COMMAND_DESERTER_BG,       false, nullptr, "", deserterBGCommandTable },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "deserter", rbac::RBAC_PERM_COMMAND_DESERTER, false, nullptr, "", deserterCommandTable },
        };
        return commandTable;
    }

    /**
    * @brief Applies the Deserter Debuff to a player
    *
    * This function applies a Deserter Debuff to the
    * selected player, with the provided duration in seconds.
    *
    * @param handler The ChatHandler, passed by the system.
    * @param args The provided duration in seconds.
    *
    * @return true if everything was correct, false if an error occured.
    *
    * Example Usage:
    * @code
    * .deserter bg add 3600 (one hour)
    * @endcode
    */
    static bool HandleDeserterAdd(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }
        char* timeStr = strtok((char*)args, " ");
        if (!timeStr)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 time = atoi(timeStr);

        if (!time)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Aura* aura = player->AddAura(BG_SPELL_DESERTER, player);

        if (!aura)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        aura->SetDuration(time * IN_MILLISECONDS);

        return true;
    }

    /**
    * @brief Removes the Deserter Debuff from a player
    *
    * This function removes a Deserter Debuff from the
    * selected player.
    *
    * @param handler The ChatHandler, passed by the system.
    * @param args Should be nothing.
    *
    * @return true if everything was correct, false if an error occured.
    *
    * Example Usage:
    * @code
    * .deserter bg remove
    * @endcode
    */
    static bool HandleDeserterRemove(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->RemoveAura(BG_SPELL_DESERTER);

        return true;
    }
};

void AddSC_deserter_commandscript()
{
    new deserter_commandscript();
}
