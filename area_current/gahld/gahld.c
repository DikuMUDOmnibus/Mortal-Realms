/* 
 * "gahld.c" to be included somewhere in between functions in spec_procs.c
 */

#include "gahld.h" 

/* predeclarations */
bool	cast_spell 	args( (CHAR_DATA *ch,char *spell) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void 	do_kill		args( ( CHAR_DATA *ch, char *argument ) );

/* objects */


/* mobiles */

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
    for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
"A lizardy voice says 'I prefer killing good things...but, a corpse IS a corpse!'"
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
    for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
"A lizardy voice says 'I prefer killing good things...but, a corpse IS a corpse!'"
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
  
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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

  if((tobj = get_obj_list(ch,"cherish", ch->carrying)))
    {
    free_string(&tobj->name);
    tobj->name=str_dup("coin unique almostcherish");
    do_say(ch,"Thanks!  I'll cherish it forever!");
    interpret(ch,"wear coin");
    do_say(ch,"And here's a small token of my gratitude.");
    interpret(ch,"remove sceptre");
    interpret(ch,"drop sceptre");
    }
  if((tobj = get_obj_list(ch,"wifes", ch->carrying)))
    {
    free_string(&tobj->name);
    tobj->name=str_dup("scarab ruby");
    do_say(ch,"At last!  A token of love from my long-dead wife!");
    do_say(ch,"Here is my beloved coin in exchange for this wonderous gift!");
    interpret(ch,"remove coin");
    if((tobj = get_obj_list(ch,"almostcherish", ch->carrying)))
      {
      AFFECT_DATA *paf;

	      paf		= alloc_mem( sizeof(*paf) );

      paf->type		= 0;
      paf->duration	= -1;
      paf->location	= APPLY_DAMROLL;
      paf->modifier	= 5;
      paf->bitvector	= 0;
      paf->next		= tobj->affected;
      tobj->affected	= paf;

      free_string(&tobj->name);
      tobj->name=str_dup("coin unique verycherished");
      interpret(ch,"drop coin");
      }
    }
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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

  if((tobj = get_obj_list(ch,"ruling", ch->carrying)))
    {
    free_string(&tobj->name);
    tobj->name=str_dup("sceptre golden");
    do_say(ch,"Thanks!  Now I'll rule the kingdom!");
    do_say(ch,"And here's you a worthless trinket from my 'beloved.'");
    interpret(ch,"remove scarab");
    interpret(ch,"drop scarab");
    }
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
    for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
      if((tch->position!=POS_FIGHTING)&&(number_percent()>90))
        return cast_spell(ch,"charm person");
  return 0;
  }

bool gahld_vampbat(CHAR_DATA *ch)
  {
  CHAR_DATA *tch;

  if(number_percent()>80)
    npc_do_emote(ch,"flaps its wings noisily.");
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
    for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
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
  for(tch=rm->people;tch!=NULL;tch=tch_next)
    {
    tch_next=tch->next_in_room;
    if((!IS_NPC(tch))&&(tch->position==POS_SLEEPING)&&(get_obj_list(tch,"firechain",tch->carrying)!=NULL))
      {
      char_from_room(tch);
      char_to_room(tch,get_room_index(GAHLD_BASE+96));
      send_to_char("You have been transported!\n\r",tch);
      }
    }
  for(tch=ch->in_room->people;tch!=NULL;tch=tch->next_in_room)
   if((!IS_NPC(tch))&&(tch->level>(ch->level)||(get_obj_list(tch,"band",tch->carrying)!=NULL)))
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

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
    v_next = victim->next_in_room;
    if ( victim->fighting == ch && number_bits( 2 ) == 0 )
      break;
    }

  if ( victim == NULL )
    return FALSE;

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
  return TRUE;
  }
