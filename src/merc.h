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

/*
 * Accommodate old non-Ansi compilers.
 */
/* #include <prof.h> */
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>

#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_OBJ_FUN( fun )		OBJ_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif

#define BV00            (1 <<  0)
#define BV01            (1 <<  1)
#define BV02            (1 <<  2)
#define BV03            (1 <<  3)
#define BV04            (1 <<  4)
#define BV05            (1 <<  5)
#define BV06            (1 <<  6)
#define BV07            (1 <<  7)
#define BV08            (1 <<  8)
#define BV09            (1 <<  9)
#define BV10            (1 << 10)
#define BV11            (1 << 11)
#define BV12            (1 << 12)
#define BV13            (1 << 13)
#define BV14            (1 << 14)
#define BV15            (1 << 15)
#define BV16            (1 << 16)
#define BV17            (1 << 17)
#define BV18            (1 << 18)
#define BV19            (1 << 19)
#define BV20            (1 << 20)
#define BV21            (1 << 21)
#define BV22            (1 << 22)
#define BV23            (1 << 23)
#define BV24            (1 << 24)
#define BV25            (1 << 25)
#define BV26            (1 << 26)
#define BV27            (1 << 27)
#define BV28            (1 << 28)
#define BV29            (1 << 29)
#define BV30            (1 << 30)
#define BV31            (1 << 31)

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/* 32 Used...that's the max, folks - Martin 14/8/98 */

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short    int			sh_int;
typedef unsigned char			bool;
#endif



/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	topic_data		TOPIC_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	poison_data		POISON_DATA;
typedef struct	bitvector_type		BITVECTOR_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  bounty_data             BOUNTY_DATA;
typedef struct	help_menu_data	        HELP_MENU_DATA;
typedef struct	obj_with_prog	        OBJ_WITH_PROG;
typedef struct	obj_prog	        OBJ_PROG;
typedef struct	npc_data			NPC_DATA;
typedef struct	mob_prog_token			NPC_TOKEN;
typedef struct	mob_prog_packet			MOB_PACKET;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef struct  player_host    		PLAYER_HOST;
typedef struct  host_names     		HOST_NAMES;
typedef struct  player_game    		PLAYER_GAME;
typedef struct  usage_data     		USAGE_DATA;
typedef struct  castle_data    		CASTLE_DATA;
typedef struct  owner_data     		OWNER_DATA;
typedef struct	tactical_map   		TACTICAL_MAP;
typedef struct  object_reference   	OBJ_REFERENCE;
typedef struct	clan_data		CLAN_DATA;
typedef struct  editor_data     	EDITOR_DATA;
typedef struct  fighting_data          	FIGHT_DATA;
typedef struct  swear_data      	SWEAR_DATA;
typedef struct  save_data       	SAVE_DATA;
typedef struct  save_area_data  	SAVE_AREA_DATA;
typedef struct  hunt_hate_fear  	HHF_DATA;
typedef struct  cmd_type        	CMDTYPE;
typedef struct  killed_data             KILLED_DATA;

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef bool OBJ_FUN	args( ( OBJ_DATA *obj, int trigger, void *data,
                                void *extra, CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH	 	    128 
#define MAX_STRING_LENGTH         60000
#define MAX_QUEST_BYTES              16

#define NEW_MEMORY_MANAGER TRUE
#define HASHSTR
#ifdef USE_THREADS
#undef USE_THREADS
#endif
#define COMPRESS_FILES FALSE
#define MAX_INPUT_LENGTH	   2000
#define MAX_ALIAS                    60
#define MAX_KILL_TRACK               10
#define MAX_LAST_LEFT                 5
#define MAX_COLOR                    17
#define MAX_BUFFER_LENGTH         16300   
#define MAX_OBJECTS_IN_CONTAINER     75
#define MAX_OBJECTS_IN_ROOM         100
#define COMBAT_STRING_LENGTH      12200
#define MAX_MOB_KILL_TRACK	     25
#define MAX_PK_ATTACKS		     30

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH         1000000
#define MIN_EXP_WORTH              75

#define MAX_SKILL		  207
#define MAX_BITVECTOR		  557
#define MAX_BODY		   14
#define MAX_CLASS		    7
#define MAX_RACE                   12
#define MAX_LEVEL		   99
#define MAX_AREA                  500
#define MAX_VNUM         100*MAX_AREA
#define MAX_PACKETS                10
#define MAX_TOPIC                   6
#define LEVEL_HERO		   (MAX_LEVEL - 3)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 2)
#define LEVEL_COUNT		   (MAX_LEVEL - 2)
#define LEVEL_DUKE		   (MAX_LEVEL - 1)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (60 * PULSE_PER_SECOND)
#define PULSE_AREA		  (PULSE_TICK)  /* Internally determines age */
#define PULSE_SHOPS		  ( 4 * PULSE_TICK)
/* number of PULSE_TICK's before an item dropped on the ground will decay */
#define OBJ_SAC_TIME          ( 25 )



/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
};



/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct killed_data
{
    sh_int              vnum;
    char                count;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
    int         temperature;     /* In degrees Farenheit */
    int         temp_summer;     /* Range of temps */
    int         temp_winter;
    int         temp_daily;
    int         wet_scale;    /* 0-Desert   10-Rain forest */
};

struct  host_names
  {
  HOST_NAMES *next;
  char *host;
  unsigned char address[4];
  unsigned char max_connect;
  };

struct  player_host
  {
  PLAYER_HOST *next;
  char *host;
  };

struct  object_reference
  {
  OBJ_REFERENCE *next;
  OBJ_DATA *obj;
  };

struct  player_game
  {
  PLAYER_GAME * next;
  CHAR_DATA   * ch;
  };


/*
 * Connected state for a channel.
 */
typedef enum {
   CON_GET_NAME = -100,
   CON_GET_OLD_PASSWORD,
   CON_CONFIRM_NEW_NAME,
   CON_GET_NEW_PASSWORD,
   CON_CONFIRM_NEW_PASSWORD,
   CON_GET_NEW_SEX	,
   CON_GET_NEW_CLASS,
   CON_GET_NEW_RACE,
   CON_READ_MOTD,
   CON_REROLL,
   CON_VT100,
   CON_REROLL_AGAIN,
   CON_MAIL,
   CON_GET_INSTRUCTIONS,
   CON_INSTRUCTIONS,
   CON_COLOR,
   CON_TERM_SIZE,
   CON_GET_NEW_NAME,
   CON_COPYOVER_RECOVER,
   CON_PLAYING = 0,
   CON_EDITING
} connection_types;

/* 
  Use these to keep track of what the person is editing...
  Martin
*/
typedef enum
{
 SUB_NONE, SUB_RESTRICTED, SUB_WRITING_NOTE, SUB_PERSONAL_DESC,
 SUB_ROOM_DESC, SUB_MOB_DESC, SUB_CLAN_DESC, SUB_CLAN_DOCTRINE,
 SUB_HELP_EDIT, SUB_MPROG_EDIT, SUB_REPEATCMD, SUB_OBJ_LONG, 
 SUB_OBJ_EXTRA, SUB_NORTH, SUB_EAST, SUB_SOUTH, SUB_WEST, SUB_UP, 
 SUB_DOWN, SUB_ROOM_EXTRA
} char_substates;

#define CHECK_SUBRESTRICTED(ch)                                 \
do                                                              \
{                                                               \
    if ( (ch)->substate == SUB_RESTRICTED )                     \
    {                                                           \
        send_to_char( "You cannot use this command from within another command.\n\r", ch );     \
        return;                                                 \
    }                                                           \
} while(0)

       /* DO not create any CON_ #'s higher than CREATOR_MAIN */
#define CREATOR_MAIN                  1000
#define CREATOR_NEW_ROOM              2000
#define CREATOR_EDIT_ROOM             3000
#define CREATOR_EDIT_ROOM_NAME        3100
#define CREATOR_EDIT_ROOM_DESC        3200
#define CREATOR_EDIT_ROOM_EXTRAS      3300
#define CREATOR_EDIT_ROOM_FLAGS       3400
#define CREATOR_EDIT_ROOM_SECTOR      3500
#define CREATOR_EDIT_DOOR             4000
#define CREATOR_MOVE                  5000

#define TRACK_NORTH	1
#define TRACK_SOUTH     2
#define TRACK_EAST      4
#define TRACK_WEST      8
#define TRACK_UP	16
#define TRACK_DOWN	32
#define TRACK_FLY	64
#define TRACK_BLOOD     128


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	prev;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    char *		domain;
    char *              old_host;
    char *		old_domain;
    sh_int		descriptor;
    sh_int		connected;
    sh_int    		inst_page;
    bool		fcommand;
    bool    		prompter;
    char		inbuf		[MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char                *back_buf;    /* Used during obj_progs */
    char		inlast		[MAX_INPUT_LENGTH];
    char                combat_buffer[COMBAT_STRING_LENGTH];
    int                 combat_buffer_length;
    int                 combat_buffer_index;
    int			repeat;
    char 		*outbuf;
    int			outsize;
    int			outtop;
    int                 intop;
    int     		port_size;
    int     		port_baud;
    int     		port_timer;
    sh_int  		comm_flags;
    bool    		lookup;
    int     		creator_location;
    int     		creator_zone;
    int     		fork_id;
    int     		remote_port;
    HOST_NAMES *	host_index;  /* index to the pointer to the host */
    sh_int  		timeout;
};

struct  editor_data
{
    sh_int              numlines;
    sh_int              on_line;
    sh_int              size;
    char                line[49][81];
};

struct fighting_data
{
    CHAR_DATA *         who;
    int                 xp;
    sh_int              duration;
    sh_int              timeskilled;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
    sh_int	manap;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3


struct	obj_with_prog
{
    OBJ_WITH_PROG *	next;
    OBJ_DATA      *     obj;
};

struct	obj_prog
{
    OBJ_PROG      *	next;
    int                 cmd;           /* command to key off  -2 is unknown */
    char          *     unknown;       /* unknown trigger  NULL if known */
    char                obj_command;   /* Object command to do */ 
    char          *     argument;      /* Argument string if required */
    char                if_check;      /* Type of if check */
    char                if_symbol;     /* If symbol:  > < = !   */
    int                 if_value;      /* Value to compare to */
    int                 if_true;       /* Index number of true dest */
    int                 if_false;      
    sh_int              quest_offset;  /* quest offset, if quest response */
    sh_int              quest_bits;    /* quest bits, if quest response */
    OBJ_PROG      *     true;          /* branch to if true */
    OBJ_PROG      *     false;
    int                 index;         /* index number for branching */
    int                 percentage;    /* percent trigger will work */
};

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA *	prev;
    sh_int	level;
    char *	keyword;
    char *	text;
    AREA_DATA *   area;
    HELP_MENU_DATA *menu;
};

struct help_menu_data
  {
  HELP_MENU_DATA   *next;
  char              option;
  HELP_DATA        *help;
  };


/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */
struct	class_type
{
    char 	who_name	[4];	/* Three-letter name for 'who'	*/
    char        who_name_long   [40];   /* Actual title  Chaos 11/7/93  */
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	weapon;			/* First weapon			*/
    sh_int	guild;			/* Vnum of guild room		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    sh_int	mana_min;		/* Min mana gained on leveling	*/
    sh_int	mana_max;		/* Max mana gained on leveling	*/
    char        desc           [256];    /* Describes the class          */
};

struct  race_type
{
   char         race_name[10];              /* Race Names                  */
   sh_int       race_mod[5];           /* Attribute modifiers         */
   sh_int       race_class[MAX_CLASS];
   sh_int       max_speed;  /* 0-walk 1-normal 2-jog 3-run */
   sh_int       move_rate;  /* .66 to 1.5  by 1 to 4 */
   sh_int       vision;     /* 0-normal  1-night   2-dark  */
   sh_int       hp_mod;     /* Stat change when advancing a level */
   sh_int       mana_mod;
   sh_int       move_mod;
   char         race_special[80];  /* String of special ability for start */
   char         race_description[512]; 
};
/*
 * Data structures for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    NOTE_DATA * prev;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    int         topic;
    char *	text;
    int         time;  /* encoded format for dates */
    int         room_vnum;    /* Room based note boards */
};

struct	topic_data
{
    char *	  name;
    int           min_level;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    AFFECT_DATA *	prev;
    sh_int		type;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
};



/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   3404
#define MOB_VNUM_FIRE_ELEMENTAL	   9906
#define MOB_VNUM_WATER_ELEMENTAL   9907
#define MOB_VNUM_AIR_ELEMENTAL	   9908
#define MOB_VNUM_EARTH_ELEMENTAL   9909
#define MOB_VNUM_SUPER_MOB   	   3


#define AFLAG_NODEBUG		BV00
#define AFLAG_NOTELEPORT	BV01
#define AFLAG_NOGOHOME		BV02
#define AFLAG_NORECALL		BV03
#define AFLAG_NOCASTLE		BV04
#define AFLAG_NORIP		BV05
#define AFLAG_FREEQUIT		BV06

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		      1		/* Auto set for mobs	*/
#define ACT_SENTINEL		      2		/* Stays in one room	*/
#define ACT_SCAVENGER		      4		/* Picks up objects	*/
#define ACT_AGGRESSIVE		     32		/* Attacks PC's		*/
#define ACT_STAY_AREA		     64		/* Won't leave area	*/
#define ACT_WIMPY		    128		/* Flees when hurt	*/
#define ACT_PET			    256		/* Auto set for pets	*/
#define ACT_TRAIN		    512		/* Can train PC's	*/
#define ACT_PRACTICE		   1024		/* Can practice PC's	*/
#define ACT_WEAK                   2048         /* Cannot carry anything*/
#define ACT_SMART                  4096         /* Cannot say anything  */
#define ACT_ONE_FIGHT              8192         /* Disapears after fight*/
#define ACT_NO_ORDER              16384         /* Cannot be ordered    */
#define ACT_RIDE                  32768         /* Creature can be ridden */
#define ACT_BODY                  65536         /* Body parts are used */
#define ACT_RACE                 131072         /* Race is defined after gold */
#define ACT_UNDEAD               262144         /* No corpse or body parts */
#define ACT_ELEMENTAL            524288 	/* Friendly Elemental type */
#define ACT_CLAN_GUARD          1048576 	/* Clan Guards          */
#define ACT_CLAN_HEALER         2097152 	/* Clan Healers 	*/
#define ACT_PROTOTYPE           4194304 	/* Prototype for OLC    */

#define ACT_WILL_DIE           16777216        /* Purely Internal */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		      1
#define AFF_INVISIBLE		      2
#define AFF_DETECT_EVIL		      4
#define AFF_DETECT_INVIS	      8
#define AFF_DETECT_MAGIC	     16
#define AFF_DETECT_HIDDEN	     32
#define AFF_HOLD		     64		/* Unused	*/
#define AFF_SANCTUARY		    128
#define AFF_FAERIE_FIRE		    256
#define AFF_INFRARED		    512
#define AFF_CURSE		   1024
#define AFF_UNDERSTAND		   2048	
#define AFF_POISON		   4096
#define AFF_PROTECT_EVIL	   8192
#define AFF_PROTECT_GOOD	  16384		
#define AFF_SNEAK		  32768         /* Bit 15 */
#define AFF_HIDE		  65536
#define AFF_SLEEP		 131072
#define AFF_CHARM		 262144
#define AFF_FLYING		 524288
#define AFF_PASS_DOOR		1048576
#define AFF_STEALTH             2097152
#define AFF_CLEAR               4194304
#define AFF_HUNT                8388608
#define AFF_TONGUES            16777216
#define AFF_ETHEREAL           33554432
#define AFF_HASTE              67108864


#define AFF2_ENHANCED_REST           -1    /* All affect2 are negative */
#define AFF2_ENHANCED_HEAL           -2    
#define AFF2_ENHANCED_REVIVE         -4    
#define AFF2_MAGE_SHIELD             -8
#define AFF2_CAMPING                -16
#define AFF2_BERSERK                -32
#define AFF2_MIRROR_IMAGE           -64
#define AFF2_HALLUCINATE           -128
#define AFF2_BLEEDING              -256
#define AFF2_STABILITY             -512
#define AFF2_ETHEREAL             -1024
#define AFF2_ASTRAL               -2048
#define AFF2_BREATH_WATER         -4096
#define AFF2_MAGE_BLAST           -8192
#define AFF2_POISONED            -16384
#define AFF2_CONFUSION           -32768
#define AFF2_HAS_FLASH           -65536
#define AFF2_POSSESS            -131072
#define AFF2_FIRESHIELD         -262144

/*
 * Sex.
 * Used in #MOBILES.
 */

#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_FINAL_TURD	     16

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SCHOOL_MACE	   3700
#define OBJ_VNUM_SCHOOL_DAGGER	   3701
#define OBJ_VNUM_SCHOOL_SWORD	   3702
#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716

#define OBJ_VNUM_RIP_KEY           9900



/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_AMMO                    30



/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1
#define ITEM_HUM		      2
#define ITEM_DARK		      4
#define ITEM_LOCK		      8
#define ITEM_EVIL		     16
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define ITEM_LEVEL_RENT     16384 /* If set forces RENT field to be level */
#define ITEM_NOT_VALID      32768 /* set to remove item from the game */
#define ITEM_AUTO_ENGRAVE   65536 /* engraved to the first to get item */
#define ITEM_FORGERY       131072 /* item is a forgery */
#define ITEM_PROTOTYPE     262144 /* Prototype for OLC */
#define ITEM_QUEST         524288 /* Item is a quest item and takes no dmg */

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		      1
#define ITEM_WEAR_FINGER	      2
#define ITEM_WEAR_NECK		      4
#define ITEM_WEAR_BODY		      8
#define ITEM_WEAR_HEAD		     16
#define ITEM_WEAR_LEGS		     32
#define ITEM_WEAR_FEET		     64
#define ITEM_WEAR_HANDS		    128 
#define ITEM_WEAR_ARMS		    256
#define ITEM_WEAR_SHIELD	    512
#define ITEM_WEAR_ABOUT		   1024 
#define ITEM_WEAR_WAIST		   2048
#define ITEM_WEAR_WRIST		   4096
#define ITEM_WIELD		   8192
#define ITEM_HOLD		  16384
#define ITEM_WEAR_HEART           32768



/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24



/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   9755
#define ROOM_VNUM_ALTAR		   9799
#define ROOM_VNUM_SCHOOL	   3700
#define ROOM_VNUM_MEETING    9784
#define ROOM_VNUM_ARENA     10500 



/*
 * Room flags.
 * Used in #ROOMS.
 */

/* Unused is 128 */

#define ROOM_DARK	       1
#define ROOM_SMOKE	       2
#define ROOM_NO_MOB	       4
#define ROOM_INDOORS	       8
#define ROOM_HALLUCINATE      16
#define ROOM_NO_GOHOME        32
#define ROOM_CLAN_DONATION    64
#define ROOM_BANK            256
#define ROOM_PRIVATE	     512
#define ROOM_SAFE	    1024
#define ROOM_SOLITARY	    2048
#define ROOM_PET_SHOP	    4096
#define ROOM_NO_RECALL	    8192
#define ROOM_RIP           16384
#define ROOM_BLOCK         32768
#define ROOM_NO_SAVE       65536
#define ROOM_MORGUE       131072 
#define ROOM_TEMPLE       262144 /* Deathroom can be set here */

#define ROOM_GOD_LO       524288     /* part of bitvector for god number 0-3 */
#define ROOM_GOD_HI       1048576
#define ROOM_ALTAR_N      ROOM_GOD_LO + ROOM_GOD_HI   /* death set neutral */
#define ROOM_ALTAR_C      ROOM_GOD_LO                 /* death set Chaos */
#define ROOM_ALTAR_O      ROOM_GOD_HI                 /* death set Order */

#define ROOM_ALLOW_0   2097152
#define ROOM_ALLOW_1   4194304
#define ROOM_ALLOW_2   8388608

#define ROOM_ALLOW_ILL  ROOM_ALLOW_0
#define ROOM_ALLOW_ELE  ROOM_ALLOW_1
#define ROOM_ALLOW_ROG  ROOM_ALLOW_0 + ROOM_ALLOW_1
#define ROOM_ALLOW_RAN  ROOM_ALLOW_2
#define ROOM_ALLOW_NEC  ROOM_ALLOW_0 + ROOM_ALLOW_2
#define ROOM_ALLOW_MON  ROOM_ALLOW_1 + ROOM_ALLOW_2
#define ROOM_ALLOW_ASN  ROOM_ALLOW_0 + ROOM_ALLOW_1 + ROOM_ALLOW_2

#define ROOM_IS_CASTLE   16777216   /* is part of a castle */
#define ROOM_IS_ENTRANCE 33554432   /* entrance to castle, needed for door-fix*/
#define ROOM_PROTOTYPE   67108864   /* Prototypes for OLC */
#define ROOM_NOTE_BOARD  134217728  /* Allow a room note board here */
#define ROOM_NO_CASTLE   268435456
#define ROOM_NO_RIP      536870912

/*
 * Directions.
 * Used in #ROOMS.
 */

#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		    BV00
#define EX_CLOSED		    BV01
#define EX_LOCKED		    BV02
#define EX_HIDDEN		    BV03
#define EX_RIP                      BV04
#define EX_PICKPROOF		    BV05
#define EX_BASHPROOF		    BV06
#define EX_MAGICPROOF		    BV07
#define EX_BASHED		    BV08
#define EX_UNBARRED		    BV09
#define EX_BACKDOOR		    BV10
#define EX_CLAN_BACKDOOR	    BV11
#define MAX_EXFLAG	    	    11



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_LAVA                    11
#define SECT_INN                     12
#define SECT_ETHEREAL                13
#define SECT_ASTRAL                  14
#define SECT_UNDER_WATER             15
#define SECT_UNDER_GROUND            16
#define SECT_DEEP_EARTH              17
#define SECT_MAX		     18



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_HEART      	     18
#define WEAR_DUAL_WIELD      	     19
#define MAX_WEAR		     20



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_FIGHTING		      6
#define POS_STANDING		      7



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      1		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      2
#define PLR_TERMINAL                  4
#define PLR_AUTOEXIT		      8
#define PLR_AUTOLOOT		     16
#define PLR_AUTOSAC                  32
#define PLR_BLANK		     64
#define PLR_BRIEF		    128
#define PLR_REPEAT                  256   /* Repeat the input line */
#define PLR_COMBINE		    512
#define PLR_PROMPT		   1024   /* Turn off tactial and make prompt*/
#define PLR_TELNET_GA		   2048
#define PLR_HOLYLIGHT		   4096 
#define PLR_WIZINVIS		   8192 
#define PLR_WIZTIME		  16384 
#define	PLR_SILENCE		  32768
#define PLR_OUTCAST		  65536
#define PLR_COMBAT_PERCENT       131072 /* unused...most of these notell stuff*/
#define PLR_NO_TELL		 262144 /* is never used too...Martin 9/11/98*/
#define PLR_LOG			 524288
#define PLR_DENY                1048576
#define PLR_FREEZE	   	2097152
#define PLR_THIEF		4194304
#define PLR_KILLER		8388608
#define PLR_DAMAGE             16777216
#define PLR_AUTO_SPLIT         33554432
#define PLR_QUIET              67108864
#define PLR_PAGER             134217728
#define PLR_CHAT              268435456
#define PLR_PLAN              536870912
       /* Ack...   no more bits here */
      

  /* Player Bits set 2 */
#define PLR2_PEEK                     1    /* turns off Peek */
#define PLR2_LOGIN                    2    /* enable 'login' command */
#define PLR2_EXP_TO_LEVEL             4    /* Toggle for exp display */
#define PLR2_EXTERNAL_FILES           8    /* Toggle for files like midi */
#define PLR2_EXTERNAL_METER          16    /* Toggle for download meter */
#define PLR2_MRTERM_STATS            32    /* Toggle for Binary Stat Transfer*/
#define PLR2_CASTLES                 64    /* Disables viewing castles*/
#define PLR2_AFK                    128    /* Set afk flag  - Martin*/
#define PLR2_BATTLE                 256   /* Listen to BATTLE channel - Martin*/
#define PLR2_VICTIM_LIST            512    /* See Victim List ? - Martin */
#define PLR2_HEARLOG		   1024    /* Toggle immortals hearing log file
					      Martin 9/8/98 */
#define PLR2_CENSOR		   2048    /* Mom-like censor bullshit */
#define PLR2_ITEM_REF		   4096    /* The #123 stuff */


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		      1
#define	CHANNEL_CHAT		      2
#define	CHANNEL_HACKER		      4
#define	CHANNEL_IMMTALK		      8
#define	CHANNEL_MUSIC		     16
#define	CHANNEL_QUESTION	     32
#define	CHANNEL_SHOUT		     64
#define	CHANNEL_YELL		    128

/*
 * Player class defines
 */
#define CLASS_ILLUSIONIST             0
#define CLASS_ELEMENTALIST            1
#define CLASS_ROGUE                   2
#define CLASS_RANGER                  3
#define CLASS_NECROMANCER             4
#define CLASS_MONK                    5
#define CLASS_ASSASSIN                6
#define CLASS_MONSTER                 99

#define FLAG_CLASS_ILLUSIONIST             1
#define FLAG_CLASS_ELEMENTALIST            2
#define FLAG_CLASS_ROGUE                   4
#define FLAG_CLASS_RANGER                  8
#define FLAG_CLASS_NECROMANCER            16
#define FLAG_CLASS_MONK                   32
#define FLAG_CLASS_ASSASSIN               64
#define FLAG_CLASS_MONSTER               128

/* Player Races  */
#define RACE_HUMAN                    0
#define RACE_HALFLING                 1
#define RACE_ELF                      2
#define RACE_DROW                     3
#define RACE_DWARF                    4
#define RACE_GNOME                    5
#define RACE_ORC                      6
#define RACE_OGRE                     7
#define RACE_AVIARAN                  8
#define RACE_CENTAUR                  9
#define RACE_GITH                    10
#define RACE_TSARIAN                 11


/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    AREA_DATA *   	area;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    int			vnum;
    sh_int		count;
    sh_int		killed;
    sh_int		sex;
    sh_int		level;
    int			act;
    int			affected_by;
    sh_int		alignment;
    sh_int		hitroll;	
    sh_int		ac;			
    sh_int		hitnodice;	
    sh_int		hitsizedice;	
    sh_int		hitplus;		
    sh_int		damnodice;	
    sh_int		damsizedice;
    sh_int		damplus;
    sh_int		position;
    int			gold;
    MPROG_DATA *	mobprogs;
    int			progtypes;
    int      		max_mobs;
    bool      		corrected;  /* Flag is the game thought the critter was odd */
    int     		armor;  /* Basic starting armor added to AC */
    int     		body_parts;   /* Flags in Thac0 field for body parts */
    int     		attack_parts; /* Flags in AC field for attacking parts */
    int 		race;  /* Defines and hardwires race of mobile */
    int                 creator_pvnum;
    char *    		picture;   /* Dispayable picture of mobile */
    char *		mob_file; /*File of external for mob */
};

#define BODY_HEAD		1
#define BODY_MOUTH       	2   
#define BODY_EYE		4  /* Only 1 part is required */
#define BODY_TORSO      	8
#define BODY_HIP		16
#define BODY_LEG		32
#define BODY_ARM		64
#define BODY_WING		128
#define BODY_TAIL		256
#define BODY_TENTICLE		512
#define BODY_HORN		1024
#define BODY_CLAW		2048
#define BODY_HAND		4096
#define BODY_FOOT		8192


/*
 * One character (PC or NPC).
 */


struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		prev;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		prev_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    /*CHAR_DATA *		fighting;*/
    FIGHT_DATA *	fighting;
    CHAR_DATA *		reply;
    SPEC_FUN *		spec_fun;
    RESET_DATA *	reset;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    NOTE_DATA *		pnote;
    OBJ_DATA *          first_carrying;
    OBJ_DATA *          last_carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    PC_DATA *		pcdata;
    NPC_DATA *		npcdata;
    EDITOR_DATA *       editor;
    DO_FUN *		last_cmd;
    HHF_DATA *		hunting;
    HHF_DATA *		fearing;
    HHF_DATA *		hating;
    void *              spare_ptr;
    void *              dest_buf;
    sh_int		substate;
    sh_int              tempnum;
    int       		ansi;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		level;
    sh_int		trust;
    int			played;
    int			killer_played;
    int			outcast_played;
    time_t		logon;
    time_t		save_time;
    sh_int		timer;
    sh_int		wait;
    sh_int		hit;
    sh_int		max_hit;
    sh_int		actual_max_hit;
    sh_int		mana;
    sh_int		max_mana;
    sh_int		actual_max_mana;
    sh_int		move;
    sh_int		max_move;
    sh_int		actual_max_move;
    int			gold;
    int			exp;
    int     		exp_lost;
    int			act;
    int			affected_by;
    int			affected2_by;
    sh_int		position;
    sh_int		practice;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor;
    sh_int		wimpy;
    sh_int		deaf;
    int                 note_amount;
    int                 vt100;
    int                 vt100_type;
    int                 clock;
    sh_int              hook;
    int     		recall;
    sh_int              mclass[MAX_CLASS];
    sh_int              mclass_switched;
    sh_int              alias_ip;
    sh_int              shot_timer;
    sh_int		shot_from;
    bool		shot_aware;
    MPROG_ACT_LIST *	mpact;
    int			mpactnum;
    sh_int              speed;
    CHAR_DATA *		shadowing;
    CHAR_DATA *		shadowed_by;
    int 		critical_hit_by;  /* The PVNUM of the hitter */
    sh_int              distracted;
    int                 language;
    int                 speak;
    sh_int              attack;   /* Used to allow 1 disarm, etc.  */
    sh_int              which_god;
    OBJ_WITH_PROG     * obj_with_prog;
    sh_int              obj_prog_ip;    /* 1 while in process */
    OBJ_DATA          * asn_obj;  /* object assassinating for */
    char                rank;  /* values of 0 to 10 */
    int 		retran;
    int			regoto;
    sh_int		inter_substate;
    sh_int 		num_fighting;
};


/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    CLAN_DATA *		clan;
    HELP_DATA *		last_help;
    HELP_DATA *		prev_help;
    CHAR_DATA *		bet_victim;
    OBJ_DATA *		corpse;  /* Pointer to player's corpse */
    CASTLE_DATA *	castle;
    TACTICAL_MAP *	tactical; /* The recorded tactical map for comparison */
    ROOM_INDEX_DATA *	travel_from;  /* Traveling old room  */
    POISON_DATA *	poison;
    AREA_DATA * 	area;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *              alias           [MAX_ALIAS];
    char *              alias_c         [MAX_ALIAS];
    char * 		killname        [MAX_KILL_TRACK];

    char *              mail_address;
    char *              html_address;
    char *    		clan_name;
    char *    		clan_pledge;
    char *		back_buf[26]; /* Change to pointer to string mode */
    char *		page_buf;
    char * 		tracking;
    char * 		prompt_layout;  /* For reconfiguration */
    char * 		subprompt;  /* subprompt for OLC*/
    char *		auto_command;
    char *		tactical_index;  /* The two byte index for a player */
    char *		block_list;  /* list of characters to notell/nobeep */
    KILLED_DATA         killed          [MAX_MOB_KILL_TRACK];
    int			topic_stamp [MAX_TOPIC];
    int			last_note;
    int       		note_topic;
    int          	give_prac_pos;
    int          	give_prac_neg;
    int       		just_died_ctr;
    int       		time_of_death;
    int       		demisedlevel;
    int       		port_size;
    int       		port_baud;
    int       		pvnum;         /* Player's virtual number   */
    int       		color[2][MAX_COLOR];     /* The color array for ANSI */
    int       		corpse_room;  /* The room of the player's corpse */
    int      		clan_position;
    int       		scroll_start;
    int       		scroll_end;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		mod_str;
    sh_int		mod_int;
    sh_int		mod_wis;
    sh_int		mod_dex;
    sh_int		mod_con;
    sh_int		condition	[3];
    sh_int		learned		[MAX_SKILL];
    sh_int  		killnum;
    sh_int    		page_mode;
    unsigned char    	*quest[MAX_AREA];
    char      		scroll_buf[MAX_BUFFER_LENGTH];
    sh_int    		scroll_ip;
    sh_int     		old_hit;
    sh_int     		old_max_hit;
    sh_int     		old_mana;
    sh_int     		old_move;
    int        		old_gold;
    int        		old_exp;
    int        		old_time;
    int        		old_dir;
    bool       		help_mode;
    sh_int     		auto_flags;
    int        		bet_amount;
    int        		creation_room;  /* Room where creation starts */
    int        		creator_zone; /* Assigned zone number */
    int        		last_connect;   /* min*60+sec  to block tintin repeat */
    int        		last_time;   /* current_time of last save */
    int        		death_room;     /* vnum of saved death room */
    int        		channel;  /* The channel number tuned to */
    int        		channel_request;  /* The channel number requested */
    int        		spam;   /*  Spam out the combat lines */
    int        		cont_tick;   /* The continuous command counter  */
    int        		tactical_mode;  /* The tactical map mode */
    bool       		tactical_update;  /* Does the tactical need to be checked? */
    int        		term_info;  /* Information on the terminal program */
    bool       		unsafe_password;
    unsigned char   	compass_width;  /* Compass setting in Tactical  */
    int 		last_saved;   /* time of last save */
    int        		previous_hours;   /* Hours played before reincarnation */
    bool      		oprog_started;   /* test to check */
    bool       		switched;    /* Comm routines need to know this */
    bool      		dump;  /* For the dump command */
    int       		last_real_room;  /* For Astral projection spell */
    int       		player2_bits;    /* Additional player bits */
    int        		travel;  /* Traveling Direction, Normally -1 */
    int	       		account; /*Character's Bank Account*/
    int        		jailtime;
    int        		jaildate;
    int      		MrTerm_Experience;
    int       		MrTerm_Exp_Level;
    int       		MrTerm_Gold;
    sh_int     		packet[MAX_PACKETS];   /* Bitvectors of used packets */
    sh_int     		request;     /* Bitvectors for requests           */
    sh_int     		bet_mode;
    sh_int     		obj_version_number;  /* This is used to wipe out all items */
    sh_int     		reincarnation;
    sh_int     		allow_reincarnate; /* 0=false, 1=true */
    sh_int     		death_timer;    /* Fixes multiple deaths */
    sh_int     		army_status;  /* 0-Not, 1-In, 2-Out */
    sh_int     		arrested;
    sh_int		eqdamroll;
    sh_int		eqhitroll;
    sh_int		eqsaves;
           /* The following stats are for the auto-updating MrTerm stuff */
    sh_int    		MrTerm_Max_Hit;
    sh_int    		MrTerm_Max_Mana;
    sh_int    		MrTerm_Max_Move;
    sh_int    		MrTerm_Hit;
    sh_int    		MrTerm_Mana;
    sh_int    		MrTerm_Move;
    sh_int    		MrTerm_Thirst;
    sh_int    		MrTerm_Full;
    sh_int    		MrTerm_Position;
    int                 r_range_lo;     /* room range */
    int                 r_range_hi;
    int                 m_range_lo;     /* mob range  */
    int                 m_range_hi;
    int                 o_range_lo;     /* obj range  */
    int                 o_range_hi;
    sh_int     		old_wait;
    sh_int     		history[7];
    /* pthread_t beingsaved; */
    int			last_pk_attack_time[MAX_PK_ATTACKS];
    int			last_pk_attack_pvnum[MAX_PK_ATTACKS];
    char		*last_pk_attack_name[MAX_PK_ATTACKS];
    char		*last_command;
    int			area_kills[ MAX_AREA ];  /* Kills in an area */
};

/*
 * a PC's CASTLE information
 */
struct	castle_data
  {
  int     entrance;     /* room# of first room in castle */
  int     door_room;    /* room# of first door command */
  sh_int     door_dir;     /* direction of first door command */
  bool       has_backdoor; /* TRUE if the PC has already created a back door */
  int        cost;         /* the total cost of the castle */
  sh_int     num_rooms;    /* number of rooms created must be < level/4 */
  sh_int     num_mobiles;  /* number of mobiles created must be < level/4 */
  sh_int     num_objects;  /* number of objects created must be < level/4 */
  int     reign_room;   /* room# of main room for Lords */
  };

struct owner_data
{
  OWNER_DATA * next;
  OWNER_DATA * prev;
  char * name;
  int pvnum;
  int lastentry;
};  
/*
 * The saved tactical map to be compared with.
 */
#define MAX_TACTICAL 20
struct	tactical_map
  {
  unsigned char map[MAX_TACTICAL][80];  
         /* Add 128 to normal ascii code for BOLD text */
  unsigned char color[MAX_TACTICAL][80];
        /*  format:    fg%8 + bg%8 * 8        */
  };

/*
 * Data for Clans.
 */

struct	clan_data
{
    CLAN_DATA *		next;	/* pointer to next clan */
    CLAN_DATA *		prev;	/* pointer to previous clan */
    char *		name;		/* Clan name */
    char * 		filename;	/* file name */
    char *		motto;		/* Clan Motto */
    char *		description;	/* Short description of clan */
    char * 		leader;		/* Clan leader name */
    char * 		number1;	/* Clan number one name */
    char * 		number2;	/* Clan number two name */
    char * 		number3;	/* Clan number three name */
    char * 		number4;	/* Clan number four name */
    char * 		email;		/* Clan Councillor email address */
    int			pkills[5];	/* Number of kills by clan */
    int			pdeaths[5];	/* Number of deaths by player */
    int			mkills;		/* Number of mob kills */
    int 		mdeaths;	/* Number of mob deaths by whole clan*/
    int 		tax;		/* Tax rate on mob kills */
    long        	coffers;	/* How much gold the clan has */
    int			members;	/* Number of members */
    int 		clanobj1;	/* Vnum of first clan obj */
    int 		clanobj2;	/* Vnum of second clan obj */
    int 		home;		/* Vnum of clans gohome destination */
    int 		store;		/* Vnum of clan's store room */
    int			guard;          /* Vnum of clan guard */
    int 		healer;		/* Vnum of clan healer */
    int			last_saved;     /* Track save time of clans, save
					   asynchronously if possible */
    int 		founder_pvnum;
    int 		num_guards;
    int 		num_healers;
    int                 num_backdoors;
    int                 num_leaders;
    int			type;
};

#define REQUEST_PACKETS          1
#define REQUEST_HOOK             2
#define REQUEST_RAW_DATA         4
#define REQUEST_VT_SAVE_ON       8  /* turns off vt/ansi but saves on */
#define REQUEST_ANSI_SAVE_ON    16 
#define REQUEST_PROMPT_SAVE_ON  32  /* Turns off prompt but saves on */

#define HISTORY_KILL_PC     0
#define HISTORY_KILL_EN     1
#define HISTORY_KILL_BY_PC  2
#define HISTORY_KILL_BY_EN  3
#define HISTORY_KILL_BY_NPC 4
#define HISTORY_KILL_ARMY   5
#define HISTORY_LOSE_ARMY   6

#define CLAN_ASSASSINATE_KILL  	0
#define CLAN_ASSASSINATE_DEATH 	0
#define CLAN_ORDER_KILL 	1
#define CLAN_ORDER_DEATH 	1
#define CLAN_CHAOS_KILL 	2
#define CLAN_CHAOS_DEATH 	2
#define CLAN_RACE_KILL 		3
#define CLAN_RACE_DEATH		3
#define CLAN_KILL 		4
#define CLAN_DEATH 		4

#define AUTO_OFF            0
#define AUTO_AUTO           1
#define AUTO_QUICK          2

#define GOD_NEUTRAL 	0
#define GOD_ORDER   	1
#define GOD_CHAOS   	2
#define GOD_DEMISE  	3
#define GOD_POLICE  	4
#define GOD_CREATOR 	5
#define GOD_INIT_ORDER 	6
#define GOD_INIT_CHAOS 	7

/*
 * Data which only NPC's have.
 */

struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct  mob_prog_act_list
  {
  MPROG_ACT_LIST * next;
  char *           buf;
  CHAR_DATA *      ch;
  OBJ_DATA *       obj;
  void *           vo;
  };

MOB_PACKET *mob_packet_list;
struct  mob_prog_packet
  {
  MOB_PACKET *next;
  CHAR_DATA *actor;
  CHAR_DATA *mob;
  CHAR_DATA *rndm;
  OBJ_DATA *obj;
  void *vo;
  };

struct  mob_prog_data
{
  MPROG_DATA *next;
  int         type;
  bool	      triggered;
  char *      arglist;
  char *      comlist;
  NPC_TOKEN * token_list;
};

struct  mob_prog_token
  {
  NPC_TOKEN * next;
  int         type;     /* 
			   1 - SOCIAL
			   2 - COMMAND
		           3 - IF
			   4 - OR
			   5 - ELSE
			   6 - ENDIF
			   7 - ENDIF
		 		     */	
  char *      string;
  int         value;	/* The value of the command or social returned by
			   find_command or social_table[value] */
  void *      function; /* Which do_fun to use */
  char        level;    /* The functional IF level */
  int         line;     /* The line number */
  };

/* The following structure is set up so that individual monsters may have
   different stats     -  Chaos 5/25/94 */
struct	npc_data
{
    NPC_DATA *		next;  /* Currently not used */
    unsigned char       *mob_quest;
    sh_int		damnodice;	
    sh_int		damsizedice;
    sh_int		damplus;
    sh_int    		armor;
    int                 pvnum_last_hit;  /* For cheating check */
    int                 pvnum_last_hit_leader;  /* For cheating check */
    int                 mob_number;
    bool                mob_prog_started;
    POISON_DATA        *poison;
};

bool    MOBtrigger;


/* costs are in thousands of dollars */
#define COST_OF_ENTRANCE 30000
#define COST_OF_ROOM     20000
#define COST_OF_DOOR      4000
#define COST_OF_BACKDOOR 70000
#define COST_OF_STAT        10
#define COST_OF_SET       1000
#define COST_OF_CREATE   15000

#define RENT_BASIC_CLAN_HALL  50000000
#define RENT_BASIC_ORDER_HALL 50000000
#define RENT_PER_GUARD        1500000
#define RENT_PER_HEALER       3000000
#define RENT_PER_BACKDOOR     12500000
#define CLANHALL_CONSTRUCTION 150000000

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
#define RANGE_PROG      2048
#define SOCIAL_PROG     4096
#define KILL_PROG       8192
#define GROUP_GREET_PROG  16384
#define TIME_PROG	32768

/*
 *  Communication flags for reference during game_loop_unix
 */
#define COMM_FLAG_KILL        1
#define COMM_FLAG_DISCONNECT  2   /* used by write_to_descriptor */
#define COMM_FLAG_QUIT        4

/*
 * Object triggers
 */
#define OBJ_UPDATE	       1
#define OBJ_COMMAND        2
#define OBJ_UNKNOWN        3


/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		16

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[3];
};



/*
 * BOUNTY_DATA
 *
 * Structure for storing bounty information.
 *
 * Presto 2-20-99
 */
struct bounty_data
{
  BOUNTY_DATA *next;
  BOUNTY_DATA *prev;
  char  *name;
  int   amount;
  int   postdate;
  int   expires;
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    EXTRA_DESCR_DATA *prev;	/* Prev in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *       first_affect;
    AFFECT_DATA *       last_affect;
    OBJ_FUN *		obj_fun;
    AREA_DATA *		area;
    OBJ_PROG *		obj_prog;   /* start of object's program */
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    int			vnum;
    sh_int		item_type;
    int                 extra_flags;
    sh_int		wear_flags;
    int			count;
    sh_int		weight;
    int			cost;
    int     		max_objs;
    int			value	[4];
    int     		level_rent;
    int     		level;
    int           	total_objects;  /* Total for resets */
    char *		attack_string;
    int                 class_flags;   /* List of which classes can use weapon*/
    int                 creator_pvnum;
    char *		picture;  /* The 80x40 bit picture */
    char *		obj_file; /* External file */
};


struct	poison_data
{
    POISON_DATA *next;
    bool    for_npc;
    sh_int  poison_type;
    int     instant_damage_low;
    int     instant_damage_high;
    int     constant_duration;        /* In combat rounds */
    int     constant_damage_low;
    int     constant_damage_high;
    int     owner;              /* Pvnum of creator of poison */
    int     poisoner;    /* PVNUM of poisoner */
};

/*
 * One swear word.
 */
struct swear_data
{
  SWEAR_DATA * next;
  char       * word;
};


/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		prev;
    OBJ_DATA *		next_content;
    OBJ_DATA *		prev_content;
    OBJ_DATA *          first_content;
    OBJ_DATA *          last_content;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    RESET_DATA *	reset;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    POISON_DATA * 	poison;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		long_descr;
    sh_int		item_type;
    int                 extra_flags;
    sh_int		wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int		timer;
    sh_int		sac_timer;
    int			value	[4];
    unsigned char       *obj_quest;
    int                 owned_by;  /*  PVNUM of owner */
    bool                saved;
    bool                basic;  /* internal flag for basic vnum */
    bool                test_obj;
    char *              unforged_short_descr;
    char *              unforged_name;
    sh_int              content_weight; /* total weight of container */
    int                 index_reference[2]; /* Used to track item duping */
    sh_int              condition;  /* Condition of item 0-Death 100-Perfect */
};



/*
 * Exit data.
 */
struct	exit_data
{
    ROOM_INDEX_DATA *	to_room;
    int			vnum;
    int                 pvnum;
    sh_int		exit_info;
    int			key;
    char *		keyword;
    char *		description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *        prev;
    char		command;
    int			arg0;
    int			arg1;
    int			arg2;
    int			arg3;
    CHAR_DATA *		mob;
    OBJ_DATA *		obj; 
    RESET_DATA *	container; 
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    AREA_DATA *		prev;
    RESET_DATA *	first_reset;
    RESET_DATA *	last_reset;
    AREA_DATA *		next_sort_name;
    AREA_DATA *		prev_sort_name;
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    char *		name;
    char *              filename;
    sh_int		age;
    sh_int		nplayer;
    int			flags;
    int       		average_level;
    int       		tmp;
    int       		count;
    int                 hi_hard_range;    /* This is enforced */
    int                 low_hard_range;
    int                 hi_soft_range;    /* This is reccomended */
    int                 low_soft_range;
    char *		authors;
    char *		resetmsg;
    int                 mkills;
    int                 mdeaths;
    int                 pkills;
    int                 pdeaths;
    WEATHER_DATA	weather_info;
    int                 low_r_vnum;
    int                 hi_r_vnum;
    int                 low_o_vnum;
    int                 hi_o_vnum;
    int                 low_m_vnum;
    int                 hi_m_vnum;
    /* pthread_t beingreset; */
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		first_person;
    CHAR_DATA *		last_person;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    char *		name;
    char *		description;
    int			vnum;
    int		    	room_flags;
    sh_int		light;
    sh_int		sector_type;
    int              	fall_room;  /* Room the player goes to on a fall */
    int              	fall_slope;  /* Slope of a cliff for falling */
    sh_int              distance_of_fall;  /*  Distance in feet of fall */
    int       		owned_by;   /* The pvnum of the player that owns that room */
    char                last_left[MAX_LAST_LEFT][21];
    int                 last_left_bits[MAX_LAST_LEFT];
    int             	creator_zone;
    int             	content_count;
    int                 creator_pvnum;
    sh_int          	sanctify_timer;
    ROOM_INDEX_DATA *	hallucinate_room;
    sh_int          	hallucinate_timer;
    sh_int          	smoke_timer;
    CHAR_DATA *     	sanctify_char;
    char *          	room_file;
};

/*
 * System usage information
 */
struct usage_data
  {
  long numPlayers;
  long numRecons;
  long players[24][7];
  long recons[24][7];
  };


/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_NOMESSAGE               -2
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000
#define TYPE_NOFIGHT                 999
#define TYPE_HOOVES                  998



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4

/*
 * Maximum # of rooms seeable in a directional look
 */
#define SHORT_RANGE 1
#define MED_RANGE   2
#define LONG_RANGE  5

struct bitvector_type  /* text of particular body parts */
  {
    char *name;  /* name of bit */
    int value;
  };

struct body_type  /* text of particular body parts */
  {
    char *name;            /* names of body part when sliced */
    char *short_descr;     /* short_descr of body part when sliced */
    char *description;     /* description of sliced body part */
    char *long_descr;      /* long_descr of body part when sliced */
    char *attack;          /* kind of attack the body part does */
    char *sliced;          /* what it does when it is sliced off */
  };

/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			  /* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *  cmd_name;   /* name of corresponding command */
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	sh_int	gsn_backstab;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_climb;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_martial_arts;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_fourth_attack;
extern  sh_int  gsn_shoot;
extern  sh_int  gsn_throw;
extern  sh_int  gsn_short_range;
extern  sh_int  gsn_med_range;
extern  sh_int  gsn_long_range;
extern  sh_int  gsn_shadow;
extern  sh_int  gsn_divert;
extern  sh_int  gsn_voice;
extern  sh_int  gsn_bargain;
extern  sh_int  gsn_lock;
extern  sh_int  gsn_knife;
extern  sh_int  gsn_make_poison;
extern  sh_int  gsn_greater_hide;
extern  sh_int  gsn_greater_sneak;
extern  sh_int  gsn_stealth;
extern  sh_int  gsn_disguise;
extern  sh_int  gsn_spy;
extern  sh_int  gsn_attack;
extern  sh_int  gsn_armor_usage;
extern  sh_int  gsn_distract;
extern  sh_int  gsn_greater_peek;
extern  sh_int  gsn_snatch;
extern  sh_int  gsn_greater_pick;
extern  sh_int  gsn_track;
extern  sh_int  gsn_clear_path;
extern  sh_int  gsn_hunt;
extern  sh_int  gsn_notice;
extern  sh_int  gsn_rearm;

extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_mage_blast;
extern	sh_int	gsn_sleep;
extern  sh_int  gsn_write_spell;
extern  sh_int  gsn_stability;
extern  sh_int  gsn_zap;
extern	sh_int	gsn_haste;
extern  sh_int  gsn_glance;
extern  sh_int  gsn_assassinate;
extern  sh_int  gsn_guard;
extern  sh_int  gsn_plant;
extern  sh_int  gsn_camp;
extern  sh_int  gsn_mass;
extern  sh_int  gsn_berserk;
extern  sh_int  gsn_forge;
extern  sh_int  gsn_repair;
extern  sh_int  gsn_detect_forgery;
extern  sh_int  gsn_critical_hit;

extern  sh_int  gsn_greater_backstab;
extern  sh_int  gsn_gouge;
extern  sh_int  gsn_circle;
extern  sh_int  gsn_trip;
extern  sh_int  gsn_anatomy;

extern  sh_int  gsn_pass_without_trace;
extern  sh_int  gsn_quick_draw;
extern  sh_int  gsn_flash_powder;
extern  sh_int  gsn_greater_stealth;
extern  sh_int  gsn_muffle;

extern  sh_int  gsn_truesight;
extern  sh_int	gsn_improved_invis;
extern  sh_int	gsn_nightmare;

extern  sh_int	gsn_anti_magic_shell;
extern  sh_int	gsn_slow;
extern  sh_int	gsn_fire_shield;
extern  sh_int	gsn_vampiric_touch;

extern  sh_int	gsn_bashdoor;
extern  sh_int	gsn_greater_hunt;
extern  sh_int	gsn_greater_track;
extern  sh_int	gsn_bash;
extern  sh_int	gsn_dual_wield;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define IS_SHIFT(val, bit)      ((val) & (1)<<(bit))
#define NOT(val)     ((val)^0xFFFFFFFF)


/*
 * Character macros.
 */

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)


#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_CREATOR(ch)		(which_god(ch) == GOD_CREATOR)
#define IS_AFFECTED(ch, sn)	( sn>-1 ? IS_SET((ch)->affected_by, (sn)) :  \
                                   IS_SET((ch)->affected2_by, (0-sn)) )

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch)		((ch)->armor				    \
				    + ( IS_AWAKE(ch)			    \
				    ? dex_app[get_curr_dex(ch)].defensive   \
				    : 0 ))
/* #define GET_HITROLL(ch)		((ch)->hitroll+str_app[get_curr_str(ch)].tohit) */
/*  #define GET_DAMROLL(ch)		((ch)->damroll+str_app[get_curr_str(ch)].todam) */

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))
#define IS_DRUNK(ch, drunk)     (number_percent() < \
                                ( (ch)->pcdata->condition[COND_DRUNK] \
                                * 2 / (drunk) ) )

#define NO_WEATHER_SECT(sect)   ( sect == SECT_INSIDE ||		    \
				  sect == SECT_UNDER_WATER ||		    \
				  sect == SECT_UNDER_GROUND ||		    \
 				  sect == SECT_ASTRAL ||		    \
				  sect == SECT_ETHEREAL ||		    \
				  sect == SECT_DEEP_EARTH )

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse*3/2)))

#define DIR_FROM(ch)            ((ch)->shot_stat&&0x07)
#define DIST_FROM(ch)           (((ch)->shot_stat&&0x038)/8)
#define SET_DIR_FROM(ch,dir)    ((ch)->shot_stat=(ch)->shot_stat&&0x0f8+dir)
#define SET_DIST_FROM(ch,d)     ((ch)->shot_stat=(ch)->shot_stat&&0x0c7+(d*8))



/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )



/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    sh_int            * cmd_number;
};

struct save_data
{
   CHAR_DATA * ch;
   int         type;
};

struct hunt_hate_fear
{
    char *              name;
    CHAR_DATA *         who;
};

struct save_area_data
{
 AREA_DATA * Area;
 bool Reality;
};


/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char * const	name;
    char * const	char_no_arg;
    char * const	others_no_arg;
    char * const	char_found;
    char * const	others_found;
    char * const	vict_found;
    char * const	char_auto;
    char * const	others_auto;
};



/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app		[36];
extern	const	struct	int_app_type	int_app		[36];
extern	const	struct	wis_app_type	wis_app		[36];
extern	const	struct	dex_app_type	dex_app		[36];
extern	const	struct	con_app_type	con_app		[36];

extern	const	struct	bitvector_type	bitvector_table	[MAX_BITVECTOR];
extern  const   struct  topic_data     	topic_table	[MAX_TOPIC];
extern	const	struct	class_type	class_table	[MAX_CLASS];
extern  const   struct  race_type       race_table      [MAX_RACE];
extern	const	struct	cmd_type	cmd_table	[];
extern  sh_int cmd_gsn [];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern	const	struct	body_type	body_table	[];
extern	const	struct	social_type	social_table	[];
extern	char *	const			title_table	[MAX_CLASS]
							[MAX_LEVEL+1]
							[2];
extern  char *	const	dir_name	[];
extern  const	sh_int	rev_dir		[];
extern  const	sh_int	monthdays		[];

extern int		top_exit;
extern int		top_room;
extern ROOM_INDEX_DATA *room_index_hash		[MAX_KEY_HASH];
extern int		highest_vnum;

extern char *  const   mprog_flags 	[];
extern char *  const   area_flags	[];
extern char *  const   ex_flags 	[];
extern char *  const   r_flags 		[];
extern char *  const   o_flags 		[];
extern char *  const   act_flags 	[];
extern char *  const   a_flags 		[];
extern char *  const   plr_flags 	[];
extern char *  const   wear_locs 	[];
extern char *  const   part_flags 	[];
extern char *  const   o_types 		[];
extern char *  const   w_flags 		[];
extern char *  const   a_types 		[];

/*
 * Global variables.
 */

extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;

extern		BOUNTY_DATA	  *	first_bounty;
extern		BOUNTY_DATA	  *	last_bounty;

extern		BAN_DATA	  *	ban_list;

extern		DESCRIPTOR_DATA   *	first_descriptor;
extern		DESCRIPTOR_DATA   *	last_descriptor;

extern		NOTE_DATA	  *	first_note;
extern		NOTE_DATA	  *	last_note;

extern		OBJ_DATA	  *	first_object;
extern		OBJ_DATA	  *	last_object;

extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;

extern 		OWNER_DATA 	  *	first_owner;
extern 		OWNER_DATA 	  *	last_owner;

extern 		CLAN_DATA 	  *	first_clan;
extern 		CLAN_DATA 	  *	last_clan;

extern 		AREA_DATA	  *	first_area;
extern 		AREA_DATA	  *	last_area;

extern          AREA_DATA         *     first_area_name; 
extern          AREA_DATA         *     last_area_name; 

extern		PLAYER_GAME  	  *	first_player;
extern		PLAYER_GAME  	  *	last_player;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		FILE *			fpAppend;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;

extern          USAGE_DATA              usage;
extern          struct tm               tme;
extern          struct act_prog_data *  mob_act_list;
extern 		int 			num_descriptors;
extern          SWEAR_DATA        *     swear_list;
extern 		CHAR_DATA 	  * 	Greeter;
extern 		CHAR_DATA 	  * 	Greeted;

extern 		MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
extern		OBJ_INDEX_DATA *        obj_index_hash          [MAX_KEY_HASH];


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN(	do_afk    	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN(	do_arrest	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_bank		);
DECLARE_DO_FUN(	do_bounty	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_identify);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_mana		);
DECLARE_DO_FUN(	do_move		);
DECLARE_DO_FUN(	do_rcast	);
DECLARE_DO_FUN(	do_channel 	);
DECLARE_DO_FUN(	do_channel_talk );
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN(	do_beep		);
DECLARE_DO_FUN(	do_plan		);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_skills       );
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_config2	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_decline	);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_forage       );
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_etell        );
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_timemode	);
DECLARE_DO_FUN(	do_prompt	);
DECLARE_DO_FUN(	do_initiate	);
DECLARE_DO_FUN(	do_maze		);
DECLARE_DO_FUN(	do_giveprac	);
DECLARE_DO_FUN(	do_external	);
DECLARE_DO_FUN(	do_download	);
DECLARE_DO_FUN(	do_pload	);
DECLARE_DO_FUN(	do_pquit	);
DECLARE_DO_FUN(	do_victory_list	);
DECLARE_DO_FUN(	do_war 		);
DECLARE_DO_FUN(	do_army		);
DECLARE_DO_FUN(	do_roomfragment );
DECLARE_DO_FUN(	do_travel	);
DECLARE_DO_FUN(	do_most		);
DECLARE_DO_FUN(	do_dump		);
DECLARE_DO_FUN(	do_llog		);
DECLARE_DO_FUN(	do_map 		);
DECLARE_DO_FUN(	do_a		);
DECLARE_DO_FUN(	do_termlist	);
DECLARE_DO_FUN(	do_reincarnate	);
DECLARE_DO_FUN(	do_reign	);
DECLARE_DO_FUN(	do_rank		);
DECLARE_DO_FUN(	do_mpquiet	);
DECLARE_DO_FUN(	do_terminal	);
DECLARE_DO_FUN(	do_tactical	); 
DECLARE_DO_FUN(	do_spam		);
DECLARE_DO_FUN(	do_grant	);
DECLARE_DO_FUN(	do_display	);
DECLARE_DO_FUN(	do_resetarea	);
DECLARE_DO_FUN(	do_savearea	);
DECLARE_DO_FUN(	do_email	);
DECLARE_DO_FUN(	do_html		);
DECLARE_DO_FUN(	do_strength	);
DECLARE_DO_FUN(	do_intelligence );
DECLARE_DO_FUN(	do_wisdom  	);
DECLARE_DO_FUN(	do_constitution );
DECLARE_DO_FUN(	do_dexterity    );
DECLARE_DO_FUN(	do_lookup	);
DECLARE_DO_FUN(	do_know         );
DECLARE_DO_FUN(	do_create	);
DECLARE_DO_FUN(	do_bet		);
DECLARE_DO_FUN(	do_resign	);
DECLARE_DO_FUN(	do_accept	);
DECLARE_DO_FUN(	do_request	);
DECLARE_DO_FUN(	do_engrave	);
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_auto         );
DECLARE_DO_FUN(	do_repeat	);
DECLARE_DO_FUN(	do_release       );
DECLARE_DO_FUN(	do_finger	);
DECLARE_DO_FUN(	do_history);
DECLARE_DO_FUN(	do_door	);
DECLARE_DO_FUN(	do_key	);
DECLARE_DO_FUN(	do_rescale );
DECLARE_DO_FUN(	do_connect	);
DECLARE_DO_FUN(	do_mprogram	);
DECLARE_DO_FUN(	do_oprogram	);
DECLARE_DO_FUN(	do_shutoff 	);
DECLARE_DO_FUN(	do_undeny 	);
DECLARE_DO_FUN(	do_fixpass 	);
DECLARE_DO_FUN(	do_test1	);
DECLARE_DO_FUN(	do_test2	);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_bash		);
DECLARE_DO_FUN(	do_bashdoor	);
DECLARE_DO_FUN(	do_martial_arts );
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_scan		);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_cpu		);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpareaecho   );
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpgoto       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpjunk_person );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN( do_mpmset       );
DECLARE_DO_FUN( do_mposet       );
DECLARE_DO_FUN( do_mpmadd       );
DECLARE_DO_FUN( do_mpoadd       );
DECLARE_DO_FUN( do_mpgorand     );
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_resetquest	);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mlist	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_owhere       );
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_object_rape	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN(	do_olist	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_rename	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_death	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_revert	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN(	do_status	);
DECLARE_DO_FUN(	do_affects);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_slaughter	);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_hook   );
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN( do_level        );
DECLARE_DO_FUN( do_vt100        );
DECLARE_DO_FUN( do_color        );
DECLARE_DO_FUN( do_buffer       );
DECLARE_DO_FUN( do_grep         );
DECLARE_DO_FUN( do_port         );
DECLARE_DO_FUN( do_speed        );
DECLARE_DO_FUN( do_alias        );
DECLARE_DO_FUN( do_unalias      );
DECLARE_DO_FUN( do_block        );
DECLARE_DO_FUN( do_class        );
DECLARE_DO_FUN( do_multi   );
DECLARE_DO_FUN( do_clock        );
DECLARE_DO_FUN( do_refresh      );
DECLARE_DO_FUN(	do_tick         );
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_retran	);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_language	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN(	do_shoot	);
DECLARE_DO_FUN( do_track        );
DECLARE_DO_FUN( do_notice       );
DECLARE_DO_FUN( do_delete       );
DECLARE_DO_FUN( do_suicide      );
DECLARE_DO_FUN( do_speak        );
DECLARE_DO_FUN(	do_throw	);
DECLARE_DO_FUN( do_shadow       );
DECLARE_DO_FUN( do_divert       );
DECLARE_DO_FUN( do_voice	);
DECLARE_DO_FUN( do_knife        );
DECLARE_DO_FUN( do_make_poison  );
DECLARE_DO_FUN( do_make_flash   );
DECLARE_DO_FUN( do_stealth      );
DECLARE_DO_FUN( do_disguise     );
DECLARE_DO_FUN( do_spy		);
DECLARE_DO_FUN( do_distract     );
DECLARE_DO_FUN( do_snatch       );
DECLARE_DO_FUN( do_clear_path   );
DECLARE_DO_FUN( do_hunt         );
DECLARE_DO_FUN(	do_evaluate   	);
DECLARE_DO_FUN(	do_usage	);
DECLARE_DO_FUN(	do_castle       );
DECLARE_DO_FUN( do_assassin     );
DECLARE_DO_FUN( do_assassinate  );
DECLARE_DO_FUN( do_glance       );
DECLARE_DO_FUN(	do_plant	);
DECLARE_DO_FUN(	do_camp		);
DECLARE_DO_FUN(	do_mass         );
DECLARE_DO_FUN(	do_berserk      );
DECLARE_DO_FUN(	do_forge        );
DECLARE_DO_FUN( do_delcastle    );
DECLARE_DO_FUN( do_gohome	);
DECLARE_DO_FUN(	do_copyover     );
DECLARE_DO_FUN(	do_trip         );
DECLARE_DO_FUN(	do_gouge        );
DECLARE_DO_FUN(	do_circle       );
DECLARE_DO_FUN(	do_hearlog      );

DECLARE_DO_FUN( do_attack	);
DECLARE_DO_FUN( do_clans	);
DECLARE_DO_FUN( do_protect      );
DECLARE_DO_FUN( do_outcast	);
DECLARE_DO_FUN( do_makeclan	);
DECLARE_DO_FUN( do_destroy_clan	);
DECLARE_DO_FUN( do_setclan	);
DECLARE_DO_FUN( do_hedit	);
DECLARE_DO_FUN(	do_coffer	);
DECLARE_DO_FUN(	do_pledge	);
DECLARE_DO_FUN( do_clanwhere	);
DECLARE_DO_FUN( do_renounce	);
DECLARE_DO_FUN( do_nominate	);
DECLARE_DO_FUN(	do_heal		);

DECLARE_DO_FUN( do_orders	);

/* OLC stuff Martin 2/1/99 */
DECLARE_DO_FUN( do_rassign	);
DECLARE_DO_FUN( do_oassign	);
DECLARE_DO_FUN( do_massign	);
DECLARE_DO_FUN( do_rdelete	);
DECLARE_DO_FUN( do_mdelete	);
DECLARE_DO_FUN( do_odelete	);
DECLARE_DO_FUN(	do_regoto 	);
DECLARE_DO_FUN( do_mcreate	);
DECLARE_DO_FUN( do_ocreate	);
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_medit	);
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN(	do_aset		);
DECLARE_DO_FUN(	do_astat	);
DECLARE_DO_FUN(	do_reset        );
DECLARE_DO_FUN(	do_rreset       );
DECLARE_DO_FUN(	do_instaroom    );
DECLARE_DO_FUN(	do_instazone    );

DECLARE_DO_FUN( do_forcerent	);
DECLARE_DO_FUN( do_forceren	);
DECLARE_DO_FUN( do_send_clan_message	);
DECLARE_DO_FUN( do_donate	);
DECLARE_DO_FUN(	do_immtalk	);
/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_color_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_haste  		);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_tremor		);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(      spell_invis_obj         );
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_energy_shift	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_astal             );
DECLARE_SPELL_FUN(	spell_homonculous       );
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_protection_fe	);
DECLARE_SPELL_FUN(	spell_protection_fg	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_rift              );
DECLARE_SPELL_FUN(	spell_rip               );
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_sanctify 		);
DECLARE_SPELL_FUN(	spell_ethereal 		);
DECLARE_SPELL_FUN(	spell_astral   		);
DECLARE_SPELL_FUN(	spell_induction   	);
DECLARE_SPELL_FUN(	spell_enhanced_rest	);
DECLARE_SPELL_FUN(	spell_enhanced_heal	);
DECLARE_SPELL_FUN(	spell_enhanced_revive	);
DECLARE_SPELL_FUN(	spell_animate_dead	);
DECLARE_SPELL_FUN(	spell_banish		);
DECLARE_SPELL_FUN(	spell_mage_shield	);
DECLARE_SPELL_FUN(	spell_enhance_object	);
DECLARE_SPELL_FUN(	spell_dispel_undead	);
DECLARE_SPELL_FUN(	spell_dispel_good	);
DECLARE_SPELL_FUN(	spell_remove_fear	);
DECLARE_SPELL_FUN(	spell_feast	        );
DECLARE_SPELL_FUN(	spell_restore	        );
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_write_spell       );
DECLARE_SPELL_FUN(	spell_stability         );
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_block_area        );
DECLARE_SPELL_FUN(     	spell_demon             );
DECLARE_SPELL_FUN(      spell_beast             );
DECLARE_SPELL_FUN(      spell_shade             );
DECLARE_SPELL_FUN(      spell_phantasm          );
DECLARE_SPELL_FUN(      spell_tongues           );
DECLARE_SPELL_FUN(      spell_understand        );
DECLARE_SPELL_FUN(      spell_illusion          );
DECLARE_SPELL_FUN(      spell_mirror_image      );
DECLARE_SPELL_FUN(      spell_hallucinate       );
DECLARE_SPELL_FUN(      spell_breath_water      );
DECLARE_SPELL_FUN(      spell_mage_blast        );
DECLARE_SPELL_FUN(      spell_confusion         );

DECLARE_SPELL_FUN(	spell_benediction	);
DECLARE_SPELL_FUN(	spell_righteous_fury	);
DECLARE_SPELL_FUN(	spell_soothing_touch	);
DECLARE_SPELL_FUN(	spell_farheal		);
DECLARE_SPELL_FUN(	spell_farrevive		);
DECLARE_SPELL_FUN(	spell_holy_word		);
DECLARE_SPELL_FUN(	spell_unholy_word	);
DECLARE_SPELL_FUN(	spell_invigorate	);

DECLARE_SPELL_FUN( 	spell_improved_invis	);
DECLARE_SPELL_FUN(	spell_truesight		);
DECLARE_SPELL_FUN(	spell_hallucinatory_terrain);
DECLARE_SPELL_FUN(	spell_nightmare		);
DECLARE_SPELL_FUN(	spell_smoke		);

DECLARE_SPELL_FUN( 	spell_recharge		);
DECLARE_SPELL_FUN(	spell_transport		);
DECLARE_SPELL_FUN(	spell_anti_magic_shell	);
DECLARE_SPELL_FUN( 	spell_possess		);
DECLARE_SPELL_FUN( 	spell_vampiric_touch	);

DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(	spell_elemental		);
DECLARE_SPELL_FUN( 	spell_unbarring_ways	);
DECLARE_SPELL_FUN( 	spell_brew_potion	);
DECLARE_SPELL_FUN( 	spell_fire_shield	);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if 	defined(SYSV)
size_t 	fread		args( ( void *ptr, size_t size, size_t n, 
				FILE *stream ) );
#else
/* int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );*/
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""		/* Player files			*/
#define NULL_FILE	"proto.are"	/* To reserve one stream	*/
#endif

#if defined(MSDOS)
#define PLAYER_DIR	""		/* Player files                 */
#define NULL_FILE	"nul"		/* To reserve one stream	*/
#endif

#if defined(unix)
#define PLAYER_DIR	"../player"	/* Player files			*/
#define CLAN_DIR	"../clans"	/* Clan files			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define MOB_DIR		"MOBprogs/"	/* Mobile files			*/
#endif

#define AREA_LIST	"area.lst"	/* List of areas		*/
#define CLAN_LIST	"clan.lst"	/* List of clans		*/
#define CLAN_LIST_TMP	"clan.tmp"	/* List of clans		*/
#define OWNER_LIST	"owner.lst"	/* List of clans		*/
#define OWNER_LIST_TMP	"owner.tmp"	/* List of clans		*/

#define BUG_FILE_T	"gamebugs.txt"      /* For bug( )		*/
#define USER_BUG_FILE_T "bugs.txt"      /* For 'bug' */
#define IDEA_FILE_T	"ideas.txt"	/* For 'idea'			*/
#define TYPO_FILE_T	"typos.txt"     /* For 'typo'			*/
#define NOTE_FILE_T  	"notes.txt"	/* For 'notes'			*/
#define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/
#define COPYOVER_FILE   "copyover.dat"  /* for warm reboots             */
#define TEST_FILE        "../bin/md"     /* executable path              */
#define REAL_FILE        "../bin/current_md" /* executable path              */
#define SWEAR_FILE_T    "swear.txt"      /* Has all the cuss words in it */
#define BOUNTY_FILE     "bounty.txt"     /* Stores the bounties */
#define BOUNTY_FILE_TMP "bounty.tmp"     /* Stores the bounties */
#define VICTORY_LIST    "victory.txt"/* Keep track of killers through reboots */
#define VICTORY_LIST_TMP  "victory.tmp"


FILE *BUG_FILE;   /* Initialize in db.c */
FILE *USER_BUG_FILE;
FILE *IDEA_FILE;
FILE *TYPO_FILE;
FILE *NOTE_FILE;

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define RD	RESET_DATA
#define SF	SPEC_FUN
#define OF	OBJ_FUN

/* act_comm.c */
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
void	add_shadow	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_shadow	args( ( CHAR_DATA *ch ) );
void	die_shadow	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
bool	vnum_in_group	args( ( CHAR_DATA *ach, int mobvnum ) );
bool    pvnum_in_group  args( ( CHAR_DATA *ch, int pvnum ) );
void    do_battle       args( ( char * argument ) );
void	vt100on		args( ( CHAR_DATA *) );
void 	vt100off	args( ( CHAR_DATA *) );
void 	vt100prompt	args( ( CHAR_DATA *) );
void 	vt100prompter	args( ( CHAR_DATA *) );
CD * 	start_partial_load args( ( CHAR_DATA *ch, char *argument) );
void 	clear_partial_load args( ( CHAR_DATA *ch ) );


/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
HELP_DATA *get_help     args( ( CHAR_DATA *ch, char *argument ) );


/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door ) );
bool    can_move_char(  CHAR_DATA *, int );
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );


/* act_obj.c */
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
OBJ_DATA * find_char_corpse (CHAR_DATA *, bool );
int	give_gold	args( ( CHAR_DATA *ch, int amount ) );
int     gold_transaction args( ( CHAR_DATA *ch, int amount ) );
void    post_bounty     args( ( char *name, int amount ) );
void    sort_bounty     args( ( BOUNTY_DATA * bptr) );
void    remove_bounty   args( ( BOUNTY_DATA * bptr) );
BOUNTY_DATA *     get_bounty      args( ( char *name ) );


/* act_wiz.c */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
void    auto_release    args( ( CHAR_DATA *ch ) );

/* castle.c */
bool get_bitvector_value args( (char *name,int *number,char *allowed) );
void list_bitvectors     args( (CHAR_DATA *ch,char *prefix) );
void clear_castles       args( ( void ) );

/* comm.c */
void	close_socket	args( ( DESCRIPTOR_DATA *dclose , bool Force) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_char_color	args( ( char *txt, CHAR_DATA *ch ) );
void	ship_to_char	args( ( const char *txt, CHAR_DATA *ch, int leng) );
void	ship_to_char_color	args( ( char *txt, CHAR_DATA *ch, int leng) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void    log_printf      args( ( char *fmt, ...) );
void    ch_printf       args( ( CHAR_DATA *ch, char *fmt, ... ) );
void    ch_printf_color args( ( CHAR_DATA *ch, char *fmt, ... ) );




/* db.c */
void	boot_db		args( ( bool fCopyOver ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void    copyover_recover        args( (void) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
RID *	get_room_index_old	args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *	fread_string_nohash	args( ( FILE *fp ) );
char *	fread_string_max      ( FILE *fp , int );
void	fread_to_eol	args( ( FILE *fp ) );
char *  fread_line      args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
char	qread_letter	args( ( void ) );
int	qread_number	args( ( void ) );
char *	qread_string	args( ( void ) );
void	qread_to_eol	args( ( void ) );
char *	qread_word	args( ( void ) );
char *	str_dup		( const char * );
char *	str_dup_max     ( const char *, int );
int     str_cat_max     ( const char *, const char *, int );
int     str_apd_max     ( const char *, const char *, int, int );
int     char_apd_max    ( const char *, const char *, int, int );
int     str_cpy_max     ( const char *, const char *, int );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
/* bool	str_cmp		args( ( const char *astr, const char *bstr ) ); */
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *  strlower        args( ( const char *str ) );
char *  strupper        args( ( const char *str ) );
char *str_resize  args( (const char *st,char *buf,int length) );
char *	capitalize	args( ( const char *str ) );
char *	capitalize_name	args( ( const char *str ) );
void	append_file	( CHAR_DATA *, char *, FILE *, char * );
void    bug             args( ( const char *str, ... ) );
void	log_string	args( ( char *str ) );
void	tail_chain	args( ( void ) );
void 	load_owners	args( ( void ) );
void 	save_owners	args( ( void ) );
bool    delete_room     args( ( ROOM_INDEX_DATA *room ) );
bool    delete_obj      args( ( OBJ_INDEX_DATA *obj ) );
bool    delete_mob      args( ( MOB_INDEX_DATA *mob ) );
RID *   make_room       args( ( int vnum ) );
OID *   make_object     args( ( int vnum, int cvnum, char *name ) );
MID *   make_mobile     args( ( int vnum, int cvnum, char *name ) );
EXIT_DATA  *   make_exit       args( ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door ) );

void *  alloc_mem       args( ( int sMem ) );
void    free_mem        args( ( void *pAdd, int sMem ) );
void  	add_char	args( ( CHAR_DATA *ch ) );
char *	UPPER_ALLOC	args( ( char *str) );
char *	LOWER_ALLOC	args( ( char *str) );
void 	load_sites	args( ( void ) );
void 	save_sites	args( ( void ) );
void 	save_victors	args( ( void ) );
void 	load_victors	args( ( void ) );
void    load_bounties   args( ( void ) );
void    save_bounties   args( ( void ) );
AREA_DATA * get_area_from_vnum args ( ( int vnum, int type ) );
void    sort_area_by_name  args( (AREA_DATA *pArea) ); 



/* edit.c */
void    start_editing   args( ( CHAR_DATA *ch, char *data ) );
void    stop_editing    args( ( CHAR_DATA *ch ) );
void    edit_buffer     args( ( CHAR_DATA *ch, char *argument ) );
char *  copy_buffer     args( ( CHAR_DATA *ch ) );
void    assign_area     args( ( CHAR_DATA *ch ) );
char *  sprint_reset    args( ( CHAR_DATA *ch, RESET_DATA *pReset, sh_int num, bool rlist ) );
int 	get_dir 	args( ( char *txt) );
bool    can_rmodify     args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );
bool    can_omodify     args( ( CHAR_DATA *ch, OBJ_DATA *obj  ) );
bool    can_mmodify     args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
bool    can_medit       args( ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) );
char *  flag_string     args( ( int bitvector, char * const flagarray[] ) );


/* reset.c */
RD  *   make_reset      args( ( char letter, int arg1, int arg2, int arg3 ) );
RD  *   add_reset       args( ( AREA_DATA *tarea, char letter, int arg1, int arg2, int arg3 ) );
RD  *   place_reset     args( ( AREA_DATA *tarea, char letter, int arg1, int arg2, int arg3 ) );
void    reset_area      args( ( AREA_DATA * pArea ) );




/* hashstr.c */
int     quick_str_size  args( ( char *str ) );
char *  str_alloc       args( ( char *str , int str_type) );
char *  str_alloc_perm  args( ( char *str ) );
char *  str_alloc_single  args( ( char *str ) );
char *  quick_link      args( ( char *str ) );
int     str_free        args( ( char *str ) );
void    show_hash       args( ( int count ) );
char *  hash_stats      args( ( char *argument) );
char *  check_hash      args( ( char *str ) );
void    hash_dump       args( ( int hash ) );
void    show_high_hash  args( ( int top ) );

/* fight.c */
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	damage_hurt	( CHAR_DATA *, CHAR_DATA *, int , int );
void	damage		( CHAR_DATA *, CHAR_DATA *, int , int );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
int     getDirNumber    args( ( char *dirName ) );
bool    check_hit       args( ( CHAR_DATA *, CHAR_DATA *, int, int ) );
bool    can_attack      args( ( CHAR_DATA *, CHAR_DATA *) );
void    stop_hunting    args( ( CHAR_DATA *ch ) );
void    stop_hating     args( ( CHAR_DATA *ch ) );
void    stop_fearing    args( ( CHAR_DATA *ch ) );
void    start_hunting   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    start_hating    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    start_fearing   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_hunting      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_hating       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_fearing      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    found_prey      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    free_fight      args( ( CHAR_DATA *ch ) );
void    add_kill        args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
CD *    who_fighting    args( ( CHAR_DATA *ch ) );





/* handler.c */
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_age		args( ( CHAR_DATA *ch ) );
int	get_curr_str	args( ( CHAR_DATA *ch ) );
int	get_curr_int	args( ( CHAR_DATA *ch ) );
int	get_curr_wis	args( ( CHAR_DATA *ch ) );
int	get_curr_dex	args( ( CHAR_DATA *ch ) );
int	get_curr_con	args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_name_short	args( ( const char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected_external	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
int	count_mob_list	args( ( MOB_INDEX_DATA *obj, OBJ_DATA *list ) );
int     char_exists     args( ( char *arg ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
void    extract_exit    args( ( ROOM_INDEX_DATA *room, EXIT_DATA * pexit, int door ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_room_even_hidden	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_room_even_hidden_vnum	args( ( CHAR_DATA *ch, int vnum ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_player_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	( OBJ_INDEX_DATA * , ROOM_INDEX_DATA *);
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_list_vnum	args( ( CHAR_DATA *ch, int vnum, OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_carry_keeper	args( ( CHAR_DATA *keeper, char *argument, CHAR_DATA *ch, int bargain ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear_vnum	args( ( CHAR_DATA *ch, int vnum ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int amount ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_hear	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
void    char_reset      args( (CD *ch) );
sh_int  obj_level_estimate args( (OID *objIndex) );
void    invalidate_obj_copies args( (OD *list,int level) );
bool    blocking        args( (CD *victim,CD *ch) );
bool    login_allowed   args( (DESCRIPTOR_DATA *d,CHAR_DATA *ch) );
bool 	can_use_exit    args( (CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir) );
bool 	nifty_is_name   args( ( char *str, char *namelist ) );


/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argue ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	one_argument_nolower	args( ( char *argument, char *arg_first ) );
void    censorstring    args( ( char *input_string ) );

/* magic.c */
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *ch, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* mob_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif

void    mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, OBJ_DATA* object,
					void* vo, int type ) );
void    mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
					OBJ_DATA* object, void* vo,
					int type ) );
void    mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
		                        CHAR_DATA* ch, OBJ_DATA* obj,
					void* vo ) );
void    mprog_social_trigger    ( char*, CHAR_DATA* , CHAR_DATA*);
void    mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
		                        int amount ) );
void    mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                		        OBJ_DATA* obj ) );
void    mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void 	mprog_range_trigger     args ( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
void    mprog_kill_trigger      args ( ( CHAR_DATA* mob  ) );
void    mprog_time_trigger      args ( ( CHAR_DATA *mob ) );


/* save.c */
void *threaded_save_char args ( (void * args) );

void	save_char_obj	args( ( CHAR_DATA *ch, int which_type ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *  spec_name_lookup args( ( SPEC_FUN *fun ) );
OF *    obj_lookup      args( ( const char *name ) );
char *  obj_name_lookup args( ( OBJ_FUN *fun ) );
OF *	obj_fun_lookup	args( ( const char *name ) );
void	obj_get_cmds    args( () );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool fSave ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );

/* castle.c */
void    del_castle      args( (CHAR_DATA *victim));
extern bool castle_needs_saving;

/* clans.c */

CLAN_DATA * get_clan	args( ( char *name ) );
CLAN_DATA *get_clan_from_vnum args ( ( int vnum ) );
void 	load_clans	args( ( void ) );
void 	save_all_clans	args( ( void ) );
void 	save_clan	args( ( CLAN_DATA *clan ) );
void 	destroy_clan	args( ( CLAN_DATA *clan ) );
void 	send_clan_message args( (CLAN_DATA *clan, char *message) );



#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF

int exp_level(int,int);
int multi( CHAR_DATA *, int);
int multi_pick( CHAR_DATA *, int);
void process_alias( CHAR_DATA *, int, char *);
void insert_arg_alias( CHAR_DATA *, char *, char *);
int get_pets( CHAR_DATA *);
void roll_race( CHAR_DATA *);
#define SET_SHIFT( val, bit)  ((1)<<(bit)|(val))
int UNSHIFT( int );
#define SHIFT( bit)       ((1)<<(bit))
void add_language( CHAR_DATA *);


BAN_DATA    *ban_list;
void raw_kill( CHAR_DATA *);
void show_char_to_char_1( CHAR_DATA *, CHAR_DATA *);
int which_god( CHAR_DATA *);
int amount_note( CHAR_DATA *);

bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace, bool fDisplay ) );
bool    merc_reboot;

#define MAXPORTS       1
#define MAXLINKPERPORT 200
#define LINKS_PER_HOST 5      /* The normal maximum of players from a site */
CHAR_DATA *scan_mob( RESET_DATA *);
bool scan_obj( RESET_DATA *);
void force_help( DESCRIPTOR_DATA *, char *);
bool write_to_descriptor args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
void write_to_port args( ( DESCRIPTOR_DATA *d ) );
bool is_desc_valid( CHAR_DATA *);

int Current_pvnum;    /*  loaded in first line of the block file  */


void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void reset_color( CHAR_DATA *);

CHAR_DATA *get_pvnum_index( int );
void leave_fighting( CHAR_DATA *,  ROOM_INDEX_DATA *);
char *pick_one( char *, char *, char *, char *);
CHAR_DATA *	find_keeper( CHAR_DATA * );
int mob_armor ( CHAR_DATA * );
bool IS_BOOTING;
char FILE_MODE;    /* Status flag for loading files */
ROOM_INDEX_DATA  *room_index[MAX_VNUM];/* Variables for all rooms - 256k RAM */
MOB_INDEX_DATA   *mob_index[MAX_VNUM]; /* Variables for all mobs  - 256k RAM */
OBJ_INDEX_DATA   *obj_index[MAX_VNUM]; /* Variables for all objs  - 256k RAM */
int mazegen( CHAR_DATA *, int , int , int , int , int );
  
void add_player( CHAR_DATA *);
void sub_player( CHAR_DATA *);
bool should_fight( CHAR_DATA *, CHAR_DATA *);
bool is_exit( int, int );
void set_exit( int, int, int );
int get_room( int, int, int);
int reverse( int );  /* returns opposite direction exit */
int direction_door( char *);  /* Returns door# from direction text */
int total_objects, total_characters, total_mobiles;
CHAR_DATA *lookup_char( char *);

#define SAVE_DELAY 5          /*  Delay in minutes of autosave  */

int total_io_bytes;  /* total bytes written */
int total_io_ticks;  /* total PULSE_PER_SECOND since start */
int total_io_delay;  /* total time out of program */
int total_io_exec;   /* total time executing program */

bool is_spell( int );
int get_mana( CHAR_DATA *, int);

int     desc_alloc_size; /* calculated descriptor size of allocations */
int     char_alloc_size;

bool FINGER_MODE;  /* changes method of some commands */
bool is_char_valid( CHAR_DATA *); /* scans all players for active */

bool is_enchanted_obj( OBJ_DATA *);
int  get_game_usec( void );

void send_1_byte( CHAR_DATA *, int , sh_int ); /* Use for direct data */
void send_2_byte( CHAR_DATA *, int , sh_int ); /* processing systems  */
void send_4_byte( CHAR_DATA *, int , int );
void send_string( CHAR_DATA *, int , char *);
void send_packet( CHAR_DATA *, int , char *); /* Common packet routine */
  
int compare_obj( OBJ_DATA *, OBJ_DATA *); /* difference of strength if same */

int wear_flags_to_wear_loc( OBJ_DATA *, int ); /* type conversion */
int count_total_objects( OBJ_DATA *);/* determines total objects in cont */
int get_game_realtime( void );

bool can_see_in_room( CHAR_DATA *, ROOM_INDEX_DATA *);

void start_object_program ( CHAR_DATA *, OBJ_DATA *, OBJ_PROG *, char *);
void object_program ( CHAR_DATA *, OBJ_DATA *, OBJ_PROG *, char *);

/* mob_commands.c */
char *			mprog_type_to_name	args( ( int type ) );

bool TEST_GAME;  

const char justified_buf[MAX_STRING_LENGTH];
char *justify( char *);
bool can_understand( CHAR_DATA *, CHAR_DATA *);
void transference( CHAR_DATA *, ROOM_INDEX_DATA * );

    /* Timer Code  -   Chaos  1/6/95    */
void open_timer( int );
void close_timer( int );
int display_timer( CHAR_DATA *ch, int );

#define TIMER_UPDATE 0
#define TIMER_CHAR_UPD 1
#define TIMER_MOB_UPD 2
#define TIMER_OBJ_UPD 3
#define TIMER_SHOP_UPD 4
#define TIMER_AGGR_UPD 5
#define TIMER_VIOL_UPD 6
#define TIMER_AREA_UPD 7
#define TIMER_CHAR_SAVE 8
#define TIMER_CHAR_LOAD 9
#define TIMER_INTERP 10
#define TIMER_CASTLE_SAVE 11
#define TIMER_HIT_GAIN 12
#define TIMER_GET_HOST 13
#define TIMER_MOB_PROG 14
#define TIMER_OBJ_PROG 15
#define TIMER_WRITE_DESC 16
#define TIMER_READ_DESC 17
#define TIMER_SCAN_DESC 18
#define TIMER_WEATHER 19
#define TIMER_SECTOR 20
#define TIMER_WRITE_PAGER 21
#define TIMER_WRITE_SCROLL 22
#define TIMER_WRITE_APPEND 23
#define TIMER_WRITE_PROCESS 24
#define TIMER_WRITE_PROCESS_STR 25
#define TIMER_WRITE_PROCESS_CMP 26
#define TIMER_WRITE_PROCESS_UPD 27
#define TIMER_CLAN_SAVE 28
#define TIMER_ALLOC 29
#define TIMER_FREE 30

#define MAX_TIMERS 31

int timers[ MAX_TIMERS ][5];
extern const char timer_strings[ MAX_TIMERS ][80] ;

int get_max_speed( CHAR_DATA *ch );  /* Determine fastest rate of travel */

bool is_in_room( CHAR_DATA *, ROOM_INDEX_DATA *);
bool in_camp(CHAR_DATA *ch);

const char get_color_string_buf[50];  /* The buffer */
char *get_color_string( CHAR_DATA * , int, int ); /* Returns ANSI color codes */
     /* The ints are 'COLOR_' codes, and 'VT102_' codes  */
#define COLOR_NO_CHANGE     -1     /*  Not usually supported with VT102_DIM  */
#define COLOR_TEXT           0
#define COLOR_TOP_STAT       1
#define COLOR_BOT_STAT       2
#define COLOR_SCORE          3
#define COLOR_YOU_ARE_HIT    4
#define COLOR_YOU_HIT        5
#define COLOR_PROMPT         6
#define COLOR_EXITS          7
#define COLOR_PARTY_HIT      8
#define COLOR_SPEACH         9
#define COLOR_OBJECTS       10
#define COLOR_MOBILES       11
#define COLOR_TACTICAL      12
#define COLOR_TACT_PARTY    13
#define COLOR_TACT_ENEMY    14
#define COLOR_TACT_NEUTRAL  15
#define COLOR_MAX           16

#define VT102_DIM            0
#define VT102_BOLD           1
#define VT102_REVERSE        4
#define VT102_UNDERLINE      5
#define VT102_FLASHING       7

TACTICAL_MAP *get_tactical_map( CHAR_DATA * );
char *get_tactical_string( CHAR_DATA *, TACTICAL_MAP *);
const char  get_tactical_string_string[MAX_STRING_LENGTH];
void clear_tactical_map( CHAR_DATA * );
TACTICAL_MAP *get_diff_tactical( CHAR_DATA * );



/* extern void fflush( FILE * ); */
extern void rewind( FILE * );
/* extern void system( char *); */

#define RECLOSE_FILES TRUE

char *get_color_diff( CHAR_DATA *, int, int, bool, int, int, bool);
char *get_color_diff_string[40];

void memory_dump( char *);
int  memory_hits( char *);

const char vt_command_0_string[ 40 ];
const char vt_command_1_string[ 40 ];
const char vt_command_2_string[ 40 ];
const char vt_command_3_string[ 40 ];

char *vt_command_0( CHAR_DATA *, char );
char *vt_command_1( CHAR_DATA *, int, char );
char *vt_command_2( CHAR_DATA *, int, int, char );
char *vt_command_3( CHAR_DATA *, int, int, int, char );

bool is_valid_password( char * );

TACTICAL_MAP *cont_tact;

int get_sector_color ( CHAR_DATA *, int );

bool ALLOW_OUTPUT;
bool DoingLookup;
/* int strcasecmp( char *, char *);  External command */

      /*   Command table values  */
sh_int cmd_kill;  
sh_int cmd_stop;
sh_int cmd_continuous;

char *ansi_translate_text( CHAR_DATA *, char *);
char ansi_translate_buffer[MAX_STRING_LENGTH];

int count_obj_affected( OBJ_DATA *, int, int );

char *get_name( CHAR_DATA *);
char get_name_buffer[MAX_INPUT_LENGTH];
bool    check_social_fast  ( CHAR_DATA *, int , char * );
CHAR_DATA *test_char;
void knight_adjust_hpmnmv( CHAR_DATA * );
void send_to_combat_char( char *, CHAR_DATA *);

void check_most( CHAR_DATA * );


#define MOST_EXP         0
#define MOST_EXP_LOST    1
#define MOST_REINCARNATE 2
#define MOST_KILL_EN     3
#define MOST_KILL_PC     4
#define MOST_KILL_BY_NPC 5
#define MOST_KILL_BY_EN  6
#define MOST_KILL_BY_PC  7
#define MOST_HP          8
#define MOST_MANA        9
#define MOST_MOVE       10
#define MOST_AC         11
#define MOST_DR         12
#define MOST_HR         13
#define MOST_CASTLE     14
#define MOST_HOURS      15
#define MOST_SAVE       16
#define MOST_MOST       17

char Most_Names[MOST_MOST][30];
int Most_Values[MOST_MOST];

#define TYPE_MOBILE 1
#define TYPE_OBJECT 2
#define TYPE_ROOM   3

bool REAL_GAME;
int character_expiration( CHAR_DATA *);
bool can_see_is_hidden;
CHAR_DATA *last_dead;

#define VICTORY_LIST_SIZE 25
char *victory_list[VICTORY_LIST_SIZE];
int   victory_list_current;
int   army_HQ[MAX_RACE];

bool	check_parse_name	( char * , bool );
bool load_error;
void create_object_reference( OBJ_DATA * );
OBJ_REFERENCE *object_reference_hash[1000];
void add_to_object_reference_hash( OBJ_DATA *);
void remove_from_object_reference_hash( OBJ_DATA *);
bool GLOBAL_SOCIAL_FLAG;
bool check_race_war( CHAR_DATA *, CHAR_DATA *);
sh_int Race_Battle_List[MAX_RACE];
void mix_race_war( void );
int GET_DAMROLL( CHAR_DATA *);
int GET_HITROLL( CHAR_DATA *);
int GET_SAVING_THROW( CHAR_DATA *);
bool equipment_affecting;

    /* Presto 8/3/98 */
#define NORMAL_SAVE 0
#define BACKUP_SAVE 1

    /* Chaos 11/22/97  */
AREA_DATA *clan_helps;

#define CLAN_UNDECIDED 0
#define CLAN_PEACEFUL  1
#define CLAN_WARLIKE   2

void Update_MrTerm_Stats( CHAR_DATA *);
int MrTerm_number_append( char *, int, int, int);
void add_to_clan( CLAN_DATA *, CLAN_DATA *);
void wipe_string( char *);
int rift_index;
bool DISALLOW_SNOOP;
int total_rips;
bool dual_flip;
bool can_reincarnate_attack( CHAR_DATA *, CHAR_DATA *);

#define CREATE(result, type, number)                            \
do                                                              \
{                                                               \
    if (number<=1)						\
      result = alloc_mem(sizeof(*result));			\
    else							\
     if ((sizeof(type))>0)					\
      result = alloc_mem(number* (sizeof(type)));		\
     else							\
      result = alloc_mem(number);				\
} while(0)

/*
#define DISPOSE(point)                                          \
do                                                              \
{                                                               \
  free_mem(&point,0); 					\
} while(0)
*/

void DISPOSE( void * );

/* 
#define CREATE(result, type, number)                            \
do                                                              \
{                                                               \
    if (!((result) = (type *) calloc ((number), sizeof(type)))) \
    {                                                           \
        perror("malloc failure");                               \
        fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
        abort();                                                \
    }                                                           \
} while(0)

#define RECREATE(result,type,number)                            \
do                                                              \
{                                                               \
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {                                                           \
        perror("realloc failure");                              \
        fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
        abort();                                                \
    }                                                           \
} while(0)

#define DISPOSE(point)                                          \
do                                                              \
{                                                               \
  open_timer(TIMER_FREE);					\
  if (!(point))                                                 \
  {                                                             \
        bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
        fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }                                                             \
  else free(point);						\
  point = NULL;                                               \
  close_timer(TIMER_FREE);					\
} while(0)

*/

int get_string_size( unsigned char * );
int get_alloc_size_perm( unsigned char * );
char *str_empty;

unsigned char *set_block_header( unsigned char, char, int, unsigned char *);
void set_block_type( char, unsigned char *);
struct hashstr_data
{
    struct hashstr_data	*next;		/* next hash element */
    struct hashstr_data	*prev;		/* prev hash element */
    unsigned short int	 links;		/* number of links to this string */
    unsigned short int	 length;	/* length of string */
};

int total_memory_warnings;

#define STRALLOC(point)         str_alloc((point), 0)
#define QUICKLINK(point)        quick_link((point))
#define QUICKMATCH(p1, p2)      (int) (p1) == (int) (p2)
#define STRFREE(point)                                          \
do                                                              \
{                                                               \
  if (str_free((point))==-1)                               \
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
  point = NULL;                                                 \
} while(0)
/*
Took this out ...too many damn lists expect new links at the end.
Martin 7/2/99
#define LINK(link, first, last, next, prev)                     \
do                                                              \
{                                                               \
    if ( !(first) )                                             \
      (last)                   = (link);                        \
    else							\
      (first)->prev           = (link);                    \
    (link)->next              = (first);                      \
    (link)->prev              = NULL;                         \
    (first)                     = (link);                      \
} while(0)
*/
#define LINK(link, first, last, next, prev)                     \
do                                                              \
{                                                               \
    if ( !(first) )                                             \
      (first)                   = (link);                       \
    else                                                        \
      (last)->next              = (link);                       \
    (link)->next                = NULL;                         \
    (link)->prev                = (last);                       \
    (last)                      = (link);                       \
} while(0)


#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                 = (insert)->prev;           \
    if ( !(insert)->prev )                                     \
      (first)                     = (link);                     \
    else                                                        \
      (insert)->prev->next    = (link);                     \
    (insert)->prev              = (link);                     \
    (link)->next                = (insert);                   \
} while(0)

#define UNLINKCHECK

#define UNLINK(link, first, last, next, prev)                 \
do                                                            \
{                                                             \
    if ( !(link)->prev )                                      \
      (first)                   = (link)->next;               \
    else                                                      \
      (link)->prev->next    	= (link)->next;               \
    if ( !(link)->next )                                       \
      (last)                    = (link)->prev;               \
    else                                                      \
      (link)->next->prev    	= (link)->prev;               \
} while(0)

#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        (first)->name );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        (first)->name);					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            (first)->name, ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            (first)->name, ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            (first)->name, ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            (first)->name, ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)

void set_quest_bits( unsigned char **,unsigned int, unsigned int, unsigned int);
int get_quest_bits( unsigned char *, unsigned int, unsigned int);
char *quest_bits_to_string( unsigned char *);
char quest_bits_to_out[100];
bool is_quest( unsigned char *);
void get_string_score_v1( CHAR_DATA *, CHAR_DATA *);
char get_string_score_txt[MAX_STRING_LENGTH];
char *get_bar_graph( CHAR_DATA *, int, int);
char get_bar_graph_txt[MAX_INPUT_LENGTH];
void ListCheck(void);
  CHAR_DATA *violence_rch_next;
  CHAR_DATA *violence_ch_next;
  CHAR_DATA *aggr_ch_next;
  CHAR_DATA *aggr_wch_next;
RESET_DATA *get_reset_from_obj( int, RESET_DATA *, RESET_DATA * );
RESET_DATA *get_reset_from_mob( RESET_DATA *, RESET_DATA * );
void sort_alias( CHAR_DATA *);
bool str_contains( char *, char *);
char *str_replace( char *, char *, char *);
char str_replace_txt[MAX_STRING_LENGTH];
void damage_equipment( CHAR_DATA *, bool);
bool is_object_reference( OBJ_DATA *, char *);
void get_attack_string( CHAR_DATA *, CHAR_DATA *, char *);
bool is_valid_save( char *, char *);
int initiate_hp_loss( int, int );    /* Returns hp loss */
int initiate_mana_loss( int, int );  /* Returns mana loss */
int initiate_move_loss( int, int );  /* Returns move loss */
CASTLE_DATA *get_castle_data( CHAR_DATA *);
CASTLE_DATA get_castle_data_data;
