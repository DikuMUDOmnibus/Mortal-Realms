/***************************************************************************
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

#define ABORT_ON_ERROR FALSE
#define MEMORY_DEBUG
#define USE_48_RAND

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
/* #include <strings.h> */
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#include "merc.h"

#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

extern int port,control; /* db.c */

FILE *fpReserve;
FILE *fpAppend;

void remove_objects( void );

int objects_adjusted;

/*
 * Globals.
 */
HELP_DATA *             help_first;
HELP_DATA *             help_last;

SHOP_DATA *             shop_first;
SHOP_DATA *             shop_last;

OBJ_DATA *              first_object;
OBJ_DATA *              last_object;

CHAR_DATA *             first_char;
CHAR_DATA *             last_char;

OWNER_DATA *            first_owner;
OWNER_DATA *            last_owner;

NOTE_DATA *             first_note;
NOTE_DATA *             last_note;

PLAYER_GAME *     	first_player;
PLAYER_GAME *     	last_player;

BOUNTY_DATA *           first_bounty;
BOUNTY_DATA *           last_bounty;

char                    bug_buf         [3*MAX_STRING_LENGTH/2];
char                    log_buf         [3*MAX_STRING_LENGTH/2];
KILL_DATA               kill_table      [MAX_LEVEL];
TIME_INFO_DATA          time_info;
USAGE_DATA              usage;
SWEAR_DATA *            swear_list;
struct tm tme;


sh_int                  gsn_backstab;
sh_int                  gsn_dodge;
sh_int                  gsn_hide;
sh_int                  gsn_peek;
sh_int                  gsn_pick_lock;
sh_int                  gsn_sneak;
sh_int                  gsn_steal;
sh_int                  gsn_climb;

sh_int                  gsn_disarm;
sh_int                  gsn_enhanced_damage;
sh_int                  gsn_kick;
sh_int                  gsn_martial_arts;
sh_int                  gsn_parry;
sh_int                  gsn_rescue;
sh_int                  gsn_second_attack;
sh_int                  gsn_third_attack;
sh_int                  gsn_fourth_attack;
sh_int                  gsn_shoot;
sh_int                  gsn_throw;
sh_int                  gsn_short_range;
sh_int                  gsn_med_range;
sh_int                  gsn_long_range;
sh_int                  gsn_shadow;
sh_int                  gsn_divert;
sh_int                  gsn_voice;
sh_int                  gsn_bargain;
sh_int                  gsn_lock;
sh_int                  gsn_knife;
sh_int                  gsn_make_poison;
sh_int                  gsn_greater_hide;
sh_int                  gsn_greater_sneak;
sh_int                  gsn_stealth;
sh_int                  gsn_disguise;
sh_int                  gsn_spy;
sh_int                  gsn_attack;
sh_int                  gsn_armor_usage;
sh_int                  gsn_distract;
sh_int                  gsn_greater_peek;
sh_int                  gsn_snatch;
sh_int                  gsn_greater_pick;
sh_int                  gsn_track;
sh_int                  gsn_clear_path;
sh_int                  gsn_hunt;
sh_int                  gsn_notice;
sh_int                  gsn_rearm;

sh_int                  gsn_blindness;
sh_int                  gsn_charm_person;
sh_int                  gsn_curse;
sh_int                  gsn_invis;
sh_int                  gsn_mass_invis;
sh_int                  gsn_poison;
sh_int                  gsn_mage_blast;
sh_int                  gsn_sleep;
sh_int                  gsn_write_spell;
sh_int                  gsn_stability;
sh_int                  gsn_zap;
sh_int                  gsn_assassinate;
sh_int                  gsn_glance;
sh_int                  gsn_guard;
sh_int                  gsn_camp;
sh_int                  gsn_mass;
sh_int                  gsn_plant;
sh_int                  gsn_berserk;
sh_int                  gsn_forge;
sh_int                  gsn_repair;
sh_int                  gsn_detect_forgery;
sh_int                  gsn_critical_hit;

sh_int  		gsn_greater_backstab;
sh_int  		gsn_gouge;
sh_int  		gsn_circle;
sh_int  		gsn_trip;
sh_int  		gsn_anatomy;

sh_int  		gsn_pass_without_trace;
sh_int  		gsn_quick_draw;
sh_int  		gsn_flash_powder;
sh_int  		gsn_greater_stealth;
sh_int  		gsn_muffle;

sh_int			gsn_improved_invis;
sh_int			gsn_truesight;
sh_int			gsn_nightmare;
sh_int  		gsn_anti_magic_shell;
sh_int  		gsn_slow;
sh_int  		gsn_fire_shield;
sh_int  		gsn_vampiric_touch;

sh_int  		gsn_bashdoor;
sh_int  		gsn_greater_hunt;
sh_int  		gsn_greater_track;
sh_int  		gsn_bash;
sh_int  		gsn_dual_wield;

/* locals defined as globals to allow creating new rooms and exits */
int                     top_exit;
int                     top_room;
ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH];
int                     highest_vnum;



/*
 * Locals.
 */
OBJ_INDEX_DATA *        obj_index_hash          [MAX_KEY_HASH];
MOB_INDEX_DATA *        mob_index_hash          [MAX_KEY_HASH];

AREA_DATA *             first_area;
AREA_DATA *             last_area;
AREA_DATA *             first_area_name;
AREA_DATA *             last_area_name;
AREA_DATA *             area_load;


int                     top_affect;
int                     top_area;
int                     top_ed;
int                     top_help;
int                     top_mob_index;
int                     top_obj_index;
int                     top_reset;
int                     top_shop;

char    old_fread_word[MAX_INPUT_LENGTH];
void    create_menu_tree();

int     mob_counter;  /* counts each mobile for total created */


int find_command( char * );

/*
 * Memory management.
 * Tune the others only if you understand what you're doing.
 */
#define                 MAX_PERM_BLOCK  524200
#define MAX_MEM_LIST     27
const int  rgSizeList   [MAX_MEM_LIST]  =
{
    8, 16, 24, 32, 48, 64, 96, 138, 192, 256, 384, 512,
    768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384,
    24591, 32768, 49152, 60010, 70000
};

int perm_block_total;

void *  alloc_perm      ( int sMem );


typedef struct  perm_block_list    PERM_BLOCK_LIST;
struct perm_block_list
  {
  PERM_BLOCK_LIST * next;
  int               iMemPerm;
  char            * pMemPerm;
  unsigned char     block_number;
  int               total_used;
  };
PERM_BLOCK_LIST *perm_block_list_list;

PERM_BLOCK_LIST *perm_block_index[ 255 ];  /* Maximum perm blocks of 255 */

typedef struct  free_mem_list    FREE_MEM_LIST;
struct free_mem_list
  {
  FREE_MEM_LIST * next;
  };

FREE_MEM_LIST *rgFreeList       [MAX_MEM_LIST];
FREE_MEM_LIST *rgFreeUsed; /* Linked list of recycled rgFreeList */

int rgFreeCount[MAX_MEM_LIST];  /* count of freed memory */
int rgUsedCount[MAX_MEM_LIST];  /* count of used freeable memory */

int                     nAllocPerm;
int                     sAllocPerm;


/*
 * Semi-locals.
 */
bool                    fBootDb;
FILE *                  fpArea;
FILE *                  fpAdjusted;
char                    strArea[MAX_INPUT_LENGTH];


int BLOCKS_REMOVED;

/*
 * Local booting procedures.
 */
void    init_mm          args( ( void ) );

void    load_area        args( ( FILE *fp ) );
void    load_army        args( ( FILE *fp ) );
void    load_helps       args( ( FILE *fp ) );
void    load_mobiles     args( ( FILE *fp ) );
void    load_objects     args( ( FILE *fp ) );
void    load_resets      args( ( FILE *fp ) );
void    load_rooms       args( ( FILE *fp ) );
void    load_shops       args( ( FILE *fp ) );
void    load_specials    args( ( FILE *fp ) );
void    load_ranges      args( ( FILE *fp ) );
void    load_resetmsg    args( ( FILE *fp ) );
void    load_authors     args( ( FILE *fp ) );
void 	load_flags	 args( ( FILE *fp ) );
void    load_temperature args( ( FILE *fp ) );
void    load_notes       args( ( void ) );
void    load_swears      args( ( void ) );
OBJ_PROG *load_object_program   args( ( FILE *fp ) );

void obj_prog_if_dest( OBJ_INDEX_DATA * );

void     expand_mob_prog( MOB_INDEX_DATA * , MPROG_DATA *);
char     expand_line_mprog( MOB_INDEX_DATA * , MPROG_DATA *, char *,bool,char);

char *load_picture( FILE *);

void    fix_exits       args( ( void ) );
void    fix_object_levels       args( ( void ) );

/*
 * MOBprogram locals
 */

int             mprog_name_to_type      args ( ( char* name ) );
MPROG_DATA *    mprog_file_read         args ( ( char* f, MPROG_DATA* mprg, 
						MOB_INDEX_DATA *pMobIndex ) );
void            load_mobprogs           args ( ( FILE* fp ) );
void            mprog_read_programs     args ( ( FILE* fp,
						MOB_INDEX_DATA *pMobIndex ) );




/*
 * Big mama top level function.
 */
void boot_db( bool fCopyOver )
{
  CHAR_DATA *fch;
  int armor;
    /*
     * Init some data space stuff.
     */
  IS_BOOTING=TRUE;
  first_player = NULL;
  for( armor=0; armor<MAX_VNUM; armor++)  /* clear room indexes */
    {
    room_index[armor]=NULL;
    obj_index[armor]=NULL;
    mob_index[armor]=NULL;
    }

	fBootDb         = TRUE;
  	total_objects=0;
  	total_mobiles=0;
  	total_characters=0;
	first_note   	= NULL;
	last_note    	= NULL;
	first_object 	= NULL;
	last_object  	= NULL;
	first_char   	= NULL;
	last_char    	= NULL;
	first_owner  	= NULL;
	last_owner   	= NULL;
	shop_first   	= NULL;
	shop_last    	= NULL;
	first_area   	= NULL;
	last_area    	= NULL;
	first_area_name = NULL;
	last_area_name  = NULL;
	help_first   	= NULL;
	help_last    	= NULL;
        first_player 	= NULL;
	last_player  	= NULL;

    /*
     * Init random number generator.
     */
    {
	init_mm( );
    }
    load_clans();
    load_owners();
    /*
     * Load the swear words.  Presto 7/98
    {
      swear_list = NULL;
      load_swears();
    }
     */

    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;
        AREA_DATA *parea;
        int rvnum;

	lhour           = (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour  = lhour  % 24;
	lday            = lhour  / 24;
	time_info.day   = lday   % 35;
	lmonth          = lday   / 35;
	time_info.month = lmonth % 17;
	time_info.year  = lmonth / 17;

        parea = NULL;
        for( rvnum=1; rvnum < MAX_VNUM; rvnum += 100)
          if( room_index[rvnum]!=NULL && room_index[rvnum]->area != parea )
          {
            parea = room_index[rvnum]->area;
	    if ( time_info.hour <  5 ) 
                 parea->weather_info.sunlight = SUN_DARK;
	    else if ( time_info.hour <  6 )
                  parea->weather_info.sunlight = SUN_RISE;
	    else if ( time_info.hour < 19 )
                  parea->weather_info.sunlight = SUN_LIGHT;
	    else if ( time_info.hour < 20 )
                  parea->weather_info.sunlight = SUN_SET;
	    else
                  parea->weather_info.sunlight = SUN_DARK;

	parea->weather_info.change     = 0;
	parea->weather_info.mmhg       = 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    parea->weather_info.mmhg += number_range( 1, 50 );
	else
	    parea->weather_info.mmhg += number_range( 1, 80 );

	     if ( parea->weather_info.mmhg <=  970 ) 
                parea->weather_info.sky = SKY_LIGHTNING;
	else if ( parea->weather_info.mmhg <=  990 )
                parea->weather_info.sky = SKY_RAINING;
	else if ( parea->weather_info.mmhg <= 1010 )
                parea->weather_info.sky = SKY_CLOUDY;
	else  
                parea->weather_info.sky = SKY_CLOUDLESS;
          }

    }

    /*
     * Assign gsn's for skills which have them.
     */
    {
	  int sn;

	  for ( sn = 0; sn < MAX_SKILL; sn++ )
	    {
	    if ( skill_table[sn].pgsn != NULL )
		    *skill_table[sn].pgsn = sn;
	    }
    }

    /*   Assign command numbers to each command    */
    {
    int cmd;

    for ( cmd = 0; *cmd_table[cmd].name != '\0'; cmd++ )
	if ( cmd_table[cmd].cmd_number != NULL )
          *cmd_table[cmd].cmd_number = cmd;
    }
/*
   if( TEST_GAME )
     {
     extern bool wizlock;
     wizlock = TRUE;
     }
*/
    /*
     * give gsn's to cmds that need them (fatigy cmds like "kick")
     */
    {
	  int sn,cmd;

    for(cmd=0;cmd_table[cmd].do_fun!=NULL;cmd++)
      {
      cmd_gsn[cmd]=-1;
	    for ( sn = 0; sn < MAX_SKILL; sn++ )
	      {
	      if ((skill_table[sn].cmd_name!=NULL)
	    && !strcasecmp(cmd_table[cmd].name,skill_table[sn].cmd_name))
	  {
		      cmd_gsn[cmd]=sn;
	  break;
	  }
	      }
      }
    }


    /*
     * give proper command numbers to variables -Dug
     */
    obj_get_cmds();

    /*
     * Read in all the area files.
     */
  {
	FILE *fpList;


	if ( ( fpList = fopen( AREA_LIST, "r") ) == NULL )
	{
	    perror( AREA_LIST );
	    abort( );
	}

	if ( ( fpAdjusted = fopen( "adjusted.txt", "w" ) ) == NULL )
	{
	    perror( "adjusted.txt" );
	    abort( );
	}

	for ( ; ; )
	  {
	  char qbuf[180];
	  strcpy( strArea, fread_word( fpList ) );
    log_string( strArea );
    /* log_string( hash_stats("") );*/
	  if ( strArea[0] == '$' )
		  break;

	  if ( strArea[0] == '-' )
	    {
		  fpArea = stdin;
	    }
	  else
	    {
		  if ( ( fpArea = fopen( strArea, "r") ) == NULL )
        {
	      sprintf( qbuf, "%s.gz", strArea);
	      if ( ( fpArea = fopen( qbuf, "r") ) != NULL )
	         {
	         fclose( fpArea );
	         sprintf( qbuf, "gzip -dqf %s.gz", strArea );
	         system( qbuf );
		       if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
		         {
		         perror( strArea );
		         abort( );
		         }
	         }
        else
		      {
		      perror( strArea );
		      abort( );
		      }
        }
      }

  for ( ; ; )
    {
	char *word;

	if ( fread_letter( fpArea ) != '#' )
	{
	    bug( "Boot_db: # not found.", 0 );
	    abort( );
	}

	word = fread_word( fpArea );

	     if ( word[0] == '$'               )                 break;
	else if ( !strcasecmp( word, "AREA"     ) ) 
	       {
	       load_area    (fpArea);
	       area_load->filename = STRALLOC( strArea );
		    /* record file name for output */
	       }
	else if ( !strcasecmp( word, "HELPS"    ) ) load_helps   (fpArea);
	else if ( !strcasecmp( word, "ARMY"     ) ) load_army    (fpArea);
	else if ( !strcasecmp( word, "RANGES"   ) ) load_ranges  (fpArea);
	else if ( !strcasecmp( word, "AUTHORS"  ) ) load_authors (fpArea);
	else if ( !strcasecmp( word, "FLAGS"    ) ) load_flags   (fpArea);
	else if ( !strcasecmp( word, "RESETMSG" ) ) load_resetmsg(fpArea);
	else if ( !strcasecmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
	else if ( !strcasecmp( word, "MOBPROGS" ) ) load_mobprogs(fpArea);
	else if ( !strcasecmp( word, "OBJECTS"  ) ) load_objects (fpArea);
	else if ( !strcasecmp( word, "RESETS"   ) ) load_resets  (fpArea);
	else if ( !strcasecmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
	else if ( !strcasecmp( word, "SHOPS"    ) ) load_shops   (fpArea);
	else if ( !strcasecmp( word, "SPECIALS" ) ) load_specials(fpArea);
	else if ( !strcasecmp( word, "TEMPERATURE" ) ) load_temperature(fpArea);
   
	else
	{
	    bug( "Boot_db: bad section name.", 0 );
	    abort( );
	}
    }
	    if ( fpArea != stdin )
		{
		fclose( fpArea );
		}
	    fpArea = NULL;
	}
	fclose( fpList );
	fclose( fpAdjusted );
    }

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
	log_string("Fixing Exits");
	fix_exits( );
	fBootDb = TRUE;
	log_string("Updating Areas");
	area_update();
	fBootDb = FALSE;
	log_string("Loading Notes");
	load_notes( );
	log_string("Creating Menu Tree");
	create_menu_tree();
	log_string("Done creating Menu Tree");
	MOBtrigger=TRUE;

  /*  Learn the basic armor of all mobiles */
  for(fch=first_char; fch!=NULL; fch=fch->next)
    {
    armor=mob_armor( fch);
    if( armor> fch->pIndexData->armor)
      fch->pIndexData->armor=armor;
    }
  /*  Reset all armor onto mobiles */
  for(fch=first_char; fch!=NULL; fch=fch->next)
      fch->npcdata->armor=fch->pIndexData->armor;

  log_string("Fixing Object levels");
  fix_object_levels();
  IS_BOOTING=FALSE;

  /*remove_objects();   system to remove initial loads of objects */

  {  /* Clear out timers */
  int cnt;

  for( cnt=0; cnt<MAX_TIMERS; cnt++)
    {
    timers[cnt][0]=0;
    timers[cnt][1]=0;
    timers[cnt][2]=0;
    timers[cnt][3]=0;
    timers[cnt][4]=0;
    }
  }
  if (fCopyOver)
   {
    log_printf("Running copyover_recover.");
    copyover_recover();
   }
    log_string("Database booted!");
    return;
}


void remove_objects( void )
{
   OBJ_DATA  *obj, *onext;

   for( obj = first_object; obj!= NULL; obj=onext)
      {
      onext = obj->next;
      if( obj->carried_by == NULL  || obj->carried_by->pIndexData->pShop==NULL)
	 {
	 if( obj->carried_by != NULL )
	   obj_from_char( obj );
	 else
	 if( obj->in_obj != NULL )
	   obj_from_obj( obj );
	 else
	   obj_from_room( obj );
	 extract_obj( obj );
	 }
     }
  return;
}
 

/*
 * Snarf an 'area' header line.
 */
void    load_area       ( FILE *fp )
{
    AREA_DATA *pArea;

    CREATE(pArea, AREA_DATA, 1);
    pArea->first_reset  = NULL;
    pArea->last_reset   = NULL;
    pArea->name         = fread_string( fp );
    pArea->age          = 99;
    pArea->nplayer      = 0;
    pArea->low_r_vnum   = MAX_VNUM-1;
    pArea->low_o_vnum   = MAX_VNUM-1;
    pArea->low_m_vnum   = MAX_VNUM-1;
    pArea->hi_r_vnum    = 0;
    pArea->hi_o_vnum    = 0;
    pArea->hi_m_vnum    = 0;
    pArea->flags  	= 0;
    pArea->resetmsg	  = NULL;
    pArea->low_soft_range = 0;
    pArea->hi_soft_range  = MAX_LEVEL;
    pArea->low_hard_range = 0;
    pArea->hi_hard_range  = MAX_LEVEL;
    pArea->authors      = STRALLOC( "unknown" );
    pArea->weather_info.temp_winter = 20;
    pArea->weather_info.temp_summer = 80;
    pArea->weather_info.temp_daily = 15;
    pArea->weather_info.wet_scale = 5;

    LINK( pArea, first_area, last_area, next, prev );
    sort_area_by_name(pArea);     
    top_area++;
    area_load = pArea;
    return;
}

void load_temperature( FILE * fp )
{
  area_load->weather_info.temp_winter=fread_number(fp);
  area_load->weather_info.temp_summer=fread_number(fp);
  area_load->weather_info.temp_daily=fread_number(fp);
  area_load->weather_info.wet_scale=fread_number(fp);
  area_load->weather_info.wet_scale=
  UMAX( 0,area_load->weather_info.wet_scale );
  area_load->weather_info.wet_scale=
  UMIN(10,area_load->weather_info.wet_scale );
}
void load_flags( FILE *fp )
{
 area_load->flags = fread_number( fp );
 return;
}

void load_authors( FILE *fp )
{
    if ( !area_load )
    {
        bug( "Load_authors: no #AREA seen yet." );
        if ( fBootDb )
        {
          log_printf( "No #AREA" );
          exit( 1 );
        }
        else
          return;
    }

    if ( area_load->authors )
      STRFREE( area_load->authors );
    area_load->authors   = fread_string( fp );
    return;
}

void load_ranges( FILE *fp )
{
   area_load->low_soft_range = fread_number( fp );
   area_load->hi_soft_range  = fread_number( fp );
   area_load->low_hard_range = fread_number( fp );
   area_load->hi_hard_range  = fread_number( fp );

   /* Presto 3/10/99 */
   if(area_load->low_soft_range > area_load->hi_soft_range)
     bug("Low soft range is greater than high soft range.");
   if(area_load->low_hard_range > area_load->hi_hard_range)
     bug("Low hard range is greater than high hard range.");

   return;
}

void load_resetmsg( FILE *fp )
{
    if ( area_load->resetmsg )
        STRFREE( area_load->resetmsg );
    area_load->resetmsg = fread_string( fp );
    return;
}

void load_army( FILE *fp )
  {
  int cnt;

  for( cnt=0; cnt<MAX_RACE; cnt++)
    army_HQ[cnt] = fread_number( fp );

  return;
  }

/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;
    char *pt;

    for ( ; ; )
    {
 	CREATE(pHelp, HELP_DATA, 1);
	pHelp->level    = fread_number( fp );
	pHelp->keyword  = fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
        pt = fread_string( fp );
	pHelp->text  = STRALLOC( pt );
        STRFREE (pt ); 
	pHelp->area     = area_load;
	LINK( pHelp, help_first, help_last, next, prev );
	top_help++;
     }

    return;
}

void load_owners( void)
{
    char letter;
    FILE *fpList;
    OWNER_DATA * pOwner;
    first_owner = NULL;

    log_string( "Loading Owner list..." );

    fclose( fpReserve );
    if ( ( fpList = fopen( OWNER_LIST, "r") ) == NULL )
    {
        perror( OWNER_LIST );
        exit( 1 );
    }

    for ( ; ; )
    {
	letter = fread_letter( fpList );
        if (letter == '$')
	{
  	 fread_to_eol( fpList);
	 break;
	}
        CREATE( pOwner, OWNER_DATA, 1);
	pOwner->pvnum     = fread_number( fpList );
	pOwner->lastentry = fread_number( fpList );
	pOwner->name      = fread_string( fpList );
	LINK (pOwner, first_owner, last_owner, next, prev);

    }
    fclose( fpList );
    log_string("Done Owners" );
    fpReserve = fopen( NULL_FILE, "r" );

    return;
}
void save_owners( )
{
    OWNER_DATA *pOwner;
    FILE *fpout;
    fclose( fpReserve );

    fpout = fopen( OWNER_LIST_TMP, "w");
    if ( !fpout )
    {
        bug( "FATAL: cannot open owner.lst for writing!\n\r", 0 );
        return;
    }
    for ( pOwner = first_owner; pOwner!=NULL; pOwner = pOwner->next )
      {
        fprintf( fpout, "O %d %d %s~\n",  pOwner->pvnum, pOwner->lastentry, pOwner->name );
      }
    fprintf( fpout, "$\nXXXXXXXXXX\n#Ownerlist\n" );

    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fpout );
    if( is_valid_save( OWNER_LIST_TMP, "Ownerlist" ) )
      {
      remove (OWNER_LIST);
      rename (OWNER_LIST_TMP, OWNER_LIST);
      }
    fpReserve = fopen( NULL_FILE, "r");
}

/*
 * Add a character to the list of all characters
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}



/*
 * Snarf a mob section.
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char *pt;

    for ( ; ; )
    {
	int vnum;
	char letter;
	int iHash;

	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    abort( );
	}

	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( vnum<1 || vnum>=MAX_VNUM )
	{
	    bug( "Load_mobiles: vnum %u out of range.", vnum );
	    abort( );
	}
	if ( get_mob_index( vnum ) != NULL )
	{
	    bug( "Load_mobiles: vnum %u duplicated.", vnum );
	    abort( );
	}
	fBootDb = TRUE;

	CREATE(pMobIndex, MOB_INDEX_DATA, 1);
	pMobIndex->vnum                 = vnum;
	if ( fBootDb )
        {
          if ( area_load->low_m_vnum> vnum )
            area_load->low_m_vnum           = vnum;
          if ( vnum > area_load->hi_m_vnum )
            area_load->hi_m_vnum            = vnum;
        }
        mob_index[ vnum ] = pMobIndex;
	pMobIndex->area         = area_load;
        pt = fread_string( fp );
        if( pt == NULL || *pt == '\0' )
          pMobIndex->player_name = STRALLOC( "--" );
        else
          pMobIndex->player_name = STRALLOC(pt);
        STRFREE (pt );
        pt = fread_string( fp );
	pMobIndex->short_descr          = STRALLOC( pt );
        STRFREE (pt );
        pt = fread_string( fp );
	pMobIndex->long_descr          = UPPER_ALLOC( pt );
        STRFREE (pt );
        pt = fread_string( fp );
	pMobIndex->description  = UPPER_ALLOC(pt);
        STRFREE (pt );

	pMobIndex->act                  = fread_number( fp ) | ACT_IS_NPC;
	pMobIndex->affected_by          = fread_number( fp );
	pMobIndex->pShop                = NULL;
	pMobIndex->alignment            = fread_number( fp );
	letter                          = fread_letter( fp );
	pMobIndex->level                = fread_number( fp );

  if( pMobIndex->level > 150 )
    pMobIndex->level = 150;
  if( pMobIndex->level < 1 )
    pMobIndex->level = 1;

  pMobIndex->max_mobs = 0;
  pMobIndex->armor    = 0;

	/*
	 * The unused stuff is for imps who want to use the old-style
	 * stats-in-files method.
	 */
      if( IS_SET( pMobIndex->act, ACT_BODY ))
	{
	pMobIndex->body_parts           = fread_number( fp );   
	pMobIndex->attack_parts         = fread_number( fp );   
	}
      else
	{
		   fread_number( fp );
	pMobIndex->body_parts = 0;
		   fread_number( fp );
	pMobIndex->attack_parts = 0;
	}
	pMobIndex->hitnodice            = fread_number( fp );
	/* 'd'          */                fread_letter( fp );   
	pMobIndex->hitsizedice          = fread_number( fp );   
	/* '+'          */                fread_letter( fp );   
	pMobIndex->hitplus              = fread_number( fp );   
	pMobIndex->damnodice            = fread_number( fp );   
	/* 'd'          */                fread_letter( fp );
	pMobIndex->damsizedice          = fread_number( fp );   
	/* '+'          */                fread_letter( fp );   
	pMobIndex->damplus              = fread_number( fp );   
	pMobIndex->gold                 = fread_number( fp );   

        if( IS_SET( pMobIndex->act, ACT_RACE))
          pMobIndex->race = fread_number( fp );  /* Get Race */
        else
          {
	  /* xp can't be used! */           fread_number( fp );   /* Unused */
          pMobIndex->race = number_range( 0, MAX_RACE-1);
          }

  pMobIndex->position   =         fread_number( fp );   
	/* start pos    */                fread_number( fp );   /* Unused */
  if(pMobIndex->position>5 || pMobIndex->position<4)
     pMobIndex->position=7;
  pMobIndex->corrected=FALSE;

	/*
	 * Back to meaningful values.
	 */
	pMobIndex->sex                  = fread_number( fp );

	  /*  Chaos 4/13/94   Check if HP and DMG is totally out of whack  */
    {
    int ahp, adm, lev, shp, sdm, nhp, ndm;
    lev= pMobIndex->level;
    shp= lev*lev+lev+10;
    if((short int)shp < 0)
      shp=32000;
    sdm= lev+5;
    ahp= pMobIndex->hitnodice* pMobIndex->hitsizedice/2+ pMobIndex->hitplus;
    if((short int)ahp < 0)
      ahp=32000;
    adm= pMobIndex->damnodice* pMobIndex->damsizedice/2+ pMobIndex->damplus;
    if( ahp<shp/3 || ahp>shp*3)
	{  /*  bad average hp */
	nhp = (shp+ahp)/2;
	if( nhp<shp/3)
	   nhp=shp/3;
	if( nhp>shp*3)
	   nhp=shp*3;
        if(nhp > 32000)
          nhp=32000;
	pMobIndex->hitplus = number_fuzzy(nhp);
	pMobIndex->hitnodice = 1;
	pMobIndex->hitsizedice = 1;
	fprintf( fpAdjusted, "Hp on %s (lv%d) %d to %d\n",
	  pMobIndex->short_descr, lev, ahp, nhp);
	pMobIndex->corrected=TRUE;
	}
    if( adm<sdm/3 || adm>sdm*3)
	{  /*  bad average damage */
	pMobIndex->damnodice = 1;
	pMobIndex->damsizedice = 1;
	ndm = (sdm+adm)/2;
	if( ndm<sdm/3)
	   ndm=sdm/3;
	if( ndm>sdm*3)
	   ndm=sdm*3;
	pMobIndex->damplus = number_fuzzy( ndm);
	fprintf( fpAdjusted, "Dam on %s (lv%d) %d to %d\n",
	    pMobIndex->short_descr, lev, adm, ndm);
	pMobIndex->corrected=TRUE;
	}

    sdm=UMAX(pMobIndex->level*pMobIndex->level*50+pMobIndex->level*500+500, 0);
    adm=pMobIndex->gold;
    if( adm > sdm )
      {
      pMobIndex->gold = number_fuzzy( sdm );
      fprintf( fpAdjusted, "Gold on %s (lv%d) %d to %d\n",
	    pMobIndex->short_descr, lev, adm, sdm);
      pMobIndex->corrected=TRUE;
      }
    

/*  Old method for gold allowed creators to get outrageous  - Chaos 4/12/99
    sdm=UMAX(pMobIndex->level*pMobIndex->level*8+pMobIndex->level*2-5, 0);
    adm=pMobIndex->gold;
    if( adm>sdm*5)
	{  
	ndm = (sdm+adm)/2;
	if( ndm>sdm*5)
	   ndm=sdm*5;
	pMobIndex->gold = number_fuzzy( ndm );
	fprintf( fpAdjusted, "Gold on %s (lv%d) %d to %d\n",
	    pMobIndex->short_descr, lev, adm, ndm);
	pMobIndex->corrected=TRUE;
	}
*/
    }
	if ( letter != 'S' )
	{
	    bug( "Load_mobiles: vnum %u non-S.", vnum );
	    abort( );
	}

	letter = fread_letter( fp );
        if ( letter == 'D' )
              {
              pMobIndex->picture = load_picture( fp );
	      letter = fread_letter( fp );
              }
	else if ( letter == 'X' )
                 {
                 pMobIndex->mob_file = fread_string( fp );
	         letter = fread_letter( fp );
                 }
	ungetc(letter,fp);
	if(letter=='>')
	  mprog_read_programs(fp,pMobIndex);

	iHash                   = vnum % MAX_KEY_HASH;
	pMobIndex->next         = mob_index_hash[iHash];
	mob_index_hash[iHash]   = pMobIndex;
	top_mob_index++;
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

    return;
}



/*
 * Snarf an obj section.
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char buf[256], *pt;
    int est;

    pObjIndex = NULL;

    for ( ; ; )
    {
	int vnum;
	char letter;
	int iHash;

	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found.", 0 );
	    abort( );
	}

	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( vnum<1 || vnum>=MAX_VNUM )
	{
	    bug( "Load_objects: vnum %u out of range.", vnum );
	    abort( );
	}
	if ( get_obj_index( vnum ) != NULL )
	{
	    bug( "Load_objects: vnum %u duplicated.", vnum );
	    abort( );
	}
	fBootDb = TRUE;

 	CREATE(pObjIndex, OBJ_INDEX_DATA, 1);
	pObjIndex->vnum                 = vnum;
	if ( fBootDb )
        {
          if ( area_load->low_o_vnum > vnum)
            area_load->low_o_vnum           = vnum;
          if ( vnum > area_load->hi_o_vnum )
            area_load->hi_o_vnum            = vnum;
        }
        obj_index[ vnum ] = pObjIndex;
	pObjIndex->area                 = area_load;
            pt = fread_string( fp );  /* Add I### system to names */
            sprintf( buf, "i%u", vnum );
            if( strstr( pt, buf ) == NULL )
              sprintf( buf, "%s i%u", pt, vnum);
            else
              strcpy( buf, pt );
            STRFREE (pt );
            pObjIndex->name = STRALLOC( buf);
        pt = fread_string( fp );
	pObjIndex->short_descr          = LOWER_ALLOC( pt );
        STRFREE (pt );
        pt = fread_string( fp );
	pObjIndex->description  = UPPER_ALLOC( pt );
        STRFREE (pt );
        pt = fread_string( fp );
	pObjIndex->long_descr           = STRALLOC( pt);
        STRFREE(pt);

	pObjIndex->item_type            = fread_number( fp );
        if( pObjIndex->item_type < 1 )
	  pObjIndex->item_type = 13 ;
          
	pObjIndex->extra_flags          = fread_number( fp );
	pObjIndex->wear_flags           = fread_number( fp );
	pObjIndex->value[0]             = fread_number( fp );
	pObjIndex->value[1]             = fread_number( fp );
	pObjIndex->value[2]             = fread_number( fp );
	pObjIndex->value[3]             = fread_number( fp );
	pObjIndex->weight               = abs(fread_number( fp ));
	pObjIndex->cost                 = abs(fread_number( fp ));
	pObjIndex->level                = 0;
	pObjIndex->count                = 0;
  pObjIndex->max_objs = 1;
	pObjIndex->level_rent = fread_number( fp );

/* Why would we not want potion dropping? -Dug 12/4/93
	if ( pObjIndex->item_type == ITEM_POTION )
	    SET_BIT(pObjIndex->extra_flags, ITEM_NODROP); */

	for ( ; ; )
	{
	    char letter;

	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	      {
		    AFFECT_DATA *paf;
		    CREATE( paf, AFFECT_DATA, 1);
		    paf->type           = -1;
		    paf->duration               = -1;
		    paf->location               = fread_number( fp );
		    paf->modifier               = fread_number( fp );
		    paf->bitvector              = 0;
		    LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
                           next, prev );
	 	    SET_BIT(pObjIndex->extra_flags, ITEM_MAGIC);
		    top_affect++;
	      }

	    else if ( letter == 'C' )
	      {
                pt = fread_string( fp );
	        pObjIndex->attack_string = STRALLOC( pt );
                STRFREE( pt );
                pObjIndex->class_flags = fread_number( fp );
              }

            else if ( letter == 'D' )
              pObjIndex->picture = load_picture( fp );

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE(ed, EXTRA_DESCR_DATA, 1);
		ed->keyword             = fread_string( fp );
                pt = fread_string( fp );
	        ed->description  = UPPER_ALLOC( pt );
                STRFREE (pt );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                          next, prev );

		top_ed++;
	    }
	   
	    else if ( letter == 'X' )
                 {
                 pObjIndex->obj_file = fread_string( fp );
                 }
	    else if ( letter == 'P' )
		{
		OBJ_PROG *prg;  /* loads in forward order */
                if( pObjIndex->obj_prog == NULL)
                  {
		  pObjIndex->obj_prog       =  load_object_program( fp );
                  pObjIndex->obj_prog->next = NULL;
                  }
                else
                  {
                  for( prg=pObjIndex->obj_prog; prg->next!=NULL; prg=prg->next);
		  prg->next       = load_object_program( fp );
                  prg->next->next = NULL;
                  }
		}

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	    pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;
	}

    if( pObjIndex->obj_prog != NULL)
      obj_prog_if_dest( pObjIndex );

	iHash                   = vnum % MAX_KEY_HASH;
	pObjIndex->next         = obj_index_hash[iHash];
	obj_index_hash[iHash]   = pObjIndex;
	top_obj_index++;

    /* Object level corrector  - Chaos  1/18/96  */

    if( pObjIndex != NULL && IS_SET(pObjIndex->extra_flags, ITEM_LEVEL_RENT) &&
        pObjIndex->vnum != 51 && pObjIndex->vnum != 50 )
      {
      int old;
      est = obj_level_estimate( pObjIndex );
      if( pObjIndex->level_rent < 1 )
        pObjIndex->level_rent = 1;
      old = pObjIndex->level_rent;
      if( est < 1 )
        est = 1;
      if( pObjIndex->level_rent < ((est / 2) - 5) )
        {
        objects_adjusted++;
        pObjIndex->level_rent = ( pObjIndex->level_rent + est ) /2;
        /*sprintf(buf, "Adjusting item [%u]%s level %d/%d to %d", pObjIndex->vnum,
          pObjIndex->name, old, est, pObjIndex->level_rent );
        log_string( buf ); */
        }
      }

    }

    return;
}



/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
    RESET_DATA *pReset;
	  ROOM_INDEX_DATA *pRoomIndex;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    RESET_DATA *last_mob=NULL, *last_object=NULL;

    if ( area_load == NULL )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	abort( );
    }

    for ( ; ; )
    {
	EXIT_DATA *pexit;
	char letter;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}
	
	CREATE(pReset, RESET_DATA, 1);
	pReset->command = letter;
	pReset->arg0    = fread_number( fp );
	pReset->arg1    = fread_number( fp );
	pReset->arg2    = fread_number( fp );
	pReset->arg3    = (letter == 'G' || letter == 'R')
			    ? 0 : fread_number( fp );
	fread_to_eol( fp );

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
   	 * And counting occurances of things.
	 */
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    abort( );
	    break;

	case 'M':
	    pMob=get_mob_index  ( pReset->arg1 );
            if(pMob==NULL)
              {
	      bug( "Load_resets: bad mobile # '%d'.", pReset->arg1);
	      abort( );
              }
            pMob->max_mobs++;
	    get_room_index ( pReset->arg3 );
	    last_mob = pReset;
	    break;

	case 'O':
	    pObj=get_obj_index  ( pReset->arg1 );
            if( pObj == NULL )
	    {
		bug( "Load_resets: 'O': invalid index.", pReset->arg1 );
		abort( );
            }
      if(pReset->arg2>pObj->max_objs)
	pObj->max_objs=pReset->arg2;
	    get_room_index ( pReset->arg3 );
	    last_object = pReset;
	    break;

	case 'P':
	    pObj=get_obj_index  ( pReset->arg1 );
            if( pObj == NULL )
	    {
		bug( "Load_resets: 'P': invalid index.", pReset->arg1 );
		abort( );
            }
      if(pReset->arg2>pObj->max_objs)
	pObj->max_objs=pReset->arg2;
	    get_obj_index  ( pReset->arg3 );
	    break;

	case 'G':
	case 'E':
	    pObj=get_obj_index  ( pReset->arg1 );
            if( pObj == NULL )
	    {
		bug( "Load_resets: 'E': invalid index.", pReset->arg1 );
		abort( );
            }
      if(pReset->arg2>pObj->max_objs)
	pObj->max_objs=pReset->arg2;
	    break;

	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );
            if( pRoomIndex == NULL )
	    {
		bug( "Load_resets: 'E': invalid index.", pReset->arg1 );
		abort( );
            }

	    if ( pReset->arg2 < 0
	    ||   pReset->arg2 > 5
	    || ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL
	    || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );

		abort( );
	    }

	    if ( pReset->arg3 < 0 || pReset->arg3 > 2 )
	    {
		bug( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
		abort( );
	    }

	    break;

	case 'R':
	    pRoomIndex          = get_room_index( pReset->arg1 );
            if( pRoomIndex == NULL )
	    {
		bug( "Load_resets: 'R': invalid index.", pReset->arg1 );
		abort( );
            }

	    if ( pReset->arg2 < 0 || pReset->arg2 > 6 )
	    {
		bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
		abort( );
	    }

	    break;
	}

     LINK(pReset, area_load->first_reset, area_load->last_reset, next, prev);
     top_reset++;
    }

  /* if( TRUE )
    {
    char buf[MAX_STRING_LENGTH];
    int cnt;
    cnt=0;
    for( pReset = area_load->first_reset; pReset != NULL; pReset=pReset->next)
      cnt++;
    sprintf( buf, "Links in area: %d", cnt);
    log_string( buf );
    } */
    /* Set up container references     -   Chaos 5/30/95   */

  for( pReset = area_load->first_reset; pReset != NULL; pReset=pReset->next)
    {
     if( pReset->command == 'G' || pReset->command == 'E' )
      pReset->container = get_reset_from_mob( pReset, area_load->first_reset );
    else
     if( pReset->command == 'P' )
      pReset->container = get_reset_from_obj( pReset->arg3, pReset,
                          area_load->first_reset );
    }
    return;
}



/*
 * Load the Swear list
 */
void load_swears( void )
{
  SWEAR_DATA *pSwearIndex = NULL;
  FILE *fp;
  char buf[MAX_INPUT_LENGTH];
  int done=0;

  if(!(fp=fopen(SWEAR_FILE_T, "r")))
    return;

  while(done==0)
  {
    strcpy(buf, fread_word(fp));
    if(!strcasecmp(buf,"NULL"))
      done=1;
    else
    {
      CREATE(pSwearIndex, SWEAR_DATA, 1);
      pSwearIndex->next = swear_list;
      swear_list        = pSwearIndex;
      pSwearIndex->word = STRALLOC(buf);
    }
  }
  fclose(fp);
}

/*
 * Snarf a room section.
 * And counting occurances of things.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;
    int temp_val;
    char *pt;

    if ( area_load == NULL )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	abort( );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;
	int iHash;

	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    abort( );
	}

	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( vnum<0 || vnum >=MAX_VNUM )
	{
	    bug( "Load_rooms: vnum %u out of range.", vnum );
	    abort( );
	}
	if ( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %u duplicated.", vnum );
	    abort( );
	}
	fBootDb = TRUE;
	
	CREATE(pRoomIndex, ROOM_INDEX_DATA, 1);
	pRoomIndex->first_person        = NULL;
	pRoomIndex->last_person         = NULL;
	pRoomIndex->first_content       = NULL;
	pRoomIndex->last_content        = NULL;
	pRoomIndex->first_extradesc     = NULL;
	pRoomIndex->last_extradesc      = NULL;
        pRoomIndex->fall_room           = 0;
        pRoomIndex->distance_of_fall    = 0;
	pRoomIndex->content_count       = 0;
  	/* update first room in area */
	if ( fBootDb )
        {
          if ( area_load->low_r_vnum > vnum )
            area_load->low_r_vnum           = vnum;
          if ( vnum > area_load->hi_r_vnum )
            area_load->hi_r_vnum            = vnum;
        }

	pRoomIndex->area                = area_load;
	pRoomIndex->vnum                = vnum;
    room_index[vnum]=pRoomIndex;    /* Put room into index file */
        pt = fread_string( fp );
	pRoomIndex->name                = STRALLOC( pt );
        STRFREE (pt );
        pRoomIndex->owned_by  = 0;
        pt = fread_string( fp );
	pRoomIndex->description  = UPPER_ALLOC( pt );
        STRFREE (pt );
	/* Area number */
        temp_val                        = fread_number( fp );
	pRoomIndex->room_flags          = fread_number( fp );
        pRoomIndex->sanctify_timer 	= 0;
        pRoomIndex->hallucinate_timer 	= 0;
        pRoomIndex->smoke_timer 	= 0;
        pRoomIndex->sanctify_char 	= NULL;
        pRoomIndex->hallucinate_room 	= NULL;

        /* area number is redefined to be creator_pvnum for castles */
        if(IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE))
          pRoomIndex->creator_pvnum=temp_val;
        else
          pRoomIndex->creator_pvnum=0;

	pRoomIndex->sector_type         = fread_number( fp );
	if( pRoomIndex->sector_type < 0 || pRoomIndex->sector_type >= SECT_MAX)
	  pRoomIndex->sector_type         = 0;

	pRoomIndex->light               = 0;
	for ( door = 0; door <= 5; door++ )
	    pRoomIndex->exit[door] = NULL;
	  /* Chaos last left code resetting,  11/14/93  */
	for ( door = 0; door< MAX_LAST_LEFT; door++)
	    strcpy( pRoomIndex->last_left[door], "");

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		door = fread_number( fp );
		if ( door < 0 || door > 5 )
		{
		    bug( "Fread_rooms: vnum %u has bad door number.", vnum );
		    abort( );
		}
		CREATE(pexit, EXIT_DATA, 1);
		pexit->description   	= fread_string( fp );
        	/*STRFREE (pt );*/
		pexit->keyword  	= fread_string( fp );  /* buf for door */
		pexit->exit_info        = fread_number( fp );
    		if( IS_SET( pexit->exit_info, EX_CLOSED))
      		  pexit->exit_info=pexit->exit_info|EX_ISDOOR;
    		if( IS_SET( pexit->exit_info, EX_LOCKED))
      		  pexit->exit_info=pexit->exit_info|EX_ISDOOR|EX_CLOSED;
    		if( IS_SET( pexit->exit_info, EX_PICKPROOF))
      		  pexit->exit_info=pexit->exit_info|EX_ISDOOR|EX_CLOSED|EX_LOCKED;
		/*  This doesn't make much sense...Martin 13/12/98
    		if( IS_SET( pexit->exit_info, EX_HIDDEN))
      		  pexit->exit_info=pexit->exit_info|EX_ISDOOR|EX_CLOSED|EX_LOCKED|
	  		EX_PICKPROOF;*/
		pexit->key              = fread_number( fp );
		pexit->vnum             = fread_number( fp );
		pexit->to_room		= NULL;
		pRoomIndex->exit[door]  = pexit;
		top_exit++;
	    }
	    else if ( letter == 'F' )
	    {
	    pRoomIndex->fall_room       = fread_number( fp );
	    pRoomIndex->fall_slope      = fread_number( fp );
            if( pRoomIndex->fall_slope < 0 )
              pRoomIndex->fall_slope = 0 ;
            if( pRoomIndex->fall_slope > 10 )
              pRoomIndex->fall_slope = 10 ;
	    pRoomIndex->distance_of_fall= fread_number( fp );
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;
		CREATE(ed, EXTRA_DESCR_DATA, 1);
		ed->keyword             = fread_string( fp );
        	pt = fread_string( fp );
		ed->description  = UPPER_ALLOC( pt );
        	STRFREE (pt );
		LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
                          next, prev );

		top_ed++;
	    }
	    else if ( letter == 'X' )
                 {
                 pRoomIndex->room_file = fread_string( fp );
                 }
	    else
	    {
		bug( "Load_rooms: vnum %u has flag not 'DES'.", vnum );
		/* abort( ); */
	    }
	}

	iHash                   = vnum % MAX_KEY_HASH;
	pRoomIndex->next        = room_index_hash[iHash];
	room_index_hash[iHash]  = pRoomIndex;
	top_room++;
	if(vnum>highest_vnum)
	  highest_vnum=vnum;
    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;
	
	CREATE( pShop, SHOP_DATA, 1);
	pShop->keeper           = fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	if ( get_mob_index( pShop->keeper ) == NULL )
		{
		    bug( "load_shop: Mobile %d does not exist", pShop->keeper );
                    abort();
		}
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]     = fread_number( fp );
	pShop->profit_buy       = fread_number( fp );
	pShop->profit_buy       = UMAX(pShop->profit_buy,100);
	pShop->profit_sell      = fread_number( fp );
	pShop->profit_sell      = UMIN(pShop->profit_sell,50);
	pShop->open_hour        = fread_number( fp );
	pShop->close_hour       = fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex               = get_mob_index( pShop->keeper );
	pMobIndex->pShop        = pShop;

	if ( shop_first == NULL )
	    shop_first = pShop;
	if ( shop_last  != NULL )
	    shop_last->next = pShop;

	shop_last       = pShop;
	pShop->next     = NULL;
	top_shop++;
    }

    return;
}



/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    abort( );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex           = get_mob_index ( fread_number ( fp ) );
	    pMobIndex->spec_fun = spec_lookup   ( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %u.", pMobIndex->vnum );
		abort( );
	    }
	    break;

	case 'O':
	    pObjIndex           = get_obj_index ( fread_number ( fp ) );
	    pObjIndex->obj_fun  = obj_fun_lookup( fread_word   ( fp ) );
	    if ( pObjIndex->obj_fun == 0 )
	    {
		bug( "Load_specials: 'O': vnum %u.", pObjIndex->vnum );
		abort( );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}



/*
 * Snarf notes file.
 */
void load_notes( void )
{
    FILE *fp;
    NOTE_DATA *pnotelast,*pnote;
    char letter,*pst;
    char *pt;

    fp = fopen( NOTE_FILE_T, "r" );

    if( fp == NULL )
	return;

    rewind( fp );

    pnotelast = NULL;
    for ( ; ; )
    {
	do
	{
	    letter = getc( fp );
	    if ( feof(fp) )
	    {
	        fclose( fp );
		return;
	    }
	}
	while ( isspace((int)letter) );
	ungetc( letter, fp );

        CREATE(pnote, NOTE_DATA, 1);
	if ( strcasecmp( fread_word( fp ), "sender" ) )
	    break;
	pnote->sender   = fread_string( fp );

	if ( strcasecmp( fread_word( fp ), "date" ) )
	    break;
	pnote->date     = fread_string( fp );

	if ( strcasecmp( fread_word( fp ), "time" ) )
	    break;
	pnote->time     = fread_number( fp );

	if ( strcasecmp( fread_word( fp ), "to" ) )
	    break;
	pnote->to_list  = fread_string( fp );

	if ( strcasecmp( fread_word( fp ), "subject" ) )
	    break;
	pnote->subject  = fread_string( fp );

  pst=fread_word(fp);
	if ( strcasecmp( pst, "topic" ) )
	  pnote->topic=0;  /* default topic */
  else
    {
	  pnote->topic= fread_number( fp );
    pst=fread_word(fp);
    }

	if ( strcasecmp( pst, "text" ) )
	    break;
        pt = fread_string( fp );
	pnote->text     = UPPER_ALLOC( pt );
        STRFREE (pt );

	if ( strcasecmp( fread_word( fp ), "Room" ) )
	    break;
	pnote->room_vnum     = fread_number( fp );
	LINK( pnote, first_note, last_note, next, prev );
    }

    strcpy( strArea, NOTE_FILE_T );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    abort( );
    return;
}


void fix_object_levels( void )
{
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  int cnt;

   /*  correct some indexs first */
  for( cnt=0; cnt< MAX_KEY_HASH; cnt++)
    for( pObjIndex = obj_index_hash[cnt]; pObjIndex != NULL;
	    pObjIndex = pObjIndex->next )
      {
      if( pObjIndex->vnum == 51 || pObjIndex->vnum == 50 ||
          pObjIndex->vnum == 53 )
        {
        pObjIndex->level = 10;
        pObjIndex->count = -1;
        continue;
        }
      
      if( IS_SET(pObjIndex->extra_flags, ITEM_LEVEL_RENT))
        {
        pObjIndex->level= pObjIndex->level_rent;
        pObjIndex->count= -1;
        }
      }


    /* Scan once to locate fixed shop items */
  for( obj=first_object; obj!=NULL; obj=obj->next)
    {
    if( obj->carried_by != NULL && obj->carried_by->pIndexData->pShop != NULL &&
	obj->pIndexData->count != -1 )
      {
      if(IS_SET(obj->extra_flags, ITEM_LEVEL_RENT))
	obj->pIndexData->level = obj->pIndexData->level_rent;
      else
	obj->pIndexData->level=obj->pIndexData->level/obj->pIndexData->count;
      obj->pIndexData->count = -1;
      }
    }
  for( obj=first_object; obj!=NULL; obj=obj->next)
    {
    if( obj->pIndexData->count == -1 )
      {
      obj->level = obj->pIndexData->level;
      continue;
      }
    if( IS_SET(obj->extra_flags, ITEM_LEVEL_RENT))
      obj->level= obj->pIndexData->level_rent;
    else
      {
      if( obj->pIndexData->count > 0)
	obj->level=obj->pIndexData->level/obj->pIndexData->count;
      else
	obj->level=obj->pIndexData->area->average_level;
			 /* default level for items */
      }
    obj->pIndexData->count=-1;
    obj->pIndexData->level=obj->level;
    }
  return;
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    extern const sh_int rev_dir [];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( door = 0; door <= 5; door++ )
		if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
		{
		    fexit = TRUE;
		    if ( pexit->vnum <= 0 )
			pexit->to_room = NULL;
		    else
			pexit->to_room = get_room_index( pexit->vnum );
		}

	    if ( !fexit )
		SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}
    }

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	  for ( pRoomIndex  = room_index_hash[iHash];
		pRoomIndex != NULL;
		pRoomIndex  = pRoomIndex->next )
	    {
	    for ( door = 0; door <= 5; door++ )
	      {
	if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
		    &&   ( to_room   = pexit->to_room               ) != NULL
		    &&  (( pexit_rev = to_room->exit[rev_dir[door]] ) == NULL
		         ||   pexit_rev->to_room != pRoomIndex ))
		      {
                      if(IS_SET(pRoomIndex->room_flags,ROOM_IS_ENTRANCE)&&
                          !IS_SET(to_room->room_flags, ROOM_IS_CASTLE))
                        {/* connect the castle to the rest of the game */
                        if(pexit_rev==NULL)
                          {
			  CREATE(pexit_rev, EXIT_DATA, 1);
                          top_exit++;
                          }
                        else
                          {
                          if (pexit_rev->description != NULL) STRFREE (pexit_rev->description);
                          if (pexit_rev->keyword != NULL) STRFREE (pexit_rev->keyword);
                          }
  			if (pRoomIndex->exit[door]->description &&
			   pRoomIndex->exit[door]->description[0] != '\0')
	 		{
                          pexit_rev->description = STRALLOC(pRoomIndex->exit[door]->description);
			}
   			else
 			  pexit_rev->description = STRALLOC( "" );

  			if (pexit->keyword 
			    && pexit->keyword[0] != '\0')
                          pexit_rev->keyword = STRALLOC(pexit->keyword);
   			else
 			  pexit_rev->keyword = STRALLOC("");
                        pexit_rev->exit_info = pexit->exit_info;
                        pexit_rev->pvnum  = pexit->pvnum;
                        pexit_rev->key  = pexit->key;
                        pexit_rev->vnum  = pRoomIndex->vnum;
                        pexit_rev->to_room  = pRoomIndex;
                        to_room->exit[rev_dir[door]] = pexit_rev;
                        continue;
                        }
                      if(IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE) &&
                         !IS_SET(pRoomIndex->room_flags,ROOM_IS_ENTRANCE) &&
                         (!IS_SET(to_room->room_flags, ROOM_IS_CASTLE) ||
                          to_room->creator_pvnum!=pRoomIndex->creator_pvnum))
                        {/* take care of ANY back doors */
                        pexit->pvnum=pRoomIndex->creator_pvnum;
                        continue;
                        }
                      if(IS_SET( pRoomIndex->area->flags, AFLAG_NODEBUG))
	                continue;
                      if(pexit_rev==NULL)
		        sprintf( buf, "Fix_exits: %u:%d -> %u:%d -> 0.",
				 pRoomIndex->vnum, door, to_room->vnum,
                                 rev_dir[door]);
                      else
		        sprintf( buf, "Fix_exits: %u:%d -> %u:%d -> %d.",
				 pRoomIndex->vnum, door, to_room->vnum,
                                 rev_dir[door],
				 (pexit_rev->to_room == NULL)
				  ? 0 : pexit_rev->to_room->vnum );
		      /* bug( buf, -1 );   get out of game log */
		      }
	      }
	    }
    }

    return;
}



/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoomIndex;
    PLAYER_GAME *gpl;
    char buf[MAX_STRING_LENGTH];

    for ( pArea = first_area; pArea != NULL; pArea = pArea->next )
    {
	CHAR_DATA *pch;

	pArea->age++;

	/*
	 * Check for PC's.
	 */
	if ( pArea->nplayer > 0 && pArea->age == 39 )
	{
                int rnd;
                rnd=number_range(1,3);
	    for ( gpl = first_player; gpl != NULL; gpl = gpl->next )
	    {
              pch = gpl->ch;
		if(  IS_AWAKE(pch)
		&&   pch->in_room != NULL
		&&   pch->in_room->area == pArea )
	        {
	        if (pArea->resetmsg != NULL && 
		     pArea->resetmsg[0] != '\0' )
		{
		  sprintf(buf, "%s\n\r", pArea->resetmsg);
		}
 	 	else
		{
                 switch( pch->in_room->sector_type )
                  {
		   case SECT_INSIDE:
		   if( rnd==1 )
  		    sprintf( buf, "You hear the boards creaking.\n\r" );
		   if( rnd==2 )
  		    sprintf( buf, "You hear growling echoing in the distance.\n\r" );
		   if( rnd==3 )
		    sprintf( buf, "You hear wax dripping off a candle.\n\r" );
		break;
		   case SECT_CITY:
		    if( rnd==1 )
		     sprintf( buf, "You hear the shuffle of feet out in the street.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear someone calling for the guards.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear shopkeepers announcing their wares.\n\r" );
		break;
		   case SECT_FIELD:
		    if( rnd==1 )
		     sprintf( buf, "You hear small creatures rustling the grass.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear large creatures rustling the grass.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear some field mice scurrying about.\n\r" );
		break;
		   case SECT_FOREST:
		    if( rnd==1 )
		     sprintf( buf, "You hear the trees creaking in the wind.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear wolves howling in the distance.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear some lumberjacks calling timber.\n\r" );
		break;
		   case SECT_HILLS:
		    if( rnd==1 )
		     sprintf( buf, "You hear quiet, unintelligible muttering from no particular direction.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the wind whistling past your face.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear green grass growing around you.\n\r" );
		break;
		   case SECT_MOUNTAIN:
		    if( rnd==1 )
		     sprintf( buf, "You hear rocks tumbling down the mountain.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear echos of large hooved animals wandering the mountain.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear small avalanches crashing down the mountain.\n\r" );
		break;
		   case SECT_WATER_SWIM:
		    if( rnd==1 )
		     sprintf( buf, "You hear fish splashing in the water.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear something large fall into the water.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear some old fisherman cast their lines about you.\n\r" );
		break;
		   case SECT_WATER_NOSWIM:
		    if( rnd==1 )
		     sprintf( buf, "You hear the waves cresting loudly.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear a large fish splash in the water.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear a fog horn echoing through the darkness.\n\r" );
		break;
		   case SECT_UNUSED:
		    if( rnd==1 )
		     sprintf( buf, "You hear thousands of creatures rushing at you from all directions.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear nothing at all.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear the sound of infinity.\n\r" );
		break;
		   case SECT_AIR:
		    if( rnd==1 )
		     sprintf( buf, "You hear the wind blowing harshly.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear a hawk screeching loudly.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear a storm forming around you.\n\r" );
		break;
		   case SECT_DESERT:
		    if( rnd==1 )
		     sprintf( buf, "You hear the sand whipping fiercely.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the hollow cries of something just barely alive.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear a sandstorm growing in the distance.\n\r" );
		break;
		   case SECT_LAVA:
		   if( rnd==1 )
		    sprintf( buf, "You hear the bubbling of solid stone.\n\r" );
		   if( rnd==2 )
		    sprintf( buf, "You hear grinding of rocks.\n\r" );
		   if( rnd==3 )
		    sprintf( buf, "You hear mother nature's fury at work.\n\r" );
		break;
		   case SECT_INN:
		   if( rnd==1 )
		    sprintf( buf, "You hear a patron yell for the serving wench.\n\r" );
		   if( rnd==2 )
		    sprintf( buf, "You hear a loud and off key song start up throughout the inn.\n\r" );
		   if( rnd==3 )
		    sprintf( buf, "You hear a bartender filling up some glasses of ale.\n\r" );
		break;
		   case SECT_ETHEREAL:
		    if( rnd==1 )
		     sprintf( buf, "You hear echos of an unknown creature.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the distant explosions of power.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear an indescribable sound.\n\r" );
		break;
		   case SECT_ASTRAL:
		    if( rnd==1 )
		     sprintf( buf, "You hear silence echoing in your head.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the pain of a lost soul.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear the beating of your own heart.\n\r" );
		break;
		   case SECT_UNDER_WATER:
		    if( rnd==1 )
		     sprintf( buf, "You hear the gurgling of water.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the clicking of a large marine animal.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear the squeal of a dying fish.\n\r" );
		break;
		   case SECT_UNDER_GROUND:
		    if( rnd==1 )
		     sprintf( buf, "You hear a loud crash!\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "Some cracks appear in the rock above your head.\n\r" );
		    if( rnd==3 )
		     sprintf( buf, "You hear someone kick some loose rumble and a mumbled apology.\n\r" );
		break;
		   case SECT_DEEP_EARTH:
		    if( rnd==1 )
		     sprintf( buf, "You hear a distant cry echoing all around you.\n\r" );
		    if( rnd==2 )
		     sprintf( buf, "You hear the groan of the immense weight above you.\n\r");
		    if( rnd==3 )
		     sprintf( buf, "You smell ozone.\n\r");
		break;
		   default:
		  sprintf( buf, "You hear the patter of little feet.\n\r" );
		break;
		}
  	       }
	        send_to_char(buf, pch);
              }
  	     
	    }
	}


	/*
	 * Check age and reset.
	 */
  pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
   if ( ( pRoomIndex!=NULL && pArea==pRoomIndex->area && pArea->age >= 3)
       || pArea->age >=40 )
	    {
	    /*log_printf("Resetting %s", pArea->name);*/
	    reset_area( pArea );
	    if( pRoomIndex!=NULL && pArea==pRoomIndex->area)
	      pArea->age = 0;
	    else
	      {
	      if( fBootDb )
		{
		if( pArea->tmp != 0)
		  pArea->average_level /= pArea->tmp;
		pArea->age = 25;
		}
	      else
		pArea->age = number_fuzzy( (100-pArea->average_level)/3 );
	      }
	    }
    }
    return;
}

RESET_DATA *get_reset_from_obj( int vnum , 
                   RESET_DATA *lReset, RESET_DATA *fReset )
  {
  RESET_DATA *pReset;
  RESET_DATA *oReset;

     /* Locate last instance of obj reset */
    oReset = NULL;
    for ( pReset = fReset; pReset != NULL && pReset != lReset;
          pReset = pReset->next )
      if( pReset->arg1 == vnum )
        if( pReset->command == 'O' || pReset->command == 'G' ||
            pReset->command == 'E' )
          oReset = pReset;

       return( oReset );
   }

RESET_DATA *get_reset_from_mob( RESET_DATA *lReset, RESET_DATA *fReset )
  {
  RESET_DATA *pReset;
  RESET_DATA *oReset;

     /* Locate last instance of mobile reset */
    oReset = NULL;
    for ( pReset = fReset; pReset != NULL && pReset != lReset;
          pReset = pReset->next )
        if( pReset->command == 'M')
          oReset = pReset;

       return( oReset );
   }

OBJ_DATA *get_obj_from_index( int vnum , RESET_DATA *lReset, AREA_DATA *pArea )
  {
  OBJ_DATA *obj;
  RESET_DATA *oReset;
  RESET_DATA *mReset;
  CHAR_DATA *mob;
  ROOM_INDEX_DATA *oRoomIndex;
  /* char buf[200]; */

  oReset = lReset->container;
    if( oReset == NULL )
       return( NULL );

     /* Object in room */
    if( oReset->command == 'O' && 
	(oRoomIndex = get_room_index( oReset->arg3 )) != NULL )
        {
        for( obj = oRoomIndex->first_content; obj != NULL;
             obj = obj->next_content )
          if( obj->pIndexData->vnum == vnum )
            return( obj );
        return( NULL );
        }

     /* Object is on a mobile here */
     /* Locate last instance of to_mob reset */
        mReset = oReset->container;

    if( mReset != NULL  &&  (mob=mReset->mob) != NULL )
      {
        for( obj = mob->first_carrying; obj != NULL;
             obj = obj->next_content )
          if( obj->pIndexData->vnum == vnum )
            return( obj );
        return( NULL );
      }

  return( NULL );
  }
             


void do_resetarea( CHAR_DATA *ch, char *arg)
{
  AREA_DATA *pArea;
  char buf[200];

  if( arg[0]!='a')
    {
    reset_area( ch->in_room->area );
    sprintf( buf, "You reset the area.  Count is %d.\n\r", ch->in_room->area->count);
    send_to_char( buf, ch);
    }
    for ( pArea = first_area; pArea != NULL; pArea = pArea->next )
      reset_area( pArea );
  return;
}




/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;
    NPC_DATA *np;
    int cnt;

    if ( pMobIndex == NULL )
    {
	bug( "Create_mobile: NULL pMobIndex.", 0 );
	abort( );
    }
    CREATE(mob, CHAR_DATA, 1);
    total_mobiles++;
    clear_char( mob );
    CREATE(np, NPC_DATA, 1);
    mob->npcdata=np;

    mob->pIndexData     = pMobIndex;

    mob->editor         = NULL;
    mob->name           = QUICKLINK(pMobIndex->player_name);
    mob->short_descr    = QUICKLINK(pMobIndex->short_descr);
    mob->long_descr     = QUICKLINK(pMobIndex->long_descr);
    mob->description    = QUICKLINK(pMobIndex->description);
    mob->spec_fun       = pMobIndex->spec_fun;

    mob->level          = number_fuzzy(pMobIndex->level);

    mob->act            = pMobIndex->act;
    mob->affected_by    = pMobIndex->affected_by;
    mob->alignment      = pMobIndex->alignment;
    mob->sex            = pMobIndex->sex;
    mob->reset  = NULL;
    mob->armor          = pMobIndex->ac;  /* Not used at all */

    mob->max_hit = dice( pMobIndex->hitnodice, pMobIndex->hitsizedice) +
	 pMobIndex->hitplus;
    if( mob->max_hit < 1 )
      mob->max_hit = 32000;
    mob->hit            = mob->max_hit;
    mob->gold           = pMobIndex->gold;

    /* Chaos 5/25/94 */
    mob->npcdata->damnodice = abs( pMobIndex->damnodice );
    mob->npcdata->damsizedice = abs( pMobIndex->damsizedice );
    mob->npcdata->damplus = abs( pMobIndex->damplus );
    mob->npcdata->next = NULL;
    mob->npcdata->poison = NULL;
    mob->npcdata->armor = pMobIndex->armor;
    mob->npcdata->mob_number = mob_counter;
    mob->npcdata->mob_prog_started = FALSE;

    mob_counter++;
    if(mob_counter > 10000)
      mob_counter=1;

/* dug */
    mob->shot_timer     = 0;
    mob->shot_from      = 0;
    mob->shot_aware     = FALSE;
    mob->recall         = ROOM_VNUM_LIMBO;
    mob->race           = pMobIndex->race;
    mob->language       = SHIFT(pMobIndex->race);
    mob->speak          = mob->language;
    mob->position       = pMobIndex->position;
    for(cnt=0;cnt<MAX_CLASS;cnt++)
      mob->mclass[cnt]=mob->level/4;
    mob->class          = CLASS_MONSTER;
	    
    /*
     * Insert in list.
     */
    add_char( mob );
    pMobIndex->count++;
    return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if ( pObjIndex == NULL )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	abort( );
    }

    CREATE(obj, OBJ_DATA, 1);
    *obj                = obj_zero;
    obj->pIndexData     = pObjIndex;
    obj->in_room        = NULL;
    if(IS_BOOTING)
      {
      obj->pIndexData->count++;
      obj->pIndexData->level+=level;
      }
      
    obj->level          = pObjIndex->level;
    obj->wear_loc       = WEAR_NONE;

    obj->basic          = TRUE;
    obj->condition	= 70+number_range(0,29);

    add_to_object_reference_hash( obj );

    obj->name           = QUICKLINK(pObjIndex->name);
    obj->short_descr    = QUICKLINK(pObjIndex->short_descr);
    obj->long_descr     = QUICKLINK(pObjIndex->long_descr);
    obj->description    = QUICKLINK(pObjIndex->description);
    obj->item_type      = pObjIndex->item_type;
    obj->extra_flags    = pObjIndex->extra_flags;
    obj->wear_flags     = pObjIndex->wear_flags;
    obj->value[0]       = pObjIndex->value[0];
    obj->value[1]       = pObjIndex->value[1];
    obj->value[2]       = pObjIndex->value[2];
    obj->value[3]       = pObjIndex->value[3];
    obj->weight         = pObjIndex->weight;
    obj->reset    	= NULL;
    obj->next_content	= NULL;
    obj->owned_by = 0;  /* Set to no owner */
    obj->cost           = pObjIndex->cost;
    if(IS_SET(obj->extra_flags, ITEM_LEVEL_RENT))
      obj->level=pObjIndex->level_rent;
    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %u bad type.", pObjIndex->vnum );
	break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
	break;

    case ITEM_SCROLL:
	obj->value[0]   = number_fuzzy( obj->value[0] );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	obj->value[0]   = number_fuzzy( obj->value[0] );
	obj->value[1]   = number_fuzzy( obj->value[1] );
	obj->value[2]   = obj->value[2];
	break;

    case ITEM_WEAPON:
	break;

    case ITEM_ARMOR:
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	obj->value[0]   = number_fuzzy( number_fuzzy( obj->value[0] ) );
	break;

    case ITEM_MONEY:
	obj->value[0]   = obj->cost;
	break;

    case ITEM_AMMO:
	obj->value[1]   = number_fuzzy(obj->value[1]);
	break;
    }

    LINK( obj, first_object, last_object, next, prev );

    /* Arg! This damn code had me hunting for hours for a bug
       Death to over zealous anti cheating !!
       9/1/99 Martin 
    */
    obj->test_obj = FALSE;
    ++total_objects;
    ++pObjIndex->total_objects;
    return obj;
}



/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    static CHAR_DATA ch_zero;

    *ch                         = ch_zero;
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->hunting                 = NULL;
    ch->fearing                 = NULL;
    ch->hating                  = NULL;
    ch->logon                   = current_time;
    ch->armor                   = 100;
    ch->position                = POS_STANDING;
    ch->practice                = 21;
    ch->hit                     = 20;
    ch->max_hit                 = 20;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    ch->editor                  = NULL;
    ch->last_cmd 		= NULL;
    ch->dest_buf 		= NULL;
    ch->substate		= 0;
    ch->first_carrying          = NULL;
    ch->last_carrying           = NULL;
    ch->next_in_room            = NULL;
    ch->prev_in_room            = NULL;
    ch->first_affect            = NULL;
    ch->last_affect             = NULL;
    ch->next			= NULL;
    ch->prev			= NULL;
    return;
}



/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    int cnt;

    for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
      {
	    obj_next = obj->next_content;
	    extract_obj( obj );
      }

    for ( paf = ch->first_affect; paf != NULL; paf = paf_next )
      {
	    paf_next = paf->next;
	    affect_remove( ch, paf );
      }

  if( IS_NPC( ch ) && ch->name != ch->pIndexData->player_name )
    STRFREE (ch->name               );
  if( IS_NPC( ch ) && ch->short_descr != ch->pIndexData->short_descr )
    STRFREE (ch->short_descr        );
  if( IS_NPC( ch ) && ch->long_descr != ch->pIndexData->long_descr )
    STRFREE (ch->long_descr         );
  if( IS_NPC( ch ) && ch->description != ch->pIndexData->description )
    STRFREE (ch->description        );
  if( ch->editor)
    stop_editing(ch);
  if( IS_NPC( ch ) )
    {
    if( ch->npcdata->mob_quest!=NULL )
      DISPOSE( ch->npcdata->mob_quest );
    ch->npcdata->mob_quest = NULL;
    }
  else
    {
  for (cnt = 0; cnt < MAX_AREA; cnt++)
    if( ch->pcdata->quest[cnt] != NULL )
      {
      DISPOSE( ch->pcdata->quest[cnt] );
      }
    }

  stop_hunting( ch );
  stop_hating ( ch );
  stop_fearing( ch );
  free_fight  ( ch );

    if ( ch->pcdata != NULL )
      {
      if( ch->pcdata->last_command != NULL )
        STRFREE(ch->pcdata->last_command);
      STRFREE(ch->pcdata->pwd              );
      STRFREE(ch->pcdata->bamfin           );
      STRFREE(ch->pcdata->bamfout  );
      STRFREE (ch->pcdata->title            );
      STRFREE (ch->pcdata->auto_command );
      STRFREE(ch->pcdata->mail_address);
      STRFREE(ch->pcdata->html_address);
      STRFREE (ch->pcdata->prompt_layout);
      STRFREE (ch->pcdata->block_list);
      if ( ch->pnote != NULL )
	{
	STRFREE (ch->pnote->text );
	STRFREE (ch->pnote->subject );
	STRFREE (ch->pnote->to_list );
	STRFREE (ch->pnote->date );
	STRFREE (ch->pnote->sender );
	DISPOSE( ch->pnote );
	ch->pnote           = NULL;
	}
      for(cnt=0;cnt<MAX_PK_ATTACKS;cnt++)
	STRFREE (ch->pcdata->last_pk_attack_name[cnt] );
      for(cnt=0;cnt<MAX_ALIAS;cnt++)
	{
	STRFREE (ch->pcdata->alias[cnt] );
	STRFREE (ch->pcdata->alias_c[cnt] );
	}
      for(cnt=0;cnt<MAX_KILL_TRACK;cnt++)
	{
	STRFREE (ch->pcdata->killname[cnt] );
	}
      for(cnt=0;cnt<26;cnt++)
	STRFREE(ch->pcdata->back_buf[cnt] );

      if (ch->pcdata->poison!=NULL)
        {
        POISON_DATA *pd, *npd;
        for( pd=ch->pcdata->poison; pd!=NULL;pd=npd)
          {
          npd = pd->next;
          DISPOSE(pd );
          }
        }
      if (ch->pcdata->tactical!=NULL)
        DISPOSE(ch->pcdata->tactical );
      if (ch->pcdata->castle!=NULL)
        DISPOSE(ch->pcdata->castle );
      DISPOSE( ch->pcdata );
      }


    if ( ch->npcdata!=NULL && ch->npcdata->poison != NULL )
        {
        POISON_DATA *pd, *npd;
        for( pd=ch->npcdata->poison; pd!=NULL;pd=npd)
          {
          npd = pd->next;
          DISPOSE(pd );
          }
        }
    if ( ch->npcdata != NULL )
      DISPOSE( ch->npcdata);

    DISPOSE( ch );
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name( name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
  if( vnum<0 || vnum>=MAX_VNUM )
    return( NULL );

    return( mob_index[ vnum ] );

  /*     Chaos   -   1/18/95
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex != NULL;
	  pMobIndex  = pMobIndex->next )
    {
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_mob_index: bad vnum %d.", vnum );
	abort( );
    }

    return NULL;

  */
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
  if( vnum<0 || vnum>=MAX_VNUM )
    return( NULL );

   return( obj_index[ vnum ] );

  /*    Sped up by Chaos    -   1/18/95
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex != NULL;
	  pObjIndex  = pObjIndex->next )
    {
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_obj_index: bad vnum %d.", vnum );
	abort( );
    }

    return NULL;  */
}


ROOM_INDEX_DATA *get_room_index( int vnum )
  {
  if( vnum<0 || vnum>=MAX_VNUM )
    return( NULL );

  return( room_index[ vnum] );
  }

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index_old( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

  if( vnum<0 || vnum>=MAX_VNUM )
    return( NULL );


    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next )
    {
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_room_index: bad vnum %u.", vnum );
	abort( );
    }

    return NULL;
}


/*

Okay...to prevent stupid crashes from bugged player files I basically
added some further bug checks to the fread commands that will end with
returning "What we've got so far" instead of aborting the whole mud.

Also checks for buffer over-runs and EOF 

- Martin
*/
/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return '\0';
        }
        c = getc( fp );
    }
    while ( isspace((int)c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number, cnt;
    bool sign;
    bool bit;
    char c;
    char buf[2], buf2[100];

    buf[1]='\0';
    buf2[0]='\0';
    bit = FALSE;
    do
    {
	if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return 0;
        }
	c = getc( fp );
    }
    while (isspace((int)c) );

    number = 0;

    if( c == '/' )
      {
      fread_to_eol( fp );
      return( fread_number( fp ));
      }

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }
    while( ( c>='A' && c<='Z' ) || c=='_')
    {
     bit = TRUE;
     buf[0]=c;
     strcat( buf2, buf);
     c = getc( fp );
    }

  if( bit )
    {
    bool foundb= FALSE;
    for( cnt=0; cnt < MAX_BITVECTOR; cnt++ )
    {
      if( !strcasecmp( bitvector_table[cnt].name, buf2))
	{
	 foundb=TRUE;
	 number=bitvector_table[cnt].value;
	 break;
	}
    }
    if( !foundb)
      {
      sprintf( buf, "Fread_number: bad format1 '%s'.", buf2);
      bug( buf, 0 );
      }
    }
  else
   {
    if ( !isdigit((int)c) )
    {
     char buf[160], *tmp;
     tmp=fread_word( fp );
     sprintf( buf, "Fread_number: bad format2 '%c%s'.", c, tmp);
     bug( buf, 0 );
     /*if (fBootDb)
      abort( ); */
     return 0;
    }

    while ( isdigit((int)c) )
    {
     if (feof(fp))
     {
      bug("fread_number: EOF encountered on read.\n\r",0);
      if ( fBootDb )
       abort();
      return number;
     }
     number = number * 10 + c - '0';
     c      = getc( fp );
    }
   }
    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}



/*
 * Read and allocate space for a string from a file.
 * Strings are created and placed in Dynamic Memory.
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
            bug("fread_string: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return STRALLOC("");
        }
        c = getc( fp );
    }
    while ( isspace((int)c) );

    if ( ( *plast++ = c ) == '~' )
        return STRALLOC( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
             bug( "fread_string: string too long" );
             *plast = '\0';
             return STRALLOC( buf );
        }
        switch ( *plast = getc( fp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "Fread_string: EOF" );
            if ( fBootDb )
              exit( 1 );
            *plast = '\0';
            return STRALLOC(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return STRALLOC( buf );
        }
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.

     */
    do
    {
        if ( feof(fp) )
        {
            bug("fread_string_no_hash: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return str_dup("");
        }
        c = getc( fp );
    }
    while ( isspace((int)c) );

    if ( ( *plast++ = c ) == '~' )
        return str_dup( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
           bug( "fread_string_no_hash: string too long" );
           *plast = '\0';
           return str_dup( buf );
        }
        switch ( *plast = getc( fp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "Fread_string_no_hash: EOF" );
            if ( fBootDb )
              exit( 1 );
            *plast = '\0';
            return str_dup(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return str_dup( buf );
        }
    }
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_to_eol: EOF encountered on read.\n\r", 0);
          if ( fBootDb )
            abort();
          return;
        }
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer                       -Martin
   To be used in new code to prevent bugged characters crashing the mud
 */
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
          bug("fread_line: EOF encountered on read.\n\r",0);
          if ( fBootDb )
            abort();
          strcpy(line, "");
          return line;
        }
        c = getc( fp );
    }
    while ( isspace((int)c) );

    ungetc( c, fp );
    do
    {
        if ( feof(fp) )
        {
          bug("fread_line: EOF encountered on read.\n\r",0);
          if ( fBootDb )
            abort();
          *pline = '\0';
          return line;
        }
        c = getc( fp );
        *pline++ = c; ln++;
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
           bug( "fread_line: line too long",0 );
           break;
        }
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );
    ungetc( c, fp );
    *pline = '\0';
    return line;
}


/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_word: EOF encountered on read.\n\r", 0);
          if ( fBootDb )
            abort();
          word[0] = '\0';
          return word;
        }
	cEnd = getc( fp );
    }
    while ( isspace((int) cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        if ( feof(fp) )
        {
          bug("fread_word: EOF encountered on read.\n\r", 0);
          if ( fBootDb )
            abort();
          *pword = '\0';
          return word;
        }

	*pword = getc( fp );
	if (( cEnd == ' ' ? isspace((int)*pword) : *pword == cEnd ) ||
              *pword == EOF )
	{
	    if ( cEnd == ' '  || *pword == EOF )
		ungetc( *pword, fp );
	    *pword = '\0';
            strcpy(old_fread_word, word);
	    return word;
	}
    }


    {
    char buf[MAX_INPUT_LENGTH];

    word[10]='\0';
    sprintf(buf,"Fread_word: word '%s' too long.\n\r    Previous word was '%s'",
	    word, old_fread_word );
    bug( buf, 0 );
    abort( );
    return NULL;
    }
}



/*
 * Duplicate a string into dynamic memory.
 */
char *str_dup( char const *str )
{

  return( (char *)str_alloc((char *)str, 1) ); 
  /*

    static char *ret;
    int len;

    if ( !str )
        return NULL;

    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;   */
}


void do_areas( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char mainauth[32];
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *room;
  int pcnt, mcnt, rcnt;
  CHAR_DATA *rch;
  int hash;
  int lower_bound = 0;
  int upper_bound = MAX_LEVEL + 1;
  
  if( argument != NULL && *argument != '\0' )
   {
   if (!is_number(argument))
    {
     for( pArea = first_area; pArea != NULL  ; pArea = pArea->next )
       {
       strcpy( buf, pArea->name );
       if( !str_prefix( argument, buf ) )
         {
         sprintf( buf, "%s\n\r", pArea->name );
         send_to_char( buf, ch);
         sprintf( buf, "Suggested levels:          %2d-%2d", pArea->low_soft_range,
                 pArea->hi_soft_range);
         send_to_char( buf, ch);
         sprintf( buf, "Min/Max levels:            %2d-%2d", pArea->low_hard_range,
                 pArea->hi_hard_range);
         send_to_char( buf, ch);

         pcnt=0;
         mcnt=0;
         rcnt=0;
         for( hash=0; hash < MAX_KEY_HASH; hash++)
           for( room=room_index_hash[hash]; room != NULL; room=room->next )
             if( room->area == pArea )
               {
               rcnt++;
               for( rch=room->first_person; rch!= NULL; rch=rch->next_in_room)
                 if( IS_NPC( rch )&& can_see(ch,rch) && !IS_AFFECTED(rch, AFF_STEALTH))
                   mcnt++;
                 else if (can_see(ch,rch)&& !IS_AFFECTED(rch, AFF_STEALTH))
                   pcnt++;
               }
          
         sprintf( buf, "Rooms in area:              %d\n\r", rcnt );
         send_to_char( buf, ch);
         sprintf( buf, "Creatures in area:          %d\n\r", mcnt );
         send_to_char( buf, ch);
         sprintf( buf, "Players in area:            %d\n\r", pcnt );
         send_to_char( buf, ch);
	 if( pArea->authors!=NULL )
	   {
           sprintf( buf, "Authors of area:            %s\n\r", pArea->authors);
           send_to_char( buf, ch);
	   }
#ifdef AREA_KILL
         sprintf( buf, "Creatures killed in area:   %d\n\r",
              ch->pcdata->area_kills[pArea->low_r_vnum/100]);
         send_to_char( buf, ch);
         sprintf( buf, "Damage percentage:          %d\n\r",
              ch->pcdata->area_kills[pArea->low_r_vnum/100]<=250 ? 100 :
              50000/(250+ch->pcdata->area_kills[pArea->low_r_vnum/100]));
         send_to_char( buf, ch);
#endif
         return;
         }
      }
    send_to_char( "That area was not found.\n\r", ch);
    return;
    }
   else
    {
      upper_bound = atoi(argument);
      lower_bound = upper_bound;

      argument = one_argument(argument,arg);

      if(arg[0] != '\0')
      {
        if(!is_number(arg))
        {
          send_to_char("Area may only be followed by numbers.\n\r", ch);
          return;
        }

        upper_bound = atoi(arg);

        argument = one_argument(argument,arg);
        if(arg[0] != '\0')
        {
          send_to_char("Only two level numbers allowed.\n\r",ch);
          return;
        }
      }

    if(lower_bound > upper_bound)
    {
      int swap = lower_bound;
      lower_bound = upper_bound;
      upper_bound = swap;
    }

  sprintf(buf2," {160}%-10s {130}%-28s  {120}Suggested  {110}Enforced\n\r", 
	"Author", "Area Name");
  strcat(buf2, "-------------------------------------------------------------------------------\n\r");

    for (pArea = first_area_name; pArea; pArea = pArea->next_sort_name)
    {
      if (pArea->hi_soft_range >= lower_bound
      &&  pArea->low_soft_range <= upper_bound)
       {
       one_argument_nolower(pArea->authors, mainauth);
   	if (!IS_IMMORTAL (ch) )
	    sprintf(buf," {060}%-10s {030}%-30s{020}  %2d-%-2d      {010}%2d-%-2d\n\r", 
	      mainauth,
	      pArea->name, 
	      pArea->low_soft_range, 
	      pArea->hi_soft_range, 
	      pArea->low_hard_range, 
	      pArea->hi_hard_range);
	else
	    sprintf(buf," {060}%-10s {030}%-30s{020}  %2d-%-2d      {010}%2d-%-2d  {060}[%5d]\n\r", 
	      mainauth,
	      pArea->name, 
	      pArea->low_soft_range, 
	      pArea->hi_soft_range, 
	      pArea->low_hard_range, 
	      pArea->hi_hard_range,
	      pArea->low_r_vnum);
	    strcat( buf2, buf);
       }
    }
    send_to_char_color( buf2, ch );
    return;
   }
  }
  else
  {
  sprintf(buf2," {160}%-10s {130}%-28s  {120}Suggested  {110}Enforced\n\r", 
	"Author", "Area Name");
  strcat(buf2, "-------------------------------------------------------------------------------\n\r");

    for ( pArea = first_area_name; pArea != NULL; pArea=pArea->next_sort_name )
      {
       one_argument_nolower(pArea->authors, mainauth);
   	if (!IS_IMMORTAL (ch) )
	    sprintf(buf," {060}%-10s {030}%-30s{020}  %2d-%-2d      {010}%2d-%-2d\n\r", 
	      mainauth,
	      pArea->name, 
	      pArea->low_soft_range, 
	      pArea->hi_soft_range, 
	      pArea->low_hard_range, 
	      pArea->hi_hard_range);
	else
	    sprintf(buf," {060}%-10s {030}%-30s{020}  %2d-%-2d      {010}%2d-%-2d  {060}[%5d]\n\r", 
	      mainauth,
	      pArea->name, 
	      pArea->low_soft_range, 
	      pArea->hi_soft_range, 
	      pArea->low_hard_range, 
	      pArea->hi_hard_range,
	      pArea->low_r_vnum);
	    strcat( buf2, buf);
      }
  send_to_char_color( buf2, ch );
  }
  return;
}

void do_cpu( CHAR_DATA *ch, char *arg)
  {
   char buf[200];
   /*int time_get, old_time;
   OBJ_DATA *obj;*/
   int tim, tot, cpu;

    sprintf( buf, "Average IO bandwidth: %d bytes/second\n\r",
	total_io_bytes*PULSE_PER_SECOND/total_io_ticks);
    send_to_char( buf, ch);

    sprintf( buf, "Average CPU Usage: %3.4f percent\n\r", 
	(double) total_io_exec*100.0/(double)(total_io_delay+total_io_exec) );
    send_to_char( buf, ch);

    tot = 0;

    send_to_char( "    Section                      Time (usec)  Freq (msec)    %Prog     %CPU\n\r" , ch );

    for( tim=0 ; tim<MAX_TIMERS ; tim++)
      tot += display_timer( ch, tim );
  
    cpu = (int)( 1000 * (double) total_io_exec*100.0/
                        (double)(total_io_delay+total_io_exec) );

    sprintf( buf, " Total subfunction CPU usage: %3.4f %% CPU usage\n\r",
           (double)tot / 1000.0 );
    send_to_char( buf, ch);

    sprintf( buf, " Unknown CPU usage: %3.4f %% CPU usage\n\r",
           (double)(cpu - tot) / 1000.0 );
    send_to_char( buf, ch);
    return;
}

   /* Returns 1000 times normal percentage value   */
int display_timer( CHAR_DATA *ch, int timer )
  {
  char buf[200];
  double total_usage, ind_usage;

  total_usage = (double) total_io_exec*100.0 /
                         (double)(total_io_delay+total_io_exec) ;

  if( total_usage == 0.0 )
    return( 0 );
  if( timers[ timer ][1] == 0 || timers[ timer ][4] == 0)
    return(0);

  ind_usage = (float)(timers[ timer ][0] / timers[ timer ][1] ) /
              (float)(timers[ timer ][3] / timers[ timer ][4])  / 10.0 ;

  sprintf( buf, "%s%7d    %8d        %4.3f    %4.4f\n\r",
          timer_strings[ timer] ,
          timers[ timer ][0] / timers[ timer ][1],
          timers[ timer ][3] / timers[ timer ][4] ,
            100.0 * ind_usage / total_usage , ind_usage );
  send_to_char( buf, ch );

       /* Do not include the first timer */
  if( timer == 0 || timer >= TIMER_WRITE_PAGER )  
    return( 0);

  return ((int)( (float)(timers[ timer ][0] / timers[ timer ][1] ) /
          (float)(timers[ timer ][3] / timers[ timer ][4]) * 100.0 ) );
  }
     
void open_timer( int timer )
  {
  int cur_time;
  cur_time = get_game_usec();
  if( timers[timer][2] > cur_time || timers[timer][2] == 0 )
     {
     timers[timer][2] = cur_time ;
     return;
     }
  timers[timer][3] += ( cur_time/1000 - timers[timer][2]/1000 );
  timers[timer][2] = cur_time;
  timers[timer][4] ++;
  if( timers[timer][3] > 10000000 )
    {
    timers[timer][3] /= 4;
    timers[timer][4] /= 4;
    }
  return;
  }

void close_timer( int timer )
  {
  int cur_time;
  cur_time = get_game_usec();
  if( timers[timer][2] > cur_time || timers[timer][2] == 0 )
     {
     timers[timer][2] = cur_time ;
     return;
     }
  timers[timer][0] += ( cur_time - timers[timer][2] );
  timers[timer][1] ++;
  if( timers[timer][0] > 1000000000 )
    {
    timers[timer][0] /= 2;
    timers[timer][1] /= 2;
    }
  
  return;
  }


/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
  int range;
    range=number/10+1;    /* increase fuzz proportionally */
    number= number - range + number_range( 0, range*2);
    /*switch ( number_bits( 2 ) )
    {
    case 0:  number -= (number/10); break;
    case 3:  number += (number/10); break;
    } */

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
#ifdef USE_48_RAND
  int val;
  if( from>to )
    {
    val = from;
    from = to;
    to = from;
    }
  val = abs((abs(mrand48())%(to-from+1))+from);
  return( val );

#else
    int power;
    int number;
    int val;

    if ( ( val = to - from + 1 ) <= 1 )
	return abs(from);

    for ( power = 2; power < val; power <<= 1 )
	;
    while ( ( number = number_mm( ) & (power - 1) ) >= val )
	; 

    return abs(from + number);
#endif
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
#ifdef USE_48_RAND
      return( number_range( 1,100 ));

#else
    int percent;

    while ( ( percent = number_mm( ) & (128-1) ) > 99 )
	;

    return 1 + percent;  
#endif
}



/*
 * Generate a random door.
 */
int number_door( void )
{
#ifdef USE_48_RAND
  return( number_range( 0, 5) );
#else
    int door;

    while ( ( door = number_mm( ) & (8-1) ) > 5 )
	;
    return door; 
#endif
}



int number_bits( int width )
{
#ifdef USE_48_RAND
  return( number_range( 0, (1<<width)-1 ));
#else
  return number_mm( ) & ( ( 1 << width ) - 1 );
#endif
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static  int     rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState     = &rgiState[2];

    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    srand48( (int) current_time );
    piState[0]  = ((int) current_time) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
#ifdef USE_48_RAND
  return( abs(mrand48()) );

#else
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return abs(iRand >> 6);

#endif
}



/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
 /*   if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }*/
    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && 
        !strcasecmp( (char *)astr, (char *)( bstr + sstr2 - sstr1 ) ) )
	return FALSE;
    else
	return TRUE;
}

/*
 * Returns a string of the specified length, containing at most the first
 * length number of characters of st, will pad if necessary
 * if length is positive, string will be right justified, if length is
 * negative it will left justify
 */
char *str_resize(const char *st,char *buf,int length)
  {
  const char *f;
  char *t;
  int  stLen;
  bool rightJustified;

  buf[0]='\0';
  rightJustified=(length>=0);
  if(!rightJustified)
    length=-length;
  stLen=strlen(st);
  if((!rightJustified)||(stLen>=length))
    {/* normal or left justified */
    for(f=st,t=buf;(*f!=0)&&(length>0);f++,t++,length--)
      *t=*f;
    if(length>0)
      {
      for(;(length>0);length--,t++)
	*t=' ';
      }
    *t='\0';
    }
  else if(length!=0)
    {/* right justified */
    for(t=buf;(length>stLen);t++,length--)
      *t=' ';
    if(length!=0)
      {
      for(f=st;(*f!=0)&&(length>0);f++,t++,length--)
	*t=*f;
      }
    *t='\0';
    }
  return buf;
  }



/*
 * Returns an initial-capped string.
 *   !!!!!  Do not use more than one capitalize in a sprintf or other
            similar command.  Only the last one is read for all values,
            since the static variables are all set before the sprintf puts
            everything together.         Chaos 12/26/94
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    char *pti, *pto;

    if(str==NULL)
      {
      *strcap='\0';
      return strcap;
      }
    pti=(char *)str;
    pto=(char *)strcap;
    for ( ; *pti != '\0'; pti++, pto++ )
       *pto=*pti;
    *pto='\0';
    *strcap = UPPER(*strcap);
    return strcap;
}


char *capitalize_name( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    char *pti, *pto;

    if(str==NULL)
      {
      *strcap='\0';
      return strcap;
      }
    pti=(char *)str;
    pto=(char *)strcap;
    for ( ; *pti != '\0'; pti++, pto++ )
       *pto=LOWER(*pti);
    *pto='\0';
    *strcap = UPPER(*strcap);
    return strcap;
}



/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *fname, FILE *fp, char *str )
{

    if ( ( ch!=NULL && IS_NPC(ch)) || str==NULL || str[0] == '\0' )
	return;

  fclose( fpAppend );

  if( fp == NULL )
    fp = fopen( fname, "a");

  if( fp == NULL )
    {
    perror( fname );
    fpAppend = fopen( NULL_FILE, "r");
    return;
    }
 
  if( ch == NULL )
    {
    fprintf( fp, "%s\n", str );
    fflush( fp );
    fpAppend = fopen( NULL_FILE, "r");
    return;
    }

	fprintf( fp, "[%5u] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );

    fflush( fp );

    fpAppend = fopen( NULL_FILE, "r");

    return;
}


extern char lastplayercmd[MAX_INPUT_LENGTH*2];

/*
 * Reports a bug.        -1 param does not put in logs.
 */
void bug( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
        int iLine;
        int iChar;

        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
            iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );
            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( getc( fpArea ) != '\n' )
                    ;
            }
            fseek( fpArea, iChar, 0 );
        }

        sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
        log_string( buf );

        if ( stat( SHUTDOWN_FILE, &fst ) != -1 )        /* file exists */
        {
            if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
            {
                fprintf( fp, "[*****] %s\n", buf );
                fclose( fp );
            }
        }
    }

    strcpy( buf, "[*****] BUG: " );
    {
        va_list param;

        va_start(param, str);
        vsprintf( buf + strlen(buf), str, param );
        va_end(param);
    }
    log_string( buf );
    log_string( lastplayercmd );

    fclose( fpReserve );
    if ( ( fp = fopen( BUG_FILE_T, "a") ) != NULL )
    {
        fprintf( fp, "%s\n", buf );
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r");
    return;
}

/*
 * Writes a string to the log.
 */
void log_string( char *str )
{
  char *strtime;
  CHAR_DATA  *fch;
  PLAYER_GAME *fpl;

  for(fpl=first_player ; fpl!=NULL ; fpl=fpl->next )
    {
      fch=fpl->ch;
      if(IS_NPC(fch)) continue;
      if( !IS_SET( fch->pcdata->player2_bits, PLR2_HEARLOG )|| 
           !IS_IMMORTAL(fch) )
        continue;
      if( fch->ansi==1)
        ch_printf( fch, "\033[0;1;33mLog: %s\n\r", justify (str) );
      else
        ch_printf( fch, "Log: %s\n\r", justify (str) );
    }

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-6] = '\0';
    fprintf( stderr, "%s- %s\n", strtime, str );
    return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

int mprog_name_to_type ( char *name )
{
   if ( !strcasecmp( name, "in_file_prog"   ) )    return IN_FILE_PROG;
   if ( !strcasecmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !strcasecmp( name, "social_prog"    ) )    return SOCIAL_PROG;
   if ( !strcasecmp( name, "speech_prog"    ) )    return SPEECH_PROG;
   if ( !strcasecmp( name, "rand_prog"      ) )    return RAND_PROG;
   if ( !strcasecmp( name, "fight_prog"     ) )    return FIGHT_PROG;
   if ( !strcasecmp( name, "hitprcnt_prog"  ) )    return HITPRCNT_PROG;
   if ( !strcasecmp( name, "death_prog"     ) )    return DEATH_PROG;
   if ( !strcasecmp( name, "kill_prog"      ) )    return KILL_PROG;
   if ( !strcasecmp( name, "entry_prog"     ) )    return ENTRY_PROG;
   if ( !strcasecmp( name, "greet_prog"     ) )    return GREET_PROG;
   if ( !strcasecmp( name, "all_greet_prog" ) )    return ALL_GREET_PROG;
   if ( !strcasecmp( name, "group_greet_prog" ) )  return GROUP_GREET_PROG;
   if ( !strcasecmp( name, "give_prog"      ) )    return GIVE_PROG;
   if ( !strcasecmp( name, "bribe_prog"     ) )    return BRIBE_PROG;
   if ( !strcasecmp( name, "range_prog"     ) )    return RANGE_PROG;
   if ( !strcasecmp( name, "time_prog"      ) )    return TIME_PROG;

   return( ERROR_PROG );
}

/* This routine reads in scripts of MOBprograms from a file */

MPROG_DATA* mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MOBProgfile[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg2;
  FILE       *progfile;
  char        letter;
  bool        done = FALSE;

  sprintf( MOBProgfile, "%s%s", MOB_DIR, f );

  progfile = fopen( MOBProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %u couldnt open mobprog file", pMobIndex->vnum );
     abort( );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mobprog file.", 0 );
       abort( );
     break;
    default:
       bug( "in mobprog file syntax error.", 0 );
       abort( );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "mobprog file type error", 0 );
	abort( );
      break;
     case IN_FILE_PROG:
	bug( "mprog file contains a call to file.", 0 );
	abort( );
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );

        expand_mob_prog( pMobIndex, mprg2 );  /* Tokenize Mobprog */

	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE(mprg2->next, MPROG_DATA, 1);
	     mprg2       = mprg2->next;
	     mprg2->next = NULL;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in mobprog file syntax error.", 0 );
	     abort( );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Snarf a MOBprogram section from the area file.
 */
void load_mobprogs( FILE *fp )
{
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mobprogs: bad command '%c'.",letter);
      abort();
      break;
    case 'S':
    case 's':
      fread_to_eol( fp ); 
      return;
    case '*':
      fread_to_eol( fp ); 
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mobprogs: vnum %u doesnt exist", value );
	abort( );
      }
    
      /* Go to the end of the prog command list if other commands
	 exist */

      if ( ( original = iMob->mobprogs ) )
	for ( ; original->next != NULL; original = original->next );

      CREATE(working, MPROG_DATA, 1);
      if ( original )
	original->next = working;
      else
	iMob->mobprogs = working;
      working       = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

} 

/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;
  char *pts;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_mobiles: vnum %u MOBPROG char", pMobIndex->vnum );
      abort( );
  }
  CREATE(pMobIndex->mobprogs, MPROG_DATA, 1);
  mprg = pMobIndex->mobprogs;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_mobiles: vnum %u MOBPROG type.", pMobIndex->vnum );
	abort( );
      break;
     case IN_FILE_PROG:
        pts = fread_string( fp );
	mprg = mprog_file_read( pts, mprg,pMobIndex );
	fread_to_eol( fp );
        STRFREE (pts );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
    	     CREATE(mprg->next, MPROG_DATA, 1);	
	     mprg       = mprg->next;
	     mprg->next = NULL;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %u bad MOBPROG.", pMobIndex->vnum );
	     abort( );
	   break;
	}
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
        expand_mob_prog( pMobIndex, mprg );  /* Tokenize Mobprog */
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE(mprg->next, MPROG_DATA, 1);
	     mprg       = mprg->next;
	     mprg->next = NULL;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %u bad MOBPROG.", pMobIndex->vnum );
	     abort( );
	   break;
	}
      break;
    }
  }

  return;

}

void load_victors( void )
{
  FILE *fp;
  int i;
  char *ptx;

  fclose( fpReserve );

  fp = fopen( VICTORY_LIST, "r" );
  if( fp == NULL )
  {
    fpReserve = fopen( NULL_FILE, "r" );
    log_string( "victor.txt load unsuccessful" );
    return;
  }
  for (i=0;i<VICTORY_LIST_SIZE;i++)
  {
    if (victory_list[i][0] == '\0')
    {
	ptx = fread_string( fp );
	if (ptx[0]=='-' && ptx[1] =='1' )
	{
	   STRFREE(ptx);
	   break;
	}
	STRFREE (victory_list[i] );
	victory_list[i] = STRALLOC( ptx );
	STRFREE( ptx );
    }
  }
  fclose(fp);
  
  log_string( "victor.txt loaded successfully" );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void save_victors( void)
{
  FILE *fp;
  int i;

  fclose( fpReserve );

  fp=fopen( VICTORY_LIST_TMP, "w");
  if( fp == NULL )
    {
    fpReserve = fopen( NULL_FILE, "r");
    return;
    }
  for (i=0;i<VICTORY_LIST_SIZE;i++)
  {
    if (!strcasecmp(victory_list[i], ""))
      continue;
    fprintf (fp, "%s~\n", victory_list[i]);
  }
  fprintf(fp, "-1\nXXXXXXXXXX\n#Victorlist\n");
  
    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fp );
    if( is_valid_save( VICTORY_LIST_TMP, "Victorlist" ) )
      {
      rename(VICTORY_LIST_TMP, VICTORY_LIST );
      log_string( "victor.txt saved successfully" );
      }
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void save_sites( void)
{
  FILE *fp;
  /*BAN_DATA *bp;*/
  int hour;
  int tester;

  fclose( fpReserve );

  fp=fopen( "siteban.new", "w");
  if( fp == NULL )
    {
    fpReserve = fopen( NULL_FILE, "r");
    return;
    }
  tester = fprintf( fp, "%u\n", Current_pvnum);
  if( tester == EOF )
    {
    fclose( fp );
      log_string( "siteban.lst save unsuccessful" );
      fpReserve = fopen( NULL_FILE, "r");
    return;
    }
  tester = fprintf( fp, "NULL\n");
  if( tester == EOF )
    {
    fclose( fp );
      log_string( "siteban.lst save unsuccessful" );
      fpReserve = fopen( NULL_FILE, "r");
    return;
    }
  /* write usage info */
  for(hour=0;hour<24;hour++)
    {
    tester=fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld\n", usage.players[hour][0], 
	    usage.players[hour][1], usage.players[hour][2], 
	    usage.players[hour][3], usage.players[hour][4],
	    usage.players[hour][5], usage.players[hour][6]);
    if( tester == EOF )
      {
      fclose( fp );
      log_string( "siteban.lst save unsuccessful" );
      fpReserve = fopen( NULL_FILE, "r");
      return;
      }
    }
  for(hour=0;hour<24;hour++)
    {
    tester = fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld\n", usage.recons[hour][0], 
	    usage.recons[hour][1], usage.recons[hour][2], 
	    usage.recons[hour][3], usage.recons[hour][4],
	    usage.recons[hour][5], usage.recons[hour][6]);
    if( tester == EOF )
      {
      fclose( fp );
      log_string( "siteban.lst save unsuccessful" );
        fpReserve = fopen( NULL_FILE, "r");
      return;
      }
    }
    fprintf( fp, "\nXXXXXXXXXX\n#Siteban\n" );

    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fp );
    if( is_valid_save( "siteban.new", "Siteban" ) )
      {
      rename( "siteban.new", "siteban.lst" );
      log_string( "siteban.lst saved successfully" );
      }

      fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void load_sites( void)
{
  FILE *fp;
  BAN_DATA *bp;
  char buf[MAX_STRING_LENGTH];
  int done=0, day, hour;

#ifdef undef
  BITVECTOR_DATA *bit;
  log_string( "Loading bitvector list" );
  fp=fopen( "bitvector.lst", "r");
  if(fp!=NULL)
    {
    done=0;
    while(done==0)
      {
      strcpy(buf, fread_word(fp));
      if(!strcasecmp(buf,"NULL"))
	done=1;
      else
	{
	CREATE(bit, BITVECTOR_DATA, 1);
	bit->name   = STRALLOC( buf);
        bit->value  = fread_number( fp);
	LINK(bit, first_bitvector, last_bitvector, next, prev);
	}
      }
    fclose(fp);
    }
#endif
  done=0;
  log_string( "Loading siteban list" );
  fp=fopen( "siteban.lst", "r");
  if(fp!=NULL)
    {
    Current_pvnum=fread_number( fp);
    while(done==0)
      {
      strcpy(buf, fread_word(fp));
      if(!strcasecmp(buf,"NULL"))
	done=1;
      else
	{
	CREATE(bp, BAN_DATA, 1);
	bp->next=ban_list;
	ban_list=bp;
	bp->name=STRALLOC( buf);
	}
      }
    /* read in usage info */
    for(hour=0;hour<24;hour++)
      {
      for(day=0;day<7;day++)
	{
	usage.players[hour][day]=fread_number(fp);
	}
      }
    for(hour=0;hour<24;hour++)
      {
      for(day=0;day<7;day++)
	{
	usage.recons[hour][day]=fread_number(fp);
	}
      }
    fclose(fp);
    }
  first_player=NULL;

  return;
}

void create_menu_tree()
      {
      int ptmode;
      char *pt;
      int cnt, counter;
      char line[200];
      HELP_DATA *pHelp,*tHelp;
      HELP_MENU_DATA *menu;
      char buf[ MAX_STRING_LENGTH ];
/*  This is to debug help stuff
      counter=0;
      for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next,counter++ )
          {
          sprintf( buf, "Help %d<:%s>strlen=%d", counter, pHelp->keyword,
              strlen( pHelp->text) );
          log_string( buf );
          } */
      counter=0;
      for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next,counter++ )
	{
	ptmode=0;
        /* if( counter%100==0 )
          {
          sprintf( buf, "Help counter %d/%d.", counter, top_help );
          log_string( buf );
          } */
        strcpy( buf, pHelp->text );
	pt=buf;
	while( ptmode!=3 && *pt!='\0')
	  {
	  if( *pt=='{' && ptmode==0)
	    ptmode=1;
	  else
	    if( *pt=='}' && ptmode==2)
	      ptmode=3;
	    else
	      if( ptmode==1 )
		 ptmode=2;
	      else
		ptmode=0;

	 if( ptmode!=3)
	   {
	   pt++;
	   continue;
	   }

	   pt-=3;
	   *pt='\0';
	   pt++;
	   ptmode=0;
           STRFREE (pHelp->text );
           pHelp->text =STRALLOC(buf );

	   /* find menu items here */
	     cnt=0;
	     while( *pt!='\r' && *pt!='\n' && *pt!='\0')
	       {
	       line[cnt]=*pt;
	       cnt++;
	       pt++;
	       }
	     line[cnt]='\0';
	     if( line[0]=='{' && line[2]=='}')
	       for ( tHelp = help_first; tHelp != NULL; tHelp = tHelp->next )
		 if ( is_name( &line[3], tHelp->keyword ))
		     /*is_name_short( &line[3], tHelp->keyword))*/
	       {
	       CREATE(menu, HELP_MENU_DATA, 1);
               if( line[1] >= 'A' && line[1] <= 'Z' )
                 line[1] += ( 'a' - 'A' );
	       menu->option = line[1];
	       menu->help = tHelp;
	       if( pHelp->menu==NULL)
	         pHelp->menu = menu;
	       else
		 {
		 menu->next=pHelp->menu;
		 pHelp->menu=menu;
		 }
	       }
	 }
       }     
      return;
   }

int find_command( char *cmd_str )
  {
  int cmd;
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    if(!str_prefix( cmd_str, cmd_table[cmd].name ))
      return( cmd );
  return( -1);
  }


OBJ_PROG * load_object_program( FILE *fp)
  {
  char keyword;
  OBJ_PROG *prg;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  CREATE(prg, OBJ_PROG, 1);

  prg->index = fread_number( fp );  /* get index number */
  keyword = (char)fread_number( fp );  /* get trigger command type */
  switch( keyword )
    {
    case 'C':  /* game command */
      prg->percentage = fread_number( fp );  /* get chance number */
      strcpy( buf, fread_word( fp ) );
      prg->cmd = find_command( buf ) ;
      if( prg->cmd == -1)
	{
        sprintf( buf2, "Bad  obj_command command name: %s",  buf );
	log_string( buf2 );
	abort();
	}
      break;
    case 'U':    /* unknown command or social */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->unknown = STRALLOC(fread_word( fp )) ;
      prg->cmd = -2;
      break;
    case 'T':    /* Tick check */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->cmd = -3;
      break;
    case 'X':   /* void trigger */
      prg->cmd = -4;
      break;
    case 'H':    /* Got hit check */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->cmd = -5;
      break;
    case 'D':    /* Damaged another check */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->cmd = -6;
      break;
    case 'W':    /* Wear item check */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->cmd = -7;
      break;
    case 'R':    /* Remove item check */
      prg->percentage = fread_number( fp );  /* get chance number */
      prg->cmd = -8;
      break;
   default:
	log_string( "Bad obj_command type");
   }

  prg->obj_command = (char)fread_number( fp );  /* get reaction command */
  switch( prg->obj_command )
    {
    case 'E':    /* screen echo */
      prg->argument = fread_string( fp ) ;
      break;
    case 'C':    /* user command at level 99 without arg, but with multi-line*/
      prg->argument = fread_string( fp ) ;
      break;
    case 'G':    /* user command at level 99 with argument */
      prg->argument = fread_string( fp ) ;
      break;
    case 'S':   /* Set quest bit to value */
      prg->quest_offset = fread_number( fp );
      prg->quest_bits = fread_number( fp );
      prg->if_value = fread_number( fp );
      break;
    case 'D':   /* Add to quest bit */
      prg->quest_offset = fread_number( fp );
      prg->quest_bits = fread_number( fp );
      prg->if_value = fread_number( fp );
      break;
    case 'P':   /* Player quest bit if check */
      prg->quest_offset = fread_number( fp );
      prg->quest_bits = fread_number( fp );
      prg->if_symbol = fread_letter( fp );
      prg->if_value = fread_number( fp );
      prg->if_true = fread_number( fp );
      prg->if_false = fread_number( fp );
      break;
    case 'Q':   /* Object quest bit if check */
      prg->quest_offset = fread_number( fp );
      prg->quest_bits = fread_number( fp );
      prg->if_symbol = fread_letter( fp );
      prg->if_value = fread_number( fp );
      prg->if_true = fread_number( fp );
      prg->if_false = fread_number( fp );
      break;
    case 'H':   /* If has object check */
      prg->if_value = fread_number( fp );
      prg->if_true = fread_number( fp );
      prg->if_false = fread_number( fp );
      break;
    case 'I':   /* If check */
      prg->if_check = (char)fread_number( fp );
      prg->if_symbol = fread_letter( fp );
      prg->if_value = fread_number( fp );
      prg->if_true = fread_number( fp );
      prg->if_false = fread_number( fp );
      break;
    case 'A':   /* Apply to temp stats */
      prg->if_check = (char)fread_number( fp );
      prg->if_value = fread_number( fp );
      break;
    case 'J':   /* Junk the item */
      break;
   default:
	log_string( "Bad obj_command reaction type");
   }

  return( prg );
  
  }

void obj_prog_if_dest( OBJ_INDEX_DATA *obj )
  {
  OBJ_PROG *prg, *dest;

  for( prg = obj->obj_prog; prg!=NULL; prg=prg->next )
    {
    prg->true = NULL;
    prg->false = NULL;
    if( prg->obj_command == 'H' || 
        prg->obj_command == 'I' || 
        prg->obj_command == 'P' || 
        prg->obj_command == 'Q' )
      {
      for( dest = prg->next; prg->true==NULL && dest!=NULL; dest=dest->next )
	if( prg->if_true == dest->index )
	{
	  prg->true = dest;
	}
      for( dest = prg->next; prg->false==NULL && dest!=NULL; dest=dest->next )
	if( prg->if_false == dest->index )
	{
	  prg->false = dest;
	}
      }
    else
      {  /* scan for continuing programs and use true_dest for prg */
      for( dest = prg->next; prg->true==NULL && dest!=NULL; dest=dest->next )
	if( prg->index == dest->index )
	  prg->true = dest;
      }
   }
  return;
  } 


char * load_picture( FILE *fp )
  {
  char *pt, *bt;
  int x, y;

  CREATE(bt, char, 3121);
  pt = bt;

  for( y=0; y<40; y++)
    for( x=0; x<78; x++)
      {
      *pt = fread_letter( fp ) - '0';
      pt++;
      }

  return( bt );
  }
  
  
int get_alloc_size_perm( unsigned char *str )
  {
  unsigned char iBlock;

     if( str==NULL || str == (unsigned char*)&str_empty[0]  )
	    return(0);

   iBlock = (unsigned char)( ( (int)*(str-4) + 
        (int)*(str-3) + (int)*(str-2) ) % 256 ) ; 

    if( *(str-3)=='P' && iBlock == *(str-1) )  
      return( (int)*(str-2) );

   return( -1 );
   }

int get_alloc_size( unsigned char *str )
  {
  unsigned char crc;

     if( str==NULL || str == (unsigned char*)&str_empty[0]  )
	    return(0);

    crc = (unsigned char)( ( (int)*(str-4) + 
        (int)*(str-3) + (int)*(str-2) ) % 256 ) ; 

    if( ( *(str-3)=='D' || *(str-3)=='T' || *(str-3)=='S' )
        && crc == *(str-1) )  
      return( (int)*(str-2) );

   return( -1 );
   }

int get_string_size( unsigned char *str )
  {
  unsigned char crc;

     if( str==NULL || str == (unsigned char*)&str_empty[0]  )
	    return(0);

    crc = (unsigned char)( ( (int)*(str-4) + 
        (int)*(str-3) + (int)*(str-2) ) % 256 ) ; 

    if( *(str-3)=='d' && crc == *(str-1) )  
      return( (int)*(str-2) );
    if( *(str-3)=='S' && crc == *(str-1) )  
      return( (int)*(str-2) );

   return( -1 );
   }

int get_string_size_perm( unsigned char *str )
  {
  unsigned char crc;

     if( str==NULL || str == (unsigned char*)&str_empty[0]  )
	    return(0);

    crc = (unsigned char)( ( (int)*(str-4) + 
        (int)*(str-3) + (int)*(str-2) ) % 256 ) ; 

    if( *(str-3)=='p' && crc == *(str-1) )  
      return( (int)*(str-2) );

   return( -1 );
   }

int get_free_size( unsigned char *str )
  {
  unsigned char crc;

     if( str==NULL || str == (unsigned char*)&str_empty[0]  )
	    return(0);

    crc = (unsigned char)( ( (int)*(str-4) + 
        (int)*(str-3) + (int)*(str-2) ) % 256 ) ; 

    if( *(str-3)=='F' && crc == *(str-1) )  
      return( (int)*(str-2) );

   return( -1 );
   }


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}

     
int str_cat_max( const char *out_str, const char *add_str, int max_size )
  {
  register char *str_pt1, *str_pt2;
  register int str_actual, str_size;

     /* Set limits of size, and subtract for the terminator  */
  str_actual = UMIN(UMAX( 1, max_size ), MAX_STRING_LENGTH ) - 1;

  for( str_size=0, str_pt1=(char *)out_str; *str_pt1!='\0'; 
                            str_pt1++, str_size++)
    if( str_size == str_actual )
      {
      *str_pt1= '\0';
      return( str_size );
      }

   
  for( str_pt2=(char *)add_str ; *str_pt2!='\0'; 
                        str_pt1++, str_pt2++, str_size++)
    if( str_size == str_actual )
      {
      *str_pt1= '\0';
      return( str_size );
      }
    else
      *str_pt1=*str_pt2;
    
  *str_pt1='\0';
  return( str_size );
  }

     
int char_apd_max( const char *out_str, const char *add_str, int start, 
                          int max_size )
  {
  register char *str_pt1;
  register int str_actual;

     /* Set limits of size, and subtract for the terminator  */
  str_actual = UMIN(UMAX( 1, max_size ), MAX_STRING_LENGTH ) - 1;
  if( str_actual <= start )
    {
    *((char *)out_str + str_actual) = '\0';
    return( str_actual );
    }

  str_pt1 =(char *) ((int)out_str+start);
  *str_pt1=*add_str; 
  *(str_pt1+1)='\0';

  return( start+1 );
  }

int str_apd_max( const char *out_str, const char *add_str, int start,
                          int max_size )
  {
  register char *str_pt1, *str_pt2;
  register int str_actual, str_size;

     /* Set limits of size, and subtract for the terminator  */
  str_actual = UMIN(UMAX( 1, max_size ), MAX_STRING_LENGTH ) - 1;
  if( str_actual <= start )
    {
    *((char *)out_str + str_actual) = '\0';
    return( str_actual );
    }

  str_pt1 =(char *) ((int)out_str+start);
  str_size = start;

  for( str_pt2=(char *)add_str ; *str_pt2!='\0';
                   str_pt1++, str_pt2++, str_size++)
    if( str_size == str_actual )
      {
      *str_pt1= '\0';
      return( str_size );
      }
    else
      *str_pt1=*str_pt2;

  *str_pt1='\0';
  return( str_size );
  }

     
int str_cpy_max( const char *out_str, const char *add_str, int max_size )
  {
  register char *str_pt1, *str_pt2;
  register int str_actual, str_size;

     /* Set limits of size, and subtract for the terminator  */
  str_actual = UMIN(UMAX( 1, max_size ), MAX_STRING_LENGTH ) - 1;

  for( str_size=0, str_pt1=(char *)out_str, str_pt2=(char *)add_str ;
                     *str_pt2!='\0'; str_pt1++, str_pt2++, str_size++)
    if( str_size == str_actual )
      {
      *str_pt1= '\0';
      return( str_size );
      }
    else
      *str_pt1=*str_pt2;
    
  *str_pt1='\0';
  return( str_size );
  }


void expand_mob_prog( MOB_INDEX_DATA *mind, MPROG_DATA *mprog )
  {
  char buf[MAX_INPUT_LENGTH];
  char *pti, *pto;
  char level;

  pti = mprog->comlist;
  level = 0;
  while( *pti != '\0' )
    {
    while( *pti=='\n' || *pti=='\r')
      pti++;
    if( *pti != '\0' )
      {
      for( pto=buf; *pti!='\r' && *pti!='\n' && *pti!='\0'; pti++, pto++)
        *pto=*pti;
      *pto='\0';
      level = expand_line_mprog( mind, mprog, buf, FALSE, level );
      }
    }


  /* When working, remove the comlist field here to conserve memory */
  STRFREE (mprog->comlist );
  return;
  }

char expand_line_mprog( MOB_INDEX_DATA *mind, MPROG_DATA *mprog, char *line,
      bool iForce, char level )
  {
  char *pti, *pto;
  char buf[MAX_INPUT_LENGTH];
  char string[MAX_INPUT_LENGTH];
  int type;
  void *func;
  int value;
  NPC_TOKEN *curr, *last;
  char achange, bchange;
  char *bkpt;

  func = NULL;
  for( pti=line; *pti==' '; pti++);  /* remove initial spaces */

  for( pto=buf; isalnum((int)*pti) ; pti++, pto++)
    *pto = *pti;
  *pto = '\0';

  if( *buf == '\0' )
    return(level);

  type = 0;   /* Error type */
  value = 0;
  achange = 0;
  bchange = 0;
  if( !strcasecmp( buf, "if" ) && !iForce )
    {
    type = 3;   /* IF type */
    achange = 1;
    }
  else
  if( !strcasecmp( buf, "or" ) && !iForce )
    {
    type = 4;   /* OR type */
    bchange =-1;
    achange = 1;
    }
  else
  if( !strcasecmp( buf, "else" ) && !iForce )
    {
    type = 5;   /* ELSE type */
    bchange =-1;
    achange = 1;
    }
  else
  if( !strcasecmp( buf, "endif" ) && !iForce )
    {
    type = 6;   /* ENDIF type */
    bchange =-1;
    }
  else
  if( !strcasecmp( buf, "break" ) && !iForce )
    {
    type = 7;   /* BREAK type */
    }
  else
  if( !strcasecmp( buf, "unknown" ) )
      {
      value = 0;
      type = 0;
      }
  else
    {
        /* Either an interp or social */
    value = find_command( buf );

    if( value > -1 )    /* Found command */
      {
      func = (void *)cmd_table[value].do_fun;
      type = 2;  /* COMMAND type */
      }
    else
      {  /* check for social */
      for( value = 0; *social_table[value].name != '\0'; value++ )
        if( !str_prefix( social_table[value].name, buf) )
         {
         func = (void *)social_table[value].name;  /* fill to != NULL */
         type = 1;  /* SOCIAL type */
         break;
         }
      }

    if( func == NULL )
      {
      value = 0;
      type = 0;
      }

    }

  CREATE(curr, NPC_TOKEN, 1);
  curr->next  = NULL;
  curr->type = type;
  curr->function = func;
  curr->value = value;
  
  if( mprog->token_list == NULL )
    {
    mprog->token_list = curr;
    last = NULL;
    curr->line = 1;
    }
  else
    {
    for( last = mprog->token_list; last->next != NULL;
        last = last->next );
    last->next = curr;
    curr->line = last->line +1;
    } 

  level += bchange;      /* before change */
  if( level < 0 )
    level = 0;
  curr->level = level;
  level += achange;      /* after change */
  
  if( type == 1 || type == 2)  /* Possible multi-line */
    {
    while( *pti==' ')
      pti++;
    for( pto=string; *pti!='\0' && *pti!='\r' && *pti!='\n' && *pti!='&' ;
             pti++, pto++)
      *pto=*pti;
    *pto = '\0';
    for( pto = string; *pto== ' '; pto++);
    if( *pto=='\0' )
      curr->string = NULL;
    else
      curr->string = STRALLOC( pto );
    if( *pti == '&' )
      {
      pti++;
      level = expand_line_mprog( mind, mprog, pti, TRUE, level);
      }
    }
  else
    {
    while( *pti==' ')
      pti++;
    for( pto=string; *pti!='\0' && *pti!='\r' && *pti!='\n' ; pti++, pto++)
      *pto=*pti;
    *pto = '\0';
    for( pto = string; *pto== ' '; pto++);
    if( *pto=='\0' )
      curr->string = NULL;
    else
      {
      if( type == 3 || type == 4 )
        {
        bkpt = NULL ;
        if( bkpt == NULL )
          bkpt = strstr( pto, " OR ");
        if( bkpt == NULL )
          bkpt = strstr( pto, " Or ");
        if( bkpt == NULL )
          bkpt = strstr( pto, " oR ");
        if( bkpt == NULL )
          bkpt = strstr( pto, " or ");
        if( bkpt != NULL )
          {
          *bkpt = '\0';
          pti = bkpt +1;
          level = expand_line_mprog( mind, mprog, pti, FALSE, level);
          }
        }
      curr->string = STRALLOC( pto );
      }
    }
 
  return(level);
  }

void create_object_reference( OBJ_DATA *obj )
  {
  int val;

  val = number_range(0,255) + number_range(0,255)*256;
  val += 256*256*number_range(0,255) + number_range(0,63)*256*256*256;

  obj->index_reference[0] = val;

  val = number_range(0,255) + number_range(0,255)*256;
  val += 256*256*number_range(0,255) + number_range(0,63)*256*256*256;

  obj->index_reference[1] = val;

  return;
  }

  
bool delete_room( ROOM_INDEX_DATA *room )
{
  int hash, door;
  ROOM_INDEX_DATA *prev, *limbo = get_room_index(ROOM_VNUM_LIMBO);
  OBJ_DATA *o;
  CHAR_DATA *ch;
  EXTRA_DESCR_DATA *ed;
  EXIT_DATA *pexit, *pexit_rev;
  RESET_DATA *next_reset, *pReset;

  for (pReset=room->area->first_reset;pReset!=NULL;pReset=next_reset)
  {
   next_reset = pReset->next;
   if ( ( (pReset->command == 'M' 
     || pReset->command == 'O' )
    && pReset->arg3 == room->vnum) ||
     ( ( pReset->command == 'D' ||
 	 pReset->command == 'R' )
    && pReset->arg1 == room->vnum) )
    {
#ifdef UNLINKCHECK
     RESET_DATA *tReset;
     bool foundr;

    for( foundr=FALSE, tReset=room->area->first_reset; tReset!=NULL;
      tReset=tReset->next )
      if( tReset==pReset )
        {
        foundr = TRUE;
        break;
        }
     if( foundr )
       UNLINK(pReset,room->area->first_reset,room->area->last_reset,next,prev);
     else
       bug( "UNLINK ERROR could not find room %d reset.", room->vnum );
#else
       UNLINK(pReset,room->area->first_reset,room->area->last_reset,next,prev);
#endif
     DISPOSE(pReset);
     top_reset--;
    }
  }

  while ((ch = room->first_person) != NULL)
  {
    if (!IS_NPC(ch))
    {
      char_from_room(ch);
      char_to_room(ch, limbo);
    }
    else
      extract_char(ch, TRUE);
  }
  while ((o = room->first_content) != NULL)
    extract_obj(o);
  
  if (room->first_extradesc !=NULL)
  while ((ed = room->first_extradesc) != NULL)
  {
    room->first_extradesc = ed->next;
    STRFREE (ed->keyword);
    STRFREE (ed->description);
    DISPOSE(ed );
    --top_ed;
  }

  for (door=0;door<=5;door++)
   {
    if ( ( pexit = room->exit[door] ) == NULL) 
      continue;
    
    if ( pexit->to_room != NULL 
        && (pexit_rev = pexit->to_room->exit[rev_dir[door]] ) != NULL )
     {
      fprintf(stderr, "delete reverse room, direction %d, room name %s\n", 
	rev_dir[door], pexit->to_room->name);
      if (pexit_rev->keyword != NULL) STRFREE( pexit_rev->keyword );
      if (pexit_rev->description != NULL ) STRFREE( pexit_rev->description );
      pexit->to_room->exit[rev_dir[door]]= NULL;
      DISPOSE ( pexit_rev );
      top_exit--;
     }

    if (pexit->keyword !=NULL) STRFREE( pexit->keyword );
    if (pexit->description !=NULL) STRFREE( pexit->description );
    DISPOSE ( pexit );
    top_exit--;
   }

  STRFREE (room->name);
  STRFREE (room->description);

  room_index[room->vnum] = NULL;

  hash = room->vnum%MAX_KEY_HASH;
  if (room == room_index_hash[hash])
    room_index_hash[hash] = room->next;
  else
  {
    for (prev = room_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == room)
        break;
    if (prev)
      prev->next = room->next;
    else
      log_printf("delete_room: room %d not in hash bucket %d.", room->vnum, hash);
  }
  DISPOSE(room );
  --top_room;
  return TRUE;
}

bool delete_obj( OBJ_INDEX_DATA *obj )
{
  int hash;
  OBJ_INDEX_DATA *prev;
  OBJ_DATA *o, *o_next;
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *af;

  RESET_DATA *next_reset, *pReset;

  for (pReset=obj->area->first_reset;pReset!=NULL;pReset=next_reset)
  {
   next_reset = pReset->next;
   if ( ( (pReset->command == 'G' 
     || pReset->command == 'O' 
     || pReset->command == 'P' 
     || pReset->command == 'E' )
    && pReset->arg1 == obj->vnum) ||
     (  pReset->command == 'P' 
    && pReset->arg3 == obj->vnum) )
    {
#ifdef UNLINKCHECK
     RESET_DATA *tReset;
     bool foundr;

    for( foundr=FALSE, tReset=obj->area->first_reset; tReset!=NULL;
      tReset=tReset->next )
      if( tReset==pReset )
        {
        foundr = TRUE;
        break;
        }
     if( foundr )
       UNLINK(pReset, obj->area->first_reset, obj->area->last_reset, next,prev);
     else
       bug( "UNLINK ERROR could not find obj %d reset.",obj->vnum);
#else
     UNLINK(pReset, obj->area->first_reset, obj->area->last_reset, next,prev);
#endif
     DISPOSE(pReset);
     top_reset--;
    }
  }
  /* Remove references to object index */
  for (o = first_object; o; o = o_next)
  {
    o_next = o->next;
    if (o->pIndexData == obj)
      extract_obj(o);
  }
  while ((ed = obj->first_extradesc) != NULL)
  {
    obj->first_extradesc = ed->next;
    STRFREE (ed->keyword);
    STRFREE (ed->description);
    DISPOSE(ed);
    --top_ed;
  }
  while ((af = obj->first_affect) != NULL)
  {
    obj->first_affect = af->next;
    DISPOSE(af );
    --top_affect;
  }
  STRFREE (obj->name);
  STRFREE (obj->short_descr);
  STRFREE (obj->description);
  STRFREE (obj->long_descr);
  if (obj->attack_string != NULL)
   STRFREE (obj->attack_string);


  obj_index[obj->vnum] = NULL;

  hash = obj->vnum%MAX_KEY_HASH;
  if (obj == obj_index_hash[hash])
    obj_index_hash[hash] = obj->next;
  else
  {
    for (prev = obj_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == obj)
        break;
    if (prev)
      prev->next = obj->next;
    else
      log_printf("delete_obj: object %d not in hash bucket %d.", obj->vnum, hash);
  }
  DISPOSE(obj);
  --top_obj_index;
  return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
  int hash;
  MOB_INDEX_DATA *prev;
  CHAR_DATA *ch, *ch_next;
  SHOP_DATA *pShop;
  RESET_DATA *pReset, *next_reset=NULL, *prev_reset=NULL;

  for (ch = first_char; ch; ch = ch_next)
  {
    ch_next = ch->next;

    if (ch->pIndexData == mob)
      extract_char(ch, TRUE);
  }
  for (pReset=mob->area->first_reset;pReset!=NULL;pReset=next_reset)
  {
   next_reset = pReset->next;
   if (next_reset !=NULL && next_reset->command == 'M' 
    && next_reset->arg1 == mob->vnum)
    {
     prev_reset=pReset;
    } 
   else if (pReset->command == 'M' && pReset->arg1 == mob->vnum)
    {
     prev_reset->next=next_reset;
  fprintf(stderr, "About to free m reset\n");
     DISPOSE(pReset);
     top_reset--;
    }
  }
  for (pReset=next_reset;pReset !=NULL; pReset=next_reset)
  {
   if (pReset->command=='M')
   {
    prev_reset->next=pReset;  
    break;
   }
   if (pReset->command == 'G' || pReset->command == 'E' )
   {
    prev_reset->next=pReset;
  fprintf(stderr, "About to free g or e reset\n");
    DISPOSE( pReset );
    top_reset--;
   }
  }
  if (mob->pShop)
  {
   for (pShop=shop_first;pShop !=NULL;pShop=pShop->next)
   { 
    if (pShop->next == mob->pShop)
     {
      pShop->next = mob->pShop->next;
      break;
     }
    }
  fprintf(stderr, "About to free shop\n");
    DISPOSE(mob->pShop);
    --top_shop;
  }

  STRFREE (mob->player_name);
  STRFREE (mob->short_descr);
  STRFREE (mob->long_descr);
  STRFREE (mob->description);

  mob_index[mob->vnum] = NULL;
  hash = mob->vnum%MAX_KEY_HASH;
  if (mob == mob_index_hash[hash])
    mob_index_hash[hash] = mob->next;
  else
  {
    for (prev = mob_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == mob)
        break;
    if (prev)
      prev->next = mob->next;
    else
      log_printf("delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash);
  }
  fprintf(stderr, "About to free mob\n");
  DISPOSE(mob);
  --top_mob_index;
  return TRUE;
}


/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    unsigned char *pMem;
    register int iList, iSize;
    register unsigned char *pt;
    register int cnt;
    PERM_BLOCK_LIST *pBlock;
    unsigned char iBlock;
    /* FREE_MEM_LIST *fcur, *fmin, *fprev, *fmin_prev;
    int low_blk; */

       /* Do not include size adjust in iList */
    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }
    iSize = rgSizeList[iList];

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Alloc_mem: size %d too large.", sMem );
        abort( );
    }

    if ( rgFreeList[iList] == NULL )
    {
    pMem            = (unsigned char *)alloc_perm( sMem );
    nAllocPerm -= 1;
    sAllocPerm -= sMem;
    rgUsedCount[iList]++;
    }
    else
    {
    pMem              = (unsigned char *)rgFreeList[iList];
    if( get_free_size( (unsigned char *)pMem ) != iList )
        {
        log_string( "ERROR:Alloc_mem: allocating a free with wrong size" );
        memory_dump( (char *)(pMem));

             /* allocate some normal mem */
        pMem            = (unsigned char *)alloc_perm( iSize );
        rgUsedCount[iList]++;

                /* Get rid of the faulty memory */
        rgFreeList[iList] = rgFreeList[ iList]->next;

        total_memory_warnings++;

        if( ABORT_ON_ERROR )
          abort();
        }
    else
      {
      /*   Let's find the farthest away block */
      /*fmin_prev = NULL;
      for(fprev=NULL,fcur=rgFreeList[iList], low_blk=0, fmin=NULL; fcur!=NULL;
          fprev=fcur, fcur=fcur->next  )
        if( (int)(*((unsigned char *)fcur-4)) > low_blk )
          {
          fmin = fcur;
          fmin_prev = fprev;
          low_blk = *((unsigned char *)fcur-4);
          }
      if( fmin_prev == NULL )
        rgFreeList[iList] = rgFreeList[ iList]->next;
      else
        fmin_prev->next = fmin->next;  */


      rgFreeList[iList] = rgFreeList[ iList]->next;
      rgFreeCount[iList]--;
      rgUsedCount[iList]++;
      iBlock = *((unsigned char *)pMem-4);
      pBlock = perm_block_index[ iBlock ];
      if( pBlock==NULL ||  (char *)pBlock->pMemPerm>(char *)pMem || 
         (char *)(MAX_PERM_BLOCK+pBlock->pMemPerm)<(char *)pMem )
        {
        char bufx[MAX_INPUT_LENGTH];

        sprintf( bufx, "ERROR:Free_mem: Outside Range Mem. pBlock=%x ",
               (int)pBlock);
       log_string( bufx );
        memory_dump( (char *)(pMem));
        total_memory_warnings++;
        if( ABORT_ON_ERROR )
          abort();
        return(NULL);
        }

      pBlock->total_used ++;

        /* Hey, ALL routines want memory to be zeroes to start with. */
        /* And assume that only this particular part would not initialize  */
      for( cnt=0, pt=pMem; cnt < iSize ; cnt++, pt++ )
        *pt = 0;

      }
    }

    set_block_type( 'D', pMem );

    return( (void *)pMem);
}



/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 * The pAdd is the pointer to the pointer of the memory.
 */

  /* Make DISPOSE a function so we can use gdb   -  Chaos 4/2/99  */
void DISPOSE( void *pAdd )
  {
  free_mem( &pAdd, 0 );
  return;
  }
void free_mem( void *pAdd, int sMem )
{
    int iList;
    FREE_MEM_LIST *fm;
    void *pMem;
    register int iSize, cnt;
    register unsigned char *pt;
    PERM_BLOCK_LIST *pBlock;
    unsigned char iBlock;

           /* Use address of variable instead of variable */
    pMem =(void *) (* (int *)pAdd);

    if( pMem == NULL || pMem==str_empty )
      return;

    iList = -1;


    if( (iList = get_alloc_size( (unsigned char*)pMem ) ) < 0 )
      if( (iList = get_string_size( (unsigned char*)pMem ) ) < 0 )
        {
        log_string( "ERROR:Free_mem: This was not a valid mem.");
        memory_dump( (char *)(pMem));
        total_memory_warnings++;
        if( ABORT_ON_ERROR )
          abort();
        *(char *)pAdd = 0;
        return;
        }

      iSize = rgSizeList[ iList ];
      for( cnt=0, pt=(char *)pMem; cnt < iSize ; cnt++, pt++ )
        *pt = 0;

        /* always record after header info */
       /* Use the dealloced mem as data spot for freed */
     fm = (FREE_MEM_LIST *)pMem;
     fm->next=rgFreeList[iList];
     rgFreeList[iList]=fm;

     iBlock = *((unsigned char *)pMem-4);
     pBlock = perm_block_index[ iBlock ];

      if( pBlock==NULL ||  (char *)pBlock->pMemPerm>(char *)pMem || 
         (char *)(MAX_PERM_BLOCK+pBlock->pMemPerm)<(char *)pMem )
        {
        char bufx[MAX_INPUT_LENGTH];

        sprintf( bufx, "ERROR:Free_mem: Outside Range Mem. pBlock=%x ",
               (int)pBlock);
        log_string( bufx );
        memory_dump( (char *)(pMem));
        total_memory_warnings++;
        if( ABORT_ON_ERROR )
          abort();
        *(char *)pAdd = 0;
        return;
        }

     pBlock->total_used --;

     rgFreeCount[iList]++;
     rgUsedCount[iList]--;

     set_block_type( 'F', pMem );
    *(char *)pAdd = 0; /* Clear the original variable so no over-writes */

     /*  Let's removed unused memory from pool   -  Chaos 11/11/95 */
     /*  This will shrink the size of the game when first_person leave 
         Note: This code is not incredibly stable.
     if( pBlock != NULL  && pBlock->total_used == 0 )
       {
       FREE_MEM_LIST *fp, *fprev, *fnext;
       int ind;
       PERM_BLOCK_LIST *idBlock, *pvBlock;

       for( ind = 0; ind < MAX_MEM_LIST; ind++)

         {
         fprev = NULL;
         for( fp=rgFreeList[ind]; fp!=NULL; fp=fnext)
           {
           fnext = fp->next;
           pt = (unsigned char *)&fp;
           if( *((unsigned char *)pt-4)==iBlock )
             {
             rgFreeCount[ind]--;
             if( fprev==NULL )
               {
               rgFreeList[ind]=fnext;
               }
             else
               {
               fprev->next = fnext;
               }
             }
           else
             fprev = fp;
           }
         }

       pvBlock = NULL;
       for( idBlock = perm_block_list_list; idBlock!=NULL && idBlock!=pBlock;
         idBlock=idBlock->next )

         pvBlock = idBlock;
       if( idBlock != NULL )
         {
         perm_block_index[ pBlock->block_number ] = NULL;
         if( pvBlock != NULL )
           pvBlock->next = pBlock->next;
         else
           perm_block_list_list = pBlock->next;
         log_string( "Freeing up a memory block." );
         free( pBlock );   
         BLOCKS_REMOVED ++;
         }
       } */

    return;
}





unsigned char *set_block_header( unsigned char iBlock, char typ, int iList,
        unsigned char *pMem )
{

    if( iBlock < 255 )
      *pMem = (unsigned char)iBlock ;   /* Reference block */
    else
      iBlock = *pMem;
    pMem++;         /* Word alignment padding */
    *pMem = (unsigned char)typ;    /* D for dynamic, P for perm */
    pMem++;
    *pMem = (unsigned char) iList;  /* Size value */
    pMem++;
 /* checksum */
    *pMem = (unsigned char)( ( iList + (int)typ + (int)iBlock ) % 256 ) ;
    pMem++;
    return( pMem );
}

void set_block_type( char typ, unsigned char *pMem )
{
  unsigned char iBlock, iList;


    iBlock = *(pMem-4);
    *(pMem-3) = (unsigned char)typ;    /* D dynamic, P perm, s String, F Free*/
    iList = *(pMem-2);
 /* checksum */
    *(pMem-1) = (unsigned char)( ( iList + (int)typ + (int)iBlock ) % 256 ) ;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
    /* higher efficiency version of allocations */

void *alloc_perm( int sMem )
{
  unsigned char *pMem;
  register unsigned char *pt;
  register int sz, iList;
  PERM_BLOCK_LIST *pBlock, *last;
  unsigned char block_index;


    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
          break;
    }

  sMem = rgSizeList[iList];  /* Make size the next block specified */
  sMem += 4;  /* Add for header info */

  while ( sMem % sizeof(long) != 0 )
          sMem++;
  if ( sMem > MAX_PERM_BLOCK )
    {
    bug( "ERROR:Alloc_perm: %d too large.", sMem );
    abort();
    }

  last = NULL;
  for( pBlock=perm_block_list_list; pBlock!=NULL; pBlock=pBlock->next )
    if ( pBlock->iMemPerm + sMem < MAX_PERM_BLOCK )
      break;
    else
      last = pBlock;

  if( pBlock != NULL )
    block_index = pBlock->block_number ;
  else
    {
    for( block_index = 0; block_index < 255 &&
        perm_block_index[ block_index ] != NULL ; block_index++ );
    }

      /* In the current configuration this would mean the game is taking 76M */
  if( block_index == 255 )
    {
    log_string( "Used over 255 blocks of MAX_MEM" );
    abort();
    }

  if ( pBlock == NULL )
    {

          if ( ( pMem = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
            {
            perror( "Alloc_perm" );
            abort( );
            }
         if (pMem<0)
            {
            perror( "Alloc_perm NEG" );
            abort( );
            }
        /* First item is link data */
      pBlock=(PERM_BLOCK_LIST *)pMem;
      pBlock->pMemPerm = pMem;
      pBlock->iMemPerm = sizeof( *pBlock );
      pBlock->next = NULL;
      pBlock->total_used = 0;
      pBlock->block_number = block_index;

                     /* Find the right padding */
     while ( ((unsigned int)( pBlock->pMemPerm + pBlock->iMemPerm ) % 4 ) != 0)
        pBlock->iMemPerm++;


      perm_block_index[ block_index ] = pBlock;
      perm_block_total ++;


      /* Add to list */
    if( last != NULL )
      last->next = pBlock;
    else
      perm_block_list_list = pBlock;
    }

  pMem        = (unsigned char *)( pBlock->pMemPerm + pBlock->iMemPerm );
  pBlock->iMemPerm   += sMem;
  pBlock->total_used   ++;
  nAllocPerm += 1;
  sAllocPerm += sMem;

  if( ((int)pMem % sizeof( long )) != 0 )
    {
    log_string( "ERROR:Alloc_perm: Padding off." );
    abort();
    }

  /* Let's erase the memory here */
  for( sz=0, pt=pMem; sz<sMem; pt++, sz++)
    *pt='\0';

  set_block_header( block_index, 'P', iList, pMem );
  pMem += 4;

  return( (void *)pMem );
}

  /* This function probably will not work. 
int memory_hits( char *ptr )
  {
  int blk, hits;
  PERM_BLOCK_LIST *pBlock, *last;
  unsigned char block_index;

  hits = 0;

  for( pBlock=perm_block_list_list; pBlock!=NULL; pBlock=pBlock->next )
    {
    }

  return( hits );
}  */
      


void memory_dump( char *ptr)
{
  int cnt, length;
  char buf[200];
  char buf2[MAX_STRING_LENGTH];
  unsigned char *start;

  length=48;

  start = (unsigned char*)ptr - length;

  sprintf( buf2, " Memory dump at %x to %x\n", (int)start, (int)start+2*length);

  for( cnt=0; cnt<length*2+4; cnt+=4, start+=4)
    {
    sprintf( buf, "%c %8x   %2x %2x %2x %2x    %3d %3d %3d %3d  %c %c %c %c\n",
         (char *)start==(char *)ptr?'>':' ', (int)start,
         *start, *(start+1), *(start+2), *(start+3),
         *start, *(start+1), *(start+2), *(start+3),
         *start >= ' ' ? ( *start <= '~' ? *start : '*' ) : '*',
         *(start+1) >= ' ' ? ( *(start+1) <= '~' ? *(start+1) : '*' ) : '*',
         *(start+2) >= ' ' ? ( *(start+2) <= '~' ? *(start+2) : '*' ) : '*',
         *(start+3) >= ' ' ? ( *(start+3) <= '~' ? *(start+3) : '*' ) : '*');
    strcat( buf2, buf );
    }

  log_string( buf2 );
  return;
}

void do_memory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int cnt, Ftotal, Xtotal, Utotal, Btotal, Ltotal;
    PERM_BLOCK_LIST *pBlock;
    OBJ_DATA *pobj;

    Ltotal = 0;
    for( Xtotal=0, pBlock=perm_block_list_list, Btotal=0; pBlock!=NULL;
        pBlock=pBlock->next, Btotal++ )
      {
      Xtotal += ( MAX_PERM_BLOCK - pBlock->iMemPerm );
      Ltotal = ( MAX_PERM_BLOCK - pBlock->iMemPerm );
      }

    sprintf( buf , "Affects %5d      ", top_affect    );
    sprintf( buf1, "Areas   %5d      ", top_area      );
    sprintf( buf2, "Exits   %5d\n\r", top_exit      );
    strcat( buf, buf1); strcat( buf, buf2); send_to_char( buf, ch);
    sprintf( buf , "Helps   %5d      ", top_help      );
    sprintf( buf1, "MobsIn  %5d      ", top_mob_index );
    sprintf( buf2, "ObjsIn  %5d Adj%d\n\r", top_obj_index, objects_adjusted );
    strcat( buf, buf1); strcat( buf, buf2); send_to_char( buf, ch);
    sprintf( buf , "Chars   %5d      ", total_characters     );
    sprintf( buf1, "Mobs    %5d      ", total_mobiles      );
    sprintf( buf2, "Objs    %5d \n\r", total_objects);
    strcat( buf, buf1); strcat( buf, buf2); send_to_char( buf, ch);
    sprintf( buf , "Resets  %5d      ", top_reset     );
    sprintf( buf1, "Rooms   %5d      ", top_room      );
    sprintf( buf2, "Shops   %5d      ", top_shop      );
    strcat( buf, buf1);
    strcat( buf, buf2);
    sprintf( buf2, "Rips    %5d\n\r", total_rips);
    strcat( buf, buf2);
    send_to_char( buf, ch);

  sprintf( buf, "Perms   %5d blocks  of %7d bytes.  Blocks %d  Removed %d\n\r",
        nAllocPerm, sAllocPerm, Btotal, BLOCKS_REMOVED );
    send_to_char( buf, ch );

    Utotal=0;
    Ftotal=0;
    for( cnt=0; cnt< MAX_MEM_LIST; cnt++)
      {
      Utotal+=rgSizeList[cnt]*rgUsedCount[cnt];
      Ftotal+=rgSizeList[cnt]*rgFreeCount[cnt];
      }
    sprintf( buf, "Dynamic Memory   %d Used   %d Free   %d Unused   %d Last\n\r",
        Utotal, Ftotal, Xtotal - Ltotal, Ltotal);
    send_to_char( buf, ch );


    strcpy( buf1, "");
    for( cnt=0; cnt<MAX_MEM_LIST; cnt++)
      {
      sprintf( buf, "%5d:%6d-%-5d ", rgSizeList[cnt], rgUsedCount[cnt], 
            rgFreeCount[cnt]);
      strcat( buf1, buf);
      if( cnt%4==3 )
        strcat( buf1, "\n\r");
      }
    send_to_char( buf1, ch );
/*
    sprintf( buf, "Block Type     8    16    24    32    48    64    96   128   192   256   384\n\r");
    send_to_char( buf, ch );
    strcpy( buf1, "Used      ");
    strcpy( buf2, "Free      ");
    for( cnt=0; cnt<MAX_MEM_LIST/2; cnt++)
      {
      sprintf( buf, "%6d", rgUsedCount[cnt]);
      strcat( buf1, buf);
      sprintf( buf, "%6d", rgFreeCount[cnt]);
      strcat( buf2, buf);
      }
    strcat( buf1, "\n\r");
    strcat( buf2, "\n\r");
    send_to_char( buf1, ch);
    send_to_char( buf2, ch); 

    sprintf( buf, "Block Type   512   768  1024  1536    2k    3k    4k    12K  16k   26K   32K\n\r");
    send_to_char( buf, ch );
    strcpy( buf1, "Used      ");
    strcpy( buf2, "Free      ");
    for( cnt=MAX_MEM_LIST/2; cnt<MAX_MEM_LIST; cnt++)
      {
      sprintf( buf, "%6d", rgUsedCount[cnt]);
      strcat( buf1, buf);
      sprintf( buf, "%6d", rgFreeCount[cnt]);
      strcat( buf2, buf);
      }
    strcat( buf1, "\n\r");
    strcat( buf2, "\n\r");
    send_to_char( buf1, ch);
    send_to_char( buf2, ch); */

    sprintf( buf, "CHAR_DATA          %-10d    Total Memory Warnings: %d\n\r", 
        sizeof( * ch),  total_memory_warnings);
    send_to_char( buf, ch);
    sprintf( buf, "DESCRIPTOR_DATA    %d\n\r", sizeof( * ch->desc) );
    send_to_char( buf, ch);
    sprintf( buf, "PCDATA             %d\n\r", sizeof( * ch->pcdata));
    send_to_char( buf, ch);
    sprintf( buf, "OBJ_DATA           %d\n\r", sizeof( * pobj));
    send_to_char( buf, ch);
    send_to_char( hash_stats(argument), ch );
    return;
}

char *LOWER_ALLOC( char *str )
{
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, str);
  *buf = LOWER( *buf );
  return( STRALLOC( str ) );
}
char *UPPER_ALLOC( char *str )
{
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, str);
  *buf = UPPER( *buf );
  return( STRALLOC( str ) );
}
ROOM_INDEX_DATA *make_room( int vnum )
{
        ROOM_INDEX_DATA *pRoomIndex;
        int     iHash, door;

        CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
        pRoomIndex->first_person        = NULL;
        pRoomIndex->last_person         = NULL;
        pRoomIndex->first_content       = NULL;
        pRoomIndex->last_content        = NULL;
        pRoomIndex->first_extradesc     = NULL;
        pRoomIndex->last_extradesc      = NULL;
        pRoomIndex->area                = NULL;
        pRoomIndex->vnum                = vnum;
        pRoomIndex->name                = STRALLOC("Floating in a void");
        pRoomIndex->description         = STRALLOC("");
        pRoomIndex->room_flags          = 0;
        pRoomIndex->sector_type         = 1;
        pRoomIndex->light               = 0;
    	for ( door = 0; door <= 5; door++ )
          pRoomIndex->exit[door] = NULL;
        iHash                   = vnum % MAX_KEY_HASH;
        pRoomIndex->next        = room_index_hash[iHash];
        room_index_hash[iHash]  = pRoomIndex;
        top_room++;
	room_index[vnum]=pRoomIndex;

        return pRoomIndex;
}
/*
 * Create a new INDEX object (for online building)              
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, char *name )
{
        OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
        char buf[MAX_STRING_LENGTH];
        int     iHash;

        if ( cvnum > 0 )
          cObjIndex = get_obj_index( cvnum );
        else
          cObjIndex = NULL;
        CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
        pObjIndex->vnum                 = vnum;
        pObjIndex->name                 = STRALLOC( name );
        pObjIndex->first_affect         = NULL;
        pObjIndex->last_affect          = NULL;
        pObjIndex->first_extradesc      = NULL;
        pObjIndex->last_extradesc       = NULL;
        if ( !cObjIndex )
        {
          sprintf( buf, "A newly created %s", name );
          pObjIndex->short_descr        = STRALLOC( buf  );
          sprintf( buf, "Some god dropped a newly created %s here.", name );
          pObjIndex->description        = STRALLOC( buf );
          pObjIndex->long_descr         = STRALLOC( "" );
          pObjIndex->short_descr[0]     = LOWER(pObjIndex->short_descr[0]);
          pObjIndex->description[0]     = UPPER(pObjIndex->description[0]);
          pObjIndex->item_type          = ITEM_TRASH;
          pObjIndex->extra_flags	= 0;
          pObjIndex->wear_flags         = 0;
          pObjIndex->value[0]           = 0;
          pObjIndex->value[1]           = 0;
          pObjIndex->value[2]           = 0;
          pObjIndex->value[3]           = 0;
          pObjIndex->weight             = 1;
          pObjIndex->cost               = 0;
	  pObjIndex->area		= get_area_from_vnum ( vnum, TYPE_OBJECT );
        }
        else
        {
          EXTRA_DESCR_DATA *ed,  *ced;
          AFFECT_DATA      *paf, *cpaf;

          pObjIndex->short_descr        = QUICKLINK( cObjIndex->short_descr );
          pObjIndex->description        = QUICKLINK( cObjIndex->description );
          pObjIndex->long_descr         = QUICKLINK( cObjIndex->long_descr );
          pObjIndex->item_type          = cObjIndex->item_type;
          pObjIndex->extra_flags        = cObjIndex->extra_flags;
          pObjIndex->wear_flags         = cObjIndex->wear_flags;
          pObjIndex->value[0]           = cObjIndex->value[0];
          pObjIndex->value[1]           = cObjIndex->value[1];
          pObjIndex->value[2]           = cObjIndex->value[2];
          pObjIndex->value[3]           = cObjIndex->value[3];
          pObjIndex->weight             = cObjIndex->weight;
          pObjIndex->cost               = cObjIndex->cost;
	  pObjIndex->area		= get_area_from_vnum ( vnum, TYPE_OBJECT );

          for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
          {
                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->keyword             = QUICKLINK( ced->keyword );
                ed->description         = QUICKLINK( ced->description );
                LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                          next, prev );
                top_ed++;
          }
          for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
          {
                CREATE( paf, AFFECT_DATA, 1 );
                paf->type               = cpaf->type;
                paf->duration           = cpaf->duration;
                paf->location           = cpaf->location;
                paf->modifier           = cpaf->modifier;
                paf->bitvector          = cpaf->bitvector;
                LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
                           next, prev );
                top_affect++;
          }
        }
        pObjIndex->count                = 0;
        iHash                           = vnum % MAX_KEY_HASH;
        pObjIndex->next                 = obj_index_hash[iHash];
        obj_index_hash[iHash]           = pObjIndex;
        top_obj_index++;

        return pObjIndex;
}

AREA_DATA * get_area_from_vnum ( int vnum, int type )
{
 AREA_DATA * pArea;

 for (pArea=first_area;pArea!= NULL; pArea=pArea->next)
 {
  if (type==TYPE_MOBILE)
  { 
    if (vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum)
      return pArea;
  }
  else if (type==TYPE_OBJECT)
  { 
    if (vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum)
      return pArea;
  }
  else if (type==TYPE_ROOM)
  { 
    if (vnum >= pArea->low_r_vnum && vnum <= pArea->hi_r_vnum)
      return pArea;
  }
 }
 return NULL;
}
/*
 * Create a new INDEX mobile (for online building)             
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, char *name )
{
        MOB_INDEX_DATA *pMobIndex, *cMobIndex;
        char buf[MAX_STRING_LENGTH];
        int     iHash;

        if ( cvnum > 0 )
          cMobIndex = get_mob_index( cvnum );
        else
          cMobIndex = NULL;
        CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
        pMobIndex->vnum                 = vnum;
        pMobIndex->count                = 0;
        pMobIndex->killed               = 0;
        pMobIndex->player_name          = STRALLOC( name );
        if ( !cMobIndex )
        {
          sprintf( buf, "A newly created %s", name );
          pMobIndex->short_descr        = STRALLOC( buf  );
          sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
          pMobIndex->long_descr         = STRALLOC( buf );
          pMobIndex->description        = STRALLOC( "" );
          pMobIndex->short_descr[0]     = LOWER(pMobIndex->short_descr[0]);
          pMobIndex->long_descr[0]      = UPPER(pMobIndex->long_descr[0]);
          pMobIndex->description[0]     = UPPER(pMobIndex->description[0]);
          pMobIndex->act		= 0;
          SET_BIT(pMobIndex->act, ACT_IS_NPC);
          pMobIndex->affected_by	= 0;
          pMobIndex->pShop              = NULL;
          pMobIndex->spec_fun           = NULL;
          pMobIndex->mobprogs           = NULL;
          pMobIndex->alignment          = 0;
          pMobIndex->level              = 1;
          pMobIndex->ac                 = 0;
          pMobIndex->hitnodice          = 0;
          pMobIndex->hitsizedice        = 0;
          pMobIndex->hitplus            = 0;
          pMobIndex->damnodice          = 0;
          pMobIndex->damsizedice        = 0;
          pMobIndex->damplus            = 0;
          pMobIndex->gold               = 0;
          pMobIndex->position           = POS_STANDING;
          pMobIndex->sex                = 0;
	  pMobIndex->area		= get_area_from_vnum ( vnum, TYPE_MOBILE );
    	  pMobIndex->max_mobs           = 0;
    	  pMobIndex->armor              = 0;
    	  pMobIndex->body_parts         = 0;
      	  pMobIndex->attack_parts       = 0;

        }
        else
        {
          pMobIndex->short_descr        = QUICKLINK( cMobIndex->short_descr );
          pMobIndex->long_descr         = QUICKLINK( cMobIndex->long_descr  );
          pMobIndex->description        = QUICKLINK( cMobIndex->description );
          pMobIndex->act                = cMobIndex->act;
          pMobIndex->affected_by        = cMobIndex->affected_by;
          pMobIndex->pShop              = NULL;
          pMobIndex->spec_fun           = cMobIndex->spec_fun;
          pMobIndex->mobprogs           = NULL;
          pMobIndex->alignment          = cMobIndex->alignment;
          pMobIndex->level              = cMobIndex->level;
          pMobIndex->ac                 = cMobIndex->ac;
          pMobIndex->hitnodice          = cMobIndex->hitnodice;
          pMobIndex->hitsizedice        = cMobIndex->hitsizedice;
          pMobIndex->hitplus            = cMobIndex->hitplus;
          pMobIndex->damnodice          = cMobIndex->damnodice;
          pMobIndex->damsizedice        = cMobIndex->damsizedice;
          pMobIndex->damplus            = cMobIndex->damplus;
          pMobIndex->gold               = cMobIndex->gold;
          pMobIndex->position           = cMobIndex->position;
          pMobIndex->sex                = cMobIndex->sex;
          pMobIndex->race               = cMobIndex->race;
	  pMobIndex->area		= get_area_from_vnum ( vnum, TYPE_MOBILE );
    	  pMobIndex->max_mobs           = cMobIndex->max_mobs;
    	  pMobIndex->armor              = cMobIndex->armor;
    	  pMobIndex->body_parts         = cMobIndex->body_parts;
      	  pMobIndex->attack_parts       = cMobIndex->attack_parts;
        }
        iHash                           = vnum % MAX_KEY_HASH;
        pMobIndex->next                 = mob_index_hash[iHash];
        mob_index_hash[iHash]           = pMobIndex;
        mob_index[vnum]			= pMobIndex;
        top_mob_index++;

        return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.                                            -Martin
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door )
{
 EXIT_DATA *pexit;

 CREATE(pexit, EXIT_DATA, 1);
 pexit->exit_info        = 0;
 pexit->pvnum            = 0;
 pexit->key              = -1;
 pexit->to_room          = to_room;
 if (to_room)
   pexit->vnum       = to_room->vnum;
 room_index[pRoomIndex->vnum]->exit[door]= pexit;
 top_exit++;
 return pexit;
}



/*
 * LOAD_BOUNTIES
 *
 * This code loads the bounties from "bounty.txt" in the area directory.
 *
 * Presto 2-20-99
 */
void load_bounties(void)
{
  int done=0;
  FILE *fp;
  BOUNTY_DATA *bptr;
  char buf[MAX_STRING_LENGTH];

  log_string("Loading bounty list.");
  if((fp=fopen(BOUNTY_FILE, "r")))
  {
    do
    {
      strcpy(buf, fread_word(fp));
      if(!strcasecmp(buf,"NULL"))
        done=1;
      else
      {
        CREATE(bptr, BOUNTY_DATA, 1);
        bptr->name	= STRALLOC(capitalize(buf));
        bptr->amount	= fread_number(fp);
        bptr->postdate	= fread_number(fp);
        bptr->expires	= fread_number(fp);
	sort_bounty( bptr );
      }
    }
    while(done==0);
    fclose(fp);
  }
  return;
}



/*
 * SAVE_BOUNTIES
 *
 * This code saves the bounties to "bounty.txt" in the area directory.
 *
 * Presto 2-20-99
 */
void save_bounties(void)
{
  FILE *fp;
  BOUNTY_DATA *bptr;

  if((fp=fopen(BOUNTY_FILE_TMP, "w")))
  {
    for(bptr=first_bounty; bptr; bptr=bptr->next)
    {
      fprintf(fp, "%-12s %9d %9d %9d\n", capitalize(bptr->name), bptr->amount,
              bptr->postdate, bptr->expires);
    }
    fprintf(fp, "NULL\nXXXXXXXXXX\n#Bounty\n");
  }
  else
    {
    log_string( "Failed to write bounty.txt." );
    return;
    }

    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fp );
    if( is_valid_save( BOUNTY_FILE_TMP, "Bounty" ) )
      {
      remove( BOUNTY_FILE );
      rename(BOUNTY_FILE_TMP,BOUNTY_FILE);
      }
  return;
}

/*
 * Sort areas by name alphanumercially
 */
void sort_area_by_name(AREA_DATA *pArea)
{ AREA_DATA *temp_area;

        if(!pArea)
        {
                bug("Sort_area_by_name: NULL pArea");
                return;
        }
        for(temp_area = first_area_name; temp_area;
                        temp_area = temp_area->next_sort_name)
        {
                if(strcmp(pArea->name, temp_area->name) < 0)
                {
                        INSERT(pArea, temp_area, first_area_name,
                                next_sort_name, prev_sort_name);
                        break;
                }
        }
        if(!temp_area)
        {
                LINK(pArea, first_area_name, last_area_name,
                        next_sort_name, prev_sort_name);
        }
        return;
}

  /* This routine makes sure that files are not cross linked or chopped */
  /* Chaos  - 4/26/99  */
bool is_valid_save( char *file_name , char *text_crc)
  {
  char buf[MAX_INPUT_LENGTH];
  char tbuf[MAX_INPUT_LENGTH];
  int cnt;
  int cf;
  FILE *fp;
  char *pt;

  fp=fopen( file_name, "r");

  if(fp==NULL)
    return( FALSE );

  fseek( fp, 0, SEEK_END );
  if(ftell(fp) < (long)10)
    {
    fclose( fp );
    sprintf( buf, "Oops, file system full!  %s failed.", file_name);
    log_string( buf );
    return( FALSE );
    }

  cf = ' ';
  cnt = 0;

  while( cf != '#' && cnt>-20 )
    {
/*    sprintf( buf, "File %s: Char %c   cnt %d", file_name, cf, cnt);
      log_string( buf ); */
    cnt --;
    fseek( fp, cnt, SEEK_END );
    cf = fgetc( fp );
    }

  if( cnt == -20 )
    {
    sprintf( buf, "Didn't find an #%s on %s", text_crc, file_name );
    log_string( buf );
    fclose( fp );
    return( FALSE );
    }

  pt=buf;
  *pt = '\0';
  while( cf != '\r' && cf != '\n' && cf != EOF )
    {
    cf = fgetc( fp );
    *pt = cf;
    if( cf != '\r' && cf != '\n' && cf != EOF )
      pt++;
    }
  *pt = '\0';
  
     /* Old style character */
  if( !strcasecmp( buf, text_crc ) )
    {
    fclose( fp );
    return( TRUE );
    }

  sprintf( tbuf, "Cross linked file %s on %s", pt, file_name);
  log_string( tbuf );
  fclose( fp );
  return( FALSE );
}


