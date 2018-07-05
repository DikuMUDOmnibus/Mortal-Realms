/**************************************************************************
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

extern bool can_see_is_hidden;

char *  const   where_name      [] =
{
    "   <Used as Light> ",
    "<Worn on L Finger> ",
    "<Worn on R Finger> ",
    "<Worn around Neck> ",
    "<Worn around Neck> ",
    "    <Worn on Body> ",
    "    <Worn on Head> ",
    "    <Worn on Legs> ",
    "    <Worn on Feet> ",
    "   <Worn on Hands> ",
    "    <Worn on Arms> ",
    "          <Shield> ",
    " <Worn about Body> ",
    "<Worn about Waist> ",
    " <Worn on L Wrist> ",
    " <Worn on R Wrist> ",
    "         <Wielded> ",
    "            <Held> ",
    "           <Heart> ",
    "    <Dual Wielded> "
};


/*
 * Global functions.
 */

/*
 * Local functions.
 */
char *  format_obj_to_char      args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    int fShort ) );
void    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    int fShort, bool fShowNothing ) );
void    show_char_to_char_0     args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool    check_blind             args( ( CHAR_DATA *ch ) );
void    get_affects_string    ( CHAR_DATA *, CHAR_DATA *, char *, int );

int lookup_race( char *arg )
  {
  int cnt;

  for( cnt=0; cnt<MAX_RACE; cnt++)
    if( !str_prefix( arg, race_table[cnt].race_name) )
      return( cnt );

  return( -1 );
  }


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, int fShort )
{
    static char buf[MAX_STRING_LENGTH];
    int blen;

    blen = 0;
    buf[0] = '\0';
/*
  if( TEST_GAME && obj->obj_quest>0)
    {
    char buf2[32];
    sprintf( buf2, "[%8X]", (unsigned int)obj->obj_quest);
    blen = str_apd_max( buf, buf2, blen, MAX_STRING_LENGTH );
    }
*/

#ifdef EQUIPMENT_DAMAGE
   if( obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_WEAPON )
    {
    if( ch->level > 50 )
      {
      char buf2[32];
      sprintf( buf2, "%2d%%  ", obj->condition);
      blen = str_apd_max( buf, buf2, blen, MAX_STRING_LENGTH );
      }
    else if( obj->condition >=90 )
      blen = str_apd_max( buf, "[E] ", blen, MAX_STRING_LENGTH );
    else if( obj->condition >=70 )
      blen = str_apd_max( buf, "[G] ", blen, MAX_STRING_LENGTH );
    else if( obj->condition >=50 )
      blen = str_apd_max( buf, "[F] ", blen, MAX_STRING_LENGTH );
    else if( obj->condition >=30 )
      blen = str_apd_max( buf, "[P] ", blen, MAX_STRING_LENGTH );
    else if( obj->condition >=10 )
      blen = str_apd_max( buf, "[B] ", blen, MAX_STRING_LENGTH );
    else
      blen = str_apd_max( buf, "[D] ", blen, MAX_STRING_LENGTH );
    }
  else
    {
    if( ch->level > 50 )
      blen = str_apd_max( buf, "     ", blen, MAX_STRING_LENGTH );
    else
      blen = str_apd_max( buf, "    ", blen, MAX_STRING_LENGTH );
    }
#endif
     
    if ( fShort==0 )
    {
	if ( obj->short_descr != NULL )
          blen = str_apd_max( buf, capitalize(obj->short_descr), 
                       blen, MAX_STRING_LENGTH );
    }
    else
    if ( fShort==1 )
    {
	if ( obj->description != NULL )
          blen = str_apd_max( buf, obj->description,
                  blen, MAX_STRING_LENGTH );
    }
    else
    if ( fShort==2 )
    {
	if ( obj->long_descr != NULL )
          blen = str_apd_max( buf, obj->long_descr, blen, MAX_STRING_LENGTH );
    }

if( ( IS_OBJ_STAT(obj, ITEM_INVIS)     ) 
    || ( IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL)   )  
    || ( IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC)  ) 
    || ( IS_OBJ_STAT(obj, ITEM_GLOW)      )
    || ( IS_OBJ_STAT(obj, ITEM_HUM)       ) 
    || ( IS_OBJ_STAT(obj, ITEM_FORGERY))  
    || ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE))  
    || obj->poison!=NULL )
  {
  if( !IS_NPC(ch ) && ch->ansi == 1 )
    blen = str_apd_max( buf, get_color_string(ch,COLOR_TEXT,VT102_BOLD),
                    blen, MAX_STRING_LENGTH );

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     ) 
        blen = str_apd_max( buf, " (Invis)" , blen, MAX_STRING_LENGTH    );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL)   )  
        blen = str_apd_max( buf, " (Red Aura)" , blen, MAX_STRING_LENGTH    );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC)  ) 
        blen = str_apd_max( buf, " (Magical)" , blen, MAX_STRING_LENGTH    );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )
        blen = str_apd_max( buf, " (Glowing)" , blen, MAX_STRING_LENGTH    );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       ) 
        blen = str_apd_max( buf, " (Humming)" , blen, MAX_STRING_LENGTH    );
    if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE)       ) 
        blen = str_apd_max( buf, " (PROTO)" , blen, MAX_STRING_LENGTH    );
    if ((IS_NPC(ch)||number_percent()<ch->pcdata->learned[gsn_detect_forgery])
	 && IS_OBJ_STAT(obj, ITEM_FORGERY))
        blen = str_apd_max( buf, " (Forgery)" , blen, MAX_STRING_LENGTH    );
    if (!IS_NPC(ch) && obj->poison!=NULL && 
        obj->poison->owner==ch->pcdata->pvnum )
      {
      char buf2[100];
      if( obj->poison->for_npc )
        sprintf( buf2, " (Poisoned M%c)", obj->poison->poison_type );
      else
        sprintf( buf2, " (Poisoned C%c)", obj->poison->poison_type );
      blen = str_apd_max( buf, buf2, blen, MAX_STRING_LENGTH);
      }
  }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, int fShort,
       bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH], buf3[10];
    int leng, lenb;
    char **prgpstrShow, dim[10], bold[10];
    int *prgnShow;
    int *refShow;
    char *pstrShow;
    OBJ_DATA *obj, *tobj;
    int nShow, iShow;
    int count;
    bool tPrev;

    if ( ch==NULL )
	    return;
    if ( !ch->desc )
        return;

    /*
     * if there's no list... Let's get outta here!  -Martin
     */
    if ( !list )
    {
        if ( fShowNothing )
           send_to_char( "Nothing.\n\r", ch );
        return;
    }


    if(!IS_NPC(ch) && ch->ansi==0 && ch->vt100==1)
      {
      strcpy(dim,"\033[m");
      strcpy(bold,"\033[1m");
      }
    else if(!IS_NPC(ch) && ch->ansi==1)
      {
      sprintf(dim,"\033[0;%d;%dm", ch->pcdata->color[0][10], 
	  ch->pcdata->color[1][10]);
      sprintf(bold,"\033[1;%d;%dm", ch->pcdata->color[0][10], 
	  ch->pcdata->color[1][10]);
      }
    else
      {
      strcpy(dim,"");
      strcpy(bold,"");
      }
    leng = str_cpy_max(buf2, dim, MAX_STRING_LENGTH);

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;

    CREATE( prgpstrShow, char *, count );
    CREATE( prgnShow, int, count );
    CREATE( refShow, int, count );
    nShow       = 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	  {

          /* Look for previous matches - Chaos 4/17/99 */
          tPrev = FALSE;
	  if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ||
             (obj->basic && obj->item_type!=ITEM_ARMOR &&
             obj->item_type!=ITEM_CONTAINER && obj->item_type!=ITEM_WEAPON ))
          for ( tobj = list; tobj != obj; tobj = tobj->next_content )
	    if ( tobj->wear_loc == WEAR_NONE && can_see_obj( ch, tobj ) )
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ||
             (tobj->basic && tobj->item_type!=ITEM_ARMOR &&
             tobj->item_type!=ITEM_CONTAINER && tobj->item_type!=ITEM_WEAPON ))
              if(tobj->pIndexData->vnum==obj->pIndexData->vnum &&
                 tobj->short_descr == obj->short_descr )
                {
                tPrev = TRUE;
                break;
                }
           if(tPrev)
             continue;

	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    prgnShow[nShow]=1;
	    refShow[nShow]=obj->index_reference[0]%1000;
            if( (obj->basic && obj->item_type!=ITEM_ARMOR &&
                obj->item_type!=ITEM_CONTAINER && obj->item_type!=ITEM_WEAPON)
                || !CAN_WEAR(obj, ITEM_TAKE) )
              refShow[nShow]=-1;
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
              refShow[nShow]=-1;

	    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    {
            /* Look for further matches - Chaos 4/17/99 */
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ||
             (obj->basic && obj->item_type!=ITEM_ARMOR &&
              obj->item_type!=ITEM_CONTAINER && obj->item_type!=ITEM_WEAPON ))
            for ( tobj=obj->next_content; tobj!=NULL; tobj=tobj->next_content)
	     if ( tobj->wear_loc == WEAR_NONE && can_see_obj( ch, tobj ) )
    if ( IS_NPC(ch) || IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ||
             (tobj->basic && tobj->item_type!=ITEM_ARMOR &&
             tobj->item_type!=ITEM_CONTAINER && tobj->item_type!=ITEM_WEAPON ))
              if(tobj->pIndexData->vnum==obj->pIndexData->vnum &&
                 tobj->short_descr == obj->short_descr )
                {
		prgnShow[nShow]++;
                }

		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 * This code is really shitty  -  Chaos 4/17/99
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}  */
	    }

		prgpstrShow [nShow] = STRALLOC( pstrShow );
		nShow++;
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
      {
      lenb=0;
      if (( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) && prgnShow[iShow]!=1 )
          {
          sprintf( buf, "%s(%2d) ", dim, prgnShow[iShow] );
          lenb = strlen( buf );
          }
      else
       if( refShow[iShow]>0 )
        {
        lenb = str_cpy_max( buf, dim, MAX_STRING_LENGTH);
	lenb = str_apd_max( buf, "#", lenb, MAX_STRING_LENGTH);
        *buf3 = '0'+(refShow[iShow]/100)%10;
        *(buf3+1) = '0'+(refShow[iShow]/10)%10;
        *(buf3+2) = '0'+(refShow[iShow])%10;
        *(buf3+3) = '\0';
	lenb = str_apd_max( buf, buf3, lenb, MAX_STRING_LENGTH);
	lenb = str_apd_max( buf, " ", lenb, MAX_STRING_LENGTH);
        }
      else
	lenb = str_cpy_max( buf, "     ", MAX_STRING_LENGTH);
      lenb = str_apd_max( buf, bold, lenb, MAX_STRING_LENGTH);
      lenb = str_apd_max( buf, prgpstrShow[iShow], lenb, MAX_STRING_LENGTH );
      lenb = str_apd_max( buf, dim, lenb, MAX_STRING_LENGTH);
      lenb = str_apd_max( buf, "\n\r", lenb, MAX_STRING_LENGTH);
      leng = str_apd_max ( buf2, buf, leng, MAX_STRING_LENGTH );
      STRFREE (prgpstrShow[iShow] );
      }

    if ( fShowNothing && nShow == 0 )
      send_to_char( "Nothing.\n\r", ch );
    if ( nShow != 0 )
      send_to_char( buf2, ch );

    /*
     * Clean up.
     */
    DISPOSE( prgpstrShow );
    DISPOSE( prgnShow );
    DISPOSE( refShow );
    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int leng;

  if( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_ETHEREAL ) && 
      (( !IS_NPC(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT) ) || IS_NPC(ch) ) )
    return;

    if( !IS_NPC(ch) )
        leng = str_cpy_max( buf,  get_color_string(ch,COLOR_MOBILES,VT102_DIM),
                                        MAX_STRING_LENGTH );
    else
      {
      *buf = '\0';
      leng = 0;
      }
   if(  !IS_NPC( victim )
        && victim->long_descr!=NULL
 	&& strlen(victim->long_descr)>0
	&& victim->position == POS_STANDING 
        && !is_affected(ch, gsn_truesight) )

       leng = str_cpy_max( buf, victim->long_descr, MAX_STRING_LENGTH );
   else
   {
    if ( !IS_NPC(victim) &&  IS_SET(ch->act, PLR_HOLYLIGHT)
	&&  IS_SET(victim->act, PLR_WIZINVIS) )
     leng = str_apd_max ( buf, "(Wizinvis) " , leng, MAX_STRING_LENGTH);
    if (!IS_NPC(victim) && !victim->desc)
     leng = str_apd_max ( buf, "(Link Dead) " , leng, MAX_STRING_LENGTH);
    if ( !IS_NPC(victim) && IS_SET(victim->pcdata->player2_bits, PLR2_AFK) ) 
     leng = str_apd_max ( buf, "[AFK] " , leng, MAX_STRING_LENGTH);

    if ( victim->desc && victim->desc->connected == CON_EDITING )
     leng = str_apd_max ( buf, "(Writing) ", leng, MAX_STRING_LENGTH );

    if( !can_see( ch, victim )  && IS_NPC( victim ) )	
     {
      if( !can_see_in_room(ch, ch->in_room ) )
         leng = str_apd_max ( buf, "(Dark Room) " , leng, MAX_STRING_LENGTH);
      else
         leng = str_apd_max ( buf, "(Shouldn't see) ", leng,MAX_STRING_LENGTH);	
     }

    if ( IS_AFFECTED(victim, AFF2_MIRROR_IMAGE)) 
          leng = str_apd_max (buf, "(Multiple) " , leng, MAX_STRING_LENGTH);
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) 
          leng = str_apd_max ( buf, "(Invis) "   , leng, MAX_STRING_LENGTH   );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) 
          leng = str_apd_max ( buf, "(Hide) "     , leng, MAX_STRING_LENGTH  );
    if ( IS_AFFECTED(victim, AFF_STEALTH)     )
          leng = str_apd_max ( buf, "(Stealth) "  , leng, MAX_STRING_LENGTH  );
    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE ) )
	  leng = str_apd_max ( buf, "(PROTO) "    , leng, MAX_STRING_LENGTH );
    if(IS_NPC(ch)||
       ch->mclass[CLASS_ILLUSIONIST]>=30||ch->mclass[CLASS_NECROMANCER]>=30)
      if(IS_AFFECTED(victim, AFF_CHARM)       ) 
          leng = str_apd_max ( buf, "(Charmed) " , leng, MAX_STRING_LENGTH   );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) 
          leng = str_apd_max ( buf, "(Translucent) ", leng, MAX_STRING_LENGTH);
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) 
          leng = str_apd_max ( buf, "(Pink Aura) ", leng, MAX_STRING_LENGTH  );
    if ( IS_AFFECTED(victim, AFF2_MAGE_SHIELD) ) 
          leng = str_apd_max ( buf, "(Shimmering) " , leng, MAX_STRING_LENGTH );
    if ( IS_AFFECTED(victim, AFF2_POSSESS) && is_affected(ch, gsn_truesight) ) 
          leng = str_apd_max ( buf, "(Possessed) " , leng, MAX_STRING_LENGTH );
    if ( IS_AFFECTED(victim, AFF2_FIRESHIELD) )
          leng = str_apd_max ( buf, "(Burning) " , leng, MAX_STRING_LENGTH );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) 
          leng = str_apd_max ( buf, "(Red Aura) "  , leng, MAX_STRING_LENGTH );
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) 
          leng = str_apd_max ( buf, "(White Aura) ", leng, MAX_STRING_LENGTH );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
	  leng = str_apd_max ( buf, "(KILLER) "    , leng, MAX_STRING_LENGTH );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_OUTCAST ) )
	  leng = str_apd_max ( buf, "(OUTCAST) "    , leng, MAX_STRING_LENGTH );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
	  leng = str_apd_max ( buf, "(THIEF) "     , leng, MAX_STRING_LENGTH );
    if ( IS_NPC(victim) && !IS_NPC(ch) && ch->race==RACE_HALFLING &&
         IS_SET(victim->act, ACT_AGGRESSIVE ) )
	  leng = str_apd_max ( buf, "(Aggressive) ", leng, MAX_STRING_LENGTH );
    if ( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_ETHEREAL ) )
	  leng = str_apd_max ( buf, "(ETHEREAL) ", leng, MAX_STRING_LENGTH );

  if(IS_NPC(victim))
    if ( victim->position == POS_STANDING && victim->long_descr[0] != '\0' )
      {
	    leng = str_apd_max ( buf, victim->long_descr, leng, 
                                      MAX_STRING_LENGTH );
      if( buf[strlen(buf)-1]!='\n' && buf[strlen(buf)-1]!='\r')
	leng = str_apd_max ( buf, "\n\r", leng, MAX_STRING_LENGTH );
      buf[0] = UPPER(buf[0]);
      send_to_char( buf, ch );
      return;
      }

      leng = str_apd_max ( buf, get_name(victim), leng, MAX_STRING_LENGTH );
    

    switch ( victim->position )
    {
    case POS_DEAD:     
          leng = str_apd_max ( buf, " is DEAD!!", leng, MAX_STRING_LENGTH );
          break;
    case POS_MORTAL:   
          leng = str_apd_max ( buf, " is mortally wounded.", leng, 
                                                    MAX_STRING_LENGTH );
          break;
    case POS_INCAP:    
          leng = str_apd_max ( buf, " is incapacitated.", leng, 
                                                    MAX_STRING_LENGTH );
          break;
    case POS_STUNNED:  
          leng = str_apd_max ( buf, " is lying here stunned.", leng, 
                                                    MAX_STRING_LENGTH );
          break;
    case POS_SLEEPING: 
          leng = str_apd_max ( buf, " is sleeping here.", leng, 
                                                    MAX_STRING_LENGTH );
          break;
    case POS_RESTING:  
          leng = str_apd_max ( buf, " is resting here.", leng, 
                                                    MAX_STRING_LENGTH );
          break;
    case POS_FIGHTING:
	leng = str_apd_max ( buf, " is here, fighting ", leng, 
                                                    MAX_STRING_LENGTH );
	if ( victim->fighting == NULL )
	    leng = str_apd_max ( buf, "thin air??", leng, 
                                                    MAX_STRING_LENGTH );
	else if ( who_fighting( victim ) == ch )
	    leng = str_apd_max ( buf, "YOU!", leng, MAX_STRING_LENGTH );
	else if ( victim->in_room == victim->fighting->who->in_room )
	{
	    leng = str_apd_max ( buf, PERS( victim->fighting->who, ch ) , leng, 
                                                     MAX_STRING_LENGTH );
	    leng = str_apd_max ( buf, ".", leng, MAX_STRING_LENGTH  );
	}
	else
	    leng = str_apd_max ( buf, "somone who left??", leng,
                                                     MAX_STRING_LENGTH );
	break;
    default: 
    /*
      What's the point in going through all that if the person has a different
      long descr anyway ? :) Martin 7/8/98 
        if((!IS_NPC(victim))&&(victim->long_descr!=NULL)&&
	 (strlen(victim->long_descr)>0))
	leng = str_cpy_max( buf, victim->long_descr, MAX_STRING_LENGTH );
      else */
	leng = str_apd_max ( buf, " is here.", leng, MAX_STRING_LENGTH );
      break;
    }
    }
    leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *fch;
    int iWear;
    int percent;
    bool found;
    int leng;

  if( ch != NULL)
   {
    if ( can_see( victim, ch ) && !FINGER_MODE)
    {
	act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
    if( IS_NPC( victim ) )
	send_to_char( (const char *)ansi_translate_text( ch,
            victim->description), ch );
    else
	send_to_char( (const char *)ansi_translate_text( ch,
            victim->description), ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }
   }

if( FINGER_MODE )
  return;


    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;
    if( ch!= NULL)
      leng =str_cpy_max( buf, capitalize(PERS(victim, ch)), MAX_STRING_LENGTH );
    else
      {
      *buf='\0';
      leng = 0;
      }

	 if ( percent >= 100 )
            leng = str_apd_max ( buf, " is in perfect health."  , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  90 )
            leng = str_apd_max ( buf, " is slightly scratched." , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  80 )
            leng = str_apd_max ( buf, " has a few bruises."     , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  70 )
            leng = str_apd_max ( buf, " has some cuts."         , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  60 )
            leng = str_apd_max ( buf, " has several wounds."    , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  50 )
            leng = str_apd_max ( buf, " has many nasty wounds." , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  40 )
            leng = str_apd_max ( buf, " is bleeding freely."    , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  30 )
            leng = str_apd_max ( buf, " is covered in blood."   , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  20 )
            leng = str_apd_max ( buf, " is leaking guts."       , leng,
                                        MAX_STRING_LENGTH );
    else if ( percent >=  10 )
            leng = str_apd_max ( buf, " is almost dead."        , leng,
                                        MAX_STRING_LENGTH );
    else   
            leng = str_apd_max ( buf, " is DYING."              , leng,
                                        MAX_STRING_LENGTH );

    buf[0] = UPPER(buf[0]);
  if( ch != NULL )
    send_to_char( buf, ch );
  else
    {
    strcpy( buf2, "    ");
    strcat( buf2, capitalize(get_name( victim )));
    strcat( buf2, buf);
    strcat( buf2, "\n\r");
    for( fch = victim->in_room->first_person ; fch != NULL; fch = fch->next_in_room)
      if( !IS_NPC( fch) && victim!=fch)
        if(fch->position>POS_SLEEPING&&(can_see(fch,victim)||can_see_is_hidden)
           && !IS_SET(fch->act,PLR_DAMAGE))
          {
             if( is_same_group( fch, victim ) &&
             fch->ansi==1 && fch->leader!=NULL)
	          {
	          char buf3[160];
	          sprintf( buf3, "\033[0;%d;%dm%s", fch->pcdata->color[0][8],
	             fch->pcdata->color[1][8], buf2);
                  if( !IS_SET( fch->pcdata->spam, 16))
	            send_to_combat_char( buf3, fch);
	          }
         else if(victim->fighting!=NULL &&
                   is_same_group( fch, victim->fighting->who ) &&
                   fch->ansi==1 && fch->leader!=NULL)
	         {
	         char buf3[160];
	         sprintf( buf3, "\033[0;%d;%dm%s", fch->pcdata->color[0][5],
	         fch->pcdata->color[1][5], buf2);
                 if( !IS_SET( fch->pcdata->spam, 64) &&
                     !IS_SET( fch->pcdata->spam, 1))
	           send_to_combat_char( buf3, fch);
	         }
	       else
                 if( !IS_SET( fch->pcdata->spam, 256))
	           send_to_combat_char( buf2, fch);
	       }
    return;
    }

    /* greater peek junk -Dug */
    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < ch->pcdata->learned[gsn_greater_peek] )
      {
      sprintf(buf,"%s has about %d hitpoints left.\n\r",
	  capitalize(PERS(victim,ch)), victim->hit);
      send_to_char( buf, ch );
      }

    leng = 0;
    *buf = '\0';

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
            leng = str_apd_max ( buf,get_name(victim),leng,MAX_STRING_LENGTH );
            leng = str_apd_max ( buf, " is using:\n\r", leng,
                                              MAX_STRING_LENGTH );
		found = TRUE;
	    }
	    leng = str_apd_max(buf, where_name[iWear], leng,MAX_STRING_LENGTH);
	    leng = str_apd_max(buf, format_obj_to_char( obj, ch, 0), leng,
                                              MAX_STRING_LENGTH);
	    leng = str_apd_max(buf, "\n\r", leng, MAX_STRING_LENGTH);
	}
    }

	    if ( !found )
	    {
            leng = str_apd_max ( buf,get_name(victim),leng,MAX_STRING_LENGTH );
            leng = str_apd_max ( buf, " is using:    Nothing!\n\r", leng,
                                              MAX_STRING_LENGTH );
            }

    send_to_char( buf, ch );


    if ( victim != ch
    &&   !IS_NPC(ch)
    &&  !IS_SET( ch->pcdata->player2_bits, PLR2_PEEK )
    &&  ((number_percent( ) < ch->pcdata->learned[gsn_peek] ) ||
         (which_god(ch) == GOD_POLICE) || (ch->level == MAX_LEVEL)))
    {
	send_to_char( "You peek at the inventory:\n\r", ch );
	show_list_to_char( victim->first_carrying, ch, 0, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
        &&  !IS_SET(ch->act, PLR_HOLYLIGHT)
	&&  IS_SET(rch->act, PLR_WIZINVIS) )
	    continue;

        /* Slight chance that person doesn't see someone due to smoke 
           Martin 14/8/98 */

	if( ch->position>POS_SLEEPING&& (can_see( ch, rch )) &&
	   ((!IS_AFFECTED( rch, AFF_HIDE)) ||
         ( IS_AFFECTED( rch, AFF_HIDE) && IS_AFFECTED( ch, AFF_DETECT_HIDDEN))))
	{
           if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE) 
                && number_percent()<(40-get_curr_int(ch)))
            send_to_char("The smoke obscures the outline of a large shape.\n\r", ch);
           else
	    show_char_to_char_0( rch, ch );
	}
	else if ( !can_see_in_room( ch, ch->in_room ) 
		  && !IS_AFFECTED( rch, AFF_HIDE)
		  && IS_AFFECTED(rch, AFF_INFRARED ) )
	{
          if( IS_NPC(rch) && !IS_NPC(ch) && ch->race==RACE_HALFLING &&
             IS_SET(rch->act, ACT_AGGRESSIVE ) )
	    send_to_char( "You see glowing aggressive eyes watching YOU!\n\r", ch );
          else
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
        else
          if( IS_NPC(rch) && !IS_NPC(ch) && ch->race==RACE_HALFLING &&
             IS_SET(rch->act, ACT_AGGRESSIVE ) )
	    send_to_char( "You see a lurking presense.\n\r", ch );
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}


int sCRrNum;

void showCharRoom(CHAR_DATA *ch,ROOM_INDEX_DATA *rm,int dirNum,int look_range)
  {
  CHAR_DATA *rch;
  char buf[MAX_STRING_LENGTH];
  int look_chance;

  if(++sCRrNum>look_range)
    return;

if( rm==NULL )
  {
  log_string( "Null room in showChar");
  return;
  }

  switch(look_range)
    {
    case SHORT_RANGE:
    default:
      if( !IS_NPC(ch) && ch->pcdata->learned[gsn_short_range]==0)
	return;
      if(!IS_NPC(ch))
	look_chance=ch->pcdata->learned[gsn_short_range];
      else
	look_chance=90;
      break;
    case MED_RANGE:
      if( !IS_NPC(ch) && ch->pcdata->learned[gsn_med_range]==0)
	return;
      if(!IS_NPC(ch))
	look_chance=(2*ch->pcdata->learned[gsn_short_range]+
		     ch->pcdata->learned[gsn_med_range])/3;
      else
	look_chance=80;
      break;
    case LONG_RANGE:
      if( !IS_NPC(ch) && ch->pcdata->learned[gsn_long_range]==0)
	return;
      if(!IS_NPC(ch))
	look_chance=(3*ch->pcdata->learned[gsn_short_range]+
		   2*ch->pcdata->learned[gsn_med_range]+
		   ch->pcdata->learned[gsn_long_range])/6;
      else
	look_chance=70;
      break;
    case 6:  /* stop over the limit */
      return;
    }

  if(IS_SET(rm->room_flags,ROOM_SAFE|ROOM_SOLITARY|ROOM_PRIVATE))
    {
    sprintf(buf,"%d> A stretch of nothing...\n\r", look_range);
    send_to_char(buf,ch);
    }
  else if(!IS_NPC(ch) && !IS_AFFECTED(ch,AFF_INFRARED) &&
	  !IS_SET(ch->act, PLR_HOLYLIGHT) && !can_see_in_room(ch,rm) )
    {
    sprintf(buf,"%d> A length of darkness...\n\r", look_range);
    send_to_char(buf,ch);
    }
  else
    {
    if(IS_NPC(ch) || IS_SET(ch->act, PLR_HOLYLIGHT) || can_see_in_room(ch,rm) )
      sprintf(buf,"%d> %s\n\r",look_range,rm->name);
    else
      sprintf(buf,"%d> A length of darkness...\n\r", look_range);
    send_to_char(buf,ch);
  
    for(rch=rm->first_person;rch!=NULL;rch=rch->next_in_room)
      {
      if(can_see(ch,rch)&&!(IS_AFFECTED(rch,AFF_HIDE)||
	 IS_AFFECTED(rch,AFF_STEALTH)))
        {
	if(number_percent()<look_chance)
	  {
	  sprintf(buf,"    %s%s\n\r",
              get_color_string(ch,COLOR_MOBILES,VT102_BOLD),
              get_name( rch ) );
	  send_to_char(buf,ch);
	  }
        }
      else
       if( !IS_NPC(ch) && IS_NPC(rch) && IS_SET(rch->act, ACT_AGGRESSIVE) &&
           ch->race==RACE_HALFLING )
        {
	if(number_percent()<look_chance)
	  {
	  sprintf(buf,"    %sA lurking presense.\n\r",
              get_color_string(ch,COLOR_MOBILES,VT102_BOLD) );
	  send_to_char(buf,ch);
	  }
        }
      }
    }
  if(rm->exit[dirNum]==NULL)
    return;
  if(IS_SET(rm->exit[dirNum]->exit_info, EX_CLOSED))
    return;
  if(rm->exit[dirNum]->to_room==NULL)
    return;

      if(IS_SET(rm->exit[dirNum]->exit_info, EX_CLOSED) ||
        (IS_SET(rm->exit[dirNum]->exit_info,EX_HIDDEN) &&
	  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))  ||
	(!can_use_exit(ch, rm, dirNum) ) )
        return;

  showCharRoom(ch,rm->exit[dirNum]->to_room,dirNum,look_range+1);
  return;
  }

int showCharDir(CHAR_DATA *ch,int dirNum)
  {
  /* extern char * const dir_name[]; */
  int look_range;

  if(ch->in_room->exit[dirNum]!=NULL)
    if(ch->in_room->exit[dirNum]->to_room!=NULL)
      {
      if(IS_SET(ch->in_room->exit[dirNum]->exit_info, EX_CLOSED) ||
        (IS_SET(ch->in_room->exit[dirNum]->exit_info,EX_HIDDEN) &&
	  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))  ||
	  !can_use_exit(ch, ch->in_room, dirNum))
	return 0;
      if(IS_NPC(ch))
	look_range=MED_RANGE;
      else
	look_range=ch->pcdata->learned[gsn_long_range]>0?LONG_RANGE:
	  ch->pcdata->learned[gsn_med_range]>0?MED_RANGE:SHORT_RANGE;
      /* switch(dirNum)
	{
	case DIR_UP:
	  strcpy(buf1,"above");
	  break;
	case DIR_DOWN:
	  strcpy(buf1,"below");
	  break;
	default:
	  sprintf(buf1,"to the %s of",dir_name[dirNum]);
	  break;
	}
      sprintf(buf,"...in the distance %s you, you see...",buf1);
      send_to_char(buf,ch); */
      sCRrNum=0;
      showCharRoom(ch,ch->in_room->exit[dirNum]->to_room,dirNum,1);
      }
  return sCRrNum;
  }

    /* Chaos 12/15/93 
       modified slightly by Martin 7/3/99 to do extract_char and close_socket
       seeing as level 1 characters now save...  */
void do_delete( CHAR_DATA *ch, char *arg)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[80] ;
  char name_buf[20];
  bool PassRqd;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *rch;

  DISALLOW_SNOOP = TRUE;

  if(IS_NPC(ch)||ch->position==POS_FIGHTING||ch->fighting!=NULL)
    {
    send_to_char( "You may not do that now.\n\r", ch);
    wipe_string( arg );
    return;
    }
  if (ch->which_god== GOD_DEMISE && ch->level < ch->pcdata->demisedlevel+3)
  {
   send_to_char("You must gain 3 levels before you can delete !\n\r",ch);
   return;
  }
  if( arg == NULL )
    PassRqd = FALSE;
  else
    PassRqd = TRUE;
  if(PassRqd && arg[0]=='\0')
    {
    send_to_char( "You must use a password with this command.\n\rThis command permanently deletes your character!!!!\n\rWarning!   Warning!\n\r", ch);
    wipe_string( arg );
    return;
    }
  if( !PassRqd || !strcmp(crypt(arg,ch->pcdata->pwd),ch->pcdata->pwd) )
    {
    strcpy( buf3, ch->name);
    buf3[1]='\0';
    if( buf3[0]>='A' && buf3[0]<='Z')
      buf3[0]+=( 'a' - 'A');
    if( PassRqd )
      send_to_char( "You have permanently deleted your character!\n\r", ch);
    else
      {
      send_to_char( "Your character has been permanently deleted!\n\r", ch);
      send_to_char( "You have a serious bug that could not be corrected.\n\r",ch);
      }
    strcpy( name_buf, capitalize( ch->name ) );

    if( ch->pcdata->clan != NULL)
     ch->pcdata->clan->members--;

    save_clan (ch->pcdata->clan);

    if( COMPRESS_FILES)
      {
      sprintf( buf, "%s/%s/%s.gz", PLAYER_DIR, buf3, name_buf);
      sprintf(buf2,"%s/%s/delete.%s.gz",PLAYER_DIR,buf3, name_buf);
      }
    else
      {
      sprintf( buf, "%s/%s/%s", PLAYER_DIR, buf3, name_buf );
      sprintf(buf2,"%s/%s/delete.%s", PLAYER_DIR, buf3, name_buf );
      }
    remove( buf2 );
    rename( buf, buf2);

/*    ch->level=1;
    do_quit( ch, "arglebargle");*/

    if (ch->master != NULL)
      stop_follower (ch);
 
    if (ch->pcdata->bet_mode == 2)
      do_resign (ch, NULL);
    do_return (ch, NULL);

    if (ch->vt100 != 0 && is_desc_valid (ch))
      vt100off (ch);

    for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room)
     if (can_see (rch, ch) && rch != ch && rch->position > POS_SLEEPING)
      ch_printf (rch, "%s has left the game.\n\r", get_name (ch));

    log_printf ("%s has deleted.", get_name(ch));

  if (ch->desc != NULL)
  {
   d = ch->desc;
   force_help (d, "goodbye");
   write_to_port (d);
   *d->inbuf = '\0';
  }
  extract_char (ch, TRUE);
  if (d != NULL)
    d->character = NULL;
  if (d != NULL)
    close_socket (d, TRUE);

    wipe_string( arg );
    return;
    }
  else
    {
    send_to_char( "That was not your password.\n\rThis command permanently deletes your character!!!!\n\rWarning!   Warning!\n\r", ch);
    }
    wipe_string( arg );
  return;
}

void do_speak( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   int cnt;
   if(IS_NPC(ch) || argument[0]=='\0')
      return;
   buf[0]=argument[0];
   buf[1]=argument[1];
   buf[2]='\0';
   buf2[2]='\0';
   for(cnt=0;cnt<MAX_RACE;cnt++)
      {
      buf2[0]=race_table[cnt].race_name[0];
      buf2[1]=race_table[cnt].race_name[1];
      if(!strcasecmp(buf,buf2))
	if(IS_SHIFT(ch->language,cnt))
	  {
	  sprintf(buf,"Language set to %s.\n\r", race_table[cnt].race_name);
	  send_to_char( buf, ch);
	  ch->speak=SHIFT(cnt);
	  return;
	  }
      }
    send_to_char( "You cannot speak that language.\n\r", ch);
    return;
    }

void do_notice( CHAR_DATA *ch, char *argument)
    {
    CHAR_DATA *list;
    char buf[MAX_STRING_LENGTH];
    if(IS_NPC(ch))
      return;
    if(IS_AFFECTED(ch, AFF_BLIND))
      {
      send_to_char("You are to blind to do that.\n\r", ch);
      return;
      }
    if(ch->pcdata->learned[gsn_notice]>number_percent())
      {
      send_to_char("You search the room for creatures.\n\r", ch);
      for( list=ch->in_room->first_person; list!=NULL; list=list->next_in_room)
	if(ch!=list && (IS_NPC(list) || !IS_SET(list->act, PLR_WIZINVIS)) &&
          ( !IS_NPC(list) || !IS_AFFECTED( list, AFF_ETHEREAL ) ) )
	  {
	  if((!IS_AFFECTED(list, AFF_INVISIBLE))
	     ||(IS_AFFECTED(list,AFF_INVISIBLE)
		&& (is_affected(ch, gsn_truesight) || IS_AFFECTED(ch, AFF_DETECT_INVIS))))
	    {
	    if(!IS_NPC(list))
	      strcpy(buf,list->name);
	    else
	      strcpy(buf,list->short_descr);
	    }
	  else
	    strcpy(buf,"Someone");
	  if(buf[0]>='a' && buf[0]<='z')
	    buf[0]-=('a'-'A');
	  strcat(buf, " is here.");
	  send_to_char(buf, ch);
	  }
      }
    return;
    }
      
void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    char buft[200];
    int cnt;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch) 
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !can_see_in_room( ch, ch->in_room )
    &&   !IS_AFFECTED(ch, AFF_INFRARED))
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->first_person, ch );
	  return;
    }
  
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !strcasecmp( arg1, "auto" ) )
    {
    int leng;
	/* 'look' or 'look auto' */
    if( !IS_NPC(ch) && ch->ansi==1)
      {
      leng = str_cpy_max( buf,  get_color_string(ch,COLOR_PROMPT,VT102_BOLD),
                                        MAX_STRING_LENGTH );
      leng = str_apd_max( buf, ch->in_room->name, leng, MAX_STRING_LENGTH );
      leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
      }
    else
      {
      leng = str_cpy_max( buf, "  ", MAX_STRING_LENGTH );
      leng = str_apd_max( buf, ch->in_room->name, leng, MAX_STRING_LENGTH );
      leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
      }
    send_to_char( buf,  ch );

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT))
	    do_exits( ch, "auto" );


	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) )
      {
      if( !IS_NPC(ch) && ch->ansi==1)
        leng = str_cpy_max( buf,  get_color_string( ch, COLOR_TEXT, VT102_DIM),
                                        MAX_STRING_LENGTH );
      else
        {
        leng = 0;
        *buf = '\0';
        }
      if (IS_SET(ch->in_room->room_flags, ROOM_HALLUCINATE) && 
          !is_affected(ch, gsn_truesight) && !IS_SET(ch->act, PLR_HOLYLIGHT)) 
        leng = str_apd_max( buf, (const char *)ansi_translate_text( ch,
              ch->in_room->hallucinate_room->description), 
              leng, MAX_STRING_LENGTH );
      else
        leng = str_apd_max( buf, (const char *)ansi_translate_text( ch,
              ch->in_room->description) , leng, 
              MAX_STRING_LENGTH );
      if( buf[leng-2]!='\n' &&  buf[leng-2]!='\r')
	leng = str_apd_max( buf, "\n", leng, MAX_STRING_LENGTH);
      if( buf[leng-1]!='\r')
	leng = str_apd_max( buf, "\r", leng, MAX_STRING_LENGTH);
      send_to_char( buf, ch );
      }
        if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE))
         send_to_char( "A cloud of dark purple smoke partially obscures your vision.\n\r", ch );
        for(cnt=0;cnt<MAX_LAST_LEFT;cnt++)
        {
         if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_BLOOD))
         {
          char buf[MAX_INPUT_LENGTH];

          if (ch->ansi==1)
           sprintf(buf, "     \033[1;31mA trail of blood "); 
          else
           sprintf(buf, "     A trail of blood "); 
          
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
          break;
         }
        }
	show_list_to_char( ch->in_room->first_content, ch, 1, FALSE );
	show_char_to_char( ch->in_room->first_person,   ch );
	return;
    }

    if ( !strcasecmp( arg1, "i" ) || !strcasecmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->first_content, ch, 0, TRUE );
	    break;
	}
	return;
    }

    /*  

      Changed this to normal get_char_room...otherwise ppl can look at 
      stealthed ppl without even having detect hidden...big hole
      Martin 6/8/98
     if(( victim = get_char_room_even_hidden( ch, arg1 ) ) != NULL )
    */
    if(( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );


  if( !IS_NPC( ch ) && ch->desc!=NULL && IS_NPC(victim) &&
      ch->desc->character==ch && victim->pIndexData->mob_file !=NULL &&
      !IS_SET( ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES ) &&
      IS_SET( ch->act, PLR_TERMINAL ) )
    {
    /* Send notice of file */

    write_to_port( ch->desc );
    sprintf( buf, "%c%c%s%c", 29, 'E', victim->pIndexData->mob_file, 29);
    write_to_buffer( ch->desc, buf, 1000000 );
    }
    


        if( IS_NPC( victim ) && !IS_NPC( ch ) )
          if( ch->ansi != 0 && victim->pIndexData->picture != NULL)
            act( "$N has a displayable picture.", ch, NULL, victim, TO_CHAR );
/*
        if( TEST_GAME )
          {
          AREA_DATA *pArea;
  if(!IS_NPC(victim))
    {
    unsigned long num;

    for(pArea=first_area;pArea!=NULL;pArea=pArea->next)
      {
      if(victim->pcdata->quest[(num=(pArea->low_r_vnum/100))]!=0)
	{
	sprintf(buf,"%s:%08lX\n",pArea->name, victim->pcdata->quest[num]);
	send_to_char( buf, ch );
	}
      }
    }
  else
    {
    sprintf(buf,"Quest '%s' #%08lX\n",victim->pIndexData->area->name,
	   victim->pIndexData->mob_quest);
	  send_to_char( buf, ch );
    }
          }
*/
	return;
    }

    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
          {
	    pdesc = get_extra_descr( arg1, obj->first_extradesc );
	    if ( pdesc != NULL )
	    {
		send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
         if( !IS_NPC( ch ) )
          if( ch->ansi != 0 && obj->pIndexData->picture != NULL)
              act( "$p has a picture.", ch, obj, NULL, TO_CHAR);
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->first_extradesc );
	    if ( pdesc != NULL )
	    {
		send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
         if( !IS_NPC( ch ) )
          if( ch->ansi != 0 && obj->pIndexData->picture != NULL)
              act( "$p has a picture.", ch, obj, NULL, TO_CHAR);
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) || 
             ( is_object_reference(obj,arg1) &&
    !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ))
	{

#ifdef EQUIPMENT_DAMAGE
  if( ch->level > 50 )
    {
      sprintf( buft, "%s has a condition of %d%%.\n\r",
         capitalize(obj->short_descr), obj->condition );
    }
  else
    {
        if( obj->condition >= 90 )
          sprintf( buft, "%s is in great condition.\n\r",
              capitalize(obj->short_descr));
        else if( obj->condition >= 70 )
          sprintf( buft, "%s is in good condition.\n\r",
              capitalize(obj->short_descr));
        else if( obj->condition >= 50 )
          sprintf( buft, "%s is in fair condition.\n\r",
              capitalize(obj->short_descr));
        else if( obj->condition >= 30 )
          sprintf( buft, "%s is in poor condition.\n\r",
              capitalize(obj->short_descr));
        else if( obj->condition >= 10 )
          sprintf( buft, "%s is in bad condition.\n\r",
              capitalize(obj->short_descr));
        else 
          sprintf( buft, "%s is about to fall apart.\n\r",
              capitalize(obj->short_descr));
      }
    send_to_char( buft, ch );
#endif

	    if( obj->long_descr[0]=='\0')
	      {
	      sprintf( buft, "You see nothing special about %s.\n\r",
		  obj->short_descr );
	      send_to_char( buft, ch );
	      }
	    else
              send_to_char( (const char *)ansi_translate_text(ch, obj->long_descr), ch );
         if( !IS_NPC( ch ) )
          {
  if( !IS_NPC( ch ) && ch->desc!=NULL && 
      ch->desc->character==ch && obj->pIndexData->obj_file !=NULL &&
      !IS_SET( ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES ) &&
      IS_SET( ch->act, PLR_TERMINAL ) )
    {
    /* Send notice of file */

    write_to_port( ch->desc );
    sprintf( buf, "%c%c%s%c", 29, 'E', obj->pIndexData->obj_file, 29);
    write_to_buffer( ch->desc, buf, 1000000 );
    }

          if( ch->ansi != 0 && obj->pIndexData->picture != NULL)
              act( "$p has a picture.", ch, obj, NULL, TO_CHAR);
          if( obj->poison != NULL && obj->poison->owner == ch->pcdata->pvnum )
            {
            char bufc[12];
            if( obj->poison->instant_damage_high > 0 &&
                obj->poison->constant_damage_high > 0 )
              strcpy( bufc, "green" );
            else
            if( obj->poison->constant_damage_high > 0 )
              strcpy( bufc, "blue" );
            else
              strcpy( bufc, "purple" );
            if( obj->poison->for_npc )
              sprintf( buf, "There is murky %s poison on this object.\n\r", 
                 bufc );
            else
              sprintf( buf, "There is clear %s poison on this object.\n\r", 
                 bufc );
            send_to_char( buf, ch );
            }
          }
	    return;
	}
    }

    for ( obj = ch->in_room->first_content; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->first_extradesc );
	    if ( pdesc != NULL )
	    {
		send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->first_extradesc );
	    if ( pdesc != NULL )
	    {
		send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) || (is_object_reference(obj,arg1) &&
    !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) ))
	{
	    if( obj->long_descr[0]=='\0')
	      {
	      sprintf( buft, "You see nothing special about %s.\n\r",
		 obj->short_descr );
	      send_to_char( buft, ch );
	      }
	    else
	      send_to_char( (const char *)ansi_translate_text(ch, obj->long_descr), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
	    return;
	}
    }

    pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc );
    if ( pdesc != NULL )
    {
	send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
	return;
    }
    for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->first_extradesc );
	    if ( pdesc != NULL )
	    {
	send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->first_extradesc );
	    if ( pdesc != NULL )
	    {
	send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
		return;
	    }
	}

	if ( is_name_short( arg1, obj->name ) )
	{
	    if( obj->long_descr[0]=='\0')
	      {
	      sprintf( buft, "You see nothing special about %s.\n\r",
		  obj->short_descr );
	      send_to_char( buft, ch );
	      }
	    else
	      send_to_char( (const char *)ansi_translate_text(
                            ch, obj->long_descr), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
	    return;
	}
     }
    for ( obj = ch->in_room->first_content; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->first_extradesc );
	    if ( pdesc != NULL )
	    {
	send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->first_extradesc );
	    if ( pdesc != NULL )
	    {
	send_to_char( (const char *)ansi_translate_text(ch, pdesc), ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
		return;
	    }
	}

	if ( is_name_short( arg1, obj->name ) )
	{
	    if( obj->long_descr[0]=='\0')
	      {
	      sprintf( buft, "You see nothing special about %s.\n\r",
		 obj->short_descr );
	      send_to_char( buft, ch );
	      }
	    else
	send_to_char( (const char *)ansi_translate_text(ch, obj->long_descr),
                       ch );
            if( obj->pIndexData->picture != NULL )
              send_to_char( "This object has a picture.\n\r", ch );
	    return;
	}
     }

	 if ( !strcasecmp( arg1, "n" ) || !strcasecmp( arg1, "north" ) ) door = 0;
    else if ( !strcasecmp( arg1, "e" ) || !strcasecmp( arg1, "east"  ) ) door = 1;
    else if ( !strcasecmp( arg1, "s" ) || !strcasecmp( arg1, "south" ) ) door = 2;
    else if ( !strcasecmp( arg1, "w" ) || !strcasecmp( arg1, "west"  ) ) door = 3;
    else if ( !strcasecmp( arg1, "u" ) || !strcasecmp( arg1, "up"    ) ) door = 4;
    else if ( !strcasecmp( arg1, "d" ) || !strcasecmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&  !IS_SET(pexit->exit_info,EX_UNBARRED)
    &&   pexit->keyword[0] != ' ' )
      {
      if(((!IS_SET(pexit->exit_info,EX_HIDDEN))||
	  IS_AFFECTED(ch,AFF_DETECT_HIDDEN))
	 && can_use_exit(ch,ch->in_room, door) )
	{
	      if ( IS_SET(pexit->exit_info, EX_CLOSED) )
		{
		act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		}
	      else if ( IS_SET(pexit->exit_info, EX_BASHED) )
		{
		act( "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );
		}
	      else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
		{
		act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
		}
	}
      }
    if(IS_NPC(ch)||
       (((ch->pcdata->learned[gsn_short_range]>0)||
	 (ch->pcdata->learned[gsn_med_range]>0)||
	 (ch->pcdata->learned[gsn_long_range]>0))&&
	((!IS_SET(pexit->exit_info,EX_HIDDEN))||
	 IS_AFFECTED(ch,AFF_DETECT_HIDDEN))&&
	 can_use_exit(ch, ch->in_room, door)))
	 showCharDir(ch,door);

    return;
}


void do_scan( CHAR_DATA *ch, char *argument )
  {
  char arg[MAX_INPUT_LENGTH],*c,buf[MAX_INPUT_LENGTH];
  bool found;
  char bufc[20];
  
  one_argument( argument, arg );

  if(arg[0]=='\0')
    strcpy(arg,"neswud");

  strcpy( bufc,  get_color_string(ch,COLOR_PROMPT,VT102_BOLD) );

  for(c=arg;*c!='\0';c++)
    {
    found = TRUE;
    switch(*c)
      {
      case 'n':
         if ( ch->in_room->exit[0] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sTo the north:\n\r", bufc);
         break;
      case 'e':
         if ( ch->in_room->exit[1] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sTo the east:\n\r", bufc);
         break;
      case 's':
         if ( ch->in_room->exit[2] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sTo the south:\n\r", bufc);
         break;
      case 'w':
         if ( ch->in_room->exit[3] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sTo the west:\n\r", bufc);
         break;
       case 'u':
         if ( ch->in_room->exit[4] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sUp:\n\r", bufc);
         break;
       case 'd':
         if ( ch->in_room->exit[5] == NULL )
           found = FALSE;
         else
	   sprintf(buf, "%sDown:\n\r", bufc);
         break;
      default:
	sprintf(buf,"There is no direction abbreviated by '%c'.\n\r",*c);
	send_to_char(buf,ch); 
	strcpy( buf, "");
	break;
     }
    if( buf[0]=='\0')
      continue;
    if( found)
      {
      send_to_char(buf,ch); 
      sprintf(buf,"%c",*c);
      do_look(ch,buf);
      }
    }
  return;
  }



void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], *cnt;
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door,num_exits=0;
    char dim[10], bold[10], buf3[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * room;

    buf[0] = '\0';
    fAuto  = !strcasecmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if( ch->ansi==1)
      {
      sprintf( bold, "\033[1;%d;%dm", ch->pcdata->color[0][7],
	  ch->pcdata->color[1][7]);
      sprintf( dim, "\033[0;%d;%dm", ch->pcdata->color[0][7], 
	  ch->pcdata->color[1][7]);
      }
  else if(ch->vt100==1)
     {
      strcpy(bold,"\033[1m");
      strcpy(dim,"\033[m");
     }
  else
    {
    bold[0]='\0';
    dim[0]='\0';
    }
    if (IS_SET(ch->in_room->room_flags, ROOM_HALLUCINATE) && 
          !is_affected(ch, gsn_truesight) && !IS_SET(ch->act, PLR_HOLYLIGHT)) 
     room = ch->in_room->hallucinate_room;
    else
     room = ch->in_room;

    found = FALSE;
    /* Slight chance first_person miss exits due to smoke - Martin 14/8/98 */
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = room->exit[door] ) != NULL
	&& pexit->to_room != NULL
        && (!IS_SET(ch->in_room->room_flags, ROOM_SMOKE) 
            || number_percent()>(40-get_curr_int(ch)))
        && ( !IS_SET(pexit->to_room->room_flags,ROOM_IS_CASTLE)  
	    || IS_NPC( ch ) || ch->pcdata==NULL
	    || !IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES))
	&& ((!IS_SET(pexit->exit_info,EX_HIDDEN))||
           IS_AFFECTED(ch,AFF_DETECT_HIDDEN))
           &&  can_use_exit(ch, ch->in_room, door))
	    {
	    found = TRUE;
            num_exits++;
	    if(fAuto)
	      {
	      buf2[0]=dir_name[door][0];
	      buf2[1]='-';
	      buf2[2]='\0';
	      }
	     else
	      {
	      strcpy(buf2, dir_name[door]);
	      while(strlen(buf2)<7)
		strcat(buf2,"-");
	      }

	      if(buf2[0]>'Z')
		buf2[0]-=('a'-'A');
	      strcpy(buf3, bold);
	      strcat(buf3, buf2);
	      strcat(buf3, dim);
	      strcpy(buf2, buf3);

	      if(!IS_SET(pexit->exit_info, EX_CLOSED))
	      {
		if( !can_see_in_room( ch, pexit->to_room))
		  sprintf(buf3, "Too dark to tell");
		else
		  sprintf(buf3, pexit->to_room->name);
	      }
	      else
		if(fAuto)
	        {
		 if(pexit->keyword!=NULL && 
		    pexit->keyword[0]!='\0')
		  sprintf( buf3, pexit->keyword);
		 else
		  sprintf( buf3, "door");
	        }
		else
		  {
		  if(pexit->description!=NULL && pexit->description[0]!='\0')
		    strcat( buf2, pexit->description);
		  else
		    strcat(buf2, "You see nothing special.");
		  while((cnt=strstr(buf2,"\n"))!=NULL)
		     *cnt=' ';
		  while((cnt=strstr(buf2,"\r"))!=NULL)
		     *cnt=' ';
		  }
              if (fAuto)
             {
               while(strlen(buf3)<18) strcat (buf3, " ");
               buf3[17]=' ';
               buf3[18]='\0';
               if (num_exits%4==0) strcat(buf3,"\n\r");
             }
             else
               strcat(buf3,"\n\r");

             strcat(buf2,buf3);
             strcat(buf,buf2);
        }
    }

    if ( !num_exits )
        strcat( buf, fAuto ? "You see no visible exits." : "None." );

    if (buf[strlen(buf)-1]!='\r' && buf[strlen(buf)-2]!='\n')
      strcat(buf,"\n\r");

    send_to_char( buf, ch);
    return;
}



void do_score( CHAR_DATA *ch, char *argument )
  {
  get_string_score_v1( ch, ch);

  send_to_char( get_string_score_txt, ch );
  return;
  }


void get_string_score_v1( CHAR_DATA *ch, CHAR_DATA *viewer)
  {
  char buf[MAX_STRING_LENGTH],buf1[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  char tbuf1[MAX_INPUT_LENGTH],tbuf2[MAX_INPUT_LENGTH];
  /* AFFECT_DATA *paf;*/
  char bold[10],dim[10];
  int cnt, gac, leng;

  *get_string_score_txt = '\0';
  if( IS_NPC( ch))
    return;

    /* Universal code for color shifting, regardless of terminal type */
  strcpy( dim,  get_color_string( viewer, COLOR_SCORE, VT102_DIM) );
  strcpy( bold, get_color_string( viewer, COLOR_SCORE, VT102_BOLD) );

  sprintf(buf,"%s  __________________________________________________________________________\n\r",bold);
  leng = str_cpy_max( get_string_score_txt, buf, MAX_STRING_LENGTH);
  sprintf(buf,"%s-|__________________________________________________________________________|-\n\r",bold);
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf1,"%s%s",ch->name,IS_NPC(ch) ? " the monster" : ch->pcdata->title);
  if(strlen(buf1)>70)
    buf1[70]='\0';
  sprintf(buf,"%s   |%s|\n\r",bold,str_resize(buf1,tbuf1,-70));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf,"%s   |%s    Class:%s%s%s  Age:%s%3d%s yrs (%05d hrs) RACE:%s %s|\n\r"
	  ,bold,dim,bold,
	  str_resize(class_table[ch->class].who_name_long,tbuf1,-20),
	  dim,bold,
	  get_age(ch),dim,
	  (ch->played)/3600 ,
	  bold,str_resize(race_table[ch->race].race_name,tbuf2,-8) );
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf,"%s   | %s   Level:%s%2d%s (%s:%s%2d%s  %s:%s%2d%s  %s:%s%2d%s  %s:%s%2d%s  %s:%s%2d%s  %s:%s%2d%s  %s:%s%2d%s) %s|\n\r", bold,dim,bold,
	  ch->level,dim,
	  class_table[0].who_name,bold,ch->mclass[0],dim,
	  class_table[1].who_name,bold,ch->mclass[1],dim,
	  class_table[2].who_name,bold,ch->mclass[2],dim,
	  class_table[3].who_name,bold,ch->mclass[3],dim,
	  class_table[4].who_name,bold,ch->mclass[4],dim,
	  class_table[5].who_name,bold,ch->mclass[5],dim,
	  class_table[6].who_name,bold,ch->mclass[6],dim, bold);
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf,"%s   | %s   Stats:Str:%s%2d%s  Int:%s%2d%s  Wis:%s%2d%s  Dex:%s%2d%s  Con:%s%2d                      |\n\r",bold,dim,bold,
	  get_curr_str(ch),dim,bold,
	  get_curr_int(ch),dim, bold,
	  get_curr_wis(ch),dim, bold,
	  get_curr_dex(ch),dim, bold,
	  get_curr_con(ch));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf1,"%d/%d",ch->hit,ch->max_hit);
  sprintf(buf2,"%d (%d lost)",ch->exp,ch->exp_lost);
sprintf(buf,"%s   |%sHitpoints:%s%s%s    Experience:%s%s |\n\r", 
	bold, dim,bold,str_resize(buf1,tbuf1,-11),dim,bold,
	str_resize(buf2,tbuf2,-33));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf1,"%d/%d",ch->mana,ch->max_mana);
  sprintf(buf,"%s   |  %s   Mana:%s%s%s          Gold:%s%-9d                         |\n\r", bold, dim, bold,str_resize(buf1,tbuf1,-11),dim,bold, ch->gold);
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf1,"%d/%d",ch->move,ch->max_move);
  sprintf(buf2,"%d/%d items",ch->carry_number,can_carry_n(ch));
  sprintf(buf,"   %s|%s Movement:%s%s%s      Carrying:%s%s                   |\n\r",bold,dim,bold,str_resize(buf1,tbuf1,-11),dim,bold,str_resize(buf2,tbuf2,-15));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf1,"%d/%d",ch->carry_weight,can_carry_w(ch));
  sprintf(buf,"%s   |%sPractices:%s%-3d%s          Carry Weight:%s%s                   |\n\r", bold, dim, bold,ch->practice,dim,bold,
	  str_resize(buf1,tbuf1,-15));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  gac = GET_AC(ch)+ch->level*3;
       if ( gac >=  101 ) strcpy( buf2, "WORSE than naked!");
  else if ( gac >=   80 ) strcpy( buf2, "naked.");
  else if ( gac >=   60 ) strcpy( buf2, "wearing clothes.");
  else if ( gac >=   40 ) strcpy( buf2, "slightly armored.");
  else if ( gac >=   20 ) strcpy( buf2, "somewhat armored.");
  else if ( gac >=    0 ) strcpy( buf2, "armored.");
  else if ( gac >= - 20 ) strcpy( buf2, "well armored.");
  else if ( gac >= - 40 ) strcpy( buf2, "strongly armored.");
  else if ( gac >= - 60 ) strcpy( buf2, "heavily armored.");
  else if ( gac >= - 80 ) strcpy( buf2, "superbly armored.");
  else if ( gac >= -100 ) strcpy( buf2, "divinely armored.");
  else                           strcpy( buf2, "invincible!");
/* level > 24 sees ac number */
  if(ch->level>24)
    {
    sprintf(buf,"(%2d)",GET_AC(ch));
    strcat(buf2,buf);
    }
  switch(ch->speed)
    {
      case 0: strcpy(buf1,"walking"); break;
      case 1: strcpy(buf1,"normal"); break;
      case 2: strcpy(buf1,"jogging"); break;
      case 3: strcpy(buf1,"running"); break;
      case 4: strcpy(buf1,"haste"); break;
    }
  sprintf(buf,"%s   |%s    Speed:%s%s%s      Armor Class:%s%s|\n\r",
	  bold,dim,bold,str_resize(buf1,tbuf1,-8),dim,bold,
	  str_resize(buf2,tbuf2,-34));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
       if ( ch->alignment >  900 ) strcpy( buf2, "angelic.");
  else if ( ch->alignment >  700 ) strcpy( buf2, "saintly.");
  else if ( ch->alignment >  350 ) strcpy( buf2, "good.");
  else if ( ch->alignment >  100 ) strcpy( buf2, "kind.");
  else if ( ch->alignment > -100 ) strcpy( buf2, "neutral.");
  else if ( ch->alignment > -350 ) strcpy( buf2, "mean.");
  else if ( ch->alignment > -700 ) strcpy( buf2, "evil.");
  else if ( ch->alignment > -900 ) strcpy( buf2, "demonic.");
  else                             strcpy( buf2, "satanic.");
/* level > 17 sees alignment number */
  if(ch->level>17)
    {
    sprintf(buf,"(%d)",ch->alignment);
    strcat(buf2,buf);
    }
  sprintf(buf1,"%d hp",ch->wimpy);
  sprintf(buf,"%s   | %s   Wimpy:%s%s%s        Alignment:%s%s|\n\r",
	  bold,dim,bold,str_resize(buf1,tbuf1,-8),dim,bold,
	  str_resize(buf2,tbuf2,-34));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  strcpy(buf1,get_room_index(ch->recall)->name);
  if(strlen(buf1)>33)
    buf1[33]='\0';
  sprintf(buf,"%s   |%s AutoExit:%s%s%s           Recall Room:%s%s|\n\r", 
       bold, dim, bold,
       (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no ",dim,bold,
       str_resize(buf1,tbuf1,-34));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  strcpy(buf1,get_room_index(ch->pcdata->death_room)->name);
  sprintf(buf,"%s   |%s AutoLoot:%s%s%s            Death Room:%s%s|\n\r",
       bold,dim,bold,
       (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no ",dim,bold,
       str_resize(buf1,tbuf1,-34));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf,"%s   |%s    Quiet:%s%s%s               AutoSac:%s%s                               |\n\r",bold,dim,bold,
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_QUIET)) ? "yes" : "no ", dim, bold,
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no ");
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
/* only for level > 14 */
  if(ch->level>14)
    {
    OBJ_DATA *wield;

    wield = get_eq_char(ch, WEAR_WIELD );
    if((!IS_NPC(ch))&&(ch->mclass[CLASS_MONK]>0)&&(wield==NULL))
      {
      int numDice,sizeDice,sizePlus;

      numDice=1;
      sizeDice=4+ch->level/6+ch->mclass[CLASS_MONK]*2/3;
      sizePlus=GET_DAMROLL(ch);

      sprintf(buf1,"%dd%d+%d",numDice,sizeDice,sizePlus);
      }
    else
      sprintf(buf1,"%-3d",GET_DAMROLL(ch));
    sprintf(buf,"%s   |%s  HitRoll:%s%-3d%s               DamRoll:%s%s                       |\n\r",bold,dim,bold,
	    GET_HITROLL(ch),dim,bold,str_resize(buf1,tbuf1,-11));
    leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
    }

  buf[0]='\0';
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
    strcat(buf,"You are drunk. ");
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    strcat(buf,"You are thirsty. ");
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
    strcat(buf,"You are hungry. ");
  if(IS_NPC(ch))
    strcat(buf,"You are an NPC! ");
  switch ( ch->position )
    {
    case POS_DEAD:     
	strcat(buf, "You are DEAD!! ");  break;
    case POS_MORTAL:
	strcat(buf, "You are mortally wounded! ");  break;
    case POS_INCAP:
	strcat(buf, "You are incapacitated! ");  break;
    case POS_STUNNED:
	strcat(buf, "You are stunned! ");  break;
    case POS_SLEEPING:
	strcat(buf, "You are sleeping. ");  break;
    case POS_RESTING:
	strcat(buf, "You are resting. ");  break;
    case POS_STANDING:
	strcat(buf, "You are standing. ");  break;
    case POS_FIGHTING:
	strcat(buf, "You are fighting! ");  break;
    }
  buf[63]='\0';
  sprintf(buf2,"%s   |%sOTHER: %s%s|\n\r",bold, dim, bold,
	  str_resize(buf,tbuf1,-63));
  leng = str_apd_max( get_string_score_txt, buf2, leng, MAX_STRING_LENGTH);
  buf2[0]='\0';
  for(cnt=0;cnt<MAX_RACE;cnt++)
    if(IS_SHIFT( ch->language, cnt))
    {
      char buffer3[20];
      strcpy(buffer3, race_table[cnt].race_name);
      buffer3[3]='\0';
      strcat(buf2,buffer3);
      strcat(buf2," ");
    }
  sprintf(buf,"%s   |%sKnown: %s%s|\n\r",bold, dim, 
	  str_resize(buf2,tbuf1,-63), bold);
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf( buf, "%s   |%sCurrently Speaking: %s%s|\n\r", bold,dim,bold,
	    str_resize(race_table[UNSHIFT(ch->speak)].race_name,tbuf1,-50));
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);
  sprintf(buf,"%s   |______________________________________________________________________| \n\r",bold);
  leng = str_apd_max( get_string_score_txt, buf, leng, MAX_STRING_LENGTH);

  return;
  }


void do_status( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH],buf1[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  char tbuf1[MAX_INPUT_LENGTH],tbuf2[MAX_INPUT_LENGTH];
  /* AFFECT_DATA *paf;*/
  char bold[10],dim[10];
  int gac;
  int cnt;

  if( IS_NPC( ch))
    return;

  strcpy( dim,  get_color_string( ch, COLOR_SCORE, VT102_DIM) );
  strcpy( bold, get_color_string( ch, COLOR_SCORE, VT102_BOLD) );

  sprintf(buf,"%s  __________________________________________________________________________\n\r",bold);
  send_to_char( buf, ch);
  sprintf(buf,"%s-|__________________________________________________________________________|-\n\r",bold);
  send_to_char( buf, ch);
  sprintf(buf1,"%s%s",ch->name,IS_NPC(ch) ? " the monster" : ch->pcdata->title);
  if(strlen(buf1)>70)
    buf1[70]='\0';
  sprintf(buf,"%s   |%s|\n\r",bold,str_resize(buf1,tbuf1,-70));
  if( strcasecmp( argument, "reincarnate") )
    send_to_char( buf, ch);
  sprintf(buf,"%s   | %s     Stats:  Str:%s%2d%s  Int:%s%2d%s  Wis:%s%2d%s  Dex:%s%2d%s  Con:%s%2d                  |\n\r",bold,dim,bold,
	  get_curr_str(ch),dim,bold,
	  get_curr_int(ch),dim, bold,
	  get_curr_wis(ch),dim, bold,
	  get_curr_dex(ch),dim, bold,
	  get_curr_con(ch));
  send_to_char( buf, ch);
  sprintf(buf1,"%d/%d",ch->carry_weight,can_carry_w(ch));
  sprintf(buf,"%s   |%s  Practices:%s%-3d%s                      Carry Weight:%s%s     |\n\r", bold, dim, bold,ch->practice,dim,bold,
	  str_resize(buf1,tbuf1,-15));
  send_to_char( buf, ch);
  gac = GET_AC(ch)+ch->level*3;
       if ( gac >=  101 ) strcpy( buf1, "WORSE than naked!");
  else if ( gac >=   80 ) strcpy( buf1, "naked.");
  else if ( gac >=   60 ) strcpy( buf1, "wearing clothes.");
  else if ( gac >=   40 ) strcpy( buf1, "slightly armored.");
  else if ( gac >=   20 ) strcpy( buf1, "somewhat armored.");
  else if ( gac >=    0 ) strcpy( buf1, "armored.");
  else if ( gac >= - 20 ) strcpy( buf1, "well armored.");
  else if ( gac >= - 40 ) strcpy( buf1, "strongly armored.");
  else if ( gac >= - 60 ) strcpy( buf1, "heavily armored.");
  else if ( gac >= - 80 ) strcpy( buf1, "superbly armored.");
  else if ( gac >= -100 ) strcpy( buf1, "divinely armored.");
  else                           strcpy( buf1, "invincible!");
/* level > 24 sees ac number */
  if(ch->level>24)
    {
    sprintf(buf,"(%2d)",GET_AC(ch));
    strcat(buf1, buf);
    }
       if ( ch->alignment >  900 ) strcpy( buf2, "angelic.");
  else if ( ch->alignment >  700 ) strcpy( buf2, "saintly.");
  else if ( ch->alignment >  350 ) strcpy( buf2, "good.");
  else if ( ch->alignment >  100 ) strcpy( buf2, "kind.");
  else if ( ch->alignment > -100 ) strcpy( buf2, "neutral.");
  else if ( ch->alignment > -350 ) strcpy( buf2, "mean.");
  else if ( ch->alignment > -700 ) strcpy( buf2, "evil.");
  else if ( ch->alignment > -900 ) strcpy( buf2, "demonic.");
  else                             strcpy( buf2, "satanic.");
  if(ch->level>17)
    {
    sprintf(buf,"(%d)",ch->alignment);
    strcat(buf2, buf);
    }
  sprintf(buf,"%s   |%sArmor Class:%s%s%s  Alignment:%s%s|\n\r",
	  bold,dim,bold,str_resize(buf1,tbuf1,-26),dim,bold,
	  str_resize(buf2,tbuf2,-20));
  if( strcasecmp( argument, "reincarnate") )
    send_to_char( buf, ch);
/* only for level > 14 */
  if(ch->level>14)
    {
    OBJ_DATA *wield;

    wield = get_eq_char(ch, WEAR_WIELD );
    if((!IS_NPC(ch))&&(ch->mclass[CLASS_MONK]>0)&&(wield==NULL))
      {
      int numDice,sizeDice,sizePlus;

      numDice=1;
      sizeDice=4+ch->level/6+ch->mclass[CLASS_MONK]*2/3;
      sizePlus=GET_DAMROLL(ch);

      sprintf(buf1,"%dd%d+%d",numDice,sizeDice,sizePlus);
      }
    else
      sprintf(buf1,"%-3d",GET_DAMROLL(ch));
    sprintf(buf,"%s   |%s    HitRoll:%s%-3d%s                           DamRoll:%s%s         |\n\r",bold,dim,bold,
	    GET_HITROLL(ch),dim,bold,str_resize(buf1,tbuf1,-11));
  if( strcasecmp( argument, "reincarnate") )
    send_to_char( buf, ch);
    }

  buf[0]='\0';
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
    strcat(buf,"You are drunk. ");
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    strcat(buf,"You are thirsty. ");
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
    strcat(buf,"You are hungry. ");
  if(IS_NPC(ch))
    strcat(buf,"You are an NPC! ");
  switch ( ch->position )
    {
    case POS_DEAD:     
	strcat(buf, "You are DEAD!! ");  break;
    case POS_MORTAL:
	strcat(buf, "You are mortally wounded! ");  break;
    case POS_INCAP:
	strcat(buf, "You are incapacitated! ");  break;
    case POS_STUNNED:
	strcat(buf, "You are stunned! ");  break;
    case POS_SLEEPING:
	strcat(buf, "You are sleeping. ");  break;
    case POS_RESTING:
	strcat(buf, "You are resting. ");  break;
    case POS_STANDING:
	strcat(buf, "You are standing. ");  break;
    case POS_FIGHTING:
	strcat(buf, "You are fighting! ");  break;
    }
  buf[63]='\0';
  sprintf(buf2,"%s   |%s      OTHER: %s%s|\n\r",bold, dim, bold,
	  str_resize(buf,tbuf1,-57));
  if( strcasecmp( argument, "reincarnate") )
    send_to_char( buf2, ch);
  sprintf(buf,"%s   |%s    Class:%s%s%s  Age:%s%3d%s yrs (%05d hrs) RACE:%s %s|\n\r"
	  ,bold,dim,bold,
	  str_resize(class_table[ch->class].who_name_long,tbuf1,-20),
	  dim,bold,
	  get_age(ch),dim,
	  (ch->played)/3600 + (IS_NPC(ch)?0:ch->pcdata->previous_hours),
	  bold,str_resize(race_table[ch->race].race_name,tbuf2,-8) );
  if( !strcasecmp( argument, "reincarnate") )
    send_to_char( buf, ch);
  buf2[0]='\0';
  for(cnt=0;cnt<MAX_RACE;cnt++)
    if(IS_SHIFT( ch->language, cnt))
      {
      strcat(buf2,race_table[cnt].race_name);
      strcat(buf2," ");
      }
  sprintf(buf,"%s   |%sKnown: %s%s|\n\r",bold, dim, 
	  str_resize(buf2,tbuf1,-63), bold);
  if( !strcasecmp( argument, "reincarnate") )
    send_to_char( buf, ch);

  if (IS_IMMORTAL(ch))
  {
        ch_printf(ch, "%s   |%sIMMORTAL DATA:  %sWizinvis [%s]\n\r",
                bold, dim, bold, IS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ");

        ch_printf(ch, "%s   |%sBamfin:%s  %s %s.\n\r", bold, dim, bold, ch->name, (ch->pcdata->bamfin[0] != '\0')
                ? ch->pcdata->bamfin : "appears in a swirling mist.");
        ch_printf(ch, "%s   |%sBamfout:%s %s %s.\n\r", bold, dim, bold,ch->name, (ch->pcdata->bamfout[0]
 != '\0')
                ? ch->pcdata->bamfout : "leaves in a swirling mist.");


        if (ch->pcdata->area)
        {
            ch_printf(ch, "%s   |%sVnums:   Room%s (%-5.5d - %-5.5d)   %sObject%s (%-5.5d - %-5.5d)   %sMob%s (%-5.5d - %-5.5d)\n\r",
		bold, dim,bold,
                ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
		dim, bold,
                ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
		dim, bold,
                ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
	}
  }
  sprintf(buf,"%s   |______________________________________________________________________| \n\r",bold);
  send_to_char( buf, ch);
  return;
}




char *  const   day_name        [] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *  const   month_name      [] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};


  /* Enhanced AFFECTS command  -  Chaos  4/3/99  */
void do_affects( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];

  get_affects_string( ch, ch, buf, MAX_STRING_LENGTH );
  send_to_char( buf, ch );

  return;
  }

void get_affects_string( CHAR_DATA *ch, CHAR_DATA *viewer, char *outbuf,
      int outbufleng )
  {
  char buf[MAX_INPUT_LENGTH],buf1[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  int oleng, slen;
  char tbuf1[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  char bold[10],dim[10];

    /* Universal code for color shifting, regardless of terminal type */
  strcpy( dim,  get_color_string( viewer, COLOR_SCORE, VT102_DIM) );
  strcpy( bold, get_color_string( viewer, COLOR_SCORE, VT102_BOLD) );

  sprintf(buf,"%s  __________________________________________________________________________\n\r",bold);
  oleng = str_cpy_max( outbuf, buf, outbufleng );
  sprintf(buf,"-|__________________________________________________________________________|-\n\r");
  oleng = str_apd_max( outbuf, buf, oleng, outbufleng );
  sprintf(buf1,"%s%s",ch->name,IS_NPC(ch) ? " the monster" : ch->pcdata->title);
  if(strlen(buf1)>70)
    buf1[70]='\0';
  sprintf(buf,"   |%s|\n\r",str_resize(buf1,tbuf1,-70));
  oleng = str_apd_max( outbuf, buf, oleng, outbufleng );
    sprintf(buf,"   |%sAFFECTS:                                                              %s|\n\r", dim, bold);
  oleng = str_apd_max( outbuf, buf, oleng, outbufleng );
  if ( ch->first_affect != NULL )
    {
    for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
      {
      sprintf( buf, "Spell: '%s%s%s'", bold,skill_table[paf->type].name,dim );
      slen = strlen( skill_table[paf->type].name ) + 9;
      if ( ch->level >= 30 )
	{
	sprintf( buf2, " modifies %s by %d for %d hours", 
            affect_loc_name( paf->location ), paf->modifier, paf->duration );
        slen += strlen( buf2 );
	strcat(buf,buf2);
	}
      for( *buf2='\0'; slen < 68; slen++)
        strcat( buf2, " " );
      sprintf(buf1,"   |%s  %s%s%s|\n\r",dim,buf,buf2,bold);
      oleng = str_apd_max( outbuf, buf1, oleng, outbufleng );
      }
    }
  else
    {
    strcpy( buf, "   |  none.                                                               |\n\r");
      oleng = str_apd_max( outbuf, buf, oleng, outbufleng );
    }
   sprintf(buf,"   |______________________________________________________________________| \n\r");
      oleng = str_apd_max( outbuf, buf, oleng, outbufleng );

  return;
  }

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d %s, the %d%s day of the Month of %s.\n\rMrMud started up at %s\rThe system time is %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	/*day_name[day % 7], */
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
        sprintf( buf, "The temperature is in the %d0's.\n\r",
              ch->in_room->area->weather_info.temperature / 10 );
        send_to_char( buf, ch );
	return;
    }

    if( ch->in_room->area->weather_info.temperature < 32 )
      switch( ch->in_room->area->weather_info.sky )
        {
        case 0: strcpy( buf, "The sky is clear" ); break;
        case 1: strcpy( buf, "The sky is a bit frosty" ); break;
        case 2: strcpy( buf, "It is snowing" ); break;
        case 3: strcpy( buf, "You are in a blizzard" ); break;
        }
    else
    if( ch->in_room->area->weather_info.temperature < 34 )
      switch( ch->in_room->area->weather_info.sky )
        {
        case 0: strcpy( buf, "The sky is clear" ); break;
        case 1: strcpy( buf, "The sky is chilling" ); break;
        case 2: strcpy( buf, "It is sleeting" ); break;
        case 3: strcpy( buf, "You are in a hail storm" ); break;
        }
    else
      switch( ch->in_room->area->weather_info.sky )
        {
        case 0: strcpy( buf, "The sky is clear" ); break;
        case 1: strcpy( buf, "The sky is cloudy" ); break;
        case 2: strcpy( buf, "It is raining" ); break;
        case 3: strcpy( buf, "You are in a storm" ); break;
        }


     if( ch->in_room->area->weather_info.change > 50 )
       strcat( buf, " and a southernly breeze blows" );
     if( ch->in_room->area->weather_info.change < -50 )
       strcat( buf, " and a northern gust blows" );

    strcat( buf, ".\n\r" );
    send_to_char( buf, ch );

    if( ch->in_room->area->weather_info.temperature < 0 )
      sprintf( buf, "The temperature is in the %d0's.\n\r",
          ch->in_room->area->weather_info.temperature / 10 - 1 );
    else
      sprintf( buf, "The temperature is in the %d0's.\n\r",
          ch->in_room->area->weather_info.temperature / 10 );
    send_to_char( buf, ch );

    if( ch->level < 99 )
      return;

    sprintf( buf, "Sky: %d\n\rChange: %d\n\rmmhg: %d\n\rSunlight: %d\n\rTemperature: %d\n\r",
        ch->in_room->area->weather_info.sky,
        ch->in_room->area->weather_info.change,
        ch->in_room->area->weather_info.mmhg,
        ch->in_room->area->weather_info.sunlight,
        ch->in_room->area->weather_info.temperature);
    send_to_char( buf, ch );

    return;
}

HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char argnew[MAX_INPUT_LENGTH];
    HELP_DATA *pHelp;
    int lev;

    if ( isdigit((int)argument[0]) )
    {
        lev = number_argument( argument, argnew );
        argument = argnew;
    }
    else
        lev = -2;
    /*
     * Tricky argument handling so 'help a b' doesn't match a.
     */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );
        if ( argall[0] != '\0' )
            strcat( argall, " " );
        strcat( argall, argone );
    }

    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
        if ( pHelp->level > get_trust( ch ) )
            continue;
        if ( lev != -2 && pHelp->level != lev )
            continue;

        if ( is_name( argall, pHelp->keyword ) )
            return pHelp;
    }

    return NULL;
}


void do_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *pHelp;
    bool found;

    if ( argument[0] == '\0' )
	argument = "mainmenu";

    if( IS_NPC( ch ) )
      return;

    if ( (pHelp = get_help( ch, argument )) == NULL )
    {
        send_to_char( "No help on that word.\n\r", ch );
        return;
    }

    found=FALSE;
    buf[0]='\0';

    send_to_char_color("{300}", ch);

    /*
     * Strip leading '.' to allow initial blanks.
     */
    ch->pcdata->prev_help=ch->pcdata->last_help;
    ch->pcdata->last_help=pHelp;
    ch->pcdata->help_mode=TRUE;

    if ( pHelp->text[0] == '.' )
      send_to_char_color( pHelp->text+1,ch);
    else
      send_to_char_color( pHelp->text,ch);

  return;
}


void do_multi( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf_race[20],buf_lang[80];
    char buf2[MAX_STRING_LENGTH], buf_name[40];
    CHAR_DATA *fch;
    DESCRIPTOR_DATA *d;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int nTotal;
    bool fNameRestrict;
    bool fImmortalOnly, tong, under;
    int  cnt, lrang, hrang;
	CHAR_DATA *wch;
	char const *class;
    bool follower;
    bool enemy;
    PLAYER_GAME *fpl, *matchpl;
    int leng;
 
    /*
     * Set default arguments.
     */
    fch = NULL;
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fNameRestrict = FALSE;
    fImmortalOnly  = FALSE;
    lrang=ch->level*3/4-2;
    hrang=ch->level*5/4+2;
    follower       = FALSE;
    enemy          = FALSE;
    matchpl=NULL;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;
   
    if( !strcasecmp( arg, "god"))
	follower=TRUE;
    enemy=!strcasecmp(arg,"enemy");
	if( !strcasecmp( arg, "all") || follower || enemy )
	  {
	  lrang=0;
	  hrang=100;
	  arg[0]='\0';
	  break;
	  }

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atol(arg);lrang=iLevelLower;hrang=MAX_LEVEL;break;
	    case 2: iLevelUpper = atol(arg);hrang=iLevelUpper;break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{
	    /*
	     * Look for classes to turn on.
	     */
	    if ( !strcasecmp( arg, "imm" ) )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {     
		lrang=0;
		hrang=100;
                for( fpl=first_player; fpl!=NULL && !fNameRestrict; 
                        fpl=fpl->next)
                  {
                  fch = fpl->ch;
		    if ( is_name( arg, fch->name ) )
		      {
  		      if( IS_SET( fch->act, PLR_WIZINVIS) 
			  && ch->level<(MAX_LEVEL-3))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
  			if(fch!=ch && is_affected(fch, gsn_greater_stealth) 
			   && ( !is_affected(ch, gsn_truesight)
	                   || ch->mclass[CLASS_ILLUSIONIST] < fch->level )
			   && !IS_SET(ch->act, PLR_HOLYLIGHT))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
		      fNameRestrict = TRUE;
		      matchpl = fpl;
		      break;
		      }
                   }
	       if(!fNameRestrict)
                for( fpl=first_player; fpl!=NULL && !fNameRestrict; 
                        fpl=fpl->next)
                  {
                  fch = fpl->ch;
		    if ( is_name_short( arg, fch->name ) )
		      {
  		      if( IS_SET( fch->act, PLR_WIZINVIS) 
			  && ch->level<(MAX_LEVEL-3))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
  			if(fch!=ch && is_affected(fch, gsn_greater_stealth) 
			   && ( !is_affected(ch, gsn_truesight)
	                   || ch->mclass[CLASS_ILLUSIONIST] < fch->level )
			   && !IS_SET(ch->act, PLR_HOLYLIGHT))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
		      fNameRestrict = TRUE;
		      matchpl = fpl;
		      break;
		      }
		   }
		if(!fNameRestrict)
		  {
		  send_to_char( "No one has that name.\n\r", ch);
		  return;
		  }
	    }
	}
    }

    /*
     * Now show matching chars.
     */

    nMatch = 0;
    nTotal = 0;
    buf[0] = '\0';
    leng = 0;
   for( fpl=first_player; fpl!=NULL; fpl=fpl->next)
    {
    wch=fpl->ch;
    d=NULL;
    if(is_desc_valid( wch))
      d=wch->desc;
  if( IS_SET( wch->act, PLR_WIZINVIS) && ch->level<(MAX_LEVEL-3))
    continue;
  if (wch!=ch && is_affected(wch, gsn_greater_stealth) &&
     !((is_affected(ch, gsn_truesight) && ch->mclass[CLASS_ILLUSIONIST] >= wch->level) || IS_SET(ch->act, PLR_HOLYLIGHT)))
    continue;

  nTotal++;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_HERO )
	|| ( fNameRestrict && fpl!=matchpl) 
  || ( wch->level < lrang)
  || ( wch->level > hrang)
  || ( follower && ((which_god( ch) != which_god( wch))))
  || ( enemy && ((which_god( ch) == which_god( wch)) ||
                  which_god(wch) == GOD_NEUTRAL)))
      continue;
	nMatch++;
     

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	default: break;
	case MAX_LEVEL - 0: class = "GOD"; break;
	case MAX_LEVEL - 1: class = "DUK"; break;
	case MAX_LEVEL - 2: if (which_god(wch) == GOD_POLICE)
			     class = "COP";
			    else
			     class = "COU"; break;
	/*case MAX_LEVEL - 3: class = "ANG"; break;*/
	}

	strcpy(buf_race,race_table[wch->race].race_name);
	buf_race[3]='\0';

	cnt=0;
	/* get rid of stinking ESC sequence for nonVT100 -Dug 12/2/93 */
	buf_lang[0]='\0';
	tong           = FALSE;
	under          = FALSE;

    sprintf( buf_lang,"Ill:%2d  Ele:%2d  Rog:%2d  Ran:%2d  Nec:%2d  Mon:%2d  Asn:%2d",
	  wch->mclass[0],
	  wch->mclass[1],
	  wch->mclass[2],
	  wch->mclass[3],
	  wch->mclass[4],
	  wch->mclass[5],
	  wch->mclass[6]);
	/*
	 * Format it up.
	 */
  strcpy(buf_name, wch->name);
  buf_name[11]='\0';
  while(strlen(buf_name)<11)
    strcat( buf_name, " ");
	sprintf( buf2, "%s%s%s [%2d %s %s] %s%s",
	  ch->vt100==1 ? "\033[1m" : "" ,
	  buf_name,
	  ch->vt100==1 ? "\033[m" : "" ,
		wch->level,
		class,
	  buf_race,
	  ch->vt100==1 ? "\033[1m"  : "" ,
	  buf_lang);
       leng = str_apd_max( buf, buf2, leng, MAX_STRING_LENGTH );
       leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
    }

    sprintf( buf2, "%d/%d players.  %s\n\r", nMatch, nTotal,
     fNameRestrict || lrang!=0 || hrang!=100 ? "This is a partial list." : "");
    send_to_char( buf2, ch );
    send_to_char( buf, ch );
    return;
}
/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void doit_who( CHAR_DATA *, char *, int );
void do_language( CHAR_DATA *ch, char *argument )
  {
  doit_who( ch, argument, 1);
  return;
  }
void do_who( CHAR_DATA *ch, char *argument )
  {
  doit_who( ch, argument, 0);
  return;
  }

void doit_who( CHAR_DATA *ch, char *argument, int typ )
{
    char buf[MAX_STRING_LENGTH],buf_race[20],buf_lang[100];
    char buf2[MAX_STRING_LENGTH];
    char pbuf[MAX_STRING_LENGTH];
    char *pt;
    int plen;
    CHAR_DATA *fch;
    DESCRIPTOR_DATA *d;
    CLAN_DATA *nc;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int nTotal;
    bool fClass, fRace;
    bool fClan, fOrder;
    CLAN_DATA * iClan = NULL;
    int iRaceClass;
    bool fNameRestrict;
    bool fImmortalOnly, tong, under;
    int  cnt, lrang, hrang;
	CHAR_DATA *wch;
	char const *class;
    bool follower;
    bool enemy,target;
    PLAYER_GAME *fpl,*matchpl;
 
    /*
     * Set default arguments.
     */
    matchpl=NULL;
    *buf='\0';
    *buf_race='\0';
    *buf_lang='\0';
    *buf2='\0';
    nc=NULL;
    fch = NULL;
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fNameRestrict = FALSE;
    fImmortalOnly  = FALSE;
    fRace = FALSE;
    fClass = FALSE;
    fClan = FALSE;
    fOrder = FALSE;
    iRaceClass = -1;
    lrang=ch->level*3/4-2;
    hrang=ch->level*5/4+2;
    follower       = FALSE;
    enemy          = FALSE;
    target         = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    plen=0;
    pbuf[0]='\0';
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;
   
    follower=!strcasecmp( arg, "god");
    enemy=!strcasecmp( arg, "enemy");
    target=!strcasecmp( arg, "target");

    if( ( fClan = !strcasecmp( arg, "clan" ) ) == TRUE )
      {
      bool found=FALSE;
      if (strlen(argument)>=2)
       for (iClan= first_clan;iClan!=NULL;iClan=iClan->next)
	{
	 if (!str_prefix (argument, iClan->name)
	     && iClan->type ==CLAN_WARLIKE)
	 { 
	  found=TRUE; 
	  break;
 	 }
	}
      if( !found)
        {
        if (!IS_NPC(ch) && ch->pcdata->clan !=NULL )
	 {
          if (ch->pcdata->clan->type==CLAN_WARLIKE) 
 	   iClan = ch->pcdata->clan;
	  else
	   {
            send_to_char( "You are not in a clan, and did not specify a clan to list.\n\r", ch );
            return;
	   }
	 }
        else
 	 {
          send_to_char( "That clan does not exist.\n\r", ch );
          return;
         }
        }
      }
    if( ( fOrder = !strcasecmp( arg, "order" ) ) == TRUE )
      {
      bool found=FALSE;
      if (strlen(argument)>=2)
        for (iClan= first_clan;iClan!=NULL;iClan=iClan->next)
	{
	 if (!str_prefix (argument, iClan->name)
	     && iClan->type ==CLAN_PEACEFUL )
	 { found=TRUE; break;}
	}
      if( !found)
        {
        if (!IS_NPC(ch) && ch->pcdata->clan)
	 {
          if (ch->pcdata->clan->type==CLAN_PEACEFUL) 
 	   iClan = ch->pcdata->clan;
	  else
	   {
            send_to_char( "You are not in an order, and did not specify an order to list.\n\r", ch );
            return;
	   }
	 }
        }
      }
    if( ( fRace = !strcasecmp( arg, "race" ) ) == TRUE )
      {
      if (strlen(argument)>=2)
        iRaceClass = lookup_race( argument );
      else
	iRaceClass = ch->race;
      }
    if( ( fClass = !strcasecmp( arg, "class" ) ) == TRUE )
      {
      iRaceClass = -1;
      if (strlen(argument)>=2)
      {
       for(cnt=0;cnt<MAX_CLASS;cnt++)
        if(!str_prefix(argument, class_table[cnt].who_name_long ))
	   iRaceClass = cnt;
      }
      else
	iRaceClass=ch->class;
      }

    if( (!strcasecmp( arg, "all")) || target || follower || enemy || fClan
				   || fRace || fOrder || fClass )
      {
      lrang=0;
      hrang=100;
      arg[0]='\0';
      break;
      }

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atol(arg);lrang=iLevelLower;hrang=MAX_LEVEL;break;
	    case 2: iLevelUpper = atol(arg);hrang=iLevelUpper;break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{
	    /*
	     * Look for classes to turn on.
	     */
	    if ( !strcasecmp( arg, "imm" ) )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {     
		lrang=0;
		hrang=100;
                if( !enemy && !follower )
                for( fpl=first_player; fpl!=NULL && !fNameRestrict; 
                        fpl=fpl->next)
                  {
                  fch = fpl->ch;
		    if ( is_name( arg, fch->name ) )
		      {
  		      if( IS_SET( fch->act, PLR_WIZINVIS) 
			  && ch->level<(MAX_LEVEL-3))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
  			if(fch!=ch && is_affected(fch, gsn_greater_stealth) 
			   && ( !is_affected(ch, gsn_truesight)
	                   || ch->mclass[CLASS_ILLUSIONIST] < fch->level )
			   && !IS_SET(ch->act, PLR_HOLYLIGHT))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
		      fNameRestrict = TRUE;
		      matchpl = fpl;
		      break;
		      }
		    }
	       if(!fNameRestrict)
                if( !enemy && !follower )
                for( fpl=first_player; fpl!=NULL && !fNameRestrict; 
                        fpl=fpl->next)
                  {
                  fch = fpl->ch;
		    if ( is_name_short( arg, fch->name ) )
		      {
  		      if( IS_SET( fch->act, PLR_WIZINVIS) 
			  && ch->level<(MAX_LEVEL-3))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
  			if(fch!=ch && is_affected(fch, gsn_greater_stealth) 
			   && ( !is_affected(ch, gsn_truesight)
	                   || ch->mclass[CLASS_ILLUSIONIST] < fch->level )
			   && !IS_SET(ch->act, PLR_HOLYLIGHT))
		       {
		  	send_to_char( "No one has that name.\n\r", ch);
		  	return;
		       }
		      fNameRestrict = TRUE;
		      matchpl = fpl;
		      break;
		      }
		    }
		if(!fNameRestrict)
		  {
		  send_to_char( "No one has that name.\n\r", ch);
		  return;
		  }
	    }
	}
    }

    /*
     * Now show matching chars.
     */


    nMatch = 0;
    nTotal = 0;
    buf[0] = '\0';
    plen = 0;
   for( fpl=first_player; fpl!=NULL; fpl=fpl->next)
    {
    wch=fpl->ch;
    d=NULL;
    if(is_desc_valid( wch))
      d=wch->desc;
	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	      Chaos set to see all chars, invis or not.
	 */
  if( IS_SET( wch->act, PLR_WIZINVIS) && ch->level<(MAX_LEVEL-3))
    continue;

  if (wch!=ch && is_affected(wch, gsn_greater_stealth) && 
     !((is_affected(ch, gsn_truesight) && ch->mclass[CLASS_ILLUSIONIST] >= wch->level) || IS_SET(ch->act, PLR_HOLYLIGHT)))
    continue;

  nTotal++;
	if( wch->level < iLevelLower
           ||   wch->level > iLevelUpper
           || ( fImmortalOnly  && wch->level < LEVEL_HERO )
           || ( fNameRestrict && fpl!=matchpl) 
           || ( wch->level < lrang)
           || ( wch->level > hrang)
           || ( fRace && wch->race != iRaceClass )
           || ( fClass && wch->class != iRaceClass )
           || ( (fClan || fOrder) && !IS_NPC(wch) && wch->pcdata->clan != iClan )
           || ( follower && ((which_god( ch) != which_god( wch))))
      	   || ( target && (!(can_attack (ch, wch)) || wch==ch))
           || ( enemy && ((which_god(ch)==which_god(wch)) ||
                           which_god(wch)==GOD_INIT_ORDER ||
                           which_god(wch)==GOD_INIT_CHAOS ||
                           which_god(wch)==GOD_NEUTRAL)) )
          continue;
	nMatch++;
     

        plen=str_apd_max( pbuf, "{030}[", plen, MAX_STRING_LENGTH);
        sprintf(buf_race,"%2d", wch->level);
        plen=str_apd_max( pbuf, buf_race, plen, MAX_STRING_LENGTH);
        plen=str_apd_max( pbuf, " ", plen, MAX_STRING_LENGTH);

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	default: break;
	case MAX_LEVEL - 0: class = "{120}GOD{030}"; break;
	case MAX_LEVEL - 1: class = "{120}DUK{030}"; break;
	case MAX_LEVEL - 2: if (which_god(wch) == GOD_POLICE)
			     class = "{120}COP{030}";
			    else
			     class = "{120}COU{030}"; break;
	case MAX_LEVEL - 3: class = "{120}LRD{030}"; break;
	}
        plen=str_apd_max( pbuf, class, plen, MAX_STRING_LENGTH);

        plen=str_apd_max( pbuf, " ", plen, MAX_STRING_LENGTH);
	strcpy(buf_race,race_table[wch->race].race_name);
	buf_race[3]='\0';
	if(wch->level>MAX_LEVEL-4)
	  strcpy(buf_race,"---");
        if( wch->pcdata->army_status == 1 )
          {
          plen=str_apd_max( pbuf, "{140}", plen, MAX_STRING_LENGTH);
          if( *buf_race >='a' && *buf_race <= 'z' )
            *buf_race -= ('a' - 'A');
          if( *(buf_race+1) >='a' && *(buf_race+1) <= 'z' )
            *(buf_race+1) -= ('a' - 'A');
          if( *(buf_race+2) >='a' && *(buf_race+2) <= 'z' )
            *(buf_race+2) -= ('a' - 'A');
          }
        plen=str_apd_max( pbuf, buf_race, plen, MAX_STRING_LENGTH);
        if( wch->pcdata->army_status == 1 )
          plen=str_apd_max( pbuf, "{030}", plen, MAX_STRING_LENGTH);
        plen=str_apd_max( pbuf, "]", plen, MAX_STRING_LENGTH);

	switch(which_god(wch))
	  {
	  case GOD_INIT_ORDER: 
            plen=str_apd_max( pbuf, "{060}o{020}<", plen, MAX_STRING_LENGTH);
            break;
	  case GOD_INIT_CHAOS: 
            plen=str_apd_max( pbuf, "{030}c{020}<", plen, MAX_STRING_LENGTH);
            break;
	  case GOD_ORDER: 
            plen=str_apd_max( pbuf, "{160}O{020}<", plen, MAX_STRING_LENGTH);
            break;
	  case GOD_CHAOS: 
            plen=str_apd_max( pbuf, "{130}C{020}<", plen, MAX_STRING_LENGTH);
            break;
	  case GOD_DEMISE: 
            plen=str_apd_max( pbuf, "{110}D{020}<", plen, MAX_STRING_LENGTH);
            break;
	  case GOD_POLICE: 
            plen=str_apd_max( pbuf, "{150}E{020}<", plen, MAX_STRING_LENGTH);
            break;
	  default: 
            plen=str_apd_max( pbuf, "-{020}<", plen, MAX_STRING_LENGTH);
            break;
	  }

	cnt=0;
	tong           = FALSE;
	under          = FALSE;
	if( wch->level >= 90 || IS_AFFECTED( wch, AFF_TONGUES))
	  tong=TRUE;
	if( IS_AFFECTED( wch, AFF_UNDERSTAND) && wch->level <= MAX_LEVEL-5 )
	  under=TRUE;
	if(tong)
          {
          if( typ==0 )
            plen=str_apd_max( pbuf, "{120}All{020}", plen, MAX_STRING_LENGTH);
          else
            plen=str_apd_max( pbuf, "{120}", plen, MAX_STRING_LENGTH);
          }
       if(typ==1)
        {
	for(cnt=0;cnt<MAX_RACE;cnt++)
	  if(IS_SHIFT(wch->language,cnt) || under)
	    {
	    if(IS_SHIFT(wch->speak,cnt)  && !tong)
                plen=str_apd_max( pbuf, "{120}", plen, MAX_STRING_LENGTH);
	    strcpy(buf2,race_table[cnt].race_name);
	    if(IS_SHIFT(wch->speak,cnt))
	      {
	      if(ch->vt100!=1)
                {
		/*if(buf2[2]>'Z')
		  buf2[2]-=('a'-'A');*/
		if(buf2[1]>'Z')
		  buf2[1]-=('a'-'A');
                }
	      }
	    else
	    if(!IS_SHIFT(wch->language,cnt))
		{
		/*buf2[2]='.';*/
		buf2[1]='.';
		buf2[0]+=('a'-'A');
		}
	    /*buf2[3]='\0';*/
	    buf2[2]='\0';
            plen=str_apd_max( pbuf, buf2, plen, MAX_STRING_LENGTH);

	    if(IS_SHIFT(wch->speak,cnt)  && !tong)
                plen=str_apd_max( pbuf, "{020}", plen, MAX_STRING_LENGTH);
	    }
	  else
            /*plen=str_apd_max( pbuf, " | ", plen, MAX_STRING_LENGTH);*/
            plen=str_apd_max( pbuf, " |", plen, MAX_STRING_LENGTH);
        plen=str_apd_max( pbuf, "{020}", plen, MAX_STRING_LENGTH);
        }
      else
       if( !tong )
        {
	for(cnt=0;cnt<MAX_RACE;cnt++)
	  if(IS_SHIFT(wch->speak,cnt))
            break;

        strcpy(buf2,race_table[cnt].race_name);
	      if(ch->vt100!=1)
                {
		if(buf2[2]>'Z')
		  buf2[2]-=('a'-'A');
		if(buf2[1]>'Z')
		  buf2[1]-=('a'-'A');
                }
        buf2[3]='\0';
        plen=str_apd_max( pbuf, buf2, plen, MAX_STRING_LENGTH);
        }

       if(  IS_SET(wch->act, PLR_KILLER) )
         plen=str_apd_max( pbuf, ">{010}K{170}", plen, MAX_STRING_LENGTH);
       else
       if(  IS_SET(wch->act, PLR_THIEF) )
         plen=str_apd_max( pbuf, ">{020}T{170}", plen, MAX_STRING_LENGTH);
       else
       if( IS_SET(wch->pcdata->player2_bits, PLR2_AFK))
         plen=str_apd_max( pbuf, ">{040}A{170}", plen, MAX_STRING_LENGTH);
       else
         plen=str_apd_max( pbuf, "> {170}", plen, MAX_STRING_LENGTH);

      if( typ==0 )
        {
        if( IS_NPC(wch) )
          strcpy( buf, "the monster" );
        else
          strcpy( buf, wch->pcdata->title );
        for( pt=buf; *pt!='\0'; pt++)
          {
          if( *pt=='{' )
            *pt='[';
          else
          if( *pt=='}' )
            *pt=']';
          }
        sprintf( buf2,"%s{070}%s                                                                      ",wch->name,
            buf);
        buf2[52]='\0';
        }
      else
        {  
          if (!IS_NPC(wch) && wch->pcdata->clan_name[0] != '\0')
	  {
            sprintf(buf2, "%-15s {002}%-15s                                                                            ",wch->name, wch->pcdata->clan_name);
            buf2[31]='\0';
	  }
 	  else
	  {
            sprintf(buf2, "%-15s                                                                             ",wch->name);
            buf2[26]='\0';
	  }
        }

      plen=str_apd_max( pbuf, buf2, plen, MAX_STRING_LENGTH);
      
      plen=str_apd_max( pbuf, "{134}", plen, MAX_STRING_LENGTH);
       if( (wch->pcdata->switched || wch->desc !=NULL) &&
               !IS_AFFECTED( wch, AFF_STEALTH ))
	 {
         sprintf(buf2, "%-14s", wch->in_room->area->name);
	 buf2[13]='\0';
	 }
       else
         if( wch->desc == NULL )
	   strcpy(buf2, "  LinkLost   ");
         else
	   strcpy(buf2, "  Unknown    ");  /* Stealth Mode */

      plen=str_apd_max( pbuf, buf2, plen, MAX_STRING_LENGTH);
      plen=str_apd_max( pbuf, "{300}", plen, MAX_STRING_LENGTH);
      plen=str_apd_max( pbuf, "\n\r", plen, MAX_STRING_LENGTH);
    }

    sprintf( buf2, "{000}{030}%d/%d players.  %s{030}\n\r", nMatch, nTotal,
     fNameRestrict || lrang!=0 || hrang!=100 ? "This is a partial list." : "");

    plen=str_apd_max( pbuf, buf2, plen, MAX_STRING_LENGTH);
    send_to_char( ansi_translate_text( ch, pbuf), ch );
    return;
}



void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, 0, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;
    int leng;
    char buf[MAX_STRING_LENGTH], dim[10], bold[10], buf4[10];

    if(!IS_NPC(ch) && ch->ansi==0 && ch->vt100==1)
      {
      strcpy(dim,"\033[m");
      strcpy(bold,"\033[1m");
      }
    else
    if(!IS_NPC(ch) && ch->ansi==1)
      {
      sprintf(dim,"\033[0;%d;%dm", ch->pcdata->color[0][10], 
	  ch->pcdata->color[1][10]);
      sprintf(bold,"\033[1;%d;%dm", ch->pcdata->color[0][10], 
	  ch->pcdata->color[1][10]);
      }
    else
      {
      strcpy(dim,"");
      strcpy(bold,"");
      }
    leng = str_cpy_max( buf, dim, MAX_STRING_LENGTH );
    leng = str_apd_max( buf, "You are using:\n\r", leng, MAX_STRING_LENGTH );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;
	leng = str_apd_max(buf,where_name[iWear], leng, MAX_STRING_LENGTH );
	if ( can_see_obj( ch, obj ) )
	{
            if(( !obj->basic || obj->item_type==ITEM_WEAPON ||
                obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_CONTAINER )
                && CAN_WEAR( obj, ITEM_TAKE ) &&
     !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF ) )
              {
              *buf4 = '0'+(obj->index_reference[0]/100)%10;
              *(buf4+1) = '0'+(obj->index_reference[0]/10)%10;
              *(buf4+2) = '0'+(obj->index_reference[0])%10;
              *(buf4+3) = '\0';
	      leng = str_apd_max(buf, "#", leng, MAX_STRING_LENGTH);
	      leng = str_apd_max(buf, buf4, leng, MAX_STRING_LENGTH);
	      leng = str_apd_max(buf, " ", leng, MAX_STRING_LENGTH);
              }
            else
	      leng = str_apd_max(buf, "     ", leng, MAX_STRING_LENGTH);
	    leng = str_apd_max(buf, bold, leng, MAX_STRING_LENGTH);
	    leng = str_apd_max(buf,format_obj_to_char( obj, ch, 0 ),
                                          leng, MAX_STRING_LENGTH);
	    leng = str_apd_max(buf, dim, leng, MAX_STRING_LENGTH);
	    leng = str_apd_max(buf, "\n\r", leng, MAX_STRING_LENGTH);
	}
	else
	{
	    leng = str_apd_max( buf, "something.\n\r", leng, MAX_STRING_LENGTH);
	}
	found = TRUE;
    }

    if ( !found )
	leng = str_apd_max(buf, "Nothing.\n\r", leng, MAX_STRING_LENGTH);
    send_to_char( buf, ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
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
	    send_to_char( "You aren't wearing anything comparable.\n\r", ch );
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
	    
    msg         = NULL;
    value1      = 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
    value1 = compare_obj( obj1, obj2 );
    }

    if ( msg == NULL )
    {
	     if ( value1 == 0 ) msg = "$p and $P look about the same.";
	else if ( value1  > 0 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}

int compare_obj( OBJ_DATA *obj1, OBJ_DATA *obj2)
  {
  int value1, value2;
	switch ( obj1->item_type )
	{
	default:
	    return( FALSE );
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:   /* the val[3] is type...why affect value? -dug*/
	    value1 = obj1->value[1] * obj1->value[2] /* + obj1->value[3]*/;
	    value2 = obj2->value[1] * obj2->value[2] /* + obj2->value[3]*/;
	    break;

/* added value of ammo is DAM+HITROLL-SPEED -dug */
	case ITEM_AMMO:
	    value1 = obj1->value[1] + obj1->value[2] - obj1->value[3];
	    value2 = obj2->value[1] + obj2->value[2] - obj2->value[3];
	    break;
      }
  return( value1 - value2 );
  }


void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], buf4[MAX_STRING_LENGTH],buf5[MAX_INPUT_LENGTH];
    char tbuf1[MAX_INPUT_LENGTH],tbuf2[MAX_INPUT_LENGTH],
	 tbuf3[MAX_INPUT_LENGTH],tbuf4[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    /* DESCRIPTOR_DATA *d; */
    bool found,doAll;
    int test;
    PLAYER_GAME *fpl;
    int leng;
    char hoster[MAX_INPUT_LENGTH];
    char sss[MAX_STRING_LENGTH];

    test=0;
    victim=NULL;
    one_argument( argument, arg );
    doAll=!strcasecmp(arg,"all");
    *sss = '\0';
    leng = 0;

    if ( doAll )
	    leng = str_apd_max( sss, "List of All Players.\n\r", 
                                      leng, MAX_STRING_LENGTH);
    if ( arg[0] != '\0' && !doAll )
	    leng = str_apd_max( sss, "List of Specific Name.\n\r", 
                                      leng, MAX_STRING_LENGTH );
    if ( arg[0] == '\0' && !doAll )
	    leng = str_apd_max( sss, "List of Players in your Area.\n\r", 
                                      leng, MAX_STRING_LENGTH);

    if ( arg[0] == '\0' || doAll )
    {
	leng = str_apd_max( sss, "Player___________Location__________________Leader____________Address__________\n\r", leng, MAX_STRING_LENGTH);
	found = FALSE;
       for( fpl = first_player; fpl!= NULL; fpl = fpl->next )
	/* for ( d = first_descriptor; d!=NULL ; d = d->next ) */
	{
	  /*   Change to character list, not descriptors.  Chaos - 2/15/95
         if(d->character==NULL)      
      continue; 
    if(d->connected!=CON_PLAYING && d->connected != CON_EDITING)
      continue;
	  if((victim = d->original ? d->original : d->character ) == NULL)
      continue;
    if(IS_NPC(victim))
      continue;  */
       victim = fpl->ch ;
     if( victim->desc == NULL )
       continue;
    if( !IS_NPC(victim) && IS_SET( victim->act, PLR_WIZINVIS ) )
      continue;
    if(!can_see( ch, victim) )
      continue;
	  if(!IS_IMMORTAL(ch) &&(IS_AFFECTED(victim, AFF_HIDE) ||   IS_AFFECTED(victim, AFF_SNEAK)
	     ||   IS_AFFECTED(victim, AFF_STEALTH)))
      continue;
    if( victim->in_room != NULL)
     if(doAll || (victim->in_room->area==ch->in_room->area))
	    {
		  found = TRUE;
      strcpy(buf2,  victim->name );
      buf2[15]='\0';
      if(victim->in_room->area==ch->in_room->area)
	strcpy(buf3, victim->in_room->name);
      else
	{
        strcpy(buf3, victim->in_room->area->name);
	}
      buf3[44]='\0';
      if(victim->leader!=NULL && is_same_group( victim, victim->leader) &&
	 victim != victim->leader )
	strcpy(buf4, victim->leader->name);
      else
       switch(test)
	{
	case 0: strcpy( buf4, "\\  "); break;
	case 1: strcpy( buf4, " \\ "); break;
	case 2: strcpy( buf4, "  \\"); break;
	case 3: strcpy( buf4, "  /"); break;
	case 4: strcpy( buf4, " / "); break;
	case 5: strcpy( buf4, "/  "); break;
	}
      test++;
      if(test>5)
	test=0;
      buf4[15]='\0';
      if(victim->desc==NULL)
	strcpy(buf5, "location.unknown");
      else
	strcpy(buf5, victim->desc->domain);
/*
      ct=0;
      for(cnt=0;hoster[cnt]!='\0';cnt++)
	if( hoster[cnt]!='.' &&  (hoster[cnt]>57 || hoster[cnt]<48))
	  ct=1;
     if(ct==1)
      {
      cnt=0;
      for(pt=strlen(hoster)-1; pt>=0 && cnt<2; pt--)
	 if( hoster[pt]=='.')
	   cnt++;
      if(cnt<2)
	pt=-2;
      ct=0;
      for(cnt=pt+2; hoster[cnt]!='\0'; cnt++,ct++)
	buf5[ct]=hoster[cnt];
      buf5[ct]='\0';
      }
     else
      {
      ct=0;
      for(cnt=0;hoster[cnt]!='\0' && ct<2;cnt++)
	if(hoster[cnt]=='.')
	  ct++;
      if(ct==2)
	cnt--;
      hoster[cnt]='\0';
      strcpy(buf5,hoster);
      }
*/
		  sprintf( buf, " %s %s %s %s\n\r",
	      str_resize(buf2,tbuf1,-16),
	      str_resize(buf3,tbuf2,-25),
	      str_resize(buf4,tbuf3,-17),
	      str_resize(buf5,tbuf4,-17));
		  leng = str_apd_max( sss, buf, leng, MAX_STRING_LENGTH );
	    }
	}
	if ( !found )
	    leng = str_apd_max( sss, "None.\n\r" , leng, MAX_STRING_LENGTH);
    send_to_char( sss, ch );
  return;
 }
	found = FALSE;
	for ( victim = first_char; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&  (( victim->in_room->area == ch->in_room->area && IS_NPC(victim)) ||
	   ( !IS_NPC( victim)))
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   !IS_AFFECTED(victim, AFF_STEALTH)
	    &&   can_see( ch, victim )
	    &&   (is_name( arg, victim->name )))
	    {
		found = TRUE;
		if (!IS_NPC(victim))
 		{
		 if (!is_desc_valid(victim))
		  sprintf(hoster, "linklost");
		 else if (IS_IMMORTAL(ch) || ch->which_god == GOD_POLICE)
		  sprintf(hoster, victim->desc->host);
		 else
		  sprintf(hoster, victim->desc->domain);
	
		sprintf( buf, "%s\n\r%s\n\r%s\n\r",
		    PERS(victim, ch), victim->in_room->name , hoster);
		}
		else
		  sprintf( buf, "%s\n\r%s\n\r",
		    PERS(victim, ch), victim->in_room->name);
		leng = str_apd_max( sss, buf, leng, MAX_STRING_LENGTH);
		break;
	    }
	}
	if ( !found )
           {
	   sprintf( buf, "You didn't find '%s'.\n\r", arg);
           leng = str_apd_max( sss, buf, leng, MAX_STRING_LENGTH );
           }
    send_to_char( sss, ch );

  return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    diff = (victim->level - ch->level)*10/(10+ch->level);

	 if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}

void do_know( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int aln;

    if( IS_NPC(ch) || ch->race!=RACE_DROW )
      {
      send_to_char( "You should know that you can't know that.\n\r", ch );
      return;
      }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Know the alignment of whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room_even_hidden( ch, arg ) ) == NULL )
    {
	send_to_char( "You should know they are not here.\n\r", ch );
	return;
    }

    aln = victim->alignment;

	 if ( aln <= -800 ) msg = "$N is totally evil.";
    else if ( aln <= -600 ) msg = "$N is very evil.";
    else if ( aln <= -400 ) msg = "$N evil.";
    else if ( aln <= -200 ) msg = "$N is slightly evil.";
    else if ( aln <=  0 ) msg = "$N neutral on the evil side.";
    else if ( aln <=  200 ) msg = "$N neutral on the good side.";
    else if ( aln <=  400 ) msg = "$N is slightly good.";
    else if ( aln <=  600 ) msg = "$N is good.";
    else if ( aln <=  800 ) msg = "$N is very good.";
    else                    msg = "$N is totally good.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha((int)title[0]) || isdigit((int)title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    STRFREE (ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_email( CHAR_DATA *ch, char *argument )
{
    char buf2[200];
    if ( IS_NPC(ch) || !is_desc_valid( ch ))
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your email address account name to?\n\r", ch );
	return;
    }

    smash_tilde( argument );
    STRFREE(ch->pcdata->mail_address );
    argument[30]='\0';
    ch->pcdata->mail_address = str_dup( argument );
    sprintf( buf2, "Address set to: %s\n\r", argument );
    send_to_char( buf2 , ch );
}

void do_html( CHAR_DATA *ch, char *argument )
{
    char buf2[200];
    char *pt;

    if ( IS_NPC(ch) || !is_desc_valid( ch ))
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your html address to?\n\r", ch );
	return;
    }

    if( *argument == '.' && *(argument+1)=='\0' )
      {
      STRFREE(ch->pcdata->html_address );
      ch->pcdata->html_address = str_dup( "" );
      send_to_char( "HTML Home Page address cleared.\n\r", ch );
      return;
      }

    sprintf( buf2, "Address set to: %s\n\r", argument );
    for( pt=argument; *pt!='\0'; pt++)
      if( *pt == '~' )
        *pt = '*';

    STRFREE(ch->pcdata->html_address );
    argument[30]='\0';
    ch->pcdata->html_address = str_dup( argument );
    send_to_char( buf2 , ch );
}


void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    argument[60] = '\0';   

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Title set.\n\r", ch );
}

void do_unalias( CHAR_DATA *ch, char *argument )
{
   int cnt;
   bool found = FALSE;

   if ( IS_NPC(ch) )
      return;

   smash_tilde( argument );

   if ( argument[0] == '\0')
     {
     send_to_char( "Unalias what ?\n\r", ch);
     }
   if (!strcasecmp(argument, "all"))
   {
    for(cnt=0;cnt<MAX_ALIAS;cnt++)
       {
       STRFREE (ch->pcdata->alias[cnt] );
       ch->pcdata->alias[cnt]=STRALLOC("");
       STRFREE (ch->pcdata->alias_c[cnt] );
       ch->pcdata->alias_c[cnt]=STRALLOC("");
       }
     ch_printf(ch, "All aliases removed.\n\r");
     return;
    }
   for(cnt=0;cnt<MAX_ALIAS;cnt++)
     if( !strcasecmp(ch->pcdata->alias_c[cnt],argument))
       {
       STRFREE (ch->pcdata->alias[cnt] );
       ch->pcdata->alias[cnt]=STRALLOC("");
       STRFREE (ch->pcdata->alias_c[cnt] );
       ch->pcdata->alias_c[cnt]=STRALLOC("");
       ch_printf(ch, "Alias %s removed.\n\r", argument );
       found = TRUE;
       return;
       }
  if (!found)
    ch_printf(ch, "You don't have an alias for %s.\n\r", argument);
  return;
}

  /* Sort alphabetically   -  Chaos  4/4/99  */
void sort_alias( CHAR_DATA *ch )
  {
  int cnt1, cnt2;
  int acase;
  char *pt1, *pt2;

  /* Use simple Sieve filter here */
  for( cnt1=0; cnt1<MAX_ALIAS-2; cnt1++)
    for( cnt2=MAX_ALIAS-2; cnt2>=cnt1; cnt2--)
      {
      acase=0;
      if(ch->pcdata->alias[cnt2][0]=='\0')
        {
        if(ch->pcdata->alias[cnt2+1][0]!='\0')
          acase=+1;
        else
          acase=0;
        }
      else
        if(ch->pcdata->alias[cnt2+1][0]=='\0')
          acase=-1;
      else
      acase=strcasecmp(ch->pcdata->alias_c[cnt2], ch->pcdata->alias_c[cnt2+1]);

        /* Swap */
      if( acase>0 )
        {
        pt1=ch->pcdata->alias_c[cnt2];
        pt2=ch->pcdata->alias[cnt2];
        ch->pcdata->alias_c[cnt2]=ch->pcdata->alias_c[cnt2+1];
        ch->pcdata->alias[cnt2]=ch->pcdata->alias[cnt2+1];
        ch->pcdata->alias_c[cnt2+1]=pt1;
        ch->pcdata->alias[cnt2+1]=pt2;
        }
      }

  return;
  }



void do_alias( CHAR_DATA *ch, char *argument )
{
   int cnt,found=4096;
   char buf[MAX_STRING_LENGTH];
   int blen, tot;
   char tbuf[MAX_STRING_LENGTH];
   char *pt;
   bool match=FALSE,lowest=FALSE;

/*
  Re-wrote most of this to get rid of the overrunning buffers and also
  to speed it up a little...Martin
*/

  *buf='\0';

   if ( IS_NPC(ch) )
      return;

    /* Colorfull version -  Chaos 4/4/99  */
   if ( argument[0] == '\0')
     {
     tot=0;
     blen = str_cpy_max( buf, "{130}Alias list:\n\r", MAX_STRING_LENGTH);
     for(cnt=0;cnt<MAX_ALIAS;cnt++)
       if(ch->pcdata->alias[cnt][0]!='\0')
	 {
         tot++;
	 sprintf( tbuf, "{170}%-12s {030}%s\n\r", ch->pcdata->alias_c[cnt],
	   ch->pcdata->alias[cnt] );
         blen = str_apd_max( buf, tbuf, blen, MAX_STRING_LENGTH);
	 }
     sprintf( tbuf, "{130}%d Aliases used out of %d available.\n\r", 
         tot, MAX_ALIAS );
     blen = str_apd_max( buf, tbuf, blen, MAX_STRING_LENGTH);
     send_to_char( ansi_translate_text( ch, buf), ch );
     return;
     }
   
   smash_tilde( argument );
   argument=one_argument_nolower(argument, buf);

  for(pt=buf; *pt!='\0'; pt++)
    if( *pt<'A' || *pt>'z' || (*pt>'Z' && *pt<'a' ) )
      {
      send_to_char( "You cannot make an alias command with non-letters.\n\r",
          ch);
      return;
      }

   if (argument[0]=='\0')
   {
   for(cnt=0;cnt<MAX_ALIAS;cnt++) 
   {
     if( !strcasecmp(ch->pcdata->alias_c[cnt],buf))
      {
       ch_printf(ch, "Use unalias to delete your alias for %s (currently %s).\n\r", buf, ch->pcdata->alias[cnt]);
       return;
      }
    }
    ch_printf(ch, "Alias %s to what ?\n\r",buf);
    return;
   }
   for(cnt=0;cnt<MAX_ALIAS;cnt++)
    {
     if (ch->pcdata->alias[cnt][0]=='\0' && !match) 
      { 
       if (!lowest) 
        {
         found=cnt;lowest=TRUE;
        }
       continue;
      }
     if( !strcasecmp(ch->pcdata->alias_c[cnt],buf))
       {
       found=cnt;
       match=TRUE;
       }
      }
  if (found!=4096)
   {
       STRFREE (ch->pcdata->alias[found] );
       STRFREE (ch->pcdata->alias_c[found] );
       ch->pcdata->alias_c[found] = STRALLOC(buf);
       ch->pcdata->alias[found]= STRALLOC(argument);

       sort_alias( ch );

       send_to_char( "Alias set.\n\r" , ch );
       return; 
    }
   send_to_char( "Alias list is full.\n\r", ch);
}


void do_description( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Monsters are too dumb to do that!\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        bug( "do_description: no descriptor", 0 );
        return;
    }

    switch( ch->substate )
    {
        default:
           bug( "do_description: illegal substate", 0 );
           return;

        case SUB_RESTRICTED:
           send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
           return;

        case SUB_NONE:
           ch->substate = SUB_PERSONAL_DESC;
           start_editing( ch, ch->description );
           return;
        case SUB_PERSONAL_DESC:
           STRFREE (ch->description );
           ch->description = copy_buffer( ch );
           stop_editing( ch );
           return;
    }
}

/*
    char buf[MAX_STRING_LENGTH];

    if( IS_NPC( ch ))
      return;

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace((int)*argument) )
		argument++;
	}

	if ( strlen( ansi_translate_text(ch,buf)) + strlen(ansi_translate_text(ch, argument)) >= MAX_STRING_LENGTH - 2 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	STRFREE (ch->description );
	ch->description = STRALLOC( buf);
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ansi_translate_text( ch,
          ch->description) :
              "(None).\n\r", ch );
    return;
}
*/


void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char tbuf1[MAX_INPUT_LENGTH];
    int sn, cnt;

    if ( IS_NPC(ch) || ch->pcdata == NULL )
	return;

    if ( ch->level < 3 )
    {
	send_to_char(
	    "You must be third level to practice.  Go train instead!\n\r",
	    ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	int col;
	char tstc;
	int adept;

	col    = 0;
	buf[0]='\0';
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    tstc=' ';
	    cnt=multi_pick(ch , sn );
	    if(cnt!=-1)
	      {
	      adept = class_table[cnt].skill_adept;
              if( get_curr_int( ch ) < 25 )
                {
                adept -= (25-get_curr_int(ch))*2 ;
                if( adept<50 )
                  adept=50;
                }
	      if( adept > class_table[cnt].skill_adept )
	          adept = class_table[cnt].skill_adept;
	      if(ch->mclass[ch->class] < skill_table[sn].skill_level[ch->class])
		 tstc='*';
	      strcpy(buf3,skill_table[sn].name);
	      buf3[18]='\0';
	      sprintf( buf2, "%s%s%c%2d/%2d%%", buf,
		 str_resize(buf3,tbuf1,19), tstc, ch->pcdata->learned[sn],
                 adept );
	      strcpy(buf,buf2);
	      if ( ++col % 3 == 0 )
		{
		strcat(buf,"\n\r");
		send_to_char( buf, ch );
		buf[0]='\0';
		}
	      }
	}

	if ( col % 3 != 0 )
	    {
	    strcat(buf,"\n\r");
	    send_to_char( buf, ch );
	    buf[0]='\0';
	    }

	sprintf( buf, "You have %d practice sessions left.         * Outside current class.\n\r",
	    ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->first_person; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}
 
	if( ch->gold < ch->level*25*ch->level )
	  {
	  send_to_char( "You do not have enough money to practice.\n\r", ch );
	  return;
	  }

        
	if ( ( sn = skill_lookup( argument ) ) < 0)
	    {
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	    }
        if( ( cnt =multi_pick(ch , sn ) ) == -1 )
	    {
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	    }
	if( ch->mclass[cnt] < skill_table[sn].skill_level[cnt] ) 
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}
	if(ch->mclass[ch->class] < skill_table[sn].skill_level[ch->class] &&
            ch->practice < 2)
	  {
	    send_to_char( "You don't have enough practices for that.\n\r", ch );
	    return;
	  }

	adept = class_table[cnt].skill_adept;
      if( get_curr_int( ch ) < 25 )
        {
        adept -= (25-get_curr_int(ch))*2 ;
        if( adept<50 )
          adept=50;
        }
	      if( adept > class_table[cnt].skill_adept )
	          adept = class_table[cnt].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept /* adept */ )
	    {
	    if ( ch->pcdata->learned[sn] >= class_table[cnt].skill_adept )
	      {
	      sprintf( buf, "You are already a master of %s.\n\r",
		    skill_table[sn].name );
	      send_to_char( buf, ch );
              return;
	      }
	    sprintf( buf, "You are already an adept of %s, but you practice more.\n\r",
		  skill_table[sn].name );
	    ch->pcdata->learned[sn] ++;
	    if(ch->mclass[ch->class] < skill_table[sn].skill_level[ch->class])
	      ch->practice-=2;
            else
	      ch->practice--;
	    send_to_char( buf, ch );
	    }
	  else
	    {
	    if(ch->mclass[ch->class] < skill_table[sn].skill_level[ch->class])
	      ch->practice-=2;
            else
	      ch->practice--;
      ch->gold-=(25*ch->level*ch->level);
	      ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;
	      if ( ch->pcdata->learned[sn] < adept )
		{
		      act( "You practice $T.",
			  ch, NULL, skill_table[sn].name, TO_CHAR );
		      act( "$n practices $T.",
			  ch, NULL, skill_table[sn].name, TO_ROOM );
		}
	      else
		{
		      ch->pcdata->learned[sn] = adept;
		      act( "You are now an adept of $T.",
			  ch, NULL, skill_table[sn].name, TO_CHAR );
		      act( "$n is now an adept of $T.",
			  ch, NULL, skill_table[sn].name, TO_ROOM );
		}
	    }
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int wimpy;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
/*  wimpy = ch->max_hit / 5;    Why default wimpy?  Show current wimpy. */
	  wimpy = ch->wimpy;
  else
	  wimpy = atol( arg );

  if ( wimpy < 0 )
    {
	  send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	  return;
    }

  if ( wimpy > ch->max_hit )
    {
	  send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	  return;
    }

  ch->wimpy     = wimpy;
  sprintf( buf, "Wimpy is set to %d hit points.\n\r", wimpy );
  send_to_char( buf, ch );
  return;
  }



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    /* char *p; */
    char cEnd;

    if ( IS_NPC(ch) )
	return;

      DISALLOW_SNOOP = TRUE;
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
	send_to_char( "Syntax: password <old> <new> <new>.\n\r", ch );
	return;
    }

    if( strcmp( arg2, arg3 ) )
      {
      send_to_char( "Your new password does not verify.\n\rPlease try again.\n\r", ch);
      return;
      }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
	if ( !is_valid_password( arg2 ) )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    send_to_char( "The password must only contain letters (case sensitive), or numbers.\n\rYou are required to include at least one number in the password.\n\r", ch );
	    return;
	}

    pwdnew = crypt( arg2, ch->name );

    STRFREE(ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj(ch, NORMAL_SAVE);
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[20];
    char tbuf1[MAX_INPUT_LENGTH];
    int iSocial;
    int col;
 
    col = 0;
    buf[0]=0;
    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
      {
      sprintf( buf2, "%s", str_resize(social_table[iSocial].name,tbuf1,-12));
      strcat( buf,buf2 );
      if ( ++col % 6 == 0 )
	{
	strcat(buf,"\n\r");
	send_to_char( buf,ch ); 
	buf[0]='\0';
	}
      }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}



void do_skills( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    int sn;
    int col,cnt,lev,cls;
    char arg[3];

/*   Rewritten by Chaos 10/10/93 */

    col = 0;
    buf[2] = '\0';
    arg[0]=argument[0];
    arg[1]=argument[1];
    arg[2]='\0';
    cls=ch->class;

    if(!strcasecmp(arg,"al"))
      cls=-1;
    for(cnt=0;cnt<MAX_CLASS;cnt++)
      {
      buf[0]=class_table[cnt].who_name[0];
      buf[1]=class_table[cnt].who_name[1];
      if(!strcasecmp(arg,buf))
	 cls=cnt;
      }
    buf[0]='\0';

    for(cnt=0;cnt<MAX_CLASS;cnt++)
     {
     if( cnt!=cls && cls!=-1)
       continue;
     sprintf(buf2, "Class - %s\n\r", class_table[cnt].who_name_long);
     send_to_char( buf2, ch);
     for(lev=0;lev<MAX_LEVEL;lev++)
      for(sn=0;sn<MAX_SKILL;sn++)
       if(skill_table[sn].name != NULL)
	if(skill_table[sn].skill_level[cnt]==lev)
	  {
	  sprintf( buf2, "%2d-", lev);
	  strcat( buf2, skill_table[sn].name );
	  buf2[25]='\0';
	  while(strlen(buf2)<25)
	    strcat( buf2, " ");
	  buf[24]=' ';
	  strcat( buf, buf2);
	  if ( ++col % 3 == 0 )
	    {
	    strcat(buf,"\n\r");
	    send_to_char( buf,ch ); 
	    buf[0]='\0';
	    col=0;
	    }
	  }
      if ( col % 3 != 0 )
	{
	strcat(buf, "\n\r");
	send_to_char( buf, ch );
	buf[0]='\0';
	col=0;
	}
      }
    return;
}



/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[20];
    char tbuf1[MAX_INPUT_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    buf[0]=0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      {
      if ( cmd_table[cmd].level <  LEVEL_HERO
	&&   cmd_table[cmd].level <= get_trust( ch ) )
	{
	      sprintf( buf2, "%s", str_resize(cmd_table[cmd].name,tbuf1,-14) );
	strcat(buf,buf2);
	      if ( ++col % 5 == 0 )
	  {
	  strcat(buf,"\n\r");
	  send_to_char( buf,ch ); 
	  buf[0]=0;
	  }
	}
      }
 
    if ( col % 6 != 0 )
	  send_to_char( "\n\r", ch );
    return;
}

typedef struct chan_list CHAN_LIST;
struct chan_list
  {
  CHAR_DATA *ch;
  CHAN_LIST *next;
  };

void do_channel( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  PLAYER_GAME *fpl;
  CHAR_DATA *fch;
  CHAN_LIST *fcn, *cn_start, *cn_new, *prev;
  int value;

  if( IS_NPC( ch ))
    return;

  if( argument[0]!='\0')
    {
    bool others;
    PLAYER_GAME *gpl;
    value = atol( argument );
    if( value < 0 || value > 32 )
      {
      send_to_char( "There are only channels numbers 0 to 32.\n\rChannel 0 turns off the feature.\n\r", ch);
      return;
      }
    others = FALSE;
    if( value > 0 )
      for( gpl = first_player; !others && gpl != NULL; gpl=gpl->next)
        if( gpl->ch->pcdata->channel == value )
          others= TRUE;
    if( !others  || ch->pcdata->channel == value || IS_IMMORTAL(ch))
      {
      sprintf( buf, "Channel set to %d.\n\r", value);
      ch->pcdata->channel = value;
      ch->pcdata->channel_request = 0;
      send_to_char( buf, ch );
      return;
      }
    for( gpl = first_player; gpl != NULL; gpl=gpl->next)
      if( gpl->ch->pcdata->channel == value )
        {
        if(!blocking(gpl->ch,ch))
          {
        sprintf(buf,"%s is requesting to join conference on your channel.\n\r",
            get_name( ch ));
          send_to_char( buf, gpl->ch );
          }
        }
    sprintf( buf, "Waiting for access to channel %d.\n\r", value);
    send_to_char( buf, ch );
    ch->pcdata->channel_request = value;
    return;
    }
  send_to_char( "List of Channels:\n\r", ch);
  buf[0]='\0';

  cn_start = NULL;
  for( fpl = first_player; fpl != NULL; fpl = fpl->next )
    {
    fch = fpl->ch;
    if(  IS_SET(fch->act, PLR_WIZINVIS) || fch->pcdata->channel == 0 )
          continue;
  if (fch!=ch && is_affected(fch, gsn_greater_stealth) &&
     !((is_affected(ch, gsn_truesight) && ch->mclass[CLASS_ILLUSIONIST] >= fch->level) || IS_SET(ch->act, PLR_HOLYLIGHT)))
    continue;

    CREATE( cn_new, CHAN_LIST, 1);
    cn_new->ch = fch;
    cn_new->next = NULL;
    if( cn_start == NULL || cn_start->ch->pcdata->channel >
        fch->pcdata->channel )
      {
      cn_new->next = cn_start;
      cn_start = cn_new;
      }
    else
      {
      prev = NULL;
      for( fcn=cn_start; fcn!=NULL; prev=fcn, fcn=fcn->next )
        if( fcn->next != NULL && fcn->next->ch->pcdata->channel >
          fch->pcdata->channel )
          {
          cn_new->next = fcn->next;
          fcn->next = cn_new;
          break;
          }
      if( fcn==NULL && prev!=NULL )
        prev->next = cn_new;
      }
    }

  for( fcn = cn_start; fcn != NULL; fcn = fcn->next )
    {
    fch = fcn->ch;
    if( buf[0]=='\0')
      {
      sprintf( buf, "%-16s  %2d%19s", capitalize(fch->name), 
                                      fch->pcdata->channel, "");
      }
    else
      {
      sprintf( buf2, "%-16s  %2d%19s", capitalize(fch->name), 
                                      fch->pcdata->channel, "");
      strcat( buf, buf2);
      strcat( buf, "\n\r");
      send_to_char( buf, ch );
      buf[0]='\0';
      }
    }
  if( buf[0]!='\0')
      {
      strcat( buf, "\n\r");
      send_to_char( buf, ch );
      }

  for( fcn=cn_start; fcn != NULL; fcn=prev )
    {
    prev = fcn->next;
    DISPOSE(fcn);
    }
  
    return; 
}

      /* By Chaos 11/13/94 */
void do_decline ( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  PLAYER_GAME *fpl;


  if( IS_NPC( ch ))
    return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Decline channel access to whom?\n\r", ch );
	return;
    }

      if ( ( victim = get_char_room( ch, argument ) ) == NULL )
      {
      if ( ( victim = get_player_world( ch, argument ) ) == NULL )
        {
	send_to_char( "They aren't here.\n\r", ch );
	return;
        }
      }

 if( ch->pcdata->channel == 0 )
    {
	send_to_char( "You cannot decline channel access to channel 0.\n\r", ch );
	return;
    }

 if( IS_NPC( victim ) )
    {
	send_to_char( "They are not requesting your channel.\n\r", ch );
	return;
    }
 if ( IS_IMMORTAL (victim ) )
    {
        send_to_char("You cannot decline an immortal!\n\r", ch);
        return;
    }
 if( victim->pcdata->channel != ch->pcdata->channel &&
         victim->pcdata->channel_request != ch->pcdata->channel )
    {
	send_to_char( "They are not requesting your channel.\n\r", ch );
	return;
    }

  victim->pcdata->channel_request = 0;
  if( victim->pcdata->channel == ch->pcdata->channel )
    victim->pcdata->channel = 0;

  sprintf( buf, "You decline %s.\n\r", get_name( victim ) );
  send_to_char ( buf, ch );
  sprintf( buf, "You are declined access to channel %d.\n\r", 
        ch->pcdata->channel );
  send_to_char ( buf, victim );

 
  strcpy( buf2, get_name( ch ));
  sprintf( buf, "%s declines access to %s.\n\r", 
          buf2, get_name( victim ));
  for( fpl = first_player; fpl != NULL; fpl = fpl->next )
    if( fpl->ch != ch && fpl->ch != victim && 
        fpl->ch->pcdata->channel == ch->pcdata->channel)
      send_to_char( buf, fpl->ch );

  return;
  }



      /* By Chaos 11/13/94 */
void do_grant ( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  PLAYER_GAME *fpl;


  if( IS_NPC( ch ))
    return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Grant channel access to whom?\n\r", ch );
	return;
    }

      if ( ( victim = get_char_room( ch, argument ) ) == NULL )
      {
      if ( ( victim = get_player_world( ch, argument ) ) == NULL )
        {
	send_to_char( "They aren't here.\n\r", ch );
	return;
        }
      }

 if( ch->pcdata->channel == 0 )
    {
	send_to_char( "You cannot grant channel access to channel 0.\n\r", ch );
	return;
    }

 if( IS_NPC( victim ) )
    {
	send_to_char( "They are not requesting your channel.\n\r", ch );
	return;
    }

  if ( victim->pcdata->channel_request != ch->pcdata->channel )
    {
	send_to_char( "They are not requesting your channel.\n\r", ch );
	return;
    }

  victim->pcdata->channel = ch->pcdata->channel ;
  victim->pcdata->channel_request = 0;

  sprintf( buf, "You grant %s.\n\r", get_name( victim ) );
  send_to_char ( buf, ch );
  sprintf( buf, "You are granted access to channel %d.\n\r", 
        ch->pcdata->channel );
  send_to_char ( buf, victim );

  strcpy( buf2, get_name( ch ));
  sprintf( buf, "%s grants access to %s.\n\r", 
          buf2 , get_name( victim ));
  for( fpl = first_player; fpl != NULL; fpl = fpl->next )
    if( fpl->ch != ch && fpl->ch != victim && 
        fpl->ch->pcdata->channel == ch->pcdata->channel)
      send_to_char( buf, fpl->ch );

  return;
  }



/*  By Chaos   9/20/93   */
void do_level ( CHAR_DATA *ch, char *argument )
{
  int lvn, lvx, cnt, ld;
  char buf[81];

  if( ch->level >= 95 )
    {
    send_to_char( "You are already at the maximum level.\n\r", ch );
    return;
    }

  if( IS_NPC( ch ) )
    return;
  lvn=ch->mclass[ch->class]-3;
  lvx=ch->mclass[ch->class]+3;
  ld=ch->level-ch->mclass[ch->class];

  if(lvn<0)
    lvn=0;
  if(ld==0 && lvn<1)
    lvn=1;
  if(lvx+ld>MAX_LEVEL)
    lvx=MAX_LEVEL-ld;
  if( lvx > 95 )
    lvx = 95;
  send_to_char( "Level - Experience Required    - Title\n\r", ch);
  for(cnt=lvn;cnt<=lvx;cnt++)
    {
    if((ch->level-ld)==cnt)
      {
      sprintf(buf, " You  - %12d\n\r", ch->exp);
      send_to_char(buf,ch);
      }
    sprintf(buf, " %3d  - %12d\n\r", cnt+1+ld, exp_level(ch->class,cnt+ld));
    send_to_char(buf,ch);
    }
  if(ch->level<MAX_LEVEL)
    {
    sprintf( buf, " You need %d experience points to gain a level.\n\r",
      exp_level(ch->class, ch->level)-ch->exp);
    send_to_char( buf, ch);
    }
}

/* By Chaos 9/20/93  */
void do_clock( CHAR_DATA *ch, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int clk;

  if( ch->desc->original != NULL)
    return;

  if( IS_NPC(ch) )
     return;

  one_argument( arg, buf);
  if(buf[0]=='\0')
    {
    send_to_char( "Usage:   clock <offset>\n\rThis sets the clock to your local time zone.\n\r", ch );
    send_to_char( "         clock mil  - This sets the clock to military time.\n\r" , ch);
    send_to_char( "         clock civ  - This sets the clock to normal am/pm mode.\n\r", ch);
    return;
    }
  if(!strcasecmp(buf,"mil"))
    {
    ch->clock=ch->clock%100+100;
    return;
    }
  if(!strcasecmp(buf,"civ"))
    {
    ch->clock=ch->clock%100;
    return;
    }
  clk=atol(buf);
  while(clk<0)
    clk+=24;
  clk=clk%24;
  ch->clock=(ch->clock/100)*100+clk;
  }

void do_class( CHAR_DATA *ch, char *arg)
  {
  int cnt;
  char buf[MAX_INPUT_LENGTH];
  
  if( IS_NPC(ch))
    return;

  if(ch->level%10!=0)
    {
    send_to_char("You cannot switch classes now.\n\r", ch );
    return;
    }

  if(ch->mclass_switched==1)
    {
    send_to_char("You have already switched classes at this level.\n\r", ch );
    return;
    }
  arg = one_argument_nolower( arg, buf);

  if(buf[0]=='\0')
    {
    strcpy(buf,"You may switch to the following classes: ");
    for(cnt=0;cnt<MAX_CLASS;cnt++)
      if(ch->mclass[cnt]==0 && race_table[ch->race].race_class[cnt]==0)
	{
	strcat(buf," ");
	strcat(buf,class_table[cnt].who_name);
	}
    strcat(buf,"\n\r");
    send_to_char( buf, ch);
    return;
    }

    if ( strcmp( crypt( arg, ch->pcdata->pwd ), ch->pcdata->pwd ) )
       {
       send_to_char( "You must enter your password after the class name.\n\r",ch);
       return;
       }

  if( which_god(ch)==GOD_INIT_CHAOS || which_god(ch)==GOD_INIT_ORDER )
    for(cnt=0;cnt<MAX_CLASS;cnt++)
      if(!strcasecmp(buf,(char *)class_table[cnt].who_name) &&
          cnt == CLASS_ASSASSIN ) 
        {
        send_to_char("God Initiates cannot be an assassin.\n\r", ch);
        return;
        }

  for(cnt=0;cnt<MAX_CLASS;cnt++)
    if(!strcasecmp(buf,(char *)class_table[cnt].who_name) && ch->mclass[cnt]==0
       && race_table[ch->race].race_class[cnt]==0 )
      {
      sprintf(buf, "Switching class to: %s\n\r", class_table[cnt].who_name);
      send_to_char( buf, ch );
      ch->class=cnt;
      ch->exp=exp_level(cnt,ch->level-1)+1;
      ch->mclass_switched=1;
      ch->practice/=2;
      remove_obj( ch, WEAR_WIELD, TRUE, TRUE);
      return;
      }

  send_to_char("You cannot switch to that class.\n\r", ch);

  return;
  }

void do_vt100( CHAR_DATA *ch, char *arg)
{
  int cnt; 
  char buf[MAX_INPUT_LENGTH];

  if( IS_NPC(ch) )
      return;

  one_argument(arg, buf);
  if( buf[0] == '\0')
    {
    send_to_char("Usage:   vt102 <command>\n\r<on,off> turns vt100 mode on and off.\n\r" , ch );
    send_to_char("<##> sets up the amount of rows per terminal. (default 24)\n\r", ch );
    send_to_char("<bold,nobold> sets the highlighting of lines with the word 'you' in it.\n\r", ch );
    if(ch->vt100!=0)
      send_to_char("Vt102 mode is enabled.\n\r", ch);
    else
      send_to_char("Vt102 mode is disabled.\n\r", ch);
    sprintf(buf, "Rows set to %d.\n\r",ch->vt100_type%100);
    send_to_char(buf, ch );
    if((ch->vt100_type/1000)%10==0)
      send_to_char("There is no highlighting.\n\r", ch );
    else
      send_to_char("Highlighting is enabled.\n\r", ch );
    if((ch->vt100_type/10000)%10==0)
      send_to_char("Vt102 speed is set FAST.\n\r", ch );
    else
      send_to_char("Vt102 speed is set SLOW.\n\r", ch );
    }

  if(!strcasecmp(buf,"on") && ch->vt100==0)
        vt100on(ch);
  if(!strcasecmp(buf,"off") && ch->vt100!=0)
    {
    /*ch->ansi=0;*/
    vt100off(ch);
    }
  if((cnt=atol(buf))!=0)
    {
    if(cnt<15 || cnt>99)
      {
      send_to_char("Rows must be between 15 and 99.\n\r", ch );
      return;
      }
    ch->vt100_type=(ch->vt100_type/100)*100+cnt;
    if(ch->vt100==1)
      {
      vt100off(ch);
      ch->vt100=2;
      if( IS_SET( ch->act, PLR_PROMPT ) )
        vt100prompt(ch);
      else
        vt100on(ch);
      }
    return;
    }

  if(!strcasecmp(buf,"slow"))
  {
    if( (ch->vt100_type/10000)%10==0)
      {
      ch->vt100_type+=10000;
      send_to_char( "Slow mode on.\n\r", ch );
      }
    else
      send_to_char( "Slow mode already on.\n\r", ch );
  }

  if(!strcasecmp(buf,"fast"))
  {
    if( (ch->vt100_type/10000)%10==1)
      {
      ch->vt100_type-=10000;
      send_to_char( "Fast mode on.\n\r", ch );
      }
    else
      send_to_char( "Fast mode already on.\n\r", ch );
  }

  if(!strcasecmp(buf,"bold"))
  {
    if( (ch->vt100_type/1000)%10==0)
      {
      ch->vt100_type+=1000;
      send_to_char( "Bold mode on.\n\r", ch );
      }
    else
      send_to_char( "Bold mode already on.\n\r", ch );
  }

  if(!strcasecmp(buf,"nobold"))
  {
    if( (ch->vt100_type/1000)%10==1)
      {
      ch->vt100_type-=1000;
      send_to_char( "Bold mode off.\n\r", ch );
      }
    else
      send_to_char( "Bold mode already off.\n\r", ch );
  }

  return;
  }
      

void do_config2( CHAR_DATA *ch, char *argument )
  {
  do_config( ch, argument );
  return;
  }
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *arg3;
    char buf[MAX_STRING_LENGTH];
    char buft[MAX_INPUT_LENGTH];
    int  bit1, bit2, cnt, cnt2;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    arg3 = argument;

    bit1 = 0;
    bit2 = 0;

    if ( arg1[0] == '-' || arg1[0]=='x' || arg1[0]=='X' )
      {
      strcpy( buf, "{130}Configuration finished.\n\r\n\r");
      send_to_char( ansi_translate_text( ch, buf ), ch );
      if( ch->pcdata->last_command != NULL )
        STRFREE( ch->pcdata->last_command );
      ch->pcdata->last_command = NULL;
      return;
      }

     /* Tactical and VT102 configuration */
    if ( arg1[0] == 'b' || arg1[0]=='B' )
      {
      if( *arg2 == 'x' || *arg2 == 'X' )
        {
        strcpy( buf, "{130}Configuration finished.\n\r\n\r");
        send_to_char( ansi_translate_text( ch, buf ), ch );
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        return;
        }
      if( *arg2 == '-' )
        {
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        do_config( ch, "" );
        return;
        }

      if( *arg2 == 'b' || *arg2=='B')
       if( *arg3 == '\0' )
        {
        strcpy( buf, "{120}Enter amount of terminal lines now.\n\r" );
        send_to_char( ansi_translate_text( ch, buf ), ch );

        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        ch->pcdata->last_command = str_dup( "config B B " );
        return;
        }

      if( *arg2 == 'e' || *arg2=='E')
       if( *arg3 == '\0' )
        {
        cnt = MAX_TACTICAL - 1;
        if ((ch->vt100_type % 100) - 10 < MAX_TACTICAL - 1)
          cnt = (ch->vt100_type % 100) - 10;
        if (cnt < 3)
          cnt = 1;

        sprintf( buf, "{120}Enter amount of tactical lines now. (1-%d)\n\r",
            cnt );
        send_to_char( ansi_translate_text( ch, buf ), ch );

        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        ch->pcdata->last_command = str_dup( "config B E " );
        return;
        }

      if( *arg2 == 'f' || *arg2=='F')
       if( *arg3 == '\0' )
        {
      strcpy( buf, "{120}Enter compass width now.  (4-16, or 0 for OFF)\n\r" );
        send_to_char( ansi_translate_text( ch, buf ), ch );

        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        ch->pcdata->last_command = str_dup( "config B F " );
        return;
        }

      if( *arg2 == 'i' || *arg2=='I')
       if( *arg3 == '\0' )
        {
      strcpy( buf, "{120}Select your two character index now.\n\r" );
        send_to_char( ansi_translate_text( ch, buf ), ch );

        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        ch->pcdata->last_command = str_dup( "config B I " );
        return;
        }

      switch( *arg2 )
        {
        case 'a': case 'A':
          if(ch->vt100==0)
            vt100on(ch);
          else
            vt100off(ch);
          break;
        case 'b': case 'B':
            cnt=atol(arg3);
            if(cnt<15 || cnt>99)
              {
         strcpy( buf, "  {110}Terminal rows must be between 15 and 99.\n\r" );
              send_to_char( ansi_translate_text( ch, buf ), ch );
              if( ch->pcdata->last_command != NULL )
                STRFREE( ch->pcdata->last_command );
              ch->pcdata->last_command = str_dup( "config B B " );
              return;
              }
            ch->vt100_type=(ch->vt100_type/100)*100+cnt;
            if(ch->vt100==1)
              {
              vt100off(ch);
              ch->vt100=2;
              if( IS_SET( ch->act, PLR_PROMPT ) )
                vt100prompt(ch);
              else
                vt100on(ch);
              }
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " );
          break;

        case 'c': case 'C':
          if( (ch->vt100_type/1000)%10==0 )
            ch->vt100_type+=1000;
          else
            ch->vt100_type-=1000;
          break;

        case 'd': case 'D':
          if( (ch->vt100_type/10000)%10==0 )
            ch->vt100_type+=10000;
          else
            ch->vt100_type-=10000;
          break;
        case 'e':  case 'E':
          cnt = MAX_TACTICAL - 1;
          if ((ch->vt100_type % 100) - 10 < MAX_TACTICAL - 1)
            cnt = (ch->vt100_type % 100) - 10;
          if (cnt < 3)
            cnt = 1;

          cnt2=atol(arg3);
            if(cnt2<1 || cnt2>cnt)
              {
         sprintf( buf, "  {110}Tactical rows must be between 1 and %d.\n\r",
                    cnt );
              send_to_char( ansi_translate_text( ch, buf ), ch );
              if( ch->pcdata->last_command != NULL )
                STRFREE( ch->pcdata->last_command );
              ch->pcdata->last_command = str_dup( "config B E " );
              return;
              }
            ch->pcdata->tactical_mode=(ch->pcdata->tactical_mode/100)*100+cnt2;
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " ); 
          do_refresh( ch, "" );
          return;

        case 'f':  case 'F':
          cnt2=atol(arg3);
            if(cnt2!=0)
            if(cnt2<4 || cnt2>16)
              {
         sprintf( buf, "  {110}Compass width must be between 4 and 16, or 0 to turn off.\n\r");
              send_to_char( ansi_translate_text( ch, buf ), ch );
              if( ch->pcdata->last_command != NULL )
                STRFREE( ch->pcdata->last_command );
              ch->pcdata->last_command = str_dup( "config B F " );
              return;
              }
            ch->pcdata->compass_width=cnt2;
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " );
          break;

        case 'g': case 'G':
          if( (ch->pcdata->tactical_mode/100)%10==0 )
            ch->pcdata->tactical_mode+=100;
          else
            ch->pcdata->tactical_mode-=100;
          break;

        case 'h': case 'H':
          if( (ch->pcdata->tactical_mode/1000)%10==0 )
            ch->pcdata->tactical_mode+=1000;
          else
            ch->pcdata->tactical_mode-=1000;
          break;

        case 'i': case 'I':
      if (arg3[0] <= ' ' || arg3[0] > 'z' || arg3[1] <= ' ' || arg3[1] > 'z')
              {
         sprintf( buf, "  {110}That is an incorrect index.\n\rChoose a two character index now.\n\r");
              send_to_char( ansi_translate_text( ch, buf ), ch );
              if( ch->pcdata->last_command != NULL )
                STRFREE( ch->pcdata->last_command );
              ch->pcdata->last_command = str_dup( "config B I " );
              return;
              }
      if (arg3[0] == '~')
	arg3[0] = '-';
      if (arg3[1] == '~')
	arg3[1] = '-';

      buft[0] = arg3[0];
      buft[1] = arg3[1];
      buft[2] = '\0';

      STRFREE (ch->pcdata->tactical_index);
      ch->pcdata->tactical_index = str_dup (buft);


          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " );
          break;


        case '\0':  
          break;
        case 't': case 'v':
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " );
          return;

        default:
          sprintf( buf, "{110}'%c' Is not an option.\n\r", *arg2 );
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config B " );
          return;
        }

      strcpy( buf, "\n\r\n\r{130}    VT102/Tactical Setup Menu\n\r\n\r");

      strcat(  buf, ch->vt100!=0
? "{150} (A)  {120}[ ON  ]  {030}VT102 mode is enabled.\n\r"
: "{150} (A)  {020}[ off ]  {030}VT102 mode is disabled.\n\r" );


    sprintf(buft, "{150} (B)  {020}[ %-3d ]  {030}VT102 terminal lines.  (15-99)\n\r",
            ch->vt100_type%100);
    strcat( buf, buft);

    strcat( buf, (ch->vt100_type/1000)%10!=0
? "{150} (C)  {120}[ ON  ]  {030}'YOU' highlighting is enabled.\n\r"
: "{150} (C)  {020}[ off ]  {030}'YOU' highlighting is disabled.\n\r" );

    strcat( buf, (ch->vt100_type/10000)%10==0
? "{150} (D)  {120}[ ON  ]  {030}VT102 speed is set to FAST.\n\r"
: "{150} (D)  {020}[ off ]  {030}VT102 speed is set to SLOW.\n\r" );


    sprintf(buft, "{150} (E)  {020}[ %-3d ]  {030}Tactical terminal lines.  (1-18)\n\r",
            ch->pcdata->tactical_mode%100);
    strcat( buf, buft);


    if( ch->pcdata->compass_width < 4 )
      strcat( buf, 
         "{150} (F)  {020}[ off ]  {030}Your tactical compass is off.\n\r" );
    else
      sprintf(buft, "{150} (F)  {020}[ %-3d ]  {030}Tactical compass width.  (4-16)\n\r",
      ch->pcdata->compass_width  );
    strcat( buf, buft);

    strcat( buf, (ch->pcdata->tactical_mode/100)%10==0
? "{150} (G)  {120}[ ON  ]  {030}Your tactical indexs are on.\n\r"
: "{150} (G)  {020}[ off ]  {030}Your tactical indexs are off.\n\r" );

    strcat( buf, (ch->pcdata->tactical_mode/1000)%10==0
? "{150} (H)  {120}[ ON  ]  {030}Your top stat bar is above the tactical.\n\r"
: "{150} (H)  {020}[ off ]  {030}Your top stat bar is below the tactical.\n\r" );

    sprintf( buft, 
         "{150} (I)  {020}[ %c%c  ]  {030}Your tactical index.\n\r" ,
      *ch->pcdata->tactical_index, *(ch->pcdata->tactical_index+1));
    strcat( buf, buft);


      strcat( buf, "\n\r{150} (X)  {020}EXIT\n\r" );
      strcat( buf, "{150} (-)  {020}Return\n\r" );

      send_to_char( ansi_translate_text( ch, buf ), ch );
      if( ch->pcdata->last_command != NULL )
        STRFREE( ch->pcdata->last_command );
      ch->pcdata->last_command = str_dup( "config B " );
      return;
      }


    
    if ( *arg1 == 'a' || *arg1=='A' )
      {
      if( *arg2 == 'x' || *arg2 == 'X' )
        {
        strcpy( buf, "{130}Configuration finished.\n\r\n\r");
        send_to_char( ansi_translate_text( ch, buf ), ch );
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        return;
        }
      if( *arg2 == '-' )
        {
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        do_config( ch, "" );
        return;
        }

      switch( *arg2 )
        {
        case 'a': case 'A':
          bit1 = PLR_AUTOEXIT;
          break;
        case 'b': case 'B':
          bit1 = PLR_QUIET;
          break;
        case 'c': case 'C':
          bit1 = PLR_BLANK;
          break;
        case 'd': case 'D':
          bit1 = PLR_BRIEF;
          break;
        case 'e': case 'E':
          bit1 = PLR_PAGER;
          break;
        case 'f': case 'F':
          bit1 = PLR_REPEAT;
          break;
        case 'g': case 'G':
          do_refresh( ch, "" );
          bit1 = PLR_PROMPT;
          break;
        case 'h': case 'H':
          bit1 = PLR_TELNET_GA;
          break;
        case 'i': case 'I':
          bit1 = PLR_TERMINAL;
          break;
        case 'j': case 'J':
          bit2 = PLR2_EXP_TO_LEVEL;
          break;
        case 'k': case 'K':
          bit2 = PLR2_EXTERNAL_FILES;
          break;
        case 'l': case 'L':
          bit2 = PLR2_EXTERNAL_METER;
          break;
        case 'm': case 'M':
          bit2 = PLR2_MRTERM_STATS;
          break;
        case 'n': case 'N':
          bit2 = PLR2_ITEM_REF;
          break;
        case '\0':
         break;

        default:
          sprintf( buf, "{110}'%c' Is not an option.\n\r", *arg2 );
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config A " );
          return;
        }

      if( bit1 != 0 )
        {
        if( !IS_SET( ch->act, bit1 ) )
	    SET_BIT    (ch->act, bit1);
	else
	    REMOVE_BIT (ch->act, bit1);
        }
      if( bit2 != 0 )
        {
        if( !IS_SET( ch->pcdata->player2_bits, bit2 ) )
	    SET_BIT    (ch->pcdata->player2_bits, bit2);
	else
	    REMOVE_BIT (ch->pcdata->player2_bits, bit2);
        }

      strcpy( buf, "\n\r\n\r{130}    Display/MrTerm Setup Menu\n\r\n\r");

      strcat(  buf, IS_SET(ch->act, PLR_AUTOEXIT)
? "{150} (A)  {120}[ ON  ] {010}[AUTOEXIT] {030}You automatically see exits.\n\r"
: "{150} (A)  {020}[ off ] {010}[autoexit] {030}You don't automatically see exits.\n\r" );

      strcat(  buf, IS_SET(ch->act, PLR_QUIET)
? "{150} (B)  {120}[ ON  ] {010}[QUIET   ] {030}You don't see items that spill out of corpses.\n\r"
: "{150} (B)  {020}[ off ] {010}[quiet   ] {030}You see items that spill out of corpses.\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_BLANK)
? "{150} (C)  {120}[ ON  ] {010}[BLANK   ] {030}You have a blank line before your prompt.\n\r"
: "{150} (C)  {020}[ off ] {010}[blank   ] {030}You have no blank line before your prompt.\n\r" );

      strcat( buf,  IS_SET(ch->act, PLR_BRIEF)
? "{150} (D)  {120}[ ON  ] {010}[BRIEF   ] {030}You see brief descriptions.\n\r"
: "{150} (D)  {020}[ off ] {010}[brief   ] {030}You see long descriptions.\n\r" );
	 
      strcat( buf,  !IS_SET(ch->act, PLR_PAGER)
? "{150} (E)  {120}[ ON  ] {010}[PAGER   ]{030} You are using the page pauser.\n\r"
: "{150} (E)  {020}[ off ] {010}[pager   ] {030}You are not using the page pauser.\n\r" );

      strcat( buf,  IS_SET(ch->act, PLR_REPEAT)
? "{150} (F)  {120}[ ON  ] {010}[REPEAT  ] {030}You see what you've typed in.\n\r"
: "{150} (F)  {020}[ off ] {010}[repeat  ] {030}You don't see what you type in.\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_PROMPT)
? "{150} (G)  {120}[ ON  ] {010}[PROMPT  ] {030}You use tactical and not prompt.\n\r"
: "{150} (G)  {020}[ off ] {010}[prompt  ] {030}You use the prompt and not tactical.\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_TELNET_GA)
? "{150} (H)  {120}[ ON  ] {010}[TELNETGA] {030}You receive a telnet GA sequence.\n\r"
: "{150} (H)  {020}[ off ] {010}[telnetga] {030}You don't receive a telnet GA sequence.\n\r");

      strcat( buf, !IS_SET(ch->act, PLR_TERMINAL)
? "{150} (I)  {020}[ off ] {010}[term    ] {030}You do not have MrTerm.\n\r"
: "{150} (I)  {120}[ ON  ] {010}[TERM    ] {030}You have MrTerm activated.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->player2_bits, PLR2_EXP_TO_LEVEL)
? "{150} (J)  {020}[ off ] {010}[exp     ] {030}Your tactical experience is the total.\n\r"
: "{150} (J)  {120}[ ON  ] {010}[EXP     ] {030}Your tactical experience is the required.\n\r");

      strcat(  buf, IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES)
? "{150} (K)  {020}[ off ] {010}[external] {030}You will not download or use external files.\n\r"
: "{150} (K)  {120}[ ON  ] {010}[EXTERNAL] {030}You will download and use external files.\n\r");

      strcat(  buf, IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_METER)
? "{150} (L)  {020}[ off ] {010}[meter   ] {030}You will not show download meter on external files.\n\r"
: "{150} (L)  {120}[ ON  ] {010}[METER   ] {030}You will show download meter on external files.\n\r");

      strcat( buf,  !IS_SET(ch->pcdata->player2_bits, PLR2_MRTERM_STATS)
? "{150} (M)  {020}[ off ] {010}[stats   ] {030}MrTerm will not recieve binary stat information.\n\r"
: "{150} (M)  {120}[ ON  ] {010}[STATS   ] {030}MrTerm will recieve binary stat information.\n\r");

      strcat(  buf, IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF)
? "{150} (N)  {020}[ off ] {010}[ref     ] {030}You do not use item index references.\n\r"
: "{150} (N)  {120}[ ON  ] {010}[REF     ] {030}You use item index references.\n\r");

      strcat( buf, "\n\r{150} (X)  {020}EXIT\n\r" );
      strcat( buf, "{150} (-)  {020}Return\n\r" );


      send_to_char( ansi_translate_text( ch, buf ), ch );
      if( ch->pcdata->last_command != NULL )
        STRFREE( ch->pcdata->last_command );
      ch->pcdata->last_command = str_dup( "config A " );
      return;
      }

      /* General options */
    if ( *arg1 == 'c' || *arg1=='C' )
      {
      if( *arg2 == 'x' || *arg2 == 'X' )
        {
        strcpy( buf, "{130}Configuration finished.\n\r\n\r");
        send_to_char( ansi_translate_text( ch, buf ), ch );
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        return;
        }
      if( *arg2 == '-' )
        {
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        do_config( ch, "" );
        return;
        }

      switch( *arg2 )
        {
        case 'a': case 'A':
          bit1 = PLR_AUTOLOOT;
          break;
        case 'b': case 'B':
          bit1 = PLR_AUTOSAC;
          break;
        case 'c': case 'C':
          bit1 = PLR_DAMAGE;
          break;
        case 'd': case 'D':
          bit1 = PLR_AUTO_SPLIT;
          break;
        case 'e': case 'E':
          bit1 = PLR_CHAT;
          break;
        case 'f': case 'F':
          bit1 = PLR_PLAN;
          break;
        case 'g': case 'G':
          bit2 = PLR2_PEEK;
          break;
        case 'h': case 'H':
          bit2 = PLR2_CASTLES;
          break;
        case 'i': case 'I':
          bit2 = PLR2_BATTLE;
          break;
        case 'j': case 'J':
          bit2 = PLR2_VICTIM_LIST;
          break;

        case '\0':
         break;

        default:
          sprintf( buf, "{110}'%c' Is not an option.\n\r", *arg2 );
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config C " );
          return;
        }

      if( bit1 != 0 )
        {
        if( !IS_SET( ch->act, bit1 ) )
	    SET_BIT    (ch->act, bit1);
	else
	    REMOVE_BIT (ch->act, bit1);
        }
      if( bit2 != 0 )
        {
        if( !IS_SET( ch->pcdata->player2_bits, bit2 ) )
	    SET_BIT    (ch->pcdata->player2_bits, bit2);
	else
	    REMOVE_BIT (ch->pcdata->player2_bits, bit2);
        }

      strcpy( buf, "\n\r\n\r{130}    General Options Menu\n\r\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_AUTOLOOT)
? "{150} (A)  {120}[ ON  ] {010}[AUTOLOOT] {030}You automatically loot corpses.\n\r"
: "{150} (A)  {020}[ off ] {010}[autoloot] {030}You do not automatically loot corpses.\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_AUTOSAC)
? "{150} (B)  {120}[ ON  ] {010}[AUTOSAC ] {030}You automatically sacrifice corpses.\n\r"
: "{150} (B)  {020}[ off ] {010}[autosac ] {030}You do not automatically sacrifice corpses.\n\r");

      strcat( buf,  !IS_SET(ch->act, PLR_DAMAGE)
? "{150} (C)  {120}[ ON  ] {010}[DAMAGE  ] {030}You see damage status in combat.\n\r"
: "{150} (C)  {020}[ off ] {010}[damage  ] {030}You do not see damage status in combat.\n\r");

      strcat( buf,  IS_SET(ch->act, PLR_AUTO_SPLIT)
? "{150} (D)  {120}[ ON  ] {010}[SPLIT   ] {030}You split gold automatically.\n\r"
: "{150} (D)  {020}[ off ] {010}[split   ] {030}You do not split gold automatically.\n\r");

      strcat( buf,  !IS_SET(ch->act, PLR_CHAT)
? "{150} (E)  {120}[ ON  ] {010}[CHAT    ] {030}You hear racial chatter.\n\r"
: "{150} (E)  {020}[ off ] {010}[chat    ] {030}You do not hear racial chatter.\n\r");

      strcat( buf,  !IS_SET(ch->act, PLR_PLAN)
? "{150} (F)  {120}[ ON  ] {010}[PLAN    ] {030}You hear godly plans.\n\r"
: "{150} (F)  {020}[ off ] {010}[plan    ] {030}You do not hear godly plans.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->player2_bits, PLR2_PEEK)
? "{150} (G)  {120}[ ON  ] {010}[PEEK    ] {030}You peek at inventories when given the chance.\n\r"
: "{150} (G)  {020}[ off ] {010}[peek    ] {030}You never peek into inventories.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES)
? "{150} (H)  {120}[ ON  ] {010}[CASTLES ] {030}You do acknowledge castles of players and clans.\n\r"
: "{150} (H)  {020}[ off ] {010}[castles ] {030}You do not acknowledge the existance of castles.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->player2_bits, PLR2_BATTLE)
? "{150} (I)  {120}[ ON  ] {010}[BATTLE  ] {030}You hear about battles being fought.\n\r"
: "{150} (I)  {020}[ off ] {010}[battle  ] {030}You do not hear about battles being fought.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->player2_bits, PLR2_VICTIM_LIST)
? "{150} (J)  {120}[ ON  ] {010}[VICTIM  ] {030}You see a list of people's kills.\n\r"
: "{150} (J)  {020}[ off ] {010}[victim  ] {030}You so not see a list of people's kills.\n\r");


      strcat( buf, "\n\r{150} (X)  {020}EXIT\n\r" );
      strcat( buf, "{150} (-)  {020}Return\n\r" );

      send_to_char( ansi_translate_text( ch, buf ), ch );
      if( ch->pcdata->last_command != NULL )
        STRFREE( ch->pcdata->last_command );
      ch->pcdata->last_command = str_dup( "config C " );
      return;
      }

     /* Combat Spamming configuration */
    if ( arg1[0] == 'd' || arg1[0]=='D' )
      {
      if( *arg2 == 'x' || *arg2 == 'X' )
        {
        strcpy( buf, "{130}Configuration finished.\n\r\n\r");
        send_to_char( ansi_translate_text( ch, buf ), ch );
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        return;
        }
      if( *arg2 == '-' )
        {
        if( ch->pcdata->last_command != NULL )
          STRFREE( ch->pcdata->last_command );
        do_config( ch, "" );
        return;
        }

      switch( *arg2 )
        {
        case 'a': case 'A':
          bit1 = 1;
          break;
        case 'b': case 'B':
          bit1 = 2;
          break;
        case 'c': case 'C':
          bit1 = 4;
          break;
        case 'd': case 'D':
          bit1 = 8;
          break;
        case 'e': case 'E':
          bit1 = 16;
          break;
        case 'f': case 'F':
          bit1 = 32;
          break;
        case 'g': case 'G':
          bit1 = 64;
          break;
        case 'h': case 'H':
          bit1 = 128;
          break;
        case 'i': case 'I':
          bit1 = 256;
          break;
        case 'j': case 'J':
          bit1 = 512;
          break;
        case 'l': case 'L':
          bit1 = 1024;
          break;
        case 'k': case 'K':
          bit1 = 1023;
          break;
        case 'm': case 'M':
          bit1 = 2048;
          break;

        case '\0':
         break;

        default:
          sprintf( buf, "{110}'%c' Is not an option.\n\r", *arg2 );
          send_to_char( ansi_translate_text( ch, buf ), ch );
          if( ch->pcdata->last_command != NULL )
            STRFREE( ch->pcdata->last_command );
          ch->pcdata->last_command = str_dup( "config C " );
          return;
        }

      if( bit1 != 0 )
        {
        if( !IS_SET( ch->pcdata->spam, bit1 ) )
	    SET_BIT    (ch->pcdata->spam, bit1);
	else
	    REMOVE_BIT (ch->pcdata->spam, bit1);
        }

      strcpy( buf, "\n\r\n\r{130}    Combat Spamming Menu\n\r\n\r");


      strcat(  buf, !IS_SET(ch->pcdata->spam, 1)
? "{150} (A)  {120}[ ON  ]  {030}You hit.\n\r"
: "{150} (A)  {020}[ off ]  {030}You hit.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 2)
? "{150} (B)  {120}[ ON  ]  {030}You miss.\n\r"
: "{150} (B)  {020}[ off ]  {030}You miss.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 4)
? "{150} (C)  {120}[ ON  ]  {030}They hit you.\n\r"
: "{150} (C)  {020}[ off ]  {030}They hit you.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 8)
? "{150} (D)  {120}[ ON  ]  {030}They miss you.\n\r"
: "{150} (D)  {020}[ off ]  {030}They miss you.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 16)
? "{150} (E)  {120}[ ON  ]  {030}Party hits.\n\r"
: "{150} (E)  {020}[ off ]  {030}Party hits.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 32)
? "{150} (F)  {120}[ ON  ]  {030}Party misses.\n\r"
: "{150} (F)  {020}[ off ]  {030}Party misses.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 64)
? "{150} (G)  {120}[ ON  ]  {030}They hit party.\n\r"
: "{150} (G)  {020}[ off ]  {030}They hit party.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 128)
? "{150} (H)  {120}[ ON  ]  {030}They miss party.\n\r"
: "{150} (H)  {020}[ off ]  {030}They miss party.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 256)
? "{150} (I)  {120}[ ON  ]  {030}Other hit.\n\r"
: "{150} (I)  {020}[ off ]  {030}Other hit.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 512)
? "{150} (J)  {120}[ ON  ]  {030}Other miss.\n\r"
: "{150} (J)  {020}[ off ]  {030}Other miss.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 1)
? "{150} (K)  {120}[ ON  ]  {030}Control all items A through J.\n\r"
: "{150} (K)  {020}[ off ]  {030}Control all items A through J.\n\r");

      strcat(  buf, IS_SET(ch->pcdata->spam, 1024)
? "{150} (L)  {120}[ ON  ]  {030}Show party status line.\n\r"
: "{150} (L)  {020}[ off ]  {030}Show party status line.\n\r");

      strcat(  buf, !IS_SET(ch->pcdata->spam, 2048)
? "{150} (M)  {120}[ ON  ]  {030}Party movement.\n\r"
: "{150} (M)  {020}[ off ]  {030}Party movement.\n\r");


      strcat( buf, "\n\r{150} (X)  {020}EXIT\n\r" );
      strcat( buf, "{150} (-)  {020}Return\n\r" );

      send_to_char( ansi_translate_text( ch, buf ), ch );
      if( ch->pcdata->last_command != NULL )
        STRFREE( ch->pcdata->last_command );
      ch->pcdata->last_command = str_dup( "config D " );
      return;
      }



    strcpy( buf, "\n\r{130}    Configuration Main Menu\n\r\n\r");
    strcat( buf, "{150} (A)  {020}Display/MrTerm Setup\n\r" );
    strcat( buf, "{150} (B)  {020}VT102/Tactical Setup\n\r" );
    strcat( buf, "{150} (C)  {020}General Options\n\r" );
    strcat( buf, "{150} (D)  {020}Combat Spamming\n\r\n\r" );
    strcat( buf, "{150} (-)  {020}EXIT\n\r" );
    strcat( buf, "{150} (X)  {020}EXIT\n\r" );
    send_to_char( ansi_translate_text( ch, buf ), ch );
    if( ch->pcdata->last_command != NULL )
      STRFREE( ch->pcdata->last_command );
    ch->pcdata->last_command = str_dup( "config " );

  return;
  }

/*

	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "[ Keyword  ] Option\n\r", ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_PEEK)
	    ? "[-peek     ] You don't automatically see inventories.\n\r"
	    : "[+PEEK     ] You automatically see inventories.  (Rogues)\n\r"
	    , ch );

	send_to_char(  !IS_SET(ch->pcdata->player2_bits, PLR2_EXP_TO_LEVEL)
	    ? "[-exp      ] Your tactical experience is the total.\n\r"
	    : "[+EXP      ] Your tactical experience is the required.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES)
	    ? "[-external ] You will not download or use external files.\n\r"
	    : "[+EXTERNAL ] You will download and use external files.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_METER)
	    ? "[-meter    ] You will not show download meter on external files.\n\r"
	    : "[+METER    ] You will show download meter on external files.\n\r"
	    , ch );

	send_to_char(  !IS_SET(ch->pcdata->player2_bits, PLR2_MRTERM_STATS)
	    ? "[-stats    ] MrTerm will not recieve binary stat information.\n\r"
	    : "[+STATS    ] MrTerm will recieve binary stat information.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES))
	  ? "[-castles  ] You do not acknowledge the existence of castles.\n\r"
	  : "[+CASTLES  ] You do acknowledge castles.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_BATTLE))
	  ? "[-battle   ] You do not hear about battles being fought.\n\r"
	  : "[+BATTLE   ] You hear about battles being fought.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_VICTIM_LIST))
	  ? "[-victim   ] You do not see a list of people's kills.\n\r"
	  : "[+VICTIM   ] You see a list of people's kills.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF))
	  ? "[-ref      ] You do not use item index references.\n\r"
	  : "[+REF      ] You use item index references.\n\r"
	    , ch );

    }
    else
    {
	bool fSet;
	int bit;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

	     if ( !strcasecmp( arg+1, "peek" ) ) bit = PLR2_PEEK;
	else
	     if ( !strcasecmp( arg+1, "exp" ) ) bit = PLR2_EXP_TO_LEVEL;
	else
	     if ( !strcasecmp( arg+1, "external" ) ) bit = PLR2_EXTERNAL_FILES;
	else
	     if ( !strcasecmp( arg+1, "meter" ) ) bit = PLR2_EXTERNAL_METER;
	else
	     if ( !strcasecmp( arg+1, "stats" ) ) bit = PLR2_MRTERM_STATS;
	else
	     if ( !strcasecmp( arg+1, "castles" ) ) bit = PLR2_CASTLES;
	else
	     if ( !strcasecmp( arg+1, "battle" ) ) bit = PLR2_BATTLE;
	else
	     if ( !strcasecmp( arg+1, "ref" ) ) bit = PLR2_ITEM_REF;
	else
	     if ( !strcasecmp( arg+1, "victim" ) ) bit = PLR2_VICTIM_LIST;
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}
	if( bit==PLR2_PEEK || bit==PLR2_EXTERNAL_FILES || bit==PLR2_VICTIM_LIST
            || bit==PLR2_EXTERNAL_METER || bit==PLR2_BATTLE 
            || bit==PLR2_CENSOR ||bit==PLR2_CASTLES || bit==PLR2_ITEM_REF)
	  {
	  if(fSet)
	    fSet=FALSE;
	  else
	    fSet=TRUE;
	  }
	if ( fSet )
	    SET_BIT    (ch->pcdata->player2_bits, bit);
	else
	    REMOVE_BIT (ch->pcdata->player2_bits, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
} */

/*  Old configs
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "[ Keyword  ] Option\n\r", ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOEXIT)
	    ? "[+AUTOEXIT ] You automatically see exits.\n\r"
	    : "[-autoexit ] You don't automatically see exits.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOLOOT)
	    ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
	    : "[-autoloot ] You don't automatically loot corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOSAC)
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_QUIET)
	    ? "[+QUIET    ] You don't see items that spill out of corpses.\n\r"
	    : "[-quiet    ] You see items that spill out of corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BLANK)
	    ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
	    : "[-blank    ] You have no blank line before your prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BRIEF)
	    ? "[+BRIEF    ] You see brief descriptions.\n\r"
	    : "[-brief    ] You see long descriptions.\n\r"
	    , ch );
	 
	send_to_char(  !IS_SET(ch->act, PLR_PAGER)
	    ? "[+PAGER    ] You are using the page pauser.\n\r"
	    : "[-pager    ] You are not using the page pauser.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_REPEAT)
	    ? "[+REPEAT   ] You see what you've typed in.\n\r"
	    : "[-repeat   ] You don't see what you type in.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_PROMPT)
	    ? "[+TACT     ] You use tactical and not prompt.\n\r"
	    : "[-tact     ] You use the prompt and not tactical.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_TELNET_GA)
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "[+SILENCE  ] You are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\n\r"
	    , ch );

	send_to_char( IS_SET(ch->act, PLR_DAMAGE)
	    ? "[-damage   ] You do not see damage status in combat.\n\r"
	    : "[+DAMAGE   ] You see damage status in combat.\n\r"
	    , ch );
	send_to_char( !IS_SET(ch->act, PLR_AUTO_SPLIT)
	    ? "[-split    ] You do not split gold automatically.\n\r"
	    : "[+SPLIT    ] You split gold automatically.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->act, PLR_CHAT)
	    ? "[-chat     ] You do not hear racial chatter.\n\r"
	    : "[+CHAT     ] You hear racial chatter.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->act, PLR_PLAN)
	    ? "[-plan     ] You do not hear godly plans.\n\r"
	    : "[+PLAN     ] You hear godly plans.\n\r"
	    , ch );
	send_to_char( !IS_SET(ch->act, PLR_TERMINAL)
	    ? "[-term     ] You do not have MrTerm.\n\r"
	    : "[+TERM     ] You have MrTerm activated.\n\r"
	    , ch );
    }
    else
    {
	bool fSet;
	int bit;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

	     if ( !strcasecmp( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !strcasecmp( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !strcasecmp( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !strcasecmp( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !strcasecmp( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !strcasecmp( arg+1, "pager"   ) ) bit = PLR_PAGER;
	else if ( !strcasecmp( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !strcasecmp( arg+1, "damage"   ) ) bit = PLR_DAMAGE;
	else if ( !strcasecmp( arg+1, "split"    ) ) bit = PLR_AUTO_SPLIT;
	else if ( !strcasecmp( arg+1, "quiet"    ) ) bit = PLR_QUIET;
	else if ( !strcasecmp( arg+1, "repeat"    ) ) bit = PLR_REPEAT;
	else if ( !strcasecmp( arg+1, "chat"    ) ) bit = PLR_CHAT;
	else if ( !strcasecmp( arg+1, "tact"    ) ) bit = PLR_PROMPT;
	else if ( !strcasecmp( arg+1, "plan"    ) ) bit = PLR_PLAN;
	else if ( !strcasecmp( arg+1, "term"    ) ) bit = PLR_TERMINAL;
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}
	if( bit==PLR_DAMAGE || bit==PLR_PAGER || bit==PLR_CHAT ||
            bit==PLR_PLAN  )
	  {
	  if(fSet)
	    fSet=FALSE;
	  else
	    fSet=TRUE;
	  }
	if ( fSet )
	    SET_BIT    (ch->act, bit);
	else
	    REMOVE_BIT (ch->act, bit);

	if( bit==PLR_PROMPT && ch->vt100==1)
	  do_refresh( ch, "");
	send_to_char( "Ok.\n\r", ch );
    }

    return;
}

void do_config2( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "[ Keyword  ] Option\n\r", ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_PEEK)
	    ? "[-peek     ] You don't automatically see inventories.\n\r"
	    : "[+PEEK     ] You automatically see inventories.  (Rogues)\n\r"
	    , ch );

	send_to_char(  !IS_SET(ch->pcdata->player2_bits, PLR2_EXP_TO_LEVEL)
	    ? "[-exp      ] Your tactical experience is the total.\n\r"
	    : "[+EXP      ] Your tactical experience is the required.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES)
	    ? "[-external ] You will not download or use external files.\n\r"
	    : "[+EXTERNAL ] You will download and use external files.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->player2_bits, PLR2_EXTERNAL_METER)
	    ? "[-meter    ] You will not show download meter on external files.\n\r"
	    : "[+METER    ] You will show download meter on external files.\n\r"
	    , ch );

	send_to_char(  !IS_SET(ch->pcdata->player2_bits, PLR2_MRTERM_STATS)
	    ? "[-stats    ] MrTerm will not recieve binary stat information.\n\r"
	    : "[+STATS    ] MrTerm will recieve binary stat information.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES))
	  ? "[-castles  ] You do not acknowledge the existence of castles.\n\r"
	  : "[+CASTLES  ] You do acknowledge castles.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_BATTLE))
	  ? "[-battle   ] You do not hear about battles being fought.\n\r"
	  : "[+BATTLE   ] You hear about battles being fought.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_VICTIM_LIST))
	  ? "[-victim   ] You do not see a list of people's kills.\n\r"
	  : "[+VICTIM   ] You see a list of people's kills.\n\r"
	    , ch );

	send_to_char(  (IS_SET(ch->pcdata->player2_bits, PLR2_ITEM_REF))
	  ? "[-ref      ] You do not use item index references.\n\r"
	  : "[+REF      ] You use item index references.\n\r"
	    , ch );

    }
    else
    {
	bool fSet;
	int bit;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

	     if ( !strcasecmp( arg+1, "peek" ) ) bit = PLR2_PEEK;
	else
	     if ( !strcasecmp( arg+1, "exp" ) ) bit = PLR2_EXP_TO_LEVEL;
	else
	     if ( !strcasecmp( arg+1, "external" ) ) bit = PLR2_EXTERNAL_FILES;
	else
	     if ( !strcasecmp( arg+1, "meter" ) ) bit = PLR2_EXTERNAL_METER;
	else
	     if ( !strcasecmp( arg+1, "stats" ) ) bit = PLR2_MRTERM_STATS;
	else
	     if ( !strcasecmp( arg+1, "castles" ) ) bit = PLR2_CASTLES;
	else
	     if ( !strcasecmp( arg+1, "battle" ) ) bit = PLR2_BATTLE;
	else
	     if ( !strcasecmp( arg+1, "ref" ) ) bit = PLR2_ITEM_REF;
	else
	     if ( !strcasecmp( arg+1, "victim" ) ) bit = PLR2_VICTIM_LIST;
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}
	if( bit==PLR2_PEEK || bit==PLR2_EXTERNAL_FILES || bit==PLR2_VICTIM_LIST
            || bit==PLR2_EXTERNAL_METER || bit==PLR2_BATTLE 
            || bit==PLR2_CENSOR ||bit==PLR2_CASTLES || bit==PLR2_ITEM_REF)
	  {
	  if(fSet)
	    fSet=FALSE;
	  else
	    fSet=TRUE;
	  }
	if ( fSet )
	    SET_BIT    (ch->pcdata->player2_bits, bit);
	else
	    REMOVE_BIT (ch->pcdata->player2_bits, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
} */

void do_disguise( CHAR_DATA *ch, char *argument )
  {
  if(multi(ch,gsn_disguise)==-1 )
    return;

  if ( strlen(argument) > 79 )
    argument[79] = '\0';

  smash_tilde( argument );
  if( IS_NPC( ch ) && ch->long_descr != ch->pIndexData->long_descr 
     && (number_percent()<ch->pcdata->learned[gsn_disguise]))
     STRFREE (ch->long_descr );
  ch->long_descr = STRALLOC(argument);
  send_to_char( "Ok.\n\r", ch );
  }

void do_spy( CHAR_DATA *ch, char *argument )
  {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int door=0,lev;

  if((lev=multi(ch,gsn_spy))==-1)
    return;

  if(IS_NPC(ch))
    lev=ch->level;
  else
    lev=ch->mclass[lev];

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
    send_to_char( "Spy on whom?\n\r", ch );
    return;
    }

  victim=get_char_world(ch,arg);

  if( victim == ch )
    {
    send_to_char( "You pick up your brain, and put it back on your head.\n\r", ch );
    return;
    }

  if((victim!=NULL)&&(!IS_NPC(victim))&&(victim->in_room!=ch->in_room))
    for(door=0;door<6;door++)
      if(ch->in_room!=NULL &&
	 ch->in_room->exit[door]!=NULL &&
	 ch->in_room->exit[door]->to_room==victim->in_room)
	{
	door=0;
	break;
	}
  if ((door!=0)||(victim == NULL )||IS_NPC(victim)||!can_see(ch,victim))
    {
    send_to_char( "They aren't anyway near.\n\r", ch );
    return;
    }

  if((victim->level>lev)||
     which_god( victim ) == GOD_POLICE ||
     (number_percent()>ch->pcdata->learned[gsn_spy]))
    {
    send_to_char( "You failed.\n\r", ch );
    return;
    }

  if( argument==NULL || *argument=='\0' || *argument=='S' || *argument=='s' )
    {
    get_string_score_v1( victim, ch);
    send_to_char( get_string_score_txt, ch);
    return;
    }

  if( *argument=='a' || *argument=='A' )
    {
    char buf[MAX_STRING_LENGTH];
    get_affects_string( victim, ch, buf, MAX_STRING_LENGTH );
    send_to_char( buf, ch );
    return;
    }

  send_to_char("That is not a valid stat to spy.\n\rTry Affects or Score.\n\r",
      ch );

  return;
  }

int which_god( CHAR_DATA *ch)
  {
  int gd;
  bool found;
  OBJ_DATA *obj;

  if(ch->which_god!=-1)
    return(ch->which_god);
  if (IS_NPC(ch)) return 0;
  found = FALSE;
  gd = 0;

  for ( obj = ch->first_carrying; obj != NULL && found==FALSE; obj=obj->next_content )
   {
   if(obj->pIndexData->vnum==50)
      {
      found=TRUE;
      gd=GOD_ORDER;
      }
   if(obj->pIndexData->vnum==51)
      {
      found=TRUE;
      gd=GOD_CHAOS;
      }
   if(obj->pIndexData->vnum==52)
      {
      found=TRUE;
      gd=GOD_DEMISE;
      }
   if(obj->pIndexData->vnum==53)
      {
      found=TRUE;
      gd=GOD_POLICE;
      }
   if(obj->pIndexData->vnum==54)
      {
      found=TRUE;
      gd=GOD_POLICE;
      }
   }
   ch->which_god=gd;
   if (ch->which_god == GOD_DEMISE) 
      ch->pcdata->demisedlevel= ch->level;
 
  return(gd);
  }


void do_timemode( CHAR_DATA *ch, char *argument )
  {
  if( get_trust( ch ) < MAX_LEVEL || IS_NPC( ch))
    return;
  if( IS_SET( ch->act, PLR_WIZTIME))
    {
    REMOVE_BIT( ch->act, PLR_WIZTIME);
    send_to_char( "Times deactivated.\n\r", ch);
    }
  else
    {
    SET_BIT( ch->act, PLR_WIZTIME);
    send_to_char( "Times activated.  Ignore following line.\n\r", ch);
    }
  return;
  }

void do_history( CHAR_DATA *ch, char *argument )
  {
    char buf[MAX_INPUT_LENGTH], *tmp;
    int cnt;

    if( IS_NPC( ch ))
      return;

   if( which_god(ch)==GOD_CHAOS || which_god(ch)==GOD_ORDER ||
       which_god(ch)==GOD_DEMISE || which_god(ch)==GOD_POLICE )
    sprintf( buf, "Your standings:\n\rNeutrals Dead: %3d    Neutrals that have killed you: %3d\n\r Enemies Dead: %3d    Enemies that have killed you:  %3d\n\r                      Monsters that have killed you: %3d\n\r",
	ch->pcdata->history[HISTORY_KILL_PC],
	ch->pcdata->history[HISTORY_KILL_BY_PC],
	ch->pcdata->history[HISTORY_KILL_EN],
	ch->pcdata->history[HISTORY_KILL_BY_EN],
	ch->pcdata->history[HISTORY_KILL_BY_NPC]);
    else
    sprintf( buf, "Your standings:\n\rPlayers Dead: %3d    Players that have killed you:  %3d\n\r                     Monsters that have killed you: %3d\n\r",
	ch->pcdata->history[HISTORY_KILL_PC],
	ch->pcdata->history[HISTORY_KILL_BY_PC],
	ch->pcdata->history[HISTORY_KILL_BY_NPC]);

    send_to_char( buf, ch );

   if( which_god(ch)==GOD_CHAOS || which_god(ch)==GOD_ORDER ||
       which_god(ch)==GOD_DEMISE || which_god(ch)==GOD_POLICE )
    sprintf( buf, "%s's standings:\n\rNeutrals Dead: %3d    Neutrals that have killed %s: %3d\n\rEnemies Dead:  %3d    Enemies that have killed %s:  %3d\n\r                      Monsters that have killed %s: %3d",
	ch->name,
	ch->pcdata->history[HISTORY_KILL_PC],
	ch->name,
	ch->pcdata->history[HISTORY_KILL_BY_PC],
	ch->pcdata->history[HISTORY_KILL_EN],
	ch->name,
	ch->pcdata->history[HISTORY_KILL_BY_EN],
	ch->name,
	ch->pcdata->history[HISTORY_KILL_BY_NPC]);
    else
    sprintf( buf, "%s's standings:\n\rPlayers Dead: %3d    Players that have killed %s:  %3d\n\r                     Monsters that have killed %s: %3d",
	ch->name,
	ch->pcdata->history[HISTORY_KILL_PC],
	ch->name,
	ch->pcdata->history[HISTORY_KILL_BY_PC],
	ch->name,
	ch->pcdata->history[HISTORY_KILL_BY_NPC]);

    act( buf, ch, NULL, NULL, TO_ROOM );

  if( ch->pcdata->army_status > 0 )
    {
    if( ch->pcdata->army_status == 1 )
      sprintf( buf, "Army Status: Active      Kills: %3d    Losses: %3d\n\r",
        ch->pcdata->history[HISTORY_KILL_ARMY],
        ch->pcdata->history[HISTORY_LOSE_ARMY]);
    else
      sprintf( buf, "Army Status: Retired     Kills: %3d    Losses: %3d\n\r",
        ch->pcdata->history[HISTORY_KILL_ARMY],
        ch->pcdata->history[HISTORY_LOSE_ARMY]);
    act( buf, ch, NULL, NULL, TO_ROOM );
    act( buf, ch, NULL, NULL, TO_CHAR );
    }
   if (!IS_SET(ch->pcdata->player2_bits, PLR2_VICTIM_LIST))
   {
    if (ch->pcdata->killname[ch->pcdata->killnum][0] !='\0')
    {
     act( "Victim List:", ch, NULL, NULL, TO_CHAR);
     act( "Victim List:", ch, NULL, NULL, TO_ROOM);
    }
    for (cnt=ch->pcdata->killnum;cnt<MAX_KILL_TRACK;cnt++)
    { 
     if (ch->pcdata->killname[cnt][0]=='\0') continue;
      tmp=ch->pcdata->killname[cnt]+1; 
      act(tmp,ch, NULL, NULL, TO_CHAR);
      act(tmp,ch, NULL, NULL, TO_ROOM);
    }
    for (cnt=0;cnt<ch->pcdata->killnum;cnt++)
    { 
     if (ch->pcdata->killname[cnt][0]=='\0') continue;

      tmp=ch->pcdata->killname[cnt]+1; 
      act(tmp,ch, NULL, NULL, TO_CHAR);
      act(tmp,ch, NULL, NULL, TO_ROOM);
    }
   }
    return;
  }

CHAR_DATA *lookup_char( char *name)
  {
  PLAYER_GAME *gpl;
  
  for( gpl=first_player; gpl!=NULL; gpl=gpl->next)
    if( !strcasecmp( gpl->ch->name, name))
      return( gpl->ch );
  return( NULL);
  }

bool is_char_valid( CHAR_DATA *fch)
  {
  PLAYER_GAME *gpl;
  
  for( gpl=first_player; gpl!=NULL; gpl=gpl->next)
    if( fch==gpl->ch )
      return( TRUE );
  return( FALSE);
  }


void do_usage( CHAR_DATA *ch, char *argument )
  {
  char obuf[MAX_STRING_LENGTH],buf[MAX_INPUT_LENGTH],arg[MAX_STRING_LENGTH];
  bool weekly;
  int  day,hour,topPlayers=0,topRecons=0,topVal=0,curVal,val;
  PLAYER_GAME *fpl;
  int nTotal;

  argument = one_argument( argument, arg );
  weekly=arg[0]=='w';

  if(weekly)
    {/* weekly usage */
    long nPlayers[7],nRecons[7];

    for(day=0;day<7;day++)
      {
      nPlayers[day]=nRecons[day]=0;
      for(hour=0;hour<24;hour++)
	{
	nPlayers[day]+=usage.players[hour][day];
	nRecons[day]+=usage.recons[hour][day];
	}
      nPlayers[day]/=24;  /* Got it. (note from Chaos to Order) */
      nRecons[day]/=24;
      if(topPlayers<nPlayers[day])
	topPlayers=nPlayers[day];
      if(topRecons<nRecons[day])
	topRecons=nRecons[day];
      topVal=(topVal<topPlayers)?topPlayers:topVal;
      }
    topVal=UMAX(topVal,10);
    strcpy(obuf,"MrMUD usage for each day of the week:\n\r");
    for(val=0;val<10;val++)
      {
      curVal=topVal-val*(float)((float)topVal/(float)10.0);
      sprintf(buf,"%4d:",curVal);
      strcat(obuf,buf);
      for(day=0;day<7;day++)
	{
	if(nPlayers[day]>=curVal)
	  strcat(obuf," P");
	else
	  strcat(obuf,"  ");
	strcat(obuf,"  ");
	}
      strcat(obuf,"\n\r");
      }
    strcat(obuf,"    +---+---+---+---+---+---+---+\n\r     Sun Mon Tue Wed Thu Fri Sat\n\r       (P)layers\n\r");
    send_to_char(obuf,ch);
    }
  else
    {/* daily usage */
    day=tme.tm_wday;
    for(hour=0;hour<24;hour++)
      {
      if(hour >= tme.tm_hour)
        {
        if(topPlayers<usage.players[hour][(day+6)%7])
	  topPlayers=usage.players[hour][(day+6)%7];
        if(topRecons<usage.recons[hour][(day+6)%7])
	  topRecons=usage.recons[hour][(day+6)%7];
        }
      else
        {
        if(topPlayers<usage.players[hour][day])
	  topPlayers=usage.players[hour][day];
        if(topRecons<usage.recons[hour][day])
	  topRecons=usage.recons[hour][day];
        }
      }
    topVal=(topVal<topPlayers)?topPlayers:topVal;
    topVal=UMAX(topVal,10);
    strcpy(obuf,"MrMUD usage for each hour of the day:\n\r");
    for(val=0;val<10;val++)
      {
      curVal=topVal-val*(float)((float)topVal/(float)10.0);
      sprintf(buf,"%4d:",curVal);
      strcat(obuf,buf);
      for(hour=0;hour<24;hour++)
        {
        if(hour >= tme.tm_hour)
	  {
	  if(usage.players[hour][(day+6)%7]>=curVal)
	    strcat(obuf,"P  ");
	  else
	    strcat(obuf,"   ");
          }
        else
	  {
	  if(usage.players[hour][day]>=curVal)
	    strcat(obuf,"P  ");
	  else
	    strcat(obuf,"   ");
          }
	}
      strcat(obuf,"\n\r");
      }
    strcat(obuf,"    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n\r     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\r                (P)layers\n\r");
    send_to_char(obuf,ch);
    }

    /* Listing to totals  - Chaos   9/26/96  */
   for( fpl=first_player, nTotal=0; fpl!=NULL; fpl=fpl->next)
    {
    if( IS_SET( fpl->ch->act, PLR_WIZINVIS) && ch->level<(MAX_LEVEL-3))
      continue;
    nTotal++;
    }
  sprintf( obuf, "    %d Current Players out of a maximum of %d Players.\n\r",
      nTotal,  MAXLINKPERPORT );
  send_to_char( obuf, ch );
  return;
  }

/* Stat commands by Chaos - 1/15/96  */

void do_strength( CHAR_DATA *ch, char *arg)
  {
  int st, stlow, sthigh;
  char buf[200];


  send_to_char( "Strength    Weapon Weight   ToHit Bonus   Dam Bonus   Carry Weight\n\r", ch);

  stlow = get_curr_str( ch )-4;
  sthigh = get_curr_str( ch )+4;

  if( stlow < 0)
    stlow = 0;
  if( sthigh > 35 )
    sthigh = 35;

  for( st=stlow; st<=sthigh; st++)
     {
     if( get_curr_str( ch ) == st )
       sprintf( buf, "->%2d         %5d          %5d         %5d         %5d\n\r", st, 
	 str_app[st].wield,  str_app[st].tohit, str_app[st].todam, 
         str_app[st].carry );
     else
       sprintf( buf, "  %2d         %5d          %5d         %5d         %5d\n\r", st, 
	 str_app[st].wield,  str_app[st].tohit, str_app[st].todam, 
         str_app[st].carry );
     send_to_char( buf, ch );
     }

  return;
  }

void do_wisdom( CHAR_DATA *ch, char *arg)
  {
  int st, stlow, sthigh;
  char buf[200];


  send_to_char( "Wisdom      Amount of Practices per Level\n\r", ch);

  stlow = get_curr_wis( ch )-4;
  sthigh = get_curr_wis( ch )+4;

  if( stlow < 0)
    stlow = 0;
  if( sthigh > 35 )
    sthigh = 35;

  for( st=stlow; st<=sthigh; st++)
     {
     if( get_curr_wis( ch ) == st )
       sprintf( buf, "->%2d               %5d\n\r", st, wis_app[st].practice);
     else
       sprintf( buf, "  %2d               %5d\n\r", st, wis_app[st].practice);
     send_to_char( buf, ch );
     }

  return;
  }

void do_intelligence( CHAR_DATA *ch, char *arg)
  {
  int st, stlow, sthigh;
  char buf[200];


  send_to_char( "Intelligence   Percent Learned w/Pracs   Concentration Bonus  Mana Bonus\n\r", ch);

  stlow = get_curr_int( ch )-4;
  sthigh = get_curr_int( ch )+4;

  if( stlow < 0)
    stlow = 0;
  if( sthigh > 35 )
    sthigh = 35;

  for( st=stlow; st<=sthigh; st++)
     {
     if( get_curr_int( ch ) == st )
     sprintf( buf, "->%2d                 %5d                    %5d                 %5d\n\r", st, 
	 int_app[st].learn,  (st - 13)*2, int_app[st].manap);
     else
     sprintf( buf, "  %2d                 %5d                    %5d                 %5d\n\r", st, 
	 int_app[st].learn,  (st - 13)*2, int_app[st].manap);
     send_to_char( buf, ch );
     }

  send_to_char( "Note: Concentration bonus adds to spell's percentage learned at casting time.\n\r", ch);

  return;
  }


void do_dexterity( CHAR_DATA *ch, char *arg)
  {
  int st, stlow, sthigh;
  char buf[200];


  send_to_char( "Dexterity      Bonus AC\n\r", ch );

  stlow = get_curr_dex( ch )-4;
  sthigh = get_curr_dex( ch )+4;

  if( stlow < 0)
    stlow = 0;
  if( sthigh > 35 )
    sthigh = 35;

  for( st=stlow; st<=sthigh; st++)
     {
     if( get_curr_dex( ch ) == st )
       sprintf( buf, "->%2d           %5d\n\r", st, dex_app[st].defensive );
     else
       sprintf( buf, "  %2d           %5d\n\r", st, dex_app[st].defensive );
     send_to_char( buf, ch );
     }

  return;
  }

void do_constitution( CHAR_DATA *ch, char *arg)
  {
  int st, stlow, sthigh;
  char buf[200];


  send_to_char( "Constitution        Bonus HP/Level\n\r", ch );

  stlow = get_curr_con( ch )-4;
  sthigh = get_curr_con( ch )+4;

  if( stlow < 0)
    stlow = 0;
  if( sthigh > 35 )
    sthigh = 35;

  for( st=stlow; st<=sthigh; st++)
     {
     if( get_curr_con( ch ) == st )
     sprintf( buf, "->%2d                 %5d\n\r", st, con_app[st].hitp );
      else
     sprintf( buf, "  %2d                 %5d\n\r", st, con_app[st].hitp );
     send_to_char( buf, ch );
     }

  return;
  }

void do_display( CHAR_DATA *ch, char *arg)
  {
  int x,y,tc,tp,bc,bp;
  char buf[MAX_STRING_LENGTH], buf2[80], *pt, *pto;
  OBJ_DATA *obj;
  CHAR_DATA *victim;

  if( arg[0]=='\0' || IS_NPC( ch ) || ch->vt100!=1 || ch->ansi!=1)
    return;

    if ( ch->desc == NULL )
	return;

    victim = NULL;
    obj = NULL;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !can_see_in_room( ch, ch->in_room ))
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	  return;
    }
  
  obj = get_obj_list( ch, arg, ch->first_carrying );
  if ( obj == NULL )
    {
    victim = get_char_room( ch, arg );
    if( victim == NULL )
      {
      act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
      return;
      }
    }

  if( obj != NULL && obj->pIndexData->picture == NULL )
    {
    act( "There is no picture for $T.", ch, NULL, arg, TO_CHAR );
    return;
    }

  if( victim != NULL && (!IS_NPC( victim) || 
                          victim->pIndexData->picture == NULL))
    {
    act( "There is no picture for $T.", ch, NULL, arg, TO_CHAR );
    return;
    }
	    

  if( obj != NULL )
    pt = obj->pIndexData->picture;
  else
    pt = victim->pIndexData->picture;

if( ch->pcdata->term_info == 0 )
  {
  strcpy(buf, "\033[m");
  for( y=0;y<40;y+=2)
    {
    bp=-1;
    tp=-1;
    strcat( buf, " " );
    for( x=0;x<78;x++)
      {
      tc=(int)( *(pt+y*78+x));
      bc=(int)( *(pt+y*78+78+x));
      if( tp==tc && bp==bc )
        sprintf( buf2, "%c", 220);
      else
      if( tp==tc )
        sprintf( buf2, "\033[%dm%c", 30+bc, 220);
      else
      if( bp==bc )
        sprintf( buf2, "\033[%dm%c", 40+tc, 220);
      else
        sprintf( buf2, "\033[%d;%dm%c", 40+tc, 30+bc, 220);
      strcat( buf, buf2);
      bp = bc;
      tp = tc;
      }
    strcat( buf, "\n\r" );
    if( y == 38 )
      strcat( buf, "\033[m" );
    send_to_char( buf, ch);
    buf[0]='\0';
    }
  }
else
      /* Code for MrTerm display */
  {
  pto = buf;
  *pto = 14;
  pto++;
  *pto = 14;
  pto++;
  for( y=0; y<40; y++)
    for( x=0; x<78; x+=2, pt+=2, pto++)
      *pto = (*pt) + ( *(pt+1) * 8 ) + 48 ;
  *pto = '\0';
  write_to_descriptor( ch->desc, buf, 0 );
  /* write_to_buffer( ch->desc, buf, 1000000 ); */
  }

  return;
  }

void do_glance( CHAR_DATA *ch, char *argument )
  {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if ( ch->desc == NULL )
    return;

  if ( !check_blind( ch ) )
    return;

  if ( !IS_NPC(ch) 
      &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
      &&   !can_see_in_room( ch, ch->in_room )
      &&   !IS_AFFECTED(ch, AFF_INFRARED))
    {
    send_to_char( "It is pitch black ... \n\r", ch );
    show_char_to_char( ch->in_room->first_person, ch );
    return;
    }
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if(( victim = get_char_room_even_hidden( ch, arg1 ) ) != NULL )
    {
    /* see if they succeed */
    if(IS_NPC(ch) || (number_percent( ) < ch->pcdata->learned[gsn_glance]))
      {
      act("You look at $N without $M noticing.", ch, NULL, victim, TO_CHAR);
      /* Make them wizinvis so only imps can see them look */
      SET_BIT(ch->act,PLR_WIZINVIS);
      show_char_to_char_1( victim, ch );
      /* Un wizinvis them so they don't abuse the affect */
      REMOVE_BIT(ch->act,PLR_WIZINVIS);
      }
    else
      {
      act("You notice $N notice you looking at $M.", ch, NULL, victim, TO_CHAR);
      show_char_to_char_1( victim, ch );
      }
    return;
    }
  else
    {
    send_to_char("Look at who?\n\r",ch);
    return;
    }

  return;
  }

/*  This routine returns a static string.  - Chaos 2/24/95 */
/*  Using a -1 in register will ignore colors */

char *get_color_string( CHAR_DATA *ch, int regist , int vt_code )
  {

  if( IS_NPC( ch) ) 
    {
    strcpy( (char *)get_color_string_buf, "" );
    return( (char *)get_color_string_buf );
    }

  if( ch->ansi !=1 && ch->vt100==0 ) 
    {
    strcpy( (char *)get_color_string_buf, "" );
    return( (char *)get_color_string_buf );
    } 

    if( regist >= 0 && regist < COLOR_MAX )
     {
    if( ch->ansi != 0 && 
           (vt_code==-1 || vt_code == 0 || vt_code == 1) && 
            ch->pcdata->term_info != 0 )
      {
      int num;
      if(vt_code==0)
        num = 128;
      else if(vt_code==1)
        num = 128 + 64;
      else
        num = 40;
      num+=((int)ch->pcdata->color[0][regist]-30) +
           ((int)ch->pcdata->color[1][regist]-40) * 8;
      sprintf( (char *)get_color_string_buf, "%c%c", 14, num );
      return( (char *)get_color_string_buf );
      }

     if( ch->ansi==1)
      if( vt_code==0 || vt_code==1 || vt_code==4 || vt_code==5 || vt_code==7 )
        {
        sprintf( (char *)get_color_string_buf, "\033[%d;%d;%dm", vt_code,
          ch->pcdata->color[0][regist], ch->pcdata->color[1][regist]);
        return( (char *)get_color_string_buf );
        }

     }

    if( ch->ansi != 0 && ch->vt100 != 0 &&
            ch->pcdata->term_info != 0 )
      {
      int num;
      if( vt_code == 0 )
        num = 32;
      else
        num = 33;
      sprintf( (char *)get_color_string_buf, "%c%c", 14, num );
      return( (char *)get_color_string_buf );
      }

      if( vt_code == 1 || vt_code == 4 || vt_code == 5 || vt_code == 7 )
        {
        sprintf( (char *)get_color_string_buf, "\033[%dm", vt_code );
        return( (char *)get_color_string_buf );
        }


      if( vt_code == 0 )
        {
        strcpy((char *)get_color_string_buf,"\033[m");
        return( (char *)get_color_string_buf );
        }

      strcpy((char *)get_color_string_buf, "");
  return( (char *)get_color_string_buf );

  }

ROOM_INDEX_DATA * RoomDir( CHAR_DATA *ch, ROOM_INDEX_DATA *in_room,
          int dir, int dist)
   {
   ROOM_INDEX_DATA *room;
   int cnt;

   cnt = dist;
   room = in_room;

   while( cnt > 0 )
     {
     if( room->exit[ dir ] == NULL )
       return( NULL );

     if(IS_SET(room->exit[dir]->exit_info, EX_CLOSED))
       return( NULL );
     if(room->exit[dir]->to_room==NULL)
       return( NULL );

     if(  (IS_SET(room->exit[dir]->exit_info,EX_HIDDEN) &&
	  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))  ||
	(!can_use_exit(ch, room, dir)))
        return( NULL );

     room = room->exit[ dir ]->to_room;
     cnt--;
     }

  return( room );
  }

bool is_valid_exit( CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir )
  {
     if( room->exit[ dir ] == NULL )
       return( FALSE );

     if(IS_SET(room->exit[dir]->exit_info, EX_CLOSED))
       return( FALSE );
     if(room->exit[dir]->to_room==NULL)
       return( FALSE );

     if(  (IS_SET(room->exit[dir]->exit_info,EX_HIDDEN) &&
	  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))  ||
	  !can_use_exit(ch, room, dir) )
        return( FALSE );

   return( TRUE );
   }

bool is_valid_door( CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir )
  {
     if( room->exit[ dir ] == NULL )
       return( FALSE );

     if(room->exit[dir]->to_room==NULL)
       return( FALSE );

     if(  (IS_SET(room->exit[dir]->exit_info,EX_HIDDEN) &&
	  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN))  ||
	  !can_use_exit(ch, room, dir) )
        return( FALSE );

   return( TRUE );
   }

void make_length( char *arg, int len )
  {
  int leng, spt, cnt;
  char tbuf[MAX_INPUT_LENGTH];
  *(arg+len) = '\0';
  leng = strlen( arg );
  spt = (len-leng)/2;
  if( spt > 0 )
    {
    strcpy( tbuf, arg );
    for( cnt=0; cnt<spt; cnt++)
      *(arg+cnt) = ' ';
    strcpy( arg+spt, tbuf );
    }
  leng = strlen( arg );
  for( ; leng<len; leng++)
    *(arg+leng) = ' ';
  *(arg+len) = '\0';
    
   return;
  }

void strNameRoom( char *buf, ROOM_INDEX_DATA *room )
  {
          int offset;
    char *buf3;

          offset = 0;
    buf3 = room->name ;
         
             if( *buf3=='t' || *buf3=='T' )
             if( *(buf3+1)=='h' || *(buf3+1)=='H' )
             if( *(buf3+2)=='e' || *(buf3+2)=='E' )
             if( *(buf3+3)==' ' )
               offset = 4;

             if( *buf3=='a' || *buf3=='A' )
             if( *(buf3+1)==' ' )
               offset = 2;

   strcpy( buf, capitalize( buf3 + offset) );
   return;
  }

void do_map( CHAR_DATA *ch, char *argument )
  {
  char buf[MAX_STRING_LENGTH];
  char tbuf1[MAX_INPUT_LENGTH];
  char tbuf3[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *room1, *room2;
  int wwords, wrds;
  int look_range, col;
  int cnt;
  int map_range;

  if( IS_NPC( ch ) )
    return;

  map_range = atoi( argument );

  look_range=ch->pcdata->learned[gsn_long_range]>0?LONG_RANGE:
	  ch->pcdata->learned[gsn_med_range]>0?MED_RANGE:
	  ch->pcdata->learned[gsn_short_range]>0?SHORT_RANGE:0;

  if( map_range > 0 && map_range < look_range )
    look_range = map_range;

  wrds = 1 + look_range * 2;
  wwords = (79 - 2*(1+look_range*2))/ wrds;
  wwords = UMIN( wwords, 12 );

  strcpy( buf, vt_command_1( ch, 0, 'm' ));
  *tbuf3='\0';
  room1 = NULL;
  room2 = NULL;

        /* The north and up directions */
  for( cnt=look_range; cnt>0; cnt--)
    {
    room1 = RoomDir( ch, ch->in_room, 0, cnt);
    room2 = RoomDir( ch, ch->in_room, 4, cnt);
    if( room1==NULL && room2==NULL )
      continue;

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Top alignment */
    make_length( tbuf3, look_range*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      make_length( tbuf3, 1+wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 0 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room1, 0 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2 - 1  );
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 4 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room1, 4 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      make_length( tbuf3, 1+wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 0 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room2, 0 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2 - 1  );
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 4 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room2, 4 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Center alignment */
    make_length( tbuf3, look_range*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      if( is_valid_exit( ch, room1, 3 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room1, 3 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      col = get_sector_color( ch, room1->sector_type );
      strcat( buf, vt_command_3( ch, 0, col % 8 + 30,col / 8 + 40, 'm' ));
      strNameRoom( tbuf1, room1 );
      make_length( tbuf1, wwords );
      strcat( buf, tbuf1 );
      strcat( buf, vt_command_1( ch, 0, 'm' ));
      if( is_valid_exit( ch, room1, 1 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room1, 1 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      if( is_valid_exit( ch, room2, 3 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room2, 3 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      col = get_sector_color( ch, room2->sector_type );
      strcat( buf, vt_command_3( ch, 0, col % 8 + 30,col / 8 + 40, 'm' ));
      strNameRoom( tbuf1, room2 );
      make_length( tbuf1, wwords );
      strcat( buf, tbuf1 );
      strcat( buf, vt_command_1( ch, 0, 'm' ));
      if( is_valid_exit( ch, room2, 1 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room2, 1 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Bottom alignment */
    make_length( tbuf3, look_range*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      if( is_valid_exit( ch, room1, 5 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room1, 5 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 2 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room1, 2 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2  );
      strcat( buf, tbuf3 );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      if( is_valid_exit( ch, room2, 5 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room2, 5 ) )
         strcat( buf, "/" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 2 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room2, 2 ) )
         strcat( buf, "|" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );
    }
   
    /* The center row, top alignment */

    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2  );
    strcat( buf, tbuf3 );
    for( cnt=look_range; cnt>=(0-look_range); cnt--)
      {
      if( cnt>0 )
        room1 = RoomDir( ch, ch->in_room, 3, cnt);
      else
      if( cnt==0 )
        room1 = ch->in_room;
      else
        room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
      if( room1 == NULL )
        {
        make_length( tbuf3, wwords + 2 );
        strcat( buf, tbuf3 );
        }
      else
        {
        make_length( tbuf3, wwords/2+1 );
        strcat( buf, tbuf3 );
        if( is_valid_exit( ch, room1, 0 ) )
          strcat( buf, "|" );
        else
        if( is_valid_door( ch, room1, 0 ) )
          strcat( buf, "+" );
        else
          strcat( buf, " " );
        make_length( tbuf3, wwords - wwords/2 - 1  );
        strcat( buf, tbuf3 );
        if( is_valid_exit( ch, room1, 4 ) )
          strcat( buf, "/" );
        else
        if( is_valid_door( ch, room1, 4 ) )
          strcat( buf, "X" );
        else
          strcat( buf, " " );
        }
      }
    strcat( buf, "\n\r" );

    /* The center row, center alignment */

    *tbuf3='\0';
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );
    for( cnt=look_range; cnt>=(0-look_range); cnt--)
      {
      if( cnt>0 )
        room1 = RoomDir( ch, ch->in_room, 3, cnt);
      else
      if( cnt==0 )
        room1 = ch->in_room;
      else
        room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
      if( room1 == NULL )
        {
        make_length( tbuf3, wwords + 2 );
        strcat( buf, tbuf3 );
        }
      else
        {
        if( is_valid_exit( ch, room1, 3 ) )
          strcat( buf, "-" );
        else
        if( is_valid_door( ch, room1, 3 ) )
          strcat( buf, "+" );
        else
          strcat( buf, " " );
        col = get_sector_color( ch, room1->sector_type );
        strcat( buf, vt_command_3( ch, 0, col % 8 + 30,col / 8 + 40, 'm' ));
        if( ch->in_room == room1 )
          strcpy( tbuf1, "You are here" );
        else
          strNameRoom( tbuf1, room1 );
        make_length( tbuf1, wwords );
        strcat( buf, tbuf1 );
        strcat( buf, vt_command_1( ch, 0, 'm' ));
        if( is_valid_exit( ch, room1, 1 ) )
          strcat( buf, "-" );
        else
        if( is_valid_door( ch, room1, 1 ) )
          strcat( buf, "+" );
        else
          strcat( buf, " " );
        }
      }
    strcat( buf, "\n\r" );

    /* The center row, bottom alignment */

    *tbuf3='\0';
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );
    for( cnt=look_range; cnt>=(0-look_range); cnt--)
      {
      if( cnt>0 )
        room1 = RoomDir( ch, ch->in_room, 3, cnt);
      else
      if( cnt==0 )
        room1 = ch->in_room;
      else
        room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
      if( room1 == NULL )
        {
        make_length( tbuf3, wwords + 2 );
        strcat( buf, tbuf3 );
        }
      else
        {
        if( is_valid_exit( ch, room1, 5 ) )
          strcat( buf, "/" );
        else
        if( is_valid_door( ch, room1, 5 ) )
          strcat( buf, "X" );
        else
          strcat( buf, " " );
        make_length( tbuf3, wwords/2 );
        strcat( buf, tbuf3 );
        if( is_valid_exit( ch, room1, 2 ) )
          strcat( buf, "|" );
        else
        if( is_valid_door( ch, room1, 2 ) )
          strcat( buf, "+" );
        else
          strcat( buf, " " );
        make_length( tbuf3, wwords - wwords/2 );
        strcat( buf, tbuf3 );
        }
      }
    strcat( buf, "\n\r" );
 

        /* The Down and South directions */
  for( cnt=1; cnt<=look_range; cnt++)
    {
    room1 = RoomDir( ch, ch->in_room, 5, cnt);
    room2 = RoomDir( ch, ch->in_room, 2, cnt);
    if( room1==NULL && room2==NULL )
      continue;

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Top alignment */
    make_length( tbuf3, (look_range-cnt)*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      make_length( tbuf3, 1+wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 0 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room1, 0 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2 - 1  );
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 4 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room1, 4 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      make_length( tbuf3, 1+wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 0 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room2, 0 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2 - 1  );
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 4 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room2, 4 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Center alignment */
    make_length( tbuf3, (look_range-cnt)*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      if( is_valid_exit( ch, room1, 3 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room1, 3 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      col = get_sector_color( ch, room1->sector_type );
      strcat( buf, vt_command_3( ch, 0, col % 8 + 30,col / 8 + 40, 'm' ));
      strNameRoom( tbuf1, room1 );
      make_length( tbuf1, wwords );
      strcat( buf, tbuf1 );
      strcat( buf, vt_command_1( ch, 0, 'm' ));
      if( is_valid_exit( ch, room1, 1 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room1, 1 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      if( is_valid_exit( ch, room2, 3 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room2, 3 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      col = get_sector_color( ch, room2->sector_type );
      strcat( buf, vt_command_3( ch, 0, col % 8 + 30,col / 8 + 40, 'm' ));
      strNameRoom( tbuf1, room2 );
      make_length( tbuf1, wwords );
      strcat( buf, tbuf1 );
      strcat( buf, vt_command_1( ch, 0, 'm' ));
      if( is_valid_exit( ch, room2, 1 ) )
         strcat( buf, "-" );
      else
      if( is_valid_door( ch, room2, 1 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );

       /* Correct for centering */
    make_length( tbuf3, (79-(look_range*2+1)*(wwords+2))/2);
    strcat( buf, tbuf3 );

       /* Bottom alignment */
    make_length( tbuf3, (look_range-cnt)*(wwords+2));
    strcat( buf, tbuf3 );
    if( room1 != NULL )
      {
      if( is_valid_exit( ch, room1, 5 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room1, 5 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room1, 2 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room1, 2 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords - wwords/2  );
      strcat( buf, tbuf3 );
      }
    else
      {
      make_length( tbuf3, wwords+2 );
      strcat( buf, tbuf3 );
      }

    make_length( tbuf3, (wwords+2)*( cnt-1 ) );
    strcat( buf, tbuf3 );
    if( room2 != NULL )
      {
      if( is_valid_exit( ch, room2, 5 ) )
         strcat( buf, "/" );
      else
      if( is_valid_door( ch, room2, 5 ) )
         strcat( buf, "X" );
      else
         strcat( buf, " " );
      make_length( tbuf3, wwords/2);
      strcat( buf, tbuf3 );
      if( is_valid_exit( ch, room2, 2 ) )
         strcat( buf, "|" );
      else
      if( is_valid_door( ch, room2, 2 ) )
         strcat( buf, "+" );
      else
         strcat( buf, " " );
      }
    strcat( buf, "\n\r" );
    }
   

  send_to_char( buf, ch );

  return;
  }

char Most_Type[MOST_MOST][30]=
  {  "Experience", "Exp Lost", "Reincarnate", "Killed Enemies",
    "Killed Neutrals", "Killed", "Killed by Enemies", "Killed by Neutrals",
    "Hit Points", "Mana Points", "Move Points", "Armor Class",
    "Damage Roll", "Hit Roll", "Castle Cost", "Hours", "Save Vs Spell" };

void check_most( CHAR_DATA *ch )
  {

  if( IS_NPC( ch ) || ch->level > 96 || ch->which_god==GOD_POLICE )
    return;

  if( ch->exp > Most_Values[ MOST_EXP ] )
    {
    Most_Values[ MOST_EXP ] = ch->exp;
    strcpy( Most_Names[ MOST_EXP ], get_name( ch ) );
    }

  if( ch->exp_lost > Most_Values[ MOST_EXP_LOST ] )
    {
    Most_Values[ MOST_EXP_LOST ] = ch->exp_lost;
    strcpy( Most_Names[ MOST_EXP_LOST ], get_name( ch ) );
    }

  if( ch->pcdata->reincarnation > Most_Values[ MOST_REINCARNATE ] )
    {
    Most_Values[ MOST_REINCARNATE ] = ch->pcdata->reincarnation;
    strcpy( Most_Names[ MOST_REINCARNATE ], get_name( ch ) );
    }

   if( which_god(ch)==GOD_CHAOS || which_god(ch)==GOD_ORDER ||
       which_god(ch)==GOD_DEMISE || which_god(ch)==GOD_POLICE )
  if( ch->pcdata->history[HISTORY_KILL_EN] > Most_Values[ MOST_KILL_EN ] )
    {
    Most_Values[ MOST_KILL_EN ] = ch->pcdata->history[HISTORY_KILL_EN];
    strcpy( Most_Names[ MOST_KILL_EN ], get_name( ch ) );
    }

  if( ch->pcdata->history[HISTORY_KILL_PC] > Most_Values[ MOST_KILL_PC ] )
    {
    Most_Values[ MOST_KILL_PC ] = ch->pcdata->history[HISTORY_KILL_PC];
    strcpy( Most_Names[ MOST_KILL_PC ], get_name( ch ) );
    }

  if( ch->pcdata->history[HISTORY_KILL_BY_NPC]>Most_Values[MOST_KILL_BY_NPC] )
    {
    Most_Values[ MOST_KILL_BY_NPC ] = ch->pcdata->history[HISTORY_KILL_BY_NPC];
    strcpy( Most_Names[ MOST_KILL_BY_NPC ], get_name( ch ) );
    }

   if( which_god(ch)==GOD_CHAOS || which_god(ch)==GOD_ORDER ||
       which_god(ch)==GOD_DEMISE || which_god(ch)==GOD_POLICE )
  if( ch->pcdata->history[HISTORY_KILL_BY_EN]>Most_Values[ MOST_KILL_BY_EN ] )
    {
    Most_Values[ MOST_KILL_BY_EN ] = ch->pcdata->history[HISTORY_KILL_BY_EN];
    strcpy( Most_Names[ MOST_KILL_BY_EN ], get_name( ch ) );
    }

  if( ch->pcdata->history[HISTORY_KILL_BY_PC]>Most_Values[MOST_KILL_BY_PC] )
    {
    Most_Values[ MOST_KILL_BY_PC] = ch->pcdata->history[HISTORY_KILL_BY_PC];
    strcpy( Most_Names[ MOST_KILL_BY_PC], get_name( ch ) );
    }

  if( ch->max_hit > Most_Values[ MOST_HP ] )
    {
    Most_Values[ MOST_HP ] = ch->max_hit;
    strcpy( Most_Names[ MOST_HP ], get_name( ch ) );
    }

  if( ch->max_mana > Most_Values[ MOST_MANA ] )
    {
    Most_Values[ MOST_MANA ] = ch->max_mana;
    strcpy( Most_Names[ MOST_MANA ], get_name( ch ) );
    }

  if(  GET_AC( ch ) < Most_Values[ MOST_AC ] )
    {
    Most_Values[ MOST_AC ] = GET_AC( ch );
    strcpy( Most_Names[ MOST_AC ], get_name( ch ) );
    }

  if( GET_DAMROLL( ch ) > Most_Values[ MOST_DR ] )
    {
    Most_Values[ MOST_DR ] = GET_DAMROLL( ch );
    strcpy( Most_Names[ MOST_DR ], get_name( ch ) );
    }

  if( GET_HITROLL( ch ) > Most_Values[ MOST_HR ] )
    {
    Most_Values[ MOST_HR ] = GET_HITROLL( ch );
    strcpy( Most_Names[ MOST_HR ], get_name( ch ) );
    }

  if( ch->pcdata->castle != NULL &&
      ch->pcdata->castle->cost > Most_Values[ MOST_CASTLE ] )
    {
    Most_Values[ MOST_CASTLE ] = ch->pcdata->castle->cost;
    strcpy( Most_Names[ MOST_CASTLE ], get_name( ch ) );
    }

  if( ch->played/3600+ch->pcdata->previous_hours > Most_Values[ MOST_HOURS ] )
    {
    Most_Values[ MOST_HOURS ] = ch->played/3600+ch->pcdata->previous_hours;
    strcpy( Most_Names[ MOST_HOURS ], get_name( ch ) );
    }

  if( ch->played/3600+ch->pcdata->previous_hours > Most_Values[ MOST_HOURS ] )
    {
    Most_Values[ MOST_HOURS ] = ch->played/3600+ch->pcdata->previous_hours;
    strcpy( Most_Names[ MOST_HOURS ], get_name( ch ) );
    }
  if( GET_SAVING_THROW (ch) < Most_Values[ MOST_SAVE ] )
    {
    Most_Values[ MOST_SAVE ] = GET_SAVING_THROW (ch);
    strcpy( Most_Names[ MOST_SAVE ], get_name( ch ) );
    }
  return;
  }

void do_most( CHAR_DATA *ch, char *argument )
  {
  char buf[ MAX_STRING_LENGTH];
  char tbuf[ MAX_INPUT_LENGTH];
  int cnt, leng;

  send_to_char( "Information on the most of stuff.\n\r", ch);

  leng = 0;
  *buf = '\0';
  for( cnt=0; cnt< MOST_MOST; cnt++)
   if( Most_Names[cnt]!=NULL && *Most_Names[cnt]!='\0' &&
       Most_Values[cnt]!=0 )
    {
    sprintf( tbuf, "%20s %10d %s\n\r", Most_Type[ cnt ],
        Most_Values[ cnt ], Most_Names[ cnt ] );
    leng = str_apd_max( buf, tbuf, leng, MAX_STRING_LENGTH );
    }

  send_to_char( buf, ch );
  return;
  }

void do_victory_list( CHAR_DATA *ch, char *argument )
{
  char *pt, buf[ MAX_STRING_LENGTH], tbuf[MAX_INPUT_LENGTH];
  int cnt;

    strcpy( buf, "                             {130}VICTORY LIST\n\r" );
    sprintf( tbuf, "            Victor                     Defeated               Time{140}\n\r");
    strcat( buf, tbuf );

    for( cnt=0; cnt<VICTORY_LIST_SIZE; cnt++)
     if( strcasecmp(victory_list[cnt], ""))
      {
       pt = victory_list[cnt]+1;
       sprintf( tbuf, "%s {140}\n\r", pt );
       strcat( buf, tbuf );
      }

    send_to_char( ansi_translate_text(ch,buf), ch );

    return;
}

void do_army( CHAR_DATA *ch, char *argument )
  {
  PLAYER_GAME *fpl;
  int race_pick, cnt;
  bool list;
  char buf[MAX_STRING_LENGTH];
  char *pt, first_word[MAX_INPUT_LENGTH];

  cnt=0;
  list = FALSE;

  if( argument == NULL || *argument == '\0' )
    {
    list = TRUE;
    race_pick = ch->race;
    }
  else
    {
    for(pt=first_word ; *argument!=' ' && *argument !='\0'; argument ++, pt++)
      *pt=*argument;
    *pt='\0';
    while( *argument==' ' )
      argument++;
    if( !strcasecmp( first_word, "join" ) )
      {
      if( which_god(ch)==GOD_INIT_CHAOS || which_god(ch)==GOD_INIT_ORDER ||
          which_god(ch)==GOD_DEMISE || which_god(ch)==GOD_POLICE )
         {
         send_to_char( "Your current status does not allow you to join an army.\n\r",ch);
         return;
         }
      if( ch->pcdata->army_status == 1 )
         {
         send_to_char( "You are already in the army!\n\r",ch);
         return;
         }
      if( ch->pcdata->army_status > 1 )
         {
         send_to_char( "You may not rejoin the army!\n\r",ch);
         return;
         }
      if( ch->in_room != room_index[ (army_HQ[ ch->race ]) ] )
         {
         send_to_char( "You must be at your army head-quarters to join.\n\r",ch);
         return;
         }
      if( ch->level < 10 )
         {
         send_to_char( "You must 10th level to join.\n\r",ch);
         return;
         }
      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
         {
         send_to_char( "You must enter your password to join.\n\r",ch);
         return;
         }
      send_to_char( "You join the army!\n\r", ch );
      ch->pcdata->army_status = 1;
      return;
      }
    if( !strcasecmp( first_word, "retire" ) )
      {
      if( ch->pcdata->army_status < 1 )
         {
         send_to_char( "You are not in the army!\n\r",ch);
         return;
         }
      if( ch->pcdata->army_status > 1 )
         {
         send_to_char( "You may not retire twice!\n\r",ch);
         return;
         }
      if( ch->in_room != room_index[ (army_HQ[ ch->race ]) ] )
         {
         send_to_char( "You must be at your army head-quarters to retire.\n\r",ch);
         return;
         }
      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
         {
         send_to_char( "You must enter your password to retire.\n\r",ch);
         return;
         }
      send_to_char( "You retire from the army.\n\r", ch );
      ch->pcdata->army_status = 2;
      return;
      }
    race_pick = lookup_race( first_word );
    if( race_pick == -1 )
      {
      send_to_char( "That is not a valid race name.\n\r", ch );
      return;
      }
    list = TRUE;
    }

  if( list )
    {
    sprintf(buf,"{150}The %s Army{130}\n\r",capitalize(race_table[race_pick].race_name));
    for( cnt=0, fpl=first_player; fpl!=NULL; fpl=fpl->next)
     if( fpl->ch->race == race_pick && fpl->ch->pcdata->army_status == 1 
   && !( IS_SET(fpl->ch->act,PLR_WIZINVIS)
      || (is_affected(fpl->ch, gsn_greater_stealth) && !(IS_SET(ch->act, PLR_HOLYLIGHT) || (is_affected(ch, gsn_truesight) && ch->mclass[CLASS_ILLUSIONIST] >= fpl->ch->level) ))))

      {
      if( cnt%4 !=3 )
        sprintf( first_word, "%-19s", capitalize(fpl->ch->name));
      else
        sprintf( first_word, "%-19s\n\r", capitalize(fpl->ch->name));
      cnt++;
      strcat( buf, first_word );
      }
    send_to_char( ansi_translate_text(ch,buf), ch );
    return;
    }

  return;
  }

  /* Clan Code      - Chaos 11/22/97 */


void do_afk( CHAR_DATA *ch, char *argument )
{
     if ( IS_NPC(ch) )
     return;

     if IS_SET(ch->pcdata->player2_bits, PLR2_AFK)
     {
        REMOVE_BIT(ch->pcdata->player2_bits, PLR2_AFK);
        send_to_char( "You are no longer afk.\n\r", ch );
        act("$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
     }
     else
     {
        SET_BIT(ch->pcdata->player2_bits, PLR2_AFK);
        send_to_char( "You are now afk.\n\r", ch );
        act("$n is now afk.", ch, NULL, NULL, TO_ROOM);
        return;
     }
}

char *get_bar_graph( CHAR_DATA *ch, int size, int percent )
  {
  char buf[MAX_INPUT_LENGTH];
  int cnt, leng, cut;


  *get_bar_graph_txt='\0';
  if( IS_NPC(ch))
    return( get_bar_graph_txt );

  if( size<2 )
    size=2;
  if( size > 78 )
    size = 78;
  if( percent<0 )
    percent = 0;
  if( percent>100 )
    percent = 100;

  if( percent < 25 )
    leng = str_cpy_max( buf, "{171}", MAX_INPUT_LENGTH);
  else
  if( percent < 50 )
    leng = str_cpy_max( buf, "{173}", MAX_INPUT_LENGTH);
  else
  if( percent < 75 )
    leng = str_cpy_max( buf, "{172}", MAX_INPUT_LENGTH);
  else
    leng = str_cpy_max( buf, "{174}", MAX_INPUT_LENGTH);

  leng = str_apd_max( buf, "[", leng, MAX_INPUT_LENGTH);

  cut = size * percent / 100;
  if( cut==size )
    cut--;
  for( cnt=1; cnt<cut; cnt++)
   if( ch->vt100>0 )
    leng = str_apd_max( buf, "+", leng, MAX_INPUT_LENGTH);
   else
    leng = str_apd_max( buf, "H", leng, MAX_INPUT_LENGTH);

  if( percent == 100 )
    {
    leng = str_apd_max( buf, "]{000}", leng, MAX_INPUT_LENGTH);
    strcpy( get_bar_graph_txt, ansi_translate_text( ch, buf ) );
    return( get_bar_graph_txt );
    }

  leng = str_apd_max( buf, "{170}", leng, MAX_INPUT_LENGTH);
  for( cnt++; cnt<size; cnt++)
    leng = str_apd_max( buf, "-", leng, MAX_INPUT_LENGTH);
  leng = str_apd_max( buf, "]{000}", leng, MAX_INPUT_LENGTH);
  
  strcpy( get_bar_graph_txt, ansi_translate_text( ch, buf ) );
  return( get_bar_graph_txt );
  }


void do_protect( CHAR_DATA *ch, char *arg)
  {
  int loss_hp, loss_mana, loss_move, cnt, cnt2;

  DISALLOW_SNOOP = TRUE;

  if(IS_NPC(ch)||ch->position==POS_FIGHTING||ch->fighting!=NULL)
    {
    send_to_char( "You may not do that now.\n\r", ch);
    wipe_string( arg );
    return;
    }

  if( which_god(ch)!=GOD_NEUTRAL )
    {
    if( which_god(ch)==GOD_INIT_CHAOS || which_god(ch)==GOD_INIT_ORDER )
      send_to_char("You are already protected by a God.\n\r",ch);
    else
    if( which_god(ch)==GOD_CHAOS || which_god(ch)==GOD_ORDER || 
        which_god(ch)==GOD_DEMISE )
      send_to_char("You follow a God now and cannot become an initiate.\n\r",ch);
    else
      send_to_char("You cannot become an initiate.\n\r",ch);
    wipe_string( arg );
    return;
    }

  if( ch->pcdata->army_status == 1 )
    {
    send_to_char( "You may not do that while in the army.\n\r", ch);
    wipe_string( arg );
    return;
    }

  if ( IS_SET(ch->act, PLR_KILLER ) || ch->mclass[CLASS_ASSASSIN]>0 )
    {
    send_to_char( "You are a Killer, and no God will not protect you.\n\r", ch);
    wipe_string( arg );
    return;
    }


  if( ch->in_room->vnum!=9719 && ch->in_room->vnum!=9799 )
    {
    send_to_char( "You cannot ask for the protection of a God here.\n\r", ch);
    wipe_string( arg );
    return;
    }

  if( arg[0]=='\0')
    {
    send_to_char( "You must use a password with this command.\n\r", ch);
    wipe_string( arg );
    return;
    }

  if( !strcmp(crypt(arg,ch->pcdata->pwd),ch->pcdata->pwd) )
    {
    wipe_string( arg );
    if( ch->in_room->vnum == 9799 )
        {
        ch->which_god = GOD_INIT_ORDER;
        send_to_char( "Lord Order accepts your request for protection.\n\rYou are now an initiate of Order.\n\r", ch);
        }
    else
    if( ch->in_room->vnum == 9719 )
        {
        ch->which_god = GOD_INIT_CHAOS;
        send_to_char( "Lord Chaos accepts your request for protection.\n\rYou are now an initiate of Chaos.\n\r", ch);
        }
    else
      return;

    /* Let's lose some stats here -  Chaos 5/2/99  */
    loss_hp=0;
    loss_mana=0;
    loss_move=0;
    for(cnt=0; cnt<MAX_CLASS; cnt++)
      for(cnt2=0; cnt2<ch->mclass[cnt]; cnt2++)
        {
        loss_hp   += initiate_hp_loss( cnt, ch->pcdata->reincarnation );
        loss_mana += initiate_mana_loss( cnt, ch->pcdata->reincarnation );
        loss_move += initiate_move_loss( cnt, ch->pcdata->reincarnation );
        }

    loss_hp   = UMAX( loss_hp   * (ch->level-25) / ch->level, 0);
    loss_mana = UMAX( loss_mana * (ch->level-25) / ch->level, 0);
    loss_move = UMAX( loss_move * (ch->level-25) / ch->level, 0);

    ch->actual_max_hit -= loss_hp;
    ch->actual_max_mana -= loss_mana;
    ch->actual_max_move -= loss_move;
    ch->max_hit -= loss_hp;
    ch->max_mana -= loss_mana;
    ch->max_move -= loss_move;

    save_char_obj(ch, NORMAL_SAVE);
    return;
    }
  else
    {
    send_to_char( "That is not your password.\n\r", ch);
    wipe_string( arg );
    return;
    }
  wipe_string( arg );
  return;
  }


