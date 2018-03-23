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

/*
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_gen_".
 */

 /* ContentData
 npc_pet_gen_egbert                 100%    Egbert run's around
 npc_pet_gen_mojo                   100%    Mojo follows you when you kiss it
 EndContentData */

#include "ScriptMgr.h"
#include "DBCStructure.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"

enum EgbertMisc
{
    SPELL_EGBERT = 40669,
    EVENT_RETURN = 3
};

class npc_pet_gen_egbert : public CreatureScript
{
    public:
        npc_pet_gen_egbert() : CreatureScript("npc_pet_gen_egbert") {}

        struct npc_pet_gen_egbertAI : public NullCreatureAI
        {
            npc_pet_gen_egbertAI(Creature* creature) : NullCreatureAI(creature)
            {
                if (Unit* owner = me->GetCharmerOrOwner())
                    if (owner->GetMap()->GetEntry()->addon > 1)
                        me->SetCanFly(true);
            }

            void Reset() override
            {
                events.Reset();
                if (Unit* owner = me->GetCharmerOrOwner())
                    me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
            }

            void EnterEvadeMode(EvadeReason why) override
            {
                if (!_EnterEvadeMode(why))
                    return;

                Reset();
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (Unit* owner = me->GetCharmerOrOwner())
                {
                    if (!me->IsWithinDist(owner, 40.f))
                    {
                        me->RemoveAura(SPELL_EGBERT);
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
                    }
                }

                if (me->HasAura(SPELL_EGBERT))
                    events.ScheduleEvent(EVENT_RETURN, urandms(5, 20));

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RETURN:
                            me->RemoveAura(SPELL_EGBERT);
                            break;
                        default:
                            break;
                    }
                }
            }
        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pet_gen_egbertAI(creature);
        }
};

enum Mojo
{
    SAY_MOJO                = 0,

    SPELL_FEELING_FROGGY    = 43906,
    SPELL_SEDUCTION_VISUAL  = 43919
};

class npc_pet_gen_mojo : public CreatureScript
{
    public:
        npc_pet_gen_mojo() : CreatureScript("npc_pet_gen_mojo") { }

        struct npc_pet_gen_mojoAI : public ScriptedAI
        {
            npc_pet_gen_mojoAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                victimGUID.Clear();

                if (Unit* owner = me->GetOwner())
                    me->GetMotionMaster()->MoveFollow(owner, 0.0f, 0.0f);
            }

            void JustEngagedWith(Unit* /*who*/) override { }
            void UpdateAI(uint32 /*diff*/) override { }

            void ReceiveEmote(Player* player, uint32 emote) override
            {
                me->HandleEmoteCommand(emote);
                Unit* owner = me->GetOwner();
                if (emote != TEXT_EMOTE_KISS || !owner || owner->GetTypeId() != TYPEID_PLAYER ||
                    owner->ToPlayer()->GetTeam() != player->GetTeam())
                {
                    return;
                }

                Talk(SAY_MOJO, player);

                if (victimGUID)
                    if (Player* victim = ObjectAccessor::GetPlayer(*me, victimGUID))
                        victim->RemoveAura(SPELL_FEELING_FROGGY);

                victimGUID = player->GetGUID();

                DoCast(player, SPELL_FEELING_FROGGY, true);
                DoCastSelf(SPELL_SEDUCTION_VISUAL, true);
                me->GetMotionMaster()->MoveFollow(player, 0.0f, 0.0f);
            }

        private:
            ObjectGuid victimGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_pet_gen_mojoAI(creature);
        }
};

void AddSC_generic_pet_scripts()
{
    new npc_pet_gen_egbert();
    new npc_pet_gen_mojo();
}
