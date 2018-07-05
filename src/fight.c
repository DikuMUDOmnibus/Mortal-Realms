/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        * 
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <math.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Global functions.
 */
sh_int martial_arts_attack;    /* The attack number for the global MA */

bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace, bool fDisplay ) );

/*
 * Local functions.
 */
bool check_add_attack( CHAR_DATA *, CHAR_DATA * );
bool check_race_war( CHAR_DATA *, CHAR_DATA *);
void add_to_victory_list( CHAR_DATA *, CHAR_DATA *);
bool    check_dodge     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_parry     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
int dt ) );
void    death_cry       args( ( CHAR_DATA *ch ) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     xp_compute      args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
bool    is_safe         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    make_corpse     args( ( CHAR_DATA *ch ) );
void    one_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    raw_kill        args( ( CHAR_DATA *victim ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    trip            args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    backstab        args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void    centaur_hoof    args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void    orc_brawl       args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool    has_mirror      args( (CHAR_DATA *ch, CHAR_DATA *victim) );

bool OGRE_INTIMIDATE;

void    show_party_line      ( CHAR_DATA * );

void check_asn_obj(CHAR_DATA *victim)
  {
  if(!IS_NPC(victim) &&
     victim->fighting!=NULL &&
     victim->fighting->who->asn_obj!=NULL)
    {
    OBJ_DATA *obj;

    obj=victim->fighting->who->asn_obj;
    victim->fighting->who->asn_obj=NULL;
    if(obj->carried_by == victim)
      {
      obj_from_char( obj );
      obj_to_char(obj,victim->fighting->who);
      act("You quickly snatch the $p from $N's failing grasp!",
          victim->fighting->who, obj, victim, TO_CHAR );
      }
    }
  }

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */


void violence_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *victim;
  CHAR_DATA *rch;
  PLAYER_GAME *gpl,*gpl_next;
  char ch_col[81];

  victim=NULL;

  /* Object Program Tick */
  for( gpl = first_player; gpl != NULL; gpl=gpl_next )
    {
    OBJ_DATA *obj;
    OBJ_PROG *prg;
    OBJ_WITH_PROG *with, *with_next;
  
    gpl_next=gpl->next;
    ch = gpl->ch;
    
    if( ch->desc != NULL && ch->obj_with_prog != NULL )
    for(with=ch->obj_with_prog;with!=NULL;with=with_next)
      {
      with_next = with->next;
      if( (obj = with->obj ) == NULL 
	   || obj->pIndexData == NULL )
        continue;
      if( ch->obj_prog_ip==0 && obj->pIndexData->obj_prog!=NULL)
        for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
          if( prg->cmd == -3 )
           /* Let's reduce the rate of calls by 1/2 with 200% range */
      if( number_range(0,200) < prg->percentage )
        {
        open_timer( TIMER_OBJ_PROG );
        start_object_program( ch, obj, prg, "");
        close_timer( TIMER_OBJ_PROG );
        tail_chain();
        }
      }
    }

  for( ch = first_char; ch != NULL; ch = violence_ch_next)
    {
    violence_ch_next = ch->next;

    if( last_dead == ch || ch->name==NULL)
     continue;

    ch->attack=0;

      /* Cludge if they are not in the same room    -  Chaos 4/3/99 */
    if( ch->fighting != NULL && ch->fighting->who != NULL &&
        ch->in_room != ch->fighting->who->in_room )
       {
       stop_fighting( ch, FALSE );
       continue;
       }

      /* Cludge if they are not really fighting  -  Chaos 4/2/99  */
    if( ch->position == POS_FIGHTING && ch->fighting == NULL )
       {
       stop_fighting( ch, FALSE );
       continue;
       }

    if (ch == first_char  && ch->prev)
    {
           bug( "ERROR: first_char->prev != NULL, fixing..." );
           ch->prev = NULL;
    }
        /*
         * Experience gained during battle decreases as battle drags on
        if ( ch->fighting )
          if ( (++ch->fighting->duration % 24) == 0 )
            ch->fighting->xp = ((ch->fighting->xp * 9) / 10); */


 
         /* Fix the mobiles that show up for combat, but live */
    if( IS_NPC( ch )  &&  ch->fighting == NULL && ch->position==POS_STANDING )
      {
      if( IS_SET( ch->pIndexData->affected_by ,  AFF_INVISIBLE ) &&
         !IS_SET( ch->affected_by, AFF_INVISIBLE ) )
        {
        SET_BIT(ch->affected_by, AFF_INVISIBLE );
        }
      if( IS_SET( ch->pIndexData->affected_by ,  AFF_HIDE ) &&
         !IS_SET( ch->affected_by, AFF_HIDE ) )
        {
        SET_BIT(ch->affected_by, AFF_HIDE );
        }
      if( IS_SET( ch->pIndexData->affected_by ,  AFF_STEALTH ) &&
         !IS_SET( ch->affected_by, AFF_STEALTH ) )
        {
        SET_BIT(ch->affected_by, AFF_STEALTH );
        }
      }
    if (is_affected(ch, gsn_nightmare))
    {
     OBJ_DATA * obj;

      if (number_percent() < 5 && (obj = get_eq_char(ch, WEAR_LIGHT)) !=NULL)  
      {
       act( "$n screams and flings $s $p away!", ch, obj, NULL, TO_ROOM);
       switch (number_range(1,3))
       {
       case 1:act( "A large demon leers at you from within the glaring light of your $p!", ch, obj, NULL, TO_CHAR);
              break;
       case 2:act( "A soul rending shriek tears through your rational mind! Your $p vibrates in harmony with it!", ch, obj, NULL, TO_CHAR);
              break;
       case 3:act( "A mental picture of your imminent death emanates from your $p!", ch, obj, NULL, TO_CHAR);
              break;
       }
       act( "You fling your $p away!", ch, obj, NULL, TO_CHAR);
       obj_from_char( obj );
       obj_to_char( obj, ch );
      }
      else if ( number_percent() < 5 && (obj = get_eq_char(ch, WEAR_HOLD ))  !=NULL)
      {
       act( "$n screams and flings $s $p away!", ch, obj, NULL, TO_ROOM);
       switch (number_range(1,3))
       {
        case 1:act( "Your $p starts to slowly drain away your dwindling vitality!", ch, obj, NULL, TO_CHAR);
               break;
        case 2:act( "Your $p moans loudly and starts to switch! It's coming alive!", ch, obj, NULL, TO_CHAR);
               break;
        case 3:act( "A stabbing pain lances through you from your $p!", ch, obj, NULL, TO_CHAR);
               break;
       }
       act( "You fling your $p away!", ch, obj, NULL, TO_CHAR);
       obj_from_char( obj );
       obj_to_char( obj, ch );
      }
      else if ( number_percent() < 5 && (obj = get_eq_char(ch, WEAR_DUAL_WIELD ))  !=NULL)
      {
       act( "$n screams and flings $s $p away!", ch, obj, NULL, TO_ROOM);
       switch (number_range(1,3))
       {
        case 1:act( "Your $p starts to slowly drain away your dwindling vitality!", ch, obj, NULL, TO_CHAR);
               break;
        case 2:act( "Your $p moans loudly and starts to switch! It's coming alive!", ch, obj, NULL, TO_CHAR);
               break;
        case 3:act( "A stabbing pain lances through you from Your $p!", ch, obj, NULL, TO_CHAR);
               break;
       }
       act( "You fling your $p away!", ch, obj, NULL, TO_CHAR);
       obj_from_char( obj );
       obj_to_char( obj, ch );
      }
    }

    if (IS_AFFECTED(ch,AFF2_BLEEDING) )
    {
      AFFECT_DATA *paf;
      int dam = 0;
      ch_col[0]  = '\0';

      if(ch->ansi==1)
        {
        /* determine color of messages */
        if(ch->pcdata->color[4]!=0)
	  sprintf( ch_col, "\033[1;%d;%dm", ch->pcdata->color[0][4],
	           ch->pcdata->color[1][4]);
        }
      act("$tYour life is bleeding out of a nasty wound!",
          ch,ch_col,NULL,TO_CHAR);
      act("$n's life is bleeding out of a nasty wound!",ch,NULL,NULL,TO_ROOM);

        for ( paf = ch->first_affect; paf != NULL; paf = paf->next)
          if(paf->bitvector==AFF2_BLEEDING)
            dam+=paf->modifier;
        if( dam < 1 )
          dam = 1;
        if( ch->critical_hit_by != 0 )
          victim = get_pvnum_index( ch->critical_hit_by );
        else
          victim = NULL;
        if( victim == NULL )
          victim = ch;
	damage(victim,ch,dam,TYPE_NOFIGHT);
     
        if( last_dead == ch )
          continue;
    }

    /* Add poison continuous damage here */
    if( ( IS_NPC( ch ) && ch->npcdata->poison != NULL ) ||
        ( !IS_NPC( ch ) && ch->pcdata->poison != NULL ) )
      {
      POISON_DATA *pd, *npd, *ppd;
      CHAR_DATA *attacker, *fch;
      char buf[MAX_INPUT_LENGTH];
      int dmg;
     
      if( IS_NPC( ch ) )
        pd = ch->npcdata->poison;
      else
        pd = ch->pcdata->poison;
      ppd = NULL;


      if( pd!=NULL )
        {
        npd = pd->next;

        attacker = NULL;
        if( pd->poisoner >= 0 )
          attacker = get_pvnum_index( pd->poisoner );
        if( attacker==NULL )
          attacker = ch;

        if( pd->instant_damage_high > 0 )
          {
	  dmg = number_range( pd->instant_damage_low,pd->instant_damage_high);
          if( ( pd->for_npc && !IS_NPC( ch ) ) ||
              ( !pd->for_npc && IS_NPC( ch ) ) )
            dmg /= 3;

          for( fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room)
            if( fch == ch )
              send_to_char( "You are shocked by poison.\n\r", ch );
            else
            if( fch == attacker )
              {
              sprintf( buf, "%s is shocked by poison for %d hp.\n\r", 
                 IS_NPC(ch)?capitalize(ch->short_descr):capitalize(ch->name),
                 dmg );
              send_to_combat_char( buf, fch );
              }
            else
              {
              sprintf( buf, "%s is shocked by poison.\n\r", 
                 IS_NPC(ch)?capitalize(ch->short_descr):capitalize(ch->name));
              send_to_combat_char( buf, fch );
              }
   

          pd->instant_damage_low = 0;
          pd->instant_damage_high = 0;
 	
          damage(attacker, ch, dmg, TYPE_NOFIGHT);
          }
        else
          {
	  dmg=number_range( pd->constant_damage_low, pd->constant_damage_high);
          if( ( pd->for_npc && !IS_NPC( ch ) ) ||
              ( !pd->for_npc && IS_NPC( ch ) ) )
            dmg /= 3;
          for( fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room)
            if( fch == ch )
              send_to_char( "You shake from poison.\n\r", ch );
            else
            if( fch == attacker )
              {
              sprintf( buf, "%s is poisoned for %d hp.\n\r", 
                 IS_NPC(ch)?capitalize(ch->short_descr):capitalize(ch->name),
                 dmg );
              send_to_combat_char( buf, fch );
              }
            else
              {
              sprintf( buf, "%s shivers from poison.\n\r", 
                 IS_NPC(ch)?capitalize(ch->short_descr):capitalize(ch->name));
              send_to_combat_char( buf, fch );
              }
   
          damage(attacker, ch, dmg, TYPE_NOFIGHT);
          }


        if( pd->constant_duration <= 0 )
          {
          if( ppd == NULL )
            {
            if( IS_NPC( ch ) )
              ch->npcdata->poison = npd;
            else
              ch->pcdata->poison = npd;
            }
          else
            ppd->next = npd;
          DISPOSE( pd );
          pd = NULL;
          }
        else
          {
          pd->constant_duration --;
          ppd = pd;
          }

      pd = npd;
      }
    }

    if( last_dead == ch || ch->name==NULL)
      continue;
    if ( (  victim = who_fighting (ch)  ) == NULL || ch->in_room == NULL)
      continue;

    if( !is_in_room( victim, ch->in_room ) || ch == victim )
      {
      stop_fighting( ch, FALSE );
      continue;
      }

    if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
      {
      if( IS_NPC(ch) || ch->pcdata->auto_flags!=AUTO_QUICK )
        multi_hit( ch, victim, TYPE_UNDEFINED );
        if( last_dead == ch || ch->name==NULL)
          continue;
      set_fighting( ch, victim ); /*This is redundant Martin 19/3/99*/
      }
    else
      stop_fighting( ch, FALSE );

    /*
     * Fun for the whole family!
     */
        if( last_dead == ch || ch->name==NULL)
          continue;
      mprog_hitprcnt_trigger(ch,victim);
        if( last_dead == ch || ch->name==NULL)
          continue;
      mprog_fight_trigger(ch,victim);
        if( last_dead == ch || ch->name==NULL)
          continue;

    for ( rch = ch->in_room->first_person; rch != NULL; rch = violence_rch_next)
      {
      violence_rch_next = rch->next_in_room;

      if ( IS_AWAKE(rch) && rch->fighting == NULL )
        {
        /*  rch assist ch against victim
         * PC's auto-assist others in their group unless PK.
         * Pets always assist their masters, even PK.
	 * Added a line so that if the mobile is grouped with the character
	 * They will assist the character rather than attacking a random 
	 * Opponent. - Martin
         */
        if( !IS_NPC( ch ) )
          if( ( !IS_NPC( rch ) && is_same_group(ch, rch) && IS_NPC(victim)) ||
                ( IS_NPC(rch) && IS_AFFECTED(rch, AFF_CHARM) &&
                         rch->master == ch ) ||
		(IS_NPC(rch) && is_same_group(ch,rch)) )

              if( !IS_AFFECTED(victim, AFF_CHARM))
                 {
                 multi_hit( rch, victim, TYPE_UNDEFINED );
                 continue;
                 }

        /*
         * NPC's assist NPC's of same type or 12.5% chance regardless.
         */
        if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM) &&
             IS_NPC(ch)  && !IS_AFFECTED(ch, AFF_CHARM)  &&
            !IS_AFFECTED( rch, AFF_ETHEREAL ))
          {
          if ( rch->pIndexData == ch->pIndexData ||
               ( (number_bits( 3 ) == 0) && ((rch->level-30)<=ch->level) ) )
            {
            CHAR_DATA *vch;
            CHAR_DATA *target;
            int number;

            target = NULL;
            number = 0;
            for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
              {
              if ( can_see( rch, vch ) &&   is_same_group( vch, victim )
                  &&   number_range( 0, number ) == 0)
                {
                target = vch;
                number++;
                }
              }

            if ( target != NULL )
              multi_hit( rch, target, TYPE_UNDEFINED );
        
            }
          }
        }
      }
    }


  /*   Add Spam's party stat line here    */
  for( gpl = first_player; gpl != NULL; gpl=gpl_next )
    {
    gpl_next = gpl->next;
    if( IS_NPC( gpl->ch ) )
      sub_player( gpl->ch );
    else
      if( gpl->ch->fighting!= NULL && IS_SET(gpl->ch->pcdata->spam, 1024))
          show_party_line( gpl->ch );

     if(  gpl->ch->fighting!= NULL && gpl->ch->position == POS_FIGHTING 
	    &&   gpl->ch->hit > 0
	    &&   gpl->ch->hit <= gpl->ch->wimpy
	    &&   gpl->ch->wait == 0 
            &&   gpl->ch->desc==NULL)
      {
      do_flee( gpl->ch, "" );
      tail_chain();
      }
    }
  return;
}



/*  The SPAM stat line  */
void show_party_line( CHAR_DATA *ch )
  {
  CHAR_DATA *fch;
  char buf[MAX_STRING_LENGTH], buf2[200], buf3[200];

  buf[0]='\0';

  for( fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room )
    if( fch != ch && is_same_group( ch, fch ) )
      {
      strcpy( buf3, !IS_NPC(fch)?capitalize(fch->name):
            capitalize(fch->short_descr));
      buf3[6]='\0';
      sprintf( buf2, "%s:%d/%d  ", buf3, fch->hit, fch->max_hit );
      strcat( buf, buf2 );
      }
  if( buf[0] != '\0' )
    send_to_combat_char( justify( buf ), ch );
  return;
  }
      



/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	int     chance;
	int     level;
        int     cnt;
        int     dual_bonus;

   OGRE_INTIMIDATE=FALSE;
   /* Disallow fighting in different rooms and shops */
    if(victim->in_room!=ch->in_room )
      {
      stop_fighting( ch, FALSE);
      return;
      }

    if(find_keeper( ch ) !=NULL)
    {
     ch_printf(ch, "%s tells you 'No fighting in here! You thug!'\n\r", capitalize(find_keeper(ch)->short_descr));
     ch->reply=find_keeper( ch );
     return;
    }

 if( IS_NPC(victim))
  if( IS_SET( victim->act, ACT_PET) && 
      ((victim->fighting && victim->fighting->who==victim->master) ||
       victim->master==ch))
    {
    raw_kill( victim);
    return;
    }

   if( IS_AFFECTED( victim, AFF_CHARM) && 
     ((victim->fighting && victim->fighting->who==victim->master) ||
       victim->master==ch))
    {
    stop_follower( victim);
    affect_strip( victim, gsn_charm_person );
    }


	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
                stop_fighting( ch, TRUE);
		return;
	}

     if( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) )
       return;

  if( ch->in_room != victim->in_room )
    return;
 
    /*Added Ogre Intimidate here  - Chaos 8/20/98 */
      if( !IS_NPC(ch) && ch->race==RACE_OGRE )
        if( number_range( 1,6 )==1 )
          {
          OGRE_INTIMIDATE=TRUE;
          send_to_combat_char( "You intimidate your opponent.\n\r", ch);
          send_to_combat_char( "You are intimidated by your opponent.\n\r",
               victim);
          }

	one_hit( ch, victim, dt );
	if ( !ch->fighting || ch->fighting->who != victim || 
	     dt == gsn_backstab || last_dead == victim)
		return;

      /* Added Orcish brawling  - Chaos 8/20/98  */
  if( !IS_NPC(ch) && ch->race == RACE_ORC )
    if ( number_percent( ) < 10 )
      {
      orc_brawl( ch, victim );
      if ( !ch->fighting || ch->fighting->who != victim || last_dead == victim )
        return;
      }

      /* Added Centaur Hoof Attack - Presto 4/17/99  */
  if( !IS_NPC(ch) && ch->race == RACE_CENTAUR )
    if ( number_percent( ) < (30 + ch->level/3) )
      {
      centaur_hoof( ch, victim );
      if ( !ch->fighting || ch->fighting->who != victim || last_dead == victim )
        return;
      }

  if( ch->position != POS_FIGHTING && ch->position> POS_STUNNED)
    {
    set_fighting( ch, victim );
    }

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
      dual_bonus = IS_NPC(ch) ? (ch->level / 10) : ch->pcdata->learned[gsn_dual_wield] / 10;
      chance = IS_NPC(ch) ? ch->level : ch->pcdata->learned[gsn_dual_wield];
      if ( number_percent( ) < chance )
        one_hit( ch, victim, dt );
    if ( ch->fighting == NULL || ch->fighting->who != victim || last_dead == victim )
      return;
    }
    else
      dual_bonus = 0;

  cnt = multi(ch, gsn_second_attack);
  if( cnt==-1 && !IS_NPC(ch))
    return;
  level = ch->mclass[multi(ch, gsn_second_attack)];

  if( ch->in_room != victim->in_room )
    return;

  chance=IS_NPC(ch)?ch->level*2:(ch->pcdata->learned[gsn_second_attack]+dual_bonus)*level/50;
  if ( number_percent( ) < chance )
    {
    one_hit( ch, victim, dt );
    if ( ch->fighting == NULL || ch->fighting->who != victim || last_dead == victim )
      return;
    }

  cnt = multi(ch, gsn_third_attack);
  if( cnt==-1 && !IS_NPC(ch))
    return;
  level = ch->mclass[multi(ch, gsn_third_attack)];

  chance=IS_NPC(ch) ? ch->level*2/3 :
           (ch->pcdata->learned[gsn_third_attack]+dual_bonus)*level/120;

  if( ch->in_room != victim->in_room )
    return;

  if ( number_percent( ) < chance )
    {
    one_hit( ch, victim, dt );
    if ( ch->fighting == NULL || ch->fighting->who != victim || last_dead == victim )
      return;
    }

  if( ch->in_room != victim->in_room )
    return;

  cnt = multi(ch, gsn_fourth_attack);
  if( cnt==-1 && !IS_NPC(ch))
    return;
  level = ch->mclass[multi(ch, gsn_fourth_attack)];

chance=IS_NPC(ch)?ch->level/3:(ch->pcdata->learned[gsn_fourth_attack]+dual_bonus)*level/320;
  if ( number_percent( ) < chance )
    one_hit( ch, victim, dt );

  return;
}


bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int hit_roll, int dam_type )
{
	int thac0;
	int diceroll;
        int level;
        int scale;
        int cnt;

  /*   Chaos 10/5/93
   * Calculate to-hit-armor-class-0 versus armor.
   */

      if (dam_type ==  gsn_throw )
          {
	  cnt = multi(ch,gsn_throw);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 13;
          }
      else if (dam_type ==  gsn_kick )
          {
	  cnt = multi(ch,gsn_kick);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 17;
          }
      else if (dam_type ==  gsn_knife )
          {
	  cnt = multi(ch,gsn_knife);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_shoot )
          {
	  cnt = multi(ch,gsn_shoot);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_backstab )
          {
	  cnt = multi(ch,gsn_backstab);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_circle )
          {
	  cnt = multi(ch,gsn_circle);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_gouge )
          {
	  cnt = multi(ch,gsn_gouge);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_trip )
          {
	  cnt = multi(ch,gsn_trip);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else if (dam_type ==  gsn_bash )
          {
	  cnt = multi(ch,gsn_bash);
          if( cnt == -1 )
            return( FALSE );
          level = ch->mclass[ cnt ] + ( ch->level - ch->mclass[cnt] ) / 3;
          scale = 15;
          }
      else   
         {   /* Normal attack here */
          level = ch->level;
          scale = 10;   /* later to be divided by 10 */
	 }
      if ( IS_NPC( victim ) )
        thac0 = mob_armor(victim)- victim->npcdata->armor/3 +
                victim->level * 4 + 2;
      else
	thac0 = 0 - (GET_AC(victim) -100 );

      /* Nice AC bonus for improved invis 8/8/98 Martin */
      if(is_affected(victim, gsn_improved_invis) 
        && !(IS_AFFECTED(ch, AFF_DETECT_INVIS) 
        || is_affected(ch, gsn_truesight)))
         thac0 -= 40;

      if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
  	  thac0 -= 20;

      if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )
  	  thac0 -= 20;

          /* Let's make knife based on dex  -  Chaos  4/23/99  */
      if ( dam_type==gsn_knife )
  	  thac0 += (get_curr_dex(ch)*2 - 35);

      if( OGRE_INTIMIDATE )
          thac0 += (ch->level/10);

      if( victim->position == POS_SLEEPING )
        hit_roll += level/10+5;
      if( victim->position == POS_RESTING )
        hit_roll += level/20+3;

      if( !IS_NPC( ch ) && ch->in_room->sector_type == SECT_ETHEREAL )
        thac0 += 200 - (get_curr_wis( ch ) + get_curr_int( ch )) * 2;
      if( !IS_NPC( ch ) && ch->in_room->sector_type == SECT_ASTRAL )
        thac0 += 100 - get_curr_int( ch ) * 2;

      if( ch->distracted > 0 )
        {
        hit_roll -= level/10+5;
        ch->distracted --;  /* Hey, let's get a bit of our bearings */
        }
      if( victim->distracted > 0 )
        {
        hit_roll += level/7+4;
        victim->distracted --;
        }

      if ( IS_NPC( ch ) )
        diceroll=number_range(0,34+scale*level+hit_roll+GET_HITROLL(ch))+
                       level*2;
      else
        diceroll=number_range(0,39+(scale*level*12)/10
                       +hit_roll+GET_HITROLL(ch))+level*2;

      if ( ch->move<=0 )
	diceroll -= 10;

      if ( !can_see( ch, victim ) )
	diceroll -= 40;

      return( diceroll >= thac0 );
}




/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	OBJ_DATA *wield;
	int dam;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}
	/*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
	if ( victim->position == POS_DEAD  || ch->in_room != victim->in_room )
		return;
    /*
     * Figure out the weapon doing the damage                   
     * Dual wield support -- switch weapons each attack
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
        if ( dual_flip == FALSE )
        {
            dual_flip = TRUE;
            wield = get_eq_char( ch, WEAR_WIELD );
        }
        else
            dual_flip = FALSE;
    }
    else
        wield = get_eq_char( ch, WEAR_WIELD );


    /* Start the cheating check */
      if( IS_NPC( victim ) )
        {
        CHAR_DATA *ch_ld;
        ch_ld = ch;
        if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) && ch->master != NULL )
          ch_ld = ch->master;
        if( ch_ld != NULL && !IS_NPC( ch_ld ) )
          victim->npcdata->pvnum_last_hit = ch_ld->pcdata->pvnum;
        if( ch_ld != NULL )
          {
          if( ch_ld->leader != NULL && !IS_NPC(ch_ld->leader))
            victim->npcdata->pvnum_last_hit_leader = 
                   ch_ld->leader->pcdata->pvnum;
          else
          if( !IS_NPC(ch_ld))
            victim->npcdata->pvnum_last_hit_leader = 
                   ch_ld->pcdata->pvnum;
          }
        }


    /*  Make them a bit tired   */
      if( !IS_NPC( ch ) && ch->move > 0 )
        ch->move--;

	/*
     * Figure out the type of damage message.
     */
	if ( dt == TYPE_UNDEFINED )
	{
		dt = TYPE_HIT;
		if ( wield != NULL && wield->item_type == ITEM_WEAPON )
			dt += wield->value[3];
	}

   /* Use check_hit with no hit_roll modifiers here     - Chaos 17/12/95  */
	if( !check_hit( ch, victim, 0, dt) )
	{
		/* Miss. */

          /* Remove poison from the weapon    -   Chaos 4/20/99  */
        if( wield != NULL && wield->poison != NULL )
          {
          POISON_DATA *npd, *pd;

          for( pd=wield->poison; pd!=NULL; pd=npd )
            {
            npd=pd->next;
            DISPOSE( pd );
            }
          wield->poison = NULL;
          }

	 damage( ch, victim, 0, dt );
	 tail_chain( );
	 return;
	}


         /* Add poison off of item */
    if( wield != NULL && wield->poison != NULL )
     {
     POISON_DATA *npd, *pd;

     if( IS_NPC(ch) || ch->pcdata->pvnum != wield->poison->owner )
       {
       for( pd=wield->poison; pd!=NULL; pd=npd )
         {
         npd=pd->next;
         DISPOSE( pd );
         }
       wield->poison = NULL;
       }
     else
        {
  /*
        npd = wield->poison;
        for( pd=wield->poison; pd != NULL; pd=pd->next )
          {
          if( pd->next != NULL )
            npd = pd->next;
          if( IS_NPC( ch ) )
            pd->poisoner=0;
          else
            pd->poisoner=ch->pcdata->pvnum;
          }
  
        if( IS_NPC( victim) )
          {
          npd->next = victim->npcdata->poison;
          victim->npcdata->poison = wield->poison;
          }
        else
          {
          npd->next = victim->pcdata->poison;
          victim->pcdata->poison = wield->poison;
          }
        wield->poison = NULL;  */

        if( IS_NPC( ch ) )
          wield->poison->poisoner=0;
        else
          wield->poison->poisoner=ch->pcdata->pvnum;

        if( IS_NPC( victim) )
          {
          wield->poison->next = victim->npcdata->poison;
          victim->npcdata->poison = wield->poison;
          }
        else
          {
          wield->poison->next = victim->pcdata->poison;
          victim->pcdata->poison = wield->poison;
          }

        wield->poison = NULL;  

       }
     }
        
   if( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) )
    {
    WAIT_STATE( ch, PULSE_VIOLENCE*2 );
    return;
    } 

	/*
     * Hit.
     * Calc damage.
     */
  dam=0;
	if ( IS_NPC(ch) )
    {
		if((wield != NULL)&&(wield->item_type==ITEM_WEAPON))
			dam = dice( wield->value[1], wield->value[2] ) /2 +
			    dice( ch->npcdata->damnodice, ch->npcdata->damsizedice) +
			    ch->npcdata->damplus;
		else
			dam = dice( ch->npcdata->damnodice, ch->npcdata->damsizedice) + 
			    ch->npcdata->damplus;
    }
  else
	  if((wield != NULL)&&(wield->item_type==ITEM_WEAPON))
		  dam = dice( wield->value[1], wield->value[2] );
	  else
		  dam=number_range(1,4+ch->level/6)+ch->mclass[CLASS_MONK]*2/3;

    /*
     * Bonuses.
     */
dam += GET_DAMROLL(ch);
    if ( !IS_NPC(ch) && multi(ch,gsn_enhanced_damage)!=-1 &&
      ch->pcdata->learned[gsn_enhanced_damage] > 0 )
	dam += dam * ch->pcdata->learned[gsn_enhanced_damage] / 250;
/* was *2    now is *1.33   Chaos 10/10/93  */
if ( !IS_AWAKE(victim) )
	dam = 4*dam/3;

if( !IS_NPC( ch ) && ch->in_room->sector_type == SECT_ETHEREAL )
  dam = 2 * dam / 3;
else
if( !IS_NPC( ch ) && ch->in_room->sector_type == SECT_ASTRAL )
  dam = 2 * dam / 3;

if ( dt == gsn_backstab && (multi(ch, gsn_backstab)!=-1 || IS_NPC(ch)))
  {
  if(!IS_NPC(ch))
    dam = dam*(12 + ch->mclass[multi(ch,gsn_backstab)])/10;/* Chaos 11/3/93 */
  else
    dam = dam*(15 + ch->level/4)/10;
  }
if ( dt == gsn_circle && (multi(ch, gsn_circle)!=-1 || IS_NPC(ch)))
  {
    dam = (dam*(12 + ch->mclass[multi(ch,gsn_circle)])/10)/4;
  }
if ( dt == gsn_knife && (multi(ch, gsn_knife)!=-1 || IS_NPC(ch)))
  {
  if(!IS_NPC(ch))
    {
    dam = dam*(30 + ch->mclass[multi(ch,gsn_knife)])/30;/* Order 11/16/93 */
    /* now check to see if it's a critical knife */
    if (!IS_AFFECTED(victim,AFF2_BLEEDING) )
    if(number_percent() < 25 && multi(ch,gsn_critical_hit) != -1 &&
       number_percent() < ch->pcdata->learned[gsn_critical_hit])
      {
      AFFECT_DATA af;
      char ch_col[81];

      af.type      = gsn_critical_hit;
      af.duration  = 0;
      af.modifier  = number_range(5,ch->mclass[multi(ch,gsn_critical_hit)]/1.5);
      af.location  = APPLY_NONE;
      af.bitvector = AFF2_BLEEDING;
      if( !IS_NPC( ch ) )
        victim->critical_hit_by = ch->pcdata->pvnum;
      else
        victim->critical_hit_by = 0;

      affect_join( victim, &af );
      ch_col[0]='\0';
      if(ch->vt100==1 && (ch->ansi==1))
        {
        /* determine color of messages */
        if(ch->pcdata->color[5]!=0)
	  sprintf( ch_col, "\033[1;%d;%dm", ch->pcdata->color[0][5],
	           ch->pcdata->color[1][5]);
        }
      SET_BIT(victim->affected2_by, 0-AFF2_BLEEDING);
      act("$tYou feel particularly satisfied as...",ch,ch_col,NULL,TO_CHAR);
      }
    }
  else
    dam = dam*(30 + ch->level/4)/30;/* Order 11/16/93 */
  }

if ( dam <= 0 )
  dam = 1;
damage( ch, victim, dam, dt );
tail_chain( );
return;
}



void damage_hurt( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
  {
  bool auto_sac, auto_loot;
  bool confused;
  char buf[MAX_INPUT_LENGTH];
  BOUNTY_DATA *bptr;

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
	victim->hit -= dam;
    /*
     * Get experience based on % of damage done                 -Martin
    if ( dam && ch != victim
    &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
    {
	int xp_gain;
        if ( ch->fighting->who == victim )
            xp_gain = (int) (ch->fighting->xp * dam) / victim->max_hit;
        else
            xp_gain = (int) (xp_compute( ch, victim ) * 0.85 * dam) / victim->max_hit;
        gain_exp( ch, xp_gain );
    }  */

	if ( !IS_NPC(victim)
	    &&   victim->level >= LEVEL_IMMORTAL
	    &&   victim->hit < 1 )
		victim->hit = 1;
	if ( !IS_NPC(victim)
	    &&   victim->which_god == GOD_POLICE
	    &&   victim->hit < 1 )
		victim->hit = 1;

         /* Dave is seriously cheating here for Angi
                       - Chaos  4/14/96 
            I guess we forgive you, Martin 18/8/98*/
        if( (IS_AFFECTED( ch, AFF_CHARM) || !IS_NPC( ch )) &&
            !IS_NPC( victim ) &&
            victim->hit < 1 &&
            victim->pcdata->pvnum == 56092 && 
            !strcasecmp(victim->name,"lovely") )
          victim->hit = 1;

      if( ch!=victim )
	update_pos( victim );

	if( victim->position != POS_DEAD && dam>0)
		show_char_to_char_1( victim, NULL);

	switch( victim->position )
	{
	case POS_MORTAL:
		act( "$n is mortally wounded, and will die soon, if not aided.",
		    victim, NULL, NULL, TO_ROOM );
		send_to_combat_char( 
		    "You are mortally wounded, and will die soon, if not aided.\n\r",
		    victim );
		break;

	case POS_INCAP:
		act( "$n is incapacitated and will slowly die, if not aided.",
		    victim, NULL, NULL, TO_ROOM );
		send_to_combat_char(
		    "You are incapacitated and will slowly die, if not aided.\n\r",
		    victim );
		break;

	case POS_STUNNED:
		act( "$n is stunned, but will probably recover.",
		    victim, NULL, NULL, TO_ROOM );
		send_to_combat_char("You are stunned, but will probably recover.\n\r",
		    victim );
		break;

	case POS_DEAD:
		act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
		send_to_combat_char( "You have been KILLED!!\n\r\n\r", victim );
		break;

	default:
		if ( dam > victim->max_hit / 4 )
			send_to_combat_char( "*** That really did HURT!\n\r", victim
			    );
		if ( victim->hit < victim->max_hit / 4 )
			send_to_combat_char( "*** You sure are BLEEDING!\n\r", victim
			    );
		break;
	}

 /*
  * Sleep spells and extremely wounded folks.
  */
  if ( !IS_AWAKE(victim) || ch==victim ) 
  if( dt!=TYPE_NOFIGHT )
  {
    if (victim->fighting != NULL
    && victim->fighting->who->hunting != NULL
    && victim->fighting->who->hunting->who == victim)
	stop_hunting(victim->fighting->who);

    if (victim->fighting != NULL
    && victim->fighting->who->hating != NULL
    && victim->fighting->who->hating->who == victim)
	stop_hating(victim->fighting->who);

    stop_fighting( victim, FALSE );
  }

  /* Intercept object programs for damaging others */
  if( !IS_NPC( ch ) && dam > 0 && victim->position != POS_DEAD )
  if( dt!=TYPE_NOFIGHT )
    {
    OBJ_DATA *obj;
    OBJ_PROG *prg;
    OBJ_WITH_PROG *with;

    if( ch->desc != NULL )
    for(with=ch->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if( ch->obj_prog_ip==0 && obj->pIndexData->obj_prog!=NULL)
	for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
	  if( prg->cmd == -6 && number_percent() < prg->percentage )
	    {
            open_timer( TIMER_OBJ_PROG );
	    start_object_program( ch, obj, prg, "");
            close_timer( TIMER_OBJ_PROG );
	    tail_chain();
	    }
      }
    }

/* Intercept object programs for getting damaged */
  if( !IS_NPC( victim ) && dam > 0 && victim->position != POS_DEAD )
    {
    OBJ_DATA *obj;
    OBJ_PROG *prg;
    OBJ_WITH_PROG *with;

    if( victim->desc != NULL )
     for(with=victim->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if( victim->obj_prog_ip==0 && obj->pIndexData->obj_prog!=NULL)
	for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
	  if( prg->cmd == -5 && number_percent() < prg->percentage )
	    {
            open_timer( TIMER_OBJ_PROG );
	    start_object_program( victim, obj, prg, "");
            close_timer( TIMER_OBJ_PROG );
	    tail_chain();
	    }
      }
    }

    /*
     * Payoff for killing things.
     */
	if ( victim->position == POS_DEAD )
	{
         last_dead = victim;
      if( victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA &&
         !IS_NPC( victim))
      {
      int i;
      if( !IS_NPC( ch))
	{
        if (ch!=victim)
          add_to_victory_list( ch, victim );
        if(( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE ) &&
          ( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_POLICE ) &&
	  ( which_god( ch)!=0 && which_god(victim)!=0 ) )
	    {
            char buf2 [MAX_INPUT_LENGTH];
            sprintf(buf2, "%s", get_name(ch));

	    victim->pcdata->history[HISTORY_KILL_BY_EN]++;
	    ch->pcdata->history[HISTORY_KILL_EN]++;

            bptr = get_bounty(victim->name);
            if( bptr != NULL)
            {
              gold_transaction( ch, bptr->amount );
              remove_bounty( bptr );
              save_bounties();
              sprintf(buf, "You collect the bounty on %s of %d gold coins.\n\r",
                      capitalize(victim->name), bptr->amount);
              send_to_char(buf, ch);
            }

   	    if (ch->pcdata->clan != NULL && victim->pcdata->clan != NULL )
	    {
  	     ch->pcdata->clan->pkills[which_god(ch)]++;
  	     victim->pcdata->clan->pdeaths[which_god(victim)]++;
	     save_clan( ch->pcdata->clan );
	     save_clan( victim->pcdata->clan );
	    }
   	    ch->hit = ch->max_hit;
            ch->mana = ch->max_mana;
            ch->move = ch->max_move;
/* Stupid highlander reference here.  Removed by Chaos  4/22/99
            act( "Bolts of blue energy rise from the corpse, seeping into $n.", ch, victim->name, NULL, TO_ROOM );
            act( "Bolts of blue energy rise from the corpse, seeping into you.", ch, victim->name, NULL, TO_CHAR ); */
 
            sprintf(buf, "%s has slain %s!", buf2, get_name(victim));
            do_battle(buf);
	    save_char_obj(ch, NORMAL_SAVE);  /*save killer, and killed is saved in raw_kill*/
	    }
        else
        if( check_race_war( ch, victim ) )
          {
	  victim->pcdata->history[HISTORY_LOSE_ARMY]++;
	  ch->pcdata->history[HISTORY_KILL_ARMY]++;

            bptr = get_bounty(victim->name);
            if( bptr != NULL)
            {
              gold_transaction( ch, bptr->amount );
              remove_bounty( bptr );
              save_bounties();
              sprintf(buf, "You collect the bounty on %s of %d gold coins.\n\r",
                      capitalize(victim->name), bptr->amount);
              send_to_char(buf, ch);
            }

   	    if (ch->pcdata->clan != NULL && victim->pcdata->clan != NULL )
 	    {
  	     ch->pcdata->clan->pkills[CLAN_RACE_KILL]++;
	     save_clan( ch->pcdata->clan );
  	     victim->pcdata->clan->pdeaths[CLAN_RACE_DEATH]++;
	     save_clan( victim->pcdata->clan );
	    }
	  save_char_obj(ch, NORMAL_SAVE);  /*save killer, and killed is saved in raw_kill*/
          }
        else if( victim != ch )
	    {
	    victim->pcdata->history[HISTORY_KILL_BY_PC]++;
	    ch->pcdata->history[HISTORY_KILL_PC]++;

            bptr = get_bounty(victim->name);
            if( bptr != NULL)
            {
              gold_transaction( ch, bptr->amount );
              remove_bounty( bptr );
              save_bounties();
              sprintf(buf, "You collect the bounty on %s of %d gold coins.\n\r",
                      capitalize(victim->name), bptr->amount);
              send_to_char(buf, ch);
            }

   	    if (ch->pcdata->clan != NULL && victim->pcdata->clan != NULL )
   	    {
  	     ch->pcdata->clan->pkills[CLAN_ASSASSINATE_KILL]++;
	     save_clan( ch->pcdata->clan );
  	     victim->pcdata->clan->pdeaths[CLAN_ASSASSINATE_DEATH]++;
	     save_clan( victim->pcdata->clan );
	    }
   	     save_char_obj(ch, NORMAL_SAVE);  /*save killer, and killed is saved in raw_kill*/
	    }
       
         if (victim != ch) 
         {
           i=ch->pcdata->killnum-1;
           if (i<0) i=MAX_KILL_TRACK-1;
           STRFREE (ch->pcdata->killname[i] );
           sprintf(buf, "Z%12s (%dx Level %2d) on %s", victim->name, victim->pcdata->reincarnation, victim->level, (char *) ctime( &current_time)); 
           buf[strlen(buf)-1]='\0';
           ch->pcdata->killname[i] = STRALLOC(buf);
           ch->pcdata->killnum=i;
         }
	}
     else
      {
       victim->pcdata->history[HISTORY_KILL_BY_NPC]++;
       if (victim->pcdata->clan != NULL) 
        {
  	 victim->pcdata->clan->mdeaths++;
	 save_clan( victim->pcdata->clan );
    	}
      }
     }
  
     if(IS_NPC(victim) && !IS_NPC(ch) && ch->pcdata->clan != NULL) 
      {
        ch->pcdata->clan->mkills++;
      }
     if ( !IS_NPC(victim)  )
      {
       sprintf( log_buf, "%s killed by %s at %u",
	                  victim->name, get_name( ch ), 
		          victim->in_room->vnum );
       log_string( log_buf );

       if (victim->level < 2)
	gain_exp( victim, 0-(victim->exp/2));
       else
       if(victim->exp>exp_level(victim->class,(victim->level)-1 )+1)
        gain_exp( victim, exp_level(victim->class, (victim->level)-1 )+1 -
		   victim->exp);

        if( victim->level == 91   && victim->pcdata->death_timer == 0 &&
          victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
             /* Knights Death     - Chaos 9/14/95 */
          {
          victim->max_hit -= 20 * (victim->pcdata->reincarnation + 3 ) / 3;
          victim->actual_max_hit -= 20 * (victim->pcdata->reincarnation + 3 ) / 3;
          victim->pcdata->death_timer = 2;
           save_char_obj(ch, NORMAL_SAVE);
          }
        if( victim->level > 91 && victim->level < 96  &&
            victim->pcdata->death_timer == 0 &&
             victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
          {
          int add_hp, add_mana, add_move;
          victim->level --;
          victim->mclass[ victim->class ]  --;
          add_hp = 130;
          add_mana = 100;
          add_move = 85;
          add_hp      *= (victim->pcdata->reincarnation + 2 ) / 2;
          add_mana    *= (victim->pcdata->reincarnation + 3 ) / 3;
          add_move    *= (victim->pcdata->reincarnation + 3 ) / 3;

          victim->max_hit -= add_hp;
          victim->max_move -= add_mana;
          victim->max_mana -= add_move;
          victim->actual_max_hit -= add_hp;
          victim->actual_max_move -= add_mana;
          victim->actual_max_mana -= add_move;

          knight_adjust_hpmnmv( victim );

		  if(victim->exp>exp_level(victim->class,(victim->level)-1 )+1)
	  gain_exp( victim, exp_level(victim->class, (victim->level)-1 )+1 -
		   victim->exp);

          REMOVE_BIT( ch->act, PLR_THIEF);
          sub_player( ch);
          add_player( ch);
          victim->pcdata->death_timer = 2;
          save_char_obj(ch, NORMAL_SAVE);
          save_char_obj(ch, BACKUP_SAVE);
          }


       if( victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
        if(IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF))
         {
          if(victim->level>1)
	    if(victim->exp>exp_level(victim->class, (victim->level)-2 )+1)
	      gain_exp( victim, exp_level(victim->class, (victim->level)-2 )+1-
		   victim->exp);
          if(victim->level<2)
	      gain_exp( victim, -victim->exp);
          if(IS_SET(victim->act, PLR_KILLER))
            victim->act-=PLR_KILLER;
          if(IS_SET(victim->act, PLR_THIEF))
            victim->act-=PLR_THIEF;
          }
        }
   else
	  group_gain( ch, victim );

	  if (IS_NPC(victim)&&(victim == ch))
             {
             if( victim->position == POS_DEAD )
               raw_kill( victim ) ;
	     return;
             }

    if( !confused )
      set_fighting(victim, ch);
    /*victim->fighting->who=ch;  -Presto 3/28/99*/


        auto_loot = FALSE;
        auto_sac = FALSE;
	if ( !IS_NPC(ch) && IS_NPC(victim) )
            {
            if ( IS_SET(ch->act, PLR_AUTOSAC) )
              auto_sac = TRUE;
            if ( IS_SET(ch->act, PLR_AUTOLOOT)|| IS_SET( ch->act,
			    PLR_AUTO_SPLIT) )
              auto_loot = TRUE;
            }

      if( !IS_NPC( ch ) && !IS_NPC( victim ) && 
          ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA )
        {
        PLAYER_GAME *gpl;
        sprintf( buf, "%s has killed %s.\n\r", ch->name, victim->name );
      for( gpl = first_player ; gpl != NULL; gpl = gpl->next )
         if( gpl->ch != ch && gpl->ch != victim && 
             gpl->ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA )
           send_to_combat_char( buf, ch );
         }
          
         /* keep track of mob vnum killed */
        /* 
        if ( !IS_NPC(ch) )       
            add_kill( ch, victim );
           */

#ifdef AREA_KILL
  if( IS_NPC( victim) && !IS_NPC(ch ) )
    {
    int anum;
    anum = victim->pIndexData->area->low_r_vnum/100;
    ch->pcdata->area_kills[ anum ]++;
    }
#endif

         raw_kill( victim );

        if ( auto_loot )
        {
          FIGHT_DATA *fptr;

          fptr=ch->fighting;
          ch->fighting=NULL;
	  do_get( ch, NULL );
          ch->fighting=fptr;
        }

        if( auto_sac )
	  do_sacrifice( ch, "corpse" );

        if( !IS_NPC( ch ) )
         if(ch->vt100==1 && IS_SET( ch->act, PLR_PROMPT))
          vt100prompt( ch );

		return;
	}


	/*
     * Wimp out?
     */
	if ( IS_NPC(victim) && dam > 0 )
	{
		if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0 
		    &&   victim->hit < victim->max_hit / 2 )
		    ||   ( IS_AFFECTED(victim, AFF_CHARM) 
		    && victim->master != NULL 
		    && victim->master->in_room != victim->in_room ) )
  		    {
			start_fearing(victim, ch);
			stop_hunting(victim);
			do_flee( victim, "" );
		    }
	}

	if ( !IS_NPC(victim)
	    &&   victim->hit > 0
	    &&   victim->hit <= victim->wimpy
	    &&   victim->wait == 0 )
             {
		 if( victim->desc!=NULL)
                   {
                   if( victim->desc->back_buf != NULL )
                     {
                     int leny;
                     leny=str_cpy_max( buf, victim->desc->back_buf, 
                            MAX_STRING_LENGTH );
                     leny=str_apd_max( buf, "\rflee\r", leny,
                            MAX_STRING_LENGTH );
                     STRFREE(victim->desc->back_buf );
                     victim->desc->back_buf = str_dup( buf );
                     }
                   else
		    victim->desc->intop = str_apd_max( victim->desc->inbuf,
                         "\rflee\r", victim->desc->intop, MAX_INPUT_LENGTH);
                   }
		 /* This particular set of code will stop fights
                 else
		   do_flee( victim, "" );*/
            }

        if( !IS_NPC( ch ) )
         if(ch->vt100==1 && IS_SET( ch->act, PLR_PROMPT))
          vt100prompt( ch );

        if( !IS_NPC( victim ) )
         if(victim->vt100==1 && IS_SET( victim->act, PLR_PROMPT))
          vt100prompt( victim );

	tail_chain( );
	return;
}

void damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
  /* char buf[MAX_INPUT_LENGTH]; */
  bool confused;

  if( victim==NULL )
    return;
  if ( victim->position == POS_DEAD )
   return;

  if( ch==NULL )
    ch=victim;

  confused = FALSE;

  if( dt!=TYPE_NOFIGHT )
    switch(ch->position)
      {
      case POS_SLEEPING:
      case POS_RESTING:
        do_stand(ch,"");
        return;
        break;
      case POS_STANDING:
        ch->position=POS_FIGHTING;
        break;
      }

  /* Disallow fighting in shops */
  if(find_keeper(ch)!=NULL && dt!=TYPE_NOFIGHT)
  {
    stop_fighting(ch, TRUE);
    return;
  }

  if( ch->fighting==NULL && ch!=victim  && dt!=TYPE_NOFIGHT)
    set_fighting(ch,victim);
  if( victim->fighting==NULL && ch!=victim  && dt!=TYPE_NOFIGHT)
    set_fighting(victim,ch);


      /* Confusion spell   - Chaos  7/26/96 */
  if( IS_AFFECTED( ch, AFF2_CONFUSION ) && number_range(0,2)==0  
      && dt!=TYPE_NOFIGHT)
    {
    CHAR_DATA *fch;
    int tch, pch;

    for( tch=0, fch=ch->in_room->first_person; fch != NULL; fch=fch->next_in_room )
      {
          if (IS_AFFECTED( fch, AFF_ETHEREAL ))
            continue; /* Stop confused ppl being so confused they start 
			 attacking ethereals - Martin */
          if(IS_AFFECTED( fch,AFF2_MIRROR_IMAGE))
            continue; /* Stop very strange behaviour when confused person
			 strikes mirrored person... - Martin */
          if(!is_same_group(ch, fch))
            continue;
      if( fch != ch )
        tch++;
      }

    if( tch > 1 )
      {
      pch = number_range( 1, tch );
      for( tch=0, fch=ch->in_room->first_person; fch != NULL; fch=fch->next_in_room )
        if( fch != ch )
          {
          if (IS_AFFECTED( fch, AFF_ETHEREAL ))
            continue; /* Stop confused ppl being so confused they start 
			 attacking ethereals - Martin */
          if(IS_AFFECTED( fch,AFF2_MIRROR_IMAGE))
            continue; /* Stop very strange behaviour when confused person
			 strikes mirrored person... - Martin */
          if(!is_same_group(ch, fch))
            continue;
          tch++;
          if( tch == pch )
            {
            victim = fch;
            confused = TRUE;
            break;
            }
          }
      }
    }

    /* Start the cheating check */
      if( IS_NPC( victim ) && victim!=ch && !confused && dam>0 &&
          dt!=TYPE_NOFIGHT)
        {
        CHAR_DATA *ch_ld;
        ch_ld = ch;
        if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) && ch->master != NULL )
          ch_ld = ch->master;
        if( ch_ld != NULL && !IS_NPC( ch_ld ) )
          victim->npcdata->pvnum_last_hit = ch_ld->pcdata->pvnum;
        if( ch_ld != NULL )
          {
          if( ch_ld->leader != NULL && !IS_NPC(ch_ld->leader))
            victim->npcdata->pvnum_last_hit_leader = 
                   ch_ld->leader->pcdata->pvnum;
          else
          if( !IS_NPC(ch_ld))
            victim->npcdata->pvnum_last_hit_leader = 
                   ch_ld->pcdata->pvnum;
          }
        }

  if( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
    if( ch->in_room->sector_type == SECT_ETHEREAL || 
        ch->in_room->sector_type == SECT_ASTRAL )
      if( dt != TYPE_HIT && dt != TYPE_UNDEFINED)
        {
        dam = 4 * dam / 7;
        }

  if (dam && IS_NPC(victim) && ch != victim && dt!=TYPE_NOFIGHT)
  {
        if ( !IS_SET( victim->act, ACT_SENTINEL ) )
        {
            if ( victim->hunting )
            {
                if ( victim->hunting->who != ch )
                {
                    STRFREE( victim->hunting->name );
                    victim->hunting->name = QUICKLINK( ch->name );
                    victim->hunting->who  = ch;
                }
            }
            else
                start_hunting( victim, ch );
        }

        if ( victim->hating )
        {
            if ( victim->hating->who != ch )
            {
            if( IS_NPC( ch ) )
              stop_hating( victim );
            else
              {
                STRFREE( victim->hating->name );
                victim->hating->name = QUICKLINK( ch->name );
                victim->hating->who  = ch;
              }
            }
        }
        else
            start_hating( victim, ch );
    }

  /*
   * Stop up any residual loopholes.
   *  Modified moving dam cap  -  Chaos  4/27/99
   */
    {
    int dam_cap;

    if ( dt == gsn_backstab &&  dam > 1600 &&
         number_percent()<ch->pcdata->learned[gsn_greater_backstab] ) 
        dam_cap = 800+ch->level*10;
    else
        dam_cap = 800+ch->level*6;

	if ( dam > dam_cap)
          dam = dam_cap;
    }


	if ( victim != ch )
	{

        /*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
                if ( is_safe( ch, victim ) && dt!=TYPE_NOFIGHT)
	          return;

		if ( ( dt < 0 || dt > MAX_SKILL || !is_spell(dt) ) && 
                   dt!=TYPE_NOFIGHT &&
              ( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) ) )
                  return;


		if ( victim->position > POS_STUNNED && ch!=victim &&
                     dt!=TYPE_NOFIGHT )
		{
			if ( victim->fighting == NULL && !confused )
				set_fighting( victim, ch );
		}

		if ( ch->position > POS_STUNNED && ch!=victim &&
                     dt!=TYPE_NOFIGHT )
		{
			if ( ch->fighting == NULL && !confused )
				set_fighting( ch, victim );

	    /*  
	     * If victim is charmed, ch might attack victim's master.
	     */
			if ( IS_NPC(ch)
			    &&   IS_NPC(victim)
			    &&   IS_AFFECTED(victim, AFF_CHARM)
			    &&   victim->master != NULL
			    &&   victim->master->in_room == ch->in_room
			    &&   number_bits( 3 ) == 0 
                            &&   dt!=TYPE_NOFIGHT )
			{
				stop_fighting( ch, TRUE );
				multi_hit( ch, victim->master, TYPE_UNDEFINED
				    );
				return;
			}
		}


        /*
	 * Inviso attacks ... not.
	 */
	/* Stuff like starvation and poison shouldn't take them out of
	   stealth or invis ... Martin 9/2/99 */
	if (dt != TYPE_NOMESSAGE && dt!=TYPE_NOFIGHT)
	{
		if ( IS_AFFECTED(ch, AFF_HIDE) )
		  {
		  affect_strip( ch, gsn_hide );
		  REMOVE_BIT( ch->affected_by, AFF_HIDE );
		  act( "$n steps out from $s hiding spot.", ch, NULL, NULL,
			      TO_ROOM );
		  act( "You step out from your hiding spot.", ch, NULL, NULL,
			      TO_CHAR );
		  }

		if ( IS_AFFECTED(ch, AFF_STEALTH) )
		{
			affect_strip( ch, gsn_stealth );
			affect_strip( ch, gsn_greater_stealth );
			REMOVE_BIT( ch->affected_by, AFF_STEALTH );
			act( "$n steps out into the room.", ch, NULL, NULL,
			    TO_ROOM );
			act( "You step out into the room .", ch, NULL, NULL,
			    TO_CHAR );
		}

		if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
		{
			affect_strip( ch, gsn_invis );
			affect_strip( ch, gsn_mass_invis );
                        if (!is_affected(ch, gsn_improved_invis) )
 			{
			 REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
			 act( "$n fades into existence.", ch, NULL, NULL,
			    TO_ROOM );
 			}
		}
 	}
	/*
	 * Damage modifiers.
	 */
		if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
			dam = dam * 2/3;

    /* Gith Damage +25% astral, +10% ethereal, -10% material plane */
    if(ch->race == RACE_GITH && ch->in_room != NULL)
    {
      if(ch->in_room->sector_type == SECT_ASTRAL)
        dam = dam * 5/4;
      else if(ch->in_room->sector_type == SECT_ETHEREAL)
        dam = dam * 11/10;
      else
        dam = dam * 9/10;
    }

/*
  		Changing effect of protection...adding protection from good
		- Martin 10/8/98
		if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
			dam -= dam * 1/3;
*/
		if ( dam < 0 )
			dam = 0;

	/*
	 * Check for disarm, trip, parry, and dodge.
	 */
	      if ( dt >= TYPE_HIT) 
		{
			if ( IS_NPC(ch) && number_percent( ) < (ch->level / 20 + 1) )
					trip( ch, victim );
			if ( check_parry( ch, victim ) )
				return;
			if ( check_dodge( ch, victim ) )
				return;
			if ( IS_NPC(ch) && number_percent( ) < (ch->level / 30 + 1) )
					disarm( ch, victim );
	                if (IS_AFFECTED(victim, AFF2_FIRESHIELD))
                        {
       			    
        		 damage(victim, ch, UMAX(1, number_fuzzy(dam/4)), gsn_fire_shield);
  			 if (ch==last_dead) return;
                        }
		}
                 
               if (dt != gsn_anatomy) 
		dam_message( ch, victim, dam, dt );
           }
          else if (dt != gsn_anatomy) 
	    dam_message( ch, victim, dam, dt );

        if (dt == gsn_anatomy)
           dam=victim->hit;

    /*  Adjust damage based on monster kills  - Chaos  5/17/99  */
#ifdef AREA_KILL
  if( IS_NPC( victim) && !IS_NPC(ch ) )
    {
    int anum;
    anum = victim->pIndexData->area->low_r_vnum/100;
    if( ch->pcdata->area_kills[ anum ]>250 )
      dam = dam * 500 / ( 250 + ch->pcdata->area_kills[ anum ] );
    }
#endif

    damage_hurt( ch, victim, dam, dt );

#ifdef EQUIPMENT_DAMAGE

      /* Armor damage  - Chaos  4/17/99  */
    if( dam>0 && dt == TYPE_HIT )
      if( number_range( 0, 2000 ) < (170-ch->level)/5 )
        damage_equipment( victim , FALSE );

      /* Weapon damage */
    if( dam>0 && dt == TYPE_HIT )
      if( number_range( 0, 20000 ) < (170-ch->level)/5 )
        damage_equipment( ch , TRUE );
#endif

    return;
}


bool has_mirror(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if(IS_AFFECTED(victim,AFF2_MIRROR_IMAGE))
    {
    AFFECT_DATA *paf;
    bool all;

       /* Let's make intelligence define who sees images */
    if( number_range( 10, 60) < get_curr_int( ch ) )
      all = TRUE;
    else
      all = FALSE;

    for ( paf = victim->first_affect; paf != NULL; paf = paf->next)
      {
      if(paf->bitvector==AFF2_MIRROR_IMAGE)
        {
        if(all)
          {
          affect_strip(victim,paf->type);
          }
        else
        if(number_range(0,paf->modifier)!=0)
          {
          act("You destroy $N in one blow!  Huh?",ch,NULL,victim,TO_CHAR);
          act("$n destroys $N in one blow!",ch,NULL,victim,TO_ROOM);

          paf->modifier-=1;
          if(paf->modifier==0)
            affect_strip(victim,paf->type);
          if( ch!=victim )
            set_fighting( ch, victim );
          if( ch!=victim )
            set_fighting( victim, ch );
          return TRUE;
          }
        break;
        }
      }
    if(all)
      {
      act("You discover the real $N.",ch,NULL,victim,TO_CHAR);
      act("$N's images disappear.",ch,NULL,victim,TO_ROOM);
      set_fighting( ch, victim );
      set_fighting( victim, ch );
      }
    }
  return FALSE;
}

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_NPC(ch) || IS_NPC(victim) )
		return FALSE;

	/* Thx Josh! */
	if ( victim->fighting && who_fighting( victim ) == ch )
		return FALSE;

        if(victim->mclass[CLASS_ASSASSIN] == victim->level &&
           victim->level!=95 && IS_SET(victim->act, PLR_KILLER) &&
           ch->level <= victim->level+10)
          return FALSE;

	if ( ch->level/2 > victim->level )
	{
		send_to_combat_char( "You may not attack that low of a level player.\n\r",
		    ch );
		return TRUE;
	}

	return FALSE;
}


/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * NPC's are fair game.
     */
	if ( (IS_NPC(victim) && !IS_AFFECTED(victim, AFF_CHARM))
     || ch->in_room->area->low_r_vnum==ROOM_VNUM_ARENA)
		return;

     if ( which_god(victim) == GOD_DEMISE ) 
         return;

    /*
     * Charm-o-rama.
     */
	if ( IS_SET(ch->affected_by, AFF_CHARM) )
	{
		if ( ch->master == NULL )
		{
			char buf[MAX_INPUT_LENGTH];

			sprintf( buf, "Check_killer: %s bad AFF_CHARM",
			    IS_NPC(ch) ? ch->short_descr : ch->name );
			bug( buf, 0 );
			affect_strip( ch, gsn_charm_person );
			REMOVE_BIT( ch->affected_by, AFF_CHARM );
			return;
		}

		send_to_combat_char( "*** You are now a KILLER!! ***\n\r", ch->master
		    );
		SET_BIT(ch->master->act, PLR_KILLER);
                ch->master->killer_played=ch->master->played;
		/*      stop_follower( ch );  */
		return;
	}

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * And current killers stay as they are.
     */
	if ( IS_NPC(ch)
	    ||   ch == victim
	    ||   IS_SET(victim->act, PLR_KILLER) 
	    ||   IS_SET(ch->act, PLR_KILLER) )
		return;

	send_to_combat_char( "*** You are now a KILLER!! ***\n\r", ch );
	if(ch->desc->original!=NULL)
	{
		SET_BIT(ch->desc->original->act, PLR_KILLER);
                ch->desc->original->killer_played=ch->desc->original->played;
		save_char_obj(ch->desc->original, NORMAL_SAVE);
	}
	else
	{
		SET_BIT(ch->act, PLR_KILLER);
                ch->killer_played=ch->played;
		save_char_obj(ch, NORMAL_SAVE);
	}
	return;
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;

	if ( !IS_AWAKE(victim) )
		return FALSE;

	if ( IS_NPC(victim) )
	{
		/* Tuan was here.  :) */
		chance  = UMIN( 30, victim->level/2+5 );
	}
	else
	{
          if( multi(victim, gsn_parry) == -1 )
            return( FALSE );

		if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
			return FALSE;
		chance  = victim->pcdata->learned[gsn_parry]/2;
	}

      if( OGRE_INTIMIDATE )
        chance -= 13;

	if ( number_percent( ) >= chance + (victim->level - ch->level)/4 )
		return FALSE;

	if( IS_SET( victim->attack, 2))
		return FALSE;
	else
		victim->attack+=2;

     if( IS_NPC( victim ) || !IS_SET( victim->pcdata->spam, 8))
	act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
     if( IS_NPC( ch ) || !IS_SET( ch->pcdata->spam, 2))
	act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
	return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;

	if ( !IS_AWAKE(victim) )
		return FALSE;

	if ( IS_NPC(victim) )
		/* Tuan was here.  :) */
		chance  = UMIN( 25, victim->level/3+5 );
	else
          {
          if( multi(victim, gsn_dodge) == -1 )
            return( FALSE );
		chance  = victim->pcdata->learned[gsn_dodge] / 3;
	  }

      if( OGRE_INTIMIDATE )
        chance -= 10;

	if ( number_percent( ) >= chance + (victim->level - ch->level)/4 )
		return FALSE;

	if ( IS_SET( victim->attack, 4))
		return FALSE;
	else
		victim->attack+=4;

     if( IS_NPC( victim ) || !IS_SET( victim->pcdata->spam, 8))
	act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
     if( IS_NPC( ch ) || !IS_SET( ch->pcdata->spam, 2))
	act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
	return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
  if ( victim->hit > 0 )
  {
    if ( victim->position <= POS_STUNNED )
      victim->position = POS_STANDING;
	return;
  }

	if ( IS_NPC(victim) || victim->hit <= -11 )
	{
		victim->position = POS_DEAD;
		return;
	}

	if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
	else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
	else victim->position = POS_STUNNED;

  if(( victim->position < POS_RESTING || victim->position == POS_FIGHTING ) &&
         in_camp(victim))
    {
    CHAR_DATA *gch;
    for(gch=victim->in_room->first_person;gch!=NULL;gch=gch->next_in_room)
      if(is_same_group(victim,gch) && IS_AFFECTED(gch,AFF2_CAMPING))
        {
        send_to_combat_char( "Your camp is destroyed!\n\r", victim );
        REMOVE_BIT(gch->affected2_by, 0-AFF2_CAMPING);
        break;
        }
    }
  return;
}

void add_kill( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    sh_int vnum;

    return;

    if ( IS_NPC(ch) )
    {
        bug( "add_kill: trying to add kill to npc", 0 );
        return;
    }
    if ( !IS_NPC(mob) )
    {
        bug( "add_kill: trying to add kill non-npc", 0 );
        return;
    }
    vnum = mob->pIndexData->vnum;
    for ( x = 0; x < MAX_MOB_KILL_TRACK; x++ )
        if ( ch->pcdata->killed[x].vnum == vnum )
        {
            if ( ch->pcdata->killed[x].count < 50 )
                ++ch->pcdata->killed[x].count;
            return;
        }
        else
        if ( ch->pcdata->killed[x].vnum == 0 )
            break;
    memmove( (char *) ch->pcdata->killed+sizeof(KILLED_DATA),
                ch->pcdata->killed, (MAX_MOB_KILL_TRACK-1) * sizeof(KILLED_DATA) );
    ch->pcdata->killed[0].vnum  = vnum;
    ch->pcdata->killed[0].count = 1;
}

int times_killed( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    sh_int vnum;

    return(0);

    if ( IS_NPC(ch) )
    {
        bug( "times_killed: ch is not a player", 0 );
        return 0;
    }
    if ( !IS_NPC(mob) )
    {
        bug( "times_killed: mob is not a mobile", 0 );
        return 0;
    }

    vnum = mob->pIndexData->vnum;
    for ( x = 0; x < MAX_MOB_KILL_TRACK; x++ )
        if ( ch->pcdata->killed[x].vnum == vnum )
            return ch->pcdata->killed[x].count;
        else
        if ( ch->pcdata->killed[x].vnum == 0 )
            break;

    return 0;
}

void free_fight( CHAR_DATA *ch )
{
   if ( ch==NULL )
   {
        bug( "Free_fight: null ch!", 0 );
        return;
   }
   if ( ch->fighting!=NULL )
   {
     if ( !ch->fighting->who && ch->fighting->who != last_dead )
       --ch->fighting->who->num_fighting;
     DISPOSE( ch->fighting );
   }
   ch->fighting = NULL;
   ch->position = POS_STANDING;
   ch->asn_obj  = NULL;
   return;
}

/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
  FIGHT_DATA *fight;

  if( ch==victim )
    return;

  if ( ch->fighting != NULL )
    {
/*
    log_printf("Set_fighting: %s -> %s (already fighting %s)",
	ch->name, victim->name, ch->fighting->who->name ); */
    return;
    }

  if ( IS_AFFECTED(ch, AFF_SLEEP) )
    affect_strip( ch, gsn_sleep );
   CREATE( fight, FIGHT_DATA, 1 );
   fight->who   = victim;
   fight->xp    =  0;
   /* fight->xp    = (int) xp_compute( ch, victim ) * 0.85; */
   if ( !IS_NPC(ch) && IS_NPC(victim) )
     fight->timeskilled = times_killed(ch, victim);
   ch->num_fighting = 1;
   ch->fighting = fight;
   victim->num_fighting++;

  ch->position = POS_FIGHTING;
  update_pos( ch );

  return;
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  CHAR_DATA *fch;
  char *name;

  if( IS_NPC( ch ) && IS_SET( ch->act, ACT_UNDEAD ) )
    {
    for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
      {
      obj_next = obj->next_content;
      obj_from_char( obj );
      if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        extract_obj( obj );
      else
        obj_to_room( obj, ch->in_room );
      }
    return;
    }

  if ( IS_NPC(ch) )
    {
    name            = ch->short_descr;
    corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
    corpse->timer   = number_range( 2, 4 );
    corpse->level   = ch->level;
    if( ch->fighting != NULL)
      {
      if( IS_NPC(ch->fighting->who) && IS_AFFECTED( ch->fighting->who, AFF_CHARM))
        if( !IS_NPC( ch->fighting->who->master))
          corpse->owned_by = ch->fighting->who->master->pcdata->pvnum;
      if( !IS_NPC( ch->fighting->who))
        corpse->owned_by = ch->fighting->who->pcdata->pvnum;
      if( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM) && ch->master!=NULL &&
          ch->master->pcdata!=NULL )
        corpse->owned_by = ch->master->pcdata->pvnum;
      }
    else
      corpse->owned_by=-1;
    if ( ch->gold > 0 )
      {
      obj_to_obj( create_money( ch->gold ), corpse );
      ch->gold = 0;
      }
    }
  else
    {
    name            = ch->name;
    corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer   = number_range( 50, 80 );
    if( ch->pcdata->corpse==NULL)
      {
      ch->pcdata->corpse=corpse;
      ch->pcdata->corpse_room=ch->in_room->vnum;
      }
    corpse->owned_by= ch->pcdata->pvnum;

    for( fch=first_char; fch!=NULL; fch=fch->next)
      if( IS_NPC(fch) && fch->master==ch && IS_SET( fch->act, ACT_PET ) )
        SET_BIT( fch->act, ACT_WILL_DIE);
    }

  sprintf( buf, corpse->short_descr, name );
  STRFREE (corpse->short_descr );
  corpse->short_descr = STRALLOC( buf );

  sprintf( buf, "corpse %s", ch->name );
  STRFREE (corpse->name );
  corpse->name = STRALLOC( buf );

  sprintf( buf, corpse->description, name );
  STRFREE (corpse->description );
  corpse->description = STRALLOC( buf  );
  
  for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
    {
    obj_next = obj->next_content;
    /* modification to keep object with character -Dug 12/4/93 */
    if(!IS_SET(obj->extra_flags, ITEM_INVENTORY) && can_drop_obj(ch, obj))
      {
      obj_from_char( obj );
      if( IS_NPC( ch ))
        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
          extract_obj( obj );
      obj_to_obj( obj, corpse );
      }
    }

  obj_to_room( corpse, ch->in_room );
  if( !IS_NPC( ch ) )
  {
    ch->pcdata->time_of_death = current_time;
    ch->pcdata->just_died_ctr = 300; /* 5 minutes */
    ch->pcdata->condition[COND_FULL] = 20;
    ch->pcdata->condition[COND_THIRST] = 20;
    send_to_combat_char( "Your perils have ended in defeat.\n\r", ch);
  }

  return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;
	int ran;
	bool IS_BODY;

  /* check for null ch  -Presto 3/28/99 */
  if (ch == NULL)
  {
    bug("null ch in death_cry.",0);
    return;
  }

	ran = number_bits( 4 );
	vnum = 0;
      IS_BODY = ( IS_NPC( ch ) && IS_SET( ch->pIndexData->act, ACT_BODY )  &&
	    ch->pIndexData->body_parts!=0);

      if( IS_BODY  && number_bits(2)==0  && !IS_SET( ch->act, ACT_UNDEAD))
	  {
	  int dir, cnt;

          
	  ran=number_range( 0, MAX_BODY -1);
	  dir=(number_range(0,1)*2)-1;
          cnt=0;
	  while( !IS_SET( ch->pIndexData->attack_parts, 1<<ran) &&
                 cnt<MAX_BODY  )
	    {
            cnt++;
	    ran+=dir;
	    if(ran<0)
	      ran=MAX_BODY-1;
	    if(ran>=MAX_BODY)
	      ran=0;
	    }
          if( cnt==MAX_BODY )
	     msg  = "$n splatters blood on your armor.";             
          else
             {
	     vnum = OBJ_VNUM_SLICED_LEG;                           
	     msg = body_table[ran].sliced;
             }
	  }
     else
       msg  = "You hear $n's death cry.";                               


   if( IS_SET( ch->act, ACT_UNDEAD ) )
     {
     vnum = 0;
     ran = 7;
     }

     if( vnum==0 )
      switch ( ran )
	{
	default: 
		break;
	case  0: 
		msg  = "$n hits the ground ... DEAD.";                  
		break;
	case  1: 
		msg  = "$n splatters blood on your armor.";             
		break;
	case  2: 
	       if(IS_BODY)
		 break;
		msg  = "You smell $n's sphincter releasing in death.";
		vnum = OBJ_VNUM_FINAL_TURD;                             
		break;
	case  3: 
	       if(IS_BODY)
		 break;
		msg  = "$n's severed head plops on the ground.";
		vnum = OBJ_VNUM_SEVERED_HEAD;                           
		break;
	case  4: 
	       if(IS_BODY)
		 break;
		msg  = "$n's heart is torn from $s chest.";
		vnum = OBJ_VNUM_TORN_HEART;                             
		break;
	case  5: 
	       if(IS_BODY)
		 break;
		msg  = "$n's arm is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_ARM;                             
		break;
	case  6: 
	       if(IS_BODY)
		 break;
		msg  = "$n's leg is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_LEG;                             
		break;
         case 7:
             msg = "$n turns to dust.";
             break;
	}
       if (!IS_SET(ch->act, ACT_ELEMENTAL))
	act( msg, ch, NULL, NULL, TO_ROOM );

	if ( vnum != 0 )
	{
		char buf[MAX_INPUT_LENGTH];
		OBJ_DATA *obj;
		char *name;
  

		name            = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj             = create_object( get_obj_index( vnum ), 0 );
		obj->timer      = number_range( 4, 7 );

	     if( IS_BODY )
		sprintf( buf, body_table[ran].short_descr, name );
	     else
		sprintf( buf, obj->short_descr, name );
	     STRFREE (obj->short_descr );
	     obj->short_descr = STRALLOC( buf );

	     if( IS_BODY )
		sprintf( buf, body_table[ran].description, name );
	     else
		sprintf( buf, obj->description, name );
		STRFREE (obj->description );
		obj->description = STRALLOC(  buf );

	     if( IS_BODY )
		{
		sprintf( buf, body_table[ran].name, name );
		STRFREE (obj->name );
		obj->name = STRALLOC( buf );
		sprintf( buf, body_table[ran].long_descr, name );
		STRFREE (obj->long_descr );
		obj->long_descr = STRALLOC( buf );
		}

		obj_to_room( obj, ch->in_room );
	}

	if ( IS_NPC(ch) )
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	for ( door = 0; door <= 5; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = was_in_room->exit[door] ) != NULL
		    &&   pexit->to_room != NULL
		    &&   pexit->to_room != was_in_room )
		{
                        char_from_room( ch );
			char_to_room( ch, pexit->to_room);
			act( msg, ch, NULL, NULL, TO_ROOM );
		}
	}
        char_from_room( ch );
        char_to_room( ch, was_in_room );

	return;
}



void raw_kill( CHAR_DATA *victim )
{
  ROOM_INDEX_DATA *old_room;
  CHAR_DATA *fch;

  for( fch=first_char; fch != NULL; fch=fch->next )
    {
    if( who_fighting(fch) == victim && 
	IS_AFFECTED( fch, AFF_CHARM) &&
        IS_NPC( fch) && 
	IS_SET( fch->act, ACT_ONE_FIGHT) )
      SET_BIT( fch->act, ACT_WILL_DIE);
     
    if( IS_NPC( fch) && IS_AFFECTED( fch, AFF_CHARM ) &&
         fch->master == victim && victim != fch )
      {
      /*fch->fighting->who=fch->master;*/
      raw_kill( fch );
      }
    }


  if(IS_NPC(victim))
     mprog_death_trigger(victim);/* This must be before make_corpse */
  else     /* check for assassin stuff */
     {
      check_asn_obj(victim);/* This must be before make_corpse */
      if( victim->fighting!=NULL)
        mprog_kill_trigger(victim->fighting->who);
     } 

  if( ( victim->pcdata != NULL && victim->pcdata->poison != NULL ) ||
      ( victim->npcdata != NULL && victim->npcdata->poison != NULL ) )
            {
            POISON_DATA *pd, *ppd;
            if( IS_NPC( victim ) )
              pd = victim->npcdata->poison;
            else
              pd = victim->pcdata->poison;
            for( ; pd!=NULL; pd=ppd )
              {
              ppd = pd->next;
              DISPOSE( pd );
              }
            if( IS_NPC( victim ) )
              victim->npcdata->poison=NULL;
            else
              victim->pcdata->poison=NULL;
            }

  make_corpse( victim );
  old_room=victim->in_room;

  if ( IS_NPC(victim) )
    {
    if( IS_SET( victim->act, ACT_PET) && victim->master!=NULL
        && !IS_SET( victim->act, ACT_ELEMENTAL) )
      send_to_combat_char("Your pet dies a horrible death.\n\r", victim->master);
    victim->pIndexData->killed++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
    leave_fighting( victim, old_room);
    extract_char( victim, TRUE );
    return;
    }
  leave_fighting( victim, old_room);
  extract_char( victim, FALSE );
  while ( victim->first_affect )
    affect_remove( victim, victim->first_affect );
  victim->affected_by = 0;
  victim->armor   = 100;
  victim->position= POS_RESTING;
  victim->hit     = UMAX( 1, victim->hit  );
  victim->mana    = UMAX( 1, victim->mana );
  victim->move    = UMAX( 1, victim->move );

  save_char_obj(victim, NORMAL_SAVE);

  return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  CHAR_DATA *lch;
  int xp;
  int levelst;
  OBJ_DATA *obj, *tmpobj;
  OBJ_DATA *obj_next;


  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if ( victim == ch )
    return;

  if( IS_AFFECTED( victim, AFF_CHARM))   /* No exp for pets */
    return;

  if( IS_NPC( victim) && victim->pIndexData->vnum==9901 ) /* No Demons */
    return;

  levelst = 0;

  /* for ( gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room ) */

  for ( gch = first_char; gch != NULL; gch = gch->next ) 
    {
    if ( is_same_group( gch, ch ) || (IS_NPC(gch) && gch->master == ch ) ||
        ( IS_NPC(ch) && ch->master!=NULL && is_same_group( gch, ch->master ) ) )
      {
      levelst+=gch->level;
      if( ch->in_room == gch->in_room )
        {
        gch->alignment-=victim->alignment/50;
        if(gch->alignment<-1000)
          gch->alignment=-1000;
        if(gch->alignment>1000)
          gch->alignment=1000;
        }
      }
    }


  if ( levelst == 0 )
    {
    bug( "Group_gain: 0 levelst.", levelst );
    levelst = 1;
    }

  
  if( ch->master != NULL && IS_NPC( ch ) )
    lch = (ch->master->leader != NULL) ? ch->master->leader : ch->master;
  else
    lch = (ch->leader != NULL) ? ch->leader : ch;

  for ( gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room )
    {

    if ( !is_same_group( gch, ch ))
      continue;

    if(!IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
      {
      int cnt;
      xp = xp_compute( gch, victim )* gch->level / levelst;
      if( gch->pcdata->reincarnation > 0 )
       {
       for( cnt=0; cnt<gch->pcdata->reincarnation; cnt++)
         xp /= 2;
       gch->pcdata->allow_reincarnate = 0;
       }

      if( gch->level == 90 )  
        xp /= 2;
      if( gch->level > 90 && gch->level <= 95 )
        xp = 3 * xp / 4;

        /* God initiates  -  Chaos  4/18/99  */
      if( which_god(gch)==GOD_INIT_ORDER || which_god(gch)==GOD_INIT_CHAOS )
        {
        int found_follower;
        CHAR_DATA *tgch;

        found_follower = GOD_NEUTRAL;
        for ( tgch=gch->in_room->first_person; tgch!=NULL; 
              tgch=tgch->next_in_room )
        if ( is_same_group( gch, tgch ) && gch!=tgch && !IS_NPC(tgch) )
          if( which_god(tgch)==GOD_CHAOS || which_god(tgch)==GOD_ORDER )
            {
            found_follower =  which_god(tgch );
            break;
            }
        if( found_follower == GOD_NEUTRAL )
          {
          xp = (75-(ch->pcdata->reincarnation*10)) * xp / 100;
          sprintf( buf, "You receive %d experience points.\n\r", xp );
          }
        else
          if( which_god( gch ) == GOD_INIT_CHAOS && found_follower==GOD_CHAOS )
            {
            xp = (95-(ch->pcdata->reincarnation*8)) * xp / 100;
            sprintf( buf, "With assistance from Chaos, you receive %d experience points.\n\r", xp );
            }
        else
          if( which_god( gch ) == GOD_INIT_ORDER && found_follower==GOD_ORDER )
            {
            xp = (95-(ch->pcdata->reincarnation*8)) * xp / 100;
            sprintf( buf, "With Order's assistance, you receive %d experience points.\n\r", xp );
            }
        else
            {
            xp = (70-(ch->pcdata->reincarnation*13)) * xp / 100;
            sprintf( buf, "Your god only allows you %d experience points.\n\r", xp );
            }
        }
     else
      sprintf( buf, "You receive %d experience points.\n\r", xp );

      send_to_combat_char( buf, gch );
      gain_exp( gch, xp );
      }

   if( !IS_NPC( gch ) )
    for ( obj = gch->first_carrying; obj != NULL; obj = obj_next )
      {
      obj_next = obj->next_content;
      if ( obj->wear_loc == WEAR_NONE )
        continue;

      if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch)    )
      ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch)    )
      ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch) ) )
        {
        act( "You can no longer wear $p until your alignment is fixed.", gch, obj, NULL, TO_CHAR );
        act( "$n is zapped by $p.",   gch, obj, NULL, TO_ROOM );
        if ( obj == get_eq_char( victim, WEAR_WIELD )
         &&  (tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD)) != NULL )
         tmpobj->wear_loc = WEAR_WIELD;
        unequip_char(gch, obj);
        }
      }
    }

  return;
}


/*
 * Calculate roughly how much experience a character is worth
 */
int get_exp_worth( CHAR_DATA *ch )
{
    int exp;

    exp = ch->level * ch->level * ch->level * 5;
    exp += ch->max_hit;
    exp -= (ch->armor-50) * 2;
    if (IS_NPC ( ch ) )
      exp += ( ch->npcdata->damnodice * ch->npcdata->damsizedice + 
	         GET_DAMROLL(ch) ) * 50;
    exp += GET_HITROLL(ch) * ch->level * 10;
    if ( IS_AFFECTED(ch, AFF_SANCTUARY) )
      exp += exp * 1.5;
    exp = URANGE( MIN_EXP_WORTH, exp, MAX_EXP_WORTH );

    return exp;
}


/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
  int xp;
  int spread;

  if( gch->in_room == NULL )
    return( 0 );
  if( IS_SET( gch->in_room->room_flags, ROOM_RIP ) )
    return( 0 );
  if( IS_SET( gch->in_room->room_flags, ROOM_IS_CASTLE ) )
    return( 0 );
  if (IS_NPC(victim) && (victim->pIndexData->vnum==9900 ))
    return( 0 );

  xp=(int)((float)exp_level(CLASS_MONSTER,victim->level)*
           ((float)(number_range(0,40)-20)/100.0+1.0));
  if(!IS_NPC(victim))
    xp=0;
  if(xp<75)
    xp=75;
  spread=10+gch->level/8;
  if(gch->level>victim->level)
    xp=xp * spread / ( gch->level - victim->level + spread );

/*
    xp    = (get_exp_worth( victim )
          *  URANGE( 0, (victim->level - gch->level) + 10, 13 )) / 10; */

  if( gch->in_room != NULL && gch->in_room->sector_type == SECT_ASTRAL )
    xp = xp + (xp * 35 / 100);
  if( gch->in_room != NULL && gch->in_room->sector_type == SECT_ETHEREAL )
    xp = xp + (xp * 30 / 100);

    xp = number_range( (xp*3) >> 2, (xp*5) >> 2 );
    /* reduce exp for killing the same mob repeatedly           -Martin 
    if ( !IS_NPC( gch ) )
    {
        int times = times_killed( gch, victim );

        if ( times >= 20 )
           xp = 0;
        else
        if ( times )
        {
           xp = (xp * (20-times)) / 20;
           if ( times > 15 )
             xp /= 3;
           else
           if ( times > 10 )
             xp >>= 1;
        }
    } */

    /*
     * semi-intelligent experienced player vs. novice player xp gain
     * "bell curve"ish xp mod 
     * based on time played vs. level - Martin 21/12/98 
    if ( !IS_NPC( gch ) && gch->level > 5 )
    {
        xp_ratio = (int) gch->played / gch->level;
        if ( xp_ratio < 16000 )         
            xp = (xp*3) >> 2;
        else
        if ( xp_ratio < 10000 )         
            xp >>= 1;
        else
        if ( xp_ratio < 5000 )          
            xp >>= 2;
        else
        if ( xp_ratio < 3500 )          
            xp >>= 3;
        else
        if ( xp_ratio < 2000 )          
            xp >>= 4;
    }   */
  return xp;
}



char * const attack_table[] =
	{
		"hit",
		"slice",  "stab",  "slash", "whip", "claw",
		"blast",  "pound", "crush", "grep", "bite",
		"pierce", "hit",
            "combination punch",  "palm punch",  "thrust kick",
            "spinning back-hand",  "jump kick",  "round house",
            "knee",       "claw",     "bite",
            "head butt",  "elbow slam", "uppercut",
            "hooves" 
	};

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
  CHAR_DATA *fch;
  char buf1[256], buf2[256], buf3[256], bufc[256];
  char youHit[41],hitYou[41];
  const char *vs;
  const char *vp;
  const char *attack;
  char punct;
  int percent;
  int blen;

  percent = 100 * dam / victim->max_hit;

  attack = NULL;
  if ( dam ==   0 ) { 
	vs = "miss";    
	vp = "misses";          
	}
  else if ( percent <=   2 ) { 
	vs = "scratch"; 
	vp = "scratches";       
	}
  else if ( percent <=   4 ) { 
	vs = "graze";   
	vp = "grazes";          
	}
  else if ( percent <=   6 ) { 
	vs = "hit";     
	vp = "hits";            
	}
  else if ( percent <=   9 ) { 
	vs = "injure";  
	vp = "injures";         
	}
  else if ( percent <=  12 ) { 
	vs = "wound";   
	vp = "wounds";          
	}
  else if ( percent <=  15 ) { 
	vs = "maul";       
	vp = "mauls";           
	}
  else if ( percent <=  18 ) { 
	vs = "decimate";        
	vp = "decimates";       
	}
  else if ( percent <=  21 ) { 
	vs = "devastate";       
	vp = "devastates";      
	}
  else if ( percent <=  25 ) { 
	vs = "maim";    
	vp = "maims";           
	}
  else if ( percent <=  29 ) { 
	vs = "MUTILATE";        
	vp = "MUTILATES";       
	}
  else if ( percent <=  33 ) { 
	vs = "DISEMBOWEL";      
	vp = "DISEMBOWELS";     
	}
  else if ( percent <=  37 ) { 
	vs = "* EVISCERATE *";      
	vp = "* EVISCERATES *";     
	}
  else if ( percent <=  41 ) { 
	vs = "** MASSACRE **";        
	vp = "** MASSACRES **";       
	}
  else if ( percent <= 50 ) { 
	vs = "*** DEMOLISH ***";
	vp = "*** DEMOLISHES ***";                      
	}
  else if ( percent <= 70 ) { 
	vs = "**** ANNIHILATE ****";
	vp = "**** ANNIHILATES ****";             
	}
  else { 
	vs = "**** *OBLITERATE* ****";
	vp = "**** *OBLITERATES* ****";             
	}

  punct   = (percent <= 18) ? '.' : '!';

  youHit[0]='\0';
  hitYou[0]='\0';
  if(victim->vt100==1)
    if((dam>0) && (victim->ansi==1))
    {
      /* determine color of messages */
      if(victim->pcdata->color[4]!=0)
	sprintf( hitYou, "\033[1;%d;%dm", victim->pcdata->color[0][4],
	     victim->pcdata->color[1][4]);
    }
  if(ch->vt100==1)
    if((dam>0) && (ch->ansi==1))
    {
      /* determine color of messages */
      if(ch->pcdata->color[5]!=0)
	sprintf( youHit, "\033[1;%d;%dm", ch->pcdata->color[0][5],
	    ch->pcdata->color[1][5]);
    }

  if ( dt == TYPE_HIT )
  {
    if( ch->level>50 && dam>0)
      sprintf( buf2, "%sYou %s $N for %dhp%c", youHit,   vs, dam, punct );
    else
      sprintf( buf2, "%sYou %s $N%c", youHit,   vs, punct );

    if( IS_NPC( ch ) && IS_SET(ch->pIndexData->act, ACT_BODY) &&
      ch->pIndexData->attack_parts!=0)
    {
      int part, dir;
      char buf4[200];

      part=number_range( 0, MAX_BODY -1);
      dir=(number_range(0,1)*2)-1;
      while( !IS_SET( ch->pIndexData->attack_parts, 1<<part))
      {
        part+=dir;
        if(part<0)
          part=MAX_BODY-1;
        if(part>=MAX_BODY)
          part=0;
      }

      sprintf( buf4, body_table[part].attack, capitalize(get_name(ch)));
      sprintf( buf1, "%s %s %s%c", capitalize(buf4), vp,
        get_name( victim ), punct );

      sprintf( buf4, body_table[part].attack, capitalize(get_name(ch)));
      if(victim->level>50 && dam>0)
        sprintf( buf3, "%s%s %s you for %dhp%c", hitYou, buf4, vp, dam, punct);
      else
        sprintf( buf3, "%s%s %s you%c", hitYou, buf4, vp, punct );
    }
    else
    {
      strcpy( bufc, get_name( ch ) );
      sprintf( buf1, "%s %s %s%c",   bufc, vp, get_name( victim ), punct );
      if(victim->level>50 && dam>0)
        sprintf( buf3, "%s%s %s you for %dhp%c", hitYou,  
          IS_NPC( ch ) ? capitalize( ch->short_descr ) : 
          get_name( ch ), vp, dam, punct );
      else
        sprintf( buf3, "%s%s %s you%c", hitYou,  
          IS_NPC( ch ) ? capitalize( ch->short_descr ) : 
          get_name( ch ), vp, punct );
    }
  }
  else if (dt == TYPE_NOMESSAGE || dt==TYPE_NOFIGHT )
    return;
  else
  {
    if( dt == gsn_martial_arts )
    {
      if( martial_arts_attack > 0 )
        attack  = attack_table[12+martial_arts_attack];
      else
        attack  = attack_table[18-martial_arts_attack];
    }
    else
      if( dt == TYPE_HOOVES )
        attack = attack_table[25];
      else if ( dt >= 0 && dt < MAX_SKILL )
        attack  = skill_table[dt].noun_damage;
      else if ( dt >= TYPE_HIT )
      {
        OBJ_DATA *obj;
        if (dual_flip == FALSE && get_eq_char(ch, WEAR_DUAL_WIELD) != NULL)
          obj = get_eq_char( ch, WEAR_DUAL_WIELD );
        else
          obj = get_eq_char( ch, WEAR_WIELD );

        if(  obj->pIndexData->attack_string!=NULL)
          attack = obj->pIndexData->attack_string;
        else
          if(dt<TYPE_HIT+sizeof(attack_table)/sizeof(attack_table[0]) )
            attack  = attack_table[dt - TYPE_HIT];
      }
      else
      {
        if( dt!=TYPE_UNDEFINED)
	  bug( "Dam_message: bad dt %d.", dt );
        dt  = TYPE_HIT;
	attack  = attack_table[0];
      }

    strcpy(bufc, capitalize(get_name( ch )) );
    sprintf(buf1, "%s's %s %s %s%c", bufc, attack, vp, get_name(victim), punct);

    if( ch->level>50 && dam>0)
      sprintf( buf2, "%sYour %s %s $N for %dhp%c", youHit,attack,vp,dam,punct );
    else
		  sprintf( buf2, "%sYour %s %s $N%c",  youHit,   attack, vp, punct );
    if( victim->level > 50 && dam>0)
      {
      sprintf(buf3,"%s%s's %s %s you for %dhp%c",hitYou,
                IS_NPC( ch ) ? capitalize( ch->short_descr ) : 
                get_name( ch ), attack, vp, dam, punct );
      }
    else
      {
      sprintf(buf3,"%s%s's %s %s you%c",hitYou,
                IS_NPC( ch ) ? capitalize( ch->short_descr ) : 
                get_name( ch ), attack, vp, punct );
      }
	  }

    /*  Spam code for combat      -  Chaos 12/20/94    */
    blen = strlen( buf1 );
    for( fch=ch->in_room->first_person ; fch != NULL ; fch=fch->next_in_room )
      if( !IS_NPC( fch ) && fch != ch && fch != victim )
       if( fch->position > POS_SLEEPING )
        {
        bool  gch, gvic;
        gch = is_same_group( ch, fch );
        gvic = is_same_group( victim, fch );
        if( gch && !gvic )
          {
          if( dam <= 0 && !IS_SET( fch->pcdata->spam, 32))
            send_to_char( buf1, fch );
          if( dam > 0 && !IS_SET( fch->pcdata->spam, 16))
            send_to_char( buf1, fch );
          } 
        else
          if( !gch && gvic )
            {
            if( dam <= 0 && !IS_SET( fch->pcdata->spam, 128))
              send_to_char( buf1, fch );
            if( dam > 0 && !IS_SET( fch->pcdata->spam, 64))
              send_to_char( buf1, fch );
            } 
        else
          if( !gch && !gvic )
            {
            if( dam <= 0 && !IS_SET( fch->pcdata->spam, 512))
              send_to_char( buf1, fch );
            if( dam > 0 && !IS_SET( fch->pcdata->spam, 256))
              send_to_char( buf1, fch );
            } 
       else
         send_to_combat_char( buf1, fch );
       }

    if( dam == 0 )
      {
      if( IS_NPC( ch ) || !IS_SET( ch->pcdata->spam, 2 ))
       if( ch->position > POS_SLEEPING )
	      act( buf2, ch, "", victim, TO_CHAR );
      if( IS_NPC( victim ) || !IS_SET( victim->pcdata->spam, 8 ))
       if( victim->position > POS_SLEEPING )
	      act( buf3, ch, "", victim, TO_VICT );
      }
    else
      {
      if( IS_NPC( ch ) || !IS_SET( ch->pcdata->spam, 1 ))
       if( ch->position > POS_SLEEPING )
	      act( buf2, ch, "", victim, TO_CHAR );
      if( IS_NPC( victim ) || !IS_SET( victim->pcdata->spam, 4 ))
       if( victim->position > POS_SLEEPING )
	      act( buf3, ch, "", victim, TO_VICT );
      }
	return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	OBJ_DATA *obj,*tmpobj;

	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
		return;
        if ( ( tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL
           &&     number_bits( 1 ) == 0 )
          obj = tmpobj;

	if (IS_SET(obj->extra_flags,ITEM_INVENTORY))
		return;

	if ( get_eq_char( ch, WEAR_WIELD ) == NULL && number_bits( 1 ) ==
	    0 )
		return;

	/*   if( IS_SET(ch->attack, 1))      Temp fix by Chaos 12/17/93
      return;
    else
      ch->attack+=1;   */

	act( "$n disarms you!", ch, NULL, victim, TO_VICT    );
	act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

	if( !IS_NPC(victim) && victim->pcdata->learned[gsn_rearm] > number_percent())
	{
		act( "You rearm yourself!!!", ch, NULL, victim, TO_VICT);
		act( "$N rearms $Mself!!!", ch, NULL, victim, TO_CHAR );
		act( "$N rearms $Mself!!!", ch, NULL, victim, TO_NOTVICT
		    );
		return;
	}
    if ( obj == get_eq_char( victim, WEAR_WIELD )
    &&  (tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

  unequip_char( victim, obj);
	return;
}



/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void do_trip( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
        int cnt;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}

	cnt = multi( ch, gsn_trip);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "Huh?!\n\r", ch);
		return;
	}

	if ( ( victim = who_fighting(ch) ) == NULL )
	{
		send_to_combat_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

        if ( IS_NPC(ch) 
          || (number_percent()< (ch->pcdata->learned[gsn_trip])
          && check_hit( ch, victim, 0-GET_HITROLL(ch), gsn_trip ) ) )
	WAIT_STATE( ch, skill_table[gsn_trip].beats*200/(200+ch->mclass[cnt]));
    
	{
		act( "$n trips you and you go down!", ch, NULL, victim, TO_VICT
		    );
		act( "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR
		    );
		act( "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT
		    );

		WAIT_STATE( victim, PULSE_VIOLENCE/2 );
		victim->position = POS_RESTING;
	}
		WAIT_STATE( ch,     PULSE_VIOLENCE/2 );

	return;
}
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_SET(ch->attack, 8))
		return;
	else
		ch->attack+=8;

	if ( victim->wait == 0 )
	{
		act( "$n trips you and you go down!", ch, NULL, victim, TO_VICT
		    );
		act( "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR
		    );
		act( "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT
		    );

		WAIT_STATE( ch,     PULSE_VIOLENCE/2 );
		WAIT_STATE( victim, PULSE_VIOLENCE/6 );
		victim->position = POS_RESTING;
	}

	return;
}



void do_kill( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool found;

	if( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}
    if(find_keeper( ch ) !=NULL)
    {
     ch_printf(ch, "%s tells you 'No fighting in here! You thug!'\n\r", capitalize(find_keeper(ch)->short_descr));
     ch->reply=find_keeper( ch );
     return;
    }

	if(! IS_NPC(ch))
	  ch->pcdata->just_died_ctr=0;


      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
      /* if wounded below 50% - Presto 06/22/98 */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < (ch->max_hit/2) )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

  one_argument( argument, arg );
  found=FALSE;
  victim = NULL;
  if ( argument[0] == '\0' )
  {
    for(victim=ch->in_room->first_person;victim!=NULL;victim=victim->next_in_room)
      if(IS_NPC( victim) && ch!=victim &&
         ch->level<=victim->level-(ch->level*2+10))
      {
        sprintf(buf, "%s doesn't think you should fight in here.\n\r",
	              capitalize(victim->short_descr));
        send_to_combat_char( buf, ch);
        return;
      }
	
    for(victim=ch->in_room->first_person;found==FALSE && victim!=NULL;
        victim=victim->next_in_room)
      if(can_see(ch, victim) && !IS_AFFECTED( victim, AFF_CHARM) &&
         IS_NPC( victim) && ch!=victim && !is_same_group(ch,victim))
      {
        found=TRUE;
        break;
      }

    if(!found)
    {
      send_to_combat_char( "You cannot see anyone you can attack.\n\r", ch);
      return;
  }
	  }

	if(!found)
		if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
		  {
			send_to_combat_char( "They aren't here.\n\r", ch );
			return;
		  }

	if ( !IS_NPC(victim) && ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
	  {
    send_to_combat_char( "You must MURDER a player.\n\r", ch);
    return;
	  }
	else
	  {
		if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
		  {
			if( victim->master == ch && IS_SET( victim->act, ACT_PET))
	      {
	      raw_kill( victim );
	      return;
	      }
		  send_to_combat_char( "You cannot kill a slave.\n\r", ch );
		  return;
		  }
    }

	if ( victim == ch )
	{
		send_to_combat_char( "You hit yourself.  Ouch!\n\r", ch );
		multi_hit( ch, ch, TYPE_UNDEFINED );
		return;
	}

	if ( is_safe( ch, victim ) )
    {
          act("You shouldn't try that here.",ch,NULL,NULL,TO_CHAR);
	  return;
    }

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
		act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR
		    );
		return;
	}

  if( (!IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
          capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }


  if( IS_NPC( victim) && victim->fighting!=NULL && 
             (!IS_NPC( ch ) || IS_AFFECTED(ch,AFF_CHARM)) &&
             !is_same_group( ch, victim->fighting->who ))
  {
    sprintf( buf ,"%s seems to be in combat already!\n\r", capitalize(victim->short_descr));
    send_to_combat_char( buf, ch);
    return;
  }

  if ( ch->position == POS_FIGHTING )
  {
    if((who_fighting(ch)==victim) ||
       IS_NPC(ch) ||
       (number_percent() > ch->pcdata->learned[gsn_attack] ))
    {
      send_to_combat_char( "You do the best you can!\n\r", ch);
      return;
    }
/*    Stop a fight if you try to kill during a fight???  Chaos - 4/4/99
    else
      stop_fighting(ch,FALSE); */
  }

     if( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) )
          {
	  WAIT_STATE( ch, PULSE_VIOLENCE*2 );
          return;
          } 

	WAIT_STATE( ch, PULSE_VIOLENCE/2 );
  if(!IS_NPC(ch) &&
     ch->mclass[CLASS_ASSASSIN]==ch->level &&
     number_percent () < (float)ch->pcdata->learned[gsn_quick_draw]*.95)
    {
      OBJ_DATA * obj;
	if((obj=get_eq_char(ch,WEAR_WIELD))!=NULL
	    &&(obj->value[3]==11||obj->value[3]==2))
	{
	 if( number_percent( ) < ch->pcdata->learned[gsn_knife] )
	  one_hit( ch, victim, gsn_knife );
	 else
	  damage( ch, victim, 0, gsn_knife );
	}
    }
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
	send_to_combat_char( "If you want to MURDER, spell it out.\n\r", ch );
	return;
}

void add_to_victory_list( CHAR_DATA *ch, CHAR_DATA *victim)
{
  int i;
  char buf[MAX_STRING_LENGTH];
  i=victory_list_current-1;
  if (i<0) i=VICTORY_LIST_SIZE-1;
  STRFREE (victory_list[i] );
  sprintf(buf, "Z%12s (%dx Level %2d) %12s (%dx Level %2d) %s", ch->name, ch->pcdata->reincarnation, ch->level, victim->name, victim->pcdata->reincarnation, victim->level, (char *) ctime( &current_time)); 
  buf[strlen(buf)-1]='\0';
  victory_list[i] = STRALLOC(buf);
  victory_list_current=i;
  save_victors();
  return;
}


bool check_race_war( CHAR_DATA *ch, CHAR_DATA *victim )
{

  if( IS_NPC( ch ) || IS_NPC( victim ) )
    return( FALSE );

  if( ch->pcdata->army_status != 1 || victim->pcdata->army_status != 1 )
    return( FALSE );

  if( victim->race == Race_Battle_List[ ch->race ] )
    return( TRUE );

  /*  Changing to random based wars  -  Chaos 6/22/97  
  switch( ch->race )
    {
    case RACE_HUMAN:
      if( victim->race == RACE_HALFLING )
        return( TRUE );
      else
        return( FALSE );
    case RACE_HALFLING:
      if( victim->race == RACE_HUMAN )
        return( TRUE );
      else
        return( FALSE );
    case RACE_ELF:
      if( victim->race == RACE_OGRE )
        return( TRUE );
      else
        return( FALSE );
    case RACE_DROW:
      if( victim->race == RACE_GNOME )
        return( TRUE );
      else
        return( FALSE );
    case RACE_DWARF:
      if( victim->race == RACE_ORC )
        return( TRUE );
      else
        return( FALSE );
    case RACE_GNOME:
      if( victim->race == RACE_DROW )
        return( TRUE );
      else
        return( FALSE );
    case RACE_ORC:
      if( victim->race == RACE_DWARF )
        return( TRUE );
      else
        return( FALSE );
    case RACE_OGRE:
      if( victim->race == RACE_ELF )
        return( TRUE );
      else
        return( FALSE );
    default:
      return( FALSE );
    }      */

  return( FALSE );
}



void do_murder( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}

      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < (ch->max_hit/2) )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;


	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "Murder whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_combat_char( "Suicide is a mortal sin.\n\r", ch );
		return;
	}

        if(find_keeper( ch)!=NULL)
      	{
		ch_printf(ch, "%s tells you 'No fighting in here! You thug!'\n\r", capitalize(find_keeper(ch)->short_descr));
                ch->reply=find_keeper( ch );
		return;
	}
	if ( is_safe( ch, victim ) )
		return;

	if( IS_AFFECTED( victim, AFF_CHARM) && victim->master == ch)
	  {
           if(IS_SET(victim->act,ACT_PET))
	    {
              raw_kill( victim);
   	      return;
	    }
           else
            {
             send_to_combat_char( "You cannot murder your slave.\n\r", ch);
             return;
            }
	  }
        else if( IS_AFFECTED( victim, AFF_CHARM) )
         {
          send_to_combat_char( "You cannot murder someone's pet.\n\r", ch);
          return;
         }

	if( IS_AFFECTED( ch, AFF_CHARM) )
	{
	  send_to_combat_char( "You're too charmed to murder right now.\n\r",
	    ch);
	  return;
	}

	if ( ch->position == POS_FIGHTING )
	{
		send_to_combat_char( "You do the best you can!\n\r", ch );
		return;
	}
    if( !IS_NPC(victim))
       if(victim->pcdata->just_died_ctr > 0)
      {
        send_to_char("That person is currently protected by the gods.\n\r", ch);
        return;
      }

  if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) && ( !IS_NPC( ch ) ||
        IS_AFFECTED( victim, AFF_CHARM ) ) )
     return;

 if (!IS_NPC(ch) && !IS_NPC(victim) && victim->fighting != NULL )
  {
   send_to_combat_char( "You may not murder that person, at the moment.\n\r", ch);
   return;
  }

	/* Lagging players for murdering gives Rangers an immense advantage
	   over all other classes. If a spell caster or knifer does the murder
	   he/she must wait two rounds before being able to actually attack,
	   whereas rangers start hammering away right away.
	   - Martin 7/8/98  */
        
           WAIT_STATE( ch, PULSE_VIOLENCE*2 );

  if (!IS_NPC(ch)) 
   ch->pcdata->just_died_ctr = 0;
  /* if they are an assassin with K flag, ignore the following checks */

 if (!can_attack (ch, victim) )
  {
   send_to_combat_char( "You may not murder that person, at the moment.\n\r", ch);
   return;
  }
  
  if ((victim->mclass[CLASS_ASSASSIN] == victim->level &&
     victim->level != 95 && IS_SET(victim->act, PLR_KILLER) &&
     ch->level <= victim->level+10))

    /* Took out gang-banging against Asns  -  Chaos  4/17/99  */
  if( victim->fighting!=NULL  || victim->hit<victim->max_hit/4 )
   {
    send_to_combat_char( "You may not murder that person, at the moment.\n\r", ch);
    return;
   }

  if ( !IS_NPC(victim ) && victim->pcdata->corpse != NULL)
   {
    char buf12[180];
    sprintf( buf12, "%s has been killed quite recently.\n\r", 
             get_name( victim ) );
    send_to_combat_char( buf12, ch );
    return;
   }

  /* Code for total and max pvnum attacks  -  Chaos 5/6/99  */
  if( !IS_NPC(ch) && !IS_NPC(victim) )
    if( !check_add_attack( ch, victim ) )
      {
       char buf12[180];
       sprintf( buf12, "%s has been attacked too many times today.  You should leave them alone.\n\r", 
                get_name( victim ) );
       send_to_combat_char( buf12, ch );
       return;
      }


    if( !IS_NPC(victim))
    {
        if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE )
        if( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_DEMISE || which_god(victim) == GOD_POLICE )
          if( which_god(victim) != which_god(ch) )
          {
            char buf2 [MAX_INPUT_LENGTH];
            sprintf(buf2, "%s", get_name(ch));

	    sprintf( buf, "%s battles 'Help!  I am being attacked by %s!'", 
              get_name(victim), buf2);
	    do_battle( buf );
          }
          else
          {
	    sprintf( buf, "Help!  I am being attacked by %s!", get_name( ch ) );
	    do_shout( victim, buf );
	    do_chat( victim, buf );

            victim->wait = 0;  /* remove time penalty of shout */
          }
	  check_killer( ch, victim );
    }
   
  if ( IS_AFFECTED(victim, AFF2_HAS_FLASH) && !IS_NPC(ch) && !IS_NPC(victim))
  {
    if (number_percent() < victim->pcdata->learned[gsn_flash_powder]/2)
    {
     AFFECT_DATA af;
     int attempt;
     ROOM_INDEX_DATA *was_in;
     ROOM_INDEX_DATA *now_in;

     REMOVE_BIT(victim->affected2_by, 0-AFF2_HAS_FLASH); 
    act( "$n throws some powder onto the ground and there is a bright flash!", victim, NULL, NULL, TO_ROOM );
    act( "You throw some powder onto the ground and there is a bright flash!", victim, NULL, ch, TO_CHAR );

     if (!is_affected(ch, gsn_truesight))
     {
      af.type=gsn_flash_powder;
      af.bitvector = AFF_BLIND;
      af.duration=0;
      af.modifier=0;
      affect_to_char(ch, &af);
     }
     else
      {
      act( "$N is unaffected by the bright flash!", ch, NULL, victim, TO_VICT );
      act( "You ignore the effects of the flash.", ch, NULL, victim, TO_CHAR );
      }
 

  was_in = victim->in_room;

  for ( attempt = 0; attempt < 6; attempt++ )
    {
    EXIT_DATA *pexit;
    int door;

    door = number_door( );
    if ( ( pexit = was_in->exit[door] ) == 0
        ||   pexit->to_room == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || ( IS_NPC(victim)
        &&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
      continue;

    if( IS_NPC(victim) && pexit->to_room->area != victim->in_room->area )
      continue;

    move_char( victim, door );
    if ( ( now_in = victim->in_room ) == was_in )
      continue;

    char_from_room( victim );
    char_to_room( victim, was_in );
    char_from_room( victim );
    char_to_room( victim, now_in );
    }
    if (victim->in_room != was_in)
      return;
   }
   else
   {
     send_to_char( "You fumble the flash powder and it spills everywhere!\n\r", victim);
     REMOVE_BIT(victim->affected2_by, 0-AFF2_HAS_FLASH); 
   }
  }


     if( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) )
          {
	  WAIT_STATE( ch, PULSE_VIOLENCE*2 );
          return;
          } 

  if(!IS_NPC(ch) &&
     number_percent () < (float)ch->pcdata->learned[gsn_quick_draw]*.75)
    {
      OBJ_DATA * obj;
	if((obj=get_eq_char(ch,WEAR_WIELD))!=NULL
	    &&(obj->value[3]==11||obj->value[3]==2))
	{
	 if( number_percent( ) < ch->pcdata->learned[gsn_knife] )
	  one_hit( ch, victim, gsn_knife );
	 else
	  damage( ch, victim, 0, gsn_knife );
	}
    }
  /* if they are set to auto backstab then do it for murder */
  if(!IS_NPC(ch) && ch->pcdata->auto_flags!=AUTO_OFF &&
     skill_lookup(ch->pcdata->auto_command)==gsn_backstab &&
     victim->hit == victim->max_hit &&
     ch->mclass[CLASS_ROGUE]==ch->level)
    backstab(ch,victim);
  else
    multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void backstab(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int cnt;
	cnt = multi( ch, gsn_backstab);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char(
		    "You are not that practiced a cut-throat.\n\r", ch
		    );
		return;
	}
  WAIT_STATE( ch, skill_table[gsn_backstab].beats*200/(200+ch->mclass[cnt]));

/* Anatomy skill - Martin 4/8/98 */
  if (IS_NPC(victim) && !IS_NPC(ch) 
      && (victim->level<(ch->mclass[CLASS_ROGUE]-10)) 
      && number_percent()<ch->pcdata->learned[gsn_anatomy]/12)
    {
      act( "$n thrusts $s $p into $N, killing $M instantly!", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_ROOM);
      act( "You thrust your $p into $N, killing $M instantly!", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_CHAR);
      one_hit( ch, victim, gsn_anatomy );
      return;
    }
  one_hit( ch, victim, gsn_backstab );
  return;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char( "You cannot do that here.\n\r", ch);
    return;
    }
  one_argument( argument, arg );


      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;


  if ( arg[0] == '\0' )
    {
    send_to_combat_char( "Backstab whom?\n\r", ch );
    return;
    }

  if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
    {
    send_to_combat_char( "They aren't here.\n\r", ch );
    return;
    }

  if ( victim == ch )
    {
    send_to_combat_char( "How can you sneak up on yourself?\n\r", ch);
    return;
    }

  if ( is_safe( ch, victim ) )
    return;

  if( !IS_NPC(victim) && ( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) ||
      !IS_NPC(ch)))
    if ( ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA )
      {
      send_to_combat_char( "You may not backstab another player.\n\r", ch);
      return;
      }
  if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if( IS_NPC( victim) && IS_AFFECTED( victim, AFF_CHARM))
      {
      send_to_combat_char( "You can't do that to a pet.\n\r", ch);
      return;
      }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
      ||  ( obj->value[3] != 11   &&  obj->value[3] != 2))
    {
    send_to_combat_char( "You need to wield a piercing weapon.\n\r", ch );
    return;
    }

  if ( victim->fighting != NULL )
    {
    send_to_combat_char( "You can't backstab a fighting person.\n\r", ch );
  	 return;
    }

  if (( victim->hit < victim->max_hit )/*&&(victim->distracted<5)*/)
    {
    act( "$N is hurt and suspicious ... you can't sneak up.",
        ch, NULL, victim, TO_CHAR );
    return;
    }
  if(!IS_NPC(ch))
    if( 1 > ch->pcdata->learned[gsn_backstab] )
      {
      act( "You cannot backstab anything.",
          ch, NULL, victim, TO_CHAR );
      return;
      }

  if( !IS_NPC(ch) && !IS_NPC(victim) )
	if ( !can_reincarnate_attack( ch, victim) )
	{
	send_to_combat_char( "They have not reached your spiritual ability yet.\n\r", ch );
		return;
	}

  if( (!IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;
    char buf[MAX_INPUT_LENGTH];

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
           capitalize( victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }
     if( !is_affected(ch, gsn_truesight) && has_mirror( ch, victim ) )
    {
    WAIT_STATE( ch, PULSE_VIOLENCE*2 );
    return;
    } 

  backstab(ch,victim);
  return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim, *fch;
  int attempt, exp;
  char buf[MAX_INPUT_LENGTH];

  if( ch->wait > 0 )
    return;

  if ( ( victim = who_fighting(ch) ) == NULL )
    if(!IS_NPC(ch) || IS_AFFECTED( ch, AFF_CHARM))
      {
      send_to_combat_char( "You aren't fighting anyone.\n\r", ch );
      return;
      }

  was_in = ch->in_room;

  /* Slight lag for fleers */
  WAIT_STATE( ch, PULSE_VIOLENCE/3 );

  if( IS_NPC( ch ) || (ch->class != CLASS_MONK &&
      !(ch->race == RACE_AVIARAN && IS_OUTSIDE(ch))))
  {
  for ( attempt = 0; attempt < 6; attempt++ )
    {
    EXIT_DATA *pexit;
    int door;

    door = number_door( );
    if ( ( pexit = was_in->exit[door] ) == 0
        ||   pexit->to_room == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || ( IS_NPC(ch)
        &&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
      continue;

    if( IS_NPC(ch) && pexit->to_room->area != ch->in_room->area )
      continue;

    move_char( ch, door );
    if ( ( now_in = ch->in_room ) == was_in )
      continue;

    if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE)
            && number_percent()<(40-get_curr_int(ch)))

      {
       send_to_char( "You blunder around and get lost in the smoke!\n\r", ch);
       continue;
      }

    char_from_room( ch );
    char_to_room( ch, was_in );
    act( "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, now_in );
    act( "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);

    if ( !IS_NPC(ch) )
      {
      exp = ch->level*ch->level*2+ch->level*10+50;
      sprintf( buf, "You flee from combat!  You lose %d exp.\n\r",
          exp);
      send_to_combat_char( buf, ch);
      gain_exp( ch, 0-exp);
      }

    /* Chaos 11/19/93 */
    if(was_in != now_in)
      for(fch = was_in->first_person; fch != NULL; fch = fch->next_in_room)
        if(fch->fighting && 
	   who_fighting (fch) == ch &&
	   IS_AFFECTED (fch, AFF_HUNT) &&
	   !IS_AFFECTED (fch, AFF_CHARM) )

          if((IS_NPC(fch) && number_percent()<50) ||
             (!IS_NPC(fch) && number_percent()<
              (fch->pcdata->learned[gsn_hunt] *
	       (45+fch->mclass[multi(fch,gsn_hunt)]/2)/100)))
            {
            send_to_combat_char("You hunt them down.\n\r", fch);
            char_from_room( fch );
            char_to_room( fch, ch->in_room );
            send_to_combat_char("You have been hunted down.\n\r", ch);

  	    stop_fighting( ch, TRUE);
            stop_fighting( fch, TRUE);

            set_fighting( ch, fch);
            set_fighting( fch, ch); 
            return;
            }

    if( was_in == now_in)
      {
      if( !IS_NPC( ch))
        {
        exp = ch->level*ch->level*1+20;
        sprintf( buf, "You failed!  You lose %d exp.\n\r", exp);
        send_to_combat_char( buf, ch);
        gain_exp( ch, -exp);
        }
      return;
      }
   if (victim!=NULL)
        if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE )
        if( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_DEMISE || which_god(victim) == GOD_POLICE )
          if( which_god(victim) != which_god(ch) )
    {
     char buf2 [MAX_INPUT_LENGTH];
     sprintf(buf2, "%s", get_name(ch));
     if (ch->desc == NULL)
       sprintf(buf, "%s's arcane connection to the realm has been severed like a coward while in battle with %s!", buf2, get_name(victim));
     else
       sprintf(buf, "%s has fled like a coward from the wrath of %s!", buf2, get_name(victim));

     do_battle( buf );
    }
    leave_fighting( ch, was_in);
    return;
    }
  send_to_combat_char("Ah!!!  You can't find an exit!\n\r",ch);
  return;
  }
else
  {
  int door_count, door_cnt;
  /* ROOM_INDEX_DATA *location; */
  EXIT_DATA *pexit;
  int door;

      door_count = 0;
      now_in = NULL;

         /* Hey, we're testing directions and it's noisy */
      ALLOW_OUTPUT = FALSE;

      for( door = 0;   door < 6; door++ )
        if ( ( pexit = was_in->exit[door] ) == 0
            ||   pexit->to_room == NULL
            ||   pexit->to_room == was_in
            ||   IS_SET(pexit->exit_info, EX_CLOSED)
            || ( IS_NPC(ch)
            &&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
          continue;
        else
          {
          move_char( ch, door );
          if ( ( now_in = ch->in_room ) == was_in )
            continue;

          char_from_room( ch );
          char_to_room( ch, was_in );
          door_count++;
          }

      ALLOW_OUTPUT = TRUE;

      if( door_count == 0 )
        {
        send_to_combat_char( "You cannot flee from here.\n\r", ch );
        return;
        }

      door_cnt = number_range( 0, door_count - 1 );
      door_count = 0;
      for( door = 0;   door < 6; door++ )
        if ( ( pexit = was_in->exit[door] ) == 0
            ||   pexit->to_room == NULL
            ||   pexit->to_room == was_in
            ||   IS_SET(pexit->exit_info, EX_CLOSED)
            || ( IS_NPC(ch)
            &&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
          continue;
        else
          {
          move_char( ch, door );
          if ( ( now_in = ch->in_room ) == was_in )
            continue;

          char_from_room( ch );
          char_to_room( ch, was_in );
          if( door_count == door_cnt )
            {
            break;
            }
          door_count++;
          }
       }


    act( "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, now_in );
    act( "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );


    if ( !IS_NPC(ch) )
     {
     exp = ch->level*ch->level+ch->level*50+25;
     /* exp = ( exp_level( ch->class, ch->level+1 ) - exp_level( ch->class, ch->level ) ) * 0.03; */
      if (ch->level < 95) 
       {
        sprintf( buf, "You flee head over heels from combat, losing %d experience.\n\r", exp);
        send_to_combat_char( buf, ch);
        gain_exp( ch, 0-exp);
       }
      else
        send_to_combat_char( "You flee head over heels from combat.\n\r", ch);
      }

    /* Chaos 11/19/93 */
    if(was_in != now_in)
      for(fch = was_in->first_person; fch != NULL; fch = fch->next_in_room)
        if(fch->fighting && 
	   who_fighting (fch) == ch &&
	   IS_AFFECTED (fch, AFF_HUNT) &&
	   !IS_AFFECTED (fch, AFF_CHARM) )

          if((IS_NPC(fch) && number_percent()<50) ||
             (!IS_NPC(fch) && number_percent()<
              (fch->pcdata->learned[gsn_hunt] *
	       (45+fch->mclass[multi(fch,gsn_hunt)]/2)/100)))
            {
            send_to_combat_char("You hunt them down.\n\r", fch);
            char_from_room( fch );
            char_to_room( fch, ch->in_room );
            send_to_combat_char("You have been hunted down.\n\r", ch);

            stop_fighting( ch, TRUE);
            stop_fighting( fch, TRUE);

            set_fighting( ch, fch);
            set_fighting( fch, ch);
            return;
            }

   if( was_in == now_in)
   {
      if( !IS_NPC( ch))
        {
        exp = ch->level*ch->level*1+20;
	/*exp = ( exp_level( ch->class, ch->level+1 ) - exp_level( ch->class, ch->level ) ) * 0.01;*/
	if (ch->level<95)
          sprintf( buf, "You attempt fo flee from combat, losing %d exp.\n\r", exp);
	else
          sprintf( buf, "You attempt fo flee from combat, but can't escape .\n\r" );
        send_to_combat_char( buf, ch);
        gain_exp( ch, -exp);
        }
      return;
   }

        if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE )
        if( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_DEMISE || which_god(victim) == GOD_POLICE )
          if( which_god(victim) != which_god(ch) )
    {
     char buf2 [MAX_INPUT_LENGTH];
     sprintf(buf2, "%s", get_name(ch));

     if (ch->desc == NULL)
       sprintf(buf, "%s's arcane connection to the realm has been severed like a coward while in battle with %s!", buf2, get_name(victim));
     else
       sprintf(buf, "%s has fled like a coward from the wrath of %s!", buf2, get_name(victim));
     do_battle( buf );
    }
    stop_fighting( ch, TRUE);
    return;
}


void do_rescue( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "Rescue whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_combat_char( "What about fleeing instead?\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch) && IS_NPC(victim) && victim->master!=ch)
	{
		send_to_combat_char( "Doesn't need your help!\n\r", ch );
		return;
	}

	if ( !ch->fighting )
	{
		send_to_combat_char( "Too late.\n\r", ch );
		return;
	}

	if ( ( fch = who_fighting( victim ) ) == NULL )
	{
		send_to_combat_char( "That person is not fighting right now.\n\r",
		    ch );
		return;
	}

        if( !IS_NPC( victim ) && !IS_NPC( fch ) )
	{
		send_to_combat_char( "That person cannot be rescued.\n\r", ch );
		return;
	}

        if( multi( ch, gsn_rescue)==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "You cannot rescue.\n\r", ch );
		return;
	}


	WAIT_STATE( ch, skill_table[gsn_rescue].beats );
	if ((!IS_NPC(ch) && number_range(0,150) >
	    ch->pcdata->learned[gsn_rescue]) || !is_same_group( ch, victim))
	{
		send_to_combat_char( "You fail the rescue.\n\r", ch );
                ch_printf(victim, "%s tried to rescue you, but failed!\n\r", get_name(ch));
		return;
	}

	act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
	act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );

	/* Chaos fixed on  4/22/99  */
    stop_fighting( victim, FALSE );
    for(fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room)
      if( who_fighting( fch ) == victim)
        {
        stop_fighting( fch, FALSE );
        if ( who_fighting(ch)!=NULL )
          stop_fighting( ch, FALSE );
        set_fighting( fch, ch );
        set_fighting( ch, fch );
        set_fighting( victim, fch );
        }

    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int cnt;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}
      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;


	cnt = multi( ch, gsn_kick);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "You better leave the martial arts to fighters.\n\r", ch
		    );
		return;
	}

	if ( ( victim = who_fighting ( ch ) ) == NULL )
	{
		send_to_combat_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_kick].beats*200/(200+ch->mclass[cnt]));
    

	if ( IS_NPC(ch))
		damage( ch, victim, number_range( 1, ch->level ), gsn_kick);
	/* else if( number_percent( ) < ch->pcdata->learned[gsn_kick] ) */
	else if( number_percent( ) < ch->pcdata->learned[gsn_kick] &&
                 check_hit( ch, victim, 0-GET_HITROLL(ch), gsn_kick ) )
		damage( ch, victim, GET_DAMROLL(ch)+5+
		    number_range(ch->mclass[cnt],4*ch->mclass[cnt])/3, gsn_kick);
	else
		damage( ch, victim, 0, gsn_kick );

	return;
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance,cnt;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if (get_eq_char(ch, WEAR_SHIELD) == NULL)
    {
        send_to_char("You cannot bash without a shield!\n\r", ch );
        return;
    }

    cnt = multi( ch, gsn_kick);
    if(cnt==-1 && !IS_NPC(ch))
    {
     send_to_combat_char( "You better leave the martial arts to fighters.\n\r", ch);
     return;
    }

    if ( ( victim = who_fighting (ch) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

   chance = ((get_curr_dex(ch) + get_curr_str(ch))
           -  (get_curr_dex(victim)     + get_curr_str(victim)))  + 10;

    WAIT_STATE( ch, skill_table[gsn_bash].beats*200/(200+ch->mclass[cnt]));

  if ( (number_percent () < ch->pcdata->learned[gsn_bash]/2 )
       && check_hit(ch, victim, chance, gsn_bash) )
    {
        damage( ch, victim, number_range( ch->level, ch->level*2 ), gsn_bash );
        if (number_percent()<chance)
        {
          WAIT_STATE( victim, PULSE_VIOLENCE );
          victim->position = POS_RESTING;
        }
    }
    else
    {
        damage( ch, victim, 0, gsn_bash );
    }
    return;
}


void do_martial_arts( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int cnt, level;
        int dmgL, dmgH;
        int tshares, pshare;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}

      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	cnt = multi( ch, gsn_kick);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char(
		    "You better leave the martial arts to fighters.\n\r", ch
		    );
		return;
	}
      if( IS_NPC( ch ) )
        level = ch->level;
      else
        level = ch->mclass[ cnt ];

	if(!IS_NPC(ch) && level < 40 )
	{
		send_to_combat_char(
		    "You better leave the martial arts to fighters.\n\r", ch
		    );
		return;
	}

	if ( ( victim = who_fighting(ch) ) == NULL )
	{
		send_to_combat_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

        WAIT_STATE( ch, skill_table[gsn_martial_arts].beats);
    

            /* Find total shares */
    if( level < 50 )
      tshares = 8;
    else if( level < 60 )
      tshares = 15;
    else if( level < 70 )
      tshares = 21;
    else if( level < 80 )
      tshares = 26;
    else if( level < 90 )
      tshares = 30;
    else
      tshares = 33;

    pshare = number_range( 1, tshares );

    if( pshare <= 8 )
      martial_arts_attack = 1;
    else if( pshare <= 15 )
      martial_arts_attack = 2;
    else if( pshare <= 21 )
      martial_arts_attack = 3;
    else if( pshare <= 26 )
      martial_arts_attack = 4;
    else if( pshare <= 30 )
      martial_arts_attack = 5;
    else
      martial_arts_attack = 6;

    switch( martial_arts_attack )
      {
      case 2:
        dmgL = 5* level / 8;
        dmgH = 7 * level / 4;
        break;
      case 3:
        dmgL = 6 * level / 8;
        dmgH = 2 * level;
        break;
      case 4:
        dmgL = 7 * level / 8;
        dmgH = 9 * level / 4;
        break;
      case 5:
        dmgL = level;
        dmgH = 10 * level / 4;
        break;
      case 6:
        dmgL = 9 * level / 8;
        dmgH = 11 * level / 4;
        break;
      case 1:
      default:
        dmgL = level / 2;
        dmgH = 3 * level / 2;
        break;
      }

	if ( IS_NPC(ch))
	  damage( ch, victim, number_range( dmgL, dmgH), gsn_martial_arts);
	else
          if( number_percent( ) < ch->pcdata->learned[gsn_martial_arts] &&
              check_hit( ch, victim, 0-GET_HITROLL(ch), gsn_kick ) )
	    damage( ch, victim, GET_DAMROLL(ch)+5+
		    number_range(dmgL, dmgH), gsn_martial_arts);
	else
		damage( ch, victim, 0, gsn_martial_arts );

	return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent,cnt;

	cnt = multi( ch, gsn_disarm);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "You don't know how to disarm opponents.\n\r",
		    ch );
		return;
	}

	if ( ( victim = who_fighting(ch) ) == NULL )
	{
		send_to_combat_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	{
		send_to_combat_char( "Your opponent is not wielding a weapon.\n\r",
		    ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	percent = number_percent( ) + victim->level - ch->level;
	if ( IS_NPC(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2
	    / 3 )
		disarm( ch, victim );
	else
		send_to_combat_char( "You failed.\n\r", ch );
	return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
	send_to_combat_char( "If you want to SLAY, spell it out.\n\r", ch );
	return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "Slay whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
	{
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_combat_char( "Suicide is a mortal sin.\n\r", ch );
		return;
	}

	/*
        if ( IS_NPC(victim) && ch->level<99 && !IS_NPC(ch) )
	{
		send_to_combat_char( "You failed.\n\r", ch );
		return;
	}
       */
	if ( !IS_NPC(victim) && victim->level >= ch->level )
	{
		send_to_combat_char( "You failed.\n\r", ch );
		return;
	}

	act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR   );
	act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT   );
	act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);

/*        victim->fighting->who=ch;*/
	raw_kill( victim );
        stop_hating( ch );
        stop_fearing( ch );
        stop_hunting( ch );
	return;
}

void do_slaughter( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "Slay whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
	{
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_combat_char( "Suicide is a mortal sin.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) && ch->level<99 && !IS_NPC(ch) )
	{
		send_to_combat_char( "You failed.\n\r", ch );
		return;
	}

	if ( !IS_NPC(victim) && victim->level > ch->level )
	{
		send_to_combat_char( "You failed.\n\r", ch );
		return;
	}

	act( "You slaughter $M in cold blood!",  ch, NULL, victim, TO_CHAR   );
	act( "$n slaughters you in cold blood!", ch, NULL, victim, TO_VICT   );
	act( "$n slaughters $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);
  /*victim->fighting->who=ch;*/
	if ( !IS_NPC(victim)  )
	{
	 log_printf( "%s slaughtered by %s at %u",
			    victim->name, get_name( ch ) ,
			    victim->in_room->vnum );

	 if (victim->level < 2)
  	  gain_exp( victim, 0-(victim->exp/2));
	 else
 	  if(victim->exp>exp_level(victim->class,(victim->level)-1)+1)
	   gain_exp( victim, exp_level(victim->class, (victim->level)-1)+1-
		   victim->exp);
	  if(IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF))
	  {
  	   if(victim->level>1)
             if(victim->exp>exp_level(victim->class, (victim->level)-2)+1)
	       gain_exp( victim, exp_level(victim->class, (victim->level)-2)+1-
		   victim->exp);
	   if(victim->level<2)
	     gain_exp( victim, -victim->exp);
           if(IS_SET(victim->act, PLR_KILLER))
	     victim->act-=PLR_KILLER;
	   if(IS_SET(victim->act, PLR_THIEF))
	     victim->act-=PLR_THIEF;
	  }
	 }
	raw_kill( victim );
	return;
}

void do_suicide( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
  bool killer, thief;
  killer=FALSE;
  thief=FALSE;
  if(!IS_NPC(ch))
    {
    if( IS_SET( ch->act, PLR_KILLER) )
      killer = TRUE;
    if( IS_SET( ch->act, PLR_THIEF) )
      thief = TRUE;
    }
  if(IS_AFFECTED(ch,AFF_CHARM)&&!IS_SET(ch->act,ACT_PET))
    {
    wipe_string( argument );
    return;
    }
  if(ch->fighting != NULL)
    {
    sprintf( arg, "%s is trying to kill you, and suicide is not an option.\n\r",
      ch->fighting->who->name );
    send_to_combat_char( arg, ch);
    wipe_string( argument );
    return;
    }
  if( IS_NPC( ch))
    {
    /*if( IS_AFFECTED( ch, AFF_CHARM))
      ch->fighting->who=ch->master;*/
    raw_kill( ch );
    wipe_string( argument );
    return;
    }
	one_argument_nolower( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "!!!  WARNING  !!!   This command is used to kill yourself.\n\rTo use you must enter in your password after the command.\n\r", ch );
    wipe_string( argument );
		return;
	}
  if(!strcasecmp(crypt(arg,ch->pcdata->pwd),ch->pcdata->pwd))
    {
    send_to_combat_char( "You commited suicide!\n\r", ch);
    /*ch->fighting->who=ch;*/
    raw_kill(ch);
    if( killer)
      SET_BIT( ch->act, PLR_KILLER);
    if( thief)
      SET_BIT( ch->act, PLR_THIEF);
    wipe_string( argument );
    return;
    }
  else
    {
    send_to_combat_char( "That was not your password.  Are you sure you want to commit suicide?\n\r", ch);
    wipe_string( argument );
    return;
    }
  wipe_string( argument );
  return;
}


/* klugee globals...-dug */
int fCRrNum,fCRvNum,fCRvCnt;

/*
 * Find a char in the room.
 */
CHAR_DATA *fCR_get_char_room(CHAR_DATA *ch,ROOM_INDEX_DATA *rm, char *argument
)
{
	CHAR_DATA *rch;

	if(IS_SET(rm->room_flags,ROOM_PRIVATE|ROOM_SAFE|ROOM_SOLITARY)||
	    (!IS_NPC(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT) && room_is_dark(rm)))
		return NULL;
	for(rch = rm->first_person; rch != NULL; rch = rch->next_in_room )
	{
		if(!can_see(ch,rch)||!is_name(argument,rch->name))
			continue;
		if(++fCRvCnt==fCRvNum)
			return rch;
	}
	for(rch = rm->first_person; rch != NULL; rch = rch->next_in_room )
	{
		if(!can_see(ch,rch)||!is_name_short(argument,rch->name))
			continue;
		if(++fCRvCnt==fCRvNum)
			return rch;
	}
	return NULL;
}

void findCharRoom(CHAR_DATA *ch,ROOM_INDEX_DATA *rm,CHAR_DATA **victim,int
dirNum,int shoot_range,char *chName)
{
	if(++fCRrNum>shoot_range)
		return;
	if((*victim=fCR_get_char_room(ch,rm,chName))==NULL)
	{
		if(rm->exit[dirNum]==NULL)
			return;
		if(IS_SET(rm->exit[dirNum]->exit_info, EX_CLOSED))
			return;
		if(rm->exit[dirNum]->to_room==NULL)
			return;
		findCharRoom(ch,rm->exit[dirNum]->to_room,victim,dirNum,shoot_range,
		    chName);
	}
	return;
}

int findCharDir(CHAR_DATA *ch,CHAR_DATA **victim, int dirNum, char *chName)
{
	char arg[MAX_INPUT_LENGTH];
	int shoot_range;

	fCRrNum=0;
	fCRvCnt=0;
	fCRvNum=number_argument(chName,arg);
	if(IS_NPC(ch))
		shoot_range=MED_RANGE;
	else
		shoot_range=ch->pcdata->learned[gsn_long_range]>0?LONG_RANGE:
		    ch->pcdata->learned[gsn_med_range]>0?MED_RANGE:SHORT_RANGE;
	if(ch->in_room->exit[dirNum]==NULL)
		return 0;
	if(IS_SET(ch->in_room->exit[dirNum]->exit_info, EX_CLOSED))
		return 0;
	if(ch->in_room->exit[dirNum]->to_room==NULL)
		return 0;
	findCharRoom(ch,ch->in_room->exit[dirNum]->to_room,victim,dirNum,
	    shoot_range,arg);
	return fCRrNum;
}

int getDirNumber(char *dirName)
{
	int dirLen;

	dirLen=strlen(dirName);
	if(dirLen==0)
		return -1;
	if(strncmp(dirName,"north",dirLen)==0)
		return DIR_NORTH;
	if(strncmp(dirName,"east",dirLen)==0)
		return DIR_EAST;
	if(strncmp(dirName,"south",dirLen)==0)
		return DIR_SOUTH;
	if(strncmp(dirName,"west",dirLen)==0)
		return DIR_WEST;
	if(strncmp(dirName,"up",dirLen)==0)
		return DIR_UP;
	if(strncmp(dirName,"down",dirLen)==0)
		return DIR_DOWN;
	return -1;
}

void do_shoot( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *ammo,*ammo2,*weapon;
	int cnt,ammoNum,dirNum,distance,dam;
	char *dirName,*chName,*arg,buf1[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];


      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	cnt = multi(ch,gsn_shoot);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char("You'd better leave the shooting to the assassins.\n\r",
		    ch );
		return;
	}
	if(ch->wait!=0)
	{
		send_to_combat_char("You fumble around and don't get any shots off!\n\r",
		    ch);
		return;
	}
	if(IS_SET(ch->in_room->room_flags,ROOM_SAFE))
	{
		send_to_combat_char("You can't shoot from this location!\n\r",ch);
		return;
	}
	if(ch->fighting!=NULL)
	/*	if(ch->in_room==(ch->fighting->who)->in_room)*/
		{
			send_to_combat_char( "You are too distracted to shoot!\n\r", ch );
			return;
		}
	weapon=get_eq_char(ch,WEAR_WIELD);
	if( weapon==NULL)
	{
		send_to_combat_char( "You must wield a weapon to shoot!\n\r", ch);
		return;
	}
	if(weapon->item_type!=ITEM_WEAPON)
	{
		send_to_combat_char( "You must wield a weapon to shoot!\n\r", ch);
		return;
	}
	if(weapon->value[0]==0)
	{
		send_to_combat_char("You are not wielding a weapon that can shoot!\n\r", ch);
		return;
	}
	ammoNum=weapon->value[0];

	for(ammo=ch->first_carrying,ammo2=NULL;ammo!=NULL;ammo=ammo->next_content)
	{
		if(ammo->item_type==ITEM_AMMO)
			if(ammo->value[0]==ammoNum)
				break;
		if(ammo->item_type==ITEM_CONTAINER)
			if((ammo->first_content!=NULL)&&!IS_SET(ammo->value[1], CONT_CLOSED))
			{
				for(ammo2=ammo->first_content;ammo2!=NULL;ammo2= ammo2->next_content)
				{
					if(ammo2->item_type==ITEM_AMMO)
						if(ammo2->value[0]==ammoNum)
						{
							OBJ_DATA *ammoTmp;
							ammoTmp=ammo;
							ammo=ammo2;
							ammo2=ammoTmp;
							break;
						}
				}
				if(ammo2!=NULL)
					break;
			}
	}
	if(ammo==NULL)
	{
		send_to_combat_char("Ooops...forgot to stock up on ammunition, eh?\n\r", ch);
		return;
	}
	arg=one_argument(argument,buf1);
	one_argument(arg,buf2);
	if((dirNum=getDirNumber(buf1))==-1)
	{
		if((dirNum=getDirNumber(buf2))==-1)
		{
			send_to_combat_char( "Shoot which direction?\n\r", ch );
			return;
		}
		dirName=buf2;
		chName=buf1;
	}
	else
	{
		dirName=buf1;
		chName=buf2;
	}
	if (chName[0]=='\0')
	{
		send_to_combat_char( "Shoot at whom?\n\r", ch );
		return;
	}
	victim=NULL; /* to get rid of warnings */
	distance=findCharDir(ch,&victim,dirNum,chName);
	if(victim==NULL)
	{
		send_to_combat_char("Your victim must have slipped out of sight!\n\r", ch);
		return;
	}
	if(ch==victim)
	{
		send_to_combat_char("Shooting yourself could prove painful.\n\r", ch );
		return;
	}
      if( ch->in_room->area != victim->in_room->area )
	{
		send_to_combat_char("They are too far to shoot at.\n\r", ch );
		return;
	}
	if(distance==0)
	{
		send_to_combat_char("They are too close to shoot at!\n\r", ch );
		return;
	}
	if(IS_NPC(victim)&&victim->shot_aware)
	{
		send_to_combat_char("Your target is too wary to be shot at!\n\r", ch);
		return;
	}
  if( !IS_NPC(victim) && !IS_NPC(ch) && 
       ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
    {
    send_to_combat_char( "You may not shoot another player.\n\r", ch);
    return;
    }
  if( IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
	!is_same_group( ch, victim->fighting->who ))
    {
    sprintf( buf ,"%s seems to be busy!\n\r", capitalize(victim->short_descr));
    send_to_combat_char( buf, ch);
    return;
    }

  if( (!IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
           capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }

	/* set wait time for speed of archer...*/
	if(ch->mclass[cnt]==-1 && !IS_NPC(ch) )
    {
    send_to_combat_char( "You can't do that!\n\r", ch);
    return;
    }
	if(!IS_NPC(ch))
		WAIT_STATE( ch, ammo->value[2]/(1+(ch->mclass[cnt]/10)));
	else
		WAIT_STATE( ch, ammo->value[2]);
	if(ammo2==NULL)
		obj_from_char(ammo);
	else
	{
		act( "$n pulls $p from $P.", ch, ammo, ammo2, TO_ROOM );
		act( "You pull $p from $P.", ch, ammo, ammo2, TO_CHAR );
		obj_from_obj(ammo);
	}
	if(IS_NPC(victim))
	{
		victim->shot_timer=2;
		victim->shot_from=rev_dir[dirNum];
	}
	if(IS_NPC(ch))
	{
		if(number_percent()<30)
			extract_obj( ammo );
		else
			obj_to_char( ammo, victim);
		dam=number_range( 1, ammo->value[1]*ammo->value[3]/distance);
	}
	else if(number_percent( ) < ch->pcdata->learned[gsn_shoot] &&
               check_hit( ch, victim, 0, gsn_shoot) )
	{
		if(number_percent()<25)
			extract_obj( ammo );
		else
			obj_to_char( ammo, victim);
		dam=number_range( 1, ammo->value[1]*ammo->value[3]/distance);
	}
	else
	{
		if ( IS_SET( ammo->extra_flags, ITEM_INVENTORY ) &&(number_percent()<
		    25))
			extract_obj( ammo );
		else
      {
      ammo->sac_timer=OBJ_SAC_TIME;
			obj_to_room(ammo,victim->in_room);
      }
		dam=0;
	}
	  mprog_range_trigger(victim,ch);
	damage(ch,victim,dam,gsn_shoot);
  stop_fighting( ch, TRUE);
	return;
}

void do_throw( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *weapon;
	int cnt,dirNum,distance,dam;
	char buf1[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH],buf3[MAX_INPUT_LENGTH],
	    *arg,*dirName,*chName,*objName,buf[MAX_INPUT_LENGTH];
	bool wielded;


      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	cnt = multi(ch,gsn_throw);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char("You decide not to throw...might hurt your back.\n\r",
		    ch );
		return;
	}
	if(ch->wait!=0)
	{
		send_to_combat_char("You fumble around and don't throw a thing!\n\r",
		    ch);
		return;
	}
	if(IS_SET(ch->in_room->room_flags,ROOM_SAFE))
	{
		send_to_combat_char("You can't throw from this location!\n\r",ch);
		return;
	}
	if(ch->fighting!=NULL)
	/*	if(ch->in_room==(ch->fighting->who)->in_room)*/
		{
			send_to_combat_char( "You are too distracted to throw!\n\r",
			    ch );
			return;
		}
	buf1[0]='\0';
	buf2[0]='\0';
	buf3[0]='\0';
	arg=one_argument(argument,buf1);
	arg=one_argument(arg,buf2);
	arg=one_argument(arg,buf3);
	if((dirNum=getDirNumber(buf1))==-1)
	{
		if((dirNum=getDirNumber(buf2))==-1)
		{
			if((dirNum=getDirNumber(buf3))==-1)
			{
				send_to_combat_char( "Throw which direction?\n\r",
				    ch );
				return;
			}
			else
			{
				dirName=buf3;
				chName=buf2;
				objName=buf1;
			}
		}
		else
		{
			chName=buf3;
			dirName=buf2;
			objName=buf1;
		}
	}
	else
	{
		chName=buf3;
		objName=buf2;
		dirName=buf1;
	}
	if (chName[0]=='\0')
	{
		send_to_combat_char( "Throw what at whom?\n\r", ch );
		return;
	}
	victim=NULL; /* to get rid of warnings */
	distance=findCharDir(ch,&victim,dirNum,chName);
	if(victim==NULL)
	{
		send_to_combat_char("Your victim must have slipped out of sight!\n\r",
		    ch);
		return;
	}
	if(ch==victim)
	{
		send_to_combat_char("You can't throw that at yourself!\n\r", ch
		    );
		return;
	}
      if( ch->in_room->area != victim->in_room->area )
	{
		send_to_combat_char("They are too far to throw things at.\n\r", ch );
		return;
	}
	if(distance==0)
	{
		send_to_combat_char("They are too close to throw at!\n\r", ch );
		return;
	}
	if(IS_NPC(victim)&&victim->shot_aware)
	{
		send_to_combat_char("You can't seem to get a good sight of your target!\n\r",
		    ch);
		return;
	}
  if( !IS_NPC(victim) && !IS_NPC(ch) &&
       ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
    {
    send_to_combat_char( "You may not throw at another player.\n\r", ch);
    return;
    }
  if( IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
	!is_same_group( ch, victim->fighting->who ))
    {
    sprintf( buf ,"%s seems to be busy!\n\r", capitalize(victim->short_descr));
    send_to_combat_char( buf, ch);
    return;
    }
  if( (!IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
          capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }
	wielded=FALSE;
	weapon=get_eq_char(ch,WEAR_WIELD);
	if(weapon!=NULL)
		if(is_name(objName,weapon->name))
			wielded=TRUE;
	if(weapon!=NULL && !wielded)
		if(is_name_short(objName,weapon->name))
			wielded=TRUE;
	if(!wielded)
	{
		if(!remove_obj(ch,WEAR_WIELD,TRUE, FALSE))
		{
			send_to_combat_char("You can't throw with another weapon wielded!\n\r",
			    ch);
			return;
		}
		for(weapon=ch->first_carrying; weapon!=NULL; weapon=weapon->next_content)
		{
			if(weapon->item_type==ITEM_WEAPON)
				if(is_name(objName,weapon->name))
					break;
		}
		for(weapon=ch->first_carrying; weapon!=NULL; weapon=weapon->next_content)
		{
			if(weapon->item_type==ITEM_WEAPON)
				if(is_name_short(objName,weapon->name))
					break;
		}
	}
	if(weapon==NULL)
	{
		send_to_combat_char("Hmmm...you don't seem to have that weapon.\n\r",
		    ch);
		return;
	}
	if(weapon->value[3]!=2&&weapon->value[3]!=6&&weapon->value[3]!=7 &&
     weapon->value[3]!=8&&weapon->value[3]!=11)
	{
		send_to_combat_char("You can't throw that type of weapon!",ch);
		return;
	}

	if(!wielded)
		WAIT_STATE( ch, skill_table[gsn_throw].beats );
	else
		WAIT_STATE( ch, skill_table[gsn_throw].beats/2 );
	if(IS_NPC(victim))
		victim->shot_timer=2;
	dam=1;
	if(IS_NPC(ch))
	{
		obj_from_char(weapon);
		if(number_percent()<20)
			extract_obj( weapon );
		else
			obj_to_char( weapon, victim);
	}
	else if(number_percent( ) < ch->pcdata->learned[gsn_throw] &&
               !check_hit( ch, victim, 0, gsn_throw )  )
	{
		obj_from_char(weapon);
		if(number_percent()<5)
			extract_obj( weapon );
		else
			obj_to_char( weapon, victim);
	}
	else
	{
		obj_from_char(weapon);
		if ( IS_SET( weapon->extra_flags, ITEM_INVENTORY ) &&(number_percent()<
		    5))
			extract_obj( weapon );
		else
      {
      weapon->sac_timer=OBJ_SAC_TIME;
			obj_to_room( weapon, victim->in_room);
      }
		dam=0;
	}
	if(dam==1)
		one_hit(ch,victim,gsn_throw);
	else
		damage(ch,victim,0,gsn_throw);
	if(victim->position!=POS_DEAD)
		mprog_range_trigger(victim,ch);
  stop_fighting( ch, TRUE);
	return;
}

void do_divert( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;


      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	if ( !IS_NPC(ch) && multi(ch,gsn_divert)==-1)
	{
		send_to_combat_char( "You can't do that!\n\r", ch );
		return;
	}

	if(ch->fighting==NULL)
	{
		send_to_combat_char( "You aren't fighting!\n\r", ch );
		return;
	}
	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_combat_char( "Divert to whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_combat_char( "That's stupid.\n\r", ch );
		return;
	}

	if(( victim->level < ch->level )&&(!is_same_group(ch,victim)))
	{
		send_to_combat_char( "You can't divert to a lower level character!\n\r",
		    ch );
		return;
	}

	if ( ch->fighting->who == victim )
	{
		act( "Trying to make $m fight $mself?",  ch, NULL, victim,
		    TO_CHAR    );
		return;
	}

	if ( ( fch = who_fighting( victim ) ) == NULL )
	{
		send_to_combat_char( "That person is not fighting right now.\n\r",
		    ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_divert].beats );
	if ((!IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_divert])
	     || !IS_NPC(fch))
	{
		send_to_combat_char( "You fail the divert.\n\r", ch );
		return;
	}

	act( "You divert the attack to $N!",  ch, NULL, victim, TO_CHAR 
	    );
	act( "$n diverts the attack to you!", ch, NULL, victim, TO_VICT 
	    );
	act( "$n diverts the attack to $N!",  ch, NULL, victim, TO_NOTVICT
	    );

	/* Chaos 12/27/93  
	for( fch=first_char; fch!=NULL; fch=fch->next)
		if( fch->fighting->who == ch)
			fch->fighting->who = victim;*/

    	/* Re-done by Martin 9/19/99 */
	stop_fighting(victim, FALSE);
	if (ch->fighting)
	  stop_fighting(ch, FALSE);
	for( fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room)
	{
		if( fch->fighting && fch->fighting->who == ch)
		{
			stop_fighting(fch, FALSE);
			set_fighting(fch, victim);
		}
	}

	return;
}

void do_knife( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int cnt;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}

      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	cnt = multi( ch, gsn_knife);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "You'd better leave the nasty stuff to assassins.\n\r",
		    ch );
		return;
	}

	if((obj=get_eq_char(ch,WEAR_WIELD))==NULL
	    ||(obj->value[3]!=11&&obj->value[3]!=2))
	{
		send_to_combat_char( "You need to wield a piercing weapon.\n\r",
		    ch );
		return;
	}
	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	  {
	       if ( ( victim = who_fighting(ch) ) == NULL )
		{
			send_to_combat_char("You are not fighting anyone.\n\r",ch);
			return;
		}
          }
	else
	{
		victim=get_char_room_even_hidden(ch,arg);
		if(victim==NULL)
		{
			send_to_combat_char("Knife who?\n\r",ch);
			return;
		}
	}

    if( !IS_NPC(ch) && ( !IS_NPC(victim) || IS_AFFECTED( victim, AFF_CHARM)) )
     {
      if(ch->fighting==NULL || ( ch->fighting->who != victim ) )
	{
	send_to_combat_char( "You may not do that.\n\r", ch);
	return;
	}
     }

 if( IS_NPC( victim ))
 if ( victim->fighting && victim->fighting->who!=ch 
    && IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
  {
  if( victim->master == ch && IS_SET( victim->act, ACT_PET))
    {
    raw_kill( victim );
    return;
    }
  send_to_combat_char( "You cannot kill a slave.\n\r", ch );
  return;
  }

  if( victim->fighting != NULL && !is_same_group( victim->fighting->who, ch ))
    {
    send_to_combat_char( "They are currently busy.\n\r", ch );
    return;
    }
   
  if( ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      char buf[MAX_INPUT_LENGTH];
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
          capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }

     WAIT_STATE( ch, skill_table[gsn_knife].beats*100/(100+ch->mclass[cnt]));
	if ( IS_NPC(ch))
	  one_hit( ch, victim, gsn_knife );
	else if( number_percent( ) < ch->pcdata->learned[gsn_knife] )
	  one_hit( ch, victim, gsn_knife );
	else
	  damage( ch, victim, 0, gsn_knife );

	return;
}

void do_distract( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int cnt;
        char buf[MAX_INPUT_LENGTH];

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_combat_char( "You cannot do that here.\n\r", ch);
		return;
	}
	cnt = multi( ch, gsn_distract);
	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_combat_char( "They'd just get mad at you if you tried.\n\r",
		    ch );
		return;
	}
	one_argument(argument,arg);
	if((victim=get_char_room(ch,arg))==NULL)
	{
		send_to_combat_char("Distract who?\n\r",ch);
		return;
	}

  if( (!IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) &&
    ( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) ) 
    {
    CHAR_DATA *ch_ld;
    int pvnum_ld;

    ch_ld = ch;
    if( IS_NPC( ch ) )
      ch_ld = ch_ld->master;
    if( ch_ld->leader != NULL )
      ch_ld = ch_ld->leader;
    if( !IS_NPC( ch_ld ) )
      pvnum_ld=ch_ld->pcdata->pvnum;
    else
      pvnum_ld=0;
    if( pvnum_ld != victim->npcdata->pvnum_last_hit_leader )
      {
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
          capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }

	WAIT_STATE( ch, skill_table[gsn_distract].beats );
	if ( IS_NPC(ch)||
	    ( number_percent( ) < ch->pcdata->learned[gsn_distract]+
	    (ch->level-victim->level)*10 ))
	{
		send_to_combat_char("You convince them that their shoes are untied!\n\r",
		    ch);
		victim->distracted+=10;/* number of waits they will be distracted for */
		return;
	}
	else
	{
		send_to_combat_char( "They suddenly look very suspicious.\n\r", ch );
		act( "$n tries to distract you!", ch, NULL, victim, TO_VICT);
		victim->distracted=0;
		if(IS_NPC(victim)&&(victim->level>ch->level)&&(number_percent()>40))
		{
			act( "$n suddenly looks VERY angry!",victim,NULL,
			    NULL,TO_ROOM);
			if(victim->fighting!=NULL)
				stop_fighting(victim,FALSE);
			one_hit(victim,ch,TYPE_UNDEFINED);
			if((ch->position!=POS_DEAD)&&(number_percent()<60))
			{
				stop_fighting(victim,FALSE);
				one_hit(victim,ch,TYPE_UNDEFINED);
			}
		}
		return;
	}

	return;
}

CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
    if ( !ch )
    {
        bug( "who_fighting: null ch", 0 );
        return NULL;
    }

    if ( !ch->fighting )
      return NULL;

    return ch->fighting->who;
}

/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
  CHAR_DATA *fch;

  free_fight( ch );
  update_pos( ch );

  if (!fBoth)
   return; 

  for ( fch = first_char ; fch != NULL; fch = fch->next )
    if ( who_fighting( fch ) == ch ) 
      {
      free_fight( fch );
      update_pos( fch );
      }
  return;
}

void leave_fighting( CHAR_DATA *ch, ROOM_INDEX_DATA *was_in)
{
  CHAR_DATA *fch, *gch;
  bool done;

  stop_fighting( ch, TRUE );
  return;
  for(fch = was_in->first_person; fch != NULL; fch = fch->next_in_room)
    if(fch->fighting->who==ch && fch!= ch->master &&
       !is_same_group( ch, fch) && !( IS_NPC( ch) && IS_NPC( fch) &&
       !IS_AFFECTED( ch, AFF_CHARM) && !IS_AFFECTED( fch, AFF_CHARM )))
      {/* Found someone in the attacking party */
      done=FALSE;
      for(gch = was_in->first_person; !done && gch != NULL; gch = gch->next_in_room)
        if( gch!=ch && gch!=fch)
          if( is_same_group( ch, gch) || 
              ( IS_NPC( gch) && !IS_AFFECTED( gch, AFF_CHARM ) &&
                IS_NPC( ch) && !IS_AFFECTED( ch, AFF_CHARM) &&
                ch->fighting->who != NULL && is_same_group(ch->fighting->who,fch)))
            if( ( IS_NPC( gch) && !IS_AFFECTED( gch, AFF_CHARM )) ||
                (IS_NPC( fch) && !IS_AFFECTED( fch, AFF_CHARM)) )
              if( should_fight( fch, gch ))
                {/* Set attacking player to fight next opponent */
                fch->fighting->who=gch;
                done=TRUE;
                }
      if(!done)
        stop_fighting( fch, FALSE);
      }


  return;
}

bool should_fight( CHAR_DATA *ch, CHAR_DATA *victim)
  {
  if( IS_NPC( ch) && !IS_AFFECTED( ch, AFF_CHARM) && 
      (ch->pIndexData->vnum!=9900 || IS_AFFECTED(ch, AFF2_POSSESS))
      && (!IS_NPC( victim) || IS_AFFECTED( victim, AFF_CHARM)))
    if( ch->level +60 < victim->level)
      return( FALSE );
  return(TRUE);
  }

void do_auto( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_INPUT_LENGTH];
  int sn, cnt;

  if( IS_NPC( ch) || IS_AFFECTED( ch, AFF_CHARM))
    return;

  if( argument[0]=='\0')
    {
    send_to_combat_char( "Syntax:  AUTO [AUTO] [QUICK] [<skill/spell>]\n\r", ch);
    if( ch->pcdata->auto_command !=NULL || ch->pcdata->auto_command[0]=='\0')
      sprintf( buf, "Current command:  %s\n\r", ch->pcdata->auto_command);
    else
      sprintf( buf, "Current command:  Not defined\n\r" );
    send_to_combat_char( buf, ch);
    if( ch->pcdata->auto_flags == AUTO_OFF)
      send_to_combat_char( "Current mode:  Off\n\r", ch);
    if( ch->pcdata->auto_flags == AUTO_AUTO)
      send_to_combat_char( "Current mode:  Auto\n\r", ch);
    if( ch->pcdata->auto_flags == AUTO_QUICK)
      send_to_combat_char( "Current mode:  Quick\n\r", ch);
    return;
    }

  if( !strcasecmp( argument, "auto") )
    {
    send_to_combat_char( "Turning AUTO mode on.\n\r", ch);
    ch->pcdata->auto_flags=AUTO_AUTO;
    return;
    }
  
  if( !strcasecmp( argument, "quick") )
    {
    send_to_combat_char( "Turning QUICK mode on.\n\r", ch);
    ch->pcdata->auto_flags=AUTO_QUICK;
    return;
    }

  if( !strcasecmp( argument, "off") )
    {
    send_to_combat_char( "Turning auto modes off.\n\r", ch);
    ch->pcdata->auto_flags=AUTO_OFF;
    return;
    }

  /* scan for valid skill/spell */

    sn = skill_lookup( argument );
    if( sn < 0)
       {
       send_to_combat_char( "That is not a skill/spell.\n\r", ch );
       return;
       }
  /* Disallow ppl to do silly things like auto parry or auto short range
     Martin 7/8/98 */
    if ( skill_table[sn].minimum_position != POS_FIGHTING 
         && sn != gsn_backstab )
       {
       send_to_combat_char( "You cannot put that skill/spell on auto.\n\r", ch );
       return;
       }
      
    cnt = multi( ch, sn);

    if(cnt==-1 && !IS_NPC(ch))
       {
	send_to_combat_char( "You can't do that.\n\r", ch );
	return;
       }

  strcpy( argument, skill_table[sn].name);

  STRFREE (ch->pcdata->auto_command);
  ch->pcdata->auto_command = STRALLOC( argument );
  sprintf( buf, "Auto command set to: %s\n\r", argument );
  send_to_combat_char( buf, ch);
  return;
  }

void do_spam( CHAR_DATA *ch, char *arg)
  {
  int val;

  if( arg == NULL || arg[0]=='\0' )
    {
    send_to_combat_char( "SPAM control settings:\n\r", ch);
      send_to_combat_char( "Toggle  Status  Description\n\r", ch);

    if( !IS_SET( ch->pcdata->spam, 1))
      send_to_combat_char( "  A       On    You hit\n\r" , ch);
    else
      send_to_combat_char( "  A       Off   You hit\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 2))
      send_to_combat_char( "  B       On    You miss\n\r" , ch);
    else
      send_to_combat_char( "  B       Off   You miss\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 4))
      send_to_combat_char( "  C       On    They hit you\n\r" , ch);
    else
      send_to_combat_char( "  C       Off   They hit you\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 8))
      send_to_combat_char( "  D       On    They miss you\n\r" , ch);
    else
      send_to_combat_char( "  D       Off   They miss you\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 16))
      send_to_combat_char( "  E       On    Party hits\n\r" , ch);
    else
      send_to_combat_char( "  E       Off   Party hits\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 32))
      send_to_combat_char( "  F       On    Party misses\n\r" , ch);
    else
      send_to_combat_char( "  F       Off   Party misses\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 64))
      send_to_combat_char( "  G       On    They hit party\n\r" , ch);
    else
      send_to_combat_char( "  G       Off   They hit party\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 128))
      send_to_combat_char( "  H       On    They miss party\n\r" , ch);
    else
      send_to_combat_char( "  H       Off   They miss party\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 256))
      send_to_combat_char( "  I       On    Other hit\n\r" , ch);
    else
      send_to_combat_char( "  I       Off   Other hit\n\r" , ch);
    
    if( !IS_SET( ch->pcdata->spam, 512))
      send_to_combat_char( "  J       On    Other miss\n\r" , ch);
    else
      send_to_combat_char( "  J       Off   Other miss\n\r" , ch);
    
    if( IS_SET( ch->pcdata->spam, 1024))
      send_to_combat_char( "  K       On    Show party status line\n\r" , ch);
    else
      send_to_combat_char( "  K       Off   Show party status line\n\r" , ch);

    return;
    }

  val = -1;

  if( arg[0]=='a' || arg[0]=='A')
    val = 1;
  if( arg[0]=='b' || arg[0]=='B')
    val = 2;
  if( arg[0]=='c' || arg[0]=='C')
    val = 4;
  if( arg[0]=='d' || arg[0]=='D')
    val = 8;
  if( arg[0]=='e' || arg[0]=='E')
    val = 16;
  if( arg[0]=='f' || arg[0]=='F')
    val = 32;
  if( arg[0]=='g' || arg[0]=='G')
    val = 64;
  if( arg[0]=='h' || arg[0]=='H')
    val = 128;
  if( arg[0]=='i' || arg[0]=='I')
    val = 256;
  if( arg[0]=='j' || arg[0]=='J')
    val = 512;
  if( arg[0]=='k' || arg[0]=='K')
    val = 1024;
    
  if( val == -1 )
    {
    send_to_combat_char( "That is not an option.\n\r", ch );
    return;
    }
  if( IS_SET( ch->pcdata->spam, val ))
    ch->pcdata->spam -= val;
  else
    ch->pcdata->spam += val;

  do_spam( ch, "" );   /* show stat change */
   
  return;
  }

void do_assassin( CHAR_DATA *ch, char *argument )
  {
  send_to_combat_char( "If you want to ASSASSINATE, spell it out.\n\r", ch );
  return;
  }

void do_assassinate( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int exp_cost,level;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char( "You cannot do that here.\n\r", ch);
    return;
    }

  if( multi( ch, gsn_assassinate)==-1 || IS_NPC(ch) || 
      ch->class!=CLASS_ASSASSIN)
    {
    send_to_combat_char( "Only assassins can assassinate.\n\r", ch );
    return;
    }
  level=IS_NPC(ch) ? ch->level : ch->mclass[multi(ch,gsn_assassinate)];
  if(level!=ch->level)
    {
    send_to_combat_char( "Only pure assassins can assassinate.\n\r", ch );
    return;
    }


  argument=one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
    send_to_combat_char( "Assassinate whom?\n\r", ch );
    return;
    }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    send_to_combat_char( "They aren't here.\n\r", ch );
    return;
    }
    
  if ( victim == ch )
    {
    send_to_combat_char( "Suicide is a mortal sin.\n\r", ch );
    return;
    }

  if(find_keeper( ch)!=NULL)
      	{
		ch_printf(ch, "%s tells you 'No fighting in here! You thug!'\n\r", capitalize(find_keeper(ch)->short_descr));
                ch->reply=find_keeper( ch );
		return;
	}
  if ( is_safe( ch, victim ) || ch->level > victim->level+10 )
    {
    send_to_combat_char( "You may not assassinate them.\n\r", ch );
    return;
    }

  if( IS_AFFECTED( victim, AFF_CHARM) && victim->master == ch)
    {
    send_to_combat_char( "Um....ur...you do realize that you could just 'kill' it?\n\r", ch );
    return;
    }
  
  /* Increase cost to assassinate Peaceful first_person - Martin 25/12/98

  if (!IS_NPC(victim) && victim->pcdata->clan !=NULL &&
	victim->pcdata->clan->type == CLAN_PEACEFUL )
    exp_cost=(exp_level(ch->class,level)-exp_level(ch->class,level-1))/8;
  else */

    exp_cost=(exp_level(ch->class,level)-exp_level(ch->class,level-1))/10;

  if(ch->exp < exp_level(ch->class,level-1)+exp_cost )
    {
    ch_printf(ch,"You need a minimum of %d experience to assassinate %s!\n\r",
            exp_level(ch->class,level-1)+exp_cost, get_name(victim));
    return;
    }
  if( IS_AFFECTED( ch, AFF_CHARM) )
    {
    send_to_combat_char( "You're too charmed to assassinate right now.\n\r", ch);
    return;
    }

  if ( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You do the best you can!\n\r", ch );
    return;
    }

  if(victim->fighting!=NULL)
    {
    send_to_combat_char("You may not assassinate that person, at the moment.\n\r",ch);
    return;
    }

/*   Removed by Chaos   - 12/19/98 
  if( !IS_NPC(ch) && !IS_NPC(victim) )
	if ( !can_reincarnate_attack( ch, victim) )
	{
	send_to_combat_char( "They have not reached your spiritual ability yet.\n\r", ch );
		return;
	}
*/

  if( !IS_NPC(victim))
     if(victim->pcdata->just_died_ctr > 0)
    {
      send_to_char("That person is currently protected by the gods.\n\r", ch);
      return;
    }

  if ( !IS_NPC(victim ))
    if( victim->pcdata->corpse != NULL)
    {
    char buf12[180];
    sprintf( buf12, "%s has been killed quite recently.\n\r",
             get_name( victim ) );
    send_to_combat_char( buf12, ch );
    return;
    }

  /* Code for total and max pvnum attacks  -  Chaos 4/20/99  */
  if( !IS_NPC(ch) && !IS_NPC(victim) )
    {
    int oldest_attack, total_pvnum_attacks, attacks_today, cnt;

    attacks_today = 0;
    oldest_attack = 0;
    total_pvnum_attacks=0;
    for( cnt=0; cnt<MAX_PK_ATTACKS; cnt++)
      {
      if( victim->pcdata->last_pk_attack_time[cnt] == 0 )
          oldest_attack = cnt;
      else
        if( victim->pcdata->last_pk_attack_time[oldest_attack] > 
            victim->pcdata->last_pk_attack_time[cnt] )
          oldest_attack = cnt;
      if( victim->pcdata->last_pk_attack_time[cnt]+24*60*60 > current_time )
        attacks_today++;
      if( ch->pcdata->pvnum == victim->pcdata->last_pk_attack_pvnum[cnt] && 
          victim->pcdata->last_pk_attack_time[cnt]+24*60*60 > current_time )
        total_pvnum_attacks++;
      }

    if( attacks_today == MAX_PK_ATTACKS )
      {
       char buf12[180];
       sprintf( buf12, "%s has been attacked too many times today.  You should leave them alone.\n\r", 
                get_name( victim ) );
       send_to_combat_char( buf12, ch );
       return;
      }

    if( total_pvnum_attacks >= 6 )
      {
       char buf12[180];
       sprintf( buf12, "You have attacked %s too many times today.\n\r", 
                get_name( victim ) );
       send_to_combat_char( buf12, ch );
       return;
      }

    victim->pcdata->last_pk_attack_time[oldest_attack]=current_time;
    victim->pcdata->last_pk_attack_pvnum[oldest_attack]=ch->pcdata->pvnum;
    victim->pcdata->last_pk_attack_name[oldest_attack]=
          str_dup(capitalize(ch->name));
        
    }


  /* Let's protect the little first_person  - Chaos 12/19/98  */
  if( !IS_NPC(victim) && ( which_god(victim)==GOD_INIT_ORDER ||
      which_god(victim)==GOD_INIT_CHAOS) )
    {
    send_to_combat_char("You may not assassinate a neutral citizen of the Realm.\n\r",ch);
    return;
    }

  if(!IS_NPC(victim) && which_god( victim ) == GOD_POLICE )
    {
    send_to_combat_char("You may not assassinate that person.\n\r",ch);
    return;
    }

  /* ok...check and see if the victim has the specified item */
  argument=one_argument( argument, arg2 );
  if ( arg2[0] != '\0' )
    {
    OBJ_DATA *obj;

    if ( ( obj = get_obj_list( victim, arg2, victim->first_carrying ) ) == NULL )
      {
      send_to_combat_char( "They don't seem to have it!\n\r", ch );
      return;
      }

    if(!can_drop_obj( ch, obj) ||
       IS_SET(obj->extra_flags,ITEM_INVENTORY) ||
       obj->level>ch->level)
      {
      send_to_combat_char( "You wouldn't be able to pry it away.\n\r", ch );
      return;
      }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
      {
      send_to_combat_char( "You have your hands full.\n\r", ch );
      return;
      }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
      {
      send_to_combat_char( "You can't carry that much weight.\n\r", ch );
      return;
      }

    ch->asn_obj=obj;
    }
  else
    ch->asn_obj=NULL;

  /* We don't mind a lag on assassinate, it's an anti-social skill :)
     Martin 7/8/98 */  
  WAIT_STATE( ch, PULSE_VIOLENCE*1 );
  if( !IS_NPC(victim))
    {
    if (!IS_NPC(ch) && !(number_percent() < ch->pcdata->learned[gsn_muffle]/2))
     {
      sprintf( buf, "Help!  %s is trying to assassinate me!", get_name( ch ) );
      do_shout( victim, buf );
           do_chat( victim, buf );
      victim->wait = 4;  /* remove time penalty of shout */
     }
    else
     {
      act( "You quickly silence $N before they can call for help!", ch, NULL, victim, TO_CHAR);
      act( "You are silenced by $n before you can call for help!", ch, NULL, victim, TO_VICT);
     }
    check_killer( ch, victim );
    }
  if(IS_NPC(ch) || (number_percent( ) < ch->pcdata->learned[gsn_assassinate]))
   {

/* Quick Draw support....we grab what we need from do_knife. No extra lag
   here as the assassin will be lagged from assassinate anyway.
   Martin 7/8/98 */

  if(!IS_NPC(ch) && victim->hit == victim->max_hit &&
     ch->mclass[CLASS_ASSASSIN]==ch->level &&
     number_percent () < (float)ch->pcdata->learned[gsn_quick_draw]*.95)
    {
      OBJ_DATA * obj;
	if((obj=get_eq_char(ch,WEAR_WIELD))!=NULL
	    &&(obj->value[3]==11||obj->value[3]==2))
	{
	 if( number_percent( ) < ch->pcdata->learned[gsn_knife] )
	  one_hit( ch, victim, gsn_knife );
	 else
	  damage( ch, victim, 0, gsn_knife );
	}
    }
    multi_hit( ch, victim, TYPE_UNDEFINED );
   }
  else
    ch->asn_obj=NULL;
  gain_exp(ch,0-exp_cost);
  return;
  }

void do_gouge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    sh_int dam=0;
    int chance;
    int cnt;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }
    cnt = multi(ch, gsn_gouge);
    if ( cnt == -1 && !IS_NPC(ch)) 
     {
 	ch_printf(ch, "You haven't been taught this dirty trick!\n\r");
        return;
     }

    if ( ( victim = who_fighting(ch) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    chance = (( get_curr_dex(victim) - get_curr_dex(ch)) * 10) + 10;
    if ( IS_NPC(ch) 
        || (number_percent()< (ch->pcdata->learned[gsn_gouge]+ chance )
        && check_hit( ch, victim, 0-GET_HITROLL(ch), gsn_gouge ) ) )
    {
        dam = number_range( 5, ch->level );
        damage( ch, victim, dam, gsn_gouge );
        if ( ch->fighting && ch->fighting->who == victim && last_dead != victim)
        {
             if ( !IS_AFFECTED( victim, AFF_BLIND ) )
              {
                if (!is_affected(ch, gsn_truesight))
                {
                  af.type      = gsn_blindness;
                  af.location  = APPLY_HITROLL;
                  af.modifier  = -6;
                  af.duration = ( ch->mclass[multi(ch,gsn_gouge)]/2 ) / get_curr_con( victim );
                  af.bitvector = AFF_BLIND;
                  affect_to_char( victim, &af );
                  act( "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
                }
     else
      {
      act( "$N seems to be able to see despite your attack!", ch, NULL, victim, TO_CHAR );
      act( "You are unaffected by $n's attack.", ch, NULL, victim, TO_VICT );
      }
              }
                WAIT_STATE( ch,     PULSE_VIOLENCE );
/*
                if ( !IS_NPC( ch ) && !IS_NPC( victim ) )
                {
                  if ( number_bits( 1 ) == 0 )
                  {
                    ch_printf( ch, "%s looks momentarily dazed.\n\r",
                        victim->name );
                    send_to_char( "You are momentarily dazed ...\n\r",
                        victim );
                    WAIT_STATE( victim, PULSE_VIOLENCE );
                  }
                }
                else
                  WAIT_STATE( victim, PULSE_VIOLENCE );
*/
        }
        else
        if ( last_dead == victim)
        {
            act( "Your fingers plunge into your victim's brain, causing immediate death!",
                ch, NULL, NULL, TO_CHAR );
        }
    }
    else
    {
        WAIT_STATE( ch, skill_table[gsn_gouge].beats );
        damage( ch, victim, 0, gsn_gouge );
    }

    return;
}

void do_circle( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;


    if ( ( victim = who_fighting(ch) ) == NULL )
    {
        send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "How can you sneak up on yourself?\n\r", ch );
        return;
    }


    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[3] != 11 && obj->value[3] != 2 ) )
    {
        send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch
 );
        return;
    }

    if ( !ch->fighting )
    {
        send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
        return;
    }

    if ( !victim->fighting )
    {
        send_to_char( "You can't circle around a person who is not fighting.\n\r", ch );
        return;
    }

    if ( victim->num_fighting < 2 )
    {
    act( "You can't circle around them without a distraction.",
            ch, NULL, victim, TO_CHAR );
        return;
    }


    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( IS_NPC(ch) || (number_percent()< ch->pcdata->learned[gsn_circle] 
        && check_hit( ch, victim, 0-GET_HITROLL(ch), gsn_circle ) ) )
    {
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        one_hit( ch, victim, gsn_circle );
    }
    else
    {
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        damage( ch, victim, 0, gsn_circle );
    }
    return;
}

void do_berserk( CHAR_DATA *ch, char *argument )
  {
  int cnt;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char( "You cannot do that here.\n\r", ch);
    return;
    }
  cnt = multi( ch, gsn_berserk);
  if(!IS_NPC(ch) && ch->mclass[cnt]!=ch->level)
    {
    send_to_combat_char("You spit foam and roar but nobody seems to notice.\n\r", ch);
    return;
    }

  WAIT_STATE( ch, skill_table[gsn_berserk].beats );
    
  if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_berserk] )
    {
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch,AFF2_BERSERK) )
      {
      send_to_combat_char( "You are berserk!  Kill!  Kill!  Kill!\n\r", ch );
      return;
      }

    af.type      = gsn_berserk;
    af.duration  = 5;
    af.modifier  = 0 - (IS_NPC(ch)?ch->level:ch->mclass[cnt]) / 2 ;
    /* af.modifier  = (ch->armor>0)?ch->armor/4:-ch->armor/4; */
    af.location  = APPLY_AC;
    af.bitvector = AFF2_BERSERK;
    affect_to_char( ch, &af );
    af.type      = gsn_berserk;
    af.duration  = 5;
    af.modifier  = (IS_NPC(ch)?ch->level:ch->mclass[cnt]) / 2 ;
    /* af.modifier  = (ch->hitroll>0)?ch->hitroll/5:-ch->hitroll/5; */
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF2_BERSERK;
    affect_to_char( ch, &af );
    af.type      = gsn_berserk;
    af.duration  = 5;
    af.modifier  = (IS_NPC(ch)?ch->level:ch->mclass[cnt]) / 3 ;
    /* af.modifier  = (ch->damroll>0)?ch->damroll/5:-ch->damroll/5; */
    af.location  = APPLY_DAMROLL;
    af.bitvector = AFF2_BERSERK;
    affect_to_char( ch, &af );
    send_to_combat_char( "Now you REALLY wanna KILL!\n\r",ch);
    SET_BIT(ch->affected2_by, 0-AFF2_BERSERK);
    }
  else
    send_to_combat_char("You failed to go berserk!\n\r",ch);

  return;
  }

void do_rank( CHAR_DATA *ch, char *arg )
  {
  char buf[200];
  int rnk;

  if( *arg == '\0' && !IS_NPC( ch ) )
    {
    sprintf( buf, "Rank: %d\n\r", ch->rank );
    send_to_combat_char( buf, ch );
    return;
    }

  rnk = atol( arg );
  if( rnk > 10 )
    rnk = 10;
  if( rnk < 0 )
    rnk = 0;
  ch->rank = (unsigned char)rnk;

  if( !IS_NPC( ch ) )
    {
    sprintf( buf, "Rank: %d\n\r", ch->rank );
    send_to_combat_char( buf, ch );
    }

  return;

  }


void knight_adjust_hpmnmv( CHAR_DATA *ch )
  {
  int val_hp, val_mana, val_move;
  int tmp;

  val_hp = ch->actual_max_hit;
  val_mana = ch->actual_max_mana;
  val_move = ch->actual_max_move;

  if( val_mana < 100 && val_move < 100 )
    {
    tmp = 200 - val_move - val_mana;
    val_hp -= tmp;
    val_mana = 100;
    val_move = 100;
    }
  else
  if( val_mana < 100 )
    {
    tmp = 100 - val_mana;
    if( val_move - tmp/2 < 100 )
      {
      tmp -= ( val_move - 100 );
      val_move = 100;
      val_mana = 100;
      val_hp -= tmp;
      }
    else
      {
      val_mana = 100;
      val_move -= (tmp/2);
      val_hp -= (tmp/2);
      }
    }
  else
  if( val_move < 100 )
    {
    tmp = 100 - val_move;
    if( val_mana - tmp/2 < 100 )
      {
      tmp -= ( val_mana - 100 );
      val_move = 100;
      val_mana = 100;
      val_hp -= tmp;
      }
    else
      {
      val_move = 100;
      val_mana -= (tmp/2);
      val_hp -= (tmp/2);
      }
    }


  tmp = ch->actual_max_hit - ch->max_hit;
  ch->actual_max_hit = val_hp;
  ch->max_hit = val_hp - tmp;

  tmp = ch->actual_max_mana - ch->max_mana;
  ch->actual_max_mana = val_mana;
  ch->max_mana = val_mana - tmp;

  tmp = ch->actual_max_move - ch->max_move;
  ch->actual_max_move = val_move;
  ch->max_move = val_move - tmp;

  if( ch->hit > ch->max_hit )
    ch->hit = ch->max_hit;

  if( ch->mana > ch->max_mana )
    ch->mana = ch->max_mana;

  if( ch->move > ch->max_move )
    ch->move = ch->max_move;

  return;
  }

      /* Mixed race wars, this lists who fights who  --  Chaos  6/22/97 */
void do_war( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_INPUT_LENGTH];
  char fbuf[MAX_STRING_LENGTH];
  int tn;

  sprintf( fbuf, "                -------- Race War List --------\n\r" );
  for( tn=0; tn<MAX_RACE; tn++)
    if( Race_Battle_List[tn]>tn)
      {
      sprintf( buf, "      %20s  battles  %s\n\r",
        race_table[tn].race_name, race_table[Race_Battle_List[tn]].race_name);
       strcat( fbuf, buf );
      }
  send_to_char( fbuf, ch );
  return;
  }

void mix_race_war( void )
  {
  int rmonth, tn, pick;
  bool found;
  /* int cnt, ccnt, scnt; */
  CHAR_DATA *fch;

  rmonth = time_info.year*17 + time_info.month;
  srand( rmonth );

  for( tn=0; tn<MAX_RACE ; tn++)
   Race_Battle_List[tn]=-1;

  for( found=FALSE, tn=0; tn<MAX_RACE; tn++, found=FALSE)
    while( !found )
      {
      if( Race_Battle_List[tn]!=-1 )
        found = TRUE;
      else
        {
        pick = rand() % MAX_RACE;
        if( pick!=tn && Race_Battle_List[ pick ] == -1 )
          {
          found = TRUE;
          Race_Battle_List[ tn ] = pick;
          Race_Battle_List[ pick ] = tn;
          }
        }
      }

        /* We need to erase existing mixes 
      (Why remove old attacks?  Doesn't matter really.  - Chaos 6/30/97 )
    for( ccnt=0; ccnt<MAX_RACE; ccnt++)
     for( cnt=0; cnt<ARMY_LIST_SIZE; cnt++)
      {
      for( scnt=0; scnt<2; scnt++)
        {
        if( army_loses_name[scnt][ccnt][cnt]!=NULL )
          STRFREE (army_loses_name[scnt][ccnt][cnt] );
        army_loses_name[scnt][ccnt][cnt]=NULL;
        if( army_kills_name[scnt][ccnt][cnt]!=NULL )
          STRFREE (army_kills_name[scnt][ccnt][cnt] );
        army_kills_name[scnt][ccnt][cnt]=NULL ;
        }
      army_kills_amt[ccnt][cnt]=0;
      army_loses_amt[ccnt][cnt]=0;
      }  */

  for( fch=first_char; fch!=NULL; fch=fch->next )
    if( !IS_NPC( fch ) && fch->pcdata->army_status==1 )
      {
      send_to_char( "A new war begins!\n\r\n\r", fch );
      do_war( fch, "" );
      }

  return;
  }
      
bool can_reincarnate_attack( CHAR_DATA *ch, CHAR_DATA *victim )
  {

  if( IS_NPC(ch) || IS_NPC(victim) )
    return( TRUE );
   
  if ( ch->pcdata->reincarnation==0 && victim->pcdata->reincarnation==0 )
    return TRUE;
  else
  if ( ch->pcdata->reincarnation != 0 && victim->pcdata->reincarnation != 0)
    return TRUE;

  return( FALSE );
  }

/* Put all the attack permission stuff into one nice neat function
 Martin 2/11/98  actually, I dunno about the whole neat thing 8/11/98 Martin*/
 
bool can_attack( CHAR_DATA *ch, CHAR_DATA *victim)
{
  bool reincok=FALSE;

  if (IS_NPC(ch) || IS_NPC(victim))
    return TRUE;

  if (which_god(ch) == GOD_INIT_ORDER || which_god(ch) == GOD_INIT_CHAOS )
    return FALSE;
  if (which_god(victim)==GOD_INIT_ORDER || which_god(victim)==GOD_INIT_CHAOS )
    return FALSE;

  if ((which_god(victim) == GOD_DEMISE || which_god(ch) == GOD_POLICE)
      && victim->level>=ch->level*3/4  )
    return TRUE;

  if (which_god(ch) == GOD_DEMISE || which_god(victim) == GOD_POLICE)
    return FALSE;

  if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)  
    return TRUE;

  if ((victim->mclass[CLASS_ASSASSIN] == victim->level &&
     victim->level != 95 && IS_SET(victim->act, PLR_KILLER) &&
     ch->level <= victim->level+10))
    return TRUE;

  if ( ch->pcdata->reincarnation==0 && victim->pcdata->reincarnation==0 )
    reincok=TRUE;
  else
  if ( ch->pcdata->reincarnation != 0 && victim->pcdata->reincarnation != 0)
    reincok=TRUE;

        if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE )
        if( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_DEMISE || which_god(victim) == GOD_POLICE )
          if( which_god(victim) != which_god(ch) )
      if( reincok && victim->level>=ch->level*3/4 )
    return TRUE;


  if (( ch->pcdata->army_status == 1 && victim->pcdata->army_status == 1 
       && victim->race == Race_Battle_List[ch->race] ) && reincok
      && victim->level>=ch->level*3/4  )
    return TRUE;
  return FALSE;
}

  /* Centaur Hooves  - Presto 4/17/99  */
void centaur_hoof( CHAR_DATA *ch, CHAR_DATA *victim)
{
  int dam;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    return;

  if( IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit )
    if( !IS_AFFECTED( ch, AFF_CHARM ) )
      return;

  if(IS_NPC(ch) || ch->race!=RACE_CENTAUR || victim==NULL)
    return;

  dam=number_range(1, 6 + ch->level / 5 ) + ch->level * 3 / 4;

  if( check_hit( ch, victim, 0-GET_HITROLL(ch), 0 ) )
    damage( ch, victim, GET_DAMROLL(ch)+dam, TYPE_HOOVES);
  else
    damage( ch, victim, 0, TYPE_HOOVES );

  return;
}

  /* Orish Brawling  - Chaos 8/20/98  */
void orc_brawl( CHAR_DATA *ch, CHAR_DATA *victim)
{
	int level;
        int dmgL, dmgH;
        int tshares, pshare;

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
		return;

    if( IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	if(IS_NPC(ch) || ch->race!=RACE_ORC || victim==NULL)
		return;

    level=ch->level;
    
            /* Find total shares */
    if( level < 30 )
      tshares = 8;
    else if( level < 50 )
      tshares = 15;
    else if( level < 65 )
      tshares = 21;
    else if( level < 80 )
      tshares = 26;
    else if( level < 90 )
      tshares = 30;
    else
      tshares = 33;

    pshare = number_range( 1, tshares );

    if( pshare <= 8 )
      martial_arts_attack = -1;
    else if( pshare <= 15 )
      martial_arts_attack = -2;
    else if( pshare <= 21 )
      martial_arts_attack = -3;
    else if( pshare <= 26 )
      martial_arts_attack = -4;
    else if( pshare <= 30 )
      martial_arts_attack = -5;
    else
      martial_arts_attack = -6;

    switch( martial_arts_attack )
      {
      case -2:
        dmgL = 3* level / 8;
        dmgH = 3 * level / 4;
        break;
      case -3:
        dmgL = 4 * level / 8;
        dmgH = level;
        break;
      case -4:
        dmgL = 5 * level / 8;
        dmgH = 5 * level / 4;
        break;
      case -5:
        dmgL = 2*level/3;
        dmgH = 7 * level / 4;
        break;
      case -6:
        dmgL = 7 * level / 8;
        dmgH = 9 * level / 4;
        break;
      case -1:
      default:
        dmgL = level / 3;
        dmgH = 2 * level / 3;
        break;
      }

     if( check_hit( ch, victim, 0-GET_HITROLL(ch), 0 ) )
	    damage( ch, victim, GET_DAMROLL(ch)+5+
		    number_range(dmgL, dmgH), gsn_martial_arts);
	else
		damage( ch, victim, 0, gsn_martial_arts );

	return;
}

/*
 * hunting, hating and fearing code                             
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;

    return TRUE;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )

{
  if( IS_NPC( ch ) && IS_NPC( victim ) )
    return FALSE ;

    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;

    return TRUE;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;

    return TRUE;
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
        STRFREE( ch->hunting->name );
        DISPOSE( ch->hunting );
        ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
        STRFREE( ch->hating->name );
        DISPOSE( ch->hating );
        ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
        STRFREE( ch->fearing->name );
        DISPOSE( ch->fearing );
        ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
      stop_hating( ch );

  if( IS_NPC( victim ) )
    return;

  if( victim->level < 25 )
    return;

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}

void found_prey( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char victname[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (victim == NULL)
    {
        bug("Found_prey: null victim", 0);
        return;
    }

    if ( victim->in_room == NULL )
    {
        bug( "Found_prey: null victim->in_room", 0 );
        return;
    }

    sprintf( victname, IS_NPC( victim ) ? victim->short_descr : victim->name );

/*  This is stupid    -  Chaos 4/18/99 */
    if ( !can_see(ch, victim) )
    {
        if ( number_percent( ) < 90 )
            return;
        switch( number_bits( 2 ) )
        {
        case 0: sprintf( buf, "Don't make me find you, %s!", victname );
                do_say( ch, buf );
                break;
        case 1: act( "$n sniffs around the room for $N.", ch, NULL, victim, TO_NOTVICT );
                act( "You sniff around the room for $N.", ch, NULL, victim, TO_CHAR );
                act( "$n sniffs around the room for you.", ch, NULL, victim, TO_VICT );
                sprintf( buf, "I can smell your blood!" );
                do_say( ch, buf );
                break;
        case 2: sprintf( buf, "I'm going to tear %s apart!", victname );
                do_shout( ch, buf );
                break;
        case 3: do_say( ch, "Just wait until I find you...");
                break;
        }
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        if ( number_percent( ) < 90 )
            return;
        switch( number_bits( 2 ) )
        {
        case 0: do_say( ch, "C'mon out, you coward!" );
                sprintf( buf, "%s is a bloody coward!", victname );
                do_shout( ch, buf );
                break;
        case 1: sprintf( buf, "Let's take this outside, %s", victname );
                do_say( ch, buf );
                break;
        case 2: sprintf( buf, "%s is a yellow-bellied wimp!", victname );
                do_shout( ch, buf );
                break;
        case 3: act( "$n takes a few swipes at $N.", ch, NULL, victim, TO_NOTVICT );
                act( "You try to take a few swipes $N.", ch, NULL, victim, TO_CHAR );
                act( "$n takes a few swipes at you.", ch, NULL, victim, TO_VICT );
                break;
        }
        return;
    }

    switch( number_bits( 2 ) )
    {
     case 0: sprintf( buf, "Your blood is mine, %s!", victname );
             do_shout( ch, buf);
             break;
     case 1: sprintf( buf, "Alas, we meet again, %s!", victname );
             do_say( ch, buf );
             break;
     case 2: sprintf( buf, "What do you want on your tombstone, %s?", victname );
             do_say( ch, buf );
             break;
     case 3: act( "$n lunges at $N from out of nowhere!", ch, NULL, victim, TO_NOTVICT );
             act( "You lunge at $N catching $M off guard!", ch, NULL, victim, TO_CHAR );
             act( "$n lunges at you from out of nowhere!", ch, NULL, victim, TO_VICT );
    }  

    stop_hunting( ch );
    set_fighting( ch, victim );
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void do_attack( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];

  get_attack_string( ch, ch, buf );
  send_to_char( ansi_translate_text( ch, buf), ch );

  return;
  }
  
void get_attack_string( CHAR_DATA *ch, CHAR_DATA *viewer, char *buf )
  {
  int cnt, leng;
  int attacks;
  char buf2[MAX_INPUT_LENGTH];

  if( ch==NULL || IS_NPC(ch) )
    {
    strcpy( buf, "You cannot list attackers.\n\r" );
    return;
    }

  leng = str_cpy_max( buf, "{130}  Total Player Killer Attacks:\n\r" ,
                MAX_STRING_LENGTH);

  attacks=0;
  for( cnt=0; cnt<MAX_PK_ATTACKS; cnt++)
    if( ch->pcdata->last_pk_attack_time[cnt]>0)
      {
      attacks++;
      if( ch->pcdata->last_pk_attack_time[cnt]+24*60*60 > current_time )
        sprintf( buf2, "{170}%s {120}attacked you today on %s\r",
            ch->pcdata->last_pk_attack_name[cnt],
	    ctime((const time_t *)&ch->pcdata->last_pk_attack_time[cnt]));
      else
        sprintf( buf2, "{070}%s {020}attacked you before today on %s\r",
            ch->pcdata->last_pk_attack_name[cnt],
	    ctime((const time_t *)&ch->pcdata->last_pk_attack_time[cnt]));
      leng = str_apd_max( buf, buf2, leng, MAX_STRING_LENGTH );
      }

  if(attacks == 0 )
    {
    if( viewer == ch )
      sprintf( buf, "{130}  You have never been attacked.\n\r" );
    else
      sprintf( buf, "{130}  %s has never been attacked.\n\r", ch->name );
    }

  return;
  }

  /* Code for total and max pvnum attacks  -  Chaos 4/20/99  */
bool check_add_attack( CHAR_DATA *ch, CHAR_DATA *victim )
  {
  bool god_war;
  if( !IS_NPC(ch) && !IS_NPC(victim) )
    {
    int oldest_attack, total_pvnum_attacks, attacks_today, cnt;

    attacks_today = 0;
    oldest_attack = 0;
    total_pvnum_attacks=0;
    for( cnt=0; cnt<MAX_PK_ATTACKS; cnt++)
      {
      if( victim->pcdata->last_pk_attack_time[cnt] == 0 )
          oldest_attack = cnt;
      else
        if( victim->pcdata->last_pk_attack_time[oldest_attack] > 
            victim->pcdata->last_pk_attack_time[cnt] )
          oldest_attack = cnt;
      if( victim->pcdata->last_pk_attack_time[cnt]+24*60*60 > current_time )
        attacks_today++;
      if( ch->pcdata->pvnum == victim->pcdata->last_pk_attack_pvnum[cnt] && 
          victim->pcdata->last_pk_attack_time[cnt]+24*60*60 > current_time )
        total_pvnum_attacks++;
      }

    if( attacks_today == MAX_PK_ATTACKS )
      return( FALSE );

    god_war = FALSE;
        if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
            which_god(ch) == GOD_DEMISE || which_god(ch) == GOD_POLICE )
        if( which_god(victim) == GOD_CHAOS || which_god(victim) == GOD_ORDER ||
            which_god(victim) == GOD_DEMISE || which_god(victim) == GOD_POLICE )
          if( which_god(victim) != which_god(ch) )
        god_war = TRUE;

    if( god_war && total_pvnum_attacks >= 10 )
      return( FALSE );
    if( !god_war && total_pvnum_attacks >= 8 )
      return( FALSE );

    victim->pcdata->last_pk_attack_time[oldest_attack]=current_time;
    victim->pcdata->last_pk_attack_pvnum[oldest_attack]=ch->pcdata->pvnum;
    victim->pcdata->last_pk_attack_name[oldest_attack]=
          str_dup(capitalize(ch->name));
        
    }

  return( TRUE );
  }
