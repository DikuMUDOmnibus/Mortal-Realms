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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include <pthread.h>

/*
 * Local functions.
 */
void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void  obj_set_to_char( CHAR_DATA *, OBJ_DATA * );



/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{

    if( IS_NPC(ch) && ch->pIndexData->vnum!=9900 && !IS_AFFECTED(ch,AFF_CHARM)  
        && !IS_AFFECTED(ch, AFF2_POSSESS) && ch->desc==NULL )
       return( 99 );

    if ( ch->desc != NULL && ch->desc->original != NULL )
	return (ch->desc->original->level);

    if( IS_NPC(ch) )
      return(1);

    if( ch->trust<ch->level )
      ch->trust = ch->level;
    return (ch->trust);
}



/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return (17 + (ch->played  / 20000));
}



/*
 * Retrieve character's current strength.
 */
int get_curr_str( CHAR_DATA *ch )
{
    int max;

    if ( IS_NPC(ch) )
	return 13;

    if ( class_table[ch->class].attr_prime == APPLY_STR )
	max = 22+ch->level/9;
    else
	max = 19+ch->level/9;

    max += race_table[ ch->race ].race_mod[0];

    return URANGE( 3, ch->pcdata->perm_str + ch->pcdata->mod_str, max );
}



/*
 * Retrieve character's current intelligence.
 */
int get_curr_int( CHAR_DATA *ch )
{
    int max;

    if ( IS_NPC(ch) )
	return 13;

    if ( class_table[ch->class].attr_prime == APPLY_INT )
	max = 22+ch->level/9;
    else
	max = 19+ch->level/9;

    max += race_table[ ch->race ].race_mod[2];

    return URANGE( 3, ch->pcdata->perm_int + ch->pcdata->mod_int, max );
}



/*
 * Retrieve character's current wisdom.
 */
int get_curr_wis( CHAR_DATA *ch )
{
    int max;

    if ( IS_NPC(ch) )
	return 13;

    if ( class_table[ch->class].attr_prime == APPLY_WIS )
	max = 22+ch->level/9;
    else
	max = 19+ch->level/9;

    max += race_table[ ch->race ].race_mod[3];

    return URANGE( 3, ch->pcdata->perm_wis + ch->pcdata->mod_wis, max );
}



/*
 * Retrieve character's current dexterity.
 */
int get_curr_dex( CHAR_DATA *ch )
{
    int max;

    if ( IS_NPC(ch) )
	return 13;

    if ( class_table[ch->class].attr_prime == APPLY_DEX )
	max = 22+ch->level/9;
    else
	max = 19+ch->level/9;

    max += race_table[ ch->race ].race_mod[1];

    return URANGE( 3, ch->pcdata->perm_dex + ch->pcdata->mod_dex, max );
}



/*
 * Retrieve character's current constitution.
 */
int get_curr_con( CHAR_DATA *ch )
{
    int max;

    if ( IS_NPC(ch) )
	return 13;

    if ( class_table[ch->class].attr_prime == APPLY_CON )
	max = 22+ch->level/9;
    else
	max = 19+ch->level/9;

    max += race_table[ ch->race ].race_mod[4];

    return URANGE( 3, ch->pcdata->perm_con + ch->pcdata->mod_con, max );
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR + 2 * get_curr_dex( ch ) / 3+ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[get_curr_str(ch)].carry;
}



/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( *name == '\0' )
	    return FALSE;
	if ( !strcasecmp( (char *)str, name ) )
	    return TRUE;
    }
}

bool is_name_short( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];
    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	name[strlen(str)]='\0';
	if ( *name == '\0' || *(name+1)=='\0')
	    return FALSE;
	if ( !strcasecmp( (char *)str, name ) )
	    return TRUE;
    }
}

bool is_object_reference( OBJ_DATA *obj, char *argument )
  {
  int number;
    if( *argument=='#'  &&
        ( *(argument+1)>='0' && *(argument+1)<='9' ) &&
        ( *(argument+2)>='0' && *(argument+2)<='9' ) &&
        ( *(argument+3)>='0' && *(argument+3)<='9' ) )
      {
      number = (*(argument+1)-'0')*100 + (*(argument+2)-'0')*10 + 
               (*(argument+3)-'0');
          if( !obj->basic || obj->item_type==ITEM_ARMOR ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_WEAPON )
	    if( number==obj->index_reference[0]%1000 )
	      return TRUE;
      }

      return( FALSE );
      }


bool is_short_of_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];
    char str2[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        strcpy(str2,str);
	namelist = one_argument( namelist, name );
	if ( *name == '\0' || *(name+1)=='\0')
	    return FALSE;
	str2[strlen(name)]='\0';
	if ( !strcasecmp( str2, name ) )
	    return TRUE;
    }
}

bool is_short_of_name_reverse( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];
    char *str2,*strEnd;

    strEnd=(char *)str+strlen(str);
    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( *name == '\0' || *(name+1)=='\0')
	    return FALSE;
        str2=strEnd-strlen(name);
	if ( !strcasecmp( str2, name ) )
	    return TRUE;
    }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
  {
  /*OBJ_DATA *wield;*/
  int mod;


  if( paf == NULL )
    return;

  mod = paf->modifier;


  if ( fAdd )
    {
       if( paf->bitvector < 0 )
	  SET_BIT( ch->affected2_by, 0-paf->bitvector );
       else
	  SET_BIT( ch->affected_by, paf->bitvector );
    }
  else
    {
       if( paf->bitvector < 0 )
	  REMOVE_BIT( ch->affected2_by, 0-paf->bitvector );
       else
	  REMOVE_BIT( ch->affected_by, paf->bitvector );
	  mod = 0 - mod;
    }

  if( IS_SET( paf->bitvector, AFF_HASTE )  && paf->bitvector > 0 )
    ch->speed = get_max_speed( ch );

  /*if ( IS_NPC(ch) )
	  return;
*/
  switch ( paf->location )
    {
    default:
	  bug( "Affect_modify: unknown location %d.", paf->location );
	  return;

    case APPLY_NONE:                                            break;
    case APPLY_STR:           if (!IS_NPC(ch)) ch->pcdata->mod_str       += mod; break;
    case APPLY_DEX:           if (!IS_NPC(ch)) ch->pcdata->mod_dex       += mod; break;
    case APPLY_INT:           if (!IS_NPC(ch)) ch->pcdata->mod_int       += mod; break;
    case APPLY_WIS:           if (!IS_NPC(ch)) ch->pcdata->mod_wis       += mod; break;
    case APPLY_CON:           if (!IS_NPC(ch)) ch->pcdata->mod_con       += mod; break;
    case APPLY_SEX:           ch->sex                   += mod;
	    if( ch->sex>2)
	      ch->sex=0;
	    if( ch->sex<0)
	      ch->sex=2;
		break;
    case APPLY_CLASS:                                           break;
    case APPLY_LEVEL:                                           break;
    case APPLY_AGE:                                               break;
    case APPLY_HEIGHT:                                  break;
    case APPLY_WEIGHT:                                  break;
    case APPLY_MANA:          ch->max_mana              += mod; 
			      if(ch->max_mana<1)
				{
				ch->max_mana=1;
				break;
				}
			      if(ch->max_mana<ch->mana)
				ch->mana=ch->max_mana;
			      break;
    case APPLY_HIT:           ch->max_hit               += mod;
			      if(ch->max_hit<1)
				{
				ch->max_hit=1;
				break;
				}
			      if(ch->max_hit<ch->hit)
				ch->hit=ch->max_hit;
			      break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
			      if(ch->max_move<1)
				{
				ch->max_move=1;
				break;
				}
			      if(ch->max_move<ch->move)
				ch->move=ch->max_move;
			      break;
    case APPLY_GOLD:                                            break;
    case APPLY_EXP:                                               break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; 
                              if( equipment_affecting && !IS_NPC( ch ))
                                ch->pcdata->eqhitroll += mod;
                              break;
    case APPLY_DAMROLL:       if (IS_NPC(ch))
			       ch->npcdata->damplus      += mod; 
		   	      else
			       ch->damroll               += mod; 
                              if( equipment_affecting && !IS_NPC( ch ))
                                ch->pcdata->eqdamroll += mod;
                              break;
    case APPLY_SAVING_PARA:   
    case APPLY_SAVING_ROD:    
    case APPLY_SAVING_PETRI:  
    case APPLY_SAVING_BREATH: 
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod;
                              if( equipment_affecting && !IS_NPC( ch ))
                                ch->pcdata->eqsaves += mod;
                              break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).

       This code was crashing, so let them wield heavy weapons, till drop.

  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > str_app[get_curr_str(ch)].wield )
    {
	  static int depth;

	  if ( depth == 0 )
	    {
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
      wield->sac_timer=OBJ_SAC_TIME;
	    obj_to_room( wield, ch->in_room );
	    depth--;
	    }
    } */

  return;
  }



/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  AFFECT_DATA *paf_new;

  CREATE(paf_new, AFFECT_DATA, 1);
  LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );

  paf_new->type       = paf->type;
  paf_new->duration   = paf->duration;
  paf_new->location   = paf->location;
  paf_new->modifier   = paf->modifier;
  paf_new->bitvector  = paf->bitvector;

  affect_modify( ch, paf_new, TRUE );
  return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  AFFECT_DATA *tpaf;
  bool found;

  for( found=FALSE, tpaf=ch->first_affect; tpaf!=NULL; tpaf=tpaf->next)
    if( tpaf==paf )
      found=TRUE;
  if ( !found )
    {
        bug( "UNLINK ERROR Affect_remove(%s, %d): no affect.", ch->name,
                paf ? paf->type : 0 );
    return;
    }

  affect_modify( ch, paf, FALSE );
  UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
  DISPOSE( paf );
  return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->first_affect; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

             /*  Kill of charmed mobiles */
        if( paf->type == gsn_charm_person && IS_NPC( ch ) &&
            !IS_SET( ch->act, ACT_PET ) )
          SET_BIT( ch->act, ACT_WILL_DIE );

	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is first_affect by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
  AFFECT_DATA *paf;

  if( ch==NULL )
    return( FALSE );

  for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
      if ( paf->type == sn )
          return TRUE;

  return FALSE;
}

bool is_affected_external( CHAR_DATA *ch, int sn )
{
  AFFECT_DATA *paf;

  sn = slot_lookup( sn );
  for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
      if ( paf->type == sn )
          return TRUE;

  return FALSE;
}


/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->first_affect; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    CHAR_DATA *tch;
    bool found;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if( ch == ch->in_room->sanctify_char )
      {
      CHAR_DATA *fch;
      REMOVE_BIT( ch->in_room->room_flags, ROOM_SAFE );
      ch->in_room->sanctify_timer = 0;
      ch->in_room->sanctify_char = NULL;
      for( fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room )
        send_to_char( "The area does not look safe now.\n\r", fch );
      }

    if( aggr_ch_next == ch )
      aggr_ch_next = ch->next_in_room;
    if( violence_rch_next == ch )
      violence_rch_next = ch->next_in_room;

  if(IS_AFFECTED(ch,AFF2_CAMPING))
    {
    send_to_char( "You break camp.\n\r", ch );
    act( "$n breaks camp.", ch, NULL, NULL, TO_ROOM );
    REMOVE_BIT(ch->affected2_by,0-AFF2_CAMPING);
    }


    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    for( found=FALSE, tch=ch->in_room->first_person; tch!=NULL;
        tch=tch->next_in_room )
      if( tch==ch )
        found=TRUE;
    /* if( found )
      bug( "Not found in room on removal, %s", ch->name ); */
    if( found )
     {
#ifdef UNLINKCHECK
     CHAR_DATA *tch;
     bool foundc;

      for( foundc=FALSE,tch=ch->in_room->first_person; tch!=NULL; 
        tch=tch->next_in_room )
        if( tch==ch )
          {
          foundc=TRUE;
          break;
          }
    if( foundc )
     UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person,
                next_in_room, prev_in_room );
    /*else
      bug( "UNLINK ERROR character %s not found in room.", ch->name ); */
#else
     UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person,
                next_in_room, prev_in_room );
#endif
     }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->prev_in_room = NULL;
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	bug( "Char_to_room: NULL.", 0 );
	pRoomIndex=room_index[ ROOM_VNUM_TEMPLE ];
    }

    ch->in_room         = pRoomIndex;

    LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person,
              next_in_room, prev_in_room );

    if ( !IS_NPC(ch) )
	++ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;

    if( !IS_NPC( ch ) && ch->pcdata->bet_mode == 2 &&
	ch->in_room->area->low_r_vnum != ROOM_VNUM_ARENA )
	do_resign( ch, NULL );


  if( !IS_NPC( ch ) && ch->desc != NULL && ch->desc->character == ch )
    if( ch->pcdata->tactical != NULL )
     if( IS_SET( ch->act, PLR_PROMPT))
      vt100prompt( ch );

  if( !IS_NPC( ch ) && ch->desc!=NULL &&
      ch->desc->character==ch &&
      !IS_SET( ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES ) &&
      IS_SET( ch->act, PLR_TERMINAL ) )
    {
    /* Send notice of file */
    char buf[200];

    if( ch->in_room->room_file != NULL )
      sprintf( buf, "%c%c%s%c", 30, 'E', ch->in_room->room_file, 29);
    else
      sprintf( buf, "%c%c", 30, 'S');
    write_to_port( ch->desc );
    write_to_buffer( ch->desc, buf, 1000000 );
    }
    
    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
  OBJ_DATA *otmp=NULL;

    if( obj->in_room != NULL )
      obj_from_room( obj );
    if( obj->in_obj != NULL )
      obj_from_obj( obj );
    if( obj->carried_by != NULL )
      obj_from_char( obj );

    if( !IS_NPC( ch ) )
      {
      obj->reset = NULL;
 	LINK( obj, ch->first_carrying, ch->last_carrying,
                next_content, prev_content );
      }
    else
      {
      if( ch->first_carrying == NULL || obj->level < ch->first_carrying->level )
        {
 	    LINK( obj, ch->first_carrying, ch->last_carrying,
                next_content, prev_content );
            obj->carried_by                 = ch;
            obj->in_room                    = NULL;
            obj->in_obj                     = NULL;

        }
      else /* If ch is a shopkeeper, add the obj using an insert sort */
 	 {
            for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content)
            {
                if ( obj->level > otmp->level )
                {
                    INSERT(obj, otmp, ch->first_carrying,
                        next_content, prev_content);
                    break;
                }
                else
                if ( obj->level == otmp->level
                &&   strcmp(obj->short_descr,otmp->short_descr) < 0 )
                {
                    INSERT(obj, otmp, ch->first_carrying,
                        next_content, prev_content);
                    break;
                }
            }
 	    if ( !otmp)
            {
                LINK(obj, ch->first_carrying, ch->last_carrying,
                    next_content, prev_content);
            }
	}
            obj->carried_by = ch;
            obj->in_room = NULL;
            obj->in_obj = NULL;
        }

    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
    
    obj_set_to_char( ch, obj );
}

void  obj_set_to_char( CHAR_DATA *ch, OBJ_DATA *obj )
  {
  OBJ_DATA *c_obj;
  obj->carried_by = ch;

  for( c_obj = obj->first_content; c_obj != NULL; c_obj=c_obj->next_content )
    obj_set_to_char( ch, c_obj);

 if( ch!= NULL && !IS_NPC( ch ) && obj->in_obj==NULL)
  if( obj->pIndexData->obj_prog != NULL || obj->pIndexData->obj_fun!=NULL)
    {
    OBJ_WITH_PROG *with=NULL, *pwith=NULL;
        /* Let's attempt to add this at the end, to possibly fix a mem prob */
        /*   Chaos   10/24/95   */
    for( with=ch->obj_with_prog; with != NULL; with = with->next )
	pwith=with;

    CREATE(with, OBJ_WITH_PROG, 1);
    with->obj = obj;
    with->next = NULL;
    if( pwith == NULL )
     ch->obj_with_prog = with;
    else
     pwith->next = with;
    }

  /* Auto engrave stuff */
 if( ch != NULL )
  if( IS_SET(obj->pIndexData->extra_flags, ITEM_AUTO_ENGRAVE) &&
      obj->owned_by<1 && !IS_NPC(ch) && ch->level < MAX_LEVEL)
        {
        obj->owned_by = ch->pcdata->pvnum;
        obj->basic = FALSE;
        }

  return;
  }                                      

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    char buf[200];

  if( obj == NULL )
    return;

    if( obj->in_obj != NULL )
      {
      obj_from_obj( obj );
      return;
      }

    if( obj->in_room != NULL )
      {
      obj_from_room( obj );
      return;
      }

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );
#ifdef UNLINKCHECK
    {
    bool foundo;
    OBJ_DATA *tobj;

    for(tobj=ch->first_carrying, foundo=FALSE; tobj!=NULL;
            tobj=tobj->next_content )
      if( tobj==obj )
        {
        foundo=TRUE;
        break;
        }
    if( foundo )
      UNLINK( obj, ch->first_carrying, ch->last_carrying,
              next_content, prev_content );
    else
        bug( "UNLINK ERROR object %s not carried by %s", obj->name,
              ch->name );
    }
#else
    UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
#endif

    obj_set_to_char( NULL, obj);
    obj->next_content    = NULL;
    ch->carry_number    -= get_obj_number( obj );
    ch->carry_weight    -= get_obj_weight( obj );


       /* Let's make the item update when removed  */
    if( obj->first_content != NULL )
      {
      OBJ_DATA *c_obj;

      for( c_obj = obj->first_content; c_obj != NULL; c_obj=c_obj->next_content )
        if( c_obj->reset != NULL )
          {
          c_obj->reset->obj = NULL;
          c_obj->reset = NULL;
          }
      }

    if( obj->reset != NULL && obj->reset->obj == obj )
      {
      obj->reset->obj = NULL;
      obj->reset = NULL;
      }  

 if( ch!=NULL && !IS_NPC( ch ))
  if( obj->pIndexData->obj_prog != NULL || obj->pIndexData->obj_fun!=NULL)
    {
    OBJ_WITH_PROG *with, *prev;
    bool found_it;
    prev = NULL;
    found_it = FALSE;
    if(ch->obj_with_prog!=NULL && ch->obj_with_prog->obj == obj )
         {
         with = ch->obj_with_prog;
         ch->obj_with_prog = with->next;
	 DISPOSE( with );
         found_it = TRUE;
         }
    else
    for( with = ch->obj_with_prog; with!=NULL; prev=with, with = with->next)
       if( with->obj == obj)
	 {
	 prev->next = with->next;
	 DISPOSE( with );
         found_it = TRUE;
	 break;
	 }
    if( !found_it )
      {
      sprintf( buf, "Found obj(%s) with prog not referenced on char(%s)",
          obj->name, ch->name );
      log_string( buf ); 
      }
    }
    obj->in_room         = NULL;
    obj->carried_by    = NULL;
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR )
	    return 0;

    switch ( iWear )
    {
    case WEAR_BODY:
       if(IS_NPC(obj->carried_by) )
	      return 3 * obj->value[0];
	 if (obj->carried_by->pcdata->learned[gsn_armor_usage]<=0)
            {
	      return 3 * obj->value[0];
            }
        
       return(int)((((3.0*obj->carried_by->pcdata->learned[gsn_armor_usage])/
	       100)+3)*obj->value[0]);

    case WEAR_HEAD:     
       if( IS_NPC( obj->carried_by ) )
         return 2 * obj->value[0];
       if( obj->carried_by->class == 0 || obj->carried_by->class == 2 )
         return 3 * obj->value[0] / 2;
       if( obj->carried_by->class == 6 )
         return obj->value[0];
       return 2 * obj->value[0];

    case WEAR_LEGS:
       if( IS_NPC( obj->carried_by ) )
         return 2 * obj->value[0];
       if( obj->carried_by->class == 5 )
         return obj->value[0];
       return 2 * obj->value[0];

    case WEAR_FEET:     return     obj->value[0];
    case WEAR_HANDS:    return     obj->value[0];
       if( IS_NPC( obj->carried_by ) )
         return obj->value[0];
       if( obj->carried_by->class == 4 )
         return obj->value[0] / 2;
       return obj->value[0];

    case WEAR_ARMS:     return     obj->value[0];
       if( IS_NPC( obj->carried_by ) )
         return obj->value[0];
       if( obj->carried_by->class == 3 )
         return 3 * obj->value[0] / 2;
       if( obj->carried_by->class == 5 )
         return 4 * obj->value[0] / 5;
       if( obj->carried_by->class == 2 )
         return obj->value[0] / 2;
       if( obj->carried_by->class == 4 )
         return 0;
       return obj->value[0];

    case WEAR_SHIELD: 
       if( IS_NPC( obj->carried_by ) )
         return obj->value[0];
       if( obj->carried_by->class == 3 )
         return 3 * obj->value[0] / 2;
       if( obj->carried_by->class == 2 || obj->carried_by->class == 5 )
         return obj->value[0] / 2;
       if( obj->carried_by->class == 0 || obj->carried_by->class == 6 ||
           obj->carried_by->class == 4)
         return 0;
       return obj->value[0];

    case WEAR_FINGER_L: return     obj->value[0];
    case WEAR_FINGER_R: return     obj->value[0];
    case WEAR_NECK_1:   return     obj->value[0];
    case WEAR_NECK_2:   return     obj->value[0];
    case WEAR_ABOUT:    return 2 * obj->value[0];
    case WEAR_WAIST:    return     obj->value[0];
    case WEAR_WRIST_L:  return     obj->value[0];
    case WEAR_WRIST_R:  return     obj->value[0];
    case WEAR_HOLD:     return     obj->value[0];
    case WEAR_HEART:     return     obj->value[0];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }
    return NULL;
}

  /* Determine is an object has an extra affect.    -1 is any apply type */
int count_obj_affected( OBJ_DATA *obj, int iAffect, int iLocation )
  {
  AFFECT_DATA *paf;
  int cnt;

  cnt = 0;

  for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
    if( paf->type == iAffect && ( iLocation==-1 || paf->location==iLocation ))
      cnt++;

  return( cnt );
  }


/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
  {
  AFFECT_DATA *paf;
  OBJ_PROG *prg;

  if ( get_eq_char( ch, iWear ) != NULL )
    {
	  bug( "Equip_char: already equipped (vnum %u).", 
        IS_NPC(ch) ? ch->pIndexData->vnum : 0  );
	  return;
    }

  if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
	  act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
	  act( "$n is zapped by $p.",  ch, obj, NULL, TO_ROOM );
	  return;
    }

  ch->armor     -= apply_ac( obj, iWear );
  obj->wear_loc  = iWear;

  equipment_affecting = TRUE;
  for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
	  affect_modify( ch, paf, TRUE );
  for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
	  affect_modify( ch, paf, TRUE );
  equipment_affecting = FALSE;

  if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	  ++ch->in_room->light;


      if( ch->obj_prog_ip==0 && obj->pIndexData->obj_prog!=NULL)
        for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
          if( prg->cmd == -7 && number_percent() < prg->percentage )
            {
            open_timer( TIMER_OBJ_PROG );
            start_object_program( ch, obj, prg, "");
            close_timer( TIMER_OBJ_PROG );
            tail_chain();
            }
  return;
  }



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
  {
  AFFECT_DATA *paf;
  OBJ_PROG *prg;

  if ( obj->wear_loc == WEAR_NONE )
    {
	  bug( "Unequip_char: already unequipped.", 0 );
	  return;
    }

  ch->armor             += apply_ac( obj, obj->wear_loc );
  obj->wear_loc  = -1;

  equipment_affecting = TRUE;
  for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
	  affect_modify( ch, paf, FALSE );
  for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
	  affect_modify( ch, paf, FALSE );
  equipment_affecting = FALSE;

  if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	  --ch->in_room->light;

      if( ch->obj_prog_ip==0 && obj->pIndexData->obj_prog!=NULL)
        for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
          if( prg->cmd == -8 && number_percent() < prg->percentage )
            {
            open_timer( TIMER_OBJ_PROG );
            start_object_program( ch, obj, prg, "");
            close_timer( TIMER_OBJ_PROG );
            tail_chain();
            }
  return;
  }



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    if(list==NULL)
      nMatch = pObjIndex->total_objects;
     /*
      for ( obj = first_object; obj != NULL; obj = obj->next )
	{
	      if ( obj->pIndexData == pObjIndex )
		nMatch++;
	}
      */
    else
      for ( obj = list; obj != NULL; obj = obj->next_content )
	{
	      if ( obj->pIndexData == pObjIndex )
		nMatch++;
	if ( obj->item_type == ITEM_CONTAINER &&
	     obj->first_content  != NULL )
	  nMatch+=count_obj_list(pObjIndex,obj->first_content);
	}
      

    return nMatch;
}

int count_mob_list( MOB_INDEX_DATA *pMobIndex, OBJ_DATA *list )
{
    CHAR_DATA *mob;
    int nMatch;

    nMatch = 0;
  if(list==NULL)
    for ( mob = first_char; mob != NULL; mob = mob->next )
	    if ( IS_NPC(mob) && mob->pIndexData == pMobIndex )
	      nMatch++;
    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
   
    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL room.", 0);
	return;
    }
#ifdef UNLINKCHECK
    {
    bool foundo;
    OBJ_DATA *tobj;

    for(tobj=in_room->first_content, foundo=FALSE; tobj!=NULL;
            tobj=tobj->next_content )
      if( tobj==obj )
        {
        foundo=TRUE;
        break;
        }
    if( foundo )
      UNLINK( obj, in_room->first_content, in_room->last_content,
        next_content, prev_content );
    else
      bug( "UNLINK ERROR object %s not found in room.", obj->name );
    }
#else
    UNLINK( obj, in_room->first_content, in_room->last_content,
        next_content, prev_content );
#endif
       /* Let's make the item update when removed  */
    if( obj->first_content != NULL )
      {
      OBJ_DATA *c_obj;

      for( c_obj = obj->first_content; c_obj != NULL; c_obj=c_obj->next_content )
        if( c_obj->reset != NULL )
          {
          c_obj->reset->obj = NULL;
          c_obj->reset = NULL;
          }
      }
    if( obj->reset != NULL && obj->reset->obj == obj )
      {
      obj->reset->obj = NULL;
      obj->reset = NULL;
      } 

    in_room->content_count--;
    obj->in_room      = NULL;
    obj->in_obj       = NULL;
    obj->carried_by   = NULL;
    obj->next_content   = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    if( obj->in_room != NULL )
      obj_from_room( obj );
    if( obj->in_obj != NULL )
      obj_from_obj( obj );
    if( obj->carried_by != NULL )
      obj_from_char( obj );

    if( pRoomIndex==NULL || pRoomIndex->content_count >= MAX_OBJECTS_IN_ROOM )
      {
      extract_obj( obj );
      return;
      }

    LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content,
               next_content, prev_content );
    obj->in_room                                = pRoomIndex;
    obj->carried_by                             = NULL;
    obj->in_obj                                 = NULL;

    pRoomIndex->content_count++;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    CHAR_DATA *who;

    /* Think there's a null object reference crashing the game.  -Presto */
    if(obj == NULL || obj_to == NULL)
    {
      bug("Null object reference in function OBJ_TO_OBJ.");
      return;
    }

    if( obj->in_room != NULL )
      obj_from_room( obj );
    if( obj->in_obj != NULL )
      obj_from_obj( obj );
    if( obj->carried_by != NULL )
      obj_from_char( obj );

    if ( (who=obj_to->carried_by) != NULL )
        who->carry_weight += get_obj_weight(obj);

    LINK( obj, obj_to->first_content, obj_to->last_content,
        next_content, prev_content );

    obj->in_obj                          = obj_to;
    obj->in_room                         = NULL;
    obj->carried_by                      = NULL;
    return;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;
    char buf[MAX_STRING_LENGTH];

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	sprintf( buf, "Obj_from_obj: null obj_from  item: %s", obj->name);
	bug( buf, 0 );
	return;
    }
#ifdef UNLINKCHECK
    {
    bool foundo;
    OBJ_DATA *tobj;

    for(tobj=obj_from->first_content, foundo=FALSE; tobj!=NULL;
            tobj=tobj->next_content )
      if( tobj==obj )
        {
        foundo=TRUE;
        break;
        }
    if( foundo )
      UNLINK( obj, obj_from->first_content, obj_from->last_content,
        next_content, prev_content );
    else
      bug( "UNLINK ERROR object %s not found in object %s.",
            obj->name, obj_from->name );
    }
#else
      UNLINK( obj, obj_from->first_content, obj_from->last_content,
        next_content, prev_content );
#endif
    obj->in_obj       = NULL;
    obj->in_room      = NULL;
    obj->carried_by   = NULL;

    for ( ; obj_from; obj_from = obj_from->in_obj )
     if ( obj_from->carried_by )
      obj_from->carried_by->carry_weight -= get_obj_weight( obj );

       /* Let's make the item update when removed  */
    if( obj->first_content != NULL )
      {
      OBJ_DATA *c_obj;

      for( c_obj = obj->first_content; c_obj != NULL; c_obj=c_obj->next_content )
        if( c_obj->reset != NULL )
          {
          c_obj->reset->obj = NULL;
          c_obj->reset = NULL;
          c_obj->carried_by = NULL;
          }
      }

    if( obj->reset != NULL && obj->reset->obj == obj )
      {
      obj->reset->obj = NULL;
      obj->reset = NULL;
      } 

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    MOB_PACKET *packet;
    int cnt;

    if( obj==NULL )
      return;

    for ( cnt=0; cnt<2 && obj->in_room != NULL ; cnt++)
	obj_from_room( obj );

    for ( cnt=0; cnt<2 && obj->in_obj != NULL ; cnt++)
	  obj_from_obj( obj );

    for ( cnt=0; cnt<2 && obj->carried_by != NULL ; cnt++)
	  obj_from_char( obj );

    for ( obj_content = obj->first_content; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }
    
    remove_from_object_reference_hash( obj );
#ifdef UNLINKCHECK
    {
    bool foundo;
    OBJ_DATA *tobj;

    for(tobj=first_object, foundo=FALSE; tobj!=NULL;
            tobj=tobj->next )
      if( tobj==obj )
        {
        foundo=TRUE;
        break;
        }
    if( foundo )
      UNLINK( obj, first_object, last_object, next, prev );
    else
      bug( "UNLINK ERROR object %s not in first_object list.", obj->name);
    }
#else
    UNLINK( obj, first_object, last_object, next, prev );
#endif

  for( packet = mob_packet_list; packet != NULL; packet = packet->next )
    {
    if( packet->obj == obj )
      packet->obj = NULL;
    if( packet->vo == obj )
      packet->vo = NULL;
    }

    /* remove affects */
    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for ( paf = obj->first_affect; paf; paf = paf_next )
        {
            paf_next    = paf->next;
            DISPOSE( paf );
        }
        obj->first_affect = obj->last_affect = NULL;
    }
    /* remove extra descriptions */
    {
        EXTRA_DESCR_DATA *ed;
        EXTRA_DESCR_DATA *ed_next;

        for ( ed = obj->first_extradesc; ed; ed = ed_next )
        {
            ed_next = ed->next;
            STRFREE( ed->description );
            STRFREE( ed->keyword     );
            DISPOSE( ed );
        }
        obj->first_extradesc = obj->last_extradesc = NULL;
    }

    if( obj->reset != NULL )
      {
      obj->reset->obj = NULL;
      obj->reset = NULL;
      }  

    if( obj->poison != NULL )
      {
      DISPOSE( obj->poison );
      obj->poison = NULL ;
      }
    STRFREE( obj->name        );
    STRFREE( obj->description );
    STRFREE( obj->short_descr );
    STRFREE( obj->long_descr );
    if( obj->obj_quest != NULL )
      DISPOSE( obj->obj_quest );
    if( obj->pIndexData != NULL )
      {
      --obj->pIndexData->count;
      --obj->pIndexData->total_objects;  /* Total for resets */
      }
    total_objects--;
    DISPOSE( obj);
    return;
}
extern int top_exit;

void extract_exit ( ROOM_INDEX_DATA *room, EXIT_DATA * pexit, int door )
{
    if (pexit->keyword != NULL )
      STRFREE (pexit->keyword );
    STRFREE (pexit->description );
    DISPOSE( pexit );
    room_index[ room->vnum ]->exit[door]=NULL;
    top_exit--;
}

/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
  {
  CHAR_DATA *wch, *old_ch;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  MOB_PACKET *packet;
  bool ARENA_DEATH;

  if(ch==NULL)
    {
    bug("extract_char: NULL ch!",0);
    return;
    }

  if( !IS_NPC( ch ))
    if( ch->pcdata->bet_mode == 2)
      do_resign( ch, NULL );

  if ( fPull )
    {
    die_follower( ch );
    }
  /*if (!IS_NPC(ch) && ch->pcdata->beingsaved!=NULL)
  {
   pthread_join(ch->pcdata->beingsaved, NULL);
   ch->pcdata->beingsaved = NULL;
  } */

  die_shadow(ch);
  if(ch->shadowed_by!=NULL)
    die_shadow(ch->shadowed_by);

  stop_fighting( ch, FALSE );

    if( aggr_ch_next == ch )
      aggr_ch_next = ch->next_in_room;
    if( aggr_wch_next == ch )
      aggr_wch_next = ch->next;
    if( violence_ch_next == ch )
      violence_ch_next = ch->next;
    if( violence_rch_next == ch )
      violence_rch_next = ch->next_in_room;

  for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
    {
    obj_next = obj->next_content;
    if( fPull || !IS_SET(obj->extra_flags, ITEM_INVENTORY))
      extract_obj( obj );
    }
    
  ARENA_DEATH = FALSE;
  if ( ch->in_room != NULL )
    {
    if( ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA )
      ARENA_DEATH = TRUE;
    char_from_room( ch );
    }
  ch->was_in_room = NULL;

  if( (IS_NPC(ch) && ch->npcdata->poison!=NULL) ||
      (!IS_NPC(ch) && ch->pcdata->poison!=NULL) )
    {
    POISON_DATA *npd, *pd;
    if( !IS_NPC(ch) )
      {
      pd = ch->pcdata->poison;
      ch->pcdata->poison=NULL;
      }
    else
      {
      pd = ch->npcdata->poison;
      ch->npcdata->poison=NULL;
      }

    while( pd!=NULL )
      {
      npd = pd->next;
      DISPOSE( pd );
      pd = npd;
      }
    }


  if ( !fPull )
    {
    if( ch->level <2)
      char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL) );
    else
      {
         /* This is a source coded area specific command.
            It makes the arena deaths go to the frog. - Chaos 11/22/95  */
      if( !ARENA_DEATH )
        char_to_room( ch, get_room_index( ch->pcdata->death_room) );
      else
        char_to_room( ch, get_room_index( 9755 ) );
      }
    ch->recall=ch->in_room->vnum;
    return;
    }

  if ( ch->desc != NULL && ch->desc->original != NULL )
    {
    old_ch = ch;
    /* stuff to do if this is a Homonculous switch */
    if(is_name("homonculous",ch->name))
      {
      ch->desc->original->hit-=50;
      if(ch->desc->original->hit<0)
	ch->desc->original->hit=0;
      send_to_char("You suffer from the death of your imp!\n\r",ch);
      };
    /* do regular switch stuff */
    do_return( ch, NULL );
    ch = old_ch;  /* remember to reset ch here */
    ch->desc = NULL;
    }


  for( packet = mob_packet_list; packet != NULL; packet=packet->next )
    {
    if( packet->actor == ch )
      packet->actor = NULL;
    if( packet->mob == ch )
      packet->mob = NULL;
    if( packet->rndm == ch )
      packet->rndm = NULL;
    if( packet->vo == ch )
      packet->vo = NULL;
    }

  if ( IS_NPC(ch) )
    {
    --ch->pIndexData->count;
    total_mobiles--;

    if( ch->reset != NULL && ch->reset->mob == ch )
      ch->reset->mob = NULL;  
    }
  else
    sub_player( ch );

  for ( wch = first_char; wch != NULL; wch = wch->next )
    {
    if ( wch->reply == ch )
    wch->reply = NULL;
    }
  if ( !IS_NPC(ch) && is_desc_valid(ch) )
    for ( wch = first_char; wch != NULL; wch = wch->next )
      {
      if( !IS_NPC(wch) && is_desc_valid(wch) && wch->desc->snoop_by==ch->desc )
        wch->desc->snoop_by= NULL;
      }

  if( !IS_NPC( ch) && ch->pcdata->corpse != NULL)
    {
    obj_from_room( ch->pcdata->corpse);
    extract_obj( ch->pcdata->corpse );
    }

  for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
    {
    obj_next = obj->next_content;
    extract_obj( obj );
    }
#ifdef UNLINKCHECK
     {
     CHAR_DATA *tch;
     bool foundc;

      for( foundc=FALSE,tch=first_char; tch!=NULL; tch=tch->next )
        if( tch==ch )
          {
          foundc=TRUE;
          break;
          }
    if( foundc )
      UNLINK( ch, first_char, last_char, next, prev );
    /*else
      bug( "UNLINK ERROR character %s not found in char_list.", ch->name );*/
    }
#else
    UNLINK( ch, first_char, last_char, next, prev );
#endif

  if ( ch->desc != NULL )
    ch->desc->character = NULL;
  free_char( ch );
  return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    if( argument == NULL || ch->in_room == NULL )
      return NULL;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !strcasecmp( arg, "self" ) )
	    return ch;
    for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
      {
	    if(!can_see(ch,rch) && !IS_NPC(ch))
              continue;
            if (IS_AFFECTED( rch, AFF_ETHEREAL ) )
              continue;
            if(!is_name(arg,rch->name))
	      continue;
	    if ( ++count == number )
	      return rch;
      }
    count=0;
    for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
      {
	    if((!can_see(ch,rch))||!is_name_short(arg,rch->name))
	      continue;
            if (IS_AFFECTED( rch, AFF_ETHEREAL ) )
              continue;
	    if ( ++count == number )
	      return rch;
      }

    return NULL;
}


/*
 * Find a char in the room, even if hidden.
 */
CHAR_DATA *get_char_room_even_hidden( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    if( argument == NULL )
      return NULL;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !strcasecmp( arg, "self" ) )
	    return ch;
    for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
      {
	    if(!can_see(ch,rch) && !IS_NPC(ch)&&!can_see_is_hidden)
              continue;
            if (IS_AFFECTED( rch, AFF_ETHEREAL ) )
              continue;
            if(!is_name(arg,rch->name))
	      continue;
	    if ( ++count == number )
	      return rch;
      }
    count=0;
    for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
      {
	    if((!can_see(ch,rch)&&!can_see_is_hidden)||!is_name_short(arg,rch->name))
	      continue;
            if (IS_AFFECTED( rch, AFF_ETHEREAL ) )
              continue;
	    if ( ++count == number )
	      return rch;
      }

    return NULL;
}

/*
 * Find a char (by vnum) in the room, even if hidden.
 */
CHAR_DATA *get_char_room_even_hidden_vnum( CHAR_DATA *ch, int vnum)
  {
  CHAR_DATA *rch;

  for ( rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room )
    {
	  if((!can_see(ch,rch)&&!can_see_is_hidden)||rch->pIndexData->vnum!=vnum)
	    continue;
            if (IS_AFFECTED( rch, AFF_ETHEREAL ) )
              continue;
	  return rch;
    }

  return NULL;
  }


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;
    if ( ( wch = get_player_world( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = first_char; wch != NULL ; wch = wch->next )
     {
     if(!can_see(ch,wch)||!is_name(arg,wch->name))
	    continue;
            if (IS_AFFECTED( wch, AFF_ETHEREAL ) )
              continue;
	if ( ++count == number )
	    return wch;
     }
    count=0;
    for ( wch = first_char; wch != NULL ; wch = wch->next )
     {
     if(!can_see(ch,wch)||!is_name_short(arg,wch->name))
	    continue;
            if (IS_AFFECTED( wch, AFF_ETHEREAL ) )
              continue;
	if ( ++count == number )
	    return wch;
     }

    return NULL;
}

/*
 * CHAR_EXISTS
 *
 * Determine if a PC with the specified name exists.  This function will first
 * search the active player list, and then the player directories.
 *
 * Presto 2-20-99
 */
int char_exists(char *arg)
{
  char         pfile[MAX_STRING_LENGTH];
  char         pfilegz[MAX_STRING_LENGTH];
  PLAYER_GAME *fpl;
  FILE        *charfile;

  if(*arg=='\0' || *arg==' ')
    return FALSE;

  for(fpl=first_player;fpl!=NULL;fpl=fpl->next)
  {
    if(is_name(arg,fpl->ch->name))
      return TRUE;
  }

#if !defined(macintosh) && !defined(MSDOS)
  sprintf(pfile,  "%s/%c/%s",   PLAYER_DIR,tolower(arg[0]),capitalize(arg));
  sprintf(pfilegz,"%s/%c/%s.gz",PLAYER_DIR,tolower(arg[0]),capitalize(arg));
#else
  sprintf(pfile,   "%s%s",    PLAYER_DIR, capitalize(arg));
  sprintf(pfilegz, "%s%s.gz", PLAYER_DIR, capitalize(arg));
#endif

  if(!(charfile=fopen(pfile, "r")))
    if(!(charfile=fopen(pfilegz, "r")))
      return FALSE;

  fclose(charfile);
  return TRUE;
}



/*
 * Find a player in the world that you can see.
 */
CHAR_DATA *get_player_world( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  PLAYER_GAME *fpl;
  CHAR_DATA *wch;

  if((wch = get_char_room(ch,argument)) != NULL && !IS_NPC(wch) )
    return wch;
  argument = one_argument( argument, arg );

  for(fpl=first_player;fpl!=NULL;fpl=fpl->next)
    {
    if(!can_see(ch,fpl->ch)||!is_name(arg,fpl->ch->name))
      continue;
    return fpl->ch;
    }
  for(fpl=first_player;fpl!=NULL;fpl=fpl->next)
    {
    if(!can_see(ch,fpl->ch)||!is_name_short(arg,fpl->ch->name))
      continue;
    return fpl->ch;
    }

  return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex , ROOM_INDEX_DATA *room)
{
    OBJ_DATA *obj;

    for ( obj = room->first_content; obj != NULL; obj = obj->next_content )
	if ( obj->pIndexData == pObjIndex)
	    return obj;

    return NULL;
}


/*
 * Find an obj in a list by it's vnum.
 */
OBJ_DATA *get_obj_list_vnum( CHAR_DATA *ch, int vnum, OBJ_DATA *list )
  {
  OBJ_DATA *obj;

  for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	  if ( can_see_obj( ch, obj ) && obj->pIndexData->vnum==vnum )
		  return obj;
    }

  return NULL;
  }

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    /*  Adding index references  -  Chaos  4/18/99 */
    if( *argument=='#'  &&
        ( *(argument+1)>='0' && *(argument+1)<='9' ) &&
        ( *(argument+2)>='0' && *(argument+2)<='9' ) &&
        ( *(argument+3)>='0' && *(argument+3)<='9' ) )
      {
      number = (*(argument+1)-'0')*100 + (*(argument+2)-'0')*10 + 
               (*(argument+3)-'0');
      for( obj = list; obj != NULL; obj = obj->next_content )
	if( can_see_obj( ch, obj ) )
          if( !obj->basic || obj->item_type==ITEM_ARMOR ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_WEAPON )
	   if( CAN_WEAR(obj, ITEM_TAKE ) )
    if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	    if( number==obj->index_reference[0]%1000 )
	      return obj;

      return( NULL );
      }


      number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }
    count=0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name_short( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    /*  Adding index references  -  Chaos  4/18/99 */
    if( *argument=='#'  &&
        ( *(argument+1)>='0' && *(argument+1)<='9' ) &&
        ( *(argument+2)>='0' && *(argument+2)<='9' ) &&
        ( *(argument+3)>='0' && *(argument+3)<='9' ) )
      {
      number = (*(argument+1)-'0')*100 + (*(argument+2)-'0')*10 + 
               (*(argument+3)-'0');
      for( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
	if( can_see_obj( ch, obj ) && obj->wear_loc == WEAR_NONE)
          if( !obj->basic || obj->item_type==ITEM_ARMOR ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_WEAPON )
	   if( CAN_WEAR(obj, ITEM_TAKE ) )
    if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	    if( number==obj->index_reference[0]%1000 )
	      return obj;

      return( NULL );
      }

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }
    count=0;
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name_short( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
  }

/*
 * Find an obj in keeper's inventory.
 */
OBJ_DATA *get_obj_carry_keeper( CHAR_DATA *keeper, char *argument, CHAR_DATA *ch, int bargain )
  {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj,*oobj;
  int number;
  int count,cost;

    /*  Adding index references  -  Chaos  4/18/99 */
    if( *argument=='#'  &&
        ( *(argument+1)>='0' && *(argument+1)<='9' ) &&
        ( *(argument+2)>='0' && *(argument+2)<='9' ) &&
        ( *(argument+3)>='0' && *(argument+3)<='9' ) )
      {
      number = (*(argument+1)-'0')*100 + (*(argument+2)-'0')*10 + 
               (*(argument+3)-'0');
      for( obj = keeper->first_carrying; obj != NULL; obj = obj->next_content )
        {
	cost = (get_cost( keeper, obj, TRUE )*bargain)/100;
	if( cost>0 && can_see_obj( ch, obj ) )
	 if ( obj->wear_loc == WEAR_NONE && (cost<=ch->gold)
	       && (obj->level<=ch->level) )
          if( !obj->basic || obj->item_type==ITEM_ARMOR ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_WEAPON )
	   if( CAN_WEAR(obj, ITEM_TAKE ) )
    if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	    if( number==obj->index_reference[0]%1000 )
	      return obj;
        }
      return( NULL );
      }

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = keeper->first_carrying; obj != NULL; obj = obj->next_content )
    {
	  cost = (get_cost( keeper, obj, TRUE )*bargain)/100;
	  if ( cost <= 0 || !can_see_obj( ch, obj ) )
      continue;

	  if ( obj->wear_loc == WEAR_NONE
	&& (cost<=ch->gold)
	&& (obj->level<=ch->level)
	      && is_name_short( arg, obj->name ) )
	    {
      if(number==0)
	return obj;
      for( oobj = keeper->first_carrying; oobj!=obj; oobj = oobj->next_content )
	{
	      cost = (get_cost( keeper, oobj, TRUE )*bargain)/100;
	      if ( cost <= 0 || !can_see_obj( ch, oobj ) )
	  continue;
	if( oobj->wear_loc == WEAR_NONE && (is_name_short(arg, oobj->name)))
	  {
	  if(oobj->pIndexData->vnum == obj->pIndexData->vnum &&
	     obj->level == oobj->level)
	    {
	    oobj=NULL;
	    break;
	    }
	  }
	}
	    if ((oobj!=NULL)&& ++count == number )
		    return obj;
	    }
    }

  return NULL;
  }


/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    /*  Adding index references  -  Chaos  4/18/99 */
    if( *argument=='#'  &&
        ( *(argument+1)>='0' && *(argument+1)<='9' ) &&
        ( *(argument+2)>='0' && *(argument+2)<='9' ) &&
        ( *(argument+3)>='0' && *(argument+3)<='9' ) )
      {
      number = (*(argument+1)-'0')*100 + (*(argument+2)-'0')*10 + 
               (*(argument+3)-'0');
      for( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
	if( can_see_obj( ch, obj ) && obj->wear_loc != WEAR_NONE )
          if( !obj->basic || obj->item_type==ITEM_ARMOR ||
                obj->item_type==ITEM_CONTAINER || obj->item_type==ITEM_WEAPON )
	   if( CAN_WEAR(obj, ITEM_TAKE ) )
    if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
	    if( number==obj->index_reference[0]%1000 )
	      return obj;

      return( NULL );
      }

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }
    count=0;
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name_short( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj vnum in player's equipment.
 */
OBJ_DATA *get_obj_wear_vnum( CHAR_DATA *ch, int vnum)
  {
  OBJ_DATA *obj;

  for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	  if ( obj->wear_loc != WEAR_NONE
	      &&   can_see_obj( ch, obj )
	      &&   obj->pIndexData->vnum==vnum )
		  return obj;
    }

  return NULL;
  }

/*
 * Find an obj vnum in a players inventory.
 */
OBJ_DATA *get_obj_carry_vnum( CHAR_DATA *ch, int vnum)
  {
  OBJ_DATA *obj;

  for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	  if (obj->pIndexData->vnum==vnum)
		  return obj;
    }

  return NULL;
  }


/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->first_content );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }
    count=0;
    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name_short( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
	sprintf( buf, obj->short_descr, amount );
	STRFREE( obj->short_descr );
	obj->short_descr        = STRALLOC( buf );
	obj->value[0]           = amount;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
  {
/* objects inside a container no longer count as numbers -Dug 12/16/93 
  int number;

    if ( obj->item_type == ITEM_CONTAINER )
	number = 0;
    else
	number = 1;

    for ( obj = obj->first_content; obj != NULL; obj = obj->next_content )
	number += get_obj_number( obj );

    return number;
 */
  return 1;
  }



/*
 * Return weight of an object, including weight of first_content.
 * But this linked list is extremely slow.
 */
int get_obj_weight( OBJ_DATA *obj )
{


  return( obj->weight + obj->content_weight );

/*
    int weight;

    weight = obj->weight;
    for ( obj = obj->first_content; obj != NULL; obj = obj->next_content )
	weight += get_obj_weight( obj );

    return weight;  */
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex == NULL)
       return TRUE;
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY 
    ||   pRoomIndex->sector_type == SECT_INN )
	return FALSE;

       /* All times are same, so pick any area here */
    if ( room_index[1]->area->weather_info.sunlight == SUN_SET
    ||   room_index[1]->area->weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    count = 0;
    for ( rch = pRoomIndex->first_person; rch != NULL; rch = rch->next_in_room )
     {  
       if (!IS_NPC(rch))
         count++;
     }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;

    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
  {
  can_see_is_hidden=FALSE;

  if ( ch == victim )
	  return TRUE;

  if( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_ETHEREAL ))
    return FALSE;

  if( IS_NPC( ch ) && ch->pIndexData->pShop != NULL)
    return( TRUE );
    
  if ( (!IS_NPC(ch)) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	  return TRUE;

  if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
	  return FALSE;

  if ( IS_AFFECTED(ch, AFF_BLIND) )
	  return FALSE;

  if ( !IS_NPC(ch) && is_affected(ch, gsn_truesight))
          return TRUE;
 
  if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
     {
     if( race_table[ ch->race ].vision == 0)
	  return FALSE;
     if( race_table[ ch->race ].vision == 1 && 
	 (ch->in_room->sector_type == SECT_INSIDE ||
	  IS_SET( ch->in_room->room_flags, ROOM_INDOORS)))
	  return FALSE;
     }

  if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	  return FALSE;

  if ( IS_AFFECTED(victim, AFF_HIDE)
    &&  !IS_AFFECTED(ch, AFF_DETECT_HIDDEN) 
    &&  (victim->fighting==NULL || victim->position!=POS_FIGHTING) )
    {
    can_see_is_hidden=TRUE;
	  return FALSE;  
    }

  if(IS_AFFECTED(victim,AFF_STEALTH) &&
     (!IS_AFFECTED(ch,AFF_DETECT_HIDDEN)) &&
     (victim->fighting==NULL || victim->position!=POS_FIGHTING) )
    {
    can_see_is_hidden=TRUE;
	  return FALSE;  
    }

  return TRUE;
  }


/*
 * True if char can hear victim.
 */
bool can_hear( CHAR_DATA *ch, CHAR_DATA *victim )
  {
  if ( ch == victim )
	  return TRUE;

  if( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_ETHEREAL ))
    return FALSE;

  if( IS_NPC( ch ) && ch->pIndexData->pShop != NULL)
    return( TRUE );
    
  if ( (!IS_NPC(ch)) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	  return TRUE;

  if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
	  return FALSE;

  if ( IS_AFFECTED(victim, AFF_HIDE)
    &&  (!IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
    &&  (victim->fighting==NULL || victim->position!=POS_FIGHTING) )
    {
	  return FALSE;  
    }

  if(IS_AFFECTED(victim,AFF_STEALTH) &&
     (!IS_AFFECTED(ch,AFF_DETECT_HIDDEN)) &&
     (victim->fighting==NULL || victim->position!=POS_FIGHTING) )
    {
	  return FALSE;  
    }

  return TRUE;
  }



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    /*if ( obj->item_type == ITEM_POTION )
	return TRUE;*/

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    /* INFRARED should not be able to see items, but night vision should */
    if ( !IS_NPC(ch) && !can_see_in_room( ch, ch->in_room ))
	return FALSE;

    if ( !IS_NPC(ch) && is_affected(ch, gsn_truesight))
          return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;
    if( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
    case ITEM_LIGHT:            return "light";
    case ITEM_SCROLL:           return "scroll";
    case ITEM_WAND:             return "wand";
    case ITEM_STAFF:            return "staff";
    case ITEM_WEAPON:           return "weapon";
    case ITEM_TREASURE:         return "treasure";
    case ITEM_ARMOR:            return "armor";
    case ITEM_POTION:           return "potion";
    case ITEM_FURNITURE:        return "furniture";
    case ITEM_TRASH:            return "trash";
    case ITEM_CONTAINER:        return "container";
    case ITEM_DRINK_CON:        return "drink container";
    case ITEM_KEY:              return "key";
    case ITEM_FOOD:             return "food";
    case ITEM_MONEY:            return "money";
    case ITEM_BOAT:             return "boat";
    case ITEM_CORPSE_NPC:       return "npc corpse";
    case ITEM_CORPSE_PC:        return "pc corpse";
    case ITEM_FOUNTAIN:         return "fountain";
    case ITEM_PILL:             return "pill";
    case ITEM_AMMO:             return "ammunition";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:            return "none";
    case APPLY_STR:             return "strength";
    case APPLY_DEX:             return "dexterity";
    case APPLY_INT:             return "intelligence";
    case APPLY_WIS:             return "wisdom";
    case APPLY_CON:             return "constitution";
    case APPLY_SEX:             return "sex";
    case APPLY_CLASS:           return "class";
    case APPLY_LEVEL:           return "level";
    case APPLY_AGE:             return "age";
    case APPLY_MANA:            return "mana";
    case APPLY_HIT:             return "hp";
    case APPLY_MOVE:            return "moves";
    case APPLY_GOLD:            return "gold";
    case APPLY_EXP:             return "experience";
    case APPLY_AC:              return "armor class";
    case APPLY_HITROLL:         return "hit roll";
    case APPLY_DAMROLL:         return "damage roll";
    case APPLY_SAVING_PARA:     return "save vs paralysis";
    case APPLY_SAVING_ROD:      return "save vs rod";
    case APPLY_SAVING_PETRI:    return "save vs petrification";
    case APPLY_SAVING_BREATH:   return "save vs breath";
    case APPLY_SAVING_SPELL:    return "save vs spell";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
  if( vector > 0 )
    {
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_HOLD          ) strcat( buf, " hold"          );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " protect_evil"  );
    if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " protect_good"  );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_STEALTH       ) strcat( buf, " stealth"       );
    if ( vector & AFF_HASTE         ) strcat( buf, " haste"       );

    }
  else
    {
    vector = 0-vector;
    if ( vector & (0-AFF2_MAGE_SHIELD)     ) strcat( buf, " mage_shield"     );
    if ( vector & (0-AFF2_ENHANCED_REST)   ) strcat( buf, " enhanced rest"   );
    if ( vector & (0-AFF2_ENHANCED_HEAL)  ) strcat( buf, " enhanced heal"   );
    if ( vector & (0-AFF2_ENHANCED_REVIVE) ) strcat( buf, " enhanced revive" );
    if ( vector & (0-AFF2_CAMPING)         ) strcat( buf, " camping"         );
    if ( vector & (0-AFF2_BERSERK)         ) strcat( buf, " berserk"         );
    }

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOT_VALID    ) strcat( buf, " not_valid"    );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*  Chaos 10/10/93
    Determines which class has highest level in a skill
    Return -1 is none.    */
int multi( CHAR_DATA *ch, int sn)
  {
  int cnt, mlv, tst;
  
  mlv=-1;
  tst=-1;
  if(IS_NPC(ch))
    return(-1);
  for(cnt=0;cnt<MAX_CLASS;cnt++)
    if( ch->mclass[cnt] >= skill_table[sn].skill_level[cnt])
      if(mlv<ch->mclass[cnt])
	{
	tst=cnt;
	mlv=ch->mclass[cnt];
	}
  return(tst);
  }

/* This is the variant of multi that selects the highest adept level.
       Chaos - 9/14/98 */
int multi_pick( CHAR_DATA *ch, int sn)
  {
  int cnt, mlv, tst;
  
  mlv=-1;
  tst=-1;
  if(IS_NPC(ch))
    return(-1);
  for(cnt=0;cnt<MAX_CLASS;cnt++)
    if( ch->mclass[cnt] >= skill_table[sn].skill_level[cnt])
      if(mlv<class_table[cnt].skill_adept)
	{
	tst=cnt;
	mlv=class_table[cnt].skill_adept;
	}
  return(tst);
  }

int get_pets( CHAR_DATA *ch)
{
  int cnt;
  CHAR_DATA *fch;

  cnt=0;
  for(fch=first_char ; fch!=NULL ; fch=fch->next )
     if( fch->master == ch)
       if( IS_AFFECTED( fch, AFF_CHARM) )
	 cnt++;
  return( cnt );
}

void char_reset(CHAR_DATA *ch)
  {/* reset ch's stats */
  OBJ_DATA *obj;
	AFFECT_DATA *paf;
  int old_max_hit, old_max_mana, old_max_move;

  ch->armor=100;
  ch->hitroll=0;
  ch->damroll=0;
  ch->saving_throw=0;
  ch->pcdata->eqhitroll=0;
  ch->pcdata->eqdamroll=0;
  ch->pcdata->eqsaves=0;
  ch->pcdata->mod_str=0;
  ch->pcdata->mod_dex=0;
  ch->pcdata->mod_wis=0;
  ch->pcdata->mod_con=0;
  ch->pcdata->mod_int=0;
  old_max_hit = ch->max_hit;
  old_max_mana = ch->max_mana;
  old_max_move = ch->max_move;
  ch->max_hit = ch->actual_max_hit;
  ch->max_mana = ch->actual_max_mana;
  ch->max_move = ch->actual_max_move;

  for(obj=ch->first_carrying;obj!=NULL;obj=obj->next_content)

    {
    if(obj->wear_loc!=WEAR_NONE)
      {
      ch->armor-=apply_ac(obj,obj->wear_loc);
      for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
	{
	switch(paf->location)
	  {
          case APPLY_MANA: ch->max_mana += paf->modifier; break;
          case APPLY_HIT:  ch->max_hit += paf->modifier; break;
          case APPLY_MOVE:     ch->max_move += paf->modifier; break;
	  case APPLY_AC:       ch->armor+= paf->modifier;       break;
	  case APPLY_HITROLL:  ch->hitroll+= paf->modifier;     
                               ch->pcdata->eqhitroll += paf->modifier;  break;
	  case APPLY_DAMROLL:  ch->damroll+= paf->modifier;
                               ch->pcdata->eqdamroll += paf->modifier;  break;
	  case APPLY_STR:      ch->pcdata->mod_str+= paf->modifier;     break;
	  case APPLY_DEX:      ch->pcdata->mod_dex+= paf->modifier;     break;
	  case APPLY_WIS:      ch->pcdata->mod_wis+= paf->modifier;     break;
	  case APPLY_INT:      ch->pcdata->mod_int+= paf->modifier;     break;
	  case APPLY_CON:      ch->pcdata->mod_con+= paf->modifier;     break;
          case APPLY_SAVING_PARA:   
          case APPLY_SAVING_ROD:    
          case APPLY_SAVING_PETRI: 
          case APPLY_SAVING_BREATH:
          case APPLY_SAVING_SPELL: ch->saving_throw+= paf->modifier;
                                   ch->pcdata->eqsaves+= paf->modifier;  break;
	  default:
	    break;
	  }
	}
      for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
	{
	switch(paf->location)
	  {
          case APPLY_MANA: ch->max_mana += paf->modifier; break;
          case APPLY_HIT:  ch->max_hit += paf->modifier; break;
          case APPLY_MOVE:     ch->max_move += paf->modifier; break;
	  case APPLY_AC:       ch->armor+= paf->modifier;       break;
	  case APPLY_HITROLL:  ch->hitroll+= paf->modifier;   
                               ch->pcdata->eqhitroll += paf->modifier;  break;
	  case APPLY_DAMROLL:  ch->damroll+= paf->modifier;
                               ch->pcdata->eqdamroll += paf->modifier;  break;
	  case APPLY_STR:      ch->pcdata->mod_str+= paf->modifier;     break;
	  case APPLY_DEX:      ch->pcdata->mod_dex+= paf->modifier;     break;
	  case APPLY_WIS:      ch->pcdata->mod_wis+= paf->modifier;     break;
	  case APPLY_INT:      ch->pcdata->mod_int+= paf->modifier;     break;
	  case APPLY_CON:      ch->pcdata->mod_con+= paf->modifier;     break;
          case APPLY_SAVING_PARA:   
          case APPLY_SAVING_ROD:    
          case APPLY_SAVING_PETRI: 
          case APPLY_SAVING_BREATH:
          case APPLY_SAVING_SPELL: ch->saving_throw+= paf->modifier;
                                   ch->pcdata->eqsaves+= paf->modifier;  break;
	  default:
	    break;
	  }
	}
      }
    }
  for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
    {
    switch(paf->location)
      {
          case APPLY_MANA: ch->max_mana += paf->modifier; break;
          case APPLY_HIT:  ch->max_hit += paf->modifier; break;
          case APPLY_MOVE:     ch->max_move += paf->modifier; break;
	  case APPLY_AC:       ch->armor+= paf->modifier;       break;
	  case APPLY_HITROLL:  ch->hitroll+= paf->modifier;     break;
	  case APPLY_DAMROLL:  ch->damroll+= paf->modifier;     break;
	  case APPLY_STR:      ch->pcdata->mod_str+= paf->modifier;     break;
	  case APPLY_DEX:      ch->pcdata->mod_dex+= paf->modifier;     break;
	  case APPLY_WIS:      ch->pcdata->mod_wis+= paf->modifier;     break;
	  case APPLY_INT:      ch->pcdata->mod_int+= paf->modifier;     break;
	  case APPLY_CON:      ch->pcdata->mod_con+= paf->modifier;     break;
          case APPLY_SAVING_PARA:   ch->saving_throw+= paf->modifier; break;
          case APPLY_SAVING_ROD:    ch->saving_throw+= paf->modifier; break;
          case APPLY_SAVING_PETRI:  ch->saving_throw+= paf->modifier; break;
          case APPLY_SAVING_BREATH: ch->saving_throw+= paf->modifier; break;
          case APPLY_SAVING_SPELL:  ch->saving_throw+= paf->modifier; break;
      default:
	break;
      }
    }

    /* Fix limits    -  Chaos  1/31/97 */
			      if(ch->max_mana<1)
				ch->max_mana=1;
			      if(ch->max_mana<ch->mana)
				ch->mana=ch->max_mana;
			      if(ch->max_hit<1)
				ch->max_hit=1;
			      if(ch->max_hit<ch->hit)
				ch->hit=ch->max_hit;
			      if(ch->max_move<1)
				ch->max_move=1;
			      if(ch->max_move<ch->move)
				ch->move=ch->max_move;

  if( ch->actual_max_hit == 0 )  /* Reconstruct Actuals  - Chaos 12/22/94 */
    {
    ch->actual_max_hit = old_max_hit - ch->max_hit;
    ch->max_hit += ch->actual_max_hit;
    ch->actual_max_mana = old_max_mana - ch->max_mana;
    ch->max_mana += ch->actual_max_mana;
    ch->actual_max_move = old_max_move - ch->max_move;
    ch->max_move += ch->actual_max_move;
    }
  return;
  }

int mob_armor(CHAR_DATA *ch)
  {/* get mob's armor w/o basic AC */
  OBJ_DATA *obj;
	AFFECT_DATA *paf;
  int AC;

  AC=0;
  for(obj=ch->first_carrying;obj!=NULL;obj=obj->next_content)
    {
	  if(obj->wear_loc!=WEAR_NONE)
      {
      AC+=apply_ac(obj,obj->wear_loc);
      for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
	{
	switch(paf->location)
	  {
	  case APPLY_AC:            AC                          += paf->modifier;       break;
	  default:
	    break;
	  }
	}
      for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
	{
	switch(paf->location)
	  {
	  case APPLY_AC:            AC                          += paf->modifier;       break;
	  default:
	    break;
	  }
	}
      }
    }
  for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
    {
    switch(paf->location)
      {
      case APPLY_AC:            AC                      += paf->modifier;       break;
      default:
	break;
      }
    }
  if( ch->level <5)
    return(0);
  if( AC> ch->level)
    AC=ch->level;
  return( AC );
  }

CHAR_DATA * scan_mob( RESET_DATA *pReset)
  {
  CHAR_DATA *mob;
  
  if( pReset==NULL)
    return(NULL);
  for(mob=first_char; mob!=NULL; mob=mob->next)
    if(IS_NPC(mob) &&  mob->reset==pReset)
       return(mob);
  return(NULL);
  }

bool scan_obj( RESET_DATA *pReset)
  {
  OBJ_DATA *obj;
  
  for(obj=first_object; obj!=NULL; obj=obj->next)
    if(obj->reset==pReset)
       return(TRUE);
  return(FALSE);
  }

char *pick_one( char *in1, char *in2, char *in3, char *in4)
  {
  int pick;
  pick=number_range( 1, 4);
  if(pick==1)
    return( in1);
  if(pick==2)
    return( in2);
  if(pick==3)
    return( in3);
  return( in4);
  }

bool can_see_in_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room)
  {

  if(IS_NPC( ch))
    return(!room_is_dark( room ));

  if(!room_is_dark( room ))
    return( TRUE );
  if( race_table[ch->race].vision==0)
    return( FALSE );
  if( race_table[ch->race].vision==1 &&
     (room->sector_type == SECT_INSIDE ||
      IS_SET( room->room_flags, ROOM_INDOORS)))
	  return(FALSE);
  return( TRUE );   /* for vision==2 */
  }


sh_int obj_level_estimate(OBJ_INDEX_DATA *objIndex)
  {
  AFFECT_DATA *aff;
  int level;
  int value[4];

  level=1;
  for(aff=objIndex->first_affect;aff!=NULL;aff=aff->next)
    {
      switch(aff->location)
	{
	case APPLY_STR:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(3*aff->modifier))/2;
	  else
	    level-=(aff->modifier*(aff->modifier));
	  break;
	case APPLY_DEX:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(2*aff->modifier))/3;
	  else
	    level-=(aff->modifier*(aff->modifier))/3;
	  break;
	case APPLY_INT:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(2*aff->modifier))/3;
	  else
	    level-=(aff->modifier*(aff->modifier))/4;
	  break;
	case APPLY_WIS:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(4*aff->modifier))/3;
	  else
	    level-=(aff->modifier*(2*aff->modifier))/3;
	  break;
	case APPLY_CON:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(4*aff->modifier))/3;
	  else
	    level-=(aff->modifier*(2*aff->modifier))/3;
	  break;
	case APPLY_MANA:
	  if(aff->modifier > 0)
	    level+=aff->modifier/3;
	  else
	    level+=aff->modifier/5;
	  break;
	case APPLY_HIT:
	  if(aff->modifier > 0)
	    level+=aff->modifier/2;
	  else
	    level+=aff->modifier/4;
	  break;
	case APPLY_MOVE:
	  if(aff->modifier > 0)
	    level+=aff->modifier/3;
	  else
	    level+=aff->modifier/5;
	  break;
	case APPLY_AC:
	  if(aff->modifier < 0)
	    level+=(aff->modifier*(aff->modifier))/3;
	  else
	    level-=(aff->modifier*(aff->modifier))/6;
	  break;
	case APPLY_HITROLL:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(aff->modifier))*2/3;
	  else
	    level-=(aff->modifier*(aff->modifier))/6;
	  break;
	case APPLY_DAMROLL:
	  if(aff->modifier > 0)
	    level+=(aff->modifier*(aff->modifier));
	  else
	    level-=(aff->modifier*(aff->modifier))/3;
	  break;
	case APPLY_SAVING_BREATH:
	  if(aff->modifier < 0)
	    level+=(aff->modifier*(aff->modifier))/7;
	  else
	    level-=(aff->modifier*(aff->modifier))/9;
	  break;
	case APPLY_SAVING_SPELL:
	  if(aff->modifier < 0)
	    level+=(aff->modifier*(aff->modifier))/6;
	  else
	    level-=(aff->modifier*(aff->modifier))/8;
	  break;
	case APPLY_NONE:
	case APPLY_SEX:
	default:
	  break;
	}
    }

  value[0]=objIndex->value[0];
  value[1]=objIndex->value[1];
  value[2]=objIndex->value[2];
  value[3]=objIndex->value[3];
  switch(objIndex->item_type)
    {
    case ITEM_LIGHT:
      if(value[2] < 0)
	{
          if(level < 25)
	    level = 25;
        }
      else
	if( level+value[2]/2400 > 3*level/2)
	  level = level+value[2]/2400;
	else
      	  level=3*level/2;
      break;
    case ITEM_SCROLL:
      if(value[1]>0)
	level+=3*value[0]/5;
      if(value[2]>0)
	level+=3*value[0]/5;
      if(value[3]>0)
	level+=3*value[0]/5;
      break;
    case ITEM_POTION:
      if(value[1]>0)
	level+=3*value[0]/5;
      if(value[2]>0)
	level+=3*value[0]/5;
      if(value[3]>0)
	level+=3*value[0]/5;
      break;
    case ITEM_PILL:
      if(value[1]>0)
	level+=2*value[0]/3;
      if(value[2]>0)
	level+=2*value[0]/3;
      if(value[3]>0)
	level+=2*value[0]/3;
      break;
    case ITEM_WAND:
      level+=2*value[0]/3 + 2*value[0]*(value[1])/24;
      break;
    case ITEM_STAFF:
      level+=3*value[0]/5 + 3*value[0]*(value[1])/30;
      break;
    case ITEM_WEAPON:
      level+=((5*(value[1]*(1+(value[2]-1)/2.0))/2)-10);
      break;
    case ITEM_ARMOR:
      if( value[0] < 0 )
	level-=value[0]*(value[0])/4+1;
      else
	level+=value[0]*(value[0])/4+1;
      break;
    case ITEM_AMMO:
      level+=value[1]*value[3]*value[2]/20;
      break;
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
      level+=value[0]/15;
      break;
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_MONEY:
    case ITEM_BOAT:
    case ITEM_FOUNTAIN:
    default:
      break;
    }

  if((objIndex->extra_flags & ITEM_INVIS) != 0)
    level+=(level/20);
  if((objIndex->extra_flags & ITEM_NODROP) != 0)
    level-=(level/15);
  if((objIndex->extra_flags & ITEM_ANTI_GOOD) != 0)
    level-=(level/20);
  if((objIndex->extra_flags & ITEM_ANTI_EVIL) != 0)
    level-=(level/20);
  if((objIndex->extra_flags & ITEM_ANTI_NEUTRAL) != 0)
    level-=(level/20);
  if((objIndex->extra_flags & ITEM_NOREMOVE) != 0)
    level-=(level/15);
  if((objIndex->extra_flags & ITEM_INVENTORY) != 0)
    level+=(level/10);
  if((objIndex->extra_flags & ITEM_AUTO_ENGRAVE) != 0)
    level-=(level/15);

  if(level>0)
    return level;
  else
    return 1;
  }


sh_int obj_cnt[32768];
void invalidate_obj_copies(OBJ_DATA *list,int level)
  {
  OBJ_DATA *obj;
  int i;

  if(level==0)
    for(i=0;i<32768;i++)
      obj_cnt[i]=0;

  for(obj=list;obj!=NULL;obj=obj->next_content)
    {
    if(obj_cnt[obj->pIndexData->vnum]>=obj->pIndexData->max_objs)
      {
      if(obj->wear_loc==WEAR_NONE)
	{
	 if (obj->carried_by != NULL)
	   log_printf("invalidate_obj_copies deleting %s carried by %s.", obj->name, obj->carried_by->name);
	 else
	   log_printf("invalidate_obj_copies deleting %s.", obj->name );
         SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
	}
      }
    else
      obj_cnt[obj->pIndexData->vnum]++;
    if (obj->item_type == ITEM_CONTAINER)
      if(obj->first_content!=NULL)
        invalidate_obj_copies(obj->first_content,level+1);
    }
  return;
  }

bool blocking(CHAR_DATA *victim,CHAR_DATA *ch)
  {
  /* can't block NPC's */
  if(IS_NPC(ch) || IS_NPC(victim))
    return FALSE;

  /* can't block gods or police */
  if(ch->level >= LEVEL_IMMORTAL || which_god(ch)==GOD_POLICE)
    return FALSE;

  /* You can't talk to someone who is disconnected */
  /* nor can someone who is disconnected talk to you */
  if(ch->desc==NULL || victim->desc==NULL)
    return TRUE;
 
  /* victim isn't blocking anyone */
  if(victim->pcdata->block_list==NULL)
    return FALSE;

  /* victim is blocking ch by name */
  if(is_name(ch->name,victim->pcdata->block_list))
    return TRUE;

  /* ch has no host entry, so you can't block his host */
  if(ch->desc->host==NULL)
    return FALSE;

  /* named sites need to be shortened in reverse of numbered sites */
  if(isalpha((int)ch->desc->host[0]))
    return is_short_of_name_reverse(ch->desc->host,victim->pcdata->block_list);
  else
    return is_short_of_name(ch->desc->host,victim->pcdata->block_list);
  }

bool is_in_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room)
  {
  CHAR_DATA *fch;

  for( fch = room->first_person; fch != NULL; fch = fch->next_in_room )
    if( ch == fch )
      return( TRUE );
  return( FALSE );
  }

bool in_camp(CHAR_DATA *ch)
  {
  CHAR_DATA *gch;
  if( ch->in_room == NULL )
    return( FALSE );
  for(gch=ch->in_room->first_person;gch!=NULL;gch=gch->next_in_room)
    if(is_same_group(ch,gch) && IS_AFFECTED(gch,AFF2_CAMPING))
      return TRUE;
  return FALSE;
  }

char domain_string[256];
char *domain_of(char *site)
  {
  char *p,*d;
  int pCnt;

  pCnt=0;
  if(isalpha((int)*site))
    {
    for(p=site+strlen(site);p>=site;p--)
      {
      if(*p=='.')
        {
        if((++pCnt)==2)
          return (p+1);
        continue;
        }
      }
    }
  else
    {
    d=domain_string;
    for(p=site;*p!='\0';p++)
      {
      *(d++)=*p;
      if(*p=='.')
        {
        if((++pCnt)==2)
          {
          *(d-1)='\0';
          return domain_string;
          }
        continue;
        }
      }
    }
  return site;
  }

char *ansi_translate_text( CHAR_DATA *ch, char *text_in )
  {
  char *pti, *pto, *pt2;
  int mode, val1, val2, val3;

  pti = text_in;
  pto = ansi_translate_buffer;
  mode = 0;
  val1 = 0;
  val2 = 0;
  val3 = 0;

  pt2 = get_color_string( ch, COLOR_TEXT, VT102_DIM ) ;
  for( ; *pt2!='\0'; pt2++, pto++)
    *pto = *pt2;

  while( *pti != '\0' )
    {
    switch( mode )
      {
      case 0:    /* No escape sequence current */
        if( *pti == '{' )
          mode = 1;
        else
          {
          *pto = *pti;
          pto++;
          }
        pti++;
        break;

      case 1:
        if( *pti < '0' || *pti > '7' )    /* Invalid escape */
          {
          *pto = '{';
          pto ++;
          *pto = *pti;
          pto ++;
          pti ++;
          mode = 0;
          break;
          }
        val1 = *pti - '0';
        mode = 2;
        pti++;
        break;

      case 2:
        if( *pti < '0' || *pti > '7' )    /* Invalid escape */
          {
          *pto = '{';
          pto ++;
          *pto = val1 + '0';
          pto++;
          *pto = *pti;
          pto ++;
          pti ++;
          mode = 0;
          break;
          }
        val2 = *pti - '0';
        mode = 3;
        pti++;
        break;
    
      case 3:
        if( *pti < '0' || *pti > '7' )    /* Invalid escape */
          {
          *pto = '{';
          pto ++;
          *pto = val1 + '0';
          pto++;
          *pto = val2 + '0';
          pto++;
          *pto = *pti;
          pto ++;
          pti ++;
          mode = 0;
          break;
          }
        val3 = *pti - '0';
        mode = 4;
        pti++;
        break;
    
      case 4:
        if( *pti != '}' )    /* Invalid escape */
          {
          *pto = '{';
          pto ++;
          *pto = val1 + '0';
          pto++;
          *pto = val2 + '0';
          pto++;
          *pto = val3 + '0';
          pto++;
          *pto = *pti;
          pto ++;
          pti ++;
          mode = 0;
          break;
          }
        mode = 0;
        pti++;

        if( val1 == 3 )
          {
          pt2 = get_color_string( ch, COLOR_TEXT, VT102_DIM ) ;
          for( ; *pt2!='\0'; pt2++, pto++)
            *pto = *pt2;
          }
       else
        if( val1 != 2 && val1 !=6 )
          {
          pt2 = get_color_diff( ch, -1, -1, 1-val1, val2, val3, val1 ) ;
          for( ; *pt2!='\0'; pt2++, pto++)
            *pto = *pt2;
          }

        break;
      }

    }

  if( mode > 0 )
    {
    *pto = '{';
    pto++;
    }
  if( mode > 1 )
    {
    *pto = val1 + '0';
    pto++;
    }
  if( mode > 2 )
    {
    *pto = val2 + '0';
    pto++;
    }
  if( mode > 0 )
    {
    *pto = val3 + '0';
    pto++;
    }

  *pto = '\0';

  return( ansi_translate_buffer );
  }
    

char *get_name( CHAR_DATA *ch )
{
  char buf[MAX_INPUT_LENGTH];

  *get_name_buffer='\0'; 
  *buf='\0'; 

  if( !IS_NPC( ch ) )
    {

    if( ch->name == NULL )
      {
      *get_name_buffer='\0'; 
      return( get_name_buffer );
      }

    if( which_god( ch )==GOD_POLICE  && ch->level<98 )
      {
      if( ch->sex == 2 )
        strcpy( get_name_buffer, "Sheriff " );
      else
        strcpy( get_name_buffer, "Sheriff " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
      }

        /* Angi's Title - Chaos 10/17/97 */
    if( ch->pcdata->pvnum == 56092 && !strcasecmp(ch->name,"lovely"))
      {
      strcpy( get_name_buffer, "Queen " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
      }

    /* Presto's Title - Presto 6-24-98 */
    if( ch->pcdata->pvnum == 108 && !strcasecmp(ch->name,"presto"))
      {
      strcpy( get_name_buffer, "High Wizard " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
      }
     
    if(!strcasecmp(ch->name,"emperor"))
      {
      strcpy( get_name_buffer, "Elder Wizard " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
       }  
   
      if(!strcasecmp(ch->name,"grog"))
      {
      strcpy( get_name_buffer, "Holy Knight " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
      }  */

    if( ch->level > 98 )
      {
      if( ch->sex == 2 )
        strcpy( get_name_buffer, "Lady " );
      else
        strcpy( get_name_buffer, "Knight " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer);
      }
    else
    if( ch->level > 97 )
      {
      if( ch->sex == 2 )
        strcpy( get_name_buffer, "Duchess " );
      else
        strcpy( get_name_buffer, "Duke " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer);
      }
    else
    if( ch->level > 96 )
      {
      if( ch->sex == 2 )
        strcpy( get_name_buffer, "Countess " );
      else
        strcpy( get_name_buffer, "Count " );
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer);
      }
    else
    if( ch->level > 95 )
      {
      if( ch->sex == 2 )
        strcpy( get_name_buffer, "Lady " );
      else
        strcpy( get_name_buffer, "Knight " );
      }
/*    Most players didn't like this idea  - Chaos removed 8/17/98
      Subtitles for followers - Chaste 6/20/98
    else if (which_god(ch)==GOD_CHAOS ||
             which_god(ch)==GOD_ORDER ||
             which_god(ch)==GOD_DEMISE )
    {
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      if (which_god(ch)==GOD_ORDER)
       strcat(buf, " the Lawful");
      if (which_god(ch)==GOD_CHAOS)
       strcat(buf, " the Chaotic");
      if (which_god(ch)==GOD_DEMISE)
       strcat(buf, " the Deceitful");
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
    }  */
    else if( ch->level > 90 )
     {
     if( ch->mclass[ch->class]==ch->level )
       switch( ch->class )
         {
         case CLASS_ROGUE: 
               if( ch->sex == 2 )
		strcpy( get_name_buffer, "Temptress " );
               else
                strcpy( get_name_buffer, "Cutthroat " ); break;
         case CLASS_ILLUSIONIST: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Sorceress " );
               else
		strcpy( get_name_buffer, "Sorcerer " ); break;
         case CLASS_ELEMENTALIST: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Druidess " );
               else
		strcpy( get_name_buffer, "Druid " ); break;
         case CLASS_RANGER: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Swordmistress " );
               else
		strcpy( get_name_buffer, "Swordmaster " ); break;
         case CLASS_NECROMANCER: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Witch " );
               else
		strcpy( get_name_buffer, "Warlock " ); break;
         case CLASS_MONK: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Sister " );
               else
		strcpy( get_name_buffer, "Brother " ); break;
         case CLASS_ASSASSIN: 
               if( ch->sex == 2 )
                strcpy( get_name_buffer, "Blackwidow " );
               else
		strcpy( get_name_buffer, "Widowmaker " ); break;
         }
     else
       if( ch->sex != SEX_FEMALE )
        strcpy( get_name_buffer, "Sir " );
       else
        strcpy( get_name_buffer, "Maiden " );

     }
    else
    if( ch->level > 89 )
     {
     if( ch->sex != SEX_FEMALE )
      strcpy( get_name_buffer, "Squire " );
     else
      strcpy( get_name_buffer, "Miss " );
     }
      strcpy( buf, ch->name );
      *buf = UPPER( *buf );
      strcat( get_name_buffer, buf);
      return( get_name_buffer );
    }

  strcpy( get_name_buffer, ch->short_descr );
  return( get_name_buffer );
}


int GET_HITROLL( CHAR_DATA *ch )
{
  int val, tmp, max;

  val =(ch->hitroll+str_app[get_curr_str(ch)].tohit);

  if( IS_NPC(ch) )
    return( val );

  tmp = ch->pcdata->eqhitroll;
  max = (ch->level*120)/95 + 15;

  if( tmp>max )
    val -= ( tmp-max );

  return( val );
}

int GET_DAMROLL( CHAR_DATA *ch )
{
  int val, tmp, max;

  val = (ch->damroll+str_app[get_curr_str(ch)].todam) ;

  if( IS_NPC(ch) )
    return( val );

  tmp = ch->pcdata->eqdamroll;
  max = (ch->level*120)/95 + 10;

  if( tmp>max )
    val -= ( tmp-max );

  return( val );
}

int GET_SAVING_THROW( CHAR_DATA *ch )
{
  int val, tmp, max;

  val = ch->saving_throw;

  if( IS_NPC(ch) )
    return( val );

  tmp = ch->pcdata->eqsaves;
  max = (ch->level*45)/95 + 10;

  if( tmp>max )
    val -= ( tmp-max );

  return( val );
}

bool can_use_exit(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir)
{
 if (ch==NULL || room == NULL || room->exit[dir] == NULL || dir<0 || dir>5) return FALSE;

 if (IS_SET(room->exit[dir]->exit_info,EX_RIP) &&
             !pvnum_in_group(ch,room->exit[dir]->pvnum))
    return FALSE;
 else if (IS_SET(room->exit[dir]->exit_info, EX_BACKDOOR) &&
             !pvnum_in_group(ch,room->exit[dir]->pvnum))
    return FALSE;
 else if (IS_SET(room->exit[dir]->exit_info, EX_CLAN_BACKDOOR) &&
	     !IS_NPC(ch) && ((ch->pcdata->clan != NULL &&
             (get_clan_from_vnum(room->exit[dir]->pvnum)!= ch->pcdata->clan) )
	     || ch->pcdata->clan == NULL ))
    return FALSE;
 else
    return TRUE;
}

 
/*                                                             
 * 1/3/99 -Martin
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( !str || str[0] == '\0' )
      return FALSE;

    for ( ; ; )
    {
        str = one_argument( str, name );
        if ( name[0] == '\0' )
            return TRUE;
        if ( !is_name( name, namelist ) )
            return FALSE;
    }
}

char *str_replace( char *in_str, char *orig_txt, char *new_txt)
  {
  char *pti, *pto, *ptt, *ptb;
  char pi, pb;
  bool foundit;

  if( in_str==NULL || orig_txt==NULL || new_txt==NULL )
    return( NULL );

  if( *in_str=='\0' )
    {
    *str_replace_txt = '\0';
    return( str_replace_txt );
    }

  if( *orig_txt=='\0' )
    {
    strcpy( str_replace_txt, in_str );
    return( str_replace_txt );
    }

  for( pti=in_str, pto=str_replace_txt; *pti!='\0'; pti++, pto++)
    {
    pi=0;
    pb=0;
    foundit=TRUE;
    for(ptt=pti,ptb=orig_txt;foundit && *ptt!='\0' && *ptb!='\0';)
      {
      pi = *ptt;
      pb = *ptb;
      if( pi>='a' && pi<='z' )
        pi -= ('a'-'A');
      if( pb>='a' && pb<='z' )
        pb -= ('a'-'A');
      if( pi!=pb )
        foundit=FALSE;
      ptb++;
      ptt++;
      if( *ptb=='\0' )
        foundit=FALSE;
      }
    if( foundit )  /* Found it */
      {
      for( ptb=new_txt; *ptb!='\0'; )
        {
        *pto = *ptb;
        ptb++;
        if( *ptb!='\0' )
          pto++;
        }
      pti = ptt-1;
      }
    else
      *pto = *pti;
    }
  *pto='\0';
  return( str_replace_txt );
  }

bool str_contains( char *in_str, char *orig_txt)
  {
  char *pti, *ptt, *ptb;
  char pi, pb;

  if( in_str==NULL || orig_txt==NULL )
    return( FALSE );

  if( *in_str=='\0' )
    {
    return( FALSE );
    }

  if( *orig_txt=='\0' )
    {
    return( FALSE );
    }

  for( pti=in_str; *pti!='\0'; pti++)
    {
    pi=0;
    pb=0;
    for(ptt=pti, ptb=orig_txt; pb==pi && *ptt!='\0' && *ptb!='\0'; ptb++, ptt++)
      {
      pi = *ptt;
      pb = *ptb;
      if( pi>='a' && pi<='z' )
        pi -= ('a'-'A');
      if( pb>='a' && pb<='z' )
        pb -= ('a'-'A');
      }
    if( *ptb=='\0' )  /* Found it */
      return( TRUE );
    }
  return( FALSE );
  }
