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
#include <signal.h>
#include <pthread.h>
#include "merc.h"

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );*/

   /* Increasing this number will wipe all items of previous chars!!! */
   /* This is used to clean out problem items and such when out of hand */

#define OBJECT_VERSION_NUMBER 1

/*
 * Local functions.
 */
int total_language( CHAR_DATA *);
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_corpse	args( ( CHAR_DATA *ch,  FILE *fp ) );
OBJ_DATA *fread_corpse_item	( OBJ_DATA *, CHAR_DATA *, FILE *, bool );
void  fwrite_corpse ( CHAR_DATA *, FILE *);
void	fwrite_corpse_item	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void clear_objects( OBJ_DATA *);
void fwrite_poison_data( POISON_DATA *, FILE *);
POISON_DATA *fread_poison_data( FILE *);

bool is_valid_file( CHAR_DATA *, FILE *);

int gsn_enchant;

int fork( void );

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
#ifdef USE_THREADS
void save_char_obj(CHAR_DATA *ch, int which_type)
{
 SAVE_DATA * threaddata;
 pthread_t save_thread;
 CREATE(threaddata, SAVE_DATA, 1);
 threaddata->ch = ch;
 threaddata->type = which_type;
 if (IS_NPC(ch))
  return;

 if (pthread_create(&save_thread, NULL, threaded_save_char, threaddata) <0)
  {
   perror("save_thread barf");
   return;
  }
 ch->pcdata->beingsaved=save_thread;
 DISPOSE(threaddata);
}

void *threaded_save_char(void * args)
{
    SAVE_DATA *DaSwag = (SAVE_DATA *) args;
    CHAR_DATA *ch = DaSwag->ch;
    int which_type = DaSwag->type;
#else
void save_char_obj(CHAR_DATA *ch, int which_type)
{
#endif

    char strsave[MAX_INPUT_LENGTH], strtemp[MAX_INPUT_LENGTH],buf[80];
    char cap_name[30];
    FILE *fp;
    ROOM_INDEX_DATA *troom;
    bool IS_DESC;
    int old_time;
    int game_time_1, game_time_2, game_time_3, game_time_4, game_time_5;
       /* let parent back out of save  */

    old_time = get_game_usec();

    if ( IS_NPC(ch) || ch->level < 1)
      { 
#ifdef USE_THREADS
 	if (!IS_NPC(ch))
	 ch->pcdata->beingsaved=NULL;
	log_printf("Thread committing suicide. NPC or level 0.");
 	pthread_detach(pthread_self());
	pthread_exit(NULL);
#else
        return;
#endif
      }
    IS_DESC = is_desc_valid( ch );
    if ( IS_DESC && ch->desc->original != NULL )
      {
      send_to_char( "You can't save out of body!\n\r", ch);
#ifdef USE_THREADS
 	if (!IS_NPC(ch))
	 ch->pcdata->beingsaved=NULL;
	log_printf("Thread committing suicide. Out of body experience.");
 	pthread_detach(pthread_self());
	pthread_exit(NULL);
#else
        return;
#endif
      }
    troom=ch->in_room;
    if( troom == NULL )
      {
	  bug( "Save_char_obj: char was not in a room", 0 );
#ifdef USE_THREADS
 	if (!IS_NPC(ch))
	 ch->pcdata->beingsaved=NULL;
	log_printf("Thread committing suicide. Room was NULL.");
 	pthread_detach(pthread_self());
	pthread_exit(NULL);
#else
        return;
#endif
      }
      
    if( IS_SET(troom->room_flags,ROOM_NO_SAVE))
      { /* set save room to first room in area */
      char_from_room( ch );

          /* Check to see if they were in a Rip */
      if( troom->area->low_r_vnum != 1 )
        char_to_room( ch, get_room_index(troom->area->low_r_vnum) );
      else
        char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
      }
    ch->save_time = current_time;
    game_time_1=get_game_usec();
    game_time_3=0;

    open_timer( TIMER_CHAR_SAVE );

#ifndef USE_THREADS
      fclose( fpReserve );
#endif

    buf[0]= *ch->name;
    buf[1]='\0';
    if( buf[0]>='A' && buf[0]<='Z')
      buf[0]+= ('a' - 'A');
    strcpy( cap_name, capitalize_name( ch->name ) );

    if(which_type == NORMAL_SAVE)
    {
      sprintf(strsave,"%s/%s/%s", PLAYER_DIR,  buf , cap_name );
      sprintf(strtemp,"%s/%s/temp.%s",PLAYER_DIR,buf, cap_name );
    }
    else
    {
      sprintf(strsave,"%s/%s/bak/%s", PLAYER_DIR,  buf , cap_name );
      sprintf(strtemp,"%s/%s/bak/temp.%s",PLAYER_DIR,buf, cap_name );
    }
/* save char to "temp.name" then rename to "name" after all done for safety */
    game_time_2=get_game_usec();

    remove( strtemp );
    fp = fopen( strtemp, "w" ) ;
    if ( fp == NULL )
      {
	  bug( "Save_char_obj: first fopen", 0 );
	  perror( strsave );
          send_to_char( "Through some wierd game error, your character did not save.\n\r", ch);
          fpReserve = fopen( NULL_FILE, "r");

#ifdef USE_THREADS
 	if (!IS_NPC(ch))
	 ch->pcdata->beingsaved=NULL;
	log_printf("Thread committing suicide. Wierd game error. fp NULL.");
 	pthread_detach(pthread_self());
	pthread_exit(NULL);
#else
        return;
#endif
      }
      {
      game_time_3=get_game_usec();
      fwrite_char( ch, fp );
      if ( ch->first_carrying != NULL )
        {
        clear_objects( ch->first_carrying );
        fwrite_obj( ch, ch->first_carrying, fp, 0 );
        }
      if ( !IS_NPC(ch) && ch->pcdata->corpse!=NULL)
        fwrite_corpse( ch , fp);
      fprintf( fp, "#END %s\n", ch->name );
      }

    game_time_4=get_game_usec();
    if(ftell(fp) < (long)100)
      {
      fclose( fp );
      send_to_char("Oops, file system full! tell Order or Chaos!\n\r",ch);
      }
    else
     {
       fclose( fp );


       fp = fopen( strtemp, "r" ) ;
       if( !is_valid_file( ch, fp ) )
         {
         char tbuf[200];
         fclose( fp );
         sprintf( tbuf, "SAVE not valid for %s", ch->name );
         log_string( tbuf );
         send_to_char( "The file system has become unstable.  Please inform the Gods.\n\r", ch );
         remove( strtemp ); 
         }
       else
         {
         fclose( fp );
         remove( strsave ); 
         rename( strtemp, strsave ); 
         }
     }


      if( COMPRESS_FILES )
        {
        char qbuf[200];
        sprintf( qbuf, "gzip -1fq %s &", strsave);
        system( qbuf );
        } 

#ifndef USE_THREADS
      fpReserve = fopen( NULL_FILE, "r");
#endif

    game_time_5=get_game_usec();

    close_timer( TIMER_CHAR_SAVE );
    
    ch->pcdata->last_saved = get_game_realtime();

    /* restore char to proper room if in NO_SAVE room */
    if( ch->in_room != troom )
      {
      char_from_room( ch );
      char_to_room( ch, troom );
      }

    if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_WIZTIME ))
      {
      char qbuf[200];
      sprintf( qbuf, "(%d usec to close fpReserved)\n\r", 
                   game_time_2-game_time_1);
      send_to_char( qbuf, ch);
      sprintf( qbuf, "(%d usec to open fp)\n\r", 
                   game_time_3-game_time_2);
      send_to_char( qbuf, ch);
      sprintf( qbuf, "(%d usec to write fp)\n\r", 
                   game_time_4-game_time_3);
      send_to_char( qbuf, ch);
      sprintf( qbuf, "(%d usec to close fp)\n\r", 
                   game_time_5-game_time_4);
      send_to_char( qbuf, ch);
      sprintf( qbuf, "(%d usec to open fpReserved)\n\r", 
                   get_game_usec()-game_time_5);
      send_to_char( qbuf, ch);
      }
#ifdef USE_THREADS
 	if (!IS_NPC(ch))
	 ch->pcdata->beingsaved=NULL;
        log_printf("Thread commiting suicide. Save successful.");
 	pthread_detach(pthread_self());
	pthread_exit(NULL);
        return NULL;
#else
        return;
#endif
}


void clear_objects( OBJ_DATA *sobj )
{
  OBJ_DATA *obj;
  obj=sobj;
  while( obj!=NULL )
    {
    obj->saved=FALSE;
    if( obj->first_content != NULL )
      clear_objects( obj->first_content );
    obj=obj->next_content;
    }
  return;
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    bool IS_DESC;
    int sn;
    sh_int cnt;
    
    IS_DESC = is_desc_valid( ch );
    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);

    fprintf( fp, "Name         %s~\n",	ch->name		);
    if(IS_DESC)
    {
        fprintf( fp, "Lastlogin    %s~\n",  ch->desc->host      );
        fprintf( fp, "LastDomain   %s~\n",  ch->desc->domain    );
    }
    fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr		);
    fprintf( fp, "LongDescr    %s~\n",	ch->long_descr		);
    fprintf( fp, "Description  %s~\n",  ch->description		);
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Class        %d\n",	ch->class		);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Language     %d\n",   ch->language            );
    fprintf( fp, "Speak        %d\n",   ch->speak               );
    fprintf( fp, "Level        %d\n",	ch->level		);
    fprintf( fp, "Trust        %d\n",	ch->trust		);
    fprintf( fp, "Played       %d\n",   ch->played 		);
    fprintf( fp, "KLR_Played   %d\n",   ch->killer_played       );
    fprintf( fp, "Critical     %d\n",	ch->critical_hit_by	);
    fprintf( fp, "OutCastPlay  %d\n",   ch->outcast_played      );
    fprintf( fp, "Room         %u\n",
	(  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&& ch->was_in_room != NULL
        && !IS_SET(ch->was_in_room->room_flags,ROOM_NO_SAVE))
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum );

    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Actuals   %d %d %d\n",
	ch->actual_max_hit, ch->actual_max_mana, ch->actual_max_move );
    fprintf( fp, "Gold         %d\n",	ch->gold		);
    fprintf( fp, "Account      %d\n",	ch->pcdata->account		);
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    fprintf( fp, "Explost      %d\n",	ch->exp_lost);
    if( IS_SET(ch->pcdata->request, REQUEST_PROMPT_SAVE_ON))
      fprintf( fp, "Act          %d\n",   ch->act|PLR_PROMPT);
    else
      fprintf( fp, "Act          %d\n",   ch->act			);
    fprintf( fp, "AffectedBy   %d\n",	ch->affected_by		);
    fprintf( fp, "Affected2By   %d\n",	ch->affected2_by		);
    /* Bug fix from Alander */
    fprintf( fp, "Position     %d\n",
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Player2_Bits %d\n", ch->pcdata->player2_bits );

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "GivePracNeg  %d\n",	ch->pcdata->give_prac_neg);
    fprintf( fp, "GivePracPos  %d\n",	ch->pcdata->give_prac_pos);
    if (ch->pcdata->demisedlevel != 0)
      fprintf( fp, "DemiseLevel  %d\n",	ch->pcdata->demisedlevel);
    fprintf( fp, "SavingThrow  %d\n",	ch->saving_throw	);
    fprintf( fp, "EQSaves      %d\n",	ch->pcdata->eqsaves     );
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "EQHitroll    %d\n",	ch->pcdata->eqhitroll	);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "EQDamroll    %d\n",	ch->pcdata->eqdamroll	);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    fprintf( fp, "Deaf         %d\n",	ch->deaf		);
    fprintf( fp, "Channel      %d\n",	ch->pcdata->channel	);
    if (ch->pcdata->clan_name[0] != '\0')
      fprintf( fp, "ClanName     %s~\n",	ch->pcdata->clan_name	);
    if (ch->pcdata->clan_pledge[0] != '\0')
      fprintf( fp, "ClanPledge     %s~\n",	ch->pcdata->clan_pledge	);
    if (ch->pcdata->clan_position != 0)
      fprintf( fp, "ClanPosition     %d\n",	ch->pcdata->clan_position	);
    fprintf( fp, "LastTime     %d\n",   (int)current_time );
    fprintf( fp, "Arrested     %d\n",   ch->pcdata->arrested);
    fprintf( fp, "Jailtime     %d\n",   ch->pcdata->jailtime);
    fprintf( fp, "Jaildate     %d\n",   ch->pcdata->jaildate);

    fprintf( fp, "Mailaddress  %s~\n",  ch->pcdata->mail_address);
    fprintf( fp, "Htmladdress  %s~\n",  ch->pcdata->html_address);
    fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
    fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
    fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
    fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
      /*  This will go in for deleting items- changing number will wipe all */
    fprintf( fp, "Obj_Ver_Num  %d\n",OBJECT_VERSION_NUMBER);
 
	  fprintf( fp, "AttrPerm     %d %d %d %d %d\n",
	    ch->pcdata->perm_str,
	    ch->pcdata->perm_int,
	    ch->pcdata->perm_wis,
	    ch->pcdata->perm_dex,
	    ch->pcdata->perm_con );

	fprintf( fp, "AttrMod      %d %d %d %d %d\n",
	    ch->pcdata->mod_str, 
	    ch->pcdata->mod_int, 
	    ch->pcdata->mod_wis,
	    ch->pcdata->mod_dex, 
	    ch->pcdata->mod_con );

	fprintf( fp, "Condition    %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2] );

        for(cnt=0;cnt<MAX_CLASS;cnt++)
          fprintf( fp, "Mclass  %d %d\n", cnt, ch->mclass[cnt]);

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
            if( multi(ch, sn) != -1 )
	    {
		fprintf( fp, "Skill        %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

  {
  AREA_DATA *pArea;
  int cnt;

  for(pArea=first_area;pArea!=NULL;pArea=pArea->next)
    {
    sn=pArea->low_r_vnum/100;
    if(is_quest(ch->pcdata->quest[sn]))
      {
      fprintf(fp,"Qstb %d %d ",MAX_QUEST_BYTES, sn);
      for(cnt=0; cnt<MAX_QUEST_BYTES; cnt++)
        fprintf(fp,"%d ",ch->pcdata->quest[sn][cnt]);
      fprintf(fp,"\n");
      }
    }
  }
        fprintf( fp, "Speed   %d\n", ch->speed);
        fprintf( fp, "Notes   %d\n", amount_note(ch));
        if(ch->vt100!=0 || IS_SET(ch->pcdata->request, REQUEST_VT_SAVE_ON))
          fprintf( fp, "Vt100     %d\n", 2);
        else
          fprintf( fp, "Vt100     %d\n", 0);
        fprintf( fp, "Reincarnation     %d\n", ch->pcdata->reincarnation);
        fprintf( fp, "Reincarn_allow     %d\n", ch->pcdata->allow_reincarnate);
        fprintf( fp, "Previous_Hours    %d\n", ch->pcdata->previous_hours);
        fprintf( fp, "Tactical     %d\n", ch->pcdata->tactical_mode);
        fprintf( fp, "TactIndex    %s~\n", ch->pcdata->tactical_index);
        fprintf( fp, "Compass     %d\n", (int)ch->pcdata->compass_width);
        fprintf( fp, "LastConnect     %d\n", ch->pcdata->last_connect);
        fprintf( fp, "CreationRoom %d\n", ch->pcdata->creation_room);
        fprintf( fp, "CreationZone %d\n", ch->pcdata->creator_zone);
        fprintf( fp, "P__vnum      %d\n", ch->pcdata->pvnum);
        fprintf( fp, "Vt100type   %d\n", ch->vt100_type );
        fprintf( fp, "Portbaud   %d\n", ch->pcdata->port_baud );
        fprintf( fp, "Portsize   %d\n", ch->pcdata->port_size );
        fprintf( fp, "Prompt_layout  %s~\n", ch->pcdata->prompt_layout);
        fprintf( fp, "Spam   %d\n", ch->pcdata->spam );
        fprintf( fp, "Clock    %d\n", ch->clock);
        fprintf( fp, "Hook     %d\n", ch->hook);
        fprintf( fp, "Recall   %d\n", ch->recall);
        fprintf( fp, "DeathRoom   %d\n", ch->pcdata->death_room);
        fprintf( fp, "Whichgod   %d\n", which_god(ch));
        fprintf( fp, "Ansi   %d\n", ch->ansi);
        fprintf( fp, "Army   %d\n", ch->pcdata->army_status);
        fprintf( fp, "Last_Real_Room   %d\n", ch->pcdata->last_real_room);
        fprintf( fp, "Rank   %d\n", (int)ch->rank);
	if (IS_IMMORTAL(ch))
	{
         if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
          fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
                                               ch->pcdata->r_range_hi   );
         if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
          fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
                                               ch->pcdata->o_range_hi   );
         if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
          fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
                                               ch->pcdata->m_range_hi   );
	}

        if( ch->pcdata->block_list != NULL)
          fprintf( fp, "Block_list    %s~\n", ch->pcdata->block_list);
        if( ch->pcdata->auto_command != NULL)
          fprintf( fp, "Auto_Command  %s~\n", ch->pcdata->auto_command);
        fprintf( fp, "Auto_Flags  %d\n", ch->pcdata->auto_flags);
        for(cnt=0;cnt<7;cnt++)
          fprintf( fp, "History %d %d\n", cnt, ch->pcdata->history[cnt]);

        fprintf( fp, "LastNote        %d\n",	ch->pcdata->last_note );
        for(cnt=0;cnt<MAX_TOPIC;cnt++)
          fprintf( fp, "Topic %d %d\n", cnt, ch->pcdata->topic_stamp[cnt]);

        for(cnt=0;cnt<MAX_COLOR;cnt++)
          fprintf( fp, "Color  %d %d\n", cnt, ch->pcdata->color[0][cnt]);
        for(cnt=0;cnt<MAX_COLOR;cnt++)
          fprintf( fp, "Colorb %d %d\n", cnt, ch->pcdata->color[1][cnt]);
        fprintf( fp, "Mswitched %d\n", ch->mclass_switched);
        fprintf( fp, "KillNum %d\n", ch->pcdata->killnum);
        for(cnt=0;cnt<MAX_KILL_TRACK;cnt++)
          if(ch->pcdata->killname[cnt][0]!='\0')
            fprintf( fp, "KillName %s~\n", ch->pcdata->killname[cnt]);
        for(cnt=0;cnt<MAX_ALIAS;cnt++)
          if(ch->pcdata->alias[cnt][0]!='\0')
            fprintf( fp, "Alias %s~%s~\n", ch->pcdata->alias_c[cnt],
               ch->pcdata->alias[cnt]);
        /* castle stuff */
        if(ch->pcdata->castle!=NULL)
          {
          fprintf( fp, "Castle_ent %d\n",ch->pcdata->castle->entrance);
          fprintf( fp, "Castle_dor %d\n",ch->pcdata->castle->door_room);
          fprintf( fp, "Castle_dir %d\n",ch->pcdata->castle->door_dir);
          fprintf( fp, "Castle_has %d\n",ch->pcdata->castle->has_backdoor);
          fprintf( fp, "Castle_cst %d\n",ch->pcdata->castle->cost);
          fprintf( fp, "Castle_nrm %d\n",ch->pcdata->castle->num_rooms);
          fprintf( fp, "Castle_nmo %d\n",ch->pcdata->castle->num_mobiles);
          fprintf( fp, "Castle_nob %d\n",ch->pcdata->castle->num_objects);
          fprintf( fp, "Castle_rgn %d\n",ch->pcdata->castle->reign_room);
          }
            

    for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
    {
	/* Thx Alander */
	if ( paf->type < 0 || paf->type >= MAX_SKILL )
	    continue;

	fprintf( fp, "AffectData   '%s' %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

  if( !IS_NPC(ch) )
    if( ch->pcdata->poison!=NULL)
      fwrite_poison_data( ch->pcdata->poison, fp );

  /* Let's save the Attack data     -   Chaos 4/20/99    */
  fprintf( fp, "PK_ATTACKS %d\n", MAX_PK_ATTACKS );
  for( cnt=0; cnt<MAX_PK_ATTACKS; cnt++)
    {
    fprintf( fp, "%d %d %s~\n", ch->pcdata->last_pk_attack_time[cnt],
         ch->pcdata->last_pk_attack_pvnum[cnt], 
         ch->pcdata->last_pk_attack_name[cnt] );
    }

  /* Let's save the area kill data     -   Chaos 4/20/99    */
#ifdef AREA_KILLS
  fprintf( fp, "AREA_KILLS %d\n", MAX_AREA );
  for( cnt=0; cnt<MAX_AREA; cnt++)
    {
    if( cnt%10==9)
      fprintf( fp, "%d\n", ch->pcdata->area_kills[cnt] );
    else
      fprintf( fp, "%d ", ch->pcdata->area_kills[cnt] );
    }
#endif
/*
    for ( sn = 0; sn < MAX_MOB_KILL_TRACK; sn++ )
    {
        if ( ch->pcdata->killed[sn].vnum == 0 )
          break;
        fprintf( fp, "Killed       %d %d\n",
                ch->pcdata->killed[sn].vnum,
                ch->pcdata->killed[sn].count );
    }  */

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its first_content.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    /* EXTRA_DESCR_DATA *ed; */
    AFFECT_DATA *paf;
    /* OBJ_DATA *copy; */
    int copies; 
    copies=1;     /* Too lazy to reverse logic */
   if(( (ch->level+5) < obj->level || obj->test_obj ||
         obj->item_type == ITEM_KEY ) && obj->wear_loc==WEAR_NONE)
    {
   fprintf(stderr, "1:%s NOT written to file\n", obj->name);
	fprintf(stderr, "level ch: %d obj: %d = %d\n", ch->level+5, obj->level, (ch->level+5) < obj->level);
	fprintf(stderr, "test_obj = %d\n", obj->test_obj);
	fprintf(stderr, "key  = %d\n", obj->item_type == ITEM_KEY);
	fprintf(stderr, "wear_loc = %d\n", obj->wear_loc== WEAR_NONE);
 	
    copies=1;     /* Too lazy to reverse logic */
    }
 else
    if( !obj->saved )
      {
      int cnt;

    copies=1;
     obj->saved=TRUE;

    fprintf( fp, "#OBJECT\n" );
    
    if( obj->basic && obj->first_content == NULL
         && !is_quest (obj->obj_quest) 
	 && obj->item_type != ITEM_CONTAINER  )
      {
      fprintf( fp, "BasicVnum      %u\n",	obj->pIndexData->vnum );
      fprintf( fp, "Nest         %d\n",	iNest			     );
      fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
      }
    else
      {
      /*  Old Method  9/27/94
      fprintf( fp, "Nest         %d\n",	iNest			     );
      fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      fprintf( fp, "Name         %s~\n",        obj->unforged_name   );
    else
      fprintf( fp, "Name         %s~\n",        obj->name            );
    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      fprintf( fp, "ShortDescr   %s~\n",	obj->unforged_short_descr);
    else
      fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    if( obj->description == obj->pIndexData->description ||
          !strcmp( obj->description,
         obj->pIndexData->description ) )
      fprintf( fp, "Description  NULL~\n");
    else
      fprintf( fp, "Description  %s~\n",obj->description);
    if( obj->long_descr == obj->pIndexData->long_descr ||
          !strcmp( obj->long_descr, obj->pIndexData->long_descr ) )
      fprintf( fp, "LongDescr NULL~\n");
    else
      fprintf( fp, "LongDescr    %s~\n",	obj->long_descr 	     );
    fprintf( fp, "Vnum         %u\n",	obj->pIndexData->vnum	     );
    SET_BIT(obj->extra_flags,ITEM_LEVEL_RENT);
    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      {
      REMOVE_BIT(obj->extra_flags,ITEM_FORGERY);
      fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags);
      SET_BIT(obj->extra_flags,ITEM_FORGERY);
      }
    else
      fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags	     );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Owner        %d\n",	obj->owned_by		   );
    fprintf( fp, "Values       %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );
    fprintf( fp, "Qst          %d\n",	obj->obj_quest );
    */

      fprintf( fp, "Quick2 \n");   /* New mode for non-Basic vnums  9/27/94 */
      fprintf( fp, "%d\n",	iNest			     );
      fprintf( fp, "%d\n",	obj->wear_loc		     );
    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      {
      if(obj->unforged_name!=NULL)
        fprintf( fp, "%s~\n",   obj->unforged_name           );
      else
        fprintf( fp, "%s~\n",   obj->name        );
      }
    else
      {
      if( obj->name == obj->pIndexData->name ||
          !strcmp( obj->name, obj->pIndexData->name ) )
        fprintf( fp, "NULL~\n");
      else
        fprintf( fp, "%s~\n",   obj->name                    );
      }

    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      {
      if(obj->unforged_short_descr!=NULL)
        fprintf( fp, "%s~\n",	obj->unforged_short_descr    );
      else
        fprintf( fp, "NULL~\n");
      }
    else
      {
      if( obj->short_descr == obj->pIndexData->short_descr||
          !strcmp( obj->short_descr, obj->pIndexData->short_descr ) )
        fprintf( fp, "NULL~\n");
      else
        fprintf( fp, "%s~\n",	obj->short_descr	     );
      }
    if( obj->description == obj->pIndexData->description ||
          !strcmp( obj->description,
                obj->pIndexData->description ) )
      fprintf( fp, "NULL~\n" );
    else
      fprintf( fp, "%s~\n",obj->description);
    if( obj->long_descr == obj->pIndexData->long_descr ||
          !strcmp( obj->long_descr, obj->pIndexData->long_descr ) )
      fprintf( fp, "NULL~\n");
    else
      fprintf( fp, "%s~\n",	obj->long_descr 	     );
    fprintf( fp, "%u\n",	obj->pIndexData->vnum	     );
    SET_BIT(obj->extra_flags,ITEM_LEVEL_RENT);
    if(IS_OBJ_STAT(obj, ITEM_FORGERY))
      {
      REMOVE_BIT(obj->extra_flags,ITEM_FORGERY);
      fprintf( fp, "%d\n",	obj->extra_flags);
      SET_BIT(obj->extra_flags,ITEM_FORGERY);
      }
    else
      fprintf( fp, "%d\n",	obj->extra_flags	     );
    fprintf( fp, "%d\n",	obj->wear_flags		     );
    fprintf( fp, "%d\n",	obj->item_type		     );
    fprintf( fp, "%d\n",	obj->weight		     );
    fprintf( fp, "%d\n",	obj->level		     );
    fprintf( fp, "%d\n",	obj->timer		     );
    fprintf( fp, "%d\n",	obj->cost		     );
    fprintf( fp, "%d\n",	obj->owned_by		   );
    fprintf( fp, "%d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );
    
    if( is_quest( obj->obj_quest ) )
      {
      fprintf(fp,"%d ",MAX_QUEST_BYTES);
      for(cnt=0; cnt<MAX_QUEST_BYTES; cnt++)
          fprintf(fp,"%d ",obj->obj_quest[cnt]);
      fprintf(fp,"\n");
      }
    else
      fprintf(fp,"0\n");
    }
           /* End of Quick1 mode */


    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
    {
	if ( paf->type < 0 || paf->type >= MAX_SKILL )
	    continue;

	    fprintf( fp, "AffectData   '%s' %d %d %d %d\n",
	    skill_table[paf->type].name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }
    if( obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_WEAPON )
      fprintf( fp, "Cond %d\n", obj->condition );
    fprintf( fp, "Indexrefa %d\nIndexrefb %d\n",
         obj->index_reference[0], obj->index_reference[1] );

    if( obj->poison!=NULL)
      fwrite_poison_data( obj->poison, fp );

    fprintf( fp, "End\n\n" );
  }
  else
   fprintf(stderr, "2:%s NOT written to file\n", obj->name);

     /* scan remainder of items for copies   - Removed for dup check
 if( obj->basic && obj->item_type != ITEM_CONTAINER &&
     !is_quest(obj->obj_quest) && obj->wear_loc == WEAR_NONE )
   for( copy=obj->next_content ; copy!=NULL; copy=copy->next_content)
     {
     if( copy->pIndexData == obj->pIndexData && copy->saved == FALSE &&
      copy->basic && copy->item_type != ITEM_CONTAINER &&
      !is_quest(copy->obj_quest) && copy->wear_loc == WEAR_NONE )
       {
       copy->saved=TRUE;
       copies++;
       }
    }
    if( copies > 1)
      fprintf( fp, "Copies  %d\n", copies); */

    if ( obj->first_content != NULL )
        {
	fwrite_obj( ch, obj->first_content, fp, iNest + 1 );
        }
    if ( obj->next_content != NULL )
	    fwrite_obj(ch,obj->next_content,fp,iNest);

    return;
}

void fwrite_corpse( CHAR_DATA *ch , FILE *fp)
   {
    OBJ_DATA *obj;
    bool found;


      /* Let's make sure there is a corpse there - Chaos 3/1/96  */
    found = FALSE;
    for( obj=first_object; obj!=NULL; obj=obj->next )
      if( obj == ch->pcdata->corpse && obj->item_type == ITEM_CORPSE_PC )
        {
        found = TRUE;
        break;
        }

    if( !found )
      {
      log_string( "Bug:  Corpse not found." );
      return;
      }
    
      /* Always write the object */
    ch->pcdata->corpse->test_obj = FALSE;

    fprintf( fp, "#CORPSE\n" );
    fprintf( fp, "%d\n\n\n", ch->pcdata->corpse_room);

	  fwrite_corpse_item( ch, ch->pcdata->corpse, fp, 0 );
    fprintf( fp, "#END\n" );
    return;
    }

void fwrite_corpse_item( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    AFFECT_DATA *paf;
    int cnt;

    if( obj->name == NULL )
      return;

    if( iNest!=0)
      fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    fprintf( fp, "LongDescr    %s~\n",	obj->long_descr 	     );
    fprintf( fp, "Vnum         %u\n",	obj->pIndexData->vnum	     );
    /* all items should be proper level now so make it so the level is
       permanent.  Also fixes so items don't suddenly become unwearable
     */
    SET_BIT(obj->extra_flags,ITEM_LEVEL_RENT);
    fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags	     );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Owner        %d\n",	obj->owned_by		   );
    fprintf( fp, "Values       %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );
    if( is_quest(obj->obj_quest ))
      {
      fprintf(fp,"Qstb %d ",MAX_QUEST_BYTES);
      for(cnt=0; cnt<MAX_QUEST_BYTES; cnt++)
        fprintf(fp,"%d ",obj->obj_quest[cnt]);
      fprintf(fp,"\n");
      }

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
    {
	if ( paf->type < 0 || paf->type >= MAX_SKILL )
	    continue;

	fprintf( fp, "AffectData   '%s' %d %d %d %d\n",
	    skill_table[paf->type].name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    if( obj->poison!=NULL)
      fwrite_poison_data( obj->poison, fp );

    if( obj->item_type==ITEM_ARMOR || obj->item_type==ITEM_WEAPON )
      fprintf( fp, "Cond %d\n", obj->condition );
    fprintf( fp, "Indexrefa %d\nIndexrefb %d\n",
         obj->index_reference[0], obj->index_reference[1] );

    fprintf( fp, "End\n\n" );

    if ( obj->first_content != NULL )
      {
      OBJ_DATA *cobj;
      for( cobj=obj->first_content; cobj!=NULL; cobj=cobj->next_content)
         if( !cobj->test_obj )
	      fwrite_corpse_item( ch, cobj, fp, iNest + 1 );
      }
    return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH], buf[200];
    CHAR_DATA *ch;
    FILE *fp, *fpg;
    bool found, foundgz, foundngz;
    sh_int cnt;
    int old_time;
    extern bool merc_down;

    if( name == NULL || *name =='\0' )
      return( FALSE );

    old_time = get_game_usec();
    CREATE(ch, CHAR_DATA, 1 );
    clear_char( ch );
    CREATE( ch->pcdata, PC_DATA, 1 );
    *ch->pcdata				= pcdata_zero; 
    d->character			= ch;
    ch->desc				= d;
    ch->editor				= NULL;
    ch->pcdata->demisedlevel			= 0;
    ch->name				= str_dup( name );
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->short_descr			= str_dup( "" );
    ch->long_descr			= str_dup( "" );
    ch->description			= str_dup( "" );
    ch->desc->old_host                  = str_dup( "" );
    ch->desc->old_domain                = str_dup( "" );
    ch->pcdata->mail_address            = str_dup( "" );
    ch->pcdata->html_address            = str_dup( "" );
    ch->pcdata->unsafe_password         = FALSE;
    ch->pcdata->reincarnation           = 0;
    ch->pcdata->previous_hours		= 0;
    ch->pcdata->allow_reincarnate       = 0;
    ch->pcdata->account                 = 0;
    ch->obj_with_prog 			= NULL;
    ch->pcdata->oprog_started 		= FALSE;
    ch->pcdata->dump  			= FALSE;
    ch->pcdata->death_timer 		= 0;
    ch->pcdata->travel 			= -1;
    ch->pcdata->travel_from 		= NULL;
    ch->pcdata->arrested                = FALSE;
    ch->pcdata->jailtime                = 0;
    ch->pcdata->jaildate                = current_time;
    ch->pcdata->give_prac_neg		= 0;
    ch->pcdata->give_prac_pos		= 0;
    ch->pcdata->just_died_ctr           = 0;
    ch->pcdata->time_of_death           = 0;
    ch->pcdata->r_range_lo          	= 0;
    ch->pcdata->r_range_hi          	= 0;
    ch->pcdata->m_range_lo          	= 0;
    ch->pcdata->m_range_hi          	= 0;
    ch->pcdata->o_range_lo          	= 0;
    ch->pcdata->o_range_hi          	= 0;

    ch->pcdata->tactical = NULL ;   /* allocated on vt100on time */
    ch->pcdata->compass_width = 6;
    ch->pcdata->page_buf = NULL;

    for( cnt=0; cnt<MAX_AREA; cnt++)
      ch->pcdata->area_kills[cnt]=0;

  if( d->descriptor != -999 )
   {
    ch->act				= PLR_COMBINE | PLR_PROMPT | PLR_AUTOEXIT | PLR_AUTOSAC;

    ch->speak                           = 0;
    ch->language                        = 0;
    ch->pcdata->last_connect            = 0;
    ch->race                            = 0;
    ch->rank                            = 0;

    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;

    ch->note_amount                     = 0;
    ch->pcdata->note_topic              = 0;
    ch->vt100                           = 0;
    ch->pcdata->last_time               = current_time;
    ch->pcdata->spam                    = 1024;
    ch->pcdata->tactical_mode           = 1004;
    *buf = UPPER(*name);
    *(buf+1)=LOWER(*(name+1));
    *(buf+2)='\0';
    ch->pcdata->tactical_index          = str_dup(buf);
    ch->pcdata->obj_version_number      = 0; /* always start with zero */
    ch->pcdata->creation_room           = 0;
    ch->pcdata->pvnum                   = 0;
    ch->pcdata->prompt_layout           = STRALLOC( "" );
    ch->pcdata->subprompt               = STRALLOC( "" );
    ch->pcdata->tracking           	= STRALLOC( "" );
    ch->desc->port_size                 = 10000;
    ch->pcdata->port_size               = 10000;
    ch->desc->port_baud                 = 20000;
    ch->pcdata->port_baud               = 20000;
    ch->pcdata->corpse                  = NULL;  /* Sets without corpse */
    ch->pcdata->corpse_room             = ROOM_VNUM_SCHOOL;
    ch->pcdata->channel                 = 0;
    ch->pcdata->clan_name               = STRALLOC( "" );
    ch->pcdata->clan_pledge             = STRALLOC( "" );
    ch->pcdata->clan			= NULL;
    ch->pcdata->clan_position		= 0;
    ch->pcdata->area 			= NULL;
    ch->which_god                       = -1;
    ch->vt100_type                      = 1124;
    ch->exp_lost                        = 0;
    ch->clock                           = 0;
    ch->pcdata->death_room              = ROOM_VNUM_SCHOOL;
    ch->recall                          = -1;
    ch->speed                           = 1;
    ch->hook                            = 4;
    ch->pcdata->scroll_buf[0]           = '\0';
    ch->pcdata->scroll_start            = 0;
    ch->pcdata->scroll_end              = 0;
    ch->pcdata->scroll_ip               = 0;
    ch->ansi                    = 0;
    ch->npcdata = NULL;
    ch->first_carrying = NULL;
    ch->pcdata->request=0;
    ch->pcdata->bet_amount=0;
    ch->pcdata->bet_mode=0;
    ch->pcdata->bet_victim=NULL;
    ch->critical_hit_by=0;
    /* ch->pcdata->beingsaved = NULL; */

    for(cnt=0;cnt<MAX_PACKETS;cnt++)
      ch->pcdata->packet[cnt]=0;

    for(cnt=0;cnt<MAX_AREA;cnt++)
      ch->pcdata->quest[cnt]=NULL;

    for(cnt=0;cnt<MAX_TOPIC;cnt++)
      ch->pcdata->topic_stamp[cnt]=0;

    ch->pcdata->last_note =0;

    for(cnt=0;cnt<MAX_COLOR;cnt++)
      {
      ch->pcdata->color[0][cnt]=0;
      ch->pcdata->color[1][cnt]=0;
      }
    
    ch->alias_ip                        = 0;

    for(cnt=0;cnt<MAX_CLASS;cnt++)
      ch->mclass[cnt]=0;
    ch->mclass_switched=0;

    for(cnt=0;cnt<MAX_PK_ATTACKS;cnt++)
      {
      ch->pcdata->last_pk_attack_time[cnt]=0;
      ch->pcdata->last_pk_attack_pvnum[cnt]=0;
      ch->pcdata->last_pk_attack_name[cnt]=str_dup("NULL");
      }

    for(cnt=0;cnt<7;cnt++)
      ch->pcdata->history[cnt]=0;

    ch->pcdata->auto_flags = 0;

  }
    /* the following is allocated for file reading purposes, gets deallocated
       if the player has no castle, need to load for finger too */
    CREATE(ch->pcdata->castle, CASTLE_DATA, 1);
    ch->pcdata->castle->entrance=0;
    ch->pcdata->castle->door_room=-1;
    ch->pcdata->castle->door_dir=-1;
    ch->pcdata->castle->has_backdoor=FALSE;
    ch->pcdata->castle->cost=0;
    ch->pcdata->castle->num_rooms=0;
    ch->pcdata->castle->num_mobiles=0;
    ch->pcdata->castle->num_objects=0;
    ch->pcdata->killnum=0;

    ch->pcdata->last_real_room = ROOM_VNUM_TEMPLE;

    for(cnt=0;cnt<MAX_ALIAS;cnt++)
     {
     ch->pcdata->alias[cnt]=STRALLOC("");
     ch->pcdata->alias_c[cnt]=STRALLOC( "" );
     }
    for(cnt=0;cnt<MAX_KILL_TRACK;cnt++)
     ch->pcdata->killname[cnt]=STRALLOC("");

    for(cnt=0;cnt<MAX_MOB_KILL_TRACK;cnt++)
     ch->pcdata->killed[cnt].vnum = 0;

    for(cnt=0;cnt<26;cnt++)
     ch->pcdata->back_buf[cnt]=str_dup("\r");
    ch->pcdata->auto_command = STRALLOC( "" );
    ch->pcdata->block_list = STRALLOC( "" );

    found = FALSE;
    strcpy( buf, name );
    buf[1]='\0';
    if( buf[0]>='A' && buf[0]<='Z')
      buf[0]+= ('a' - 'A');

    open_timer( TIMER_CHAR_LOAD );

      fclose( fpReserve );

    #if defined(unix)

   /* if(  ch->pcdata->player_fp == NULL ) */
    {
      /* check for normal save file */
    sprintf( strsave, "%s/%s/%s", PLAYER_DIR, buf, capitalize_name( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
       {
       foundngz=TRUE;
       }
    else
       foundngz=FALSE;
      /* find if gzipped */
    sprintf( strsave, "%s/%s/%s.gz", PLAYER_DIR, buf, capitalize_name(name));
    if ( !foundngz && ( fpg = fopen( strsave, "r" ) ) != NULL )
       {
       foundgz=TRUE;
       fclose(fpg);
       }
    else
       foundgz=FALSE;
   if( foundgz && !foundngz)
           /* decompress if .gz file exists and not normal */
      {
      char qbuf[200];
      fclose(fp);
      sprintf(qbuf,"gzip -dfq %s",strsave);
      system(qbuf);
      sprintf( strsave, "%s/%s/%s", PLAYER_DIR, buf, capitalize_name( name ) );
      fp = fopen( strsave, "r");
      }  
    #endif
 
    }
   /* else
    fp = ch->pcdata->player_fp; */

    if ( fp != NULL )
    {
	int iNest;


	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;
    
        if( d->descriptor != -999  )
          {
          sprintf( buf, "Loading character file: %s D%d", name, d->descriptor);
          log_string( buf );
          }

      if( !is_valid_file( ch, fp ) )
        {
        char buf[100], buf2[100], name_buf[100], buf3[100];

        strcpy( buf3, name );
        buf3[1]='\0';
        if( buf3[0]>='A' && buf3[0]<='Z')
          buf3[0]+= ('a' - 'A');

        strcpy( name_buf, capitalize_name( name ) );
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
        ch->desc = NULL;
        extract_char( ch, TRUE );
        if( load_error == TRUE )
          {
          merc_down = TRUE;
          IS_BOOTING=TRUE;
          return( FALSE );
          }
        remove( buf2 );
        rename( buf, buf2);
        sprintf( name_buf, "Erasing old char %s", name );
        log_string( name_buf );
        load_error = TRUE;
        return( load_char_obj( d, name ) );
        }

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	      {
              char buf[600];
              sprintf( buf, "Load_char_obj: # not found.  word was '%c%s'",
                     letter, fread_word( fp ));
	      log_string( buf );
		break;
	      }

	    word = fread_word( fp );
	    if ( !strcasecmp( word, "PLAYER" ) )
                 {
                 fread_char ( ch, fp );
                 if( IS_SET( ch->act, PLR_WIZTIME ) )
                   ch->act -= PLR_WIZTIME;
                 if( d->descriptor == -999 )
                   break;
                 if( strcasecmp( ch->name, name ) )
                   {
                   sprintf( buf, "Incorrect name. #END on %s", name );
                   log_string( buf );
                   break;
                   }
                 }
            else 
              if( !strcasecmp( word, "OBJECT" ) )
                fread_obj  ( ch, fp );
            else
              if( !strcasecmp( word, "CORPSE" ) ) 
                 {
                 if( ch->pcdata->obj_version_number == OBJECT_VERSION_NUMBER)
                   fread_corpse ( ch, fp );
                 }
	    else
              if ( !strcasecmp( word, "END"  ) )
                   break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}


      /* if they have no castle then deallocate the ->castle */
      if(ch->pcdata->castle!=NULL)
        {
        CASTLE_DATA *tcastle;
        tcastle = get_castle_data( ch );
        if( tcastle->num_rooms==0  || (ch->pcdata->castle->entrance==0 ) )
          {
          DISPOSE ( ch->pcdata->castle );
          ch->pcdata->castle=NULL;
          }
        }
                     /* remove all items and extract on faulty number */
      if( d->descriptor != -999 )
       if( ch->pcdata->obj_version_number != OBJECT_VERSION_NUMBER)
         {
         OBJ_DATA *obj, *obj_next;
         for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
           {
	   obj_next = obj->next_content;
           if( !IS_SET(obj->extra_flags, ITEM_INVENTORY) )
             {
             ch->gold+=(obj->pIndexData->cost/2);
                  if( obj->wear_loc != WEAR_NONE )
	            remove_obj(ch,obj->wear_loc,TRUE, FALSE);
	            extract_obj( obj );
             }
           }
         if( ch->gold > 1000000 * ch->level || ch->gold < 0 )
           ch->gold = 1000000 * ch->level;
         }
       if( fp != NULL )
         fclose( fp );  
    }

      fpReserve = fopen( NULL_FILE, "r");

    /* Fix up a few flags -    Chaos 10/1/95 */
  ch->trust = ch->level;
   if( d->descriptor == -999 )
     return found;

    ch->pcdata->last_saved = get_game_realtime();
   if( strlen( ch->pcdata->tactical_index ) == 1 )
     {
     buf[0]=ch->pcdata->tactical_index[0];
     buf[1]='-';
     buf[2]='\0';
     STRFREE (ch->pcdata->tactical_index );
     ch->pcdata->tactical_index = str_dup( buf );
     }

 if (IS_IMMORTAL( ch ) )
   assign_area( ch );

  /* Any gods out there? */
    if( ch->level >= 99  && REAL_GAME)
            if( ch->pcdata->pvnum != 166 &&    /* Chaos */
                ch->pcdata->pvnum != 107     /* Order */  )
              {
              SET_BIT( ch->act, PLR_DENY);
              }

  /* Deny players with screwed up levels  - Chaos  3/7/99  */
    {
    int cnt2;
    for(cnt=0,cnt2=0;cnt<MAX_CLASS;cnt++)
      cnt2+=ch->mclass[cnt];
    if( ch->level < 99 && ch->level != cnt2 )
      SET_BIT( ch->act, PLR_DENY);
    }

  if( IS_SET( ch->act, PLR_WIZINVIS ) && ch->level<MAX_LEVEL-2 )
    REMOVE_BIT( ch->act, PLR_WIZINVIS );

/* Check for illegal items, fix those that lost bits due to forgery bug
   Order 7/3/1995 */
   {
   OBJ_DATA *obj;
   for( obj=ch->first_carrying; obj!=NULL; obj=obj->next_content)
     {
     if(obj->wear_loc!=WEAR_NONE && obj->level>ch->level)
       {
       remove_obj(ch,obj->wear_loc,TRUE, FALSE);
       continue;
       }
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_GLOW)
                    && !IS_SET(obj->extra_flags,ITEM_GLOW))
       SET_BIT(obj->extra_flags,ITEM_GLOW);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_HUM)
                    && !IS_SET(obj->extra_flags,ITEM_HUM))
       SET_BIT(obj->extra_flags,ITEM_HUM);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_DARK)
                    && !IS_SET(obj->extra_flags,ITEM_DARK))
       SET_BIT(obj->extra_flags,ITEM_DARK);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_MAGIC)
                    && !IS_SET(obj->extra_flags,ITEM_MAGIC))
       SET_BIT(obj->extra_flags,ITEM_MAGIC);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_BLESS)
                    && !IS_SET(obj->extra_flags,ITEM_BLESS))
       SET_BIT(obj->extra_flags,ITEM_BLESS);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_ANTI_GOOD)
                    && !IS_SET(obj->extra_flags,ITEM_ANTI_GOOD))
       SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_ANTI_EVIL)
                    && !IS_SET(obj->extra_flags,ITEM_ANTI_EVIL))
       SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_ANTI_NEUTRAL)
                    && !IS_SET(obj->extra_flags,ITEM_ANTI_NEUTRAL))
       SET_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);
     if(IS_SET(obj->pIndexData->extra_flags,ITEM_INVENTORY)
                    && !IS_SET(obj->extra_flags,ITEM_INVENTORY))
       SET_BIT(obj->extra_flags,ITEM_INVENTORY);
     }
   }

/* Invalidate any extra item copies 
   if(ch->first_carrying!=NULL)
     invalidate_obj_copies(ch->first_carrying,0);
*/
   char_reset( ch );

   /*  Add missing languages   -  chaos 4/17/99  */
   {
   int lan;

   lan = (ch->level/5) + 3;
   if( lan >MAX_RACE )
     lan = MAX_RACE;
   lan = lan - total_language( ch );
    /* Woops - Too many known languages  -  Chaos  5/17/99 */
   if( lan < 0 )
     {
     ch->language = SHIFT(ch->race);
     ch->speak = ch->language;
     lan = (ch->level/5) + 3;
     if( lan >MAX_RACE )
       lan = MAX_RACE;
     lan = lan - total_language( ch );
     }
   for( ; lan>0 ; lan-- )
     add_language( ch );
   }


   if( ch->level >= 90 )
     knight_adjust_hpmnmv( ch );

/* Check for staying at the Inn  - Chaos 12/31/94  */
if(ch->in_room!=NULL && ch->in_room->sector_type == SECT_INN )
  {
  int hpgain;
  hpgain = ch->level * (current_time - ch->pcdata->last_time) / 3600;
  if( hpgain <0)
    hpgain = 0;
  if( ch->hit + hpgain > ch->max_hit )
    ch->hit = ch->max_hit;
  else
    ch->hit = ch->hit + hpgain;
  if( ch->mana + hpgain > ch->max_mana )
    ch->mana = ch->max_mana;
  else
    ch->mana = ch->mana + hpgain;
  if( ch->move + hpgain > ch->max_move )
    ch->move = ch->max_move;
  else
    ch->move = ch->move + hpgain;
  }

   close_timer( TIMER_CHAR_LOAD );

    /* Check for old character   -  Chaos 10/30/95 */
   if( ch->level > 0 || strcasecmp( ch->name, name ) )
      {

      if( character_expiration( ch ) < 0 || strcasecmp( ch->name, name ) )
        {
        char buf[100], buf2[100], name_buf[100], buf3[100];

        strcpy( buf3, name );
        buf3[1]='\0';
        if( buf3[0]>='A' && buf3[0]<='Z')
          buf3[0]+= ('a' - 'A');

        strcpy( name_buf, capitalize_name( name ) );
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
        ch->desc = NULL;
        extract_char( ch, TRUE );
        remove( buf2 );
        rename( buf, buf2);
        sprintf( name_buf, "Erasing old char %s", name );
        log_string( name_buf );
        return( load_char_obj( d, name ) );
        }
      }   


   return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcasecmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

#if defined(SKEY)
#undef SKEY
#endif
#define SKEY( literal, field, value )					\
				if ( !strcasecmp( word, literal ) )	\
				{					\
                                    STRFREE( field );                   \
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}



void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    char *line;
    bool fMatch;
    sh_int killcnt=0, cnt,tst;
    int x1, x2, x3, x4, x5;

      /* for item fixed.  here for speed */
    gsn_enchant = skill_lookup( "enchant" );

    for ( ; ; )
      {
      word   = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Account",	ch->pcdata->account,	fread_number( fp ) );
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "Ansi",	ch->ansi,	fread_number( fp ) );
	    KEY( "Army",	ch->pcdata->army_status,fread_number( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "Affected2By",	ch->affected2_by,	fread_number( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Arrested",    ch->pcdata->arrested,   fread_number( fp ) );
	    KEY( "Armor",	ch->armor,		fread_number( fp ) );
	    KEY( "Auto_Flags",	ch->pcdata->auto_flags,	fread_number( fp ) );
	    SKEY( "Auto_Command",ch->pcdata->auto_command,fread_string( fp ) );

	    if ( !strcasecmp( word, "Affect" ) || !strcasecmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;

		CREATE(paf, AFFECT_DATA, 1);
		if ( !strcasecmp( word, "Affect" ) )
		{
		    /* Obsolete 2.0 form. */
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			log_string( "Fread_char: unknown skill.");
		    else
			paf->type = sn;
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
                LINK(paf, ch->first_affect, ch->last_affect, next, prev );
		fMatch = TRUE;
		break;
	    }

	    if ( !strcasecmp( word, "AttrMod"  ) )
	    {
                line = fread_line( fp );
                x1=x2=x3=x4=x5=0;
                sscanf( line, "%d %d %d %d %d",
                      &x1, &x2, &x3, &x4, &x5 );
                ch->pcdata->mod_str = x1;
                ch->pcdata->mod_int = x2;
                ch->pcdata->mod_wis = x3;
                ch->pcdata->mod_dex = x4;
                ch->pcdata->mod_con = x5;
/*
		ch->pcdata->mod_str  = fread_number( fp );
		ch->pcdata->mod_int  = fread_number( fp );
		ch->pcdata->mod_wis  = fread_number( fp );
		ch->pcdata->mod_dex  = fread_number( fp );
		ch->pcdata->mod_con  = fread_number( fp );
*/
		fMatch = TRUE;
		break;
	    }

	    if ( !strcasecmp( word, "AttrPerm" ) )
	    {
                line = fread_line( fp );
                x1=x2=x3=x4=x5=13;
                sscanf( line, "%d %d %d %d %d",
                      &x1, &x2, &x3, &x4, &x5 );
                ch->pcdata->perm_str = x1;
                ch->pcdata->perm_int = x2;
                ch->pcdata->perm_wis = x3;
                ch->pcdata->perm_dex = x4;
                ch->pcdata->perm_con = x5;
/*
		ch->pcdata->perm_str = fread_number( fp );
		ch->pcdata->perm_int = fread_number( fp );
		ch->pcdata->perm_wis = fread_number( fp );
		ch->pcdata->perm_dex = fread_number( fp );
		ch->pcdata->perm_con = fread_number( fp );
*/
		fMatch = TRUE;
		break;
	    }

	    if ( !strcasecmp( word, "Actuals" ) )
	    {
		ch->actual_max_hit	= fread_number( fp );
		ch->actual_max_mana	= fread_number( fp );
		ch->actual_max_move	= fread_number( fp );
		fMatch = TRUE;
                break;
            }

            if ( !strcmp( word, "AREA_KILLS" ) )
              {
              tst = fread_number( fp );
              for(cnt=0;cnt<tst;cnt++)
                {
                if( cnt<MAX_AREA )
                  ch->pcdata->area_kills[cnt] = fread_number( fp );
                else
                  fread_number( fp );
                }
              fMatch = TRUE;
              break;
              }
            if ( !strcmp( word, "Alias" ) )
              {
              tst=0;
              for(cnt=0;cnt<MAX_ALIAS && tst==0;cnt++)
                if(ch->pcdata->alias[cnt][0]=='\0')
                  {
                  tst=1;
                  STRFREE (ch->pcdata->alias[cnt] );
                  STRFREE (ch->pcdata->alias_c[cnt] );
                  ch->pcdata->alias_c[cnt]=fread_string( fp ) ;
                  ch->pcdata->alias[cnt]=fread_string( fp ) ;
                  }
              if(tst==0)
                {
                char *ptx1;
                ptx1 = fread_string( fp );
                STRFREE (ptx1 );
                ptx1 = fread_string( fp );
                STRFREE (ptx1 );
                }
              fMatch = TRUE;
              break;
              }

	    break;

	case 'B':
	    SKEY( "Bamfin",	ch->pcdata->bamfin,	fread_string_nohash( fp ) );
	    SKEY( "Bamfout",	ch->pcdata->bamfout,	fread_string_nohash( fp ) );
	    SKEY( "Block_list", ch->pcdata->block_list, fread_string( fp ) );
	    break;

	case 'C':
	    KEY( "Channel",	ch->pcdata->channel,	fread_number( fp ) );
	    SKEY( "ClanName",	ch->pcdata->clan_name,	fread_string( fp ) );
	    SKEY( "ClanPledge",	ch->pcdata->clan_pledge,fread_string( fp ) );
	    KEY( "ClanPosition",ch->pcdata->clan_position,fread_number( fp ) );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Critical",	ch->critical_hit_by,    fread_number( fp ) );
            KEY( "Clock",       ch->clock,              fread_number( fp ) );
            KEY( "CreationRoom", ch->pcdata->creation_room,fread_number( fp ) );
            KEY( "Compass", ch->pcdata->compass_width,fread_number( fp ) );
            KEY( "CreationZone", ch->pcdata->creator_zone,fread_number( fp ) );
	    if ( !strcasecmp( word, "Condition" ) )
	    {
                 line = fread_line( fp );
                sscanf( line, "%d %d %d",
                      &x1, &x2, &x3 );
                ch->pcdata->condition[0] = x1;
                ch->pcdata->condition[1] = x2;
                ch->pcdata->condition[2] = x3;
/*
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
*/
		fMatch = TRUE;
		break;
	    }
            if( !strcasecmp( word, "Colorb") )
              {
              cnt = fread_number( fp );
              ch->pcdata->color[1][cnt] = fread_number( fp );
              fMatch=TRUE;
              break;
              }
            if( !strcasecmp( word, "Color") )
              {
              cnt = fread_number( fp );
              ch->pcdata->color[0][cnt] = fread_number( fp );
              fMatch=TRUE;
              break;
              }
            if(ch->pcdata->castle!=NULL)
              {
              KEY("Castle_ent",ch->pcdata->castle->entrance,fread_number(fp));
              KEY("Castle_dor",ch->pcdata->castle->door_room,fread_number(fp));
              KEY("Castle_dir",ch->pcdata->castle->door_dir,fread_number(fp));
              KEY("Castle_has",ch->pcdata->castle->has_backdoor,fread_number(fp));
              KEY("Castle_cst",ch->pcdata->castle->cost,fread_number(fp));
              KEY("Castle_nrm",ch->pcdata->castle->num_rooms,fread_number(fp));
              KEY("Castle_nmo",ch->pcdata->castle->num_mobiles,fread_number(fp));
              KEY("Castle_rgn",ch->pcdata->castle->reign_room,fread_number(fp));
              if( !strcasecmp( word, "Castle_nmb") )
                {/* reimburse for old mobiles */
                int nmob;
                nmob=fread_number(fp);
                ch->gold+=nmob*7000000;
                fMatch=TRUE;
                break;
                }
              KEY("Castle_nob",ch->pcdata->castle->num_objects,fread_number(fp));
              }
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "DemisedLevel",ch->pcdata->demisedlevel,fread_number( fp ) );
	    KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
	    KEY( "DeathRoom",   ch->pcdata->death_room,	fread_number( fp ) );
	    SKEY( "Description", ch->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "EQDamroll",	ch->pcdata->eqdamroll,	fread_number( fp ) );
	    KEY( "EQHitroll",	ch->pcdata->eqhitroll,	fread_number( fp ) );
	    KEY( "EQSaves",	ch->pcdata->eqsaves,	fread_number( fp ) );
	    if ( !strcasecmp( word, "End" ) )
                {     /*  ENDLOAD     Chaos  10/11/93*/
                if (!ch->short_descr)
                  ch->short_descr       = STRALLOC( "" );
                if (!ch->long_descr)
                  ch->long_descr        = STRALLOC( "" );
                if (!ch->description)
                  ch->description       = STRALLOC( "" );
                if (!ch->pcdata->pwd)
                  ch->pcdata->pwd       = str_dup( "" );
                if (!ch->pcdata->bamfin)
                  ch->pcdata->bamfin    = str_dup( "" );
                if (!ch->pcdata->bamfout)
                  ch->pcdata->bamfout   = str_dup( "" );
                if (!ch->pcdata->title)
                  ch->pcdata->title     = STRALLOC( "" );
                if (!ch->pcdata->prompt_layout )
                  ch->pcdata->prompt_layout    = STRALLOC( "" );
                if (!ch->pcdata->mail_address )
                  ch->pcdata->mail_address    = str_dup( "" );
                if (!ch->pcdata->html_address )
                  ch->pcdata->html_address    = str_dup( "" );
                if (!ch->pcdata->auto_command )
                  ch->pcdata->auto_command      = STRALLOC( "" );
	        if (ch->pcdata->clan_name && ch->pcdata->clan_name[0] !='\0'
  		    && get_clan(ch->pcdata->clan_name) != NULL)
	          ch->pcdata->clan = get_clan(ch->pcdata->clan_name);
		else
		  ch->pcdata->clan = NULL;
		
                for(cnt=0;cnt<MAX_ALIAS;cnt++)
                {
                  if (!ch->pcdata->alias[cnt])
                     ch->pcdata->alias[cnt]= STRALLOC( "");
                  if (!ch->pcdata->alias_c[cnt])
                     ch->pcdata->alias_c[cnt]= STRALLOC( "");
                }

                if(ch->recall <= 2)
                  ch->recall = ROOM_VNUM_TEMPLE;
                if(ch->pcdata->death_room <= 2)
                  ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
                tst=0;
                for(cnt=0;cnt<MAX_CLASS;cnt++)
                  if(ch->mclass[cnt]!=0)
                    tst=1;
                if(tst==0)
                  ch->mclass[ch->class]=ch->level;
		      /* Force all ports to 10K  - Chaos 4/22/98 */
                /* if( ch->pcdata->port_size < 16 )
                   ch->pcdata->port_size = 16;
                if( ch->pcdata->port_size > 10000 ) */
                   ch->pcdata->port_size = 10000;
                ch->desc->port_size=ch->pcdata->port_size;
                ch->desc->port_baud=ch->pcdata->port_baud;
                ch->desc->port_timer=0;
                ch->editor=NULL;
                ch->position=POS_STANDING;
                if( *ch->name >= 'a' && *ch->name <= 'z' )
                  *ch->name -= ('a' - 'A');
                if(ch->hit<0)
                  ch->hit=1;
    
		return;
                }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    KEY( "Explost",		ch->exp_lost,		fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
	    KEY( "GivePracNeg",	ch->pcdata->give_prac_neg,fread_number( fp ) );
	    KEY( "GivePracPos",	ch->pcdata->give_prac_pos,fread_number( fp ) );
	    break;

	case 'H':
	    SKEY( "Htmladdress",	ch->pcdata->html_address, fread_string_nohash( fp ) );
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hook",	  ch->hook,	    	fread_number( fp ) );
            if( !strcasecmp( word, "History") )
              {
              cnt = fread_number( fp );
              ch->pcdata->history[cnt] = fread_number( fp );
              fMatch=TRUE;
              break;
              }

	    if ( !strcasecmp( word, "HpManaMove" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
                /* Fix wierd hp/mana/move */
                  if( ch->max_hit<10 )
                    {
                    char tbuf[100];
                    int new_hit;
                    new_hit =  10 + ch->level * 10;
                    sprintf(tbuf,"adjust hp %d to %d",ch->max_hit,new_hit);
                    log_string( tbuf );
                    ch->max_hit=new_hit;
                    ch->hit = new_hit;
                    }
                  if( ch->max_mana < 10 )
                    {
                    char tbuf[100];
                    sprintf( tbuf, "adjust mn %d to %d", ch->max_mana, 100);
                    log_string( tbuf );
                    ch->max_mana=100;
                    ch->mana=100;
                    }
                  if( ch->max_move < 10 )
                    {
                    char tbuf[100];
                    int new_move;
                    new_move = 100 + ch->level * 4;
                    sprintf( tbuf, "adjust mv %d to %d", ch->move, new_move );
                    log_string( tbuf );
                    ch->max_move=new_move;
                    ch->move=new_move;
                    }
		break;
	    }
	    break;
        case 'J':
	    KEY( "Jaildate",   ch->pcdata->jaildate,   fread_number( fp ) );
	    KEY( "Jailtime",   ch->pcdata->jailtime,   fread_number( fp ) );
            break;
        case 'K':

	    KEY( "KLR_Played",	ch->killer_played,	fread_number( fp ) );
	    KEY( "KillNum",	ch->pcdata->killnum,	fread_number( fp ) );
            if ( !strcmp( word, "KillName" ) )
              {
              tst=0;
              for(cnt=0;cnt<MAX_KILL_TRACK && tst==0;cnt++)
                if(ch->pcdata->killname[cnt][0]=='\0')
                  {
                  tst=1;
                  STRFREE (ch->pcdata->killname[cnt] );
                  ch->pcdata->killname[cnt]=fread_string( fp ) ;
                  }
              if(tst==0)
                {
                char *ptx1;
                ptx1 = fread_string( fp );
                STRFREE (ptx1 );
                }
              fMatch = TRUE;
              break;
              }
            if ( !strcmp( word, "Killed" ) )
            {
                fMatch = TRUE;
                if ( killcnt >= MAX_MOB_KILL_TRACK )
                  bug( "fread_char: killcnt (%d) >= MOB_KILL_TRACK", killcnt );
                else
                {
                    ch->pcdata->killed[killcnt].vnum    = fread_number( fp );
                    ch->pcdata->killed[killcnt++].count = fread_number( fp );
                }
	     break;
            }
            break;
	case 'L':
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    SKEY("LongDescr",	ch->long_descr,		fread_string( fp ) );
            KEY( "Language",    ch->language,           fread_number( fp ) );
            KEY( "LastNote",    ch->pcdata->last_note,  fread_number( fp ) );
            SKEY("Lastlogin",   ch->desc->old_host,     fread_string_nohash( fp ) );
            SKEY("LastDomain",  ch->desc->old_domain,     fread_string_nohash( fp ) );
            KEY( "LastTime",  ch->pcdata->last_time,   fread_number( fp ));
            KEY( "LastConnect",  ch->pcdata->last_connect, fread_number( fp ));
            KEY("Last_Real_Room",ch->pcdata->last_real_room,fread_number( fp ));
	    break;

        case 'M':
            if( !strcasecmp( word, "Mclass") )
              {
              cnt = fread_number( fp );
              ch->mclass[cnt] = fread_number( fp );
              fMatch=TRUE;
              break;
              }
            if ( !strcmp( word, "MobRange" ) )
            {
                ch->pcdata->m_range_lo = fread_number( fp );
                ch->pcdata->m_range_hi = fread_number( fp );
                fMatch = TRUE;
            }

            KEY( "Mswitched",   ch->mclass_switched,    fread_number( fp ) );
            SKEY( "Mailaddress", ch->pcdata->mail_address, fread_string_nohash( fp ) );
            break;

	case 'N':
            KEY( "Notes",  ch->note_amount,    fread_number( fp) );
	    if ( !strcasecmp( word, "Name" ) )
	    {
		/*
		 * Name already set externally.
		 */

		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }

	    break;
        case 'O':
            KEY( "Obj_Ver_Num",ch->pcdata->obj_version_number,fread_number(fp));
	    KEY( "OutCastPlay",	ch->outcast_played,	fread_number( fp ) );
            if ( !strcmp( word, "ObjRange" ) )
            {
                ch->pcdata->o_range_lo = fread_number( fp );
                ch->pcdata->o_range_hi = fread_number( fp );
                fMatch = TRUE;
            }
            break;

	case 'P':
	    SKEY( "Password",	ch->pcdata->pwd,	fread_string_nohash( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Player2_Bits",ch->pcdata->player2_bits,fread_number( fp ) );
	    KEY( "Previous_Hours",ch->pcdata->previous_hours,fread_number(fp));
	    KEY( "Portsize",	ch->pcdata->port_size,		fread_number( fp ) );
	    SKEY( "Prompt_Layout",ch->pcdata->prompt_layout,fread_string(fp));
	    KEY( "Portbaud",	ch->pcdata->port_baud,		fread_number( fp ) );
	    KEY( "P__vnum",	ch->pcdata->pvnum,		fread_number( fp ) );
            /* useless lines to keep old character files readable */
	    KEY( "P_vnum",	cnt,		fread_number( fp ) );
	    KEY( "Pvnum",	cnt,		fread_number( fp ) );
            if( !strcasecmp( word, "PK_ATTACKS" ) )
              {
              int cnt, cnt2;
              cnt = fread_number(fp);
              if( cnt > MAX_PK_ATTACKS )
                cnt = MAX_PK_ATTACKS;
              for(cnt2=0; cnt2<cnt; cnt2++)
                {
                ch->pcdata->last_pk_attack_time[cnt2]=fread_number(fp);
                ch->pcdata->last_pk_attack_pvnum[cnt2]=fread_number(fp);
                STRFREE( ch->pcdata->last_pk_attack_name[cnt2] );
                ch->pcdata->last_pk_attack_name[cnt2]=fread_string(fp);
                }
              fMatch = TRUE;
              break;
              }
            if( !strcasecmp( word, "POISON_DATA" ) )
              {
              POISON_DATA *pd;
              pd = fread_poison_data( fp );
              if( pd!=NULL && ch->pcdata->poison != NULL )
                {
                pd->next = ch->pcdata->poison;
                ch->pcdata->poison = pd;
                }
              else
               if( pd!=NULL )
                ch->pcdata->poison = pd;
              fMatch = TRUE;
              break;
              }
            /* end of useless lines */
	    break;

  case 'Q':
	  if ( !strcasecmp( word, "Qst" ) )
      {
      int qn;
      qn=fread_number( fp );
      set_quest_bits( &ch->pcdata->quest[qn], 0, 32, fread_number( fp ));
	    fMatch = TRUE;
      }
	  if ( !strcasecmp( word, "Qstb" ) )
      {
      int qn, byt, cnt;
      byt=fread_number( fp );
      qn=fread_number( fp );
      CREATE(ch->pcdata->quest[qn],unsigned char, MAX_QUEST_BYTES);
      for( cnt=0; cnt<byt; cnt++)
        ch->pcdata->quest[qn][cnt]=fread_number( fp );
      fMatch = TRUE;
      }
    break;

	case 'R':
	    KEY( "Race",        ch->race,		fread_number( fp ) );
	    KEY( "Rank",        ch->rank,		fread_number( fp ) );
	    KEY( "Reincarnation", ch->pcdata->reincarnation,fread_number( fp ) );
	    KEY( "Reincarn_allow", ch->pcdata->allow_reincarnate,
                            fread_number( fp ) );
            KEY( "Recall",      ch->recall,             fread_number( fp ) );
            if ( !strcmp( word, "RoomRange" ) )
            {
                ch->pcdata->r_range_lo = fread_number( fp );
                ch->pcdata->r_range_hi = fread_number( fp );
                fMatch = TRUE;
            }

	    if ( !strcasecmp( word, "Room" ) )
              {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );
              fMatch = TRUE;
	      break;
	      }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    SKEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
            KEY( "Speed",       ch->speed,              fread_number( fp ) );
            KEY( "Speak",       ch->speak,              fread_number( fp ) );
            KEY( "Spam",        ch->pcdata->spam,       fread_number( fp ) );

	    if ( !strcasecmp( word, "Skill" ) )
	    {
		int sn, cnt;
		int value;

		value = fread_number( fp );
                if(value<0)
                  value=0;
                if( value>99)
                  value=99;
		sn    = skill_lookup( fread_word( fp ) );
		if ( sn < 0 )
		    log_string( "Fread_char: unknown skill.");
		else
                    {
                    cnt = multi_pick(ch, sn);
                    if( cnt>=0 && value > class_table[cnt].skill_adept )
                      value = class_table[cnt].skill_adept;
		    ch->pcdata->learned[sn] = value;
                    }
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Trust",ch->trust,		fread_number( fp ) );
	    KEY( "Tactical",ch->pcdata->tactical_mode, fread_number( fp ) );
	    SKEY( "TactIndex",ch->pcdata->tactical_index, fread_string_nohash(fp ) );

            if( !strcasecmp( word, "Topic") )
              {
              cnt = fread_number( fp );
              ch->pcdata->topic_stamp[cnt] = fread_number( fp );
              fMatch=TRUE;
              break;
              }
	    if ( !strcasecmp( word, "Title" ) )
	    {
                STRFREE (ch->pcdata->title );
		ch->pcdata->title = fread_string( fp );
		if ( isalpha((int)ch->pcdata->title[0])
		||   isdigit((int)ch->pcdata->title[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    STRFREE (ch->pcdata->title );
		    ch->pcdata->title = STRALLOC( buf );
		}
               if( strlen( ch->pcdata->title ) > 60 )
		{
		    strcpy( buf, ch->pcdata->title );
                    buf[ 60 ] = '\0' ;
		    STRFREE (ch->pcdata->title );
		    ch->pcdata->title = STRALLOC( buf );
		}

		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
            KEY( "Vt100",       ch->vt100,          fread_number( fp ) );
            KEY( "Vt100type",   ch->vt100_type,     fread_number( fp ) );
	    if ( !strcasecmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
      KEY( "Whichgod", ch->which_god, fread_number( fp) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    break;
	}

      if ( !fMatch )
        {
        fread_to_eol( fp );
        log_string( "Fread_char: no match.");
        log_string( word);
        }
      }
}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj, *cobj;
    char *pt;
    char *word;
    int iNest, cnt;
    int copies, BasicVnum, WearLoc;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool item_not_valid=FALSE;
 
    copies=1;
    BasicVnum=0;
    WearLoc=WEAR_NONE;
    CREATE(obj, OBJ_DATA, 1);
    *obj		= obj_zero; 
    obj->name		= STRALLOC( "NULL" );
    obj->short_descr	= STRALLOC( "NULL" );
    obj->long_descr	= STRALLOC( "NULL" );
    obj->description	= STRALLOC( "NULL" );
    /* obj->carried_by     = ch; */
    obj->pIndexData=obj_index[OBJ_VNUM_MUSHROOM];
    obj->test_obj = FALSE;
    obj->condition = 70+number_range(0,29);
    obj->index_reference[0] = 0;
    obj->index_reference[1] = 0;

    fNest		= FALSE;
    fVnum		= FALSE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !strcasecmp( word, "Affect" ) || !strcasecmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;
		int sn;
		CREATE(paf, AFFECT_DATA, 1);
                sn = -1;

		if ( !strcasecmp( word, "Affect" ) )
		{
		    /* Obsolete 2.0 form. */
		    paf->type	= fread_number( fp );
		}
		else
		{

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			log_string( "Fread_obj: unknown skill.");
		    else
			paf->type = sn;

  /* Wierd item bug check  -  Chaos  2/5/97 */
#ifdef undef
        if( skill_table[sn].target == TAR_CHAR_OFFENSIVE )
                  SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
#endif
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
  /* Duplicate affect on item bug check  -  Chaos  2/5/97 */
#ifdef undef
    if( sn != -1 )
      {
      if( sn != gsn_enchant )
        {
        if( count_obj_affected( obj, sn, paf->location ) > 1 )
          SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
        }
      else
        {
        if( count_obj_affected( obj, sn, paf->location ) > 2 )
          SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
        }
      }
#endif
		LINK(paf, obj->first_affect, obj->last_affect, next, prev);
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "BasicVnum",	BasicVnum,		fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    KEY( "Copies",	copies,		fread_number( fp ) );
	    break;

	case 'D':
            SKEY( "Description", obj->description, fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );

	    if ( !strcasecmp( word, "ExtraDescr" ) )
	    {
	      EXTRA_DESCR_DATA *ed;

 	      CREATE(ed, EXTRA_DESCR_DATA, 1);
	      ed->keyword		= fread_string( fp );
              pt = fread_string( fp );
              ed->description=UPPER_ALLOC( pt );
              STRFREE (pt );
              LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );

	      fMatch = TRUE;
	    }

	    if ( !strcasecmp( word, "End" ) )
	      {
              bool bad_enchant;
	      EXTRA_DESCR_DATA *ed;
              AFFECT_DATA *paf;
              
	if ( !fNest || (!fVnum && BasicVnum==0) )
	{
	  log_string( "Fread_obj: incomplete object.");
	  STRFREE ( obj->name        );
	  STRFREE ( obj->description );
	  STRFREE ( obj->short_descr );
	  STRFREE ( obj->long_descr );
          while ( (ed=obj->first_extradesc) != NULL )
          {
           STRFREE( ed->keyword );
           STRFREE( ed->description );
           UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
           DISPOSE( ed );
          }
          while ( (paf=obj->first_affect) != NULL )
           {
            UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            DISPOSE( paf );
           }
	  DISPOSE( obj );
	  return;
	}

         /* Look for extra enchants */
       bad_enchant = FALSE;
       switch( obj->item_type )
         {
         case ITEM_WEAPON:
           if( count_obj_affected( obj, gsn_enchant, APPLY_AC ) > 0 )
             {
             bad_enchant = TRUE;
             break;
             }
           if( count_obj_affected( obj, gsn_enchant, APPLY_DAMROLL ) > 1 )
             {
             bad_enchant = TRUE;
             break;
             }
           if( count_obj_affected( obj, gsn_enchant, APPLY_HITROLL ) > 2 )
             {
             bad_enchant = TRUE;
             break;
             }
           break;
        case ITEM_ARMOR:
           if( count_obj_affected( obj, gsn_enchant, APPLY_AC ) > 1 )
             {
             bad_enchant = TRUE;
             break;
             }
           if( count_obj_affected( obj, gsn_enchant, APPLY_DAMROLL ) > 0 )
             {
             bad_enchant = TRUE;
             break;
             }
           if( count_obj_affected( obj, gsn_enchant, APPLY_HITROLL ) > 0 )
             {
             bad_enchant = TRUE;
             break;
             }
           break;

         default:
           if( count_obj_affected( obj, gsn_enchant, -1 ) > 0 )
             {
             bad_enchant = TRUE;
             break;
             }
           break;
         }
	if( bad_enchant )
  	{
	 log_string( "Fread_obj: extra enchants on object.");
	 if( obj->wear_loc != WEAR_NONE )
	   remove_obj(ch,obj->wear_loc,TRUE, FALSE);
	 STRFREE (obj->name        );
	 STRFREE (obj->description );
	 STRFREE (obj->short_descr );
	 STRFREE (obj->long_descr );
	 while ( (paf=obj->first_affect) != NULL )
	 {
	  UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
	  DISPOSE( paf );
	 }
	 DISPOSE( obj );
	 return;
	}

	if( BasicVnum==0)
	{
	 LINK( obj, first_object, last_object, next, prev );
	 total_objects++;
  	 if(  obj->first_affect == NULL  &&
      	  obj->value[0]==obj->pIndexData->value[0] &&
          obj->value[1]==obj->pIndexData->value[1] &&
          obj->value[2]==obj->pIndexData->value[2] &&
          obj->value[3]==obj->pIndexData->value[3] &&
          obj->owned_by<1 && !is_quest(obj->obj_quest) &&
          !strcasecmp(obj->short_descr, obj->pIndexData->short_descr) &&
          !strcasecmp(obj->description, obj->pIndexData->description) &&
          !strcasecmp(obj->name, obj->pIndexData->name) ) 
          {
           obj->basic = TRUE;
          }
          else
          {
           obj->basic = FALSE;
          }
         }
         else             /* a BasicVnum object */
	 {
          OBJ_INDEX_DATA *index;
          int cond, ref0, ref1;
          ref0 = obj->index_reference[0];
          ref1 = obj->index_reference[1];
          cond = obj->condition;
	  STRFREE (obj->name        );
	  STRFREE (obj->description );
	  STRFREE (obj->short_descr );
	  STRFREE (obj->long_descr );
          DISPOSE ( obj );
  	  while ( ( index = get_obj_index( BasicVnum ) ) == NULL )
          {
           char nBuf[81];
           if( !TEST_GAME )
           {
	    sprintf(nBuf, "Fread_obj: bad vnum %u.", BasicVnum );
            log_string(nBuf);
           }
           BasicVnum=OBJ_VNUM_MUSHROOM;
          }
          obj = create_object( index, index->level);
          obj->condition=cond;
          obj->index_reference[0] = ref0;
          obj->index_reference[1] = ref1;
          obj->test_obj = FALSE;
         }

         obj->weight = abs( obj->weight );
         obj->cost = abs( obj->cost );

         /* Special chains of the Gods */
         if( obj->pIndexData->vnum == 51 || obj->pIndexData->vnum == 50 )
         {
          obj->value[0] = (ch->level/4) -5;
          obj->level = 10;
          obj->wear_loc = WearLoc = WEAR_HEART;
         }
         if( obj->pIndexData->vnum == 53 )
         {
          obj->value[0] = (ch->level/3) -5;
          obj->level = 10;
         }

         obj->test_obj = FALSE;

       if(!IS_SET(obj->extra_flags,ITEM_LEVEL_RENT) || ( obj->level<=0) ||
         ( obj->level < obj->pIndexData->level - obj->level/5 - 1 ) )
          obj->level=obj->pIndexData->level;

       if( !strcmp( obj->description , "NULL" ) )
       {
        STRFREE( obj->description );
        obj->description = QUICKLINK(obj->pIndexData->description);
       }
       if( !strcmp( obj->name, "NULL" ) )
       {
        STRFREE( obj->name );
        obj->name= QUICKLINK(obj->pIndexData->name);
       }
       if( !strcmp( obj->short_descr, "NULL" ) )
       {
        STRFREE( obj->short_descr );
        obj->short_descr= QUICKLINK(obj->pIndexData->short_descr);
       }
       if( !strcmp( obj->long_descr, "NULL" ) )
       {
        STRFREE( obj->long_descr );
        obj->long_descr= QUICKLINK(obj->pIndexData->long_descr);
       }

	obj->pIndexData->count++; 
	obj->pIndexData->total_objects++; /* for resets */
        obj->wear_loc = WearLoc;
	if ( iNest == 0 || rgObjNest[iNest] == NULL )
        {
	 obj_to_char( obj, ch );
         obj->wear_loc = WearLoc;
        }
	else
	 obj_to_obj( obj, rgObjNest[iNest-1] );

        if( copies>1 )
         for( cnt=1; cnt<copies; cnt++)
         {
          cobj = create_object( obj->pIndexData , obj->level );
          cobj->value[0]= obj->value[0];
          cobj->value[1]= obj->value[1];
          cobj->value[2]= obj->value[2];
          cobj->value[3]= obj->value[3];
          cobj->test_obj = FALSE;
          if ( iNest == 0 || rgObjNest[iNest] == NULL )
          {
           obj_to_char( cobj, ch);
           cobj->wear_loc = WEAR_NONE;
          }
          else
           obj_to_obj( cobj , rgObjNest[iNest-1] );
         }
#ifdef undef
                if(item_not_valid)
                  SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
#endif

         add_to_object_reference_hash( obj );

	 return;
	}
	break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Indexrefa",	obj->index_reference[0],fread_number( fp ) );
	    KEY( "Indexrefb",	obj->index_reference[1],fread_number( fp ) );
	    break;

	case 'L':
	    if ( !strcasecmp( word, "LongDescr" ) )
              {
              pt = fread_string( fp );
              STRFREE(obj->long_descr);
              obj->long_descr = UPPER_ALLOC(pt);
              STRFREE (pt );
              fMatch=TRUE;
              break;
              }
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    SKEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !strcasecmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
                  char nBuf[81];
                  sprintf(nBuf, "Fread_obj: bad nest %d.", iNest );
                  log_string(nBuf);
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;
	case 'O':
	    KEY( "Owner",	obj->owned_by,		fread_number( fp ) );
	    break;
        case 'P':
            if( !strcasecmp( word, "POISON_DATA" ) )
              {
              POISON_DATA *pd;
              pd = fread_poison_data( fp );
              if( pd!=NULL && obj->poison != NULL )
                {
                pd->next = obj->poison;
                obj->poison = pd;
                }
              else
               if( pd!=NULL )
                obj->poison = pd;
              fMatch = TRUE;
              break;
              }
            break;
  case 'Q':
      if( !strcasecmp( word, "Quick1"))  
        /* do not remove this section for backward compatibility */
        {
	      fread_to_eol( fp ); 
              iNest = fread_number( fp );
	      rgObjNest[iNest] = obj;
	      fNest = TRUE;
              WearLoc = fread_number( fp );
              STRFREE( obj->name );
              STRFREE( obj->short_descr );
              STRFREE( obj->long_descr );
              obj->name = fread_string( fp );
              obj->short_descr = fread_string( fp );
              pt = fread_string( fp );
              STRFREE (obj->description );
              obj->description=UPPER_ALLOC( pt );
              STRFREE (pt );

              obj->long_descr = fread_string( fp );

	    {
		int vnum;

		vnum = fread_number( fp );
		while ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                    {
                    char nBuf[81];
                    if( !TEST_GAME )
                      {
		      sprintf(nBuf, "Fread_obj: bad vnum %u.", vnum );
                      log_string(nBuf);
                      }
                    vnum=OBJ_VNUM_MUSHROOM;
                    item_not_valid=TRUE;
                    }
		fVnum = TRUE;
	    }
        obj->extra_flags = fread_number( fp );
        obj->wear_flags = fread_number( fp );
        obj->item_type = fread_number( fp );
        obj->weight = fread_number( fp );
        obj->level = fread_number( fp );
        obj->timer = fread_number( fp );
        obj->cost = fread_number( fp );
        obj->owned_by = fread_number( fp );
        obj->value[0] = fread_number( fp );
        obj->value[1] = fread_number( fp );
        obj->value[2] = fread_number( fp );
        obj->value[3] = fread_number( fp );
        set_quest_bits( &obj->obj_quest, 0, 32, fread_number(fp));
	fMatch = TRUE;
        break;
        }
      if( !strcasecmp( word, "Quick2"))  
        /* do not remove this section for backward compatibility */
        {
        int byt, tot, cnt;
	      fread_to_eol( fp ); 
              iNest = fread_number( fp );
	      rgObjNest[iNest] = obj;
	      fNest = TRUE;
              WearLoc = fread_number( fp );
              STRFREE( obj->name );
              STRFREE( obj->short_descr );
              STRFREE( obj->long_descr );
              obj->name = fread_string( fp );
              obj->short_descr = fread_string( fp );
              pt = fread_string( fp );
              STRFREE (obj->description );
              obj->description=UPPER_ALLOC( pt );
              STRFREE (pt );

              obj->long_descr = fread_string( fp );

	    {
		int vnum;

		vnum = fread_number( fp );
		while ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                    {
                    char nBuf[81];
                    if( !TEST_GAME )
                      {
		      sprintf(nBuf, "Fread_obj: bad vnum %u.", vnum );
                      log_string(nBuf);
                      }
                    vnum=OBJ_VNUM_MUSHROOM;
                    item_not_valid=TRUE;
                    }
		fVnum = TRUE;
	    }
        obj->extra_flags = fread_number( fp );
        obj->wear_flags = fread_number( fp );
        obj->item_type = fread_number( fp );
        obj->weight = fread_number( fp );
        obj->level = fread_number( fp );
        obj->timer = fread_number( fp );
        obj->cost = fread_number( fp );
        obj->owned_by = fread_number( fp );
        obj->value[0] = fread_number( fp );
        obj->value[1] = fread_number( fp );
        obj->value[2] = fread_number( fp );
        obj->value[3] = fread_number( fp );
        byt = fread_number( fp );
        CREATE( obj->obj_quest, unsigned char, MAX_QUEST_BYTES );
        for( cnt=0; cnt<byt; cnt++)
          obj->obj_quest[cnt] = fread_number( fp );
        for( tot=0,cnt=0; cnt<byt; cnt++)
          tot+=obj->obj_quest[cnt];
        if( tot==0)
          {
          DISPOSE( obj->obj_quest );
          obj->obj_quest=NULL;
          }
	fMatch = TRUE;
        break;
        }

	  if ( !strcasecmp( word, "Qst" ) )
      {
      set_quest_bits( &obj->obj_quest, 0, 32, fread_number(fp ) );
	    fMatch = TRUE;
        break;
      }
	  if ( !strcasecmp( word, "Qstb" ) )
      {
      int byt, cnt;
        byt = fread_number( fp );
        CREATE(obj->obj_quest,unsigned char, MAX_QUEST_BYTES);
        for( cnt=0; cnt<byt; cnt++)
          obj->obj_quest[cnt] = fread_number( fp );
	    fMatch = TRUE;
        break;
      }
    break;

  
	case 'S':
	    SKEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !strcasecmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
                  char nBuf[81];
		  sprintf(nBuf, "Fread_obj: bad iValue %d.", iValue );
                  log_string(nBuf);
		}
		else if ( sn < 0 )
		{
		    log_string( "Fread_obj: unknown skill.");
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !strcasecmp( word, "Values" ) )
	    {
                int x1,x2,x3,x4;
                char *ln = fread_line( fp );

                x1=x2=x3=x4=0;
                sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

                obj->value[0]   = x1;
                obj->value[1]   = x2;
                obj->value[2]   = x3;
                obj->value[3]   = x4;
/*
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
*/
		fMatch		= TRUE;
		break;
	    }

	    if ( !strcasecmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		while ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                    {
                    char nBuf[81];
                    if( !TEST_GAME )
                      {
		      sprintf(nBuf, "Fread_obj: bad vnum %u.", vnum );
                      log_string(nBuf);
                      }
                    vnum=OBJ_VNUM_MUSHROOM;
                    item_not_valid=TRUE;
                    }
		fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	WearLoc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    fread_to_eol( fp );
	    log_string( "Fread_obj: no match.");
            log_string( word);
	}
    }
#ifdef undef
  if(item_not_valid)
    SET_BIT(obj->extra_flags,ITEM_NOT_VALID);
#endif
}

void fread_corpse( CHAR_DATA *ch, FILE *fp )
{
    bool found;
    OBJ_DATA *obj;

    obj = NULL;
    ch->pcdata->corpse_room = fread_number( fp );
    obj = fread_corpse_item( obj, ch, fp, TRUE);
    obj->test_obj = FALSE;
    ch->pcdata->corpse = obj;
    obj->owned_by=ch->pcdata->pvnum;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	      {
		    fread_to_eol( fp );
		    continue;
	      }

	    if ( letter != '#' )
	      {
		    log_string( "Load_char_corpse: # not found.");
		    break;
        }

	    word = fread_word( fp );
	    if ( !strcasecmp( word, "OBJECT" ) ) 
        fread_corpse_item( obj, ch, fp, FALSE );
	    else
        if ( !strcasecmp( word, "END"    ) )
          break;
	      else
	        {
		      log_string( "Load_char_corpse: bad section.");
		      break;
	        }
	    }
  return;
}

OBJ_DATA *fread_corpse_item( OBJ_DATA *ch_obj, CHAR_DATA *ch, FILE *fp ,
                bool item_corpse)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj, *out_obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;

    out_obj=NULL;
    CREATE(obj,	OBJ_DATA, 1);
    *obj		= obj_zero; 
    obj->name		= STRALLOC( "" );
    obj->short_descr	= STRALLOC( "" );
    obj->long_descr	= STRALLOC( "" );
    obj->description	= STRALLOC( "" );
    obj->condition = 70+number_range(0,29);

    fNest		= FALSE;
    fVnum		= FALSE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !strcasecmp( word, "Affect" ) || !strcasecmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;
	        CREATE( paf, AFFECT_DATA, 1 );

		if ( !strcasecmp( word, "Affect" ) )
		{
		    /* Obsolete 2.0 form. */
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			log_string( "Fread_obj: unknown skill.");
		    else
			paf->type = sn;
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
                LINK(paf, obj->first_affect, obj->last_affect, next, prev );

		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    if ( !strcasecmp( word, "Description" ) )
              {
              char *pt;
              pt = fread_string( fp );
              STRFREE (obj->description );
              obj->description=UPPER_ALLOC( pt );
              STRFREE (pt );
              fMatch = TRUE;
              break;
              }
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );

	    if ( !strcasecmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;
                char *pt;
                CREATE(ed, EXTRA_DESCR_DATA, 1 );

		ed->keyword		= fread_string( fp );
                pt = fread_string( fp );
                ed->description=UPPER_ALLOC( pt );
                STRFREE (pt );
                LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		fMatch = TRUE;
	    }

	    if ( !strcasecmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    log_string( "Fread_obj: incomplete object.");
		    STRFREE (obj->name        );
		    STRFREE (obj->description );
		    STRFREE (obj->short_descr );
		    STRFREE (obj->long_descr );
        	    DISPOSE( obj );
		    return(out_obj);
		}
		else
		{
		    LINK( obj, first_object, last_object, next, prev );
                    obj->test_obj = FALSE;
                    out_obj = FALSE;
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
                      {
                      if( item_corpse )  /*  True for actual corpse */
                        out_obj=obj;    /* Return pointer to corpse */
                      else
		        obj_to_obj( obj, ch_obj );
                      }
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
       if(!IS_SET(obj->extra_flags,ITEM_LEVEL_RENT) || ( obj->level<=0) ||
            obj->level < obj->pIndexData->level - obj->level/5 - 1 )
                      obj->level=obj->pIndexData->level;
                    total_objects++;

		    obj->pIndexData->total_objects++; /* for resets */
		    return(out_obj);
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Indexrefa",	obj->index_reference[0],fread_number( fp ) );
	    KEY( "Indexrefb",	obj->index_reference[1],fread_number( fp ) );
	    break;

	case 'L':
	    SKEY( "LongDescr",	obj->long_descr,	fread_string( fp ) );
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    SKEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !strcasecmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
                    char nBuf[81];
		    sprintf(nBuf, "Fread_obj: bad nest %d.", iNest );
                    log_string(nBuf);
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;
	case 'O':
	    KEY( "Owner",	obj->owned_by,		fread_number( fp ) );
	    break;
  case 'Q':
	  if ( !strcasecmp( word, "Qst" ) )
      {
      set_quest_bits( &obj->obj_quest, 0, 32, fread_number(fp ) );
	    fMatch = TRUE;
      }
	  if ( !strcasecmp( word, "Qstb" ) )
      {
      int byt, cnt;
        byt = fread_number( fp );
        CREATE(obj->obj_quest,unsigned char, MAX_QUEST_BYTES);
        for( cnt=0; cnt<byt; cnt++)
          obj->obj_quest[cnt] = fread_number( fp );
	    fMatch = TRUE;
      }
    break;


	case 'S':
	    SKEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !strcasecmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
                    char nBuf[81];
		    sprintf(nBuf, "Fread_obj: bad iValue %d.", iValue );
                    log_string(nBuf);
		}
		else if ( sn < 0 )
		{
		    log_string( "Fread_obj: unknown skill.");
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !strcasecmp( word, "Values" ) )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

	    if ( !strcasecmp( word, "Vnum" ) )
	    {
		int vnum;
		vnum = fread_number( fp );
		while ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                    {
                    char nBuf[81];
                    sprintf(nBuf,"corpse_fread_obj: bad vnum %u.", vnum );
		    log_string(nBuf);
                    vnum=OBJ_VNUM_MUSHROOM;
                    }
		fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    log_string( "Fread_corpse_obj: no match.");
      log_string( word);
	    fread_to_eol( fp );
	}
    }
 return(out_obj);
}

void roll_race( CHAR_DATA *ch )
{
  int race, cnt;
  race = ch->race;
  ch->pcdata->perm_str= 11+number_range(0,4)+race_table[race].race_mod[0];
  ch->pcdata->perm_dex= 11+number_range(0,4)+race_table[race].race_mod[1];
  ch->pcdata->perm_int= 11+number_range(0,4)+race_table[race].race_mod[2];
  ch->pcdata->perm_wis= 11+number_range(0,4)+race_table[race].race_mod[3];
  ch->pcdata->perm_con= 11+number_range(0,4)+race_table[race].race_mod[4];

  ch->language = SHIFT(race);
  ch->speak = ch->language;
  for(cnt=0;cnt<2;cnt++)
    add_language(ch);
return;
}

void add_language( CHAR_DATA *ch )
{
  int cnt,scnt;
  cnt=number_range(0,MAX_RACE-1);
  scnt=cnt;
  while(IS_SHIFT(ch->language,cnt))
    {
    cnt++;
    if(cnt==MAX_RACE)
      cnt=0;
    if(cnt==scnt)
      return;
    }
  ch->language=SET_SHIFT(ch->language,cnt);
  return;
}

int UNSHIFT( int bits )
{
  int cnt,bit;
  if(bits==0)
    return(-1);
  bit=bits;
  cnt=0;
  while(bit%2!=1)
    {
    bit/=2;
    cnt++;
    }
  return(cnt);
}

bool is_enchanted_obj( OBJ_DATA *obj )
{
  AFFECT_DATA *obj_aff, *ind_aff;
  int total_ind_dam, total_ind_hit;
  int total_obj_dam, total_obj_hit;

  total_ind_dam = 0;
  total_ind_hit = 0;
  total_obj_dam = 0;
  total_obj_hit = 0;

  for( ind_aff=obj->pIndexData->first_affect; ind_aff!=NULL ; ind_aff=ind_aff->next)
    {
    if( ind_aff->location == 18 )
      total_ind_dam+=ind_aff->modifier;
    if( ind_aff->location == 19 )
      total_ind_hit+=ind_aff->modifier;
    }

  for( obj_aff=obj->first_affect; obj_aff!=NULL ; obj_aff=obj_aff->next)
    {
    if( obj_aff->location == 18 )
      total_obj_dam+=obj_aff->modifier;
    if( obj_aff->location == 19 )
      total_obj_hit+=obj_aff->modifier;
    }

  if( total_obj_dam != total_ind_dam || total_obj_hit != total_ind_hit )
    return( TRUE );
   
  return( FALSE );
}


  /* This routine makes sure that files are not cross linked of chopped */
  /* Chaos  - 5/30/96  */
bool is_valid_file( CHAR_DATA *ch, FILE *fp )
{
  char buf[MAX_INPUT_LENGTH];
  char tbuf[MAX_INPUT_LENGTH];
  int cnt;
  int cf;
  char *pt, *pt2;

  cf = ' ';
  cnt = 0;

  while( cf != '#' && cnt>-50 )
    {
    cnt --;
    fseek( fp, cnt, SEEK_END );
    cf = fgetc( fp );
    }

  if( cnt == -50 )
    {
    sprintf( buf, "Didn't find an #END on %s", ch->name );
    log_string( buf );
    rewind( fp );
    return( FALSE );
    }

  *buf = '#';
  pt=buf+1;
  *pt = '\0';
  while( cf != '\r' && cf != '\n' && cf != EOF )
    {
    cf = fgetc( fp );
    *pt = cf;
    pt++;
    }
  *pt = '\0';
  
     /* Old style character */
  if( !strcasecmp( buf, "#END" ) )
    {
    rewind( fp );
    return( TRUE );
    }


    /* Might find #OBJ or something or other  */
  if( *(buf+1)!='E' || *(buf+2)!='N' || *(buf+3)!='D' )
    {
    sprintf( buf, "Didn't find an #END on %s", ch->name );
    log_string( buf );
    rewind( fp );
    return( FALSE );
    }
    

  for( pt=buf; *pt != '\0' && *pt != ' '; pt++ ) ;
  for( ; *pt==' '; pt++);

  if( *pt == '\0' )
    {
    rewind( fp );
    return( TRUE );
    }

  for( pt2=pt+1; *pt2!=' ' && *pt2!='\r' && *pt2!='\n' && *pt2!='\0'; pt2++);
  *pt2 = '\0';
   
  if( !strcasecmp( pt, ch->name ) )
    {
    /* It's a flawless file */
    rewind( fp );
    return( TRUE );
    }
    
  sprintf( tbuf, "Cross linked file %s on %s", pt, ch->name );
  log_string( tbuf );
  rewind( fp );
  return( FALSE );
}

int total_language( CHAR_DATA *ch )
  {
  int total, cnt;

  for( total=0, cnt=0; cnt<MAX_RACE; cnt++)
    if( IS_SET( ch->language, SHIFT(cnt) ) )
      total++;

  return( total );
  }

  /* Let's save that poison data   -  Chaos 4/20/99   */
void fwrite_poison_data( POISON_DATA *pd, FILE *fp )
  {

  fprintf( fp, "POISON_DATA 1 %d %d %d %d %d %d %d %d %d\n",
      pd->for_npc?1:0,
      (int)pd->poison_type,
      pd->instant_damage_low,
      pd->instant_damage_high,
      pd->constant_duration,
      pd->constant_damage_low,
      pd->constant_damage_high,
      pd->owner,
      pd->poisoner );

  if( pd->next != NULL )
    fwrite_poison_data( pd->next, fp );

  return;
  }

POISON_DATA *fread_poison_data( FILE *fp )
  {
  POISON_DATA *pd;
  int ptype;

  CREATE( pd, POISON_DATA, 1 );
  
  ptype = fread_number( fp );

  if( ptype == 1 )
    {
    ptype = fread_number( fp );
    if( ptype==0 )
      pd->for_npc = FALSE;
    else
      pd->for_npc = FALSE;

    pd->poison_type=(sh_int)fread_number(fp);
    pd->instant_damage_low=fread_number(fp);
    pd->instant_damage_high=fread_number(fp);
    pd->constant_duration=fread_number(fp);
    pd->constant_damage_low=fread_number(fp);
    pd->constant_damage_high=fread_number(fp);
    pd->owner=fread_number(fp);
    pd->poisoner =fread_number(fp);
    pd->next = NULL;

    return( pd );
    }
  else
    fread_to_eol( fp );

  DISPOSE( pd );
  return( NULL );
  }

CASTLE_DATA *get_castle_data( CHAR_DATA *ch )
  {
  int rvnum;
  CASTLE_DATA *ocastle;
  ROOM_INDEX_DATA *pRoomIndex;

  ocastle = &get_castle_data_data;
    ocastle->entrance=0;
    ocastle->door_room=-1;
    ocastle->door_dir=-1;
    ocastle->has_backdoor=FALSE;
    ocastle->cost=0;
    ocastle->num_rooms=0;
    ocastle->num_mobiles=0;
    ocastle->num_objects=0;

  for( rvnum=1; rvnum<MAX_VNUM; rvnum++)
    if( room_index[rvnum]!=NULL )
      {
      pRoomIndex = room_index[rvnum];
      if(IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE))
        if( pRoomIndex->creator_pvnum == ch->pcdata->pvnum )
          {
          ocastle->num_rooms++;
          }
      }

      
  return( ocastle );
  }
