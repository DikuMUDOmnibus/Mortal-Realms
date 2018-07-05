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
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );


int value;    /* This is the value after the target */

void make_char_fight_char( CHAR_DATA *ch, CHAR_DATA *victim )
{
 if (ch==NULL || victim == NULL || victim->position == POS_DEAD)
     return;
    /* Start the cheating check */
      if( IS_NPC( victim ) )
        {
        CHAR_DATA *ch_ld;
        ch_ld = ch;
        if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) && ch->master!=NULL)
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
  return;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0]) &&
             !strcasecmp((char *)name,(char *)skill_table[sn].name))
	    return sn;
    }
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0]) &&
	     !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	slot = -1;
    

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( ch->class==rch->class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* int save; */
    int range, point,  bph, bpl;
    int attack_levels;
    CHAR_DATA *fch;

    point = 50 - victim->level / 5 + level * 2 / 3 ;
    if( IS_NPC( victim ) && !IS_AFFECTED( victim, AFF_CHARM ) &&
        IS_SET( victim->act, ACT_UNDEAD ) )
      range = 100 + victim->level * 3 / 4 - GET_SAVING_THROW(victim)* 2;
    else if( IS_NPC( victim ) )
      range = 100 + victim->level / 2 - GET_SAVING_THROW(victim)* 2;
    else
      range = 100 - GET_SAVING_THROW(victim)* 3;


        /* Add a bit of distraction to spell casters - Chaos 7/11/96  */
        /* Should make them think twice about being tank  */
    if( ch!=NULL && !IS_NPC(ch) && ch->in_room!=NULL )
      {
      attack_levels = 0;
      for( fch=ch->in_room->first_person; fch!=NULL; fch=fch->next_in_room )
        if( fch!=ch && fch->fighting!=NULL && who_fighting( fch ) ==ch )
          {
          attack_levels += fch->level;
          }
      range += ( attack_levels / 5 );
      range -= ( ch->level / 5 );
      if( attack_levels > 0 )
        range += 5;
      else
        range -= 2;
      }

       /* If the guy is smart enough then give bonus   -  Chaos 7/12/96  */
    if( ch!=NULL && !IS_NPC(ch) )
      {
      bph = ( ( 6*ch->level )/95 ) + 18 ;
      bpl = ( ( 6*ch->level )/95 ) + 13 ;
      if( ch->pcdata->perm_int >= bph )
        range -= 8;
      if( ch->pcdata->perm_wis >= bph )
        range -= 8;
      if( ch->pcdata->perm_int <= bpl )
        range += 12;
      if( ch->pcdata->perm_wis <= bpl+2 )
        range += 12;
      }

    if( !IS_NPC(victim) && victim->race==RACE_DWARF )
      {
      if( victim->in_room!=NULL &&
         (victim->in_room->sector_type==SECT_INSIDE ||
          victim->in_room->sector_type==SECT_INN ) )
        range += 10;
      else
        range += 7;
      }

      if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
          range -= 20;

      if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )
          range -= 20;


    if( number_range( 1, range) >= point )
      return( TRUE );    /* Spell Fails */
    else
      return( FALSE );   /* Spell Works */

  /*  Chaos changing to universal scaling system  12/16/94 */
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[10];
    CHAR_DATA *victim=NULL;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn,cnt;
    int level, percentage;

      /*  No homonculous or pets */
    if ( IS_NPC(ch) && ( IS_AFFECTED( ch, AFF_CHARM) || 
          ch->pIndexData->vnum==9900))
	return;

    if( find_keeper( ch ) != NULL )
      {
      send_to_combat_char( "You can't cast a spell in a shop!\n\r", ch);
      return;
      }
 
  if (!IS_NPC(ch) && is_affected(ch, gsn_anti_magic_shell))
    {  
     act("No matter how hard you try, the flows of magic will not break through the shell\n\rsurrounding you.", ch, NULL, NULL, TO_CHAR);
     return; 
    }
  if( strlen( argument ) > 100 )
    *(argument+80)='\0';

  percentage = 100;

  argument = one_argument( argument, arg1 );

    /* Grab reduced spell strength percentage value     -   Chaos 7/26/96 */
  if( *arg1 != '\0' && is_number( arg1 ) )
    {
    percentage = atol( arg1 );
    argument = one_argument( argument, arg1 );
    if( percentage > 100 )
      percentage = 100;
    else
    if( percentage < 1 )
      percentage = 1;
    }

  target_name=argument;
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  *(target_name+50)='\0';

    arg4[0]='\0';

    if ( arg1[0] == '\0' )
    {
	send_to_combat_char( "Cast which what where?\n\r", ch );
	return;
    }

    value = -1;   /* Default value */
    if( is_number( arg2 ) )   /*  Special numbers only spell */
      {
      value = atol( arg2 );
      strcpy( arg2, "");
      if ( value < 1 )
         value = -1;   /* Default value */
      }
     else
      if( arg3[0]!='\0' )     /* Read the extra value  - Chaos  2/8/95  */
       {
       value = atol( arg3 );
       if ( value < 1 )
         value = -1;   /* Default value */
       }


    sn = skill_lookup( arg1 );
    if( sn < 0 || !is_spell( sn))
       {
       send_to_combat_char( "That is not a spell.\n\r", ch );
       return;
       }

    cnt = multi( ch, sn);
/* 
   Added several IS_IMMORTAL checks so that immortals can use all spells
   - Martin
*/
    if(cnt==-1 && !IS_NPC(ch) && !IS_IMMORTAL(ch))
       {
	send_to_combat_char( "You can't do that.\n\r", ch );
	return;
       }

    level = ch->mclass[cnt];

    if( !IS_NPC(ch) && skill_table[sn].skill_level[cnt] > level && !IS_IMMORTAL(ch) )
      {
      send_to_combat_char( "You are not quite high enough level to cast that spell.\n\r", ch );
      return;
      }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE) &&
       /*IS_SET(ch->in_room->room_flags, ROOM_RIP) && *What? Order 1/94*/
       ((skill_table[sn].target==TAR_IGNORE &&
         skill_table[sn].minimum_position==POS_FIGHTING) ||
        (skill_table[sn].target==TAR_CHAR_OFFENSIVE))/* && !IS_NPC(ch)*/ )
      {
      send_to_combat_char( "You cannot do that here.\n\r", ch);
      return;
      }
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_combat_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    mana = get_mana(ch,sn);


    level = level * percentage / 100 ;
    if( level < 1 )
      level = 1;

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
      /* ch->max_hit/2 now */
        if( IS_NPC(ch) && ch->fighting == NULL && ch->hit < (ch->max_hit/2) )
          if( !IS_AFFECTED( ch, AFF_CHARM ) )
            return;

	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = who_fighting(ch) ) == NULL )
	    {
		send_to_combat_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
           if( victim->name == NULL )
		return;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	    }
           if( victim->name == NULL )
		return;

	 }


  /* Check the just died counter */
  if(!IS_NPC(victim) && !IS_NPC(ch))
    if(victim->pcdata->just_died_ctr > 0 &&
       ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
    {
     send_to_char("That character is currently protected by the gods.\n\r", ch);
     return;
    }
  if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("$N shrugs as you attempt to direct the flows of magic towards $M.", ch, NULL, victim, TO_CHAR);
     return; 
    }
  if(!IS_NPC(ch))
    ch->pcdata->just_died_ctr = 0;

    /* Limitations of player vs. player  -  Chaos  3/24/99   */
  if( ch!=victim )
  if( ch->fighting==NULL || ch->fighting->who!=victim )
   if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if((IS_NPC(victim) && IS_AFFECTED(victim,AFF_CHARM)) || !IS_NPC(victim))
     if( ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
      {
      send_to_combat_char( "You can't do that.\n\r", ch);
      return;
      }

  if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if( IS_NPC( victim) && IS_AFFECTED( victim, AFF_CHARM) 
        && (victim->fighting == NULL || who_fighting( victim) !=ch))
      {
      send_to_combat_char( "You can't do that to a pet.\n\r", ch);
      return;
      }

  if( IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
        !is_same_group( ch, victim->fighting->who ))
    {
    char buf[160];
    sprintf( buf ,"%s seems to be busy!\n\r", capitalize(victim->short_descr));
    send_to_combat_char( buf, ch);
    return;
    }
  if( (!IS_NPC(ch ) || IS_AFFECTED(ch, AFF_CHARM ) )
      && IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 ) 
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
            capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_combat_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
        if(ch!=victim && (!IS_NPC(victim) || IS_AFFECTED(victim,AFF_CHARM)) &&
            victim->fighting!=NULL && 
            ( !IS_NPC(victim->fighting->who) || !is_same_group( ch, victim )) &&
            !IS_NPC( ch ) )
          {
          char buf[160];
          sprintf(buf ,"%s seems to be busy!\n\r", get_name( victim ) );
          send_to_combat_char( buf, ch);
          return;
          }
        if(ch!=victim && (IS_AFFECTED(victim, AFF_CHARM) || !IS_NPC( victim))&&
               !IS_NPC(ch) && ( 60 * level / 100 > victim->level+5 ))
          {
          char buf[160];
          sprintf(buf ,"Your spell is reduced in strength to keep from killing %s.\n\r",
                                                   get_name( victim ) );
          send_to_combat_char( buf, ch);
          level = victim->level * 100 / 60 + 5;
          }

  if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("$N shrugs as you attempt to direct the flows of magic towards $M.", ch, NULL, victim, TO_CHAR);
     return; 
    }
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name_short( arg2, ch->name ) )
	{
	    send_to_combat_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

  if (!IS_NPC(ch) && is_affected(ch, gsn_anti_magic_shell))
    {  
     act("No matter how hard you try, the flows of magic will not break through the shell\n\rsurrounding you.", ch, NULL, victim, TO_CHAR);
     return; 
    }
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_combat_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_combat_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	break;
    }
	    
    if ( !IS_IMMORTAL(ch) && !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_combat_char( "You don't have enough mana.\n\r", ch );
	return;
    }
      
    if ( strcasecmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
      
   WAIT_STATE( ch, skill_table[sn].beats * 4 / 5 );
      
   if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE &&   victim != ch )
      make_char_fight_char( ch, victim );

    if ( !IS_IMMORTAL(ch) && !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[sn]+ (get_curr_int(ch)-13)*2 )
    {
        switch( number_bits(2) )
        {
            case 0:     /* too busy */
                if ( ch->fighting!=NULL )
                  send_to_char( "This round of battle is too hectic to concentrate properly.\n\r", ch );
                else
                  send_to_char( "You lost your concentration.\n\r", ch );
                break;
            case 1:     /* irritation */
                if ( number_bits(2) == 0 )
                {
                  switch( number_bits(2) )
                  {
                     case 0: send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r", ch ); break;
                     case 1: send_to_char( "An itch on your leg keeps you from properly casting your spell.\n\r", ch ); break;
                     case 2: send_to_char( "Something in your throat prevents you from uttering the proper phrase.\n\r", ch ); break;
                     case 3: send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r", ch ); break;
                  }
                }
                else
                  send_to_char( "Something distracts you, and you lose your concentration.\n\r", ch );
                break;
            case 2:     /* not enough time */
                if ( ch->fighting!=NULL )
                  send_to_char( "There wasn't enough time this round to complete
 the casting.\n\r", ch );
                else
                  send_to_char( "You lost your concentration.\n\r", ch );
                break;
            case 3:
                send_to_char( "You get a mental block mid-way through the casting.\n\r", ch );
                break;
        }

	ch->mana -= mana / 2;
    }
    else
    {
       if(!IS_IMMORTAL(ch))
         ch->mana -= mana;
	(*skill_table[sn].spell_fun) ( sn, (IS_IMMORTAL(ch) ? 99 : IS_NPC(ch) ? ch->level *3/4 : level), ch, vo );
    }


    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}

void do_mana( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn, cnt;
    if(*argument == '\0')
       {
         send_to_combat_char("Usage: mana <spell name>.\n\r", ch);
         return;
       }
    sn = skill_lookup( argument );
    if( sn < 0 || !is_spell(sn))
       {
       send_to_combat_char( "That is not a spell.\n\r", ch );
       return;
       }

    cnt = multi( ch, sn);

    if(cnt==-1 && !IS_NPC(ch))
       {
	send_to_combat_char( "You can't do that.\n\r", ch );
	return;
       }
      sprintf( buf, "Mana usage for '%s' is: %d\n\r",
         skill_table[sn].name, get_mana( ch, sn));
      send_to_combat_char( buf, ch);
      return;
      }

int get_mana( CHAR_DATA *ch, int sn)
    {
    int mana,mnx,cnt;

    cnt = multi( ch, sn);
    if(!IS_NPC(ch))
      mnx = (skill_table[sn].skill_level[cnt]/2)+1;
    else
      mnx=0;

    mana = IS_NPC(ch) ? 0 : UMAX( skill_table[sn].min_mana,
	50*mnx / ( mnx + ch->mclass[cnt] - skill_table[sn].skill_level[cnt] )-
   (get_curr_wis(ch)-13)/2);

    return(mana);
    }

void do_move( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  int sn, cnt, move;

  sn = skill_lookup( argument );
  if( sn < 0 || is_spell(sn))
    {
    send_to_combat_char( "That doesn't cost any movement.\n\r", ch );
    return;
    }

  cnt = multi( ch, sn);

  if(cnt==-1 && !IS_NPC(ch))
    {
	  send_to_combat_char( "You can't do that.\n\r", ch );
	  return;
    }
  move = IS_NPC(ch) ? 0 : skill_table[sn].min_mana;

  sprintf(buf,"Movement usage for '%s' is: %d\n\r",skill_table[sn].name,move);
  send_to_combat_char(buf,ch);
  return;
  }

bool is_spell( int sn)
  {
  int cnt;
  for(cnt=0;cnt<MAX_CLASS && skill_table[sn].skill_level[cnt]>95;cnt++);
    if(cnt==0 || cnt==1 || cnt==4 || cnt==5)
      return TRUE;
  return FALSE;
  }

int caster_levels( CHAR_DATA *ch )
  {
  int levels;

  if( IS_NPC( ch ) )
    return( ch->level );

  levels  = ch->mclass[ CLASS_ILLUSIONIST ];
  levels += ch->mclass[ CLASS_ELEMENTALIST ];
  levels += ch->mclass[ CLASS_MONK ];
  levels += ch->mclass[ CLASS_NECROMANCER ];

  if( levels > ch->level )
    levels = ch->level;

  return( levels );
  }


/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    char buf[180];

  if( obj != NULL )
    obj->basic = FALSE;

    if ( sn <= 0 )
	return;

       /* Chaos added 10/54/94 */
    if( sn <= MAX_SKILL && skill_table[sn].slot == -1 )
        return;

    if( find_keeper( ch ) != NULL )
      {
      send_to_combat_char( "You can't cast a spell in a shop!\n\r", ch);
      return;
      }

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
      {
      if(obj!=NULL)
        sprintf( buf, "Obj_cast_spell: Vnum %u bad sn %d.", 
            obj->pIndexData->vnum, sn );
      else
        sprintf( buf, "Unknown item cast sn %d", sn);
      log_string( buf );
      return;
      }


       /* Pure magic users are unaffectd, while fighters cast at 1/2 */
       /*  Chaos - 7/12/96 */
    level = ( level * ( caster_levels( ch ) + ch->level ) ) / 2 / ch->level ;

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:

      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

	if ( victim == ch && ch->fighting != NULL )
	    victim = who_fighting( ch );
	if ( victim == NULL && ch->fighting != NULL )
	    victim = who_fighting( ch );
	if ( victim == NULL || victim->name == NULL)
	{
	    send_to_combat_char( "You can't do that.\n\r", ch );
	    return;
	}

    /* Limitations of player vs. player  -  Chaos  3/24/99   */
  if( ch!=victim )
  if( ch->fighting==NULL || ch->fighting->who!=victim )
   if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if((IS_NPC(victim) && IS_AFFECTED(victim,AFF_CHARM)) || !IS_NPC(victim))
     if( ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
      {
      send_to_combat_char( "You can't do that.\n\r", ch);
      return;
      }

  if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if( IS_NPC( victim) && IS_AFFECTED( victim, AFF_CHARM)
        && (victim->fighting == NULL || who_fighting( victim) !=ch))
      {
      send_to_combat_char( "You can't do that to a pet.\n\r", ch);
      return;
      }

  if(( IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
        !is_same_group( ch, victim->fighting->who )) ||
  ( !IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
        !is_same_group( ch, victim )))
    {
    char buf[160];
    sprintf( buf ,"%s seems to be busy!\n\r", get_name( victim ) );
    send_to_combat_char( buf, ch);
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
      sprintf( buf ,"%s was recently fought.  Try later.\n\r", 
          capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }

  if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("$N shrugs as you attempt to direct the flows of magic towards $M.", ch, NULL, victim, TO_CHAR);
     return; 
    }
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL )
	    victim = ch;
        if(ch!=victim && (!IS_NPC( victim) || IS_AFFECTED(victim,AFF_CHARM)) &&
            victim->fighting!=NULL && 
            ( !IS_NPC(victim->fighting->who) || !is_same_group( ch, victim )) &&
            !IS_NPC( ch ) )
          {
          char buf[160];
          sprintf(buf ,"%s seems to be busy!\n\r", get_name( victim ) );
          send_to_combat_char( buf, ch);
          return;
          }
        if( (IS_AFFECTED(victim, AFF_CHARM) || !IS_NPC( victim))&&
               !IS_NPC(ch) && ( 60 * level / 100 > victim->level+5 ))
          {
          char buf[160];
          sprintf(buf ,"The spell is reduced in strength to keep from killing %s.\n\r",
                       ch==victim?"You":get_name( victim ) );
          send_to_combat_char( buf, ch);
          level = victim->level * 100 / 60 + 5;
          }
  if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("$N shrugs as you attempt to direct the flows of magic towards $M.", ch, NULL, victim, TO_CHAR);
     return; 
    }
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
        if( victim == NULL )
          victim = ch;
        if( (IS_AFFECTED(victim, AFF_CHARM) || !IS_NPC( victim))&&
               !IS_NPC(ch) && ( 60 * level / 100 > victim->level+5 ))
          {
          char buf[160];
          sprintf(buf ,"The spell is reduced in strength to keep from killing %s.\n\r",
                       ch==victim?"You":get_name( victim ) );
          send_to_combat_char( buf, ch);
          level = victim->level * 100 / 60 + 5;
          }
  if (!IS_NPC(ch) && is_affected(ch, gsn_anti_magic_shell))
    {  
     act("No matter how hard you try, the flows of magic will not break through the shell\n\rsurrounding you.", ch, NULL, victim, TO_CHAR);
     return; 
    }
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_combat_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo );

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE &&   victim != ch )
      make_char_fight_char( ch, victim );

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch 
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level*3+40, level*5+100);
    if ( saves_spell( level, ch, victim ) )
	     dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    if((!IS_NPC(ch)) && ch->mclass[CLASS_ELEMENTALIST]>0)
      {
      af.duration  = (ch==victim)?48:24;
      af.modifier  = (ch==victim)?-15:-5;
      }
    else
      {
      af.duration  = 24;
      af.modifier  = -5;
      }
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your armor begins to glow softly as it is enhanced by a cantrip.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
      {
      send_to_combat_char( "They are fighting.\n\r", ch );
      return;
      }
    if( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = 6+level/2;
    af.location  = APPLY_HITROLL;
    af.modifier  = level/9;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_combat_char( "A powerful blessing is laid upon you.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    if ( is_affected(victim, gsn_truesight ))
      {
      act( "$N sees through the veil on reality you try to impose.", ch, NULL, victim, TO_CHAR );
      act( "You see through the veil on reality $n attempts to impose.", ch, NULL, victim, TO_VICT );
      return;
      }
     
    if( saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Nothing happens.\n\r", ch );
      return;
      }

    af.type      = sn;
    af.location  = APPLY_HITROLL;
    af.modifier  = -2*level/5;
    af.duration  = 1+level/5;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_combat_char( "You are blinded!\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level-5, level+5);
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_combat_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( ch->in_room->area->weather_info.sky < SKY_RAINING )
    {
	send_to_combat_char( "You need bad weather.\n\r", ch );
	return;
    }


    send_to_combat_char( "Elemental lightning strikes your foes!\n\r", ch );
    act( "$n calls elemental lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = first_char; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
        if( vch->in_room == ch->in_room )
          {
          if (vch==ch || IS_AFFECTED( vch, AFF_CHARM) )
            continue;
          if (!IS_NPC(vch) && who_fighting( ch )!=vch )
            continue;
          if( IS_AFFECTED( vch , AFF_ETHEREAL ) )
            continue;
  if( IS_NPC( vch) && vch->fighting==NULL && 
     vch->npcdata->pvnum_last_hit_leader > 0 &&
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
    if( pvnum_ld != vch->npcdata->pvnum_last_hit_leader )
      {
      continue;
      }
    }
          dam   = number_range( level*2 , level*3);
          damage( ch, vch, saves_spell( level, ch, vch ) ? dam/2 : dam, sn );
          continue;
          }

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_combat_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}




void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn );
    return;
}



void spell_cause_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
    damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn );
    return;
}



void spell_cause_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
    damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn );
    return;
}



void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      }
    if ( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level/4;
    af.location  = APPLY_SEX;
    if( victim->sex == 0 )
      {
       if( number_range(0,1) == 1 )
	af.modifier  = 1;
       else
	af.modifier  = 2;
      }
    else
    if( victim->sex == 1 )
      {
       if( number_range(0,5) < 5 )
	af.modifier  = 1;
       else
	af.modifier  = -1;
      }
    else
    if( victim->sex == 2 )
      {
       if( number_range(0,5) < 5 )
	af.modifier  = -1;
       else
	af.modifier  = -2;
      }
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_combat_char( "A chill runs through you as your gender changes.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_combat_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( ch->in_room->area->low_r_vnum==ROOM_VNUM_ARENA )
    {
	send_to_combat_char( "You can't charm in the Arena!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF2_POSSESS)
    ||   level*2/3 < victim->level
    ||   saves_spell( level, ch, victim ) 
    ||   find_keeper( ch ) !=NULL    /* Not in shops */
    ||   get_pets(ch)>=2)
      {
      if(IS_NPC(victim))
        {
        if(number_percent()<35)
          {
          do_say(victim,"I don't want to be charmed!");
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
        }
      else
        {
        act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
        act("Hey!  Wait a minute!  $n isn't so nice!  What's going on here?",
            ch,NULL,victim,TO_VICT );
        }
	    return;
      }

    if ( victim->master )
	    stop_follower( victim );
    add_follower( victim, ch );
    af.type      = sn;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	    send_to_combat_char( "Ok.\n\r", ch );
    if ( IS_NPC( victim ) )
    {
      start_hating( victim, ch );
      start_hunting( victim, ch );
    }

    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam   = number_range( level+2, level+6);
    if ( !saves_spell( level, ch, victim ) )
    {
	af.type      = sn;
	af.duration  = 6;
        if (!IS_NPC(victim))
	 af.location  = APPLY_STR;
        else
	 af.location  = APPLY_DAMROLL;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    damage( ch, victim, dam, sn );
    return;
}



void spell_color_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level*3+20, level*4+60);
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;

    damage( ch, victim, dam, sn );
    return;
}



void spell_continual_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    light->pIndexData->max_objs=2000000000;
    light->timer=48;
    light->level=UMAX(level+(ch->level-level)/5,level);
    act( "Shards of iridescent light collide to form a dazzling ball.", ch, NULL, NULL, TO_CHAR );
    act( "Shards of iridescent light collide to form a dazzling ball.", ch, NULL, NULL, TO_ROOM );
    obj_to_room( light, ch->in_room );
    return;
}



void spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
    if ( !strcasecmp( target_name, "better" ) )
       {
	ch->in_room->area->weather_info.change += (3+level/15);
	ch->in_room->area->weather_info.mmhg += (3+level/15);
       }
    else if ( !strcasecmp( target_name, "worse" ) )
       {
	ch->in_room->area->weather_info.change -= (3+level/15);
	ch->in_room->area->weather_info.mmhg -= (3+level/15);
       }
    else
	send_to_combat_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_combat_char( "Ok.\n\r", ch );
    return;
}


void spell_feast( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  CHAR_DATA *fch;
  char buf[ MAX_INPUT_LENGTH ];

  for( fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room )
     if( !IS_NPC( fch ) && fch->position >= POS_RESTING )
       {
       gain_condition( fch, COND_FULL, 50 );
       gain_condition( fch, COND_THIRST, 50 );
       if( fch != ch )
         {
         sprintf( buf, "You join in %s's feast.\n\rYou are full.\n\r", 
                 get_name( ch ) );
         send_to_combat_char( buf, fch );
         }
       else
         send_to_combat_char( "You create a large selection of food and drinks.\n\rEveryone in the room joins you as you eat your fill.\n\r", ch );
       }
  return;
  }

void spell_restore( int sn, int level, CHAR_DATA *ch, void *vo )
  {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal, scale_top, scale_bot;
    char buf[MAX_INPUT_LENGTH];

    if( victim==NULL || victim->in_room == NULL || ch->in_room==NULL ||
        victim->in_room != ch->in_room )
      {
      send_to_combat_char( "That person is not here.\n\r", ch );
      return;
      }

    scale_top = 10;  /* The fraction for the hp/mana ratio */
    scale_bot = 7;

    if( value == -1 || value > ch->mana )
      value = ch->mana;

    heal = value * scale_top / scale_bot ;
    if( victim->hit + heal > victim->max_hit )
      heal = victim->max_hit - victim->hit ;
    value = ( heal * scale_bot / scale_top );

    sprintf( buf, "Restoring %d hitpoints for %d mana.\n\r", heal, 
                      value + get_mana(ch,sn));
    send_to_combat_char( buf, ch);

    victim->hit += heal;
    ch->mana -= value;

    update_pos( victim );
    send_to_combat_char( "You are filled with an overwhelming feeling of warmth.\n\r", victim );
    affect_strip(victim,gsn_critical_hit);
    if ( ch != victim )
	send_to_combat_char( "You restore them.\n\r", ch );
    return;
    }

void spell_energy_shift( int sn, int level, CHAR_DATA *ch, void *vo )
  {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal, scale_top, scale_bot;
    char buf[MAX_INPUT_LENGTH];

    if( victim==NULL || victim->in_room == NULL || ch->in_room==NULL ||
        victim->in_room != ch->in_room )
      {
      send_to_combat_char( "That person is not here.\n\r", ch );
      return;
      }

    scale_top = 65;  /* The fraction for the mana_to/mana_from ratio */
    scale_bot = 100;

    if( value == -1 || value > ch->mana )
      value = ch->mana;

    heal = value * scale_top / scale_bot ;
    if( victim->mana + heal > victim->max_mana )
      heal = victim->max_mana - victim->mana ;
    value = ( heal * scale_bot / scale_top );

    sprintf( buf, "Shifting %d mana for %d mana.\n\r", heal,
                      value + get_mana(ch,sn));
    send_to_combat_char( buf, ch);

    victim->mana += heal;
    ch->mana -= value;

    update_pos( victim );
    send_to_combat_char( "You feel a surge of power.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "You transfer energy to them.\n\r", ch );
    return;
  }


void spell_induction( int sn, int level, CHAR_DATA *ch, void *vo )
  {
    int heal, scale_top, scale_bot;
    char buf[MAX_INPUT_LENGTH];

    scale_top = 9;  /* The fraction for the mana_to/hp_from ratio */
    scale_bot = 20;


    if( value < 0 || value > ch->hit - 10 )
      value = ch->hit-10;
    heal = ((value * scale_top) / scale_bot) - 5;
    if( ch->mana + heal > ch->max_mana )
      heal = ch->max_mana - ch->mana ;
    if (heal<0) heal=0;
    /*value = ( heal * scale_bot / scale_top );*/

    sprintf( buf, "Inducing %d mana points for %d hitpoints.\n\r", heal, 
                      value );
    send_to_combat_char( buf, ch);

    ch->mana += heal;
    ch->hit -= value;

    update_pos( ch );
    if (heal>0);
    {
     send_to_combat_char( "You feel a surge of power.\n\r", ch );
     act( "$n drains $s strength to further $s magic.", ch, NULL, NULL, TO_ROOM);
    }
    return;
  }

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 5 + level;
    mushroom->pIndexData->max_objs=2000000000;
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    obj_to_room( mushroom, ch->in_room );
    return;
}



void spell_create_spring( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    act( "Tracing a ring before you, the graceful flow of a mystical spring emerges.", ch, spring, NULL, TO_CHAR );
    act( "As $n traces a ring through the air, the graceful flow of a mystical spring emerges.", ch, spring, NULL, TO_ROOM );
    obj_to_room( spring, ch->in_room );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_combat_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_combat_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
      level * (ch->in_room->area->weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    STRFREE (obj->name );
	    obj->name = STRALLOC( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if ( !is_affected( victim, gsn_blindness ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    affect_strip( victim, gsn_blindness );
    send_to_combat_char( "Your vision returns!\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_combat_char( "Your critical wounds close and your pain ebbs away.\n\r", victim );
    affect_strip(victim,gsn_critical_hit);
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_combat_char( "Your light wounds mend and your pain ebbs slightly.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
  bool cured;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    cured = FALSE;
    if ( is_affected( victim, gsn_poison ) )
    {
	affect_strip( victim, gsn_poison );
        cured = TRUE;
    }

  if( (IS_NPC(victim) && victim->npcdata->poison!=NULL) ||
      (!IS_NPC(victim) && victim->pcdata->poison!=NULL) )
    {
    POISON_DATA *npd, *pd;
    cured = TRUE;
    if( !IS_NPC(victim) )
      {
      pd = victim->pcdata->poison;
      victim->pcdata->poison=NULL;
      }
    else
      {
      pd = victim->npcdata->poison;
      victim->npcdata->poison=NULL;
      }

    while( pd!=NULL )
      {
      npd = pd->next;
      DISPOSE( pd );
      pd = npd;
      }
    }

    if( cured )
        {
	act( "$N looks better.", ch, NULL, victim, TO_CHAR );
	act( "$N looks better.", ch, NULL, victim, TO_ROOM );
	send_to_combat_char( "A warm feeling runs through your body.\n\r", victim );
        }
    else
	act( "$N does not look any better.", ch, NULL, victim, TO_CHAR );
    return;
}



void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_combat_char( "Your serious wounds mend and your pain ebbs away.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_CURSE) )
      {
      send_to_combat_char( "They are already cursed.\n\r", ch );
      return;
      }
    if( saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Your curse did not strike them.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level/4;
    af.location  = APPLY_HITROLL;
    af.modifier  = -3;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 1;
    affect_to_char( victim, &af );

    send_to_combat_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Your foe is now cursed.\n\r", ch );
    return;
}



void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_combat_char( "Traces of red outline all evil in plain sight.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_hidden( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your senses are heightened to those of an animal.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your eyes fixate as they gain the ability to see the unseen.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_combat_char( "Traces of blue outline the magical objects in your field of vision.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_combat_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_combat_char( "It looks very delicious.\n\r", ch );
    }
    else
    {
	send_to_combat_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "God protects $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    dam = dice( level, 4 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );

    ch->alignment += 1;   /*  2000 casts to total goodness   */
    if( ch->alignment > 1000 )
      ch->alignment = 1000;

    return;
}

void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
	victim = ch;
  
    if ( IS_EVIL(victim) )
    {
	act( "God protects $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    dam = dice( level, 4 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );

    ch->alignment -= 2;   /*  1000 casts to total evilness   */
    if( ch->alignment < -1000 )
      ch->alignment = -1000;

    return;
}

void spell_dispel_undead( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC( victim ) || !IS_SET(victim->act , ACT_UNDEAD ) )
    {
	act( "$N is not affected.", ch, NULL, victim, TO_CHAR);
	return;
    }

    dam = dice( level, 7 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 5;
    damage( ch, victim, dam, sn );
    return;
}



void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    /*int affected_by; */
    AFFECT_DATA *paf,*paf_prev;

    if ((victim->last_affect == NULL)
/*   Chaos allowing any level casting
        ||   level < victim->level    */
        ||   saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "You failed.\n\r", ch );
      return;
      }

    if(IS_AFFECTED( victim, AFF_CHARM) && IS_NPC(victim))
      {
      send_to_combat_char( "Leave those pets alone!\n\r", ch );
      return;
      }

/* Order removing bit-removal...and affected_by decl above 2/24/94
    if(victim->affected_by!=0)
      {
      for ( ;; )
        {
        affected_by = 1 << number_bits( 5 );
        if ( IS_SET(victim->affected_by, affected_by) )
           break;
        }
      REMOVE_BIT(victim->affected_by, affected_by);
      }
    else */ if(victim->last_affect !=NULL)
      {
      for ( paf = victim->last_affect; paf != NULL; paf = paf_prev )
        {
        paf_prev	= paf->prev;
        if(!is_spell(paf->type))
          continue;
        if(paf->duration<0)
          ;
        else if ( paf_prev == NULL
            ||   paf_prev->type != paf->type
            ||   paf_prev->duration > 0 )
          {
          if ( paf->type > 0 && skill_table[paf->type].msg_off )
            {
            send_to_combat_char( skill_table[paf->type].msg_off, victim );
            send_to_combat_char( "\n\r", victim );
            }
          }
     
  	    affect_remove( victim, paf );
        break;
        }
      }
    else
      {
      send_to_combat_char( "Nothing appears to happen.\n\r", ch );
      return;
      }

    send_to_combat_char( "Thier magic spells seem to vanish slowly.\n\r", ch );

    return;
}



void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    ROOM_INDEX_DATA *was_in_room = ch->in_room;
    EXIT_DATA *pexit;
    int door;


    for ( vch = ch->in_room->first_person; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next_in_room;
	if ( vch->in_room == NULL )
	    continue;
        if (vch!=ch && ((!IS_NPC(ch) || IS_AFFECTED( ch, AFF_CHARM)) &&
             ( !IS_NPC( vch) || IS_AFFECTED( vch, AFF_CHARM))))
           if( !is_same_group( ch, vch) && ch->fighting!=NULL && ch->fighting->who != vch)
             {
             send_to_combat_char( "It would be unsafe to do that now.\n\r", ch);
             return;
             }
     }

    send_to_combat_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( door = 0; door <= 5; door++ )
    {
      if ( ( pexit = was_in_room->exit[door] ) != NULL
             &&   pexit->to_room != NULL
             &&   pexit->to_room != was_in_room )
      {
        ch->in_room = pexit->to_room;
        act("The earth trembles and shivers.", ch, NULL, NULL, TO_ROOM );
      }
    }
    ch->in_room = was_in_room;

    for ( vch = first_char; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
        if( vch->in_room == NULL )
          continue;
        if( vch->in_room == ch->in_room )
          {
          if (vch==ch || IS_AFFECTED( vch, AFF_CHARM) )
            continue;
          if (!IS_NPC(vch) && who_fighting( ch )!=vch )
            continue;
          if( IS_AFFECTED( vch , AFF_ETHEREAL ) )
            continue;
  if( IS_NPC( vch) && vch->fighting==NULL && 
     vch->npcdata->pvnum_last_hit_leader > 0 &&
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
    if( pvnum_ld != vch->npcdata->pvnum_last_hit_leader )
      {
      continue;
      }
    }
          damage( ch, vch, level + dice(2, 8), sn );
          continue;
          }

/* Eliminate spam.  Presto 8/1/98
	if ( vch->in_room->area == ch->in_room->area   &&   IS_AWAKE(vch) )
	    send_to_combat_char( "The earth trembles and shivers.\n\r", vch );
*/
    }

    return;
}


void spell_tremor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    ROOM_INDEX_DATA *was_in_room = ch->in_room;
    EXIT_DATA *pexit;
    int door;


    for ( vch = ch->in_room->first_person; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next_in_room;
	if ( vch->in_room == NULL )
	    continue;
        if (vch!=ch && ((!IS_NPC(ch) || IS_AFFECTED( ch, AFF_CHARM)) &&
             ( !IS_NPC( vch) || IS_AFFECTED( vch, AFF_CHARM))))
           if( !is_same_group( ch, vch) && ch->fighting!=NULL && ch->fighting->who != vch)
             {
             send_to_combat_char( "It would be unsafe to do that now.\n\r", ch);
             return;
             }
     }

    for ( door = 0; door <= 5; door++ )
    {
      if ( ( pexit = was_in_room->exit[door] ) != NULL
             &&   pexit->to_room != NULL
             &&   pexit->to_room != was_in_room )
      {
        ch->in_room = pexit->to_room;
        act("You hear a loud wrenching noise from the ground.", ch, NULL, NULL, TO_ROOM );
      }
    }
    ch->in_room = was_in_room;
    send_to_combat_char( "The earth tremors beneath your feet!\n\r", ch );
    act( "$n makes the earth shake.", ch, NULL, NULL, TO_ROOM );

    for ( vch = first_char; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
        if( vch->in_room == NULL )
          continue;
        if( vch->in_room == ch->in_room )
          {
          if (vch==ch || IS_AFFECTED( vch, AFF_CHARM) )
            continue;
          if (!IS_NPC(vch) && who_fighting( ch )!=vch )
            continue;
          if( IS_AFFECTED( vch , AFF_ETHEREAL ) )
            continue;
          if( IS_NPC( vch) && vch->fighting!=NULL && !IS_NPC( ch ) &&
                    !is_same_group( ch, vch->fighting->who ))
            continue;
  if( IS_NPC( vch) && vch->fighting==NULL && 
     vch->npcdata->pvnum_last_hit_leader > 0 &&
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
    if( pvnum_ld != vch->npcdata->pvnum_last_hit_leader )
      {
      continue;
      }
    }
          if( number_range( 0, 9) > 3 )
            {
            vch->position = POS_RESTING;
            /*update_pos( vch );*/
            }
          damage( ch, vch, level + dice(12, 8), sn );
          continue;
          }

/*	if ( vch->in_room->area == ch->in_room->area   &&   IS_AWAKE(vch) )
	    send_to_combat_char( "The earth shakes.\n\r", vch );*/
    }

    return;
}



void spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if(multi(ch,sn)==-1 && !IS_NPC(ch))
      {
       send_to_combat_char( "You can't do that!\n\r", ch);
       return;
      }

    if ( obj->item_type != ITEM_WEAPON
    ||   IS_OBJ_STAT(obj, ITEM_MAGIC)
    ||   obj->first_affect != NULL )
      {
       send_to_combat_char( "That object cannot be enchanted.\n\r", ch);
       return;
      }

    obj->basic = FALSE;
    CREATE( paf, AFFECT_DATA, 1);

    /* sn in next line used to be -1 ... -Dug 12/7/93 */
    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= level / 5;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    CREATE( paf, AFFECT_DATA, 1);

    /* sn in next line used to be -1 ... -Dug 12/7/93 */
    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= level / 10;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );


    obj->level+=level/3;

	  SET_BIT(obj->extra_flags, ITEM_LEVEL_RENT);
	  SET_BIT(obj->extra_flags, ITEM_MAGIC);
    if ( IS_GOOD(ch) )
    {
	SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	act( "$p glows blue.", ch, obj, NULL, TO_ROOM );
    }
    else if ( IS_EVIL(ch) )
    {
	SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	act( "$p glows red.", ch, obj, NULL, TO_ROOM );
    }
    else
    {
	SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	act( "$p glows yellow.", ch, obj, NULL, TO_ROOM );
    }

    send_to_combat_char( "It is enchanted.\n\r", ch );
    return;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell( 5*level/4, ch, victim ) )
      {
      switch(number_range(1,3))
      {
      
      case 1: act( "$N shrugs off your deathly embrace!", ch, NULL, victim, TO_CHAR);
	break;
      case 2: act( "$N flinches but is unharmed.", ch, NULL, victim, TO_CHAR);
	break;
      case 3: act( "$N moans softly from your spine chilling spell, but is otherwise unharmed.", ch, NULL, victim, TO_CHAR);
	break;
      }
      return;
      }

    if(!IS_NPC(victim))
      gain_exp( victim, 0 - number_range( level / 4, 3 * level / 4 ) );
    dam		 = number_range(level*2 , level*4) + 5;
    ch->hit		+= (dam/6);
    victim->move	= UMAX(0,number_fuzzy(victim->move-dam*10/13));

    if(ch->hit>ch->max_hit)
      ch->hit=ch->max_hit;

    damage( ch, victim, dam, sn );

    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level*3 , level*5);
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6, 8);
    if ( saves_spell( level, ch, victim ) )
	    dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_combat_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->first_person; ich != NULL; ich = ich->next_in_room )
    {
	if ( !IS_NPC(ich) && IS_SET(ich->act, PLR_WIZINVIS) )
	    continue;

	if ( ich == ch || saves_spell( level, ch, ich ) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_sneak			);
	affect_strip ( ich, gsn_stealth			);
	affect_strip ( ich, gsn_greater_stealth		);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	REMOVE_BIT   ( ich->affected_by, AFF_STEALTH	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_combat_char( "You are revealed!\n\r", ich );
    }

    return;
}



void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) || victim->race == RACE_AVIARAN)
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
   /*  Chaos removed 12/4/93
    char_to_room( create_mobile( get_mob_index(MOB_VNUM_VAMPIRE) ),
	ch->in_room );  */
    return;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_giant_strength( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_combat_char( "You feel the strength of a giant.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell( level, ch, victim ) )
	dam = UMIN( 50, dam / 4 );
    dam = UMIN( 100, dam );
    damage( ch, victim, dam, sn );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_combat_char( "A warm feeling fills your body.\n\r", victim );
    affect_strip(victim,gsn_critical_hit);
    affect_strip(victim,631); /* HALLUCINATE */
    if ( ch != victim )
	send_to_combat_char( "Healing powers at work.\n\r", ch );
    return;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void spell_high_explosive( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    bool found;

    sprintf( buf,
	"%s is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->short_descr,
	item_type_name( obj ),
	extra_bit_name( obj->extra_flags ),
	obj->weight,
	obj->cost,
	obj->level
	);
    send_to_combat_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_combat_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_combat_char( " '", ch );
	    send_to_combat_char( skill_table[obj->value[1]].name, ch );
	    send_to_combat_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_combat_char( " '", ch );
	    send_to_combat_char( skill_table[obj->value[2]].name, ch );
	    send_to_combat_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_combat_char( " '", ch );
	    send_to_combat_char( skill_table[obj->value[3]].name, ch );
	    send_to_combat_char( "'", ch );
	}

	send_to_combat_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d/%d charges of level %d",
	    obj->value[2], obj->value[1], obj->value[0] );
	send_to_combat_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_combat_char( " '", ch );
	    send_to_combat_char( skill_table[obj->value[3]].name, ch );
	    send_to_combat_char( "'", ch );
	}

	send_to_combat_char( ".\n\r", ch );
	break;
      
    case ITEM_WEAPON:
        if(obj->value[0]!=0)
          {
	  sprintf( buf, 
            "Item is a range weapon.  Hand to hand damage is %dd%d.\n\rIt shoots ammo type %d.\n\r",
	    obj->value[1], obj->value[2], obj->value[0]);
          }
        else
	  sprintf( buf, "Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
	send_to_combat_char( buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, "Armor class is %d.\n\r", obj->value[0] );
	send_to_combat_char( buf, ch );
	break;

    case ITEM_AMMO:
        sprintf(buf,
          "Ammo type is %d, Damage is %d, Speed is %d, Range is %d.\n\r",
          obj->value[0],obj->value[1],obj->value[2],obj->value[3]);
	send_to_combat_char( buf, ch );
	break;
    }

    for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_combat_char( buf, ch );
	}
    }

    for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_combat_char( buf, ch );
	}
    }

    sprintf( buf, "Wear locations: " );
    found = FALSE;

    if( ! IS_SET( obj->wear_flags, ITEM_TAKE ) )
      {
      strcat( buf, " Cannot take");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_FINGER ) )
      {
      strcat( buf, " Fingers");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_NECK ) )
      {
      strcat( buf, " Neck");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_BODY ) )
      {
      strcat( buf, " Body");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_HEAD ) )
      {
      strcat( buf, " Head");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_LEGS ) )
      {
      strcat( buf, " Legs");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_FEET ) )
      {
      strcat( buf, " Feet");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_HANDS ) )
      {
      strcat( buf, " Hands");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_ARMS ) )
      {
      strcat( buf, " Arms");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_SHIELD ) )
      {
      strcat( buf, " Shield");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_ABOUT ) )
      {
      strcat( buf, " About");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_WAIST ) )
      {
      strcat( buf, " Waist");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_WRIST ) )
      {
      strcat( buf, " Wrist");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WIELD ) )
      {
      strcat( buf, " Wield");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_HOLD ) )
      {
      strcat( buf, " Hold");
      found = TRUE;
      }
    
    if( IS_SET( obj->wear_flags, ITEM_WEAR_HEART ) )
      {
      strcat( buf, " Heart");
      found = TRUE;
      }
    
    if( !found)
      {
      strcat( buf, " Carry only");
      }
    
    strcat( buf, ".\n\r");
    send_to_combat_char( buf, ch);
    

    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    act( "$n's eyes glow red.", ch, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your eyes glow red.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_enhanced_heal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    /*if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      } Death 2/97*/
    if ( IS_AFFECTED(victim, AFF2_ENHANCED_HEAL) )
      {
      send_to_combat_char("They are already healing easily.\n\r", ch);
      return;
      }

    act( "$n heals easily.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_ENHANCED_HEAL; 
    affect_to_char( victim, &af );
    send_to_combat_char( "You heal easily.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_enhanced_revive( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    /*if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      } Death 2/97*/
    if ( IS_AFFECTED(victim, AFF2_ENHANCED_REVIVE) )
      {
      send_to_combat_char("They are already reviving easily.\n\r", ch);
      return;
      }

    act( "$n revives easily.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_ENHANCED_REVIVE; 
    affect_to_char( victim, &af );
    send_to_combat_char( "You revive easily.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_enhanced_rest( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    /*if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      } Death */
    if ( IS_AFFECTED(victim, AFF2_ENHANCED_REST) )
      {
      send_to_combat_char("They are already resting easily.\n\r", ch);
      return;
      }

    act( "$n rests easily.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_ENHANCED_REST; 
    affect_to_char( victim, &af );
    send_to_combat_char( "You rest easily.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_fear( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, ch, victim )  || !IS_NPC( victim ) )
       {
       send_to_combat_char( "They are not made fearless.\n\r", ch );
       return;
       }
    if( !IS_SET( victim->act, ACT_WIMPY ) )
       {
       send_to_combat_char( "They are already fearless.\n\r", ch );
       return;
       }
    REMOVE_BIT( victim->act, ACT_WIMPY );
    act( "$n becomes fearless!", victim, NULL, NULL, TO_ROOM );
    return;
}


void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    /*if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      } Death 2/97*/

    if(victim->race == RACE_AVIARAN)
    {
      send_to_combat_char("Aviarans may not be hastened.\n\r", ch);
      return;
    }

    if ( IS_AFFECTED(victim, AFF_HASTE) )
      {
      send_to_combat_char("They are already fast.\n\r", ch);
      return;
      }

    act( "$n speeds up.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_combat_char( "You speed up.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch != victim)
      {
      send_to_combat_char("You are forbidden from casting that on another here.\n\r", ch);
      return;
      }
    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
      {
      send_to_combat_char("They are already invisible.\n\r", ch);
      return;
      }

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_combat_char( "You fade out of existence.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_invis_obj( int sn, int level, CHAR_DATA *ch, void *vo)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    SET_BIT( obj->extra_flags, ITEM_INVIS);
    obj->basic = FALSE;

    send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_tongues( int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_TONGUES))
     {
     send_to_combat_char( "That person is already speaking in tongues.\n\r", ch);
     return;
     }

  act( "$n starts speaking in tongues.", victim, NULL, NULL, TO_ROOM);
    af.type      = sn;
    af.duration  = 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_TONGUES;
    affect_to_char( victim, &af);
    send_to_combat_char( "You start speaking in tongues.\n\r", victim);
    if(ch!=victim)
      send_to_combat_char( "Ok.\n\r", ch);
    return;
}
void spell_understand( int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if(IS_AFFECTED(victim,AFF_UNDERSTAND))
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
  act( "$n starts understanding everyone.", victim, NULL, NULL, TO_ROOM);
   af.type      = sn;
   af.duration  = 12;
   af.location  = APPLY_NONE;
   af.modifier  = 0;
   af.bitvector = AFF_UNDERSTAND;
   affect_to_char( victim, &af);
   send_to_combat_char( "You start understanding everyone.\n\r", victim);
   if(ch!=victim)
     send_to_combat_char( "Ok.\n\r", ch);
   return;
}


void spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    if( victim==NULL || victim->in_room==NULL || victim->in_room!=ch->in_room )
      {
      send_to_char( "They are not here.\n\r", ch );
      return;
      }

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N's slash DISEMBOWELS you!";
    else msg = "I'd rather just not say anything at all about $N.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level*2+10, level*4+40);
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int count;

    found = FALSE;
    count=0;
    for ( obj = first_object; obj != NULL && count<10+level/5; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !nifty_is_name( target_name, obj->name ) )
	    continue;

	found = TRUE;
    count++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL )
	{
            if(obj->carried_by !=NULL 
 	       && !IS_NPC(obj->carried_by) 
	       && IS_SET( obj->carried_by->act, PLR_WIZINVIS ) )
              continue; 
           if( obj->carried_by != NULL
               && is_affected(obj->carried_by, gsn_greater_stealth) 
               && !((is_affected(ch, gsn_truesight) 
               && ch->mclass[CLASS_ILLUSIONIST] >= obj->carried_by->level) 
               || IS_SET(ch->act, PLR_HOLYLIGHT)))
              continue;
 
            if (can_see(ch,in_obj->carried_by))
	      sprintf( buf, "%s carried by %s.\n\r",
		obj->short_descr, PERS(in_obj->carried_by, ch) );
            else
	      sprintf( buf, "%s carried by someone.\n\r", obj->short_descr);
	}
	else
	{
	    sprintf( buf, "%s in %s.\n\r",
		obj->short_descr, in_obj->in_room == NULL
		    ? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	send_to_combat_char( buf, ch );
    }

    if ( !found )
	send_to_combat_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level/3+2, level/2+6);
    /* Save vs magic missile ?
    if ( saves_spell( level, ch, victim ) )
	     dam /= 2;
    */
    damage( ch, victim, dam, sn );
    return;
}



void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_combat_char( "You slowly fade out of existence.\n\r", gch );
	af.type      = gsn_invis;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_combat_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_combat_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You turn translucent.\n\r", victim );
    return;
}



void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_POISON) )
      {
      send_to_combat_char( "They are already poisoned.\n\r", ch );
      return;
      }
    if ( saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Your poison spell does not work.\n\r", ch );
      return;
      }
    if(IS_NPC(victim))
      {
      af.type      = sn;
      af.duration  = level;
      af.location  = APPLY_AC;
      af.modifier  = 2*level;
      af.bitvector = AFF_POISON;
      }
    else
      {
      af.type      = sn;
      af.duration  = level/4;
      af.location  = APPLY_STR;
      af.modifier  = -2;
      af.bitvector = AFF_POISON;
      }
    affect_join( victim, &af );
    send_to_combat_char( "You feel very sick from the poison that stikes you.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "They now feel your poison.\n\r", ch );
    return;
}


void spell_mage_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if( IS_AFFECTED( victim, AFF2_MAGE_BLAST ) )
      {
      send_to_combat_char( "Your blast was ineffective.\n\r", ch );
      return;
      }
    if ( saves_spell( level, ch, victim ) )
      {
      af.type      = sn;
      af.duration  = level/80;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF2_MAGE_BLAST;
      affect_join( victim, &af );
      send_to_combat_char( "Your blast was repelled.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level/20;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level/5;
    af.bitvector = AFF2_MAGE_BLAST;
    affect_join( victim, &af );
    send_to_combat_char( "Your magic aura has been weakened.\n\r", victim );
    if ( ch != victim )
     send_to_combat_char( "You blast through their magical defenses.\n\r", ch );
    return;
}


void spell_protection_fe( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }


    af.type      = sn;
    af.duration  = level/5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_combat_char( "You feel protected from evil.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Protection has been applied from evil.\n\r", ch );
    return;
}

void spell_protection_fg( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }

    af.type      = sn;
    af.duration  = level/5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_combat_char( "You feel protected from good.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Protection from good is now working.\n\r", ch );
    return;
}


void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    send_to_combat_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Movement is now restoring.\n\r", ch );
    return;
}



void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    if ( is_affected( victim, gsn_curse ) )
    {
	affect_strip( victim, gsn_curse );
	send_to_combat_char( "You feel better.\n\r", victim );
	if ( ch != victim )
	    send_to_combat_char( "You spell has taken the curse away.\n\r", ch );
        return;
    }
    for(obj=victim->first_carrying; obj!=NULL ; obj=obj->next_content)
       if((!IS_SET(obj->extra_flags, ITEM_INVENTORY)) &&
          (IS_SET(obj->extra_flags, ITEM_NODROP) || 
           IS_SET(obj->extra_flags, ITEM_NOREMOVE)))
         {
         REMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE);
         REMOVE_BIT( obj->extra_flags, ITEM_NODROP);
         if( victim!=ch)
           send_to_combat_char( "Ok.\n\r", ch);
         sprintf( buf, "%s is no longer cursed.\n\r", obj->short_descr);
         if( buf[0]>='a' && buf[0]<='z')
           buf[0]-=('a'-'A');
         send_to_combat_char( buf, victim);
         return;
         }
    send_to_combat_char( "That had no effect.\n\r", ch);
    return;
}



void spell_ethereal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *paf, *paf_next;
    AFFECT_DATA af;

    if( IS_NPC( victim ) )
      {
      send_to_char( "You may only cast that on players.\n\r", ch );
      return;
      }

    if ( IS_AFFECTED(victim, AFF2_ETHEREAL) )
      {
      for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
        {
        paf_next = paf->next;
        if( paf->bitvector==AFF2_ETHEREAL )
          {
          paf->duration  = number_fuzzy( level / 2 );
          send_to_char( "You faze out a bit more.\n\r", victim );
          if( victim != ch )
            send_to_char( "Ok.\n\r", ch );
          return;
          }
        }
      }

    af.type      = sn;
    af.duration  = number_fuzzy( level / 2 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_ETHEREAL;
    affect_to_char( victim, &af );
    act( "$n partially fades out.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You partially fade out.\n\r", victim );
    return;
}



void spell_astral( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af, *paf, *paf_next;

    if( IS_NPC( victim ) )
      {
      send_to_char( "You may only cast that on players.\n\r", ch );
      return;
      }

    if ( IS_AFFECTED(victim, AFF2_ASTRAL) )
      {
      for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
        {
        paf_next = paf->next;
        if( paf->bitvector == AFF2_ASTRAL )
          {
          paf->duration  = number_fuzzy( level / 3 );
          send_to_char( "You no longer have the urge to return to your body.\n\r", victim );
          if( victim != ch )
            send_to_char( "Ok.\n\r", victim );
          return;
          }
        }
      }

    af.type      = sn;
    af.duration  = number_fuzzy( level / 3 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_ASTRAL;
    affect_to_char( victim, &af );
    act( "$n steps outside of $s body.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You step outside your body.\n\r", victim );
    return;
}

void spell_breath_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af, *paf, *paf_next;

    if( IS_NPC( victim ) )
      {
      send_to_char( "You may only cast that on players.\n\r", ch );
      return;
      }

    if ( IS_AFFECTED(victim, AFF2_BREATH_WATER) )
      {
      for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
        {
        paf_next = paf->next;
        if( paf->bitvector == AFF2_BREATH_WATER )
          {
          paf->duration  = number_fuzzy( level / 3 );
          act( "$n takes another deep breath.", victim, NULL, NULL, TO_ROOM );
          send_to_combat_char( "You take another deep breath.\n\r", victim );
          /*if( victim != ch )
            send_to_char( "Gills appear so water can be breathed.\n\r", victim );*/
          return;
          }
        }
      }

    af.type      = sn;
    af.duration  = number_fuzzy( level / 3 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_BREATH_WATER;
    affect_to_char( victim, &af );
    act( "$n takes a deep breath.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You take a deep breath.\n\r", victim );
    return;
}


void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
        {
        send_to_combat_char( "That person already has Sanctuary.\n\r", ch);
	return;
        }
    af.type      = sn;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        {
        send_to_combat_char( "That person already has Shield.\n\r", ch);
	return;
        }
    af.type      = sn;
    af.duration  = 8 + level/2;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}



void spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam   = number_range( level*3/2+5, level*5/2+10);
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      }
    if( !IS_NPC( ch ) && !IS_NPC( victim ) )
     if( ( which_god(ch)==GOD_NEUTRAL ) ||       /*Neutral can't sleep player*/
         ( which_god(ch)==GOD_INIT_CHAOS ) ||    /*Neutral can't sleep player*/
         ( which_god(ch)==GOD_INIT_ORDER ) ||    /*Neutral can't sleep player*/
         ( which_god(victim)==GOD_NEUTRAL ) ||    /*Neutral can't sleep player*/
         ( which_god(victim)==GOD_INIT_CHAOS ) || /*Neutral can't sleep player*/
         ( which_god(victim)==GOD_INIT_ORDER ) || /*Neutral can't sleep player*/
         ( which_god(ch)==which_god(victim) ) || /*Can't sleep same god*/
         ( which_god(victim)==GOD_POLICE ) )     /*Can't sleep an officer*/
      {
      send_to_combat_char("You cannot cast sleep on that person.\n\r", ch);
      return;
      }
    if ( IS_AFFECTED(victim, AFF_SLEEP) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    if(  level < victim->level )
      {
      send_to_combat_char( "They are too high of a level.\n\r", ch );
      return;
      }
    if(  saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Nothing happens.\n\r", ch );
      return;
      }

    af.type      = sn;
    af.duration  = 4 + level;
    if(!IS_NPC(victim))
      af.duration= 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_combat_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    if ( IS_NPC( victim ) )
      start_hating( victim, ch );

    return;
}



void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "Your skin turns to stone.\n\r", victim );
    return;
}

void spell_demon( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
    return;
    }

  if(get_pets(ch)>0)
    {
    send_to_combat_char("You have too many pets.\n\r", ch);
    return;
    }
  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9901 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=level*3/4;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level;
  mh->npcdata->damsizedice = 4;
  mh->npcdata->damplus = 3;
  mh->max_hit=80+mh->level*4;
  mh->hit=mh->max_hit;
  mh->max_move=1200;
  mh->move=1200;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100-mh->level*5;
  mh->npcdata->armor = mh->level*5;

  SET_BIT( mh->act, ACT_UNDEAD);

  if( number_percent() < 66)
    {
    SET_BIT( mh->affected_by , AFF_CHARM );
    SET_BIT( mh->act , ACT_PET );
    send_to_combat_char( "A demon appears out of a rip in space.\n\r", ch);
    act("A demon appears out of a rip in space.", ch, NULL, NULL, TO_ROOM);
    add_follower( mh , ch );
    return;
    }
  mh->level=level;
  mh->max_hit=30+mh->level*5;
  send_to_combat_char( "A demon appears out of a rip in space.\n\rHe seems angry.\n\r", ch);
  multi_hit( mh, ch, TYPE_UNDEFINED);
  return;
}

void spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  OBJ_DATA  *obj, *cobj;
  CHAR_DATA *mh;
  char buf[MAX_STRING_LENGTH];

  if( get_pets( ch) >0 )
    {
    send_to_combat_char( "You have too many pets.\n\r", ch);
    return;
    }

  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  cobj = NULL;

  for( obj = ch->in_room->first_content ; obj != NULL ; obj = obj->next_content )
    if( obj->item_type == ITEM_CORPSE_NPC )
      if( obj->level <= level )
        break;
      
  if( obj == NULL )
    {
    send_to_combat_char( "You find no suitable corpse.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9903 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=obj->level;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level;
  mh->npcdata->damsizedice = 3;
  mh->npcdata->damplus = 2;
  mh->max_hit=50+mh->level*5;
  mh->hit=mh->max_hit;
  mh->max_move=200;
  mh->move=200;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100- mh->level*5;
  mh->npcdata->armor = mh->level*5;

  SET_BIT( mh->affected_by , AFF_CHARM );
  SET_BIT( mh->act, ACT_PET);
  SET_BIT( mh->act, ACT_UNDEAD);

  for( cobj = obj->first_content; cobj != NULL ; cobj = cobj->next_content )
    {
    obj_from_obj( cobj );
    obj_to_char( cobj, mh );
    }
  sprintf( buf, "$n raises %s from death.", obj->short_descr );
  act( buf, ch, NULL, NULL, TO_ROOM );


   STRFREE (mh->name );
   STRFREE (mh->short_descr );
   STRFREE (mh->long_descr );
   STRFREE (mh->description );
   mh->name = STRALLOC( obj->name );
   mh->short_descr = STRALLOC( obj->short_descr );
   mh->long_descr = STRALLOC( obj->long_descr );
   mh->description = STRALLOC( obj->description );

  obj_from_room( obj );
  extract_obj( obj );

  add_follower( mh , ch );
  if( ch->fighting!= NULL )
    multi_hit( mh, ch->fighting->who, TYPE_UNDEFINED);
  return;
}

void spell_beast( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;

  if( get_pets( ch) >0 )
    {
    send_to_combat_char( "You have too many pets.\n\r", ch);
    return;
    }

  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9902 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=level*3/4;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level;
  mh->npcdata->damsizedice = 2;
  mh->npcdata->damplus = 5;
  mh->max_hit=30+level*4;
  mh->hit=mh->max_hit;
  mh->max_move=800;
  mh->move=800;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100- mh->level*5;
  mh->npcdata->armor = mh->level*5;

  SET_BIT( mh->affected_by , AFF_CHARM );
  SET_BIT( mh->act, ACT_UNDEAD);
  SET_BIT( mh->act, ACT_PET);
  send_to_combat_char( "A shadow beast appears out of a ripple in the wall.\n\r", ch);
  add_follower( mh , ch );
  if( ch->fighting!= NULL )
    multi_hit( mh, ch->fighting->who, TYPE_UNDEFINED);
  return;
}

void spell_shade( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
    return;
    }

  if(get_pets(ch)>0)
    {
    send_to_combat_char("You have too many pets.\n\r", ch);
    return;
    }
  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9903 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=level*2/3;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level/2;
  mh->npcdata->damsizedice = 2;
  mh->npcdata->damplus = 5;
  mh->max_hit=30+level*3;
  mh->hit=mh->max_hit;
  mh->max_move=400;
  mh->move=400;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100-mh->level*4;
  mh->npcdata->armor = mh->level*4;

  SET_BIT( mh->affected_by , AFF_CHARM );
  SET_BIT( mh->act, ACT_UNDEAD);
  SET_BIT( mh->act , ACT_PET );
  send_to_combat_char( "A Shade appears out of a ripple in the floor.\n\r", ch);
  add_follower( mh , ch );
  return;
}

void spell_phantasm( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;

  if( get_pets(ch)>0)
   {
   send_to_combat_char( "You have too many pets.\n\r", ch);
   return;
   }
  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9904 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=level*1/2;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level;
  mh->npcdata->damsizedice = 5;
  mh->npcdata->damplus = 50;
  mh->max_hit=30+mh->level*2;
  mh->hit=mh->max_hit;
  mh->max_move=1500;
  mh->move=1500;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 0-mh->level*8;
  mh->npcdata->armor = mh->level*8;

  SET_BIT( mh->affected_by , AFF_CHARM );
  SET_BIT( mh->act, ACT_UNDEAD);
  SET_BIT( mh->act , ACT_PET );
  send_to_combat_char( "A phantasmal killer appears out of the palm of your hand.\n\r", ch);
  add_follower( mh , ch );
  if( ch->fighting!= NULL)
    multi_hit( mh, ch->fighting->who, TYPE_UNDEFINED );
  return;
}

void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL)
       {
       send_to_combat_char( "There is none named that.\n\r", ch);
       return;
       }
    if ((IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE)
        || IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE))
        || (IS_NPC(victim)  &&  level/3 < victim->level ))
         {
         send_to_combat_char( "You cannot sommon those that do not wish to leave.\n\r",
             ch);
         return;
         }
  
  if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("The flows of magic are repelled by some sort of shell around $N.", ch, NULL, victim, TO_CHAR);
     return; 
    }
    if(  victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->in_room->sector_type == SECT_ETHEREAL 
    ||   victim->in_room->sector_type == SECT_ASTRAL 
    ||   ch->in_room->sector_type == SECT_ETHEREAL 
    ||   ch->in_room->sector_type == SECT_ASTRAL 
    ||   IS_SET( ch->in_room->area->flags, AFLAG_NORECALL) 
    ||   victim->level > level - 3
    ||   victim->fighting != NULL
    ||   IS_AFFECTED( victim, AFF2_STABILITY )
/*  Chaos not allowing other area summoning    12/3/93  */
    ||   victim->in_room->area != ch->in_room->area    )
    {
	send_to_combat_char( "You failed.\n\r", ch );
	return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    ch_printf(victim, "%s has summoned you!", get_name(ch));
    do_look( victim, "auto" );
    return;
}



void spell_banish( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
        || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
        || IS_SET( victim->in_room->area->flags, AFLAG_NORECALL) 
        || victim == ch
        || IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE)
 	|| victim->fighting!=NULL
 	|| ch->fighting!=NULL
        || ( who_fighting(victim)!= ch  && who_fighting(ch)!= victim ) 
        || saves_spell( level, ch, victim ) 
        || saves_spell( level, ch, victim ) 
        || saves_spell( level, ch, victim ) )
      {
	send_to_combat_char( "You failed.\n\r", ch );
	return;
      }
    if( saves_spell( level, ch, victim ) && saves_spell( level, ch, victim ) )
    {
	send_to_combat_char( "You failed badly.\n\r", ch );
        victim = ch;
    }

    for ( ; ; )
    {
	pRoomIndex = get_room_index( victim->in_room->area->low_r_vnum +
                                 number_range( 0, 99 ) );
	if ( pRoomIndex != NULL )
	if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP) )
	    break;
    }

    act( "$n is banished from existence.", victim, NULL, NULL, TO_ROOM );
    if( victim->position == POS_FIGHTING )
      leave_fighting( victim, victim->in_room);
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n is banished from existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *pRoomIndex;
  char buf[MAX_STRING_LENGTH];
  int attempts = 0;

  /* This is causing an infinite loop...disabling until I find the 
     problem - Martin 27/2/99
     I couldn't find any reason for this to lock up.  Worked successfully
     for me every time I tried it, and that was a lot.  Put in a counter
     to see how many times it attempts and fails.  If it fails 1000 times in
     a row, it'll exit.  - Presto 3/10/99
  send_to_combat_char("This spell is disabled until further notice. Sorry!\n\r", ch);
  return;
  */

  if(!IS_NPC(ch))
    victim = ch;

  if(victim->in_room == NULL ||
     IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ||
     IS_SET(victim->in_room->area->flags, AFLAG_NORECALL) ||
     (!IS_NPC(ch) &&
      victim->fighting != NULL) ||
     (IS_NPC(ch) &&
      saves_spell(level, ch, victim))) 
  {
    send_to_combat_char( "You failed.\n\r", ch );
    return;
  }

  if(IS_AFFECTED(victim, AFF2_STABILITY))
  {
    send_to_combat_char( "You fail.\n\r", ch );
    return;
  }

  for(;;)
  {
    /* Read note above. - Presto */
    attempts++;
    if(attempts > 1000)
    {
      send_to_combat_char("The earth decides you should remain where you are.\n\r", ch);
      sprintf(buf, "Teleport failure: Name %s, Room %d", capitalize(ch->name),
              ch->in_room->vnum);
      bug(buf);
      return;
    }
    pRoomIndex = get_room_index( number_range(1, MAX_VNUM-1));
    if(pRoomIndex != NULL)
      if(!IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) &&
         !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) &&
         !IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE) &&
         !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP) &&
         !IS_SET(pRoomIndex->room_flags, ROOM_RIP) &&
         !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) &&
         !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT) &&
         !IS_SET(pRoomIndex->area->flags, AFLAG_NORECALL) &&
         pRoomIndex->area->average_level*2 < victim->level+15 &&
         pRoomIndex->sector_type != SECT_ETHEREAL &&
         pRoomIndex->sector_type != SECT_ASTRAL &&
         !((!IS_IMMORTAL(victim)) &&
           (victim->level<pRoomIndex->area->low_hard_range ||
            victim->level>pRoomIndex->area->hi_hard_range)))
        break;
  }

  act("$n slowly fades out of existence.", victim, NULL, NULL, TO_ROOM);
  if(victim->position == POS_FIGHTING && !IS_NPC(ch))
    leave_fighting(victim, victim->in_room);
  char_from_room(victim);
  char_to_room(victim, pRoomIndex);
  act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
  do_look(victim, "auto");
  return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf1[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

  if( ch->in_room == NULL )
    return;
  for( vch = ch->in_room->first_person; vch != NULL; vch=vch->next_in_room )
    if( !IS_NPC( vch ) && vch->level >= 99 )
      {
      sprintf( buf1, "You may not cast that with %s in the room.\n\r",
        capitalize( vch->name ) );
      send_to_combat_char( buf1, ch );
      return;
      }

    target_name = one_argument( target_name, speaker );


    for ( vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room )
     if( vch->position >= POS_RESTING )
       {
	if ( !is_name_short( speaker, vch->name ) )
          {
          if( !saves_spell( level, ch, vch ) )
            sprintf( buf1, "%s%s says '%s'\n\r",
                get_color_string(vch,COLOR_SPEACH,VT102_DIM),
                capitalize( speaker ), target_name );
          else
            sprintf( buf1, "%sSomeone makes %s say '%s'\n\r",
                get_color_string(vch,COLOR_SPEACH,VT102_DIM),
                capitalize(speaker), target_name );
	  send_to_combat_char( buf1, vch );
          }
       }

    return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    if( saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Nothing happens.\n\r", ch );
      return;
      }

      af.type      = sn;
      af.duration  = level / 2;
      if(IS_NPC(victim))
       af.location  = APPLY_DAMROLL;
      else
       af.location  = APPLY_STR;
      af.modifier  = 0-(level/10)-2;
      af.bitvector = 0;

    affect_to_char( victim, &af );
    send_to_combat_char( "Your strength flows out of your body.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "They now lost the strenght to fight.\n\r", ch );
    return;
}



/*
 * This is for muds that _want_ scrolls of recall.
 * Ick.
 */
void spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    do_recall( (CHAR_DATA *) vo, NULL );
    return;
}



/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, ch, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose != NULL; obj_lose = obj_next )
	{
	    int iWear;

	    obj_next = obj_lose->next_content;

	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    case ITEM_ARMOR:
		if ( obj_lose->value[0] > 0 )
		{
		    act( "$p is pitted and etched!",
			victim, obj_lose, NULL, TO_CHAR );
		    if ( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
			victim->armor -= apply_ac( obj_lose, iWear );
		    obj_lose->value[0] -= 1;
		    obj_lose->cost      = 0;
		    if ( iWear != WEAR_NONE )
			victim->armor += apply_ac( obj_lose, iWear );
		}
		break;

/*	    case ITEM_CONTAINER:
		act( "$p fumes and dissolves!",
		    victim, obj_lose, NULL, TO_CHAR );
      {
      OBJ_DATA *objc,*obj_next;

      for ( objc = obj_lose->first_content; objc != NULL; objc = obj_next )
	      {
 	      obj_next = objc->next_content;
        obj_from_obj( objc);
        objc->sac_timer=OBJ_SAC_TIME;
        obj_to_room( objc, ch->in_room);
        }
      } 
		  extract_obj( obj_lose );
		break;  */
	    }
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, ch, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose != NULL;
	obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( number_bits( 2 ) != 0 )
		continue;
            if( obj_lose->first_content != NULL )
                continue;

	    switch ( obj_lose->item_type )
	    {
	    default:             continue;
	    case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
	    case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
	    case ITEM_SCROLL:    msg = "$p crackles and burns!";  break;
	    case ITEM_STAFF:     msg = "$p smokes and chars!";    break;
	    case ITEM_WAND:      msg = "$p sparks and sputters!"; break;
	    case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
	    case ITEM_PILL:      msg = "$p melts and drips!";     break;
	    }

	    act( msg, victim, obj_lose, NULL, TO_CHAR );
      /*{
      OBJ_DATA *objc,*obj_next;

      for ( objc = obj_lose->first_content; objc != NULL; objc = obj_next )
	      {
 	      obj_next = objc->next_content;
        obj_from_obj( objc);
        objc->sac_timer=OBJ_SAC_TIME;
        obj_to_room( objc, ch->in_room);
        }
      } */
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, ch, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose != NULL;
	obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( number_bits( 2 ) != 0 )
		continue;
            if( obj_lose->first_content != NULL )
                continue;

	    switch ( obj_lose->item_type )
	    {
	    default:            continue;
	    case ITEM_CONTAINER:
	    case ITEM_DRINK_CON:
	    case ITEM_POTION:   msg = "$p freezes and shatters!"; break;
	    }
      
	    act( msg, victim, obj_lose, NULL, TO_CHAR );
      {
      OBJ_DATA *objc,*obj_next;

      for ( objc = obj_lose->first_content; objc != NULL; objc = obj_next )
	      {
 	      obj_next = objc->next_content;
        obj_from_obj( objc);
        objc->sac_timer=OBJ_SAC_TIME;
        obj_to_room( objc, ch->in_room);
        }
      }
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}



void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;

    for ( vch = ch->in_room->first_person; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
	{
	    hpch = UMAX( 10, ch->hit );
	    dam  = number_range( hpch/16+1, hpch/8 );
	    if ( saves_spell( level, ch, vch ) )
		dam /= 2;
	    damage( ch, vch, dam, sn );
	}
    }
    return;
}



void spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int hpch;

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch/16+1, hpch/8 );
    if ( saves_spell( level, ch, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return;
}


void spell_block_area( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  if(IS_SET(ch->in_room->room_flags,ROOM_SAFE) ||
     ch->in_room->sector_type < 2 ||
     ch->in_room->sector_type > 4 )
    {
    send_to_combat_char("You are restricted from blocking this area.\n\r",ch);
    return;
    }
  if(IS_SET(ch->in_room->room_flags,ROOM_BLOCK))
    {
    send_to_combat_char("The area pulsates with life but nothing new appears.\n\r",ch);
    return;
    }
  SET_BIT(ch->in_room->room_flags, ROOM_BLOCK);
  act( "All sorts of plant growth sprout up everywhere!",ch,NULL,NULL,TO_ROOM);
  act( "Your magic fills the area with plant life!", ch, NULL, NULL, TO_CHAR );
  return;
  }

void spell_write_spell( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  OBJ_DATA *scroll;
  int sn2,i;

    if(IS_NPC(ch) || multi(ch, sn)==-1)
      {
       send_to_combat_char( "You can't do that!\n\r", ch);
       return;
      }

  if (( ( scroll = get_eq_char( ch,WEAR_HOLD) ) == NULL )||
      ( scroll->item_type != ITEM_SCROLL ))
    {
    send_to_combat_char("You need to hold the scroll you wish to write upon.\n\r",ch);
    return;
    }

  for(i=0;i<100;i++)
    {
    sn2=number_range(0,MAX_SKILL);
    if( skill_table[sn2].skill_level[multi(ch, sn2)]>55 ) continue;
    if( skill_table[sn2].slot == 602 || skill_table[sn2].slot == 653) continue;
    if((skill_table[sn2].slot!=0)&&(IS_NPC(ch)||(ch->pcdata->learned[sn2]!=0)))
      break;
    }
  if(i>=100)
    {
    send_to_combat_char("Your mind goes blank!\n\r",ch);
    ch->mana=0;
    return;
    }

  if( sn2>=MAX_SKILL || skill_table[sn2].name==NULL )
    {
    send_to_combat_char("The spell fails.\n\r",ch);
    return;
    }


  if(level<21)
    sprintf(buf,"a haphazardly written scroll of %s",skill_table[sn2].name);
  else if(level<31)
    sprintf(buf,"a decently scribed scroll of %s",skill_table[sn2].name);
  else
    sprintf(buf,"a skillfully crafted scroll of %s",skill_table[sn2].name);
  STRFREE (scroll->short_descr);
  scroll->short_descr=STRALLOC(buf);
  STRFREE (scroll->description);
  scroll->description=STRALLOC(buf );

  sprintf(buf2,"scroll %s",skill_table[sn2].name);
  STRFREE (scroll->name);
  scroll->name=STRALLOC(buf2);

  scroll->cost=0;
  scroll->level=ch->level *3 /4;
  SET_BIT(scroll->extra_flags, ITEM_LEVEL_RENT);

    if((multi(ch,sn2)==-1 || multi(ch, sn)==-1)&& !IS_NPC(ch))
      {
       send_to_combat_char( "You can't do that!\n\r", ch);
       return;
      }

  if(!IS_NPC(ch))
    scroll->value[0]=ch->mclass[multi(ch,sn2)];
  else
    scroll->value[0]=ch->level/4;
  scroll->value[1]=sn2;
  scroll->value[2]=-1;
  scroll->value[3]=-1;

  scroll->basic = FALSE;
  
  act("$n writes what is hoped to be a useful spell.",
      ch,scroll,NULL,TO_ROOM);
  act("You enter a trance and...\n\r...you now hold $p!",
      ch,scroll,NULL,TO_CHAR);

  return;
  }

void spell_homonculous( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;
  char buf[81];

  /* send_to_combat_char( "This spell is temporarily offline.\n\r", ch);
  return;  */

  if(ch->desc->original!=NULL)
    {
    send_to_combat_char( "You already are switched into something.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9900 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->hitroll = 0;
  mh->npcdata->damnodice = 1;
  mh->npcdata->damsizedice = 6;
  mh->npcdata->damplus = 1;
  mh->max_hit=15;
  mh->hit=mh->max_hit;
  mh->max_move=900;
  mh->move=900;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 70;
  mh->npcdata->armor = 30;
  mh->master = NULL;
  REMOVE_BIT(mh->act,ACT_AGGRESSIVE);
  SET_BIT(mh->act,ACT_SENTINEL);


  act("A clap of thunder and a small, ugly creature appears!",ch,NULL,NULL,TO_ROOM);
  act("A clap of thunder and you suddenly feel smaller!",ch,NULL,NULL,TO_CHAR);

  sprintf(buf,"%s casting Homonculous spell.",ch->name);
  log_string(buf);

  ch->desc->character = mh;
  ch->desc->original  = ch;
  mh->desc            = ch->desc;
  ch->desc            = NULL;
  ch->pcdata->switched = TRUE;

  return;
  }

void spell_enhance_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int stat;

    if( obj==NULL)
      {
      send_to_combat_char( "You do not have that object.\n\r", ch);
      return;
      }

    if( obj->owned_by!=0)
      {
      send_to_combat_char( "You cannot enhance that object.\n\r", ch);
      return;
      }
      
    stat=1;
    if( level >30)
      stat=2;
    if( level >50)
      stat=3;
    if( level >70)
      stat=4;

    obj->basic = FALSE;

    
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
	      send_to_combat_char( "You cannot enhance that kind of an object.\n\r", ch);
	      return;
	    }
      obj->owned_by=ch->pcdata->pvnum;
      obj->basic = FALSE;

    paf->type           = skill_lookup( "enchant" );
    paf->duration       = -1;
    paf->bitvector      = 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    char_reset( ch );

     send_to_combat_char( "It is enhanced.\n\r", ch);
    return;
}

void spell_mage_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( ch, sn ) )
      {
      send_to_combat_char( "You are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = level/3;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -level/5;
    af.bitvector = AFF2_MAGE_SHIELD;
    affect_to_char( ch, &af );
    act( "$n is quickly covered by a shimmering glow.",
                ch, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You are quickly covered by a shimmering glow.\n\r", victim );
    return;
}



#define HIGHEST_RIFT_VNUM   20000
void spell_rift( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  char buf1[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *to_room;
  int door,ripDoor,index,seed;

  /* make sure room is a rip */
  if((ch->in_room->room_flags & ROOM_RIP)==0)
    {
    send_to_combat_char("You can only create rifts from inside of a rip.\n\r",ch);
    return;
    }
  /* get wrinkle factor */
  if(value==32000)
    {
    send_to_combat_char("You must specify a wrinkle factor to create a rift.\n\r",ch);
    return;
    }
  seed=value;
  seed+=(IS_NPC(ch))?ch->pIndexData->vnum:ch->pcdata->pvnum;
  /* seed the random number generator */
  srand(seed+1);

  /* find a room to connect to */
  for(;((to_room=get_room_index(rand() % HIGHEST_RIFT_VNUM))==NULL)||
       IS_SET(to_room->room_flags, ROOM_SAFE)||
       IS_SET(to_room->room_flags, ROOM_IS_CASTLE)||
       IS_SET(to_room->room_flags, ROOM_PET_SHOP) ||
       IS_SET(to_room->room_flags, ROOM_PRIVATE) ||
       IS_SET(to_room->room_flags, ROOM_SOLITARY) ||
       IS_SET(to_room->room_flags, ROOM_NO_RECALL) ||
       to_room->sector_type == SECT_ETHEREAL ||
       to_room->sector_type == SECT_ASTRAL ||
       IS_SET(to_room->area->flags, AFLAG_NOTELEPORT) ||
       IS_SET(to_room->area->flags, AFLAG_NORECALL)  ||
      ( (to_room->area->low_hard_range!=0 ||
       to_room->area->hi_hard_range!=0) &&
               ( !IS_IMMORTAL(ch)) &&
        (ch->level<to_room->area->low_hard_range ||
           ch->level>to_room->area->hi_hard_range) ) ;)
    continue;

  /* pick a random exit */
  ripDoor=-1;
  for(index=0;index<12;index++)
    {
    door=number_range(0,5);
    /* check for existing rifts, is possible that existing rift isn't seen */
    if((ch->in_room->exit[door]!=NULL)&&
       ((ch->in_room->exit[door]->exit_info & EX_ISDOOR)!=0))
      {
      send_to_combat_char("Sorry, only one rift per rip.\n\r",ch);
      return;
      }
    if((to_room->exit[door]==NULL) &&
       (ch->in_room->exit[rev_dir[door]]==NULL))
      {
      ripDoor=rev_dir[door];
      break;
      }
    }
  if(ripDoor==-1)
    {
    send_to_combat_char("You failed to open a rift using that wrinkle in this room.\n\r",ch);
    return;
    }

  /* new room exit points to old room */
  CREATE(pexit, EXIT_DATA, 1);
  pexit->description	= STRALLOC("");
  pexit->keyword        = STRALLOC("");
  pexit->exit_info	= EX_RIP;
  if(IS_NPC(ch))
    pexit->pvnum  = -1;
  else
    pexit->pvnum  = ch->pcdata->pvnum;
  pexit->key		= -1;
  pexit->vnum		= ch->in_room->vnum;
  pexit->to_room        = ch->in_room;
  to_room->exit[rev_dir[ripDoor]]= pexit;
  top_exit++;

  /* old rooms exit points to new room */
  CREATE(pexit, EXIT_DATA, 1);
  pexit->description	= STRALLOC( "A rift in space and time." );
  pexit->keyword        = STRALLOC("");
  pexit->exit_info	= EX_ISDOOR;
  pexit->key		= -1;
  pexit->vnum		= to_room->vnum;
  pexit->to_room        = to_room;
  ch->in_room->exit[ripDoor] = pexit;
  top_exit++;

  /* show the happenings */
  act("Space and time appear to warp for a split second!",ch,NULL,NULL,TO_ROOM);
  switch(ripDoor)
    {
    case DIR_UP:
      strcpy(buf2,"above you");
      break;
    case DIR_DOWN:
      strcpy(buf2,"below you");
      break;
    default:
      sprintf(buf2,"to the %s",dir_name[ripDoor]);
      break;
    }
  sprintf(buf1,"You cause space and time to warp irreparably %s!",buf2);
  act(buf1, ch, NULL, NULL, TO_CHAR );

  return;
  }

void spell_rip( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *pRoomIndex;
  int door,ripDoor,vnum,iHash,range;

  if (IS_NPC(ch))
   return;

  /* make sure room isn't a NO_RECALL */
  if((!IS_SET(ch->in_room->room_flags, ROOM_RIP))&&
      (IS_SET(ch->in_room->room_flags, ROOM_NO_RIP)||
      (IS_SET(ch->in_room->room_flags, ROOM_SAFE)||
      (IS_SET(ch->in_room->area->flags, AFLAG_NORECALL))||
      (IS_SET(ch->in_room->area->flags, AFLAG_NORIP))||
       IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))))
    {
    send_to_combat_char("You are prevented from ripping space here!\n\r",ch);
    return;
    }

  /* check for existing door in room */
  for(door=0,range=0;door<6;door++)
    if(ch->in_room->exit[door]==NULL)
      range++;
  if(range==0)
    {
    send_to_combat_char("You cannot find space to create the rip here!\n\r",ch);
    return;
    }
  /* pick a random empty direction */
  iHash=number_range(1,range);
  ripDoor=0;
  for(door=0;door<6;door++)
    {
    if(ch->in_room->exit[door]==NULL)
      if((--iHash)==0)
        {
        ripDoor=door;
        break;
        }
    }
      
  /* find valid vnum */
  for(vnum=32000;vnum<MAX_VNUM && get_room_index(vnum)!=NULL;vnum++);
  if( vnum>=MAX_VNUM )
    {
    send_to_combat_char("You are prevented from ripping up the Realm!\n\r",ch);
    return;
    }

  /* create room ROOM_RIP, NO_SAVE, NO_MOB, INDOORS, SAFE, NO_RECALL */
  sprintf(buf,"Haven a la %s.",ch->name);
  CREATE(pRoomIndex, ROOM_INDEX_DATA, 1);
  pRoomIndex->name= STRALLOC(buf);
  pRoomIndex->first_person	  = NULL;
  pRoomIndex->last_person	  = NULL;
  pRoomIndex->first_content	  = NULL;
  pRoomIndex->first_extradesc = NULL;
  pRoomIndex->area	  = get_room_index(ROOM_VNUM_LIMBO)->area;

  pRoomIndex->vnum	  = vnum;
  pRoomIndex->owned_by  = ch->pcdata->pvnum;
  total_rips++;

  pRoomIndex->description = STRALLOC( "You see an area that defies all description.");
  pRoomIndex->room_flags  = ROOM_RIP | ROOM_NO_MOB | ROOM_INDOORS |
              /*            ROOM_SAFE | */ ROOM_NO_RECALL | ROOM_NO_SAVE;
  pRoomIndex->sector_type = SECT_INSIDE;
  pRoomIndex->light       = 0;
  for ( door = 0; door <= 5; door++ )
    pRoomIndex->exit[door] = NULL;
  for ( door = 0; door< MAX_LAST_LEFT; door++)
    strcpy( pRoomIndex->last_left[door], "");

  /* new room exit points to old room */
  CREATE(pexit, EXIT_DATA, 1);
  pexit->description=STRALLOC("The EXIT." );
  pexit->keyword        = STRALLOC("");
  pexit->exit_info	= 0;
  pexit->pvnum  = -1;
  pexit->key		= -1;
  pexit->vnum		= ch->in_room->vnum;
  pexit->to_room        = ch->in_room;
  pRoomIndex->exit[rev_dir[ripDoor]]= pexit;
  top_exit++;

  /* old rooms exit points to new room */
  CREATE(pexit, EXIT_DATA, 1);
  pexit->description	= STRALLOC("");
  pexit->keyword        = STRALLOC("");
  pexit->exit_info	= EX_RIP;
  if(IS_NPC(ch))
    pexit->pvnum  = -1;
  else
    pexit->pvnum  = ch->pcdata->pvnum;
  pexit->key		= -1;
  pexit->vnum		= pRoomIndex->vnum;
  pexit->to_room        = pRoomIndex;
  ch->in_room->exit[ripDoor] = pexit;
  top_exit++;

  /* put new room in hash table */
  iHash			= vnum % MAX_KEY_HASH;
  pRoomIndex->next	= room_index_hash[iHash];
  room_index_hash[iHash]= pRoomIndex;
  top_room++;
  room_index[vnum]=pRoomIndex;

  /* show the happenings */
  act("A door appears!",ch,NULL,NULL,TO_ROOM);
  switch(ripDoor)
    {
    case DIR_UP:
      strcpy(buf2,"above you");
      break;
    case DIR_DOWN:
      strcpy(buf2,"below you");
      break;
    default:
      sprintf(buf2,"to the %s",dir_name[ripDoor]);
      break;
    }
  sprintf(buf,"You create a room of haven %s!",buf2);
  act(buf, ch, NULL, NULL, TO_CHAR );

  return;
  }

void do_rcast( CHAR_DATA *ch, char *argument )
  {
  char *arg1,buf1[MAX_INPUT_LENGTH];
  char *arg2,buf2[MAX_INPUT_LENGTH];
  char *arg3,buf3[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *old_room;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn,cnt,dir,actn;
  int level;

  old_room=ch->in_room;

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_combat_char( "You cannot do that here.\n\r", ch);
    return;
    }
      /*  No homonculous or pets */
    if ( IS_NPC(ch) && ( IS_AFFECTED( ch, AFF_CHARM) || 
          ch->pIndexData->vnum==9900))
	return;

  if( strlen( argument) > 120 )
    *(argument+120)='\0';

  target_name = one_argument( argument, buf1 );
  target_name = one_argument( target_name, buf2 );
  target_name = one_argument( target_name, buf3 );

  /*
   * Get proper room for given direction
   */
  if((dir=getDirNumber(buf1))==-1)
    {
    if((dir=getDirNumber(buf2))==-1)
      {
      if((dir=getDirNumber(buf3))==-1)
        {
        send_to_combat_char("Range cast which direction?\n\r",ch);
        return;
        }
      /* buf3 is direction */
      arg1=buf1;
      arg2=buf2;
      arg3=buf3;
      }
    /* buf2 is direction */
    arg1=buf1;
    arg2=buf3;
    arg3=buf2;
    }
  else
    {
    /* buf1 is direction */
    arg1=buf2;
    arg2=buf3;
    arg3=buf1;
    }

  if(ch->in_room->exit[dir]==NULL || ch->in_room->exit[dir]->to_room==NULL ||
     IS_SET(ch->in_room->exit[dir]->to_room->room_flags, ROOM_SAFE) ||
     IS_SET(ch->in_room->exit[dir]->exit_info,EX_CLOSED))
    {
    send_to_combat_char( "You can't range cast that direction!\n\r", ch );
    return;
    }

  /* can't cast into a shop, so check for a shop_keeper */
  for ( victim = ch->in_room->exit[dir]->to_room->first_person; victim;
        victim = victim->next_in_room )
    {
	  if ( IS_NPC(victim) && (victim->pIndexData->pShop != NULL ))
      {
      send_to_combat_char( "You can't range cast that direction!\n\r", ch );
      return;
      }
    }

    /* Disallow casting across area boundaries   -  Chaos  3/2/98 */
  if( ch->in_room->area != ch->in_room->exit[dir]->to_room->area )
      {
      send_to_combat_char( "You can't range cast that direction!\n\r", ch );
      return;
      }
    

  if ( arg1[0] == '\0' )
    {
    send_to_combat_char( "Cast which what where?\n\r", ch );
    return;
    }

  sn = skill_lookup( arg1 );
  if( sn < 0 || !is_spell( sn))
    {
    send_to_combat_char( "That is not a spell.\n\r", ch );
    return;
    }

  cnt = skill_table[sn].skill_level[CLASS_ELEMENTALIST];
  if(cnt>ch->mclass[CLASS_ELEMENTALIST] && !IS_NPC(ch))
    {
    send_to_combat_char("Only Elementalist spells are able to be cast at a range.\n\r"
                 ,ch);
    return;
    }
  
  if ( ch->position < skill_table[sn].minimum_position )
    {
    send_to_combat_char( "You can't concentrate enough.\n\r", ch );
    return;
    }

  cnt = multi(ch,sn);

  /* range casts cost three times as much mana as normal -Dug */
  mana = 3 * get_mana(ch,sn);

  /*
   * Locate targets.
   */
  victim	= NULL;
  obj		= NULL;
  vo		= NULL;
  level = ch->mclass[cnt];
      
    if( skill_table[sn].skill_level[cnt] > level )
      {
      send_to_combat_char( "You are not quite high enough level to cast that spell.\n\r", ch );
      return;
      }

  switch ( skill_table[sn].target )
    {
    default:
      bug( "Do_rcast: bad target for sn %d.", sn );
      return;

    case TAR_CHAR_SELF:
    case TAR_OBJ_INV:
      send_to_combat_char("That type of target is not allowed at range.\n\r",ch);
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

      if ( arg2[0] == '\0' )
	{
        if ( ( victim = who_fighting(ch) ) == NULL )
          {
          send_to_combat_char( "Cast the spell on whom?\n\r", ch );
          return;
          }
           if( victim->name == NULL )
		return;
        }
      else
        {
        char_from_room( ch );
        char_to_room( ch, old_room->exit[dir]->to_room );
        if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
          {
          send_to_combat_char( "They aren't there.\n\r", ch );
          char_from_room( ch );
          char_to_room( ch, old_room );
          return;
          }
        char_from_room( ch );
        char_to_room( ch, old_room );
           if( victim->name == NULL )
		return;
        }
  if(( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if( IS_NPC( victim) && IS_AFFECTED( victim, AFF_CHARM)
        && (victim->fighting == NULL || victim->fighting->who!=ch))
      {
      send_to_combat_char( "You can't do that to a pet.\n\r", ch);
      return;
      }

  if( IS_NPC( victim) && IS_AFFECTED( victim, AFF_CHARM))
    {
    send_to_combat_char( "You can't cast that on a pet.\n\r", ch);
    return;
    }
    /* Limitations of player vs. player  -  Chaos  3/24/99   */
  if( ch!=victim )
  if( ch->fighting==NULL || ch->fighting->who!=victim )
   if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if((IS_NPC(victim) && IS_AFFECTED(victim,AFF_CHARM)) || !IS_NPC(victim))
     if( ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
      {
      send_to_combat_char( "You can't do that.\n\r", ch);
      return;
      }
 
  if( IS_NPC( victim) && victim->fighting!=NULL && !IS_NPC( ch ) &&
        !is_same_group( ch, victim->fighting->who ))
    {
    char buf[160];
    sprintf( buf ,"%s seems to be busy!\n\r", capitalize(victim->short_descr));
    send_to_combat_char( buf, ch);
    return;
    }
  if( IS_NPC( victim) && victim->fighting==NULL && 
      victim->npcdata->pvnum_last_hit_leader > 0 &&
      ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) ) )
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
            capitalize(victim->short_descr));
      send_to_combat_char( buf, ch);
      return;
      }
    }

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:
      if ( arg2[0] == '\0' )
        {
        victim = ch;
        }
      else
        {
        old_room=ch->in_room;
        char_from_room( ch );
        char_to_room( ch, old_room->exit[dir]->to_room );
        if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
          {
          send_to_combat_char( "They aren't here.\n\r", ch );
          char_from_room( ch );
          char_to_room( ch, old_room );
          return;
          }
        char_from_room( ch );
        char_to_room( ch, old_room );
        }
        if(ch!=victim && (!IS_NPC( victim) || IS_AFFECTED(victim,AFF_CHARM)) &&
            victim->fighting!=NULL && 
            ( (victim->fighting!=NULL && !IS_NPC(victim->fighting->who)) || !is_same_group( ch, victim )) &&
            !IS_NPC( ch ) )
          {
          char buf[160];
          sprintf(buf ,"%s seems to be busy!\n\r", get_name( victim ) );
          send_to_combat_char( buf, ch);
          return;
          }
        if(ch!=victim && (IS_AFFECTED(victim, AFF_CHARM) || !IS_NPC( victim))&&
               !IS_NPC(ch) && ( 60 * level / 100 > victim->level+5 ))
          {
          char buf[160];
          sprintf(buf ,"Your spell is reduced in strength to keep from killing %s.\n\r",
                                                   get_name( victim ) );
          send_to_combat_char( buf, ch);
          level = victim->level * 100 / 60 + 5;
          }

      vo = (void *) victim;
      break;
    }
	    
  if ( !IS_NPC(ch) && ch->mana < mana )
    {
    send_to_combat_char( "You don't have enough mana.\n\r", ch );
    return;
    }
      
  WAIT_STATE( ch, skill_table[sn].beats  * 4 / 5 );
      
  if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[sn] +
         (get_curr_int(ch)-13)*2 )
    {
    send_to_combat_char( "You lost your concentration.\n\r", ch );
    ch->mana -= mana / 2;
    }
  else
    {
    ch->mana -= mana;
    old_room=ch->in_room;
    /* temporarily move caster to victim */
    char_from_room(ch);
    char_to_room(ch,old_room->exit[dir]->to_room);
    /* turn off autosac, autoloot */
    actn=ch->act;
    REMOVE_BIT(ch->act,PLR_AUTOSAC|PLR_AUTOLOOT);
    send_to_combat_char("You close your eyes, envision that area, and cast!\n\r",ch);
    if(IS_NPC(ch))
      (*skill_table[sn].spell_fun) ( sn, ch->level/4, ch, vo );
    else
      if(multi(ch,sn)!=-1)
        {
        (*skill_table[sn].spell_fun) ( sn, level, ch, vo );
        if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE &&   victim != ch )
          make_char_fight_char( ch, victim );
        }
      else
        {
        send_to_combat_char( "You can't do that!\n\r", ch);
        return;
        }
    /* restore previous settings */
    ch->act=actn;
    char_from_room(ch);
    char_to_room(ch,old_room);
    }

  /* Move some creatures into room of caster */
  for( victim=first_char; victim!=NULL ; victim=victim->next)
   if(victim->fighting!=NULL && victim->fighting->who == ch && victim->in_room != ch->in_room)
    {
    if(IS_NPC(victim) && !IS_AFFECTED(victim, AFF_ETHEREAL) &&
       !(IS_AFFECTED( victim, AFF_CHARM)&&victim->master!=NULL))
      {
      CHAR_DATA *wch, *fch[51];
      int counter;

      victim->shot_timer=2;
      victim->shot_from=rev_dir[dir];
      mprog_range_trigger(victim,ch);

      for( wch=victim->in_room->first_person, counter=0; counter<50 && wch != NULL; 
             wch= wch->next_in_room )
         if((!IS_SET(wch->act,ACT_TRAIN   )) && 
            (!IS_SET(wch->act,ACT_PRACTICE)) &&
            (wch->in_room &&(!IS_SET(wch->in_room->room_flags, ROOM_PET_SHOP))))
           fch[counter++]=wch;
      fch[counter]=NULL;
      for(counter=0; fch[counter]!=NULL; counter++)
        {
        wch=fch[counter];
        if( !IS_AFFECTED(wch, AFF_ETHEREAL)&& (( number_range(1 , 4) ==1 && IS_NPC(wch) && wch->level <= ch->level
            && !(IS_AFFECTED( wch, AFF_CHARM)&&victim->master!=NULL )) ||
            ( number_range(1 , 8) ==1 && IS_NPC(wch) && 
            !(IS_AFFECTED( wch, AFF_CHARM)&&victim->master!=NULL) )
            || ( wch==victim && number_range(1,3) != 1) ))
          {
          char_from_room(wch);
          char_to_room(wch,ch->in_room);
          act("$n arrives, glaring around angrily!",wch,NULL,NULL,TO_ROOM);
          if( wch->in_room == ch->in_room  && wch->fighting!=NULL && wch->fighting->who != ch &&
              wch == victim )
            set_fighting( wch, ch);
          }
        }
      }
    }
  return;
  }

/*
 * Takes any offensive spell for an Illusionist and
 * defensive spell for a monk and applies it to all the appropriate char's.
 * Costs more mana than the combined casts, but takes less time.
 */
void do_mass( CHAR_DATA *ch, char *argument )
  {
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim,*next_vict;
  int mana;
  int sn,cnt,learned,mlev,ilev;
  int level;

  /*  No homonculous or pets */
  if(IS_NPC(ch)&&(IS_AFFECTED(ch,AFF_CHARM) || ch->pIndexData->vnum==9900))
    return;

  if( find_keeper( ch ) != NULL )
    {
    send_to_combat_char( "You can't cast a spell in a shop!\n\r", ch);
    return;
    }

  if (!IS_NPC(ch) && is_affected(ch, gsn_anti_magic_shell))
   {  
     act("No matter how hard you try, the flows of magic will not break through the shell\n\rsurrounding you.", ch, NULL, NULL, TO_CHAR);
     return; 
   }

  argument = one_argument( argument, arg1 );

  if(arg1[0] == '\0')
    {
    send_to_combat_char( "Cast which what where?\n\r", ch );
    return;
    }

  learned=(IS_NPC(ch))?100:ch->pcdata->learned[gsn_mass];
  if(learned<=0)
    {
    send_to_combat_char( "You can't do that!\n\r", ch );
    return;
    }
  if(number_percent()>learned)
    {
    send_to_combat_char("You fail to enter the proper state of mind to amass that much energy!\n\r",ch);
    WAIT_STATE( ch, 16 );
    return;
    }

  sn = skill_lookup( arg1 );
  if( sn < 0 || !is_spell( sn))
    {
    send_to_combat_char( "That is not a spell.\n\r", ch );
    return;
    }

    cnt = multi( ch, sn);

    if(cnt==-1 && !IS_NPC(ch))
       {
	send_to_combat_char( "You can't do that.\n\r", ch );
	return;
       }
  if( cnt==-1 )
    level = ch->level;
  else
    level = ch->mclass[cnt];
  mlev=(IS_NPC(ch))?100:ch->mclass[CLASS_MONK];
  ilev=(IS_NPC(ch))?100:ch->mclass[CLASS_ILLUSIONIST];

  if(((skill_table[sn].target==TAR_CHAR_OFFENSIVE && 
      ilev<skill_table[gsn_mass].skill_level[CLASS_ILLUSIONIST]) ||
      ( skill_table[sn].target==TAR_CHAR_DEFENSIVE && 
      mlev<skill_table[gsn_mass].skill_level[CLASS_MONK])) &&
     !IS_NPC(ch) )
    {
    send_to_combat_char( "You don't have enough skill in the proper class!\n\r", ch );
    return;
    }

  if(!IS_NPC(ch) && skill_table[sn].target==TAR_CHAR_OFFENSIVE &&
     (ch->fighting==NULL))
    {
    send_to_combat_char( "You aren't even fighting!\n\r", ch );
    return;
    }
    
  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE) &&
      skill_table[sn].target==TAR_CHAR_OFFENSIVE && !IS_NPC(ch))
    {
    send_to_combat_char( "You cannot do that here.\n\r", ch);
    return;
    }
  
  if ( ch->position < skill_table[sn].minimum_position )
    {
    send_to_combat_char( "You can't concentrate enough.\n\r", ch );
    return;
    }

  /* mass'ing spells costs 5 more mana */
  mana=5*get_mana(ch,sn)/4;

  /*
   * Locate targets.
   */
  WAIT_STATE( ch, 12 );
  for(victim=ch->in_room->first_person;victim!=NULL;victim=next_vict)
    {
    next_vict=victim->next_in_room;
    if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
    {  
     act("$N shrugs as you attempt to direct the flows of magic towards $M.", ch, NULL, victim, TO_CHAR);
     continue; 
    }
    switch(skill_table[sn].target)
      {
      case TAR_CHAR_OFFENSIVE: 
      /* Stop fights of aggressives if currently hurt - Chaos 11/10/97  */
    if( IS_NPC(ch) && ch->fighting==NULL && ch->hit < ch->max_hit )
      if( !IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    /* Limitations of player vs. player  -  Chaos  3/24/99   */
  if( ch!=victim )
  if( ch->fighting==NULL || ch->fighting->who!=victim )
   if( (IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM)) || !IS_NPC(ch))
    if((IS_NPC(victim) && IS_AFFECTED(victim,AFF_CHARM)) || !IS_NPC(victim))
     if( ch->in_room->area->low_r_vnum!=ROOM_VNUM_ARENA)
      {
      break;
      }

        {
        if ( victim->fighting!=NULL && victim->fighting->who==ch )
          {
          if ( !IS_NPC(ch) && ch->mana < mana )
            {
            act("You don't have enough mana for $N.",ch,NULL,victim,TO_CHAR);
            return;
            }
          WAIT_STATE( ch, skill_table[sn].beats * 1 / 3 );
          if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[sn]+
                 (get_curr_int(ch)-13)*2 )
            {
            act("You lost your concentration on $N.",ch,NULL,victim,TO_CHAR);
            ch->mana -= mana / 2;
            }
          else
            {
            ch->mana -= mana;
            (*skill_table[sn].spell_fun) ( sn, IS_NPC(ch) ? ch->level *3/4 : 
                     ch->mclass[cnt], ch, victim );
            }
          if ( victim->fighting == NULL )
            {
            multi_hit( victim, ch, TYPE_UNDEFINED );
            break;
            }
          }
        break;
        }
      case TAR_CHAR_DEFENSIVE: 
        {
        if(ch!=victim && (!IS_NPC( victim) || IS_AFFECTED(victim,AFF_CHARM)) &&
            victim->fighting!=NULL && 
            ( !IS_NPC(victim->fighting->who) || !is_same_group( ch, victim )) &&
            !IS_NPC( ch ) )
          {
          char buf[160];
          sprintf(buf ,"%s seems to be busy!\n\r", get_name( victim ) );
          send_to_combat_char( buf, ch);
          return;
          }
        /*
        What on earth is this supposed to do ??!? It breaks mass! - Martin

	if(ch!=victim && (IS_AFFECTED(victim, AFF_CHARM) || !IS_NPC( victim))&&
               !IS_NPC(ch) && ( 60 * level / 100 > victim->level+5 ))
          {
          char buf[160];
          sprintf(buf ,"Your spell is reduced in strength to keep from killing %s.\n\r",
                                                   get_name( victim ) );
          send_to_combat_char( buf, ch);
          return;
          }*/
        if(is_same_group(ch,victim))
          {
          if ( !IS_NPC(ch) && ch->mana < mana )
            {
            act("You don't have enough mana for $N.",ch,NULL,victim,TO_CHAR);
            return;
            }
          WAIT_STATE( ch, skill_table[sn].beats * 1 / 3 );
          if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[sn]+
                 (get_curr_int(ch)-13)*2 )
            {
            act("You lost your concentration on $N.",ch,NULL,victim,TO_CHAR);
            ch->mana -= mana / 2;
            }
          else
            {
            ch->mana -= mana;
            (*skill_table[sn].spell_fun) ( sn, IS_NPC(ch) ? ch->level *3/4 : 
                     ch->mclass[cnt], ch, victim );
        if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE &&   victim != ch )
          make_char_fight_char( ch, victim );
    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
            }
          }
        break;
        }
      }
    }
      
  return;
  }

void spell_illusion( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh,*victim;

  if(get_pets(ch)>0)
    {
    send_to_combat_char("You have too many pets.\n\r", ch);
    return;
    }
  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }

  pMob = get_mob_index( 9905 );   /* Hard coded in merc.are */
  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=3*level/4;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level;
  mh->npcdata->damsizedice = 2;
  mh->npcdata->damplus = 3;
  mh->max_hit=1;
  mh->hit=mh->max_hit;
  mh->max_move=900;
  mh->move=900;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100-mh->level*5;
  mh->npcdata->armor = mh->level*5;

  if(target_name==NULL || target_name[0]=='\0')
    {/* nothing specified so make it look like the caster */
    STRFREE (mh->name);
    STRFREE (mh->short_descr);
    if(IS_NPC(ch))
      {
      mh->name=STRALLOC(ch->name);
      mh->short_descr=STRALLOC(ch->short_descr);
      }
    else
      {
      mh->name=STRALLOC(ch->name);
      mh->short_descr=STRALLOC(ch->name);
      }
    STRFREE (mh->long_descr);
    mh->long_descr=STRALLOC(ch->long_descr);
    STRFREE (mh->description);
    mh->description=STRALLOC(ch->description);
    mh->race=ch->race;
    mh->language=ch->language;
    mh->sex=ch->sex;
    }
  else if((victim=get_char_room(ch,target_name))!=NULL)
    {/* specified a name in the room so make it look like the named */
    STRFREE (mh->name);
    STRFREE (mh->short_descr);
    if(IS_NPC(victim))
      {
      mh->name=STRALLOC(victim->name);
      mh->short_descr=STRALLOC(victim->short_descr);
      }
    else
      {
      mh->name=STRALLOC(victim->name);
      mh->short_descr=STRALLOC(victim->name);
      }
    STRFREE (mh->long_descr);
    mh->long_descr=STRALLOC(victim->long_descr);
    STRFREE (mh->description);
    mh->description=STRALLOC(victim->description);
    mh->race=victim->race;
    mh->language=victim->language;
    mh->sex=victim->sex;
    }
  else
    {/* specified the short description so make one */
    STRFREE (mh->name);
    mh->name=STRALLOC(target_name);
    STRFREE (mh->short_descr);
    mh->short_descr=STRALLOC(target_name);
    STRFREE (mh->long_descr);
    mh->long_descr=STRALLOC("");
    STRFREE (mh->description);
    mh->description=STRALLOC("");
    mh->race=ch->race;
    mh->language=ch->language;
    mh->sex=ch->sex;
    }

  SET_BIT( mh->affected_by , AFF_CHARM );
  act("$N takes shape before your eyes.",ch,NULL,mh,TO_CHAR);
  act("$N takes shape before your eyes.",ch,NULL,mh,TO_ROOM);
  add_follower( mh , ch );
  return;
  }

void spell_mirror_image( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
    send_to_combat_char( "Mirror images can't be compounded!\n\r", ch );
    return;
    }
  af.type      = sn;
  if((!IS_NPC(ch)) && ch->mclass[CLASS_ILLUSIONIST]>0)
    {
    af.duration = (ch==victim)?10:5;
    af.modifier = UMAX( (((ch==victim)?4:1)*(1+(level/20)) / 2), 1) ;
    }
  else
    {
    af.duration  = 10;
    af.modifier  = 1+(level/30);
    }
  af.location  = APPLY_NONE;
  af.bitvector = AFF2_MIRROR_IMAGE;
  affect_to_char( victim, &af );
  send_to_combat_char("Several of you take a step out and away from yourself.\n\r",
               victim );
  if ( ch != victim )
    send_to_combat_char( "Ok.\n\r", ch );
  return;
  }

void spell_hallucinate( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
    send_to_combat_char( "They can't get any worse!\n\r", ch );
    return;
    }
    if ( is_affected(victim, gsn_truesight ))
      {
      act( "$N sees through the veil on reality you try to impose.", ch, NULL, victim, TO_CHAR );
      act( "You see through the veil on reality $n attempts to impose.", ch, NULL, victim, TO_VICT );
      return;
      }
  af.type      = sn;
  af.duration  = level/31;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF2_HALLUCINATE;
  affect_to_char( victim, &af );
  send_to_combat_char("Wow!  You see lot's of pretty colors!\n\r", victim );
  if(ch!=victim)
    send_to_combat_char( "Ok.\n\r", ch );
  return;
  }


void spell_stability( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
    if( victim != ch )
      send_to_combat_char( "They are quite stable.\n\r", ch );
    else
      send_to_combat_char( "You already feel the solidity of the earth.\n\r", ch );
    return;
    }
  af.type      = sn;
  af.duration  = level/4;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF2_STABILITY;
  affect_to_char( victim, &af );
  send_to_combat_char("The weight of the earth begins to creep into your soul.\n\r",
       victim );
  if(ch!=victim)
    send_to_combat_char( "The earth feels heavy now.\n\r", ch );
  return;
  }


void spell_confusion( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF2_CONFUSION) || 
         saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "You cast confusion.\n\r", ch );
      return;
      }
    
      af.type      = sn;
      af.duration  = level/30+1;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF2_CONFUSION;
       
    affect_join( victim, &af );
    send_to_combat_char( "Your eyes shimmer.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "You cast confusion.\n\r", ch );
    return;
}



void spell_sanctify( int sn, int level, CHAR_DATA *ch, void *vo )
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *fch;

  if( ch->in_room == NULL )
    return;

  room = ch->in_room;

  if( IS_SET( room->room_flags, ROOM_SAFE ) )
    {
    send_to_char( "This room is already safe enough.\n\r", ch );
    return;
    }

  for(fch=room->first_person; fch!=NULL; fch=fch->next_in_room )
    if( fch->position==POS_FIGHTING || fch->fighting != NULL )
      {
      send_to_char( "There is too much violence in the room to cast sanctify.\n\r", ch );
      return;
      }

  /* Can't sanctify the arena.  Presto  8-1-98 */
  if(ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
  {
    send_to_char("The arena may not be sanctified.\n\r", ch);
    return;
  }

  room->sanctify_timer = 16 + level/5;
  room->sanctify_char = ch;
  SET_BIT( room->room_flags, ROOM_SAFE );

  send_to_char( "The room becomes sanctified, and is a sanctuary for all.\n\r",
     ch);
  act( "$n prays to God and makes this small area a sanctuary to all.",
     ch, NULL, NULL, TO_ROOM );

  return;
  }


void spell_benediction( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
      {
       send_to_combat_char( "They are fighting.\n\r", ch );
       return;
      }
    if( is_affected( victim, sn ) )
      {
       send_to_combat_char( "They are already affected.\n\r", ch );
       return;
      }
    if ( ( ch->level < (((victim->level)*3)/4)-2 ||
            ch->level > (((victim->level)*5)/4)+2  ) )
     {
      ch_printf(ch,  "Your god feels that %s is unworthy of your benediction.\n\r", get_name(victim) );
      return; 
     }

    af.type      = sn;
    af.location  = APPLY_HIT;
    af.modifier  = number_fuzzy((ch->mclass[CLASS_MONK]/6)*25);

    if (victim != ch)
     af.modifier /=2;

    af.duration  = number_fuzzy((ch->mclass[CLASS_MONK]/15));
    af.bitvector = 0;
    affect_to_char( victim, &af );
   
    if (victim!=ch)
    {
     act("You make a sign of benediction over $N's head.",ch,NULL,victim,TO_CHAR);
     act("$n makes a sign of benediction over your head.",ch,NULL,victim,TO_VICT);
    }
    else
     act("You make the sign of benediction.",ch,NULL,victim,TO_CHAR);
    return;
}
void spell_righteous_fury( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
      {
      send_to_combat_char( "They are fighting.\n\r", ch );
      return;
      }
    if( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
    af.type      = sn;
    af.duration  = number_fuzzy(ch->mclass[CLASS_MONK]/14);
    af.location  = APPLY_HITROLL;
    af.modifier  = number_fuzzy (level / 7); 
    if (victim != ch)
     af.modifier /= 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = number_fuzzy (level / 8);
    if (victim != ch)
     af.modifier /= 2;
    affect_to_char( victim, &af );
   
    act("You are filled with divine fury!",ch,NULL,victim,TO_VICT);
    if (ch!=victim)
     act("You invoke the wrath of your god, filling $N with divine fury.",ch,NULL,victim,TO_CHAR);
    else
     act("You invoke the wrath of your god!",ch,NULL,victim,TO_CHAR);

   return;
}

void spell_soothing_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch;
    int heal;

    heal = dice(10, 10) + level*5/2 ;
    for (gch=ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
    {
     if (is_same_group(gch, ch) 
         && (gch->fighting== NULL || IS_NPC(gch->fighting->who)))
     {
      gch->hit = UMIN( gch->hit + heal, gch->max_hit );
      update_pos( gch );
      if (gch != ch)
       act("$n's soothing touch makes your wounds close and your pain fade.", ch,NULL, gch, TO_VICT );
      else
       act("Your soothing touch makes your wounds close and your pain fade.", ch,NULL, gch, TO_VICT );
     }
    }
   send_to_combat_char( "Ok.\n\r", ch );
   return;
}
void spell_farheal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    int heal;

    if ( ( victim = get_player_world( ch, target_name ) ) == NULL)
       {
       send_to_combat_char( "Farheal who ?\n\r", ch);
       return;
       }

    if(!IS_NPC(ch) && ch->mclass[CLASS_MONK]!=ch->level)
     {
      send_to_combat_char("You lack the single minded devotion to reach through the void.\n\r", ch);
      return;
     }

    if( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   victim->in_room->sector_type == SECT_ETHEREAL 
    ||   victim->in_room->sector_type == SECT_ASTRAL 
    ||   ch->in_room->sector_type == SECT_ETHEREAL 
    ||   ch->in_room->sector_type == SECT_ASTRAL 
    ||   IS_NPC(victim)
    ||   victim->fighting != NULL )
    {
      ch_printf(ch, "You can not reach %s through the void.\n\r", 
 	get_name(victim));
      return;
    }
    if (victim == ch)
    {
 	send_to_char( "You can't farheal yourself!\n\r", ch); 
        return;
    }
    if (victim->in_room == ch->in_room)
    {
      ch_printf(ch, "%s is in the same room as you!\n\r", 
 	get_name(victim));
      return;
    }
      heal = dice(11, 9) + level;
      victim->hit = UMIN( victim->hit + heal, victim->max_hit );
      update_pos( victim );
      if ( ch != victim ) 
      {
       act( "$n reaches through the void to fill you with healing energy.", ch, NULL, victim, TO_VICT );
       act ( "You reach through the void to $N, filling $M with healing energy.", ch, NULL, victim, TO_CHAR );
      }
     else send_to_combat_char( "You have healed them from afar.\n\r", ch);
  
    return;
}
void spell_holy_word( int sn, int level, CHAR_DATA *ch, void *vo )
{
 return;
}
void spell_unholy_word( int sn, int level, CHAR_DATA *ch, void *vo )
{
 return;
}

void spell_invigorate( int sn, int level, CHAR_DATA *ch, void *vo )
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal, scale_top, scale_bot;
    char buf[MAX_INPUT_LENGTH];

    if( victim==NULL || victim->in_room == NULL || ch->in_room==NULL ||
        victim->in_room != ch->in_room )
      {
      send_to_combat_char( "That person is not here.\n\r", ch );
      return;
      }

    scale_top = 29;  /* The fraction for the hp/mana ratio */
    scale_bot = 7;

    if( value == -1 || value > ch->mana )
      value = ch->mana;

    heal = value * scale_top / scale_bot ;
    if( victim->move + heal > victim->max_move )
      heal = victim->max_move - victim->move ;
    value = ( heal * scale_bot / scale_top );

    sprintf( buf, "Invigorating %d moves for %d mana.\n\r", heal, 
                      value + get_mana(ch,sn));
    send_to_combat_char( buf, ch);

    victim->move += heal;
    ch->mana -= value;

    send_to_combat_char( "A delicious chill runs up your spine, cleansing the lethargy from your limbs.\n\r", victim );
    return;
}

void spell_improved_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
      {
      send_to_combat_char("They are already invisible.\n\r", ch);
      return;
      }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch != victim)
      {
      send_to_combat_char("You are forbidden from casting that on another here.\n\r", ch);
      return;
      }

    act( "Something unseen enshrouds $n.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = number_fuzzy(ch->mclass[CLASS_ILLUSIONIST]/7);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_combat_char( "You are enshrouded by something unseen.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
 return;
}

void spell_truesight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    affect_strip( victim, gsn_truesight);

    af.type      = sn;
    af.duration  = number_fuzzy(level/4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_combat_char( "Your eyes gain the ability to see through the facades of mundane reality.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_anti_magic_shell( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    /*AFFECT_DATA * paf;
    AFFECT_DATA * paf_next;*/

    if ( victim->position == POS_FIGHTING )
      {
      send_to_combat_char( "They are fighting.\n\r", ch );
      return;
      }

    if( victim!=ch )
      {
      send_to_combat_char( "You may only cast this on yourself.\n\r", ch );
      return;
      }

    if( is_affected( victim, sn ) )
      {
      send_to_combat_char( "You are already affected.\n\r", ch );
      return;
      }
/*
    for ( paf = victim->first_affect; paf != NULL; paf = paf_next)
      {
         paf_next = paf->next;
         affect_remove(victim,paf);
      }
*/

    af.type      = sn;
    af.duration  = number_fuzzy(level/12);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_combat_char( "The flows of magic recede from your body.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_smoke( int sn, int level, CHAR_DATA *ch, void *vo )
{
  ROOM_INDEX_DATA *room;

  if( ch->in_room == NULL )
    return;

  room = ch->in_room;

  if( IS_SET( room->room_flags, ROOM_SAFE ) )
    {
    send_to_char( "The smoke dissipates immediately.\n\r", ch );
    return;
    }

  room->smoke_timer = number_fuzzy(level/25);

  SET_BIT( room->room_flags, ROOM_SMOKE );

  act( "You conjure up an impenetrable cloud of dark purple smoke.",
     ch, NULL, NULL, TO_CHAR );
  act( "$n conjures up an impenetrable cloud of dark purple smoke.",
     ch, NULL, NULL, TO_ROOM );

  return;
}

void spell_hallucinatory_terrain( int sn, int level, CHAR_DATA *ch, void *vo )
{
  ROOM_INDEX_DATA *room;
  int dir;

  if( ch->in_room == NULL )
    return;

  if ((dir = getDirNumber(target_name)) == -1 )
   {
     send_to_combat_char( "Cast hallucinatory terrain in which direction ?\n\r", ch );
     return;
   }
  if (ch->in_room->exit[dir] == NULL ||
      ch->in_room->exit[dir]->to_room == NULL ||
      IS_SET(ch->in_room->exit[dir]->exit_info, EX_CLOSED))
   { 
     send_to_combat_char( "You can't make hallucinatory terrain in that direction.\n\r", ch);
     return;
   }

  room = ch->in_room;

  if( IS_SET( room->room_flags, ROOM_SAFE ) )
  {
    send_to_char( "You cannot cast that spell here.\n\r", ch );
    return;
  }

  room->hallucinate_timer = number_fuzzy(level/12);
  room->hallucinate_room = ch->in_room->exit[dir]->to_room;
  SET_BIT( room->room_flags, ROOM_HALLUCINATE );

  send_to_char( "The room shimmers as a veil of illusion covers it.\n\r",
     ch);
  act( "The room shimmers as $n covers it with a veil of illusion.",
     ch, NULL, NULL, TO_ROOM );

  return;
}

void spell_nightmare( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
      {
      act( "$N is still recovering from $S last fright!", ch, NULL, victim, TO_CHAR );
      return;
      }

    if( IS_EVIL(victim) && saves_spell(level/3, ch, victim))
      {
      act("$N smirks at your feeble attempt to scare $M.", ch, NULL, victim, TO_CHAR );
      return;
      }
    else
    if( saves_spell( level, ch, victim ) )
      {
      send_to_combat_char( "Nothing happens.\n\r", ch );
      return;
      }
    if(IS_NPC(victim) )
      { 
       if (!IS_SET(victim->act, ACT_WIMPY))
          SET_BIT(victim->act, ACT_WIMPY);
       af.type      = sn;
       af.duration  = number_fuzzy(level / 5);
       af.location  = APPLY_DAMROLL;
       af.modifier  = 0-(level/7);
       af.bitvector = 0;
       affect_to_char( victim, &af );
       af.location  = APPLY_HITROLL;
       af.modifier  = 0-(level/7);
       affect_to_char( victim, &af );
       af.location  = APPLY_SAVING_SPELL;
       af.modifier  = number_fuzzy(level/7);
       affect_to_char( victim, &af );
      }
    else
      {
       af.type      = sn;
       af.duration  = number_fuzzy(level/21);
       af.location  = APPLY_STR;
       af.modifier  = 0 - number_fuzzy(level/7);
       af.bitvector = 0;
       affect_to_char( victim, &af );
       af.location  = APPLY_DEX;
       af.modifier  = 0 - number_fuzzy(level/7);
       affect_to_char( victim, &af );
       af.location  = APPLY_SAVING_SPELL;
       af.modifier  = number_fuzzy(level/7);
       affect_to_char( victim, &af );
      }
   if (ch!=victim) 
   {
    ch_printf(victim, "A nightmarish apparition flies from %s's hands towards your face!\n\r", get_name(ch) );
    ch_printf(ch, "A nightmarish apparition flies from your hands towards %s's face!\n\r", get_name(victim) );
   }
  else
   send_to_combat_char("Ok.\n\r", ch);
    damage( ch, victim, number_fuzzy(number_range(20,level+10)), sn );
 return;
}

void spell_possess( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];
    AFFECT_DATA af;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      }

   if (ch->desc->original)
    {
        send_to_char("You are not in your original state.\n\r", ch);
        return;
    }

    if ( (victim = get_char_room( ch, target_name ) ) == NULL)
    {
        send_to_char("They aren't here!\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You can't possess yourself!\n\r", ch);
        return;
    }

    if (victim->position == POS_FIGHTING )
    {
        send_to_char("You can't possess someone while they're fighting!\n\r", ch);
        return;
    }
    if (!IS_NPC(victim))
    {
        send_to_char("You can't possess another player!\n\r", ch);
        return;
    }

    if (victim->desc)
    {
        ch_printf(ch, "%s is already possessed.\n\r", victim->short_descr);
        return;
    }

    if ( IS_AFFECTED(victim, AFF2_POSSESS)
    ||   IS_AFFECTED(victim, AFF_CHARM)
    ||   level < victim->level-5
    ||   victim->desc
    ||   saves_spell( level/2, ch, victim ) )
    {
        act("$N is unhappy with that you tried to seize control of $S brain.", ch, NULL, victim, TO_CHAR);
        one_hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    af.type      = sn;
    af.duration  = 20 + (ch->level - victim->level) / 2;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF2_POSSESS;
    affect_to_char( victim, &af );

    sprintf(buf, "You have possessed %s!\n\r", victim->short_descr);

log_printf("%s has possessed %s", victim->short_descr);
    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->pcdata->switched = TRUE;
    send_to_char( buf, victim );

    return;
}

void spell_transport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char arg3[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    target_name = one_argument(target_name, arg3 );

    if ( ( victim = get_player_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   victim->in_room->sector_type == SECT_ASTRAL
    ||   victim->in_room->sector_type == SECT_ETHEREAL
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
    ||   victim->level >= level + 15
    ||  (IS_NPC(victim) && saves_spell( level, ch, victim )) )
    {
        send_to_combat_char("Your transport spell failed!\n\r", ch);
        return;
    }

    if (victim->in_room == ch->in_room)
    {
        send_to_char("They are right beside you!", ch);
        return;
    }

    if ( (obj = get_obj_carry( ch, arg3 ) ) == NULL
    || ( victim->carry_weight + get_obj_weight ( obj ) ) > can_carry_w(victim))
    {
        send_to_combat_char( "Your transport spell failed!\n\r", ch);
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
    {
        send_to_char( "You can't seem to let go of it.\n\r", ch );
        return;
    }

    act( "$p slowly dematerializes...", ch, obj, NULL, TO_CHAR );
    act( "$p slowly dematerializes from $n's hands..", ch, obj, NULL, TO_ROOM );
    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$p from $n appears in your hands!", ch, obj, victim, TO_VICT);
    act( "$p appears in $n's hands!", victim, obj, NULL, TO_ROOM );
    return;
}
void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
      {
      send_to_combat_char("You are forbidden from casting that here.\n\r", ch);
      return;
      }

    if( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
     
    if ( IS_AFFECTED(victim, AFF_HASTE) )
      {
      REMOVE_BIT(victim->act, AFF_HASTE);
      }

    act( "$n slows down.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = number_fuzzy(level/9);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->speed = 0; 

    send_to_combat_char( "You slow down.\n\r", victim );
    if ( ch != victim )
	send_to_combat_char( "Ok.\n\r", ch );
    return;
}

void spell_brew_potion( int sn, int level, CHAR_DATA *ch, void *vo )
  {
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  OBJ_DATA *potion;
  int sn2,i;

    if(IS_NPC(ch) || multi(ch, sn)==-1)
      {
       send_to_combat_char( "You can't do that!\n\r", ch);
       return;
      }

  if (( ( potion = get_eq_char( ch,WEAR_HOLD) ) == NULL )||
      ( potion->item_type != ITEM_POTION ))
    {
    send_to_combat_char("You need to hold a vial for the potion you shall brew.\n\r",ch);
    return;
    }

  for(i=0;i<100;i++)
    {
    sn2=number_range(0,MAX_SKILL);
    if( skill_table[sn2].skill_level[multi(ch, sn2)]>55 ) continue;
    if( skill_table[sn2].slot == 602 || skill_table[sn2].slot == 653) continue;
    if((skill_table[sn2].slot!=0)&&(IS_NPC(ch)||(ch->pcdata->learned[sn2]!=0)))
      break;
    }
  if(i>=100)
    {
    send_to_combat_char("Your mind goes blank!\n\r",ch);
    ch->mana=0;
    return;
    }

  if( sn2>=MAX_SKILL || skill_table[sn2].name==NULL )
    {
    send_to_combat_char("The spell fails.\n\r",ch);
    return;
    }


  if(level<51)
   {
    sprintf(buf,"a murky potion of %s",skill_table[sn2].name);
    sprintf(buf2,"murky potion %s",skill_table[sn2].name);
   }
  else if(level<61)
   {
    sprintf(buf,"a cloudy potion of %s",skill_table[sn2].name);
    sprintf(buf2,"cloudy potion %s",skill_table[sn2].name);
   }
  else
   {
    sprintf(buf,"a clear potion of %s",skill_table[sn2].name);
    sprintf(buf2,"clear potion %s",skill_table[sn2].name);
   }

  STRFREE (potion->short_descr);
  potion->short_descr=STRALLOC(buf);
  STRFREE (potion->description);
  potion->description=STRALLOC(buf );

  STRFREE (potion->name);
  potion->name=STRALLOC(buf2);

  potion->cost=0;
  potion->level=ch->level *3 /4;
  SET_BIT(potion->extra_flags, ITEM_LEVEL_RENT);

    if((multi(ch,sn2)==-1 || multi(ch, sn)==-1)&& !IS_NPC(ch))
      {
       send_to_combat_char( "You can't do that!\n\r", ch);
       return;
      }

  if(!IS_NPC(ch))
    potion->value[0]=ch->mclass[multi(ch,sn2)];
  else
    potion->value[0]=ch->level/4;
  potion->value[1]=sn2;
  potion->value[2]=-1;
  potion->value[3]=-1;

  potion->basic = FALSE;
  
  act("$n brews up a potion.",
      ch,potion,NULL,TO_ROOM);
  act("You enter a trance and...\n\r...you now hold $p!",
      ch,potion,NULL,TO_CHAR);

  return;
}
void spell_elemental( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *mh;

  if( get_pets( ch) >0 )
    {
    send_to_combat_char( "You have too many pets.\n\r", ch);
    return;
    }

  if( ch->position == POS_FIGHTING )
    {
    send_to_combat_char( "You cannot cast this during combat.\n\r", ch);
    return;
    }
  switch (ch->in_room->sector_type)
   {
    case SECT_LAVA : 
 		     send_to_combat_char( "A fiery shape emerges from the flames to do your bidding.\n\r", ch );
  		     pMob = get_mob_index(MOB_VNUM_FIRE_ELEMENTAL);
 		     break;
    case SECT_WATER_SWIM : case SECT_WATER_NOSWIM : case SECT_UNDER_WATER :
    		     send_to_combat_char( "A ripple in the water forms into a man sized shape.\n\r", ch );
  		     pMob = get_mob_index(MOB_VNUM_WATER_ELEMENTAL);
		     break;
    case SECT_AIR :
    		     send_to_combat_char( "A blast of wind circles you, ready to do your bidding.\n\r", ch );
  		     pMob = get_mob_index(MOB_VNUM_AIR_ELEMENTAL);
		     break;
    case SECT_FOREST : case SECT_DESERT       : case SECT_MOUNTAIN   : 
    case SECT_HILLS  : case SECT_UNDER_GROUND : case SECT_DEEP_EARTH :
    		     send_to_combat_char( "A large mound of earth rises up out of the ground to do your bidding.\n\r", ch );
  		     pMob = get_mob_index(MOB_VNUM_EARTH_ELEMENTAL);
		     break;
    default :        send_to_combat_char( "No elementals appear to do your bidding.\n\r", ch );
                     return;
   }

  if( pMob == NULL )
    {
    send_to_char( "You cannot create an elemental now.\n\r", ch );
    return;
    }

  mh = create_mobile( pMob );
  char_to_room( mh, ch->in_room );

  mh->level=level*1/2;
  mh->hitroll = 0;
  mh->npcdata->damnodice = mh->level/2;
  mh->npcdata->damsizedice = 2;
  mh->npcdata->damplus = 7;
  mh->max_hit=30+level/4;
  mh->hit=mh->max_hit;
  mh->max_move=200;
  mh->move=200;
  mh->max_mana=0;
  mh->mana=0;
  mh->armor = 100- mh->level*5;
  mh->npcdata->armor = mh->level*5;

  SET_BIT( mh->affected_by , AFF_CHARM );
  SET_BIT( mh->act, ACT_UNDEAD);
  SET_BIT( mh->act, ACT_PET);
  add_follower( mh , ch );
  if( ch->fighting!= NULL )
    multi_hit( mh, ch->fighting->who, TYPE_UNDEFINED);
  return;
}

void spell_unbarring_ways( int sn, int level, CHAR_DATA *ch, void *vo )
{
 int door;
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;
    if (target_name[0]=='\0')
     {
      send_to_char( "What direction do you wish to cast this spell in ?\n\r", ch);
      return;
     }
  if ((door = getDirNumber(target_name)) >= 0 )
   {
 
        pexit = ch->in_room->exit[door];
        if ( pexit==NULL || !IS_SET(pexit->exit_info, EX_ISDOOR) ||
            (!IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && 
              IS_SET(pexit->exit_info, EX_HIDDEN)))
            {
	      send_to_char( "There is no door in that direction.\n\r",  ch ); 
              return; 
            }
        if ( IS_SET(pexit->exit_info, EX_MAGICPROOF) )
            {
	      send_to_char( "It remains firm.\n\r",  ch ); 
              return; 
            }
 
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        SET_BIT   (pexit->exit_info, EX_UNBARRED);

        /* pick the other side */
        if ( ( to_room   = pexit->to_room               ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->to_room == ch->in_room )
        {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED);
            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED);
            REMOVE_BIT( pexit->exit_info, EX_CLOSED);
            SET_BIT   ( pexit_rev->exit_info, EX_UNBARRED);
        }
        act( "$n causes the $d to dissolve.", ch, NULL, pexit->keyword, TO_ROOM );
        act( "You cause the $d to dissolve.", ch, NULL, pexit->keyword, TO_CHAR );
     }
     else
      send_to_combat_char("There's no valid target in that direction!\n\r", ch);


   return;
}

void spell_fire_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
      {
      send_to_combat_char( "They are already affected.\n\r", ch );
      return;
      }
   if (ch->class != CLASS_ELEMENTALIST )
      {
       send_to_combat_char("You lack the skill with the elements to control the flame!\n\r", ch);
       spell_fireball( skill_lookup("fireball"), number_range(80,120), ch, ch);
       return;
      }
    af.type      = sn;
    af.duration  = number_fuzzy(level/14);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF2_FIRESHIELD;
    affect_to_char( victim, &af );
    act( "$n bursts into flame!", victim, NULL, NULL, TO_ROOM );
    send_to_combat_char( "You burst into flame!\n\r", victim );
    return;
}

void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_STAFF
    ||   obj->item_type == ITEM_WAND)
    {
        if ( obj->value[2] == obj->value[1]
        ||   obj->value[1] > (obj->pIndexData->value[1] * 4) )
        {
            act( "$p bursts into flames, injuring you!", ch, obj, NULL,
 TO_CHAR );
            act( "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
            damage(ch, ch, obj->level * 2, TYPE_UNDEFINED);
            return;
        }

        if ( number_percent() <=2)
        {
            act( "$p glows with a blinding magical luminescence.",
                ch, obj, NULL, TO_CHAR);
            obj->value[1] *= 2;
            obj->value[2] = obj->value[1];

            return ;
        }
        else
        if ( number_percent() <=5 )
        {
            act( "$p glows brightly for a few seconds...",
                ch, obj, NULL, TO_CHAR);
            obj->value[2] = obj->value[1];
            return;
        }
        else
        if ( number_percent()<= 10 )
        {
            act( "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
            act( "$n's attempt at recharging fails, and $p disintegrates.",
                ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
            return;
        }
        else
        if ( number_percent() <= 10 + (ch->mclass[CLASS_NECROMANCER]/4) )
        {
            send_to_char("Nothing happens.\n\r", ch);
            return;
        }
        else
        {
            act( "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR);
            --obj->value[1];
            obj->value[2] = obj->value[1];
            return;
        }
    }
    else
    {
        send_to_char( "You can't recharge that!\n\r", ch);
        return ;
    }
 }

void spell_vampiric_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    if( victim==NULL )
      {
      send_to_char( "They are not here.\n\r", ch );
      return;
      }

    if (check_hit(ch, victim, 0, gsn_vampiric_touch) )
    {
     dam		=  number_fuzzy(number_range(level*3, level*5));
     ch->hit	        = UMIN(ch->max_hit, ch->hit+(2*dam/9));
     damage( ch, victim, dam, sn );
    }
    else
    {
      act( "$n's hand glows with dark energy, but fails to strike $N.", ch, NULL, victim, TO_ROOM );
      act( "The dark energies in your hand fail to strike $N.", ch, NULL, victim, TO_CHAR );
    } 
   return;
}
