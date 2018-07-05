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

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"

struct act_prog_data *mob_act_list;

extern int sscanf();
/*
 * Global function prototypes
 */
OBJ_DATA *get_obj_carry_vnum args( ( CHAR_DATA *ch, int vnum) );

#define CAN_SEE_ALWAYS TRUE
/*
 * Local function prototypes
 */
char mprog_cmd_translate_buf[MAX_INPUT_LENGTH];
char *mprog_cmd_translate( char *, CHAR_DATA *, CHAR_DATA *,
			OBJ_DATA *, void *, CHAR_DATA *);
NPC_TOKEN *execute_mob_prog ( NPC_TOKEN *, MOB_PACKET *, char );

char *	mprog_next_command	args( ( char* clist ) );
bool	mprog_seval		args( ( char* lhs, char* opr, char* rhs ) );
bool	mprog_veval		args( ( int lhs, char* opr, int rhs ) );
bool	mprog_do_ifchck		args( ( char* ifchck, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
char *	mprog_process_if	args( ( char* ifchck, char* com_list, 
				       CHAR_DATA* mob, CHAR_DATA* actor,
				       OBJ_DATA* obj, void* vo,
				       CHAR_DATA* rndm ) );
void	mprog_translate		args( ( char ch, char* t, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
void	mprog_process_cmnd	args( ( char* cmnd, CHAR_DATA* mob, 
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm ) );
void	mprog_driver		args( ( MPROG_DATA *mprog, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo ) );
bool mprog_keyword_check        args( ( const char *argu, const char *argl ) );

/***************************************************************************
 * Local function code and brief comments.
 */

/* if you dont have these functions, you damn well should... */

#ifdef DUNNO_STRSTR
char * strstr(s1,s2) const char *s1; const char *s2;
{
  char *cp;
  int i,j=strlen(s1)-strlen(s2),k=strlen(s2);
  if(j<0)
    return NULL;
  for(i=0; i<=j && strncmp(s1++,s2, k)!=0; i++);
  return (i>j) ? NULL : (s1-1);
}
#endif



void mob_act_add( CHAR_DATA *mob )
{
    struct act_prog_data *runner;

    for ( runner = mob_act_list; runner; runner = runner->next )
        if ( runner->vo == mob )
           return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = mob;
    runner->next = mob_act_list;
    mob_act_list = runner;
}
/***************************************************************************
 * Global function code and brief comments.
 */

bool mprog_keyword_check( const char *argu, const char *argl )
{
    char word[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int i;
    char *arg, *arglist;
    char *start, *end;

    strcpy( arg1, strlower( argu ) );
    arg = arg1;
    strcpy( arg2, strlower( argl ) );
    arglist = arg2;

    for ( i = 0; i < strlen( arglist ); i++ )
        arglist[i] = LOWER( arglist[i] );
    for ( i = 0; i < strlen( arg ); i++ )
        arg[i] = LOWER( arg[i] );
    if ( ( arglist[0] == 'p' ) && ( arglist[1] == ' ' ) )
    {
        arglist += 2;
        while ( ( start = strstr( arg, arglist ) ) )
            if ( (start == arg || *(start-1) == ' ' )
            && ( *(end = start + strlen( arglist ) ) == ' '
            ||   *end == '\n'
            ||   *end == '\r'
            ||   *end == '\0' ) )
                return TRUE;
            else
                arg = start+1;
    }
    else
    {
        arglist = one_argument( arglist, word );
        for ( ; word[0] != '\0'; arglist = one_argument( arglist, word ) )
            while ( ( start = strstr( arg, word ) ) )
                if ( ( start == arg || *(start-1) == ' ' )
                && ( *(end = start + strlen( word ) ) == ' '
                ||   *end == '\n'
                ||   *end == '\r'
                ||   *end == '\0' ) )
                    return TRUE;
                else
                    arg = start +1;
    }
/*    bug( "don't match" ); */
    return FALSE;
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( char *lhs, char *opr, char *rhs )
{

  if ( !strcasecmp( opr, "==" ) )
    {
     if( ( lhs == NULL || rhs == NULL ) )
       {
       if( lhs != rhs )
         return( FALSE );
       else
         return( TRUE );
       }
    return ( bool )( !strcasecmp( lhs, rhs ) );
    }
  if ( !strcasecmp( opr, "!=" ) )
    {
     if( ( lhs == NULL || rhs == NULL ) )
       {
       if( lhs == rhs )
         return( FALSE );
       else
         return( TRUE );
       }
    
    return ( bool )( strcasecmp( lhs, rhs ) );
    }
  if ( !strcasecmp( opr, "/" ) )
    {
     if( ( lhs == NULL || rhs == NULL ) )
       {
       if( lhs != rhs )
         return( FALSE );
       else
         return( TRUE );
       }
    return ( bool )( !str_infix( rhs, lhs ) );
    }
  if ( !strcasecmp( opr, "!/" ) )
    {
     if( ( lhs == NULL || rhs == NULL ) )
       {
       if( lhs == rhs )
         return( FALSE );
       else
         return( TRUE );
       }
    return ( bool )( str_infix( rhs, lhs ) );
    }

  bug ( "Improper MOBprog operator\n\r", 0 );
  return 0;

}

bool mprog_veval( int lhs, char *opr, int rhs )
{

  if ( !strcasecmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !strcasecmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !strcasecmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !strcasecmp( opr, "<" ) )
    return ( lhs < rhs );
  if ( !strcasecmp( opr, "<=" ) )
    return ( lhs <= rhs );
  if ( !strcasecmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !strcasecmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !strcasecmp( opr, "|" ) )
    return ( lhs | rhs );

  bug ( "Improper MOBprog operator\n\r", 0 );
  return 0;

}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
OBJ_DATA *tirnaryObj=NULL;
short int validExit=0;
bool mprog_do_ifchck( char *ifchck, CHAR_DATA *mob, CHAR_DATA *actor,
		     OBJ_DATA *obj, void *vo, CHAR_DATA *rndm)
{

  unsigned char buf[ MAX_INPUT_LENGTH ];
  unsigned char arg[ MAX_INPUT_LENGTH ];
  unsigned char opr[ MAX_INPUT_LENGTH ];
  unsigned char val[ MAX_INPUT_LENGTH ];
  CHAR_DATA *vict = (CHAR_DATA *) vo;
  OBJ_DATA *v_obj = (OBJ_DATA  *) vo;
  char     *bufpt = buf;
  char     *argpt = arg;
  char     *oprpt = opr;
  char     *valpt = val;
  char     *point = ifchck;
  int       lhsvl;
  int       rhsvl;

  if ( *point == '\0' ) 
    {
      bug ( "Mob: %u null ifchck", mob->pIndexData->vnum ); 
      return -1;
    }   
  /* skip leading spaces */
  while ( *point == ' ' )
    point++;

  /* get whatever comes before the left paren.. ignore spaces */
  while ( *point != '(' ) 
    if ( *point == '\0' ) 
      {
	bug ( "Mob: %u ifchck syntax error", mob->pIndexData->vnum ); 
	return -1;
      }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*bufpt++ = *point++; 

  *bufpt = '\0';
  point++;

  /* get whatever is in between the parens.. ignore spaces */
  while ( *point != ')' ) 
    if ( *point == '\0' ) 
      {
	    bug ( "Mob: %u ifchck syntax error", mob->pIndexData->vnum ); 
	    return -1;
      }   
    else
      if ( *point == ' ' )
	      point++;
      else 
	      *argpt++ = *point++; 

  *argpt = '\0';
  point++;

  /* check to see if there is an operator */
  while ( *point == ' ' )
    point++;
  if ( *point == '\0' ) 
    {
      *opr = '\0';
      *val = '\0';
    }   
  else /* there should be an operator and value, so get them */
    {
      while ( ( *point != ' ' ) && ( !isalnum((int) *point ) ) ) 
	if ( *point == '\0' ) 
	  {
	    bug ( "Mob: %u ifchck operator without value",
		 mob->pIndexData->vnum ); 
	    return -1;
	  }   
	else
	  *oprpt++ = *point++; 

      *oprpt = '\0';
 
      /* finished with operator, skip spaces and then get the value */
      while ( *point == ' ' )
	point++;
      for( ; ; )
	{
	  if ( ( *point != ' ' ) && ( *point == '\0' ) )
	    break;
	  else
	    *valpt++ = *point++; 
	}

      *valpt = '\0';
    }
  bufpt = buf;
  argpt = arg;
  oprpt = opr;
  valpt = val;

  /* Ok... now buf first_content the ifchck, arg first_content the inside of the
   *  parentheses, opr first_content an operator if one is present, and val
   *  has the value if an operator was present.
   *  So.. basically use if statements and run over all known ifchecks
   *  Once inside, use the argument and expand the lhs. Then if need be
   *  send the lhs,opr,rhs off to be evaluated.
   */

  if ( !strcasecmp( buf, "rand" ) )
    {
      return ( number_percent() <= atol(arg) );
    }

  if ( !strcasecmp( buf, "isnight" ) )
    {
      if (time_info.hour>=5 && time_info.hour <=19) 
	return FALSE;
      else
	return TRUE;
    }

  if ( !strcasecmp( buf, "isday" ) )
    {
      if (time_info.hour>=5 && time_info.hour <=19) 
	return TRUE;
      else
	return FALSE;
    }

  if ( !strcasecmp( buf, "time" ) )
    {
	  lhsvl = time_info.hour;
	  rhsvl = atol(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
    }

  if ( !strcasecmp( buf, "quest" ) )
    {
    int firstBit,len;
    CHAR_DATA *victim;
    char name[MAX_INPUT_LENGTH];

    if(sscanf(arg,"%d,%d,%s",&firstBit,&len,name)!=3)
      {
      bug("Mob:%u bad parameters to 'quest' ifcheck",mob->pIndexData->vnum);
      bug(arg,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug(name,0);
      return FALSE;
      }
    if(name[0]=='$')
      {
      switch(name[1])
        {
        case 'i':
          victim = mob;
          break;
        case 'n':
          if ( actor )
            victim = actor;
          else
            return -1;
          break;
        case 't':
          if ( vict )
            victim = vict;
          else
            return -1;
          break;
        case 'r':
          if ( rndm )
            victim = rndm;
          else
            return -1;
          break;
        default:
          bug ( "Mob: %u bad argument to 'quest'", mob->pIndexData->vnum ); 
          return -1;
        }
      }
    else
      victim=NULL;
    if((victim==NULL)&&(victim=get_char_room_even_hidden(mob,name))==NULL)
      return FALSE;
    if( victim==NULL )
      return( FALSE );
    if(IS_NPC(victim))
      lhsvl=get_quest_bits(victim->npcdata->mob_quest,firstBit, len);
    else
      if( victim->pcdata == NULL || mob==NULL || mob->pIndexData==NULL )
        return( FALSE );
     else
      lhsvl=get_quest_bits(
           victim->pcdata->quest[mob->pIndexData->area->low_r_vnum/100],
           firstBit, len );
    rhsvl = atol(val);
    return mprog_veval( lhsvl, opr, rhsvl );
    }
  if ( !strcasecmp( buf, "questr" ) )
    {
    int firstBit,len, vnum;
    CHAR_DATA *victim;
    char name[MAX_INPUT_LENGTH];

    if(sscanf(arg,"%d,%d,%d,%s",&vnum,&firstBit,&len,name)!=3)
      {
      bug("Mob:%u bad parameters to 'quest' ifcheck",mob->pIndexData->vnum);
      bug(arg,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug(name,0);
      return FALSE;
      }
    if(name[0]=='$')
      {
      switch(name[1])
        {
        case 'i':
          victim = mob;
          break;
        case 'n':
          if ( actor )
            victim = actor;
          else
            return -1;
          break;
        case 't':
          if ( vict )
            victim = vict;
          else
            return -1;
          break;
        case 'r':
          if ( rndm )
            victim = rndm;
          else
            return -1;
          break;
        default:
          bug ( "Mob: %u bad argument to 'quest'", mob->pIndexData->vnum ); 
          return -1;
        }
      }
    else
      victim=NULL;
    if( vnum<0 || vnum>=MAX_VNUM || room_index[vnum]==NULL )
      return(FALSE);
    if((victim==NULL)&&(victim=get_char_room_even_hidden(mob,name))==NULL)
      return FALSE;
    if( victim==NULL )
      return( FALSE );
    if(IS_NPC(victim))
      lhsvl=get_quest_bits(victim->npcdata->mob_quest,firstBit, len);
    else
      if( victim->pcdata == NULL || mob==NULL || mob->pIndexData==NULL )
        return( FALSE );
     else
      lhsvl=get_quest_bits(
           victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],
           firstBit, len );
    rhsvl = atol(val);
    return mprog_veval( lhsvl, opr, rhsvl );
    }

  if ( !strcasecmp( buf, "objquest" ) )
    {
    int firstBit,len;
    OBJ_DATA *item;
    char name[MAX_INPUT_LENGTH];

    if(sscanf(arg,"%d,%d,%s",&firstBit,&len,name)!=3)
      {
      bug("Mob:%u bad parameters to 'objquest' ifcheck",mob->pIndexData->vnum);
      bug(arg,0);
      bug("%d",firstBit);
      bug("%d",len);
      bug(name,0);
      return FALSE;
      }
    if(name[0]=='$')
      {
      switch(name[1])
        {
        case 'o':
          if(obj)
            item = obj;
          else
            return -1;
          break;
        case 'p':
          if ( v_obj )
            item = v_obj;
          else
            return -1;
          break;
        case 'c':
          if ( tirnaryObj )
            item = tirnaryObj;
          else
            return -1;
          break;
        default:
          bug( "Mob: %u bad argument to 'objquest'", mob->pIndexData->vnum );
          return -1;
        }
      }
    else
      item=NULL;
    if((item==NULL)&&(item=get_obj_carry(mob,name))==NULL)
      return FALSE;
    lhsvl = get_quest_bits( item->obj_quest, firstBit, len);
	  rhsvl = atol(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
    }

  if ( !strcasecmp( buf, "pcsinarea" ) )
    {
    lhsvl = atol( arg);
    if( lhsvl > 0)
      {
      if( get_room_index( lhsvl ) == NULL )
        lhsvl = mob->pIndexData->area->nplayer;
      else
        lhsvl = room_index[ lhsvl ]->area->nplayer;
      }
    else
      lhsvl = mob->pIndexData->area->nplayer;
    rhsvl = atol(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
    }

  if ( !strcasecmp( buf, "pcsinroom" ) )
    {
     ROOM_INDEX_DATA *room;
     lhsvl=0;
     if (get_room_index(atol(arg)) == NULL ) 
       room=mob->in_room;
     else
        room=get_room_index(atol(arg));
          
     for ( mob = room->first_person; mob != NULL; mob = mob->next_in_room )
      if (!IS_NPC(mob))
        lhsvl++;

      rhsvl = atol(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
    }

  if ( !strcasecmp( buf, "armystatus" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( 0);
	case 'n': if ( actor && !IS_NPC( actor ))
	             return ( actor->pcdata->army_status );
	          else return 0;
	case 't': if ( vict && !IS_NPC(vict))
	             return ( vict->pcdata->army_status );
	          else return 0;
	case 'r': if ( rndm && !IS_NPC( rndm) )
	             return ( rndm->pcdata->army_status );
	          else return 0;
	default:
	  bug ( "Mob: %u bad argument to 'armystatus'", mob->pIndexData->vnum ); 
	  return 0;
        }
      }

  if ( !strcasecmp( buf, "hasobj" ) )
    {
    return ((tirnaryObj = get_obj_carry(mob, arg))!=NULL);
    }

  if ( !strcasecmp( buf, "hasobjnum" ) )
    {
    return ((tirnaryObj = get_obj_carry_vnum(mob, atol(arg)))!=NULL);
    }

  if ( !strcasecmp( buf, "actorhasobjnum" ) )
    {
    if(actor==NULL)
      {
      bug("Mob: %u no actor defined for 'actorhasobjnum'",
          mob->pIndexData->vnum);
      return FALSE;
      }
    return ((tirnaryObj = get_obj_carry_vnum(actor, atol(arg)))!=NULL);
    }

  if ( !strcasecmp( buf, "validexit" ) )
    {
    return FALSE;
/*
    int i,ex,n;
    EXIT_DATA *pexit;


    if(mob->in_room==NULL)
      return FALSE;
    ex=0;
    for(i=0;i<6;i++)
      if( ( pexit = mob->in_room->exit[i] ) != NULL
         && pexit->to_room != NULL
         && ( pexit->to_room->area == mob->in_room->area ) )
        ex++;
    ex=number_range(1,ex);
    n=0;
    for(i=0;i<5;i++)
      if( ( pexit = mob->in_room->exit[i] ) != NULL
         && pexit->to_room != NULL
         && ( pexit->to_room->area == mob->in_room->area ) )
        {
        if((++n)==ex)
          {
          validExit=i;
          return TRUE;
          }
        }
*/
    }

  if ( !strcasecmp( buf, "ispc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 0;
	case 'n': if ( actor )
 	             return ( !IS_NPC( actor ) );
	          else return -1;
	case 't': if ( vict )
                     return ( !IS_NPC( vict ) );
	          else return -1;
	case 'r': if ( rndm )
                     return ( !IS_NPC( rndm ) );
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'ispc'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isnpc" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return 1;
	case 'n': if ( actor )
	             return IS_NPC( actor );
	          else return -1;
	case 't': if ( vict )
                     return IS_NPC( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_NPC( rndm );
	          else return -1;
	default:
	  bug ("Mob: %u bad argument to 'isnpc'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isgood" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_GOOD( mob );
	case 'n': if ( actor )
	             return IS_GOOD( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_GOOD( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_GOOD( rndm );
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isgood'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isevil" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_EVIL( mob );
	case 'n': if ( actor )
	             return IS_EVIL( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_EVIL( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_EVIL( rndm );
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isevil'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isneutral" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return IS_NEUTRAL( mob );
	case 'n': if ( actor )
	             return IS_NEUTRAL( actor );
	          else return -1;
	case 't': if ( vict )
	             return IS_NEUTRAL( vict );
	          else return -1;
	case 'r': if ( rndm )
	             return IS_NEUTRAL( rndm );
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isneutral'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "iskiller" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return (!IS_NPC(mob) && IS_SET( mob->act, PLR_KILLER ));
	case 'n': if ( actor )
	             return (!IS_NPC(actor) && IS_SET( actor->act, PLR_KILLER ));
	          else return -1;
	case 't': if ( vict )
	             return (!IS_NPC(vict) && IS_SET( vict->act, PLR_KILLER ));
	          else return -1;
	case 'r': if ( rndm )
	             return (!IS_NPC(rndm) && IS_SET( rndm->act, PLR_KILLER ));
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'iskiller'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isthief" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return (!IS_NPC(mob) && IS_SET( mob->act, PLR_THIEF ));
	case 'n': if ( actor )
	             return (!IS_NPC(actor) && IS_SET( actor->act, PLR_THIEF ));
	          else return -1;
	case 't': if ( vict )
	             return (!IS_NPC(vict) && IS_SET( vict->act, PLR_THIEF ));
	          else return -1;
	case 'r': if ( rndm )
	             return (!IS_NPC(rndm) && IS_SET( rndm->act, PLR_THIEF ));
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isthief'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isfight" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->fighting ) ? 1 : 0;
	case 'n': if ( actor )
	             return ( actor->fighting ) ? 1 : 0;
	          else return -1;
	case 't': if ( vict )
	             return ( vict->fighting ) ? 1 : 0;
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->fighting ) ? 1 : 0;
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isfight'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isimmort" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( get_trust( mob ) > LEVEL_IMMORTAL );
	case 'n': if ( actor )
	             return ( get_trust( actor ) > LEVEL_IMMORTAL );
  	          else return -1;
	case 't': if ( vict )
	             return ( get_trust( vict ) > LEVEL_IMMORTAL );
                  else return -1;
	case 'r': if ( rndm )
	             return ( get_trust( rndm ) > LEVEL_IMMORTAL );
                  else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isimmort'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "ischarmed" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': 
                  if( IS_AFFECTED( mob, AFF_CHARM ) )
                     return(  1 );
                  else
                     return(  0 );
	case 'n': if ( actor )
	          {
                  if( IS_AFFECTED( actor, AFF_CHARM ) )
                     return(  1 );
                  else
                     return(  0 );
                  }
	          else return -1;
	case 't': if ( vict )
                  {
                  if( IS_AFFECTED( vict, AFF_CHARM ) )
                     return(  1 );
                  else
                     return(  0 );
                  }
	          else return -1;
	case 'r': if ( rndm )
		  {
                  if( IS_AFFECTED( rndm, AFF_CHARM ) )
                     return(  1 );
                  else
                     return(  0 );
		  }
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'ischarmed'",
	       mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isfollow" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return ( mob->master != NULL
			  && mob->master->in_room == mob->in_room );
	case 'n': if ( actor )
	             return ( actor->master != NULL
			     && actor->master->in_room == actor->in_room );
	          else return -1;
	case 't': if ( vict )
	             return ( vict->master != NULL
			     && vict->master->in_room == vict->in_room );
	          else return -1;
	case 'r': if ( rndm )
	             return ( rndm->master != NULL
			     && rndm->master->in_room == rndm->in_room );
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isfollow'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "isaffected" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{		 
	case 'i': return (is_affected_external(mob,atol(val)));
	case 'n': if ( actor )return (is_affected_external(actor,atol(val)));
	          else return -1;
	case 't': if ( vict )return (is_affected_external(vict,atol(val)));
	          else return -1;
	case 'r': if ( rndm )return (is_affected_external(rndm,atol(val)));
	          else return -1;
	default:
	  bug ( "Mob: %u bad argument to 'isaffected'",
	       mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "hitprcnt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i':
                    if( mob->max_hit == 0 )
                      return( FALSE );
                   lhsvl = mob->hit / mob->max_hit;
	          rhsvl = atol( val );
         	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
                    if( actor->max_hit == 0 )
                      return( FALSE );
		    lhsvl = actor->hit / actor->max_hit;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
                    if( vict->max_hit == 0 )
                      return( FALSE );
		    lhsvl = vict->hit / vict->max_hit;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
                    if( rndm->max_hit == 0 )
                      return( FALSE );
		    lhsvl = rndm->hit / rndm->max_hit;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'hitprcnt'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "inroom" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->in_room->vnum;
	          rhsvl = atol(val);
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->in_room->vnum;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->in_room->vnum;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->in_room->vnum;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'inroom'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "sex" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->sex;
	          rhsvl = atol( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->sex;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->sex;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->sex;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'sex'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "position" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->position;
	          rhsvl = atol( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->position;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->position;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->position;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'position'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "level" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = get_trust( mob );
	          rhsvl = atol( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = get_trust( actor );
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = get_trust( vict );
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = get_trust( rndm );
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'level'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "class" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->class;
	          rhsvl = atol( val );
                  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->class;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->class;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->class;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'class'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "race" ) )
    {
    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	    {
	    case 'i': lhsvl = mob->race;
	              rhsvl = atol( val );
                return mprog_veval( lhsvl, opr, rhsvl );
	    case 'n': if ( actor )
	                {
		              lhsvl = actor->race;
		              rhsvl = atol( val );
		              return mprog_veval( lhsvl, opr, rhsvl );
		              }
	              else 
		              return -1;
	    case 't': if ( vict )
	                {
		              lhsvl = vict->race;
		              rhsvl = atol( val );
		              return mprog_veval( lhsvl, opr, rhsvl );
		              }
	              else
		              return -1;
	    case 'r': if ( rndm )
	                {
		              lhsvl = rndm->race;
		              rhsvl = atol( val );
		              return mprog_veval( lhsvl, opr, rhsvl );
		              }
	              else
		              return -1;
	    default: bug ( "Mob: %u bad argument to 'race'", mob->pIndexData->vnum);
               return -1;
	    }
    }

  if ( !strcasecmp( buf, "goldamt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->gold;
                  rhsvl = atol( val );
                  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = actor->gold;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = vict->gold;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = rndm->gold;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'goldamt'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "objtype" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->item_type;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	         else
		   return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->item_type;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'c': if ( tirnaryObj )
	          {
		    lhsvl = tirnaryObj->item_type;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'objtype'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "objval0" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[0];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[0];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 'c': if ( tirnaryObj )
	          {
		    lhsvl = tirnaryObj->value[0];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'objval0'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "objval1" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[1];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[1];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'c': if ( tirnaryObj )
	          {
		    lhsvl = tirnaryObj->value[1];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'objval1'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "objval2" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[2];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->value[2];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'c': if ( tirnaryObj )
	          {
		    lhsvl = tirnaryObj->value[2];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'objval2'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "objval3" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	          {
		    lhsvl = obj->value[3];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj ) 
	          {
		    lhsvl = v_obj->value[3];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'c': if ( tirnaryObj ) 
	          {
		    lhsvl = tirnaryObj->value[3];
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  bug ( "Mob: %u bad argument to 'objval3'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "number" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->gold;
	          rhsvl = atol( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = actor->pIndexData->vnum;
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
	          else
		    return -1;
	case 't': if ( vict )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = vict->pIndexData->vnum;
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
                  else
		    return -1;
	case 'r': if ( rndm )
	          {
		    if IS_NPC( actor )
		    {
		      lhsvl = rndm->pIndexData->vnum;
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
		    }
		  }
	         else return -1;
	case 'o': if ( obj )
	          {
		    lhsvl = obj->pIndexData->vnum;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'p': if ( v_obj )
	          {
		    lhsvl = v_obj->pIndexData->vnum;
		    rhsvl = atol( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'c': if ( tirnaryObj )
	            {
		          lhsvl = tirnaryObj->pIndexData->vnum;
		          rhsvl = atol( val );
		          return mprog_veval( lhsvl, opr, rhsvl );
		          }
	          else
		          return -1;
	default:
	  bug ( "Mob: %u bad argument to 'number'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }

  if ( !strcasecmp( buf, "name" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return mprog_seval( mob->name, opr, val );
	case 'n': if ( actor )
	            return mprog_seval( actor->name, opr, val );
	          else
		    return -1;
	case 't': if ( vict )
	            return mprog_seval( vict->name, opr, val );
	          else
		    return -1;
	case 'r': if ( rndm )
	            return mprog_seval( rndm->name, opr, val );
	          else
		    return -1;
	case 'o': if ( obj )
	            return mprog_seval( obj->name, opr, val );
	          else
		    return -1;
	case 'p': if ( v_obj )
	            return mprog_seval( v_obj->name, opr, val );
	          else
		    return -1;
  case 'c': if ( tirnaryObj )
              return mprog_seval(tirnaryObj->name, opr, val );
            else
              return -1;
	default:
	  bug ( "Mob: %u bad argument to 'name'", mob->pIndexData->vnum ); 
	  return -1;
	}
    }
  if(!strcasecmp(buf,"shotfrom"))
    {
    switch(arg[1])
      {
      case 'i': return mprog_seval(dir_name[mob->shot_from],opr,val);
      default :
	bug ( "Mob: %u bad argument to 'shotfrom'", mob->pIndexData->vnum ); 
	return -1;
      }
    }

  if(!strcasecmp(buf,"whichgod"))
    {
    switch(arg[1])
      {
	    case 'n': 
        if ( actor )
          {
		      lhsvl = which_god(actor);
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
          }
	      else
		      return -1;
	    case 't': 
        if ( vict )
          {
		      lhsvl = which_god(vict);
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
          }
	      else
		      return -1;
	    case 'r': 
        if ( rndm )
          {
		      lhsvl = which_god(rndm);
		      rhsvl = atol( val );
		      return mprog_veval( lhsvl, opr, rhsvl );
          }
	      else
		      return -1;
      default :
	      bug ( "Mob: %u bad argument to 'whichgod'", mob->pIndexData->vnum ); 
	      return -1;
      }
    }

  /* Ok... all the ifchcks are done, so if we didnt find ours then something
   * odd happened.  So report the bug and abort the MOBprogram (return error)
   */
  bug ( "Mob: %u unknown ifchck", mob->pIndexData->vnum ); 
  return -1;

}



/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
char arg_save[MAX_INPUT_LENGTH];
void mprog_translate( char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor,
                    OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
 static char *he_she        [] = { "it",  "he",  "she" };
 static char *him_her       [] = { "it",  "him", "her" };
 static char *his_her       [] = { "its", "his", "her" };
 CHAR_DATA   *vict             = (CHAR_DATA *) vo;
 OBJ_DATA    *v_obj            = (OBJ_DATA  *) vo;
 char wbuf[MAX_INPUT_LENGTH];

 *t = '\0';
 switch ( ch ) {
     case 'd':
       sprintf(t,"%d",validExit);
       break;
     case 'D':
       strcpy(t,dir_name[validExit]);
       break;
     case 'i':
         strcpy( wbuf, mob->name );
         one_argument( wbuf, t );
      break;

     case 'I':
         strcpy( t, mob->short_descr );
      break;

     case 'n':
        if( actor == NULL )
	 {
          if( rndm != NULL )
            actor = rndm;
          else
            break;
	 }
         if( actor->name == NULL )
           return;
         strcpy( wbuf, actor->name );
	 if ( CAN_SEE_ALWAYS || can_see( mob,actor ) )
	     one_argument( wbuf, t );
         if ( !IS_NPC( actor ) )
	   *t = UPPER( *t );
      break;

     case 'N':
        if( actor == NULL )
	 {
          if( rndm != NULL )
            actor = rndm;
          else
            break;
	 }
         if ( actor ) 
            {
            if ( CAN_SEE_ALWAYS || can_see( mob, actor ) )
 	    {
	       if ( IS_NPC( actor ) )
		 strcpy( t, actor->short_descr );
	       else
	       {
		   strcpy( t, actor->name );
		   strcat( t, " " );
		   strcat( t, actor->pcdata->title );
	       }
 	    }
	    else
	      strcpy( t, "someone" );
            }
	 break;

     case 't':
         strcpy( wbuf, vict->name );
         if ( vict )
	   if ( CAN_SEE_ALWAYS || can_see( mob, vict ) )
	     one_argument( wbuf, t );
         if ( !IS_NPC( vict ) )
	   *t = UPPER( *t );
	 break;

     case 'T':
         if ( vict ) 
	  {
            if ( CAN_SEE_ALWAYS || can_see( mob, vict ) )
	    {
	       if ( IS_NPC( vict ) )
		 strcpy( t, vict->short_descr );
	       else
	       {
		 strcpy( t, vict->name );
		 strcat( t, " " );
		 strcat( t, vict->pcdata->title );
	       }
	    }
	    else
	      strcpy( t, "someone" );
	  }
	 break;
     
     case 'r':
       if ( rndm )
         {
            strcpy( wbuf, rndm->name );
	       if ( CAN_SEE_ALWAYS || can_see( mob, rndm ) )
	         one_argument( wbuf, t );
         if ( !IS_NPC( rndm ) )
	         *t = UPPER( *t );
         }
       break;

     case 'R':
       if ( rndm ) 
         {
         if ( CAN_SEE_ALWAYS || can_see( mob, rndm ) )
           {
	         if ( IS_NPC( rndm ) )
		         strcpy(t,rndm->short_descr);
	         else
	           {
		         strcpy( t, rndm->name );
		         strcat( t, " " );
		         strcat( t, rndm->pcdata->title );
	           }
           }
	       else
	         strcpy( t, "someone" );
         }
	     break;

     case 'e':
         if ( actor )
	   (CAN_SEE_ALWAYS || can_see( mob, actor )) ? strcpy( t, he_she[ actor->sex ] )
	                         : strcpy( t, "someone" );
	 break;
  
     case 'm':
         if ( actor )
	   (CAN_SEE_ALWAYS || can_see( mob, actor )) ? strcpy( t, him_her[ actor->sex ] )
                                 : strcpy( t, "someone" );
	 break;
  
     case 's':
         if ( actor )
	   (CAN_SEE_ALWAYS || can_see( mob, actor )) ? strcpy( t, his_her[ actor->sex ] )
	                         : strcpy( t, "someone's" );
	 break;
     
     case 'E':
         if ( vict )
	   (CAN_SEE_ALWAYS || can_see( mob, vict )) ? strcpy( t, he_she[ vict->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'M':
         if ( vict )
	   (CAN_SEE_ALWAYS || can_see( mob, vict )) ? strcpy( t, him_her[ vict->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'S':
         if ( vict )
	   (CAN_SEE_ALWAYS || can_see( mob, vict )) ? strcpy( t, his_her[ vict->sex ] )
                                : strcpy( t, "someone's" ); 
	 break;

     case 'j':
	 strcpy( t, he_she[ mob->sex ] );
	 break;
  
     case 'k':
	 strcpy( t, him_her[ mob->sex ] );
	 break;
  
     case 'l':
	 strcpy( t, his_her[ mob->sex ] );
	 break;

     case 'J':
         if ( rndm )
	   (CAN_SEE_ALWAYS || can_see( mob, rndm )) ? strcpy( t, he_she[ rndm->sex ] )
	                        : strcpy( t, "someone" );
	 break;
  
     case 'K':
         if ( rndm )
	   (CAN_SEE_ALWAYS || can_see( mob, rndm )) ? strcpy( t, him_her[ rndm->sex ] )
                                : strcpy( t, "someone" );
	 break;
  
     case 'L':
         if ( rndm )
	   (CAN_SEE_ALWAYS || can_see( mob, rndm )) ? strcpy( t, his_her[ rndm->sex ] )
	                        : strcpy( t, "someone's" );
	 break;

     case 'o':
         strcpy( wbuf, obj->name );
         if ( obj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, obj )) ? one_argument( wbuf, t )
                                   : strcpy( t, "something" );
	 break;

     case 'O':
         if ( obj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, obj )) ? strcpy( t, obj->short_descr )
                                   : strcpy( t, "something" );
	 break;

     case 'p':
         strcpy( wbuf, v_obj->name );
         if ( v_obj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, v_obj )) ? one_argument(wbuf,t)
                                     : strcpy( t, "something" );
	 break;

     case 'P':
         if ( v_obj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, v_obj )) ? strcpy( t, v_obj->short_descr )
                                     : strcpy( t, "something" );
      break;

     case 'c':
         if( tirnaryObj == NULL )
           break;
         strcpy( wbuf, tirnaryObj->name );
         if ( tirnaryObj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, tirnaryObj )) ?
                            one_argument( wbuf, t )
                                     : strcpy( t, "something" );
      break;

     case 'C':
         if ( tirnaryObj )
	   (CAN_SEE_ALWAYS || can_see_obj( mob, tirnaryObj )) ? strcpy( t, tirnaryObj->short_descr )
                                     : strcpy( t, "something" );
      break;

     case 'a':
         if ( obj ) 
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case 'A':
         if ( v_obj ) 
          switch ( *( v_obj->name ) )
	  {
            case 'a': case 'e': case 'i':
	    case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case '$':
         strcpy( t, "$" );
	 break;

     case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':
              case '9':
       {
       char tmp[MAX_INPUT_LENGTH];
       char *c,*d;
       int n;

       c=arg_save;
       strcpy(tmp,arg_save);
       n=(int)ch-(int)'0';
       strcpy(t,"");
       if(n>0 && n<10)
         for(c=tmp;(*c!=0)&&(n!=0);)
           {
           d=one_argument(c,t);
           if( c==d )
             {
             *t = '\0';
             break;
             }
           n--;
           if( n==0 )
             break;
           c=d;
           }
       if( *c == '\0' )
         *t = '\0';
       break;
       }

     default:
         bug( "Mob: %u bad $var", mob->pIndexData->vnum );
	 break;
       }

 return;

}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
char *mprog_cmd_translate( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
			OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
  char tmp[ MAX_INPUT_LENGTH ];
  char *str;
  char *i;
  char *point;
  point   = mprog_cmd_translate_buf;
  str     = cmnd;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    str++;
    mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
    i = tmp;
    ++str;
    while ( ( *point = *i ) != '\0' )
      ++point, ++i;
  }
  *point = '\0';

  return( mprog_cmd_translate_buf );
}

/* The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver ( MPROG_DATA *mprog, CHAR_DATA *mob, CHAR_DATA *actor,
		   OBJ_DATA *obj, void *vo)
{

 CHAR_DATA *rndm  = NULL;
 CHAR_DATA *vch   = NULL;
 int        count = 0;
 NPC_TOKEN *tok;
  MOB_PACKET *new_packet, *t_packet, *p_packet;
 if ( IS_AFFECTED(mob, AFF2_POSSESS) || IS_AFFECTED( mob, AFF_CHARM ) 
      || !IS_NPC( mob ) || mob->npcdata->mob_prog_started )
   return;
/*
 if (actor != NULL && actor == last_dead)
    actor = NULL;
  if(mob!=NULL && mob==last_dead) 
    mob=NULL;
  if(actor!=NULL && actor==last_dead)
    actor=NULL;
*/
 if( mob->in_room == NULL )
      {
      char buf[200];
      sprintf(buf,"Interp: Null room for %s. Removing.",mob->name);
      bug(buf,0);
      SET_BIT( mob->act, ACT_WILL_DIE);
      return;
      }

  open_timer( TIMER_MOB_PROG );

 /* get a random visable mortal player who is in the room with the mob */
 for ( vch = mob->in_room->first_person; vch; vch = vch->next_in_room )
   if ( !IS_NPC( vch )
    /*   &&  vch->level < MAX_LEVEL-3 Let's allow it to trigger on immortals
	Martin 19/1/99 (about to go study for final exam, honest) */
       &&  (CAN_SEE_ALWAYS || can_see( mob, vch ) ))
     {
       if ( number_range( 0, count ) == 0 )
	 rndm = vch;
       count++;
     }

    /* Let's see if we should execute program at all - Chaos 8/19/96 */
  if( rndm == NULL )
    {
    for( tok=mprog->token_list; tok!=NULL; tok=tok->next )
      if( tok->string!=NULL && strstr(tok->string, "$r")!=NULL )
        {
        close_timer( TIMER_MOB_PROG );
        if( mob != NULL )
          mob->npcdata->mob_prog_started = FALSE;
        ALLOW_OUTPUT = TRUE;
        return;
        }
    }
  tok = mprog->token_list;

  mob->npcdata->mob_prog_started = TRUE;

  CREATE(new_packet, MOB_PACKET, 1);
  new_packet->mob = mob;
  new_packet->actor = actor;
  new_packet->rndm = rndm;
  new_packet->obj = obj;
  new_packet->vo = vo;
  new_packet->next = mob_packet_list;

  mob_packet_list = new_packet;
  

  while( tok != NULL )
    {
    tok = execute_mob_prog( tok, new_packet, tok->level);
    if( tok!=NULL && new_packet->mob!=NULL && new_packet->mob==last_dead)
      new_packet->mob=NULL;
    if( tok!=NULL && new_packet->actor!=NULL && new_packet->actor==last_dead)
      new_packet->actor=NULL;
    if( tok!=NULL && new_packet->rndm!=NULL && new_packet->rndm==last_dead)
      new_packet->rndm=NULL;
    if( tok!=NULL && new_packet->vo!=NULL && new_packet->vo==last_dead)
      new_packet->vo=NULL;
    while( tok != NULL && ( tok->type == 5 || tok->type == 6 ) )
      tok = tok->next;
    }

  p_packet = NULL;
  t_packet = NULL;
  if( mob_packet_list == new_packet )
    mob_packet_list = new_packet->next;
  else
  for( t_packet=mob_packet_list; t_packet != NULL && t_packet!=new_packet;
      t_packet = t_packet->next )
    p_packet = t_packet;
  if( p_packet != NULL && t_packet != NULL )
    p_packet->next = t_packet->next;

    /* Let's grab the end-of-function mobile, if there is one */
  mob = new_packet->mob;
  DISPOSE( new_packet);

  close_timer( TIMER_MOB_PROG );

  if( mob != NULL )
    mob->npcdata->mob_prog_started = FALSE;
  ALLOW_OUTPUT = TRUE;

 return;

}

NPC_TOKEN *execute_mob_prog ( NPC_TOKEN *token, MOB_PACKET *packet, char level)
{
  bool done;
  bool if_val;
  char buf[ 120 ];
  char tstr[ MAX_STRING_LENGTH ];
  CHAR_DATA *actor, *rndm, *mob;
  OBJ_DATA *obj;
  void *vo;

  done = FALSE;


  while( !done && token != NULL )
    {
      /* What does level really matter, if we're there?  */
    level = token->level;

      actor = packet->actor;
      rndm = packet->rndm;
      mob = packet->mob;
    obj = packet->obj;
    vo = packet->vo;

    /* null mobs crashing the game in death_programs.  -Presto 3/28/99 
    if(mob == NULL)
    {
      bug("Null mob in execute_mob_prog.",0);
      return NULL;
    } */

    if( token->string != NULL )
      strcpy( tstr, token->string );
    else
      strcpy( tstr, "" );

       /* Why not have timemode give details on mob_progs */
    if( actor != NULL && !IS_NPC( actor ) && IS_SET( actor->act, PLR_WIZTIME ) )
      {
      sprintf( buf, "line: %3d lev:%2d  Type:%d  String:%s\n\r", token->line,
         level, token->type, token->string != NULL ? token->string : "-" );
      send_to_char( buf, actor );
      }

    switch( token->type )
      {
      case 1:  /* Social */
          check_social_fast( mob, token->value, 
            mprog_cmd_translate( tstr, mob, actor, obj, vo, rndm ) );
        tail_chain( );
        break;
      case 2:  /* Interp line */

            /*
             * Dispatch the command.
             */
            (*cmd_table[token->value].do_fun) ( mob,
            mprog_cmd_translate( tstr, mob, actor, obj, vo, rndm ) );

            tail_chain( );

        break;

      case 3:  /* If */
      case 4:  /* Or */
          if_val = mprog_do_ifchck( tstr, mob, actor, obj, vo, rndm );
       /* Why not have timemode give details on mob_progs */
    if( actor != NULL && !IS_NPC( actor ) && IS_SET( actor->act, PLR_WIZTIME ) )
      {
      if( if_val )
        send_to_char( "TRUE\n\r", actor );
      else
        send_to_char( "FALSE\n\r" , actor );
      }

        if( if_val )
          {
          token = token->next;
          while( token != NULL && token->type == 4 )
            token = token->next;
          if( token != NULL )
            {
            if( token->type != 5 && token->type != 6 )
              token=execute_mob_prog( token, packet, level + 1);
            if( token != NULL && token->type == 5 )
              {
              token = token->next;
              while( token != NULL && token->level > level )
                token = token->next;
              }
            }
          break;
          }
        if( !if_val && token->next != NULL && token->next->type != 4 )
          {
          token = token->next;
          while( token != NULL && token->level > level )
            token = token->next;
          
          if( token != NULL )
            if( token->type == 5 )
              {
              token = token->next;
              if( token != NULL && token->type != 6 && token->type != 5 )
                {
                token=execute_mob_prog(token, packet, level + 1);
                }
              }
          }

        break;

      case 5:  /* Else */
      case 6:  /* Endif */
        return( token );
        break;

      case 7:  /* Break */
        return( NULL );
        break;

      default:
        break;
      }


    if( token != NULL )
      token = token->next;
    if( token == NULL )
      done = TRUE;
    }

  return( token );
}


/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int type )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int         i;

  for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
    if ( mprg->type & type )
      {
        if(mprg->arglist[0]=='\0')
          /*mprog_driver( mprg->comlist, mob, actor, obj, vo );*/
          mprog_driver( mprg, mob, actor, obj, vo );
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
      strcpy(arg_save,arg);
		  mprog_driver( mprg, mob, actor, obj, vo );
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
              {
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
      strcpy(arg_save,arg);
		    mprog_driver( mprg, mob, actor, obj, vo );
		    break;
		  }
		else
		  dupl = start+1;
              dupl=temp2;
              }
	  }
      }

  return;

}

void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			 void *vo, int type)
{
 MPROG_DATA * mprg;
 for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number_percent( ) < atol( mprg->arglist ) ) )
     {
       mprog_driver( mprg, mob, actor, obj, vo );
       if ( type != GREET_PROG && type != ALL_GREET_PROG && type != GROUP_GREET_PROG )
	 break;
     }

 return;

}
void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                         void *vo, int type)
{
 MPROG_DATA * mprg;
 bool       trigger_time;
 for ( mprg = mob->pIndexData->mobprogs; mprg; mprg = mprg->next )
   {
     trigger_time = ( time_info.hour == atoi( mprg->arglist ) );
     if ( !trigger_time )
     {
       if ( mprg->triggered )
         mprg->triggered = FALSE;
       continue;
     }

     if ( mprg->type == type && !mprg->triggered )
     {
       mprg->triggered = TRUE;
       mprog_driver( mprg, mob, actor, obj, vo );
     }
   }
 return;
}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch,
		       OBJ_DATA *obj, void *vo)
{
    MPROG_ACT_LIST * tmp_act;
    MPROG_DATA *mprg;
    bool found = FALSE;

    if ( IS_NPC( mob )
    &&   IS_SET( mob->pIndexData->progtypes, ACT_PROG ) )
    {
        /* Don't let a mob trigger itself, nor one instance of a mob
          trigger another instance. */
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
          return;
        /* make sure this is a matching trigger */
        for ( mprg = mob->pIndexData->mobprogs; mprg; mprg = mprg->next )
            if ( mprg->type & ACT_PROG
            &&   mprog_keyword_check( buf, mprg->arglist ) )
            {
                found = TRUE;
                break;
            }
        if ( !found )
            return;

        CREATE(tmp_act, MPROG_ACT_LIST, 1);
        if ( mob->mpactnum > 0 )
          tmp_act->next = mob->mpact;
        else
          tmp_act->next = NULL;

        mob->mpact      = tmp_act;
        mob->mpact->buf = STRALLOC( buf );
        mob->mpact->ch  = ch;
        mob->mpact->obj = obj;
        mob->mpact->vo  = vo;
        mob->mpactnum++;
        mob_act_add( mob );
    }
    return;


}

void mprog_social_trigger( char *social, CHAR_DATA *mob, CHAR_DATA *ch )
{
  char        buf[ MAX_INPUT_LENGTH ];
  char        wbuf[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;

  if ( IS_NPC( mob )
      && ( mob->pIndexData->progtypes & SOCIAL_PROG ) )
   for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
     {
       strcpy( wbuf, mprg->arglist );
       one_argument( wbuf, buf );
       if ( ( mprg->type & SOCIAL_PROG )
	   && ( !strcasecmp( social, buf ) ) )
	 {
           GLOBAL_SOCIAL_FLAG = TRUE;
	   mprog_driver( mprg, mob, ch, NULL, NULL );
	   break;
	 }
     }
  return;

}

void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{

  char        buf[ MAX_STRING_LENGTH ];
  MPROG_DATA *mprg;
  OBJ_DATA   *obj;

  if( mob==NULL )
    return;

  if ( IS_NPC( mob )
      && ( mob->pIndexData->progtypes & BRIBE_PROG ) )
    {
      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
      sprintf( buf, obj->short_descr, amount );
      STRFREE (obj->short_descr );
      obj->short_descr = STRALLOC( buf );
      obj->value[0]    = amount;
      obj_to_char( obj, mob );
      mob->gold -= amount;

      for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
	if ( ( mprg->type & BRIBE_PROG )
	    && ( amount >= atol( mprg->arglist ) ) )
	  {
	    mprog_driver( mprg, mob, ch, obj, NULL );
	    break;
	  }
    }
  
  return;

}

void mprog_death_trigger( CHAR_DATA *mob )
{
  CHAR_DATA *fightingwho;

  if( mob==NULL )
    return;

  fightingwho = who_fighting( mob );
  if( fightingwho == NULL )
  {
    fightingwho = mob;
  }

  if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & DEATH_PROG ) )
   {
     mob->position=POS_STANDING;
     mprog_percent_check( mob, fightingwho, NULL, NULL, DEATH_PROG );
     mob->position=POS_DEAD;
   }

 death_cry( mob );
 return;
}

void mprog_kill_trigger( CHAR_DATA *mob )
{

  if( mob==NULL || mob->fighting==NULL)
    return;

 if ( IS_NPC( mob ) 
     && !IS_NPC( mob->fighting->who)
     && ( mob->pIndexData->progtypes & KILL_PROG ) )
   {
   mprog_percent_check( mob, mob->fighting->who, NULL, NULL, KILL_PROG );
   }

 return;
}
void mprog_entry_trigger( CHAR_DATA *mob )
{

  if( mob==NULL )
    return;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & ENTRY_PROG ) )
   mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );

 return;

}

void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{

  if( mob==NULL )
    return;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & FIGHT_PROG ) )
   mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );

 return;

}

void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{

 char        buf[MAX_INPUT_LENGTH];
 char        wbuf[MAX_INPUT_LENGTH];
 MPROG_DATA *mprg;

  if( mob==NULL )
    return;

 if(IS_OBJ_STAT(obj,ITEM_FORGERY))
    {
    return;
    }

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & GIVE_PROG ) )
   for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
     {
       strcpy( wbuf, mprg->arglist );
       one_argument( wbuf, buf );
       if ( ( mprg->type & GIVE_PROG )
	   && ( is_name( mprg->arglist, obj->name ) ) )
	 {
	   mprog_driver( mprg, mob, ch, obj, NULL );
	   break;
	 }
     }

 return;

}

void mprog_greet_trigger( CHAR_DATA *ch )
{

 CHAR_DATA *vmob;

  if( ch==NULL )
    return;


if(!IS_NPC( ch))  /* Greet only PC's    - Chaos 5/19/94 */
 for ( vmob = ch->in_room->first_person; vmob != NULL; vmob = vmob->next_in_room )
 {

   if ( IS_NPC( vmob )
       && ch != vmob
       && can_see( vmob, ch )
       && ( vmob->fighting == NULL )
       && IS_AWAKE( vmob )
       && ( vmob->pIndexData->progtypes & GREET_PROG) )
     {
     mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
     }
   else
     if ( IS_NPC( vmob )
	 && ( vmob->fighting == NULL )
	 && IS_AWAKE( vmob )
	 && ( vmob->pIndexData->progtypes & ALL_GREET_PROG ) )
     {
       mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
     }
   else 
     if ( IS_NPC( vmob )
       && ch != vmob
       && can_see( vmob, ch )
       && ( vmob->fighting == NULL )
       && IS_AWAKE( vmob )
       && ( vmob->pIndexData->progtypes & GROUP_GREET_PROG) )
     {
      if (Greeted==NULL)
       {
        Greeter=vmob;
        Greeted=ch;
/*        mprog_percent_check( vmob, ch, NULL, NULL,GROUP_GREET_PROG );*/
       }
      else if(!is_same_group(ch, Greeted))
       {
           Greeter=vmob;
           Greeted=ch;
/*	   mprog_percent_check( vmob, ch, NULL, NULL,GROUP_GREET_PROG );*/
       }
     }
  }
 return;

}

void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch)
{

 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & HITPRCNT_PROG ) )
   for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
     if ( ( mprg->type & HITPRCNT_PROG )
	 && ( ( 100*mob->hit / mob->max_hit ) < atol( mprg->arglist ) ) )
       {
	 mprog_driver( mprg, mob, ch, NULL, NULL );
	 break;
       }
 
 return;

}

void mprog_random_trigger( CHAR_DATA *mob )
{

  if( mob==NULL )
    return;

  if ( mob->pIndexData->progtypes & RAND_PROG)
    mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);

  return;

}

void mprog_speech_trigger( char *txt, CHAR_DATA *mob )
{

  CHAR_DATA *vmob;

  if( mob==NULL )
    return;
  if( IS_NPC( mob ) )
    return;

  for ( vmob = mob->in_room->first_person; vmob != NULL; vmob = vmob->next_in_room )
    if(IS_NPC(vmob) && vmob!=mob && (vmob->pIndexData->progtypes & SPEECH_PROG))
      mprog_wordlist_check( txt, vmob, mob, NULL, NULL, SPEECH_PROG );
  
  return;

}

void mprog_range_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
  MPROG_DATA *mprg;

  if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & RANGE_PROG ) )
    for ( mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next )
      {
      if ( mprg->type & RANGE_PROG )
	{
	mprog_driver( mprg, mob, ch, NULL, NULL );
	break;
	}
      }

  return;
}

void mprog_time_trigger( CHAR_DATA *mob )
{
    if ( IS_NPC(mob) && (mob->pIndexData->progtypes & TIME_PROG) )
        mprog_time_check(mob,NULL,NULL,NULL,TIME_PROG);
}

