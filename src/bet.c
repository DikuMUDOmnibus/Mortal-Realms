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
    for( gpl=game_list; gpl!=NULL; gpl=gpl->next )
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
              gpl->ch->fighting == old_victim &&
              old_victim->fighting == gpl->ch )
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

  if( ch->in_room->area->first_room != ROOM_VNUM_ARENA )
    {
    send_to_char( "You are not in the arena.\n\r", ch);
    return;
    }

  victim=lookup_char( name );
  if( victim == NULL || victim->in_room->area->first_room != ROOM_VNUM_ARENA ||
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
  for( gpl=game_list; gpl!=NULL; gpl=gpl->next )
    if( gpl->ch->in_room->area->first_room == ROOM_VNUM_ARENA  &&
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

  if( ch->in_room->area->first_room != ROOM_VNUM_ARENA )
    {
    send_to_char( "You are not in the arena.\n\r", ch);
    return;
    }

  victim=lookup_char( name );
  if( victim == NULL || victim->in_room->area->first_room != ROOM_VNUM_ARENA ||
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
  for( gpl=game_list; gpl!=NULL; gpl=gpl->next )
    if( gpl->ch->in_room->area->first_room == ROOM_VNUM_ARENA &&
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


