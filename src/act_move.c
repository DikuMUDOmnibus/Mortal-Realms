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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 25, 1, 2, 2, 25, 6, 3, 3
};



/*
 * Local functions.
 */
int	count_door	args( ( CHAR_DATA *ch, char *arg ) );
int	scan_door	args( ( CHAR_DATA *ch, char *arg ) );
OBJ_DATA *find_key	args( ( CHAR_DATA *ch, int key ) );
int 	find_reverse	args( ( int x, int y, int z ) );
int 	find_random	args( ( int x, int y, int z ) );
int 	which_door	args( ( int room, int dest ) );

CHAR_DATA *user;
int starting_room, size_x, size_y, size_z;
int last_door;

      /* Interesting little ACT replacement.
         Solves problem of seeing invis players leave.  - Chaos  5/22/96  */
void show_who_can_see( CHAR_DATA *ch, char *txt )
  {
  CHAR_DATA *fch;
  char buf[MAX_INPUT_LENGTH], bufn[80];

  if( ch == NULL || ch->in_room == NULL || 
      ( !IS_NPC(ch) && IS_SET( ch->act, PLR_WIZINVIS ) ) )
    return;

  strcpy( bufn, capitalize( get_name( ch ) ) );

  for( fch=ch->in_room->first_person; fch != NULL ; fch = fch->next_in_room )
    if( !IS_NPC( fch ) && fch != ch && fch->position >= POS_RESTING &&
        fch->name != NULL && fch->in_room == ch->in_room && 
        can_hear( fch, ch ) )
     if( !is_same_group( fch, ch) || !IS_SET(fch->pcdata->spam, 2048) )
      {
      if( can_see( fch, ch ) )
        strcpy( buf, bufn );
      else
        strcpy( buf, "Someone" );
      strcat( buf, txt );
      send_to_char( buf, fch );
      }

  return;
  }


void move_char( CHAR_DATA *ch, int door )
{
    bool found;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    char buf[MAX_STRING_LENGTH];
    int cnt1,cnt2,sdoor;
    CLAN_DATA *pClan;
    bool drunk = FALSE;

    found = FALSE;

    if ( door < 0 || door > 5 )
      {
	    bug( "Do_move: bad door %d.", door );
	    return;
      }
    if (!IS_NPC (ch ) && IS_DRUNK (ch, 2 ))
	drunk = TRUE;

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->to_room      ) == NULL 
    ||   !can_use_exit(ch, in_room, door ))
      {
        if ( drunk )
          send_to_char( "You hit a wall in your drunken state.\n\r", ch );
         else
	  send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
      }
    if(IS_SET(to_room->room_flags, ROOM_BLOCK))
      /* Chaos 11/16/93 */
    {
      if(IS_AFFECTED(ch, AFF_CLEAR))
        {
        to_room->room_flags-= ROOM_BLOCK;
        send_to_char("You managed to clear out a path.\n\r", ch);
        }
      else
        {
        send_to_char( "That room is currently blocked.\n\r", ch);
        return;
        }
    }
    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
        if ( ch->level < to_room->area->low_hard_range )
        {
            switch( to_room->area->low_hard_range - ch->level )
            {
                case 1:
                  send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
                  break;
                case 2:
                  send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
                  break;
                case 3:
                  send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
                  break;
                default:
                  send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
            }
            return;
        }
        else
        if ( ch->level > to_room->area->hi_hard_range )
        {
            send_to_char( "A voice in your mind says, 'There is nothing more for
 you down that path.'", ch );
            return;
        }
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) )
      {
      if((IS_SET(pexit->exit_info, EX_HIDDEN)&&
         !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))||
         !can_use_exit(ch, in_room, door ) )
	   {
	      if ( drunk )
               send_to_char( "You hit a wall in your drunken state.\n\r", ch );
              else
	       send_to_char( "Alas, you cannot go that way.\n\r", ch );
	   }
      else
	      act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	    return;
      }

    if ( IS_AFFECTED(ch, AFF_CHARM)  
    &&   (!IS_AFFECTED(ch, AFF_HUNT) || !IS_NPC(ch))
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( ch->level<LEVEL_IMMORTAL && room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if (!IS_NPC(ch) && (ch->level<LEVEL_IMMORTAL || ch->which_god!=GOD_POLICE))
    {
      for (pClan=first_clan;pClan !=NULL; pClan=pClan->next)
      {
      if (to_room->vnum == pClan->home && 
 	(ch->pcdata->clan == NULL || ch->pcdata->clan != pClan))
       {
      
	send_to_char( "You are not in the correct clan to enter that room.\n\r", ch );
	return;
       } 
      }
    }

        
    if( to_room->sector_type == SECT_ASTRAL && !IS_NPC( ch ) &&
        !IS_AFFECTED( ch, AFF2_ASTRAL ) && ch->race != RACE_GITH)
      {
	send_to_char( "You cannot enter the astral plane.\n\r", ch );
	return;
      }

    if( to_room->sector_type == SECT_ETHEREAL && !IS_NPC( ch ) &&
        !IS_AFFECTED( ch, AFF2_ETHEREAL ) )
      {
	send_to_char( "You may not leave this world.\n\r", ch );
	return;
      }
  if( !IS_NPC( ch ) && ch->pcdata->reincarnation > 0 && 
 	!IS_IMMORTAL(ch) && ch->which_god != GOD_POLICE &&
        ch->in_room->area->low_r_vnum == ROOM_VNUM_SCHOOL )
      {
	send_to_char( "That area is too delicate to contain your mighty presence.\n\r", ch);
	return;
      }


    if ( IS_SET(to_room->room_flags,ROOM_IS_CASTLE)
            && !IS_NPC( ch ) && ch->pcdata!=NULL
            && IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES))

      {
        if ( drunk )
          send_to_char( "You hit a wall in your drunken state.\n\r", ch );
         else
	  send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
      }

    if ( IS_NPC(ch)  )
      ch->speed = 2;
    else
    {
        bool belong;
        int cla;
	int move; 
        int a0, a1, a2;
        int c0, c1, c2;

        cla=ch->class+1;
        belong = TRUE;

        a0 = to_room->room_flags & ROOM_ALLOW_0;
        a1 = to_room->room_flags & ROOM_ALLOW_1;
        a2 = to_room->room_flags & ROOM_ALLOW_2;

     if( a0+a1+a2 > 0)
        {
        c0 = cla & 1;
        c1 = cla & 2;
        c2 = cla & 4;

        if( ( a0 == 0 && c0 !=0) || (a0!=0 && c0==0) ||
            ( a1 == 0 && c1 !=0) || (a1!=0 && c1==0) ||
            ( a2 == 0 && c2 !=0) || (a2!=0 && c2==0) )
            belong = FALSE;
        }


        if(!belong)
          {
          send_to_char("You don't belong in there!\n\r",ch);
	  return;
          }

	if ( in_room->sector_type == SECT_AIR ||
	     to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && ch->race != RACE_AVIARAN)
	    {
              if (vnum_in_group(ch, MOB_VNUM_AIR_ELEMENTAL ) )
              {
                send_to_char( "The air elemental bears you aloft.\n\r", ch); 
              } 
              else
              {
		send_to_char( "You can't fly.\n\r", ch );
		return;
              }
	    }

	}

    if( in_room->sector_type == SECT_ETHEREAL && !IS_NPC( ch ) )
      {
      if( !IS_AFFECTED( ch, AFF2_ETHEREAL ) )
	    {
		send_to_char( "You movement is futile.\n\r", ch );
		return;
	    }
       }

	if ((in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM)
        &&   ((!IS_AFFECTED(ch,AFF_FLYING)) && ch->race != RACE_AVIARAN))
      if( in_room->sector_type != SECT_UNDER_WATER )
	{
	    OBJ_DATA *obj;

	    /*
	     * Look for a boat.
	     */
	    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

  if(ch->race != RACE_AVIARAN)
  {
    move = (movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
           + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)])/2 ;

    if( !IS_AFFECTED( ch, AFF_HASTE ) && ch->in_room->fall_room == 0)
      switch(ch->speed)
      {
      case 0: move=1;break;
      case 1: break;
      case 2: move=5*move/4;break;
      case 3: move=3*move/2;break;
      case 4: move=2*move;break;
      default: break;
      }

     /* Add race dependancy */
     move = (move * race_table[ ch->race].move_rate + move) /3;
     if(ch->race == RACE_CENTAUR)
       move = move / 2;
     if(ch->race == RACE_TSARIAN)
       if(to_room->sector_type == SECT_WATER_SWIM ||
          to_room->sector_type == SECT_UNDER_WATER)
         move = move * 2;
   }
   else
     move = 3;

       /* Add Weather factors */
       if( IS_OUTSIDE( ch ) )
         {
         if( ch->in_room->area->weather_info.sky == 2 )
           move = 11 * move / 10;
         if( ch->in_room->area->weather_info.sky == 3 )
           move = 13 * move / 10;
         }
       move =  move *
         (100 + abs( ch->in_room->area->weather_info.temperature - 60 )/3)/100;

        if( IS_AFFECTED(ch, AFF_CLEAR))
          {
          ch->speed=0;
          move=20;
          WAIT_STATE( ch, 15);
          }
           /* Make movement based on carry weight
                  1/2 * normal without any weight
                   2  * normal with full load */
        move = move *(2 + 3 * ch->carry_weight / can_carry_w( ch )) /2 ;

        if( ch->in_room->fall_room != 0 )
          move *= 4;

	if ( ch->move < move )
	{
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

        if( ch->in_room->fall_room == 0 )
          {
          int oldspeed = ch->speed;
          if (ch->speed < 3 && vnum_in_group(ch, MOB_VNUM_EARTH_ELEMENTAL) )
           ch->speed = 3;

          if( ch->in_room->sector_type == SECT_UNDER_WATER )
            switch(ch->speed)
            {
            case 0:  WAIT_STATE( ch, 25); break;
            case 1:  WAIT_STATE( ch, 12); break;
            case 2:  WAIT_STATE( ch,  8); break;
            case 3:  WAIT_STATE( ch,  4); break;
            case 4:  WAIT_STATE( ch,  2); break;
            default: WAIT_STATE( ch,  6); break;
            }
          else
          if( ch->in_room->sector_type == SECT_WATER_SWIM || 
             ch->in_room->sector_type == SECT_WATER_NOSWIM ) 
            switch(ch->speed)
              {
              case 0:  WAIT_STATE( ch, 18); break;
              case 1:  WAIT_STATE( ch,  9); break;
              case 2:  WAIT_STATE( ch,  6); break;
              case 3:  WAIT_STATE( ch,  3); break;
              case 4:  WAIT_STATE( ch,  2); break;
              default: WAIT_STATE( ch,  5); break;
              }
          else
            switch(ch->speed)
              {
              case 0:  WAIT_STATE( ch, 12); break;
              case 1:  WAIT_STATE( ch,  6); break;
              case 2:  WAIT_STATE( ch,  4); break;
              case 3:  WAIT_STATE( ch,  2); break;
              case 4:  WAIT_STATE( ch,  1); break;
              default: WAIT_STATE( ch,  3); break;
              }
           ch->speed=oldspeed;
          }
        else
          WAIT_STATE( ch, 15);   /* All moves are the same */
    
     if (vnum_in_group(ch, MOB_VNUM_EARTH_ELEMENTAL))
     {
      move/=2;
      send_to_char( "The earth elemental carries you.\n\r", ch);
     }
     ch->move -= move;
     
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED(ch,AFF_STEALTH)
    && ( !IS_NPC( ch ) || !IS_AFFECTED( ch, AFF_ETHEREAL ) )
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
      {
      if( !IS_AFFECTED( ch, AFF_FLYING ) && 
          ch->race != RACE_AVIARAN &&
          ch->in_room->sector_type != SECT_UNDER_WATER &&
          ch->in_room->sector_type != SECT_WATER_SWIM &&
          ch->in_room->sector_type != SECT_WATER_NOSWIM )
        {
        if( IS_NPC( ch ) && ch->leader == NULL )
	{
          if ( drunk )
           sprintf(buf, " stumbles drunkenly %s.\n\r", dir_name[door]);
	  else
           sprintf(buf, " leaves %s.\n\r", dir_name[door]);
	}
        else
          switch(ch->speed)
            {
            case 0:
	      if (drunk)
               sprintf(buf," walks unsteadily %s.\n\r", dir_name[door]);
	      else
               sprintf(buf," walks %s.\n\r", dir_name[door]);
              break;
            default:
            case 1:
		if (drunk) 
                 sprintf(buf," stumbles drunkenly %s.\n\r", dir_name[door]);
		else
                 sprintf(buf," leaves %s.\n\r", dir_name[door]);
              break;
            case 2:
		if (drunk)
                 sprintf(buf," jogs unsteadily %s.\n\r", dir_name[door]);
		else
                 sprintf(buf," jogs %s.\n\r", dir_name[door]);
              break;
            case 3:
		if (drunk)
                 sprintf(buf," runs unevenly %s.\n\r", dir_name[door]);
		else
                 sprintf(buf," runs %s.\n\r", dir_name[door]);
              break;
            case 4:
		if (drunk)
                 sprintf(buf," zips unsteadily %s.\n\r", dir_name[door]);
		else
                 sprintf(buf," zips %s.\n\r", dir_name[door]);
		
              break;
            }
        show_who_can_see( ch, buf );
        }
      else
        if( IS_AFFECTED( ch, AFF_FLYING ) || (ch->race == RACE_AVIARAN && 
           ch->in_room->sector_type != SECT_UNDER_WATER))
          {
	  if ( drunk )
           sprintf(buf," flies shakily %s.\n\r",dir_name[door]);
	  else
           sprintf(buf," flies %s.\n\r",dir_name[door]);
          show_who_can_see( ch, buf );
          }
      else
        {
        if( IS_NPC( ch ))
          {
	  if (drunk)
           sprintf(buf," swims unevenly %s.\n\r",dir_name[door]);
  	  else
           sprintf(buf," swims %s.\n\r",dir_name[door]);
          show_who_can_see( ch, buf );
          }
        else
          if( found && ch->in_room->sector_type != SECT_UNDER_WATER )
          {
          sprintf(buf," sails %s.\n\r",dir_name[door]);
          show_who_can_see( ch, buf );
          }
        else
          {
	  if (drunk)
           sprintf(buf," swims unevenly %s.\n\r",dir_name[door]);
  	  else
           sprintf(buf," swims %s.\n\r",dir_name[door]);
          show_who_can_see( ch, buf );
          }
        }
      }
    char_from_room( ch );
    char_to_room( ch, to_room );
    if ( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED(ch,AFF_STEALTH)
    && ( !IS_NPC( ch ) || !IS_AFFECTED( ch, AFF_ETHEREAL ) )
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
      {
      switch(ch->speed)
        {
        case 0:
          sprintf(buf," arrives at a leisurely pace.\n\r");
          break;
        default:
        case 1:
	  if (drunk)
           sprintf(buf," stumbles drunkenly into the room.\n\r");
	  else
           sprintf(buf," has arrived.\n\r");
          break;
        case 2:
          sprintf(buf," arrives briskly.\n\r");
          break;
        case 3:
          sprintf(buf," arrives suddenly.\n\r");
          break;
        case 4:
          sprintf(buf," arrives quickly.\n\r");
          break;
        }
      show_who_can_see( ch, buf );
      }

    do_look( ch, "auto" );

  if( ch->in_room->fall_room != 0 )
    WAIT_STATE( ch, 15 );

    if( ch->in_room->sector_type != SECT_ASTRAL && !IS_NPC( ch ) &&
        ch->in_room->vnum > 5 )  /* Ignore Limbo */
      ch->pcdata->last_real_room = ch->in_room->vnum;
      
    if( ch->in_room->sector_type == SECT_INN )
      send_to_char( "You could sleep safely here.\n\r", ch );

    if( IS_SET( ch->in_room->room_flags , ROOM_GOD_HI ) ||
        IS_SET( ch->in_room->room_flags , ROOM_GOD_LO ) )
      switch( which_god( ch ))
        {
        case GOD_INIT_ORDER:
        case GOD_ORDER:
          if( IS_SET( ch->in_room->room_flags , ROOM_GOD_HI ) &&
              !IS_SET( ch->in_room->room_flags , ROOM_GOD_LO ) )
            send_to_char( "You feel very safe here.\n\r", ch);
          break;
        case GOD_INIT_CHAOS:
        case GOD_CHAOS:
          if( !IS_SET( ch->in_room->room_flags , ROOM_GOD_HI ) &&
              IS_SET( ch->in_room->room_flags , ROOM_GOD_LO ) )
            send_to_char( "You feel very safe here.\n\r", ch);
          break;
        default:
          if( IS_SET( ch->in_room->room_flags , ROOM_GOD_HI ) &&
              IS_SET( ch->in_room->room_flags , ROOM_GOD_LO ) )
            send_to_char( "You feel very safe here.\n\r", ch);
          break;
       }
    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  in_room->area != to_room->area )
    {
        if ( ch->level < to_room->area->low_soft_range )
           send_to_char("You feel uncomfortable being in this strange land...\n\r", ch);
        else
        if ( ch->level > to_room->area->hi_soft_range )
           send_to_char("You feel there is not much to gain visiting this place...\n\r", ch);
    }

    if ( !IS_NPC(ch) 
        && IS_SET(ch->in_room->room_flags,ROOM_IS_CASTLE) 
        && ch->in_room->creator_pvnum == ch->pcdata->pvnum)
        {
 	  OWNER_DATA *pOwner;
	  bool fMatch=FALSE;

 	  for (pOwner=first_owner; pOwner != NULL; pOwner = pOwner->next)
	  {
   	   if (pOwner->pvnum == ch->pcdata->pvnum)
	   {
	     pOwner->lastentry = (int) current_time;
	     fMatch=TRUE;
 	     break;
	   }
          }
	  if (!fMatch)
  	  {
 	    CREATE( pOwner, OWNER_DATA, 1);
            pOwner->pvnum     = ch->pcdata->pvnum;
            pOwner->lastentry = (int) current_time;
	    pOwner->name      = STRALLOC(ch->name);
	    LINK (pOwner, first_owner, last_owner, next, prev);
          }

	  log_printf("%s entered their castle", ch->name);
	  castle_needs_saving=TRUE;
         }
  
    for ( fch = in_room->first_person; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;
	if ( fch->master == ch && fch->position == POS_STANDING &&
             fch->in_room != ch->in_room )
	{
            if( ch->speed <= get_max_speed( fch ) )
              fch->speed=ch->speed;
            else
              {
              act( "$N can't move that fast, so you slow down.", 
                   ch, NULL, fch, TO_CHAR);
              ch->speed = get_max_speed( fch );
              fch->speed = ch->speed;
              }
	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door );
#ifdef undef
            if (IS_NPC(fch) && IS_SET(fch->act, ACT_ELEMENTAL) ) 
            {   
             switch (fch->pIndexData->vnum)
             {
              case MOB_VNUM_FIRE_ELEMENTAL: 
               if (to_room->sector_type != SECT_LAVA )
               {
	        send_to_char( "The fire elemental melts into the magma.\n\r", ch);
                SET_BIT(fch->act, ACT_WILL_DIE);
               }
 	       break;
              case MOB_VNUM_WATER_ELEMENTAL: 
               if (to_room->sector_type != SECT_WATER_SWIM 
                  && to_room->sector_type != SECT_WATER_NOSWIM 
                  && to_room->sector_type != SECT_UNDER_WATER )
               {
	        send_to_char( "The water elemental flows away in a strong undercurrent.\n\r", ch);
                SET_BIT(fch->act, ACT_WILL_DIE);
               }
 	       break;
              case MOB_VNUM_AIR_ELEMENTAL: 
               if (to_room->sector_type != SECT_AIR )
               {
	        send_to_char( "The air elemental disappears in a strong gust of wind.\n\r", ch);
                SET_BIT(fch->act, ACT_WILL_DIE);
               }
 	       break;
              case MOB_VNUM_EARTH_ELEMENTAL: 
               if (to_room->sector_type != SECT_FOREST 
                  && to_room->sector_type != SECT_DESERT 
                  && to_room->sector_type != SECT_MOUNTAIN
                  && to_room->sector_type != SECT_HILLS 
                  && to_room->sector_type != SECT_UNDER_GROUND 
                  && to_room->sector_type != SECT_DEEP_EARTH )
               {
	        send_to_char( "The earth elemental allows its form to dissolve into the ground.\n\r", ch);
                SET_BIT(fch->act, ACT_WILL_DIE);
               }
 	       break;
             }
            }
#endif
	}
    }

/*  Chaos - code for last_left - track skill   11/14/93  */
/*  only leave tracks if not IMP or flying */
    if( !IS_IMMORTAL(ch) &&
 	!is_affected(ch, gsn_pass_without_trace) &&
        in_room->sector_type != SECT_WATER_SWIM &&
        in_room->sector_type != SECT_WATER_NOSWIM &&
        in_room->sector_type != SECT_LAVA &&
        ch->race != RACE_AVIARAN)
      {
      cnt2=-1;
      for(cnt1=0;cnt1<MAX_LAST_LEFT;cnt1++)
        if(in_room->last_left[cnt1][0]=='\0')
          cnt2=cnt1;
      if(cnt2==-1)
        {
        for(cnt1=0;cnt1<MAX_LAST_LEFT-1;cnt1++)
          {
          strcpy(in_room->last_left[cnt1],in_room->last_left[cnt1+1]);
          in_room->last_left_bits[cnt1]=
            in_room->last_left_bits[cnt1+1];
          }
        cnt2=MAX_LAST_LEFT-1;
        }
      if(ch->short_descr==NULL || ch->short_descr[0]=='\0')
        strcpy(buf, ch->name);
      else
        strcpy(buf, ch->short_descr);
      buf[20]='\0';
      strcpy(in_room->last_left[cnt2],buf);
      switch(door)
       {
 	case DIR_NORTH: in_room->last_left_bits[cnt2]= TRACK_NORTH;
			break;
 	case DIR_SOUTH: in_room->last_left_bits[cnt2]= TRACK_SOUTH;
			break;
 	case DIR_EAST:  in_room->last_left_bits[cnt2]= TRACK_EAST;
			break;
 	case DIR_WEST:  in_room->last_left_bits[cnt2]= TRACK_WEST;
			break;
 	case DIR_UP:    in_room->last_left_bits[cnt2]= TRACK_UP;
			break;
 	case DIR_DOWN:  in_room->last_left_bits[cnt2]= TRACK_DOWN;
			break;
       }
       if(IS_AFFECTED(ch, AFF_FLYING) || ch->race==RACE_AVIARAN)
        SET_BIT(in_room->last_left_bits[cnt2], TRACK_FLY);
       if((float)ch->hit/(float)ch->max_hit <0.10
          && (ch->class ==CLASS_ROGUE || ch->class==CLASS_ASSASSIN))
        SET_BIT(in_room->last_left_bits[cnt2], TRACK_BLOOD);
       else if((float)ch->hit/(float)ch->max_hit <0.2)
        SET_BIT(in_room->last_left_bits[cnt2], TRACK_BLOOD);
      }


    /* make shadowers shadow player -Dug */
    if((fch=ch->shadowed_by)!=NULL)
      {
      if(fch->position==POS_STANDING && (fch->in_room!=ch->in_room))
        {
        bool in_range;

        for(sdoor=0,in_range=FALSE;sdoor<6;sdoor++)
          if((pexit=fch->in_room->exit[sdoor])!=NULL&&(pexit->to_room)==in_room)
            {
            CHAR_DATA *tch;
  
            if( ch->speed <= get_max_speed( fch ) )
              {
              fch->speed=ch->speed;
	      act( "$n leaves $t.", ch, dir_name[door], fch, TO_VICT );
	      act( "You follow at a discrete distance.",fch,NULL,NULL,TO_CHAR);
              /* clear shadowing so doesn't get reset by move -Dug */
              tch=fch->shadowing;
              fch->shadowing=NULL;
	      move_char( fch, sdoor );
              /* restore shadowing */
              fch->shadowing=tch;
              in_range=TRUE;
              break;
              }
            else
              {
	      act( "$n leaves $t.", ch, dir_name[door], fch, TO_VICT );
              send_to_char( "They are moving too fast for you.\n\r", fch);
              in_range=FALSE;
              }
            }
        /* turn off shadowing if shadower not in range -Dug */
        if(!in_range)
          stop_shadow(ch);
        }
      else
        stop_shadow(ch);
      }

    /* turn off shadowing if shadower moves -Dug */
    if(ch->shadowing!=NULL)
      stop_shadow(ch);
    /* this must be last because the character might die as a result of it
       Also, we won't jump them right away if they aren't in a clan at all,
       only if they're possessed, a homonculous or a different clan 
       Martin 13/12/98*/
   if ((IS_NPC(ch) && 
	(IS_AFFECTED(ch, AFF2_POSSESS) || ch->pIndexData->vnum==9900)) 
       || (!IS_NPC(ch) && ch->pcdata->clan != NULL) )
   {
    for ( fch = ch->in_room->first_person; fch != NULL; fch = fch_next )
    {
     fch_next = fch->next_in_room;
     if (fch
	&& IS_NPC(fch) 
	&& IS_SET(fch->act, ACT_CLAN_GUARD )
 	&& (!IS_NPC(ch) && ch->pcdata->clan != get_clan_from_vnum(fch->pIndexData->creator_pvnum) ))
       {
	fprintf(stderr, "fch(%s) attacking ch(%s)\n", fch->name, ch->name);
        multi_hit( fch, ch, TYPE_UNDEFINED );
        break;
       }
     } 
   }
     mprog_entry_trigger(ch);
     mprog_greet_trigger(ch);
    return;
}


bool can_move_char( CHAR_DATA *ch, int door )
{
    bool found;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    found = FALSE;

    if ( door < 0 || door > 5 )
      {
	    bug( "Do_move: bad door %d.", door );
	    return(FALSE);
      }

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->to_room      ) == NULL 
    ||   ( !can_use_exit(ch, in_room, door ) ) )
      {
	    return(FALSE);
      }

    if(IS_SET(to_room->room_flags, ROOM_BLOCK))
        {
        return(FALSE);
        }

    if(to_room->area->low_hard_range!=0 ||
       to_room->area->hi_hard_range!=0)
      if( !IS_IMMORTAL(ch) && !IS_NPC(ch))
        if(ch->level<to_room->area->low_hard_range ||
           ch->level>to_room->area->hi_hard_range)
          {
          return(FALSE);
          }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) )
      {
	    return(FALSE);
      }

    if ( IS_AFFECTED(ch, AFF_CHARM)  
    &&   (!IS_AFFECTED(ch, AFF_HUNT) || !IS_NPC(ch))
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	return(FALSE);
    }

    if ( ch->level<LEVEL_IMMORTAL && room_is_private( to_room ) )
    {
	return(FALSE);
    }

    if( to_room->sector_type == SECT_ASTRAL && !IS_NPC( ch ) &&
        !IS_AFFECTED( ch, AFF2_ASTRAL ) )
      {
	return(FALSE);
      }

    if( to_room->sector_type == SECT_ETHEREAL && !IS_NPC( ch ) &&
        !IS_AFFECTED( ch, AFF2_ETHEREAL ) )
      {
	return(FALSE);
      }


    if ( IS_NPC(ch)  )
      ch->speed = 2;
    else
    {
        bool belong;
        int cla;
	int move; 
        int a0, a1, a2;
        int c0, c1, c2;

        cla=ch->class+1;
        belong = TRUE;

        a0 = to_room->room_flags & ROOM_ALLOW_0;
        a1 = to_room->room_flags & ROOM_ALLOW_1;
        a2 = to_room->room_flags & ROOM_ALLOW_2;

     if( a0+a1+a2 > 0)
        {
        c0 = cla & 1;
        c1 = cla & 2;
        c2 = cla & 4;

        if( ( a0 == 0 && c0 !=0) || (a0!=0 && c0==0) ||
            ( a1 == 0 && c1 !=0) || (a1!=0 && c1==0) ||
            ( a2 == 0 && c2 !=0) || (a2!=0 && c2==0) )
            belong = FALSE;
        }


        if(!belong)
          {
	  return(FALSE);
          }

        if(ch->race != RACE_AVIARAN)
	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) )
	    {
		return(FALSE);
	    }
	}

    if( in_room->sector_type == SECT_ETHEREAL && !IS_NPC( ch ) )
      {
      if( !IS_AFFECTED( ch, AFF2_ETHEREAL ) )
	    {
		return(FALSE);
	    }
       }

       if(ch->race != RACE_AVIARAN)
       {
	if ((in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM)
  &&   !(IS_AFFECTED(ch,AFF_FLYING)))
	{
	    OBJ_DATA *obj;

	    /*
	     * Look for a boat.
	     */
	    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		return(FALSE);
	    }
	}

	move = (movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)])/2 ;

    if( !IS_AFFECTED( ch, AFF_HASTE ))
        switch(ch->speed)
        {
          case 0: move=1;break;
          case 1: break;
          case 2: move=5*move/4;break;
          case 3: move=3*move/2;break;
          case 4: move=2*move;break;
          default: break;
        }

        /* Add race dependancy */
          move = (move * race_table[ ch->race].move_rate + move) /3;
          if(ch->race == RACE_CENTAUR)
            move = move/2;
     if(ch->race == RACE_TSARIAN)
       if(to_room->sector_type == SECT_WATER_SWIM ||
          to_room->sector_type == SECT_WATER_NOSWIM ||
          to_room->sector_type == SECT_UNDER_WATER)
         move = move * 2;
       }
       else
         move = 3;

       /* Add Weather factors */
       if( IS_OUTSIDE( ch ) )
         {
         if( ch->in_room->area->weather_info.sky == 2 )
           move = 11 * move / 10;
         if( ch->in_room->area->weather_info.sky == 3 )
           move = 13 * move / 10;
         }
       move =  move *
         (100 + abs( ch->in_room->area->weather_info.temperature - 60 )/3)/100;

        if( IS_AFFECTED(ch, AFF_CLEAR))
          {
          ch->speed=0;
          move=20;
          WAIT_STATE( ch, 15);
          }
           /* Make movement based on carry weight
                  1/2 * normal without any weight
                   2  * normal with full load */
        move = move *(2 + 3 * ch->carry_weight / can_carry_w( ch )) /2 ;
	if ( ch->move < move )
	{
	    return(FALSE);
	}
     }

    return( TRUE );
    }


void do_speed( CHAR_DATA *ch, char *argument )
{
  if(argument[0]=='\0')
    {
    send_to_char( "Usage:  speed <walk,normal,jog,run,haste>\n\r" ,ch);
    send_to_char( "This allows changing of rate of movement.  Rates effect the amount of MV lost.\n\rwalk   - 1 mv lost per move\n\rnormal - normal mv lost\n\rjog    - 1.5 times normal mv lost\n\rrun    - 2 times mv lost\n\r", ch);
    switch(ch->speed)
      {
      case 0: send_to_char("Currently: walking\n\r", ch); break;
      case 1: send_to_char("Currently: normal\n\r", ch); break;
      case 2: send_to_char("Currently: jogging\n\r", ch); break;
      case 3: send_to_char("Currently: running\n\r", ch); break;
      case 4: send_to_char("Currently: haste\n\r", ch); break;
      }
    return;
    }


if(argument[0]=='w')
   ch->speed=0;
else
if(argument[0]=='n')
  ch->speed=1;
else
if(argument[0]=='j')
  ch->speed=2;
else
if(argument[0]=='r')
  ch->speed=3;      
else
if(argument[0]!='\0')
  ch->speed=4;      

if( ch->speed > get_max_speed( ch ) )
  {
  ch->speed = get_max_speed( ch );
  send_to_char( "You cannot move that fast.\n\r", ch);
  }
if (is_affected(ch, gsn_slow) && ch->speed > 0 )
  {
  ch->speed = 0;
  send_to_char( "Your limbs feel like lead! You are unable to shake the lethargy from your body.\n\r", ch);
  }
  
  switch( ch->speed )
    {
    case 0: send_to_char( "Speed set to Walk.\n\r", ch); break;
    case 1: send_to_char( "Speed set to Normal.\n\r", ch); break;
    case 2: send_to_char( "Speed set to Jog.\n\r", ch); break;
    case 3: send_to_char( "Speed set to Run.\n\r", ch); break;
    case 4: send_to_char( "Speed set to Haste.\n\r", ch); break;
    }
  return;
}

void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN );
    return;
}


int scan_door( CHAR_DATA *ch, char *arg )
{
  int count, door;
  EXIT_DATA *pexit;

  count=0;
	  for ( door = 0; door <= 5; door++ )
	   if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
 	    &&   !IS_SET(pexit->exit_info, EX_UNBARRED)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) 
	    &&   can_use_exit(ch, ch->in_room, door) )
        return(door);
  return( -1 );
}

int count_door( CHAR_DATA *ch, char *arg )
{
  int count, door;
  EXIT_DATA *pexit;
  char buf[160];

  strcpy(buf, arg);
  door=strlen( buf);
  for( count=0; count<door ; count++)
    if( buf[count]==' ')
      buf[count]='\0';

  count=0;
	  for ( door = 0; door <= 5; door++ )
	   if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( buf, pexit->keyword ) 
	    &&   can_use_exit(ch, ch->in_room, door) )
        count++;
  return( count );
}

int find_door( CHAR_DATA *ch, char *arg )
{
  EXIT_DATA *pexit;
  int door, count=0;
  char buf[80];

  door = -1;
	 if ( !strcasecmp( arg, "n" ) || !strcasecmp( arg, "north" ) ) 
     { 
     door = 0;
     strcpy( buf, "north");
     }
   else if ( !strcasecmp( arg, "e" ) || !strcasecmp( arg, "east"  ) ) 
     { 
     door = 1;
     strcpy( buf, "east");
     }
   else if ( !strcasecmp( arg, "s" ) || !strcasecmp( arg, "south" ) ) 
     { 
     door = 2;
     strcpy( buf, "south");
     }
   else if ( !strcasecmp( arg, "w" ) || !strcasecmp( arg, "west"  ) ) 
     { 
     door = 3;
     strcpy( buf, "west");
     }
   else if ( !strcasecmp( arg, "u" ) || !strcasecmp( arg, "up"    ) ) 
     { 
     door = 4;
     strcpy( buf, "up");
     }
   else if ( !strcasecmp( arg, "d" ) || !strcasecmp( arg, "down"  ) )
     { 
     door = 5;
     strcpy( buf, "down");
     }

if( door == -1 )
  {
  count=count_door( ch, arg);
    if(count > 1)
      {
	    act( "Use a direction here.  Too many similar exits.", 
         ch, NULL, NULL, TO_CHAR );
	    return -1;
      }
    if(count == 0)
      {
/* This line removed because someone made doors check before objs -Order
	    act( "You see no $T here.", ch, NULL, arg, TO_CHAR ); */
	    return -1;
      }
   door= scan_door( ch, arg);
   return door;
   }

   if ( ( pexit = ch->in_room->exit[door] ) == NULL ||
       IS_SET( ch->in_room->exit[door]->exit_info, EX_UNBARRED)  ||
       !IS_SET( ch->in_room->exit[door]->exit_info, EX_ISDOOR)  ||
	!can_use_exit(ch, ch->in_room, door ) ) 
      {
      char buf2[80];
	    sprintf( buf2, "You see no door %s from here.\n\r", buf );
      send_to_char( buf2, ch);
	    return -1;
      }
   if (pexit->keyword !=NULL)
    count=count_door( ch, pexit->keyword );

    if( count>1) 
      return door;

	  act( "What are you looking for?", ch, NULL, NULL, TO_CHAR );
    
	  return -1;
   
}


void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* open the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->first_person; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
   return;
    }


    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }
    else
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }


    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_BASHED) )
	    { send_to_char( "It's been bashed off its hinges!\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* close the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->first_person; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }
    else
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}



OBJ_DATA *find_key( CHAR_DATA *ch, int key )
  {
  OBJ_DATA *obj,*obj2;

  for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
    if((obj->pIndexData->vnum==key)||
       ((obj->item_type==ITEM_KEY)&&(key==obj->value[0])))
      return(obj);
    if(obj->item_type==ITEM_CONTAINER)
      if((obj->first_content!=NULL)&&!IS_SET(obj->value[1],CONT_CLOSED))
        for(obj2=obj->first_content;obj2!=NULL;obj2=obj2->next_content)
          {
          if((obj2->pIndexData->vnum==key)||
             ((obj2->item_type==ITEM_KEY)&&(key==obj2->value[0])))
            return(obj);
          }
    }

  return(NULL);
  }



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }


    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( find_key( ch, pexit->key )==NULL )
          {
          if(IS_NPC(ch)||(ch->pcdata->learned[gsn_lock]==0))
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
          else if(number_percent()>ch->pcdata->learned[gsn_lock])
	    { send_to_char( "You failed.\n\r",             ch ); return; }
          }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( find_key( ch, obj->value[2] )==NULL )
          {
          if(IS_NPC(ch)||(ch->pcdata->learned[gsn_lock]==0))
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
          else if(number_percent()>ch->pcdata->learned[gsn_lock])
	    { send_to_char( "You failed.\n\r",             ch ); return; }
          }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }
    else
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *key;
    int door;

    obj=NULL;
    key=NULL;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( (key=find_key( ch, pexit->key))==NULL )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d with $p.", ch, key, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
  if( number_range( 1,3) ==1 && (key->item_type != ITEM_CONTAINER && key->first_content == NULL ) && !IS_SET(ch->in_room->room_flags,ROOM_IS_CASTLE)&&
                !IS_SET(to_room->room_flags,ROOM_IS_CASTLE))
    {
    act( pick_one( "You eat $p.",
    "You drop $p into the hole, and it's gone.",
    "$p breaks in $d.",
    "$p disappears from your grasp.") , ch, key, pexit->keyword, TO_CHAR);
    obj_from_char( key);
    extract_obj( key );
    }
    return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( (key=find_key( ch, obj->value[2] ))==NULL )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p with $P.", ch, obj, key, TO_ROOM );
  if( key!=obj && number_range( 1,3) ==1 && key->item_type != ITEM_CONTAINER && key->first_content == NULL )
    {
    act( pick_one( "You eat $p.",
    "You drop $p into the keyhole, and it's gone.",
    "$p breaks in $P.",
    "$p disappears from your grasp.") , ch, key, obj, TO_CHAR);
    obj_from_char( key);
    extract_obj( key );
    }

	return;
    }
    else
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

  if(multi(ch, gsn_pick_lock)==-1 && !IS_NPC(ch))
    return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->mclass[2] + 5 < gch->level &&
     !IS_AFFECTED( gch, AFF_CHARM) && !IS_AFFECTED(gch, AFF_ETHEREAL))
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_pick_lock] )
    {
	send_to_char( "You failed.\n\r", ch);
	return;
    }


    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
    {
    if(!IS_NPC(ch)&&(ch->pcdata->learned[gsn_greater_pick]>0))
      {
      if((number_percent()>ch->pcdata->learned[gsn_greater_pick]))
	      {
        damage( ch, ch, UMIN(100,ch->max_hit/2), gsn_zap);
        act("You fail and the $d zaps you!",ch,NULL,pexit->keyword,TO_CHAR);
        act("$n glows momentarily while failing to pick $d!",
	          ch, NULL, pexit->keyword, TO_ROOM); 
        return; 
        }
      }
    else
      {
      act("You fail!",ch,NULL,NULL,TO_CHAR);
      return;
      }
    }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* pick the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
   return;
    }
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if(IS_SET(obj->value[1], CONT_PICKPROOF))
    {
    if(!IS_NPC(ch)&&(ch->pcdata->learned[gsn_greater_pick]>0))
      {
      if((number_percent()>ch->pcdata->learned[gsn_greater_pick]))
	      {
        damage( ch, ch, UMIN(100,ch->max_hit/2), gsn_zap);
        act("You fail and the $p zaps you!",ch,obj,NULL,TO_CHAR);
        act("$n glows momentarily while failing to pick $p!",
	          ch, obj, NULL, TO_ROOM); 
        return; 
        }
      }
    else
      {
      act("You fail!",ch,obj,NULL,TO_CHAR);
      return;
      }
    }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }
    else
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
  {
  if(IS_AFFECTED(ch,AFF2_CAMPING))
    {
    send_to_char( "You break camp.\n\r", ch );
    act( "$n breaks camp.", ch, NULL, NULL, TO_ROOM );
    REMOVE_BIT(ch->affected2_by,0-AFF2_CAMPING);
    }
  switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
        if( ch->fighting != NULL )
	  ch->position = POS_FIGHTING;
        else
	  ch->position = POS_STANDING;
	break;

    case POS_RESTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
        if( ch->fighting != NULL )
	  ch->position = POS_FIGHTING;
        else
	  ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

  return;
  }



void do_rest( CHAR_DATA *ch, char *argument )
  {
  switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
        if(in_camp(ch))
          {
          send_to_char("You camp and begin recuperating.\n\r", ch);
          act("$n joins the camp.\n\r",ch,NULL,NULL,TO_ROOM);
          }
	break;

    case POS_FIGHTING:
	send_to_char( "You are fighting!\n\r", ch );
	break;
    }
  return;
  }



void do_sleep( CHAR_DATA *ch, char *argument )
{
  switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;
    case POS_RESTING:
	send_to_char( "You sleep.\n\r", ch );
	act( "$n sleeps.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;
    case POS_STANDING: 
	send_to_char( "You sleep.\n\r", ch );
	act( "$n sleeps.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
        if(in_camp(ch))
          {
          send_to_char("You camp and begin recuperating.\n\r", ch);
          act("$n joins the camp.\n\r",ch,NULL,NULL,TO_ROOM);
          }
	break;
    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );
    affect_strip( ch, gsn_pass_without_trace );

  if(multi(ch, gsn_sneak)==-1 && !IS_NPC(ch) && ch->race != RACE_TSARIAN)
    return;


    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_sneak] ||
         ch->race == RACE_TSARIAN)
      {
      if(IS_NPC(ch)||number_percent()<ch->pcdata->learned[gsn_greater_sneak] ||
         ch->race == RACE_TSARIAN)
        send_to_char("You start moving silently.\n\r",ch);
      af.type      = gsn_sneak;
      af.duration  = ch->level;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_SNEAK;
      affect_to_char( ch, &af );
     if(!IS_NPC(ch) 
        && number_percent()<ch->pcdata->learned[gsn_pass_without_trace])
     {
      send_to_char("The wind would leave more trace of it's passage than you.\n\r",ch);
      af.type      = gsn_pass_without_trace;
      af.bitvector = 0;
      affect_to_char( ch, &af );
     }
      }

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

  if(multi(ch, gsn_hide)==-1 && !IS_NPC(ch))
    return;


    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_hide] )
        {
        if(IS_NPC(ch)||number_percent()<ch->pcdata->learned[gsn_greater_hide])
          send_to_char("You skillfully hide yourself.\n\r",ch);
	SET_BIT(ch->affected_by, AFF_HIDE);
        }

    return;
}



/*
 * Contributed by Alander.
 */

    /* The STOP command.  Added selectives.  - Chaos 6/22/97  */
void do_visible( CHAR_DATA *ch, char *argument )
{
  bool all;
  bool pick, tx;

  pick = FALSE;
  tx = FALSE;

  if( !strcasecmp(argument, "all" ) )
    {
    all = TRUE;
    }
  else
    all = FALSE;

    if( !strcasecmp(argument, "invis") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_INVISIBLE) )
        {
        send_to_char( "You become visible.\n\r", ch );
        tx=TRUE;
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are already visible.\n\r", ch );
        }
      affect_strip ( ch, gsn_invis			);
      affect_strip ( ch, gsn_improved_invis		);
      affect_strip ( ch, gsn_mass_invis			);
      REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
      }

    if( !strcasecmp(argument, "sneak") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_SNEAK))
        {
        tx=TRUE;
        send_to_char( "You stop sneaking.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not sneaking.\n\r", ch );
        }
      affect_strip ( ch, gsn_sneak			);
      affect_strip ( ch, gsn_pass_without_trace		);
      REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
      }

    if( !strcasecmp(argument, "greater") || all )
      {
      pick = TRUE;
      if( is_affected(ch, gsn_greater_stealth))
        {
        tx=TRUE;
        send_to_char( "You reveal yourself to mortal eyes.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not greater stealthed.\n\r", ch );
        }
      affect_strip ( ch, gsn_greater_stealth			);
      }

    if( !strcasecmp(argument, "flash") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF2_HAS_FLASH))
        {
        tx=TRUE;
        send_to_char( "You throw away the flash powder.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You don't have any flash powder.\n\r", ch );
        }
       REMOVE_BIT(ch->affected2_by, 0-AFF2_HAS_FLASH );
      }

    if( !strcasecmp(argument, "anti") || all )
      {
      pick = TRUE;
      if( is_affected(ch, gsn_anti_magic_shell))
        {
        tx=TRUE;
        send_to_char( "You dispell the anti-magic shell.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not encased in an anti-magic shell.\n\r", ch );
        }
       affect_strip(ch, gsn_anti_magic_shell);
      }

    if( !strcasecmp(argument, "stealth") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_STEALTH))
        {
        tx=TRUE;
        send_to_char( "You stop being stealthy.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not stealthy.\n\r", ch );
        }
      affect_strip ( ch, gsn_stealth			);
      affect_strip ( ch, gsn_greater_stealth			);
      REMOVE_BIT   ( ch->affected_by, AFF_STEALTH		);
      }

    if( !strcasecmp(argument, "clear") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_CLEAR))
        {
        tx=TRUE;
        send_to_char( "You stop clearing a path.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not clearing a path.\n\r", ch );
        }
      affect_strip ( ch, gsn_clear_path                   );
      REMOVE_BIT   ( ch->affected_by, AFF_CLEAR           );
      }

    if( !strcasecmp(argument, "hunt") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_HUNT))
        {
        tx=TRUE;
        send_to_char( "You stop hunting.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not hunting.\n\r", ch );
        }
      affect_strip ( ch, gsn_hunt                         );
      REMOVE_BIT   ( ch->affected_by, AFF_HUNT            );
      }

    if( !strcasecmp(argument, "hide") || all )
      {
      pick = TRUE;
      if( IS_AFFECTED(ch, AFF_HIDE))
        {
        tx=TRUE;
        send_to_char( "You stop hiding.\n\r", ch );
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not hiding.\n\r", ch );
        }
      REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
      }

    if(IS_NPC(ch)&&ch->long_descr!=ch->pIndexData->long_descr)
      {
      tx=TRUE;
      STRFREE (ch->long_descr );
      ch->long_descr = ch->pIndexData->long_descr;
      }

    if( !strcasecmp(argument, "disguise") || all )
      {
      pick = TRUE;
      if( !IS_NPC( ch ) && ch->long_descr!=NULL && ch->long_descr[0]!='\0')
        {
        STRFREE (ch->long_descr );
        ch->long_descr = STRALLOC("");
        send_to_char( "You remove your disguise.\n\r", ch );
        tx=TRUE;
        }
      else if( !all )
        {
        tx=TRUE;
        send_to_char( "You are not disguised.\n\r", ch );
        }
      }

  if( pick && !IS_NPC(ch) && !tx)
      send_to_char( "You are not doing anything.\n\r", ch);

  if( !pick && !IS_NPC( ch ))
    send_to_char( "Stop doing what?\n\rSTOP [ all, invis, sneak, stealth, hunt, hide, clear, disguise, flash,\n\rgreater, spy, anti]\n\r", ch );

    return;
}

void do_recall( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *location;
  CHAR_DATA *fch;
  CHAR_DATA *victim;
  OBJ_DATA *scroll;
  bool found;
  bool pkill;

  if( ch->in_room->sector_type == SECT_ETHEREAL )
    {
      send_to_char( "You don't know the path back to the real universe.\n\r", ch);
      return;
    }
  
  if( ch->in_room->sector_type == SECT_ASTRAL )
    {
      send_to_char( "You are too far out of your mind to recall anything.\n\r", ch);
      return;
    }
  
  
  found=FALSE;
  if(argument==NULL)
    buf[0]='\0';
  else
    one_argument( argument, buf);
  /* Chaos 10/8/93 */
  location = get_room_index( ch->recall );
  if(!strcasecmp(buf,"set"))
    {
      if(IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
	{
	  send_to_char( "You cannot do that in this room.\n\r", ch);
	  return;
	}
      else if(  ch->in_room->vnum<100)
	{
	  send_to_char( "You cannot do that in this area.\n\r", ch);
	  return;
	}
      else if(  IS_SET ( ch->in_room->area->flags, AFLAG_NORECALL) )
	{
	  send_to_char( "You cannot do that in this area.\n\r", ch);
	  return;
	}
      else
	{
	  ch->recall = ch->in_room->vnum;
	  send_to_char( "Your recall room has been set.\n\r", ch);
	  return;
	} 
    }
  if(!strcasecmp(buf,"reset") )
    {
      if( IS_NPC(ch) || ch->pcdata == NULL )
        return;
      ch->recall = ch->pcdata->death_room;
      send_to_char( "Your recall room has been reset.\n\r", ch);
      return;
    }
  
  act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );
  WAIT_STATE( ch, 5);
  
  if ( ch->recall == 0 )
    {
      send_to_char( "You are completely lost.\n\r", ch );
      return;
    }
  
  if(argument!=NULL)
    {
      if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
	  send_to_char( "You cannot do that here.\n\r", ch);
	  return;
	}
      if( !IS_NPC( ch))
	{
	  for(scroll=ch->first_carrying; scroll!=NULL; scroll=scroll->next_content)
	    if(  scroll->item_type == ITEM_SCROLL )
	      if(scroll->value[1]== slot_lookup( 42 ) ||
		 scroll->value[2]== slot_lookup( 42 ) ||
		 scroll->value[3]== slot_lookup( 42 ) )
		{
		  found=TRUE;
		  break;
		}
	  if ( !found )
	    {
	      send_to_char( "You do not have that scroll.\n\r", ch );
	      return;
	    }
	  
	  extract_obj(scroll);
	}
    }
  
  
  /* make charmed creatures recall to master's recall -Dug 12/5/93 */
  if(IS_AFFECTED(ch,AFF_CHARM)&&(ch->master!=NULL))
    {
      location=get_room_index(ch->master->in_room->vnum);
      act("$N (your slave) tries to recall to your recall room!",
          ch->master,0,ch,TO_CHAR);
    }
  
  if ( ch->in_room == location )
    return;
  
  if(location==NULL)
    return;
  
  /* Modified this so if you have more than 1/2 hp and it's a pkill, then
     you can't recall -- Death 2-97 
     */ 

  if (ch->fighting != NULL && !IS_NPC(ch->fighting->who))
    pkill=TRUE;
  else 
    pkill=FALSE;

  if((IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) ||
     (IS_AFFECTED(ch, AFF_CURSE)) ||
      (IS_SET( ch->in_room->area->flags, AFLAG_NORECALL)) ||
     (number_percent() <= ch->level / 36 + 1) ||
     ((ch->hit > ch->max_hit / 2) && pkill))
    {
      switch(which_god(ch))
	{
	case GOD_ORDER:
	case GOD_INIT_ORDER:
	  send_to_char( "Order decides you need to be disciplined.\n\r", ch );
	  break;
	case GOD_CHAOS:
	case GOD_INIT_CHAOS:
	  send_to_char( "Chaos on a whim decides to ignore you.\n\r", ch );
	  break;
	case GOD_DEMISE:
	  send_to_char( "Demise beckons you to your destiny.\n\r", ch );
	  break;
	default:
	  send_to_char( "Your god has forsaken you.\n\r", ch );
	  break;
	}
      return;
    }
 
  if ( ( victim = who_fighting(ch) ) != NULL )
    {
      int lose;
      
      lose = ch->level*ch->level*3+ch->level*300+25;
      /*lose = (exp_level(ch->class, ch->level+1) - exp_level(ch->class, ch->level)) /30;*/

      gain_exp( ch, 0 - lose );
      ch_printf( ch, "You recall from combat!  You lose %d exps.\n\r", lose );
      if(which_god(ch)==GOD_ORDER || which_god(ch)==GOD_CHAOS ||
         which_god(ch)==GOD_DEMISE )
      if(which_god(victim)==GOD_ORDER || which_god(victim)==GOD_CHAOS ||
         which_god(victim)==GOD_DEMISE )
      if(which_god(victim)!=which_god(ch) && !IS_NPC(ch) && !IS_NPC(victim) )
      {
       char buf2 [MAX_INPUT_LENGTH];
       sprintf(buf2, "%s", get_name(ch));
       if (which_god(victim)==GOD_CHAOS)
         sprintf(buf, "%s has recalled from the erratic frenzy of %s!", 
                 buf2, get_name(victim));
       if (which_god(victim)==GOD_CHAOS)
         sprintf(buf, "%s has recalled from the righteous fury of %s!", 
                 buf2, get_name(victim));
       if (which_god(victim)==GOD_DEMISE)
         sprintf(buf, "%s has recalled from the dark fury of %s!", 
                 buf2, get_name(victim));
       do_battle( buf );
      }

      if( !IS_NPC( ch))
	leave_fighting( ch, ch->in_room);
    }
  
  /* This is silly...since its magical now...Order 3/10/94 
     ch->move /= 2; */

  if( location->vnum < 3 )
    location=get_room_index(ROOM_VNUM_TEMPLE);

  act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
  if(ch->position==POS_FIGHTING)
    leave_fighting( ch, ch->in_room);
  char_from_room( ch );
  char_to_room( ch, location );
  act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
  
  /*  Pets recall with char    Chaos 12/6/93   */
  for( fch=first_char; fch!=NULL; fch=fch->next)
    if(IS_AFFECTED( fch, AFF_CHARM) && fch->master==ch)
      do_recall(fch, "");
  
  return;
}


void do_train( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mob;
  int hp_gain = 0;
  int mana_gain = 0;
  int move_gain = 0;
  sh_int *pAbility;
  char *pOutput;
  int cost, pt;
  
  if ( IS_NPC(ch) )
    return;
  
  pAbility = NULL;
  pOutput = NULL;
  
  /*
   * Check for trainer.
   */
  for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
    {
      if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	break;
    }
  
  if ( mob == NULL )
    {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }
  
  if ( argument[0] == '\0' )
    {
      sprintf( buf, "You have %d practice sessions.\n\r", ch->practice );
      send_to_char( buf, ch );
      argument = "foo";
    }
  
  cost = 5;
  pt=18+ch->level/15;
  
  if ( !strcasecmp( argument, "str" ) )
    {
      if ( class_table[ch->class].attr_prime == APPLY_STR )
	cost    = 3;
      pAbility    = &ch->pcdata->perm_str;
      pOutput     = "strength";
    }
  
  else if ( !strcasecmp( argument, "int" ) )
    {
      if ( class_table[ch->class].attr_prime == APPLY_INT )
	cost    = 3;
      pAbility    = &ch->pcdata->perm_int;
      pOutput     = "intelligence";
    }
  
  else if ( !strcasecmp( argument, "wis" ) )
    {
      if ( class_table[ch->class].attr_prime == APPLY_WIS )
	cost    = 3;
      pAbility    = &ch->pcdata->perm_wis;
      pOutput     = "wisdom";
    }
  
  else if ( !strcasecmp( argument, "dex" ) )
    {
      if ( class_table[ch->class].attr_prime == APPLY_DEX )
	cost    = 3;
      pAbility    = &ch->pcdata->perm_dex;
      pOutput     = "dexterity";
    }
  
  else if ( !strcasecmp( argument, "con" ) )
    {
      if ( class_table[ch->class].attr_prime == APPLY_CON )
	cost    = 3;
      pAbility    = &ch->pcdata->perm_con;
      pOutput     = "constitution";
    }
  
  else if ( !strcasecmp( argument, "hp" ) )
    {
      pAbility = &ch->max_hit;
      pOutput = "number of hit points";
      cost = 1;    /* this is pracs per "train hp" */
      hp_gain = 2; /* this is hp gained per "train hp" */
    }
  
  else if ( !strcasecmp( argument, "mana" ) )
    {
      pAbility = &ch->max_mana;
      pOutput = "amount of mana";
      cost =1;
      mana_gain = 3;
    }
  
  else if ( !strcasecmp( argument, "move" ) )
    {
      pAbility = &ch->max_move;
      pOutput = "amount of move";
      cost =1;
      move_gain = 3;
    }
  else if ( !strcasecmp( argument, "prac" ) )
    {
      send_to_char( "Your train costs 5 of your maximum hit points.\n\r", ch );
    }
  
  else
    {
      strcpy( buf, "You can train: hp" );
      if ( ch->class!=CLASS_RANGER   &&
	  ch->class!=CLASS_ROGUE    &&
	  ch->class!=CLASS_ASSASSIN )
	strcat( buf, " mana" );
      if ( ch->class==CLASS_RANGER   ||
	  ch->class==CLASS_ROGUE    ||
	  ch->class==CLASS_ASSASSIN )
	strcat( buf, " move" );
      strcat( buf, " prac" );
      if ( ch->pcdata->perm_str < pt ) strcat( buf, " str" );
      if ( ch->pcdata->perm_int < pt ) strcat( buf, " int" );
      if ( ch->pcdata->perm_wis < pt ) strcat( buf, " wis" );
      if ( ch->pcdata->perm_dex < pt ) strcat( buf, " dex" );
      if ( ch->pcdata->perm_con < pt ) strcat( buf, " con" );
      
      if ( buf[strlen(buf)-1] != ':' )
        {
	  strcat( buf, ".\n\r" );
	  send_to_char( buf, ch );
        }
      return;
    }

    if ( !strcasecmp( argument, "prac" ) )
      {

        if ( ch->actual_max_hit < ( 30 + ch->level*2 )  )
         {
            send_to_char( "You don't have enough hit points.\n\r", ch );
            return;
         }

             ch->practice        += 1;
             ch->max_hit         -= 5;
             ch->hit             -= 5;
             ch->hit = UMAX( ch->hit, 1 );
	     ch->actual_max_hit  -= 5;
/*
 Decreased hp cost of gaining practices so first_person will actually use it
 - Martin 4/8/98

             ch->max_hit         -= (10+ch->level/5);
             ch->hit             -= (10+ch->level/5);
             ch->hit = UMAX( ch->hit, 1 );
	     ch->actual_max_hit  -= (10+ch->level/5);
*/
             if( ch->hit > ch->max_hit )
               ch->hit = ch->max_hit;
             act( "Your practices increase!", ch, NULL, NULL, TO_CHAR );
             act( "$n's practices increase!", ch, NULL, NULL, TO_ROOM );
             return;
      }


    if ( !strcasecmp( argument, "hp" ) )
      {

        if ( cost > ch->practice )
         {
            send_to_char( "You don't have enough practices.\n\r", ch );
            return;
         }

             ch->practice        -= cost;
             ch->max_hit         += hp_gain;
             ch->hit             += hp_gain;
	     ch->actual_max_hit  += hp_gain;
             act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
             act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
             return;
      }


if ( !strcasecmp( argument, "move" ))
  {
  if ( ch->class!=CLASS_RANGER   &&
       ch->class!=CLASS_ROGUE    &&
       ch->class!=CLASS_ASSASSIN )
    {
    send_to_char("You can't train that, wrong class.\n\r",ch);
    return;
    }

  if ( cost > ch->practice )
    {
    send_to_char( "You don't have enough practices.\n\r", ch );
    return;
    }

  ch->practice        -= cost;
  ch->max_move        += move_gain;
  ch->move            += move_gain;
  ch->actual_max_move += move_gain;
  act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
  act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
  return;
  }


if ( !strcasecmp( argument, "mana" ) )
  {
  if(ch->class==CLASS_RANGER   ||
     ch->class==CLASS_ROGUE    ||
     ch->class==CLASS_ASSASSIN )
    {
    send_to_char("You can't train that, wrong class.\n\r",ch);
    return;
    }

        if ( cost > ch->practice )
          {
            send_to_char( "You don't have enough practices.\n\r", ch );
            return;
          }

             ch->practice        -= cost;
             ch->max_mana        += mana_gain;
             ch->mana            += mana_gain;
             ch->actual_max_mana += mana_gain;
             act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
             act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
             return;
  }


    if ( *pAbility >= pt )
    {
        act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
        return;
    }

    if ( cost > ch->practice )
    {
        send_to_char( "You don't have enough practices.\n\r", ch );
        return;
    }

    ch->practice        -= cost;
    *pAbility           += 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}



void do_stealth( CHAR_DATA *ch, char *argument )
  {
  AFFECT_DATA af;

  send_to_char( "You attempt to move silently and remain hidden.\n\r", ch );
  affect_strip( ch, gsn_stealth );
  affect_strip( ch, gsn_greater_stealth );

  if(multi(ch, gsn_stealth)==-1 && !IS_NPC(ch))
    return;

  if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_stealth] )
    {
    if(IS_NPC(ch)||number_percent()<ch->pcdata->learned[gsn_greater_sneak])
      send_to_char("You start moving stealthily.\n\r",ch);
    af.type      = gsn_stealth;
    if(multi(ch, gsn_stealth)==-1)
      af.duration = 12;
    else
      af.duration  = 1+ch->mclass[multi(ch,gsn_stealth)]/2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_STEALTH;
    affect_to_char( ch, &af );
    /* Greater stealth support - Martin 6/8/98 */ 
     if(!IS_NPC(ch) 
        && number_percent()<ch->pcdata->learned[gsn_greater_stealth])
     {
      send_to_char("You vanish from the prying eyes of mortality.\n\r",ch);
      af.type      = gsn_greater_stealth;
      af.bitvector = 0;
      affect_to_char( ch, &af );
     }
    }

  return;
  }

void do_clear_path( CHAR_DATA *ch, char *argument )
  {
  AFFECT_DATA af;
  affect_strip( ch, gsn_clear_path );
  if(multi(ch, gsn_clear_path)==-1 && !IS_NPC(ch))
    return;

  if( IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_clear_path])
    {
    send_to_char("You are now clearing a path.\n\r", ch);
    ch->speed=0;
    af.type     = gsn_clear_path;
    af.duration = 24;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_CLEAR;
    affect_to_char( ch, &af);
    }
  return;
  }

void do_hunt( CHAR_DATA *ch, char *argument )
   {
   AFFECT_DATA af;
   affect_strip( ch, gsn_hunt);
  if(multi(ch, gsn_hunt)==-1 && !IS_NPC(ch))
    return;

   if( IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_hunt])
     {
     send_to_char ("You start to hunt.\n\r", ch);
     af.type      = gsn_hunt;
     af.duration  = 24;
     af.location  = APPLY_NONE;
     af.modifier  = 0;
     af.bitvector = AFF_HUNT;
     affect_to_char( ch, &af);
     }
   return;
   }

void do_track( CHAR_DATA *ch, char *argument)
{
  int cnt;
  char buf[MAX_STRING_LENGTH];
  char victim[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
   return; 
  argument = one_argument(argument, victim);
  if (victim[0] == '\0')
  {
  
  if(IS_NPC( ch) || multi(ch, gsn_hunt)==-1 )
    {
     send_to_char( "You have no right to track.\n\r", ch);
     return;
    }
  if(number_percent() < ch->pcdata->learned[gsn_track] &&
        !IS_AFFECTED( ch, AFF_BLIND))
     {
     send_to_char("You locate the following tracks:\n\r", ch);
     for(cnt=0;cnt<MAX_LAST_LEFT;cnt++)
       if(ch->in_room->last_left[cnt][0]!='\0')
         {
         if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_FLY)
	  && !IS_NPC(ch) 
          && number_percent() > ch->pcdata->learned[gsn_greater_track])
   	    continue;
          
         strcpy(buf, ch->in_room->last_left[cnt]);
         if(buf[0]>='a' && buf[0]<='z')
           buf[0]-=('a'-'A');
         strcat(buf," ");
         if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_NORTH))
            strcat(buf, "leads to the north.\n\r"); 
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_SOUTH))
            strcat(buf, "leads to the south.\n\r"); 
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_EAST))
            strcat(buf, "leads to the east.\n\r"); 
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_WEST))
            strcat(buf, "leads to the west.\n\r"); 
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_UP))
            strcat(buf, "leads up.\n\r"); 
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_DOWN))
            strcat(buf, "leads down.\n\r"); 
          send_to_char(buf, ch);
         }
     }
  }
  else
  {
    bool found = FALSE;
    CHAR_DATA *rch;
    if (IS_NPC(ch) || multi(ch, gsn_greater_track) == -1)
    {
 	send_to_char("You have no right to track!\n\r", ch);
        return;
    }
    for(rch=ch->in_room->first_person;rch !=NULL; rch=rch->next_in_room)
    {
     if (!strcasecmp(rch->name, victim))
      { 
       act("You're already in the same room as $N.", ch, NULL, rch, TO_CHAR);
       return;
      }
    }
     for(cnt=0;cnt<MAX_LAST_LEFT;cnt++)
     {

       if (ch->in_room->last_left[cnt][0]== '\0') 
          continue;
       if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_FLY) &&
	   !IS_NPC(ch) &&
	   number_percent() > ch->pcdata->learned[gsn_greater_track])
          continue;
       if (!strcasecmp(victim,ch->in_room->last_left[cnt]))
       {
        ch_printf(ch, "You find the tracks of %s and quickly follow them.\n\r",
      	  capitalize(victim) );
        STRFREE (ch->pcdata->tracking );
        ch->pcdata->tracking = STRALLOC(victim); 
        found = TRUE;
        break;
       }
     }
     if (found)
     {
         if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_NORTH))
 	    do_north(ch, "");
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_SOUTH))
 	    do_south(ch, "");
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_EAST))
 	    do_east(ch, "");
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_WEST))
 	    do_west(ch, "");
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_UP))
 	    do_up(ch, "");
         else if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_DOWN))
 	    do_down(ch, "");
     }
     else
      ch_printf(ch, "You cannot find any tracks made by %s.\n\r", capitalize(victim));
  }
 return;
}

int get_max_speed( CHAR_DATA *ch )
  {
  int spd;
  spd = race_table[ch->race].max_speed;
  if( IS_AFFECTED( ch, AFF_HASTE ) )  
    spd++;
  if (IS_IMMORTAL(ch) || ch->which_god==GOD_POLICE)
   spd=4;
  return( spd );
  }

void do_camp( CHAR_DATA *ch, char *argument )
  {
  if(multi(ch, gsn_camp)==-1 && !IS_NPC(ch))
    return;

  if(ch->position<=POS_RESTING && in_camp(ch))
    {
    send_to_char("You are already camping.\n\r", ch);
    return;
    }
  if(ch->position!=POS_STANDING)
    {
    send_to_char("You must be standing in order to establish a camp.\n\r", ch);
    return;
    }
  if( IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_camp])
    {
    send_to_char("You establish a camp and begin recuperating.\n\r", ch);
    act("$n establishes a camp.",ch,NULL,NULL,TO_ROOM);
    ch->position=POS_RESTING;
    SET_BIT(ch->affected2_by, 0-AFF2_CAMPING);
    }
  else
    send_to_char("You fail to establish a good camp.\n\r", ch);
  return;
  }

void do_travel( CHAR_DATA *ch, char *argument )
  {
  if( IS_NPC( ch ) )
    return;

  if( ch->position != POS_STANDING )
    send_to_char( "You can't travel right now.\n\r", ch );

  if( ch->pcdata->travel == -1 &&
      *argument == '\0' )
    {
    send_to_char( "You must pick a direction to travel.\n\r", ch );
    return;
    }

  ch->pcdata->travel = -1;

  if( *argument=='N' || *argument=='n' )
    {
    send_to_char( "You travel north.\n\r", ch );
    ch->pcdata->travel = 0;
    }

  if( *argument=='S' || *argument=='s' )
    {
    send_to_char( "You travel south.\n\r", ch );
    ch->pcdata->travel = 2;
    }

  if( *argument=='E' || *argument=='e' )
    {
    send_to_char( "You travel east.\n\r", ch );
    ch->pcdata->travel = 1;
    }

  if( *argument=='w' || *argument=='w' )
    {
    send_to_char( "You travel west.\n\r", ch );
    ch->pcdata->travel = 3;
    }

  if( *argument=='U' || *argument=='u' )
    {
    send_to_char( "You travel up.\n\r", ch );
    ch->pcdata->travel = 4;
    }

  if( *argument=='D' || *argument=='d' )
    {
    send_to_char( "You travel down.\n\r", ch );
    ch->pcdata->travel = 5;
    }

  if( ch->pcdata->travel == -1 )
    {
    ch->pcdata->travel_from = NULL;
    send_to_char( "You stop traveling.\n\r", ch );
    }
  else
    {
    ch->pcdata->travel_from = ch->in_room;
    move_char( ch, ch->pcdata->travel );
    }


  return;
  }

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *gch;
        EXIT_DATA *pexit;
        int        door;
        char       arg [ MAX_INPUT_LENGTH ];

        if ( multi(ch, gsn_bashdoor) == -1 )
        {
            send_to_char( "You're not enough of a warrior to bash doors!\n\r", ch );
            return;
        }

        one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
            send_to_char( "Bash what?\n\r", ch );
            return;
        }

        if ( ch->fighting )
        {
            send_to_char( "You can't break off your fight.\n\r", ch );
            return;
        }

        if ( ( door = find_door( ch, arg ) ) >= 0 )
        {
            ROOM_INDEX_DATA *to_room;
            EXIT_DATA       *pexit_rev;
            int              chance;
            char            *keyword;

	    pexit = ch->in_room->exit[door];
            if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
            {
                send_to_char( "Calm down.  It is already open.\n\r", ch );
                return;
            }
            WAIT_STATE( ch, skill_table[gsn_bashdoor].beats );

            if ( IS_SET( pexit->exit_info, EX_HIDDEN ) )
                keyword = "wall";
            else
                keyword = pexit->keyword;
            if ( !IS_NPC(ch) )
                chance = ch->pcdata->learned[gsn_bashdoor]/ 2;
            else
                chance = 90;
            if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
                chance /= 3;

            if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
            &&   ch->move >= 15
            &&   number_percent( ) < ( chance + 4 * ( get_curr_str( ch ) - 19 ) ) )
            {
                REMOVE_BIT( pexit->exit_info, EX_CLOSED );
                if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
                REMOVE_BIT( pexit->exit_info, EX_LOCKED );
                SET_BIT( pexit->exit_info, EX_BASHED );

                act("Crash!  You bashed open the $d!", ch, NULL, keyword, TO_CHAR );
                act("$n bashes open the $d!",          ch, NULL, keyword, TO_ROOM );

                if ( (to_room = pexit->to_room) != NULL
                &&   (pexit_rev = pexit->to_room->exit[reverse(getDirNumber(arg))]) != NULL
                &&    pexit_rev->to_room        == ch->in_room )
                {
                        CHAR_DATA *rch;

                        REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                        if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
                          REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                        SET_BIT( pexit_rev->exit_info, EX_BASHED );

                        for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
                        {
                            act("The $d crashes open!",
                                rch, NULL, pexit_rev->keyword, TO_CHAR );
                        }
                }
                damage( ch, ch, ( ch->max_hit / 20 ), gsn_bashdoor );

            }
            else
            {
                act("WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
                        ch, NULL, keyword, TO_CHAR );
                act("WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
                        ch, NULL, keyword, TO_ROOM );
                damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
            }
        }
        else
        {
            act("WHAAAAM!!!  You bash against the wall, but it doesn't
 budge.",
                ch, NULL, NULL, TO_CHAR );
            act("WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
                ch, NULL, NULL, TO_ROOM );
            damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
        }
        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
           {
                 if ( IS_AWAKE( gch )
                 && !gch->fighting
                 && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
                 && ( ch->level - gch->level <= 4 )
                 && number_bits( 2 ) == 0 )
                 multi_hit( gch, ch, TYPE_UNDEFINED );
           }

        return;
}
  /*  Creates random maze and returns the vnum of farthest room from start */

int mazegen( CHAR_DATA *ch, int start_room, int x_size, int y_size, int z_size, int seed)
{
  int total_rooms, room_count, farthest_room, old_farthest;
  int x, y, z, door, dest, room;

  starting_room = start_room;

  if( x_size<1)
    x_size=1;
  if( y_size<1)
    y_size=1;
  if( z_size<1)
    z_size=1;
  total_rooms = x_size* y_size* z_size;
  size_x=x_size;
  size_y=y_size;
  size_z=z_size;
  old_farthest=0;
  farthest_room=starting_room;
  room_count=0;
  user=ch;

  for( room=starting_room; room< total_rooms + starting_room; room++)
    for( door=0; door<6; door++)
      set_exit( room, door, -1);
  room=starting_room;
  srand( seed );

  while( TRUE )
    {
    x=(room-starting_room)%size_x;
    y=((room-starting_room)/size_x)%size_y;
    z=(room-starting_room)/size_x/size_y;
    
    if( (dest=find_random( x, y, z)) >=0 )
      {
      set_exit( room, last_door, dest);
      room=dest;
      room_count++;
      }
    else
      if( (dest=find_reverse( x, y, z)) >=0 )
        {
        set_exit( room, last_door, dest);
        room=dest;
        room_count--;
        }
      else
        return( farthest_room);
    if( room_count > old_farthest )
      {
      farthest_room = room;
      old_farthest = room_count;
      }
      
    }
    return( farthest_room);

    
}
  
int reverse( int in )
{
  switch( in )
    {
    case 0: return (2);break;
    case 1: return (3);break;
    case 2: return (0);break;
    case 3: return (1);break;
    case 4: return (5);break;
    case 5: return (4);break;
    }
  return(-1);
}

bool is_exit( int room, int door)
{
	EXIT_DATA *pexit;
  if( room_index[room]==NULL)
    {
    log_string( "bad room.");
    return( FALSE);
    }
	pexit = room_index[room]->exit[door];
  if( pexit == NULL )
    return( FALSE );
	return( TRUE );
}

void set_exit( int room, int door, int dest)
{
    
	EXIT_DATA *pexit;
  if( room<1 || dest<-1 || room_index[ room ] == NULL || door<0 || door>5
           || ( dest != -1 && room_index[dest]==NULL))
    {
    char buf[200];
    sprintf( buf, "Bad room connect at %d door %d to %d", room, door, dest );
    log_string( buf );
    return;
    }

  pexit = room_index[ room ]->exit[door];
  if( pexit==NULL && dest>0)
    {
    CREATE(pexit, EXIT_DATA, 1);
    pexit->description	= STRALLOC("");
    pexit->keyword        = STRALLOC("");
    pexit->pvnum  = -1;
    pexit->key		= -1;
    pexit->vnum		= room;
    pexit->to_room 	= NULL;
    room_index[ room ]->exit[door]= pexit;
    top_exit++;
    }
  if( dest>0 )
    {
    pexit->exit_info=0;
    pexit->to_room = room_index[ dest ];
    }
  else
    {
    if( pexit==NULL)
      return;
    if (pexit->keyword != NULL )
      STRFREE (pexit->keyword );
    STRFREE (pexit->description );
    DISPOSE( pexit );
    room_index[ room ]->exit[door]=NULL;
    top_exit--;
    }
  return;
}

int get_room( int x, int y, int z)
{
  return( starting_room + x + ( y * size_x ) + ( z * size_x * size_y ) );
}

int find_reverse( int x, int y, int z )
{
  int door, dx, dy, dz;
  int room, dest;
  room= get_room( x, y, z);
  for( door=0; door<6; door++)
    if( !is_exit( room , door ))
      {
      dx=x;
      dy=y;
      dz=z;
      switch( door )
        {
        case 0: if( dy< size_y-1) dy++;break;
        case 1: if( dx< size_x-1) dx++;break;
        case 2: if( dy> 0) dy--;break;
        case 3: if( dx> 0) dx--;break;
        case 4: if( dz< size_z-1) dz++;break;
        case 5: if( dz> 0) dz--;break;
        }
      dest= get_room( dx, dy, dz);
      if( (x!=dx || y!=dy || z!=dz) && 
          is_exit( dest , reverse( door ) ) )
        {
        last_door=door;
        return( dest);
        }
      }
    return( -1 );
}
        
      
int find_random( int x, int y, int z )
{
  int door, dx, dy, dz;
  int start, direction , dest, room;
  bool found;

  found = FALSE;

  start=rand()%5;
  direction=(2*(rand()%2))-1;
  door=start;
  room = get_room( x, y, z);

  while( door!=start || !found )
    {
    found=TRUE;
    
    if( !is_exit( room, door ))
      {
      dx=x;
      dy=y;
      dz=z;
      switch( door )
        {
        case 0: if( dy< size_y-1) dy++;break;
        case 1: if( dx< size_x-1) dx++;break;
        case 2: if( dy> 0) dy--;break;
        case 3: if( dx> 0) dx--;break;
        case 4: if( dz< size_z-1) dz++;break;
        case 5: if( dz> 0) dz--;break;
        }
      if( (x!=dx || y!=dy || z!=dz) )
        {
        int tdoor;
        bool found;
        found=FALSE;
        dest= get_room( dx, dy, dz);
        for( tdoor=0; tdoor<6; tdoor++)
          if( is_exit( dest , tdoor ) )
            found=TRUE;
        if( !found)
          {
          last_door=door;
          return( dest);
          }
        }
      }
    door+=direction;
    if( door > 5 )
      door=0;
    if( door < 0 )
      door=5;
    }
  return( -1 );
} 
      
int which_door( int room, int dest)
{
	EXIT_DATA *pexit;
  int door;

  for( door=0; door<6; door++)
    {
	  pexit = room_index[ room ]->exit[door];
    if( pexit!=NULL)
      if( pexit->to_room->vnum == dest )
        return( door );
    }
  return( -1 );
}

