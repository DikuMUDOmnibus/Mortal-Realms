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
/* predeclarations */
bool    cast_spell      args( (CHAR_DATA *ch,char *spell) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_kill         args( ( CHAR_DATA *ch, char *argument ) );
#define CHAKKOR_BASE 9700

/* this should be the base number Gahld room/obj/mob use */
#define GAHLD_BASE   6400
/* this should be the number of the last room in the area */
#define GAHLD_TOP    6499
/* define this as first level a player becomes immortal */
/* remove this define if you already have other areas by Dug installed */
#define FIRSTIMMORTALLEVEL 99

/* objects */
#define CHAIRKEY  (GAHLD_BASE+13)

/* mobiles */
#define GHOUL     (GAHLD_BASE+0)
#define LEADGHOUL (GAHLD_BASE+1)
#define BKNIGHT   (GAHLD_BASE+3)
#define WKNIGHT   (GAHLD_BASE+4)
#define COMMGHOST (GAHLD_BASE+5)
#define NOBLGHOST (GAHLD_BASE+6)
#define MUMMY     (GAHLD_BASE+7)
#define KINGGHOST (GAHLD_BASE+9)
#define QUEEGHOST (GAHLD_BASE+10)
#define WRAITH    (GAHLD_BASE+11)
#define VAMPIRE   (GAHLD_BASE+12)
#define VAMPBAT   (GAHLD_BASE+13)
#define TERMITE   (GAHLD_BASE+14)
#define MOUSE     (GAHLD_BASE+15)
#define WENCH     (GAHLD_BASE+17)
#define SPECTRE   (GAHLD_BASE+18)
#define PENNY     (GAHLD_BASE+19)
#define PENNYHEAD (GAHLD_BASE+20)
#define CASTLEGRD (GAHLD_BASE+21)
#define SLEEPNOBL (GAHLD_BASE+22)
#define OKGHOST   (GAHLD_BASE+23)
#define LICH      (GAHLD_BASE+24)
#define WIGHT     (GAHLD_BASE+25)

/*
 * The following special functions are available for mobiles.
 */

DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_order		);
DECLARE_SPEC_FUN(	spec_cast_chaos		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_clan_healer	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(       gahld_ghoul                     );
DECLARE_SPEC_FUN(       gahld_leadghoul                 );
DECLARE_SPEC_FUN(       gahld_bknight           );
DECLARE_SPEC_FUN(       gahld_wknight           );
DECLARE_SPEC_FUN(       gahld_commghost                 );
DECLARE_SPEC_FUN(       gahld_noblghost                 );
DECLARE_SPEC_FUN(       gahld_mummy                     );
DECLARE_SPEC_FUN(       gahld_kingghost                 );
DECLARE_SPEC_FUN(       gahld_queeghost                 );
DECLARE_SPEC_FUN(       gahld_wraith                    );
DECLARE_SPEC_FUN(       gahld_vampire           );
DECLARE_SPEC_FUN(       gahld_vampbat           );
DECLARE_SPEC_FUN(       gahld_termite           );
DECLARE_SPEC_FUN(       gahld_mouse                     );
DECLARE_SPEC_FUN(       gahld_wench                     );
DECLARE_SPEC_FUN(       gahld_spectre           );
DECLARE_SPEC_FUN(       gahld_penny                     );
DECLARE_SPEC_FUN(       gahld_pennyhead                 );
DECLARE_SPEC_FUN(       gahld_castlegrd                 );
DECLARE_SPEC_FUN(       gahld_okghost           );
DECLARE_SPEC_FUN(       gahld_lich                      );
DECLARE_SPEC_FUN(       gahld_wight                     );
DECLARE_SPEC_FUN(       chakkor_mercenary );

/*
 * The following special functions are available for objects.
 */
DECLARE_OBJ_FUN(	gahld_scarab   );
DECLARE_OBJ_FUN(	chakkor_seats  );
DECLARE_OBJ_FUN(	chakkor_order_statue);
DECLARE_OBJ_FUN(	reason_writ    );
DECLARE_OBJ_FUN(	reason_bench   );

struct spec_fun_type
{
  char *name;
  SPEC_FUN *function;
  };

const struct spec_fun_type spec_fun[]=
  {
   {"spec_breath_any",spec_breath_any},
   {"spec_breath_any",spec_breath_any},
   {"spec_breath_acid",spec_breath_acid},
   {"spec_breath_fire",spec_breath_fire},
   {"spec_breath_frost",spec_breath_frost},
   {"spec_breath_gas",spec_breath_gas},
   {"spec_breath_lightning",spec_breath_lightning},
   {"spec_cast_adept",spec_cast_adept},
   {"spec_cast_order",spec_cast_order},
   {"spec_cast_chaos",spec_cast_chaos},
   {"spec_cast_cleric",spec_cast_cleric},
   {"spec_cast_cleric",spec_cast_cleric},
   {"spec_cast_judge",spec_cast_judge},
   {"spec_cast_mage" ,spec_cast_mage},
   {"spec_cast_undead",spec_cast_undead},
   {"spec_clan_healer",spec_clan_healer},
   {"spec_executioner",spec_executioner},
   {"spec_fido"      ,spec_fido},
   {"spec_guard"     ,spec_guard},
   {"spec_janitor"   ,spec_janitor},
   {"spec_mayor"     ,spec_mayor},
   {"spec_poison"    ,spec_poison},
   {"spec_thief"     ,spec_thief},
   {"gahld_ghoul"    ,gahld_ghoul},
   {"gahld_leadghoul",gahld_leadghoul},
   {"gahld_bknight"  ,gahld_bknight},
   {"gahld_wknight"  ,gahld_wknight},
   {"gahld_commghost",gahld_commghost},
   {"gahld_noblghost",gahld_noblghost},
   {"gahld_mummy"    ,gahld_mummy},
   {"gahld_kingghost",gahld_kingghost},
   {"gahld_queeghost",gahld_queeghost},
   {"gahld_wraith"   ,gahld_wraith},
   {"gahld_vampire"  ,gahld_vampire},
   {"gahld_vampbat"  ,gahld_vampbat},
   {"gahld_termite"  ,gahld_termite},
   {"gahld_mouse"    ,gahld_mouse},
   {"gahld_wench"    ,gahld_wench},
   {"gahld_spectre"  ,gahld_spectre},
   {"gahld_penny"    ,gahld_penny},
   {"gahld_pennyhead",gahld_pennyhead},
   {"gahld_castlegrd",gahld_castlegrd},
   {"gahld_okghost"  ,gahld_okghost},
   {"gahld_lich"     ,gahld_lich},
   {"gahld_wight"    ,gahld_wight},
   {"",0}
  };

struct obj_fun_type
  {
  char *name;
  OBJ_FUN *function;
  };

const struct obj_fun_type obj_fun[]=
  {
   {"gahld_scarab"   ,gahld_scarab},
   {"chakkor_seats"  ,chakkor_seats},
   {"chakkor_order_statue",chakkor_order_statue},
   {"reason_writ"    ,reason_writ},
   {"reason_bench"   ,reason_bench},
   {"",0}
  };

/*
 * Given a name, return the appropriate spec fun.
 */

/* if a spell casting mob is hating someone... try and summon them */
void summon_if_hating( CHAR_DATA *ch )
{
    CHAR_DATA *victim=NULL;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    PLAYER_GAME *gpl;
    bool found = FALSE;

    if ( ch->position <= POS_SLEEPING )
        return;

    if ( ch->fighting || ch->fearing
    ||  !ch->hating || IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      return;

    /* if player is close enough to hunt... don't summon 
    if ( ch->hunting )
      return;
    */
    one_argument( ch->hating->name, name );

    /* make sure the char exists - works even if player quits */
    for (gpl = first_player;
         gpl;
         gpl = gpl->next)
    {
	victim=gpl->ch;
	/* fprintf(stderr, "Checking if %s is hated \n", victim->name); */
        if ( !strcasecmp( ch->hating->name, victim->name ) )
        {
           found = TRUE;
           break;
        }
    }

    if ( !found )
      return;
    if ( ch->in_room == victim->in_room )
      return;
    sprintf( buf, "summon %s", name );
    do_cast( ch, buf );
    return;
}

SPEC_FUN *spec_lookup( const char *name )
  {
  int i;

  for(i=0;spec_fun[i].name!="";i++)
    if(is_name(name,spec_fun[i].name))
      return spec_fun[i].function;

  return 0;
  }

/*
 * Given a spec function, return the appropriate name.
 */
char *spec_name_lookup( SPEC_FUN *fun )
  {
  int i;

  for(i=0;spec_fun[i].name!="";i++)
    if(spec_fun[i].function==fun)
      return spec_fun[i].name;

  return 0;
  }

/*
 * Given a name, return the appropriate obj fun.
 */
OBJ_FUN *obj_lookup( const char *name )
  {
  int i;

  for(i=0;obj_fun[i].name!="";i++)
    if(is_name(name,obj_fun[i].name))
      return obj_fun[i].function;

  return 0;
  }

/*
 * Given a obj function, return the appropriate name.
 */
char *obj_name_lookup( OBJ_FUN *fun )
  {
  int i;

  for(i=0;obj_fun[i].name!="";i++)
    if(obj_fun[i].function==fun)
      return obj_fun[i].name;

  return 0;
  }

/*
 * Object spec functions
 */
OBJ_FUN *obj_fun_lookup(const char *name)
  {
  /* convert strings to functions */
  if(!strcasecmp((char *)name,"gahld_scarab")) return gahld_scarab;
  if(!strcasecmp((char *)name,"chakkor_seats")) return chakkor_seats;
  if(!strcasecmp((char *)name,"chakkor_order_statue")) return chakkor_order_statue;
  if(!strcasecmp((char *)name,"reason_writ")) return reason_writ;
  if(!strcasecmp((char *)name,"reason_bench")) return reason_bench;

  return 0;
  }

/*
 * action variables for object functions
 */
int cmd_say1;
int cmd_say2;
int cmd_down;
int cmd_sit;
int cmd_stand;
int cmd_score;
int cmd_eq;
int cmd_inv;
int cmd_look;
int cmd_help;
int cmd_who;
int cmd_where;
int cmd_multi;
int cmd_tell;
int cmd_status;
int cmd_buffer;
int cmd_grep;
int cmd_refresh;
int cmd_shout;
int cmd_title;
int cmd_note;
int cmd_chat;
int cmd_plan;
int cmd_north;
int cmd_south;
int cmd_east;
int cmd_west;
int cmd_up;
int cmd_suicide;
int cmmd_kill; /* double "m" because of other decl of cmd_kill in merc.h */
int cmd_cast;
int cmd_shoot;
int cmd_rcast;
int cmd_brandish;
int cmd_zap;
int cmd_recite;
int cmd_distract;
int cmd_quaff;
int cmd_murder;
int cmd_pick;
int cmd_get;
int cmd_throw;
int cmd_recall;
int cmd_resign;
int cmd_reply;


/*
 * Assign values to action variables
 */
void obj_get_cmds()
  {
  int cmd;
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
    if(!strcasecmp(cmd_table[cmd].name,"'"         )){cmd_say1=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"say"  	)){cmd_say2=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"down"	)){cmd_down=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"sit"	)){cmd_sit=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"stand"	)){cmd_stand=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"score"	)){cmd_score=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"equipment"	)){cmd_eq=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"inventory"	)){cmd_inv=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"look"	)){cmd_look=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"help"	)){cmd_help=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"who"	)){cmd_who=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"where"	)){cmd_where=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"multi"	)){cmd_multi=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"tell"	)){cmd_tell=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"status"    )){cmd_status=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"buffer"    )){cmd_buffer=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"grep"      )){cmd_grep=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"refresh"   )){cmd_refresh=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"shout"     )){cmd_shout=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"title"     )){cmd_title=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"note"      )){cmd_note=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"chat"      )){cmd_chat=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"plan"      )){cmd_plan=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"north"     )){cmd_north=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"south"     )){cmd_south=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"east"      )){cmd_east=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"west"      )){cmd_west=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"up"        )){cmd_up=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"suicide"   )){cmd_suicide=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"kill"      )){cmmd_kill=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"cast"      )){cmd_cast=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"shoot"     )){cmd_shoot=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"rcast"     )){cmd_rcast=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"brandish"  )){cmd_brandish=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"zap"       )){cmd_zap=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"recite"    )){cmd_recite=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"distract"  )){cmd_distract=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"quaff"     )){cmd_quaff=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"murder"    )){cmd_murder=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"pick"      )){cmd_pick=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"get"       )){cmd_get=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"throw"     )){cmd_throw=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"recall"    )){cmd_recall=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"resign"    )){cmd_resign=cmd;continue;};
    if(!strcasecmp(cmd_table[cmd].name,"reply"     )){cmd_reply=cmd;continue;};
    }
  return ;
  }

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING ||(number_bits(3)!=0))
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL );
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}


bool spec_cast_healer( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( !IS_NPC(victim) &&
	     victim != ch && can_see( ch, victim ) && 
	     number_bits( 1 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
	act( "$n utters the word 'tehctah'.", ch, NULL, NULL, TO_ROOM );
	spell_armor( skill_lookup( "armor" ), ch->level, ch, victim );
	return TRUE;

    case 1:
	act( "$n utters the word 'nhak'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
	act( "$n utters the word 'yeruf'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:
	act( "$n utters the word 'soahc'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
	act( "$n utters the word 'redro'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:
	act( "$n utters the word 'nagerd'.", ch, NULL, NULL, TO_ROOM );
	spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim );
	return TRUE;

    case 6:
        act( "$n utters the word 'selkortap'.", ch, NULL, NULL, TO_ROOM);
        spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim );
        return TRUE;

    }

    return FALSE;
}

bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
	act( "$n utters the word 'tehctah'.", ch, NULL, NULL, TO_ROOM );
	spell_armor( skill_lookup( "armor" ), ch->level, ch, victim );
	return TRUE;

    case 1:
	act( "$n utters the word 'nhak'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
	act( "$n utters the word 'yeruf'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:
	act( "$n utters the word 'soahc'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
	act( "$n utters the word 'redro'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:
	act( "$n utters the word 'nagerd'.", ch, NULL, NULL, TO_ROOM );
	spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim );
	return TRUE;

    case 6:
        act( "$n utters the word 'selkortap'.", ch, NULL, NULL, TO_ROOM);
        spell_giant_strength( skill_lookup( "giant strength" ), ch->level, ch, victim );
        return TRUE;

    }

    return FALSE;
}

bool spec_cast_order( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 &&
             which_god(victim)==GOD_ORDER && victim->level!=MAX_LEVEL)
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 4 ) )
    {
    case 0:case 8: 
        act( "$n utters the word 'tehctah'.", ch, NULL, NULL, TO_ROOM );
        spell_armor( skill_lookup( "armor" ), ch->level, ch, victim ); 
        return TRUE;

    case 1:case 9:
	act( "$n utters the word 'nhak'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
	act( "$n utters the word 'yeruf'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:case 11:case 12:case 13:
	act( "$n utters the word 'redro'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
	act( "$n utters the word 'nosaer'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:case 6:
        act( "$n utters the word 'selkortap'.", ch, NULL, NULL, TO_ROOM);
        spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim );
        return TRUE;

    case 7:case 14:
        if(!get_obj_list(ch,"mushroom", ch->in_room->first_content))
          {
          act( "$n utters the word 'ymmuy'.", ch, NULL, NULL, TO_ROOM);
      /* spell_create_food(skill_lookup("create food"),ch->level,ch,victim);
          DUG, what the heck is wrong with this?   Crashed here once. */
          }
        return TRUE;

    }

    return FALSE;
}

bool spec_cast_chaos( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 &&
             which_god(victim)==GOD_CHAOS && victim->level!=MAX_LEVEL)
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 4 ) )
    {
    case 0:case 8: 
        act( "$n utters the word 'tehctah'.", ch, NULL, NULL, TO_ROOM );
        spell_armor( skill_lookup( "armor" ), ch->level, ch, victim ); 
        return TRUE;

    case 1:case 9:
	act( "$n utters the word 'nhak'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
	act( "$n utters the word 'yeruf'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:case 11:case 12:case 13:
	act( "$n utters the word 'redro'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
	act( "$n utters the word 'nosaer'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:case 6:
        act( "$n utters the word 'selkortap'.", ch, NULL, NULL, TO_ROOM);
        spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim );
        return TRUE;

    case 7:case 14:
        if(!get_obj_list(ch,"mushroom", ch->in_room->first_content))
          {
          act( "$n utters the word 'ymmuy'.", ch, NULL, NULL, TO_ROOM);
          spell_create_food(skill_lookup("create food"),ch->level,ch,victim);
          }
        return TRUE;

    }

    return FALSE;
  }


bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  6: min_level = 12; spell = "change sex";     break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8:
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "teleport";       break;
	case  4: min_level = 11; spell = "color spray";   break;
	case  5: min_level = 12; spell = "change sex";     break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7:
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	default: min_level = 24; spell = "gate";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL ||
           IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	return FALSE;

    crime = "";
    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
	    { crime = "MURDERER"; break; }
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
	    { crime = "THIEF"; break; }  
    }

    if ( victim == NULL )
	return FALSE;

    sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	victim->name, crime );
    do_shout( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
	ch->in_room );
    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
	ch->in_room );
    return TRUE;
}



bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->first_content; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->first_content; obj; obj = obj_next )
	  {
	  obj_next = obj->next_content;
	  obj_from_obj( obj );
    obj->sac_timer=OBJ_SAC_TIME;
	  obj_to_room( obj, ch->in_room );
	  }
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL  || 
          IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
          if(!IS_AFFECTED(victim,AFF_SNEAK))
            if(!IS_AFFECTED(victim,AFF_STEALTH))
              if(number_percent()<20)
	        {
                crime = "MURDERER";
                if(victim->level > (10*(int)ch->level)/6)
                  {
                  int scale, divisor, level;

                  /* scale the guard UP to 60% of the victim's level */
                  level=victim->level;
                  scale=6 * level;
  
                  level=ch->pIndexData->level;
                  divisor = 10 * level;
                  level=level*scale/divisor;
	          ch->level= (char)level;
	          ch->hit= 1+dice(ch->pIndexData->hitnodice*scale/divisor,
	              ch->pIndexData->hitsizedice*scale/divisor)+
	              ch->pIndexData->hitplus*scale/divisor;
	          ch->max_hit= ch->hit;
	          ch->npcdata->damnodice=
                      ch->pIndexData->damnodice*scale/divisor;
	          ch->npcdata->damsizedice=
                      ch->pIndexData->damsizedice*scale/divisor;
	          ch->npcdata->damplus= ch->pIndexData->damplus*scale/divisor+1;
                  }
                break;
                }
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
          if(!IS_AFFECTED(victim,AFF_SNEAK))
            if(!IS_AFFECTED(victim,AFF_STEALTH))
              if(number_percent()<20)
	        { crime = "THIEF"; break; }

	if ( victim->fighting != NULL
	&&   victim->fighting->who != ch
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL )
    {
	sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
	    victim->name, crime );
	act("$n screams '$T'",ch,NULL,buf,TO_ROOM);
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL )
      {
      if((number_percent()<75)&&(max_evil>-300)&&ech->fighting && (ech->fighting->who->alignment<300))
        {
        act("$n says 'Alright you guys, break it up!  NOW!'",
            ch,NULL,NULL,TO_ROOM);
        for(victim=ch->in_room->first_person;victim!=NULL;victim=v_next)
          {
	        v_next = victim->next_in_room;
          if(!(IS_NPC(victim)&&
              (ch->pIndexData->vnum==victim->pIndexData->vnum)))
            stop_fighting(victim,FALSE);
          }
        return TRUE;
        }
  
      act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
          ch, NULL, NULL, TO_ROOM );
      multi_hit( ch, ech, TYPE_UNDEFINED );
      return TRUE;
      }

    return FALSE;
}

bool spec_clan_healer( CHAR_DATA *ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CLAN_DATA *clan;
    char buf[16];
    int length, namelength;

    if ( !IS_AWAKE(ch) )
	return FALSE;
    if (!IS_SET(ch->act, ACT_CLAN_HEALER) || 
 	 ch->pIndexData->creator_pvnum == 0 )
        return FALSE;

    clan = get_clan_from_vnum(ch->pIndexData->creator_pvnum);

    if (clan == NULL)
        return FALSE;
    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_range( 1, clan->num_leaders))
    {
    case 1: 
        namelength = strlen(clan->leader);
	namelength--;
   	length=0;
        do
        {
 	 buf[length++]=clan->leader[namelength--];
        }
        while (namelength>-1);
	buf[length]='\0';
	buf[length-1]=LOWER(buf[length-1]);
	act( "$n utters the word '$t'.", ch, buf, NULL, TO_ROOM );
	spell_cure_critical( skill_lookup( "cure_critical" ),
	    ch->level, ch, victim );
	return TRUE;

    case 2:
	if (clan->number1[0]=='\0') 
	  return FALSE;
        namelength = strlen(clan->number1);
	namelength--;
   	length=0;
        do
        {
 	 buf[length++]=clan->number1[namelength--];
        }
        while (namelength>-1);
	buf[length]='\0';
	buf[length-1]=LOWER(buf[length-1]);
	act( "$n utters the word '$t'.", ch, buf, NULL, TO_ROOM );
	spell_cure_critical( skill_lookup( "cure_critical" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:
	if (clan->number2[0]=='\0') 
	  return FALSE;
        namelength = strlen(clan->number2);
	namelength--;
   	length=0;
        do
        {
 	 buf[length++]=clan->number2[namelength--];
        }
        while (namelength>-1);
	buf[length]='\0';
	buf[length-1]=LOWER(buf[length-1]);
	act( "$n utters the word '$t'.", ch, buf, NULL, TO_ROOM );
	spell_cure_critical( skill_lookup( "cure_critical" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
	if (clan->number3[0]=='\0') 
	  return FALSE;
        namelength = strlen(clan->number3);
	namelength--;
   	length=0;
        do
        {
 	 buf[length++]=clan->number3[namelength--];
        }
        while (namelength>-1);
	buf[length]='\0';
	buf[length-1]=LOWER(buf[length-1]);
	act( "$n utters the word '$t'.", ch, buf, NULL, TO_ROOM );
	spell_cure_critical( skill_lookup( "cure_critical" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:
	if (clan->number4[0]=='\0') 
	  return FALSE;
        namelength = strlen(clan->number4);
	namelength--;
   	length=0;
        do
        {
 	 buf[length++]=clan->number4[namelength--];
        }
        while (namelength>-1);
	buf[length]='\0';
	buf[length-1]=LOWER(buf[length-1]);
	act( "$n utters the word '$t'.", ch, buf, NULL, TO_ROOM );
	spell_cure_critical( skill_lookup( "cure_critical" ),
	    ch->level, ch, victim );
	return TRUE;
    }

    return FALSE;
}

bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->first_content; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) )
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_cleric( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0' );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n says 'I hereby declare the city of Midgaard open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n says 'I hereby declare the city of Midgaard closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
	do_unlock( ch, "gate" );
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
	do_lock( ch, "gate" );
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = who_fighting(ch) ) == NULL
    ||   number_percent( ) > 2 * ch->level ||
        victim->in_room != ch->in_room )
	return FALSE;

    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim );
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int gold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 2 ) != 0
	||   !can_see( ch, victim ) )	/* Thx Glop */
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		ch, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    gold = victim->gold * number_range( 1, 5 ) / 100;
            gold = UMIN( gold , 500 );  /* Chaos 10/5/93 */
	    ch->gold     += 7 * gold / 8;
	    victim->gold -= gold;
	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * object special functions
 */
bool gahld_scarab( OBJ_DATA *obj, int trigger, void *data, void *extra, CHAR_DATA *ch)
  {
  switch(trigger)
    {
    case OBJ_UPDATE:
      {
      if(obj->carried_by!=NULL)
        send_to_char("You shiver uncontrollably.\n\r",obj->carried_by);
      break;
      }
    case OBJ_COMMAND:
      {
      int cmd=*(int *)data;

      if(cmd==cmd_say1 /* ' */ || cmd==cmd_say2 /* say */)
        {
        send_to_char("Something will not let you speak.\n\r",obj->carried_by);
        return TRUE;
        }
      break;
      }
    }
  return FALSE;
  }

bool chakkor_seats( OBJ_DATA *obj, int trigger, void *data, void *extra, CHAR_DATA *ch )
  {
  switch(trigger)
    {
    case OBJ_UNKNOWN:
      {
      /* char *st=(char *)data; OBJ_UNKNOWN passes the unknown command -Order */

      if( ch->level>=LEVEL_IMMORTAL || which_god(ch)==GOD_POLICE || IS_NPC(ch))
        return FALSE;

      send_to_char("You can only use the commands:\n\r",ch);
      send_to_char(" say, tell, down, score, equipment, inventory, help, who, and look\n\r",ch);
      send_to_char(" while in this room.\n\r",ch);
      return TRUE;
      }
    case OBJ_UPDATE:
    default:
      {
      return FALSE;
      }
    case OBJ_COMMAND:
      {
      int cmd=*(int *)data;

      if( ch->level>=LEVEL_IMMORTAL || which_god(ch)==GOD_POLICE || IS_NPC(ch))
        return FALSE;

      if( cmd!=cmd_say1 /* ' */ && cmd!=cmd_say2 /* say */
         && cmd!=cmd_down && cmd!=cmd_score && cmd!=cmd_eq 
         && cmd!=cmd_look && cmd!=cmd_help && cmd!=cmd_who && cmd!=cmd_inv
         && cmd!=cmd_tell && cmd!=cmd_buffer && cmd!=cmd_status
         && cmd!=cmd_grep && cmd!=cmd_refresh && cmd!=cmd_stand )
        {
        send_to_char("You can only use the commands:\n\r",ch);
        send_to_char(" say, tell, down, score, equipment, inventory, help, who, and look.\n\r",ch);
        send_to_char(" while in this room.\n\r",ch);
        return TRUE;
        }
      return FALSE;
      }
    }
  return FALSE;
  }

bool reason_writ( OBJ_DATA *obj, int trigger, void *data, void *extra, CHAR_DATA *ch )
  {
  int tot;
  switch(trigger)
    {
    case OBJ_UNKNOWN:
      {
      char *st=(char *)data;
      int rvnum;

      rvnum=ch->in_room->vnum;
      if((rvnum>=27002 && rvnum<=27004) ||
         (rvnum>=27028 && rvnum<=27032) ||
         (rvnum>=27048 && rvnum<=27050))
        {/* Level 9 */
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char arg3[MAX_INPUT_LENGTH];

        /* separate the command out */
        st = one_argument( st, arg1 );
        st = one_argument( st, arg2 );
        st = one_argument( st, arg3 );
        /* look for "mix..." */
        if(is_name_short(arg1,"mix combine make"))
          {/* ok, now make sure they have all of the ingredients */
          OBJ_DATA *sulph,*charc,*saltp;
	  if((sulph=get_obj_carry(ch,"i27006")) != NULL &&
             (charc=get_obj_carry(ch,"i27007")) != NULL &&
             (saltp=get_obj_carry(ch,"i27008")) != NULL)
            {
            send_to_char("You mix all the ingredients together and produce a black powdery substance!\n\r",ch);
            extract_obj(sulph);
            extract_obj(charc);
            extract_obj(saltp);
            sulph=create_object(get_obj_index(27013),0);
            obj_to_char(sulph,ch);
            return TRUE;
            }
          else
            {
            send_to_char("You aren't holding all of the ingredients!\n\r",ch);
            return TRUE;
            }
          }
        if(rvnum==27028 && is_name_short(arg1,"fire light ignite"))
          {/* fire the cannon! */
          /* check to make sure all that is necessary is in the cannon */
          OBJ_DATA *pipe,*powder,*shot;
          CHAR_DATA *rch,*rch_next;
          ROOM_INDEX_DATA *room;


          if((pipe=get_obj_here(ch,"i27005"))!=NULL &&
             (powder=get_obj_list(ch,"i27013",pipe->first_content))!=NULL &&
             (shot=get_obj_list(ch,"i27009",pipe->first_content))!=NULL)
            {/* OK, it's all in the pipe...fire it and kill the troll */
            /* destroy all items in the pipe */
            extract_obj(powder);
            extract_obj(shot);
            /* kill the troll */
            room=get_room_index(27032);
            for ( rch = room->first_person; rch != NULL; rch = rch_next )
              {
              rch_next=rch->next_in_room;
              extract_char(rch,TRUE);
              }
            /* create the troll skin */
            pipe=create_object(get_obj_index(27014),0);
            obj_to_room(pipe,get_room_index(27032));
            /* tell them what happened */
            send_to_char("Fire appears in the pipe and sets off the black powder...KBOOOOOOM!!!\n\r",ch);
            send_to_char("You hear a loud roar from the other end of the field.\n\r",ch);
            return TRUE;
            }
          else
            {
            send_to_char("You try to light the materials in the pipe, but none of them are flamable by themselves.\n\r",ch);
            return TRUE;
            }
          }
        }
      return FALSE;
      }
    case OBJ_UPDATE:
      {
      char st[MAX_INPUT_LENGTH];

      if(obj->carried_by==NULL)
        return FALSE;

      if(obj->carried_by->in_room->vnum == 27000 ||
         obj->carried_by->in_room->vnum == 27025)
        return FALSE;
       
      if(!is_quest(obj->obj_quest))
        set_quest_bits( &obj->obj_quest, 0, 10, 30);
      tot = get_quest_bits( obj->obj_quest, 0,10);
      set_quest_bits( &obj->obj_quest, 0, 10, tot-1);
      tot = get_quest_bits( obj->obj_quest, 0,32);
      if(tot%5 || tot<=4)
        {
        sprintf(st,"You have %d minutes left to complete this quest.\n\r",
                tot-1);
        send_to_char(st,obj->carried_by);
        }
      if(tot==1)
        {
        send_to_char("You have run out of time for this quest.\n\r",obj->carried_by);
        /* increment failed quests */
        char_from_room(obj->carried_by);
        char_to_room(obj->carried_by,get_room_index(9799));
        extract_obj(obj);
        return TRUE;
        }
      return FALSE;
      }
    default:
      {
      return FALSE;
      }
    case OBJ_COMMAND:
      {
      int cmd=*(int *)data;
      char *st=(char *)extra;
      int rvnum;
      OBJ_DATA *obj2;

      if(IS_NPC(ch))
        return FALSE;
      /* make sure the room is a valid one */
      if(ch->in_room==NULL)
        return FALSE;
      rvnum=ch->in_room->vnum;
      SET_BIT(obj->extra_flags,ITEM_NODROP);
      /* can't tell/shout/title/note/chat/plan while in area */
      if( cmd==cmd_tell || cmd==cmd_shout || cmd==cmd_title ||
          cmd==cmd_note || cmd==cmd_chat  || cmd==cmd_plan  ||
          cmd==cmd_reply )
        {
	send_to_char( "They aren't here.\n\r", ch );
        return TRUE;
        }
      if(cmd==cmd_resign)
        { /* char forfeit's */
        send_to_char("You have failed this quest.\n\r",ch);
        /* increment failed quests */
        char_from_room(ch);
        char_to_room(ch,get_room_index(9799));
        extract_obj(obj);
        return TRUE;
        }
      /* object acts differently for different rooms */
      if((rvnum>=27020 && rvnum<=27023) ||
         (rvnum>=27042 && rvnum<=27045) ||
         (rvnum>=27051 && rvnum<=27054))
        {/* Level 1: The Pattern of Order */
        int num_bumps,movestatus=0;

        if( cmd==cmd_north )
          {
          if(rvnum==27053||rvnum==27045||rvnum==27043)
            {/* ran into a wall */
            movestatus=1;
            }
          else if(rvnum==27022||rvnum==27021)
            {/* moved backwards */
            movestatus=2;
            }
          else
            {
            do_east(ch,"");
            return TRUE;
            }
          }
        else if( cmd==cmd_east )
          {
          if(rvnum==27051||rvnum==27052||rvnum==27053||rvnum==27022||
             rvnum==27043)
            {/* ran into a wall */
            movestatus=1;
            }
          else if(rvnum==27044||rvnum==27023)
            {/* moved backwards */
            movestatus=2;
            }
          else
            {
            do_south(ch,"");
            return TRUE;
            }
          }
        else if( cmd==cmd_south )
          {
          if(rvnum==27044||rvnum==27051)
            {/* ran into a wall */
            movestatus=1;
            }
          else if(rvnum==27052||rvnum==27053||rvnum==27045)
            {/* moved backwards */
            movestatus=2;
            }
          else
            {
            do_west(ch,"");
            /* "fall-back" room */
            if(rvnum==27022 && !IS_SET(obj->obj_quest[0],0x1))
              {
              send_to_char("  ARGH!  You strain to move, but the resistance is greater than you had\n\ranticipated.  You make no forward progess, causing you to vow to not let that\n\rhappen again.  You did, however, catch a glimpse of the next step in the\n\rPattern.\n\r",ch);
              char_from_room(ch);
              char_to_room(ch,get_room_index(27022));
              SET_BIT(obj->obj_quest[0],0x1);
              }
            return TRUE;
            }
          }
        else if( cmd==cmd_west )
          {
          if(rvnum==27021||rvnum==27022||rvnum==27023||rvnum==27052||
             rvnum==27051||rvnum==27044)
            {/* ran into a wall */
            movestatus=1;
            }
          else if(rvnum==27043)
            {/* moved backwards */
            movestatus=2;
            }
          else
            {
            do_north(ch,"");
            return TRUE;
            }
          }
        /* handle moving into wall */
        if(movestatus==1)
          {
          int tot;
          tot = get_quest_bits( obj->obj_quest, 0, 32);
          num_bumps=1+((tot >> 1) & 0x7);
          if(num_bumps>=5)
            {
            send_to_char("You slip off of the Pattern and it consumes you!\n\r",ch);
            raw_kill(ch);
            }
          else
            {
            send_to_char("You almost stepped off of the Pattern!  That would be a very bad idea!\n\r",ch);
            tot=(get_quest_bits(obj->obj_quest,0,30) & 0x1)|(num_bumps << 1);
            set_quest_bits( &obj->obj_quest, 0,30,tot);
            }
          return TRUE;
          }
        /* handle moving backwards */
        if(movestatus==2)
          {
          send_to_char("You attempt to move backwards and the Pattern consumes you!\n\r",ch);
          raw_kill(ch);
          return TRUE;
          }
        /* end of pattern */
        if(rvnum==27042)
          {
          int tot;
          send_to_char("You have completed this quest.\n\r",ch);
          /* increment completed quests */
          tot = get_quest_bits( ch->pcdata->quest[270], 0, 31)+1;
          /* set completed 1 */
          set_quest_bits( &ch->pcdata->quest[270], 0, 31, tot+0x4000);
          char_from_room(ch);
          char_to_room(ch,get_room_index(9799));
          return TRUE;
          }
        /* do "dead" ends */
        if(rvnum==27054 || rvnum==27020)
          {
          raw_kill(ch);
          return TRUE;
          }
        }
      else if(rvnum>=27017 && rvnum<=27019)
        {/* Level 2 */
        /*** didn't need writ for level 2 ***/
        }
      else if(rvnum==27015)
        {/* Level 3 */
        char *br="\033[1m";
        char *dm="\033[m";
        char *iv="\033[7m";
        char buf[MAX_INPUT_LENGTH];


        /* only allow murder to kill Order */
        if(cmd==cmmd_kill)
          {
          send_to_char("You must Murder another player.\n\r",ch);
          return TRUE;
          }
        if(cmd==cmd_cast || cmd==cmd_shoot || cmd==cmd_rcast ||
           cmd==cmd_brandish || cmd==cmd_zap || cmd==cmd_recite ||
           cmd==cmd_distract || cmd==cmd_quaff )
          {
          send_to_char("You can't do that here.\n\r",ch);
          return TRUE;
          }
        if(cmd==cmd_murder)
          {
          do_kill(ch,"order");
          return TRUE;
          }

        /* make Order look real */
        if(cmd==cmd_who)
          {
          do_who(ch,st);
          if(ch->vt100!=1)
            send_to_char("[99 GOD ---]O<HuHaElDrDwGnOrOg> Order the God of Reason                The Test\n\r",ch);
          else
            {
            sprintf(buf,"[99 GOD ---]%sO%s<%sHuHaElDrDwGnOrOg%s> %sOrder%s the God of Reason                %sThe Test%s\n\r",br,dm,br,dm,br,dm,iv,dm);
            send_to_char(buf,ch);
            }
          return TRUE;
          }
        if(cmd==cmd_where)
          {
          do_where(ch,st);
          send_to_char(" Order            .                         \\                 localhost\n\r",ch);
          return TRUE;
          }
        if(cmd==cmd_multi)
          {
          do_multi(ch,st);
          if(ch->vt100!=1)
            send_to_char("Order       [99 GOD Dro] Ill:99  Ele:99  Rog:99  Ran:99  Nec:99  Mon:99  Asn:99\n\r",ch);
          else
            {
            sprintf(buf,"%sOrder%s       [99 GOD Dro] %sIll:99  Ele:99  Rog:99  Ran:99  Nec:99  Mon:99  Asn:99%s\n\r",br,dm,br,dm);
            send_to_char(buf,ch);
            }
          return TRUE;
          }
        }
      else if(rvnum==27016)
        {/* Level 4 */
        }
      else if((rvnum>=27012 && rvnum<=27014) ||
         (rvnum>=27039 && rvnum<=27045) ||
         (rvnum>=27058 && rvnum<=27059) ||
         (rvnum>=27066 && rvnum<=27068) ||
         (rvnum>=27074 && rvnum<=27076))
        {/* Level 5 */

        if(cmd==cmd_cast || cmd==cmd_zap || cmd==cmd_quaff || 
           cmd==cmd_brandish || cmd==cmd_recite || cmd==cmd_pick )
          {
          send_to_char("You can't do that here.\n\r",ch);
          return TRUE;
          }
        if((rvnum>=27012 && rvnum<=27014) ||
           (rvnum>=27039 && rvnum<=27041))
          {
          /* are they first_carrying the boat? */
	  if(get_obj_carry(ch,"i27012") == NULL)
            {/* they are no longer first_carrying the boat */
            send_to_char("You hold your breath as you fall into the depths!\n\r",ch);
            /* move them "below the water" */
            char_from_room(ch);
            char_to_room(ch,get_room_index(27059));
            }
          }
        else if((rvnum>=27058 && rvnum<=27059) ||
           (rvnum>=27066 && rvnum<=27068))
          {
          /* decrement movement at alarming rate */
          ch->move-=UMIN(ch->move,ch->max_move/20);
          /* watch for "get pick" */
	  if(rvnum==27066 && cmd==cmd_get && is_name(st,"pick"))
            {/* give them the pick if they don't already have it */
            if(get_obj_carry(ch,"i27010")==NULL)
              {
              send_to_char("You reach out, break the pick loose, and add it to your belongings.\n\r",ch);
              obj2=create_object(get_obj_index(27010),0);
              obj_to_char(obj2,ch);
              }
            else
              send_to_char("You get out the pick you found and look it over again.\n\r",ch);
            return TRUE;
            }
          /* watch for "get plank" */
          if(rvnum==27068 && cmd==cmd_get && is_name(st,"plank"))
            {/* give them the plank if they have the pick */
            if(get_obj_carry(ch,"i27010")!=NULL)
              {
              send_to_char("You take the pick and work away at the plank...after a few swings, the \n\r\
plank gives way and you tumble into the interior of the ship!\n\r",ch);
              char_from_room(ch);
              char_to_room(ch,get_room_index(27074));
              }
            else /* give them a failure message if they don't have the pick */
              send_to_char("You can't seem to budge the plank without any help.\n\r",ch);
            return TRUE;
            }
          }
        else if(rvnum>=27074 && rvnum<=27076)
          {
          int tot;
          /* decrement movement at alarming rate */
          ch->move-=UMIN(ch->move,45);
          /* do they have sword? */
	  if(get_obj_carry(ch,"i27011") != NULL)
            {/* yes, finish quest for them */
            send_to_char("You have completed this quest.\n\r",ch);
            /* increment completed quests */
          tot = get_quest_bits( ch->pcdata->quest[270], 0, 31)+1;
          /* set completed 1 */
          set_quest_bits( &ch->pcdata->quest[270], 0, 31, 0x40000+tot);
            char_from_room(ch);
            char_to_room(ch,get_room_index(9799));
            return TRUE;
            }
          }
        }
      else if(rvnum==27038 || rvnum==27065 || rvnum==27073 ||
         (rvnum>=27079 && rvnum<=27081) ||
         (rvnum>=27083 && rvnum<=27085))
        {/* Level 6 Tower of Despair */
          int tot;
        if(cmd==cmd_suicide)
          {
          send_to_char("You have completed this quest.\n\r",ch);
          /* increment completed quests */
          tot = get_quest_bits( ch->pcdata->quest[270], 0, 32)+1;
          /* set completed 6 */
          set_quest_bits( &ch->pcdata->quest[270], 0, 32, 0x80000+tot);
          char_from_room(ch);
          char_to_room(ch,get_room_index(9799));
          return TRUE;
          }
        }
      else if((rvnum>=27008 && rvnum<=27011) || rvnum==27057 ||
         (rvnum>=27035 && rvnum<=27037) ||
         (rvnum>=27062 && rvnum<=27064))
        {/* Level 7 */
        if(rvnum==27057)
          {
          int tot;
          send_to_char("The object of this quest was to present you with many 'easy kills' to see if you would attack them.  Killing any of my animals would have resulted in your immediate failure.\n\r     -Order\n\r",ch);
          send_to_char("You have completed this quest.\n\r",ch);
          /* increment completed quests */
          tot = get_quest_bits( ch->pcdata->quest[270], 0, 32)+1;
          set_quest_bits( &ch->pcdata->quest[270], 0, 32, 0x100000+tot);
          /* set completed 7 */
          char_from_room(ch);
          char_to_room(ch,get_room_index(9799));
          return TRUE;
          }
        }
      else if((rvnum>=27005 && rvnum<=27007) ||
         (rvnum>=27033 && rvnum<=27034))
        {/* Level 8 */
        }
      else if((rvnum>=27002 && rvnum<=27004) ||
         (rvnum>=27028 && rvnum<=27032) ||
         (rvnum>=27048 && rvnum<=27050))
        {/* Level 9 */
        if(cmd==cmd_shoot || cmd==cmd_throw || cmd==cmd_rcast )
          {
          send_to_char("You can't do that here.\n\r",ch);
          return TRUE;
          }
	if(get_obj_carry(ch,"i27014") != NULL)
          {/* yes, finish quest for them */
          int tot;
          send_to_char("You have completed this quest.\n\r",ch);
          /* increment completed quests */
          tot = get_quest_bits( ch->pcdata->quest[270], 0, 32)+1;
          set_quest_bits( &ch->pcdata->quest[270], 0, 32, 0x400000+tot);
          /* set completed 9 */
          char_from_room(ch);
          char_to_room(ch,get_room_index(9799));
          return TRUE;
          }
        }
      else if(rvnum==27024 || rvnum==27001 ||
         (rvnum>=27026 && rvnum<=27027) ||
         (rvnum>=27046 && rvnum<=27047) ||
         (rvnum>=27055 && rvnum<=27056) ||
         (rvnum>=27060 && rvnum<=27061) ||
         (rvnum>=27069 && rvnum<=27072) ||
         (rvnum>=27077 && rvnum<=27078) || rvnum==27082)
        {/* Level 10 */
        }
      else if(rvnum!=27025)
        {/* in a room that the object should not be in so delete it */
        extract_obj(obj);
        }
      return FALSE;
      }
    }
  return FALSE;
  }

bool reason_bench( OBJ_DATA *obj, int trigger, void *data, void *extra, CHAR_DATA *ch )
  {
  if(trigger==OBJ_COMMAND)
    {
    int cmd=*(int *)data;
    /* char *st=(char *)extra; */

    if( cmd==cmd_recall )
      {
      char_from_room(ch);
      char_to_room(ch,get_room_index(9799));
      send_to_char("Wow!  That never made your head spin that much before!\n\r",
                   ch);
      }
    }
  if(trigger==OBJ_UNKNOWN)
    {
    char *st=(char *)data;
    if(strcmp(st,"bow"))
      return FALSE;
    char_from_room(ch);
    /* put char in room with puzzle master */
    char_to_room(ch,get_room_index(27025));
    send_to_char("Wow!  That never made your head spin that much before!\n\r",
                 ch);
    return TRUE;
    }
  return FALSE;
  }

bool chakkor_order_statue(OBJ_DATA *obj,int trigger,void *data,void *extra, CHAR_DATA *ch)
  {
  if(trigger==OBJ_UNKNOWN)
    {
    char *st=(char *)data;
    if(strcmp(st,"pray"))
      return FALSE;
    act("$n disappears in a flash of blinding light!",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch);
    /* put char in room with the bench */
    char_to_room(ch,get_room_index(27000));
    send_to_char("The statue of Order appears to come to life and lead you through multiple\n\r  dimensions of sight and sound...  You wake up feeling very disoriented.", ch);
    return TRUE;
    }
  return FALSE;
  }

bool gahld_ghoul(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(ch->position==POS_RESTING)
    {
    ch->position=POS_STANDING;
    do_follow(ch,"leader");
    return 0;
    }
  else if(ch->position==POS_STANDING)
    for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
      {
      if(tch->alignment>349)
        {
        act("A lizardy voice says 'I love to kill good things!'"
          ,ch, NULL, NULL, TO_ROOM );
        if(ch->fighting!=NULL)
          stop_fighting(ch,FALSE);
        set_fighting(ch,tch);
        return 0;
        }
      else if((number_percent()>98)&&(tch->level>4)&&!IS_NPC(tch))
        {
        act(
"A lizardy voice says 'I prefer killing good things...but, a corpse IS a corpse!
'"
         ,ch,NULL,NULL,TO_ROOM);
        do_kill(ch,tch->name);
        return 0;
        }
      }
  return 0;
  }

bool gahld_leadghoul(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(ch->position==POS_STANDING)
    for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
      {
      if(tch->alignment>349)
        {
        act("A lizardy voice says 'I love to kill good things!'"
         ,ch,NULL,NULL,TO_ROOM);
        if(ch->fighting!=NULL)
          stop_fighting(ch,FALSE);
        set_fighting(ch,tch);
        return 0;
        }
      else if((number_percent()>99)&&(tch->level>4)&&!IS_NPC(tch))
        {
        act(
"A lizardy voice says 'I prefer killing good things...but, a corpse IS a corpse!
'"
         ,ch,NULL,NULL,TO_ROOM);
        if(ch->fighting==NULL)
          set_fighting(ch,tch);
        return 0;
        }
      }
  return 0;
  }

void npc_do_emote(CHAR_DATA *ch,char *arg)
  {
  char buf[255];

  sprintf(buf,"%s %s",ch->short_descr,arg);
  act(buf,ch,0,0,TO_ROOM);
  }

int do_ghost_thing(CHAR_DATA *ch,CHAR_DATA *tch,char *str)
  {
  npc_do_emote(ch,str);
  interpret(tch,"scream");
  if(number_percent()>99)
    do_flee(tch,"");
  return 0;
  }

bool gahld_bknight(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    {
    if((!IS_NPC(tch))&&(number_percent()>80))
      do_ghost_thing(ch,tch,
        "momentarily removes his visor, revealing nothing!");
    }
  if(ch->in_room->vnum==(GAHLD_BASE+8))
    {
    if(number_percent()>50)
      {
      npc_do_emote(ch,
        "rears his steed and takes off in a thunder of sound");
      act("towards his destiny in the South.",ch,NULL,NULL,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,get_room_index(GAHLD_BASE+9));
      npc_do_emote(ch,"arrives in a cavalcade of thunderous sound.");
      }
    }
  else if(ch->in_room==get_room_index(GAHLD_BASE+9))
    {
    if(ch->position==POS_FIGHTING)
      {
      if(number_percent()>75)
        return cast_spell(ch,"chill touch");
      }
    else
      do_kill(ch,"white");
    if(number_percent()>90)
      {
      npc_do_emote(ch,"heads toward the king's stand in the East.");
      char_from_room(ch);
      char_to_room(ch,get_room_index(GAHLD_BASE+12));
      npc_do_emote(ch,"arrives suddenly.");
      return 0;
      }
    }
  else
    {
    if(ch->in_room==get_room_index(GAHLD_BASE+12))
      npc_do_emote(ch,"bows to the non-existant king.");
    char_from_room(ch);
    ch->mana = ch->max_mana;
    ch->hit = ch->max_hit;
    ch->move = ch->max_move;
    char_to_room(ch,get_room_index(GAHLD_BASE+8));
    update_pos(ch);
    }
  return 0;
  }

bool gahld_wknight(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    {
    if((number_percent()>80)&&(!IS_NPC(tch)))
      do_ghost_thing(ch,tch,
        "momentarily removes his visor, revealing maggots!");
    }
  if(ch->in_room==get_room_index(GAHLD_BASE+10))
    {
    if(number_percent()>50)
      {
      npc_do_emote(ch,
        "rears his steed and takes off in a thunder of sound");
      act("towards his destiny in the North.",ch,NULL,NULL,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,get_room_index(GAHLD_BASE+9));
      npc_do_emote(ch,"arrives in a cavalcade of thunderous sound.");
      }
    }
  else if(ch->in_room==get_room_index(GAHLD_BASE+9))
    {
    if(ch->position==POS_FIGHTING)
      {
      if(number_percent()>90)
        return cast_spell(ch,"chill touch");
      }
    else
      do_kill(ch,"black");
    if(number_percent()>99)
      {
      npc_do_emote(ch,"heads toward the king's stand in the East.");
      char_from_room(ch);
      char_to_room(ch,get_room_index(GAHLD_BASE+12));
      return 0;
      }
    }
  else if(ch->in_room==get_room_index(GAHLD_BASE+12))
    {
    if(ch->in_room==get_room_index(GAHLD_BASE+12))
      npc_do_emote(ch,"bows to the non-existant king.");
    char_from_room(ch);
    ch->mana = ch->max_mana;
    ch->hit = ch->max_hit;
    ch->move = ch->max_move;
    char_to_room(ch,get_room_index(GAHLD_BASE+10));
    update_pos(ch);
    }
  return 0;
  }

bool gahld_commghost(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      do_ghost_thing(ch,tch,
        "reveals his face in its entirety!");
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>90)
      return cast_spell(ch,"chill touch");
    }
  return 0;
  }

bool gahld_noblghost(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      do_ghost_thing(ch,tch,
        "reveals its true appearance!");
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>90)
      return cast_spell(ch,"chill touch");
    }
  return 0;
  }

bool gahld_mummy(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  do_close(ch,"door");
  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      do_ghost_thing(ch,tch,
        "suddenly strikes you as horribly terrifying!");
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>90)
      return cast_spell(ch,"poison");
    }
  return 0;
  }

bool gahld_kingghost(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;
  OBJ_DATA *tobj;

  if((tobj = get_obj_list(ch,"cherish", ch->first_carrying)))
    {
    STRFREE (tobj->name);
    tobj->name=STRALLOC("coin unique almostcherish");
    do_say(ch,"Thanks!  I'll cherish it forever!");
    interpret(ch,"wear coin");
    do_say(ch,"And here's a small token of my gratitude.");
    interpret(ch,"remove sceptre");
    interpret(ch,"drop sceptre");
    }
  if((tobj = get_obj_list(ch,"wifes", ch->first_carrying)))
    {
    STRFREE (tobj->name);
    tobj->name=STRALLOC("scarab ruby");
    do_say(ch,"At last!  A token of love from my long-dead wife!");
    do_say(ch,"Here is my beloved coin in exchange for this wonderous gift!");
    interpret(ch,"remove coin");
    if((tobj = get_obj_list(ch,"almostcherish", ch->first_carrying)))
      {
      AFFECT_DATA *paf;
      CREATE(paf, AFFECT_DATA, 1);
      paf->type         = 0;
      paf->duration     = -1;
      paf->location     = APPLY_DAMROLL;
      paf->modifier     = 5;
      paf->bitvector    = 0;
      LINK (paf, tobj->first_affect, tobj->last_affect, next, prev );

      STRFREE (tobj->name);
      tobj->name=STRALLOC("coin unique verycherished");
      interpret(ch,"drop coin");
      }
    }
  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      {
      do_ghost_thing(ch,tch,"reveals his true appearance!");
      npc_do_emote(ch,"recaptures his regality.");
      }
  if(ch->position==POS_FIGHTING)
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
  if(ch->position!=POS_FIGHTING)
    {
    if(number_percent()>95)
      {
      do_say(ch,"I do believe I've lost my precious coin.");
      do_say(ch,"If you see it, would you please return it to me?");
      interpret(ch,"sigh");
      }
    else if(number_percent()>95)
      interpret(ch,"hug queen");
    else if(number_percent()>95)
      interpret(ch,"kiss queen");
    else if(number_percent()>95)
      {
      interpret(ch,"cry");
      do_say(ch,"I wish my wife loved me...");
      interpret(ch,"sniff");
      }
    }
  return 0;
  }

bool gahld_queeghost(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;
  OBJ_DATA *tobj;

  if((tobj = get_obj_list(ch,"ruling", ch->first_carrying)))
    {
    STRFREE (tobj->name);
    tobj->name=STRALLOC("sceptre golden");
    do_say(ch,"Thanks!  Now I'll rule the kingdom!");
    do_say(ch,"And here's you a worthless trinket from my 'beloved.'");
    interpret(ch,"remove scarab");
    interpret(ch,"drop scarab");
    }
  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      {
      do_ghost_thing(ch,tch,"reveals her true appearance!");
      npc_do_emote(ch,"recaptures her beauty.");
      }
  if(ch->position==POS_FIGHTING)
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
  if(ch->position!=POS_FIGHTING)
    {
    if(number_percent()>95)
      {
      do_say(ch,"Has anyone seen my Scarab?");
      do_say(ch,"I seem to have misplaced it.");
      interpret(ch,"sigh");
      }
    else if(number_percent()>95)
      npc_do_emote(ch,"fusses with her hair.");
    else if(number_percent()>95)
      {
      interpret(ch,"cry");
      do_say(ch,"There is no powder-room in this castle.");
      do_say(ch,"And my nose is shiny!");
      interpret(ch,"pout");
      }
    else if(number_percent()>75)
      interpret(ch,"slap king");
    }
  return 0;
  }

bool gahld_wraith(CHAR_DATA *ch)
  {
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
    if(number_percent()>90)
      return cast_spell(ch,"energy drain");
    }
  return 0;
  }

bool gahld_vampire(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  do_close(ch,"crypt");
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>90)
      return cast_spell(ch,"chill touch");
    if(number_percent()>95)
      {
      npc_do_emote(ch,"makes some strange gestures and creates a bat!");
      tch=create_mobile(get_mob_index(GAHLD_BASE+13));
      char_to_room(tch,ch->in_room);
      return 0;
      }
    }
  else
    for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
      if((tch->position!=POS_FIGHTING)&&(number_percent()>90))
        return cast_spell(ch,"charm person");
  return 0;
  }

bool gahld_vampbat(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(number_percent()>80)
    npc_do_emote(ch,"flaps its wings noisily.");
  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((!IS_NPC(tch))&&(number_percent()>15))
      {
      do_kill(ch,tch->name);
      if(ch->fighting!=NULL)
        stop_fighting(ch,FALSE);
      return 0;
      }
  return 0;
  }

bool gahld_termite(CHAR_DATA *ch)
  {
  if(number_percent()>95)
    npc_do_emote(ch,"gnaws on a piece of wood.");
  else if(number_percent()>95)
    npc_do_emote(ch,
      "thinks flesh is a sorry substitute for good, solid wood.");
  return 0;
  }

bool gahld_mouse(CHAR_DATA *ch)
  {
  if(number_percent()>40)
    npc_do_emote(ch,"squeaks noisily.");
  else if(number_percent()>40)
    npc_do_emote(ch,"runs around under foot.");
  if(ch->in_room==get_room_index(GAHLD_BASE+34))
    move_char(ch,DIR_WEST);
  else if(ch->in_room==get_room_index(GAHLD_BASE+31))
    move_char(ch,DIR_EAST);
  return 0;
  }

bool gahld_wench(CHAR_DATA *ch)
  {
  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
    if(number_percent()>90)
      return cast_spell(ch,"energy drain");
    if(number_percent()>75)
      {
      do_say(ch,"Now, look what YOU are doing!");
      do_say(ch,"Messing up my perfectly clean floor!");
      act("You are slapped by a serving Wench!\n\r",ch,NULL,NULL,TO_ROOM);
      }
    }
  return 0;
  }

bool gahld_spectre(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
    if(number_percent()>90)
      return cast_spell(ch,"energy drain");
    }
  if(number_percent()>98)
    for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
      if((!IS_NPC(tch))&&(tch->position==POS_RESTING))
        {
        act(
"The Ancient King says 'I have taken to haunting this castle because my own
  blood employed the help of a deceitful magician.  The magician became a
  Lich and drained all of the life from my great grandson and his subjects.
  I wish to be avenged, but all I have been able to do is haunt this room.
  Something will not let me venture forth.'
"
          ,ch,NULL,NULL,TO_ROOM);
        return 0;
        }
  return 0;
  }

bool gahld_penny(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(ch->position==POS_FIGHTING)
    {
    if(number_percent()>95)
      {
      act("The head of one of the Penanggalan's pops off!"
        ,ch,NULL,NULL,TO_ROOM);
      tch=create_mobile(get_mob_index(GAHLD_BASE+20));
      char_to_room(tch,ch->in_room);
      }
    if(number_percent()>95)
      interpret(ch,"grin");
    }
  return 0;
  }

bool gahld_pennyhead(CHAR_DATA *ch)
  {
  if(number_percent()>80)
      interpret(ch,"cackle");
  return 0;
  }

bool gahld_castlegrd(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
    if((number_percent()>80)&&!IS_NPC(tch))
      do_ghost_thing(ch,tch,
        "momentarily removes his helmet!");
  if(ch->position==POS_FIGHTING)
    if(number_percent()>75)
      return cast_spell(ch,"chill touch");
  return 0;
  }

bool gahld_okghost(CHAR_DATA *ch)
  {
  OBJ_DATA *temp;

  if(ch->in_room==get_room_index(GAHLD_BASE+64)&&(ch->gold>500))
    {
    do_say(ch,"Thanks!");
    do_say(ch,"Here's a little something for your kindness.");
    do_say(ch,"But, use it quickly!");

    temp = create_object(get_obj_index(GAHLD_BASE+13),1);
    temp->timer=5;/* decays in 5 ticks */
    obj_to_room(temp,ch->in_room);

    interpret(ch,"wave");
    char_from_room(ch);
    char_to_room(ch,get_room_index(GAHLD_BASE+99));
    }
  return 0;
  }

bool gahld_lich(CHAR_DATA *ch)
  {
  CHAR_DATA *tch,*tch_next;
  ROOM_INDEX_DATA *rm;
  char buf[100];

  rm=get_room_index(GAHLD_BASE+00);
  for(tch=rm->first_person;tch!=NULL;tch=tch_next)
    {
    tch_next=tch->next_in_room;
    if((!IS_NPC(tch))&&(tch->position==POS_SLEEPING)&&(get_obj_list(tch,"firecha
in",tch->first_carrying)!=NULL))
      {
      char_from_room(tch);
      char_to_room(tch,get_room_index(GAHLD_BASE+96));
      send_to_char("You have been transported!\n\r",tch);
      }
    }
  for(tch=ch->in_room->first_person;tch!=NULL;tch=tch->next_in_room)
   if((!IS_NPC(tch))&&(tch->level>(ch->level)||(get_obj_list(tch,"band",tch->first_carrying)!=NULL)))
      {
      if(tch->fighting!=NULL)
        {
        sprintf(buf,"I will not allow %s to fight here!",tch->name);
        do_say(ch,buf);
        stop_fighting(tch,FALSE);
        do_recall(tch,"reset");
        do_recall(tch,"");
        ch->hit=ch->max_hit;
        }
      if(tch->level>=FIRSTIMMORTALLEVEL)
        {
        if(number_percent()>75)
          {
          sprintf(buf,"bow %s",tch->name);
          interpret(ch,buf);
          }
        }
      return 0;
      }
  switch(number_range(1,20))
    {
    case 1:;case 2:; case 3:
      return cast_spell(ch,"chill touch");
    case 4:;case 5:
      return cast_spell(ch,"lightning bolt");
    case 6:
      return cast_spell(ch,"blindness");
    case 7:;case 8:;case 9:
      return cast_spell(ch,"color spray");
    case 10:
      return cast_spell(ch,"curse");
    case 11:
      act(
"The Lich says 'Once upon a time, there was a great kingdom with many subjects.
  One day, I decided to obliterate the pesky mortals.  I did.'
",ch,NULL,NULL,TO_ROOM);
      break;
    case 12:
      do_say(ch,"Would you like to see my collection of skulls?");
    case 13:
      {
      do_say(ch,"Who are you?  I thought I'd killed everyone!");
      do_say(ch,"Nevermind, you'll be dead soon enough.");
      }
    }
  return 0;
  }

bool gahld_wight(CHAR_DATA *ch)
  {
  if(number_percent()>50)
    {
    do_close(ch,"door");
    if(ch->position==POS_FIGHTING)
      return cast_spell(ch,"chill touch");
    }
  return 0;
  }

bool cast_spell( CHAR_DATA *ch,char *spell)
  {
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int sn;

  for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
    {
    v_next = victim->next_in_room;
    if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
      break;
    }

  if ( victim == NULL )
    return FALSE;

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
  return TRUE;
  }

