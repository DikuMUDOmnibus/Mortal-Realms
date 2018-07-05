
/*
 * This file relies heavily on the fact that your linked lists are correct,
 * and that pArea->reset_first is the first reset in pArea.  Likewise,
 * pArea->reset_last *MUST* be the last reset in pArea.  Weird and
 * wonderful things will happen if any of your lists are messed up, none
 * of them good.  The most important are your pRoom->contents,
 * pRoom->people, rch->carrying, obj->contains, and pArea->reset_first ..
 * pArea->reset_last.  -- Altrag
 */
 
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"


/* Externals */
extern	int	top_reset;
char *		sprint_reset	args( ( CHAR_DATA *ch, RESET_DATA *pReset,
					sh_int num, bool rlist ) );
RESET_DATA *	parse_reset	args( ( AREA_DATA *tarea, char *argument,
					CHAR_DATA *ch ) );
int		get_wearloc	args( ( char *type ) );
int		get_trapflag	args( ( char *flag ) );
int		get_exflag	args( ( char *flag ) );
int		get_rflag	args( ( char *flag ) );
extern	char *	const		wear_locs[];
extern	char *	const		ex_flags[];



bool is_room_reset  args( ( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                            AREA_DATA *pArea ) );
void add_obj_reset  args( ( AREA_DATA *pArea, char cm, OBJ_DATA *obj,
                            int v2, int v3 ) );
void delete_reset   args( ( AREA_DATA *pArea, RESET_DATA *pReset ) );
void instaroom      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    bool dodoors ) );
#define RID ROOM_INDEX_DATA
RID *find_room      args( ( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom ) );
#undef RID
void edit_reset     args( ( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
                            ROOM_INDEX_DATA *aRoom ) );
#define RD RESET_DATA
RD *find_reset      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    int num ) );
#undef RD
void list_resets    args( ( CHAR_DATA *ch, AREA_DATA *pArea,
			    ROOM_INDEX_DATA *pRoom, int start, int end ) );


bool is_existing_mob  ( RESET_DATA *pReset )
  {
  CHAR_DATA *mob;

  for( mob=first_char; mob!=NULL; mob=mob->next )
    if( IS_NPC(mob) && mob->reset == pReset )
      return( TRUE );

  return( FALSE );
  }

bool is_existing_obj  ( RESET_DATA *pReset )
  {
  OBJ_DATA *obj;

  for( obj=first_object; obj!=NULL; obj=obj->next )
    if( obj->reset == pReset )
      return( TRUE );

  return( FALSE );
  }


RESET_DATA *find_reset(AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, int numb)
{
  RESET_DATA *pReset;
  int num = 0;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    if ( is_room_reset(pReset, pRoom, pArea) && ++num >= numb )
      return pReset;
  return NULL;
}

/* This is one loopy function.  Ugh. -- Altrag */
bool is_room_reset( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                    AREA_DATA *pArea )
{
  ROOM_INDEX_DATA *pRoom;
  RESET_DATA *reset;
  int pr;
  
  if ( !aRoom )
    return TRUE;
  switch( pReset->command )
  {
  case 'M':
  case 'O':
    pRoom = get_room_index( pReset->arg3 );
    if ( !pRoom || pRoom != aRoom )
      return FALSE;
    return TRUE;
  case 'P':
  case 'T':
  case 'H':
    if ( pReset->command == 'H' )
      pr = pReset->arg1;
    else
      pr = pReset->arg3;
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           (!pr || pr == reset->arg1) && get_obj_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'G':
  case 'E':
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( reset->command == 'M' && get_mob_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'D':
  case 'R':
    pRoom = get_room_index( pReset->arg1 );
    if ( !pRoom || pRoom->area != pArea || (aRoom && pRoom != aRoom) )
      return FALSE;
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

ROOM_INDEX_DATA *find_room( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( pRoom )
    return pRoom;
  one_argument(argument, arg);
  if ( !is_number(arg) && arg[0] != '\0' )
  {
    send_to_char( "Reset to which room?\n\r", ch );
    return NULL;
  }
  if ( arg[0] == '\0' )
    pRoom = ch->in_room;
  else
    pRoom = get_room_index(atoi(arg));
  if ( !pRoom )
  {
    send_to_char( "Room does not exist.\n\r", ch );
    return NULL;
  }
  return pRoom;
}

/* Separate function for recursive purposes */
#define DEL_RESET(area, reset, rprev) \
do { \
  rprev = reset->prev; \
  delete_reset(area, reset); \
  reset = rprev; \
  continue; \
} while(0)
void delete_reset( AREA_DATA *pArea, RESET_DATA *pReset )
{
  RESET_DATA *reset;
  RESET_DATA *reset_prev;

  if ( pReset->command == 'M' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      /* Break when a new mob found */
      if ( reset->command == 'M' )
        break;
      /* Delete anything mob is holding */
      if ( reset->command == 'G' || reset->command == 'E' )
        DEL_RESET(pArea, reset, reset_prev);
    }
  }
  else if ( pReset->command == 'O' || pReset->command == 'P' ||
            pReset->command == 'G' || pReset->command == 'E' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      if ( reset->command == 'T' &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'H' &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      /* Delete nested objects, even if they are the same object. */
      if ( reset->command == 'P' && (reset->arg3 > 0 ||
           pReset->command != 'P' ) &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);

      /* Break when a new object of same type is found */
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           reset->arg1 == pReset->arg1 )
        break;
    }
  }
  if ( pReset == pArea->last_mob_reset )
    pArea->last_mob_reset = NULL;
  if ( pReset == pArea->last_obj_reset )
    pArea->last_obj_reset = NULL;
  UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
  DISPOSE(pReset);
  return;
}
#undef DEL_RESET

RESET_DATA *find_oreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No object resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    OBJ_INDEX_DATA *pObjTo = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      if ( (pObjTo = get_obj_index(reset->arg1)) &&
            is_name(arg, pObjTo->name) && ++cnt == num )
        break;
    }
    if ( !pObjTo || !reset )
    {
      send_to_char( "To object not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

RESET_DATA *find_mreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No mobile resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    MOB_INDEX_DATA *pMob = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      if ( (pMob = get_mob_index(reset->arg1)) &&
            is_name(arg, pMob->player_name) && ++cnt == num )
        break;
    }
    if ( !pMob || !reset )
    {
      send_to_char( "Mobile not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

void do_rreset( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  
  if ( ch->substate == SUB_REPEATCMD )
  {
    pRoom = ch->dest_buf;
    if ( !pRoom )
    {
      send_to_char( "Your room pointer got lost.  Reset mode off.\n\r", ch);
      bug("do_rreset: %s's dest_buf points to invalid room", (int)ch->name);
    }
    ch->substate = SUB_NONE;
    ch->dest_buf = NULL;
    return;
  }
  else
    pRoom = ch->in_room;
  if ( !can_rmodify(ch, pRoom) )
    return;
  edit_reset(ch, argument, pRoom->area, pRoom);
  return;
}
void edit_reset( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
		 ROOM_INDEX_DATA *aRoom )
{
  char arg[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL;
  RESET_DATA *reset = NULL;
  MOB_INDEX_DATA *pMob = NULL;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  int num = 0;
  int vnum;
  char *origarg = argument;
  
  argument = one_argument(argument, arg);
  if ( !*arg || !strcasecmp(arg, "?") )
  {
    char *nm = (ch->substate == SUB_REPEATCMD ? "" : (aRoom ? "rreset "
    		: "reset "));
    char *rn = (aRoom ? "" : " [room#]");
    ch_printf(ch, "Syntax: %s<list|edit|delete|add|insert|place%s>\n\r",
        nm, (aRoom ? "" : "|area"));
    ch_printf( ch, "Syntax: %sremove <#>\n\r", nm );
    ch_printf( ch, "Syntax: %smobile <mob#> [limit]%s\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> [limit [room%s]]\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> give <mob name> [limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> equip <mob name> <location> "
        "[limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> put <to_obj name> [limit]\n\r",
        nm );
    ch_printf( ch, "Syntax: %srandom <last dir>%s\n\r", nm, rn );
    if ( !aRoom )
    {
      send_to_char( "\n\r[room#] will default to the room you are in, "
          "if unspecified.\n\r", ch );
    }
    return;
  }
  if ( !strcasecmp(arg, "on") )
  {
    ch->substate = SUB_REPEATCMD;
    ch->dest_buf = (aRoom ? (void *)aRoom : (void *)pArea);
    send_to_char( "Reset mode on.\n\r", ch );
    return;
  }
  if ( !aRoom && !strcasecmp(arg, "area") )
  {
    if ( !pArea->first_reset )
    {
      send_to_char( "You don't have any resets defined.\n\r", ch );
      return;
    }
    num = pArea->nplayer;
    pArea->nplayer = 0;
    reset_area(pArea);
    pArea->nplayer = num;
    send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !strcasecmp(arg, "list") )
  {
    int start, end;
    
    argument = one_argument(argument, arg);
    start = is_number(arg) ? atoi(arg) : -1;
    argument = one_argument(argument, arg);
    end   = is_number(arg) ? atoi(arg) : -1;
    list_resets(ch, pArea, aRoom, start, end);
    return;
  }
  
  if ( !strcasecmp(arg, "edit") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset edit <number> <command>\n\r", ch );
      return;
    }
    if ( !(pReset = find_reset(pArea, aRoom, num)) )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( !(reset = parse_reset(pArea, argument, ch)) )
    {
      send_to_char( "Error in reset.  Reset not changed.\n\r", ch );
      return;
    }
    reset->prev = pReset->prev;
    reset->next = pReset->next;
    if ( !pReset->prev )
      pArea->first_reset = reset;
    else
      pReset->prev->next = reset;
    if ( !pReset->next )
      pArea->last_reset  = reset;
    else
      pReset->next->prev = reset;
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !strcasecmp(arg, "add") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    add_reset(pArea, pReset->command, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !strcasecmp(arg, "place") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    place_reset(pArea, pReset->command, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !strcasecmp(arg, "insert") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset insert <number> <command>\n\r", ch );
      return;
    }
    num = atoi(arg);
    if ( (reset = find_reset(pArea, aRoom, num)) == NULL )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not inserted.\n\r", ch );
      return;
    }
    INSERT(pReset, reset, pArea->first_reset, next, prev);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !strcasecmp(arg, "delete") )
  {
    int start, end;
    bool found;
    
    if ( !*argument )
    {
      send_to_char( "Usage: reset delete <start> [end]\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    start = is_number(arg) ? atoi(arg) : -1;
    end   = is_number(arg) ? atoi(arg) : -1;
    num = 0; found = FALSE;
    for ( pReset = pArea->first_reset; pReset; pReset = reset )
    {
      reset = pReset->next;
      if ( !is_room_reset(pReset, aRoom, pArea) )
        continue;
      if ( start > ++num )
        continue;
      if ( (end != -1 && num > end) || (end == -1 && found) )
        return;
      UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      if ( pReset == pArea->last_mob_reset )
        pArea->last_mob_reset = NULL;
      DISPOSE(pReset);
      top_reset--;
      found = TRUE;
    }
    if ( !found )
      send_to_char( "Reset not found.\n\r", ch );
    else
      send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !strcasecmp(arg, "remove") )
  {
    int iarg;
    
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Delete which reset?\n\r", ch );
      return;
    }
    iarg = atoi(arg);
    for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    {
      if ( is_room_reset( pReset, aRoom, pArea ) && ++num == iarg )
        break;
    }
    if ( !pReset )
    {
      send_to_char( "Reset does not exist.\n\r", ch );
      return;
    }
    delete_reset( pArea, pReset );
    send_to_char( "Reset deleted.\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "mobile" ) )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which mobile vnum?\n\r", ch );
      return;
    }
    if ( !(pMob = get_mob_index(atoi(arg))) )
    {
      send_to_char( "Mobile does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      num = 1;
    else if ( !is_number(arg) )
    {
      send_to_char( "Reset how many mobiles?\n\r", ch );
      return;
    }
    else
      num = atoi(arg);
    if ( !(pRoom = find_room(ch, argument, aRoom)) )
      return;
    pReset = make_reset('M', pMob->vnum, num, pRoom->vnum);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Mobile reset added.\n\r", ch );
    return;
  }
  if ( !str_prefix(arg, "object") )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which object vnum?\n\r", ch );
      return;
    }
    if ( !(pObj = get_obj_index(atoi(arg))) )
    {
      send_to_char( "Object does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      strcpy(arg, "room");
    if ( !str_prefix( arg, "put" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
/*      pReset = make_reset('P', 1, pObj->vnum, num, reset->arg1);*/
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('P', pObj->vnum, vnum, 0);
      /* Grumble.. insert puts pReset before reset, and we need it after,
         so we make a hackup and reverse all the list params.. :P.. */
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset in object created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "give" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('G', pObj->vnum, vnum, 0);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset to mobile created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "equip" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      num = get_wearloc(argument);
      if ( num < 0 )
      {
        send_to_char( "Reset object to which location?\n\r", ch );
        return;
      }
      for ( pReset = reset->next; pReset; pReset = pReset->next )
      {
        if ( pReset->command == 'M' )
          break;
        if ( pReset->command == 'E' && pReset->arg3 == num )
        {
          send_to_char( "Mobile already has an item equipped there.\n\r", ch);
          return;
        }
      }
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('E', pObj->vnum, vnum, num);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset equipped by mobile created.\n\r", ch );
      return;
    }
    if ( arg[0] == '\0' || !(num = (int)strcasecmp(arg, "room")) ||
         is_number(arg) )
    {
      if ( !(bool)num )
        argument = one_argument(argument, arg);
      if ( !(pRoom = find_room(ch, argument, aRoom)) )
        return;
      if ( pRoom->area != pArea )
      {
        send_to_char( "Cannot reset objects to other areas.\n\r", ch );
        return;
      }
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('O', pObj->vnum, vnum, pRoom->vnum);
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char( "Object reset added.\n\r", ch );
      return;
    }
    send_to_char( "Reset object to where?\n\r", ch );
    return;
  }
  if ( !strcasecmp(arg, "random") )
  {
    argument = one_argument(argument, arg);
    vnum = get_dir( arg );
    if ( vnum < 0 || vnum > 9 )
    {
      send_to_char( "Reset which random doors?\n\r", ch );
      return;
    }
    if ( vnum == 0 )
    {
      send_to_char( "There is no point in randomizing one door.\n\r", ch );
      return;
    }
    pRoom = find_room(ch, argument, aRoom);
    if ( pRoom->area != pArea )
    {
      send_to_char( "Cannot randomize doors in other areas.\n\r", ch );
      return;
    }
    pReset = make_reset('R', pRoom->vnum, vnum, 0);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Reset random doors created.\n\r", ch );
    return;
  }
  if ( ch->substate == SUB_REPEATCMD )
  {
    ch->substate = SUB_NONE;
    interpret(ch, origarg);
    ch->substate = SUB_REPEATCMD;
    ch->last_cmd = (aRoom ? do_rreset : do_reset);
  }
  else
    edit_reset(ch, "", pArea, aRoom);
  return;
}

void do_reset( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea = NULL;
  char arg[MAX_INPUT_LENGTH];
  char *parg;
  
  parg = one_argument(argument, arg);
  if ( ch->substate == SUB_REPEATCMD )
  {
    pArea = ch->dest_buf;
    if ( pArea && pArea != ch->pcdata->area && pArea != ch->in_room->area )
    {
      AREA_DATA *tmp;
      
      for ( tmp = first_area; tmp; tmp = tmp->next )
        if ( tmp == pArea )
          break;
      if ( !tmp )
      {
        send_to_char("Your area pointer got lost.  Reset mode off.\n\r", ch);
        bug("do_reset: %s's dest_buf points to invalid area",
		ch->name);	/* why was this cast to an int? */
        ch->substate = SUB_NONE;
        ch->dest_buf = NULL;
        return;
      }
    }
    if ( !*arg )
    {
      ch_printf(ch, "Editing resets for area: %s\n\r", pArea->name);
      return;
    }
    if ( !strcasecmp(arg, "done") || !strcasecmp(arg, "off") )
    {
      send_to_char( "Reset mode off.\n\r", ch );
      ch->substate = SUB_NONE;
      ch->dest_buf = NULL;
      return;
    }
  }
  if ( !pArea && get_trust(ch) > MAX_LEVEL )
  {
    char fname[80];
    
    sprintf(fname, "%s.are", capitalize(arg));
    for ( pArea = first_area; pArea; pArea = pArea->next )
      if ( !strcasecmp(fname, pArea->filename) )
      {
        argument = parg;
        break;
      }
    if ( !pArea )
      pArea = ch->pcdata->area;
    if ( !pArea )
      pArea = ch->in_room->area;
  }
  else
    pArea = ch->pcdata->area;
  if ( !pArea )
  {
    send_to_char( "You do not have an assigned area.\n\r", ch );
    return;
  }
  edit_reset(ch, argument, pArea, NULL);
  return;
}


void add_obj_reset( AREA_DATA *pArea, char cm, OBJ_DATA *obj, int v2, int v3 )
{
  OBJ_DATA *inobj;
  static int iNest;
  
  add_reset( pArea, cm, obj->pIndexData->vnum, v2, v3 );
  /* Only add hide for in-room objects that are hidden and cant be moved, as
     hide is an update reset, not a load-only reset. */
  if ( cm == 'P' )
    iNest++;
  for ( inobj = obj->first_content; inobj; inobj = inobj->next_content )
    add_obj_reset( pArea, 'P', inobj, 1, 0 );
  if ( cm == 'P' )
    iNest--;
  return;
}

void instaroom( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, bool dodoors )
{
  CHAR_DATA *rch;
  OBJ_DATA *obj;
  
  for ( rch = pRoom->first_person; rch; rch = rch->next_in_room )
  {
    if ( !IS_NPC(rch) )
      continue;
    add_reset( pArea, 'M', rch->pIndexData->vnum, rch->pIndexData->count,
               pRoom->vnum );
    for ( obj = rch->first_carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE )
        add_obj_reset( pArea, 'G', obj, 1, 0 );
      else
        add_obj_reset( pArea, 'E', obj, 1, obj->wear_loc );
    }
  }
  for ( obj = pRoom->first_content; obj; obj = obj->next_content )
  {
    add_obj_reset( pArea, 'O', obj, 1, pRoom->vnum );
  }
  if ( dodoors )
  {
    EXIT_DATA *pexit;
    int door;
    for ( door = 0 ; door<6; door++)
    {
      int state = 0;
      if ( (pexit = pRoom->exit[door] ) == NULL) 
	continue;
      if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
        continue;
      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
        if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
          state = 2;
        else
          state = 1;
      }
      add_reset( pArea, 'D', pRoom->vnum, door, state );
    }
  }
  return;
}

void wipe_resets( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA *pReset;
  
  for ( pReset = pArea->first_reset; pReset; )
  {
    if ( pReset->command != 'R' && is_room_reset( pReset, pRoom, pArea ) )
    {
      /* Resets always go forward, so we can safely use the previous reset,
         providing it exists, or first_reset if it doesnt.  -- Altrag */
      RESET_DATA *prev = pReset->prev;
      
      delete_reset(pArea, pReset);
      pReset = (prev ? prev->next : pArea->first_reset);
    }
    else
      pReset = pReset->next;
  }
  return;
}

void do_instaroom( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) || get_trust(ch) < MAX_LEVEL || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  argument = one_argument(argument, arg);
  if ( !strcasecmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( !(pRoom = find_room(ch, arg, NULL)) )
  {
    send_to_char( "Room doesn't exist.\n\r", ch );
    return;
  }
  if ( !can_rmodify(ch, pRoom) )
    return;
  if ( pRoom->area != pArea && get_trust(ch) < MAX_LEVEL )
  {
    send_to_char( "You cannot reset that room.\n\r", ch );
    return;
  }
  if ( pArea->first_reset )
    wipe_resets(pArea, pRoom);
  instaroom(pArea, pRoom, dodoors);
  send_to_char( "Room resets installed.\n\r", ch );
}

void do_instazone( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  int vnum;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;

  if ( IS_NPC(ch) || get_trust(ch) < MAX_LEVEL || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  if ( !strcasecmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( pArea->first_reset )
    wipe_resets(pArea, NULL);
  for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
  {
    if ( !(pRoom = get_room_index(vnum)) || pRoom->area != pArea )
      continue;
    instaroom( pArea, pRoom, dodoors );
  }
  send_to_char( "Area resets installed.\n\r", ch );
  return;
}

int generate_itemlevel( AREA_DATA *pArea, OBJ_INDEX_DATA *pObjIndex )
{
    int olevel;
    int min = UMAX(pArea->low_soft_range, 1);
    int max = UMIN(pArea->hi_soft_range, min + 15);

    if ( pObjIndex->level > 0 )
	olevel = UMIN(pObjIndex->level, MAX_LEVEL);
    else
	switch ( pObjIndex->item_type )
	{
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  min, max );	break;
	    case ITEM_POTION:	olevel = number_range(  min, max );	break;
	    case ITEM_SCROLL:	olevel = pObjIndex->value[0];		break;
	    case ITEM_WAND:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_STAFF:	olevel = number_range( min+9, max+5 );	break;
	    case ITEM_ARMOR:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_WEAPON:	olevel = number_range( min+4, max+1 );	break;
	}
    return olevel;
}

/*
 * Reset one area.
 */

void reset_area( AREA_DATA *pArea )
{
    RESET_DATA *pReset;
    CHAR_DATA *mob;
    ROOM_INDEX_DATA *pRoomIndex;
    bool last;
    int level;
    char buf[MAX_INPUT_LENGTH];
    extern bool fBootDb;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_INDEX_DATA *pObjToIndex;
    EXIT_DATA *pexit;
    OBJ_DATA *obj;
    OBJ_DATA *obj_to;
    ROOM_INDEX_DATA *pRoomIndexPrev;
    int d0;
    int d1;

    pRoomIndex=NULL;
    mob         = NULL;   /* Pointer to CHAR_DATA of last loaded mob, or NULL */
    last        = TRUE;
    level       = 0;
    if( fBootDb )
      {
      pArea->average_level = 0;
      pArea->count=0;
      pArea->tmp=0;
      }

    for ( pReset = pArea->first_reset; pReset != NULL; pReset = pReset->next )
    {
	switch ( pReset->command )
	{
	default:
	    bug( "Reset_area: bad command %c.", pReset->command );
	    break;

	case 'M':
            mob = pReset->mob;
            if( mob != NULL )
	      {
		    last = FALSE;
		    mob = NULL;
		    break;
	      } 


            /* Check for reloading mobs - Chaos  4/2/99 
            if( is_existing_mob( pReset ) )
              continue; */

	    if ( ( pMobIndex = get_mob_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'M': bad mobile vnum %u.", pReset->arg1 );
		continue;
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'M': bad room vnum %u.", pReset->arg3 );
		continue;
	    }

	    level = pMobIndex->level ;


	    mob = create_mobile( pMobIndex );
	    mob->reset=pReset;
            pReset->mob = mob;

	    pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
	    if ( pRoomIndexPrev != NULL
		&&   IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP) )
		    SET_BIT(mob->act, ACT_PET);

	    if ( room_is_dark( pRoomIndex ) )
		SET_BIT(mob->affected_by, AFF_INFRARED);
            if(IS_SET(pRoomIndex->room_flags,ROOM_IS_CASTLE))
              pMobIndex->creator_pvnum=pRoomIndex->creator_pvnum;

	    char_to_room( mob, pRoomIndex );

	    if( fBootDb )
	      {
              if( !IS_SET( pMobIndex->act , ACT_AGGRESSIVE ))
	        pArea->average_level += (level/3);
              else
	        pArea->average_level += level;
	      pArea->tmp ++;
	      }
	    last  = TRUE;
	    break;

	case 'O':
               /* Let's not make more than one */
          if( pReset->obj != NULL )
	    {
		last = FALSE;
		break;
	    }


	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'O': bad obj vnum %u.", pReset->arg1 );
		continue;
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'O': bad room vnum %u.", pReset->arg3 );
		continue;
	    }

            /* Check for reloading objs - Chaos  4/2/99 
            if( is_existing_obj( pReset ) )
              continue; */

	if ( number_range(1,5)!=1)
	    if ( pObjIndex->total_objects >= pObjIndex->max_objs )
	    {
		last = FALSE;
		break;
	    }
      
	    obj       = create_object( pObjIndex, level/2 );
            obj->reset = pReset;
            pReset->obj = obj;
	    obj_to_room( obj, pRoomIndex );
	    last = TRUE;
	    break;

	case 'P':
               /* Let's not make more than one */
          if( pReset->obj != NULL )
	    {
		last = FALSE;
		break;
	    }
	    
	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'P': bad obj vnum %u.", pReset->arg1 );
		continue;
	    }

	    if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'P': bad vnum %u.", pReset->arg3 );
		continue;
	    }

            /* Check for reloading objs - Chaos  4/2/99 
            if( is_existing_obj( pReset ) ) 
              continue; */
            
           obj_to = NULL;
           if( pReset->container != NULL )
               obj_to = pReset->container->obj;
           if( obj_to == NULL )
	    {
		/* bug( "Reset_area: 'P': bad to_obj %d.", pReset->arg3 ); */
		continue;
	    }
 

	    if ( pObjIndex->total_objects >= pObjIndex->max_objs &&
		  number_range(1,5)!=1)
	    {
		last = FALSE;
		break;
	    }

	    obj = create_object( pObjIndex, obj_to->level );
	    obj->reset = pReset;
            pReset->obj = obj;
	    obj_to_obj( obj, obj_to );
	    last = TRUE;
	    break;

	case 'G':
	case 'E':
               /* Let's not make more than one */
            if( pReset->obj != NULL )
	      {
		    last = FALSE;
		    break;
	      }

            mob = NULL;
            if( pReset->container != NULL )
              mob = pReset->container->mob;
            if( mob == NULL )
	      {
		    last = FALSE;
		    break;
	      }


	    if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
	      {
		    bug( "Reset_area: 'E' or 'G': bad vnum %u.", pReset->arg1 );
		    sprintf( buf, "Reset_area: 'E' or 'G': bad vnum %u.", pReset->arg1 );
	log_string( buf);
		    last = FALSE;
	break;
	      }

            /* Check for reloading objs - Chaos  4/2/99 
            if( is_existing_obj( pReset ) )
              continue; */

	    if ( mob->pIndexData->pShop != NULL )
	      {
		    int olevel;

		    switch ( pObjIndex->item_type )
		      {
		      default:          olevel = 1; break;
		      case ITEM_PILL:   olevel = 8; break;
		      case ITEM_POTION: olevel = 5; break;
		      case ITEM_SCROLL: olevel = 7; break;
		      case ITEM_WAND:   olevel = 12; break;
		      case ITEM_STAFF:  olevel = 17; break;
		      case ITEM_ARMOR:  olevel = 2; break;
		      case ITEM_WEAPON: olevel = 2; break;
		      }
		    obj = create_object( pObjIndex, olevel);
	     obj->reset = pReset;
             pReset->obj = obj;
		    SET_BIT( obj->extra_flags, ITEM_INVENTORY );
	      }
	    else
	      {
	if ( number_range(1,5)!=1)
	       if ( pObjIndex->total_objects >=  pObjIndex->max_objs)
		{
		      last = FALSE;
		      break;
		}
	obj = create_object( pObjIndex, mob->pIndexData->level*3/4 );
	obj->reset = pReset;
             pReset->obj = obj;
	      }
	    obj_to_char( obj, mob );
	    if ( pReset->command == 'E' )
		    equip_char( mob, obj, pReset->arg3 );
	    last = TRUE;
	    break;

	case 'D':
	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'D': bad vnum %u.", pReset->arg1 );
		continue;
	    }

	    if ( ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL )
		break;

            REMOVE_BIT( pexit->exit_info, EX_UNBARRED);
            REMOVE_BIT( pexit->exit_info, EX_BASHED);

	    switch ( pReset->arg3 )
	    {
	    case 0:
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		break;

	    case 1:
		SET_BIT(    pexit->exit_info, EX_CLOSED );
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		break;

	    case 2:
		SET_BIT(    pexit->exit_info, EX_CLOSED );
		SET_BIT(    pexit->exit_info, EX_LOCKED );
		break;
	    }

	    last = TRUE;
	    break;

	case 'R':
	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
	    {
		bug( "Reset_area: 'R': bad vnum %u.", pReset->arg1 );
		continue;
	    }

	    {

		for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
		{
		    d1                   = number_range( d0, pReset->arg2-1 );
		    pexit                = pRoomIndex->exit[d0];
		    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
		    pRoomIndex->exit[d1] = pexit;
		}
	    }
	    break;
	}
    }

   pArea->count ++;  /* count of each reset */
#ifdef USE_THREADS_ON_RESETS
   fprintf(stderr, "thread commiting suicide reset over\n");
   pArea->beingreset = NULL;
   pthread_detach(pthread_self());
   pthread_exit(NULL);
   return NULL;
#else
    return;
#endif
}
void list_resets( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
		  int start, int end )
{
  RESET_DATA *pReset;
  ROOM_INDEX_DATA *room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj, *obj2;
  OBJ_INDEX_DATA *lastobj;
  RESET_DATA *lo_reset;
  bool found;
  int num = 0;
  const char *rname = "???", *mname = "???", *oname = "???";
  char buf[256];
  char *pbuf;
  
  if ( !ch || !pArea )
    return;
  room = NULL;
  mob = NULL;
  obj = NULL;
  lastobj = NULL;
  lo_reset = NULL;
  found = FALSE;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    if ( !is_room_reset(pReset, pRoom, pArea) )
      continue;
    ++num;
    sprintf(buf, "%2d) ", num);
    pbuf = buf+strlen(buf);
    switch( pReset->command )
    {
    default:
      sprintf(pbuf, "*** BAD RESET: %c %d %d %d ***\n\r",
          pReset->command, pReset->arg1, pReset->arg2,
          pReset->arg3);
      break;
    case 'M':
      if ( !(mob = get_mob_index(pReset->arg1)) )
        mname = "Mobile: *BAD VNUM*";
      else
        mname = mob->player_name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "%s (%d) -> %s (%d) [%d]", mname, pReset->arg1, rname,
          pReset->arg3, pReset->arg2 );
      if ( !room )
        mob = NULL;
      if ( (room = get_room_index(pReset->arg3-1)) &&
            IS_SET(room->room_flags, ROOM_PET_SHOP) )
        strcat( buf, " (pet)\n\r" );
      else
        strcat( buf, "\n\r" );
      break;
    case 'G':
    case 'E':
      if ( !mob )
        mname = "* ERROR: NO MOBILE! *";
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      sprintf( pbuf, "%s (%d) -> %s (%s) [%d]", oname, pReset->arg1, mname,
          (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]),
          pReset->arg2 );
      if ( mob && mob->pShop )
        strcat( buf, " (shop)\n\r" );
      else
        strcat( buf, "\n\r" );
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'O':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "(object) %s (%d) -> %s (%d) [%d]\n\r", oname,
          pReset->arg1, rname, pReset->arg3, pReset->arg2 );
      if ( !room )
        obj = NULL;
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'P':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object1: *BAD VNUM*";
      else
        oname = obj->name;
      obj2 = NULL;
      if ( pReset->arg3 > 0 )
      {
        obj2 = get_obj_index(pReset->arg3);
        rname = (obj2 ? obj2->name : "Object2: *BAD VNUM*");
        lastobj = obj2;
      }
      else if ( !lastobj )
        rname = "Object2: *NULL obj*";
      else
      {
        RESET_DATA *reset;
        
        reset = lo_reset->next;
        if ( !reset )
          rname = "Object2: *BAD NESTING*";
        else if ( !(obj2 = get_obj_index(reset->arg1)) )
          rname = "Object2: *NESTED BAD VNUM*";
        else
          rname = obj2->name;
      }
      sprintf( pbuf, "(Put) %s (%d) -> %s (%d) [%d]\n\r", oname,
          pReset->arg1, rname, (obj2 ? obj2->vnum : pReset->arg3),
          pReset->arg2 );
      break;
    case 'D':
      {
      char *ef_name;
      
      pReset->arg2 = URANGE(0, pReset->arg2, 6);
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      switch(pReset->arg3)
      {
      default:	ef_name = "(* ERROR *)";	break;
      case 0:	ef_name = "Open";		break;
      case 1:	ef_name = "Close";		break;
      case 2:	ef_name = "Close and lock";	break;
      }
      sprintf(pbuf, "%s [%d] the %s%s [%d] door %s (%d)\n\r", ef_name,
          pReset->arg3, dir_name[pReset->arg2],
          (room && room->exit[pReset->arg2] != NULL ? "" : " (NO EXIT!)"),
          pReset->arg2, rname, pReset->arg1);
      }
      break;
    case 'R':
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf(pbuf, "Randomize exits 0 to %d -> %s (%d)\n\r", pReset->arg2,
          rname, pReset->arg1);
      break;
    }
    if ( start == -1 || num >= start )
      send_to_char( buf, ch );
    if ( end != -1 && num >= end )
      break;
  }
  if ( num == 0 )
    send_to_char( "You don't have any resets defined.\n\r", ch );
  return;
}

/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets( AREA_DATA *pArea )
{
  RESET_DATA *pReset, *lastobj = NULL;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    switch(pReset->command)
    {
    default:
      break;
    case 'G': case 'E': case 'O':
      lastobj = pReset;
      break;
    }
  }
  return;
}

/*
 * Create a new reset (for online building)			-Martin
 */
RESET_DATA *make_reset( char letter, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;

	CREATE( pReset, RESET_DATA, 1 );
	pReset->command	= letter;
	pReset->arg1	= arg1;
	pReset->arg2	= arg2;
	pReset->arg3	= arg3;
	top_reset++;	
	return pReset;
}

/*
 * Add a reset to an area				-Martin
 */
RESET_DATA *add_reset( AREA_DATA *tarea, char letter, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset;

    if ( !tarea )
    {
	bug( "add_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, arg1, arg2, arg3 );
    switch( letter )
    {
	case 'M':  tarea->last_mob_reset = pReset;	break;
	case 'E':  case 'G':  case 'P':
	case 'O':  tarea->last_obj_reset = pReset;	break;
    }

    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}

/*
 * Place a reset into an area, insert sorting it		-Martin
 */
RESET_DATA *place_reset( AREA_DATA *tarea, char letter, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset, *tmp, *tmp2;

    if ( !tarea )
    {
	bug( "place_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, arg1, arg2, arg3 );
    if ( letter == 'M' )
	tarea->last_mob_reset = pReset;

    if ( tarea->first_reset )
    {
	switch( letter )
	{
	    default:
		bug( "place_reset: Bad reset type %c", letter );
		return NULL;
	    case 'D':	case 'R':
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		if ( tmp )	/* organize by location */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 > arg1; tmp = tmp->prev );
		if ( tmp )	/* organize by direction */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 == tmp->arg1 && tmp->arg2 > arg2; tmp = tmp->prev );
		if ( tmp )
		    INSERT( pReset, tmp, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'M':	case 'O':
		/* find last reset of same type */
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		tmp2 = tmp ? tmp->next : NULL;
		/* organize by location */
		for ( ; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter && tmp->arg3 <= arg3 )
		    {
			tmp2 = tmp->next;
			/* organize by vnum */
			if ( tmp->arg3 == arg3 )
			  for ( ; tmp; tmp = tmp->prev )
			    if ( tmp->command == letter
			    &&   tmp->arg3 == tmp->arg3
			    &&   tmp->arg1 <= arg1 )
			    {
				tmp2 = tmp->next;
				break;
			    }
			    break;
			}
		/* skip over E or G for that mob */
		if ( tmp2 && letter == 'M' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'E' && tmp2->command != 'G' )
			    break;
		}
		else
		/* skip over P, T or H for that obj */
		if ( tmp2 && letter == 'O' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'P' && tmp2->command != 'T'
			&&   tmp2->command != 'H' )
			    break;
		}
		if ( tmp2 )
		    INSERT( pReset, tmp2, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'G':	case 'E':
		/* find the last mob */
		if ( (tmp=tarea->last_mob_reset) != NULL )
		{
		    /*
		     * See if there are any resets for this mob yet,
		     * put E before G and organize by vnum
		     */
		    if ( tmp->next )
		    {
			tmp = tmp->next;
			if ( tmp && tmp->command == 'E' )
			{
			    if ( letter == 'E' )
				for ( ; tmp && tmp->command == 'E' && tmp->arg1 < arg1; tmp = tmp->next );
			    else
				for ( ; tmp && tmp->command == 'E'; tmp = tmp->next );
			}
			else
			if ( tmp && tmp->command == 'G' && letter == 'G' )
			    for ( ; tmp && tmp->command == 'G' && tmp->arg1 < arg1; tmp = tmp->next );
			if ( tmp )
			    INSERT( pReset, tmp, tarea->first_reset, next, prev );
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    }
		    else
			LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}
		break;
	    case 'P':	case 'T':   case 'H':
		/* find the object in question */
		if ( ((letter == 'P' && arg3 == 0))
		&&    (tmp=tarea->last_obj_reset) != NULL )
		{
		    if ( (tmp=tmp->next) != NULL )
		      INSERT( pReset, tmp, tarea->first_reset, next, prev );
		    else
		      LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}

		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		   if ( (tmp->command == 'O' || tmp->command == 'G'
		   ||    tmp->command == 'E' || tmp->command == 'P')
		   &&    tmp->arg1 == arg3 )
		   {
			/*
			 * See if there are any resets for this object yet,
			 * put P before H before T and organize by vnum
			 */
			if ( tmp->next )
			{
			    tmp = tmp->next;
			    if ( tmp && tmp->command == 'P' )
			    {
				if ( letter == 'P' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'T' )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'H' )
			    {
				if ( letter == 'H' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'H' )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'T' && letter == 'T' )
				for ( ; tmp && tmp->command == 'T' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
			    if ( tmp )
				INSERT( pReset, tmp, tarea->first_reset, next, prev );
			    else
				LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			}
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			return pReset;
		   }
		break;
	}
	/* likely a bad reset if we get here... add it anyways */
    }
    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}


/*
 * Parse a reset command string into a reset_data structure
 */
RESET_DATA *parse_reset( AREA_DATA *tarea, char *argument, CHAR_DATA *ch )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	char letter;
	int extra, val1, val2, val3;
	int value;
	ROOM_INDEX_DATA *room;
	EXIT_DATA	*pexit;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	extra = 0; letter = '*';
	val1 = atoi( arg2 );
	val2 = atoi( arg3 );
	val3 = atoi( arg4 );
	if ( arg1[0] == '\0' )
	{
	   send_to_char( "Reset commands: mob obj give equip door rand trap hide.\n\r", ch );
	   return NULL;
	}

	if ( !strcasecmp( arg1, "hide" ) )
	{
	    if ( arg2[0] != '\0' && !get_obj_index(val1) )
	    {
		send_to_char( "Reset: HIDE: no such object\n\r", ch );
		return NULL;
	    }
	    else
		val1 = 0;
	    extra = 1;
	    val2 = 0;
	    val3 = 0;
	    letter = 'H';
	}
	else
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Reset: not enough arguments.\n\r", ch );
	    return NULL;
	}
	else
	if ( val1 < 1 || val1 > 32767 )
	{
	    send_to_char( "Reset: value out of range.\n\r", ch );
	    return NULL;
	}
	else
	if ( !strcasecmp( arg1, "mob" ) )
	{
	    if ( !get_mob_index(val1) )
	    {
		send_to_char( "Reset: MOB: no such mobile\n\r", ch );
		return NULL;
	    }
	    if ( !get_room_index(val2) )
	    {
		send_to_char( "Reset: MOB: no such room\n\r", ch );
		return NULL;
	    }
	    if ( val3 < 1 )
		val3 = 1;
	    letter = 'M';
	}
	else
	if ( !strcasecmp( arg1, "obj" ) )
	{
	    if ( !get_obj_index(val1) )
	    {
		send_to_char( "Reset: OBJ: no such object\n\r", ch );
		return NULL;
	    }
	    if ( !get_room_index(val2) )
	    {
		send_to_char( "Reset: OBJ: no such room\n\r", ch );
		return NULL;
	    }
	    if ( val3 < 1 )
		val3 = 1;
	    letter = 'O';
	}
	else
	if ( !strcasecmp( arg1, "give" ) )
	{
	    if ( !get_obj_index(val1) )
	    {
		send_to_char( "Reset: GIVE: no such object\n\r", ch );
		return NULL;
	    }
	    if ( val2 < 1 )
		val2 = 1;
	    val3 = val2;
	    val2 = 0;
	    extra = 1;
	    letter = 'G';
	}
	else
	if ( !strcasecmp( arg1, "equip" ) )
	{
	    if ( !get_obj_index(val1) )
	    {
		send_to_char( "Reset: EQUIP: no such object\n\r", ch );
		return NULL;
	    }
	    if ( !is_number(arg3) )
		val2 = get_wearloc(arg3);
	    if ( val2 < 0 || val2 >= MAX_WEAR )
	    {
		send_to_char( "Reset: EQUIP: invalid wear location\n\r", ch );
		return NULL;
	    }
	    if ( val3 < 1 )
	      val3 = 1;
	    extra  = 1;
	    letter = 'E';
	}
	else
	if ( !strcasecmp( arg1, "put" ) )
	{
	    if ( !get_obj_index(val1) )
	    {
		send_to_char( "Reset: PUT: no such object\n\r", ch );
		return NULL;
	    }
	    if ( val2 > 0 && !get_obj_index(val2) )
	    {
		send_to_char( "Reset: PUT: no such container\n\r", ch );
		return NULL;
	    }
	    extra = UMAX(val3, 0);
	    argument = one_argument(argument, arg4);
	    val3 = (is_number(argument) ? atoi(arg4) : 0);
	    if ( val3 < 0 )
		val3 = 0;
	    letter = 'P';
	}
	else
	if ( !strcasecmp( arg1, "door" ) )
	{
	    if ( (room = get_room_index(val1)) == NULL )
	    {
		send_to_char( "Reset: DOOR: no such room\n\r", ch );
		return NULL;
	    }
	    if ( val2 < 0 || val2 > 9 )
	    {
		send_to_char( "Reset: DOOR: invalid exit\n\r", ch );
		return NULL;
	    }
	    if ( (pexit = room->exit[val2]) == NULL
	    ||   !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		send_to_char( "Reset: DOOR: no such door\n\r", ch );
		return NULL;
	    }
	    if ( val3 < 0 || val3 > 2 )
	    {
		send_to_char( "Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\n\r", ch );
		return NULL;
	    }
	    letter = 'D';
	    value = val3;
	    val3  = val2;
	    val2  = value;
	}
	else
	if ( !strcasecmp( arg1, "rand" ) )
	{
	    if ( !get_room_index(val1) )
	    {
		send_to_char( "Reset: RAND: no such room\n\r", ch );
		return NULL;
	    }
	    if ( val2 < 0 || val2 > 9 )
	    {
		send_to_char( "Reset: RAND: invalid max exit\n\r", ch );
		return NULL;
	    }
	    val3 = val2;
	    val2 = 0;
	    letter = 'R';
	}

	if ( letter == '*' )
	    return NULL;
	else
	    return make_reset( letter, val1, val3, val2 );
}
