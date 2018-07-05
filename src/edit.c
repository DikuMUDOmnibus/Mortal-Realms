#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "merc.h"

extern int 	top_mob_index;
extern int 	top_reset;
extern int      top_ed;


char *  const   mprog_flags [] =
{
"act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
"allgreet", "give", "bribe", "range", "social", "kill", "group_greet", "time"
};

char *  const   area_flags[] = 
{
"nodebug", "noteleport", "nogohome", "norecall", "nocastle", "norip", 
"freequit"
};
char *  const   ex_flags [] =
{
"isdoor", "closed", "locked", "hidden", "rip", "pickproof", "bashproof", 
"magicproof", "bashed", "unbarred", "backdoor", "clan_backdoor"
};

char *  const   r_flags [] =
{
"dark", "smoke", "nomob", "indoors", "hallucinate", "nogohome", "clan_donation",
"unused", "bank", "private", "safe", "solitary", "petshop", "norecall",
"rip", "block", "no_save", "morgue", "temple", "god_lo", "god_hi",
"allow_0", "allow_1", "allow_2", "castle", "entrance",
"prototype", "note_board", "nocastle", "norip","",""
};

char *  const   o_flags [] =
{
"glow", "hum", "dark", "lock", "evil", "invis", "magic", "nodrop", "bless",
"antigood", "antievil", "antineutral", "noremove", "inventory",
"levelrent", "notvalid", "autoengrave", "forgery", "prototype","",
"","","","","","","","","","","",""
};

char *  const   act_flags [] =
{
"npc", "sentinel", "scavenger", "r1", "r2", "aggressive", "stayarea",
"wimpy", "pet", "train", "practice", "weak", "smart", "onefight",
"noorder", "ride", "body", "race", "undead", "elemental",
"clanguard", "clanhealer", "prototype","","","","","","","","",""
};

char *  const   a_flags [] =
{
"blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
"detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
"understand", "poison", "protect_evil","protect_good", "sneak", "hide", "sleep",
"charm", "flying", "pass_door", "stealth", "clear", "hunt",
"tongues", "ethereal", "haste" };

char *  const   plr_flags [] =
{
"npc", "boughtpet", "terminal", "autoexits", "autoloot", "autosac", "blank",
"brief", "repeat", "combine", "prompt", "telnet_ga",
"holylight", "wizinvis", "wiztime", "silence", "outcast", "combat_percent", "notell", "log", "deny", "freeze","thief", "killer", "damage", "split", "quiet",
"pager" ,"chat", "plan","","",""
};

char *  const   wear_locs [] =
{
"light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
"feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
"wield", "hold", "heart", "dual_wield"
};

char *  const   part_flags [] =
{
"head", "mouth", "eye", "torso", "hip", "leg", "arm", "wing", "tail",
"tenticle", "horn", "claw", "hand", "foot"
};

char *  const   o_types [] =
{
"light", "scroll", "wand", "staff", "weapon", "unused", "unused",
"treasure", "armor", "potion", "unused", "furniture", "trash", "unused",
"container", "unused", "drinkcon", "key", "food", "money", "unused", "boat",
"corpse", "corpse_pc", "fountain", "pill", "unused", "unused",
"unused", "ammo"
};

char *  const   w_flags [] =
{
"take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
"shield", "about", "waist", "wrist", "wield", "hold", "heart"
};

char *  const   a_types [] =
{
"none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
"sex", "class", "level", "age", "height", "weight", "mana", "hit", "move",
"gold", "experience", "armor", "hitroll", "damroll", "save_para", "save_rod",
"save_petri", "save_breath", "save_spell"
};

char *flag_string( int bitvector, char * const flagarray[] )
{
    static char buf[MAX_STRING_LENGTH];
    int x;

    buf[0] = '\0';
    for ( x = 0; x < 32 ; x++ )
      if ( IS_SET( bitvector, 1 << x ) )
      {
        strcat( buf, flagarray[x] );
        strcat( buf, " " );
      }
    if ( (x=strlen( buf )) > 0 )
      buf[--x] = '\0';

    return buf;
}

int get_dir (char *txt)
{
    int edir;
    char c1;

    c1 = txt[0];
    if ( c1 == '\0' )
      return 0;
    edir = 0;
    switch ( c1 )
    {
          case 'n': case '0':  edir = 0; break; /* north */ 
          case 'e': case '1':  edir = 1; break; /* east  */
          case 's': case '2':  edir = 2; break; /* south */ 
          case 'w': case '3':  edir = 3; break; /* west  */
          case 'u': case '4':  edir = 4; break; /* up    */
          case 'd': case '5':  edir = 5; break; /* down  */
    }
    return edir;
} 

bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	int vnum = room->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	    return FALSE;
	if ( get_trust( ch ) >= MAX_LEVEL )
	    return TRUE;
	if ( !IS_SET( room->room_flags, ROOM_PROTOTYPE) )
	{
	    send_to_char( "You cannot modify this room.\n\r", ch );
	    return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	    send_to_char( "You must have an assigned area to modify this room.\n\r", ch );
	    return FALSE;
	}
	if ( vnum >= pArea->low_r_vnum
	&&   vnum <= pArea->hi_r_vnum )
	    return TRUE;

	send_to_char( "That room is not in your allocated range.\n\r", ch );
	return FALSE;
}

bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj )
{
	int vnum = obj->pIndexData->vnum;
	AREA_DATA *pArea;
	
	if ( IS_NPC( ch ) )
	    return FALSE;
	if ( get_trust( ch ) >= MAX_LEVEL )
	    return TRUE;
	if ( !IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	{
	    send_to_char( "You cannot modify this object.\n\r", ch );
	    return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	    send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
	    return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum
	&&   vnum <= pArea->hi_o_vnum )
	    return TRUE;

	send_to_char( "That object is not in your allocated range.\n\r", ch );
	return FALSE;
}

bool can_oedit( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
	int vnum = obj->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	    return FALSE;
	if ( get_trust( ch ) >= MAX_LEVEL )
	    return TRUE;
	if ( !IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	{
	    send_to_char( "You cannot modify this object.\n\r", ch );
	    return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	    send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
	    return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum
	&&   vnum <= pArea->hi_o_vnum )
	    return TRUE;

	send_to_char( "That object is not in your allocated range.\n\r", ch );
	return FALSE;
}


bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob )
{
	int vnum;
	AREA_DATA *pArea;

        if ( ch == mob && !REAL_GAME)
	  return TRUE;	
	if ( !IS_NPC(mob) )
	{
	     if ( ch->level >= MAX_LEVEL )
		return TRUE;
	     else
		send_to_char( "You can't do that.\n\r", ch );
	     return FALSE;
	}

	vnum = mob->pIndexData->vnum;

	if ( IS_NPC( ch ) )
	    return FALSE;
	if ( get_trust( ch ) >= MAX_LEVEL )
	    return TRUE;
	if ( !IS_SET(mob->act, ACT_PROTOTYPE) )
	{
	    send_to_char( "You cannot modify this mobile.\n\r", ch );
	    return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	    send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
	    return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum
	&&   vnum <= pArea->hi_m_vnum )
	    return TRUE;

	send_to_char( "That mobile is not in your allocated range.\n\r", ch );
	return FALSE;
}

bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob )
{
	int vnum = mob->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC(ch) )
	    return FALSE;
	if ( get_trust(ch) >= MAX_LEVEL )
	    return TRUE;
	if ( !IS_SET(mob->act, ACT_PROTOTYPE) )
	{
	    send_to_char( "You cannot modify this mobile.\n\r", ch );
	    return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	    send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
	    return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum
	&&   vnum <= pArea->hi_m_vnum )
	    return TRUE;

	send_to_char( "That mobile is not in your allocated range.\n\r", ch );
	return FALSE;
}

int get_areaflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !strcasecmp( flag, area_flags[x] ) )
        return x;
    return -1;
}

int get_exflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(ex_flags) / sizeof(ex_flags[0])); x++ )
      if ( !strcasecmp(flag, ex_flags[x]) )
        return x;
    return -1;
}
int get_wearloc( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(wear_locs) / sizeof(wear_locs[0])); x++ )
      if ( !strcasecmp(type, wear_locs[x]) )
        return x;
    return -1;
}
int get_wflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(w_flags) / sizeof(w_flags[0])); x++ )
      if ( !strcasecmp(flag, w_flags[x]) )
        return x;
    return -1;
}

int get_rflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(r_flags) / sizeof(r_flags[0])); x++ )
      if ( !strcasecmp(flag, r_flags[x]) )
        return x;
    return -1;
}

int get_mpflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(mprog_flags) / sizeof(mprog_flags[0])); x++ )
      if ( !strcasecmp(flag, mprog_flags[x]) )
        return x;
    return -1;
}

int get_oflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(o_flags) / sizeof(o_flags[0])); x++ )
      if ( !strcasecmp(flag, o_flags[x]) )
        return x;
    return -1;
}

int get_otype( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(o_types) / sizeof(o_types[0])); x++ )
      if ( !strcasecmp(type, o_types[x]) )
        return x;
    return -1;
}

int get_partflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(part_flags) / sizeof(part_flags[0])); x++ )
      if ( !strcasecmp(flag, part_flags[x]) )
        return x;
    return -1;
}


int get_actflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(act_flags) / sizeof(act_flags[0])); x++ )
      if ( !strcasecmp(flag, act_flags[x]) )
        return x;
    return -1;
}

int get_atype( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(a_types) / sizeof(a_types[0])); x++ )
      if ( !strcasecmp (type, a_types[x]) )
        return x;
    return -1;
}

int get_aflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(a_flags) / sizeof(a_flags[0])); x++ )
      if ( !strcasecmp(flag, a_flags[x]) )
        return x;
    return -1;
}


int get_plrflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(plr_flags) / sizeof(plr_flags[0])); x++ )
      if ( !strcasecmp(flag, plr_flags[x]) )
        return x;
    return -1;
}

EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
          if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
        ed->keyword     = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
    {
          if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}
EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
          if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword     = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
          if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}
EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
          if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword     = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
          if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

void start_editing( CHAR_DATA *ch, char *data )
{
        EDITOR_DATA *edit;
        sh_int lines, size, lpos;
        char c;
        if ( !ch->desc )
        {
           bug( "Fatal: start_editing: no desc", 0 );
           return;
        }

        ch->desc->connected = CON_EDITING;
        send_to_combat_char( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\n\r", ch );
        send_to_combat_char( "-----------------------------------------------------------------------\n\r", ch );
        write_to_buffer( ch->desc, "> ", 1000000 );
        if ( ch->editor )
          stop_editing( ch );
        CREATE( edit, EDITOR_DATA, 1);
        edit->numlines = 0;
        edit->on_line  = 0;
        edit->size     = 0;
        size = 0;  lpos = 0;  lines = 0;
        if ( !data )
            bug("editor: data is NULL!\n\r",0);
        else
        for ( ;; )
        {
           c = data[size++];
           if ( c == '\0' )
           {
                edit->line[lines][lpos] = '\0';
                break;
           }
           else
           if ( c == '\r' );
           else
           if ( c == '\n' || lpos > 78)
           {
                edit->line[lines][lpos] = '\0';
                lines++;
                lpos = 0;
           }
           else
             edit->line[lines][lpos++] = c;
           if ( lines >= 49 || size > 4096 )
           {
                edit->line[lines][lpos] = '\0';
                break;
           }
        }
        edit->numlines = lines;
        edit->size = size;
        edit->on_line = lines;
        ch->editor = edit;
}

char *copy_buffer( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   char tmp[100];
   sh_int x, len;

   if ( !ch )
   {
        bug( "copy_buffer: null ch", 0 );
        return STRALLOC( "" );
   }

   if ( !ch->editor )
   {
        bug( "copy_buffer: null editor", 0 );
        return STRALLOC( "" );
   }

   buf[0] = '\0';
   for ( x = 0; x < ch->editor->numlines; x++ )
   {
      strcpy( tmp, ch->editor->line[x] );
      smash_tilde( tmp );
      len = strlen(tmp);
      if ( tmp && tmp[len-1] == '~' )
        tmp[len-1] = '\0';
      else
        strcat( tmp, "\n\r" );
      strcat( buf, tmp );
   }
   return STRALLOC( buf );
}
void stop_editing( CHAR_DATA *ch )
{
    DISPOSE( ch->editor);
    ch->editor = NULL;
    if (ch->vt100!=0)
     ch_printf(ch, "Done.\n\r\0338\033[0K");
    else
     ch_printf(ch, "Done.\n\r");
    ch->substate  = SUB_NONE;
    if ( !ch->desc )
    {
        bug( "Fatal: stop_editing: no desc", 0 );
        return;
    }
    ch->desc->connected = CON_PLAYING;
}


/*
 * Simple but nice and handle line editor.                     Martin 
 */
void edit_buffer( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    EDITOR_DATA *edit;
    char cmd[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    sh_int x, line, max_buf_lines;
    bool save;

    if ( (d = ch->desc) == NULL )
    {
        send_to_combat_char( "You have no descriptor.\n\r", ch );
        return;
    }

   if ( d->connected != CON_EDITING )
   {
        send_to_combat_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
        return;
   }

   if ( !ch->editor )
   {
        send_to_combat_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: null editor", 0 );
        d->connected = CON_PLAYING;
        return;
   }

   edit = ch->editor;
   save = FALSE;
   max_buf_lines = 50;

   if ( argument[0] == '/' || argument[0] == '\\' )
   {
        one_argument( argument, cmd );
        if ( !strcasecmp( cmd+1, "?" ) )
        {
            send_to_combat_char( "Editing commands\n\r---------------------------------\n\r", ch );
            send_to_combat_char( "/l              list buffer\n\r",    ch );
            send_to_combat_char( "/c              clear buffer\n\r",   ch );
            send_to_combat_char( "/d [line]       delete line\n\r",    ch );
            send_to_combat_char( "/g <line>       goto line\n\r",      ch );
            send_to_combat_char( "/i <line>       insert line\n\r",    ch );
            send_to_combat_char( "/r <old> <new>  global replace\n\r", ch );
            send_to_combat_char( "/a              abort editing\n\r",  ch );
            send_to_combat_char( "/s              save buffer\n\r",ch );
            send_to_combat_char( "/! <command>    execute command (do not use another editing command)\n\r",  ch );
            write_to_buffer( ch->desc, "> ", 1000000 );
            return;
        }
        if ( !strcasecmp( cmd+1, "c" ) )
        {
            memset( edit, '\0', sizeof(EDITOR_DATA) );
            edit->numlines = 0;
            edit->on_line   = 0;
            send_to_combat_char( "Buffer cleared.\n\r", ch );
            write_to_buffer( ch->desc, "> ", 1000000 );
            return;
        }
        if ( !strcasecmp( cmd+1, "r" ) )
        {
            char word1[MAX_INPUT_LENGTH];
            char word2[MAX_INPUT_LENGTH];
            char *sptr, *wptr, *lwptr;
            int x, count, wordln, word2ln, lineln;

            sptr = one_argument( argument, word1 );
            sptr = one_argument( sptr, word1 );
            sptr = one_argument( sptr, word2 );
            if ( word1[0] == '\0' || word2[0] == '\0' )
            {
                send_to_combat_char( "Need word to replace, and replacement.\n\r", ch);
                write_to_buffer( ch->desc, "> ", 1000000 );
                return;
            }
            if ( strcmp( word1, word2 ) == 0 )
            {
                send_to_combat_char( "Done.\n\r", ch );
                write_to_buffer( ch->desc, "> ", 1000000 );
                return;
            }
            count = 0;  wordln = strlen(word1);  word2ln = strlen(word2);
            ch_printf( ch, "Replacing all occurrences of %s with %s...\n\r", word1, word2 );
            for ( x = edit->on_line; x < edit->numlines; x++ )
            {
                lwptr = edit->line[x];
                while ( (wptr = strstr( lwptr, word1 )) != NULL )
                {
                    sptr = lwptr;
                    lwptr = wptr + wordln;
                    sprintf( buf, "%s%s", word2, wptr + wordln );
                    lineln = wptr - edit->line[x] - wordln;
                    ++count;
                    if ( strlen(buf) + lineln > 79 )
                    {
                        lineln = UMAX(0, (79 - strlen(buf)));
                        buf[lineln] = '\0';
                        break;
                    }
                    else
                        lineln = strlen(buf);
                    buf[lineln] = '\0';
                    strcpy( wptr, buf );
                }
            }
            ch_printf( ch, "Found and replaced %d occurrence(s).\n\r", count );
            write_to_buffer( ch->desc, "> ", 1000000 );
            return;
        }

        if ( !strcasecmp( cmd+1, "i" ) )
        {
            if ( edit->numlines >= max_buf_lines )
            {
                send_to_combat_char( "Buffer is full.\n\r", ch );
                write_to_buffer( ch->desc, "> ", 1000000 );
            }
            else
            {
                if ( argument[2] == ' ' )
                  line = atoi( argument + 2 ) - 1;
                else
                  line = edit->on_line;
                if ( line < 0 )
                  line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                {
                  send_to_combat_char( "Out of range.\n\r", ch );
                  write_to_buffer( ch->desc, "> ", 1000000 );
                }
                else
                {
                  for ( x = ++edit->numlines; x > line; x-- )
                        strcpy( edit->line[x], edit->line[x-1] );
                  strcpy( edit->line[line], "" );
                  send_to_combat_char( "Line inserted.\n\r", ch );
        	  write_to_buffer( ch->desc, "> ", 1000000 );
                }
            }
            return;
        }
        if ( !strcasecmp( cmd+1, "d" ) )
        {
            if ( edit->numlines == 0 )
            {
                send_to_combat_char( "Buffer is empty.\n\r", ch );
                write_to_buffer( ch->desc, "> ", 1000000 );
            }
            else
            {
                if ( argument[2] == ' ' )
                  line = atoi( argument + 2 ) - 1;
                else
                  line = edit->on_line;
                if ( line < 0 )
                  line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                {
                  send_to_combat_char( "Out of range.\n\r", ch );
                  write_to_buffer( ch->desc, "> ", 1000000 );
                }
                else
                {
                  if ( line == 0 && edit->numlines == 1 )
                  {
                        memset( edit, '\0', sizeof(EDITOR_DATA) );
                        edit->numlines = 0;
                        edit->on_line   = 0;
                        send_to_combat_char( "Line deleted.\n\r", ch );
        		write_to_buffer( ch->desc, "> ", 1000000 );
                        return;
                  }
                  for ( x = line; x < (edit->numlines - 1); x++ )
                        strcpy( edit->line[x], edit->line[x+1] );
                  strcpy( edit->line[edit->numlines--], "" );
                  if ( edit->on_line > edit->numlines )
                    edit->on_line = edit->numlines;
                  send_to_combat_char( "Line deleted.\n\r", ch );
        	  write_to_buffer( ch->desc, "> ", 1000000 );
                }
            }
            return;
        }
        if ( !strcasecmp( cmd+1, "g" ) )
        {
            if ( edit->numlines == 0 )
            {
                send_to_combat_char( "Buffer is empty.\n\r", ch );
        	write_to_buffer( ch->desc, "> ", 1000000 );
            }
            else
            {
                if ( argument[2] == ' ' )
                  line = atoi( argument + 2 ) - 1;
                else
                {
                    send_to_combat_char( "Goto what line?\n\r", ch );
         	    write_to_buffer( ch->desc, "> ", 1000000 );
                    return;
                }
                if ( line < 0 )
                  line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                {
                  send_to_combat_char( "Out of range.\n\r", ch );
        	  write_to_buffer( ch->desc, "> ", 1000000 );
                }
                else
                {
                  edit->on_line = line;
                  ch_printf( ch, "(On line %d)\n\r", line+1 );
        	  write_to_buffer( ch->desc, "> ", 1000000 );
                }
            }
            return;
        }
        if ( !strcasecmp( cmd+1, "l" ) )
        {
            if ( edit->numlines == 0 )
            {
              send_to_combat_char( "Buffer is empty.\n\r", ch );
              write_to_buffer( ch->desc, "> ", 1000000 );
            }
            else
            {
              send_to_combat_char( "------------------\n\r", ch );
              for ( x = 0; x < edit->numlines; x++ )
               {
                 ch_printf( ch, "%c%2d> %s\n\r", x==edit->on_line?'*':' ', x+1, edit->line[x] );
               }
              send_to_combat_char( "------------------\n\r", ch );
              write_to_buffer( ch->desc, "> ", 1000000 );
            }
            return;
        }
        if ( !strcasecmp( cmd+1, "a" ) )
        {
            send_to_combat_char( "\n\rAborting... ", ch );
            stop_editing( ch );
            return;
        }
        if ( !strcasecmp( cmd+1, "!" ) )
        {
            DO_FUN *last_cmd;
            int substate = ch->substate;

            last_cmd = ch->last_cmd;
            ch->substate = SUB_RESTRICTED;
            interpret(ch, argument+3);
            ch->substate = substate;
            ch->last_cmd = last_cmd;
            write_to_buffer( ch->desc, "> ", 1000000 );
            return;
        }

        if ( !strcasecmp( cmd+1, "s" ) )
        {
            d->connected = CON_PLAYING;
            if ( !ch->last_cmd )
              return;
            (*ch->last_cmd) ( ch, "" );
            return;
        }
   }

   x=edit->on_line;

   if ( edit->size + strlen(argument) + 1 >= (MAX_STRING_LENGTH/2) - 1 )
       {
        send_to_combat_char( "Your buffer is full.\n\r", ch );
       }
   else
   {
/* Added line justification for annoying and loud players *grin* 
   Martin 23/7/98*/
        char * p1,*p2;
        int length=0;
        strcpy( buf, justify(argument) ); 
         p1=&buf[0];
         while ((p2 = strstr(p1, "\r")) !=NULL)
         {
          while (p1<p2)
          {
           if (*p1=='\n') break;

           length = char_apd_max(edit->line[edit->on_line], p1++, length, 
                                  MAX_STRING_LENGTH);  
          } 
          length=0;
          edit->on_line++;
          if ( edit->on_line > edit->numlines )
           edit->numlines++;
          if ( edit->numlines > max_buf_lines )
          {
            edit->numlines = max_buf_lines;
            send_to_combat_char( "Buffer full.\n\r", ch );
            save = TRUE;
            break;
          }
          p1=p2+1;
         }
         strcpy(edit->line[edit->on_line++], p1);

         if ( edit->on_line > edit->numlines )
           edit->numlines++;

        if ( edit->numlines > max_buf_lines )
        {
          edit->numlines = max_buf_lines;
          send_to_combat_char( "Buffer full.\n\r", ch );
          /*write_to_buffer( ch->desc, "> ", 1000000 );*/
          save = TRUE;
        }
     } 

   if ( save )
   {
    d->connected = CON_PLAYING;
    if ( !ch->last_cmd )
      return;
    (*ch->last_cmd) ( ch, "" );
    return;
   }
  if (ch->vt100!=0)
  {
    int i;
    write_to_buffer( d, "\033[0K", 1000000); 
    for (i=x;i<edit->on_line;i++)
     ch_printf( ch, "> %s\n\r", edit->line[i] );
  }
  write_to_buffer( ch->desc, "> ", 1000000 );
}


/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
    int len;
    char last;
    char *strptr;

    strptr = str;

    len  = strlen( str );
    if ( len )
      last = strptr[len-1];
    else
      last = '\0';

    for ( ; *str != '\0'; str++ )
    {
        if ( *str == '~' )
            *str = '-';
    }
    if ( len )
      strptr[len-1] = last;

    return;
}

/*
 * Help editor 
 */
void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }
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
          return;
    }
    if ( (pHelp = get_help(ch, argument)) == NULL )
    {
        send_to_char( "That help does not exist.\n\r", ch );
        return;
    }
    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

void do_mcreate( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *mob;
    RESET_DATA *pReset;
    AREA_DATA *pArea;
    int vnum;
    int iHash,mob_cost;
    bool fullcont=FALSE, clanleader=TRUE;
    char arg1[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) && ch->pcdata->clan!=NULL &&
      ( !strcasecmp( ch->name, ch->pcdata->clan->leader  )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number1 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number2 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number3 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number4 ) ) )
    {
      clanleader=TRUE;
    }
    else 
      clanleader=FALSE;


    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME) 
        || ch->level==MAX_LEVEL)
      fullcont=TRUE;

   if (!fullcont)
   {
    argument = one_argument( argument, arg1 );
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
    /* make sure they can afford it */
    mob_cost=COST_OF_CREATE;
    mob_cost=((IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100)*mob_cost)/100*1000;

    if(ch->gold<mob_cost)
      {
      ch_printf(ch ,"It will cost you $%d to do an mcreate.\n\r",mob_cost);
      return;
      }

    if(ch->pcdata->castle->num_mobiles >= ch->level/3+2)
      {
      ch_printf(ch,"You cannot create anymore mobiles at your level.\n\r");
      return;
      }
    if (clanleader && !strcasecmp( arg1, "clanguard" ) )
     {
	if (ch->pcdata->clan->guard == 0)
	{
 	 ch_printf(ch, "You have not specified a mob to be your guard.\n\r");
	 return;
	}
	vnum = ch->pcdata->clan->guard;
	pMobIndex = get_mob_index(vnum);
	ch->pcdata->clan->num_guards++;
     }
    else if (clanleader && !strcasecmp( arg1, "clanhealer" ) )
     {
	if (ch->pcdata->clan->healer == 0)
	{
 	 ch_printf(ch, "You have not specified a mob to be your healer.\n\r");
	 return;
	}
	vnum = ch->pcdata->clan->healer;
	pMobIndex = get_mob_index(vnum);
	ch->pcdata->clan->num_healers++;
      }
     else
     {
    /* find an empty vnum */
    for(vnum=20000;get_mob_index(vnum)!=NULL&&vnum<27000;vnum++);
    if(vnum>=27000)
      {
      send_to_char("I'm sorry, you can't create a mobile, tell the Implementors.\n\r",ch);
      return;
      }

    CREATE( pMobIndex, MOB_INDEX_DATA, 1);
    pMobIndex->creator_pvnum        = ch->pcdata->pvnum;
    pMobIndex->vnum                 = vnum;
    pMobIndex->area                 = ch->in_room->area;
    pMobIndex->player_name          = STRALLOC("golem wafer");
    pMobIndex->short_descr          = STRALLOC("A wafer golem");
    pMobIndex->long_descr           = STRALLOC("A wafer golem stands here, looking useless.");
    pMobIndex->description          = STRALLOC(
     "The wafer golem looks *very* dumb.");
    pMobIndex->act                  = ACT_IS_NPC|ACT_STAY_AREA|ACT_WEAK;
    pMobIndex->affected_by          = 0;
    pMobIndex->pShop                = NULL;
    pMobIndex->alignment            = ch->alignment;
    pMobIndex->level                = 0;
    pMobIndex->max_mobs             = 0;
    pMobIndex->armor                = 0;
    pMobIndex->body_parts           = 0;
    pMobIndex->attack_parts         = 0;
    pMobIndex->hitnodice            = 1;
    pMobIndex->hitsizedice          = 1;
    pMobIndex->hitplus              = 0;
    pMobIndex->damnodice            = 1;
    pMobIndex->damsizedice          = 1;
    pMobIndex->damplus              = 0;
    pMobIndex->gold                 = 0;
    pMobIndex->race                 = number_range( 0, MAX_RACE-1);
    pMobIndex->position             = POS_STANDING;
    pMobIndex->corrected            = TRUE;
    pMobIndex->sex                  = SEX_NEUTRAL;
    pMobIndex->mobprogs             = NULL;

    iHash                           = vnum % MAX_KEY_HASH;
    pMobIndex->next                 = mob_index_hash[iHash];
    mob_index_hash[iHash]           = pMobIndex;
    mob_index[vnum]                 = pMobIndex;
    }
    if (vnum > pMobIndex->area->hi_m_vnum)
      pMobIndex->area->hi_m_vnum = vnum;
    if (vnum < pMobIndex->area->low_m_vnum)
      pMobIndex->area->low_m_vnum = vnum;
    top_mob_index++;
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;

    /* make the mobile reset in this room */
    CREATE(pReset, RESET_DATA, 1);
    pReset->command = 'M';
    pReset->arg0    = '0';
    pReset->arg1    = vnum;
    pReset->arg2    = '1';
    pReset->arg3    = ch->in_room->vnum;
    pArea = room_index[20000]->area;
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    top_reset++;

    /* reset the mob into this room */
    mob = create_mobile( pMobIndex );
    mob->reset=pReset;
    mob->language             = pMobIndex->race;
    mob->class                = number_range( 0, MAX_CLASS-1);
    char_to_room( mob, room_index[ch->in_room->vnum] );

    /* let them know it happened */
    act("A being takes form in the center of the room.",ch,NULL,NULL,TO_ROOM);
    act("A being takes form in the center of the room.",ch,NULL,NULL,TO_CHAR);

    /* modify the appropriate character and castle data */
    ch->gold-=mob_cost;
    ch->pcdata->castle->cost+=mob_cost/1000;
    castle_needs_saving=TRUE;
    ch->pcdata->castle->num_mobiles+=1;
   }
   else
   {
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA      *pMobIndex;
    CHAR_DATA           *mob;
    int                  vnum, cvnum;
    AREA_DATA *pArea;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage:  mcreate <vnum> [cvnum] <mobile name>\n\r", ch );
        return;
    }
    if ( vnum < 1 || vnum > MAX_VNUM )
    {
        send_to_char( "Vnum out of range.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
        cvnum = 0;

    if ( get_mob_index( vnum ) )
    {
        send_to_char( "A mobile with that number already exists.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;

    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
          send_to_char( "You must have an assigned area to create mobiles.\n\r",
 ch );
          return;
    }
    if ( vnum < pArea->low_m_vnum
     ||   vnum > pArea->hi_m_vnum )
    {
     send_to_char( "That number is not in your allocated range.\n\r", ch );
     return;
    }

    pMobIndex = make_mobile( vnum, cvnum, argument );
    if ( !pMobIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string( "do_mcreate: make_mobile failed." );
        return;
    }
    mob = create_mobile( pMobIndex );
    SET_BIT( mob->act, ACT_PROTOTYPE );
    pMobIndex->act = mob->act;
    char_to_room( mob, ch->in_room );
    act( "$n waves $s arms about, and $N appears at $s command!", ch,
 NULL, mob, TO_ROOM );
    ch_printf_color( ch, "You wave your arms about, and %s appears at your command!\n\rMobVnum:  %d   Keywords:  %s\n\r",
        pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name );
   }
   return;
}

void do_ocreate( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA      *pObjIndex;
    OBJ_DATA            *obj;
    int                  vnum, cvnum;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage:  ocreate <vnum> [copy vnum] <item name>\n\r", ch );
        return;
    }
    if ( vnum < 1 || vnum > MAX_VNUM )
    {
        send_to_char( "Vnum out of range.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
        cvnum = 0;

    if ( get_obj_index( vnum ) )
    {
        send_to_char( "An object with that number already exists.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;
    if ( get_trust( ch ) < MAX_LEVEL )
    {
        AREA_DATA *pArea;

        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
          send_to_char( "You must have an assigned area to create objects.\n\r",
 ch );
          return;
        }
        if ( vnum < pArea->low_o_vnum
        ||   vnum > pArea->hi_o_vnum )
        {
          send_to_char( "That number is not in your allocated range.\n\r", ch );
          return;
        }
    }

    pObjIndex = make_object( vnum, cvnum, argument );
    if ( !pObjIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string( "do_ocreate: make_object failed." );
        return;
    }
    obj = create_object( pObjIndex, get_trust(ch) );
    SET_BIT(obj->extra_flags,ITEM_PROTOTYPE);
    obj_to_char( obj, ch );
    act( "$n makes arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
    ch_printf_color( ch, "You make arcane gestures, and open your hands to reveal %s!\n\rObjVnum:  %d   Keywords:  %s\n\r",
        pObjIndex->short_descr,pObjIndex->vnum, pObjIndex->name );
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value, delta, mset_cost=0;
    bool fullcont=FALSE, clanleader=FALSE;

    if (!IS_NPC(ch) && ch->pcdata->clan!=NULL &&
      ( !strcasecmp( ch->name, ch->pcdata->clan->leader  )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number1 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number2 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number3 )
    ||   !strcasecmp( ch->name, ch->pcdata->clan->number4 ) ) )
    {
      clanleader=TRUE;
    }
    else 
      clanleader=FALSE;

    /* fullcont stands for Full Control. It determines what privileges the
       player has with regard to editing...Martin*/

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't mset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }
   switch (ch->substate)
   {
        default: break;

        case SUB_MOB_DESC:
           if (!ch->dest_buf)
           {
             send_to_char("Fatal error: report to Chaste.\n\r", ch);
             bug( "do_castle: sub_mob_desc: NULL ch->dest_buf", 0);
             ch->substate = SUB_NONE;
             return;
           }
           victim = ch->dest_buf;

           if(victim->description!=victim->pIndexData->description)
            STRFREE (victim->description );
           STRFREE (victim->pIndexData->description );
           victim->pIndexData->description = STRALLOC(copy_buffer (ch ) );
           victim->description = STRALLOC( copy_buffer (ch ));
           stop_editing( ch );
           ch->substate = SUB_NONE;
           return;
   }

    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME) 
        || ch->level==MAX_LEVEL)
      fullcont=TRUE;

    if( strlen( argument) > 160 )
      *(argument+160)='\0';
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

   if (!fullcont)
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
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    { 
        if (fullcont)
        {
         send_to_char( "Syntax: mset <victim> <field>  <value>\n\r",     ch );
         send_to_char( "or:     mset <victim> <string> <value>\n\r",     ch );
         send_to_char( "\n\r",                                           ch );
         send_to_char( "Field being one of:\n\r",                        ch );
         send_to_char( "  str int wis dex con sex class level\n\r",      ch );
         send_to_char( "  gold hp mana move practice align\n\r",         ch );
         send_to_char( "  thirst drunk full exp language race",
              ch );
         send_to_char( "\n\r",                                           ch );
         send_to_char( "String being one of:\n\r",                       ch );
         send_to_char( "  name short long description title spec\n\r",   ch ); 
        }
        else
        {
         send_to_char( "Syntax: mset <victim> <field>  <value>\n\r",ch );
         send_to_char( "    or: mset <victim> <string> <value>\n\r",ch );
         send_to_char( "\n\r",                                           ch );
         send_to_char( "Field being one of:\n\r",                        ch );
         send_to_char( "  sex level race act tag\n\r",                   ch );
         if (clanleader)
           send_to_char("   clanguard clanhealer\n\r", 			 ch );
         send_to_char( "\n\r",                                           ch );
         send_to_char( "String being one of:\n\r",                       ch );
         send_to_char( "  name short long desc\n\r",        ch );
        }

        return;
    }

   if (!fullcont)
   {  
    /* determine cost */
 
    mset_cost=COST_OF_SET;
    mset_cost=((IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100)*mset_cost)/100*1000;

    /* determine if they can afford it */
    if(ch->gold<mset_cost)
      {
      ch_printf(ch,"It will cost you $%d to do an mset.\n\r",mset_cost);
      return;
      }
   }
    if( ch->level < 98 )
    {

      if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      {
        send_to_char( "You cannot mset that.\n\r", ch );
        return;
      }
      if( !IS_NPC(victim) || victim->pIndexData->area!=ch->in_room->area )
      {
        send_to_char( "You cannot mset that.\n\r", ch );
        return;
      }
    }
    else
      if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
        send_to_char( "They aren't here.\n\r", ch );
        return;
      }


    if (!fullcont && (!IS_NPC(victim) || victim->pIndexData->creator_pvnum !=ch->pcdata->pvnum ))
     {
      ch_printf(ch, "You can't work on '%s' here!\n\r", arg1);
      return;
     }
    if (!fullcont && ( IS_SET(victim->act, ACT_CLAN_GUARD) ||
		       IS_SET(victim->act, ACT_CLAN_HEALER) ))
     {
      ch_printf(ch, "You cannot edit your clan guards or healers once they've been selected.\n\rMake a new mobile and set it as the new healer or guard.\n\rNote: The current clan guard or healer will be removed from the game.\n\r");
      return;
     }

    if ( !strcasecmp( arg2, "sex" ) )
    {
       if(!get_bitvector_value(arg3,&value,"SEX_"))
        {
         send_to_char("You must specify a sex for it!\n\r",ch);
         list_bitvectors(ch,"SEX_");
         return;
        }

       /*if ( value < 0 || value > 2 )
       {
           send_to_char( "Sex range is 0 to 2.\n\r", ch );
           return;
       }*/
       victim->sex = value;
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }

    else if ( !strcasecmp( arg2, "level" ) )
    {
        
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
        value = is_number( arg3 ) ? atol( arg3 ) : -1;

        if ( !fullcont && ( value < 0 || value > (ch->level/2) ))
        {
        send_to_char( "Level range is 0 to LEVEL/2.\n\r", ch );
        return;
        }

        if ( value < 0 || value > 150 )
        {
            send_to_char( "Level range is 0 to 150.\n\r", ch );
            return;
        }
        if (!fullcont)
         {
          victim->level = value;
          victim->max_hit = victim->level*victim->level+victim->level+10;
          victim->hit=victim->max_hit;
          victim->pIndexData->damplus = victim->level+5;
          victim->pIndexData->level=victim->level;
          victim->pIndexData->hitplus=victim->max_hit;
         }
        else
         victim->level = value;
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }

    else if ( !strcasecmp( arg2, "race" ) )
    {
     if( !get_bitvector_value( arg3, &value, "RACE_") )
      {
       send_to_char( "You must specify a race for it!\n\r", ch);
       list_bitvectors( ch, "RACE_");
       return;
      }
     victim->race     		= value;
     victim->language 		= victim->race;
     if (IS_NPC(victim))
     { 
      SET_BIT(victim->act,ACT_RACE);
      victim->pIndexData->race	=victim->race;
      victim->pIndexData->act	=victim->act;
     }
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }

    else if (clanleader && !strcasecmp( arg2, "clanguard") )
      { 
  	MOB_INDEX_DATA *oldguard = NULL;
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
	if (ch->pcdata->clan->guard !=0)
	{
 	 send_to_char("Your clan already has a guard.\n\r", ch);
	 return;
	}
 	if (ch->pcdata->clan->guard != 0 && ch->pcdata->clan->guard != victim->pIndexData->vnum)
	{
 	 oldguard = get_mob_index( ch->pcdata->clan->guard);
	 delete_mob( oldguard );
  	}

        SET_BIT(victim->act, ACT_CLAN_GUARD);
        SET_BIT(victim->act, ACT_AGGRESSIVE);
        SET_BIT(victim->affected_by, AFF_DETECT_INVIS);
        SET_BIT(victim->affected_by, AFF_DETECT_HIDDEN);
        SET_BIT(victim->affected_by, AFF_SANCTUARY);

        victim->pIndexData->act		= victim->act;
	victim->pIndexData->affected_by = victim->affected_by;
	victim->pIndexData->level	= victim->level 	= 100;
	victim->pIndexData->armor	= victim->npcdata->armor 	= -500;
	victim->pIndexData->hitnodice	= 10;
	victim->pIndexData->hitsizedice	= 1000;
	victim->pIndexData->hitplus	= 9000;
	victim->pIndexData->damnodice	= victim->npcdata->damnodice     = 10;
	victim->pIndexData->damsizedice	= victim->npcdata->damsizedice   = 10;
	victim->pIndexData->damplus	= victim->npcdata->damplus       = 40;
     	victim->max_hit = dice( victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice) +
			  victim->pIndexData->hitplus;
  	victim->hit = victim->max_hit;
        ch->pcdata->clan->guard=victim->pIndexData->vnum; 
 	ch_printf(ch, "%s is now %s's clan guard.\n\r", victim->short_descr,
		ch->pcdata->clan_name);
	ch->pcdata->clan->num_guards=1;
  	return;
       }
    else if (clanleader && !strcasecmp( arg2, "clanhealer") )
      { 
  	MOB_INDEX_DATA *oldhealer = NULL;
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
	if (ch->pcdata->clan->healer !=0)
	{
 	 send_to_char("Your clan already has a healer.\n\r", ch);
	 return;
	}
 	if (ch->pcdata->clan->healer != 0 && ch->pcdata->clan->healer != victim->pIndexData->vnum)
	{
 	 oldhealer = get_mob_index( ch->pcdata->clan->healer);
	 delete_mob(oldhealer);
  	}

        SET_BIT(victim->act, ACT_CLAN_HEALER);
        SET_BIT(victim->affected_by, AFF_DETECT_INVIS);
        SET_BIT(victim->affected_by, AFF_DETECT_HIDDEN);

        victim->pIndexData->act		= victim->act;
	victim->pIndexData->level	= victim->level 	= 100;
	victim->pIndexData->armor	= victim->npcdata->armor 	= -500;
	victim->pIndexData->hitnodice	= 10;
	victim->pIndexData->hitsizedice	= 1000;
	victim->pIndexData->hitplus	= 9000;
	victim->pIndexData->damnodice	= victim->npcdata->damnodice     = 10;
	victim->pIndexData->damsizedice	= victim->npcdata->damsizedice   = 10;
	victim->pIndexData->damplus	= victim->npcdata->damplus       = 40;
     	victim->max_hit = dice( victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice) +
			  victim->pIndexData->hitplus;
  	victim->hit = victim->max_hit;
   	victim->pIndexData->spec_fun    = victim->spec_fun = spec_lookup ("spec_clan_healer");
        ch->pcdata->clan->healer=victim->pIndexData->vnum; 

 	ch_printf(ch, "%s is now %s's clan healer.\n\r", victim->short_descr,
		ch->pcdata->clan_name);
	ch->pcdata->clan->num_healers=1;
  	return;
       }
    else if( !strcasecmp( arg2, "act") )
      { 
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
       
       if (fullcont)
        {
        if(!get_bitvector_value(arg3,&value,"ACT_"))
         {
          send_to_char("You must specify an act flag to toggle!\n\r",ch);
          list_bitvectors(ch,"ACT_");
          return;
         }
        } 
       else
       {
        if(!get_bitvector_value(arg3,&value,"CACT_"))
         {
          send_to_char("You must specify an act flag to toggle!\n\r",ch);
          list_bitvectors(ch,"CACT_");
          return;
         }
       }
        if(IS_SET(victim->act,value))
         REMOVE_BIT(victim->act,value);
        else
         SET_BIT(victim->act,value);
        victim->pIndexData->act=victim->act;
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
       }

    else if ( !strcasecmp( argument, "tag" ) )
     { /* taging doesn't require a victim name, tags all mobiles w/ "mob" */
       /* put in to allow refering to mobiles that you forgot the name of */
      for(victim=ch->in_room->first_person;victim;victim=victim->next_in_room)
        if(IS_NPC(victim)&&victim->pIndexData->creator_pvnum==ch->pcdata->pvnum)
          {
          strcpy(arg2,victim->pIndexData->player_name);
          strcat(arg2," mob ");
          if(victim->name!=victim->pIndexData->player_name)
            STRFREE (victim->name );
          STRFREE (victim->pIndexData->player_name );
          victim->pIndexData->player_name=STRALLOC(arg2);
          victim->name = STRALLOC(arg2);
          }
      send_to_char("You can now refer to your mobiles in this room as 'mob', '2.mob', etc.\n\r",ch);
      return;
     }

    else if ( !strcasecmp( arg2, "name" ) )
    {
      if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
      if(victim->name!=victim->pIndexData->player_name)
        STRFREE (victim->name );
      STRFREE (victim->pIndexData->player_name );
      victim->pIndexData->player_name=STRALLOC(arg3);
      victim->name = STRALLOC(arg3);
      /*STRFREE (victim->name );
      if( !strcasecmp( arg3, "null") )
       victim->name = QUICKLINK(victim->pIndexData->player_name);
      else
       victim->name = STRALLOC( arg3 );
      return;*/
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }

    else if ( !strcasecmp( arg2, "short" ) )
    {
     if(!IS_NPC(victim) || victim->short_descr!=victim->pIndexData->short_descr)
      STRFREE (victim->short_descr );
     if( !strcasecmp( arg3, "null") )
      {
       if( IS_NPC( victim ) )
        victim->short_descr = QUICKLINK (victim->pIndexData->short_descr);
       else
        victim->short_descr = STRALLOC( "" );
       }
      else
       {
         if (IS_NPC( victim ) )
         {
          STRFREE (victim->pIndexData->short_descr );
          victim->pIndexData->short_descr=STRALLOC(justify(arg3));
         }
        victim->short_descr = STRALLOC(justify(arg3));
       }
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }

    else if ( !strcasecmp( arg2, "long" ) )
    {
       if(!IS_NPC(victim) || victim->long_descr!=victim->pIndexData->long_descr)
          STRFREE (victim->long_descr );
       if( !strcasecmp( arg3, "null") )
        {
         if( IS_NPC( victim ) )
           victim->long_descr = QUICKLINK (victim->pIndexData->long_descr);
         else
           victim->long_descr = STRALLOC( "" );
        }
       else
        {
         if (IS_NPC( victim ) )
         {
          STRFREE (victim->pIndexData->long_descr );
          victim->pIndexData->long_descr=STRALLOC(justify(arg3)); 
         }
         victim->long_descr = STRALLOC(justify(arg3));
        }

/*      STRFREE (victim->long_descr );
        if( !strcasecmp( arg3, "null") )
          {
          if( IS_NPC( victim ) )
            victim->long_descr = victim->pIndexData->long_descr;
          else
            victim->long_descr = STRALLOC( "" );
          }
        else
          victim->long_descr = STRALLOC( arg3 );
        return;
*/
       if (fullcont)
        {
         ch_printf(ch, "Ok.\n\r");
         return;
        }
    }
    else if ( !strcasecmp( arg2, "desc" ) )
      {
       switch( ch->substate)
       {
        default:
           bug( "do_description: illegal substate", 0 );
           return;

        case SUB_RESTRICTED:
           send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
           return;

        case SUB_NONE:
           ch->substate = SUB_MOB_DESC;
           ch->dest_buf = victim;
           start_editing( ch, victim->pIndexData->description);
           return;
     }
    if (fullcont)
     {
      ch_printf(ch, "Ok.\n\r");
      return;
     }
    }
    else if (!fullcont)
      {
      send_to_char("Mset what?  Use help CASTLE for a list of mset options.\n\r",ch);
      return;
      }
    if (!fullcont)
    {
     act("Your eyes go fuzzy for a moment.",ch,NULL,NULL,TO_ROOM);
     act("The contractor comes, takes your money, looks around, pinches the mobile, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);
 
     /* affect changes due to modifications */
     ch->gold-=mset_cost;
     ch->pcdata->castle->cost+=mset_cost/1000;
     castle_needs_saving=TRUE;
     return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
      value = is_number( arg3 ) ? atol( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !strcasecmp( arg2, "str" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 25 )
        {
            send_to_char( "Strength range is 3 to 25.\n\r", ch );
            return;
        }

        victim->pcdata->perm_str = value;
        return;
    }

    if ( !strcasecmp( arg2, "int" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 25 )
        {
            send_to_char( "Intelligence range is 3 to 25.\n\r", ch );
            return;
        }

        victim->pcdata->perm_int = value;
        return;
    }

    if ( !strcasecmp( arg2, "wis" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 25 )
        {
            send_to_char( "Wisdom range is 3 to 25.\n\r", ch );
            return;
        }

        victim->pcdata->perm_wis = value;
        return;
    }

    if ( !strcasecmp( arg2, "dex" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 25 )
        {
            send_to_char( "Dexterity range is 3 to 25.\n\r", ch );
            return;
        }

        victim->pcdata->perm_dex = value;
        return;
    }

    if ( !strcasecmp( arg2, "con" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 25 )
        {
            send_to_char( "Constitution range is 3 to 25.\n\r", ch );
            return;
        }

        victim->pcdata->perm_con = value;
        return;
    }


    if ( !strcasecmp( arg2, "class" ) )
    {
        if ( value < 0 || value >= MAX_CLASS )
        {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Class range is 0 to %d.\n", MAX_CLASS-1 );
            send_to_char( buf, ch );
            return;
        }
        victim->class = value;
        return;
    }
    if ( !strcasecmp( arg2, "language" ) )
    {
          victim->language = value;
          return;
    }


 if ( !strcasecmp( arg2, "exp" ) )
   {
   if ( value < 0 )
     {
     send_to_char( "Exp range is greater than 0.\n\r", ch );
     return;
     }
   victim->exp = value;
   return;
   }

    if ( !strcasecmp( arg2, "gold" ) )
    {
        victim->gold = value;
        return;
    }

    if ( !strcasecmp( arg2, "hp" ) )
    {
        if ( value < 1 || value > 2000000000 )
        {
            send_to_char( "Hp range is 1 to 2 billion hit points.\n\r", ch );
            return;
        }
        delta = value - victim->max_hit;
        victim->max_hit = value;
        victim->actual_max_hit += delta;
        return;
    }

    if ( !strcasecmp( arg2, "mana" ) )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
            return;
        }
        delta = value - victim->max_mana;
        victim->max_mana = value;
        victim->actual_max_mana += delta;
        return;
    }

    if ( !strcasecmp( arg2, "move" ) )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
            return;
        }
        delta = value - victim->max_move;
        victim->max_move = value;
        victim->actual_max_move += delta;
        return;
    }

    if ( !strcasecmp( arg2, "practice" ) )
    {
        if ( value < 0 || value > 1000 )
        {
            send_to_char( "Practice range is 0 to 1000 sessions.\n\r", ch );
            return;
        }
        victim->practice = value;
        return;
    }

    if ( !strcasecmp( arg2, "align" ) )
    {
        if ( value < -1000 || value > 1000 )
        {
            send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
            return;
        }
        victim->alignment = value;
        return;
    }

    if ( !strcasecmp( arg2, "thirst" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Thirst range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        return;
    }

    if ( !strcasecmp( arg2, "drunk" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Drunk range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        return;
    }

    if ( !strcasecmp( arg2, "full" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Full range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        return;
    }



    if ( !strcasecmp( arg2, "title" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        set_title( victim, arg3 );
        return;
    }

    if ( !strcasecmp( arg2, "spec" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        return;
    }

    if ( !strcasecmp( arg2, "castle" ) )
      if ( !IS_NPC(victim) )
        {
        if(victim->pcdata->castle!=NULL)
          {
          DISPOSE(victim->pcdata->castle );
          victim->pcdata->castle=NULL;
          send_to_char("Their castle is cleared.\n\r",ch);
          }
        return;
        }
    if ( !strcasecmp( arg2, "clan" ) )
    {
        CLAN_DATA *clan;

        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( !arg3 || arg3[0] == '\0' )
        {
            /* Crash bug fix, oops guess I should have caught this one :)
             * But it was early in the morning :P --Shaddai
             */
            if ( victim->pcdata->clan == NULL )
                return;
            /* Added a check on immortals so immortals don't take up
             * any membership space. --Shaddai
             */
            if ( !IS_IMMORTAL( victim ) ) {
                --victim->pcdata->clan->members;
                save_clan( victim->pcdata->clan );
            }
            STRFREE( victim->pcdata->clan_name );
            victim->pcdata->clan_name   = STRALLOC( "" );
            victim->pcdata->clan        = NULL;
            return;
        }
        clan = get_clan( arg3 );
        if ( !clan )
        {
           send_to_char( "No such clan.\n\r", ch );
           return;
        }
        if ( victim->pcdata->clan != NULL && !IS_IMMORTAL( victim ) )
        {
                --victim->pcdata->clan->members;
                save_clan( victim->pcdata->clan );
        }
        STRFREE( victim->pcdata->clan_name );
        victim->pcdata->clan_name = QUICKLINK( clan->name );
        victim->pcdata->clan = clan;
        if ( !IS_IMMORTAL( victim ) ) {
           ++victim->pcdata->clan->members;
           save_clan( victim->pcdata->clan );
        }
        return;
    }

  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      send_to_char("Bad parameters to 'mset quest'",ch);
      bug("%u bad parameters to 'mset quest'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits(
          &victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],
          firstBit, len, value );
    send_to_char( "Set.\n\r", ch );
    return;
    }

  if ( !strcasecmp( arg2, "questr" ) )
    {
    int firstBit,len, vnum;

    if(sscanf(arg3,"%d %d %d %d",&vnum, &firstBit,&len,&value)!=4)
      {
      send_to_char("Bad parameters to 'mset questr'",ch);
      bug("%u bad parameters to 'mset questr'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      return;
    if(IS_NPC(victim))
      set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
    else
      set_quest_bits(
          &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
          firstBit, len, value );
    send_to_char( "Set.\n\r", ch );
    return;
    }

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax: oset <object> <field>  <value>\n\r",     ch );
        send_to_char( "or:     oset <object> <string> <value>\n\r",     ch );
        send_to_char( "\n\r",                                           ch );
        send_to_char( "Field being one of:\n\r",                        ch );
        send_to_char( "  value0 value1 value2 value3\n\r",              ch );
        send_to_char( "  extra wear level weight cost timer sactimer\n\r",      ch );
        send_to_char( "\n\r",                                           ch );
        send_to_char( "String being one of:\n\r",                       ch );
        send_to_char( "  name short long ed\n\r",                       ch );
        return;
    }

    if( ch->level < 98 )
      {
      if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
        {
              send_to_char( "You cannot oset that.\n\r", ch );
              return;
        }
      if( obj->pIndexData->area != ch->in_room->area )
        {
              send_to_char( "You cannot oset that.\n\r", ch );
              return;
        }
      }
    else
    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atol( arg3 );

    obj->basic = FALSE;

    /*
     * Set something.
     */
    if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
    {
        obj->value[0] = value;
        return;
    }

    if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
    {
        obj->value[1] = value;
        return;
    }

    if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
    {
        obj->value[2] = value;
        return;
    }

    if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
    {
        obj->value[3] = value;
        return;
    }

    if ( !strcasecmp( arg2, "extra" ) )
    {
        obj->extra_flags = value;
        return;
    }

    if ( !strcasecmp( arg2, "wear" ) )
    {
        obj->wear_flags = value;
        return;
    }

    if ( !strcasecmp( arg2, "level" ) )
    {
        obj->level = value;
        return;
    }

    if ( !strcasecmp( arg2, "weight" ) )
    {
        obj->weight = value;
        return;
    }

    if ( !strcasecmp( arg2, "cost" ) )
    {
        obj->cost = value;
        return;
    }

    if ( !strcasecmp( arg2, "timer" ) )
    {
        obj->timer = value;
        return;
    }

  if ( !strcasecmp( arg2, "sactimer" ) )
    {
          obj->sac_timer = value;
          return;
    }

    if ( !strcasecmp( arg2, "name" ) )
    {
      STRFREE (obj->name );
        if( !strcasecmp( arg3, "null") )
          obj->name = obj->pIndexData->name;
        else
          obj->name = STRALLOC( arg3 );
        return;
    }

    if ( !strcasecmp( arg2, "short" ) )
    {
        STRFREE (obj->short_descr );
        if( !strcasecmp( arg3, "null") )
          obj->short_descr = obj->pIndexData->short_descr;
        else
          obj->short_descr = STRALLOC( arg3 );
        return;
    }

    if ( !strcasecmp( arg2, "long" ) )
    {
        STRFREE (obj->description );
        if( !strcasecmp( arg3, "null") )
          obj->description = obj->pIndexData->description;
        else
          obj->description = STRALLOC( arg3 );
        return;
    }


    if ( !strcasecmp( arg2, "ed" ) )
    {
        EXTRA_DESCR_DATA *ed;

        argument = one_argument( argument, arg3 );
        if ( argument == NULL )
        {
            send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
                ch );
            return;
        }

	CREATE( ed, EXTRA_DESCR_DATA, 1);
        ed->keyword             = STRALLOC( arg3     );
        ed->description=STRALLOC( argument );
	LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        return;
    }
/*
  if ( !strcasecmp( arg2, "quest" ) )
    {
    int firstBit,len,lhsvl;

    if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
      {
      bug("%u bad parameters to 'oset quest'",ch->pcdata->pvnum);
      bug(arg3,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug("%d",value);
      return;
      }
    lhsvl=(lhsvl & obj->obj_quest)|(value << firstBit);
    obj->obj_quest=lhsvl;
    return;
    } */

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int argn, rset_cost=0;
    bool fullcont=FALSE;
    ROOM_INDEX_DATA *location;

    /* fullcont stands for Full Control. It determines what privileges the
       player has with regard to editing...Martin*/

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't rset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }
   switch (ch->substate)
   {
        default: break;

        case SUB_ROOM_DESC:
           if (!ch->dest_buf)
           {
             send_to_char("Fatal error: report to Chaste.\n\r", ch);
             bug( "do_rset: sub_room_desc: NULL ch->dest_buf", 0);
             ch->substate = SUB_NONE;
             return;
           }
           location = ch->dest_buf;
           STRFREE (location->description );
           location->description = copy_buffer( ch );
           stop_editing( ch );
           return;
       }

    if ((ch->in_room->area->authors
        && is_name( ch->name, ch->in_room->area->authors) && !REAL_GAME) 
        || IS_IMMORTAL(ch))
      fullcont=TRUE;

    if( strlen( argument) > 160 )
      *(argument+160)='\0';
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

   if (!fullcont)
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
    if ( arg1[0] == '\0' )
    { 
         send_to_char( "Syntax: rset <field>  <value>\n\r", ch );
         send_to_char( "or:     rset <string> <value>\n\r", ch );
         send_to_char( "\n\r",                              ch );
         send_to_char( "Field being one of:\n\r",           ch );
         send_to_char( "  name desc flag sector door extra\n\r", ch );
        return;
    }

   if (!fullcont)
   {  
    /* determine cost */
 
    rset_cost=COST_OF_SET;
    rset_cost=((IS_NPC(ch)?100:100-(25*(ch->pcdata->learned[gsn_bargain]))/100)*rset_cost)/100*1000;

    /* determine if they can afford it */
    if(ch->gold<rset_cost)
      {
      ch_printf(ch,"It will cost you $%d to do an rset.\n\r",rset_cost);
      return;
      }
   }
    /* make the modification */
    if(!strcasecmp(arg1,"name"))
      {
      STRFREE (ch->in_room->name );
      ch->in_room->name = STRALLOC( argument );
      }
    else if(!strcasecmp(arg1,"desc"))
      {
       switch( ch->substate)
       {
        default:
           bug( "do_description: illegal substate", 0 );
           return;

        case SUB_RESTRICTED:
           send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
           return;

        case SUB_NONE:
           ch->substate = SUB_ROOM_DESC;
           ch->dest_buf = ch->in_room;
           start_editing( ch, ch->in_room->description );
           return;
        case SUB_ROOM_DESC:
           location = ch->dest_buf;
           STRFREE (location->description );
           location->description = copy_buffer( ch );
           stop_editing( ch );
           return;
       }
      }
    else if(!strcasecmp(arg1,"flag"))
      {
       if (fullcont)
        {
        if(!get_bitvector_value(argument,&argn,"ROOM_"))
         {
          send_to_char("You must specify a flag to toggle!\n\r",ch);
          list_bitvectors(ch,"ROOM_");
          return;
         }
        } 
       else
      if(!get_bitvector_value(argument,&argn,"CROOM_"))
        {
        send_to_char("You must specify a flag to toggle!\n\r",ch);
        list_bitvectors(ch,"CROOM_");
        return;
        }
      if(IS_SET(ch->in_room->room_flags,argn))
        REMOVE_BIT(ch->in_room->room_flags,argn);
      else
        SET_BIT(ch->in_room->room_flags,argn);
      }
    else if(!strcasecmp(arg1,"sector"))
      {
      if(!get_bitvector_value(argument,&argn,"SECT_"))
        {
        send_to_char("You must specify a new sector type!\n\r",ch);
        list_bitvectors(ch,"SECT_");
        return;
        }
      ch->in_room->sector_type=argn;
      }
    else if(!strcasecmp(arg1,"door"))
      {
      EXIT_DATA *pexit;
      sh_int door_dir;

      argument = one_argument( argument, arg1 );
      if(!get_bitvector_value(arg1,&argn,"DIR_"))
        {
        send_to_char("You must specify the direction of the door to modify:\n\rUSE: castle rset door DIR doorField newData\n\r",ch);
        list_bitvectors(ch,"DIR_");
        return;
        }
      door_dir=argn;
      /* make sure the door is valid, if not, create one */
      if ( (pexit = ch->in_room->exit[door_dir] ) == NULL )
        {
        CREATE(pexit, EXIT_DATA, 1);
        pexit->description      = STRALLOC("");
        pexit->keyword        = STRALLOC("");
        pexit->exit_info = 0;
        pexit->pvnum  = ch->pcdata->pvnum;
        pexit->key  = -1;
        pexit->vnum  = ch->in_room->vnum;
        pexit->to_room  = NULL;
        ch->in_room->exit[door_dir] = pexit;
        top_exit++;
        }

      /* parse the options */
      argument = one_argument( argument, arg1 );
      if(!strcasecmp(arg1,"keyword"))
        {
        STRFREE (pexit->keyword );
        pexit->keyword = STRALLOC( argument );

        if((pexit->to_room!=NULL)&&
           (pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
          {
          /* set the opposite side of the door the same way if it's outside */
          EXIT_DATA *pexit2;

          pexit2=pexit->to_room->exit[rev_dir[door_dir]];
          if(pexit2!=NULL)
            {
            if(pexit2->keyword!=NULL)
              STRFREE (pexit2->keyword);
            pexit2->keyword = STRALLOC( pexit->keyword );
            }
          }
        }
      else if(!strcasecmp(arg1,"del"))
        {
	if (!fullcont)
        {
         if( pexit->to_room!=NULL
	   && pexit->to_room->creator_pvnum!=ch->pcdata->pvnum
	   && !IS_SET(pexit->exit_info, EX_BACKDOOR)
	   && !IS_SET(pexit->exit_info, EX_CLAN_BACKDOOR) )
          {
          send_to_char("That's your entrance!  You can't delete that!\n\r",ch);
          return;
          }
 	}
	if (IS_SET(pexit->exit_info, EX_BACKDOOR))
	 ch->pcdata->castle->has_backdoor=FALSE;
        
	if (IS_SET(pexit->exit_info, EX_CLAN_BACKDOOR))
	 ch->pcdata->clan->num_backdoors--;
	if (ch->pcdata->clan->num_backdoors<0)
	 ch->pcdata->clan->num_backdoors=0;
	
        ch->in_room->exit[door_dir]=NULL;
        STRFREE (pexit->description);
        STRFREE (pexit->keyword);
        DISPOSE(pexit);
        top_exit--;
        if (!fullcont) rset_cost/=100;
        }
      else if(!strcasecmp(arg1,"desc"))
        {
        STRFREE (pexit->description);
        if(argument[0]=='\'' || argument[0]=='"')
          {
          argument=one_argument_nolower(argument,arg1);
          pexit->description = STRALLOC( justify(arg1));
          }
        else
          pexit->description = STRALLOC( justify(argument) );
        if((pexit->to_room!=NULL)&&
           (pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
          {
          /* set the opposite side of the door the same way if it's outside */
          EXIT_DATA *pexit2;

          pexit2=pexit->to_room->exit[rev_dir[door_dir]];
          if(pexit2!=NULL)
            {
            if(pexit2->description!=NULL)
              STRFREE (pexit2->description);
            pexit2->description = STRALLOC( pexit->description );
            }
          }
        }
      else if(!strcasecmp(arg1,"flag"))
        {
        if(!get_bitvector_value(argument,&argn,"CEX_"))
          {
          send_to_char("You must specify a flag to toggle!\n\r",ch);
          list_bitvectors(ch,"CEX_");
          return;
          }
        if(IS_SET(pexit->exit_info,argn))
          REMOVE_BIT(pexit->exit_info,argn);
        else
          SET_BIT(pexit->exit_info,argn);
        if((pexit->to_room!=NULL)&&
           (pexit->to_room->creator_pvnum!=ch->pcdata->pvnum))
          {
          /* set the opposite side of the door the same way if it's outside */
          EXIT_DATA *pexit2;

          pexit2=pexit->to_room->exit[rev_dir[door_dir]];
          if(pexit2!=NULL)
            pexit2->key = pexit->key;
          }
        }
      else
        {
        send_to_char("Castle rset: change what part of that door?\n\rUse help CASTLE for a list of rset options.\n\r",ch);
        return;
        }
      }
    else if(!strcasecmp(arg1,"extra"))
      {
      EXTRA_DESCR_DATA *ed;
      char *desc;

      argument = one_argument( argument, arg1 );

      if(!strcasecmp(arg1,"keyword"))
        {
        desc=STRALLOC("");
        strcpy(arg2,argument);
        argument = one_argument( argument, arg1 );
        }
      else if(!strcasecmp(arg1,"desc"))
        {
        argument = one_argument( argument, arg1 );
        strcpy(arg2,arg1);
        desc = STRALLOC(justify(argument));
        }
      else if(!strcasecmp(arg1,"del"))
        {
        argument = one_argument( argument, arg1 );

        for ( ed = ch->in_room->first_extradesc;
              ed; 
	      ed = ed->next )
          if(is_name(arg1,ed->keyword))
            break;

        if(ed!=NULL)
          {
          sprintf(arg2,"Deleted the following extras: '%s'\n\r",ed->keyword);
	  UNLINK( ed, ch->in_room->first_extradesc, 
		      ch->in_room->last_extradesc, next, prev);
          STRFREE (ed->description);
          STRFREE (ed->keyword);
          DISPOSE(ed);
          send_to_char(arg2,ch);
 	  if (!fullcont)
	  {
           rset_cost/=100;
           ch->gold-=rset_cost;
           ch->pcdata->castle->cost+=rset_cost/1000;
           castle_needs_saving=TRUE;
 	  }
          return;
          }
        else
          {
          sprintf(arg2,"Couldn't find an extra named '%s' to delete.\n\r",arg1);
          send_to_char(arg2,ch);
          return;
          }
        }
      else
        {
        send_to_char("Castle rset: which extra?  Use help CASTLE for a list of rset options.\n\r",ch);
        return;
        }
      for ( ed = ch->in_room->first_extradesc; ed; ed = ed->next )
        if(is_name(arg1,ed->keyword))
          break;

      if ( ed == NULL )
        {
        CREATE(ed, EXTRA_DESCR_DATA, 1);
        ed->description=STRALLOC("");
        ed->keyword=STRALLOC(arg2);
	
	UNLINK( ed, ch->in_room->first_extradesc, 
		    ch->in_room->last_extradesc, next, prev);

        }
      if(ed->description!=NULL)
        STRFREE (ed->description);
      ed->description=desc;
      }
    else
      {
       do_rset( ch, "" );
       return;
      }

    /* report the modification */
    act("The room appears to flicker for an instant.",ch,NULL,NULL,TO_ROOM);
    act("The contractor comes, takes your money, looks around, hammers on a post, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

    /* affect changes due to modifications */
    if (!fullcont)
    {
     ch->gold-=rset_cost;
     ch->pcdata->castle->cost+=rset_cost/1000;
    }
    castle_needs_saving=TRUE;
    return;
}
extern char *			mprog_type_to_name	args( ( int type ) );

bool  straight_numbers;
void save_area( AREA_DATA *, bool );

void break_bits( FILE *fp, int number, char *vector , bool linear)
{
  int bit;
  int cnt;
  bool found,bitfound;

  if( straight_numbers )
    {
    fprintf( fp, "%d" , number);
    return;
    }

  found=FALSE;
  bit=0;

  if( !linear )
    while( bit<=number )
      {
      if( (bit & number) != 0)
        {
        bitfound=FALSE;
        for(cnt=0; cnt<MAX_BITVECTOR; cnt++ )
          if( bit == bitvector_table[cnt].value )
            if( is_name_short( vector, bitvector_table[cnt].name ))
              {
              if( found )
                fprintf( fp, "|" );
              fprintf( fp, "%s", bitvector_table[cnt].name );
              found=TRUE;
              bitfound=TRUE;
              }
        if(!bitfound)
          {
          if( found )
            fprintf( fp, "|" );
          fprintf( fp, "%d", bit );
          found=TRUE;
          }
        }
      if(bit!=0)
        bit*=2;
      else
        bit++;
      }
  else
    for( cnt=0; cnt < MAX_BITVECTOR; cnt++ )
     {
      if( number == bitvector_table[cnt].value )
        if( is_name_short( vector, bitvector_table[cnt].name ))
          {
          fprintf( fp, "%s", bitvector_table[cnt].name ); 
          found=TRUE;
          }
     }
  if( !found )
    fprintf( fp, "%d", number );
  return;
}

char fixit[MAX_STRING_LENGTH];

char *fixer_mprog( MPROG_DATA *mprog)
  {
  NPC_TOKEN *token;
  int cnt, lfix;
  char buf[MAX_STRING_LENGTH];
  char lbuf[100];
  lfix = 0;
  *fixit = '\0';

      for( token = mprog->token_list; token != NULL; token = token->next )
        {
        for( cnt=0; cnt < token->level*2; cnt++)
           lbuf[cnt]=' ';
        lbuf[cnt]='\0';
        switch( token->type )
          {
          case 1:
            sprintf( buf, "%s%s %s\n", lbuf,
                (char *) token->function,
                token->string != NULL ? token->string : "" );
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 2:
            sprintf( buf, "%s%s %s\n", lbuf, 
                cmd_table[token->value].name,
                token->string != NULL ? token->string : "" );
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 3:
            sprintf( buf, "%sif %s\n", lbuf,
                token->string != NULL ? token->string : "" );
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 4:
            sprintf( buf, "%sor %s\n", lbuf, 
                token->string != NULL ? token->string : "" );
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 5:
            sprintf( buf, "%selse\n", lbuf);
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 6:
            sprintf( buf, "%sendif\n", lbuf);
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          case 7:
            sprintf( buf, "%sbreak\n", lbuf);
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          default:
            sprintf( buf, "%sUnknown %s\n", lbuf,
                token->string != NULL ? token->string : "" );
            lfix = str_apd_max( fixit, buf, lfix, MAX_STRING_LENGTH );
            break;
          }

        }
  return( fixit );
  }

char *fixer( char *arg)
  {
  char *str,*fix;

  if(arg==NULL)
    {
    fixit[0]='\0';
    return ( fixit );
    }
  for ( str=arg, fix=fixit; *str != '\0'; str++ )
    {
	if ( *str != '\r' )
            {
	    *fix=*str ;
            fix++;
            }
    }
  *fix='\0';
  return( fixit );
  }

void save_header( FILE *fp, AREA_DATA *area)
  {
  /* #AREA { 5 35} Merc    Prototype for New Area~ */
  fprintf( fp, "#AREA ");
  fprintf( fp, area->name);
  fprintf( fp, "~\n\n");
  if(area->authors!=NULL)
    fprintf( fp, "#AUTHORS %s~\n", area->authors);
  fprintf( fp, "#RANGES\n");
  fprintf( fp, "%d %d %d %d\n", area->low_soft_range,
                                area->hi_soft_range,
                                area->low_hard_range,
                                area->hi_hard_range );
  if (area->resetmsg)
    fprintf( fp, "#RESETMSG %s~\n\n", area->resetmsg ); 
  fprintf( fp, "#FLAGS " );
  break_bits( fp, area->flags, "AFLAG_", FALSE);
  fprintf( fp, "\n\n");
  fprintf( fp, "#TEMPERATURE %d %d %d %d\n",
                     area->weather_info.temp_winter,
                     area->weather_info.temp_summer,
                     area->weather_info.temp_daily,
                     area->weather_info.wet_scale);

  return;
  }

void save_rooms( FILE *fp, AREA_DATA *area)
{
  ROOM_INDEX_DATA *room;
  int vnum, door;
  char buf[MAX_STRING_LENGTH];
  EXTRA_DESCR_DATA *ed;

  fprintf( fp, "#ROOMS\n" );
  for ( vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++ )
  {

      if ( (room = get_room_index( vnum )) == NULL )
        continue;

      fprintf( fp, "#%d\n", vnum );
      fprintf( fp, "%s~\n", fixer(room->name));
      fprintf( fp, "%s~\n", fixer(room->description));
      if(room->creator_pvnum!=0)
        fprintf( fp, "%d ", room->creator_pvnum);
      else
        fprintf( fp, "%d ", room->area->low_r_vnum/100);
      break_bits( fp, room->room_flags, "ROOM_", FALSE);
      fprintf( fp, " ");
      break_bits( fp, room->sector_type, "SECT_", TRUE);
      fprintf( fp, "\n");
      for( door=0; door<6; door++)
        if(room->exit[door]!=NULL)
          {
          if( room->exit[door]->to_room!=NULL &&
	     !IS_SET(room->exit[door]->exit_info, EX_RIP) &&
             !IS_SET(room->exit[door]->to_room->room_flags,ROOM_RIP))
            {
            fprintf( fp, "D" );
            break_bits( fp, door, "DIR_" , TRUE);
            fprintf( fp, "\n%s~\n", fixer( room->exit[door]->description ));
            fprintf( fp, "%s~\n", fixer(room->exit[door]->keyword) );
            break_bits( fp, room->exit[door]->exit_info, "EX_" , FALSE);
            fprintf( fp, " %d %u\n", room->exit[door]->key,
                                       room->exit[door]->to_room->vnum);
            } 
          /*else what's the point the else ? We _dont_ want to save this exit
					Martin 10/12/98
            {
            fprintf( fp, "D" );
            break_bits( fp, door, "DIR_" , TRUE);
            fprintf( fp, "\n%s~\n", fixer(room->exit[door]->description) );
            fprintf( fp, "%s~\n", fixer(room->exit[door]->keyword) );
            break_bits( fp, room->exit[door]->exit_info, "EX_" , FALSE);
            fprintf( fp, " %d 0\n", room->exit[door]->key );
            }*/
          }
      for( ed=room->first_extradesc; ed != NULL ; ed = ed->next)
         {
         fprintf( fp, "E\n" );
         fprintf( fp, "%s~\n", fixer(ed->keyword ));
         sprintf(buf, "%s", fixer(ed->description ));
         if(buf[0] == ' ')
           fprintf( fp, "." );
         fprintf( fp, "%s~\n", buf);
         }
      if( room->fall_room > 0 )
        fprintf( fp, "F %d %d %d\n", room->fall_room, room->fall_slope,
                room->distance_of_fall );
      if( room->room_file != NULL )
        fprintf( fp, "X %s~\n", fixer(room->room_file) );
      fprintf( fp, "S\n");
  }

  fprintf( fp, "#0\n" );
  return;
}

void save_mobiles( FILE *fp, AREA_DATA *area)
{
  MOB_INDEX_DATA *pMobIndex;
  MPROG_DATA *mpg;
  int vnum;

  fprintf( fp, "#MOBILES\n" );
  
  for (vnum = area->low_m_vnum; vnum <=area->hi_m_vnum; vnum++ )
  {
    if ( (pMobIndex = get_mob_index( vnum )) == NULL)
	continue;
      fprintf( fp, "#%d\n", 	vnum 				);
      fprintf( fp, "%s~\n", 	fixer(pMobIndex->player_name)	);
      fprintf( fp, "%s~\n", 	fixer(pMobIndex->short_descr)	);
      fprintf( fp, "%s~\n", 	fixer(pMobIndex->long_descr)	);
      fprintf( fp, "%s~\n", 	fixer(pMobIndex->description)	);
      REMOVE_BIT(pMobIndex->act,ACT_IS_NPC);
      break_bits( fp, pMobIndex->act, "ACT_", FALSE		);
      SET_BIT(pMobIndex->act,ACT_IS_NPC);
      fprintf( fp, "\n");
      break_bits( fp, pMobIndex->affected_by, "AFF_", FALSE	);
      fprintf( fp, "\n%d S\n",	pMobIndex->alignment		);
      fprintf( fp, "%d ",	pMobIndex->level		);
      if(IS_SET(pMobIndex->act,ACT_BODY))
        {
        break_bits( fp, pMobIndex->body_parts, "BODY_", FALSE);
        fprintf( fp, " ");
        break_bits( fp, pMobIndex->attack_parts, "BODY_", FALSE);
        }
      else
        fprintf( fp, "%d %d",pMobIndex->hitroll,pMobIndex->ac);
      fprintf( fp, " %dd%d+%d %dd%d+%d\n",
               pMobIndex->hitnodice,pMobIndex->hitsizedice,pMobIndex->hitplus,
               pMobIndex->damnodice,pMobIndex->damsizedice,pMobIndex->damplus);
      if( IS_SET( pMobIndex->act, ACT_RACE ))
        {
        fprintf( fp, "%d ",pMobIndex->gold);
        break_bits( fp, pMobIndex->race, "RACE_", TRUE);
        fprintf( fp, "\n");
        }
      else
        fprintf( fp, "%d %d\n",pMobIndex->gold,exp_level(CLASS_MONSTER,pMobIndex->level));
      break_bits( fp, pMobIndex->position, "POS_", TRUE);
      fprintf( fp, " ");
      break_bits( fp, pMobIndex->position, "POS_", TRUE);
      fprintf( fp, " ");
      break_bits( fp, pMobIndex->sex, "SEX_", TRUE);
      fprintf( fp, "\n");

      if( pMobIndex->mob_file != NULL )
        fprintf( fp, "X %s~\n", fixer(pMobIndex->mob_file) );

      /* mobprogs */
      if(pMobIndex->mobprogs!=NULL)
        {
        for(mpg=pMobIndex->mobprogs;mpg!=NULL;mpg=mpg->next)
          {
          fprintf( fp, ">%s ",fixer(mprog_type_to_name(mpg->type)));
          fprintf( fp, "%s~\n",fixer(mpg->arglist));
          fprintf( fp, "%s~\n", fixer_mprog(mpg));
          }
        fprintf( fp, "|\n");
        }
   }

  fprintf( fp, "#0\n" );
  return;
}

void save_helps( FILE *fp, AREA_DATA *area)
{
  HELP_DATA *help;
  HELP_MENU_DATA *menu;
  char buf[MAX_STRING_LENGTH];

  fprintf(fp,"#HELPS\n");

  for(help=help_first;help!=NULL;help=help->next)
    if(help->area==area)
      {
      fprintf(fp,"%d %s~\n",help->level,fixer(help->keyword));
      sprintf(buf, "%s", fixer( help->text));
      if(buf[0] == ' ')
        fprintf(fp,".");
      fprintf(fp,"%s", buf);
      if(help->menu!=NULL)
        {
        for(menu=help->menu;menu!=NULL;menu=menu->next)
          fprintf(fp,"\n{%c}%s",menu->option,menu->help->keyword);
        /* fprintf(fp,"\n"); Presto 6-20-98 */
        }
      fprintf(fp,"~\n");
      }

  fprintf(fp,"0 $~\n");
  return;
}


void save_objects( FILE *fp, AREA_DATA *area)
{
  OBJ_INDEX_DATA *pObjIndex;
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *aff;
  OBJ_PROG *prg;
  int vnum;
  char buf[MAX_STRING_LENGTH], buf2[200], *pt;

  fprintf(fp,"#OBJECTS\n");

  for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
  {
    if ( (pObjIndex = get_obj_index( vnum )) == NULL)
	continue;      
      fprintf( fp, "#%d\n", vnum );
      strcpy( buf, pObjIndex->name );
      sprintf( buf2, "i%u", pObjIndex->vnum );
      if( (pt = strstr( buf, buf2 )) != NULL )
        *( pt -1 ) = '\0';
      fprintf( fp, "%s~\n", fixer( buf ));
      fprintf( fp, "%s~\n", fixer(pObjIndex->short_descr));
      fprintf( fp, "%s~\n", fixer(pObjIndex->description));
      fprintf( fp, "%s~\n", fixer(pObjIndex->long_descr));
      break_bits( fp, pObjIndex->item_type, "ITEM_TYPE_", TRUE);
      fprintf( fp, "\n");
      SET_BIT(pObjIndex->extra_flags,ITEM_LEVEL_RENT );
      break_bits( fp, pObjIndex->extra_flags, "ITEM_FLAG_", FALSE);
      fprintf( fp, "\n");
      break_bits( fp, pObjIndex->wear_flags, "ITEM_WEAR_", FALSE);
      fprintf( fp, "\n");
      switch(pObjIndex->item_type)
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
          fprintf( fp, "%d ",pObjIndex->value[0]);
          break_bits( fp, skill_table[pObjIndex->value[1]].slot, "SPELL_", TRUE);
          fprintf( fp, " ");
          break_bits( fp, skill_table[pObjIndex->value[2]].slot, "SPELL_", TRUE);
          fprintf( fp, " ");
          break_bits( fp, skill_table[pObjIndex->value[3]].slot, "SPELL_", TRUE);
          break;
        case ITEM_WAND:
        case ITEM_STAFF:
          fprintf( fp, "%d %d %d ",pObjIndex->value[0],pObjIndex->value[1],pObjIndex->value[2]);
          break_bits( fp, skill_table[pObjIndex->value[3]].slot, "SPELL_", TRUE);
          break;
        case ITEM_WEAPON:
          fprintf( fp, "%d %d %d ",pObjIndex->value[0],pObjIndex->value[1],pObjIndex->value[2]);
          break_bits( fp, pObjIndex->value[3], "WEAPON_", TRUE);
          break;
        case ITEM_CONTAINER:
          fprintf( fp, "%d ",pObjIndex->value[0]);
          break_bits( fp, pObjIndex->value[1], "CONT_", TRUE);
          fprintf( fp, " %d %d",pObjIndex->value[2],pObjIndex->value[3]);
          break;
        case ITEM_DRINK_CON:
          fprintf( fp, "%d %d ",pObjIndex->value[0],pObjIndex->value[1]);
          break_bits( fp, pObjIndex->value[2], "LIQ_", TRUE);
          fprintf( fp, " ");
          if(pObjIndex->value[3]!=0)
            fprintf(fp,"POISONED");
          else
            fprintf(fp,"NOT_POISONED");
          break;
        case ITEM_FOOD:
          fprintf( fp, "%d %d %d ",pObjIndex->value[0],pObjIndex->value[1],
                   pObjIndex->value[2]);
          if(pObjIndex->value[3]!=0)
            fprintf(fp,"POISONED");
          else
            fprintf(fp,"NOT_POISONED");
          break;
        default:
          fprintf( fp, "%d %d %d %d",pObjIndex->value[0],pObjIndex->value[1],
                   pObjIndex->value[2],pObjIndex->value[3]);
          break;
        }
      fprintf( fp, "\n");
      fprintf( fp, "%d %d %d\n",pObjIndex->weight,pObjIndex->cost,pObjIndex->level);
      for( ed=pObjIndex->first_extradesc; ed != NULL ; ed = ed->next)
         {
         fprintf( fp, "E\n" );
         fprintf( fp, "%s~\n", fixer(ed->keyword ));
         fprintf( fp, "%s~\n", fixer(ed->description ));
         }
      for( aff=pObjIndex->first_affect; aff != NULL ; aff = aff->next)
         {
         fprintf( fp, "A\n" );
         break_bits( fp, aff->location , "APPLY_", TRUE);
         fprintf( fp, " %d\n", aff->modifier );
         }
      if( pObjIndex->obj_file != NULL )
        fprintf( fp, "X %s~\n", fixer(pObjIndex->obj_file) );
      if( pObjIndex->attack_string != NULL || pObjIndex->class_flags > 0)
        {
        fprintf( fp, "C\n");
        if( pObjIndex->attack_string != NULL )
          fprintf( fp, "%s~\n", pObjIndex->attack_string );
        else
          fprintf( fp, "~\n");
        break_bits( fp, pObjIndex->class_flags, "FLAG_CLASS_", FALSE );
        fprintf( fp, "\n" );
        }
      for( prg=pObjIndex->obj_prog;prg!=NULL;prg=prg->next)
        {
        char keyword;

        fprintf( fp, "P %d\n", prg->index);

        /* setup the appropriate keyword */
        if(prg->cmd>-2)
          keyword='C';
        else
          switch(prg->cmd)
            {
            case -2:
              keyword='U';break;
            case -3:
              keyword='T';break;
            case -5:
              keyword='H';break;
            case -6:
              keyword='D';break;
            case -4:
            default:
              keyword='X';break;
            }

        break_bits( fp, (int)keyword, "TRIG_", TRUE);
        switch(keyword)
          {
          case 'C':  /* game command */
            fprintf( fp, " %d %s\n",prg->percentage,cmd_table[prg->cmd].name);
            break;
          case 'U':  /* unknown command or social */
            fprintf( fp, " %d %s\n",prg->percentage,prg->unknown);
            break;
          case 'T':    /* Tick check */
            fprintf( fp, " %d\n", prg->percentage);
            break;
          case 'H':    /* Hit check */
            fprintf( fp, " %d\n", prg->percentage);
            break;
          case 'D':    /* Damage check */
            fprintf( fp, " %d\n", prg->percentage);
            break;
          case 'X':   /* void trigger */
            fprintf( fp, "\n");
            break;
          default:
            fprintf( fp, "\n");
            log_string( "(savearea.c)Bad obj_command type");
            break;
          }
        break_bits( fp, (int)prg->obj_command, "OPROG_", TRUE);
        switch(prg->obj_command)
          {
          case 'E':    /* screen echo */
            fprintf( fp, "\n%s~\n",fixer(prg->argument));
            break;
          case 'C':    /* user command at level 99 multi-line */
          case 'G':    /* user command at level 99 with arg */
            fprintf( fp, "\n%s~\n",fixer(prg->argument));
            break;
          case 'S':  /* Set quest bits on object */
          case 'D':  /* Add quest bits on object */
            fprintf( fp, " %d %d %d\n", prg->quest_offset, 
                prg->quest_bits, prg->if_value );
            break;
          case 'J':    /* Junk the object */
            fprintf( fp, "\n" );
            break;
          case 'A':   /* Apply to stats */
            fprintf( fp, " ");
            break_bits( fp, prg->if_check, "OAPPLY_", TRUE);
            fprintf( fp, " %d\n", prg->if_value );
            break;
          case 'H':   /* Has object */
            fprintf( fp, " %d %d %d\n", prg->if_value,
                prg->if_true,prg->if_false);
            break;
          case 'P':   /* Player quest bit check */
          case 'Q':   /* Object quest bit check */
            fprintf( fp, " %d %d ", prg->quest_offset, prg->quest_bits);
          case 'I':   /* If check */
            if(prg->obj_command == 'I')
              {
              fprintf( fp, " " );
              break_bits( fp, (int)prg->if_check, "OIF_", TRUE);
              }
            fprintf( fp, " %c ",prg->if_symbol);
            switch(prg->if_check)
              {
              case 'w':
                break_bits( fp, (int)prg->if_value, "WEAR_", TRUE);
                break;
              case 'c':
                break_bits( fp, (int)prg->if_value, "CLASS_", TRUE);
                break;
              case 'o':
                break_bits( fp, (int)prg->if_value, "POS_", TRUE);
                break;
              case 'r':
                break_bits( fp, (int)prg->if_value, "RACE_", TRUE);
                break;
              default:
                fprintf( fp, "%d",prg->if_value);
                break;
              }
            fprintf( fp, " %d %d\n",prg->if_true,prg->if_false);
            break;
          default:
            log_string( "(savearea.c)Bad obj_command reaction type");
            break;
          }
        }
  }
  fprintf(fp,"#0\n");
  return;
}

void save_shops( FILE *fp, AREA_DATA *area)
  {
  SHOP_DATA *shop;
  MOB_INDEX_DATA *mob;
  int i;

  fprintf(fp,"#SHOPS\n");

  for(shop=shop_first;shop!=NULL;shop=shop->next)
    if( (mob=get_mob_index(shop->keeper))!=NULL && mob->area == area)
      {
      fprintf(fp,"%d ", shop->keeper);
      for(i=0;i<MAX_TRADE;i++)
        {
        break_bits( fp, shop->buy_type[i], "ITEM_TYPE_", TRUE);
        if(i!=3)
          fprintf(fp," ");
        else
          fprintf(fp,"\n     ");
        }
      fprintf(fp,"%3d %3d %2d %2d", shop->profit_buy, shop->profit_sell,
              shop->open_hour,shop->close_hour);
      fprintf(fp," ;%s\n",fixer(mob->short_descr));
      }

  fprintf(fp,"0\n");
  return;
  }

void save_resets( FILE *fp, AREA_DATA *area)
  {
  RESET_DATA *reset;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj;
  ROOM_INDEX_DATA *room;
  char tmp[MAX_INPUT_LENGTH];

  fprintf(fp,"#RESETS\n");

  for(reset=area->first_reset;reset!=NULL;reset=reset->next)
    {
    switch(reset->command)
      {
      case 'M':
        fprintf(fp,"%c %d %5d %3d %5d ",reset->command,
                reset->arg0,reset->arg1,reset->arg2,reset->arg3);
        if((mob=get_mob_index(reset->arg1))!=NULL)
          fprintf(fp,";%s",str_resize(fixer(mob->short_descr),tmp,25));
        if((room=get_room_index(reset->arg3))!=NULL)
          fprintf(fp," in %s\n",str_resize(fixer(room->name),tmp,-25));
        break;
      case 'O':
        fprintf(fp,"%c %d %5d %3d %5d ",reset->command,
                reset->arg0,reset->arg1,reset->arg2,reset->arg3);
        if((obj=get_obj_index(reset->arg1))!=NULL)
          fprintf(fp,";%s",str_resize(fixer(obj->short_descr),tmp,25));
        if((room=get_room_index(reset->arg3))!=NULL)
          fprintf(fp," in %s\n",str_resize(fixer(room->name),tmp,-25));
        break;
      case 'G':
        fprintf(fp,"%c %d %5d %3d       ",reset->command,
                reset->arg0,reset->arg1,reset->arg2);
        if((obj=get_obj_index(reset->arg1))!=NULL)
          fprintf(fp,";%s\n",str_resize(fixer(obj->short_descr),tmp,25));
        break;
      case 'E':
        fprintf(fp,"%c %d %5d %2d ",reset->command,
                reset->arg0,reset->arg1,reset->arg2);
        break_bits( fp, reset->arg3, "WEAR_", TRUE);
        if((obj=get_obj_index(reset->arg1))!=NULL)
          fprintf(fp," ;equip %s\n",
                  str_resize(fixer(obj->short_descr),tmp,-25));
        break;
      case 'P':
        fprintf(fp,"%c %d %5d %3d %5d ",reset->command,
                reset->arg0,reset->arg1,reset->arg2,reset->arg3);
        if((obj=get_obj_index(reset->arg1))!=NULL)
          fprintf(fp,";%s",str_resize(fixer(obj->short_descr),tmp,25));
        if((obj=get_obj_index(reset->arg3))!=NULL)
          fprintf(fp," in %s\n",str_resize(fixer(obj->short_descr),tmp,-25));
        break;
      case 'D':
        fprintf(fp,"%c %d %5d ",reset->command,reset->arg0,reset->arg1);
        break_bits( fp, reset->arg2, "DIR_", TRUE);
        fprintf(fp," ");
        break_bits( fp, reset->arg3, "DOOR_", TRUE);
        if((room=get_room_index(reset->arg1))!=NULL)
          fprintf(fp," ;door in %s\n",str_resize(fixer(room->name),tmp,-25));
        break;
      case 'R':
        fprintf(fp,"%c %d %5d %3d       ",reset->command,
                reset->arg0,reset->arg1,reset->arg2);
        if((room=get_room_index(reset->arg1))!=NULL)
          fprintf(fp,";randomize exits in %s\n",
                  str_resize(fixer(room->name),tmp,-25));
        break;
      default:
        fprintf(fp,"%c %d %d %d %d\n",reset->command,
                reset->arg0,reset->arg1,reset->arg2,reset->arg3);
        break;
      }
    }

  fprintf(fp,"S\n");
  return;
  }

void save_specials( FILE *fp, AREA_DATA *area)
{
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;

  fprintf(fp,"#SPECIALS\n");
  for ( vnum = area->low_m_vnum; vnum <= area->hi_m_vnum; vnum++ )
  {
        if ( (pMobIndex = get_mob_index( vnum )) == NULL )
          continue;
        if ( !pMobIndex->spec_fun )
          continue;
        fprintf(fp,"M %d %s\n",vnum,fixer(spec_name_lookup(pMobIndex->spec_fun)));
  }
  for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
  {
        if ( (pObjIndex = get_obj_index( vnum )) == NULL )
          continue;
        if ( !pObjIndex->obj_fun )
          continue;
        fprintf(fp,"O %d %s\n",vnum,fixer(obj_name_lookup(pObjIndex->obj_fun)));
  }
  fprintf(fp,"S\n");
  return;
}


void do_savearea( CHAR_DATA *ch, char *arg)
{
  char buf[200];
  AREA_DATA *area;
  bool forreal;
  bool all;
  int cnt;

  if( ch != NULL )
    if( IS_NPC( ch ) || REAL_GAME )
      return;

  forreal=!strcasecmp(arg,"forreal");
  all=!strcasecmp(arg,"all");

  if(forreal && ch==NULL)
    area=room_index[20000]->area;
 
  else if ( ch != NULL && 
       IS_IMMORTAL(ch) && 
       ch->pcdata->area != NULL)
  {
    sprintf( buf, "Saving %s.\n\r", ch->pcdata->area->name );
    send_to_char(buf, ch );
    save_area( ch->pcdata->area, TRUE );
    send_to_char("Finished.\n\r", ch );
    return;
  }
  
  else if( ch!=NULL && all )
    {
    if( ch->level >= 98 )
     for( cnt = 1; cnt < MAX_VNUM; cnt+=100)
      if( get_room_index(cnt) != NULL )
        {
        area = room_index[cnt]->area ;
        sprintf( buf, "Saving %s.\n\r", area->name );
        send_to_char(buf, ch );
        save_area( area, TRUE );
        }
    send_to_char("Finished.\n\r", ch );
    return;
    }
  else
    area=ch->in_room->area;

    
  if( arg[0]=='n' )
    straight_numbers = TRUE;
  else
    straight_numbers = FALSE;
  save_area( area, forreal );
  if(ch!=NULL)
    send_to_char("Ok.\n\r", ch );
  return;
}

#ifdef USE_THREADS
void *threaded_save_area(void * args);

void save_area( AREA_DATA *area, bool forreal )
{
 SAVE_AREA_DATA * MyData;
 pthread_t save_area_thread;
 CREATE(MyData, SAVE_AREA_DATA, 1);
 MyData->Area = area;
 MyData->Reality = forreal;
 if (pthread_create(&save_area_thread, NULL, threaded_save_area, MyData) <0)
 {
  perror("save_clan_thread barf");
  return;
 }
 DISPOSE(MyData);
}

void *threaded_save_area(void * args)
{
 SAVE_AREA_DATA * MyData = (SAVE_AREA_DATA *) args;
 AREA_DATA * area = MyData->Area;
 bool forreal = MyData->Reality;
#else 
void save_area( AREA_DATA *area, bool forreal )
{
#endif
  char buf[200];
  FILE *fp;
  
  sprintf( buf, "%s.bak", area->filename );

      fclose( fpReserve );
  fp=fopen( buf, "w");

  save_header( fp, area);
  fprintf( fp, "\n\n\n\n" );  /* put some space between sections */
  save_helps( fp, area);
  fprintf( fp, "\n\n\n\n" );
  save_mobiles( fp, area );
  fprintf( fp, "\n\n\n\n" );
  save_objects( fp, area );
  fprintf( fp, "\n\n\n\n" );
  save_rooms( fp, area );
  fprintf( fp, "\n\n\n\n" );
  save_resets( fp, area );
  fprintf( fp, "\n\n\n\n" );
  save_shops( fp, area );
  fprintf( fp, "\n\n\n\n" );
  save_specials( fp, area );
  fprintf( fp, "\n\n\n\n#$\n" );
  fprintf( fp, "XXXXXXXXXX\n#Savearea\n" );

    /*  Let's make sure this works.  -  Chaos  4/25/99  */
    fclose( fp );
    if(is_valid_save(buf,"Savearea"))
      {
      if(forreal)
        {
        remove( area->filename );
        rename(buf,area->filename);
        }
      }

      fpReserve = fopen( NULL_FILE, "r" );
#ifdef USE_THREADS
        pthread_detach(pthread_self());
        pthread_exit(NULL);
        return NULL;
#else
        return;
#endif
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    AREA_DATA *pArea;
    int vnum;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        vnum = atol( arg );
        if ( vnum < 0 || get_room_index( vnum ) )
        {
                send_to_char( "You cannot find that...\n\r", ch );
                return;
        }
        if (  ch->level  < LEVEL_COUNT
        ||   vnum < 1 || IS_NPC(ch) || !ch->pcdata->area )
        {
          send_to_char( "No such location.\n\r", ch );
          return;
        }
        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
          send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
          return;
        }
        if ( vnum < pArea->low_r_vnum ||   vnum > pArea->hi_r_vnum )
        {
          send_to_char( "That room is not within your assigned range.\n\r", ch );
          return;
        }
        location = make_room( vnum );
        if ( !location )
        {
          bug( "Goto: make_room failed", 0 );
          return;
        }
        SET_BIT(location->room_flags, ROOM_PROTOTYPE);
        location->area = ch->pcdata->area;
        send_to_char( "Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\n\r", ch );
    }

    if ( room_is_private( location ) )
    {
        if ( get_trust( ch ) < LEVEL_IMMORTAL )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
        else
            send_to_char( "Overriding private flag!\n\r", ch );
    }

    if( IS_NPC(ch) && location->vnum < 3 )
      location = room_index[3];

    in_room = ch->in_room;
    if ( ch->fighting )
        stop_fighting( ch, TRUE );

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
        act( "$n $T", ch, NULL,
            (ch->pcdata && ch->pcdata->bamfout[0] != '\0')
            ? ch->pcdata->bamfout : "leaves in a swirling mist.",  TO_ROOM );

    ch->regoto = ch->in_room->vnum;
    char_from_room( ch );
    char_to_room( ch, location );

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
        act( "$n $T", ch, NULL,
            (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
            ? ch->pcdata->bamfin : "appears in a swirling mist.", TO_ROOM );
    do_look( ch, "auto" );
    if ( ch->in_room == in_room )
      return;
    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;
        if ( fch->master == ch && IS_IMMORTAL(fch) )
        {
            act( "You follow $N.", fch, NULL, ch, TO_CHAR );
            do_goto( fch, argument );
        }
        else if ( IS_NPC(fch) && fch->master == ch )
        {
           char_from_room (fch);
           char_to_room( fch, location );
        }
    }
    return;
}

void do_regoto( CHAR_DATA *ch, char *argument )
{
        char buf[MAX_STRING_LENGTH];

        sprintf(buf, "%d", ch->regoto);
        do_goto(ch, buf);
        return;
}

extern int top_area;

void write_area_list( )
{
    AREA_DATA *tarea;
    FILE *fpout;

    fpout = fopen( AREA_LIST, "w" );
    if ( !fpout )
    {
        bug( "FATAL: cannot open area.lst for writing!\n\r", 0 );
        return;
    }
    fprintf( fpout, "help.are\n" );
    for ( tarea = first_area; tarea; tarea = tarea->next )
        fprintf( fpout, "%s\n", tarea->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void assign_area( CHAR_DATA *ch )
{
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char taf[1024];
        AREA_DATA *tarea, *tmp;
        bool created = FALSE;
        if ( IS_NPC( ch ) )
          return;
        if (  ch->level  >= LEVEL_COUNT
        &&   ch->pcdata->r_range_lo
        &&   ch->pcdata->r_range_hi )
        {
          tarea = ch->pcdata->area;
          sprintf( taf, "%s.are", capitalize( ch->name ) );
          if ( !tarea )
          {
                for ( tmp = first_area; tmp; tmp = tmp->next )
                        if ( !strcasecmp( taf, tmp->filename ) )
                        {
                          tarea = tmp;
                          break;
                        }
          }
          if ( !tarea )
          {
            log_printf( "Creating area entry for %s", ch->name );
            CREATE( tarea, AREA_DATA, 1 );
            tarea->first_reset  = NULL;
            tarea->last_reset   = NULL;
            sprintf( buf, "%s's area in progress", ch->name );
            tarea->name         = str_dup( buf );
            tarea->filename     = str_dup( taf );
            sprintf( buf2, "%s", ch->name );
            tarea->authors      = STRALLOC( buf2 );
            tarea->age          = 0;
            tarea->nplayer      = 0;
            tarea->flags        = 0;
            tarea->resetmsg        = NULL;
    	    tarea->weather_info.temp_winter = 20;
    	    tarea->weather_info.temp_summer = 80;
    	    tarea->weather_info.temp_daily = 15;
    	    tarea->weather_info.wet_scale = 5;
            LINK( tarea, first_area, last_area, next, prev );
	    sort_area_by_name( tarea );
	    top_area++;
            created = TRUE;
          }
          else
          {
            log_printf( "Updating area entry for %s", ch->name );
          }
          tarea->low_r_vnum     = ch->pcdata->r_range_lo;
          tarea->low_o_vnum     = ch->pcdata->o_range_lo;
          tarea->low_m_vnum     = ch->pcdata->m_range_lo;
          tarea->hi_r_vnum      = ch->pcdata->r_range_hi;
          tarea->hi_o_vnum      = ch->pcdata->o_range_hi;
          tarea->hi_m_vnum      = ch->pcdata->m_range_hi;
          ch->pcdata->area      = tarea;
	  if (created)
   	    write_area_list();	
        }
}

void do_medit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    int  num,size,plus;
    char char1,char2;
    CHAR_DATA *victim;
    int value;
    int minattr, maxattr;
    bool lockvictim;
    char *origarg = argument;


    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't medit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MOB_DESC:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Chaste.\n\r", ch );
		bug( "do_medit: sub_mob_desc: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  victim = ch->dest_buf;
	  if ( victim == last_dead )
	  {
		send_to_char( "Your victim died!\n\r", ch );
		stop_editing( ch );
		return;
	  }
	  STRFREE( victim->description );
	  victim->description = copy_buffer( ch );
	  if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  {
		STRFREE( victim->pIndexData->description );
		victim->pIndexData->description = QUICKLINK( victim->description );
	  }
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
    }

    victim = NULL;
    lockvictim = FALSE;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
	victim = ch->dest_buf;
	
	if ( victim == last_dead )
	{
	    send_to_char( "Your victim died!\n\r", ch );
	    victim = NULL;
	    argument = "done";
	}
	if ( argument[0] == '\0' || !strcasecmp( argument, " " )
	||   !strcasecmp( argument, "stat" ) )
	{
	    if ( victim )
		do_mstat( ch, victim->name );
	    else
	        send_to_char( "No victim selected.  Type '?' for help.\n\r", ch );
	    return;
	}
	if ( !strcasecmp( argument, "done" ) || !strcasecmp( argument, "off" ) )
	{
	    send_to_char( "Medit mode off.\n\r", ch );
	    ch->substate = SUB_NONE;
	    ch->dest_buf = NULL;
	    if ( ch->pcdata && ch->pcdata->subprompt )
	    {
		STRFREE( ch->pcdata->subprompt );
		ch->pcdata->subprompt = NULL;
	    }
	    return;
	}
    }
    if ( victim )
    {
	lockvictim = TRUE;
	strcpy( arg1, victim->name );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }
    else
    {
	lockvictim = FALSE;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }

    if ( !strcasecmp( arg1, "on" ) )
    {
	send_to_char( "Syntax: medit <victim|vnum> on.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD)
    ||   !strcasecmp( arg1, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	{
	    if ( victim )
		send_to_char( "Syntax: <field>  <value>\n\r",		ch );
	    else
		send_to_char( "Syntax: <victim> <field>  <value>\n\r",	ch );
	}
	else
	    send_to_char( "Syntax: medit <victim> <field>  <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  str int wis dex con cha sex class clan title\n\r",	ch );
	send_to_char( "  gold hp mana move practice align race\n\r",	ch );
	send_to_char( "  hitroll damroll armor affected level\n\r",	ch );
	send_to_char( "  thirst drunk full flags\n\r",		ch );
	send_to_char( "  pos part \n\r",		ch );
	send_to_char( "  name short long description spec \n\r", ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "For editing index/prototype mobiles:\n\r",	ch );
	send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\n\r",ch );
	send_to_char( "  damnumdie damsizedie damplus (damage roll)\n\r",ch );
	return;
    }

    if ( !victim && get_trust( ch ) < 98 )
    {
	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }
    else
    if ( !victim )
    {
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "No one like that in all the realms.\n\r", ch );
	    return;
	}
    }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	ch->dest_buf = NULL;
	return;
    }
    if ( lockvictim )
      ch->dest_buf = victim;

    if ( IS_NPC(victim) )
    {
	minattr = 1;
	maxattr = 25;
    }
    else
    {
	minattr = 3;
	maxattr = 18;
    }

    if ( !strcasecmp( arg2, "on" ) )
    {
	CHECK_SUBRESTRICTED( ch );
	ch_printf( ch, "Medit mode on. (Editing %s).\n\r",
		victim->name );
	ch->substate = SUB_REPEATCMD;
	ch->dest_buf = victim;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   if ( IS_NPC(victim) )
		sprintf( buf, "Editing Mobile Vnum: %d", victim->pIndexData->vnum );
	   else
		sprintf( buf, "Editing Player : %s", victim->name );
	   ch->pcdata->subprompt = STRALLOC( buf );
	}
	return;
    }
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    if ( atoi(arg3) < -1 && value == -1 )
      value = atoi(arg3);

    if ( !strcasecmp( arg2, "sex" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->sex = value;
	return;
    }

    if ( !strcasecmp( arg2, "class" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;

	if ( value < 0 || value >= MAX_CLASS )
	{
	    ch_printf( ch, "Class range is 0 to %d.\n", MAX_CLASS );
	    return;
	}
	victim->class = value;
	return;
    }

    if ( !strcasecmp( arg2, "race" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	value = atoi ( arg3 );
	if ( value < 0 )
	  value = atoi( arg3 );
	if ( !IS_NPC(victim) && (value < 0 || value >= MAX_RACE) )
	{
	    ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE-1 );
	    return;
	}
	victim->race = value;
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->race = value;
	return;
    }

    if ( !strcasecmp( arg2, "level" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 150 )
	{
            ch_printf( ch, "Level range is 0 to 150.\n\r");
	    return;
	}
	victim->level = value;
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->level = value;
	return;
    }

    if ( !strcasecmp( arg2, "gold" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	victim->gold = value;
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->gold = value;
	return;
    }

    if ( !strcasecmp( arg2, "hp" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 1 || value > 32700 )
	{
	    send_to_char( "Hp range is 1 to 32,700 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
	return;
    }

    if ( !strcasecmp( arg2, "mana" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
	return;
    }

    if ( !strcasecmp( arg2, "move" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	return;
    }

    if ( !strcasecmp( arg2, "practice" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Practice range is 0 to 100 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !strcasecmp( arg2, "align" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->alignment = value;
	return;
    }

    if ( !strcasecmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Thirst range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !strcasecmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Drunk range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !strcasecmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Full range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !strcasecmp( arg2, "name" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	STRFREE( victim->name );
	victim->name = STRALLOC( arg3 );
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	{
	   STRFREE( victim->pIndexData->player_name );
	   victim->pIndexData->player_name = QUICKLINK( victim->name );
	}
	return;
    }

    if ( !strcasecmp( arg2, "clan" ) )
    {
	CLAN_DATA *clan;

	if ( get_trust( ch ) < MAX_LEVEL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( !arg3 || arg3[0] == '\0' )
	{
	    if ( victim->pcdata->clan == NULL )
		return;
	    if ( !IS_IMMORTAL( victim ) ) {
  	        --victim->pcdata->clan->members; 
	        save_clan( victim->pcdata->clan );
	    }
	    STRFREE( victim->pcdata->clan_name );
	    victim->pcdata->clan_name	= STRALLOC( "" );
	    victim->pcdata->clan	= NULL;
	    return;
	}
	clan = get_clan( arg3 );
	if ( !clan )
	{
	   send_to_char( "No such clan.\n\r", ch );
	   return;
	}
	if ( victim->pcdata->clan != NULL && !IS_IMMORTAL( victim ) )
	{
		--victim->pcdata->clan->members; 
	        save_clan( victim->pcdata->clan );
        }
	STRFREE( victim->pcdata->clan_name );
	victim->pcdata->clan_name = QUICKLINK( clan->name );
	victim->pcdata->clan = clan;
	if ( !IS_IMMORTAL( victim ) ) {
	   ++victim->pcdata->clan->members; 
	   save_clan( victim->pcdata->clan );
	}
	return;
    }

    if ( !strcasecmp( arg2, "short" ) )
    {
	STRFREE( victim->short_descr );
	victim->short_descr = STRALLOC( arg3 );
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	{
	   STRFREE( victim->pIndexData->short_descr );
	   victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
	}
	return;
    }

    if ( !strcasecmp( arg2, "long" ) )
    {
	STRFREE( victim->long_descr );
	strcpy( buf, arg3 );
	strcat( buf, "\n\r" );
	victim->long_descr = STRALLOC( buf );
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	{
	   STRFREE( victim->pIndexData->long_descr );
	   victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
	}
	return;
    }

    if ( !strcasecmp( arg2, "description" ) )
    {
	if ( arg3[0] )
	{
	   STRFREE( victim->description );
	   victim->description = STRALLOC( arg3 );
	   if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	   {
	      STRFREE(victim->pIndexData->description );
	      victim->pIndexData->description = QUICKLINK( victim->description );
	   }
	   return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_MOB_DESC;
	ch->dest_buf = victim;
	start_editing( ch, victim->description );
	return;
    }

    if ( !strcasecmp( arg2, "title" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	set_title( victim, arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "spec" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

        if ( !strcasecmp( arg3, "none" ) )
        {
          victim->spec_fun = NULL;
	  send_to_char( "Special function removed.\n\r", ch );
	  if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->spec_fun = victim->spec_fun;
	  return;
        }

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    send_to_char( "No such spec fun.\n\r", ch );
	    return;
	}
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->spec_fun = victim->spec_fun;
	return;
    }

    if ( !strcasecmp( arg2, "flags" ) )
    {
        bool pcflag;
	if ( !IS_NPC( victim ) && get_trust( ch ) < MAX_LEVEL )
	{
	    send_to_char( "You can only modify a mobile's flags.\n\r", ch );
	    return;
	}

	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: medit <victim> flags <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
           pcflag = FALSE;
	   argument = one_argument( argument, arg3 );
	   value = IS_NPC( victim) ? get_actflag( arg3 ) : get_plrflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
	     if ( IS_NPC(victim)
	     &&   1 << value == ACT_PROTOTYPE
	     &&   get_trust( ch ) < MAX_LEVEL )
		send_to_char( "You cannot change the prototype flag.\n\r", ch );
	     else
	     if ( IS_NPC(victim) && 1 << value == ACT_IS_NPC )
		send_to_char( "If that could be changed, it would cause many problems.\n\r", ch );
	     else
	     {
		    TOGGLE_BIT( victim->act, 1 << value );
		    if ( IS_NPC(victim) && value == ACT_PROTOTYPE )
			victim->pIndexData->act = victim->act;
	     }
	   }
	}
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->act = victim->act; 
	return;
    }

    if ( !strcasecmp( arg2, "affected" ) )
    {
	if ( !IS_NPC( victim ) && get_trust( ch ) < MAX_LEVEL )
	{
	    send_to_char( "You can only modify a mobile's flags.\n\r", ch );
	    return;
	}

	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: medit <victim> affected <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_aflag( arg3 );
	   if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
		TOGGLE_BIT( victim->affected_by, 1 << value );
	}
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->affected_by = victim->affected_by; 
	return;
    }



    if ( !strcasecmp( arg2, "pos" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( value < 0 || value > POS_STANDING )
	{
	    ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
	    return;
	}
	victim->position = value;
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->position = victim->position; 
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /*
     * save some finger-leather
     */
    if ( !strcasecmp( arg2, "hitdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
	sprintf(outbuf,"%s hitnumdie %d",arg1, num);
        do_medit( ch, outbuf );

	sprintf(outbuf,"%s hitsizedie %d",arg1, size);
        do_medit( ch, outbuf );

	sprintf(outbuf,"%s hitplus %d",arg1, plus);
        do_medit( ch, outbuf );
        return;
    }
    /*
     * save some more finger-leather
     */
    if ( !strcasecmp( arg2, "damdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
	sprintf(outbuf,"%s damnumdie %d",arg1, num);
        do_medit( ch, outbuf );
	sprintf(outbuf,"%s damsizedie %d",arg1, size);
        do_medit( ch, outbuf );
	sprintf(outbuf,"%s damplus %d",arg1, plus);
        do_medit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "hitnumdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Number of hitpoint dice range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->hitnodice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hitsizedie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Hitpoint dice size range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->hitsizedice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hitplus" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Hitpoint bonus range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->hitplus = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "damnumdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Number of damage dice range is 0 to 100.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->damnodice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "damsizedie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Damage dice size range is 0 to 100.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->damsizedice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "damplus" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 1000 )
	{
	    send_to_char( "Damage bonus range is 0 to 1000.\n\r", ch );
	    return;
	}

	if ( IS_NPC( victim ) && IS_SET(victim->act, ACT_PROTOTYPE) )
	  victim->pIndexData->damplus = value;
	send_to_char( "Done.\n\r", ch );
	return;

    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_medit;
    }
    else
	do_medit( ch, "" );
    return;
}


void do_oedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf  [MAX_STRING_LENGTH];
    OBJ_DATA *obj, *tmpobj;
    EXTRA_DESCR_DATA *ed;
    bool lockobj;
    char *origarg = argument;

    int value, tmp;


    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't oedit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;

	case SUB_OBJ_EXTRA:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Chaste.\n\r", ch );
		bug( "do_oedit: sub_obj_extra: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  /*
	   * hopefully the object didn't get extracted...
	   * if you're REALLY paranoid, you could always go through
	   * the object and index-object lists, searching through the
	   * extra_descr lists for a matching pointer...
	   */
	  ed  = ch->dest_buf;
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  tmpobj = ch->spare_ptr;
	  stop_editing( ch );
	  ch->dest_buf = tmpobj;
	  ch->substate = ch->tempnum;
	  return;
	
	case SUB_OBJ_LONG:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Chaste.\n\r", ch );
		bug( "do_oedit: sub_obj_long: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  obj = ch->dest_buf;
	  STRFREE( obj->description );
	  obj->description = copy_buffer( ch );
	  if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  {
		if ( can_omodify(ch, obj) )
		{
		  STRFREE( obj->pIndexData->description );
		  obj->pIndexData->description = QUICKLINK( obj->description );
		}
	  }
	  tmpobj = ch->spare_ptr;
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  ch->dest_buf = tmpobj;
	  return;
    }

    obj = NULL;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
	obj = ch->dest_buf;
	if ( argument[0] == '\0' || !strcasecmp( argument, " " )
	||   !strcasecmp( argument, "stat" ) )
	{
	    if ( obj )
		do_ostat( ch, obj->name );
	    else
	        send_to_char( "No object selected.  Type '?' for help.\n\r", ch );
	    return;
	}
	if ( !strcasecmp( argument, "done" ) || !strcasecmp( argument, "off" ) )
	{
	    send_to_char( "Oedit mode off.\n\r", ch );
	    ch->substate = SUB_NONE;
	    ch->dest_buf = NULL;
	    if ( ch->pcdata && ch->pcdata->subprompt )
	    {
		STRFREE( ch->pcdata->subprompt );
		ch->pcdata->subprompt = NULL;
	    }
	    return;
	}
    }
    if ( obj )
    {
	lockobj = TRUE;
	strcpy( arg1, obj->name );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }
    else
    {
	lockobj = FALSE;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }

    if ( !strcasecmp( arg1, "on" ) )
    {
	send_to_char( "Syntax: oedit <object|vnum> on.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !strcasecmp( arg1, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	{
	    if ( obj )
		send_to_char( "Syntax: <field>  <value>\n\r",		ch );
	    else
		send_to_char( "Syntax: <object> <field>  <value>\n\r",	ch );
	}
	else
	    send_to_char( "Syntax: oedit <object> <field>  <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  flags wear level weight cost rent timer\n\r",	ch );
	send_to_char( "  name short long ed rmed actiondesc\n\r",	ch );
	send_to_char( "  type value0 value1 value2 value3\n\r",	ch );
	send_to_char( "For weapons:             For armor:\n\r",	ch );
	send_to_char( "  weapontype 		  ac \n\r",	ch );
	send_to_char( "For scrolls, potions and pills:\n\r",		ch );
	send_to_char( "  slevel spell1 spell2 spell3\n\r",		ch );
	send_to_char( "For wands and staves:\n\r",			ch );
	send_to_char( "  slevel spell maxcharges charges\n\r",		ch );
	send_to_char( "For containers:\n\r", ch );
	send_to_char( "  cflags key capacity\n\r",		ch );
	return;
    }

    if ( !obj && get_trust(ch) < MAX_LEVEL )
    {
	if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
	{
	   send_to_char( "You can't find that here.\n\r", ch );
	   return;
	}
    }
    else
    if ( !obj )
    {
	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
	{
	   send_to_char( "There is nothing like that in all the realms.\n\r", ch );
	   return;
 	}
    }
    if ( lockobj )
	ch->dest_buf = obj;
    else
	ch->dest_buf = NULL;

    value = atoi( arg3 );

    if ( !strcasecmp( arg2, "on" ) )
    {
	ch_printf( ch, "Oedit mode on. (Editing '%s' vnum %d).\n\r",
		obj->name, obj->pIndexData->vnum );
	ch->substate = SUB_REPEATCMD;
	ch->dest_buf = obj;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   sprintf( buf, "Editing Object Vnum: %d", obj->pIndexData->vnum );

	   ch->pcdata->subprompt = STRALLOC( buf );
	}
	return;
    }

    if ( !strcasecmp( arg2, "name" ) )
    {
	bool proto = FALSE;

 	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
		proto = TRUE;
	if ( proto && !can_omodify( ch, obj ) )
	  return;
	STRFREE( obj->name );
	obj->name = STRALLOC( arg3 );
	if ( proto )
	{
	   STRFREE(obj->pIndexData->name );
	   obj->pIndexData->name = QUICKLINK( obj->name );
	}
	return;
    }

    if ( !strcasecmp( arg2, "short" ) )
    {
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	{
           if ( !can_omodify( ch, obj ) )
              return;
	   STRFREE( obj->short_descr );
	   obj->short_descr = STRALLOC( arg3 );
	   STRFREE(obj->pIndexData->short_descr );
	   obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
	}
        else
        /* Feature added by Narn, Apr/96 
	 * If the item is not proto, add the word 'rename' to the keywords
	 * if it is not already there.
	 */
        {
	  STRFREE( obj->short_descr );
	  obj->short_descr = STRALLOC( arg3 );
          if ( str_infix( "rename", obj->name ) )
          {
            sprintf( buf, "%s %s", obj->name, "rename" );
	    STRFREE( obj->name );
	    obj->name = STRALLOC( buf );
          }
        }
	return;
    }

    if ( !strcasecmp( arg2, "long" ) )
    {
	if ( arg3[0] )
	{
	   if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	   {
              if ( !can_omodify( ch, obj ) )
                return;
	      STRFREE( obj->description );
	      obj->description = STRALLOC( arg3 );
	      STRFREE(obj->pIndexData->description );
	      obj->pIndexData->description = QUICKLINK( obj->description );
	      return;
	   }
	   STRFREE( obj->description );
	   obj->description = STRALLOC( arg3 );
	   return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	if ( lockobj )
	  ch->spare_ptr = obj;
	else
	  ch->spare_ptr = NULL;
	ch->substate = SUB_OBJ_LONG;
	ch->dest_buf = obj;
	start_editing( ch, obj->description );
	return;
    }

    if ( get_trust(ch) < MAX_LEVEL )
    {
	send_to_char("You can only oedit the name, short and long right now.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[0] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[0] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[1] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[1] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[2] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	{
	  obj->pIndexData->value[2] = value;
	  if ( obj->item_type == ITEM_WEAPON && value != 0 )
	    obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
	}
	return;
    }

    if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[3] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[3] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value4" ) || !strcasecmp( arg2, "v4" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[4] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[4] = value;
	return;
    }

    if ( !strcasecmp( arg2, "value5" ) || !strcasecmp( arg2, "v5" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[5] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[5] = value;
	return;
    }

    if ( !strcasecmp( arg2, "type" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oedit <object> type <type>\n\r", ch );
	   return;
	}
	value = get_otype( argument );
	if ( value < 1 )
	{
	     ch_printf( ch, "Unknown type: %s\n\r", arg3 );
	     return;	
	}
	obj->item_type = (sh_int) value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->item_type = obj->item_type; 
	return;	
    }

    if ( !strcasecmp( arg2, "flags" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oedit <object> flags <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_oflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
	     if ( value == ITEM_PROTOTYPE
	     &&   get_trust( ch ) < MAX_LEVEL )
	       send_to_char( "You cannot change the prototype flag.\n\r", ch );
	     else
	     {
	       TOGGLE_BIT(obj->extra_flags, 1 << value);
	       if ( value == ITEM_PROTOTYPE )
	         obj->pIndexData->extra_flags = obj->extra_flags;
	     }
	   }
	}
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->extra_flags = obj->extra_flags; 
	return;
    }

    if ( !strcasecmp( arg2, "wear" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oedit <object> wear <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_wflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	     TOGGLE_BIT( obj->wear_flags, 1 << value );
	}

	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->wear_flags = obj->wear_flags;
	return;
    }

    if ( !strcasecmp( arg2, "level" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->level = value;
	return;
    }

    if ( !strcasecmp( arg2, "weight" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->weight = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->weight = value;
	return;
    }

    if ( !strcasecmp( arg2, "cost" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->cost = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->cost = value;
	return;
    }

    if ( !strcasecmp( arg2, "timer" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->timer = value;
	return;
    }
	
    if ( !strcasecmp( arg2, "attack_string" ) )
    {
	if ( strstr( arg3, "%n" )
	||   strstr( arg3, "%d" )
	||   strstr( arg3, "%l" ) )
	{
	   send_to_char( "Illegal characters!\n\r", ch );
	   return;
	}
	STRFREE(obj->pIndexData->attack_string );
	obj->pIndexData->attack_string = STRALLOC( arg3 );
	return;
    }

    if ( !strcasecmp( arg2, "ed" ) )
    {
	if ( !arg3 || arg3[0] == '\0' )
	{
	    send_to_char( "Syntax: oedit <object> ed <keywords>\n\r",
		ch );
	    return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( obj->timer )
	{
	   send_to_char("It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r", ch );
	   return;
	}
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  ed = SetOExtraProto( obj->pIndexData, arg3 );
	else
	  ed = SetOExtra( obj, arg3 );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	if ( lockobj )
	  ch->spare_ptr = obj;
	else
	  ch->spare_ptr = NULL;
	ch->substate = SUB_OBJ_EXTRA;
	ch->dest_buf = ed;
	start_editing( ch, ed->description );
	return;
    }

    if ( !strcasecmp( arg2, "rmed" ) )
    {
	if ( !arg3 || arg3[0] == '\0' )
	{
	   send_to_char( "Syntax: oedit <object> rmed <keywords>\n\r", ch );
	   return;
	}
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	{
	    if ( DelOExtraProto( obj->pIndexData, arg3 ) )
		send_to_char( "Deleted.\n\r", ch );
	    else
		send_to_char( "Not found.\n\r", ch );
	    return;
	}
	if ( DelOExtra( obj, arg3 ) )
	  send_to_char( "Deleted.\n\r", ch );
	else
	  send_to_char( "Not found.\n\r", ch );
	return;
    }
    /*
     * save some finger-leather
     */
    if ( !strcasecmp( arg2, "ris" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oedit( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oedit( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "r" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "i" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }
    if ( !strcasecmp( arg2, "s" ) )
    {
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "ri" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oedit( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "rs" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oedit( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    if ( !strcasecmp( arg2, "is" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oedit( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oedit( ch, outbuf );
        return;
    }

    /*
     * Make it easier to set special object values by name than number
     * 						-Martin
     */
    tmp = -1;
    switch( obj->item_type )
    {
	case ITEM_WEAPON:
	    if ( !strcasecmp( arg2, "weapontype" ) )
	    {
		value = atoi(arg3);
		if ( value < 1  || value > 11)
		{
		    send_to_char( "Unknown weapon type.\n\r", ch );
		    return;
		}
		tmp = 3;
		break;
	    }
	    if ( !strcasecmp( arg2, "condition" ) )	tmp = 0;
	    break;
	case ITEM_ARMOR:
	    if ( !strcasecmp( arg2, "condition" ) )	tmp = 3;
	    if ( !strcasecmp( arg2, "ac" )	)		tmp = 1;
	    break;
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
	    if ( !strcasecmp( arg2, "slevel" ) )		tmp = 0;
	    if ( !strcasecmp( arg2, "spell1" ) )		tmp = 1;
	    if ( !strcasecmp( arg2, "spell2" ) )		tmp = 2;
	    if ( !strcasecmp( arg2, "spell3" ) )		tmp = 3;
	    if ( tmp >=1 && tmp <= 3 )			value = skill_lookup(arg3);
	    break;
	case ITEM_STAFF:
	case ITEM_WAND:
	    if ( !strcasecmp( arg2, "slevel" ) )		tmp = 0;
	    if ( !strcasecmp( arg2, "spell" ) )
	    {
	    	tmp = 3;
		value = skill_lookup(arg3);
	    }
	    if ( !strcasecmp( arg2, "maxcharges" )	)	tmp = 1;
	    if ( !strcasecmp( arg2, "charges" ) )		tmp = 2;
	    break;
	case ITEM_CONTAINER:
	    if ( !strcasecmp( arg2, "capacity" ) )		tmp = 0;
	    if ( !strcasecmp( arg2, "cflags" ) )		tmp = 1;
	    if ( !strcasecmp( arg2, "key" ) )		tmp = 2;
	    break;
    }
    if ( tmp >= 0 && tmp <= 3 )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[tmp] = value;
	if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
	  obj->pIndexData->value[tmp] = value;
	return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_oedit;
    }
    else
	do_oedit( ch, "" );
    return;
}

void do_redit( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA	*location, *tmp;
    EXTRA_DESCR_DATA	*ed;
    char		dir;
    EXIT_DATA		*xit;
    int			value;
    int			edir, ekey, evnum;
    char		*origarg = argument;

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor.\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_ROOM_DESC:
	  location = ch->dest_buf;
	  if ( !location )
	  {
		bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
		location = ch->in_room;
	  }
	  STRFREE( location->description );
	  location->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
	case SUB_ROOM_EXTRA:
	  ed = ch->dest_buf;
	  if ( !ed )
	  {
		bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
		stop_editing( ch );
		return;
	  }
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
    }

    location = ch->in_room;

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( ch->substate == SUB_REPEATCMD )
    {
	if ( arg[0] == '\0' )
	{
	    do_rstat( ch, "" );
	    return;
	}
	if ( !strcasecmp( arg, "done" ) || !strcasecmp( arg, "off" ) )
	{
	    send_to_char( "Redit mode off.\n\r", ch );
	    if ( ch->pcdata && ch->pcdata->subprompt )
	    {
		STRFREE( ch->pcdata->subprompt );
		ch->pcdata->subprompt = NULL;
	    }
	    ch->substate = SUB_NONE;
	    return;
	}
    }
    if ( arg[0] == '\0' || !strcasecmp( arg, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	    send_to_char( "Syntax: <field> value\n\r",			ch );
	else
	    send_to_char( "Syntax: redit <field> value\n\r",		ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  name desc ed rmed\n\r",			ch );
	send_to_char( "  exit bexit exdesc exflags exname exkey\n\r",	ch );
	send_to_char( "  flags sector rlist\n\r",	ch );
	return;
    }

    if ( !can_rmodify( ch, location ) )
	return;

    if ( !strcasecmp( arg, "on" ) )
    {
	send_to_char( "Redit mode on.\n\r", ch );
	ch->substate = SUB_REPEATCMD;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   ch->pcdata->subprompt = STRALLOC( "Editing Current Room ");
	}
	return;
    }

    if ( !strcasecmp( arg, "substate" ) )
    {
	  argument = one_argument( argument, arg2);
          if( !strcasecmp( arg2, "north" )  )
	  {
               ch->inter_substate = SUB_NORTH; 
	       return;
	  }
          if( !strcasecmp( arg2, "east" )  )
	  {
               ch->inter_substate = SUB_EAST; 
	       return;
	  }
          if( !strcasecmp( arg2, "south" )  )
	  {
               ch->inter_substate = SUB_SOUTH; 
	       return;
	  }
          if( !strcasecmp( arg2, "west" )  )
	  {
               ch->inter_substate = SUB_WEST; 
	       return;
	  }
          if( !strcasecmp( arg2, "up" )  )
	  {
               ch->inter_substate = SUB_UP; 
	       return;
	  }
          if( !strcasecmp( arg2, "down" )  )
	  {
               ch->inter_substate = SUB_DOWN; 
	       return;
	  }
          send_to_char( " unrecognized substate in redit\n\r", ch);
	  return;
    }


    if ( !strcasecmp( arg, "name" ) )
    {
	if ( argument[0] == '\0' )
	{
	   send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
	   send_to_char( "Usage: redit name <Room summary>\n\r", ch );
	   return;
	}
	STRFREE( location->name );
	location->name = STRALLOC( argument );
	return;
    }

    if ( !strcasecmp( arg, "desc" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	    ch->tempnum = SUB_REPEATCMD;
	else
	    ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_DESC;
	ch->dest_buf = location;
	start_editing( ch, location->description );
	return;
    }

    if ( !strcasecmp( arg, "ed" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Create an extra description.\n\r", ch );
	    send_to_char( "You must supply keyword(s).\n\r", ch );
	    return;
	}
	CHECK_SUBRESTRICTED( ch );
	ed = SetRExtra( location, argument );
	if ( ch->substate == SUB_REPEATCMD )
	    ch->tempnum = SUB_REPEATCMD;
	else
	    ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_EXTRA;
	ch->dest_buf = ed;
	start_editing( ch, ed->description );
	return;
    }

    if ( !strcasecmp( arg, "rmed" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Remove an extra description.\n\r", ch );
	    send_to_char( "You must supply keyword(s).\n\r", ch );
	    return;
	}
	if ( DelRExtra( location, argument ) )
	    send_to_char( "Deleted.\n\r", ch );
	else
	    send_to_char( "Not found.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "rlist" ) )
    {
	RESET_DATA *pReset;
	char *bptr;
	AREA_DATA *tarea;
	sh_int num;

	tarea = location->area;
	if ( !tarea->first_reset )
	{
	    send_to_char( "This area has no resets to list.\n\r", ch );
	    return;
	}
	num = 0;
	for ( pReset = tarea->first_reset; pReset; pReset = pReset->next )
	{
	    num++;
	    if ( (bptr = sprint_reset( ch, pReset, num, TRUE )) == NULL )
	      continue;
	    send_to_char( bptr, ch );
	}
	return;
    }

    if ( !strcasecmp( arg, "flags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Toggle the room flags.\n\r", ch );
	    send_to_char( "Usage: redit flags <flag> [flag]...\n\r", ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg2 );
	    value = get_rflag( arg2 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
	    else
	    {
		if ( 1 << value == ROOM_PROTOTYPE
		&&   get_trust( ch ) < MAX_LEVEL )
		    send_to_char( "You cannot change the prototype flag.\n\r", ch );
		else
		    TOGGLE_BIT( location->room_flags, 1 << value );
	    }
	}
	return;
    }


    if ( !strcasecmp( arg, "sector" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Set the sector type.\n\r", ch );
	    send_to_char( "Usage: redit sector <value>\n\r", ch );
	    return;
	}
	location->sector_type = atoi( argument );
	if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
	{
	    location->sector_type = 1;
	    send_to_char( "Out of range\n\r.", ch ); 
	}
	else
	    send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "exkey" ) )
    {
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( arg2[0] == '\0' || arg3[0] == '\0' )
	{
	    send_to_char( "Usage: redit exkey <dir> <key vnum>\n\r", ch );
	    return;
	}
	edir = get_dir( arg2 );
	xit = location->exit[edir];
	value = atoi( arg3 );

	if ( !xit )
	{
	    send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	    return;
	}
	xit->key = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "exname" ) )
    {
	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Change or clear exit keywords.\n\r", ch );
	    send_to_char( "Usage: redit exname <dir> [keywords]\n\r", ch );
	    return;
	}

	edir = get_dir( arg2 );
	xit = location->exit[edir];

	if ( !xit )
	{
	    send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	    return;
	}
	STRFREE( xit->keyword );
	xit->keyword = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg, "exflags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Toggle or display exit flags.\n\r", ch );
	    send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg2 );

	edir = get_dir( arg2 );
	xit = location->exit[edir];

	if ( !xit )
	{
	    send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	    return;
	}
	if ( argument[0] == '\0' )
	{
	    sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ",
		edir, xit->keyword, xit->key );
	    for ( value = 0; value <= 6; value++ )
	    {
		if ( IS_SET( xit->exit_info, 1 << value ) )
		{
		    strcat( buf, ex_flags[value] );
		    strcat( buf, " " );
		}
	    }
	    strcat( buf, "]\n\r" );
	    send_to_char( buf, ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg2 );
	    value = get_exflag( arg2 );
	    if ( value < 0 || value > MAX_EXFLAG )
		ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
	    else
		TOGGLE_BIT( xit->exit_info, 1 << value );
	}
	return;
    }



    if ( !strcasecmp( arg, "ex_flags" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
	    case SUB_EAST : dir = 'e'; edir = 1; break;
	    case SUB_WEST : dir = 'w'; edir = 3; break;
	    case SUB_SOUTH: dir = 's'; edir = 2; break;
	    case SUB_UP   : dir = 'u'; edir = 4; break;
	    case SUB_DOWN : dir = 'd'; edir = 5; break;
	    default:
	    case SUB_NORTH: dir = 'n'; edir = 0; break;
	}

	value = get_exflag(arg2);
        if ( value < 0 )
	{
	    send_to_char("Bad exit flag. \n\r", ch);
	    return;
	}
	if ( (xit = location->exit[edir]) == NULL )
	{ 
	    sprintf(buf,"exit %c 1",dir);
	    do_redit(ch,buf);
	    xit = location->exit[edir];
	}     
	TOGGLE_BIT( xit->exit_info, 1 << value );
	return;
    }


    if ( !strcasecmp( arg, "ex_to_room" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
	    case SUB_EAST : dir = 'e'; edir = 1; break;
	    case SUB_WEST : dir = 'w'; edir = 3; break;
	    case SUB_SOUTH: dir = 's'; edir = 2; break;
	    case SUB_UP   : dir = 'u'; edir = 4; break;
	    case SUB_DOWN : dir = 'd'; edir = 5; break;
	    default:
	    case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	evnum = atoi(arg2);
	if ( evnum < 1 || evnum > 32766 )
	{
	    send_to_char( "Invalid room number.\n\r", ch );
	    return;
	}
	if ( (tmp = get_room_index( evnum )) == NULL )
	{
	    send_to_char( "Non-existant room.\n\r", ch );
	    return;
	}
	if ( (xit = location->exit[edir]) == NULL )
	{ 
	    sprintf(buf,"exit %c 1",dir);
	    do_redit(ch,buf);
	    xit = location->exit[edir];
	}     
	xit->vnum = evnum;
	return;
    }

    if ( !strcasecmp( arg, "ex_key" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
	    case SUB_EAST : dir = 'e'; edir = 1; break;
	    case SUB_WEST : dir = 'w'; edir = 3; break;
	    case SUB_SOUTH: dir = 's'; edir = 2; break;
	    case SUB_UP   : dir = 'u'; edir = 4; break;
	    case SUB_DOWN : dir = 'd'; edir = 5; break;
	    default:
	    case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = location->exit[edir]) == NULL )
	{ 
	    sprintf(buf,"exit %c 1",dir);
	    do_redit(ch,buf);
	    xit = location->exit[edir];
	}     
	xit->key = atoi( arg2 );
	return;
    }

    if ( !strcasecmp( arg, "ex_exdesc" ) )  
    {
        switch(ch->inter_substate)
	{
	    case SUB_EAST : dir = 'e'; edir = 1; break;
	    case SUB_WEST : dir = 'w'; edir = 3; break;
	    case SUB_SOUTH: dir = 's'; edir = 2; break;
	    case SUB_UP   : dir = 'u'; edir = 4; break;
	    case SUB_DOWN : dir = 'd'; edir = 5; break;
	    default:
	    case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = location->exit[edir]) == NULL )
	{ 
	    sprintf(buf,"exit %c 1",dir);
	    do_redit(ch,buf);
	}     
	sprintf(buf,"exdesc %c %s",dir,argument);
	do_redit(ch,buf);
	return;
    }

    if ( !strcasecmp( arg, "ex_keywords" ) )  /* not called yet */
    {
        switch(ch->inter_substate)
	{
	    case SUB_EAST : dir = 'e'; edir = 1; break;
	    case SUB_WEST : dir = 'w'; edir = 3; break;
	    case SUB_SOUTH: dir = 's'; edir = 2; break;
	    case SUB_UP   : dir = 'u'; edir = 4; break;
	    case SUB_DOWN : dir = 'd'; edir = 5; break;
	    default:
	    case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = location->exit[edir]) == NULL )
	{ 
	    sprintf(buf, "exit %c 1", dir);
	    do_redit(ch,buf);
	    if ( (xit = location->exit[edir]) == NULL )
		return;
	}     
	sprintf( buf, "%s %s", xit->keyword, argument );
	STRFREE( xit->keyword );
	xit->keyword = STRALLOC( buf );
	return;
    }

    if ( !strcasecmp( arg, "exit" ) )
    {
	bool addexit, numnotdir;

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    send_to_char( "Create, change or remove an exit.\n\r", ch );
	    send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\n\r", ch );
	    return;
	}
	addexit = numnotdir = FALSE;
	switch( arg2[0] )
	{
	    default:	edir = get_dir( arg2);			  break;
	    case '+':	edir = get_dir( arg2+1);	addexit = TRUE;	  break;
	    case '#':	edir = atoi(arg2+1);	numnotdir = TRUE; break;  
	}
	if ( !arg3 || arg3[0] == '\0' )
	    evnum = 0;
	else
	    evnum = atoi( arg3 );
	xit = location->exit[edir];
	if ( !evnum )
	{
	    if ( xit )
	    {
		extract_exit(location, xit, edir);
		send_to_char( "Exit removed.\n\r", ch );
		return;
	    }
	    send_to_char( "No exit in that direction.\n\r", ch );
	    return;
	}
	if ( evnum < 1 || evnum > MAX_VNUM )
	{
	    send_to_char( "Invalid room number.\n\r", ch );
	    return;
	}
	if ( (tmp = get_room_index( evnum )) == NULL )
	{
	    send_to_char( "Non-existant room.\n\r", ch );
	    return;
	}
	if ( addexit || !xit )
	{
	    if ( numnotdir )
	    {
		send_to_char( "Cannot add an exit by number, sorry.\n\r", ch );
		return;
	    }
	    if ( addexit && xit && location->exit[edir] != NULL && 
		 location->exit[edir]->vnum == tmp->vnum )
	    {
		send_to_char( "There is already an exit in that direction leading to that location.\n\r", ch );
		return;
	    }
	    xit = make_exit( location, tmp, edir );
	    xit->keyword		= STRALLOC( "" );
	    xit->description		= STRALLOC( "" );
	    xit->key			= -1;
	    xit->exit_info		= 0;
	    act( "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
	}
	else
	    act( "Something is different...", ch, NULL, NULL, TO_ROOM );
	if ( xit->to_room != tmp )
	{
	    xit->to_room = tmp;
	    xit->vnum = evnum;
	}
	argument = one_argument( argument, arg3 );
	if ( arg3 && arg3[0] != '\0' )
	    xit->exit_info = atoi( arg3 );
	if ( argument && argument[0] != '\0' )
	{
	    one_argument( argument, arg3 );
	    ekey = atoi( arg3 );
	    if ( ekey != 0 || arg3[0] == '0' )
	    {
		argument = one_argument( argument, arg3 );
		xit->key = ekey;
	    }
	    if ( argument && argument[0] != '\0' )
	    {
		STRFREE( xit->keyword );
		xit->keyword = STRALLOC( argument );
	    }
	}
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /*
     * Twisted and evil, but works				-Martin
     * Makes an exit, and the reverse in one shot.
     */
    if ( !strcasecmp( arg, "bexit" ) )
    {
	EXIT_DATA *xit, *rxit;
	char tmpcmd[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *tmploc;
	int vnum, exnum;
	char rvnum[MAX_INPUT_LENGTH];
	bool numnotdir;

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    send_to_char( "Create, change or remove a two-way exit.\n\r", ch );
	    send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\n\r", ch );
	    return;
	}
	numnotdir = FALSE;
	switch( arg2[0] )
	{
	    default:
		edir = get_dir(  arg2 );
		break;
	    case '#':
		numnotdir = TRUE;
		edir = atoi( arg2+1 );
		break;
	    case '+':
		edir = get_dir( arg2+1 );
		break;
	}
	tmploc = location;
	exnum = edir;
	xit = tmploc->exit[edir];
	rxit = NULL;
	vnum = 0;
	rvnum[0] = '\0';
	if ( xit )
	{
	    vnum = xit->vnum;
	    if ( arg3[0] != '\0' )
		sprintf( rvnum, "%d", tmploc->vnum );
	    if ( xit->to_room )
		rxit = xit->to_room->exit[rev_dir[edir]];
	    else
		rxit = NULL;
	}
	sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
	do_redit( ch, tmpcmd );
	if ( numnotdir )
	    xit = tmploc->exit[exnum];
	else
	    xit = tmploc->exit[edir];
	if ( !rxit && xit )
	{
	    vnum = xit->vnum;
	    if ( arg3[0] != '\0' )
		sprintf( rvnum, "%d", tmploc->vnum );
	    if ( xit->to_room )
		rxit = xit->to_room->exit[rev_dir[edir]];
	    else
		rxit = NULL;
	}
	if ( vnum )
	{
	    sprintf( tmpcmd, "%d redit exit %d %s %s",
				vnum,
				rev_dir[edir],
				rvnum,
				argument );
	    do_at( ch, tmpcmd );
	}
	return;
    }


    if ( !strcasecmp( arg, "exdesc" ) )
    {
	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	   send_to_char( "Create or clear a description for an exit.\n\r", ch );
	   send_to_char( "Usage: redit exdesc <dir> [description]\n\r", ch );
	   return;
	}
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = location->exit[edir];
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = location->exit[edir];
	}
	if ( xit )
	{
	   STRFREE( xit->description );
	   if ( !argument || argument[0] == '\0' )
	     xit->description = STRALLOC( "" );
	   else
	   {
	     sprintf( buf, "%s\n\r", argument );
	     xit->description = STRALLOC( buf );
	   }
	   send_to_char( "Done.\n\r", ch );
	   return;
	}
	send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_redit;
    }
    else
	do_redit( ch, "" );
    return;
}

void mpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, char *argument )
{
    if ( mptype != -1 )
    {
	mprg->type = mptype;
	if ( mprg->arglist )
	    STRFREE( mprg->arglist );
	mprg->arglist = STRALLOC( argument );
    }
    ch->substate = SUB_MPROG_EDIT;
    ch->dest_buf = mprg;
    if ( !mprg->comlist )
	mprg->comlist = STRALLOC( "" );
    start_editing( ch, mprg->comlist );
    return;
}

/*
 * Mobprogram editing - cumbersome				-Martin
 */
void do_mpedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    CHAR_DATA  *victim;
    MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
    int value, mptype = -1, cnt;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't mpedit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MPROG_EDIT:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Chaste.\n\r", ch );
		bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  mprog	 = ch->dest_buf;
	  if ( mprog->comlist )
	    STRFREE( mprog->comlist );
	  mprog->comlist = copy_buffer( ch );
	  stop_editing( ch );
	  return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Command being one of:\n\r",			ch );
	send_to_char( "  add delete insert edit list\n\r",		ch );
	send_to_char( "Program being one of:\n\r",			ch );
	send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
	send_to_char( "  entry give bribe death time\n\r",	ch );
	return;
    }

    if ( get_trust( ch ) < MAX_LEVEL )
    {
      if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }
    else
    {
      if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
	send_to_char( "No one like that in all the realms.\n\r", ch );
	return;
      }
    }

    if ( get_trust( ch ) < victim->level || !IS_NPC(victim) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	return;
    }

    if ( !can_mmodify( ch, victim ) )
	return;

    if ( !IS_SET(victim->act, ACT_PROTOTYPE) )
    {
	send_to_char( "A mobile must have a prototype flag to be mpset.\n\r", ch );
	return;
    }

    mprog = victim->pIndexData->mobprogs;


    if ( !strcasecmp( arg2, "list" ) )
    {
	cnt = 0;
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	
	if(value < 1)
	{
		if(strcmp("full", arg3))
		{
			for(mprg = mprog; mprg; mprg = mprg->next)
			{
				ch_printf(ch, "%d>%s %s\n\r",
					++cnt,
					mprog_type_to_name(mprg->type),
					mprg->arglist);
			}
			
			return;
		}
		else
		{
			for(mprg = mprog; mprg; mprg = mprg->next)
			{
				ch_printf(ch, "%d>%s %s\n\r%s\n\r",
					++cnt,
					mprog_type_to_name(mprg->type),
					mprg->arglist,
					mprg->comlist);
			}
			
			return;
		}
	}
	
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
		if(++cnt == value)
		{
			ch_printf( ch, "%d>%s %s\n\r%s\n\r",
	      			cnt,
	      			mprog_type_to_name( mprg->type ),
	      			mprg->arglist,
	      			mprg->comlist );
	      		break;
	      	}	
	}
	
	if(!mprg)
		send_to_char("Program not found.\n\r", ch);
	
	return;
    }

    if ( !strcasecmp( arg2, "edit" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( arg4[0] != '\0' )
	{
	  mptype = get_mpflag( arg4 );
	  if ( mptype == -1 )
	  {
	    send_to_char( "Unknown program type.\n\r", ch );
	    return;
	  }
	}
	else
	  mptype = -1;
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mpedit( ch, mprg, mptype, argument );
		   victim->pIndexData->progtypes = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		   SET_BIT(victim->pIndexData->progtypes, mprg->type);
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "delete" ) )
    {
	int num;
	bool found;

	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0; found = FALSE;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mptype = mprg->type;
		found = TRUE;
		break;
	   }
	}
	if ( !found )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = num = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	   if ( mprg->type == mptype )
	     num++;
	if ( value == 1 )
	{
	   mprg_next = victim->pIndexData->mobprogs;
	   victim->pIndexData->mobprogs = mprg_next->next;
	}
	else
	for ( mprg = mprog; mprg; mprg = mprg_next )
	{
	   mprg_next = mprg->next;
	   if ( ++cnt == (value - 1) )
	   {
		mprg->next = mprg_next->next;
		break;
	   }
	}
	if ( mprg_next )
	{
	    STRFREE( mprg_next->arglist );
	    STRFREE( mprg_next->comlist );
	    DISPOSE( mprg_next );
	    if ( num <= 1 )
		REMOVE_BIT( victim->pIndexData->progtypes, mptype );
	    send_to_char( "Program removed.\n\r", ch );
	}
	return;
    }

    if ( !strcasecmp( arg2, "insert" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	mptype = get_mpflag( arg4 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	if ( value == 1 )
	{
	   CREATE( mprg, MPROG_DATA, 1 );
	   SET_BIT(victim->pIndexData->progtypes, mptype);
	   mpedit( ch, mprg, mptype, argument );
	   mprg->next = mprog;
	   victim->pIndexData->mobprogs = mprg;
	   return;
	}
	cnt = 1;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value && mprg->next )
	   {
		CREATE( mprg_next, MPROG_DATA, 1 );
		SET_BIT(victim->pIndexData->progtypes, mptype);
		mpedit( ch, mprg_next, mptype, argument );
		mprg_next->next = mprg->next;
		mprg->next	= mprg_next;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "add" ) )
    {
	mptype = get_mpflag( arg3 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( mprog != NULL )
	  for ( ; mprog->next; mprog = mprog->next );
	CREATE( mprg, MPROG_DATA, 1 );
	if ( mprog )
	  mprog->next			= mprg;
	else
	  victim->pIndexData->mobprogs	= mprg;
	SET_BIT(victim->pIndexData->progtypes,	mptype);
	mpedit( ch, mprg, mptype, argument );
	mprg->next = NULL;
	return;
    }

    do_mpedit( ch, "" );
}

char *sprint_reset( CHAR_DATA *ch, RESET_DATA *pReset, sh_int num, bool rlist )
{
    static char buf[MAX_STRING_LENGTH];
    char mobname[MAX_STRING_LENGTH];
    char roomname[MAX_STRING_LENGTH];
    char objname[MAX_STRING_LENGTH];
    static ROOM_INDEX_DATA *room;
    static OBJ_INDEX_DATA *obj, *obj2;
    static MOB_INDEX_DATA *mob;
    int rvnum = -1;

    if ( ch->in_room )
      rvnum = ch->in_room->vnum;
    if ( num == 1 )
    {
	room = NULL;
	obj  = NULL;
	obj2 = NULL;
	mob  = NULL;
    }

    switch( pReset->command )
    {
	default:
	  sprintf( buf, "%2d) *** BAD RESET: %c %d %d %d ***\n\r",
	  			num,
	  			pReset->command,
	  			pReset->arg1,
	  			pReset->arg2,
	  			pReset->arg3 );
	  break;
	case 'M':
	  mob = get_mob_index( pReset->arg1 );
	  room = get_room_index( pReset->arg3 );
	  if ( mob )
	    strcpy( mobname, mob->player_name );
	  else
	    strcpy( mobname, "Mobile: *BAD VNUM*" );
	  if ( room )
	    strcpy( roomname, room->name );
	  else
	    strcpy( roomname, "Room: *BAD VNUM*" );
	  sprintf( buf, "%2d) %s (%d) -> %s (%d) [%d]\n\r",
	  			num,
	  			mobname,
	  			pReset->arg1,
	  			roomname,
	  			pReset->arg3,
	  			pReset->arg2 );
	  break; 
	case 'E':
	  if ( !mob )
	      strcpy( mobname, "* ERROR: NO MOBILE! *" );
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name );
	  sprintf( buf, "%2d) %s (%d) -> %s (%s) [%d]\n\r",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			mobname,
	  			wear_locs[pReset->arg3],
	  			pReset->arg2 );
	  break;
	case 'H':
	  if ( pReset->arg1 > 0
	  &&  (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	  if ( !obj )
	      strcpy( objname, "Object: *NULL obj*" );
	  sprintf( buf, "%2d) Hide %s (%d)\n\r",
	  			num,
	  			objname,
	  			obj ? obj->vnum : pReset->arg1 );
	  break;
	case 'G':
	  if ( !mob )
	      strcpy( mobname, "* ERROR: NO MOBILE! *" );
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name );
	  sprintf( buf, "%2d) %s (%d) -> %s (carry) [%d]\n\r",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			mobname,
	  			pReset->arg2 );
	  break;
	case 'O':
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name );
	  room = get_room_index( pReset->arg3 );
	  if ( !room )
	      strcpy( roomname, "Room: *BAD VNUM*" );
	  else
	      strcpy( roomname, room->name );
	  sprintf( buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\n\r",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			roomname,
	  			pReset->arg3,
	  			pReset->arg2 );
	  break;
	case 'P':
	  if ( (obj2 = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object1: *BAD VNUM*" );
	  else
	      strcpy( objname, obj2->name );
	  if ( pReset->arg3 > 0
	  &&  (obj = get_obj_index( pReset->arg3 )) == NULL )
	      strcpy( roomname, "Object2: *BAD VNUM*" );
	  else
	  if ( !obj )
	      strcpy( roomname, "Object2: *NULL obj*" );
	  else
	      strcpy( roomname, obj->name );
	  sprintf( buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\n\r",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			roomname,
	  			obj ? obj->vnum : pReset->arg3,
	  			pReset->arg2 );
	  break;
	case 'D':
	  if ( pReset->arg2 < 0 || pReset->arg2 > 5 )
		pReset->arg2 = 0;
	  if ( (room = get_room_index( pReset->arg1 )) == NULL )
	  {
		strcpy( roomname, "Room: *BAD VNUM*" );
		sprintf( objname, "%s (no exit)",
				dir_name[pReset->arg2] );
	  }
	  else
	  {
		strcpy( roomname, room->name );
		sprintf( objname, "%s%s",
				dir_name[pReset->arg2],
		room->exit[pReset->arg2] ? "" : " (NO EXIT!)" );
	  }
	  switch( pReset->arg3 )
	  {
	    default:	strcpy( mobname, "(* ERROR *)" );	break;
	    case 0:	strcpy( mobname, "Open" );		break;
	    case 1:	strcpy( mobname, "Close" );		break;
	    case 2:	strcpy( mobname, "Close and lock" );	break;
	  }
	  sprintf( buf, "%2d) %s [%d] the %s [%d] door %s (%d)\n\r",
	  			num,
	  			mobname,
	  			pReset->arg3,
	  			objname,
	  			pReset->arg2,
	  			roomname,
	  			pReset->arg1 );
	  break;
	case 'R':
	  if ( (room = get_room_index( pReset->arg1 )) == NULL )
		strcpy( roomname, "Room: *BAD VNUM*" );
	  else
		strcpy( roomname, room->name );
	  sprintf( buf, "%2d) Randomize exits 0 to %d -> %s (%d)\n\r",
	  			num,
	  			pReset->arg2,
	  			roomname,
	  			pReset->arg1 );
	  break;
    }
    if ( rlist && (!room || (room && room->vnum != rvnum)) )
	return NULL;
    return buf;
}

void do_astat( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    bool found;
 

    found = FALSE; 
    for ( tarea = first_area; tarea; tarea = tarea->next )
	if ( !strcasecmp( tarea->filename, argument ) )
	{
	  found = TRUE;
	  break;
	}


    if ( !found )
    {
      if ( argument && argument[0] != '\0' ) 
      {
 	send_to_char( "Area not found.\n\r", ch );
	return;
      }
      else
      {
        tarea = ch->in_room->area;
      }
    }

    ch_printf( ch, "Name: %s\n\rFilename: %-20s\n\r",
			tarea->name,
			tarea->filename);
    ch_printf( ch, "Authors: %s\n\rAge: %d   Number of players: %d\n\r",
			tarea->authors,
			tarea->age,
			tarea->nplayer );
    ch_printf( ch, "low_room: %5d  hi_room: %d\n\r",
			tarea->low_r_vnum,
			tarea->hi_r_vnum );
    ch_printf( ch, "low_obj : %5d  hi_obj : %d\n\r",
			tarea->low_o_vnum,
			tarea->hi_o_vnum );
    ch_printf( ch, "low_mob : %5d  hi_mob : %d\n\r",
			tarea->low_m_vnum,
			tarea->hi_m_vnum );
    ch_printf( ch, "soft range: %d - %d.  hard range: %d - %d.\n\r",
			tarea->low_soft_range, 
			tarea->hi_soft_range,
			tarea->low_hard_range, 
			tarea->hi_hard_range );
    ch_printf( ch, "Resetmsg: %s\n\r", tarea->resetmsg ? tarea->resetmsg
						: "(default)" ); /* Rennard */
}


void do_aset( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool proto, found;
    int vnum, value;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    vnum = atoi( argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Usage: aset <area filename> <field> <value>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  low_room hi_room low_obj hi_obj low_mob hi_mob\n\r", ch );
	send_to_char( "  name filename low_soft hi_soft low_hard hi_hard\n\r", ch );
	send_to_char( "  author resetmsg resetfreq flags\n\r", ch );
	return;
    }

    found = FALSE; proto = FALSE;
    for ( tarea = first_area; tarea; tarea = tarea->next )
	if ( !strcasecmp( tarea->filename, arg1 ) )
	{
	  found = TRUE;
	  break;
	}

    if ( !found )
    {
	send_to_char( "Area not found.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "name" ) )
    {
	STRFREE( tarea->name );
	tarea->name = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "filename" ) )
    {
	STRFREE( tarea->filename );
	tarea->filename = str_dup( argument );
	write_area_list( );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "low_room" ) )
    {
	tarea->low_r_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hi_room" ) )
    {
	tarea->hi_r_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "low_obj" ) )
    {
	tarea->low_o_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hi_obj" ) )
    {
	tarea->hi_o_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "low_mob" ) )
    {
	tarea->low_m_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hi_mob" ) )
    {
	tarea->hi_m_vnum = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "low_soft" ) )
    {
	if ( vnum < 0 || vnum > MAX_LEVEL )
        {
	    send_to_char( "That is not an acceptable value.\n\r", ch);
	    return;
	}

	tarea->low_soft_range = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hi_soft" ) )
    {
	if ( vnum < 0 || vnum > MAX_LEVEL )
        {
	    send_to_char( "That is not an acceptable value.\n\r", ch);
	    return;
	}

	tarea->hi_soft_range = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "low_hard" ) )
    {
	if ( vnum < 0 || vnum > MAX_LEVEL )
        {
	    send_to_char( "That is not an acceptable value.\n\r", ch);
	    return;
	}

	tarea->low_hard_range = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "hi_hard" ) )
    {
	if ( vnum < 0 || vnum > MAX_LEVEL )
        {
	    send_to_char( "That is not an acceptable value.\n\r", ch);
	    return;
	}

	tarea->hi_hard_range = vnum;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "authors" ) )
    {
	STRFREE( tarea->authors );
	tarea->authors = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !strcasecmp( arg2, "resetmsg" ) )
    {
        if ( tarea->resetmsg )
          STRFREE( tarea->resetmsg );
        if ( strcasecmp( argument, "clear" ) )
          tarea->resetmsg = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    } /* Rennard */


    if ( !strcasecmp( arg2, "flags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: aset <filename> flags <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_areaflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
           if ( IS_SET( tarea->flags, 1 << value ) )
	         REMOVE_BIT( tarea->flags, 1 << value );
	       else
	         SET_BIT( tarea->flags, 1 << value );
	   }
        }
        return;
    }

    do_aset( ch, "" );
    return;
}
