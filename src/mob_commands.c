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

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

extern int sscanf();
/*
 * Local functions.
 */

bool    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container, bool fDisplay ) );
char *			mprog_type_to_name	args( ( int type ) );
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case IN_FILE_PROG:          return "in_file_prog";
    case ACT_PROG:              return "act_prog";
    case SOCIAL_PROG:           return "social_prog";
    case SPEECH_PROG:           return "speech_prog";
    case RAND_PROG:             return "rand_prog";
    case FIGHT_PROG:            return "fight_prog";
    case HITPRCNT_PROG:         return "hitprcnt_prog";
    case DEATH_PROG:            return "death_prog";
    case KILL_PROG:             return "kill_prog";
    case ENTRY_PROG:            return "entry_prog";
    case GREET_PROG:            return "greet_prog";
    case ALL_GREET_PROG:        return "all_greet_prog";
    case GROUP_GREET_PROG:      return "group_greet_prog";
    case GIVE_PROG:             return "give_prog";
    case BRIBE_PROG:            return "bribe_prog";
    case RANGE_PROG:            return "range_prog";
    case TIME_PROG:             return "time_prog";
    default:                    return "ERROR_PROG";
    }
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MOBprograms which are set.
 */

void do_mpstat( CHAR_DATA *ch, char *argument )
{
    /* char        buf[ MAX_STRING_LENGTH ];
    char        arg[ MAX_INPUT_LENGTH  ];
    MPROG_DATA *mprg;
    CHAR_DATA  *victim; */

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
/*
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "MobProg stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	send_to_char( "Only Mobiles can have Programs!\n\r", ch);
	return;
    }

    if ( !( victim->pIndexData->progtypes ) )
    {
	send_to_char( "That Mobile has no Programs set.\n\r", ch);
	return;
    }

    sprintf( buf, "Name: %s.  Vnum: %u.\n\r",
	victim->name, victim->pIndexData->vnum );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s.\n\rLong  description: %s",
	    victim->short_descr,
	    victim->long_descr[0] != '\0' ?
	    victim->long_descr : "(none).\n\r" );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move );
    send_to_char( buf, ch );

    sprintf( buf,
	"Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
	victim->level,       victim->class,        victim->alignment,
	GET_AC( victim ),    victim->gold,         victim->exp );
    send_to_char( buf, ch );

    for ( mprg = victim->pIndexData->mobprogs; mprg != NULL;
	 mprg = mprg->next )
    {
      sprintf( buf, ">%s %s\n\r%s\n\r",
	      mprog_type_to_name( mprg->type ),
	      mprg->arglist,
	      mprg->comlist );
      send_to_char( buf, ch );
    }
  */
    do_mprogram( ch, argument );
    return;

}



/* prints the argument to all the rooms aroud the mobile */

void do_mpasound( CHAR_DATA *ch, char *argument )
{

  ROOM_INDEX_DATA *was_in_room;
  int              door;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    if ( argument[0] == '\0' )
    {
    if( IS_NPC( ch ))
        bug( "Mpasound - No argument: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
      EXIT_DATA       *pexit;
      
      if ( ( pexit = was_in_room->exit[door] ) != NULL
	  &&   pexit->to_room != NULL
	  &&   pexit->to_room != was_in_room )
      {
	ch->in_room = pexit->to_room;
	MOBtrigger  = FALSE;
	act( argument, ch, NULL, NULL, TO_ROOM );
      }
    }

  ch->in_room = was_in_room;
  return;

}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {  /* Not necessarily a bug.   could just be 'mpkill $r'    */
	/* bug( "MpKill - no argument: vnum %u.",
		ch->pIndexData->vnum );  */
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {    /* same as above */
    /*	bug( "MpKill - Victim not in room: vnum %u.",
	    ch->pIndexData->vnum );  */
	return;
    }

    if ( victim == ch )
    {
    if( IS_NPC( ch ))
	bug( "MpKill - Bad victim to attack: vnum %u.",
	    ch->pIndexData->vnum );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
    if( IS_NPC( ch ))
	bug( "MpKill - Charmed mob attacking master: vnum %u.",
	    ch->pIndexData->vnum );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {	
	/* bug( "MpKill - Already fighting: vnum %u", 
	    ch->pIndexData->vnum ); */
	return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy 
   items using all.xxxxx or just plain all of them */

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    one_argument( argument, arg );

    if ( arg[0] == '\0')
    {
    if( IS_NPC( ch ))
        bug( "Mpjunk - No argument: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if ( strcasecmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
      if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
      {
	unequip_char( ch, obj );
	extract_obj( obj );
	return;
      }
      if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	return; 
      extract_obj( obj );
    }
    else
      for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
        {
          if ( obj->wear_loc != WEAR_NONE)
	    unequip_char( ch, obj );
          extract_obj( obj );
        } 
      }

    return;

}

void do_mpjunk_person( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    CHAR_DATA *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    argument=one_argument( argument, arg );

    if ( arg[0] == '\0')
    {
    if( IS_NPC( ch ))
        bug( "Mpjunk - No argument: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        return;
    }

    one_argument( argument, arg );
 
    if ( strcasecmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
      if ( ( obj = get_obj_wear( victim, arg ) ) != NULL )
      {
	unequip_char( victim, obj );
	extract_obj( obj );
	return;
      }
      if ( ( obj = get_obj_carry( victim, arg ) ) == NULL )
	return; 
      extract_obj( obj );
    }
    else
      for ( obj = victim->first_carrying; obj != NULL; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
        {
          if ( obj->wear_loc != WEAR_NONE)
	    unequip_char( victim, obj );
          extract_obj( obj );
        } 
      }

    return;

}
/* prints the message to everyone in the room other than the mob and victim */

void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
  char       arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
       send_to_char( "Huh?\n\r", ch );
       return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    /* if( IS_NPC( ch ))
       bug( "Mpechoaround - No argument:  vnum %u.", ch->pIndexData->vnum ); */
       return;
    }

    if ( !( victim=get_char_room( ch, arg ) ) )
    {
    /* if( IS_NPC( ch ))
        bug( "Mpechoaround - victim does not exist: vnum %u.",
	    ch->pIndexData->vnum ); */
	return;
    }

    act( argument, victim, NULL, NULL, TO_ROOM );
    return;
}

/* prints the message to only the victim */

void do_mpechoat( CHAR_DATA *ch, char *argument )
{
  char       arg[ MAX_INPUT_LENGTH ];
  CHAR_DATA *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
       send_to_char( "Huh?\n\r", ch );
       return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
    /* if( IS_NPC( ch ))
       bug( "Mpechoat - No argument:  vnum %u.",
	   ch->pIndexData->vnum ); */
       return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
    /* if( IS_NPC( ch ))
        bug( "Mpechoat - victim does not exist: vnum %u.",
	    ch->pIndexData->vnum ); */
	return;
    }

    /*send_to_char( argument, victim );*/
    act( argument, victim, NULL, NULL, TO_CHAR );

    return;
}

/* prints the message to the room at large */

void do_mpecho( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    if ( argument[0] == '\0' )
    {
    /*    bug( "Mpecho - called w/o argument: vnum %u.",
	    ch->pIndexData->vnum ); */
        return;
    }

    act( argument, ch, NULL, NULL, TO_ROOM );
    if (!IS_NPC(ch))
      act( argument, ch, NULL, NULL, TO_CHAR );
    return;

}

void do_mpareaecho( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    if ( argument[0] == '\0' )
    {
    /*    bug( "Mpecho - called w/o argument: vnum %u.",
	    ch->pIndexData->vnum ); */
        return;
    }
    for ( vch = first_char; vch != NULL; vch = vch->next )
    {
        if( vch->in_room == NULL )
          continue;

        if ( vch->in_room->area == ch->in_room->area   &&   IS_AWAKE(vch) )
            ch_printf( vch, "%s\n\r", ansi_translate_text(vch, argument) );
    }

    return;

}
/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

void do_mpmload( CHAR_DATA *ch, char *argument )
  {
  char            arg[ MAX_INPUT_LENGTH ];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA      *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
    send_to_char( "Huh?\n\r", ch );
	  return;
    }
  if(IS_AFFECTED(ch,AFF_CHARM))
    return;

  one_argument( argument, arg );

  if ( arg[0] == '\0' || !is_number(arg) )
    {
    if( IS_NPC( ch ))
	  bug( "Mpmload - Bad vnum as arg: vnum %u.", ch->pIndexData->vnum );
	  return;
    }

  if ( ( pMobIndex = get_mob_index( atol( arg ) ) ) == NULL )
    {
    if( IS_NPC( ch ))
	  bug( "Mpmload - Bad mob vnum: vnum %u.", ch->pIndexData->vnum );
	  return;
    }

  victim = create_mobile( pMobIndex );
  char_to_room( victim, ch->in_room );
  return;
  }

void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
    if( IS_NPC( ch ))
        bug( "Mpoload - Bad syntax: vnum %u.",
	    ch->pIndexData->vnum );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
    if( IS_NPC( ch ))
	    bug( "Mpoload - Bad syntax: vnum %u.", ch->pIndexData->vnum );
	    return;
        }
	level = atol( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
        if( IS_NPC( ch ))
	    bug( "Mpoload - Bad level: vnum %u.", ch->pIndexData->vnum );
	    return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atol( arg1 ) ) ) == NULL )
    {
        if( IS_NPC( ch ))
	bug( "Mpoload - Bad vnum arg: vnum %u.", ch->pIndexData->vnum );
	return;
    }

     /* Don't load a second object */
    if( get_obj_list_vnum( ch, pObjIndex->vnum, ch->first_carrying ) )
      return;

/* bug("Mpoload - pos = %d.",ch->position); */
    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_room( obj, ch->in_room );
        get_obj( ch, obj, NULL, FALSE );
    }
    else
    {
    obj->sac_timer=OBJ_SAC_TIME;
	  obj_to_room( obj, ch->in_room );
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */

void do_mppurge( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->first_person; victim != NULL; victim = vnext )
	{
	  vnext = victim->next_in_room;
	  if ( IS_NPC( victim ) && victim != ch )
	    extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->first_content; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;
          if( obj->item_type != ITEM_CORPSE_NPC )
	    extract_obj( obj );
	}

	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
          if( obj->item_type != ITEM_CORPSE_NPC )
	    extract_obj( obj );
	}
	else
	{
        /* if( IS_NPC( ch ))
	    bug( "Mppurge - Bad argument: vnum %u.", ch->pIndexData->vnum ); */
	}
	return;
    }

    if ( !IS_NPC( victim ) )
    {
        if( IS_NPC( ch ))
	bug( "Mppurge - Purging a PC: vnum %u.", ch->pIndexData->vnum );
	return;
    }

        /*   Use the junk room    -   Chaos 2/8/95   */
    SET_BIT( victim->act ,  ACT_WILL_DIE );
    char_from_room( victim );
    char_to_room( victim , room_index[3] );
    return;
}


/* lets the mobile goto any location it wishes that is not private */

void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;

    if( !IS_NPC( ch ) &&  ch->level < 99 && ch->fighting != NULL )
      return;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        if( IS_NPC( ch ))
	bug( "Mpgoto - No argument: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        if( IS_NPC( ch ))
	bug( "Mpgoto - No such location: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if( IS_NPC(ch) && location->vnum<3 )
      location = room_index[3];

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, location );

    return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    if( !IS_NPC( ch ) && ( ch->fighting!=NULL || ch->position==POS_FIGHTING ) )
      return;
 
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        if( IS_NPC( ch ))
	bug( "Mpat - Bad argument: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        if( IS_NPC( ch ))
	bug( "Mpat - No such location: vnum %u.", ch->pIndexData->vnum );
	return;
    }

    if( IS_NPC(ch) && location->vnum<3 )
      location = room_index[3];

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = first_char; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}
 
/* lets the mobile transfer first_person.  the all argument transfers
   everyone in the current room to the specified location */

void do_mptransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	return;
    }

     /* test for numerical name */
   if( *arg1 >= '0' && *arg1 <= '9' && *arg2=='\0' )
     return;

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    return;
	}
/* Why should it matter, when the mobile can check if it's private ?
	if ( room_is_private( location ) )
	{
	    bug( "Mptransfer - Private room: vnum %u.",
		ch->pIndexData->vnum );
	    return;
	}  */
      if( location == ch->in_room )  /* This feature only allowed for 1 arg */
        return;
    }

    if ( !strcasecmp( arg1, "all" )  )
      {
      CHAR_DATA *next_victim;

      if( arg2[0]=='\0' )
        return;

      for( victim = ch->in_room->first_person ; victim != NULL; victim = next_victim)
          {
          next_victim=victim->next_in_room;
          if( victim != ch )
            transference( victim, location );
          }
      return;
      }

    if ( !strcasecmp( arg1, "pcs" )  )
      {
      CHAR_DATA *next_victim;

      if( arg2[0]=='\0' )
        return;

      for( victim = ch->in_room->first_person ; victim != NULL; victim = next_victim)
          {
          next_victim=victim->next_in_room;
          if( victim != ch && ( !IS_NPC( victim ) ||
                                 IS_AFFECTED( victim, AFF_CHARM)))
            transference( victim, location );
          }
      return;
      }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      /* char tbuf[180];
       if( IS_NPC( ch ))
        {
	sprintf( tbuf, "Mptransfer - No such person: '%s' by vnum %u.",
	    arg1, ch->pIndexData->vnum );
        bug( tbuf, 0 );
        } */
	return;
    }

    if ( victim->in_room == NULL )
    {
      /* if( IS_NPC( ch ))
	bug( "Mptransfer - Victim in Null: vnum %u.",
	    ch->pIndexData->vnum ); */
	return;
    }

    if( victim != ch )
      transference( victim, location );

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

void do_mpforce( CHAR_DATA *ch, char *argument )
  {
  char buf[ MAX_INPUT_LENGTH ];
  char arg[ MAX_INPUT_LENGTH ];

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	  send_to_char( "Huh?\n\r", ch );
	  return;
    }
  if(IS_AFFECTED(ch,AFF_CHARM))
    return;

  argument = one_argument( argument, arg );
  if ( arg==NULL || argument==NULL || arg[0] == '\0' || argument[0] == '\0' )
    {
      /* if( IS_NPC( ch ))
	  bug( "Mpforce - Bad syntax: vnum %u.", ch->pIndexData->vnum ); */
	  return;
    }

  *buf=27;
  *(buf+1)='\0';
  strcat( buf, argument );


  if ( !strcasecmp( arg, "all" ) )
    {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

	  for ( vch = first_char; vch != NULL; vch = vch_next )
	    {
	    vch_next = vch->next;

	    if ( vch->in_room == ch->in_room &&
             can_see( ch, vch ) )
	      {
	       if (vch->desc && vch->desc->connected==CON_EDITING)
  	       {
   	        vch->desc->connected=CON_PLAYING;
   	        interpret(vch, buf);
   	        vch->desc->connected=CON_EDITING;
               }
               else
                interpret(vch, buf);
	      }
	    }
    }
  else if ( !strcasecmp( arg, "allgame" ) )
    {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

	  for ( vch = first_char; vch != NULL; vch = vch_next )
	    {
	    vch_next = vch->next;
            if(is_desc_valid(vch)) 
              {
	       if (vch->desc && vch->desc->connected==CON_EDITING)
  	       {
   	        vch->desc->connected=CON_PLAYING;
   	        interpret(vch, buf);
   	        vch->desc->connected=CON_EDITING;
               }
               else
                interpret(vch, buf);
              }
	    }
    }
  else
    {
	  CHAR_DATA *victim;

	  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	    {
	    /* bug( "Mpforce - No such victim: vnum %u.",
	  	    ch->pIndexData->vnum ); */
	    return;
	    }

	  if ( victim == ch )
    	    {
            if( IS_NPC( ch ))
	    bug( "Mpforce - Forcing oneself: vnum %u.",
	    	  ch->pIndexData->vnum );
	    return;
	    }

	       if (victim->desc && victim->desc->connected==CON_EDITING)
  	       {
   	        victim->desc->connected=CON_PLAYING;
   	        interpret(victim, buf);
   	        victim->desc->connected=CON_EDITING;
               }
               else
                interpret(victim, buf);
    }

  return;
  }

void do_mpmset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    if( strlen( argument ) > 160 )
      *(argument + 160 ) = '\0';
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atol( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !strcasecmp( arg2, "str" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 3 || value > 25 )
	{
	    return;
	}

	victim->pcdata->perm_str = value;
	return;
    }

    if ( !strcasecmp( arg2, "int" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 3 || value > 25 )
	{
	    return;
	}

	victim->pcdata->perm_int = value;
	return;
    }

    if ( !strcasecmp( arg2, "wis" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 3 || value > 25 )
	{
	    return;
	}

	victim->pcdata->perm_wis = value;
	return;
    }

    if ( !strcasecmp( arg2, "dex" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 3 || value > 25 )
	{
	    return;
	}

	victim->pcdata->perm_dex = value;
	return;
    }

    if ( !strcasecmp( arg2, "con" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 3 || value > 25 )
	{
	    return;
	}

	victim->pcdata->perm_con = value;
	return;
    }

    if ( !strcasecmp( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    return;
	}
	victim->sex = value;
	return;
    }

    if ( !strcasecmp( arg2, "class" ) )
    {
	if ( value < 0 || value >= MAX_CLASS )
	{
	    return;
	}
	victim->class = value;
	return;
    }

    if ( !strcasecmp( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 0 || value > 99 )
	{
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !strcasecmp( arg2, "trust" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 0 || value > 99 )
	{
	    return;
	}
	victim->trust= value;
	return;
    }

 if ( !strcasecmp( arg2, "exp" ) )
   {
   if ( value < 0 )
     return;
   victim->exp = value;
   return;
   }

 if ( !strcasecmp( arg2, "gold" ) )
   {
   if ( value < 0 )
     return;
   victim->gold = value;
   return;
   }

 if ( !strcasecmp( arg2, "hp" ) )
   {
   if ( value < 1 )
     return;
   victim->max_hit = value;
   return;
   }

 if ( !strcasecmp( arg2, "mana" ) )
   {
   if ( value < 1 )
     return;
   victim->max_mana = value;
   return;
   }

 if ( !strcasecmp( arg2, "move" ) )
   {
   if ( value < 1 )
     return;
   victim->max_move = value;
   return;
   }

    if ( !strcasecmp( arg2, "practice" ) )
    {
	if ( value < 0 || value > 1000 )
	{
	    return;
	}
	victim->practice = value;
	return;
    }

/*  Removed by Chaos  8/10/98
    if ( !strcasecmp( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    return;
	}
	victim->alignment = value;
	return;
    }  */

    if ( !strcasecmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !strcasecmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !strcasecmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !strcasecmp( arg2, "name" ) )
    {
	if ( !IS_NPC(victim) )
	    return;

	STRFREE (victim->name );
        if( !strcasecmp( arg3, "null") )
          victim->name = QUICKLINK(victim->pIndexData->player_name);
        else
	  victim->name = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "short" ) )
    {
        STRFREE (victim->short_descr );
        if( !strcasecmp( arg3, "null")  )
          {
          if( IS_NPC(victim))
            victim->short_descr = QUICKLINK(victim->pIndexData->short_descr);
          else
            victim->short_descr = STRALLOC( "" );
          }
        else
	  victim->short_descr = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "long" ) )
    {
      STRFREE (victim->long_descr );
        if( !strcasecmp( arg3, "null") )
          {
          if( IS_NPC( victim ) )
            victim->long_descr = QUICKLINK(victim->pIndexData->long_descr);
          else
            victim->long_descr = STRALLOC( "" );
          }
        else
	  victim->long_descr = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "title" ) )
    {
	if ( IS_NPC(victim) )
	    return;

	set_title( victim, arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "spec" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    return;
	}

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    return;
	}

	return;
    }

  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      bug("%u bad parameters to 'mpmset quest'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }

  if ( !strcasecmp( arg2, "questr" ) )
    {
    int firstBit,len,vnum;

    if(sscanf(arg3,"%d %d %d %d",&vnum, &firstBit,&len,&value)!=4)
      {
      bug("%u bad parameters to 'mpmset questr'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      return;
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }

  if ( !strcasecmp( arg2, "randquest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d",&firstBit,&len)!=2)
      {
      bug("%u bad parameters to 'mpmset randquest'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      return;
      }

    value = number_bits(len);
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }
  if ( !strcasecmp( arg2, "randquestr" ) )
    {
    int firstBit,len,vnum;

    if(sscanf(arg3,"%d %d %d",&vnum,&firstBit,&len)!=3)
      {
      bug("%u bad parameters to 'mpmset randquestr'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      return;
      }

    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      return;
    value = number_bits(len);
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }

    /*
     * Generate usage message.
     */
    return;
}



void do_mposet( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if(IS_AFFECTED(ch,AFF_CHARM))
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atol( arg3 );

    /*
     * Set something.
     */
    if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !strcasecmp( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !strcasecmp( arg2, "setextra" ) )
      {
      obj->extra_flags=obj->extra_flags | value;
      return;
      }

    if ( !strcasecmp( arg2, "clrextra" ) )
      {
      obj->extra_flags=obj->extra_flags & NOT(value);
      return;
      }


    if ( !strcasecmp( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !strcasecmp( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !strcasecmp( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !strcasecmp( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !strcasecmp( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    if ( !strcasecmp( arg2, "name" ) )
    {
      STRFREE (obj->name );
        if( !strcasecmp( arg3, "null") )
	  obj->name = obj->pIndexData->name;
        else
	  obj->name = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "short" ) )
    {
	STRFREE (obj->short_descr );
        if( !strcasecmp( arg3, "null") )
	  obj->short_descr = obj->pIndexData->short_descr;
        else
	  obj->short_descr = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "long" ) )
    {
	STRFREE (obj->description );
        if( !strcasecmp( arg3, "null") )
	  obj->description = obj->pIndexData->description;
        else
	  obj->description = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "ed" ) )
    {
	EXTRA_DESCR_DATA *ed;

	argument = one_argument( argument, arg3 );
	if ( argument == NULL )
	{
	    return;
	}

	CREATE(ed, EXTRA_DESCR_DATA, 1);
	ed->keyword		= STRALLOC( arg3     );
	ed->description=STRALLOC(argument );
	LINK (ed, obj->first_extradesc, obj->last_extradesc, next, prev);
	return;
    }

  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      
      if( IS_NPC( ch ))
        {
      bug("Mob:%u bad parameters to 'mposet quest'",ch->pIndexData->vnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
        }
      return;
      }
    set_quest_bits( &obj->obj_quest, firstBit, len, value);
    return;
    }

  if ( !strcasecmp( arg2, "randquest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d",&firstBit,&len)!=2)
      {
      if( IS_NPC( ch ))
        {
      bug("Mob:%u bad parameters to 'mposet randquest'",ch->pIndexData->vnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
        }
      return;
      }
    value = number_bits( len );
    set_quest_bits( &obj->obj_quest, firstBit, len, value);
    return;
    }

    /*
     * Generate usage message.
     */
    return;
}

void do_mpmadd( CHAR_DATA *ch, char *argument )
  {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	  send_to_char( "Huh?\n\r", ch );
	  return;
    }
  if(IS_AFFECTED(ch,AFF_CHARM))
    return;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	  return;
    }

  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	  return;
    }

  /*
   * Snarf the value (which needs to be numeric).
   */
  value = is_number( arg3 ) ? atol( arg3 ) : -1;

  /*
   * Set something.
   */
  if ( !strcasecmp( arg2, "str" ) )
    {
	  if ( IS_NPC(victim) )
	    return;
	  value+=victim->pcdata->perm_str;
    value=(value < 3)?3:(value > 25)?25:value;
    victim->pcdata->perm_str=value;
	  return;
    }

  if ( !strcasecmp( arg2, "int" ) )
    {
	  if ( IS_NPC(victim) )
	    return;
	  value+=victim->pcdata->perm_int;
    value=(value < 3)?3:(value > 25)?25:value;
    victim->pcdata->perm_int=value;
	  return;
    }

  if ( !strcasecmp( arg2, "wis" ) )
    {
	  if ( IS_NPC(victim) )
	    return;
	  value+=victim->pcdata->perm_wis;
    value=(value < 3)?3:(value > 25)?25:value;
    victim->pcdata->perm_wis=value;
	  return;
    }

  if ( !strcasecmp( arg2, "dex" ) )
    {
	  if ( IS_NPC(victim) )
	    return;
	  value+=victim->pcdata->perm_dex;
    value=(value < 3)?3:(value > 25)?25:value;
    victim->pcdata->perm_dex=value;
	  return;
    }

  if ( !strcasecmp( arg2, "con" ) )
    {
	  if ( IS_NPC(victim) )
	    return;
	  value+=victim->pcdata->perm_con;
    value=(value < 3)?3:(value > 25)?25:value;
    victim->pcdata->perm_con=value;
	  return;
    }

  if ( !strcasecmp( arg2, "sex" ) )
    {
    victim->sex = (value + victim->sex) % 3;
	  return;
    }

  if ( !strcasecmp( arg2, "class" ) )
    {
	  victim->class = (victim->class + value) % MAX_CLASS;
	  return;
    }

  if ( !strcasecmp( arg2, "level" ) )
    {
	  if ( !IS_NPC(victim) )
	    return;
	  value+=victim->level;
    value=(value < 1)?1:(value > LEVEL_IMMORTAL)?LEVEL_IMMORTAL:value;
	  victim->level=value;
	  return;
    }

  if ( !strcasecmp( arg2, "exp" ) )
    {
    if ( value < 0 )
      return;
	  value+=victim->exp;
    value=(value < 0)?0:(value > exp_level(victim->class,LEVEL_IMMORTAL)?
          exp_level(victim->class,LEVEL_IMMORTAL):value);
	  victim->exp=value;
    return;
    }

  if ( !strcasecmp( arg2, "practice" ) )
    {
	  value+=victim->practice;
    value=(value < 0)?0:(value > 1000)?1000:value;
	  victim->practice = value;
	  return;
    }

  if ( !strcasecmp( arg2, "align" ) )
    {
	  value+=victim->alignment;
    value=(value < -1000)?-1000:(value > 1000)?1000:value;
	  victim->alignment = value;
	  return;
    }

  if ( !strcasecmp( arg2, "thirst" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->pcdata->condition[COND_THIRST];
    value=(value < 0)?0:(value > 100)?100:value;
	  victim->pcdata->condition[COND_THIRST] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "drunk" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->pcdata->condition[COND_DRUNK];
    value=(value < 0)?0:(value > 100)?100:value;
	  victim->pcdata->condition[COND_DRUNK] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "full" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->pcdata->condition[COND_FULL];
    value=(value < 0)?0:(value > 100)?100:value;
	  victim->pcdata->condition[COND_FULL] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "gold" ) )
    {
	  value+=victim->gold;
    value=(value < 0)?0:(value > 2000000000)?2000000000:value;
	  victim->gold = value;
	  return;
    }

  if ( !strcasecmp( arg2, "currhp" ) )
    {
	  value+=victim->hit;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->hit= value>victim->max_hit? victim->max_hit : value;
	  return;
    }

  if ( !strcasecmp( arg2, "currmana" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->mana;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->mana = value>victim->max_mana ? victim->max_mana : value;
	  return;
    }

  if ( !strcasecmp( arg2, "currmove" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->move;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->move = value>victim->max_move ? victim->max_move : value;
	  return;
    }

  if ( !strcasecmp( arg2, "hp" ) )
    {
	  value+=victim->max_hit;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->max_hit= value;
	  return;
    }

  if ( !strcasecmp( arg2, "mana" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->max_mana;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->max_mana = value;
	  return;
    }

  if ( !strcasecmp( arg2, "move" ) )
    {
	  if ( IS_NPC(victim) )
	    return;

	  value+=victim->max_move;
    value=(value < 1)?1:(value > 32000)?32000:value;
	  victim->max_move = value;
	  return;
    }

  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      bug("%u bad parameters to 'mpmadd quest'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if(IS_NPC(victim))
      value += get_quest_bits( victim->npcdata->mob_quest, firstBit, len);
    else
      value += get_quest_bits( 
          victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],
          firstBit, len);
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }
  if ( !strcasecmp( arg2, "questr" ) )
    {
    int firstBit,len,vnum;

    if(sscanf(arg3,"%d %d %d %d",&vnum,&firstBit,&len,&value)!=4)
      {
      bug("%u bad parameters to 'mpmadd questr'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      return;
    if(IS_NPC(victim))
      value += get_quest_bits( victim->npcdata->mob_quest, firstBit, len);
    else
      value += get_quest_bits( 
          victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
          firstBit, len);
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits( 
          &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
          firstBit, len, value );
    return;
    }

  /*
   * Generate usage message.
   */
  return;
  }



void do_mpoadd( CHAR_DATA *ch, char *argument )
  {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int value;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
	  send_to_char( "Huh?\n\r", ch );
	  return;
    }
  if(IS_AFFECTED(ch,AFF_CHARM))
    return;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	  return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	  return;

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number( arg3 ) ? atol( arg3 ) : -1;

  /*
   * Set something.
   */
  if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
    {
	  value+=obj->value[0];
    value=(value < 0)?0:(value > 100)?100:value;
	  obj->value[0]=value;
	  return;
    }

  if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
    {
	  value+=obj->value[1];
    value=(value < 0)?0:(value > 100)?100:value;
	  obj->value[1] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
    {
	  value+=obj->value[2];
    value=(value < 0)?0:(value > 100)?100:value;
	  obj->value[2] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
    {
	  value+=obj->value[3];
    value=(value < 0)?0:(value > 100)?100:value;
	  obj->value[3] = value;
	  return;
    }

  if ( !strcasecmp( arg2, "level" ) )
    {
	  value+=obj->level;
    value=(value < 0)?0:(value > LEVEL_IMMORTAL)?LEVEL_IMMORTAL:value;
	  obj->level = value;
	  return;
    }
	
  if ( !strcasecmp( arg2, "weight" ) )
    {
	  value+=obj->weight;
    value=(value < 0)?0:(value > 32000)?32000:value;
	  obj->weight = value;
	  return;
    }

  if ( !strcasecmp( arg2, "cost" ) )
    {
	  value+=obj->cost;
    value=(value < 0)?0:(value > 2000000000)?2000000000:value;
	  obj->cost = value;
	  return;
    }

  if ( !strcasecmp( arg2, "timer" ) )
    {
	  value+=obj->timer;
    value=(value < 0)?0:(value > 32000)?32000:value;
	  obj->timer = value;
	  return;
    }
	
  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      if( IS_NPC( ch ))
        {
      bug("Mob:%u bad parameters to 'mpoadd quest'",ch->pIndexData->vnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
        }
      return;
      }
    value += get_quest_bits( obj->obj_quest, firstBit, len);
    set_quest_bits( &obj->obj_quest, firstBit, len, value);
    }

  /*
   * Generate usage message.
   */
  return;
  }

bool is_room_good_for_teleport( CHAR_DATA *victim, int rvnum )
  {
  ROOM_INDEX_DATA *pRoomIndex;

  pRoomIndex = get_room_index( rvnum );
  if( pRoomIndex == NULL )
    return( FALSE );


	if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE) 
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP) 
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_RIP) 
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) 
        &&   pRoomIndex->area->average_level*2 < victim->level+15
        &&   !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT) 
        &&   !IS_SET(pRoomIndex->area->flags, AFLAG_NORECALL) 
        &&   pRoomIndex->sector_type != SECT_ETHEREAL 
        &&   pRoomIndex->sector_type != SECT_ASTRAL 
        &&   !( (pRoomIndex->area->low_hard_range!=0 ||
                 pRoomIndex->area->hi_hard_range!=0) &&
               ( victim->level < MAX_LEVEL) &&
                (victim->level<pRoomIndex->area->low_hard_range ||
                 victim->level>pRoomIndex->area->hi_hard_range) ) )
	    return( TRUE );

  return( FALSE );
  }

/*
 * mpgorand <startroom> <endroom> <offset> <skipsize>
 *   say, you have a 4x4 area from room #6400 to #6415 arranged like:
 *      6400 6401 6402 6403
 *      6404 6405 6406 6407
 *      6408 6409 6410 6411
 *      6412 6413 6414 6415
 *   then "mpgorand 6400 6415 0 4" would put the mobile in one of the
 *     the following rooms: 6400 6404 6408 6412
 *   then "mpgorand 6400 6415 3 4" would put the mobile in one of the
 *     the following rooms: 6403 6407 6411 6415
 */
void do_mpgorand( CHAR_DATA *ch, char *argument )
  {
  int startroom,endroom,offset,skipsize,rvnum,numroom, beginroom;
  bool is_gate;

    if ( !IS_NPC( ch )  && get_trust( ch ) < LEVEL_IMMORTAL )
    {
    send_to_char( "Huh?\n\r", ch );
	  return;
    }

  if(IS_AFFECTED(ch,AFF_CHARM))
    return;

  if(sscanf(argument," %d %d %d %d",&startroom,&endroom,&offset,&skipsize)!=4)
    {
      if( IS_NPC( ch ))
	  bug("Mpgorand - less than four parameters: vnum %u.",ch->pIndexData->vnum);
    bug(argument,0);
	  return;
    }

  if( !IS_NPC( ch ) && (endroom-startroom)>100 )
    is_gate = TRUE;
  else
    is_gate = FALSE;

  rvnum=startroom+offset;
  if(skipsize<=0 || rvnum>endroom || rvnum<startroom)
    {
      if( IS_NPC( ch ))
	  bug("Mpgorand - bad arguments: vnum %u.",ch->pIndexData->vnum);
    bug(argument,0);
	  return;
    }
  for(numroom=0;rvnum<=endroom;rvnum+=skipsize)
    {
    numroom++;
    }
  rvnum=startroom+offset+number_range(0,numroom-1)*skipsize;
  beginroom = rvnum;
    
 while ( (!is_gate && get_room_index(rvnum)==NULL) ||
         (is_gate  && !is_room_good_for_teleport( ch, rvnum ) ) )
    {
    rvnum++;
    if( rvnum == beginroom )  /* Yikes!  It started over */
      return;
    if( rvnum > endroom)
      rvnum = startroom;
    }

  transference( ch, get_room_index( rvnum ) );

  return;
  }

void transference( CHAR_DATA *victim, ROOM_INDEX_DATA *location )
    {
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    char_from_room( victim );
    char_to_room( victim, location );
    return;
    }
