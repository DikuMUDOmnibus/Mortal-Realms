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
#include <sys/socket.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "merc.h"


extern int sscanf();

/*
 * Local functions.
 */
ROOM_INDEX_DATA *       find_location   args( ( CHAR_DATA *ch, char *arg ) );
/*int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );*/

char *broken_bits( int , char * , bool );
char broken_string[255];
bool    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container, bool fDisplay ) );

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    int cmd;
    int col, level;
 
    col = 0;
    buf2[0]='\0';
    for( level=96; level <= ch->level; level++)
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( cmd_table[cmd].level == level /* This should be LEVEL_HERO */
	&&   cmd_table[cmd].level <= get_trust( ch ) )
	{
          sprintf( buf, "%d-%-12s", cmd_table[cmd].level, cmd_table[cmd].name );
	    strcat(buf2,buf);
	    if ( ++col % 5 == 0 )
		{
		strcat(buf2,"\n\r");
		send_to_char( buf2, ch );
		buf2[0]='\0';
		}
	}
    }
 
    if ( col % 5 != 0 )
	{
	strcat(buf2, "\n\r");
	send_to_char( buf2, ch );
	}
    return;
}



void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	STRFREE(ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	STRFREE(ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}



void do_arrest( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int  jailtime=0;
  int  multiplier=0;
  CHAR_DATA *victim;
  
  argument=one_argument(argument, arg1);
  argument=one_argument(argument, arg2);

  if(*arg1=='\0')
    {
      send_to_char("Usage: arrest <name> <number> <mins/hrs/days>\n\r", ch);
      return;
    }

  if((victim = get_char_world(ch, arg1)) == NULL)
    {
      send_to_char("They aren't logged in.\n\r", ch);
      return;
    }

  if(IS_NPC(victim))
    {
      send_to_char("You can only arrest players.\n\r", ch);
      return;
    }

  if(victim->level >= MAX_LEVEL-2 || which_god(victim) == GOD_POLICE)
    {
      send_to_char("You cannot arrest that person.\n\r", ch);
      return;
    }

  if(!IS_NPC(victim) && (victim->pcdata->arrested == TRUE))
    {
      send_to_char("They've already been arrested.\n\r", ch);
      return;
    }

  if(*arg2 == '\0')
    {
      send_to_char("You must specify a duration.\n\r", ch);
      return;
    }

  jailtime = atoi(arg2);
  if(jailtime < 0)
    jailtime = 0 - jailtime;
  if(jailtime == 0)
    {
      send_to_char("You must enter a duration greater than 0.\n\r", ch);
      return;
    }
  
  switch(*argument)
    {
    case 'm': case 'M':
      multiplier=60;
      break;

    case 'h': case 'H':
      multiplier=3600;
      break;

    case 'd': case 'D':
      multiplier=86400;
      break;

    default:
      send_to_char("You must specify 'hours', 'days' or 'minutes'.\n\r", ch);
      return;
      break;
    }

  if(multiplier * jailtime > 518400000 )
    {
      send_to_char("That sentence is too long.\n\r", ch);
      return;
    }

  if ( victim->fighting != NULL )
    stop_fighting( victim, TRUE );
  act( "$n has been arrested!", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, find_location(ch, "12001"));
  do_look( victim, "auto" );
  act( "$n has arrested you.", ch, NULL, victim, TO_VICT );

  victim->pcdata->arrested = TRUE;
  victim->pcdata->jailtime = jailtime * multiplier;
  victim->pcdata->jaildate = (int)current_time;

  send_to_char("Ok.\n\r", ch);
  return;
}


void do_release( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if((victim = get_char_world(ch, argument)) == NULL)
    {
      send_to_char("That character could not be found.\n\r", ch);
      return;
    }

  if(IS_NPC(victim))
    {
      send_to_char("Only players can be released.\n\r", ch);
      return;
    }

  if(victim->pcdata->arrested == FALSE)
    {
      send_to_char("That character is not in the dungeons.\n\r", ch);
      return;
    }

  act( "$n has been released.", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, find_location(ch, "9755"));
  act( "$n has been released.", victim, NULL, NULL, TO_ROOM );
  do_look( victim, "auto" );
  act( "$n has released you.", ch, NULL, victim, TO_VICT );

  victim->pcdata->arrested = FALSE;
  victim->pcdata->jailtime = 0;
  victim->pcdata->jaildate = 0;

  send_to_char("Ok.\n\r", ch);
  return;
}


void auto_release( CHAR_DATA *ch )
{
  act( "$n has been released.", ch, NULL, NULL, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, find_location(ch, "9755"));
  act( "$n has been released.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );

  ch->pcdata->arrested = FALSE;
  ch->pcdata->jailtime = 0;
  ch->pcdata->jaildate = 0;

  send_to_char("You have been released from the dungeons.\n\r", ch);
  return;
}


void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "arglebargle" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    int port;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

   if( (port=atol(arg))==0)
   {
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }
    for ( d = first_descriptor; d != NULL; d = d->next )
    {
	if ( is_desc_valid( d->character ) && d == victim->desc )
	{
	    close_socket( d , TRUE);
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor of that name not found!\n\r", ch );
    return;
   } 
   for( d=first_descriptor; d != NULL; d=d->next)
     if(d->descriptor == port)
       {
       close_socket( d , TRUE);
       send_to_char( "Ok.\n\r", ch);
       return;
       }
   bug( "Do_disconnect: desc not found.", 0 );
   send_to_char( "Descriptor of that number not found!\n\r", ch );
   return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|thief|outcast>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	else
	{
	    SET_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag added.\n\r", ch );
	    send_to_char( "You are now a KILLER!\n\r", victim );
	}
	return;
    }

    if ( !strcasecmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removed.\n\r", ch );
	    send_to_char( "You are no longer a THIEF.\n\r", victim );
	}
	else
	{
	    SET_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag added.\n\r", ch );
	    send_to_char( "You are now a THIEF!\n\r", victim );
	}
	return;
    }

    if ( !strcasecmp( arg2, "outcast" ) )
    {
	if ( IS_SET(victim->act, PLR_OUTCAST) )
	{
	    REMOVE_BIT( victim->act, PLR_OUTCAST );
	    send_to_char( "Outcast flag removed.\n\r", ch );
	    send_to_char( "You are no longer an OUTCAST.\n\r", victim );
	}
	else
	{
	    SET_BIT( victim->act, PLR_OUTCAST );
	    send_to_char( "Outcast flag added.\n\r", ch );
	    send_to_char( "You are now an OUTCAST!\n\r", victim );
	}
	
	return;
    }
    send_to_char( "Syntax: pardon <character> <killer|thief|outcast>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Echo what?\n\r", ch );
	return;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( is_desc_valid( d->character ) && 
           (d->connected == CON_PLAYING||d->connected==CON_EDITING) )
	{
	    strcpy(buf, argument);
	    strcat(buf, "\n\r");
	    send_to_char_color( buf,   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Recho what?\n\r", ch );
	return;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING  && is_desc_valid( d->character ) 
	&&   d->character->in_room == ch->in_room )
	{
	    strcpy( buf, argument );
	    send_to_char( buf,   d->character );
	}
    }

    return;
}



ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
        {
        int rn;
        rn = atol( arg );
        if( rn>64999 || rn<0)
          return NULL;
	return(get_room_index(rn));
        }

    if ( ( victim = get_player_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg1, "all" ) )
    {
	for ( d = first_descriptor; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   is_desc_valid( d->character ) && d->character != ch
	    &&   d->character->in_room != NULL
	    &&   d->character->in_room != ch->in_room
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

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
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

        if ( ch->level<LEVEL_IMMORTAL && room_is_private( location ) )
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_retran( CHAR_DATA *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        char buf[MAX_STRING_LENGTH];

        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
                send_to_char("Retransfer whom?\n\r", ch );
                return;
        }
    	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
    	}
        sprintf(buf, "'%s' %d", arg, victim->retran);
        do_transfer(ch, buf);
        return;
}


void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    if( !IS_NPC( ch ) &&  ch->level < 99 && ch->fighting != NULL )
      return;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }
    if( IS_NPC(ch) && location->vnum<3 )
      location=room_index[3];

    if ( ch->level<LEVEL_IMMORTAL && room_is_private( location ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

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

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    if( ch->level < 99 && which_god(ch)!=GOD_POLICE )
      location = ch->in_room;
    else
      location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s.'\n\rArea: '%s' Level %d  First room:%d.\n\r",
	location->name,
	location->area->name,
	location->area->average_level,
        location->area->low_r_vnum );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %u.  Sector: %s  Light: %d.\n\r",
	location->vnum,
	broken_bits(location->sector_type, "SECT_", TRUE),
	location->light );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %s.\n\rDescription:\n\r%s",
	flag_string(location->room_flags, r_flags),
	location->description );
    send_to_char( buf, ch );

    if ( location->first_extradesc != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	strcpy( buf, "Extra description keywords: '" );
	for ( ed = location->first_extradesc; ed; ed = ed->next )
	{
	    strcat( buf, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf, " " );
	}
	strcat( buf, "'.\n\r");
	send_to_char( buf, ch );
    }

    strcpy( buf, "Characters:");
    for ( rch = location->first_person; rch; rch = rch->next_in_room )
    {
      char wbuf[200];
        strcpy( wbuf, rch->name );
	strcat( buf, " " );
	one_argument( wbuf, buf2 );
	strcat( buf, buf2 );
    }
    strcat( buf, ".\n\r");
    send_to_char( buf, ch);

    strcpy( buf, ".\n\rObjects:   " );
    for ( obj = location->first_content; obj; obj = obj->next_content )
    {
      char wbuf[200];
         strcpy( wbuf, obj->name );
	strcat( buf, " " );
	one_argument( wbuf, buf2 );
	strcat( buf, buf2 );
    }
    strcat( buf, ".\n\r" );
    send_to_char( buf, ch);

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %u.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		pexit->to_room != NULL ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword != NULL ? pexit->keyword : "",
		pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

   sprintf( buf, "This room contains %d items.\n\r", location->content_count);
   send_to_char( buf, ch);

  if( location->room_file != NULL )
    {
    sprintf( buf, "External room file: %s\n\r", location->room_file );
    send_to_char( buf, ch );
    }

  if( location->fall_room != 0 )
    {
    sprintf( buf, "This room is a cliff falling to room %d, with slope %d, for %d feet.\n\r",
        location->fall_room, location->fall_slope, location->distance_of_fall );
    send_to_char( buf, ch);
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Ostat what?\n\r", ch );
	return;
    }

    if( ch->level < MAX_LEVEL-2 && which_god(ch)!=GOD_POLICE )
      {
      if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	      send_to_char( "You cannot ostat that.\n\r", ch );
	      return;
	}
      if( obj->pIndexData->area != ch->in_room->area )
	{
	      send_to_char( "You cannot ostat that.\n\r", ch );
	      return;
	}
      }
    else
      {

	  if(is_number(arg))
      {
      int anum;

      anum=atol(arg);
      for(obj=first_object;obj!=NULL;obj=obj->next)
	if(obj->pIndexData!=NULL&&obj->pIndexData->vnum==anum)
	  break;
      if(obj==NULL)
	{
	      send_to_char( "No object loaded has that vnum.\n\r", ch );
	      return;
	}
      }
    else
      {
      if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
	{
	      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	      return;
	}
      }
      }

    sprintf( buf, "Name: %s.   Total in game: %d/%d\n\r",
	obj->name , obj->pIndexData->total_objects, obj->pIndexData->max_objs);
    send_to_char( buf, ch );

    if( obj->test_obj )
      send_to_char( "This is a test game item.\n\r", ch);

    sprintf( buf, "Vnum: %u.  Type: %s.\n\r",
	obj->pIndexData->vnum, item_type_name( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s.\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    ch_printf( ch, "Wear flags : %s\n\r", flag_string(obj->wear_flags, w_flags) );
    ch_printf( ch, "Extra flags: %s\n\r", flag_string(obj->extra_flags, o_flags)
 );

    sprintf( buf, "Number: %d/%d.  Weight: %d/%d.\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf(buf,"Cost: %d.  Timer: %d.  Sac_timer: %d.  Level: %d(est:%d).\n\r",
	obj->cost, obj->timer, obj->sac_timer, obj->level,
        obj_level_estimate(obj->pIndexData ));
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %u.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d.\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
    send_to_char( buf, ch );

    if ( obj->first_extradesc != NULL || obj->pIndexData->first_extradesc != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	strcpy( buf, "Extra description keywords: '" );

	for ( ed = obj->first_extradesc; ed != NULL; ed = ed->next )
	{
	    strcat( buf, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf, " " );
	}

	for ( ed = obj->pIndexData->first_extradesc; ed != NULL; ed = ed->next )
	{
	    strcat( buf, ed->keyword );
	    if ( ed->next != NULL )
		strcat( buf, " " );
	}
	strcat( buf, "'.\n\r" );
	send_to_char( buf, ch);
    }

    for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );
	send_to_char( buf, ch );
    }

    for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );
	send_to_char( buf, ch );
    }

  if(obj->pIndexData->obj_fun!=0)
    send_to_char( "Object has spec fun.\n\r", ch );
  if(obj->pIndexData->obj_prog!=NULL)
    send_to_char( "Object has 'Object program' (TM).\n\r",ch);

  sprintf( buf, "Index Reference Keys:   %d    %d\n\r",
      obj->index_reference[0], obj->index_reference[1] );
  send_to_char( buf, ch );

	sprintf(buf,"Quest '%s' #%s\n\r", obj->pIndexData->area->name,
            quest_bits_to_string( obj->obj_quest) );
	send_to_char( buf, ch );

  {
  CHAR_DATA *owner;
  owner=get_pvnum_index( obj->owned_by);
  if( owner!=NULL)
    {
    sprintf( buf, "This object is owned by: %s\n\r", 
      IS_NPC(owner) ? owner->short_descr: owner->name);
    send_to_char( buf, ch);
    }
  else if( obj->owned_by!=0)
    {
    sprintf( buf, "This object is owned by: not in game #%d\n\r",obj->owned_by);
    send_to_char( buf, ch);
    }

  if( is_enchanted_obj( obj ))
    send_to_char( "This object has been enchanted.\n\r", ch);

  if( obj->basic )
    send_to_char( "This is a basic object.\n\r", ch);

  }

    /*Lookup the reset info  */
 if( obj->reset != NULL && obj->reset->obj == obj )
  {
  RESET_DATA *pReset;

  pReset = obj->reset;

  while( pReset != NULL )
      {
      sprintf( buf, "Reset Info:  '%c' %d %d %d %d\n\r", pReset->command,
                            pReset->arg0,
                            pReset->arg1,
                            pReset->arg2,
                            pReset->arg3 );
      send_to_char( buf, ch );
      pReset = pReset->container;
      }
  }

  return;
}

void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    OBJ_DATA *wield;
    sh_int numDice,sizeDice;
    AREA_DATA *pArea;

    one_argument( argument, arg );

    if ( argument[0]=='\0' || arg[0] == '\0' )
    {
	send_to_char( "Mstat whom?\n\r", ch );
	return;
    }

    if( ch->level < MAX_LEVEL-2 && which_god(ch)!=GOD_POLICE )
      {
      if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	      send_to_char( "You cannot mstat that.\n\r", ch );
	      return;
	}
      if( !IS_NPC(victim) || victim->pIndexData->area!=ch->in_room->area )
	{
	      send_to_char( "You cannot mstat that.\n\r", ch );
	      return;
	}

      }
    else
      {
	  if(is_number(arg))
      {
      int anum;

      anum=atol(arg);
      for(victim=first_char;victim!=NULL;victim=victim->next)
	if(victim->pIndexData!=NULL&&victim->pIndexData->vnum==anum)
	  break;
      if(victim==NULL)
	{
	      send_to_char( "No mobile loaded has that vnum.\n\r", ch );
	      return;
	}
      }
    else
      {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	      send_to_char( "They aren't here.\n\r", ch );
	      return;
	}
      }
      }

    sprintf( buf, "Name: %s.\n\r",
	victim->name );
    send_to_char( buf, ch );

    /*if( IS_NPC( victim ))
      memory_dump ( (char *) victim->npcdata ); */

    if( IS_NPC( victim ))
      sprintf( buf, "Vnum: %u.  Sex: %s.  Room: %u.  Race: %s.\n\r",
	victim->pIndexData->vnum ,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum,
        race_table[victim->race].race_name
	);
    else
      sprintf( buf, "Played: %d.  Sex: %s.  Room: %u.  Race: %s.\n\r",
	victim->played,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum,
        race_table[victim->race].race_name
	);
    send_to_char( buf, ch );

      sprintf( buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r",
	get_curr_str(victim),
	get_curr_int(victim),
	get_curr_wis(victim),
	get_curr_dex(victim),
	get_curr_con(victim) );
      send_to_char( buf, ch );
    if( !IS_NPC( victim))
      {
      sprintf( buf, " Mod Str: %2d.  Int: %2d.  Wis: %2d.  Dex: %2d.  Con: %2d.\n\r",
	 victim->pcdata->mod_str,
	 victim->pcdata->mod_int,
	 victim->pcdata->mod_wis,
	 victim->pcdata->mod_dex,
	 victim->pcdata->mod_con);
      send_to_char( buf, ch );
      }

    sprintf( buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Practices: %d.\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	victim->practice );
    send_to_char( buf, ch );
	
    sprintf( buf,
	    "Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
	    victim->level,   victim->class, victim->alignment, IS_NPC(victim) ?
      98-mob_armor(victim)+victim->npcdata->armor/3-victim->level*4
      : GET_AC(victim),      victim->gold,         victim->exp );
    send_to_char( buf, ch );

    /* dice indicator added by Dug 10/2/93 */
    wield = get_eq_char( victim, WEAR_WIELD );
    if((wield!=NULL)&&wield->item_type!=ITEM_WEAPON)
      wield=NULL;
    numDice=(wield!=NULL)?wield->value[1]:(IS_NPC(victim)?victim->npcdata->damnodice:1);
    sizeDice=(wield!=NULL)?wield->value[2]:(IS_NPC(victim)?victim->npcdata->damsizedice:4+victim->level/4);
    sprintf( buf, "Hitroll: %d.  Damroll: (%dd%d)+%d.  Position: %d.  Wimpy: %d.\n\r", GET_HITROLL(victim), numDice, sizeDice,(IS_NPC(victim)?victim->npcdata->damplus:0)+GET_DAMROLL(victim), victim->position, victim->wimpy);
    send_to_char( buf, ch );

    sprintf( buf, "Fighting: %s.\n\r",
	victim->fighting ?
        (victim->fighting->who->name ? victim->fighting->who->name: "(fighting unknown)")
         : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d.\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->saving_throw );
	send_to_char( buf, ch );
    }

    sprintf( buf, "Carry number: %d.  Carry weight: %d.\n\r",
	victim->carry_number, victim->carry_weight );
    send_to_char( buf, ch );

    sprintf( buf, "Age: %d.  Played: %d hours.  Timer: %d.\n\r",
	get_age( victim ), (int) victim->played/3600, victim->timer);
    send_to_char( buf, ch );

    if ( IS_NPC( victim ) )
     ch_printf( ch, "Act flags: %s\n\r", flag_string(victim->act, act_flags) );
    else
     ch_printf( ch, "Player flags: %s\n\r", flag_string(victim->act, plr_flags) );

    sprintf( buf, "Master: %s.  Leader: %s.  Affected by: %s.\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	affect_bit_name( victim->affected_by ) );
    send_to_char( buf, ch );

    /* added shot_timer and shot_aware entries -dug */
    if(IS_NPC(victim))
      {
     sprintf(buf,"Shot timer: %d.  Is %saware of shots.\n\rBasic Armor: %d  Current Armor: %d\n\r",
	victim->shot_timer,victim->shot_aware?"":"Not ",
	victim->npcdata->armor, mob_armor(victim) );
      send_to_char(buf,ch);
      }

    sprintf( buf, "Short description: %s.\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
	ch_printf( ch, "Mobile has spec fun: %s.\n\r", spec_name_lookup(victim->spec_fun));

    for ( paf = victim->first_affect; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector )
	    );
	send_to_char( buf, ch );
    }
   
  /* report quest stuff */
  if(!IS_NPC(victim))
    {
    unsigned long num;

    for(pArea=first_area;pArea!=NULL;pArea=pArea->next)
      {
      num=(pArea->low_r_vnum/100);
      if( is_quest(victim->pcdata->quest[num]) )
	{
	sprintf(buf,"%-40s %s\n\r",pArea->name,
          quest_bits_to_string( victim->pcdata->quest[num]));
	send_to_char( buf, ch );
	}
      }
    }
  else
    {
	sprintf(buf,"Quest '%s' #%s\n\r", victim->pIndexData->area->name,
          quest_bits_to_string( victim->npcdata->mob_quest));
	send_to_char( buf, ch );
    }

    if( IS_NPC(victim) && victim->pIndexData->corrected)
      send_to_char( "This mobile has been corrected by the loading procedure.\n\r", ch);

    if( !IS_NPC( victim ) && victim->pcdata->bet_mode > 0)
      {
      sprintf( buf, "Betting %s for %d.  status %d\n\r",
	  victim->pcdata->bet_victim->name, victim->pcdata->bet_amount,
	  victim->pcdata->bet_mode);
      send_to_char( buf, ch );
      }
    if (!IS_NPC(victim))
    {
     ch_printf(ch, "This person's pvnum is %d.\n\r", victim->pcdata->pvnum);
    }
    else if (victim->pIndexData->creator_pvnum != 0)
    {
     ch_printf(ch, "This person's was created by pvnum %d.\n\r", victim->pIndexData->creator_pvnum);
    }
    if ( IS_NPC(victim) )
      ch_printf( ch, "Hating  : %-13s   Hunting: %-13s   Fearing   : %s\n\r",
        victim->hating ? victim->hating->name : "(none)",
        victim->hunting ? victim->hunting->name : "(none)",
        victim->fearing ? victim->fearing->name : "(none)" );

    if( IS_NPC( victim ) && IS_SET( victim->pIndexData->act, ACT_BODY))
      {
      int cnt;
      strcpy(buf, "Body parts: ");
      for( cnt=0; cnt<MAX_BODY; cnt++)
	if( IS_SET( victim->pIndexData->body_parts, 1<<cnt))
	 {
	 strcat( buf, body_table[cnt].name);
	 strcat( buf, " ");
	 }
      strcat( buf, "\n\r");
      strcat(buf, "Attack parts: ");
      for( cnt=0; cnt<MAX_BODY; cnt++)
	if( IS_SET( victim->pIndexData->attack_parts, 1<<cnt))
	 {
	 strcat( buf, body_table[cnt].name);
	 strcat( buf, " ");
	 }
      strcat( buf, "\n\r");
      send_to_char( buf, ch );
      }
    if(!IS_NPC(victim) && victim->pcdata->castle!=NULL)
      {
      send_to_char("CASTLE: ",ch);
      sprintf(buf,"Entrance: %d Cost: $%d000\n\r NRm: %d NMb: %d NOb: %d\n\r",
              victim->pcdata->castle->entrance,
              victim->pcdata->castle->cost,
              victim->pcdata->castle->num_rooms,
              victim->pcdata->castle->num_mobiles,
              victim->pcdata->castle->num_objects);
      send_to_char(buf,ch);
      }

  if( victim->reset != NULL && victim->reset->mob == victim )
    {
    sprintf( buf, "Reset Info:  'M' %d %d %d %d\n\r", 
                            victim->reset->arg0,
                            victim->reset->arg1,
                            victim->reset->arg2,
                            victim->reset->arg3 );
    send_to_char( buf, ch );
    }

  if( IS_NPC(victim) && victim->npcdata->pvnum_last_hit_leader > 0)
    {
    if( victim->npcdata->pvnum_last_hit_leader == 1 )
      send_to_char( "This mobile temporarily cannot be fought.\n\r", ch);
    if( victim->npcdata->pvnum_last_hit_leader > 1 )
      {
      CHAR_DATA *ch_ld;
      ch_ld = get_pvnum_index( victim->npcdata->pvnum_last_hit );
      if( ch_ld == NULL )
        send_to_char( "This mobile was last fought by someone not in the game.\n\r", ch);
      else
        {
        sprintf( buf, "This mobile was last fought by %s.\n\r", ch_ld->name );
        send_to_char( buf, ch );
        }
      }
    }

	 
    return;
}



void do_mfind( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int nMatch;
    int hash;
    bool fAll;
    bool found;
/*
    int vnum;
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
*/
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mfind whom?\n\r", ch );
	return;
    }

    fAll        = !strcasecmp( arg, "all" );
    found       = FALSE;
    nMatch      = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
 
    Shuddup Furey you bad tempered bastard - no one cares - Martin 1/3/99
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pMobIndex->player_name )  ||
		is_name_short( arg, pMobIndex->player_name))
	    {
		found = TRUE;
		sprintf( buf, "[%5u](%3d) %s\n\r", pMobIndex->vnum, 
	pMobIndex->max_mobs, capitalize( pMobIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
              pMobIndex;
              pMobIndex = pMobIndex->next )
            if ( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
            {
                nMatch++;
                ch_printf( ch, "[%5d] %s\n\r",
                    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
            }

    if ( nMatch )
        ch_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int hash;
    int nMatch;
    bool fAll;
    bool found;

    /*
    int vnum;
    char buf[MAX_STRING_LENGTH];
    extern int top_obj_index;
    */
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Ofind what?\n\r", ch );
	return;
    }

    fAll        = !strcasecmp( arg, "all" );
    found       = FALSE;
    nMatch      = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     I said SHUDDUP!
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pObjIndex->name ) ||
		 is_name_short( arg, pObjIndex->name))
	    {
		found = TRUE;
		sprintf( buf, "[%5u](%3d/%3d) %s\n\r", pObjIndex->vnum, 
		   pObjIndex->total_objects, pObjIndex->max_objs,
	 capitalize( pObjIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash];
              pObjIndex;
              pObjIndex = pObjIndex->next )
            if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
            {
                nMatch++;
                ch_printf( ch, "[%5d] %s\n\r",
                    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
            }


    if ( nMatch )
        ch_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
    return;
}




void do_owhere( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int count,item_vnum;
  int total, total_room, total_npc, total_pc, total_cont, total_progs;
  int total_pc_cont, total_npc_cont, total_null;

  if( *argument == '\0')
    {
    total = 0;
    total_room = 0;
    total_npc = 0;
    total_pc = 0;
    total_cont = 0;
    total_pc_cont = 0;
    total_npc_cont = 0;
    total_progs = 0;
    total_null = 0;
    for ( obj = first_object; obj != NULL; obj = obj->next )
      {
      total++;
      if( obj->in_obj != NULL )
        {
        if( obj->carried_by != NULL )
        {
          if( IS_NPC(obj->carried_by) )
            total_npc_cont++;
          else
            total_pc_cont++;
        }
        else
          total_cont++;
        }
      else
        if( obj->in_room != NULL )
          total_room++;
      else
        {
        if( obj->carried_by != NULL )
          {
          if( !IS_NPC(obj->carried_by) )
            {
            total_pc++;
            if( obj->pIndexData->obj_prog != NULL )
              total_progs++;
            }
          else
            total_npc++;
          }
        else
          total_null++;
        }
     }

   sprintf( buf, "Totalized list of objects\n\r" );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects: %d\n\r", total );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects carried by PCs: %d\n\r", total_pc );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects carried by NPCs: %d\n\r", total_npc );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects in containers in room: %d\n\r", total_cont );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects in containers on PCs: %d\n\r", total_pc_cont );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects in containers on NPCs: %d\n\r",total_npc_cont );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects in rooms: %d\n\r", total_room );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects with active obj_progs: %d\n\r", total_progs );
   send_to_char( buf, ch );
   sprintf( buf, "Total objects in NULL: %d\n\r", total_null );
   send_to_char( buf, ch );

   return;
   }

  found = FALSE;
  count= 0;

  item_vnum=(is_number(argument))?atol(argument):-1;

  for ( obj = first_object; count<100 && obj != NULL; obj = obj->next )
    {
	  if ( item_vnum!=obj->pIndexData->vnum &&
	 !is_name_short( argument, obj->name ))
	    continue;

	  found = TRUE;
    count++;

	  for ( in_obj = obj; in_obj->in_obj != NULL && in_obj->in_obj!=in_obj; in_obj = in_obj->in_obj )
	    ;

	  if ( in_obj->carried_by != NULL )
	    {
	    sprintf(buf, "[%5u]%s ([%5u]%s)held by %s\n\r",
			  obj->pIndexData->vnum,str_resize(obj->short_descr,buf1,-20),
			  in_obj->carried_by->in_room->vnum,
	      str_resize(in_obj->carried_by->in_room->name,buf2,-20),
	      str_resize(PERS(in_obj->carried_by, ch),buf3,-14));
	    }
	  else
	    {
	    sprintf(buf, "[%5u]%s ([%5u]%s)in %s\n\r", obj->pIndexData->vnum,
			  str_resize(obj->short_descr,buf1,-20),
	      (in_obj->in_room==NULL)?-1:in_obj->in_room->vnum,
	      (in_obj->in_room==NULL)?"NULL":
				   str_resize(in_obj->in_room->name,buf2,-20),
			  str_resize(in_obj == obj? "nothing":
	      (in_obj==NULL? "somewhere":in_obj->name),buf3,-19));
	    }

	  buf[0] = UPPER(buf[0]);
	  send_to_char( buf, ch );
    }

  if ( !found )
	  send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

  return;
  }

void do_mwhere( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  bool found;
  int count,mob_vnum;

  count=0;
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
	  send_to_char( "Mwhere whom?\n\r", ch );
	  return;
    }

  mob_vnum=(is_number(arg))?atol(arg):-1;

  found = FALSE;
  for ( victim = first_char; count<100 && victim != NULL; victim = victim->next )
    {
	  if ( IS_NPC(victim)
	      && victim->in_room != NULL
	&& victim->pIndexData->vnum != mob_vnum
	      && (is_name( arg, victim->name ) || is_name_short( arg, victim->name)))
	    {
	    found = TRUE;
      count++;
	    sprintf(buf, "[%5u] %s [%5u] %s\n\r", victim->pIndexData->vnum,
			  str_resize(victim->short_descr,buf1,-40),
	      (victim->in_room==NULL)?-1:victim->in_room->vnum,
			  (victim->in_room==NULL)?"NULL":
				    str_resize(victim->in_room->name,buf2,-20));
	    send_to_char( buf, ch );
	    }
    }

  if ( !found )
	  act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

  return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    CHAR_DATA *fch;

    if( argument[0]=='S' || argument[0]=='s')
      {
      sprintf( buf, "Reboot scheduled after everyone is off the Realm.\n\rCompliments of %s\n\r", ch->name);
      do_echo( ch, buf );
      merc_reboot = TRUE;
      return;
      }
    save_all_clans();
    save_victors();
    do_savearea(NULL,"forreal");
    if( argument[0]=='N' || argument[0]=='n')
      {
      CHAR_DATA *next;
      for(fch=first_char; fch!=NULL; fch=next)
	{
	next=fch->next;
	if(!IS_NPC(fch) && fch!=ch)
          {
          send_to_char( "The game is now rebooting.  Try again in 5 minutes.\n\r", fch );
          if( fch->level == 1 && is_desc_valid( fch ) )
            advance_level( fch, TRUE );
	    do_quit(fch, "arglebargle");
          }
	}
      sprintf( buf, "Reboot by %s.", ch->name );
      do_echo( ch, buf );
      do_quit(ch, "arglebargle");
      merc_down = TRUE;
      IS_BOOTING=TRUE;
      return;
      }
  send_to_char( "You must specify 'now' or 'soon'.\n\r", ch);
  return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    extern bool merc_down;
    CHAR_DATA *fch, *next;
    FILE *fp;

    if(argument[0]!='n' && argument[0]!='N')
      {
      send_to_char( "You must specify 'now' after the command.\n\r", ch);
      return;
      }
    save_all_clans(); 
    save_victors();
    do_savearea(NULL,"forreal");
    for(fch=first_char; fch!=NULL; fch=next)
      {
      next=fch->next;
      if(!IS_NPC(fch) && fch!=ch)
          {
          send_to_char( "The game is now shut down.  Try again in 15 minutes.\n\r", fch );
          if( fch->level == 1 && is_desc_valid( fch ) )
            advance_level( fch, TRUE );
	  do_quit(fch, "arglebargle");
          }
      }
    fclose( fpReserve );
    fclose( fpAppend );
    fp = fopen( SHUTDOWN_FILE, "w" );
    fprintf( fp, "Shutdown by %s.", ch->name );
    fclose( fp );
    do_quit(ch,"arglebargle");
    merc_down = TRUE;
    IS_BOOTING=TRUE;
    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
      victim = ch;     /* Default to yourself  */
   else
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = first_descriptor; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by == ch->desc )
      {
      act( "Cancelling snoop of $N.", ch, NULL, victim, TO_CHAR );
      victim->desc->snoop_by = NULL;
      return;
      }


    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) == MAX_LEVEL && REAL_GAME )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    /*  send_to_char( "This is temporarily offline,\n\r", ch );
    return;   */

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if( !IS_NPC( victim ) )
      {
	send_to_char( "You may not switch into a player's character.\n\r", ch );
	return;
      }
   

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->pcdata->switched = TRUE;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *owner;
    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
      {
      if( argument!= NULL)   /* check for forced */
	send_to_char( "You are already yourself.\n\r", ch );
	return;
      }
   if( argument!= NULL)   /* check for forced */
      send_to_char( "You return to your original body.\n\r", ch );
    owner                     = ch->desc->original;
    ch->desc->character       = owner;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    /* handler homonculous made dead */
    if(IS_NPC(ch) && ch->pIndexData->vnum==9900 && argument!=NULL)
      SET_BIT( ch->act, ACT_WILL_DIE);
    if(IS_NPC(ch) && IS_AFFECTED(ch, AFF2_POSSESS))
     {
      REMOVE_BIT(ch->affected2_by, AFF2_POSSESS);
      SET_BIT( ch->act, ACT_WILL_DIE);
     }
    return;
}



void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: mload <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atol( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
 
    argument = one_argument( argument, arg1 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char( "Syntax: oload <vnum>\n\r", ch );
	return;
    }
    
    if (ch->level<MAX_LEVEL && atol(arg1) < 100 && atol(arg1)!=52 ) 
    {
 	send_to_char( "Sorry, you can't load god chains!\n\r", ch);
        return;
    }

    if ( ( pObjIndex = get_obj_index( atol( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, pObjIndex->level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_room( obj, ch->in_room );
	act( "You create $p!", ch, obj, NULL, TO_CHAR );
        get_obj( ch, obj, NULL, FALSE );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
	act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int room, room_start;
    bool area, quite;

    if( argument !=NULL)
      {
      quite=FALSE;
      one_argument( argument, arg );
      }
    else
      {
      quite=TRUE;
      strcpy( arg, "");
      }
    room= ch->in_room->vnum;
    area = !strcasecmp( arg, "area" );
    if( area )
      {
      arg[0]='\0';
      room_start = ch->in_room->vnum;
      for( room = ch->in_room->area->low_r_vnum;
	 get_room_index( room ) != NULL &&
	 room_index[ room ]->area == room_index[ room_start ]->area; room++)
	{
	char_from_room( ch );
	char_to_room( ch, room_index[ room ]);
	do_purge( ch, NULL);
	}
      char_from_room( ch );
      char_to_room( ch, room_index[ room_start] );
      send_to_char( "You have purge the Area.\n\r", ch);
      return;
      }
  
    if ( arg[0] == '\0'  || arg==NULL)
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->first_person; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && victim!=ch)
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->first_content; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
            if( obj->item_type != ITEM_CORPSE_PC )
	      extract_obj( obj );
	}
	if( !quite)
	  {
	  act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	  send_to_char( "Ok.\n\r", ch );
	  }
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atol( arg2 ) ) < 1 || level > MAX_LEVEL  )
    {
	send_to_char( "Level must be 1 to 99.\n\r", ch );
	return;
    }

   /*if (!TEST_GAME)  level = 2;  Our system does not allow this to be used */

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
	int sn;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	victim->level    = 1;
	victim->exp      = 0;
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	    victim->pcdata->learned[sn] = 0;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	victim->mclass[victim->class]=1;
	advance_level( victim, FALSE );
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
    if( iLevel%10==0)
      add_language(victim);
    ch->mclass_switched=0;
	send_to_char( "You raise a level!!  ", victim );
	victim->level += 1;
	victim->mclass[victim->class] +=1;
	advance_level( victim, FALSE );
    }

    sub_player (victim);
    add_player (victim);
    save_char_obj (victim, NORMAL_SAVE);
    save_char_obj (victim, BACKUP_SAVE);

    victim->exp   = exp_level(victim->class,victim->level-1)+1;
    victim->trust = 0;
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atol( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 99.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restore whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( IS_IMMORTAL(victim) )
    {
	send_to_char( "Not on Immortals's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj(victim, NORMAL_SAVE);

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
	REMOVE_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}



void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Silence whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	REMOVE_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can use channels again.\n\r", victim );
	send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can't use channels!\n\r", victim );
	send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );

    }
    
    if(!IS_NPC(ch))
    {
      act( "$n booms 'PEACE!'", ch, NULL, NULL, TO_ROOM );
      send_to_char( "Ok.\n\r", ch );
    }
 	 
    return;
}


   /* Facade added 3/25/1999 */

void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    if ( IS_NPC(ch) )
	return;

   /* 
    * return;
    */

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    strcat( buf, "\n\r" );
	}
	send_to_char( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !strcasecmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }
    CREATE( pban, BAN_DATA, 1);
    pban->name  = str_dup( arg );
    pban->next  = ban_list;
    ban_list    = pban;
    save_sites();
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !strcasecmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    STRFREE (curr->name );
	    DISPOSE(curr);
	    save_sites();
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game wizlocked.\n\r", ch );
    else
	send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}



void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slookup what?\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}



void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",      ch );
	send_to_char( "or:     sset <victim> all     <value>\n\r",      ch );
	send_to_char( "Skill being any skill or spell.\n\r",            ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !strcasecmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atol( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]     = value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}





typedef struct	user_list_data		USER_LIST_DATA;
struct	user_list_data
{
    USER_LIST_DATA *	next;
    DESCRIPTOR_DATA *	desc;
};


  /* This routine returns which host is greatest by IP numbers for sorting */
  /*    Chaos - 3/19/96   */
int hostcmp( DESCRIPTOR_DATA *d1, DESCRIPTOR_DATA *d2 )
  {
  int cnt;

  if( d1->host_index==NULL || d2->host_index==NULL )
    return( 0 );

  for( cnt=0; cnt<4; cnt++)
    {
    if( d1->host_index->address[cnt] < d2->host_index->address[cnt] )
      return( -1 );
    if( d1->host_index->address[cnt] > d2->host_index->address[cnt] )
      return( 1 );
    }

  return( 0 );
  }


void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    USER_LIST_DATA *user_list, *new_uld;
    USER_LIST_DATA *uld, *prev_uld, *next_uld;
    int str_cmpd, val;
    bool mp;


    /* The complete descriptor list */
    if( *argument != '\0' )
      {
      for( count=0; count<MAXLINKPERPORT+20; count++)
        {

        sprintf( buf, "%3d ", count );

        if ( getsockopt( count, SOL_SOCKET, SO_SNDBUF,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "SN NA    ");
        else
          sprintf( buf2, "SN %-5d ", val );
        strcat( buf, buf2 );
 
        if ( getsockopt( count, SOL_SOCKET, SO_RCVBUF,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "RC NA   ");
        else
          sprintf( buf2, "RC %-4d ", val );
        strcat( buf, buf2 );
 
        if ( getsockopt( count, SOL_SOCKET, SO_REUSEADDR,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "RU N ");
        else
          sprintf( buf2, "RU %-1d ", val );
        strcat( buf, buf2 );

 /*
        if ( getsockopt( count, SOL_SOCKET, SO_DEBUG,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "DE N ");
        else
          sprintf( buf2, "DE %-1d ", val );
        strcat( buf, buf2 );
*/
 
        if ( getsockopt( count, SOL_SOCKET, SO_KEEPALIVE,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "KE N ");
        else
          sprintf( buf2, "KE %-1d ", val );
        strcat( buf, buf2 );

        if ( getsockopt( count, SOL_SOCKET, SO_LINGER,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "LG N ");
        else
          sprintf( buf2, "LG %-1d ", val );
        strcat( buf, buf2 );
 
        if ( getsockopt( count, SOL_SOCKET, SO_TYPE,
            (char *) &val, (int *) &val ) < 0 )
          sprintf( buf2, "TY N ");
        else
          sprintf( buf2, "TY %-1d ", val );
        strcat( buf, buf2 );

        sprintf( buf2, "OWN %-2d FLG %-5x ",
            fcntl( count, F_GETOWN ),
            fcntl( count, F_GETFL ) >= 0 ? fcntl(count, F_GETFL) : 0 );
        strcat( buf, buf2 );

    strcpy( buf2 , "--- No Name" );
    for ( d = first_descriptor; d != NULL; d = d->next )
      if ( is_desc_valid( d->character ) && d->character != NULL )
        if( d->descriptor == count )
          sprintf( buf2, "%-3d %s", d->connected, d->character->name );
    strcat( buf, buf2 );
        

        strcat( buf, "\n\r");
        send_to_char( buf, ch );
        }
      return;
      }



    /* Rewritten to test for multiplaying (sorting) - Chaos  3/14/96  */

    count       = 0;
    buf[0]      = '\0';
    user_list = NULL;

    for ( d = first_descriptor; d != NULL; d = d->next )
      {
      if ( is_desc_valid( d->character ) && d->character != NULL )
        {
        count++;
        if( user_list == NULL )
          {
          CREATE( user_list, USER_LIST_DATA, 1);
          user_list->desc = d;
          user_list->next = NULL;
          }
        else
          {
          prev_uld = NULL;
          for( uld=user_list; uld!=NULL; uld=next_uld)
            {
            next_uld = uld->next;
            str_cmpd = hostcmp( uld->desc, d );
            if( str_cmpd < 0 )
              {
              prev_uld = uld;
              continue;
              }
            if( str_cmpd == 0 && uld->desc->remote_port < d->remote_port )
              {
              prev_uld = uld;
              continue;
              }
            break;
            }
          CREATE(new_uld, USER_LIST_DATA, 1);
          new_uld->next = uld;
          new_uld->desc = d;
          if( prev_uld == NULL )
            user_list = new_uld;
          else
            prev_uld->next = new_uld;

          }
        }
      }

    prev_uld = NULL;
    for ( uld = user_list; uld != NULL; uld = next_uld )
      {
      next_uld = uld->next;

      /* Multiplaying?    */
      mp = FALSE;
      if( prev_uld != NULL && 
          !strcasecmp( prev_uld->desc->host, uld->desc->host ) &&
          uld->desc->remote_port - prev_uld->desc->remote_port < 5 )
        mp = TRUE;

	    sprintf( buf + strlen(buf), "[%3d %2d] %15s @ %38s (%5d) %s\n\r",
		uld->desc->descriptor,
		uld->desc->connected,
		uld->desc->original  ? uld->desc->original->name  :
		uld->desc->character ? uld->desc->character->name : "(none)",
		uld->desc->host,
                uld->desc->remote_port,
                mp ? "MP" : "" 
		);
      prev_uld = uld;
      }
    for ( uld = user_list; uld != NULL; uld = next_uld )
      {
      next_uld = uld->next;
      DISPOSE(uld);
      }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    send_to_char( buf2, ch );
    send_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
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
     if(vch!=ch)
	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
		interpret( vch, buf );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( IS_NPC(victim ) && ch->level<MAX_LEVEL && REAL_GAME )
	{
	    send_to_char( "You can't force a mobile.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim ) && victim->level  >= ch->level )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	act( "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
	interpret( victim, buf );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

    if ( IS_NPC(ch) || ch->level<MAX_LEVEL-2 )
	return;

    if (ch->level < MAX_LEVEL-1 || *argument == '\0')
       victim=ch;

    else if( ( victim = get_player_world( ch, argument ) ) == NULL )
      victim = ch;

    if ( IS_SET(victim->act, PLR_WIZINVIS) )
    {
	REMOVE_BIT(victim->act, PLR_WIZINVIS);
	act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade back into existence.\n\r", victim);
    }
    else
    {
	SET_BIT(victim->act, PLR_WIZINVIS);
	act( "$n slowly fades into thin air.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly vanish into thin air.\n\r", victim );
    }

    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_hearlog( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->pcdata->player2_bits, PLR2_HEARLOG) )
    {
	REMOVE_BIT(ch->pcdata->player2_bits, PLR2_HEARLOG);
	send_to_char( "No longer listening to log file.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->pcdata->player2_bits, PLR2_HEARLOG);
	send_to_char( "You are listening to the log file.\n\r", ch );
    }

    return;
}
void do_mlist( CHAR_DATA *ch, char *argument )
  {
  extern int top_mob_index;
  char buf[MAX_STRING_LENGTH],t1[81];
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  AREA_DATA *pArea;
  int vnum,anum;
  int nMatch;
  bool fAll;
  FILE *mobFile;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
	  send_to_char("Mlist what area's mobiles? (use the number in 'areas')\n\r",
		 ch);
	  return;
    }

  fAll  = !strcasecmp( arg, "all" );
  pArea=NULL;
  anum=0;
	if(is_number(arg))
	  anum = atol( argument );
	else if(!fAll)
    {
	  send_to_char("Mlist what area's mobiles? (use the number in 'areas')\n\r",
		 ch);
	  return;
    }
  mobFile=NULL;
  if(!fAll)
    {
    for(pArea=first_area;pArea!=NULL;pArea=pArea->next)
      if(pArea->low_r_vnum==anum)
	break;
    if(pArea==NULL)
      {
	    send_to_char( "No area like that in hell, earth, or heaven.\n\r", ch );
	    return;
      }
    }
  else
    {
    fclose( fpReserve );
    mobFile=fopen("mlist.all","wt" );
    }
    
  nMatch        = 0;

  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	  if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	    {
	    nMatch++;
	    if( fAll || pMobIndex->area==pArea)
	      {
/* old wide method -Order 4/2/94
		    sprintf(buf,"[%s%5u](%3d) %-20s,%02d,%03dd%03d+%05d,%03dd%03d+%03d,$%06d\n\r",
                (pMobIndex->picture==NULL)?"":"P",
		pMobIndex->vnum, 
		pMobIndex->max_mobs,
		str_resize(capitalize(pMobIndex->player_name),t1,-20),
		pMobIndex->level,
		pMobIndex->hitnodice,pMobIndex->hitsizedice,pMobIndex->hitplus,
		pMobIndex->damnodice,pMobIndex->damsizedice,pMobIndex->damplus,
		pMobIndex->gold);
*/
		    sprintf(buf,"[%s%5u](%3d) %-20s,%02d,%05dAvgHp,%05dAvgDam,$%06d\n\r",
                (pMobIndex->picture==NULL)?"":"P",
		pMobIndex->vnum, 
		pMobIndex->max_mobs,
		str_resize(capitalize(pMobIndex->player_name),t1,-20),
		pMobIndex->level,
		(int)(pMobIndex->hitnodice*(1+(pMobIndex->hitsizedice-1)/2.0)
		 +pMobIndex->hitplus),
		(int)(pMobIndex->damnodice*(1+(pMobIndex->damsizedice-1)/2.0)
		 +pMobIndex->damplus),
		pMobIndex->gold);
	if(!fAll)
		      send_to_char( buf, ch );
	else
	  fprintf(mobFile,buf);
	      }
	    }
    }
  if(fAll)
    {
    fclose(mobFile);
    send_to_char("Written to file: mlist.all\n\r",ch);
    fpReserve = fopen( NULL_FILE, "r" );
    }

  return;
  }

void do_olist( CHAR_DATA *ch, char *argument )
{
  extern int top_obj_index;
  char buf[MAX_STRING_LENGTH],t1[81],buf2[MAX_INPUT_LENGTH],t2[81];
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *pArea;
  int vnum,anum;
  int nMatch;
  bool fAll;
  AFFECT_DATA *paf;
  FILE *objFile;
  bool fSort;
  int level, lRange, hRange;
  extern char * const attack_table[];
  AREA_DATA *lArea;

  lArea = NULL;
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
	  send_to_char("Olist what area's objects? (use the number in 'areas')\n\r",
		 ch);
    if( ch->level > 95 )
      send_to_char( "Options:  'ALL' saves file.  'SORT' saves sorted by level file.\n\r", ch );
	  return;
    }

  if( ch->level > 95 )
    {
    fAll  = !strcasecmp( arg, "all" );
    fSort  = !strcasecmp( arg, "sort" );
    }
  else
    {
    fAll = FALSE;
    fSort = FALSE;
    }

  pArea=NULL;
  anum=0;
	if(is_number(arg))
	  anum = atol( argument );
	else if(!fAll&&!fSort)
    {
	  send_to_char("Olist what area's objects? (use the number in 'areas')\n\r",
		 ch);
	  return;
    }

  objFile=NULL;
  if(!fAll && !fSort)
    {
    for(pArea=first_area;pArea!=NULL;pArea=pArea->next)
      if(pArea->low_r_vnum==anum)
	break;
    if(pArea==NULL)
      {
	    send_to_char( "No area like that in hell, earth, or heaven.\n\r", ch );
	    return;
      }
    }
  else
    {
    fclose( fpReserve );
    objFile=fopen("olist.all","wt" );
    }
    
  nMatch        = 0;

  if( !fSort )
    {
    lRange=0;
    hRange=1;
    }
  else
    {
    lRange = 0;
    hRange = 115;
    }

for(level=lRange; level<hRange; level++)
   for ( vnum = 0,nMatch=0; nMatch < top_obj_index; vnum++ )
    {
	  if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	    {
	    nMatch++;


	    if((fSort&&level==pObjIndex->level) || fAll ||
                 pObjIndex->area==pArea)
	      {

    /* Add spot for area name  - Chaos 1/17/96  */
	if(fAll || fSort)
         if( lArea != pObjIndex->area )
          {
          lArea = pObjIndex->area;
	  fprintf(objFile,"-------  AREA: %d %s  -------\n", lArea->low_r_vnum,
                 lArea->name );
	  }


	strcpy(buf2,"");
	switch ( pObjIndex->item_type )
	  {
	  case ITEM_LIGHT:
	    sprintf(buf2, "LIGHT :%d",pObjIndex->value[0]);
	    break;
	  case ITEM_CONTAINER:
	    sprintf(buf2, "CONTNR:%d lb:%s%s%s%s",pObjIndex->value[0],
		    ((pObjIndex->value[1]&CONT_CLOSEABLE)!=0)?"CLOSE":"",
		    ((pObjIndex->value[1]&CONT_PICKPROOF)!=0)?",NOPICK":"",
		    ((pObjIndex->value[1]&CONT_CLOSED)!=0)?",CLOSED":"",
		    ((pObjIndex->value[1]&CONT_LOCKED)!=0)?",LOCKED":""
		    );
	    break;
	  case ITEM_DRINK_CON:
	    sprintf(buf2, "DRCONT:%d/%d,%s%s",pObjIndex->value[1],
		    pObjIndex->value[0],
		    liq_table[pObjIndex->value[2]].liq_name,
		    (pObjIndex->value[3]!=0)?",POISONED":"");
	    break;
	  case ITEM_KEY:
	    sprintf(buf2, "KEY   :%d",pObjIndex->value[0]);
	    break;
	  case ITEM_FOOD:
	    sprintf(buf2, "FOOD  :%d hrs%s",pObjIndex->value[0],
		    (pObjIndex->value[3]!=0)?",POISONED":"");
	    break;
	  case ITEM_MONEY:
	    sprintf(buf2, "GOLD  :$%d",pObjIndex->value[0]);
	    break;
	  case ITEM_SCROLL: 
	  case ITEM_POTION:
	  case ITEM_PILL:
	    if(pObjIndex->item_type==ITEM_SCROLL)
		    sprintf( buf2, "SCROLL:%d:", pObjIndex->value[0] );
	    else if(pObjIndex->item_type==ITEM_POTION)
		    sprintf( buf2, "POTION:%d:", pObjIndex->value[0] );
	    else
		    sprintf( buf2, "PILL  :%d:", pObjIndex->value[0] );

		  if ( pObjIndex->value[1] >= 0 && pObjIndex->value[1] < MAX_SKILL )
		    {
		    strcat(buf2, 
		     str_resize(skill_table[pObjIndex->value[1]].name,t1,-6));
		    }
  
		  if ( pObjIndex->value[2] >= 0 && pObjIndex->value[2] < MAX_SKILL )
		    {
		    strcat(buf2, ",");
		    strcat(buf2, 
		     str_resize(skill_table[pObjIndex->value[2]].name,t1,-6));
		    }
  
		  if ( pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL )
		    {
		    strcat(buf2, ",");
		    strcat(buf2, 
		     str_resize(skill_table[pObjIndex->value[3]].name,t1,-6));
		    }

		break;

	  case ITEM_WAND: 
	  case ITEM_STAFF: 
	    if(pObjIndex->item_type==ITEM_WAND)
		    sprintf( buf2, "WAND  :%d(%d/%d)", 
			     pObjIndex->value[0], pObjIndex->value[2],
		       pObjIndex->value[1] );
	    else
		    sprintf( buf2, "STAFF :%d(%d/%d)",
			     pObjIndex->value[0], pObjIndex->value[2],
		       pObjIndex->value[1] );
      
		  if ( pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL )
		    {
		    strcat(buf2, 
		     str_resize(skill_table[pObjIndex->value[3]].name,t1,-6));
		    }
		  break;
      
	  case ITEM_WEAPON:
	    if(pObjIndex->value[0]!=0)
	      {
		    sprintf(buf2, "RANGE :Hand %dd%d(%03d) ammoTyp %d",
			   pObjIndex->value[1], pObjIndex->value[2],
		     (int)(pObjIndex->value[1]*(1+(pObjIndex->value[2]-1)/2.0)),
		     pObjIndex->value[0]);
	      }
	    else
		    sprintf(buf2,"WEAPON:%dd%d(%03d),%s", pObjIndex->value[1],
		    pObjIndex->value[2],
		    (int)(pObjIndex->value[1]*(1+(pObjIndex->value[2]-1)/2.0)),
		    attack_table[pObjIndex->value[3]]);
		  break;

	  case ITEM_ARMOR:
		  sprintf( buf2, "ARMOR :%02d", pObjIndex->value[0] );
		  break;

	  case ITEM_AMMO:
	    sprintf( buf2,"AMMO  :typ%02d dam%02d spd%02d rng%02d",
	      pObjIndex->value[0],pObjIndex->value[1],pObjIndex->value[2],
	      pObjIndex->value[3]);
		  break;

	  case ITEM_TREASURE:
	    sprintf(buf2,"TREASR:");
	    break;

	  case ITEM_FURNITURE:
	    sprintf(buf2,"FURNIT:");
	    break;

	  case ITEM_TRASH:
	    sprintf(buf2,"TRASH :");
	    break;

	  case ITEM_BOAT:
	    sprintf(buf2,"BOAT  :");
	    break;

	  case ITEM_CORPSE_NPC:
	    sprintf(buf2,"CORPSN:");
	    break;

	  case ITEM_CORPSE_PC:
	    sprintf(buf2,"CORPSP:");
	    break;

	  case ITEM_FOUNTAIN:
	    sprintf(buf2,"FOUNTN:");
	    break;
	  }
	for ( paf = pObjIndex->first_affect; paf != NULL; paf = paf->next )
	  {
		if ( paf->location != APPLY_NONE && paf->modifier != 0 )
		  {
		  sprintf(t2, ":%d%s",paf->modifier,
				str_resize(affect_loc_name(paf->location),t1,-6));
	    strcat(buf2,t2);
	    t1[0]='\0';
		  }
	  }
		    sprintf(buf,"[%s%5u]%-20s,$%d,#%d,L%d/%d%s[%s]%s\n",
                (pObjIndex->picture==NULL)?"":"P",
		pObjIndex->vnum, 
		capitalize( str_resize(pObjIndex->name,t1,-20) ),
		pObjIndex->cost,
		pObjIndex->weight,
		pObjIndex->level,
                obj_level_estimate(pObjIndex),
                (pObjIndex->level<8*obj_level_estimate(pObjIndex)/10)?
                  "***":"",
		buf2,extra_bit_name(pObjIndex->extra_flags));
	if(!fAll&&!fSort)
             {
             strcat( buf, "\r" );
		      send_to_char( buf, ch );
             }
	else
	  {
	  char dummy[4];

	  strcpy(dummy,"%s" ); /* to handle objects with %s in their names */

	  fprintf(objFile,buf,dummy,dummy,dummy);
	  }
	      }
	    }
    }
  if(fAll||fSort)
    {
    fclose(objFile);
    send_to_char("Written to file: olist.all\n\r",ch);
    fpReserve = fopen( NULL_FILE, "r" );
    }

  return;
}



void do_maze( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    CHAR_DATA *fch;

     if(IS_NPC(ch) && ch->pIndexData->vnum==9900)
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    strcpy( arg4, argument );

    if( arg1[0]=='\0')
      {
      send_to_char( "Format: MAZE <X size> <Y size> <Z size> <Charcter Name>\n\r", ch);
      return;
      }
    fch=lookup_char( arg4);
    if( fch==NULL || IS_NPC( fch))
      return;
    mazegen( ch, ch->in_room->vnum, atol( arg1), atol( arg2 ), atol( arg3),
	 fch->pcdata->pvnum);
    if( !IS_NPC( ch))
      send_to_char( "Maze generated.\n\r", ch);
    return;
}

void do_connect( CHAR_DATA *ch, char *argument )
{
    int dest_vnum;
    int door_num;

    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];

     if(IS_NPC(ch) && ch->pIndexData->vnum==9900)
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if( arg1[0]=='\0')
      {
      send_to_char( "Format:   CONNECT <direction> <destination #> [both]\n\r",ch);
      return;
      }

    door_num = direction_door( arg1 );
    if( door_num < 0 )
      door_num = atol( arg1 );
    dest_vnum = atol( arg2 );

    if( door_num < 0 || door_num > 5 )
      {
      send_to_char( "Door is invalid.\n\r",ch);
      return;
      }

    if( get_room_index( dest_vnum ) == NULL && dest_vnum != -1 )
      {
      send_to_char( "Destination is invalid.\n\r",ch);
      return;
      }

    set_exit( ch->in_room->vnum, door_num, dest_vnum );
    if( arg3[0]=='b' || arg3[0]=='B')
      set_exit( dest_vnum, reverse(door_num),ch->in_room->vnum);
    if (IS_IMMORTAL(ch))send_to_char( "Connection made.\n\r", ch);
    return;
}
extern void ListCheck(void);
void do_test1( CHAR_DATA *ch, char *argument )
{
/*
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  
  argument=one_argument(argument, arg1);
  argument=one_argument(argument, arg2);

  sprintf( buf, "Replace '%s' with '%s' in '%s'.\n\r", arg1, arg2, argument );
  send_to_char( buf, ch );

  sprintf( buf, "'%s'\n\r", str_replace( argument, arg1, arg2 ));
  send_to_char( buf, ch );
*/
  send_to_char( "Siteban saving.\n\r", ch );
  save_sites();
    
  return;
}

void do_test2( CHAR_DATA *ch, char *argument )
{ 
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int  size, percent;
  
  argument=one_argument(argument, arg1);
  size = atol( arg1 );
  percent = atol( argument );

  sprintf( buf, "Bar Graph:%s:\n\r", get_bar_graph( ch, size, percent ) );
  send_to_char( buf, ch );

 return;
}
void do_rescale( CHAR_DATA *ch, char *argument )
  {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *mch;
    int vnum, scale, divisor;

     if(IS_NPC(ch) && ch->pIndexData->vnum==9900)
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
    
    if( arg1[0]=='\0')
      {
      send_to_char( "Format: RESCALE <Monster Vnum> <Player Name> <Percentage>\n\r", ch);
      return;
      }

    vnum=atol( arg1);
    if( vnum<1)
      return;

    if( (victim=lookup_char( arg2 ))== NULL ||
         IS_NPC( victim ) )
      return;

    scale=atol( arg3) * victim->level;

    divisor = 200 * (100-victim->level) + 10000;

    for( mch=first_char; mch!=NULL; mch=mch->next)
      if( IS_NPC(mch) && mch->pIndexData->vnum==vnum)
	{
	mch->level                =mch->pIndexData->level*scale/10000;
	mch->hit                  = 1+
	    dice( mch->pIndexData->hitnodice*scale/divisor,
	    mch->pIndexData->hitsizedice*scale/divisor)+
	    mch->pIndexData->hitplus*scale/divisor;
	mch->max_hit              =mch->hit;
	mch->npcdata->damnodice   =mch->pIndexData->damnodice*scale/divisor;
	mch->npcdata->damsizedice =mch->pIndexData->damsizedice*scale/divisor;
	mch->npcdata->damplus     =mch->pIndexData->damplus*scale/divisor+1;
	}
    
    return;
    }

void do_door( CHAR_DATA *ch, char *argument )
    {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    int room, door, type;
    EXIT_DATA *pexit;

     if(IS_NPC(ch) && ch->pIndexData->vnum==9900)
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if( arg1[0]=='\0')
      {
      send_to_char( "Format:   DOOR <direction> <type> [<door name>]\n\r0 - hall\n\r1 - open door\n\r2 - closed door\n\r4 - locked door\n\r8 - hidden door\n\r", ch);
      send_to_char( "16- Absolutely do not use this bit.\n\r32- pickproof door.", ch);
      return;
      }

    room= ch->in_room->vnum;
    door = direction_door( arg1 );
    if( door < 0 )
      door = atol( arg1 );
    type= atol( arg2)%63;

    if( door < 0 || door > 5 )
      {
      send_to_char( "Door is invalid.\n\r",ch);
      return;
      }

    if( IS_SET( type, EX_CLOSED))
      type=type|EX_ISDOOR;
    if( IS_SET( type, EX_LOCKED))
      type=type|EX_ISDOOR|EX_CLOSED;
    if( IS_SET( type, EX_PICKPROOF))
      type=type|EX_ISDOOR|EX_CLOSED|EX_LOCKED;
    if( IS_SET( type, EX_HIDDEN))
      type=type|EX_ISDOOR|EX_CLOSED;

    if( IS_SET( type, 16 ) )
      type-=16;

    if( arg3[0]=='\0' && IS_SET( type, EX_ISDOOR))
      strcpy( arg3, "door");

    pexit=ch->in_room->exit[door];
    if( pexit==NULL)
      {
      send_to_char( "No exit there on this side.\n\r", ch);
      return;
      }
    pexit->exit_info = type;
    pexit->keyword = STRALLOC( arg3 );
    pexit->description = STRALLOC(arg3);
    pexit->key=0;

    pexit=room_index[room]->exit[door]->to_room->exit[reverse(door)];
    if( pexit==NULL)
      {
      send_to_char( "No exit there on other side.\n\r", ch);
      return;
      }
    pexit->exit_info = type;
    pexit->keyword = STRALLOC( arg3 );
    pexit->description = STRALLOC(arg3);
    pexit->key=0;

    return;
    }

void do_key( CHAR_DATA *ch, char *argument)
  {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int room, door;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char buf[180];

     if(IS_NPC(ch) && ch->pIndexData->vnum==9900)
      return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    if( arg1[0]=='\0')
      {
      send_to_char( "Format:   KEY <door #> <key name>\n\r", ch);
      return;
      }

    room= ch->in_room->vnum;
    door= atol( arg1);

    if( !is_exit( room, door ))
      return;
    if( !IS_SET(room_index[room]->exit[door]->exit_info, EX_LOCKED))
      return;

    pObjIndex = get_obj_index( 3714 ) ;
    obj = create_object( pObjIndex, 0 );
    obj_to_char( obj, ch );

    STRFREE (obj->name);
    STRFREE (obj->short_descr);
    STRFREE (obj->long_descr);

    sprintf( buf, "key %s", arg2 );
    obj->name = STRALLOC( buf );

    sprintf( buf, "the %s key", arg2 );
    obj->short_descr = STRALLOC( buf );

    sprintf( buf, "The %s key is laying here.", arg2 );
    obj->description = STRALLOC( buf );

    sprintf( buf, "The %s key.", arg2 );
    obj->long_descr = STRALLOC( buf );

    obj->value[0] = room;
    
    room_index[ room ]->exit[door]->key = room;
    room_index[ room ]->exit[door]->to_room->exit[reverse( door)]->key = room;
    send_to_char ("Key created.\n\r", ch);
    return;
  }

extern int pulse_point;
void do_tick( CHAR_DATA *ch, char *argument)
  {
  int cnt;
  for( cnt=0; cnt< PULSE_TICK; cnt++)
    update_handler();
  send_to_char( "Time advances 1 minute.\n\r", ch);
  return;
  }

char *broken_bits( int number, char *vector , bool linear)
  {
  int bit, cnt;
  bool found,bitfound;
  char buf[100];

  found=FALSE;
  bit=0;

  broken_string[0]='\0';

  if( number > PLR_PLAN )    /* Just using the value here  */
    number = PLR_PLAN;

  if( !linear )
    while( bit<=number )
      {
      if( (bit & number) != 0)
	{
	bitfound=FALSE;
	for(cnt = 0; cnt < MAX_BITVECTOR; cnt++ )
	  if( bit == bitvector_table[cnt].value )
	    if( is_name_short( vector, bitvector_table[cnt].name ))
	      {
	      if( found )
		strcat( broken_string, "|" );
	      strcat( broken_string, bitvector_table[cnt].name );
	      found=TRUE;
	      bitfound=TRUE;
	      }
	if(!bitfound)
	  {
	  if( found )
	    strcat(broken_string, "|" );
	  sprintf( buf, "%d", bit );
	  strcat( broken_string, buf);
	  found=TRUE;
	  }
	}
      if(bit!=0)
	bit*=2;
      else
	bit++;
      }
  else
     for(cnt = 0; cnt < MAX_BITVECTOR; cnt++ )
      if( number == bitvector_table[cnt].value )
	if( is_name_short( vector, bitvector_table[cnt].name ))
	  {
	  strcat( broken_string, bitvector_table[cnt].name );
	  found=TRUE;
	  }
  if( !found )
    {
    sprintf( buf, "%d", number );
    strcat( broken_string, buf);
    }
  return( broken_string);
  }



void do_object_rape( CHAR_DATA *ch, char *argument )
	/* aset by Garion */

{
    char 	arg1 [MAX_INPUT_LENGTH];
    char 	arg2 [MAX_INPUT_LENGTH];
    char 	arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA	*obj;
    AFFECT_DATA *paf;
    int		value;


    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 ); 
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax: aset <object> <field> <value>.\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("Field being one of:\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("str int wis dex con hitroll damroll ac\n\r", ch);
	send_to_char("hp mana move para petri spell breath rod\n\r", ch);
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL)
    {
	send_to_char("Aint no such thing Jack.\n\r", ch);
	return;
    }

    if ( !is_number( arg3 ) ) 
    {	
	send_to_char("Value must be numeric.\n\r", ch);
	return;
    }

    value = atol( arg3 );
    if ( ( ch->level < MAX_LEVEL ) && ( ( value > 20 ) || ( value < -20 ) ) )
	/* value restrictions for safety... assume the IMP knows better :) */
    {
	send_to_char("Value range is -20 to 20.\n\r", ch);
	return;
    }

    CREATE(paf, AFFECT_DATA, 1);
    paf->type		= value;
    paf->duration	= -1;
    paf->modifier	= value;
    paf->bitvector	= 0;

	/* here we go */

    if ( !strcasecmp( arg2, "str" ) )
	paf->location 	= APPLY_STR;
    else if ( !strcasecmp( arg2, "dex" ) )
	paf->location	= APPLY_DEX;
    else if ( !strcasecmp( arg2, "int" ) )
	paf->location	= APPLY_INT;
    else if ( !strcasecmp( arg2, "wis" ) )
	paf->location	= APPLY_WIS;
    else if ( !strcasecmp( arg2, "con" ) )
	paf->location	= APPLY_CON;
    else if ( !strcasecmp( arg2, "mana" ) )
	paf->location 	= APPLY_MANA;
    else if ( !strcasecmp( arg2, "hp" ) )
	paf->location	= APPLY_HIT;	
    else if ( !strcasecmp( arg2, "move" ) )
	paf->location	= APPLY_MOVE;
    else if ( !strcasecmp( arg2, "ac" ) ) 
	paf->location 	= APPLY_AC;
    else if ( !strcasecmp( arg2, "hitroll" ) )
	paf->location	= APPLY_HITROLL;
    else if ( !strcasecmp( arg2, "damroll" ) )
	paf->location	= APPLY_DAMROLL;
    else if ( !strcasecmp( arg2, "para" ) )
	paf->location	= APPLY_SAVING_PARA;
    else if ( !strcasecmp( arg2, "rod" ) )
	paf->location	= APPLY_SAVING_ROD;
    else if ( !strcasecmp( arg2, "petri" ) )
	paf->location	= APPLY_SAVING_PETRI;
    else if ( !strcasecmp( arg2, "breath" ) )
	paf->location	= APPLY_SAVING_BREATH;	
    else if ( !strcasecmp( arg2, "spell" ) )
	paf->location	= APPLY_SAVING_SPELL;
    else
    {
   	do_aset( ch, "" );
    	return;
    }
	
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    return;
}

/*
 *void do_reign( CHAR_DATA *ch, char *argument )
 * {
 * char arg[MAX_INPUT_LENGTH];
 * ROOM_INDEX_DATA *location;
 *
 * one_argument( argument, arg );
 * location = ch->in_room;
 * if ( location == NULL )
 *   {
 *   send_to_char( "Where the hell are you?\n\r", ch );
 *   return ;
 *   }
 *
 * if(  ch->level<95 || IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM )  )
 *   {
 *   send_to_char( "You cannot establish your rule yet.\n\r", ch );
 *   return ;
 *   }
 *
 * if ( IS_IMMORTAL(ch))
 *   {
 *   send_to_char( "Immortals cannot establish any form of rule.\n\r", ch );
 *   return ;
 *   }
 * if(  ch->pcdata->castle->reign_room != 0 )
 *   {
 *   send_to_char( "You cannot establish your rule again.\n\r", ch );
 *   return ;
 *   }
 *
 * if ( location->creator_pvnum!=ch->pcdata->pvnum )
 *   {
 *   send_to_char( "You cannot establish your rule in this room.\n\r", ch );
 *   return ;
 *   }
 *
 * send_to_char( "You estabilish your reign in this room.\n\r", ch);
 * ch->pcdata->castle->reign_room = ch->in_room->vnum;
 *
 * save_char_obj(ch, NORMAL_SAVE);
 * return;
 * }
 */



void do_mprogram( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    MPROG_DATA *mprog;
    MOB_INDEX_DATA *mind;
    NPC_TOKEN *token;
    char lbuf[130];
    int cnt;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Mstat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
       {
	      send_to_char( "They aren't here.\n\r", ch );
	      return;
	}

    sprintf( buf, "Name: %s.\n\r", victim->name );
    send_to_char( buf, ch );

    if( !IS_NPC( victim ) )
      return;

    if( (mind = victim->pIndexData) == NULL )
      return;

    for( mprog=mind->mobprogs; mprog != NULL; mprog = mprog->next )
      {
      sprintf( buf, ">%s %s~\n\r", mprog_type_to_name( mprog->type ),
	      mprog->arglist );
      send_to_char( buf, ch );

      for( token = mprog->token_list; token != NULL; token = token->next )
        {
        for( cnt=0; cnt < 2*token->level; cnt++)
           buf[cnt]=' ';
        buf[cnt]='\0';
        sprintf( lbuf, "<%2d>%s", token->line, buf );
        
        switch( token->type )
          {
          case 1:
            sprintf( buf, "%s[%d][%d]%s %s\n\r", lbuf, token->type,
                token->value,
                (char *) token->function,
                token->string != NULL ? token->string : "" );
            send_to_char( buf, ch );
            break;
          case 2:
            sprintf( buf, "%s[%d][%d]%s %s\n\r", lbuf, token->type,
                token->value,
                cmd_table[token->value].name,
                token->string != NULL ? token->string : "" );
            send_to_char( buf, ch );
            break;
          case 3:
            sprintf( buf, "%s[%d]if %s\n\r", lbuf, token->type,
                token->string != NULL ? token->string : "" );
            send_to_char( buf, ch );
            break;
          case 4:
            sprintf( buf, "%s[%d]or %s\n\r", lbuf, token->type,
                token->string != NULL ? token->string : "" );
            send_to_char( buf, ch );
            break;
          case 5:
            sprintf( buf, "%s[%d]else\n\r", lbuf, token->type);
            send_to_char( buf, ch );
            break;
          case 6:
            sprintf( buf, "%s[%d]endif\n\r", lbuf, token->type);
            send_to_char( buf, ch );
            break;
          case 7:
            sprintf( buf, "%s[%d]break\n\r", lbuf, token->type);
            send_to_char( buf, ch );
            break;
          default:
            sprintf( buf, "%s[%d]  Unknown Type\n\r", lbuf, token->type);
            send_to_char( buf, ch );
            break;
          }

        }
      }

    return;
   }

void do_delcastle( CHAR_DATA *ch, char * arg )
  {
  char arg1 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  arg = one_argument( arg, arg1 );

  if ( arg1[0] == '\0' )
    {
    send_to_char( "Delete who's castle?\n\r", ch );
    return;
    }
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
    send_to_char( "Hmmm...can't find them.\n\r", ch );
    return;
    }
  if ( IS_NPC(victim) )
    {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
    }
  if(victim->pcdata->castle==NULL)
    {
    send_to_char( "They don't have a castle structure.\n\r", ch );
    return;
    }
  
  del_castle(victim);
  sprintf(arg1,"Deleting %s's castle...done.\n\r",victim->name);
  send_to_char( arg1, ch );
  }


void describe_object_program( CHAR_DATA *ch, OBJ_PROG *prg )
  {
  char buf[MAX_INPUT_LENGTH];

  if( prg->cmd > 0 )
    {
    sprintf( buf, "<%3d> [%3d%%] TRIG_COMMAND %d\n\r",
       prg->index, prg->percentage, prg->cmd );
    }
  else
   switch( prg->cmd )
    {
    case -2:    /* unknown command or social */
      sprintf( buf, "<%3d> [%3d%%] TRIG_UNKNOWN %s\n\r",
       prg->index, prg->percentage, prg->unknown );
      break;
    case -3:    /* Tick check */
      sprintf( buf, "<%3d> [%3d%%] TRIG_TICK\n\r",
       prg->index, prg->percentage);
      break;
    case -4:   /* void trigger */
      sprintf( buf, "<%3d> [----] TRIG_VOID\n\r", prg->index);
      break;
    case -5:    /* Got hit check */
      sprintf( buf, "<%3d> [%3d%%] TRIG_HIT\n\r",
       prg->index, prg->percentage);
      break;
    case -6:    /* Damaged another check */
      sprintf( buf, "<%3d> [%3d%%] TRIG_DAMAGE\n\r",
       prg->index, prg->percentage);
      break;
    case -7:    /* Wear Item check */
      sprintf( buf, "<%3d> [%3d%%] TRIG_WEAR\n\r",
       prg->index, prg->percentage);
      break;
    case -8:    /* Remove item check */
      sprintf( buf, "<%3d> [%3d%%] TRIG_REMOVE\n\r",
       prg->index, prg->percentage);
      break;
   default:
      sprintf( buf, "ERROR\n\r" );
   }

  send_to_char (buf, ch );

  switch( prg->obj_command )
    {
    case 'E':    /* screen echo */
      sprintf( buf, "ECHO %s\n\r", prg->argument );
      break;
    case 'C':    /* user command at level 99 without arg, but with multi-line*/
      sprintf( buf, "COMMAND %s\n\r", prg->argument );
      break;
    case 'G':    /* user command at level 99 with argument */
      sprintf( buf, "ARGUMENT %s\n\r", prg->argument );
      break;
    case 'S':   /* Set quest bit to value */
       sprintf( buf, "SET QUEST Off %d Bits %d Value %d\n\r", 
           prg->quest_offset, prg->quest_bits, prg->if_value );
      break;
    case 'D':   /* Add to quest bit */
       sprintf( buf, "ADD QUEST Off %d Bits %d Add %d\n\r", 
           prg->quest_offset, prg->quest_bits, prg->if_value );
      break;
    case 'P':   /* Player quest bit if check */
       sprintf( buf, "PLAYER IF QUEST Off %d Bits %d If %c Value %d Then %d Else %d\n\r", 
           prg->quest_offset, prg->quest_bits, prg->if_symbol,
           prg->if_value, prg->if_true, prg->if_false );
      break;
    case 'Q':   /* Object quest bit if check */
       sprintf( buf, "OBJ IF QUEST Off %d Bits %d If %c Value %d Then %d Else %d\n\r", 
           prg->quest_offset, prg->quest_bits, prg->if_symbol,
           prg->if_value, prg->if_true, prg->if_false );
      break;
    case 'H':   /* If has object check */
      sprintf( buf, "IF HAS OBJECT %d Then %d Else %d\n\r",
          prg->if_value, prg->if_true, prg->if_false );
      break;
    case 'I':   /* If check */
       sprintf( buf, "IF CHECK %d %c Value %d Then %d Else %d\n\r", 
           prg->if_check, prg->if_symbol,
           prg->if_value, prg->if_true, prg->if_false );
      break;
    case 'A':   /* Apply to temp stats */
      sprintf( buf, "APPLY TEMP %d %d\n\r", prg->if_check, prg->if_value );
      break;
    case 'J':   /* Junk the item */
      sprintf( buf, "JUNK ITEM\n\r" );
      break;
    default:
      sprintf( buf, "ERROR\n\r" );
    }

  send_to_char( buf, ch );

  return;
  }

void do_oprogram( CHAR_DATA *ch, char *argument )
  {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_PROG *oprog;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Oprogram what?\n\r", ch );
	return;
    }

	  if(is_number(arg))
      {
      int anum;

      anum=atol(arg);
      for(obj=first_object;obj!=NULL;obj=obj->next)
	if(obj->pIndexData!=NULL&&obj->pIndexData->vnum==anum)
	  break;
      if(obj==NULL)
	{
	      send_to_char( "No object loaded has that vnum.\n\r", ch );
	      return;
	}
      }
    else
      {
      if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
	{
	      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	      return;
	}
      }


    sprintf( buf, "Name: %s.   Total in game: %d/%d\n\r",
	obj->name , obj->pIndexData->total_objects, obj->pIndexData->max_objs);
    send_to_char( buf, ch );

    if( obj->test_obj )
      send_to_char( "This is a test game item.\n\r", ch);

    sprintf( buf, "Vnum: %u.  Type: %s.\n\r",
	obj->pIndexData->vnum, item_type_name( obj ) );
    send_to_char( buf, ch );


  if( obj->pIndexData->obj_prog == NULL )
    {
    send_to_char( "This object does not have a program.\n\r", ch );
    return;
    }

  oprog = obj->pIndexData->obj_prog;

  while( oprog != NULL )
    {
    describe_object_program( ch, oprog );
    oprog = oprog->next;
    }

  return;
  }


void do_shutoff( CHAR_DATA *ch, char *argument )
  {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shutoff what?\n\r", ch );
	return;
    }

  if(is_number(arg))
      {
      int anum;

      anum=atol(arg);
      for(obj=first_object;obj!=NULL;obj=obj->next)
	if(obj->pIndexData!=NULL&&obj->pIndexData->vnum==anum)
	  break;
      }
    else
      {
      obj = get_obj_world( ch, arg );
      }


  if( obj != NULL )
    {
    if( obj->pIndexData->obj_prog == NULL )
      {
      sprintf( buf, "There are no object programs for %s.\n\r", obj->name );
      send_to_char( buf, ch );
      return;
      }
    sprintf( buf, "Object programs for %s completely shut off.\n\r",
         obj->name );
    send_to_char( buf, ch );
    obj->pIndexData->obj_prog = NULL;
    return;
    }


  if(is_number(arg))
      {
      int anum;

      anum=atol(arg);
      for(victim=first_char;victim!=NULL;victim=victim->next)
	if(victim->pIndexData!=NULL&&victim->pIndexData->vnum==anum)
	  break;
      if(victim==NULL)
	{
	      send_to_char( "Nothing loaded has that vnum.\n\r", ch );
	      return;
	}
      }
    else
      {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	      send_to_char( "They aren't here.\n\r", ch );
	      return;
	}
      }

  if( victim == NULL )
    {
    send_to_char ("There is no such thing.\n\r", ch );
    return;
    }

  if( victim->pIndexData == NULL || victim->pIndexData->mobprogs == NULL )
    {
    sprintf( buf, "There are no mobile programs for %s.\n\r", victim->name );
    send_to_char( buf, ch );
    return;
    }

  sprintf( buf, "Mobile programs for %s completely shut off.\n\r",
      victim->name );
  send_to_char( buf, ch );

  victim->pIndexData->mobprogs = NULL;
  return;
  }

void do_roomfragment( CHAR_DATA *ch, char *argument )
  {
  int cnt;
  char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  strcpy( buf, "Room Fragmentation chart:\n\r");
  strcat( buf, "    0 1 2 3 4 5 6 7 8 9\n\r" );
  for( cnt=0; cnt<MAX_VNUM/100; cnt++)
    {
    if( cnt%10 == 0 )
      {
      sprintf( buf2, "%2d  ", cnt/10 );
      strcat( buf, buf2 );
      }
    if( room_index[cnt*100+1] != NULL )
      strcat( buf, "X" );
    else
      strcat( buf, "-" );
    if( room_index[cnt*100+51] != NULL )
      strcat( buf, "X" );
    else
      strcat( buf, "-" );
    if( cnt%10==9 )
    strcat( buf, "\n\r" );
    }

  send_to_char( buf, ch );
  return;
  }

void do_pload( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *fch;
  DESCRIPTOR_DATA *d;
  char buf[180];
  static DESCRIPTOR_DATA d_zero;
  bool exists;
  int expire;

	if ( !check_parse_name( argument, FALSE ) )
	  {
          send_to_char( "Either you chose a name with less than 3 characters, with a non-ascii character,\n\ror it was the name of a monster.\n\rIllegal name, try another.\n\r", ch );
	  return;
	  }
  fch=lookup_char( argument );
  if( fch!=NULL)
    {
    send_to_char( "This character is already loaded.\n\r", ch );
    return;
   }

  for (d=first_descriptor;d!=NULL;d=d->next)
  {
   if (is_desc_valid(d->character))
    if (!strcasecmp(argument, d->character->name))
   {
    act("$E was in the process of logging on. You close $s session.",ch,NULL, d->character, TO_CHAR);
    close_socket( d, TRUE);
   }
  }
  d = NULL;
    CREATE(d, DESCRIPTOR_DATA, 1); 
    *d	= d_zero;
    d->original=NULL;
    d->descriptor = -998;  /* Special case for pload loads */
    load_error = FALSE;
    exists = load_char_obj( d, argument );
    fch   = d->character;

  if( (expire = character_expiration( fch )) < 0 )
    exists = FALSE;

  if( !exists )
    {
    sprintf( buf, "The character named '%s' cannot be found.\n\r", argument);
    send_to_char( buf, ch );
    if( d != NULL )
      {
      d->character=NULL;
      d->original=NULL;
      }
    if( fch != NULL )
      {
      fch->desc=NULL;
      extract_char( fch, TRUE);
      }
    if(d != NULL )
      DISPOSE(d);
    return;
    }

  d->connected	= CON_PLAYING;
  add_char(fch);
  add_player(fch);
  fch->desc=NULL;
  if( fch->in_room != NULL )
    char_to_room( fch, fch->in_room );
  else
    char_to_room( fch, ch->in_room );

  send_to_char( "You load the character.\n\r", ch );

  d->character=NULL;
  d->original=NULL;

  DISPOSE(d);
  return;
  }


void do_pquit( CHAR_DATA *ch, char *argument )
    {
    CHAR_DATA *victim;

    if ( IS_NPC(ch) )
	return;

    if( ( victim = get_player_world( ch, argument ) ) == NULL )
      {
      send_to_char( "That player does not exist in the realm.\n\r", ch );
      return;
      }
    if (IS_NPC(victim))
    {
        send_to_char( "You are trying to pquit a mobile!\n\r", ch);
        return;
    }
    if (victim->level>ch->level 
       || (IS_IMMORTAL(victim) && is_desc_valid(victim) ) )
      {
        send_to_char( "You cannot force that player to quit.\n\r", ch);
        return;
      }
    ch_printf(ch, "You forced %s to quit.\n\r",victim->name);
    do_quit( victim, "arglebargle" );
    return;
    }

void do_fixpass (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char *pArg;
  char *pwdnew;
  /* char *p; */
  char cEnd;



    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace((int)*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace((int)*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg3;
    while ( isspace((int)*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';


    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0]=='\0')
    {
        send_to_char( "Syntax: fixpass <character> <new pass> <new pass>.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Fix the password on whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here (better pload them).\n\r", ch );
        return;
    }
/*
        if (victim->desc || (victim->desc->connected = CON_PLAYING) )
        {
                send_to_char ("This player is actually playing!\n\r",ch);
                return;
        }
*/
    if( strcmp( arg2, arg3 ) )
      {
      send_to_char( "The new password does not verify.\n\rPlease try again.\n\r", ch);
      return;
      }
        if ( !is_valid_password( arg2 ) )
        {
            send_to_char(
                "New password not acceptable, try again.\n\r", ch );
            send_to_char( "The password must only contain letters (case sensitive), or numbers.\n\rYou are required to include at least one number in the password.\n\r", ch );
            return;
        }

    pwdnew = crypt( arg2, victim->name );

    STRFREE(victim->pcdata->pwd );
    victim->pcdata->pwd = str_dup( pwdnew );
    save_char_obj(victim, NORMAL_SAVE);
    ch_printf(ch, "Ok. %s has had their password changed.\n\r", victim->name );
    return;
 
}

void do_undeny (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Undeny whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here (better pload them).\n\r", ch );
        return;
    }
/*
 if (victim->desc || (victim->desc->connected == CON_PLAYING) )
    {
     send_to_char ("This player is currently playing.\n\r",ch);
     return;
    }
*/
 REMOVE_BIT(victim->act, PLR_DENY);
 save_char_obj(victim, NORMAL_SAVE);
 ch_printf(ch, "%s has been undenied.\n\r", victim->name);
 return;
}

void replace_item_names( OBJ_DATA *obj_list, char *old_txt, char *new_txt )
  {
  char buf[MAX_STRING_LENGTH];

  for( ; obj_list!=NULL; obj_list=obj_list->next_content )
    {
    strcpy( buf, obj_list->unforged_short_descr );

    if( str_contains( obj_list->unforged_short_descr, old_txt ) )
      {
      STRFREE(obj_list->unforged_short_descr);
      obj_list->unforged_short_descr=str_dup(
  str_replace(obj_list->unforged_short_descr,old_txt, capitalize(new_txt)));
      STRFREE(obj_list->short_descr );
      obj_list->short_descr = str_dup(obj_list->unforged_short_descr);
      }

    if( obj_list->first_content !=NULL )
      replace_item_names( obj_list->first_content, old_txt, new_txt );
    }
  return;
  }

void do_rename (CHAR_DATA* ch, char* argument)
{
        char old_name[MAX_INPUT_LENGTH],
             new_name[MAX_INPUT_LENGTH],
             strsave [MAX_INPUT_LENGTH];

        CHAR_DATA* victim;
        FILE* file;
        argument = one_argument(argument, old_name); /* find new/old name */
        one_argument (argument, new_name);

        /* Trivial checks */
        if (!old_name[0])
        {
                send_to_char ("Rename who?\n\r",ch);
                return;
        }

        victim = get_char_world (ch, old_name);

        if (!victim)
        {
                send_to_char ("There is no such a person online.\n\r",ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
                return;
        }

        /* allow rename self new_name,but otherwise only lower level */
        if ( (victim == ch) || (get_trust (victim) >= get_trust (ch)) )
        {
                send_to_char ("You failed.\n\r",ch);
                return;
        }

        if (!new_name[0])
        {
                send_to_char ("Rename to what new name?\n\r",ch);
                return;
        }

        /* Insert check for clan here!! */
        /*

        if (victim->clan)
        {
                send_to_char ("This player is member of a clan, remove him from there first.\n\r",ch);
                return;
        }
        */

        if (!check_parse_name(new_name, TRUE))
        {
                send_to_char ("The new name is illegal.\n\r",ch);
                return;
        }

        /* First, check if there is a player named that off-line */
#if !defined(macintosh) && !defined(MSDOS)
    sprintf( strsave, "%s/%c/%s", PLAYER_DIR, tolower(new_name[0]),
                  capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );
#endif
        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char ("A player with that name already exists!\n\r",ch);
                fclose (file);
        fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        /* Check .gz file ! */
#if !defined(macintosh) && !defined(MSDOS)
    sprintf( strsave, "%s/%c/%s.gz", PLAYER_DIR, tolower( new_name[0] ),
                 capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );
#endif

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
                fclose (file);
        fpReserve = fopen( NULL_FILE, "r" );
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        if (get_char_world(ch,new_name)) /* check for playing level-1 non-saved */
        {
                send_to_char ("A player with the name you specified already exists!\n\r",ch);
                return;
        }

        /* Save the filename of the old name */

#if !defined(macintosh) && !defined(MSDOS)
    sprintf( strsave, "%s/%c/%s", PLAYER_DIR, tolower( victim->name[0] ),
                  capitalize( victim->name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
#endif


        /* Rename the character and save him to a new file */
        /* NOTE: Players who are level 1 do NOT get saved under a new name */

        /* We need to modify all inventory items now 
        replace_item_names( victim->first_carrying, victim->name, new_name );
        */

	STRFREE (victim->name );
        victim->name = STRALLOC (new_name);


        

        save_char_obj(victim, NORMAL_SAVE);

        /* unlink the old file */
        unlink (strsave); /* unlink does return a value.. but we do not care */

        /* That's it! */
        if( *victim->name >= 'a' && *victim->name <= 'z' )
         *victim->name -= ('a' - 'A');

        send_to_char ("Character renamed.\n\r",ch);
        victim->position = POS_STANDING; /* I am laaazy */
        act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);

        return;

} /* do_rename */



int direction_door( char *txt )
  {
  if( txt==NULL || *txt=='\0' )
    return( -1 );

  if( *txt=='N' || *txt=='n' )
    return( 0);
  if( *txt=='E' || *txt=='e' )
    return( 1);
  if( *txt=='S' || *txt=='s' )
    return( 2);
  if( *txt=='W' || *txt=='w' )
    return( 3);
  if( *txt=='U' || *txt=='u' )
    return( 4);
  if( *txt=='D' || *txt=='d' )
    return( 5);

  return( -1 );
  }

void do_giveprac( CHAR_DATA *ch, char *argument )
  {
  char arg1[MAX_INPUT_LENGTH];
  int  pracs=0;
  CHAR_DATA *victim;
  
  argument=one_argument(argument, arg1);

  if(*arg1=='\0')
    {
      send_to_char("Usage: giveprac <name> <amount>\n\r", ch);
      return;
    }

  if((victim = get_char_world(ch, arg1)) == NULL)
    {
      send_to_char("They aren't logged in.\n\r", ch);
      return;
    }

  if(IS_NPC(victim))
    {
      send_to_char("You can only give practices to players.\n\r", ch);
      return;
    }

  if(*argument == '\0')
  {
    send_to_char("You must specify a number of practices.\n\r", ch);
    return;
  }

  pracs=atoi(argument);
  if(pracs<-100 || pracs>100)
    {
      send_to_char("The range of practices is -100 to +100.\n\r", ch);
      return;
    }

  if(pracs == 0)
  {
    send_to_char("What was the point?\n\r", ch);
    return;
  }

  if( pracs > 0 )
    {
    victim->pcdata->give_prac_pos += pracs;
    victim->practice += pracs;
    send_to_char( "You give them the practices.\n\r", ch );
    send_to_char( "Your number of practices increases.\n\r", victim );
    }

  if( pracs<0 )
    {
    victim->pcdata->give_prac_neg -= pracs;
    victim->practice += pracs;
    send_to_char( "You remove their practices.\n\r", ch );
    if( victim->practice < 0 )
      {
      victim->max_hit += 2*victim->practice;
      victim->practice = 0;
      }
    }
  
  return;
  }

/* REVERT command.  Restore a player's backup file.  Presto 8/98 */
void do_revert(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char back_name[MAX_STRING_LENGTH], real_name[MAX_STRING_LENGTH];
  char victimname[MAX_STRING_LENGTH], sub_dir;
  int  i, length;
  CHAR_DATA *victim;
  FILE *back_file, *real_file;

  if(ch->level < MAX_LEVEL-1)
  {
    send_to_char("You may not do that.\n\r", ch);
    return;
  }

  argument=one_argument(argument, arg);

  if(*arg=='\0')
  {
    send_to_char("Usage: revert <name>\n\r", ch);
    return;
  }

  /* See if they're logged in. */
  if((victim = get_char_world(ch, arg)) == NULL)
  {
    /* They aren't logged in. */
  }
  else
  {
    /* They are logged in.
       Make sure they can be reverted.
       If so, log the player out. */

    if(victim->level < 2)
    {
      send_to_char("The player must be at least level 2.\n\r", ch);
      return;
    }

    if(victim == ch)
    {
      send_to_char("You may not revert yourself.\n\r", ch);
      return;
    }

    if(IS_NPC(victim))
    {
      send_to_char("You may not revert a mob.\n\r", ch);
      return;
    }

    if(victim->fighting != NULL)
    {
      send_to_char("That player is currently fighting.\n\r", ch);
      return;
    }

    /* Move character to chakkor and log them out. */
    char_from_room(victim);
    char_to_room(victim, find_location(ch, "9755"));
    send_to_char("Your backup file is being restored. Quitting...\n\r", victim);
    send_to_char("Logging out player...\n\r", ch);
    strcpy(arg, victim->name);
    do_quit(victim, "arglebargle");
  }

  /* See if the backup pfile exists. */
  strcpy(victimname, capitalize_name(arg));
  length=strlen(victimname);
  for(i=1;i<length;i++)
    victimname[i]=tolower(victimname[i]);
  sub_dir=tolower(victimname[0]);

  sprintf(back_name, "%s/%c/bak/%s", PLAYER_DIR, sub_dir, victimname);
  sprintf(real_name, "%s/%c/%s",     PLAYER_DIR, sub_dir, victimname);

  if(!(back_file=fopen(back_name, "r")))
  {
    send_to_char("Could not open player's backup file.\n\r", ch);
    return;
  }
  else
    send_to_char("Player's backup file is open...\n\r", ch);

  if(!(real_file=fopen(real_name, "w")))
  {
    send_to_char("Could not open player's real file.\n\r", ch);
    fclose(back_file);
    return;
  }
  else
    send_to_char("Player's real file is open...\n\r", ch);

  i=fgetc(back_file);
  while(i != EOF)
  {
    fprintf(real_file, "%c", i);
    i=fgetc(back_file);
  }

  fclose(real_file);
  fclose(back_file);
  send_to_char("Revert was successful.\n\r", ch);
}

void do_rdelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    bool fullcont=FALSE;
    int door;

    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME)
        || ch->level==MAX_LEVEL)
      fullcont=TRUE;
   if (!fullcont)
   {
    if (ch->pcdata->castle==NULL)
     {
      send_to_char("You have no castle established yet.\n\r", ch);
      return;
     }
    if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
     {
      send_to_char("You are NOT in your castle!\n\r", ch);
      return;
     }
   }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which room?\n\r", ch );
        return;
    }

    /* Find the room. */
    if (fullcont)  
    {
    if ( ( location = find_location( ch, arg ) ) == NULL )
     {
        send_to_char( "No such location.\n\r", ch );
        return;
     }
    }
    else
    {
     if ( ( door = getDirNumber( arg ) ) >= 0 )
     {
       if (ch->in_room->exit[door] == NULL ||
	   ch->in_room->exit[door]->to_room == NULL )
       {
 	send_to_char("That room doesn't exist!\n\r", ch);
 	return;
       }
       location = ch->in_room->exit[door]->to_room;
       if (!IS_SET(location->room_flags, ROOM_IS_CASTLE) ||
	   location->creator_pvnum!=ch->pcdata->pvnum)
       {
 	ch_printf(ch, "That room isn't part of your castle!\n\r");
        return;
       }
     }
     else
     {
        send_to_char( "No room in that direction.\n\r", ch );
        return;
     }
    }
    /* Does the player have the right to delete this room? */
/*
    if ( fullcont && !((location->area->authors
        && is_name( ch->name, location->area->authors) && !REAL_GAME)
        || ch->level==MAX_LEVEL))
    {
      send_to_char( "That room is not in your area.\n\r", ch );
      return;
    }
*/
    delete_room( location );

    send_to_char( "Room deleted.\n\r", ch );
    return;
}

void do_odelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *obj;
    OBJ_DATA *temp;
    bool fullcont=FALSE;

    argument = one_argument( argument, arg );

    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME)
        || ch->level==MAX_LEVEL)
      fullcont=TRUE;
   if (!fullcont)
   {
    if (ch->pcdata->castle==NULL)
     {
      send_to_char("You have no castle established yet.\n\r", ch);
      return;
     }
    if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
     {
      send_to_char("You are NOT in your castle!\n\r", ch);
      return;
     }
   }
    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which object?\n\r", ch );
        return;
    }

    /* Find the object. */
    if (!(obj = get_obj_index(atoi(arg))))
    {
      if (!(temp = get_obj_here(ch, arg)))
      {
        send_to_char( "No such object.\n\r", ch );
        return;
      }
      obj = temp->pIndexData;
    }

    if ( !((ch->in_room->area->authors
        && is_name( ch->name, obj->area->authors) && !REAL_GAME)
        || ch->level==MAX_LEVEL))
    {
      send_to_char( "That object is not in your area.\n\r", ch );
      return;
    }
    delete_obj( obj );

    send_to_char( "Object deleted.\n\r", ch );
    return;
}

void do_mdelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *mob;
    CHAR_DATA *temp;
    bool fullcont=FALSE;
    argument = one_argument( argument, arg );


    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME)
        || ch->level==MAX_LEVEL)
      fullcont=TRUE;
   if (!fullcont)
   {
    if (ch->pcdata->castle==NULL)
     {
      send_to_char("You have no castle established yet.\n\r", ch);
      return;
     }
    if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
     {
      send_to_char("You are NOT in your castle!\n\r", ch);
      return;
     }
   }
    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which mob?\n\r", ch );
        return;
    }

    /* Find the mob. */
    if (!(mob = get_mob_index(atoi(arg))))
    {
      if (!(temp = get_char_room(ch, arg)) || !IS_NPC(temp))
      {
        send_to_char( "No such mob.\n\r", ch );
        return;
      }
      mob = temp->pIndexData;
    }
    if ( !fullcont &&  mob->creator_pvnum != ch->pcdata->pvnum)
    {
      send_to_char( "That mob does not belong to you!\n\r", ch );
      return;
    }

    delete_mob( mob );

    send_to_char( "Mob deleted.\n\r", ch );
    return;
}

void do_forceren( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to forcerent, spell it out.\n\r", ch );
    return;
}

void do_forcerent( CHAR_DATA *ch, char *argument )
{
         CLAN_DATA *clan, *next_clan;
	 char buf[MAX_INPUT_LENGTH];
         int rent;
          for ( clan = first_clan; clan; clan = next_clan )
          {
           next_clan = clan->next;

           if (clan->type == CLAN_PEACEFUL)
             rent = RENT_BASIC_ORDER_HALL;
           else
             rent = RENT_BASIC_CLAN_HALL;

           rent +=RENT_PER_GUARD*clan->num_guards;
           rent +=RENT_PER_HEALER*clan->num_healers;
           rent +=RENT_PER_BACKDOOR*clan->num_backdoors;

           if (clan->coffers-rent>0)
           {
            clan->coffers-=rent;
            sprintf(buf, "You paid %d in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
            send_clan_message(clan, buf);
            continue;
           }
           else
           {
            if (clan->healer != 0 && clan->num_healers>0)
            {
             MOB_INDEX_DATA *oldhealer = get_mob_index( clan->healer);
             delete_mob( oldhealer );
             rent-=RENT_PER_HEALER*clan->num_healers;
             clan->coffers+=clan->num_healers*RENT_PER_HEALER/2;
             clan->healer =0;
             clan->num_healers =0;
             if (clan->coffers-rent>0)
             {
              clan->coffers-=rent;
              sprintf(buf, "Your clan's healers were all fired to cover costs.\n\rYou paid %d in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
              send_clan_message(clan, buf);
              continue;
             }
            }
            if (clan->guard != 0 && clan->num_guards>0)
            {
             MOB_INDEX_DATA *oldguard = get_mob_index( clan->guard);
             delete_mob( oldguard );
             rent-=RENT_PER_GUARD*clan->num_guards;
             clan->coffers+=clan->num_guards*RENT_PER_GUARD/2;
             clan->guard =0;
             clan->num_guards =0;
             if (clan->coffers-rent>0)
             {
              clan->coffers-=rent;
              sprintf(buf, "Your clan's guards were all fired to cover costs.\n\rYou paid %d in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
              send_clan_message(clan, buf);
              continue;
             }
            }
          /* They cant pay rent even after liquidation! Better get rid
             of the stinkin' freeloaders! */
	      sprintf(buf, "The clan of %s has been disbanded due to lack of adequate funding!\n\r", clan->name);
             do_echo(NULL, buf);

             destroy_clan(clan);
           }
          }
 return;
}
void do_send_clan_message( CHAR_DATA *ch, char *argument )
{
 CLAN_DATA *clan=NULL;
 char arg[MAX_INPUT_LENGTH];
 argument = one_argument( argument, arg );

 clan = get_clan(arg);
 if (clan == NULL)
 {
  ch_printf(ch, "%s is not a valid clan!\n\r", arg);
  return;
 }
 if (argument[0]=='\0')
 {
  ch_printf(ch, "You should enter a message for %s.\n\r", arg);
  return;
 }
 
 send_clan_message( clan, argument);

 return;
}

void do_rassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  r_lo, r_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    r_lo = atoi( arg2 );  r_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
        send_to_char( "Syntax: rassign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || victim->level < LEVEL_COUNT )
    {
        send_to_char( "They wouldn't know what to do with a room range.\n\r", ch
 );
        return;
    }
    if ( r_lo > r_hi )
    {
        send_to_char( "Unacceptable room range.\n\r", ch );
        return;
    }
    if ( r_lo == 0 )
       r_hi = 0;
    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\n\r",
                ch->name, r_lo, r_hi );
    assign_area( victim );
    do_savearea(victim, "");
    if ( !victim->pcdata->area )
    {
        bug( "rassign: assign_area failed", 0 );
        return;
    }

    return;
}
void do_oassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  o_lo, o_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    o_lo = atol( arg2 );  o_hi = atol( arg3 );

    if ( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
    {
        send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || victim->level < LEVEL_COUNT)
    {
        send_to_char( "They wouldn't know what to do with an object range.\n\r",
 ch );
        return;
    }
    if ( o_lo > o_hi )
    {
        send_to_char( "Unacceptable object range.\n\r", ch );
        return;
    }
    victim->pcdata->o_range_lo = o_lo;
    victim->pcdata->o_range_hi = o_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r",
                ch->name, o_lo, o_hi );
    return;
}

void do_massign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  m_lo, m_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    m_lo = atol( arg2 );  m_hi = atol( arg3 );

    if ( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
    {
        send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || victim->level < LEVEL_COUNT)
    {
        send_to_char( "They wouldn't know what to do with a monster range.\n\r",
 ch );
        return;
    }
    if ( m_lo > m_hi )
    {
        send_to_char( "Unacceptable monster range.\n\r", ch );
        return;
    }
    victim->pcdata->m_range_lo = m_lo;
    victim->pcdata->m_range_hi = m_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r", 	ch->name, m_lo, m_hi );
    return;
}

void set_quest_bits( unsigned char **pointer, unsigned int offset, 
        unsigned int bits, unsigned int value )
  {
  unsigned int cnt;
  unsigned int bytec, byteb, bitval, cbitval;
  unsigned char *pt;

  if( offset>=8*MAX_QUEST_BYTES || bits>32 )
    return;

  pt =(*pointer);

  if( pt==NULL )
    {
    if( value==0 )
      return;
    CREATE( pt, unsigned char, MAX_QUEST_BYTES );
    *pointer=pt;
    }

  /* {
  char buf[MAX_INPUT_LENGTH];
  sprintf( buf, "set_quest_bits at %x %d %d %d", (int)pt, offset, bits, value);
  log_string( buf );
  } */
  
  for( cnt=0; cnt<bits; cnt++, offset++)
    {
    bytec = offset/8;
    byteb = offset%8;
    if( bytec >= MAX_QUEST_BYTES )
      return;
    byteb = 1<<byteb;
    cbitval = (*(pt+bytec)) & byteb;
    bitval = value % 2 ;
    value/=2;
    if( bitval==0 && cbitval!=0 )
      *(pt+bytec)-=byteb;
    else
    if( bitval!=0 && cbitval==0 )
      *(pt+bytec)+=byteb;
    }

  if( !is_quest( pt ) )
    {
    DISPOSE( pt );
    *pointer = NULL;
    }

  return;
  }

int get_quest_bits( unsigned char *pt, unsigned int offset, unsigned int bits )
  {
  unsigned int cnt, value, refer;
  unsigned int bytec, byteb, cbitval, bitcnt;

  if( pt == NULL )
    return( 0 );

  bitcnt = offset;
  refer = 1;
  value=0;
  
  for( cnt=0; cnt<bits; cnt++, bitcnt++)
    {
    bytec = bitcnt /8;
    byteb = bitcnt %8;
    if( bytec >= MAX_QUEST_BYTES )
      return(value);
    byteb = 1<<byteb;
    cbitval = (*(pt+bytec)) & byteb;
    if( cbitval > 0 )
      value += refer;
    refer*=2;
    }
  return(value);
  }

char *quest_bits_to_string( unsigned char *pt)
  {
  char buf[10];
  int cnt;

  *quest_bits_to_out='\0';
  for( cnt=MAX_QUEST_BYTES-1; cnt>=0; cnt--)
    {
    if( pt!=NULL )
      sprintf( buf, "%2X", *(pt+cnt) );
    else
      strcpy( buf, "00" );
    if( *buf==' ' )
      *buf='0';
    strcat( quest_bits_to_out, buf );
    if( cnt%2==0 && cnt>0)
      strcat( quest_bits_to_out, "-" );
    }
  return( quest_bits_to_out );
  }

bool is_quest( register unsigned char *pt )
{
  register unsigned char cnt;
  if( pt==NULL )
    return( FALSE );
  for(cnt=0; cnt<MAX_QUEST_BYTES; cnt++)
    if(*(pt+cnt)!=0)
      return( TRUE );
  return( FALSE );
}

void do_resetquest( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int vnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: resetquest <victim> <room vnum>\n\r",     ch );
	return;
    }


    if( ch->level < LEVEL_DUKE )
      {
        send_to_char( "You cannot do that.\n\r", ch );
        return;
      }

    vnum = atoi( arg2 );
    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      {
        send_to_char( "That is not a valid room.\n\r", ch );
        return;
      }

  if((victim = get_char_world(ch, arg1)) == NULL)
    {
      send_to_char("They aren't logged in.\n\r", ch);
      return;
    }

    if(IS_NPC(victim))
      {
        send_to_char( "You cannot do that to a mobile.\n\r", ch );
        return;
      }
    else
      {
      DISPOSE(victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100]);
      victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100]=NULL;
      }
    send_to_char( "Reset.\n\r", ch );
    return;
    }
