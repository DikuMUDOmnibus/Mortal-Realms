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
 * Local functions.
 */
int hit_gain args ((CHAR_DATA * ch));
int mana_gain args ((CHAR_DATA * ch));
int move_gain args ((CHAR_DATA * ch));
void mobile_update args ((void));
void shop_update args ((void));
void weather_update args ((void));
void char_update args ((void));
void obj_update args ((void));
void aggr_update args ((void));
void bounty_update args ((void));

int save_count;			/* count of current character to save */
int save_delay;
extern time_t rent_time;
int pulse_aggressive;

/*
 * Advancement stuff.
 */
void
advance_level (CHAR_DATA * ch, bool fSave)
{
  char buf[MAX_STRING_LENGTH];
  int add_hp;
  int add_mana;
  int add_move;
  int add_prac;

  ch->pcdata->old_time = 0;
  add_hp = con_app[get_curr_con (ch)].hitp + number_range (
					      class_table[ch->class].hp_min,
					    class_table[ch->class].hp_max) +
    race_table[ch->race].hp_mod;
  add_mana = int_app[get_curr_int (ch)].manap + number_range (
					    class_table[ch->class].mana_min,
					  class_table[ch->class].mana_max) +
    race_table[ch->race].mana_mod;
  add_move = number_range (6, 6 + ((get_curr_con (ch) + get_curr_str (ch)) / 7)) +
    race_table[ch->race].move_mod;
  add_prac = wis_app[get_curr_wis (ch)].practice;

  add_hp = UMAX (1, add_hp);
  add_mana = UMAX (1, add_mana);
  add_move = UMAX (3, add_move);

  add_hp += ch->pcdata->reincarnation * 8;
  add_mana += ch->pcdata->reincarnation * 6;
  add_move += ch->pcdata->reincarnation * 5;
  add_prac += ch->pcdata->reincarnation * 3;

  if( which_god(ch)==GOD_INIT_CHAOS || which_god(ch)==GOD_INIT_ORDER )
    {
    add_hp   -= initiate_hp_loss(ch->class,ch->pcdata->reincarnation);
    add_mana -= initiate_mana_loss(ch->class,ch->pcdata->reincarnation);
    add_move -= initiate_move_loss(ch->class,ch->pcdata->reincarnation);
    }

  /* Knight stuff get double   - Chaos     9/14/95 */
  if (ch->level > 90 && ch->level < 96)
    {
      add_hp *= 2;
      add_mana *= 2;
      add_move *= 2;
      add_prac *= 2;
    }
  /* Followers get extra juice !! - Martin 22/7/98 */
  /* Chaos get random extra gain */
  if (ch->which_god == GOD_CHAOS)
    {
      int temp = number_range (1, 5);
      switch (temp)
	{
	case 1:
	  ch_printf (ch, "The fickle power of Chaos gives you a tiny portion of his strength.\n\r");
	  break;
	case 2:
	  ch_printf (ch, "The lord of Entropy gives you a small token of power.\n\r");
	  break;
	case 3:
	  ch_printf (ch, "Your muscles pulse with the erratic strength of Chaos.\n\r");
	  break;
	case 4:
	  ch_printf (ch, "Your body hums to the discordant sound of Pandemonium.\n\r");
	  break;
	case 5:
	  ch_printf (ch, "The power of Entropy flows into your veins.\n\r");
	  break;
	}
      add_hp += temp;
      add_mana += temp;
      add_move += temp;
    }
  /* Order followers get fixed 3 */
  else if (ch->which_god == GOD_ORDER)
    {
      ch_printf (ch, "The favour of Order steels your sinews.\n\r");
      add_hp += 3;
      add_mana += 3;
      add_move += 3;
    }

  ch->actual_max_hit += add_hp;
  ch->actual_max_mana += add_mana;
  ch->actual_max_move += add_move;
  ch->max_hit += add_hp;
  ch->max_mana += add_mana;
  ch->max_move += add_move;
  ch->practice += add_prac;

  if (!IS_NPC (ch))
    {
      REMOVE_BIT (ch->act, PLR_THIEF);
      REMOVE_BIT (ch->act, PLR_BOUGHT_PET);
      if (fSave)
      {
        sub_player (ch);
        add_player (ch);
        save_char_obj (ch, NORMAL_SAVE);
        save_char_obj (ch, BACKUP_SAVE);
      }
    }

  sprintf (buf,
	   "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
	   add_hp, ch->max_hit,
	   add_mana, ch->max_mana,
	   add_move, ch->max_move,
	   add_prac, ch->practice
    );
  send_to_char (buf, ch);
  return;
}



void
gain_exp (CHAR_DATA * ch, int gain)
{

  if (IS_NPC (ch) || ch->level >= (LEVEL_HERO - 1) ||
      ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
    return;

  ch->exp = UMAX (0, ch->exp + gain);
  if (gain < 0)
    ch->exp_lost -= gain;
  while (ch->level < (LEVEL_HERO - 1) &&
	 ch->exp >= exp_level (ch->class, ch->level))
    {
      send_to_char ("You raise a level!!  ", ch);
      /* prevent multiple leveling */
      ch->exp = exp_level (ch->class, ch->level) + 1;
      ch->level += 1;
      if (ch->level % 5 == 0) /*add a language every 5 levels  Chaos 4/27/99*/
	add_language (ch);
      ch->mclass[ch->class] += 1;
      ch->mclass_switched = 0;
      advance_level (ch, TRUE);
    }

  return;
}



/*
 * Regeneration stuff.
 */
int hit_gain (CHAR_DATA * ch)
{
  int gain;

  open_timer (TIMER_HIT_GAIN);
  if (IS_NPC (ch))
    {
      gain = ch->level * 4 / 3;
    }
  else
    {
      gain = ch->level * 2 / 3 + 6;

      switch (ch->position)
	{
	case POS_SLEEPING:
	  gain += get_curr_con (ch);
	  break;
	case POS_RESTING:
	  gain += get_curr_con (ch) / 2;
	  break;
	}

      if (ch->pcdata->condition[COND_FULL] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;

    }

  if (IS_AFFECTED (ch, AFF_POISON))
    gain /= 4;

  if (in_camp (ch))
    gain = 5 * gain / 4;

  if (!IS_NPC (ch) && ch->race == RACE_HUMAN &&
      ch->in_room != NULL &&
      (ch->in_room->sector_type == SECT_INSIDE ||
       ch->in_room->sector_type == SECT_INN ||
       ch->in_room->sector_type == SECT_CITY))
    {
      send_to_char ("You heal faster here.\n\r", ch);
      gain = 3 * gain / 2;
    }
  if (IS_AFFECTED (ch, AFF2_ENHANCED_HEAL))
    {
      send_to_char ("You heal easier.\n\r", ch);
      gain = 3 * gain / 2;
    }

  if (IS_AFFECTED (ch, AFF2_BERSERK))
    gain = 1 * gain / 2;

  close_timer (TIMER_HIT_GAIN);

  return UMIN (gain, ch->max_hit - ch->hit);
}



int
mana_gain (CHAR_DATA * ch)
{
  int gain;

  if (IS_NPC (ch))
    {
      gain = ch->level * 2;
    }
  else
    {
      gain = ch->level + 8;

      switch (ch->position)
	{
	case POS_SLEEPING:
	  gain += get_curr_int (ch) * 2;
	  break;
	case POS_RESTING:
	  gain += get_curr_int (ch);
	  break;
	}

      if (ch->pcdata->condition[COND_FULL] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;

    }

  if (IS_AFFECTED (ch, AFF2_BERSERK))
    return 0;

  if (IS_AFFECTED (ch, AFF_POISON))
    gain /= 4;

  if (in_camp (ch))
    gain = 5 * gain / 4;

  if (!IS_NPC (ch) && ch->race == RACE_ELF &&
      ch->in_room != NULL &&
      ch->in_room->sector_type != SECT_INSIDE &&
      ch->in_room->sector_type != SECT_ASTRAL &&
      ch->in_room->sector_type != SECT_CITY &&
      ch->in_room->sector_type != SECT_ETHEREAL)
    {
      send_to_char ("You revive quicker here.\n\r", ch);
      gain = 4 * gain / 3;
    }
  if (IS_AFFECTED (ch, AFF2_ENHANCED_REVIVE))
    {
      send_to_char ("You revive easier.\n\r", ch);
      gain = 3 * gain / 2;
    }

  return UMIN (gain, ch->max_mana - ch->mana);
}



int
move_gain (CHAR_DATA * ch)
{
  int gain;

  if (IS_NPC (ch))
    {
      gain = ch->level;
    }
  else
    {
      gain = 3 * ch->level / 2 + 16;

      switch (ch->position)
	{
	case POS_SLEEPING:
	  gain += get_curr_dex (ch) * 2;
	  break;
	case POS_RESTING:
	  gain += get_curr_dex (ch);
	  break;
	}

      if (ch->pcdata->condition[COND_FULL] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;
    }

  if (IS_AFFECTED (ch, AFF_POISON))
    gain /= 4;

  if (in_camp (ch))
    gain = 5 * gain / 4;

  if (IS_AFFECTED (ch, AFF2_ENHANCED_REST))
    {
      send_to_char ("You rest easier.\n\r", ch);
      gain = 3 * gain / 2;
    }

  if (IS_AFFECTED (ch, AFF2_BERSERK))
    gain = 2 * gain;

  return UMIN (gain, ch->max_move - ch->move);
}



void gain_condition (CHAR_DATA * ch, int iCond, int value)
{
    int condition;

    if ( value == 0 || IS_NPC (ch) || ch->level >= LEVEL_IMMORTAL )
     return;
    condition				= ch->pcdata->condition[iCond];
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
          if( ch->level > 1 )
            {
	    send_to_char( "You are STARVING!\n\r",  ch );
            act( "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
	    damage(ch, ch, 1+ch->level/7, TYPE_NOFIGHT);
            }
          else
	    send_to_char( "You are really hungry.\n\r",  ch );
	  return;
          break;

	case COND_THIRST:
            if( ch->level > 1 )
              {
	      send_to_char( "You are DYING of THIRST!\n\r", ch );
              act( "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
	      damage(ch, ch, 1+ch->level/5, TYPE_NOFIGHT);
              }
            else
	      send_to_char( "You are really thirsty.\n\r", ch );
	    return;
            break;

	case COND_DRUNK:
	    if ( condition != 0 ) {
		send_to_char( "You are sober.\n\r", ch );
	    return;
	    }
	    break;
	default:
	    bug( "Gain_condition: invalid condition type %d", iCond );
	    break;
	}
    }

    if ( ch->pcdata->condition[iCond] == 1 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	  send_to_char( "You are really hungry.\n\r",  ch );
          act( "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
	  break;

	case COND_THIRST:
	    send_to_char( "You are really thirsty.\n\r", ch );
	    act( "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
	    break;

	case COND_DRUNK:
	    if ( condition != 0 ) {
		send_to_char( "You are feeling a little less light headed.\n\r", ch );
            }
	    break;
	}
    }


    if ( ch->pcdata->condition[iCond] == 2 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are hungry.\n\r",  ch );
	  break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	  break;
	}
    }

    if ( ch->pcdata->condition[iCond] == 3 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	   send_to_char( "You are a mite peckish.\n\r",  ch );
	  break;

	case COND_THIRST:
	   send_to_char( "You could use a sip of something refreshing.\n\r", ch );
	  break;
	}
    }
  return;
}

void
shop_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  for (ch = first_char; ch != NULL; ch = ch_next)
    {
      OBJ_DATA *obj, *obj_next;
      ch_next = ch->next;
      if (IS_NPC (ch) && ch->pIndexData->pShop != NULL)
	{
	  for (obj = ch->first_carrying; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if (!IS_SET (obj->extra_flags, ITEM_INVENTORY))
		if (number_range (0, 100) == 1)
		  {
		    act ("$n discards $p.", ch, obj, NULL, TO_ROOM);
		    obj_from_char (obj);
		    extract_obj (obj);
		  }
	    }
	}
    }
  return;
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void
mobile_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  EXIT_DATA *pexit;
  int door;

  /* Examine all mobs. */
  for (ch = first_char; ch != NULL; ch = ch_next)
    {
      ch_next = ch->next;
      if (!IS_NPC (ch))
	{
	  if (ch->desc == NULL && ch->fighting != NULL)
	    do_flee (ch, "");
	  continue;
	}

      if (ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM))
	continue;

      /* Examine call for special procedure */
      if (ch->spec_fun != 0)
	{
	  if ((*ch->spec_fun) (ch))
	    continue;
	}

      /* update shot_timer and shot_aware */
      if (ch->shot_timer > 0)
	{
	  if (ch->shot_timer == 1)
	    {
	      ch->shot_aware = TRUE;
	      ch->shot_timer = 0;
	    }
	  else
	    ch->shot_timer--;
	}

      /* update distracted timer */
      if (ch->distracted > 0)
	ch->distracted--;

 	if (IS_NPC(ch))
	  mprog_time_trigger (ch);

	if (ch == last_dead || ch->position < POS_SLEEPING || 
	    ch->position==POS_FIGHTING)
	  continue;

 	if (IS_NPC(ch))
	  mprog_random_trigger (ch);

	if (ch == last_dead || ch->position < POS_SLEEPING || 
	    ch->position==POS_FIGHTING)
	  continue;

      /* That's all for sleeping / busy monster */
      if (ch->position != POS_RESTING && ch->position != POS_STANDING)
	continue;

      /* random mobile triggering  */


      /* Scavenge */
      if (IS_SET (ch->act, ACT_SCAVENGER)
      /* &&  ch->in_room->area->nplayer > 0  */
	  && ch->in_room->first_content != NULL
	  && number_bits (2) == 0)
	{
	  OBJ_DATA *obj;
	  OBJ_DATA *obj_best;
	  int max;

	  max = 1;
	  obj_best = 0;
	  for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
	    {
	      if (CAN_WEAR (obj, ITEM_TAKE) && obj->cost > max)
		{
		  obj_best = obj;
		  max = obj->cost;
		}
	    }

	  if (obj_best)
	    {
	      obj_from_room (obj_best);
	      obj_to_char (obj_best, ch);
	      act ("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
	      do_wear (ch, obj_best->name);
	    }
	}

      /* if ch changes position due to it's or somethings mobprog */
    if(ch == last_dead || ch->position<POS_STANDING)
      continue;
      /* Wander */
      if ( !IS_SET (ch->act, ACT_SENTINEL))
	  if (ch->fighting == NULL)
	  if (number_bits (4) < 8)
	  if ((abs(door = number_bits (5))) <= 5)
 	  {
	   if ((pexit = ch->in_room->exit[abs(door)]) != NULL)
	   {
	    if (pexit->to_room != NULL)
	    if (!IS_SET (pexit->exit_info, EX_CLOSED))
	    if (!IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB))
	    if (pexit->to_room->area == ch->in_room->area)
	    {
	     move_char (ch, door);
	     if (ch == last_dead || ch->position < POS_STANDING)
	      continue;
	    }
	   }
	  }

      /* Flee */
      if (			/*IS_SET(ch->act, ACT_WIMPY) 
				   && */ (ch->hit < ch->max_hit / 2)
	   && (door = number_bits (3)) <= 5
	   && (pexit = ch->in_room->exit[door]) != NULL
	   && pexit->to_room != NULL
	   && (!IS_SET (pexit->exit_info, EX_CLOSED))
	   && (!IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB))
	   && (pexit->to_room->area == ch->in_room->area))
	{
	  CHAR_DATA *rch;
	  bool found;

	  found = FALSE;
	  for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room)
	    {
	      char buf[MAX_INPUT_LENGTH];
	      if (is_fearing (ch, rch))
		{
		  switch (number_bits (2))
		    {
		    case 0:
		      sprintf (buf, "Get away from me, %s!", rch->name);
		      break;
		    case 1:
		      sprintf (buf, "Leave me be, %s!", rch->name);
		      break;
		    case 2:
		      sprintf (buf, "%s is trying to kill me!  Help!", rch->name);
		      break;
		    case 3:default:
		      sprintf (buf, "Someone save me from %s!", rch->name);
		      break;
		    }
		  do_shout (ch, buf);

		  found = TRUE;
		  break;
		}
	    }
	  if (found)
	    move_char (ch, door);
	}

    }

  return;
}



/*
 * Update the weather.
 */
void weather_update (void)
{
  char buf[MAX_STRING_LENGTH];
  int diff;
  PLAYER_GAME *gch;
  int rvnum, temp;
  AREA_DATA *parea;



  buf[0] = '\0';


  time_info.hour++;

  if (time_info.hour >= 24)
    {
      time_info.hour = 0;
      time_info.day++;
    }

  if (time_info.day >= 35)
    {
      time_info.day = 0;
      time_info.month++;
      mix_race_war ();
    }

  if (time_info.month >= 17)
    {
      time_info.month = 0;
      time_info.year++;
    }


  /* Each area has different weather, so must segregate this routine */
  /* Chaos - 8/20/95 */
  /*
   * Weather change.
   */
  parea = NULL;
  for (rvnum = 1; rvnum < MAX_VNUM; rvnum += 100)
    if (get_room_index (rvnum) != NULL && room_index[rvnum]->area != parea)
      {
	parea = room_index[rvnum]->area;

	*buf = '\0';
	switch (time_info.hour)
	  {
	  case 5:
	    parea->weather_info.sunlight = SUN_LIGHT;
	    strcat (buf, "The day has begun.\n\r");
	    break;

	  case 6:
	    parea->weather_info.sunlight = SUN_RISE;
	    strcat (buf, "The sun rises in the east.\n\r");
	    break;

	  case 19:
	    parea->weather_info.sunlight = SUN_SET;
	    strcat (buf, "The sun slowly disappears in the west.\n\r");
	    break;

	  case 20:
	    parea->weather_info.sunlight = SUN_DARK;
	    strcat (buf, "The night has begun.\n\r");
	    break;
	  }

	if (time_info.month >= 9 && time_info.month <= 16)
	  diff = parea->weather_info.mmhg > 985 ? -2 : 2;
	else
	  diff = parea->weather_info.mmhg > 1015 ? -2 : 2;

	temp = ((parea->weather_info.temp_summer - parea->weather_info.temp_winter)
		* (18 - abs (time_info.month - 17)) / 18) +
	  parea->weather_info.temp_winter +
	  (parea->weather_info.temp_daily *
	 (12 - abs (time_info.hour - 12)) / 12) + dice (1, 3) - dice (1, 3);

	parea->weather_info.change = UMAX (parea->weather_info.change, -8);
	parea->weather_info.change = UMIN (parea->weather_info.change, 8);

	parea->weather_info.change += diff * dice (1, 4) + dice (1, 5) - dice (1, 5);

	parea->weather_info.mmhg += parea->weather_info.change;

	if (parea->weather_info.wet_scale < 5)
	  parea->weather_info.mmhg = UMAX (parea->weather_info.mmhg,
			    960 + (5 - parea->weather_info.wet_scale) * 15);
	else
	  parea->weather_info.mmhg = UMAX (parea->weather_info.mmhg, 960);

	parea->weather_info.mmhg = UMIN (parea->weather_info.mmhg, 1040);
	if (parea->weather_info.wet_scale > 5)
	  parea->weather_info.mmhg = UMIN (parea->weather_info.mmhg,
			    960 - (parea->weather_info.wet_scale - 5) * 15);
	else
	  parea->weather_info.mmhg = UMIN (parea->weather_info.mmhg, 1040);

	parea->weather_info.change = 0;
	parea->weather_info.temperature = temp;

	switch (parea->weather_info.sky)
	  {
	  default:
	    bug ("Weather_update: bad sky %d.", parea->weather_info.sky);
	    parea->weather_info.sky = SKY_CLOUDLESS;
	    break;

	  case SKY_CLOUDLESS:
	    if (parea->weather_info.mmhg < 1010)
	      {
		strcat (buf, "The sky is getting cloudy.\n\r");
		parea->weather_info.sky = SKY_CLOUDY;
		parea->weather_info.mmhg = 1005;
	      }
	    break;

	  case SKY_CLOUDY:
	    if (parea->weather_info.mmhg < 990)
	      {
		if (parea->weather_info.temperature < 32)
		  strcat (buf, "It starts to snow.\n\r");
		else if (parea->weather_info.temperature < 34)
		  strcat (buf, "It starts to sleet.\n\r");
		else
		  strcat (buf, "It starts to rain.\n\r");
		parea->weather_info.sky = SKY_RAINING;
		parea->weather_info.mmhg = 985;
	      }

	    if (parea->weather_info.mmhg > 1010)
	      {
		strcat (buf, "The clouds disappear.\n\r");
		parea->weather_info.sky = SKY_CLOUDLESS;
		parea->weather_info.mmhg = 1015;
	      }
	    break;

	  case SKY_RAINING:
	    if (parea->weather_info.mmhg < 970)
	      {
		if (parea->weather_info.temperature < 32)
		  strcat (buf, "A blizzard takes hold in the realm.\n\r");
		else if (parea->weather_info.temperature < 34)
		  strcat (buf, "A hail storm developes.\n\r");
		else
		  strcat (buf, "Lightning flashes in the sky.\n\r");
		parea->weather_info.sky = SKY_LIGHTNING;
		parea->weather_info.mmhg = 965;
	      }

	    if (parea->weather_info.mmhg > 990)
	      {
		if (parea->weather_info.temperature < 32)
		  strcat (buf, "The snow stops.\n\r");
		else if (parea->weather_info.temperature < 34)
		  strcat (buf, "The sleet stops.\n\r");
		else
		  strcat (buf, "The rain stops.\n\r");
		parea->weather_info.sky = SKY_CLOUDY;
		parea->weather_info.mmhg = 995;
	      }
	    break;

	  case SKY_LIGHTNING:
	    if (parea->weather_info.mmhg > 970)
	      {
		if (parea->weather_info.temperature < 32)
		  strcat (buf, "The blizzard stops.\n\r");
		else if (parea->weather_info.temperature < 34)
		  strcat (buf, "The hail storm stops.\n\r");
		else
		  strcat (buf, "The lightning has stopped.\n\r");
		parea->weather_info.sky = SKY_RAINING;
		parea->weather_info.mmhg = 975;
	      }
	    break;
	  }

	/* Added NO_WEATHER_SECT for a bit of realism - Martin 22/8/98 */
	for (gch = first_player; gch != NULL; gch = gch->next)
	  {
	    if (buf[0] != '\0' &&
		gch->ch->in_room->area == parea &&
		IS_OUTSIDE (gch->ch) &&
		!NO_WEATHER_SECT (gch->ch->in_room->sector_type) &&
		IS_AWAKE (gch->ch))
	      send_to_char (buf, gch->ch);
	  }
      }

  return;
}

void
strip_greater (char *str)
{
  char *pt;

  for (pt = str; *pt != '\0'; pt++)
    {
      if (*pt == '<')
	*pt = '(';
      else if (*pt == '>')
	*pt = ')';
    }

  return;
}


  /* Make a html web page - Chaos  3/28/96 */
void
save_html_who (void)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH], buf_race[20];
  char buf2[MAX_STRING_LENGTH];
  int leng;
  CHAR_DATA *fch;
  DESCRIPTOR_DATA *d;
  int nMatch;
  int nTotal;
  CHAR_DATA *wch;
  char const *class;
  char god;
  char killer_thief;
  PLAYER_GAME *fpl;
  char *pt;

  if (!REAL_GAME)
    return;

  fclose (fpReserve);

  fp = fopen ("../../public_html/who.html", "w");
  if (fp == NULL)
    {
      fpReserve = fopen (NULL_FILE, "r");
      return;
    }

  fprintf (fp, "<!DOCTYPE html PUBLIC \"-//IETF//DTD// HTML 2.0//EN\">\n");
  fprintf (fp, "<BODY BACKGROUND=\"bumps1.jpg\" text=#ffff30 alink=#80FF30 vlink=#90FF30 link=#FFFF30 >\n");

  fprintf (fp, "<HTML><HEAD><TITLE>Mortal Realms Who List</TITLE></HEAD>\n");
  fprintf (fp, "<BODY><FONT SIZE=+2><CENTER>\n");
  fprintf (fp, "Mortal Realms WHO Page<p>\n");


  /*
   * Set default arguments.
   */
  fch = NULL;


  /*
   * Now show matching chars.
   */

  nMatch = 0;
  nTotal = 0;
  buf[0] = '\0';
  leng = 0;
  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      wch = fpl->ch;
      d = NULL;
      if (is_desc_valid (wch))
	d = wch->desc;
      /*
       * Check for match against restrictions.
       * Don't use trust as that exposes trusted mortals.
       Chaos set to see all chars, invis or not.
       */
      if (IS_SET (wch->act, PLR_WIZINVIS))
	continue;
      nTotal++;
      nMatch++;


      /*
       * Figure out what to print for class.
       */
      class = class_table[wch->class].who_name;
      switch (wch->level)
	{
	default:
	  break;
	case MAX_LEVEL - 0:
	  class = "GOD";
	  break;
	case MAX_LEVEL - 1:
	  class = "DUK";
	  break;
        case MAX_LEVEL - 2: if (which_god(wch) == GOD_POLICE)
                             class = "COP";
                            else
                             class = "COU"; break;

	  break;
	}

      strcpy (buf_race, race_table[wch->race].race_name);
      buf_race[3] = '\0';


      if (wch->level > MAX_LEVEL - 4)
	strcpy (buf_race, "---");


      switch (which_god (wch))
	{
	case GOD_INIT_ORDER:
	  god = 'o';
	  break;
	case GOD_INIT_CHAOS:
	  god = 'c';
	  break;
	case GOD_ORDER:
	  god = 'O';
	  break;
	case GOD_CHAOS:
	  god = 'C';
	  break;
	case GOD_DEMISE:
	  god = 'D';
	  break;
	case GOD_POLICE:
	  god = 'P';
	  break;
	default:
	  god = '-';
	  break;
	}

      if (IS_SET (wch->act, PLR_KILLER))
	killer_thief = 'K';
      else if (IS_SET (wch->act, PLR_THIEF))
	killer_thief = 'T';
      else
	killer_thief = ' ';


      /*
       * Format it up.
       */
      sprintf (buf2, "[%2d %s %s]%c%c%s%s",
	       wch->level,
	       class,
	       buf_race,
	       god,
	       killer_thief,
	       wch->name,
	       IS_NPC (wch) ? "the monster" : wch->pcdata->title);
      buf2[71] = '\0';
      strip_greater (buf2);
      while (strlen (buf2) < 71)
	str_cat_max (buf2, " ", MAX_STRING_LENGTH);
      leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);

      if ((wch->pcdata->switched || wch->desc != NULL) &&
	  !IS_AFFECTED (wch, AFF_STEALTH))
	{
	  strcpy (buf2, wch->in_room->area->name);
	  buf2[8] = '\0';
	  while (strlen (buf2) < 8)
	    str_cat_max (buf2, " ", MAX_STRING_LENGTH);
	}
      else if (wch->desc == NULL)
	strcpy (buf2, "LinkLost");
      else
	strcpy (buf2, "Unknown ");	/* Stealth Mode */

      leng = str_apd_max (buf, " {", leng, MAX_STRING_LENGTH);
      leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);
      leng = str_apd_max (buf, "} ", leng, MAX_STRING_LENGTH);

/*  They don't need to see this either -  Chaos  4/30/99   
      if (wch->desc != NULL && wch->desc->host != NULL)
	{
	  leng = str_apd_max (buf, " (", leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, wch->desc->host, leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, ")", leng, MAX_STRING_LENGTH);
	}  */

/*  Let's be a bit discreet here.   -  Chaos   4/25/99
      if (wch->pcdata->mail_address != NULL &&
	  *wch->pcdata->mail_address != '\0')
	{
	  leng = str_apd_max (buf, " Email: <a href=\"mailto:", leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, wch->pcdata->mail_address, leng,
			      MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, "\">", leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, wch->pcdata->mail_address, leng,
			      MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, "</a>", leng, MAX_STRING_LENGTH);
	}   */

      if (wch->pcdata->html_address != NULL &&
	  *wch->pcdata->html_address != '\0')
	{
	  strcpy (buf2, wch->pcdata->html_address);
	  for (pt = buf2; *pt != '\0'; pt++)
	    if (*pt == '*')
	      *pt = '~';
	  leng = str_apd_max (buf, " Home Page: <a href=\"http://", leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, buf2, leng,
			      MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, "\">", leng, MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, buf2, leng,
			      MAX_STRING_LENGTH);
	  leng = str_apd_max (buf, "</a>", leng, MAX_STRING_LENGTH);
	}

      leng = str_apd_max (buf, "<p>\n", leng, MAX_STRING_LENGTH);
    }

  sprintf (buf2, "Players: %d</CENTER></font><p><font size=-1>\n", nTotal);


  fprintf (fp, "%s", buf2);
  fprintf (fp, "%s", buf);

  fprintf (fp, "</font></BODY></HTML>\n");


  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  return;
}


#define CASTLE_TICK 10
/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void
char_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *ch_quit;
  static int castleTick = CASTLE_TICK;
  ROOM_INDEX_DATA *room;
  int iHash;
  int max_save = 0;

  /* Scan for Sanctified rooms */
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    for (room = room_index_hash[iHash]; room != NULL; room = room->next)
      {
	if (room->sanctify_timer > 0)
	  {
	    room->sanctify_timer--;
	    if (room->sanctify_timer == 0)
	      {
		REMOVE_BIT (room->room_flags, ROOM_SAFE);
		room->sanctify_char = NULL;
		for (ch = room->first_person; ch != NULL; ch = ch->next_in_room)
		  send_to_char ("The area does not look safe now.\n\r", ch);
	      }
	  }
	if (room->smoke_timer > 0)
	  {
	    room->smoke_timer--;
	    if (room->smoke_timer == 0)
	      {
		REMOVE_BIT (room->room_flags, ROOM_SMOKE);
		for (ch = room->first_person; ch != NULL; ch = ch->next_in_room)
		  send_to_char ("The smoke dissipates.\n\r", ch);
	      }
	  }
	if (room->hallucinate_timer > 0)
	  {
	    room->hallucinate_timer--;
	    if (room->hallucinate_timer == 0)
	      {
		REMOVE_BIT (room->room_flags, ROOM_HALLUCINATE);
		room->hallucinate_room = NULL;
		for (ch = room->first_person; ch != NULL; ch = ch->next_in_room)
		  send_to_char ("The room shimmers for a moment.\n\r", ch);
	      }
	  }
      }

  usage.numPlayers = 0;
  ch_quit = NULL;
  for (ch = first_char; ch != NULL; ch = ch_next)
    {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;

      ch_next = ch->next;

      if (!IS_NPC (ch))
	{
	  int lt, sk;

	  if (ch->level > 98)
	    ch->pcdata->condition[COND_DRUNK] = 0;

	  if (ch->pcdata->death_timer > 0)
	    ch->pcdata->death_timer--;

	  check_most (ch);	/* Nifty MOST command stuff - Chaos  2/27/96  */

	  usage.numPlayers++;
	  if (ch->desc != NULL)
	    ch->played += 60;	/* New clock location for seconds played */

	  /* Make sure that they have a valid set of levels -   Chaos 2/3/95 */
	  if (ch->level < MAX_LEVEL)
	    {
	      for (lt = 0, sk = 0; sk < MAX_CLASS; sk++)
		lt += ch->mclass[sk];
	      if (lt != ch->level)
		{
		  do_delete (ch, NULL);
		  continue;
		}
	    }
	  if (ch->level > 90 && ch->max_mana <= 100 && ch->max_move <= 100 &&
	      ch->max_hit < 100)
	    {
	      act ("$n dies of old age.", ch, NULL, NULL, TO_ROOM);
	      act ("You die of old age.", ch, NULL, NULL, TO_CHAR);
	      do_delete (ch, NULL);
	      continue;
	    }
	}
      else if (ch->fighting == NULL && ch->hit == ch->max_hit)
	{
	  ch->npcdata->pvnum_last_hit = 0;
	  ch->npcdata->pvnum_last_hit_leader = 0;
	}

      /*
       * Log them silly dood's that no-longer have valid rooms
       */
      if (!IS_NPC (ch) && ch->in_room == NULL)
	{
	  if (ch->pcdata != NULL)
	    bug ("Char_update:Null Room in char #%d", ch->pcdata->pvnum);
	  else
	    bug ("Char_update:Null Room in char w/o pcdata", (int) current_time);
	}

      if (!IS_NPC (ch))
	{
	  /* Fix those first_person with too much gold.  */
	  if (ch->gold > 1000000 * ch->level || ch->gold < 0)
	    ch->gold = 1000000 * ch->level;

	  /* Remove first_person from level restricted areas. */
	  if (ch->in_room->area->low_hard_range != 0 ||
	      ch->in_room->area->hi_hard_range != 0)
	    if (!IS_IMMORTAL (ch))
	      if (ch->level < ch->in_room->area->low_hard_range ||
		  ch->level > ch->in_room->area->hi_hard_range)
		{
		  char_from_room (ch);
		  char_to_room (ch, room_index[ROOM_VNUM_TEMPLE]);
		}

	  /* Adjust save vs spells to maximum allowed. */
	  max_save -= (ch->level / 5 + 1);
	  switch (ch->class)
	    {
	    case 0:
	    case 1:
	    case 4:
	    case 5:
	      max_save -= (get_curr_int (ch) * 3 / 2);
	      break;
	    default:
	      max_save -= (get_curr_int (ch) - ch->class);
	      break;
	    }
	  if (ch->saving_throw < max_save)
	    ch->saving_throw = max_save;
	}

      if (ch->position >= POS_STUNNED)
	{
	  if (ch->hit < ch->max_hit)
	    if (!IS_NPC (ch) ||
		!IS_AFFECTED (ch, AFF_CHARM) ||
		!IS_SET (ch->act, ACT_UNDEAD))
	      ch->hit += hit_gain (ch);

	  if (ch->mana < ch->max_mana)
	    ch->mana += mana_gain (ch);

	  if (ch->move < ch->max_move)
	    ch->move += move_gain (ch);
	}

      total_io_exec = 1;	/* Recalculate the CPU usage every minute */
      total_io_delay = 1;
      total_io_ticks = 1;
      total_io_bytes = 1;

      /* if ( ch->position == POS_STUNNED ) */
      update_pos (ch);

      /* Take care of those that SHOULD be dead, but are not.  */
      if (ch->position == POS_DEAD)
	{
	  raw_kill (ch);
	  continue;
	}

      if (!IS_NPC (ch))
	if (ch->pcdata->arrested == TRUE)
	  {
	    if ((ch->pcdata->jailtime - (current_time - ch->pcdata->jaildate))
		<= 0)
	      auto_release (ch);
	  }

      if (!IS_NPC (ch))
	if (ch->pcdata->just_died_ctr != 0)
	  if ((ch->pcdata->just_died_ctr - (current_time -
					    ch->pcdata->time_of_death)) <= 0)
	    {
	      ch->pcdata->just_died_ctr = 0;
	      send_to_char ("The gods are no longer protecting you.\n\r", ch);
	    }

      if (!IS_NPC (ch))
	if (ch->desc != NULL)
	  ch->desc->lookup = FALSE;	/* Set lookup false for those playing */


      if (!IS_NPC (ch))
	{
	  OBJ_DATA *obj;

	  if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
	      && obj->item_type == ITEM_LIGHT
	      && obj->value[2] > 0)
	    {
	      if (--obj->value[2] == 0 && ch->in_room != NULL)
		{
		  --ch->in_room->light;
		  act ("$p goes out.", ch, obj, NULL, TO_ROOM);
		  act ("$p goes out.", ch, obj, NULL, TO_CHAR);
		  extract_obj (obj);
		}
	    }

	  /* if( ch->desc != NULL  && ch->desc->character != ch )
	     ch->timer = 0; */

	  if (++ch->timer >= 5)
	    {
	      if (ch->was_in_room == NULL && ch->in_room != NULL)
		if (!IS_SET (ch->in_room->room_flags, ROOM_RIP) &&
		    !IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL) &&
		    !ch->pcdata->switched &&
		    !IS_SET(ch->in_room->area->flags, AFLAG_NOTELEPORT) &&
		    !IS_IMMORTAL(ch) &&
		    !IS_SET(ch->in_room->area->flags, AFLAG_NORECALL) )
		  {
		    CHAR_DATA *rch;
		    ch->was_in_room = ch->in_room;
		    if (ch->fighting != NULL)
		      stop_fighting (ch, TRUE);
		    /* Another big stealth hole...shouldn't echo 
		       this message if ppl in the room cant see it
		       Martin 6/8/98 */

		    for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room)
		      if (can_see (rch, ch) && rch != ch && rch->position > POS_SLEEPING)
			ch_printf (rch, "%s disappears into the void.\n\r",
				   get_name (ch));

		    send_to_char ("You disappear into the void.\n\r", ch);
		    save_char_obj (ch, NORMAL_SAVE);
		    char_from_room (ch);
		    char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
		  }
	    }

	  if (ch->timer > 15 && (!IS_IMMORTAL(ch) || !is_desc_valid(ch) ) )
	    {
	      if (ch->was_in_room != NULL)
		{
		  char_from_room (ch);
		  char_to_room (ch, ch->was_in_room);
		  ch->was_in_room = NULL;
		}
	      ch_quit = ch;
	    }

	  gain_condition (ch, COND_DRUNK, -1);
	  if (ch->level < 95)	/* allow some to not need food */
	    {
	      gain_condition (ch, COND_FULL, -1);
	      gain_condition (ch, COND_THIRST, -1);
	    }
	}

      for (paf = ch->first_affect; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  if (paf->duration > 0)
	    paf->duration--;
	  else if (paf->duration < 0)
	    continue;
	  else
	    {
	      if (paf_next == NULL
		  || paf_next->type != paf->type
		  || paf_next->duration > 0)
		{
		  if (paf->type > 0 && skill_table[paf->type].msg_off)
		    {
		      send_to_char (skill_table[paf->type].msg_off, ch);
		      send_to_char ("\n\r", ch);
		    }
		}

	      if (paf->bitvector == AFF2_ASTRAL && !IS_NPC (ch) && ch->pcdata != NULL &&
	      ch->in_room != NULL && ch->in_room->sector_type == SECT_ASTRAL && ch->race != RACE_GITH)
		{
		  int rroom;
		  if (ch->position == POS_FIGHTING)
		    leave_fighting (ch, ch->in_room);
		  rroom = ch->in_room->vnum;
		  char_from_room (ch);
		  if (get_room_index (ch->pcdata->last_real_room) == NULL ||
		      rroom < 5 || ch->pcdata->last_real_room < 5 ||
		      room_index[ch->pcdata->last_real_room]->sector_type == SECT_ASTRAL)
		    char_to_room (ch, room_index[ROOM_VNUM_TEMPLE]);
		  else
		    char_to_room (ch, get_room_index (ch->pcdata->last_real_room));
		  send_to_char ("Your mind snaps back into place.\n\r", ch);
		  do_look (ch, "");
		}

	      if (paf->type == gsn_charm_person)
		{
		  stop_follower (ch);
		  if (IS_NPC (ch))
		    {
		      SET_BIT (ch->act, ACT_WILL_DIE);
		      continue;
		    }
		  send_to_char ("Your will is now your own.\n\r", ch);
		}
	      if (paf->bitvector == AFF2_BLEEDING)
		{
		  ch->critical_hit_by = 0;
		  send_to_char ("You stop bleeding.\n\r", ch);
		}
	      affect_strip (ch, paf->type);
	    }
	}


      if (!IS_NPC (ch) && IS_SET (ch->act, PLR_KILLER) &&
	  (ch->played - ch->killer_played) > 60 * 60 * 24 /* 24 hours */ )
	REMOVE_BIT (ch->act, PLR_KILLER);

      if (!IS_NPC (ch) && IS_SET (ch->act, PLR_OUTCAST) &&
	  (ch->played - ch->outcast_played) > 60 * 60 * 24 /* 24 hours */ )
	REMOVE_BIT (ch->act, PLR_OUTCAST);

      /*  Update their displays to the current clock */
      if (!IS_NPC (ch) && ch->vt100 == 1 && IS_SET (ch->act, PLR_PROMPT))
	vt100prompt (ch);

      /*
       * Careful with the damages here,
       *   MUST NOT refer to ch after damage taken,
       *   as it may be lethal damage (on NPC).
       */
      if (IS_AFFECTED (ch, AFF_POISON))
	{
	  act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
	  send_to_char ("You shiver and suffer.\n\r", ch);
	  damage (ch, ch, 2 + ch->level / 3, TYPE_NOFIGHT);
	}
      else if (ch->position == POS_INCAP)
	{
	  damage (ch, ch, 1, TYPE_UNDEFINED);
	}
      else if (ch->position == POS_MORTAL)
	{
	  damage (ch, ch, 2, TYPE_UNDEFINED);
	}
    }

  /*
   * Autosave and autoquit.
   * Check that these chars still exist.
   */
  if (ch_quit != NULL && is_char_valid (ch_quit))
    {
      do_quit (ch_quit, "arglebargle");
    }

  /*
   * update the usage graph
   */
  if (TRUE)
    {
      static int lastHour;


    /*  Let's just grab the current status  -  Chaos 4/28/99 
       if ((lastHour != tme.tm_hour) ||
	  (usage.players[tme.tm_hour][tme.tm_wday] < usage.numPlayers)) */

      usage.players[tme.tm_hour][tme.tm_wday] = usage.numPlayers;
      if (lastHour != tme.tm_hour)
	{
	  save_sites ();	/* also saves usage info */
	  usage.numRecons = 0;
	}
      usage.recons[tme.tm_hour][tme.tm_wday] = usage.numRecons;
      lastHour = tme.tm_hour;
    }

  close_timer (TIMER_CHAR_UPD);

  /*
   * save the castles if they need it
   */
  if (castleTick-- <= 0)
    {
      castleTick = CASTLE_TICK;
      save_owners ();

      if (castle_needs_saving)
	{
	  open_timer (TIMER_CASTLE_SAVE);
	  do_savearea (NULL, "forreal");
	  close_timer (TIMER_CASTLE_SAVE);
	}
    }

  /* Minutely update on the WHO HTML Page    -   Chaos 3/26/96  */
  if (REAL_GAME)
    save_html_who ();

  return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void
obj_update (void)
{
  OBJ_DATA *obj, *objc;
  OBJ_DATA *obj_next, *objc_next;
  CHAR_DATA *owner;
  bool is_owned;

  for (obj = first_object; obj != NULL; obj = obj_next)
    {
      CHAR_DATA *rch;
      char *message;

      obj_next = obj->next;

      /* Look for enhanced objects */
      if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON)
	if (obj->pIndexData->vnum != 51 &&
	    obj->pIndexData->vnum != 52 &&
	    obj->pIndexData->vnum != 53)
	  if (obj->value[0] > obj->pIndexData->value[0] ||
	      obj->value[1] > obj->pIndexData->value[1] ||
	      obj->value[2] > obj->pIndexData->value[2] ||
	      obj->value[3] > obj->pIndexData->value[3])
	    {
	      obj->value[0] = obj->pIndexData->value[0];
	      obj->value[1] = obj->pIndexData->value[1];
	      obj->value[2] = obj->pIndexData->value[2];
	      obj->value[3] = obj->pIndexData->value[3];
	    }

      /* close closable items */
      if (obj->carried_by == NULL && obj->item_type == ITEM_CONTAINER &&
	  IS_SET (obj->value[1], CONT_CLOSEABLE))
	{
	  if (!IS_SET (obj->value[1], CONT_CLOSED))
	    SET_BIT (obj->value[1], CONT_CLOSED);
	  if (IS_SET (obj->value[1], CONT_CLOSED))
	    if (obj->value[2] != -1)
	      SET_BIT (obj->value[1], CONT_LOCKED);
	}

      if (obj->pIndexData == NULL)
	{
          char buf[200];
          sprintf( buf, "Object with NULL index: %s on %s", obj->name,
                    obj->carried_by != NULL ? obj->carried_by->name :
                    "none" );
          log_string( buf );

	  extract_obj (obj);
	  continue;
	}

      if (obj->pIndexData->obj_fun != 0)
	{
	  if ((*obj->pIndexData->obj_fun) (obj, OBJ_UPDATE, NULL, NULL, NULL))
	    continue;
	}

      if (!IS_SET (obj->extra_flags, ITEM_NOT_VALID) &&
	  !IS_SET (obj->pIndexData->extra_flags, ITEM_NOT_VALID))
	{
	  if ((obj->timer <= 0 && obj->sac_timer <= 0) ||
	      (obj->timer > 0 && --obj->timer > 0) ||
	      (obj->in_room != NULL &&
	       IS_SET (obj->in_room->room_flags, ROOM_CLAN_DONATION)) ||
	      (obj->in_room != NULL && obj->sac_timer > 0 && --obj->sac_timer > 0))
	    continue;
	}
      else
	{
	  if (obj->carried_by != NULL &&
	      IS_NPC (obj->carried_by) && (!IS_AFFECTED (obj->carried_by, AFF_CHARM)))
	    continue;
	  /* if ( obj->carried_by != NULL )
	     act("$p has no place in the universe.",
	     obj->carried_by,obj,NULL,TO_CHAR); */
	}

      switch (obj->item_type)
	{
	default:
	  message = "$p vanishes.";
	  break;
	case ITEM_FOUNTAIN:
	  message = "$p dries up.";
	  break;
	case ITEM_CORPSE_NPC:
	  message = "$p decays into dust.";
	  break;
	case ITEM_CORPSE_PC:
	  message = "$p decays into dust.";
	  break;
	case ITEM_FOOD:
	  message = "$p decomposes.";
	  break;
	}

      if (obj->carried_by != NULL)
	{
	  if (!IS_SET (obj->extra_flags, ITEM_NOT_VALID) &&
	      !IS_SET (obj->pIndexData->extra_flags, ITEM_NOT_VALID))
	    act (message, obj->carried_by, obj, NULL, TO_CHAR);
	}
      else if (obj->in_room != NULL &&
	       (rch = obj->in_room->first_person) != NULL)
	{
	  act (message, rch, obj, NULL, TO_ROOM);
	  act (message, rch, obj, NULL, TO_CHAR);
	}
      is_owned = FALSE;
      owner = NULL;
      if (obj->owned_by != 0 && (owner = get_pvnum_index (obj->owned_by)) != NULL
	  && !IS_NPC (owner) && owner->pcdata->corpse == obj)
	is_owned = TRUE;
      if (!IS_SET (obj->extra_flags, ITEM_NOT_VALID) &&
	  !IS_SET (obj->pIndexData->extra_flags, ITEM_NOT_VALID))
	if (obj->first_content != NULL)
	  for (objc = obj->first_content; objc != NULL; objc = objc_next)
	    {
	      objc_next = objc->next_content;
	      obj_from_obj (objc);
	      if (obj->carried_by == NULL)
		{
		  if (obj->in_room != NULL)
		    {
		      obj_to_room (objc, obj->in_room);
		      objc->sac_timer = OBJ_SAC_TIME;
		    }
		  else
		    extract_obj (objc);
		}
	      else
		obj_to_char (objc, obj->carried_by);
	    }
      if (obj->first_content != NULL)
	{
	  OBJ_DATA *tobj, *nobj;
	  for (tobj = obj->first_content; tobj != NULL; tobj = nobj)
	    {
	      nobj = tobj->next_content;
	      obj_from_obj (tobj);
	      if (owner != NULL && owner->in_room != NULL)
		obj_to_room (tobj, owner->in_room);
	      else
		extract_obj (tobj);
	    }
	}
      if (owner != NULL && obj->wear_loc != WEAR_NONE)
	remove_obj (owner, obj->wear_loc, TRUE, FALSE);
      extract_obj (obj);
      if (is_owned)
	{
	  send_to_char ("Your corpse decays.\n\r", owner);
	  owner->pcdata->corpse = find_char_corpse (owner, TRUE);
	  save_char_obj (owner, NORMAL_SAVE);
	}
    }

  return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void
aggr_update (void)
{
  CHAR_DATA *wch;
  CHAR_DATA *ch;
  PLAYER_GAME *npl, *next_npl;
  int oldest_time, oldest_clan_time, current_time, new_time;
  CHAR_DATA *oldest;
  CLAN_DATA *oldest_clan, *clan;
  int align_dif;
  struct act_prog_data *apdtmp;

  /* char buf[150]; */
  /* check mobprog act queue */
  while ((apdtmp = mob_act_list) != NULL)
    {
      wch = mob_act_list->vo;
      if (wch && wch->mpactnum > 0)
	{
	  MPROG_ACT_LIST *tmp_act;

	  while ((tmp_act = wch->mpact) != NULL)
	    {
	      if (tmp_act->obj)
		tmp_act->obj = NULL;
	      if (tmp_act->ch)
		mprog_wordlist_check (tmp_act->buf, wch, tmp_act->ch,
				      tmp_act->obj, tmp_act->vo, ACT_PROG);
	      wch->mpact = tmp_act->next;
	      STRFREE (tmp_act->buf);
	      DISPOSE (tmp_act);
	    }
	  wch->mpactnum = 0;
	  wch->mpact = NULL;
	}
      mob_act_list = apdtmp->next;
      DISPOSE (apdtmp);
    }

  for (wch = first_char; wch != NULL; wch = aggr_wch_next)
    {
      aggr_wch_next = wch->next;
      if (wch->in_room == NULL)
	{
	  if (IS_NPC (wch))
	    extract_char (wch, TRUE);
	  continue;
	}

      if (!IS_NPC (wch) && wch->desc == NULL)
	wch->wait = 0;

      if (IS_NPC (wch))
	{
	  if (IS_SET (wch->act, ACT_WILL_DIE))
	    {
	      REMOVE_BIT (wch->act, ACT_WILL_DIE);
	      raw_kill (wch);
	      continue;
	    }
	}
    }

  if (save_delay >= 0)
    save_delay--;
  else
    {

      if (total_characters > 0)
	save_delay = (SAVE_DELAY * PULSE_PER_SECOND * 30 / total_characters);
      else
	save_delay = 100;
      /* delays SAVE_DELAY minutes between saves for each character */


      /*  Look for oldest not saved */
      current_time = get_game_realtime ();
      oldest_time = 0;
      oldest_clan_time = 0;
      oldest = NULL;
      oldest_clan = NULL;
      new_time = 0;
      for (npl = first_player; npl != NULL; npl = npl->next)
	if (npl->ch->level > 0)
	  {
	    new_time = current_time - npl->ch->pcdata->last_saved;
	    if (new_time < 0)	/* 24 hour roll-over */
	      new_time += (60 * 60 * 24);
	    if (new_time > oldest_time)
	      {
		oldest_time = new_time;
		oldest = npl->ch;
	      }
	  }
      for (clan = first_clan; clan != NULL; clan = clan->next)
	{
	  new_time = current_time - clan->last_saved;
	  if (new_time < 0)
	    new_time += (60 * 60 * 24);
	  if (new_time > oldest_clan_time)
	    {
	      oldest_clan_time = new_time;
	      oldest_clan = clan;
	    }
	}
      if (oldest != NULL)
	{
	  /* new_time = (current_time - oldest->pcdata->last_saved) ;
	     if( new_time < 0)    
	     new_time += (60*24*60);
	     sprintf( buf, "Autosaving %s delayed %d sec", oldest->name , new_time );
	     log_string( buf );  */
	  save_char_obj (oldest, NORMAL_SAVE);
	  /* If the save failed, then last_saved doesn't change.
	     Let's change it now, so the loop doesn't stick on one player. */
	  oldest->pcdata->last_saved = get_game_realtime ();
	}
      if (oldest_clan != NULL)
	{
	  save_clan (oldest_clan);
	  /* If the save failed, then last_saved doesn't change.
	     Let's change it now, so the loop doesn't stick on one player. */
	  oldest_clan->last_saved = get_game_realtime ();
	}
    }

  for (npl = first_player; npl != NULL; npl = next_npl)
    {
      next_npl = npl->next;
      wch = npl->ch;

      if (npl->ch->gold > 1000000 * npl->ch->level)
	npl->ch->gold = npl->ch->level * 1000000;

/*    Code for homonculous      
   if( is_desc_valid( wch)  && wch->desc->original!=NULL)
   if( number_range( 0,5)==1)
   wch=wch->desc->character;
   else
   continue;   */

      if (wch->level >= LEVEL_IMMORTAL || wch->in_room == NULL)
	continue;

      for (ch = wch->in_room->first_person; ch != NULL; ch = aggr_ch_next)
	{
	  aggr_ch_next = ch->next_in_room;

	  if (!IS_NPC (ch)
	      || ch->fighting
	      || IS_AFFECTED (ch, AFF_CHARM)
	      || !IS_AWAKE (ch)
	      || (IS_SET (ch->act, ACT_WIMPY) && IS_AWAKE (wch))
	      || !can_see (ch, wch))
	    continue;

	  if (wch->class == CLASS_MONK)
	    {
	      align_dif = wch->alignment - ch->alignment;
	      if (align_dif < 0)
		align_dif = 0 - align_dif;
	    }
	  else
	    align_dif = 1000;

	  if (is_hating (ch, wch))
	    {
	      found_prey (ch, wch);
	      continue;
	    }

	  /*  Let's get some battle going on here  - Chaos 5/31/95  */
	  if (!IS_SET (ch->act, ACT_AGGRESSIVE)
	      || (!can_see (ch, wch) && !IS_SET (ch->act, ACT_CLAN_GUARD))
	      || (ch->hit < ch->max_hit && !IS_SET (ch->act, ACT_CLAN_GUARD))
	      || !should_fight (ch, wch)
	      || number_range (0, 3) != 0
	      || pvnum_in_group (wch, ch->pIndexData->creator_pvnum)
	      || ((IS_SET (ch->act, ACT_CLAN_GUARD) &&
		   wch->pcdata->clan != NULL &&
		   wch->pcdata->clan ==
		   get_clan_from_vnum (ch->pIndexData->creator_pvnum)) ||
		  align_dif < 700))
	    continue;

	  /* make castle aggressives not attack their creator 
	     if(!IS_NPC(victim)&&pvnum_in_group(victim,ch->pIndexData->creator_pvnum))
	     continue;   */

	  multi_hit (ch, wch, TYPE_UNDEFINED);
	}
    }

  return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
int pulse_point;
void update_handler (void)
{
  static int pulse_shops;
  static int pulse_area;
  static int pulse_mobile;
  static int pulse_violence;
  PLAYER_GAME *gch;

  open_timer (TIMER_UPDATE);

  open_timer (TIMER_SECTOR);
  for (gch = first_player; gch != NULL; gch = gch->next)
    if (gch->ch != NULL && gch->ch->in_room != NULL && gch->ch->pcdata != NULL)
      {

	if (gch->ch->name == NULL)
	  {
	    do_quit (gch->ch, "arglebargle");
	    continue;
	  }


	if (gch->ch->in_room->sector_type == SECT_ASTRAL && gch->ch->race != RACE_GITH)
	  if (!IS_AFFECTED (gch->ch, AFF2_ASTRAL))
	    {
	      if (gch->ch->position == POS_FIGHTING)
		leave_fighting (gch->ch, gch->ch->in_room);
	      char_from_room (gch->ch);
	      if (room_index[gch->ch->pcdata->last_real_room]->sector_type == SECT_ASTRAL)
		char_to_room (gch->ch, room_index[ROOM_VNUM_TEMPLE]);
	      else
		char_to_room (gch->ch, room_index[gch->ch->pcdata->last_real_room]);
	    }

	if (gch->ch->in_room->fall_room != 0)
	  {
	    if (gch->ch->in_room->sector_type == SECT_AIR)
	      {
		if (!IS_AFFECTED (gch->ch, AFF_FLYING) && gch->ch->race != RACE_AVIARAN)
		  {
		    int dmg, dist, to_room;
		    dist = abs (gch->ch->in_room->distance_of_fall);
		    if (dist > 250)
		      dist = 250;
		    dmg = dist * ((dist / 4) + 1);
		    send_to_char ("You crash to the ground!", gch->ch);
                    if(gch->ch->race == RACE_TSARIAN)
                      dmg = dmg/2;
		    damage (gch->ch, gch->ch, dmg, TYPE_NOFIGHT);
		    act ("$n falls from the sky.", gch->ch, NULL, NULL, TO_ROOM);
		    to_room = gch->ch->in_room->fall_room;
		    char_from_room (gch->ch);
		    char_to_room (gch->ch, get_room_index (to_room));
		    continue;
		  }
	      }
	    else
	      /* Climbing stuff */
	      {
		int tot, skl, rng;
		skl = 0;
		if (multi (gch->ch, gsn_climb) != -1)
		  if (gch->ch->pcdata->learned[gsn_climb] > 0)
		    skl = (gch->ch->mclass[multi (gch->ch, gsn_climb)] *
			   gch->ch->pcdata->learned[gsn_climb]) / 1000;
		tot = 120 - get_curr_dex (gch->ch);
		rng = 6 * (5 - gch->ch->in_room->fall_slope) + 45;
		if (number_range (0, rng) < 10)
		  if (number_range (0, tot) > 95 + skl)
		    {
		      int dmg, dist, to_room;
		      char buft[MAX_INPUT_LENGTH];
		      dist = abs (gch->ch->in_room->distance_of_fall);
		      if (dist > 250)
			dist = 250;
		      dmg = dist * ((dist / 4) + 1);
		      sprintf (buft, "You fall %d feet to the ground!\n\r", dist);
		      send_to_char (buft, gch->ch);
		      damage (gch->ch, gch->ch, dmg, TYPE_NOFIGHT);
		      act ("$n falls to the ground.", gch->ch, NULL, NULL, TO_ROOM);
		      to_room = gch->ch->in_room->fall_room;
		      char_from_room (gch->ch);
		      char_to_room (gch->ch, get_room_index (to_room));
		      continue;
		    }
	      }
	  }

	if (gch->ch->in_room->sector_type == SECT_UNDER_WATER)
	  if (!IS_AFFECTED (gch->ch, AFF2_BREATH_WATER))
	    if (number_range (0, 1) == 0)
	      {
		if (!vnum_in_group (gch->ch, MOB_VNUM_WATER_ELEMENTAL))
		  {
		    send_to_char ("You cannot breath!", gch->ch);
		    damage (gch->ch, gch->ch, 50, TYPE_NOFIGHT);
		  }
		continue;
	      }

	if (gch->ch->in_room->sector_type == SECT_LAVA)
	  if (number_range (0, 3) == 0)
	    {
	      if (vnum_in_group (gch->ch, MOB_VNUM_FIRE_ELEMENTAL))
		{
		  send_to_char ("The fire elemental dampens the harmful effects of the lava!\n\r", gch->ch);
		  damage (gch->ch, gch->ch, 1, TYPE_NOFIGHT);
		  continue;
		}
	      send_to_char ("That lava is REALLY hot!", gch->ch);
              if(gch->ch->race == RACE_AVIARAN)
                damage (gch->ch, gch->ch, 16, TYPE_NOFIGHT);
              else
              {
	        if (IS_AFFECTED (gch->ch, AFF_FLYING))
	  	  damage (gch->ch, gch->ch, 8, TYPE_NOFIGHT);
	        else
		  damage (gch->ch, gch->ch, 20, TYPE_NOFIGHT);
              }
	      continue;
	    }

	if (gch->ch->in_room->sector_type == SECT_WATER_NOSWIM &&
	    !IS_AFFECTED (gch->ch, AFF_FLYING) && 
            gch->ch->race != RACE_AVIARAN)
	  if (!IS_AFFECTED (gch->ch, AFF2_BREATH_WATER))
	    if (number_range (0, 3) == 0)
	      {
		OBJ_DATA *obj;
		bool found;

		/*
		 * Look for a boat.
		 */
		found = FALSE;
		for (obj = gch->ch->first_carrying; obj != NULL;
		     obj = obj->next_content)
		  if (obj->item_type == ITEM_BOAT)
		    {
		      found = TRUE;
		      break;
		    }
		if (!found)
		  {
		    send_to_char ("You are sinking FAST!", gch->ch);
		    damage (gch->ch, gch->ch, 20, TYPE_NOFIGHT);
		    continue;
		  }
	      }
      }

  close_timer (TIMER_SECTOR);
  if (--pulse_area <= 0)
    {
      struct timeval now_time;
      pulse_area = number_range (PULSE_AREA / 2, 3 * PULSE_AREA / 2);
      open_timer (TIMER_AREA_UPD);
      area_update ();

      gettimeofday (&now_time, NULL);
      if (now_time.tv_sec > rent_time)
	{
	  CLAN_DATA *clan, *next_clan;
	  long rent;
	  struct tm now;
	  char buf[MAX_INPUT_LENGTH];

	  for (clan = first_clan; clan; clan = next_clan)
	    {
	      next_clan = clan->next;
	      if (clan->type == CLAN_PEACEFUL)
		rent = RENT_BASIC_ORDER_HALL;
	      else
		rent = RENT_BASIC_CLAN_HALL;

	      rent += RENT_PER_GUARD * clan->num_guards;
	      rent += RENT_PER_HEALER * clan->num_healers;
	      rent += RENT_PER_BACKDOOR * clan->num_backdoors;

	      if (clan->coffers - rent > 0)
		{
		  clan->coffers -= rent;
		  sprintf (buf, "You paid %ld in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
		  log_printf("Clan %s paid %ld in rent. Their coffers now at %ld.", clan->name, rent, clan->coffers);
		  send_clan_message (clan, buf);
		  continue;
		}
	      else
		{
		  if (clan->healer != 0 && clan->num_healers > 0)
		    {
		      MOB_INDEX_DATA *oldhealer = get_mob_index (clan->healer);
		      delete_mob (oldhealer);
		      rent -= RENT_PER_HEALER * clan->num_healers;
		      clan->coffers += clan->num_healers * RENT_PER_HEALER / 2;
		      clan->healer = 0;
		      clan->num_healers = 0;
		      if (clan->coffers - rent > 0)
			{
			  clan->coffers -= rent;
			  sprintf (buf, "Your healers were all fired to cover costs.\n\rYou paid %ld in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
		  	  log_printf("Clan %s paid %ld in rent after firing their healers. Their coffers now at %ld.", clan->name, rent, clan->coffers);
			  send_clan_message (clan, buf);
			  continue;
			}
		    }
		  if (clan->guard != 0 && clan->num_guards > 0)
		    {
		      MOB_INDEX_DATA *oldguard = get_mob_index (clan->guard);
		      delete_mob (oldguard);
		      rent -= RENT_PER_GUARD * clan->num_healers;
		      clan->coffers += clan->num_guards * RENT_PER_GUARD / 2;
		      clan->guard = 0;
		      clan->num_guards = 0;
		      if (clan->coffers - rent > 0)
			{
			  clan->coffers -= rent;
			  sprintf (buf, "Your guards were all fired to cover costs.\n\rYou paid %ld in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
		  	  log_printf("Clan %s paid %ld in rent after firing their guards. Their coffers now at %ld.", clan->name, rent, clan->coffers);
			  send_clan_message (clan, buf);
			  continue;
			}
		    }
		  if (clan->num_backdoors > 0)
		    {
		      ROOM_INDEX_DATA *room = NULL;
		      int vnum, door;
		      for (vnum = 20000; vnum < 32000; vnum++)
			{
			  if ((room = room_index[vnum]) != NULL
			      && IS_SET (room->room_flags, ROOM_IS_CASTLE)
			      && room->creator_pvnum == clan->founder_pvnum)
			    for (door = 0; door <= 5; door++)
			      if (IS_SET (room->exit[door]->exit_info, EX_CLAN_BACKDOOR))
				{
				  room->exit[door] = NULL;
				  STRFREE (room->exit[door]->description);
				  STRFREE (room->exit[door]->keyword);
				  DISPOSE (room->exit[door]);
				  top_exit--;
				}
			}

		      rent -= RENT_PER_BACKDOOR * clan->num_backdoors;
		      clan->coffers += clan->num_backdoors * RENT_PER_BACKDOOR / 2;
		      clan->num_backdoors = 0;
		      if (clan->coffers - rent > 0)
			{
			  clan->coffers -= rent;
			  sprintf (buf, "Your backdoors were re-possessed to cover costs.\n\rYou paid %ld in rent. Coffers now at %ld.\n\r", rent, clan->coffers);
		  	  log_printf("Clan %s paid %ld in rent after having their backdoors repossessed. Their coffers now at %ld.", clan->name, rent, clan->coffers);
			  send_clan_message (clan, buf);
			  continue;
			}
		    }
		  /* They cant pay rent even after liquidation! Better get rid
		     of the stinkin' freeloaders! */
		  sprintf (buf, "The clan of %s has been disbanded due to lack of adequate funding!", clan->name);
		  do_echo (NULL, buf);

		  destroy_clan (clan);
		}
	    }
	  now = *localtime (&current_time);
	  do
	    {
	      if (++now.tm_wday > 6)
		now.tm_wday = 0;
	      now.tm_yday++;
	      if ((isleap (now.tm_year + 1900) && now.tm_mon == 1 && ++now.tm_mday > 29)
		  || (++now.tm_mday > monthdays[now.tm_mon]))
		{
		  now.tm_mon++;
		  now.tm_mday = 0;
		}
	    }
	  while (now.tm_wday != 0);
	  now.tm_min = 59;
	  now.tm_hour = 23;
	  now.tm_sec = 59;

	  rent_time = mktime (&now);

	}
      close_timer (TIMER_AREA_UPD);
    }

  if (--pulse_mobile <= 0)
    {
      pulse_mobile = PULSE_MOBILE;
      open_timer (TIMER_AREA_UPD);
      mobile_update ();
      close_timer (TIMER_AREA_UPD);
    }

  if (--pulse_shops <= 0)
    {
      pulse_shops = PULSE_SHOPS;
      open_timer (TIMER_SHOP_UPD);
      shop_update ();
      close_timer (TIMER_SHOP_UPD);
    }

  if (--pulse_violence <= 0)
    {
      pulse_violence = PULSE_VIOLENCE;
      open_timer (TIMER_VIOL_UPD);
      violence_update ();
      close_timer (TIMER_VIOL_UPD);
    }

  if (--pulse_point <= 0)
    {
      open_timer (TIMER_WEATHER);
      pulse_point = number_range (PULSE_TICK / 2, 3 * PULSE_TICK / 2);
      weather_update ();
      close_timer (TIMER_WEATHER);

      open_timer (TIMER_CHAR_UPD);
      char_update ();

      open_timer (TIMER_OBJ_UPD);
      obj_update ();
      close_timer (TIMER_OBJ_UPD);

      bounty_update();
    }

  if (--pulse_aggressive <= 0)
    {
      pulse_aggressive = PULSE_PER_SECOND / 2;
      open_timer (TIMER_AGGR_UPD);
      aggr_update ();
      close_timer (TIMER_AGGR_UPD);
    }

  close_timer (TIMER_UPDATE);

  tail_chain ();
  return;
}

int
exp_level (int class, int level)
{
  float a, b, c, d, num, cap, inc, onum;
  int cnt, lv;

  if (level == 0)
    return (0);

  switch (class)
    {
    case CLASS_ILLUSIONIST:
      a = 1685.0;
      b = 1.57;
      c = 1666.0;
      d = 0 - 250.0;
      cap = 4900000;
      inc = 1800000;
      break;

    case CLASS_ELEMENTALIST:
      a = 1675.0;
      b = 1.55;
      c = 1633.0;
      d = 0 - 280.0;
      cap = 4200000;
      inc = 1600000;
      break;

    case CLASS_ROGUE:
      a = 1500.0;
      b = 1.5;
      c = 1500.0;
      d = 0 - 200;
      cap = 3000000;
      inc = 1500000;
      break;

    case CLASS_RANGER:
      a = 1500.0;
      b = 1.55;
      c = 1500.0;
      d = 0 - 300.0;
      cap = 4000000;
      inc = 1750000;
      break;


    case CLASS_ASSASSIN:
      a = 1500.0;
      b = 1.54;
      c = 1500.0;
      d = 0 - 300.0;
      cap = 3950000;
      inc = 1700000;
      break;


    case CLASS_MONK:
      a = 1500.0;
      b = 1.52;
      c = 1500.0;
      d = 0 - 300.0;
      cap = 3800000;
      inc = 1500000;
      break;


    case CLASS_NECROMANCER:
      a = 1600.0;
      b = 1.59;
      c = 1500.0;
      d = 0 - 250.0;
      cap = 4800000;
      inc = 2000000;
      break;

    case CLASS_MONSTER:
      a = 45.0;
      b = 1.35;
      c = 120.0;
      d = 0 - 50.0;
      cap = 50000;
      inc = 7000;
      break;

    default:
      return (0);
    }

  num = b;

  for (cnt = 1; cnt < level; cnt++)
    num *= b;

  num = a * num + c * (level - 1) + d;
  if (num < cap)
    return ((int) num);

  lv = 12;
  num = 0;
  while (num < cap)
    {
      onum = b;
      for (cnt = 1; cnt < lv; cnt++)
	onum *= b;
      onum = a * onum + c * (lv - 1) + d;
      if (num < cap)
	{
	  num = onum;
	  lv++;
	}
    }
  for (; lv <= level; lv++)
    {
      num += inc;
      if (class != CLASS_MONSTER)
	inc = (int) ((float) inc * 1.03);
      else
	inc = (int) ((float) inc * 1.01);
    }
  return ((int) num);
}



/*
 * BOUNTY_UPDATE
 *
 * Delete any expired bounties.
 *
 * Presto 2-20-99
 */
void bounty_update( void )
{
  BOUNTY_DATA *bptr = NULL;
  
  for (bptr= first_bounty; bptr != NULL; bptr = bptr->next)
  {
    if((bptr->expires - (current_time - bptr->postdate)) <= 0)
    {
      remove_bounty( bptr );
      save_bounties();
    }
  }
  return;
}


/*  The loss for an advancement of level  -  Chaos 5/2/99  */
int initiate_hp_loss( int class, int reinc )
  {
  int add_hp;
    switch( class )
      {
      case CLASS_RANGER:
        add_hp   = (3+reinc*4);
        break;
      case CLASS_ROGUE:
        add_hp   = (2+reinc*3);
        break;
      case CLASS_ILLUSIONIST:
        add_hp   = (0+reinc*3);
        break;
      case CLASS_MONK:
        add_hp   = (0+reinc*1);
        break;
      case CLASS_ELEMENTALIST:
        add_hp   = (1+reinc*2);
        break;
      case CLASS_NECROMANCER:
        add_hp   = (1+reinc*2);
        break;
      default:
        add_hp   = (2+reinc*5);
        break;
      }
  return( add_hp );
  }


/*  The loss for an advancement of level  -  Chaos 5/2/99  */
int initiate_mana_loss( int class, int reinc )
  {
  int add_mana;
    switch( class )
      {
      case CLASS_RANGER:
        add_mana = (0+reinc*2);
        break;
      case CLASS_ROGUE:
        add_mana = (0+reinc*1);
        break;
      case CLASS_ILLUSIONIST:
        add_mana = (3+reinc*3);
        break;
      case CLASS_MONK:
        add_mana = (1+reinc*2);
        break;
      case CLASS_ELEMENTALIST:
        add_mana = (1+reinc*2);
        break;
      case CLASS_NECROMANCER:
        add_mana = (1+reinc*2);
        break;
      default:
        add_mana = (2+reinc*4);
        break;
      }
  return( add_mana );
  }

/*  The loss for an advancement of level  -  Chaos 5/2/99  */
int initiate_move_loss( int class, int reinc )
  {
  int add_move;

    switch( class )
      {
      case CLASS_RANGER:
        add_move = (0+reinc*1);
        break;
      case CLASS_ROGUE:
        add_move = (0+reinc*2);
        break;
      case CLASS_ILLUSIONIST:
        add_move = (0+reinc*1);
        break;
      case CLASS_MONK:
        add_move = (1+reinc*1);
        break;
      case CLASS_ELEMENTALIST:
        add_move = (0+reinc*1);
        break;
      case CLASS_NECROMANCER:
        add_move = (0+reinc*2);
        break;
      default:
        add_move = (2+reinc*3);
        break;
      }

  return(add_move);
  }
