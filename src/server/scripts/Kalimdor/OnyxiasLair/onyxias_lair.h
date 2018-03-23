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

#ifndef DEF_ONYXIAS_LAIR_H
#define DEF_ONYXIAS_LAIR_H

#include "CreatureAIImpl.h"

#define OnyxiaScriptName "instance_onyxias_lair"
#define DataHeader "OL"

uint32 const EncounterCount     = 1;

enum OLDataTypes
{
    DATA_ONYXIA                 = 0,
};

enum OLData32
{
    DATA_ONYXIA_PHASE           = 0,
    DATA_SHE_DEEP_BREATH_MORE   = 1,
    DATA_MANY_WHELPS_COUNT      = 2
};

enum OLData64
{
    DATA_ONYXIA_GUID            = 0,
    DATA_FLOOR_ERUPTION_GUID    = 1
};

enum OLOnyxiaPhases
{
    PHASE_START                 = 1,
    PHASE_BREATH                = 2,
    PHASE_END                   = 3
};

enum OLCreatureIds
{
    NPC_WHELP                   = 11262,
    NPC_LAIRGUARD               = 36561,
    NPC_ONYXIA                  = 10184,
    NPC_TRIGGER                 = 14495
};

enum OLGameObjectIds
{
    GO_WHELP_SPAWNER            = 176510,
    GO_WHELP_EGG                = 176511
};

template <class AI, class T>
inline AI* GetOnyxiaAI(T* obj)
{
    return GetInstanceAI<AI>(obj, OnyxiaScriptName);
}

#endif
