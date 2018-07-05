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

/*
 * Global functions.
 */
OBJ_DATA *get_obj_carry_vnum args( ( CHAR_DATA *ch, sh_int vnum) );

/*
 * Local functions.
 */
#define CD CHAR_DATA

bool    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container, bool fDisplay ) );
bool    wear_obj        args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace,
				int location , bool fDisplay) );
int     get_cost        args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
OBJ_DATA * find_char_corpse(CHAR_DATA *, bool );
#undef  CD

/*void sac_obj( OBJ_DATA *);*/
bool class_allowed(OBJ_DATA *obj, CHAR_DATA *ch, bool fDisplay)
{
 /*    CLASS distinction for wielding weapons   -   Chaos  10/11/93  */
       if( obj->pIndexData->class_flags == 0)
        {
	if(ch->class!=CLASS_RANGER && (obj->value[3]!=4 || ch->mclass[CLASS_ASSASSIN]==0))
	switch(ch->class)
	  {
	  case CLASS_ILLUSIONIST:
	  case CLASS_NECROMANCER:
	    if( obj->value[3]<2 || obj->value[3]==3 || obj->value[3]==5 ||
		obj->value[3]==6 || obj->value[3]==9 || obj->value[3]==10 ||
		obj->value[3]>11 )
	       {
               if( fDisplay )
	        send_to_char( "You are the wrong class to wield that.\n\r", ch);
	       return FALSE;
	       }
	    break;
	  case CLASS_ELEMENTALIST:
	    if( obj->value[3]<4 || obj->value[3]==5 || obj->value[3]>8)
	      {
               if( fDisplay )
	        send_to_char( "You are the wrong class to wield that.\n\r", ch);
	       return FALSE;
	      }
	    break;
	  case CLASS_ROGUE:
	  case CLASS_ASSASSIN:
	    if( obj->value[3]<1 || ( obj->value[3]>3 && obj->value[3]!=11 ))
	      {
               if( fDisplay )
	        send_to_char( "You are the wrong class to wield that.\n\r", ch);
	       return FALSE;
	      }
	    break;
	  case CLASS_MONK:
      if( fDisplay )
	    send_to_char( "Monks may not wield weapons.\n\r", ch);
	    return FALSE;
	  }
       }
     else
       if( !IS_NPC(ch) && !IS_SET( obj->pIndexData->class_flags, 1<<ch->class))
	      {
              if( fDisplay )
	        send_to_char( "You are the wrong class to wield that.\n\r", ch);
	      return FALSE;
	      }
 return TRUE;
}

bool could_dual( CHAR_DATA *ch )
{
    if( !IS_NPC(ch) && ch->class != CLASS_RANGER )
      return( FALSE );
    if ( IS_NPC(ch) || ch->pcdata->learned[gsn_dual_wield] )
        return TRUE;

    return FALSE;
}
bool can_dual( CHAR_DATA *ch )
{
    if ( !could_dual(ch) )
        return FALSE;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
        send_to_char( "You are already wielding two weapons!\n\r", ch );
        return FALSE;
    }
    if ( get_eq_char( ch, WEAR_SHIELD ) )
    {
        send_to_char( "You cannot dual wield while holding a shield!\n\r", ch );
        return FALSE;
    }
    if ( get_eq_char( ch, WEAR_HOLD ) )
    {
        send_to_char( "You cannot dual wield while holding something!\n\r", ch );
        return FALSE;
    }
    return TRUE;
}

bool is_obj_in_room( OBJ_DATA *obj, ROOM_INDEX_DATA *room )
    {
          OBJ_DATA *fobj;
          bool  foundit;
     if( room == NULL || obj == NULL )
       return( FALSE );

          foundit=FALSE;
          for( fobj=room->first_content; fobj!=NULL; fobj=fobj->next_content)
            if( fobj == obj )
              foundit = TRUE;
      return( foundit );
    }

bool get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay)
{
  int objCount;

  if( IS_NPC(ch) || ch->pcdata->corpse!=container || container==NULL)
    { 
  if ( !CAN_WEAR(obj, ITEM_TAKE)  && (!IS_IMMORTAL(ch) || IS_NPC(ch)) )
    { 
	if( fDisplay )
	  send_to_char( "You can't take that.\n\r", ch );
	  return FALSE;
    }

  if(obj->in_room!=NULL || (obj->in_obj!=NULL && obj->in_obj->in_room!=NULL))
    objCount=((ch->first_carrying==NULL)?0:
	      count_obj_list(obj->pIndexData,ch->first_carrying))+1+
	     ((obj->first_content==NULL)?0:
	      count_obj_list(obj->pIndexData,obj->first_content));
  else
    objCount=0;

  if(obj->in_room!=NULL)
    obj->sac_timer=OBJ_SAC_TIME;
    
  if(UMAX(obj->pIndexData->max_objs,1)<objCount)
    {
    act("$p slips easily out of your grasp.",ch,obj,NULL,TO_CHAR);
    act("$p slips easily out of $n's grasp.",ch,obj,NULL,TO_ROOM);
	if( fDisplay )
    send_to_char("You must not be able to carry any more items like that.\n\r",ch);
    return FALSE;
    }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	if( fDisplay )
	  act( "$d: you can't carry that many items.",
	      ch, NULL, obj->name, TO_CHAR );
	  return FALSE;
    }

  if(container==NULL || container->carried_by!=ch)
    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
      {
	if( fDisplay )
	    act( "$d: you can't carry that much weight.",
		ch, NULL, obj->name, TO_CHAR );
      return FALSE;
      }
  }

  obj->sac_timer=0;
  if ( container != NULL )
    {
	if( fDisplay )
	  act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	if( fDisplay )
	  act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	  obj_from_obj( obj );
    }
  else
    {
	if( fDisplay )
	  act( "You get $p.", ch, obj, container, TO_CHAR );
	if( fDisplay )
	  act( "$n gets $p.", ch, obj, container, TO_ROOM );
	  obj_from_room( obj );
    }

  if ( obj->item_type == ITEM_MONEY )
    {
       give_gold (ch, obj->value[0]);
       extract_obj( obj );
    }
  else
    {
	  obj_to_char( obj, ch );
    }
  return TRUE;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found, spl, pc_corpse, multi_obj;
    char buft[MAX_INPUT_LENGTH];
    int goldprev, cnt, amount;
    int money;


    obj=NULL;
    container=NULL;
    spl=FALSE;
    multi_obj=FALSE;
    if( argument == NULL )
      {
      strcpy( buft, "all corpse");
      argument=buft;
      spl=TRUE;
      }
    if(!IS_NPC(ch) && ch->in_room->vnum==ch->pcdata->corpse_room
	 && ch->pcdata->corpse!=NULL)
      {
      spl=FALSE;
      pc_corpse=TRUE;
      }
    else
      pc_corpse=FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* Get type. */
    if ( arg1[0] == '\0' && !pc_corpse)
      {
	    send_to_char( "Get what?\n\r", ch );
	    return;
      }

    if( is_number( arg1 ) && arg2[0]!='\0' && strcasecmp( arg2, "coin") &&
	strcasecmp( arg2, "coins"))
      {
      multi_obj=TRUE;
      amount=UMAX( 1, atol( arg1));
      strcpy( arg1, arg2);
      arg2[0]='\0';
      }
    else
      amount=1;

    if ( (arg2==NULL || arg2[0] == '\0') && !pc_corpse)
      {

    if( !IS_NPC( ch ) && ch->fighting != NULL )
      {
      send_to_char( "You are too busy to pick anything up.\n\r", ch );
      return;
      }

	    if ( strcasecmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	      {
	bool got_all;
	int amount_got;
	got_all=TRUE;
	amount_got=0;
	      /* 'get obj' */
	for( cnt=0;cnt<amount;cnt++)
	  {
		obj = get_obj_list( ch, arg1, ch->in_room->first_content );
		if ( obj == NULL )
		  {
			act( "You see no $T here.", ch, NULL, arg1, TO_CHAR );
			return;
		  }
	    
    
          money = -1;
	  if ( obj->item_type == ITEM_MONEY )
             money = obj->value[0];
	  if(get_obj( ch, obj, NULL, FALSE))
	    {
	    if ( money >= 0 )
	      {
	      char tbuf[180];
	      sprintf( tbuf, "You get %d coins.\n\r", money);
	      send_to_char( tbuf, ch);
	      sprintf( tbuf, "%s gets %d coins.", get_name( ch ), money);
	      act( tbuf, ch, NULL, NULL, TO_ROOM );
	      amount=0;
	      }
	    amount_got+=1;
	    if( amount==1)
	      {
		    act( "You get $p.", ch, obj, NULL, TO_CHAR );
		    act( "$n gets $p.", ch, obj, NULL, TO_ROOM );
	      }
	    }
	  else
	    got_all=FALSE;
	  }
	if(got_all && amount>1)
	  {
	  char tbuf[100], qbuf[80];
	  strcpy( qbuf, obj->name);
	  for( cnt=0; cnt<strlen( qbuf) && qbuf[cnt]!=' '; cnt++);
	  qbuf[cnt]='\0';
	  sprintf( tbuf, "%s gets %d %ss.", get_name( ch ),
		   amount, qbuf);
		act( tbuf, ch, NULL, NULL, TO_ROOM );
	  sprintf( tbuf, "You get %d %ss.\n\r", amount, qbuf);
	  send_to_char( tbuf, ch);
	  }
	else if (amount>1)
	  {
	  char tbuf[100];
	  sprintf( tbuf, "You got only %d items.\n\r", amount_got);
	  send_to_char( tbuf, ch);
	  }
	}
	    else
	      {
	      /* 'get all' or 'get all.obj' */
	      int num_not_got;

	      num_not_got=0;
	      found = FALSE;
	      for ( obj = ch->in_room->first_content; obj != NULL; obj = obj_next )
		{
		      obj_next = obj->next_content;
		      if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
			  &&   can_see_obj( ch, obj ) )
			{
			found = TRUE;
			if(!get_obj( ch, obj, NULL , FALSE))
			  num_not_got+=1;
			}
		}

	      if ( !found ) 
		{
		      if ( arg1[3] == '\0' )
			send_to_char( "You see nothing here.\n\r", ch );
		      else
			act( "You see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
		}
	else
	  {
	  if(num_not_got!=0)
	    {
	    char tmpbuf[MAX_INPUT_LENGTH];

	    sprintf(tmpbuf,"You get all but %d of the items.\n\r",num_not_got);
	    send_to_char(tmpbuf,ch);
	    sprintf(tmpbuf,"$n gets some items." );
	    act(tmpbuf,ch,NULL,NULL,TO_ROOM);
	    }
	  else if ( arg1[3] == '\0' )
	    {
	    send_to_char( "You get everything in the room.\n\r", ch );
	    act( "$n gets everything in the room.", ch, NULL, NULL, TO_ROOM);
	    }
	  else
	    {
	    act( "You get all the $Ts here.",ch,NULL,&arg1[4],TO_CHAR);
	    act( "$n gets all the $Ts here.", ch, NULL, &arg1[4], TO_ROOM );
	    }
	  }
	}
      }
    else
      {
	    /* 'get ... container' */
	    /*if ( !strcasecmp( arg2, "all" ) || !str_prefix( "all.", arg2 ))
	      {
	      send_to_char( "You can't do that.\n\r", ch );
	      return;
	      }  */

	    if ( !pc_corpse &&( container = get_obj_here( ch, arg2 ) ) == NULL )
	      {
	      act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
	      return;
	      }
      if(pc_corpse)
	container=ch->pcdata->corpse;
      else
    if( container->carried_by == NULL && !IS_NPC( ch ) && ch->fighting != NULL )
      {
      send_to_char( "You are too busy to pick anything up.\n\r", ch );
      return;
      }


	    switch ( container->item_type )
	      {
	      default:
		send_to_char( "That's not a container.\n\r", ch );
		return;

	      case ITEM_CONTAINER:
	  break;

	      case ITEM_CORPSE_NPC:
                  /* Allow Police  */
	  if(container->owned_by!=-1 &&
             !pvnum_in_group( ch, container->owned_by) &&
             get_trust( ch ) < MAX_LEVEL )
	    {
	    send_to_char( "You have no right to those items!\n\r", ch);
	    return;
	    }
		break;

	      case ITEM_CORPSE_PC:
               if( get_trust( ch ) < MAX_LEVEL )
		{
		      char name[MAX_INPUT_LENGTH];
		      char *pd;

		      if ( IS_NPC(ch) )
			{
			send_to_char( "You can't do that.\n\r", ch );
			return;
			}
            

		      pd = container->short_descr;
		      pd = one_argument( pd, name );
		      pd = one_argument( pd, name );
		      pd = one_argument( pd, name );

	  if( strcasecmp( name, ch->name) && !pc_corpse)
			{
			      send_to_char( "You can't do that.\n\r", ch );
			      return;
			}
		      }
	      }

	  if ( IS_SET(container->value[1], CONT_CLOSED) && !pc_corpse)
	    {
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	    }

	  if ( strcasecmp( arg1, "all" ) && str_prefix( "all.", arg1 ) && !pc_corpse)
	    {
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->first_content );
	    if ( obj == NULL )
	      {
		    act( "You see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
		    return;
	      }
	    get_obj( ch, obj, container, TRUE );
	    }
	  else
	    {
	    /* 'get all container' or 'get all.obj container' */
	    int num_not_got;

	    num_not_got=0;
	    found = FALSE;
      goldprev=ch->gold;
	    for ( obj = container->first_content; obj != NULL; obj = obj_next )
	      {
		    obj_next = obj->next_content;
		    if ((( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
			&& can_see_obj( ch, obj ) ) || pc_corpse)
	  if((!IS_SET( ch->act, PLR_AUTOLOOT) && spl &&
	      IS_SET( ch->act, PLR_AUTO_SPLIT) && obj->item_type==ITEM_MONEY)
	      || (IS_SET( ch->act, PLR_AUTOLOOT) && spl) || !spl)
			{
			found = TRUE;
			if(!get_obj( ch, obj, container , FALSE))
			  num_not_got+=1;
			}
	      }

      if ( !found )
	{
	if ( arg1[3] == '\0' )
	  act( "You see nothing in the $T.", ch, NULL, arg2, TO_CHAR );
	else
	  act("You see nothing like that in the $T.",ch,NULL, arg2, TO_CHAR );
	}
      else
	{
	if(num_not_got!=0)
	  {
	  char tmpbuf[MAX_INPUT_LENGTH];

	  sprintf(tmpbuf,"You get all but %d of the items from $p.",num_not_got);
	  act(tmpbuf,ch,container,NULL,TO_CHAR);
	  sprintf(tmpbuf,"$n gets some items from $p.");
	  act(tmpbuf,ch,container,NULL,TO_ROOM);
	  }
	else if ( !strcasecmp( arg1, "all" )  || pc_corpse)
	  {
	  act( "You get everything from $p.",ch, container, NULL, TO_CHAR );
	  act( "$n gets everything from $p.",ch, container, NULL, TO_ROOM );
	  }
	else
	  {
	  act( "You get all '$T' from $p.",ch, container, &arg1[4], TO_CHAR );
	  act( "$n gets all '$T' from $p.",ch, container, &arg1[4], TO_ROOM );
	  }
	if( IS_SET( ch->act, PLR_AUTO_SPLIT) && ch->gold!=goldprev && spl)
	  {
          CHAR_DATA *gch;
          int members;
          members = 0;
    for ( gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room )
            {
	    if ( is_same_group( gch, ch ) && !IS_NPC( gch ))
	    members++;
            }

          if ( members > 1 )
            {
	    sprintf( buft, "%d", ch->gold-goldprev);
	    do_split( ch, buft );
            }
	  }
	}
      }
    }

  if( pc_corpse )
    {
    ch->pcdata->corpse=NULL;
    obj_from_room( container);
    extract_obj( container);
    ch->pcdata->corpse=find_char_corpse( ch, TRUE); 
    save_char_obj(ch, NORMAL_SAVE); 
    }

    return;
}

int count_total_objects( OBJ_DATA *container )
  {
  int count;
  OBJ_DATA *obj;
  
  for( count=0, obj=container->first_content; obj!=NULL; obj=obj->next_content)
    {
    count++;
    /* if( obj->item_type == ITEM_CONTAINER)
	 count += count_total_objects( obj ); */
    }
  return( count );
  }

void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int objCount;
   
    found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( strcasecmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

  objCount=((obj->first_content==NULL)?0:
	    count_obj_list(obj->pIndexData,container->first_content));
	if(get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] ||
             IS_SET(obj->extra_flags, ITEM_INVENTORY) ||
            (obj->pIndexData->max_objs<objCount))
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}

  if( count_total_objects(container) >= MAX_OBJECTS_IN_CONTAINER)
     {
     send_to_char( "There are too many items there already.\n\r", ch);
     return;
     }

  if( obj->item_type == ITEM_CONTAINER)
     {
     send_to_char( "You cannot put a container in a container.\n\r", ch);
     return;
     }

  if((ch->level+5)<obj->level)
	  {
	  send_to_char( "You wouldn't want to lose your stuff, would you?\n\r", ch );
	  return;
	  }

	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	int num_not_put=0;
	objCount = count_total_objects( container );

	for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
	  {
	  obj_next = obj->next_content;

	  if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
            &&   !IS_SET(obj->extra_flags, ITEM_INVENTORY)
            &&   obj->item_type != ITEM_CONTAINER
	    &&   can_drop_obj( ch, obj ))
	    {
	    if(get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0]  &&
	       objCount < MAX_OBJECTS_IN_CONTAINER )
	      {
	      found=TRUE;
	      obj_from_char( obj );
	      obj_to_obj( obj, container );
	      objCount++;
	      }
	    else
	      num_not_put+=1;
	    }
	  }
      if( found )
	{
	if(num_not_put!=0)
	  {
	  char tmpbuf[MAX_INPUT_LENGTH];

	  sprintf(tmpbuf,"You put all but %d of the items into $p.",num_not_put);
	  act(tmpbuf,ch,container,NULL,TO_CHAR);
	  act("$n puts items into $p.",ch,container,NULL,TO_ROOM);
	  }
	else if ( !strcasecmp( arg1, "all" ) )
	  {
	  act( "$n puts everything in $P.", ch, obj, container, TO_ROOM );
	  act( "You put everything in $P.", ch, obj, container, TO_CHAR );
	  }
	else
	  {
	  act( "You put all '$T' into $p.",ch, container, &arg1[4], TO_CHAR );
	  act( "$n puts all '$T' into $p.",ch, container, &arg1[4], TO_ROOM );
	  }
	}
      else
	act( "You don't have $T.", ch, obj, arg1, TO_CHAR );

    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *found_obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg );

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    obj=NULL;
    if ( is_number( arg2 ) )
      if( !strcasecmp( arg, "coin") || !strcasecmp( arg, "coins"))
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

  if( arg2[0]!='\0' && arg[0]=='\0')
    {
    strcpy( arg, arg2);
    if ( strcasecmp( arg, "all" ) && str_prefix( "all.", arg ) )
      {
	/* 'drop obj' */
      if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	send_to_char( "You do not have that item.\n\r", ch );
	return;
	}

      if ( !can_drop_obj( ch, obj ) )
	{
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
	}

      obj_from_char( obj );
      obj->sac_timer=OBJ_SAC_TIME;
      act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
      act( "You drop $p.", ch, obj, NULL, TO_CHAR );
      obj_to_room( obj, ch->in_room );
      }
    else
      {
	/* 'drop all' or 'drop all.obj' */
      found = FALSE;
      found_obj=NULL;
      for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
	{
	obj_next = obj->next_content;

	if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	  {
	  found = TRUE;
	  found_obj=obj;
	  obj_from_char( obj );
	  obj->sac_timer=OBJ_SAC_TIME;
	  obj_to_room( obj, ch->in_room );
	  }
	}
       if(!found)
	 {
	 for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
	  {
	  obj_next = obj->next_content;

	  if ( ( arg[3] == '\0' || is_name_short( &arg[4], obj->name ) )
	     &&   can_see_obj( ch, obj )
	     &&   obj->wear_loc == WEAR_NONE
	     &&   can_drop_obj( ch, obj ) )
	   {
	   found = TRUE;
	   found_obj=obj;
	   obj_from_char( obj );
	   obj->sac_timer=OBJ_SAC_TIME;
	   obj_to_room( obj, ch->in_room );
	   }
	 }
       } 
       if( found )
	 {
	 if( arg[3]=='\0')
          {
	  if( is_obj_in_room( found_obj, ch->in_room ) )
	  {
	  act( "$n drops everything.", ch, obj, NULL, TO_ROOM );
	  act( "You drop everything.", ch, obj, NULL, TO_CHAR );
	  }
	  }
	 else
	  if( is_obj_in_room( found_obj, ch->in_room ) )
          {
	  char tbuf[100], qbuf[80];
	  int cnt;
	  strcpy( qbuf, found_obj->name);
	  for( cnt=0; cnt<strlen( qbuf) && qbuf[cnt]!=' '; cnt++);
	  qbuf[cnt]='\0';
	  sprintf( tbuf, "%s drops all %ss.", get_name( ch ), qbuf);
	  act( tbuf, ch, obj, NULL, TO_ROOM );
	  sprintf( tbuf, "You drop all %ss.", qbuf);
	  act( tbuf, ch, obj, NULL, TO_CHAR );
	  }
	 }

	if ( !found )
	  {
	  if ( arg[3] == '\0' )
	    act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
	  else
	    act( "You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR );
	  }
	}
      }
  else
    if( is_number( arg2 ) && arg[0]!='\0')
      {
      int cnt, amount;
      amount=atol( arg2);
      for( cnt=0; cnt<amount; cnt++)
	{
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	  {
	  send_to_char( "You do not have that many.\n\r", ch );
	  break;
	  }

	if ( !can_drop_obj( ch, obj ) )
	  {
	  send_to_char( "You can't let go of it.\n\r", ch );
	  return;
	  }

	obj_from_char( obj );
	obj->sac_timer=OBJ_SAC_TIME;
	obj_to_room( obj, ch->in_room );
	}
      if( obj !=NULL && is_obj_in_room( obj, ch->in_room ) )
	{
	char tbuf[100], qbuf[80];
	int cnt;
	strcpy( qbuf, obj->name);
	for( cnt=0; cnt<strlen( qbuf) && qbuf[cnt]!=' '; cnt++);
	qbuf[cnt]='\0';
	sprintf( tbuf, "%s drops %d %ss.", get_name( ch ),
	    amount, qbuf);
	act( tbuf, ch, NULL, NULL, TO_ROOM );
	sprintf( tbuf, "You drop %d %ss.\n\r", amount, qbuf);
	send_to_char( tbuf, ch);
	}
      }
  return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    int objCount;

    if( !IS_NPC( ch ) && ch->fighting != NULL  && !IS_NPC( ch->fighting->who ))
      {
      send_to_char( "You are too busy to give anything away.\n\r", ch );
      return;
      }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount, amount_before;

	amount   = atol(arg1);
	if ( amount <= 0
	|| ( strcasecmp( arg2, "coins" ) && strcasecmp( arg2, "coin" ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    if( !IS_NPC( victim ) && victim->fighting != NULL &&
               !IS_NPC( victim->fighting->who ) )
      {
      send_to_char( "They are too busy to take anything right now.\n\r", ch );
      return;
      }
    if ( find_keeper( ch ) == victim )
      {
      send_to_char( "You may only sell items to a shop keeper.\n\r", ch );
      return;
      }
      

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}
  if( amount> victim->level*victim->level*10 && !IS_NPC(victim))
    {
    send_to_char( "You cannot give that much gold to that level of a character.\n\r", ch);
    amount=victim->level*victim->level*10;
    }

	ch->gold     -= amount;
	amount_before = victim->gold;
	victim->gold += amount;
	MOBtrigger=FALSE;
	sprintf(buf,"$n gives you %d gold coins.",amount);
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d gold coins.",amount);
	act( buf, ch, NULL, victim, TO_CHAR    );
	/*MOBtrigger=TRUE;*/
	send_to_char( "OK.\n\r", ch );
	  mprog_bribe_trigger(victim,ch,amount);
	if( IS_NPC( victim ))
	  victim->gold=amount_before;  /* reset the gold of the mobile */
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) && !IS_NPC(ch))
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

  objCount=((victim->first_carrying==NULL)?0:
	    count_obj_list(obj->pIndexData,victim->first_carrying))+1+
	   ((obj->first_content==NULL)?0:
	    count_obj_list(obj->pIndexData,obj->first_content));

  if( get_trust( ch ) < MAX_LEVEL )
    if((victim->carry_number + get_obj_number( obj ) > can_carry_n( victim))||
       (obj->pIndexData->max_objs<objCount))
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

  if( get_trust( ch ) < MAX_LEVEL && !IS_NPC( ch ) )
    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

  if( get_trust( ch ) < MAX_LEVEL && !IS_NPC(ch))
    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }
    if( !IS_NPC( victim ) )
      if( IS_NPC( ch) || (!IS_NPC( ch) && IS_IMMORTAL(ch)))
       if(obj->pIndexData->vnum>=50 && obj->pIndexData->vnum<=55)
        {
/*
         for ( oldchain = ch->first_carrying; oldchain != NULL; 
	        oldchain = oldchain->next_content )
         {
	  if (!oldchain->pIndexData)
	   continue;
          if ( (oldchain->pIndexData->vnum==50 ||
               oldchain->pIndexData->vnum==51 ||
               oldchain->pIndexData->vnum==52 ||
               oldchain->pIndexData->vnum==53 )
                && oldchain!=obj )
             {
	       if( oldchain->carried_by!=NULL && oldchain->wear_loc!=WEAR_NONE )
                 remove_obj(oldchain->carried_by,oldchain->wear_loc,TRUE, FALSE);
               extract_obj(oldchain); 
               equip_char( ch, obj, WEAR_HEART );
             }
         }   
*/
       /* Initiates cannot accept chains  -  Chaos 4/17/99  */
    if( which_god(victim)==GOD_INIT_CHAOS || which_god(victim)==GOD_INIT_ORDER)
      return;

        victim->which_god=-1;
        obj->owned_by = victim->pcdata->pvnum;
        }

    obj_from_char( obj );
    obj_to_char( obj, victim );
      /* auto engrave objects to players from gods */
    if( !IS_NPC( ch) && !IS_NPC( victim) && IS_IMMORTAL(ch))
      obj->owned_by = victim->pcdata->pvnum;
    MOBtrigger=FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    /*MOBtrigger=TRUE;*/
    if(!IS_OBJ_STAT(obj,ITEM_FORGERY))
    {
       mprog_give_trigger(victim,ch,obj);
    }
    return;
}




void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->first_content; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    return;
}



void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    obj->basic = FALSE;
    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = 48;
	act( "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You are no longer thirsty.\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );


	amount = number_range(3, 10);
	amount = UMIN(amount, obj->value[1]);
	
	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	    send_to_char( "You are full.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );
	
	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned ! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );
	    af.type      = gsn_poison;
	    af.duration  = 3 * amount;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	
	obj->value[1] -= amount;
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "The empty container vanishes.\n\r", ch );
	    extract_obj( obj );
	}
	break;
    }

    return;
}


   /* Forage for Gnomes - Chaos 8/17/98 */
void do_forage( CHAR_DATA *ch, char *argument )
  {

    if( IS_NPC(ch) ||  ch->race!=RACE_GNOME )
      {
      send_to_char( "Your parents never taught you to find your own food.\n\r",
        ch );
      return;
      }

    if( ch->in_room==NULL ||
        ( ch->in_room->sector_type!=SECT_FIELD &&
          ch->in_room->sector_type!=SECT_FOREST &&
          ch->in_room->sector_type!=SECT_HILLS &&
          ch->in_room->sector_type!=SECT_MOUNTAIN  ))
      {
      send_to_char( "There is no food to be found here.\n\r",
        ch );
      return;
      }

    send_to_char( "You find enough food and water to satisfy yourself.\n\r", ch );

    ch->pcdata->condition[COND_FULL] = 25;
    ch->pcdata->condition[COND_THIRST] = 30;

  return;
  }
void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 &&
             obj->item_type != ITEM_PILL )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );

    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    obj->basic = FALSE;

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.type      = gsn_poison;
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
    if( ch->class != CLASS_ASSASSIN && 
        ( ch->fighting!=NULL || ch->position == POS_FIGHTING ) )
          {
          send_to_char( "You may not do that while fighting.\n\r", ch);
          return;
          }
    if( ch->class == CLASS_ELEMENTALIST ||
        ch->class == CLASS_ILLUSIONIST ||
        ch->class == CLASS_NECROMANCER ||
        ch->class == CLASS_MONK )
      {
      send_to_char( "You may not do that in your class.\n\r", ch);
      return;
      }
    if( obj->level > ch->level )
      {
      send_to_char( "You would over-dose if you ate that pill!\n\r", ch);
      return;
      }

	  obj_cast_spell( obj->value[1], obj->value[0], ch, ch, obj );
	  obj_cast_spell( obj->value[2], obj->value[0], ch, ch, obj );
	  obj_cast_spell( obj->value[3], obj->value[0], ch, ch, obj );

	WAIT_STATE( ch, PULSE_VIOLENCE*2);
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace , bool fDisplay )
{
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace
    &&   ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( "$d: you can't carry that many items.",
            ch, NULL, obj->name, TO_CHAR );
        return FALSE;
    }

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
     if( fDisplay )
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }
    if ( obj == get_eq_char( ch, WEAR_WIELD )
    && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    unequip_char( ch, obj );
     if( fDisplay )
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
     if( fDisplay )
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
bool wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int location ,
		bool fDisplay )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj2, *tmpobj;

    if ( ch->level < obj->level )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
      if( fDisplay )
	send_to_char( buf, ch );
      if( fDisplay )
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return FALSE;
    }
  /*if(IS_OBJ_STAT(obj,ITEM_MAGIC))*/
    if((obj2=get_obj_wear_vnum(ch,obj->pIndexData->vnum))!=NULL)
      if(obj2->pIndexData==obj->pIndexData)
	{
      if( fDisplay )
	      act("Wearing more than one of $p will not help you.",
		  ch, obj, NULL, TO_CHAR );
	return FALSE;
	}
    if(obj->owned_by!=0)
      if( IS_NPC( ch) || ch->pcdata->pvnum!=obj->owned_by)
	{
	if( fDisplay )
	  send_to_char( "You cannot wear someone else's equipment!\n\r", ch);
	return FALSE;
	}

      

    if ((location==ITEM_TAKE||location==-1) &&obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return TRUE;
    }

    if ((location==ITEM_WEAR_HEART||location==-1)&&
        (obj->pIndexData->vnum>=50 && obj->pIndexData->vnum<=52))
      {
      if ( !remove_obj( ch, WEAR_HEART, fReplace , fDisplay) )
        return FALSE;
      if( fDisplay )
        act( "$n wears $p in $s heart.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
        act( "You wear $p in your heart.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_HEART );
      return TRUE;
      }

    if ((location==ITEM_WEAR_FINGER||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_FINGER ) && ch->race != RACE_TSARIAN)
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace , fDisplay)
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace , fDisplay) )
	    return FALSE;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return TRUE;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return TRUE;
	}

	bug( "Wear_obj: no free finger.", 0 );
      if( fDisplay )
	send_to_char( "You already wear two rings.\n\r", ch );
	return FALSE;
    }

    if ((location==ITEM_WEAR_NECK||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_NECK ) && (obj->pIndexData->vnum<50 || obj->pIndexData->vnum>52))
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace , fDisplay)
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace , fDisplay) )
	    return FALSE;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return TRUE;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return TRUE;
	}

	bug( "Wear_obj: no free neck.", 0 );
      if( fDisplay )
	send_to_char( "You already wear two neck items.\n\r", ch );
	return FALSE;
    }

    if ((location==ITEM_WEAR_BODY||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return TRUE;
    }

    if ((location==ITEM_WEAR_HEAD||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return TRUE;
    }

    if ((location==ITEM_WEAR_LEGS||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_LEGS ) && ch->race != RACE_CENTAUR)
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace , fDisplay) )
	    return FALSE;
       /*  if( !IS_NPC( ch ) )
        if( ch->class == 5 )
	  {
	  if( fDisplay )
	    send_to_char( "You cannot use that kind or armor.\n\r", ch);
	  return FALSE;
	  }  */
      if( fDisplay )
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return TRUE;
    }

    if ((location==ITEM_WEAR_FEET||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_FEET ) && ch->race != RACE_CENTAUR)
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return TRUE;
    }

    if ((location==ITEM_WEAR_HANDS||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return TRUE;
    }

    if ((location==ITEM_WEAR_ARMS||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace , fDisplay) )
	    return FALSE;
       if( !IS_NPC( ch ) )
        if( ch->class == 4 /* || ch->class == 5 */ )
	  {
	  if( fDisplay )
	    send_to_char( "You cannot use that kind or armor.\n\r", ch);
	  return FALSE;
	  }
      if( fDisplay )
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return TRUE;
    }

    if ((location==ITEM_WEAR_ABOUT||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return TRUE;
    }

    if ((location==ITEM_WEAR_WAIST||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return TRUE;
    }

    if ((location==ITEM_WEAR_WRIST||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace , fDisplay)
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace , fDisplay) )
	    return FALSE;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return TRUE;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
      if( fDisplay )
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return TRUE;
	}

	bug( "Wear_obj: no free wrist.", 0 );
      if( fDisplay )
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return FALSE;
    }

    if ((location==ITEM_WEAR_SHIELD||location==-1)&& CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
        if (get_eq_char(ch, WEAR_DUAL_WIELD))
        {
         if (fDisplay)
          send_to_char("You can't use a shield AND two weapons!\n\r", ch);
         return FALSE;
        }
	if ( !remove_obj( ch, WEAR_SHIELD, fReplace , fDisplay) )
	    return FALSE;
       if( !IS_NPC( ch ) )
        if( ch->class == 0 || ch->class == 6 || ch->class == 4 )
               /* || ch->class==5) */
	  {
	  if( fDisplay )
	    send_to_char( "You cannot use that kind or armor.\n\r", ch);
	  return FALSE;
	  }

      if( fDisplay )
	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return TRUE;
    }

    if ((location==ITEM_WIELD||location==-1)&& CAN_WEAR( obj, ITEM_WIELD ) )
    {
        if ( !could_dual(ch) )
        {
         if ( !remove_obj(ch, WEAR_WIELD, fReplace, fDisplay) )
          return FALSE;
         tmpobj = NULL;
        }
       else
        {
         if ( (tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL 
              && get_eq_char(ch, WEAR_DUAL_WIELD) )
          {
            if (fDisplay)
             send_to_char( "You're already wielding two weapons.\n\r", ch);
           return FALSE;
          }
         }
    if ( tmpobj )
    {
     if ( can_dual(ch) )
      {
       if ( get_obj_weight(obj) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield )
        {
         if (fDisplay)
          send_to_char( "It is too heavy for you to wield.\n\r", ch );
         return FALSE;
        }
        if (!class_allowed(obj, ch, fDisplay) )
         return FALSE;
        if (fDisplay)
        {
         act( "$n dual-wields $p.", ch, obj, NULL, TO_ROOM );
         act( "You dual-wield $p.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_DUAL_WIELD );
       }
      return TRUE;
     }

	if ( get_obj_weight( obj ) > str_app[get_curr_str(ch)].wield )
	{
      if( fDisplay )
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return FALSE;
	}
      if (!class_allowed(obj, ch, fDisplay) )
         return FALSE;

      if( fDisplay )
	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );
	return TRUE;
    }

    if ((location==ITEM_HOLD||location==-1)&& CAN_WEAR( obj, ITEM_HOLD ) )
    {
     if (get_eq_char( ch, WEAR_DUAL_WIELD) )
       { 
        if (fDisplay)
         send_to_char("You cannot hold something AND two weapons!\n\r", ch);
        return FALSE;
       }
	if ( !remove_obj( ch, WEAR_HOLD, fReplace , fDisplay) )
	    return FALSE;
      if( fDisplay )
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
      if( fDisplay )
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return TRUE;
    }

    if(fReplace&&location!=-1 && fDisplay )
      send_to_char("You can't wear, wield, or hold that in that location.\n\r",
		   ch);
    else if ( fReplace && fDisplay)
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return FALSE;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char local[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int location;

    argument=one_argument( argument, arg );
    one_argument( argument, local );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next, *obj_old;
	bool foundi, tfound;
	int loc;

	foundi=FALSE;
	obj_old = NULL;
	for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
	{   /* Now always picks best    - Chaos 7/11/94   */
	    obj_next = obj->next_content;
	    loc=0;
	    tfound = FALSE;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	    while( loc != -1 && !tfound )
	      {
	      loc = wear_flags_to_wear_loc( obj, loc);
	      if( loc != -1)
		{
		obj_old = get_eq_char( ch, loc );
		if ( obj_old == NULL || compare_obj( obj, obj_old) < 0 ) 
		  {
		  if( wear_obj( ch, obj, TRUE, -1 , FALSE))
		    {
		    foundi = TRUE;
		    tfound = TRUE;
		    }
		  }
		loc++;
		}
	      }
	}
	if( foundi )
	  {
	  act( "You wear the best equipment that you have.",ch,NULL,NULL,TO_CHAR);
	  act( "$n wears all $s equipment.", ch, NULL, NULL, TO_ROOM);
	  }
	 else
	  send_to_char( "You wear nothing new.\n\r", ch);
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	  {
          sprintf( buf, "You do not have the item '%s'.\n\r", arg );
	  send_to_char( buf, ch );
	  return;
	  }
	location=-1;
	if(!strcasecmp(local,"light"))  location=ITEM_TAKE;
	if(!strcasecmp(local,"finger")) location=ITEM_WEAR_FINGER;
	if(!strcasecmp(local,"neck"))   location=ITEM_WEAR_NECK;
	if(!strcasecmp(local,"body"))   location=ITEM_WEAR_BODY;
	if(!strcasecmp(local,"head"))   location=ITEM_WEAR_HEAD;
	if(!strcasecmp(local,"legs"))   location=ITEM_WEAR_LEGS;
	if(!strcasecmp(local,"feet"))   location=ITEM_WEAR_FEET;
	if(!strcasecmp(local,"hands"))  location=ITEM_WEAR_HANDS;
	if(!strcasecmp(local,"arms"))   location=ITEM_WEAR_ARMS;
	if(!strcasecmp(local,"shield")) location=ITEM_WEAR_SHIELD;
	if(!strcasecmp(local,"about"))  location=ITEM_WEAR_ABOUT;
	if(!strcasecmp(local,"waist"))  location=ITEM_WEAR_WAIST;
	if(!strcasecmp(local,"wrist"))  location=ITEM_WEAR_WRIST;
	if(!strcasecmp(local,"wield"))  location=ITEM_WIELD;
	if(!strcasecmp(local,"hold"))   location=ITEM_HOLD;
	if(!strcasecmp(local,"heart"))  location=ITEM_WEAR_HEART;

	wear_obj( ch, obj, TRUE, location , TRUE);
    }

    return;
}




void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }
    /* all added by Trom */
    if(!strcasecmp(arg,"all")) {
	for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
	{
	    obj_next=obj->next_content;
	    if( obj->wear_loc != WEAR_NONE && obj->wear_loc != WEAR_LIGHT ) 
	      remove_obj(ch,obj->wear_loc,TRUE, FALSE);
	}
	act( "You remove all your equipment.",ch,NULL,NULL,TO_CHAR);
	act( "$n removes all $s equipment.", ch, NULL, NULL, TO_ROOM);
    } else {
	if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
	{
          sprintf( buf, "You do not have the item '%s'.\n\r", arg );
	  send_to_char( buf, ch );
	    return;
	}
	remove_obj( ch, obj->wear_loc, TRUE , TRUE);
    }
    return;
}


void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj,*obj_next,*objc;
    bool found, plural;
    char god[10], buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );


    switch(which_god(ch))
      {
      case GOD_INIT_ORDER:
      case GOD_ORDER: strcpy(god, "Order");  plural=FALSE; break;
      case GOD_INIT_CHAOS:
      case GOD_CHAOS: strcpy(god, "Chaos");  plural=FALSE; break;
      case GOD_DEMISE: strcpy(god, "Demise"); plural=FALSE; break;
      default: strcpy(god, "The Gods"); plural=TRUE;
      }

    if ( arg[0] == '\0' || !strcasecmp( arg, ch->name ) )
    {
      if( plural )
	act( "$n offers $mself to $t, whom graciously decline.",
	    ch, god, NULL, TO_ROOM );
      else
	act( "$n offers $mself to $t, who graciously declines.",
	    ch, god, NULL, TO_ROOM );
      if( plural )
	sprintf( buf, "%s appreciate your offer and may accept it later.\n\r",
	       god);
      else
	sprintf( buf, "%s appreciates your offer and may accept it later.\n\r",
	       god);
	send_to_char( buf, ch);
	return;
    }
    if (!strcasecmp( arg, "all" ) || !str_prefix( "all.", arg ) )
      {
      /* 'sac all' or 'sac all.obj' */
      found = FALSE;
      for ( obj = ch->in_room->first_content; obj != NULL; obj = obj_next )
	{
	obj_next = obj->next_content;
	if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	  &&   can_see_obj( ch, obj ) )
	  {
	  int price;
	  found = TRUE;
	  if ( !CAN_WEAR(obj, ITEM_TAKE) )
	    {
	    act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	    continue;
	    }
      if( obj->item_type == ITEM_CORPSE_NPC )
	  if( obj->owned_by!=-1 && !pvnum_in_group( ch, obj->owned_by))
	    continue;
	  price= obj->cost / 100 + 1 ;
       if( plural )
	  sprintf( buf, "%s give you %d gold coins for %s.\n\r", god, 
	     price, obj->short_descr);
       else
	  sprintf( buf, "%s gives you %d gold coins for %s.\n\r", god, 
	     price, obj->short_descr);
	  send_to_char( buf, ch);
	  ch->gold+=price;
	  act( "$n sacrifices $p to $T.", ch, obj, god, TO_ROOM );
		for ( objc = obj->first_content; objc != NULL; objc = obj_next )
		  {
	    CHAR_DATA *tch;
		  obj_next = objc->next_content;
	    obj_from_obj( objc);
	    objc->sac_timer=OBJ_SAC_TIME;
            sprintf( buf, "  %s falls out onto the floor.\n\r",
                    capitalize( objc->short_descr ) );
	    for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
	      if(tch->position>=POS_RESTING && !IS_SET(tch->act,PLR_QUIET))
                {
                send_to_char( buf, tch );
                }
	    obj_to_room( objc, ch->in_room);
	    }
	 /* sac_obj( obj );*/
	obj_from_room( obj);
	extract_obj( obj);
	  }
	}

      if ( !found ) 
	{
	if ( arg[3] == '\0' )
	  send_to_char( "I see nothing here.\n\r", ch );
	else
	  act( "I see no $T here.", ch, NULL, &arg[4], TO_CHAR );
	}
      }
    else
      {
      int price;
      obj = get_obj_list( ch, arg, ch->in_room->first_content );
      if ( obj == NULL )
	{
	send_to_char( "You can't find it.\n\r", ch );
	return;
	}

      if ( !CAN_WEAR(obj, ITEM_TAKE) )
	{
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
	}
      if( obj->item_type == ITEM_CORPSE_NPC )
	  if(obj->owned_by!=-1 && !pvnum_in_group( ch, obj->owned_by))
	    return;

	  price= obj->cost / 100 + 1 ;
        if( plural )
	  sprintf( buf, "%s give you %d gold coins for %s.\n\r", god, 
	     price, obj->short_descr);
	  sprintf( buf, "%s gives you %d gold coins for %s.\n\r", god, 
	     price, obj->short_descr);
	  send_to_char( buf, ch);
 	  ch->gold+=price;

      act( "$n sacrifices $p to $T.", ch, obj, god, TO_ROOM );
      for ( objc = obj->first_content; objc != NULL; objc = obj_next )
	      {
	CHAR_DATA *tch;
	      obj_next = objc->next_content;
	obj_from_obj( objc);
	objc->sac_timer=OBJ_SAC_TIME;
	for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
	  if(!IS_SET(tch->act,PLR_QUIET))
                {
                sprintf( buf, "  %s falls out onto the floor.\n\r",
                    capitalize( objc->short_descr ) );
                send_to_char( buf, tch );
                }
	obj_to_room( objc, ch->in_room);
	}
      /*sac_obj( obj );*/
    obj_from_room( obj);
    extract_obj( obj);
      }
  return;
  }

/* Removed this neato system because of clutter
void sac_obj( OBJ_DATA *obj )
  {
  CHAR_DATA *fch;
  bool found;
  int cnt, total;
  total=0;

  if(obj->item_type==ITEM_MONEY || obj->item_type==ITEM_KEY ||
     obj->item_type==ITEM_TRASH || obj->item_type==ITEM_CORPSE_NPC ||
     obj->item_type==ITEM_CORPSE_PC || number_range(1,3)!=1 ||
     obj->cost<100)
    {
    obj_from_room( obj);
    extract_obj( obj);
    return;
    }

  for(fch = first_char; fch!=NULL; fch=fch->next)
    {
    found=FALSE;
    if(IS_NPC(fch) && fch->pIndexData->pShop!=NULL)
      for( cnt=0; cnt<MAX_TRADE; cnt++)
	if(fch->pIndexData->pShop->buy_type[cnt]==obj->item_type)
	  found=TRUE;
    if(found)
      total++;
    }
  if(total==0)
    {
    obj_from_room( obj);
    extract_obj( obj);
    return;
    }
  total=number_range(1,total);
  for(fch = first_char; fch!=NULL && total>0; fch=fch->next)
    {
    found=FALSE;
    if(IS_NPC(fch) && fch->pIndexData->pShop!=NULL)
      for( cnt=0; cnt<MAX_TRADE; cnt++)
	if(fch->pIndexData->pShop->buy_type[cnt]==obj->item_type)
	  found=TRUE;
    if(found)
      total--;
    if(total==0)
      {
      obj->cost*=2;
      obj_from_room( obj);
      obj_to_char(obj, fch);
      return;   
      }
    }
  return;       
  }
*/


void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }

    if( ch->class != CLASS_ROGUE && !IS_NPC( ch ) &&
        ( ch->fighting!=NULL || ch->position == POS_FIGHTING ) )
          {
          send_to_char( "You may not do that while fighting.\n\r", ch);
          return;
          }
    if( ch->class == CLASS_ELEMENTALIST ||
        ch->class == CLASS_ILLUSIONIST ||
        ch->class == CLASS_NECROMANCER )
      {
      send_to_char( "You may not do that in your class.\n\r", ch);
      return;
      }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }
    if( obj->level > ch->level )
      {
      send_to_char( "You are too low a level to use that potion!\n\r", ch);
      return;
      }
  if( ch->position == POS_FIGHTING && number_range(1,2)!=1)
    {
    act( "$n breaks $p.", ch, obj, NULL, TO_ROOM );
    act( "You break $p.", ch, obj, NULL ,TO_CHAR );
    }
else
    {
    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, obj );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, obj );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, obj );
    }

    extract_obj( obj );
    WAIT_STATE( ch, PULSE_VIOLENCE*2/3);
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }
  if( ch->mclass[ CLASS_NECROMANCER ] == 0 )
    if( ch->class == CLASS_RANGER ||
        ch->class == CLASS_ROGUE ||
        ch->class == CLASS_ASSASSIN )
      {
      send_to_char( "You may not do that in your class.\n\r", ch);
      return;
      }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if( scroll->level > ch->level )
      {
      send_to_char( "You are too low a level to recite that scroll!\n\r", ch);
      return;
      }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if( ch->class != CLASS_NECROMANCER && scroll->value[1]!=slot_lookup(42) &&
      !IS_NPC( ch ) &&
        ( ch->fighting!=NULL || ch->position == POS_FIGHTING ) )
          {
          send_to_char( "You may not do that while fighting.\n\r", ch);
          return;
          }
    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    if( scroll->value[1]!=slot_lookup(42) )
      {
      obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
      obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
      }

    extract_obj( scroll );
    WAIT_STATE( ch, PULSE_VIOLENCE*2);
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }
    if( ch->class != CLASS_ELEMENTALIST && !IS_NPC( ch ) &&
        ( ch->fighting!=NULL || ch->position == POS_FIGHTING ) )
          {
          send_to_char( "You may not do that while fighting.\n\r", ch);
          return;
          }
  if( ch->mclass[ CLASS_ELEMENTALIST ] == 0 )
    if( ch->class == CLASS_RANGER ||
        ch->class == CLASS_ROGUE ||
        ch->class == CLASS_ASSASSIN )
      {
      send_to_char( "You may not do that in your class.\n\r", ch);
      return;
      }

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }
    if( staff->level > ch->level )
      {
      send_to_char( "You are too low a level to brandish that staff!\n\r", ch);
      return;
      }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next    = vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( !IS_NPC(vch) && ch->fighting && ch->fighting->who!=vch )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, staff );
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }
    if( ch->class != CLASS_ILLUSIONIST && 
        ( ch->fighting!=NULL || ch->position == POS_FIGHTING ) )
          {
          send_to_char( "You may not do that while fighting.\n\r", ch);
          return;
          }
  if( ch->mclass[ CLASS_ILLUSIONIST ] == 0 )
    if( ch->class == CLASS_RANGER ||
        ch->class == CLASS_ROGUE ||
        ch->class == CLASS_MONK ||
        ch->class == CLASS_ASSASSIN )
      {
      send_to_char( "You may not do that in your class.\n\r", ch);
      return;
      }

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }
    if( wand->level > ch->level )
      {
      send_to_char( "You are too low a level to use that wand!\n\r", ch);
      return;
      }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = who_fighting ( ch );
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

	obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }
    if( find_keeper( ch ) != NULL )
      {
      send_to_char( "You can't steal in a shop!\n\r", ch);
      return;
      }
    if( ch->class == CLASS_MONK )
      {
      send_to_char( "What would your guildmaster say?\n\r", ch);
      return;
      }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

  if(which_god(ch)==GOD_INIT_ORDER || which_god(ch)==GOD_INIT_CHAOS)
    {
    send_to_char("You may not steal items as an Initiate.\n\r", ch);
    return;
    }

  if(which_god(victim)==GOD_INIT_ORDER || which_god(victim)==GOD_INIT_CHAOS)
    {
    send_to_char("You may not steal items from Initiates.\n\r", ch);
    return;
    }


    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }
    if ( victim->fighting!=NULL || victim->position== POS_FIGHTING )
	{
	send_to_char( "It looks like your victim is too busy at the moment.\n\r", ch);
	return;
	}
    if( !IS_NPC(victim) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
      {
      if( which_god(victim)==GOD_NEUTRAL || 
          which_god(ch)==GOD_NEUTRAL ||
          which_god(ch)==GOD_INIT_CHAOS ||
          which_god(ch)==GOD_INIT_ORDER ||
          which_god(victim)==GOD_INIT_CHAOS ||
          which_god(victim)==GOD_INIT_ORDER ||
          get_trust(victim)>=MAX_LEVEL ||
          which_god(victim)==GOD_POLICE ||
	  which_god(victim)==which_god(ch))
	{
	send_to_char( "You can't do that.\n\r", ch);
	return;
	} 
     else
	if( multi( ch, gsn_steal)==-1  ||
	  victim->level > 5*ch->level/4) 
	{
	send_to_char( "That person is too high a level to steal from.\n\r", ch);
	return;
	}
      else
	if( victim->level < ch->level / 3 ) 
	{
	send_to_char( "You shouldn't need to steal from that person.\n\r", ch);
	return;
	}
      else
	if( victim->hit < victim->max_hit ) 
	{
	send_to_char( "They are too suspicious to allow a theft right now.\n\r", ch);
	return;
	}
      if( !can_reincarnate_attack( ch, victim )) 
	{
	send_to_char( "They are not of the correct spiritual background to steal from.\n\r", ch);
	return;
	}
      }
    if( IS_AFFECTED( victim, AFF_CHARM) )
	{
	send_to_char( "You cannot steal from someone's pet.\n\r", ch);
	return;
	}
    if(!IS_NPC(victim) && number_percent()<victim->pcdata->learned[gsn_guard])
      {
      send_to_char("They notice your attempt and brush your hand away.\n\r",ch);
      act("$n just tried to steal $t from you.\n\r",ch,arg1,victim,TO_VICT);
      WAIT_STATE( ch, skill_table[gsn_steal].beats );
      return;
      }
	   
  if( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 &&
      ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) )
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
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", victim->short_descr);
      send_to_char( buf, ch);
      return;
      }
    }


    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent=number_percent()+(IS_AWAKE(victim)?10-2*victim->distracted:-50);
    percent+= (4* ( ch->level - victim->level ));

    /*if ( ch->mclass[multi(ch,gsn_steal)]*3/2+3  < victim->level */
   
    if (    victim->position == POS_FIGHTING
    || ( !IS_NPC(ch) && percent>(ch->pcdata->learned[gsn_steal]) ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	sprintf( buf, "%s is a bloody thief!", get_name( ch ) );
	do_shout( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		log_string( buf );
		if ( !IS_SET(ch->act, PLR_THIEF) )
		{
		    SET_BIT(ch->act, PLR_THIEF);
		    send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
		    save_char_obj(ch, NORMAL_SAVE);
		}
	    }
	}

	return;
    }

    if ( !strcasecmp( arg1, "coin"  )
    ||   !strcasecmp( arg1, "coins" )
    ||   !strcasecmp( arg1, "gold"  ) )
    {
	int amount;

	amount = UMIN(1000,victim->gold * number_range(1, 10) / 100);
	if ( amount <= 0 )
	{
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
	sprintf( buf, "Bingo!  You got %d gold coins.\n\r", amount );
	send_to_char( buf, ch );
        if(!IS_NPC(victim)&&number_percent()<victim->pcdata->learned[gsn_guard])
          {
          act("You notice $n taking $s hand out of your gold collection.",
              ch,NULL,victim,TO_VICT);
          return;
          }
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->item_type == ITEM_CONTAINER
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    send_to_char( "Ok.\n\r", ch );
    if(!IS_NPC(victim)&&number_percent()<victim->pcdata->learned[gsn_guard])
      {
      act("You notice $n has your $p in $s hands.",
          ch,obj,victim,TO_VICT);
      return;
      }
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
  {
  CHAR_DATA *keeper;
  SHOP_DATA *pShop;

  if( ch==NULL )
    return(NULL);

  pShop = NULL;
  for ( keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room )
    {
	  if ( IS_NPC(keeper) && ((pShop = keeper->pIndexData->pShop) != NULL ))
	    break;
    }

  if ( pShop == NULL )
	  return NULL;

  /*  These guys should be desireable      Chaos 12/28/93
   * Undesirables.
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	  do_say( keeper, "Killers are not welcome!" );
	  sprintf( buf, "%s the KILLER is over here!\n\r", get_name( ch ) );
	  do_shout( keeper, buf );
	  return NULL;
    }
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	  do_say( keeper, "Thieves are not welcome!" );
	  sprintf( buf, "%s the THIEF is over here!\n\r", get_name( ch ) );
	  do_shout( keeper, buf );
	  return NULL;
    }
   * Shop hours.
   */

  return keeper;
  }



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
  {
  SHOP_DATA *pShop;
  int cost;

  if( keeper == NULL )
    return( 0);

  if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	  return 0;

  if ( fBuy )
    {
	  cost = obj->cost * pShop->profit_buy  / 100;
    }
  else
    {
	  OBJ_DATA *obj2;
	  int itype;

	  cost = 0;
	  for ( itype = 0; itype < MAX_TRADE; itype++ )
	    {
	    if ( obj->item_type == pShop->buy_type[itype] )
	      {
		    cost = obj->cost * pShop->profit_sell / 100;
		    break;
	      }
	    }

	  for ( obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content )
	    {
	    if ( obj->pIndexData == obj2->pIndexData )
	      {
		    cost = (cost*9)/10;
	      }
	    }
    }

  if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    if( obj->value[1] != 0 )
	  cost = cost * obj->value[2] / obj->value[1];

  if( fBuy )
    cost = cost * ( 75 + obj->condition/4 );
  else
    cost = cost * ( 25 + 3*obj->condition/4 );
  cost /= 100;
    

  return cost;
  }



void do_buy( CHAR_DATA *ch, char *argument )
  {
  char arg[MAX_INPUT_LENGTH];
  int bargain;

  if( IS_NPC( ch))
    return;

  /* handle bargain skill -dug */
  bargain=IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
	  send_to_char( "Buy what?\n\r", ch );
	  return;
    }

  if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	  char buf[MAX_STRING_LENGTH];
	  CHAR_DATA *pet;
	  ROOM_INDEX_DATA *pRoomIndexNext;
	  ROOM_INDEX_DATA *in_room;

	  if ( IS_NPC(ch) )
	    return;

	  pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	  if ( pRoomIndexNext == NULL )
	    {
	    bug( "Do_buy: bad pet shop at vnum %u.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	    }
    for(pet=first_char;pet!=NULL;pet=pet->next)
      if(((IS_SET(pet->act,ACT_PET)) || (IS_SET(pet->affected_by,AFF_CHARM)))
	 && (pet->master==ch))
	{
	send_to_char("Sorry, one pet at a time...\n\r",ch);
	return;
	}

	  in_room     = ch->in_room;
          char_from_room( ch );
          char_to_room( ch, pRoomIndexNext );
	  pet         = get_char_room( ch, arg );
          char_from_room( ch );
	  char_to_room( ch, in_room);

	  if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	    {
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	    }

	  if ( ch->gold < (100* pet->level * pet->level * bargain) / 100)
	    {
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	    }

	  if ( ch->level < pet->level )
	    {
	    send_to_char( "You're not ready for this pet.\n\r", ch );
	    return;
	    }

	  ch->gold              -= (100* pet->level * pet->level * bargain)/100;
	  pet                   = create_mobile( pet->pIndexData );
	  SET_BIT(pet->act, ACT_PET);
	  SET_BIT(pet->affected_by, AFF_CHARM);

	  argument = one_argument( argument, arg );
	  if ( arg[0] != '\0' )
	    {
	    sprintf( buf, "%s %s", pet->name, arg );
	    STRFREE(pet->name );
	    pet->name = str_dup( buf );
	    }

	  sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
		  pet->description, ch->name );
	  STRFREE(pet->description );
	  pet->description = str_dup( buf );

	  char_to_room( pet, ch->in_room );
	  add_follower( pet, ch );
	  send_to_char( "Enjoy your pet.\n\r", ch );
	  act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	  return;
    }
  else
    {
	  CHAR_DATA *keeper;
	  OBJ_DATA *obj;
	  int cost;
          char buf[MAX_STRING_LENGTH];

    obj=NULL;
	  if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

    /* handle morgue buying corpses */
    if(IS_SET(ch->in_room->room_flags,ROOM_MORGUE) &&
       is_name("corpse",arg) && (!IS_NPC(ch)))
      {
      if((obj=find_char_corpse(ch,FALSE))!=NULL)
	{
	char buf[MAX_INPUT_LENGTH];
	if( obj->in_room==ch->in_room)
	   {
	   send_to_char( "But you have already bought your corpse!\n\r", ch);
	   return;
	   }
  
	sprintf(buf,"$n tells you 'That'll be %d gold coins.'",obj->cost);
	      act(buf, keeper, obj, ch, TO_VICT );
	      ch->reply = keeper;
	if(obj->cost>ch->gold)
		{
		act( "$n tells you 'You can't afford to buy $p'.",
			  keeper, obj, ch, TO_VICT );
		return;
		}
        if( !TEST_GAME )
	  ch->gold-=obj->cost;
	obj_from_room(obj);
	      act("$n disappears into the back and appears with $p.",
			keeper, obj, ch, TO_VICT );
	ch->pcdata->corpse_room=ch->in_room->vnum;
	obj_to_room(obj,ch->in_room);
	save_char_obj(ch, NORMAL_SAVE);
	return;
	}
      else
	{
	act("You don't have any corpses here, go make one!",
	    keeper,NULL,ch,TO_VICT);;
	return;
	}
      }
    if ( ( obj = get_obj_carry_keeper( keeper, arg, ch, bargain ) ) == NULL )
      {
      char buf[MAX_INPUT_LENGTH];
      sprintf(buf,"$n tells you 'I can't sell you a %s'.",arg);
      act(buf, keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
      }

  if ( time_info.hour < keeper->pIndexData->pShop->open_hour )
    {
	  send_to_char( "The shop appears to be closed.\n\r", ch );
	  return ;
    }
    
  if ( time_info.hour > keeper->pIndexData->pShop->close_hour )
    {
	  send_to_char( "The shop appears to be closed.\n\r", ch );
	  return ;
    }

  if ( !can_see( keeper, ch ) )
    {
	  send_to_char( "You have to be visible to do that.\n\r", ch );
	  return ;
    }

	  cost = (get_cost( keeper, obj, TRUE )*bargain)/100;

	  if ( cost <= 0 || !can_see_obj( ch, obj ) )
	    {
	    act( "$n tells you 'I can't sell you that -- try 'list''.",
		      keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }

	  if ( ch->gold < cost )
	    {
	    act( "$n tells you 'You can't afford to buy $p'.",
		      keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }
	
	  if ( obj->level > ch->level )
	    {
	    act( "$n tells you 'You can't use $p yet'.",
		      keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }

	  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	    {
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	    }

	  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
	    {
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	    }

        act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
        sprintf(buf, "You buy %s for %d gold.\n\r", obj->short_descr, cost );
        send_to_char( buf, ch );
	ch->gold     -= cost;
	keeper->gold += cost;

	  if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
      {
      int cond, ref1, ref2;
      cond = obj->condition;
      ref1 = obj->index_reference[0];
      ref2 = obj->index_reference[1];
      if( obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_WEAPON )
        {
        obj->condition = 70 + number_range( 0, 29 );
        }
      create_object_reference( obj );
      obj = create_object( obj->pIndexData, obj->pIndexData->level );
      obj->pIndexData->max_objs = 2000000000;
      obj->index_reference[0]=ref1;
      obj->index_reference[1]=ref2;
      obj->condition = cond;
      }
	  else
	    obj_from_char( obj );

	  if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
            {
            REMOVE_BIT( obj->extra_flags, ITEM_INVENTORY);
            obj->basic = FALSE;
            }

	  obj_to_char( obj, ch );
	  return;
    }
  }


void do_list( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH], buf4[10];
  int bargain, leng;

  bargain=IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100;

  if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	  ROOM_INDEX_DATA *pRoomIndexNext;
	  CHAR_DATA *pet;
	  bool found;

	  pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	  if ( pRoomIndexNext == NULL )
	    {
	    bug( "Do_list: bad pet shop at vnum %u.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	    }

	  found = FALSE;
	  for ( pet = pRoomIndexNext->first_person; pet; pet = pet->next_in_room )
	    {
	    if ( IS_SET(pet->act, ACT_PET) )
	      {
		    if ( !found )
		      {
		      found = TRUE;
		      send_to_char( "Pets for sale:\n\r", ch );
		      }
		    sprintf( buf, "[%2d] %8d - %s\n\r", pet->level,
			    (100* pet->level * pet->level*bargain)/100, pet->short_descr );
		    send_to_char( buf, ch );
	      }
	    }
	  if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	  return;
    }
  else
    {
	  char arg1[MAX_INPUT_LENGTH],buf2[MAX_STRING_LENGTH];
	  char arg2[MAX_INPUT_LENGTH],*objName;
	  char arg3[MAX_INPUT_LENGTH];
    char tBuf1[MAX_INPUT_LENGTH];
	  CHAR_DATA *keeper;
	  OBJ_DATA *obj, *oobj;
	  int cost,numObj;
          int nrL, nrH;
	  bool found, ofound, listAll, listWide;

          nrL = -10;
          nrH = 120;

	  argument=one_argument( argument, arg1 );
	  argument=one_argument( argument, arg2 );
	  argument=one_argument( argument, arg3 );

	  if ( ( keeper = find_keeper( ch ) ) == NULL )
      {
      send_to_char( "There are no items to purchase here.\n\r", ch);
	    return;
      }

    if( *arg1 != '\0' && is_number( arg1 ) )
      {
      nrL = atol( arg1 );
      if( nrL < 0 )
        nrL = -10;
      if( nrL > 120 )
        nrL = 120;
      if( *arg2 != '\0' && is_number( arg2 ) )
        {
        nrH = atol( arg2 );
        if( nrH < nrL )
          nrH = nrL;
        if( nrH > 120 )
          nrH = 120;
        }
      *arg1 = '\0';
      *arg2 = '\0';
      *arg3 = '\0';
      }

    listAll=!strcasecmp(arg1,"all");
    /* listWide=(listAll)?strcasecmp(arg2,"wide"):strcasecmp(arg1,"wide"); */
    listWide = TRUE;
    objName=(listAll)?((listWide)?arg3:arg2):((listWide)?arg2:arg1);
    listWide=!listWide;

	  found = FALSE;
    buf2[0]='\0';
    leng=0;
    numObj=0;
	  for ( obj = keeper->first_carrying; obj; obj = obj->next_content )
	    {
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
            && obj->level >= nrL &&  obj->level <= nrH 
	    && ( cost = (get_cost( keeper, obj, TRUE )*bargain)/100 ) > 0
	    && ( *objName=='\0' || is_name_short( objName, obj->name ) ) )
	      {
		  if ( !found )
		    {
		    found = TRUE;
                    if( ch->level > 50 )
                      {
	              if(listWide)
                        {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
                        sprintf(buf2,"%s %s\n\r",
		         str_resize("   {170}[{070}Lv {010}Price  {170}]{130}Item",tBuf1,-74),tBuf1);
    else
                        sprintf(buf2,"%s %s\n\r",
		         str_resize("{020}Ref{170}[{070}Lv {010}Price  {170}]{130}Item",tBuf1,-74),tBuf1);
                        }
	              else
                        {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
			sprintf(buf2, "     {170}[{070}Lv   {010}Price  {170}] {130}Item\n\r");
    else
			sprintf(buf2, "{020}#Ref {170}[{070}Lv   {010}Price  {170}] {130}Item\n\r");
                        }
                      }
		    else
                      {
	              if(listWide)
                        {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
                        sprintf(buf2,"%s %s\n\r",
		         str_resize("   {170}[{070}Lv {010}Price  {170}]{130}Item",tBuf1,-73),tBuf1);
    else
                        sprintf(buf2,"%s %s\n\r",
		         str_resize("{020}Ref{170}[{070}Lv {010}Price  {170}]{130}Item",tBuf1,-73),tBuf1);
                        }
	              else
                        {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
			sprintf(buf2, "     {170}[{070}Lv   {010}Price  {170}] {130}Item\n\r" );
    else
			sprintf(buf2, "{020}#Ref {170}[{070}Lv   {010}Price  {170}] {130}Item\n\r" );
                        }
                      }
                    leng = strlen( buf2 );
		    }
	ofound = FALSE;
	for( oobj = keeper->first_carrying; oobj!=obj; oobj = oobj->next_content)
	  if( oobj->wear_loc == WEAR_NONE &&
	      can_see_obj( ch, oobj)      &&
              oobj->level >= nrL &&  oobj->level <= nrH &&
	      get_cost( keeper, oobj, TRUE)>0 &&
	      (*objName=='\0' || is_name_short(objName, oobj->name)))
	    if(oobj->pIndexData->vnum == obj->pIndexData->vnum &&
	       obj->level == oobj->level)
	      ofound = TRUE;
	if( ofound == FALSE)
	  {
	  char buf3[MAX_INPUT_LENGTH];

	  if((!listAll) && (obj->level>ch->level /* || cost>ch->gold */))
	    continue;
	  numObj++;

   if(!listWide)
     strcpy( buf4, " " );
   else
     strcpy( buf4, "" );

#ifdef EQUIPMENT_DAMAGE
   if( obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_WEAPON )
    {
    if( ch->level > 50 )
      {
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}%2d{170}]{130}%s%s.", obj->level, cost, obj->condition,
		  buf4, capitalize( obj->short_descr ) );
      }
    else if( obj->condition >=90 )
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}E{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else if( obj->condition >=70 )
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}G{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else if( obj->condition >=50 )
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}F{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else if( obj->condition >=30 )
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}P{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else if( obj->condition >=10 )
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}B{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else
            sprintf(buf, "{170}[{070}%2d {010}%7d {060}D{170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    }
  else
    {
    if( ch->level > 50 )
            sprintf(buf, "{170}[{070}%2d {010}%7d   {170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    else
            sprintf(buf, "{170}[{070}%2d {010}%7d  {170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
    }
#else
  sprintf(buf, "{170}[{070}%2d {010}%7d  {170}]{130}%s%s.", obj->level, cost, 
		  buf4, capitalize( obj->short_descr ) );
#endif

    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
      strcpy( buf4, "   " );
    else
       {
        *buf4 = '0'+(obj->index_reference[0]/100)%10;
        *(buf4+1) = '0'+(obj->index_reference[0]/10)%10;
        *(buf4+2) = '0'+(obj->index_reference[0])%10;
        *(buf4+3) = '\0';
       }

	  if(!listWide)
	    {
            if( !obj->basic || obj->item_type==ITEM_WEAPON ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_ARMOR )
              {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	        sprintf(buf3,"{020}     %s", buf);
    else
	        sprintf(buf3,"{020}#%s %s",buf4, buf);
              }
            else
              {
	        sprintf(buf3,"     %s",buf);
              }
	    leng = str_apd_max(buf2, buf3, leng, MAX_STRING_LENGTH );
	    leng = str_apd_max(buf2, "\n\r", leng, MAX_STRING_LENGTH );
	    }
	  else
	    {
            if( !obj->basic || obj->item_type==ITEM_WEAPON ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_ARMOR )
              {
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	      sprintf(buf3,"{020}   %s", str_resize(buf,tBuf1,-36) );
    else
	      sprintf(buf3,"{020}%s%s", buf4, str_resize(buf,tBuf1,-36) );
              }
	    else
              {
	      sprintf(buf3,"   %s",str_resize(buf,tBuf1,-37) );
              }

	    leng = str_apd_max(buf2, buf3, leng, MAX_STRING_LENGTH );
	    if((numObj%2)==0)
	      leng = str_apd_max(buf2, "\n\r", leng, MAX_STRING_LENGTH );
            else
	      leng = str_apd_max(buf2, " ", leng, MAX_STRING_LENGTH );
	    }
	  }
        }
      }
    if(IS_SET(ch->in_room->room_flags,ROOM_MORGUE) &&
       (obj=find_char_corpse(ch,FALSE))!=NULL)
      {

            sprintf(buf, "{170}     [{070}%2d {010}%7d  {170}] {130}%s.", obj->level, cost, 
	      capitalize( obj->short_descr ) );
      leng = str_apd_max(buf2, buf, leng, MAX_STRING_LENGTH );
      leng = str_apd_max(buf2, "\n\r", leng, MAX_STRING_LENGTH );
      found=TRUE;
      }
	  if ( !found )
	    {
	    if ( *objName == '\0' )
	  leng = str_apd_max( buf2, "There's nothing here you would want.\n\r",
                 leng, MAX_STRING_LENGTH);
	    else
	  leng = str_apd_max( buf2, "You can't buy that here.\n\r", leng,
                 MAX_STRING_LENGTH);
	    }
    send_to_char( ansi_translate_text( ch, buf2), ch);
	  return;
    }
  }

void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;
    int bargain;

    /* handle bargain skill -dug */
    bargain=IS_NPC(ch)?100:100+(25*(ch->pcdata->learned[gsn_bargain]))/100;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }
  if ( time_info.hour < keeper->pIndexData->pShop->open_hour )
    {
	  send_to_char( "The shop appears to be closed.\n\r", ch );
	  return ;
    }
    
  if ( time_info.hour > keeper->pIndexData->pShop->close_hour )
    {
	  send_to_char( "The shop appears to be closed.\n\r", ch );
	  return ;
    }

  if ( !can_see( keeper, ch ) )
    {
	  send_to_char( "You have to be visible to do that.\n\r", ch );
	  return ;
    }


    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( obj->owned_by!=0 )
    {
	send_to_char( "You can't sell personalize equipment.\n\r", ch );
	return;
    }

    if ( ( cost = (bargain*get_cost( keeper, obj, FALSE ))/100 ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if( !obj->basic )
    {
	act( "$n will not buy a custom $p.", keeper, obj, ch, TO_VICT );
	return;
    }


    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost;
    keeper->gold -= cost;
    if ( keeper->gold < 0 )
	keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
	obj_from_char( obj );
	extract_obj( obj );
    }
    else
    {
    OBJ_INDEX_DATA *pObj;
	obj_from_char( obj );
	pObj=obj->pIndexData;
	extract_obj( obj );
	obj=create_object( pObj, pObj->level );
	  if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
            {
            REMOVE_BIT( obj->extra_flags, ITEM_INVENTORY);
            obj->basic = FALSE;
            }
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;
    int bargain;

    /* handle bargain skill -dug */
    bargain=IS_NPC(ch)?100:100+(25*(ch->pcdata->learned[gsn_bargain]))/100;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }
    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = (bargain*get_cost( keeper, obj, FALSE ))/100 ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void do_evaluate( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  int bargain;
  int value1;
  int value2;
  char *msg;

  /* handle bargain skill -dug */
  bargain=IS_NPC(ch)?100:100+(25*(ch->pcdata->learned[gsn_bargain]))/100;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( ( keeper = find_keeper( ch ) ) == NULL )
	  return;

  if ( arg1[0] == '\0' )
    {
	  send_to_char( "Evaluate what to what?\n\r", ch );
	  return;
    }

  if ( ( obj1 = get_obj_carry_keeper( keeper, arg1, ch, bargain ) ) == NULL )
    {
    sprintf(buf,"$n tells you 'I don't have a %s'.",arg1);
    act(buf, keeper, NULL, ch, TO_VICT );
	  ch->reply = keeper;
	  return;
    }

  if ( arg2[0] == '\0' )
    {
	  for ( obj2 = ch->first_carrying; obj2 != NULL; obj2 = obj2->next_content )
	    {
	    if ( obj2->wear_loc != WEAR_NONE
		&&   can_see_obj( ch, obj2 )
		&&   obj1->item_type == obj2->item_type
		&& ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		    break;
	    }

	  if ( obj2 == NULL )
	    {
	    send_to_char( "You aren't wearing anything similar enough.\n\r", ch );
	    return;
	    }
    }
  else
    {
	  if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
	    {
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	    }
    }

  msg           = NULL;
  value1        = 0;
  value2        = 0;

  if ( obj1->item_type != obj2->item_type )
    {
	  msg = "You can't compare $p and $P.";
    }
  else
    {
	  switch ( obj1->item_type )
	    {
	    default:
	      msg = "You can't compare $p and $P.";
	      break;

	    case ITEM_ARMOR:
	      value1 = obj1->value[0];
	      value2 = obj2->value[0];
	      break;

	    case ITEM_WEAPON:   /* the val[3] is type...why affect value? -dug*/
	      value1 = obj1->value[1] * obj1->value[2] /* + obj1->value[3]*/;
	      value2 = obj2->value[1] * obj2->value[2] /* + obj2->value[3]*/;
	      break;

      case ITEM_AMMO:
	      value1 = obj1->value[1] + obj1->value[2] - obj1->value[3];
	      value2 = obj2->value[1] + obj2->value[2] - obj2->value[3];
	      break;
	    }
    }

  if ( msg == NULL )
    {
	       if ( value1 == value2 ) msg = "$p and $P look about the same.";
	  else if ( value1  > value2 ) msg = "$p looks better than $P.";
	  else                         msg = "$p looks worse than $P.";
    }


  act("$n tells you 'I'll let you handle $p for a minute.'.",
      keeper,obj1, ch,TO_VICT);
  act( msg, ch, obj1, obj2, TO_CHAR );
  ch->reply = keeper;

  return;
  }

void do_snatch( CHAR_DATA *ch, char *argument )
  {
  bool snatched;
  int cnt;

  cnt = multi( ch, gsn_snatch );
  if( cnt == -1 )
    {
    send_to_char( "You can't snatch things.\n\r", ch );
    return;
    }
    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_char( "You cannot do that here.\n\r", ch);
      return;
      }
  if(multi(ch,gsn_snatch)==-1 && !IS_NPC(ch))
    return;

  snatched=IS_NPC(ch)||number_percent()<ch->pcdata->learned[gsn_snatch];

  if(snatched)
    {
    /* make them invisible so no one can see them get it */
    if(ALLOW_OUTPUT==TRUE)
      ALLOW_OUTPUT=FALSE;
    else /* already invised */
      snatched=FALSE;
    }

  do_get(ch,argument);

  if(snatched)
    {
    /* make them visible again */
    ALLOW_OUTPUT=TRUE;
    ch_printf(ch, "Your snatch was unseen by human eyes.\n\r");
    }
  return;
  }

void do_identify( CHAR_DATA *ch, char *argument )
  {
  char arg[MAX_INPUT_LENGTH];
  int bargain;

  /* handle bargain skill -dug */
  bargain=IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100;

  argument = one_argument( argument, arg );

    {
	  CHAR_DATA *keeper;
	  OBJ_DATA *obj;
	  int cost;

	  if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

    if ( arg[0] == '\0' )
      {
	    send_to_char( "Have what identified?\n\r", ch );
	    return;
      }

    if ( ( obj = get_obj_carry_keeper( keeper, arg, ch, bargain ) ) == NULL )
      {
      char buf[MAX_INPUT_LENGTH];

      sprintf(buf,"$n tells you 'I can't identify a %s for you'.",arg);
      act(buf, keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
      }
	  cost = (get_cost( keeper, obj, TRUE )*bargain)/100;

	  if ( cost <= 0 || !can_see_obj( ch, obj ) )
	    {
	    act( "$n tells you 'I can't identify that for you -- try 'list''.",
		      keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }

    cost = ((5000*bargain)/100)+(0.02)*cost;
	  if ( ch->gold < cost )
	    {
	    act( "$n tells you 'You can't afford to identify $p'.",
		      keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }
	
	  if ( obj->level > ch->level )
	    {
	    act( "$n tells you 'You can't use $p yet'.",
		      keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	    }

	  act( "$n has $p identified.", ch, obj, NULL, TO_ROOM );
	  act( "You have $p identified.", ch, obj, NULL, TO_CHAR );
	  ch->gold     -= cost;
	  keeper->gold += cost;

    spell_identify(-1,keeper->level,ch,(void *)obj);

	  return;
    }
  }

OBJ_DATA * find_char_corpse (CHAR_DATA *ch, bool method)
  {
  OBJ_DATA *corpse;

  /* find the player's first corpse with objects in it */
  
 if( method)
  for(corpse=first_object;corpse!=NULL;corpse=corpse->next)
    {
    if((corpse->item_type==ITEM_CORPSE_PC)&&
       (corpse->first_content!=NULL)&&
       (is_name(ch->name,corpse->short_descr)))
      break;
    } 
  else
    corpse=ch->pcdata->corpse;

  if(corpse!=NULL)
    {
    OBJ_DATA *obj;

    corpse->cost=(650*ch->level)+(125*ch->level*ch->level);
    for(obj=corpse->first_content;obj!=NULL;obj=obj->next_content)
      corpse->cost+=(19*obj->cost)/100;
    }

  return corpse;
  }

void do_engrave( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    int cost, stat;

    if( IS_NPC( ch ))
      return;

    if( ch->in_room->vnum != 9704 )
      {
      send_to_char( "You are not at the blacksmith.\n\r", ch);
      return;
      }

    if( argument[0]=='\0' )
      {
      send_to_char( "You must engrave an object.\n\r", ch);
      return;
      }

    argument = one_argument( argument, arg );
    obj = get_obj_list( ch, arg, ch->first_carrying );

    if( obj==NULL)
      {
      send_to_char( "You do not have that object.\n\r", ch);
      return;
      }

    if( obj->owned_by!=0)
      {
      send_to_char( "You cannot engrave that object.\n\r", ch);
      return;
      }
      
    if( ch->level <10)
      {
      send_to_char( "You are not experienced enough to engrave objects.\n\r", ch);
      return;
      }
      
    stat=1;
    cost=10000;
    if( ch->level >25)
      {
      stat=2;
      cost=100000;
      }
    if( ch->level >45)
      {
      stat=3;
      cost=1000000;
      }
    if( ch->level >70)
      {
      stat=4;
      cost=10000000;
      }

    obj->basic = FALSE;

    if( ch->gold < cost)
      {
      send_to_char( "You do not have enough gold to engrave objects.\n\r", ch);
      return;
      }

    
	  switch ( obj->item_type )
	    {
	    case ITEM_ARMOR:
	       CREATE(paf, AFFECT_DATA, 1);
	       paf->location    = APPLY_AC;
	       paf->modifier    = 0-stat;
	       break;
	    case ITEM_WEAPON:
	       CREATE(paf, AFFECT_DATA, 1);
	       paf->location    = APPLY_HITROLL;
	       paf->modifier    = stat;
	       break;
	    default:
	      send_to_char( "You cannot engrave that kind of an object.\n\r", ch);
	      return;
	    }
      obj->owned_by=ch->pcdata->pvnum;
      obj->basic = FALSE;
      ch->gold-=cost;


    paf->type           = skill_lookup( "enchant" );
    paf->duration       = -1;
    paf->bitvector      = 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    sprintf( arg, "%s of %s", obj->pIndexData->short_descr ,
       capitalize( ch->name ));

    STRFREE(obj->short_descr );
    obj->short_descr = str_dup( arg );

    char_reset( ch );

     send_to_char( "It is engraved.\n\r", ch);
    return;
   }



int wear_flags_to_wear_loc( OBJ_DATA *obj, int last)
  {
  if( last == 1 || last<1 )
    if( (obj->wear_flags & 2) == 2)
      return( 1 );
  if( last == 2 || last<1 )
    if( (obj->wear_flags & 2) == 2)
      return( 2 );
  if( last == 3 || last<1 )
    if( (obj->wear_flags & 4) == 4)
      return( 3 );
  if( last == 4 || last<1 )
    if( (obj->wear_flags & 4) == 4)
      return( 4 );
  if( last == 5 || last<1 )
    if( (obj->wear_flags & 8) == 8)
      return( 5 );
  if( last == 6 || last<1 )
    if( (obj->wear_flags & 16) == 16)
      return( 6 );
  if( last == 7 || last<1 )
    if( (obj->wear_flags & 32) == 32)
      return( 7 );
  if( last == 8 || last<1 )
    if( (obj->wear_flags & 64) == 64)
      return( 8 );
  if( last == 9 || last<1 )
    if( (obj->wear_flags & 128) == 128)
      return( 9 );
  if( last == 10 || last<1 )
    if( (obj->wear_flags & 256) == 256)
      return( 10 );
  if( last == 11 || last<1 )
    if( (obj->wear_flags & 512) == 512)
      return( 11 );
  if( last == 12 || last<1 )
    if( (obj->wear_flags & 1024) == 1024)
      return( 12 );
  if( last == 13 || last<1 )
    if( (obj->wear_flags & 2048) == 2048)
      return( 13 );
  if( last == 14 || last<1 )
    if( (obj->wear_flags & 4096) == 4096)
      return( 14 );
  if( last == 15 || last<1 )
    if( (obj->wear_flags & 4096) == 4096)
      return( 15 );
  if( last == 16 || last<1 )
    if( (obj->wear_flags & 8192) == 8192)
      return( 16 );
  if( last == 17 || last<1 )
    if( (obj->wear_flags & 16384) == 16384)
      return( 17 );
  return( -1 );
  }

void do_plant( CHAR_DATA *ch, char *argument )
  {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA  *obj;
  int objCount,failed;

  if( !IS_NPC( ch ) && ch->fighting != NULL  && !IS_NPC( ch->fighting->who ))
    {
    send_to_char( "You are too busy to plant anything on someone.\n\r", ch );
    return;
    }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_char( "Plant what on whom?\n\r", ch );
    return;
    }

  if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
    }

  if ( obj->wear_loc != WEAR_NONE )
    {
    send_to_char( "You must remove it first.\n\r", ch );
    return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

  if(which_god(victim)==GOD_INIT_ORDER || which_god(victim)==GOD_INIT_CHAOS)
    {
    send_to_char("You may not plant items on Initiates.\n\r", ch);
    return;
    }
  if(which_god(ch)==GOD_INIT_ORDER || which_god(ch)==GOD_INIT_CHAOS)
    {
    send_to_char("You may not plant items as an Initiate.\n\r", ch);
    return;
    }

  if ( !can_drop_obj( ch, obj ) )
    {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
    }

    if ( find_keeper( ch ) == victim )
      {
      send_to_char( "You may only sell items to a shop keeper.\n\r", ch );
      return;
      }

  objCount=((victim->first_carrying==NULL)?0:
      count_obj_list(obj->pIndexData,victim->first_carrying))+1+
     ((obj->first_content==NULL)?0:
      count_obj_list(obj->pIndexData,obj->first_content));
    if((victim->carry_number + get_obj_number( obj ) > can_carry_n( victim))||
       (obj->pIndexData->max_objs<objCount))
    {
    act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
    return;
    }

  if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
    act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
    return;
    }
      if( !can_reincarnate_attack( ch, victim )) 
	{
	send_to_char( "They are not of the correct spiritual background to plant that.\n\r", ch);
	return;
	}

  obj_from_char( obj );
  obj_to_char( obj, victim );

  MOBtrigger=FALSE;
  act( "You attempt to plant $p on $N.", ch, obj, victim, TO_CHAR    );

  failed=FALSE;
  if(!IS_NPC(ch)&& number_percent()>ch->pcdata->learned[gsn_plant])
    {
    act( "$n tries to plant $p on $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n plants $p on you!",   ch, obj, victim, TO_VICT    );
    send_to_char("ACK!!!  You were seen!\n\r",ch);
    failed=TRUE;
    }
  /*MOBtrigger=TRUE;*/

  if(failed && !IS_OBJ_STAT(obj,ITEM_FORGERY))
    {
      mprog_give_trigger(victim,ch,obj);
    }
  return;
  }

void do_forge( CHAR_DATA *ch, char *argument )
  {
  char arg1 [MAX_INPUT_LENGTH];
  OBJ_DATA  *obj;
  bool failure;

  if( !IS_NPC( ch ) && ch->fighting != NULL  && !IS_NPC( ch->fighting->who ))
    {
    send_to_char( "You are too busy to forge anything!\n\r", ch );
    return;
    }

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
    send_to_char( "Forge what item into what?\n\r", ch );
    return;
    }

  if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
    }


  WAIT_STATE( ch, skill_table[gsn_forge].beats );
  failure=(IS_NPC(ch))?FALSE:(number_percent()>ch->pcdata->learned[gsn_forge]);

  if(failure)
    {
    if(!IS_NPC(ch) && number_percent()<33)
      {
      act("Your attempt at forgery has destroyed $p.",ch,obj,NULL,TO_CHAR);
      obj_from_char( obj );
      extract_obj(obj);
      }
    else
      act("Your attempt at forgery has failed.",ch,obj,NULL,TO_CHAR);
    }
  else if(IS_OBJ_STAT(obj,ITEM_FORGERY))
    {
    send_to_char("That object is already a forgery!\n\r",ch);
    }
  else
    {
    smash_tilde( argument );

    if( !strcasecmp( argument, "all" ) || !strcasecmp( argument, "the" ) )
      {
      send_to_char( "You can't forge something into that.\n\r", ch );
      return;
      }

    STRFREE(obj->unforged_name);
    obj->unforged_name=str_dup(obj->name);
    STRFREE(obj->name );
    obj->name = str_dup(argument);

    STRFREE(obj->unforged_short_descr);
    obj->unforged_short_descr=str_dup(obj->short_descr);
    STRFREE(obj->short_descr );
    obj->short_descr = str_dup(argument);

    obj->basic = FALSE;

    SET_BIT(obj->extra_flags,ITEM_FORGERY);
    send_to_char("You sit down, take your time...and make a decent forgery.\n\r",ch);
    }
  
  return;
  }

void do_make_poison( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  OBJ_DATA  *obj;
  bool failure;
  char buf[MAX_INPUT_LENGTH ];
  int level, cnt;
  int idmgL, idmgH, cdmgL, cdmgH, cdurL, cdurH, ptype;
  POISON_DATA *pd;
  /* CHAR_DATA *mob; */

  if( IS_NPC( ch ) || ch->fighting != NULL )
    {
    send_to_char( "You are too busy to make anything!\n\r", ch );
    return;
    }

  idmgL = 0;
  idmgH = 0;
  cdmgL = 0;
  cdmgH = 0;
  cdurL = 0;
  cdurH = 0;

  cnt = multi( ch, gsn_make_poison );
  if( cnt == -1 )
    {
    send_to_char( "You can't make poisons.\n\r", ch );
    return;
    }
  level = ch->mclass[cnt];

/*
	for ( mob = ch->in_room->first_person; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
        send_to_char( "You can only make poisons in a guild.\n\r", ch );
        return;
	}  */


  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' ||
     ( *arg1!='m' && *arg1!='M' && *arg1!='c' && *arg1!='C' ) )
    {
    send_to_char( "Make what kind of poison?  Clear or murky?\n\r", ch );
    return;
    }

  if ( arg2[0] == '\0' ||
     ( *arg2!='p' && *arg2!='P' && 
       *arg2!='b' && *arg2!='B' && *arg2!='g' && *arg2!='G' ) )
    {
    send_to_char( "Make what color of poison?  Blue, purple, or green?\n\r", ch );
    return;
    }

  if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
    }

  if( obj->poison != NULL )
    {
    send_to_char( "That object is already poisoned.\n\r", ch );
    return;
    }

  if( obj->item_type != ITEM_WEAPON )
    {
    send_to_char( "That item is not a weapon.\n\r", ch );
    return;
    }

  WAIT_STATE( ch, skill_table[gsn_make_poison].beats );
  failure=(IS_NPC(ch))?FALSE:(number_percent()>ch->pcdata->learned[gsn_make_poison]);

  if( *arg2 == 'b' || *arg2 == 'B' )
    {
    ptype='B';
    idmgL = 0;
    idmgH = 0;
    cdmgL = level/3;
    cdmgH = 2*level/3;
    cdurL = 4;
    cdurH = 8;
    }
  else
  if( *arg2 == 'p' || *arg2 == 'P' )
    {
    ptype='P';
    idmgL = 3*level/2;
    idmgH = 3*level;
    cdmgL = 0;
    cdmgH = 0;
    cdurL = 0;
    cdurH = 0;
    }
  else
    {
    ptype='G';
    idmgL = level;
    idmgH = 3*level/2;
    cdmgL = level/5;
    cdmgH = level/2;
    cdurL = 2;
    cdurH = 4;
    }
  

  if( failure )
    {
    send_to_char( "You poison yourself!\n\r", ch );
    CREATE(pd, POISON_DATA, 1);
    pd->next = ch->pcdata->poison;
    ch->pcdata->poison = pd;
    pd->for_npc = FALSE;
    pd->poison_type = ptype;
    pd->instant_damage_low = idmgL;
    pd->instant_damage_high = idmgH;
    pd->constant_damage_low = cdmgL;
    pd->constant_damage_high = cdmgH;
    pd->constant_duration = number_range( cdurL, cdurH );
    if( IS_NPC( ch ) )
      pd->poisoner = 0;
    else
      pd->poisoner = ch->pcdata->pvnum;
    if( IS_NPC( ch ) )
      pd->owner = 0;
    else
      pd->owner = ch->pcdata->pvnum;
    return;
    }
  
  if( obj->item_type == ITEM_AMMO )
    {
    idmgL /= 2;
    idmgH /= 2;
    cdmgL /= 3;
    cdmgH /= 3;
    cdurL /= 2;
    cdurH /= 2;
    }

  sprintf( buf, "You make %s %s poison and place it on %s.\n\r",
      ( *arg1=='c' || *arg1=='C' ) ? "clear" : "murky" ,
      ( *arg2=='p' || *arg2=='P' ) ? "purple" : 
      ( *arg2=='g' || *arg2=='G' ) ? "green" : "blue",
      obj->short_descr );
  send_to_char( buf, ch );
  CREATE(pd, POISON_DATA, 1);
  pd->next = obj->poison;
  obj->poison = pd;
  obj->basic = FALSE;
  if( *arg1 == 'c' || *arg1 == 'C' )
    pd->for_npc = FALSE;
  else
    pd->for_npc = TRUE;
  pd->poison_type = ptype;
  pd->instant_damage_low = idmgL;
  pd->instant_damage_high = idmgH;
  pd->constant_damage_low = cdmgL;
  pd->constant_damage_high = cdmgH;
  pd->constant_duration = number_range( cdurL, cdurH );
  if( IS_NPC( ch ) )
    pd->owner = 0;
  else
    pd->owner = ch->pcdata->pvnum;
  pd->poisoner = 0;

  return;
}

void do_make_flash( CHAR_DATA *ch, char *argument )
{
  /* CHAR_DATA *mob; */
  int cnt;

  cnt = multi( ch, gsn_flash_powder );
  if( cnt == -1 )
    {
    send_to_char( "You can't make flash powder.\n\r", ch );
    return;
    }

/*
  for ( mob = ch->in_room->first_person; mob != NULL; mob = mob->next_in_room )
  {
    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
     break;
  }

  if ( mob == NULL )
  {
   send_to_char( "You can only make flash powder in a guild.\n\r", ch );
   return;
  } */

  if (IS_AFFECTED(ch, AFF2_HAS_FLASH))
  { 
   send_to_char( "You already have some flash powder hidden away.\n\r",ch);
   return;
  }
  if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_flash_powder])
  {
   SET_BIT(ch->affected2_by, 0-AFF2_HAS_FLASH);
   send_to_char( "You make some flash powder and hide it about your person.\n\r", ch);
  }
  else
  {
   send_to_char( "BOOM! You accidentally put together the wrong components.\n\r", ch);
   damage(ch, ch, number_range(1,80), TYPE_UNDEFINED);
  }
 return;
}


/*
 * DO_BANK
 *
 * This code manages the character's bank account in pcdata.
 *
 * Presto ?-?-97
 */
void do_bank( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *target;
  char choice[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char amt[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int  amount      = 0;
  int  account_max = 0;
  int  tar_max     = 0;

  if ( !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
      send_to_char("You're not in a bank.\n\r", ch);
      return;
    }
  if(ch->pcdata->castle != NULL)
    account_max = (ch->level + ch->pcdata->castle->num_rooms) * 2000000;
  else
    account_max = ch->level * 2000000;

  argument=one_argument(argument, choice);
  argument=one_argument(argument, amt);
  amount  =atoi(amt);
  if(amount < 0)
    amount = 0 - amount;

  if(*choice == '\0' || *choice == ' ')
    *choice = 'b';

  switch(*choice)
    {
    /*Deposit*/
    case 'd': case 'D':
      if(amount > ch->gold)
        {
          send_to_char("You don't have that much gold.\n\r", ch);
          return;
        }
      else
        if(amount == 0)
          {
            send_to_char("What good would that do?\n\r", ch);
            return;
          }
        else
          {
            if(ch->pcdata->account + amount > account_max)
              amount = account_max - ch->pcdata->account;
            ch->gold -= amount;
            ch->pcdata->account += amount;
            save_char_obj(ch, NORMAL_SAVE);
            sprintf(buf, "You deposit %d gold coins into your account.\n\rYour account balance is now %d gold coins.\n\r", amount, ch->pcdata->account);
            send_to_char(buf, ch);
          }
      break;

    /*Withdraw*/
    case 'w': case 'W':
      if(amount > ch->pcdata->account)
        {
          send_to_char("You don't have that much gold in your account.\n\r",
                       ch);
          return;
        }
      else
        if(amount == 0)
          {
            send_to_char("What good would that do?\n\r", ch);
            return;
          }
        else
          {
            if(ch->gold + amount > ch->level*1000000)
            {
              send_to_char("You can't carry that much gold.\n\r", ch);
              amount=ch->level*1000000 - ch->gold;
            }
            ch->gold += amount;
            ch->pcdata->account -= amount;
            save_char_obj(ch, NORMAL_SAVE);
            sprintf(buf, "You withdraw %d gold coins from your account.\n\rYour account balance is now %d gold coins.\n\r", amount, ch->pcdata->account);
            send_to_char(buf, ch);
          }
      break;

    /*Balance*/
    case 'b': case 'B':
      sprintf(buf, "You have %d gold coins in your account.\n\r",
              ch->pcdata->account);
      send_to_char(buf, ch);
      break;

    /*Transfer*/
    case 't': case 'T':
      argument=one_argument(argument, arg);
      if(*arg == '\0')
        {
          send_to_char("You must specify a target.\n\r", ch);
          return;
        }
      if((target=get_char_room(ch, arg)) == NULL)
        {
          send_to_char("They must be in the bank with you.\n\r", ch);
          return;
        }
      if(IS_NPC(target))
        {
          send_to_char("Only players have accounts, silly.\n\r", ch);
          return;
        }
      if(ch->pcdata->account < amount)
        {
          send_to_char("You don't have enough gold in your account.\n\r", ch);
          return;
        }
      if(amount == 0)
        {
          send_to_char("What good would that do?\n\r", ch);
          return;
        }
      if(target->pcdata->castle != NULL)
        tar_max=2000000 * (target->level + target->pcdata->castle->num_rooms);
      else
        tar_max=target->level * 2000000;
      if(target->pcdata->account + amount > tar_max)
        {
          send_to_char("Their account cannot hold that much.\n\r", ch);
          amount=tar_max - target->pcdata->account;
        }
      ch->pcdata->account -= amount;
      target->pcdata->account += amount;
      save_char_obj(target, NORMAL_SAVE);
      save_char_obj(ch, NORMAL_SAVE);
      sprintf(buf, "You transfer %d coins into %s's account.\n\rYour account balance is now %d gold coins.\n\r", amount, target->name, ch->pcdata->account);
      send_to_char(buf, ch);
      sprintf(buf, "%s transfers %d coins into your account.\n\rYour account balance is now %d gold coins.\n\r", ch->name, amount, target->pcdata->account);
      send_to_char(buf, target);
      break;

    default:
      send_to_char("Usage: bank <deposit/withdraw/balance/transfer> [amount] [target]\n\r", ch);
      break;
    }
  return;
}


int give_gold(CHAR_DATA* ch, int amount)
{
 int num;
 if (!IS_NPC(ch) && ch->pcdata->clan !=NULL && ch->pcdata->clan->tax>0 &&
     ch->pcdata->clan->tax<=100)
 {
  num = (float)amount * ((float)ch->pcdata->clan->tax/100);
  if (num>0 && ch->pcdata->clan->coffers+num<2000000000)
  {
   ch->pcdata->clan->coffers+=num;
   amount -= num;
   ch->gold+=amount;
   return num;
  }
  else
  {
   ch->gold+=amount;
   return 0;
  }
 }
 else
  ch->gold+=amount;

 return 0;
}



/*
 * BOUNTY
 *
 * This code allows players to post bounties on the heads of other players.
 * In its purest form, it promotes player-killing, whether from god-wars,
 * race-wars, clan-wars, or assassinating.  Minimum bounty is 1 million gold
 * to prevent characters from cluttering the board with insignificant bounties.
 *
 * Presto 2/13/99
 */
void do_bounty( CHAR_DATA *ch, char *argument )
{
  char choice[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char amt[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  amount = 0, column, days, hours, mins;
  CHAR_DATA * victim;
  BOUNTY_DATA *bptr;
  bool loaded = FALSE;

  argument=one_argument(argument, choice);

  if(IS_NPC(ch))
  {
    send_to_char("Mobiles may not use the bounty system.\n\r", ch);
    return;
  }

  if(*choice == 'p' || *choice == 'P')
  {
    argument=one_argument(argument, amt);
    amount  =atoi(amt);
    if(amount < 0)
      amount = 0 - amount;

    if(amount == 0)
    {
      send_to_char("You can't post a bounty of 0 gold.\n\r", ch);
      return;
    }

    if(amount < 1000000)
    {
      send_to_char("The minimum bounty is 1 million gold coins.\n\r", ch);
      return;
    }

    argument=one_argument(argument, arg);
    if(*arg == '\0' || *arg==' ')
    {
      send_to_char("Usage: bounty <post/list/info> [amount] [target]\n\r", ch);
      return;
    }
    
    victim=lookup_char(arg);
    if(victim==NULL)
    {
      victim = start_partial_load(ch, arg);
      if(victim == NULL)
        return;
      loaded = TRUE;
    }

    if(IS_IMMORTAL(victim))
    {
      send_to_char("You may not post bounties on Immortals.\n\r", ch);
      if(loaded)
        clear_partial_load(victim);
      return;
    }

    if(which_god(victim)==GOD_INIT_ORDER || which_god(victim)==GOD_INIT_CHAOS)
    {
      send_to_char("You may not post bounties on Initiates.\n\r", ch);
      if(loaded)
        clear_partial_load(victim);
      return;
    }

    if(which_god(victim) == GOD_POLICE)
    {
      send_to_char("You may not post bounties on Sherrifs.\n\r", ch);
      if(loaded)
        clear_partial_load(victim);
      return;
    }

    if(!gold_transaction(ch, 0-amount))
    {
      send_to_char("You don't have that much gold.\n\r", ch);
      if(loaded)
        clear_partial_load(victim);
      return;
    }

    post_bounty(arg, amount);
    save_bounties();
    save_char_obj(ch, NORMAL_SAVE);
    send_to_char("The bounty has been posted.\n\r", ch);
    if(loaded)
      clear_partial_load(victim);
    return;
  }
  else if(*choice == 'i' || *choice == 'I')
  {
    argument=one_argument(argument, arg);
    if(!(bptr=get_bounty(arg)))
    {
      send_to_char("There is no bounty posted on that person.\n\r", ch);
      return;
    }

    amount=bptr->postdate + bptr->expires - current_time;
    if(amount < 0)
    {
      send_to_char("That bounty has just expired.\n\r", ch);
      remove_bounty(bptr);
      return;
    }

    sprintf(buf, "The bounty of %d on %s expires in", bptr->amount,
            capitalize(bptr->name));
    days    =amount / 86400;
    amount -=days   * 86400;
    hours   =amount / 3600;
    amount -=hours  * 3600;
    mins    =amount / 60;
    if(days > 0)
      sprintf(buf, "%s %d days", buf, days);
    if(hours > 0)
    {
      if(days > 0)
      {
        if(mins > 0)
          sprintf(buf, "%s, %d hours", buf, hours);
        else
          sprintf(buf, "%s and %d hours", buf, hours);
      }
      else
        sprintf(buf, "%s %d hours", buf, hours);
    }
    if(mins > 0)
    {
      if(days > 0 || hours > 0)
        sprintf(buf, "%s and %d minutes", buf, mins);
      else
        sprintf(buf, "%s %d minutes", buf, mins);
    }
    strcat(buf, ".\n\r");
    send_to_char(buf, ch);
    return;
  }
  else
  {
    sprintf(buf,  "{170}Player______  {130}___Bounty   {170}Player______  {130}___Bounty   {170}Player______  {130}___Bounty\n\r");
    column=1;
    for(bptr=first_bounty; bptr; bptr=bptr->next)
    {
      if(column == 1)
      {
        sprintf(buf2, "{070}%-12s  {030}%9d", capitalize(bptr->name), bptr->amount);
        column = 2;
      }
      else if(column == 2)
      {
        sprintf(buf2, "   {070}%-12s  {030}%9d", capitalize(bptr->name),bptr->amount);
        column = 3;
      }
      else
      {
        sprintf(buf2, "   {070}%-12s  {030}%9d\n\r", capitalize(bptr->name),bptr->amount);
        column = 1;
      }
      strcat(buf, buf2);
    }
    if(column > 1)
      strcat(buf, "\n\r");

    send_to_char_color(buf, ch);
  }

  return;
}



/*
 * POST_BOUNTY
 *
 * This adds a new bounty to the current list of bounties.  A bounty cannot
 * exceed a value of 335 million, since a level 95 character with a 25 room
 * castle can only carry 95 million on his character, and have 240 million
 * in the bank.
 *
 * Presto 2/20/99
 */
void post_bounty(char *name, int amount)
{
  BOUNTY_DATA *bptr;

  /* Increase the value of a current bounty */
  for(bptr=first_bounty; bptr; bptr=bptr->next)
  {
    if(!strcasecmp(bptr->name, name))
    {
      if(bptr->amount == 335000000)
        return;
      bptr->amount += amount;
      if(bptr->amount > 335000000)
        bptr->amount = 335000000;
      bptr->postdate = current_time;
      bptr->expires = 604800 + bptr->amount / 100;
      return;
    }
  }

  /* Add a new bounty */
  CREATE(bptr, BOUNTY_DATA, 1);
  bptr->name   =STRALLOC(capitalize(name));
  bptr->amount =amount;
  bptr->postdate = current_time;
  bptr->expires=604800 + amount / 100;
  sort_bounty(bptr);
  return;
}



/*
 * SORT_BOUNTY
 *
 * Sort 'em alphabetically.
 *
 * Martin 3/6/99
 */
void sort_bounty(BOUNTY_DATA *bptr)
{
  BOUNTY_DATA *temp_bounty;

  if(!bptr)
  {
    bug("Sort_bounty: NULL bptr");
    return;
  }

  for(temp_bounty = first_bounty; temp_bounty; temp_bounty = temp_bounty->next)
  {
    if(strcmp(bptr->name, temp_bounty->name) < 0)
    {
      INSERT(bptr, temp_bounty, first_bounty, next, prev);
      break;
    }
  }

  if(!temp_bounty)
  {
    LINK(bptr, first_bounty, last_bounty, next, prev);
  }

  return;
}



/*
 * REMOVE_BOUNTY
 *
 * When a bounty is paid or expires, delete it.
 *
 * Presto 2/20/99
 */
void remove_bounty(BOUNTY_DATA *bptr)
{
  UNLINK(bptr, first_bounty, last_bounty, next, prev);
  return;
}

/*
 * GET_BOUNTY
 *
 * This function gets the value of a bounty for a specified player.
 *
 * Presto 2-20-99
 */
BOUNTY_DATA *get_bounty(char *name)
{
  BOUNTY_DATA *bptr;

  for(bptr=first_bounty; bptr; bptr=bptr->next)
    if(!strcasecmp(bptr->name, name))
      return(bptr);

  return NULL;
}

/*
 * GOLD_TRANSACTION
 *
 * This function adds or subtracts gold from the player, starting first with
 * the player's carried amount, and then their account if necessary.
 * If the transaction is a success, the function returns 1, else 0.
 *
 * NOTE: The player will never have more gold than they can carry on them or
 * have in their account.
 *
 * Presto 2/13/99
 */
int gold_transaction(CHAR_DATA *ch, int amount)
{
  int max_account, max_pocket;

  if(IS_NPC(ch))
    return FALSE;

  if(amount < 0)
  {
    amount = 0 - amount;
    if(ch->gold >= amount)
    {
      ch->gold -= amount;
      return TRUE;
    }
    else if(ch->gold + ch->pcdata->account >= amount)
    {
      amount -= ch->gold;
      ch->gold = 0;
      ch->pcdata->account -= amount;
      return TRUE;
    }
    else
      return FALSE;
  }
  else
  {
    max_pocket = ch->level * 1000000 - ch->gold;
    if(ch->pcdata->castle != NULL)
      max_account = (ch->level + ch->pcdata->castle->num_rooms) * 2000000;
    else
      max_account = ch->level * 2000000;

    if(amount <= max_pocket)
      ch->gold += amount;
    else
    {
      amount -= max_pocket;
      ch->gold = ch->level * 1000000;
      ch->pcdata->account += amount;
      if(ch->pcdata->account > max_account)
        ch->pcdata->account = max_account;
    }
    return TRUE;
  }
}

void do_bet( CHAR_DATA *ch, char *arg)
{
  int value;
  char name[180], values[180], buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim, *old_victim;

  arg=one_argument( arg, name);
  arg=one_argument( arg, values);

if( IS_NPC( ch ))
  return;

  if( name[0]=='\0' || values[0]=='\0' )
    {
    PLAYER_GAME *gpl;
    char name1[180], name2[180];
    send_to_char( "Bet Roster             Who with                      Amount     Status\n\r", ch);
    for( gpl=first_player; gpl!=NULL; gpl=gpl->next )
        {
        if( gpl->ch->pcdata->bet_mode == 0 )
          continue;
        old_victim = gpl->ch->pcdata->bet_victim;
        sprintf( name1, "%s [%d]", capitalize(gpl->ch->name),  gpl->ch->level);
        sprintf( name2, "%s [%d]", capitalize(old_victim->name),
               old_victim->level);
        if( gpl->ch->pcdata->bet_mode == 2 )
          {
          if( old_victim->position == POS_FIGHTING && 
              gpl->ch->position == POS_FIGHTING &&
	      gpl->ch->fighting &&
              gpl->ch->fighting->who == old_victim &&
              old_victim->fighting->who == gpl->ch )
            strcpy( buf, "FIGHTING");
          else
            strcpy( buf, "ACCEPTED");
          sprintf( name, "%-23s%-23s %12d     %s\n\r", 
             name1, name2,
             gpl->ch->pcdata->bet_amount, buf );
          old_victim->pcdata->bet_mode = 3;  /* temporary till next line */
          send_to_char( name, ch);
          continue;
          }
        if( gpl->ch->pcdata->bet_mode == 3 ) /* fix mode back to normal */
          gpl->ch->pcdata->bet_mode = 2;
        if( gpl->ch->pcdata->bet_mode == 1 )
          {
          sprintf( name, "%-23s%-23s %12d     OFFER %s\n\r", 
             name1, name2,
             gpl->ch->pcdata->bet_amount, buf );
          send_to_char( name, ch);
          }
        }
    return;
    }

  if( ch->in_room->area->low_r_vnum != ROOM_VNUM_ARENA )
    {
    send_to_char( "You are not in the arena.\n\r", ch);
    return;
    }

  victim=lookup_char( name );
  if( victim == NULL || victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA ||
      IS_NPC( victim ))
    {
    send_to_char( "You cannot bet that person.\n\r", ch);
    return;
    }

  value=atol( values );

  if( value < 1 || value > ch->gold )
    {
    send_to_char( "You cannot bet that amount!\n\r", ch);
    return;
    }

  if( ch->pcdata->bet_mode == 2 )
    {
    send_to_char( "You are already in a bet.\n\r", ch);
    return;
    }

  if( victim->pcdata->bet_mode == 2 )
    {
    send_to_char( "They are already in a bet.\n\r", ch);
    return;
    }

  if( victim->gold < value )
    {
    send_to_char( "Your victim cannot cover that bet.\n\r", ch);
    value=( victim->gold * 10 / value );
    if( value == 0)
      value=1;
    if( value == 10 )
      value=9;
    sprintf( name, "Try about %d/10th of that amount.\n\r", value );
    send_to_char( name, ch);
    return;
    }

  if( ch->position == POS_FIGHTING || ch->pcdata->bet_mode == 2)
    {
    send_to_char( "You cannot bet right now!\n\r", ch);
    return;
    }

  if( victim->position == POS_FIGHTING || victim->pcdata->bet_mode == 2)
    {
    sprintf( name, "%s cannot bet right now!\n\r", get_name( victim ));
    send_to_char( name, ch);
    return;
    }

  old_victim=NULL;
  if( ch->pcdata->bet_mode == 1 && is_desc_valid( ch->pcdata->bet_victim ))
    old_victim = ch->pcdata->bet_victim;

  if( victim == ch )
    {
    if( ch->pcdata->bet_mode == 0)
      {
      send_to_char( "You cannot bet yourself!\n\r", ch);
      return;
      }
    if( ch->pcdata->bet_mode == 1)
      {
      if( old_victim != NULL )
        {
        send_to_char( "You cancel your bet!\n\r", ch);
        sprintf( name, "%s cancels his bet with you.\n\r",
            get_name( ch ));
        send_to_char( name, old_victim);
        ch->pcdata->bet_mode = 0;
        ch->pcdata->bet_victim = NULL;
        ch->pcdata->bet_amount = 0;
        return;
        }
      send_to_char( "You cancel your bet!\n\r", ch);
      ch->pcdata->bet_mode = 0;
      ch->pcdata->bet_victim = NULL;
      ch->pcdata->bet_amount = 0;
      return;
      }
    }
  if( old_victim != NULL && old_victim != victim )
    {
    sprintf( name, "You cancel your bet with %s!\n\r", 
           get_name( old_victim ));
    send_to_char( name, ch );
    sprintf( name, "%s cancels his bet with you.\n\r", get_name( ch ));
    send_to_char( name, old_victim);
    ch->pcdata->bet_mode = 0;
    ch->pcdata->bet_victim = NULL;
    ch->pcdata->bet_amount = 0;
    }


  sprintf( name, "%s bets you %d for your life!\n\r", get_name( ch ),
      value);
  send_to_char( name, victim );
  sprintf( name, "You bet %s for the amount of %d.\n\r",
      get_name( victim ), value);
  send_to_char( name, ch );

  ch->pcdata->bet_mode = 1;
  ch->pcdata->bet_victim = victim;
  ch->pcdata->bet_amount = value;

  return;
}

void do_resign( CHAR_DATA *ch, char *arg )
{
  CHAR_DATA *victim;
  char buf[180];

       /* char_to_room also calls with a NULL is not ARENA room */
       /* extract_char calls this routine if required with a NULL for arg   */

  if( IS_NPC( ch ))
    return;
  if( ch->pcdata->bet_mode != 2)
    {
    if( arg != NULL)
      send_to_char( "You are not betting anyone.\n\r", ch);
    return;
    }
  victim=NULL;
  if( is_desc_valid( ch->pcdata->bet_victim ))
    victim=ch->pcdata->bet_victim;
  if( victim == NULL)
    {
    ch->pcdata->bet_mode = 0;
    ch->pcdata->bet_victim = NULL;
    ch->pcdata->bet_amount = 0;
    return;
    }

  if( arg!= NULL && ch->position>POS_RESTING && ch->position!=POS_FIGHTING &&
     victim->position>POS_RESTING && victim->position!=POS_FIGHTING)
    {
    send_to_char( "You resign your bet!\n\r", ch);
    sprintf( buf, "%s resigns his bet!\n\r", get_name( ch ));
    send_to_char( buf, victim);
    }
  sprintf( buf, "You lose your bet with %s for %d coins.\n\r",
      get_name( ch->pcdata->bet_victim ), ch->pcdata->bet_amount);
  send_to_char( buf, ch);
  sprintf( buf, "You win your bet with %s for %d coins.\n\r",
      get_name( ch ), ch->pcdata->bet_amount);
  send_to_char( buf, victim);
  {
  PLAYER_GAME *gpl;
  char bufc[256];
  for( gpl=first_player; gpl!=NULL; gpl=gpl->next )
    if( gpl->ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA  &&
        gpl->ch != victim && gpl->ch != ch )
      {
      strcpy( bufc, get_name( ch ));
      if( arg==NULL)
        sprintf( buf, "%s loses to %s for the amount of %d.\n\r",
            bufc, get_name( victim ), 
            ch->pcdata->bet_amount);
      else
        sprintf( buf, "%s resigns to %s for the amount of %d.\n\r",
            bufc, get_name( victim ),  
            ch->pcdata->bet_amount);
      send_to_char( buf, gpl->ch );
      }
  }

  ch->gold -= ch->pcdata->bet_amount;
  if( ch->gold<0 )
    {
    victim->gold+=ch->gold;
    ch->gold = 0;
    }
  victim->gold += ch->pcdata->bet_amount;

  ch->pcdata->bet_mode = 0;
  ch->pcdata->bet_victim = NULL;
  ch->pcdata->bet_amount = 0;

  victim->pcdata->bet_mode = 0;
  victim->pcdata->bet_victim = NULL;
  victim->pcdata->bet_amount = 0;

  return;
}

void do_accept( CHAR_DATA *ch, char *arg)
{
  int value;
  char name[180];
  CHAR_DATA *victim;

  arg=one_argument( arg, name);

if( IS_NPC( ch ))
  return;

  if( name[0]=='\0' )
    {
    send_to_char( "Syntax:  ACCEPT <who>\n\rYou must be in the Arena to accept a bet.\n\r", ch);
    return;
    }

  if( ch->in_room->area->low_r_vnum != ROOM_VNUM_ARENA )
    {
    send_to_char( "You are not in the arena.\n\r", ch);
    return;
    }

  victim=lookup_char( name );
  if( victim == NULL || victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA ||
      IS_NPC( victim ))
    {
    send_to_char( "That person has not bet you.\n\r", ch);
    return;
    }

  if( ch->pcdata->bet_mode == 2 )
    {
    send_to_char( "You are already in a bet.\n\r", ch);
    return;
    }

  if( victim->pcdata->bet_mode == 2 )
    {
    send_to_char( "They are already in a bet.\n\r", ch);
    return;
    }

  if( ch != victim->pcdata->bet_victim )
    {
    send_to_char( "That person has not bet you.\n\r", ch);
    return;
    }


  value=victim->pcdata->bet_amount;

  if( value < 1 || value > ch->gold  || value > victim->gold )
    {
    send_to_char( "You cannot accept that amount!\n\r", ch);
    return;
    }

  if( ch->position == POS_FIGHTING)
    {
    send_to_char( "You cannot accept right now!\n\r", ch);
    return;
    }

  if( victim->position == POS_FIGHTING)
    {
    sprintf( name, "%s cannot be accepted right now!\n\r",get_name(victim));
    send_to_char( name, ch);
  }

  sprintf( name, "%s accepts your bet of %d.\n\r", get_name( ch ), value);
  send_to_char( name, victim );
  sprintf( name, "You accept %s's for the amount of %d.\n\r",
      get_name( victim ), value);
  send_to_char( name, ch );

  {
  PLAYER_GAME *gpl;
  char bufc[256];
  for( gpl=first_player; gpl!=NULL; gpl=gpl->next )
    if( gpl->ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA &&
        gpl->ch != victim && gpl->ch != ch )
      {
      strcpy( bufc, get_name( ch ));
      sprintf( name, "%s accepts %s's bet for the amount of %d.\n\r",
          bufc, get_name( victim ), value);
      send_to_char( name, gpl->ch );
      }
  }
  ch->pcdata->bet_mode = 2;
  victim->pcdata->bet_mode = 2;
  ch->pcdata->bet_victim = victim;
  ch->pcdata->bet_amount = value;

  return;
}

void damage_equipment( CHAR_DATA *ch , bool fWeapon )
  {
  OBJ_DATA *obj;
  char buf[MAX_INPUT_LENGTH];

  if( !fWeapon )
    {
    obj = get_eq_char( ch, number_range(1,15));
    if( obj==NULL )
      return;

    if( IS_SET(obj->extra_flags,ITEM_QUEST) ||
        obj->item_type != ITEM_ARMOR ||
        IS_SET(obj->extra_flags, ITEM_INVENTORY) )
      return;

    }
  else
    {
    obj = get_eq_char( ch, WEAR_WIELD );
    if( obj==NULL )
      return;
    
      if( obj->item_type != ITEM_WEAPON ||
          IS_SET(obj->extra_flags,ITEM_QUEST) ||
          IS_SET(obj->extra_flags, ITEM_INVENTORY) )
      return;
    }

  obj->condition -= number_range( 1,5 );
  if( obj->condition<0 )
    obj->condition = 0;

  if( ch->level > 50 )
    {
    if( obj->condition > 0 )
      sprintf( buf, "{170}%s is damaged and now has a condition of %d%%.\n\r",
         capitalize(obj->short_descr), obj->condition );
    else
      {
      sprintf( buf, "{170}%s is damaged beyond repair.\n\r", 
               capitalize(obj->short_descr));
      SET_BIT( obj->extra_flags, ITEM_NOT_VALID );
      }
    }
  else
    {
        if( obj->condition >= 90 )
          sprintf( buf, "{170}%s is damaged, but is still in great condition.\n\r",
               capitalize(obj->short_descr));
        else if( obj->condition >= 70 )
          sprintf( buf, "{170}%s is damaged, but is in good condition.\n\r",
               capitalize(obj->short_descr));
        else if( obj->condition >= 50 )
          sprintf( buf, "{170}%s is damaged and is in fair condition.\n\r",
               capitalize(obj->short_descr));
        else if( obj->condition >= 30 )
          sprintf( buf, "{170}%s is damaged and now is in poor condition.\n\r",
               capitalize(obj->short_descr));
        else if( obj->condition >= 10 )
          sprintf( buf, "{170}%s is damaged and is in bad condition.\n\r",
               capitalize(obj->short_descr));
        else if( obj->condition >= 1 )
          sprintf( buf, "{170}%s is damaged and is about to fall apart.\n\r",
               capitalize(obj->short_descr));
        else 
          {
          sprintf( buf, "{170}%s is damaged beyond repair.\n\r",
               capitalize(obj->short_descr));
          SET_BIT( obj->extra_flags, ITEM_NOT_VALID );
          }
    }

  send_to_combat_char( ansi_translate_text(ch,buf), ch );

  return;
  }


void do_repair( CHAR_DATA *ch, char *argument )
{

#ifdef EQUIPMENT_DAMAGE
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int cost, percent;

    if( IS_NPC( ch ))
      return;


    if( ch->pcdata->learned[gsn_repair]<=0 && ch->in_room->vnum != 9704 )
      {
      send_to_char( "You are not at the blacksmith.\n\r", ch);
      return;
      }

    if( argument[0]=='\0' )
      {
      send_to_char( "You can only repair an object.\n\r", ch);
      return;
      }

    argument = one_argument( argument, arg );
    obj = get_obj_list( ch, arg, ch->first_carrying );

    if( obj==NULL)
      {
      send_to_char( "You do not have that object.\n\r", ch);
      return;
      }

    if( ch->pcdata->learned[gsn_repair]<=0 )
      {
      cost=obj->level*obj->level*10+obj->cost/2;
      cost = cost + (100 - obj->condition)*cost/100;
      percent = 99;
      }
    else
      {
      cost=0;
      percent = 25+ch->pcdata->learned[gsn_repair]/2;
      }

    if( ch->gold < cost)
      {
      send_to_char( "You do not have enough gold to repair that object.\n\r", ch);
      return;
      }
    
	  switch ( obj->item_type )
	    {
	    case ITEM_ARMOR:
	       break;
	    case ITEM_WEAPON:
	       break;
	    default:
	      send_to_char( "You cannot repair that kind of an object.\n\r",ch);
	      return;
	    }
      ch->gold-=cost;

    obj->condition = percent;

    if( percent < 99 )
      sprintf( arg, "%s is repaired to %d%%.\n\r", 
         capitalize(obj->short_descr),percent );
    else
      sprintf( arg, "%s is repaired for %d gold.\n\r", 
         capitalize(obj->short_descr),cost );
    send_to_char( arg, ch );
    return;
#else
    send_to_char( "Repair is disabled.\n\r", ch );
    return;
#endif
   }
