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

#pragma hdrstop
#pragma argsused
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if defined( unix )
#include <strings.h>
#else
#include <conio.h>
#include <condefs.h>
#endif

typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct bitvector_type BITVECTOR_DATA;
typedef struct affect_data AFFECT_DATA;
typedef struct est_level_data EST_DATA;
typedef char buffers[32];

#define MAX_VNUMS 1024

int logs[8];			/* Log base 2 list */

int room_vnum[MAX_VNUMS];
int exit_data[MAX_VNUMS];
int total_room;
int obj_vnum[MAX_VNUMS];
int total_obj;
int mob_vnum[MAX_VNUMS];
int total_mob;

/*
 * An affect.
 */
struct affect_data
  {
    AFFECT_DATA *next;
    int type;
    int duration;
    int location;
    int modifier;
    int bitvector;
  };

/*
 * Prototype for an object.
 */
struct obj_index_data
  {
    AFFECT_DATA *affected;
    int vnum;
    int item_type;
    int extra_flags;
    int wear_flags;
    int weight;
    int cost;
    int value[4];
    int level_rent;
    int level;
  };



struct bitvector_type		/* text of particular body parts */
  {
    char *name;			/* name of bit */
    int value;
    BITVECTOR_DATA *next;
  };
BITVECTOR_DATA *bitvector_list;

struct est_level_data
  {
    int truelevel;
    int negatives;
    int positives;
    int estlevel;
  };

/*
 * Globals.
 */

#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#define MAX_INPUT_LENGTH 20000
#define MAX_STRING_LENGTH 80000

#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_MANA                   12
#define APPLY_HIT                    13
#define APPLY_MOVE                   14
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24

#define ITEM_LIGHT               1
#define ITEM_SCROLL              2
#define ITEM_WAND                3
#define ITEM_STAFF               4
#define ITEM_WEAPON              5
#define ITEM_TREASURE            8
#define ITEM_ARMOR               9
#define ITEM_POTION             10
#define ITEM_FURNITURE          12
#define ITEM_TRASH              13
#define ITEM_CONTAINER          15
#define ITEM_DRINK_CON          17
#define ITEM_KEY                18
#define ITEM_FOOD               19
#define ITEM_MONEY              20
#define ITEM_BOAT               22
#define ITEM_FOUNTAIN           25
#define ITEM_PILL               26
#define ITEM_AMMO               30

#define ITEM_GLOW                1
#define ITEM_HUM                 2
#define ITEM_DARK                4
#define ITEM_LOCK                8
#define ITEM_EVIL               16
#define ITEM_INVIS              32
#define ITEM_MAGIC              64
#define ITEM_NODROP            128
#define ITEM_ANTI_GOOD         512
#define ITEM_ANTI_EVIL        1024
#define ITEM_ANTI_NEUTRAL     2048
#define ITEM_NOREMOVE         4096
#define ITEM_INVENTORY        8192
#define ITEM_LEVEL           16384
#define ITEM_AUTO_ENGRAVE    65536

#define ITEM_WEAR_TAKE                1
#define ITEM_WEAR_FINGER              2
#define ITEM_WEAR_NECK                4
#define ITEM_WEAR_BODY                8
#define ITEM_WEAR_HEAD               16
#define ITEM_WEAR_LEGS               32
#define ITEM_WEAR_FEET               64
#define ITEM_WEAR_HANDS             128
#define ITEM_WEAR_ARMS              256
#define ITEM_WEAR_SHIELD            512
#define ITEM_WEAR_ABOUT            1024
#define ITEM_WEAR_WAIST            2048
#define ITEM_WEAR_WRIST            4096
#define ITEM_WEAR_WIELD            8192
#define ITEM_WEAR_HOLD            16384


char fread_string_buf[MAX_STRING_LENGTH];
char *bufx;
char word[MAX_INPUT_LENGTH];

EST_DATA obj_level_estimate (OBJ_INDEX_DATA *);


void bug (FILE *);
/*
 * Semi-locals.
 */
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];

void fread_to_eol (FILE *);
char fread_letter (FILE *);
int fread_number (FILE *);
char *fread_word (FILE *);
char *fread_string (FILE *);
void load_object_program (FILE *,int vnum);

/*
 * Local booting procedures.
 */

void load_area (FILE * fp);
void load_helps (FILE * fp);
void load_mobiles (FILE * fp);
void load_objects (FILE * fp);
void load_resets (FILE * fp);
void load_rooms (FILE * fp);
void load_shops (FILE * fp);
void load_specials (FILE * fp);
void load_sites (void);
void spew_out (char *fmt,...);

int quest_items = 0, lineno = 1;
int nummobcon=0,numobjcon=0;
int areavnums[32],totalareas=0;
int badexits[256][2],numbad=0;
buffers badmobconnects[256];
buffers badobjconnects[256];





/*
 * MOBprogram locals
 */

void mprog_read_programs (FILE * fp, int vnum);

char areafile[200];

/* Put together by David Bills (Chaos of Mortal Realms)
   IQ improved immensely by Martin Gallwey (Chaste of Mortal Realms) */
int
main (int argc, char **argv)
{
  int i,j,external=0;
  total_obj = 0;
  total_room = 0;
  total_mob = 0;

  logs[0] = 1;
  logs[1] = 2;
  logs[2] = 4;
  logs[3] = 8;
  logs[4] = 16;
  logs[5] = 32;
  logs[6] = 64;
  logs[7] = 128;

  spew_out ("Area Syntax Checker v3.3        MrMud v1.3\n");

  if (argc != 2)
    {
      spew_out ("Format:\n  areachk <area file>\n");
      exit (0);
    }
  sscanf (argv[1], "%s", areafile);

   for (i=0;i<10;i++)
    areavnums[i]=0;

  load_sites ();
  {
    FILE *fpArea;

    if ((fpArea = fopen (areafile, "r")) == NULL)
      {
	perror (areafile);
	exit (1);
      }

    for (;;)
      {
	char word[200];

	if (fread_letter (fpArea) != '#')
	  {
	    spew_out ("Boot_db: # not found.\n");
	    bug (fpArea);
	    exit (1);
	  }

	strcpy (word, fread_word (fpArea));

	if (word[0] == '$')
	  break;
	else if (!strcmp (word, "AREA"))
	  load_area (fpArea);
	else if (!strcmp (word, "HELPS"))
	  load_helps (fpArea);
	else if (!strcmp (word, "MOBILES"))
	  load_mobiles (fpArea);
	else if (!strcmp (word, "OBJECTS"))
	  load_objects (fpArea);
	else if (!strcmp (word, "RESETS"))
	  load_resets (fpArea);
	else if (!strcmp (word, "ROOMS"))
	  load_rooms (fpArea);
	else if (!strcmp (word, "SHOPS"))
	  load_shops (fpArea);
	else if (!strcmp (word, "SPECIALS"))
	  load_specials (fpArea);
	else if (!strcmp (word, "AUTHORS"))
	  fread_string (fpArea);
	else if (!strcmp (word, "NODEBUG"));
	else if (!strcmp (word, "NOTELEPORT"));
	else if (!strcmp (word, "NOGOHOME"));
	else if (!strcmp (word, "RESTRICT"))
	  {
	    fread_number (fpArea);
	    fread_number (fpArea);
	  }
	else if (!strcmp (word, "TEMPERATURE"))
	  {
	    fread_number (fpArea);
	    fread_number (fpArea);
	    fread_number (fpArea);
	    fread_number (fpArea);
	  }
	else if (!strcmp (word, "FREEQUIT"));
	else
	  {
	    spew_out ("Boot_db: bad section name.\n");
	    bug (fpArea);
	    exit (1);
	  }
      }


    fclose (fpArea);
  }
  if (quest_items > 3)
    spew_out ("This area has more than 3 quest items in it. Justify this with powerful \narguments!\n");


 for (i=0;i<numbad;i++)
  {
  int inside=0;
   for (j=0;j<totalareas;j++)
   {
    if (badexits[i][1]/100 ==areavnums[j]/100)
     inside=1;
   }
   if (!inside)
   {
    spew_out("Room %d connects to external room %d\n",badexits[i][0],badexits[i][1]);
    external=1;
   }
  }

  if (!external) spew_out("This area has no external connections.\n");
  for (i=0;i<nummobcon;i++)
  {
    int inside=0,dir=0,vnum=0,toroom=0;
     for (j=0;j<totalareas;j++)
     {
       sscanf(badmobconnects[i], "%d %d %d",&vnum,&toroom,&dir);
/*       printf("Sscanf found vnum %d dir %d toroom %d\n",vnum,dir,toroom);*/
       if (toroom/100 ==areavnums[j]/100)
        inside=1;
     }
    if (!inside)
    if (toroom>0)
/*     spew_out("Mobile %d deletes a connection in direction %d\n",vnum, toroom, dir);
     else      */
     spew_out("Mobile %d makes an external connection to room %d in direction %d\n",vnum, toroom, dir);
  }
    for (i=0;i<numobjcon;i++)
  {
    int inside=0,dir=0,vnum=0,toroom=0;
     for (j=0;j<totalareas;j++)
     {
       sscanf(badobjconnects[i], "%d %d %d",&vnum,&toroom,&dir);

       if (toroom/100 ==areavnums[j]/100)
        inside=1;
     }
    if (!inside)
    if (toroom>0)
/*     spew_out("Object %d deletes a connection in direction %d\n",vnum, toroom, dir);
     else*/
     spew_out("Object %d makes an external connection to room %d in direction %d\n",vnum, toroom, dir);
  }

  spew_out ("Finished.\n");

}



/*
 * Snarf an 'area' header line.
 */
void
load_area (FILE * fp)
{
  fread_string (fp);
  spew_out ("Loaded Area.\n");

  return;
}



/*
 * Snarf a help section.
 */
void
load_helps (FILE * fp)
{
  char *buf;

  for (;;)
    {
      fread_number (fp);
      buf = fread_string (fp);
      if (*buf == '$')
	{
	  spew_out ("Loaded Helps.\n");
	  break;
	}
      fread_string (fp);

    }

  return;
}


void
spew_out (char *fmt,...)
{
  char buf[1024];		/* better safe than sorry */
  va_list args;
  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);
  if (strstr (buf, "\n"))
    lineno++;

#ifndef unix
  fprintf (stderr, buf);
  if (lineno % 23 == 0)
    {
      printf ("Press any key to continue...\n");
      getch ();
      lineno = 1;
    }
#else
   printf(buf);
#endif

}


/*
 * Snarf a mob section.
 */
void
load_mobiles (FILE * fp)
{

  int vnum;
  int cnt, level;
  float hitsize, hitdice, hitplus, maxhit, minhit, avehit;
  float damsize, damdice, damplus, maxdam, mindam, avedam;
  for (;;)
    {
      char letter;

      letter = fread_letter (fp);
      if (letter != '#')
	{
	  spew_out ("Load_mobiles: # not found.\n");
	  bug (fp);
	  exit (1);
	}

      if ((vnum = fread_number (fp)) == 0)
	{
	  spew_out ("Loaded %d Mobiles.\n", total_mob);
	  return;
	}

      mob_vnum[total_mob] = vnum;
      for (cnt = 0; cnt < total_mob; cnt++)
	if (mob_vnum[cnt] == vnum)
	  {
	    spew_out ("Load_mobiles: vnum %d repeated.\n", vnum);
	    bug (fp);
	    exit (1);
	  }


      total_mob++;

      fread_string (fp);
      fread_string (fp);
      fread_string (fp);
      fread_string (fp);

      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      letter = fread_letter (fp);
      if ((level = fread_number (fp)) > 200)
	{
	  spew_out ("Load_mobiles: Mob %d level too high.\n", vnum);
	  bug (fp);
	  exit (1);
	}
      /*
       * The unused stuff is for imps who want to use the old-style
       * stats-in-files method.
       */
      fread_number (fp);
      fread_number (fp);
      hitdice = fread_number (fp);
      /* 'd'          */ fread_letter (fp);
      hitsize = fread_number (fp);
      /* '+'          */ fread_letter (fp);
      hitplus = fread_number (fp);
      damdice = fread_number (fp);
      /* 'd'          */ fread_letter (fp);
      damsize = fread_number (fp);
      /* '+'          */ fread_letter (fp);
      damplus = fread_number (fp);
      fread_number (fp);
      /* xp can't be used! */ fread_number (fp);
      /* Unused */
      fread_number (fp);
      /* start pos    */ fread_number (fp);
      /* Unused */

      maxhit = hitdice * hitsize + hitplus;
      minhit = hitplus + hitdice;
      avehit = (maxhit + minhit) / 2;
      /* spew_out("maxhit is %d hitdice is %d hitsize is %d hitplus is %d minhit is %d\n",maxhit,hitdice,hitsize,hitplus,minhit);
         spew_out("Average hit points of %d is %d (level %d).\n",vnum,avehit,level); */
      if (level >= 150 && avehit < (17000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 145 && avehit < (20000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 140 && avehit < (19000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 135 && avehit < (18000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 130 && avehit < (17000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 125 && avehit < (16000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 120 && avehit < (15000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 115 && avehit < (14000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 110 && avehit < (13000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 105 && avehit < (12000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 100 && avehit < (11000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 95 && avehit < (10000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 90 && avehit < (7800 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 85 && avehit < (6200 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 80 && avehit < (5000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 75 && avehit < (4200 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 70 && avehit < (3500 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 65 && avehit < (2800 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 60 && avehit < (2000 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 55 && avehit < (1650 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 50 && avehit < (1300 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 45 && avehit < (900 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 40 && avehit < (790 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 35 && avehit < (680 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 30 && avehit < (600 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 25 && avehit < (475 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 20 && avehit < (350 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 15 && avehit < (250 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 10 && avehit < (150 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      else if (level >= 5 && avehit < (60 * .8))
	spew_out ("Mobile %d has too few hit points.\n", vnum);
      maxdam = damdice * damsize + damplus;
      mindam = damplus + damdice;
      avedam = (maxdam + mindam) / 2;
/*
   spew_out("maxdam is %d damdice is %d damsize is %d damplus is %d mindam is %d\n",maxdam,damdice,damsize,damplus,mindam);
   spew_out("Average dam points of %d is %d (level %d).\n",vnum,avedam,level);
 */
      if (level >= 150 && avedam < (170 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 145 && avedam < (163 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 140 && avedam < (156 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 135 && avedam < (148 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 130 && avedam < (141 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 125 && avedam < (134 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 120 && avedam < (127 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 115 && avedam < (120 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 110 && avedam < (113 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 105 && avedam < (106 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 100 && avedam < (99 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 95 && avedam < (92 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 90 && avedam < (85 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 85 && avedam < (78 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 80 && avedam < (72 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 75 && avedam < (66 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 70 && avedam < (60 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 65 && avedam < (55 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 60 && avedam < (50 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 55 && avedam < (45 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 50 && avedam < (41 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 45 && avedam < (37 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 40 && avedam < (33 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 35 && avedam < (30 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 30 && avedam < (27 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 25 && avedam < (24 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 20 && avedam < (22 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 15 && avedam < (20 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 10 && avedam < (15 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      else if (level >= 5 && avedam < (7 * .8))
	spew_out ("Mobile %d does too little damage.\n", vnum);
      /*
       * Back to meaningful values.
       */
      fread_number (fp);

      if (letter != 'S')
	{
	  spew_out ("Load_mobiles: #%d non-S.\n", vnum);
	  bug (fp);
	  exit (1);
	}
      for (;;)
	{
	  letter = fread_letter (fp);
	  if (letter == 'D')
	    {
	      int x;
	      for (x = 0; x < 3121; x++)
		letter = fread_letter (fp);
	    }
	  else if (letter == 'X')
	    fread_string (fp);
	  else
	    break;
	}
      ungetc (letter, fp);
      if (letter == '>')
	mprog_read_programs (fp, vnum);

    }

}



/*
 * Snarf an obj section.
 */
void
load_objects (FILE * fp)
{

  OBJ_INDEX_DATA *obj;
  int cnt;
  EST_DATA est_data;

  for (;;)
    {
      int vnum;
      char letter;
      int heal=0;

      letter = fread_letter (fp);
      if (letter != '#')
	{
	  spew_out ("Load_objects: # not found.\n");
	  bug (fp);
	  exit (1);
	}

      vnum = fread_number (fp);
      if (vnum == 0)
	{
	  spew_out ("Loaded %d Objects.\n", total_obj);
	  return;
	}

      obj_vnum[total_obj] = vnum;
      for (cnt = 0; cnt < total_obj; cnt++)
	if (obj_vnum[cnt] == vnum)
	  {
	    spew_out ("Load_objects: vnum %d repeated.\n", vnum);
	    bug (fp);
	    exit (1);
	  }
      total_obj++;

      obj = (OBJ_INDEX_DATA *) malloc (sizeof (*obj));
      if (obj == NULL)
	{
	  spew_out ("Memory fault.\n");
	  exit (0);
	}
      obj->vnum = vnum;

      fread_string (fp);
      fread_string (fp);
      fread_string (fp);
      fread_string (fp);


      obj->item_type = fread_number (fp);
      if (obj->item_type < 1)
	obj->item_type = 13;

      obj->extra_flags = fread_number (fp);
      obj->wear_flags = fread_number (fp);
      obj->value[0] = fread_number (fp);
      obj->value[1] = fread_number (fp);
      obj->value[2] = fread_number (fp);
      obj->value[3] = fread_number (fp);
      obj->weight = fread_number (fp);
      obj->cost = fread_number (fp);

      obj->level = 0;
      obj->affected = 0;
      obj->level_rent = fread_number (fp);
      if (obj->cost<10) spew_out("Following item costs too little (only %d gold)\n",obj->cost);
      if (obj->cost>10000000) spew_out("Following item costs too much (%d gold!!)\n",obj->cost);

      if ((obj->extra_flags & ITEM_LEVEL) != 0)

        obj->level = obj->level_rent;
      else
       spew_out("Following item lacks the ITEM_FLAG_LEVEL flag. Please add it\n");


      for (;;)
	{
	  char letter;

	  letter = fread_letter (fp);

	  if (letter == 'A')
	    {
	      AFFECT_DATA *paf;

	      paf = (AFFECT_DATA *) malloc (sizeof (*paf));
	      if (paf == NULL)
		{
		  spew_out ("Memory fault.\n");
		  exit (0);
		}
	      paf->type = -1;
	      paf->duration = -1;
	      paf->location = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->bitvector = 0;
	      paf->next = obj->affected;
	      obj->affected = paf;

	    }

	  else if (letter == 'C')
	    {
	      fread_string (fp);
	      fread_number (fp);
	    }

	  else if (letter == 'D')
	    {
	      int x;
	      for (x = 0; x < 3120; x++)
		fread_letter (fp);
	    }

	  else if (letter == 'E')
	    {
	      fread_string (fp);
	      fread_string (fp);
	    }
	  else if (letter == 'X')
	    {
	      fread_string (fp);
	    }

	  else if (letter == 'P')
	    load_object_program (fp, vnum);

	  else
	    {
	      ungetc (letter, fp);
	      /* Let's look over the levels:  */
	      est_data = obj_level_estimate (obj);

	      /*
	       * Checking for spell type and spell level on
	       * scrolls, wands, staffs, pills and potions.
	       */
	      switch (obj->item_type)
		{
		  int i;
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
		  for (i = 1; i < 4; i++)
		    {
		      switch (obj->value[i])
			{
            case 28:heal++;break;
			case 635:
			  spew_out ("Warning! Illegal spell 'Breath Water'\n");
			  break;
			case 614:
			  spew_out ("Warning! Illegal spell 'Enhanced Rest'\n");
			  break;
			case 615:
			  spew_out ("Warning! Illegal spell 'Enhanced Heal'\n");
			  break;
			case 616:
			  spew_out ("Warning! Illegal spell 'Enhanced Revive'\n");
			  break;
			case 626:
			  spew_out ("Warning! Illegal spell 'Tremor'\n");
			  break;
			case 628:
			  spew_out ("Warning! Illegal spell 'Induction'\n");
			  break;
			case 617:
			  spew_out ("Warning! Illegal spell 'Animate Dead'\n");
			  break;
			case 618:
			  spew_out ("Warning! Illegal spell 'Banish'\n");
			  break;
			case 636:
			  spew_out ("Warning! Illegal spell 'Mage Blast'\n");
			  break;
			case 637:
			  spew_out ("Warning! Illegal spell 'Confusion'\n");
			  break;
			case 629:
			  spew_out ("Warning! Illegal spell 'Illusion'\n");
			  break;
			case 70:
			  spew_out ("Warning! Illegal spell 'Acid Blast'\n");
			  break;
			case 631:
			  spew_out ("Warning! Illegal spell 'Hallucinate'\n");
			  break;
			case 638:
			  spew_out ("Warning! Illegal spell 'Sanctify'\n");
			  break;
			case 625:
			  spew_out ("Warning! Illegal spell 'Restore'\n");
			  break;
			case 622:
			  spew_out ("Warning! Illegal spell 'Dispel Good'\n");
			  break;
			case 624:
			  spew_out ("Warning! Illegal spell 'Feast'\n");
			  break;
			}
		    }		/* Of For Loop */
		  break;
		case ITEM_WAND:
		case ITEM_STAFF:
		  switch (obj->value[3])
		    {
		    case 635:
		      spew_out ("Warning! Illegal spell 'Breath Water'\n");
		      break;
		    case 614:
		      spew_out ("Warning! Illegal spell 'Enhanced Rest'\n");
		      break;
		    case 615:
		      spew_out ("Warning! Illegal spell 'Enhanced Heal'\n");
		      break;
		    case 616:
		      spew_out ("Warning! Illegal spell 'Enhanced Revive'\n");
		      break;
		    case 626:
		      spew_out ("Warning! Illegal spell 'Tremor'\n");
		      break;
		    case 628:
		      spew_out ("Warning! Illegal spell 'Induction'\n");
		      break;
		    case 617:
		      spew_out ("Warning! Illegal spell 'Animate Dead'\n");
		      break;
		    case 618:
		      spew_out ("Warning! Illegal spell 'Banish'\n");
		      break;
		    case 636:
		      spew_out ("Warning! Illegal spell 'Mage Blast'\n");
		      break;
		    case 637:
		      spew_out ("Warning! Illegal spell 'Confusion'\n");
		      break;
		    case 629:
		      spew_out ("Warning! Illegal spell 'Illusion'\n");
		      break;
		    case 70:
		      spew_out ("Warning! Illegal spell 'Acid Blast'\n");
		      break;
		    case 631:
		      spew_out ("Warning! Illegal spell 'Hallucinate'\n");
		      break;
		    case 638:
		      spew_out ("Warning! Illegal spell 'Sanctify'\n");
		      break;
		    case 625:
		      spew_out ("Warning! Illegal spell 'Restore'\n");
		      break;
		    case 622:
		      spew_out ("Warning! Illegal spell 'Dispel Good'\n");
		      break;
		    case 624:
		      spew_out ("Warning! Illegal spell 'Feast'\n");
		      break;
		    }
		  break;

		}

        if (heal>1) spew_out("Following object has more than one heal spell!\n");
	      spew_out ("Obj #%5d Given Level: %3d Estimated at: %3d ",
			obj->vnum, obj->level, est_data.estlevel);

	      /*    if ((est_data.estlevel / 2 - 5) > obj->level)
	         spew_out ("Warning!  Level will be reset to %d.",
	         ((est_data.estlevel + obj->level) / 2));
	       */
	      if (obj->level < est_data.estlevel - (est_data.estlevel / 20 + 5))
		spew_out ("Warning! Object level is too low");
	      spew_out ("\n");


	      while (obj->affected != NULL)
		{
		  AFFECT_DATA *next_aff;
		  next_aff = obj->affected->next;
		  free (obj->affected);
		  obj->affected = next_aff;
		}

	      free (obj);
	      break;
	    }
	}
    }
}



/*
 * Snarf a reset section.
 */

void
load_resets (FILE * fp)
{
  int arg1, arg2, arg3;
  int found, cnt;

  for (;;)
    {
      char letter;

      if ((letter = fread_letter (fp)) == 'S')
	{
	  spew_out ("Loaded Resets.\n");
	  break;
	}

      if (letter == '*')
	{
	  fread_to_eol (fp);
	  continue;
	}

      fread_number (fp);
      arg1 = fread_number (fp);
      arg2 = fread_number (fp);
      if (letter != 'G' && letter != 'R')
	arg3 = fread_number (fp);
      else
	arg3 = 0;

      fread_to_eol (fp);

      /*
       * Validate parameters.
       * We're calling the index functions for the side effect.
       * And counting occurances of things.
       */
      found = FALSE;
      switch (letter)
	{
	default:
	  spew_out ("Load_resets: bad command '%c'.\n", letter);
	  bug (fp);
	  exit (1);
	  break;

	case 'M':
	  found = FALSE;
	  for (cnt = 0; cnt < total_mob; cnt++)
	    if (mob_vnum[cnt] == arg1)
	      {
		found = TRUE;
		break;
	      }
	  if (!found)
	    {
	      spew_out ("Load_resets: bad mobile #%d\n", arg1);
	      bug (fp);
	    }
	  found = FALSE;
	  for (cnt = 0; cnt < total_room; cnt++)
	    if (room_vnum[cnt] == arg3)
	      {
		found = TRUE;
		break;
	      }
	  if (!found)
	    {
	      spew_out ("Load_resets: bad room #%d\n", arg3);
	      bug (fp);
	    }
	  break;

	case 'O':
	  found = FALSE;
	  for (cnt = 0; cnt < total_obj; cnt++)
	    if (obj_vnum[cnt] == arg1)
	      found = TRUE;

	  if (!found)
	    {
	      spew_out ("Load_resets: bad object #%d\n", arg1);
	      bug (fp);
	    }
	  found = FALSE;

	  for (cnt = 0; cnt < total_room; cnt++)
	    if (room_vnum[cnt] == arg3)
	      {
		found = TRUE;
		break;
	      }
	  if (!found)
	    {
	      spew_out ("Load_resets: bad room #%d\n", arg3);
	      bug (fp);
	    }

	  break;

	case 'P':
	  found = FALSE;
	  for (cnt = 0; cnt < total_obj; cnt++)
	    if (obj_vnum[cnt] == arg1)
	      found = TRUE;

	  if (!found)
	    {
	      spew_out ("Load_resets: bad object #%d\n", arg1);
	      bug (fp);
	    }
	  found = FALSE;
  	  for (cnt = 0; cnt < total_obj; cnt++)
	    if (obj_vnum[cnt] == arg3)
        {
	      found = TRUE;
          break;
        }
	  if (!found)
	    {
	      spew_out ("Load_resets: object #%d\n", arg3);
	      bug (fp);
	    }

	  break;

	case 'G':
	case 'E':
	  found = FALSE;
	  for (cnt = 0; cnt < total_obj; cnt++)
	    if (obj_vnum[cnt] == arg1)
	      {
		found = TRUE;
		break;
	      }
	  if (!found)
	    {
	      spew_out ("Load_resets: bad object #%d\n", arg1);
	      bug (fp);
	    }
	  break;

	case 'D':
	  found = FALSE;
	  for (cnt = 0; cnt < total_room; cnt++)
	    if (room_vnum[cnt] == arg1)
	      {
		found = TRUE;
		break;
	      }

	  if (!found)
	    {
	      spew_out ("Load_resets: bad room #%d\n", arg1);
	      bug (fp);
	    }

	  if ((exit_data[cnt] & logs[arg2]) == 0)
	    {
	      spew_out ("Load_resets: bad exit #%d (%d)\n", arg2, exit_data[cnt]);
	      bug (fp);
	      exit (0);
	    }
	  break;

	case 'R':
	  found = FALSE;
	  for (cnt = 0; cnt < total_room; cnt++)
	    if (room_vnum[cnt] == arg1)
	      {
		found = TRUE;
		break;
	      }
	  if (!found)
	    {
	      spew_out ("Load_resets: bad room #%d\n", arg1);
	      bug (fp);
	    }

	  if (arg2 < 0 || arg2 > 6)
	    {
	      spew_out ("Load_resets: bad exit #%d\n", arg2);
	      bug (fp);
	      exit (0);
	    }
	  break;
	}

    }

  return;
}



/*
 * Snarf a room section.
 * And counting occurances of things.
 */
void
load_rooms (FILE * fp)
{
  int door;
  for (;;)
    {
      int vnum;
      char letter;
      int inside=0,cnt=0,mart=0;

      letter = fread_letter (fp);
      if (letter != '#')
	{
	  spew_out ("Load_rooms: # not found.\n");
	  bug (fp);
	  exit (1);
	}

      vnum = fread_number (fp);
      if (vnum == 0)
	{
	  spew_out ("Loaded %d Rooms.\n", total_room);
	  return;
	}


    if (totalareas==0)
    {
       areavnums[totalareas++]=vnum;
     }

    for (mart=0;mart<10;mart++)
    {
      if (vnum/100 ==(areavnums[mart]/100))
      {
      inside=1;
      }
    }

  if (!inside) areavnums[totalareas++]=vnum;

      room_vnum[total_room] = vnum;
      for (cnt = 0; cnt < total_room; cnt++)
	if (room_vnum[cnt] == vnum)
	  {
	    spew_out ("Load_rooms: vnum %d repeated.\n", vnum);
	    bug (fp);
	    exit (1);
	  }

      fread_string (fp);
      fread_string (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);


      for (;;)
	{
	  letter = fread_letter (fp);

	  if (letter == 'S')
	    break;
	  else if (letter == 'X')
	    fread_string (fp);
	  else if (letter == 'F')
	    {
	      fread_number (fp);
	      fread_number (fp);
          fread_number (fp);
	    }
	  else if (letter == 'D')
	    {
	      char testchar;
          int wayout=0,i=0,inside=0;
	      door = fread_number (fp);
	      if (door < 0 || door > 5)
		{
		  spew_out ("Fread_rooms: vnum %d has bad door number.\n", vnum);
		  bug (fp);
		  exit (1);
		}

	      fread_string (fp);
	      fread_string (fp);
	      fread_number (fp);

	      fread_number (fp);
	      wayout=fread_number (fp);
          for (i=0;i<totalareas;i++)
          {
           if (wayout/100==areavnums[i]/100)
             inside=1;
          }
           if (!inside)
           {
            badexits[numbad][0]=vnum;
            badexits[numbad][1]=wayout;
            numbad++;
/*            spew_out("Adding bad exit %d, from %d to %d\n",numbad,(badexits[numbad-1][0]),(badexits[numbad-1][1]));*/
           }
           /*spew_out("Room %d connects to area %d via room %d\n",vnum,(wayout/100),wayout);*/
	      testchar = fread_letter (fp);
	      if (testchar != 'X')
		ungetc (testchar, fp);
	      else
		fread_string (fp);
	      if ((exit_data[total_room] & logs[door]) != 0)
		{
		  spew_out ("Fread_rooms: vnum %d has same dir number.\n", vnum);
		  bug (fp);
		  exit (1);
		}
	      exit_data[total_room] |= logs[door];

	    }
	  else if (letter == 'E')
	    {

	      fread_string (fp);
	      fread_string (fp);

	    }
	  else
	    {
	      spew_out ("Load_rooms: vnum %d has flag not 'DES' (letter was %c).\n", vnum, letter);
	      bug (fp);
	      exit (1);
	    }
	}

      total_room++;
    }
}



/*
 * Snarf a shop section.
 */
void
load_shops (FILE * fp)
{
  int iTrade;

  for (;;)
    {

      iTrade = fread_number (fp);
      if (iTrade == 0)
	{
	  spew_out ("Loaded Shops.\n");
	  return;
	}
      for (iTrade = 0; iTrade < 5; iTrade++)
	fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      fread_to_eol (fp);
    }

}



/*
 * Snarf spec proc declarations.
 */
void
load_specials (FILE * fp)
{
  for (;;)
    {
      char letter;

      switch (letter = fread_letter (fp))
	{
	default:
	  spew_out ("Load_specials: letter '%c' not *MS.\n", letter);
	  bug (fp);
	  exit (1);

	case 'S':
	  spew_out ("Loaded Specials.\n");
	  return;

	case '*':
	  break;

	case 'M':
	  fread_number (fp);
	  fread_word (fp);

	  break;

	case 'O':
	  fread_number (fp);
	  fread_word (fp);
	  break;
	}

      fread_to_eol (fp);
    }
}





/*
 * Read a letter from a file.
 */
char
fread_letter (FILE * fp)
{
  char c;

  do
    {
      c = getc (fp);
    }
  while (c == ' ' || c == '\r' || c == '\n');

  return c;
}



/*
 * Read a number from a file.
 */
int
fread_number (FILE * fp)
{
  int number;
  int sign;
  int bit, qfound;
  char c;
  char buf[2], buf2[100];

  buf[1] = '\0';
  buf2[0] = '\0';
  bit = FALSE;
  do
    {
      c = getc (fp);
    }
  while (isspace (c));

  number = 0;
  qfound = FALSE;
  sign = FALSE;
  if (c == '+')
    {
      c = getc (fp);
    }
  else if (c == '-')
    {
      sign = TRUE;
      c = getc (fp);
    }
  if (c == 'Q')
    {
      c = getc (fp);		/* Get the following letter, and then grab a real number */
      number = (int) c - (int) 'Q' + 1;
      c = getc (fp);
      qfound = TRUE;
    }
  while ((c >= 'A' && c <= 'Z') || c == '_')
    {
      bit = TRUE;
      buf[0] = c;
      strcat (buf2, buf);
      c = getc (fp);
    }

  if (bit)
    {
      BITVECTOR_DATA *bt;
      int foundb;
      foundb = FALSE;
      for (bt = bitvector_list; bt != NULL && !foundb; bt = bt->next)
	if (!strcmp (bt->name, buf2))
	  {
	    foundb = TRUE;
	    number = bt->value;
	  }
      if (!foundb)
	{
	  spew_out ("Fread_number: bad format '%s'.\n", buf2);
	  bug (fp);
	  exit (0);
	}
    }
  else
    {
      if (!isdigit (c) && (!qfound || c != ' '))
	{
	  char *tmp;
	  tmp = fread_word (fp);
	  spew_out ("Fread_number: bad format '%c%s'.\n", c, tmp);
	  bug (fp);
	  exit (1);
	}

      while (isdigit (c))
	{
	  number = number * 10 + c - '0';
	  c = getc (fp);
	}
    }
  if (sign)
    number = 0 - number;

  if (c == '|')
    number += fread_number (fp);
  else if (c != ' ')
    ungetc (c, fp);

  return (number);
}

/*
 * Read and allocate space for a string from a file.
 * Strings are created and placed in Dynamic Memory.
 */
char *
fread_string (FILE * fp)
{
  char *plast;
  char c;

  plast = fread_string_buf;
  /*
   * Skip blanks.
   * Read first char.
   */
  do
    {
      c = getc (fp);
    }
  while (isspace (c));

  if ((*plast++ = c) == '~')
    return NULL;

  for (;;)
    {
      /*
       * Back off the char type lookup,
       *   it was too dirty for portability.
       *   -- Furey
       */
      switch (*plast = getc (fp))
	{
	default:
	  plast++;
	  break;

	case EOF:
	  spew_out ("Fread_string: EOF\n");
	  bug (fp);
	  exit (1);
	  break;

	case '\n':
	  plast++;
	  *plast++ = '\r';
	  break;

	case '\r':
	  break;

	case '~':
	  *plast = '\0';
	  return (fread_string_buf);
	  /* String space repointer was removed - Chaos 5/19/94 */
	}
    }
}



/*
 * Read to end of line (for comments).
 */
void
fread_to_eol (FILE * fp)
{
  char c;

  do
    {
      c = getc (fp);
    }
  while (c != '\n' && c != '\r');

  do
    {
      c = getc (fp);
    }
  while (c == '\n' || c == '\r');

  ungetc (c, fp);
  return;
}



/*
 * Read one word (into static buffer).
 */
char *
fread_word (FILE * fp)
{
  char *pword;
  char cEnd;



  do
    {
      cEnd = getc (fp);
    }
  while (isspace (cEnd));

  if (cEnd == '\'' || cEnd == '"')
    {
      pword = word;
    }
  else
    {
      word[0] = cEnd;
      pword = word + 1;
      cEnd = ' ';
    }

  for (; pword < word + MAX_INPUT_LENGTH; pword++)
    {
      *pword = getc (fp);
#ifdef unix
      if (*pword == EOF)
#else
      if (feof (fp))
#endif
	{
	  *pword = '\0';
	  return word;
	}
      if (cEnd == ' ' ? isspace (*pword) : *pword == cEnd)
	{
	  if (cEnd == ' ')
	    ungetc (*pword, fp);
	  *pword = '\0';
	  return word;
	}
    }

  word[10] = '\0';
  spew_out ("Fread_word: word '%s' too long.\n", word);
  bug (fp);
  exit (1);
  return NULL;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void
smash_tilde (char *str)
{
  for (; *str != '\0'; str++)
    {
      if (*str == '~')
	*str = '-';
    }

  return;
}

void
tail_chain (void)
{
  return;
}

void
mprog_read_programs (FILE * fp, int vnum)
{
  char letter;
  int done = FALSE;

  letter = ' ';

  while (!done)
    {
      letter = fread_letter (fp);
      switch (letter)
	{int i;
	case '>':
	  fread_word (fp);
	  fread_string (fp);
	  fread_string (fp);

      for(i=0;i<strlen(fread_string_buf);i++)
       fread_string_buf[i]=tolower(fread_string_buf[i]);
      while (strstr(fread_string_buf, "connect"))
      {
       int dir, to;
       char buf[64];

       strcpy(fread_string_buf,strstr(fread_string_buf, "connect"));
       if (!isdigit(fread_string_buf[strlen("connect ")]))
        break;
       sscanf(fread_string_buf,"%s %d %d", &buf, &dir, &to);
       sprintf(buf, "%s %d %d",buf,dir,to);
       strcpy(fread_string_buf,strstr(fread_string_buf, "connect")+strlen(buf));
       sprintf(buf, "%d %d %d",vnum,to,dir);
       strcpy(badmobconnects[nummobcon++],buf);
      }
	  break;
	case '|':
	  fread_to_eol (fp);
	  done = TRUE;
	  break;
	default:
	  spew_out ("Load_mobiles: bad MOBPROG.\n");
	  bug (fp);
	  exit (1);
	  break;
	}
    }

  return;

}

void
load_sites (void)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  BITVECTOR_DATA *bit;
  int done;

  done = 0;
  bitvector_list = NULL;
  if ((fp = fopen ("bitvector.lst", "r")) == NULL)
    {
      printf ("bitvector.lst not found! Download it from www.csn.ul.ie/~martin/games.html\n");
      printf ("Then store it in the same directory as this file...\n");
      exit (1);
    }
  if (fp != NULL)
    {
      done = 0;
      while (done == 0)
	{
	  strcpy (buf, fread_word (fp));
	  if (!strcmp (buf, "NULL"))
	    done = 1;
	  else
	    {
	      bit = (BITVECTOR_DATA *) malloc (sizeof (*bit));
	      if (bit == NULL)
		{
		  spew_out ("Memory fault.\n");
		  exit (0);
		}
	      bit->next = bitvector_list;
	      bitvector_list = bit;
	      bit->name = strdup (buf);
	      bit->value = fread_number (fp);

	    }
	}
    }
  fclose (fp);


  return;
}


void
bug (FILE * fpArea)
{
  if (fpArea != NULL)
    {
      int iLine;
      long iChar;


      if (fpArea == stdin)
	{
	  iLine = 0;
	}
      else
	{
	  iChar = ftell (fpArea);
	  fseek (fpArea, 0, 0);
	  for (iLine = 0; ftell (fpArea) < iChar; iLine++)
	    while (getc (fpArea) != '\n');
	  fseek (fpArea, iChar, 0);
	}

      spew_out ("LINE: %d\n", iLine);

    }

  return;
}


void
load_object_program (FILE * fp,int vnum)
{
  char keyword;


  fread_number (fp);		/* get index number */

  keyword = (char) fread_number (fp);	/* get trigger command type */
  switch (keyword)
    {
    case 'C':			/* game command */
      fread_number (fp);	/* get chance number */
      fread_word (fp);
      break;
    case 'U':			/* unknown command or social */
      fread_number (fp);	/* get chance number */
      fread_word (fp);
      break;
    case 'T':			/* Tick check */
      fread_number (fp);	/* get chance number */
      break;
    case 'X':			/* void trigger */
      break;
    case 'H':			/* Got hit check */
      fread_number (fp);	/* get chance number */
      break;
    case 'D':			/* Damaged another check */
      fread_number (fp);	/* get chance number */
      break;
    default:
      spew_out ("Bad obj_command type\n");
    }

  keyword = (char) fread_number (fp);	/* get reaction command */
  switch (keyword)
    {
    int i;
    case 'E':			/* screen echo */
      fread_string (fp);
      break;
    case 'C':			/* user command at level 99 without arg, but with multi-line */
      fread_string (fp);
      for(i=0;i<strlen(fread_string_buf);i++)
       fread_string_buf[i]=tolower(fread_string_buf[i]);

      while (strstr(fread_string_buf, "connect"))
      {
       int dir, to;
       char buf[64];
       strcpy(fread_string_buf,(strstr(fread_string_buf, "connect")));
       if (!isdigit(fread_string_buf[strlen("connect ")]))
        break;
       sscanf(fread_string_buf,"%s %d %d", &buf, &dir, &to);
       sprintf(buf, "%s %d %d",buf,dir,to);
       strcpy(fread_string_buf,(strstr(fread_string_buf, "connect")+strlen(buf)));
       sprintf(buf, "%d %d %d",vnum,to,dir);
       strcpy(badobjconnects[numobjcon++],buf);
      }
      break;
    case 'G':			/* user command at level 99 with argument */
      fread_string (fp);
      break;
    case 'S':			/* Set quest bit to value */
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'D':			/* Add to quest bit */
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'P':			/* Player quest bit if check */
      fread_number (fp);
      fread_number (fp);
      fread_letter (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'Q':			/* Quest bit if check */
      fread_number (fp);
      fread_number (fp);
      fread_letter (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'H':			/* If has object check */
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'I':			/* If check */
      fread_number (fp);
      fread_letter (fp);
      fread_number (fp);
      fread_number (fp);
      fread_number (fp);
      break;
    case 'A':			/* Apply to temp stats */
      fread_number (fp);
      fread_number (fp);
      break;
    case 'J':			/* Junk the item */
      break;
    default:
      {
	spew_out ("Bad obj_command reaction type\n");
	bug (fp);
      }
    }

  return;

}



EST_DATA
obj_level_estimate (OBJ_INDEX_DATA * objIndex)
{
  AFFECT_DATA *aff;
  EST_DATA data;
  int level = 1;
  int value[4];
  int applies = 0;
  int truelevel;
  int negatives = 0, positives = 0;
  int wearflags = 0;
  int tempapply[10][2];
  int i, j, temp = 0;


  for (i = 0; i < 10; i++)
    for (j = 0; j < 2; j++)
      tempapply[i][j] = 0;

  if (objIndex->affected != NULL)
    for (aff = objIndex->affected; aff != NULL; aff = aff->next)
      {
	if (!aff->location || !aff->modifier)
	  continue;
	tempapply[temp][0] = aff->location;
	tempapply[temp][1] = aff->modifier;
	if (tempapply[temp][0] == APPLY_MANA && tempapply[temp][1] < 0)
	  {
	    spew_out ("Following object MUST be a quest item (it has negative mana modifer).\n");
	    quest_items++;
	  }
	temp++;
      }
  for (i = 0; i <= temp; i++)
    {
      for (j = 0; j <= temp; j++)
	{
	  if (i != j && tempapply[i][0] != 0 && tempapply[i][0] == tempapply[j][0])
	    {
	      tempapply[i][1] += tempapply[j][1];
	      tempapply[j][0] = 0;
	      tempapply[j][1] = 0;
	    }
	}
    }

  for (i = 0; i <= temp; i++)
    {
      truelevel = objIndex->level;
      switch (tempapply[i][0])
	{
	case APPLY_STR:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (3 * tempapply[i][1])) / 2;
		positives += (tempapply[i][1] * (3 * tempapply[i][1])) / 2;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1]));
		negatives -= (tempapply[i][1] * (tempapply[i][1]));
	      }
	    if ((tempapply[i][1] > (truelevel / 10 + 1 - truelevel / 20)) ||
	    (tempapply[i][1] < (-2 * (truelevel / 10 + 1 - truelevel / 15))))
	      spew_out ("Following object affects strength too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_DEX:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
		positives += (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 3;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 3;
	      }
	    if ((tempapply[i][1] > (truelevel / 10 + 1 - truelevel / 20)) ||
	    (tempapply[i][1] < (-2 * (truelevel / 10 + 1 - truelevel / 15))))
	      spew_out ("Following object affects dexterity too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_INT:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
		positives += (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 4;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 4;
	      }
	    if ((tempapply[i][1] > (truelevel / 10 + 1 - truelevel / 20)) ||
	    (tempapply[i][1] < (-2 * (truelevel / 10 + 1 - truelevel / 15))))
	      spew_out ("Following object affects intelligence too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_WIS:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (4 * tempapply[i][1])) / 3;
		positives += (tempapply[i][1] * (4 * tempapply[i][1])) / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
		negatives -= (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
	      }
	    if ((tempapply[i][1] > (truelevel / 10 + 1 - truelevel / 20)) ||
	    (tempapply[i][1] < (-2 * (truelevel / 10 + 1 - truelevel / 15))))
	      spew_out ("Following object affects wisdom too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_CON:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (4 * tempapply[i][1])) / 3;
		positives += (tempapply[i][1] * (4 * tempapply[i][1])) / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
		negatives -= (tempapply[i][1] * (2 * tempapply[i][1])) / 3;
	      }
	    if ((tempapply[i][1] > (truelevel / 10 + 1 - truelevel / 20)) ||
	    (tempapply[i][1] < (-2 * (truelevel / 10 + 1 - truelevel / 15))))
	      spew_out ("Following object affects constitution too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_MANA:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += tempapply[i][1] / 3;
		positives += tempapply[i][1] / 3;
	      }
	    else
	      {
		level += tempapply[i][1] / 5;
		negatives += tempapply[i][1] / 5;
	      }
	    if ((tempapply[i][1] > truelevel * 2) ||
		(tempapply[i][1] < -2 * truelevel))
	      spew_out ("Following object affects mana too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_HIT:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += tempapply[i][1] / 2;
		positives += tempapply[i][1] / 2;
	      }
	    else
	      {
		level += tempapply[i][1] / 4;
		negatives += tempapply[i][1] / 4;
	      }
	    if ((tempapply[i][1] > truelevel * 5 / 6) ||
		(tempapply[i][1] < (-1 * (truelevel * 4 / 3))))
	      spew_out ("Following object affects hit points too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_MOVE:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += tempapply[i][1] / 4;
		positives += tempapply[i][1] / 4;
	      }
	    if ((tempapply[i][1] > truelevel * 2) || (tempapply[i][1] < (-2 * truelevel)))
	      spew_out ("Following object affects move too much.\n");
	    applies = applies + 1;
	  }
	  break;

	case APPLY_AC:
	  {
	    if (objIndex->item_type == ITEM_ARMOR)
	      spew_out ("Following object is of type armor with an APPLY_AC of %d.\n", tempapply[i][1]);
	    if (tempapply[i][1] < 0)
	      {
		level += (tempapply[i][1] * (tempapply[i][1])) / 3;
		positives += (tempapply[i][1] * (tempapply[i][1])) / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 6;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 6;
	      }
	    if ((tempapply[i][1] > truelevel / 4 + 5) ||
		(tempapply[i][1] < -1 * (truelevel / 4 + 5)))
	      spew_out ("Following object affects armor class too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_HITROLL:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (tempapply[i][1])) * 2 / 3;
		positives += (tempapply[i][1] * (tempapply[i][1])) * 2 / 3;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 6;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 6;
	      }
	    if ((tempapply[i][1] > truelevel / 5 + 1) ||
		(tempapply[i][1] < -2 * (truelevel / 4 + 1)))
	      spew_out ("Following object affects hitroll too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_DAMROLL:
	  {
	    if (tempapply[i][1] > 0)
	      {
		level += (tempapply[i][1] * (tempapply[i][1]));
		positives += (tempapply[i][1] * (tempapply[i][1]));
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 3;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 3;
	      }
	    if ((tempapply[i][1] > truelevel / 10 + 1) ||
		(tempapply[i][1] < -2 * (truelevel / 12 + 1)))
	      spew_out ("Following object affects damage roll too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_SAVING_BREATH:
	  {
	    if (tempapply[i][1] < 0)
	      {
		level += (tempapply[i][1] * (tempapply[i][1])) / 6;
		positives += (tempapply[i][1] * (tempapply[i][1])) / 6;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 7;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 7;
	      }
	    if ((tempapply[i][1] < -1 * (truelevel / 10 + 1)) ||
		(tempapply[i][1] > 2 * (truelevel / 12 + 1)))
	      spew_out ("Following object affects save versus breath too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_SAVING_SPELL:
	  {
	    if (tempapply[i][1] < 0)
	      {
		level += (tempapply[i][1] * (tempapply[i][1])) / 4;
		positives += (tempapply[i][1] * (tempapply[i][1])) / 4;
	      }
	    else
	      {
		level -= (tempapply[i][1] * (tempapply[i][1])) / 5;
		negatives -= (tempapply[i][1] * (tempapply[i][1])) / 5;
	      }
	    if ((tempapply[i][1] < -1 * (truelevel / 10 + 1)) ||
		(tempapply[i][1] > 2 * (truelevel / 12 + 1)))
	      spew_out ("Following object affects saves versus spell too much.\n");
	    applies = applies + 1;
	  }
	  break;
	case APPLY_NONE:
	case APPLY_SEX:
	default:
	  break;
	}
      if (applies > 3)
	{
	  spew_out ("Following object has TOO MANY APPLIES!\n");
	  applies = -100;
	}
    }

  value[0] = objIndex->value[0];
  value[1] = objIndex->value[1];
  value[2] = objIndex->value[2];
  value[3] = objIndex->value[3];
  switch (objIndex->item_type)
    {
    case ITEM_LIGHT:
      if (value[2] < 0)
	{
	  if (level < 25)
	    level = 25;
	}
      else if (level + value[2] / 2400 > 3 * level / 2)
	level = level + value[2] / 2400;
      else
	level = 3 * level / 2;
      break;
    case ITEM_SCROLL:
      if (value[1] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[2] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[3] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      break;
    case ITEM_POTION:
      if (value[1] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[2] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[3] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      break;
    case ITEM_PILL:
      if (value[1] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[2] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      if (value[3] > 0)
	{
	  level += value[0] / 3;
	  if (value[0] / 3 > 0)
	    positives += value[0] / 3;
	  else
	    negatives += value[0] / 3;
	}
      break;
    case ITEM_WAND:
      {
	level += value[0] * (value[1]) / 5;
	if ((value[0] * value[1] / 5) > 0)
	  positives += value[0] * value[1] / 5;
	else
	  negatives += value[0] * value[1] / 5;
      }
      break;
    case ITEM_STAFF:
      {
	level += value[0] * (value[1]) / 7;
	if ((value[0] * (value[1]) / 7) > 0)
	  positives += value[0] * (value[1]) / 7;
	else
	  negatives += value[0] * (value[1]) / 7;
      }
      break;
    case ITEM_WEAPON:
      {
	level += (int) ((5 * (value[1] * (1 + (value[2] - 1) / 2.0)) / 2) - 10);
	if (((5 * (value[1] * (1 + (value[2] - 1) / 2.0)) / 2) - 10) > 0)
	  positives += (int) ((5 * (value[1] * (1 + (value[2] - 1) / 2.0)) / 2) - 10);
	else
	  negatives += (int) ((5 * (value[1] * (1 + (value[2] - 1) / 2.0)) / 2) - 10);
      }
      break;
    case ITEM_ARMOR:
      {
	level += value[0] * abs (value[0]) / 4 + 1;
	if ((value[0] * abs (value[0]) / 4 + 1) > 0)
	  positives += value[0] * abs (value[0]) / 4 + 1;
	else
	  negatives += value[0] * abs (value[0]) / 4 + 1;
      }
      break;
    case ITEM_AMMO:
      {
	level += value[1] * value[3] * value[2] / 20;
	if ((value[1] * value[3] * value[2] / 20) > 0)
	  positives += value[1] * value[3] * value[2] / 20;
	else
	  negatives += value[1] * value[3] * value[2] / 20;
      }
      break;
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
      {
	level += value[0] / 15;
	if ((value[0] / 15) > 0)
	  positives += value[0] / 15;
	else
	  negatives += value[0] / 15;
      }
      break;
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_MONEY:
    case ITEM_BOAT:
    case ITEM_FOUNTAIN:
    default:
      break;
    }

  if ((objIndex->extra_flags & ITEM_INVIS) != 0)
    {
      level -= (level / 20);
      negatives -= (level / 20);
    }
  if ((objIndex->extra_flags & ITEM_NODROP) != 0)
    {
      level -= (level / 15);
      negatives -= (level / 15);
    }
  if ((objIndex->extra_flags & ITEM_ANTI_GOOD) != 0)
    {
      level -= (level / 20);
      negatives -= (level / 20);
    }
  if ((objIndex->extra_flags & ITEM_ANTI_EVIL) != 0)
    {
      level -= (level / 20);
      negatives -= (level / 20);
    }
  if ((objIndex->extra_flags & ITEM_ANTI_NEUTRAL) != 0)
    {
      level -= (level / 20);
      negatives -= (level / 20);
    }
  if ((objIndex->extra_flags & ITEM_NOREMOVE) != 0)
    {
      level -= (level / 15);
      negatives -= (level / 15);
    }
  if (((objIndex->extra_flags & ITEM_INVENTORY) != 0) && level > 0)
    {
      level += (level / 10);
      positives += (level / 10);
    }
  if ((objIndex->extra_flags & ITEM_AUTO_ENGRAVE) != 0)
    {
      level -= (level / 15);
      negatives -= (level / 15);
    }

  if ((objIndex->wear_flags & ITEM_WEAR_TAKE) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_FINGER) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_NECK) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_BODY) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_HEAD) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_LEGS) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_FEET) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_HANDS) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_ARMS) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_SHIELD) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_ABOUT) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_WAIST) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_WRIST) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_WIELD) != 0)
    wearflags++;
  if ((objIndex->wear_flags & ITEM_WEAR_HOLD) != 0)
    wearflags++;

  if (wearflags > 3)
    spew_out ("Following object has TOO MANY WEAR FLAGS!!\n");
  /*spew_out("Level: %d Neg+Pos+1: %d Negatives: %d Positives: %d\n",level,negatives+positives+1,negatives,positives); */
  level = negatives + positives + 1;
/*  spew_out ("True level: %d Negatives: %d\n", truelevel, negatives); */

  if (level > 0)
    data.estlevel = level;
  else
    data.estlevel = 1;
  data.positives = positives;
  data.negatives = negatives;
  data.truelevel = truelevel;

  if (((negatives * -1) > (0.5 * truelevel)) && truelevel > 5)
    {
      spew_out ("Following item has too many flags which reduce estimated level\n");
      spew_out ("Positive modifiers: %3d Negative modifiers: %3d\n", positives, negatives);
    }
  return data;
}

