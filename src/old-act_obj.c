do_forge( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  OBJ_DATA  *obj;
  bool failure;

  if( !IS_NPC( ch ) && ch->fighting != NULL  && !IS_NPC( ch->fighting ))
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
    obj->unforged_name=STRALLOC(obj->name);
    STRFREE (obj->name );
    obj->name = STRALLOC(argument);

    obj->unforged_short_descr=STRALLOC(obj->short_descr);
    STRFREE (obj->short_descr );
    obj->short_descr = STRALLOC(argument);

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
  int idmgL, idmgH, cdmgL, cdmgH, cdurL, cdurH;
  POISON_DATA *pd;
  CHAR_DATA *mob;

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


	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
        send_to_char( "You can only make poisons in a guild.\n\r", ch );
        return;
	}


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

  if( obj->item_type != ITEM_WEAPON && obj->item_type!=ITEM_AMMO &&
       obj->item_type != ITEM_FOOD && obj->item_type!=ITEM_DRINK_CON )
    {
    send_to_char( "That item is not a weapon, ammo, food or drink.\n\r", ch );
    return;
    }

  WAIT_STATE( ch, skill_table[gsn_make_poison].beats );
  failure=(IS_NPC(ch))?FALSE:(number_percent()>ch->pcdata->learned[gsn_make_poison]);

  if( *arg2 == 'b' || *arg2 == 'B' )
    {
    idmgL = 0;
    idmgH = 0;
    cdmgL = level/6;
    cdmgH = level/2;
    cdurL = 2;
    cdurH = 4;
    }
  if( *arg2 == 'p' || *arg2 == 'P' )
    {
    idmgL = level/3;
    idmgH = 2*level/3;
    cdmgL = 0;
    cdmgH = 0;
    cdurL = 0;
    cdurH = 0;
    }
  if( *arg2 == 'g' || *arg2 == 'G' )
    {
    idmgL = level/6;
    idmgH = level/3;
    cdmgL = level/12;
    cdmgH = level/5;
    cdurL = 2;
    cdurH = 4;
    }
  

  if( failure )
    {
    send_to_char( "You poison yourself!\n\r", ch );
    CREATE( pd, POISON_DATA, 1);
    pd->next = ch->pcdata->poison;
    ch->pcdata->poison = pd;
    pd->for_npc = FALSE;
    pd->instant_damage_low = idmgL;
    pd->instant_damage_high = idmgH;
    pd->constant_damage_low = cdmgL;
    pd->constant_damage_high = cdmgH;
    pd->constant_duration = number_range( cdurL, cdurH );
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
  
  CREATE( pd, POISON_DATA, 1);
  pd->next = NULL;
  obj->poison = pd;
  if( *arg1 == 'c' || *arg1 == 'C' )
    pd->for_npc = FALSE;
  else
    pd->for_npc = TRUE;
  pd->instant_damage_low = idmgL;
  pd->instant_damage_high = idmgH;
  pd->constant_damage_low = cdmgL;
  pd->constant_damage_high = cdmgH;
  pd->constant_duration = number_range( cdurL, cdurH );
  if( IS_NPC( ch ) )
    pd->owner = 0;
  else
    pd->owner = ch->pcdata->pvnum;

  return;
  }

void do_make_flash( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *mob;
  int cnt;

  cnt = multi( ch, gsn_flash_powder );
  if( cnt == -1 )
    {
    send_to_char( "You can't make flash powder.\n\r", ch );
    return;
    }

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
  {
    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
     break;
  }

  if ( mob == NULL )
  {
   send_to_char( "You can only make flash powder in a guild.\n\r", ch );
   return;
  }
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
            ch->gold += amount;
            ch->pcdata->account -= amount;
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
