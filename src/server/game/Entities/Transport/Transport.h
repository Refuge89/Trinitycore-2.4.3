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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"
#include "TransportMgr.h"

struct CreatureData;

class TC_GAME_API Transport : public GameObject
{
        friend Transport* TransportMgr::CreateTransport(uint32, ObjectGuid::LowType, Map*);

        Transport();
    public:
        typedef std::set<WorldObject*> PassengerSet;

        ~Transport();

        bool Create(ObjectGuid::LowType guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress);
        void CleanupsBeforeDelete(bool finalCleanup = true) override;

        void Update(uint32 diff) override;
        void DelayedUpdate(uint32 diff);

        void BuildUpdate(UpdateDataMapType& data_map) override;

        void AddPassenger(WorldObject* passenger);
        void RemovePassenger(WorldObject* passenger);
        PassengerSet const& GetPassengers() const { return _passengers; }

        Creature* CreateNPCPassenger(ObjectGuid::LowType guid, CreatureData const* data);
        GameObject* CreateGOPassenger(ObjectGuid::LowType guid, GameObjectData const* data);

        /**
        * @fn bool Transport::SummonPassenger(uint64, Position const&, TempSummonType, SummonPropertiesEntry const*, uint32, Unit*, uint32, uint32)
        *
        * @brief Temporarily summons a creature as passenger on this transport.
        *
        * @param entry Id of the creature from creature_template table
        * @param pos Initial position of the creature (transport offsets)
        * @param summonType
        * @param properties
        * @param duration Determines how long the creauture will exist in world depending on @summonType (in milliseconds)
        * @param summoner Summoner of the creature (for AI purposes)
        * @param spellId
        *
        * @return Summoned creature.
        */
        TempSummon* SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties = nullptr, uint32 duration = 0, Unit* summoner = nullptr, uint32 spellId = 0);

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const
        {
            CalculatePassengerPosition(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const
        {
            CalculatePassengerOffset(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        uint32 GetTransportPeriod() const override { return GetUInt32Value(GAMEOBJECT_LEVEL); }
        void SetPeriod(uint32 period) { SetUInt32Value(GAMEOBJECT_LEVEL, period); }
        uint32 GetTimer() const { return GetGOValue()->Transport.PathProgress; }

        KeyFrameVec const& GetKeyFrames() const { return _transportInfo->keyFrames; }

        void UpdatePosition(float x, float y, float z, float o);

        //! Needed when transport moves from inactive to active grid
        void LoadStaticPassengers();

        //! Needed when transport enters inactive grid
        void UnloadStaticPassengers();

        void EnableMovement(bool enabled);

        void SetDelayedAddModelToMap() { _delayedAddModel = true; }

        TransportTemplate const* GetTransportTemplate() const { return _transportInfo; }

    protected:
        static void CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
        {
            float inx = x, iny = y, inz = z;
            if (o)
                *o = Position::NormalizeOrientation(transO + *o);

            x = transX + inx * std::cos(transO) - iny * std::sin(transO);
            y = transY + iny * std::cos(transO) + inx * std::sin(transO);
            z = transZ + inz;
        }

        static void CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
        {
            if (o)
                *o = Position::NormalizeOrientation(*o - transO);

            z -= transZ;
            y -= transY;    // y = searchedY * std::cos(o) + searchedX * std::sin(o)
            x -= transX;    // x = searchedX * std::cos(o) + searchedY * std::sin(o + pi)
            float inx = x, iny = y;
            y = (iny - inx * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
            x = (inx + iny * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
        }

    private:
        void MoveToNextWaypoint();
        float CalculateSegmentPos(float perc);
        bool TeleportTransport(uint32 newMapid, float x, float y, float z, float o);
        void DelayedTeleportTransport();
        void UpdatePassengerPositions(PassengerSet& passengers);
        void DoEventIfAny(KeyFrame const& node, bool departure);

        //! Helpers to know if stop frame was reached
        bool IsMoving() const { return _isMoving; }
        void SetMoving(bool val) { _isMoving = val; }

        TransportTemplate const* _transportInfo;

        KeyFrameVec::const_iterator _currentFrame;
        KeyFrameVec::const_iterator _nextFrame;
        TimeTrackerSmall _positionChangeTimer;
        bool _isMoving;
        bool _pendingStop;

        //! These are needed to properly control events triggering only once for each frame
        bool _triggeredArrivalEvent;
        bool _triggeredDepartureEvent;

        PassengerSet _passengers;
        PassengerSet::iterator _passengerTeleportItr;
        PassengerSet _staticPassengers;

        bool _delayedAddModel;
        bool _delayedTeleport;
};

#endif
