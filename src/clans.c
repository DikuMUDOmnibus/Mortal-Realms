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
#include <pthread.h>
#include "merc.h"

extern time_t rent_time;

void    fread_clan      args( ( CLAN_DATA *clan, FILE *fp ) );
bool    load_clan_file  args( ( char *clanfile ) );
void    write_clan_list args( ( void ) );

CLAN_DATA * first_clan;
CLAN_DATA * last_clan;

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !strcasecmp( name, clan->name ) )
         return clan;
    return NULL;
}

CLAN_DATA *get_clan_from_vnum( int vnum )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( vnum == clan->founder_pvnum )
         return clan;
    return NULL;
}
void sort_clans ( CLAN_DATA *pClan )
{
 CLAN_DATA *temp_clan;

        if(!pClan)
        {
                bug("Sort_clans: NULL pClan");
                return;
        }
        for(temp_clan = first_clan; temp_clan;
                        temp_clan = temp_clan->next )
        {
                if(strcmp(pClan->name, temp_clan->name) < 0)
                {
                        INSERT(pClan, temp_clan, first_clan, next, prev);
                        break;
                }
        }
        if(!temp_clan)
        {
                LINK(pClan, first_clan, last_clan, next, prev);
        }
        return;
}

void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char tempfile[256], filename[256];
    if (TEST_GAME) return;
    sprintf( tempfile, "%s/%s.tmp", CLAN_DIR, CLAN_LIST );
    sprintf( filename, "%s/%s", CLAN_DIR, CLAN_LIST );

    fclose( fpReserve );

    remove(tempfile);
    fpout = fopen( tempfile, "w");
    if ( !fpout )
    {
        bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
        return;
    }
    for ( tclan = first_clan; tclan; tclan = tclan->next )
      {	
        fprintf( fpout, "%s\n", tclan->filename );
      }
    fprintf( fpout, "$\nXXXXXXXXXX\n#Clans\n" );
    fclose( fpout );
    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    if(is_valid_save(tempfile, "Clans"))
      {
      remove(filename);
      rename(tempfile, filename);
      }
    fpReserve = fopen( NULL_FILE, "r" );
}

void save_all_clans( )
{
 CLAN_DATA *clan;
 if (TEST_GAME) return;
 for (clan=first_clan;clan!=NULL;clan=clan->next)
    save_clan(clan);

}
#ifdef USE_THREADS
void *threaded_save_clan(void * args);
void save_clan( CLAN_DATA *clan )
{
 pthread_t save_clan_thread;
 if (pthread_create(&save_clan_thread, NULL, threaded_save_clan, clan) <0)
 {
  perror("save_clan_thread barf");
  return;
 }
}

void *threaded_save_clan(void *args)
{
    CLAN_DATA *clan = (CLAN_DATA *) args;
#else
void save_clan( CLAN_DATA *clan )
{
#endif
    FILE *fp;
    char filename[256], tempfile[256], bakfile[256];
    char buf[MAX_STRING_LENGTH];
    if (TEST_GAME) return;
    if ( !clan )
    {
        bug( "save_clan: null clan pointer!", 0 );
#ifdef USE_THREADS
	pthread_detach(pthread_self());
        pthread_exit(NULL);
	return NULL;
#else
        return;
#endif
    }

    if ( !clan->filename || clan->filename[0] == '\0' )
    {
        sprintf( buf, "save_clan: %s has no filename", clan->name );
        bug( buf, 0 );
#ifdef USE_THREADS
	pthread_detach(pthread_self());
        pthread_exit(NULL);
	return NULL;
#else
        return;
#endif
    }

    sprintf( filename, "%s/%s", CLAN_DIR, clan->filename );
    sprintf( bakfile,  "%s/%s.bak", CLAN_DIR, clan->filename );
    sprintf( tempfile, "%s/%s.tmp", CLAN_DIR, clan->filename );

    remove( tempfile );
    fclose( fpReserve );
    if ( ( fp = fopen( tempfile, "w") ) == NULL )
    {
        bug( "save_clan: fopen", 0 );
        perror( tempfile );
    }
    else
    {
        fprintf( fp, "#CLAN\n" );
        fprintf( fp, "Name         %s~\n",      clan->name              );
        fprintf( fp, "Motto        %s~\n",      clan->motto             );
        fprintf( fp, "Description  %s~\n",      clan->description       );
        fprintf( fp, "FileName     %s~\n",      clan->filename          );
        fprintf( fp, "Email        %s~\n",      clan->email             );
        fprintf( fp, "Leader       %s~\n",      clan->leader            );
        fprintf( fp, "NumberOne    %s~\n",      clan->number1           );
        fprintf( fp, "NumberTwo    %s~\n",      clan->number2           );
        fprintf( fp, "NumberThree  %s~\n",      clan->number3           );
        fprintf( fp, "NumberFour   %s~\n",      clan->number4           );
        fprintf( fp, "PKills       %d %d %d %d %d\n",
                clan->pkills[0], clan->pkills[1], clan->pkills[2],
                clan->pkills[3], clan->pkills[4]);
        fprintf( fp, "PDeaths      %d %d %d %d %d\n",
                clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2],
                clan->pdeaths[3], clan->pdeaths[4]);
        fprintf( fp, "MKills       %d\n",       clan->mkills            );
        fprintf( fp, "MDeaths      %d\n",       clan->mdeaths           );
        fprintf( fp, "Members      %d\n",       clan->members           );
        fprintf( fp, "FounderVnum  %d\n",      clan->founder_pvnum     );
        fprintf( fp, "Tax     	   %d\n",       clan->tax               );
        fprintf( fp, "Type     	   %d\n",       clan->type               );
        fprintf( fp, "Coffers      %ld\n",      clan->coffers           );
        fprintf( fp, "ClanObjOne   %d\n",       clan->clanobj1          );
        fprintf( fp, "ClanObjTwo   %d\n",       clan->clanobj2          );
        fprintf( fp, "Home         %d\n",       clan->home            	);
        fprintf( fp, "Store   	   %d\n",       clan->store		);
        fprintf( fp, "Guard	   %d\n",       clan->guard             );
        fprintf( fp, "NumGuards	   %d\n",       clan->num_guards          );
        fprintf( fp, "NumHealers   %d\n",       clan->num_healers          );
        fprintf( fp, "NumBackDoor  %d\n",       clan->num_backdoors      );
        fprintf( fp, "Healer       %d\n",       clan->healer            );
        fprintf( fp, "End\n\n"                                          );
        fprintf( fp, "#END\n\n"                                           );
        fprintf( fp, "#Clanfile\n"                                        );
    }
    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fp );
    if(is_valid_save(tempfile, "Clanfile"))
      {
      rename( filename, bakfile );
      rename( tempfile, filename);
      }
    fpReserve = fopen( NULL_FILE, "r");
    close_timer( TIMER_CLAN_SAVE );
#ifdef USE_THREADS
	pthread_detach(pthread_self());
        pthread_exit(NULL);
	return NULL;
#else
        return;
#endif
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !strcasecmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }
void fread_clan( CLAN_DATA *clan, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
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

        case 'C':
            KEY( "ClanObjOne",  clan->clanobj1,         fread_number( fp ) );
            KEY( "ClanObjTwo",  clan->clanobj2,         fread_number( fp ) );
            KEY( "Coffers",     clan->coffers,           fread_number( fp ) );
            break;

        case 'D':
            KEY( "Description", clan->description,      fread_string( fp ) );
            break;
        case 'F':
            KEY( "FileName",    clan->filename,      fread_string( fp ) );
            KEY( "FounderVnum",    clan->founder_pvnum,  fread_number( fp ) );
            break;

        case 'E':
            KEY( "Email",    clan->email,      fread_string( fp ) );
            if ( !strcasecmp( word, "End" ) )
            {
                if (!clan->name)
                  clan->name            = STRALLOC( "" );
                if (!clan->leader)
                  clan->leader          = STRALLOC( "" );
                if (!clan->description)
                  clan->description     = STRALLOC( "" );
                if (!clan->motto)
                  clan->motto           = STRALLOC( "" );
                if (!clan->number1)
                  clan->number1         = STRALLOC( "" );
                if (!clan->number2)
                  clan->number2         = STRALLOC( "" );
                if (!clan->number3)
                  clan->number3         = STRALLOC( "" );
                if (!clan->number4)
                  clan->number4         = STRALLOC( "" );
                if (!clan->email)
                  clan->email           = STRALLOC( "" );
                return;
            }
            break;

        case 'G':
            KEY( "Guard",       clan->guard,           fread_number( fp ) );
	    break;
        case 'H':
            KEY( "Home",        clan->home,           fread_number( fp ) );
            KEY( "Healer",      clan->healer,           fread_number( fp ) );
            break;

        case 'L':
            KEY( "Leader",      clan->leader,           fread_string( fp ) );
            break;

        case 'M':
            KEY( "MDeaths",     clan->mdeaths,          fread_number( fp ) );
            KEY( "Members",     clan->members,          fread_number( fp ) );
            KEY( "MKills",      clan->mkills,           fread_number( fp ) );
            KEY( "Motto",       clan->motto,            fread_string( fp ) );
            break;

        case 'N':
            KEY( "Name",        clan->name,             fread_string( fp ) );
            KEY( "NumberOne",   clan->number1,          fread_string( fp ) );
            KEY( "NumberTwo",   clan->number2,          fread_string( fp ) );
            KEY( "NumberThree", clan->number3,          fread_string( fp ) );
            KEY( "NumberFour",  clan->number4,          fread_string( fp ) );
            KEY( "NumBackDoor", clan->num_backdoors,    fread_number( fp ) );
            KEY( "NumGuards",    clan->num_guards,       fread_number( fp ) );
            KEY( "NumHealers",  clan->num_healers,       fread_number( fp ) );
            break;

        case 'P':
            if ( !strcasecmp ( word, "PDeaths" ) )
            {
                fMatch = TRUE;
                clan->pdeaths[0] = fread_number( fp );
                clan->pdeaths[1] = fread_number( fp );
                clan->pdeaths[2] = fread_number( fp );
                clan->pdeaths[3] = fread_number( fp );
                clan->pdeaths[4] = fread_number( fp );
            }
            if ( !strcasecmp ( word, "PKills" ) )
            {
                fMatch = TRUE;
                clan->pkills[0] = fread_number( fp );
                clan->pkills[1] = fread_number( fp );
                clan->pkills[2] = fread_number( fp );
                clan->pkills[3] = fread_number( fp );
                clan->pkills[4] = fread_number( fp );
            }
            break;


        case 'S':
            KEY( "Store",     clan->store,        fread_number( fp ) );
            break;
        case 'T':
            KEY( "Tax",     clan->tax,        fread_number( fp ) );
            KEY( "Type",    clan->type,        fread_number( fp ) );
            break;
        }

        if ( !fMatch )
        {
            sprintf( buf, "Fread_clan: no match: %s", word );
            bug( buf, 0 );
        }
    }
}

/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

    CREATE(clan, CLAN_DATA, 1);

    clan->pkills[0] = 0;
    clan->pkills[1] = 0;
    clan->pkills[2] = 0;
    clan->pkills[3] = 0;
    clan->pkills[4] = 0;
    clan->pdeaths[0]= 0;
    clan->pdeaths[1]= 0;
    clan->pdeaths[2]= 0;
    clan->pdeaths[3]= 0;
    clan->pdeaths[4]= 0;
    clan->mkills    = 0;
    clan->mdeaths   = 0;
    clan->tax       = 0;
    clan->coffers    	   = 0;
    clan->num_guards       = 0;
    clan->num_healers       = 0;
    clan->num_backdoors    = 0;
    clan->num_leaders    = 0;
    clan->founder_pvnum    = 0;
    clan->type = 0;

                  clan->name            = STRALLOC( "" );
                  clan->leader          = STRALLOC( "" );
                  clan->description     = STRALLOC( "" );
                  clan->motto           = STRALLOC( "" );
                  clan->number1         = STRALLOC( "" );
                  clan->number2         = STRALLOC( "" );
                  clan->number3         = STRALLOC( "" );
                  clan->number4         = STRALLOC( "" );
                  clan->email           = STRALLOC( "" );

    found = FALSE;
    sprintf( filename, "%s/%s", CLAN_DIR, clanfile );

    if ( ( fp = fopen( filename, "r+" ) ) != NULL )
    {

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
                bug( "Load_clan_file: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if ( !strcasecmp( word, "CLAN" ) )
            {
                fread_clan( clan, fp );
                break;
            }
            else
            if ( !strcasecmp( word, "END"  ) )
                break;
            else
            {
                char buf[MAX_STRING_LENGTH];

                sprintf( buf, "Load_clan_file: bad section: %s.", word );
                bug( buf, 0 );
                break;
            }
        }
    fclose( fp );
    /*    fclose( fp ); Keep 'em all open 
      No, cuz we've got finite file descriptors Martin 3/12/98*/
    }

    if (clan->leader[0] != '\0')
     clan->num_leaders++;
    if (clan->number1[0] != '\0')
     clan->num_leaders++;
    if (clan->number2[0] != '\0')
     clan->num_leaders++;
    if (clan->number3[0] != '\0')
     clan->num_leaders++;
    if (clan->number4[0] != '\0')
     clan->num_leaders++;
    
    sort_clans ( clan );
    /*LINK( clan, first_clan, last_clan, next, prev );
	Let's sort 'em by name instead Martin 5/3/99*/
    return found;
}

void load_clans( )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];


    first_clan  = NULL;
    last_clan   = NULL;

    log_string( "Loading clans..." );

    sprintf( clanlist, "%s/%s", CLAN_DIR, CLAN_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( clanlist, "r") ) == NULL )
    {
        perror( clanlist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        log_string( filename );
        if ( filename[0] == '$' )
          break;

        if ( !load_clan_file( filename ) )
        {
          sprintf( buf, "Cannot load clan file: %s", filename );
          bug( buf, 0 );
        }
    }
    fclose( fpList );
    log_string("Done clans " );
    fpReserve = fopen( NULL_FILE, "r");
    return;
}

void do_initiate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    clan = ch->pcdata->clan;

    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 )
    ||   !strcasecmp( ch->name, clan->number4 ) )
        ;
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Initiate whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
    if (IS_SET(victim->act, PLR_OUTCAST))
    {
        send_to_char( "Outcasts are pariah! Look elsewhere for new recruits.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( IS_IMMORTAL(victim) )
    {
        send_to_char( "You can't initiate such a godly presence.\n\r", ch );
        return;
    }

    if ( victim->level < 20 )
    {
            send_to_char( "This player is not worthy of initiation.\n\r", ch );
            return;
    }
    if (clan->coffers<2500000)
    {
            send_to_char( "Your clan is too poor to pay for this initiation.\n\r", ch );
            return;
    }

    if ( victim->pcdata->clan )
    {
     if ( victim->pcdata->clan == clan )
       send_to_char( "This player already belongs to your clan!\n\r", ch );
      else
       send_to_char( "This player already belongs to a clan!\n\r", ch );
     return;
    }

    STRFREE (victim->pcdata->clan_pledge);
    victim->pcdata->clan_pledge = QUICKLINK( clan->name );
    ch->pcdata->clan->coffers-=2500000;
    ch_printf(ch,  "Your clan pays 2500000 gold coins on an initiation rite for %s.\n\r", victim->name);
    act( "$n initiates $N the ways of $t.", ch, clan->name, victim, TO_NOTVICT );
    act( "$n initiates you in the ways of $t.\n\rYou must pledge your support to $t to become a full member.", ch, clan->name, victim, TO_VICT );
    save_char_obj( victim, NORMAL_SAVE );
    return;
}

void do_nominate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    int position=0;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    clan = ch->pcdata->clan;

    if ( !strcasecmp( ch->name, clan->leader  ) );
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    
    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "Nominate whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
    if (IS_SET(victim->act, PLR_OUTCAST))
    {
        send_to_char( "Outcasts are pariah! Look elsewhere for new leaders.\n\r", ch);
        return;
    }
    if (!strcasecmp(arg2, "number1"))
     position=1;
    else if (!strcasecmp(arg2, "number2"))
     position=2;
    else if (!strcasecmp(arg2, "number3"))
     position=3;
    else if (!strcasecmp(arg2, "number4"))
     position=4;
    else
    {
        ch_printf(ch, "Nominate %s for what position ?\n\r", arg1);
        return;
    }
	
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( IS_IMMORTAL(victim) )
    {
        send_to_char( "You can't nominate such a godly presence.\n\r", ch );
        return;
    }

    if ( victim->level < 50 )
    {
            send_to_char( "This player is not worthy of nomination.\n\r", ch );
            return;
    }
    if (clan->coffers<10000000)
    {
            send_to_char( "Your clan is too poor to pay for this nomination.\n\r", ch );
            return;
    }

    if ( victim->pcdata->clan )
    {
     if ( victim->pcdata->clan == clan )
       send_to_char( "This player already belongs to your clan!\n\r", ch );
      else
       send_to_char( "This player already belongs to a clan!\n\r", ch );
     return;
    }

    STRFREE (victim->pcdata->clan_pledge);
    victim->pcdata->clan_pledge = QUICKLINK( clan->name );
    ch->pcdata->clan->coffers-=10000000;
    victim->pcdata->clan_position=position;
    ch_printf(ch,  "Your clan pays 10000000 gold coins on an nomination rite for %s.\n\r", victim->name);
    act( "$n nominates $N as a leader of $t.", ch, clan->name, victim, TO_NOTVICT );
    act( "$n nominates you as a leader of $t.\n\rYou must pledge your support to $t to become a leader.", ch, clan->name, victim, TO_VICT );
    save_char_obj( victim, NORMAL_SAVE );
    return;
}
void do_pledge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Pledge support to which clan ?\n\r", ch );
        return;
    }

    if (IS_SET(ch->act, PLR_OUTCAST))
    {
        send_to_char( "Outcasts are pariah! You may not pledge support to a new clan.\n\r", ch);
        return;
    }

    if ( ch->level < 20 )
    {
            send_to_char( "You are not worthy of joining yet.\n\r", ch );
            return;
    }

    if ( ch->pcdata->clan )
    {
     send_to_char( "You are already in a clan!\n\r", ch );
     return;
    }

    if ((clan = get_clan(argument)) == NULL)
    {
     send_to_char( "That clan does not exist!\n\r", ch );
     return;
    }

    if (strcasecmp(clan->name, ch->pcdata->clan_pledge))
    {
     send_to_char( "That clan has not initiated you!\n\r", ch);
     return;
    }
    if (ch->pcdata->clan_position>0)
    {
        if ( ch->gold < 20000000)
        {
         send_to_char( "You do not have enough money to accept the leader position at this time!\n\r", ch );
         return;
        }
        ch->gold-=20000000;
	switch (ch->pcdata->clan_position)
 	{
	case 1:
          STRFREE (clan->number1 );
          clan->number1 = STRALLOC( ch->name );
	  ch_printf(ch, "You accept the position of number one in the clan of %s.\n\r", clan->name);
	  break;
	case 2:
          STRFREE (clan->number2 );
          clan->number2 = STRALLOC( ch->name );
	  ch_printf(ch, "You accept the position of number two in the clan of %s.\n\r", clan->name);
	  break;
	case 3:
          STRFREE (clan->number3 );
          clan->number3 = STRALLOC( ch->name );
	  ch_printf(ch, "You accept the position of number three in the clan of %s.\n\r", clan->name);
	  break;
	case 4:
          STRFREE (clan->number4 );
          clan->number4 = STRALLOC( ch->name );
	  ch_printf(ch, "You accept the position of number four in the clan of %s.\n\r", clan->name);
	  break;
         }
    }
    else
    {
     if ( ch->gold < 5000000)
     {
      send_to_char( "You do not have enough money to pledge support at this time!\n\r", ch );
      return;
     }
    ch->gold-=5000000;
    clan->members++;
    }
    ch->pcdata->clan = clan;
    STRFREE (ch->pcdata->clan_name);
    STRFREE (ch->pcdata->clan_pledge);
    ch->pcdata->clan_name = QUICKLINK( clan->name );
    ch->pcdata->clan_pledge = STRALLOC( "" );
    ch->pcdata->clan_position=0;
    act( "You pledge support to $t.", ch, clan->name, NULL, TO_CHAR );
    act( "$n pledges support to $t.", ch, clan->name, NULL, TO_ROOM );
    save_char_obj( ch, NORMAL_SAVE );
    save_clan( clan );
    return;
}

void do_renounce( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
  clan = ch->pcdata->clan;
  if(argument[0]=='\0')
    {
    send_to_char( "You must use a password with this command.\n\rThis command will cut your ties with your clan.\n\r", ch); 
    wipe_string( argument );
    return;
    }
  if( !strcmp(crypt(argument,ch->pcdata->pwd),ch->pcdata->pwd) )
    {
    --clan->members;
    if ( !strcasecmp( ch->name, ch->pcdata->clan->number1 ) )
    {
        STRFREE (ch->pcdata->clan->number1 );
        ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !strcasecmp( ch->name, ch->pcdata->clan->number2 ) )
    {
        STRFREE (ch->pcdata->clan->number2 );
        ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    if ( !strcasecmp( ch->name, ch->pcdata->clan->number3 ) )
    {
        STRFREE (ch->pcdata->clan->number3 );
        ch->pcdata->clan->number3 = STRALLOC( "" );
    }
    if ( !strcasecmp( ch->name, ch->pcdata->clan->number4 ) )
    {
        STRFREE (ch->pcdata->clan->number4 );
        ch->pcdata->clan->number4 = STRALLOC( "" );
    }
    ch_printf(ch, "You turn your back on %s.\n\r", ch->pcdata->clan_name);
    ch->pcdata->clan = NULL;
    STRFREE (ch->pcdata->clan_name);
    ch->pcdata->clan_name = STRALLOC( "" );

    save_char_obj( ch, NORMAL_SAVE );    /* clan gets saved when pfile is saved */
    save_clan( clan );
    return;
   }
  else
    {
    send_to_char( "That was not your password.\n\r", ch);
    }
  wipe_string( argument );
  return;
}
void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    clan = ch->pcdata->clan;

    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 )
    ||   !strcasecmp( ch->name, clan->number4 ) )
        ;
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Outcast whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
      send_to_char( "Kick yourself out of your own clan?\n\r", ch );
      return;
    }


    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
      send_to_char( "This player does not belong to your clan!\n\r", ch );
      return;
    }

    --clan->members;
    if ( !strcasecmp( victim->name, ch->pcdata->clan->number1 ) )
    {
        STRFREE (ch->pcdata->clan->number1 );
        ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !strcasecmp( victim->name, ch->pcdata->clan->number2 ) )
    {
        STRFREE (ch->pcdata->clan->number2 );
        ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    if ( !strcasecmp( victim->name, ch->pcdata->clan->number3 ) )
    {
        STRFREE (ch->pcdata->clan->number3 );
        ch->pcdata->clan->number3 = STRALLOC( "" );
    }
    if ( !strcasecmp( victim->name, ch->pcdata->clan->number4 ) )
    {
        STRFREE (ch->pcdata->clan->number4 );
        ch->pcdata->clan->number4 = STRALLOC( "" );
    }
    victim->pcdata->clan = NULL;
    STRFREE (victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    char_from_room( ch );
    switch( which_god( ch))
    {
     case GOD_INIT_ORDER:
     case GOD_ORDER: char_to_room( ch, get_room_index( 9799 ) );break;
     case GOD_INIT_CHAOS:
     case GOD_CHAOS: char_to_room( ch, get_room_index( 9719 ) );break;
     default: char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE) );break;
    }
    victim->recall = ch->in_room->vnum;
    act( "You outcast $N from $t.", ch, clan->name, victim, TO_CHAR );
    act( "$n outcasts $N from $t.", ch, clan->name, victim, TO_ROOM );
    act( "$n outcasts you from $t.", ch, clan->name, victim, TO_VICT );
    SET_BIT( victim->act, PLR_OUTCAST);
    victim->outcast_played=victim->played;

    sprintf(buf, "%s has been outcast from %s!", victim->name, clan->name);
    do_echo(ch, buf);

    save_char_obj( victim, NORMAL_SAVE );    /* clan gets saved when pfile is saved */
    save_clan( clan );
    return;
}

void do_orders( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    int count = 0;

    if ( argument[0] == '\0' )
    {
     send_to_char_color( "{120}\n\rOrder         Founder            Mkills         Mdeaths\n\r", ch);
         send_to_char_color( "{120}_______________________________________________________\n\r", ch );
        for ( clan = first_clan; clan; clan = clan->next )
        {
	   if (clan->type != CLAN_PEACEFUL) 
   	     continue;
            ch_printf_color( ch, "{070}%-13s %-13s{120}      %-7d        %7d\n\r", 
		 clan->name, clan->leader, clan->mkills, clan->mdeaths);
            count++;
        }
        if ( !count )
          send_to_char_color( "{120}There are no Orders currently formed.\n\r", ch );
        else
         send_to_char_color( "{120}_______________________________________________________\n\r\n\rUse 'orders <order>' for detailed information.\n\r", ch );
        return;
    }
    clan = get_clan( argument );
    if ( !clan )
    {
        send_to_char_color( "{120}No such order.\n\r", ch );
        return;
    }
    if (clan->type != CLAN_PEACEFUL)
    {
        send_to_char_color( "{110}No such clan.\n\r", ch );
        return;
    }
    ch_printf_color( ch, "{120}%s, '%s'\n\r\n\r", clan->name, clan->motto );
    if (IS_IMMORTAL(ch))
    {
send_to_char_color( "{070}Victories:               Defeats:\n\r", ch );
          ch_printf( ch, " Neutral Kills : %4d  Neutral Deaths : %4d\n\r", clan->pkills[CLAN_ASSASSINATE_KILL], clan->pdeaths[CLAN_ASSASSINATE_DEATH]);
          ch_printf( ch, "   Chaos Kills : %4d    Chaos Deaths : %4d\n\r", clan->pkills[CLAN_CHAOS_KILL], clan->pdeaths[CLAN_CHAOS_DEATH]);
          ch_printf( ch, "   Order Kills : %4d    Order Deaths : %4d\n\r", clan->pkills[CLAN_ORDER_KILL], clan->pdeaths[CLAN_ORDER_DEATH]);
          ch_printf( ch, "    Race Kills : %4d     Race Deaths : %4d\n\r", clan->pkills[CLAN_RACE_KILL], clan->pdeaths[CLAN_RACE_DEATH]);
          ch_printf( ch, "     Mob Kills : %4d      Mob Deaths : %4d\n\r", clan->mkills, clan->mdeaths);
     /*   ch_printf( ch, "    Clan Kills : %4d     Clan Deaths : %4d\n\r", clan->pkills[CLAN_CHAOS_KILL], clan->pdeaths[CLAN_KILL]);*/
    }
    ch_printf_color( ch, "{070}Order Leaders:  %s %s %s %s %s\n\r",
                        clan->leader[0] == '\0'? "":clan->leader,
                        clan->number1[0] == '\0'? "":clan->number1,
                        clan->number2[0] == '\0'? "":clan->number2,
                        clan->number3[0] == '\0'? "":clan->number3,
                        clan->number4[0] == '\0'? "":clan->number4);
    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 ) 
    ||   !strcasecmp( ch->name, clan->number4 ) 
    ||   IS_IMMORTAL(ch) )
    {
        ch_printf( ch, "Members    :  %3d   Tax Rate   :   %3d%%\n\r", clan->members, clan->tax );
        ch_printf( ch, "Coffers    :  %ld\n\r", clan->coffers );
        ch_printf( ch, "Email      :  %s\n\r", clan->email );
 	ch_printf( ch, "Number of Guards: %d Number of Healers: %d\n\r", clan->num_guards, clan->num_healers); 
 	ch_printf( ch, "Number of Back Doors: %d\n\r", clan->num_backdoors);
    }
    if (IS_IMMORTAL(ch))
    {
 	ch_printf( ch, "Home : %6d, Store %6d, Guard %6d, Healer %6d Founder Vnum: %d\n\r",
	   clan->home, clan->store, clan->guard, clan->healer, clan->founder_pvnum);
    }
    /* Let's not spam first_person too much */
    if (strlen(clan->description)>400) 
     clan->description[320]='\0';
    ch_printf_color( ch, "{120}Description:\n\r%s\n\r", clan->description );
    return;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    int count = 0;

    if ( argument[0] == '\0' )
    {
     send_to_char_color( "{110}\n\rClan          Founder            Pkills         Pdeaths\n\r", ch);
         send_to_char_color( "{110}_______________________________________________________\n\r", ch );
        for ( clan = first_clan; clan; clan = clan->next )
        {
	   if (clan->type != CLAN_WARLIKE)
	     continue;
            ch_printf_color( ch, "{070}%-13s %-13s{010}      %-7d        %7d\n\r", 
		 clan->name, clan->leader, clan->pkills[0]+clan->pkills[1]+
                 clan->pkills[2]+clan->pkills[3]+clan->pkills[4],	
		 clan->pdeaths[0]+clan->pdeaths[1]+
                 clan->pdeaths[2]+clan->pdeaths[3]+clan->pdeaths[4]);
            count++;
        }
        if ( !count )
          send_to_char_color( "{110}There are no Clans currently formed.\n\r", ch );
        else
         send_to_char_color( "{110}_______________________________________________________\n\r\n\rUse 'clans <clan>' for detailed information and a breakdown of victories.\n\r", ch );
        return;
    }
    clan = get_clan( argument );
    if ( !clan )
    {
        send_to_char_color( "{110}No such clan.\n\r", ch );
        return;
    }
    if (clan->type != CLAN_WARLIKE)
    {
        send_to_char_color( "{110}No such clan.\n\r", ch );
        return;
    }
    ch_printf_color( ch, "{110}%s, '%s'\n\r\n\r", clan->name, clan->motto );
send_to_char_color( "{070}Victories:               Defeats:\n\r", ch );
          ch_printf( ch, "Assassin Kills : %4d Assassin Deaths : %4d\n\r", clan->pkills[CLAN_ASSASSINATE_KILL], clan->pdeaths[CLAN_ASSASSINATE_DEATH]);
          ch_printf( ch, "   Chaos Kills : %4d    Chaos Deaths : %4d\n\r", clan->pkills[CLAN_CHAOS_KILL], clan->pdeaths[CLAN_CHAOS_DEATH]);
          ch_printf( ch, "   Order Kills : %4d    Order Deaths : %4d\n\r", clan->pkills[CLAN_ORDER_KILL], clan->pdeaths[CLAN_ORDER_DEATH]);
          ch_printf( ch, "    Race Kills : %4d     Race Deaths : %4d\n\r", clan->pkills[CLAN_RACE_KILL], clan->pdeaths[CLAN_RACE_DEATH]);
     /*     ch_printf( ch, "     Mob Kills : %4d      Mob Deaths : %4d\n\r", clan->mkills, clan->mdeaths);
        ch_printf( ch, "    Clan Kills : %4d     Clan Deaths : %4d\n\r", clan->pkills[CLAN_CHAOS_KILL], clan->pdeaths[CLAN_KILL]);*/
    ch_printf_color( ch, "{070}Clan Leaders:  %s %s %s %s %s\n\r",
                        clan->leader[0] == '\0'? "":clan->leader,
                        clan->number1[0] == '\0'? "":clan->number1,
                        clan->number2[0] == '\0'? "":clan->number2,
                        clan->number3[0] == '\0'? "":clan->number3,
                        clan->number4[0] == '\0'? "":clan->number4);
    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 ) 
    ||   !strcasecmp( ch->name, clan->number4 ) 
    ||   IS_IMMORTAL(ch) )
    {
        ch_printf( ch, "Members    :  %3d   Tax Rate   :   %3d%%\n\r", clan->members, clan->tax );
        ch_printf( ch, "Coffers    :  %ld\n\r", clan->coffers );
        ch_printf( ch, "Email      :  %s\n\r", clan->email );
 	ch_printf( ch, "Number of Guards: %d Number of Healers: %d\n\r", clan->num_guards, clan->num_healers); 
 	ch_printf( ch, "Number of Back Doors: %d\n\r", clan->num_backdoors);
    }
    if (IS_IMMORTAL(ch))
    {
 	ch_printf( ch, "Home : %6d, Store %6d, Guard %6d, Healer %6d Founder Vnum: %d\n\r",
	   clan->home, clan->store, clan->guard, clan->healer, clan->founder_pvnum);
    }
    /* Let's not spam first_person too much */
    if (strlen(clan->description)>400) 
     clan->description[320]='\0';
    ch_printf_color( ch, "{110}Description:\n\r%s\n\r", clan->description );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    /*char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];*/
    CLAN_DATA *clan;
    int rent=0;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    clan = ch->pcdata->clan;

    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 )
    ||   !strcasecmp( ch->name, clan->number4 ) )
        ;
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    ch_printf(ch, "Rent for %s's clanhall:\n\r\n\r", clan->name);
    if (clan->type == CLAN_PEACEFUL)
    {
      ch_printf(ch, "Basic Hall		: %d\n\r", RENT_BASIC_ORDER_HALL); 
      rent +=RENT_BASIC_ORDER_HALL;
    }
    else
    {
      ch_printf(ch, "Basic Hall		: %d\n\r", RENT_BASIC_CLAN_HALL); 
      rent +=RENT_BASIC_CLAN_HALL;
    }
    if (clan->num_healers>0) 
    {
    ch_printf(ch, "Rent for Healers     : %d (%d healers at %d per healer)\n\r", clan->num_healers*RENT_PER_HEALER, clan->num_healers, RENT_PER_HEALER);
     rent+=RENT_PER_HEALER*clan->num_healers;
    }
    if (clan->num_guards>0)
    {
    ch_printf(ch, "Rent for Guards	: %d (%d guards at %d per guard)\n\r", clan->num_guards*RENT_PER_GUARD, clan->num_guards, RENT_PER_GUARD);
      rent+=RENT_PER_GUARD*clan->num_guards;
    }
    if (clan->num_backdoors>0)
    {
      ch_printf(ch, "Rent for Backdoors	: %d (%d backdoors at %d per backdoor)\n\r",  clan->num_backdoors*RENT_PER_BACKDOOR, clan->num_backdoors, RENT_PER_BACKDOOR);
      rent+=clan->num_backdoors*RENT_PER_BACKDOOR;
    }
    ch_printf(ch, "\n\rTotal is                : %d gold.\n\r", rent);
    ch_printf(ch, "Rent is next due on     : %s\r", ctime(&rent_time));
    return;
}

void do_gohome( CHAR_DATA *ch, char *argument )
{
 ROOM_INDEX_DATA *location;

 if ( IS_NPC( ch ) || !ch->pcdata->clan )
 {
  send_to_char( "Huh?\n\r", ch );
  return;
 }
 if (ch->fighting !=NULL)
 {
  send_to_char( "You can't gohome while fighting!\n\r", ch);
  return;
 }
 if (ch->position < POS_RESTING )
 {
  send_to_char( "You must be standing or resting to gohome.\n\r", ch);
  return;
 }
 if (ch->hit < ch->max_hit *.75  )
 {
  send_to_char( "You are too badly injured to gohome.\n\r", ch);
  return;
 }

 location = get_room_index( ch->pcdata->clan->home );

 if (location == NULL)
 {
  send_to_char( "Your clan doesn't have a home!\n\r", ch );
  return;
 }

 if(IS_SET(ch->in_room->room_flags, ROOM_NO_GOHOME))
  {
   send_to_char( "You cannot do that in this room.\n\r", ch);
   return;
  }
  else if(  IS_SET( ch->in_room->area->flags, AFLAG_NOGOHOME) )
  {
   send_to_char( "You cannot do that in this area.\n\r", ch);
   return;
  }
  WAIT_STATE( ch, 5);
  if ( ch->in_room == location )
    return;
  act( "$n turns sideways and disappears.", ch, NULL, NULL, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, location );
  act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
  return;
}

void do_clanwhere( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    CHAR_DATA *fch;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_INPUT_LENGTH];
    int length=0;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    clan = ch->pcdata->clan;
    
    if ( !strcasecmp( ch->name, clan->leader  )
    ||   !strcasecmp( ch->name, clan->number1 )
    ||   !strcasecmp( ch->name, clan->number2 )
    ||   !strcasecmp( ch->name, clan->number3 )
    ||   !strcasecmp( ch->name, clan->number4 ) )
        ;
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
  length = str_cpy_max( buf, "Locations of members of your clan:\n\r", MAX_STRING_LENGTH);
  for( fch=first_char; fch!=NULL; fch=fch->next)
   {
    if (!IS_NPC(fch) && fch->pcdata->clan !=NULL
	&& fch->pcdata->clan == ch->pcdata->clan)
     {
       sprintf(buf1, "%-13s in %-25s ", fch->name, fch->in_room->name);
       buf1[43]='\0';
       length = str_apd_max( buf, buf1, length, MAX_STRING_LENGTH); 
       sprintf(buf1, "%-20s", fch->in_room->area->name);
       buf1[20]='\0';
       length = str_apd_max( buf, buf1, length, MAX_STRING_LENGTH); 
       length = str_apd_max( buf, "\n\r", length, MAX_STRING_LENGTH); 
     }
   } 
 ship_to_char(buf, ch, length);
 return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    char buf[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    int i, type=0;
    
    if (IS_NPC(ch)) 
	return;
    argument= one_argument(argument, arg);

    if ( !arg|| arg[0] == '\0' )
    {
        send_to_char( "Usage: makeclan <clan name>\n\r", ch );
        return;
    }
    if ( ch->pcdata->clan != NULL )
    {
       send_to_char( "You are already in a clan!\n\r", ch );
       return;
    }
    if (!strcasecmp(argument, "peaceful"))
     type=CLAN_PEACEFUL;
    if (!strcasecmp(argument, "warlike"))
     type=CLAN_WARLIKE;

    if (type==0)
    {
       send_to_char( "What type of clan would you like to form ? Please specify Peaceful or Warlike.\n\r", ch );
       return;
    }
    
	
    if (ch->pcdata->castle==NULL)
     {
      send_to_char("You have no castle established yet.\n\r", ch);
      return;
     }
    if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
     {
      send_to_char("You are NOT in your castle!\n\r", ch);
      return;
     }
    if (ch->pcdata->account < CLANHALL_CONSTRUCTION)
     {
      ch_printf(ch, "You do not have enough money in the bank to make a clan!\n\rIt costs %d.\n\r", CLANHALL_CONSTRUCTION);
      return;
     }

    ch->pcdata->account -=CLANHALL_CONSTRUCTION; 
    found = FALSE;
    for (i=0;i<strlen(arg);i++)
     arg[i]=LOWER(arg[i]);

    sprintf( filename, "%s.cln", arg);
    CREATE( clan, CLAN_DATA, 1);

    arg[0] = UPPER(arg[0]); 
    clan->name          = STRALLOC( arg);
    clan->motto         = STRALLOC( "" );
    clan->description   = STRALLOC( "" );
    clan->leader        = STRALLOC( ch->name );
    clan->number1       = STRALLOC( "" );
    clan->number2       = STRALLOC( "" );
    clan->number3       = STRALLOC( "" );
    clan->number4       = STRALLOC( "" );
    clan->filename      = STRALLOC( filename );
    clan->email         = STRALLOC( "" );
    clan->home		= ch->in_room->vnum;
    clan->founder_pvnum = ch->pcdata->pvnum;
    clan->type=type;
    clan->pkills[0] = 0;
    clan->pkills[1] = 0;
    clan->pkills[2] = 0;
    clan->pkills[3] = 0;
    clan->pkills[4] = 0;
    clan->pdeaths[0]= 0;
    clan->pdeaths[1]= 0;
    clan->pdeaths[2]= 0;
    clan->pdeaths[3]= 0;
    clan->pdeaths[4]= 0;
    clan->mkills    = 0;
    clan->mdeaths   = 0;
    clan->tax       = 0;
    clan->coffers    	   = 0;
    clan->num_guards       = 0;
    clan->num_healers       = 0;
    clan->num_backdoors    = 0;
    clan->num_leaders    = 0;

    sort_clans ( clan );
    /*LINK( clan, first_clan, last_clan, next, prev );
	Let's sort 'em by name instead Martin 5/3/99*/

    clan->members=1;
    ch->pcdata->clan = clan;
    STRFREE (ch->pcdata->clan_name);
    ch->pcdata->clan_name = QUICKLINK( clan->name );
    SET_BIT(ch->in_room->room_flags, ROOM_NOTE_BOARD);
    save_clan( clan );
    write_clan_list( );
    ch_printf( ch, "Clan %s made.\n\r", clan->name);
    if (clan->type == CLAN_PEACEFUL)
      sprintf(buf, "The order of %s is formed!", clan->name);
    else
      sprintf(buf, "The clan of %s is formed!", clan->name);
    do_echo(NULL, buf);
}
void do_destroy_clan(CHAR_DATA *ch, char *argument)
{
 CLAN_DATA *clan;
 char buf[MAX_INPUT_LENGTH];

    if ((clan = get_clan(argument)) == NULL)
    {
     send_to_char( "That clan does not exist!\n\r", ch );
     return;
    }
  if (ch->level <98)
    {
     send_to_char( "You are not allowed to delete clans.\n\r", ch );
     return;
    }
 sprintf(buf, "The clan of %s has been disbanded by the gods!", clan->name);
 do_echo(NULL, buf);

 destroy_clan(clan);   
}
void destroy_clan(CLAN_DATA *deadclan)
{
 PLAYER_GAME *fpl;
 ROOM_INDEX_DATA *room;
 for(fpl=first_player;fpl!=NULL;fpl=fpl->next)
 {
  if (!IS_NPC(fpl->ch) && fpl->ch->pcdata->clan != NULL &&
      fpl->ch->pcdata->clan == deadclan)
  {
   fpl->ch->pcdata->clan = NULL;
   STRFREE(fpl->ch->pcdata->clan_name);
   fpl->ch->pcdata->clan_name = STRALLOC("");
  }
 }
 room = get_room_index(deadclan->home);
 REMOVE_BIT(room->room_flags, ROOM_NOTE_BOARD);

#ifdef UNLINKCHECK
  {
  CLAN_DATA *tclan;
  bool foundc;

  for( foundc=FALSE, tclan=first_clan; tclan!=NULL; tclan=tclan->next )
    if( tclan==deadclan )
      {
      foundc=TRUE;
      break;
      }
  if( foundc )
    UNLINK (deadclan, first_clan, last_clan, next, prev);
  else
    bug( "UNLINK ERROR could not find clan %d.", deadclan->name );
  }
#else
    UNLINK (deadclan, first_clan, last_clan, next, prev);
#endif
  
 STRFREE(deadclan->name);
 STRFREE(deadclan->motto);
 STRFREE(deadclan->description);
 STRFREE(deadclan->leader); 
 STRFREE(deadclan->number1);
 STRFREE(deadclan->number2);
 STRFREE(deadclan->number3);
 STRFREE(deadclan->number4);
 STRFREE(deadclan->filename); 
 STRFREE(deadclan->email);
 write_clan_list( );
 return;
}


void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan=NULL; 
    CHAR_DATA *sch;
    HELP_DATA *pHelp;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( IS_IMMORTAL(ch ) ||
    ( ch->pcdata->clan &&
    ( !strcasecmp( ch->name, ch->pcdata->clan->leader  )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number1 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number2 ) 
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number3 ) 
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number4 ) ) ) )
        ;
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if( ch->desc && ch->pcdata->clan)
    {
     switch( ch->substate )
     {
       char *pt;
        default:
          break;
        case SUB_HELP_EDIT:
          if ( (pHelp = ch->dest_buf) == NULL )
          {
                bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
                stop_editing( ch );
                return;
          }
          STRFREE (pHelp->text );
          pt = copy_buffer( ch );
          pHelp->text = STRALLOC(pt );
          STRFREE (pt );
          stop_editing( ch );
	  castle_needs_saving=TRUE;
          return;

       case SUB_CLAN_DESC:
        STRFREE (ch->pcdata->clan->description );
        ch->pcdata->clan->description = copy_buffer( ch );
        stop_editing( ch );
        save_clan( ch->pcdata->clan );
        return;
     }
    }
    smash_tilde(argument);
    argument = one_argument( argument, arg1 );

    if (IS_IMMORTAL(ch))
     argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        if (IS_IMMORTAL(ch))
        {
          send_to_char( "Usage: setclan <clan> <field> <leader|number1|number2> <player>\n\r", ch );
          send_to_char( "\n\rField being one of:\n\r", ch );
          send_to_char( " leader number1 number2 number3 number4 members home store\n\r", ch);
          send_to_char( " obj1 obj2 guard healer motto desc email founder type\n\r", ch );
        }
  	else send_to_char( "Usage: setclan <motto|desc|email|tax|storeroom>\n\r", ch );
        return;
    }

    if (IS_IMMORTAL(ch))
     clan = get_clan( arg1 );
    else
     clan = ch->pcdata->clan;

    if ( !clan )
    {
        send_to_char( "No such clan.\n\r", ch );
        return;
    }

    if (( IS_IMMORTAL (ch) &&!strcasecmp( arg2, "motto" ) )
 	|| !strcasecmp( arg1, "motto" ) )
    {
        STRFREE (clan->motto );
        clan->motto = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if (( IS_IMMORTAL (ch) &&!strcasecmp( arg2, "email" ) )
 	|| !strcasecmp( arg1, "email" ) )
    {
        STRFREE (clan->email );
        clan->email = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if (( IS_IMMORTAL (ch) &&!strcasecmp( arg2, "storeroom" ) )
 	|| !strcasecmp( arg1, "storeroom" ) )
    {
     if(!IS_IMMORTAL(ch))
     {
       if (ch->pcdata->castle==NULL)
       {
        send_to_char("You have no castle established yet.\n\r", ch);
        return;
       }
       if (ch->in_room->creator_pvnum!=ch->pcdata->pvnum)
       {
        send_to_char("You are NOT in your castle!\n\r", ch);
        return;
       }
     }
    if (clan->store != 0 && get_room_index(clan->store)!=NULL )
	REMOVE_BIT(get_room_index(clan->store)->room_flags, ROOM_CLAN_DONATION);

	SET_BIT(ch->in_room->room_flags,ROOM_CLAN_DONATION);
        clan->store = ch->in_room->vnum;
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if (( IS_IMMORTAL (ch) &&!strcasecmp( arg2, "desc" ) )
 	|| !strcasecmp( arg1, "desc" ) )
    {
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
           ch->substate = SUB_CLAN_DESC;
           start_editing( ch, clan->description );
           return;
        }
    }
/*
    if (( IS_IMMORTAL (ch) &&!strcasecmp( arg2, "doctrine" ) )
 	|| !strcasecmp( arg1, "doctrine" ) )
    {
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
           if ( (pHelp = get_help(ch, ch->pcdata->clan_name)) == NULL )
           {
               send_to_char( "You don't yet have a doctrine to edit.\n\r", ch );
               return;
           }
           ch->substate = SUB_HELP_EDIT;
           ch->dest_buf = pHelp;
           start_editing( ch, pHelp->text );
 	   return;
          }
     }
*/
    if (( IS_IMMORTAL (ch) && !strcasecmp( arg2, "tax" ) )
 	|| !strcasecmp( arg1, "tax" ) )
    {
 	if (atoi(argument) < 0 || atoi(argument) > 100)
	{
	 ch_printf(ch, "Tax rate must be between 0 and 100%%.\n\r");
	 return;
	}
        clan->tax = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !IS_IMMORTAL ( ch ) )
    {
      do_setclan( ch, "" );
      return;
    }

    if ( !strcasecmp( arg2, "leader" ) )
    {
        if ((sch=lookup_char( argument )) == NULL)
         {
 	   ch_printf(ch, "%s is not logged on right now.\n\r", argument);
	   return;
         }
 	if (sch->pcdata->clan == NULL)
	{
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
 	else 
	{
	 sch->pcdata->clan->members--;
	 if ( !strcasecmp(sch->pcdata->clan->number1, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number1);
	  sch->pcdata->clan->number1 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number2, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number2);
	  sch->pcdata->clan->number2 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number3, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number3);
	  sch->pcdata->clan->number3 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number4, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number4);
	  sch->pcdata->clan->number4 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->leader, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->leader);
	  sch->pcdata->clan->leader = STRALLOC("");
	 }
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
        STRFREE (clan->leader );
        clan->leader = UPPER_ALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "number1" ) )
    {
        if ((sch=lookup_char( argument )) == NULL)
         {
 	   ch_printf(ch, "%s is not logged on right now.\n\r", argument);
	   return;
         }
 	if (sch->pcdata->clan == NULL)
	{
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
 	else 
	{
	 sch->pcdata->clan->members--;
	 if ( !strcasecmp(sch->pcdata->clan->number1, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number1);
	  sch->pcdata->clan->number1 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number2, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number2);
	  sch->pcdata->clan->number2 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number3, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number3);
	  sch->pcdata->clan->number3 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number4, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number4);
	  sch->pcdata->clan->number4 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->leader, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->leader);
	  sch->pcdata->clan->leader = STRALLOC("");
	 }
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
        STRFREE (clan->number1 );
        clan->number1 = UPPER_ALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "number2" ) )
    {
        if ((sch=lookup_char( argument )) == NULL)
         {
 	   ch_printf(ch, "%s is not logged on right now.\n\r", argument);
	   return;
         }
 	if (sch->pcdata->clan == NULL)
	{
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
 	else 
	{
	 sch->pcdata->clan->members--;
	 if ( !strcasecmp(sch->pcdata->clan->number1, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number1);
	  sch->pcdata->clan->number1 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number2, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number2);
	  sch->pcdata->clan->number2 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number3, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number3);
	  sch->pcdata->clan->number3 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number4, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number4);
	  sch->pcdata->clan->number4 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->leader, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->leader);
	  sch->pcdata->clan->leader = STRALLOC("");
	 }
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }

        STRFREE (clan->number2 );
        clan->number2 = UPPER_ALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcasecmp( arg2, "number3" ) )
    {
        if ((sch=lookup_char( argument )) == NULL)
         {
 	   ch_printf(ch, "%s is not logged on right now.\n\r", argument);
	   return;
         }
 	if (sch->pcdata->clan == NULL)
	{
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
 	else 
	{
	 sch->pcdata->clan->members--;
	 if ( !strcasecmp(sch->pcdata->clan->number1, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number1);
	  sch->pcdata->clan->number1 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number2, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number2);
	  sch->pcdata->clan->number2 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number3, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number3);
	  sch->pcdata->clan->number3 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->leader, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->leader);
	  sch->pcdata->clan->leader = STRALLOC("");
	 }
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
        STRFREE (clan->number3 );
        clan->number3 = UPPER_ALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "number4" ) )
    {
        if ((sch=lookup_char( argument )) == NULL)
         {
 	   ch_printf(ch, "%s is not logged on right now.\n\r", argument);
	   return;
         }
 	if (sch->pcdata->clan == NULL)
	{
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
 	else 
	{
	 sch->pcdata->clan->members--;
	 if ( !strcasecmp(sch->pcdata->clan->number1, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number1);
	  sch->pcdata->clan->number1 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number2, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number2);
	  sch->pcdata->clan->number2 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number3, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number3);
	  sch->pcdata->clan->number3 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->number4, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->number4);
	  sch->pcdata->clan->number4 = STRALLOC("");
	 }
	 if ( !strcasecmp(sch->pcdata->clan->leader, ch->name))
	 {
 	  STRFREE (sch->pcdata->clan->leader);
	  sch->pcdata->clan->leader = STRALLOC("");
	 }
         STRFREE (sch->pcdata->clan_name);
         sch->pcdata->clan_name = QUICKLINK ( clan->name );
         sch->pcdata->clan      = clan;
         clan->members++;
         save_char_obj( sch, NORMAL_SAVE );
        }
        STRFREE (clan->number4 );
        clan->number4 = UPPER_ALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "members" ) )
    {
        clan->members = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcasecmp( arg2, "home" ) )
    {
        clan->home = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "founder" ) )
    {
        clan->founder_pvnum = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "type" ) )
    {
        clan->type = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "store" ) )
    {
        clan->store= atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcasecmp( arg2, "obj1" ) )
    {
        clan->clanobj1 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "obj2" ) )
    {
        clan->clanobj2 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "guard" ) )
    {
        clan->guard= atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcasecmp( arg2, "healer" ) )
    {
        clan->healer = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    do_setclan( ch, "" );
    return;
}

void do_donate(CHAR_DATA *ch, char *argument )
{
 OBJ_DATA *obj;
 ROOM_INDEX_DATA *location;
 CHAR_DATA *rch;

 if ( IS_NPC( ch ) || !ch->pcdata->clan )
 {
  send_to_char( "Huh?\n\r", ch );
  return;
 }
 if (ch->fighting !=NULL)
 {
  send_to_char( "You can't donate while fighting!\n\r", ch);
  return;
 }
 if ( ch->pcdata->clan->store == 0 ||
    ((location = get_room_index(ch->pcdata->clan->store))== NULL))
 {
  send_to_char("Your clan does not have a storeroom to recieve donated items.\n\r", ch);
  return;
 }
 
 if ( (obj = get_obj_carry( ch, argument ) ) == NULL)
 {
  send_to_char("What would you like to donate ?\n\r", ch);
  return;
 }
    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
    {
        send_to_char( "You can't seem to let go of it.\n\r", ch );
        return;
    }
    obj_from_char( obj );
    obj_to_room(obj, location);
    ch_printf(ch, "You chant the words to one of %s's rituals and watch the %s slowly disappear...\n\r", ch->pcdata->clan->name, obj->short_descr);
    for (rch=location->first_person;rch != NULL;rch=rch->next_in_room)
     if (can_see(rch, ch) && rch!=ch && rch->position>POS_SLEEPING)
      ch_printf(rch, "Before your very eyes, a %s slowly appears.\n\r", obj->short_descr);
 return;
}

void do_coffer( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *target;
  char choice[MAX_STRING_LENGTH], amt[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int  amount, tar_max;
  bool leader = FALSE;

  if ( IS_NPC(ch) || ch->pcdata->clan == NULL)
    {
      send_to_char("You are not part of a clan!\n\r", ch);
      return;
    }
  if ( !IS_SET(ch->in_room->room_flags, ROOM_BANK) && ch->in_room->vnum != ch->pcdata->clan->home)
    {
      send_to_char("You must be in a bank or the home of your clan to access the coffers.\n\r", ch);
      return;
    }

  argument=one_argument(argument, choice);
  argument=one_argument(argument, amt);
  amount  =atoi(amt);
  if(amount < 0)
    amount = 0 - amount;

    if ( !strcasecmp( ch->name, ch->pcdata->clan->leader  )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number1 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number2 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number3 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number4 ) )
       leader=TRUE;
 
  switch(*choice)
    {
    case 'd': case 'D':
      if(amount > ch->gold)
        {
          send_to_char("You don't have that much gold.\n\r", ch);
          return;
        }
      else
        {
	   if(ch->pcdata->clan->coffers+amount>2000000000)
  	   {
 	    ch_printf(ch, "The clan coffers are full!\n\r" );
  	    return;
 	   }
            ch->gold -= amount;
            ch->pcdata->clan->coffers += amount;
            save_char_obj( ch, NORMAL_SAVE );
	    save_clan (ch->pcdata->clan);
            ch_printf(ch, "You deposit %d gold coins into your clan's coffers.\n\r", amount);
            return;
        }
      break;

    case 'w': case 'W':
	if (!leader)
	{
	 send_to_char("Only clan leaders can withdraw from the clan coffers.\n\r", ch);
	 return;
	}

      if(amount > ch->pcdata->clan->coffers)
        {
          send_to_char("There is not enough gold in the clan coffers.\n\r",
                       ch);
          return;
        }
        else
        {
            ch->gold += amount;
            ch->pcdata->clan->coffers -= amount;
            save_char_obj( ch, NORMAL_SAVE );
	    save_clan (ch->pcdata->clan);
            ch_printf(ch, "You withdraw %d gold coins from the clan coffers.\n\r", amount);
          }
      break;

    case 'b': case 'B':
	if (!leader)
	{
	 send_to_char("Only clan leaders can check the coffers.\n\r", ch);
	 return;
	}
      else
      ch_printf(ch, "The coffers currently hold %d.\n\r", ch->pcdata->clan->coffers);
      break;

    case 't': case 'T':
	if (!leader)
	{
	 send_to_char("Only clan leaders can organize transfers from the coffers.\n\r", ch);
	 return;
	}
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
      if(ch->pcdata->clan->coffers < amount)
	{
	  send_to_char("There isn't enough gold in the coffers!\n\r", ch);
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
      ch->pcdata->clan->coffers -= amount;
      target->pcdata->account += amount;
      save_clan(ch->pcdata->clan);
      save_char_obj( ch, NORMAL_SAVE );
      save_char_obj( target, NORMAL_SAVE );
      ch_printf(ch, "You transfer %d coins into %s's account.\n\rThe coffers now hold %d gold coins.\n\r", amount, target->name, ch->pcdata->clan->coffers);
      ch_printf(target, "%s transfers %d coins into your account.\n\rYour account balance is now %d gold coins.\n\r", ch->name, amount, target->pcdata->account);
      break;

    default:
      if (!leader)
        send_to_char("Usage: coffer deposit [amount]\n\r", ch);
      else
        send_to_char("Usage: coffer <deposit/withdraw/balance/transfer> [amount] [target]\n\r", ch);
      break;
    }  
  return; 
}

void send_clan_message(CLAN_DATA *clan, char *message)
{
 NOTE_DATA *pnote;
 char buf[MAX_INPUT_LENGTH], *strtime;
  
 CREATE( pnote, NOTE_DATA, 1);
 pnote->next = NULL;
 pnote->sender = STRALLOC( "Mortal Realms Message Service");
 strtime                         = ctime( &current_time );
 strtime[strlen(strtime)-1]      = '\0';
 pnote->date                 = STRALLOC( strtime );
 pnote->time                 = current_time;
 
 buf[0]='\0';
 if (clan->leader[0] != '\0')
 {
  strcat(buf, clan->leader);
  strcat(buf, " ");
 }

 if (clan->number1[0] != '\0')
 {
  strcat(buf, clan->number1);
  strcat(buf, " ");
 }

 if (clan->number2[0] != '\0')
 {
  strcat(buf, clan->number2);
  strcat(buf, " ");
 }

 if (clan->number3[0] != '\0')
 {
  strcat(buf, clan->number3);
  strcat(buf, " ");
 }

 if (clan->number4[0] != '\0')
 {
  strcat(buf, clan->number4);
  strcat(buf, " ");
 }
 
 pnote->to_list = STRALLOC(buf);
 pnote->subject = STRALLOC("Important!");
 pnote->room_vnum = clan->home;
 pnote->text= STRALLOC(message);
 LINK (pnote, first_note, last_note, next, prev);
}

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int sn;
    char *words;

    /* check for healer */
    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_CLAN_HEALER) )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);
    if (arg[0] == '\0')
    {
        /* display price list */
        act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
        send_to_char("  heal: cure critical wounds\n\r",ch);
        send_to_char("  blind: cure blindness\n\r",ch);
        send_to_char("  poison:  cure poison\n\r",ch);
        send_to_char("  curse: remove curse\n\r",ch);
        send_to_char("  refresh: restore movement\n\r",ch);
        send_to_char("  feast: provide food and drink\n\r",ch);
        send_to_char("  eheal: enhanced healing\n\r",ch);
        send_to_char("  giant: giant strength\n\r",ch);
        send_to_char("  erest: enhanced resting\n\r",ch);
        send_to_char("Type heal <type> to be healed.\n\r",ch);
        return;
    }


    if (!str_prefix(arg,"critical"))
    {
        sn    = skill_lookup("cure critical");
        words = "judicandus qfuhuqar";
    }
    else if (!str_prefix(arg,"blindness"))
    {
        sn    = skill_lookup("cure blindness");
        words = "judicandus noselacri";
    }

    else if (!str_prefix(arg,"poison"))
    {
        sn    = skill_lookup("cure poison");
        words = "judicandus sausabru";
    }

    else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
        sn    = skill_lookup("remove curse");
        words = "candussido judifgz";
    }

    else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
        sn    = skill_lookup("refresh");
        words = "candusima";
    }
    else if (!str_prefix(arg,"feast"))
    {
        sn    = skill_lookup("feast");
        words = "yzagh";
    }
    else if (!str_prefix(arg,"eheal"))
    {
        sn    = skill_lookup("enhanced heal");
        words = "unsopaiqze pzar";
    }
    else if (!str_prefix(arg,"erest"))
    {
        sn    = skill_lookup("enhanced rest");
        words = "unsopaiqze candusgh";
    }
    else if (!str_prefix(arg,"giant"))
    {
        sn    = skill_lookup("giant strength");
        words = "ouaih ghcandusiohp";
    }

    else
    {
        act("$N says 'Type 'heal' for a list of spells.'",
            ch,NULL,mob,TO_CHAR);
        return;
    }


    WAIT_STATE(ch,PULSE_VIOLENCE);

    act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);

     if (sn == -1)
        return;

     (*skill_table[sn].spell_fun)(sn,mob->level,mob,ch);
}
