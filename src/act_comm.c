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
#include <stdlib.h>
#include <string.h>
/* #include <strings.h> */
#include "time.h"
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "merc.h"


/*
 * Local functions.
 */
void translate (char *, char *);
bool is_note_to args ((CHAR_DATA * ch, NOTE_DATA * pnote));
void note_attach args ((CHAR_DATA * ch));
void note_remove args ((CHAR_DATA * ch, NOTE_DATA * pnote, bool remove_all));
void speech (CHAR_DATA *, CHAR_DATA *, char *);
void negate_mob_hit (int);


bool is_note_to (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  if (IS_NPC (ch) || 
      (ch->pcdata->note_topic != pnote->topic &&
       (ch->in_room == NULL ||
	(ch->in_room != NULL &&
	!IS_SET (ch->in_room->room_flags, ROOM_NOTE_BOARD)))))
    return FALSE;

  if (ch->name[0] == pnote->sender[0] && !strcasecmp (ch->name, pnote->sender))
    return TRUE;
/*
  Removed by Martin 7/3/99...was causing lots of crashes and it doesn't 
  make a huge amount of sense either
  if (pnote->sender != NULL && ch->pcdata->block_list != NULL &&
      is_name (pnote->sender, ch->pcdata->block_list))
    return FALSE;
*/
  if (is_name (ch->name, pnote->to_list))
    return TRUE;

  if (is_name ("all", pnote->to_list))
    return TRUE;

  if (IS_HERO (ch) && is_name ("immortal", pnote->to_list))
    return TRUE;

  return FALSE;
}



void
note_attach (CHAR_DATA * ch)
{
  NOTE_DATA *pnote;

  if (ch->pnote != NULL)
    return;
  CREATE (pnote, NOTE_DATA, 1);

  pnote->next = NULL;
  pnote->prev = NULL;
  pnote->sender = QUICKLINK (ch->name);
  pnote->date = STRALLOC ("");
  pnote->to_list = STRALLOC ("");
  pnote->subject = STRALLOC ("");
  pnote->text = STRALLOC ("");
  pnote->topic = -1;
  pnote->room_vnum = 0;
  ch->pnote = pnote;
  return;
}



void
note_remove (CHAR_DATA * ch, NOTE_DATA * pnote, bool remove_all)
{
  char to_new[MAX_INPUT_LENGTH];
  char to_one[MAX_INPUT_LENGTH];
  FILE *fp;
  char *to_list;

  /*
   * Build a new to_list.
   * Strip out this recipient.
   */
  to_new[0] = '\0';
  to_list = pnote->to_list;
  while (*to_list != '\0')
    {
      to_list = one_argument (to_list, to_one);
      if (to_one[0] != '\0' && strcasecmp (ch->name, to_one))
	{
	  strcat (to_new, " ");
	  strcat (to_new, to_one);
	}
    }

  /*
   * Just a simple recipient removal?
   */
  if (!remove_all && strcasecmp (ch->name, pnote->sender) && to_new[0] != '\0')
    {
      STRFREE (pnote->to_list);
      pnote->to_list = STRALLOC (to_new + 1);
      return;
    }

  /*
   * Remove note from linked list.
   */
#ifdef UNLINKCHECK
  {
  NOTE_DATA *tnote;
  bool foundn;

  for( foundn=FALSE, tnote=first_note; tnote!=NULL; tnote=tnote->next)
    if( tnote==pnote )
      {
      foundn=TRUE;
      break;
      }
  if( foundn )
    UNLINK (pnote, first_note, last_note, next, prev);
  else
    bug( "UNLINK ERROR:  did not find note %s", pnote->subject );
  }
#else
    UNLINK (pnote, first_note, last_note, next, prev);
#endif

  STRFREE (pnote->text);
  STRFREE (pnote->subject);
  STRFREE (pnote->to_list);
  STRFREE (pnote->date);
  STRFREE (pnote->sender);
  DISPOSE (pnote);

  /*
   * Rewrite entire list.
   */
  fclose (fpReserve);
  fp = fopen (NOTE_FILE_T, "w");
  if (fp == NULL)
    {
      perror (NOTE_FILE_T);
      return;
    }
  else
    {		
      for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	{
	  fprintf (fp, "Sender  %s~\nDate    %s~\nTime  %d\nTo      %s~\nSubject %s~\nTopic   %d\nText\n%s~\nRoom %u\n\n",
		   pnote->sender, pnote->date, pnote->time,
		   pnote->to_list, pnote->subject,
		   pnote->topic, pnote->text,
		   pnote->room_vnum);
	}
      fflush (fp);
    }
  fpReserve = fopen (NULL_FILE, "r");
  return;
}

void
do_refresh (CHAR_DATA * ch, char *argument)
{
  if (ch == NULL || ch->desc == NULL || ch->desc->original != NULL || ch->vt100 == 0)
    return;
  vt100off (ch);
  ch->vt100 = 2;
  if (!IS_SET (ch->act, PLR_PROMPT))
    {
      vt100on (ch);
    }
  else
    vt100prompt (ch);

}

int
amount_note (CHAR_DATA * ch)
{
  NOTE_DATA *pnote;
  int oldTopic;

  int cnt;
  cnt = 0;
  oldTopic = ch->pcdata->note_topic;
  for (pnote = first_note; pnote != NULL; pnote = pnote->next)
    {
      ch->pcdata->note_topic = pnote->topic;
      if (is_note_to (ch, pnote))
	cnt++;
    }
  ch->pcdata->note_topic = oldTopic;
  return (cnt);
}

/* NOTE: (proposed changes to note, Order 4/16/94)
   X note topic <topicnumber>
   X pcdata first_content a var which indicates which topic the char is currently
   X reading, the current topic is not saved with the character so all topics
   X are reset to 0 (should be general) when someone comes into the game.  If 
   X <topicnumber> is blank, note will list the topics available.  Topics will
   X be set up by the implementors at the beginning of the notes file, just a
   X simple list of one topic per line with lines preceded by topic reference
   X number and minimum level number ("1 95 Immail").
   X note list <key>
   X if <key> is a number:
   X   lists all notes that are <key> days old or younger
   X if <key> is a string:
   X   lists all notes that have <key> in to, from, or subject fields (that the
   X   lister is allowed to see that is)
   X note [read] #
   X make read the default parameter, so "note 5" reads note 5
   X note post [<topicname>]
   X posts to <topicname> or to the current topic if <topicname> is left blank
   X notes will delete themselves if not read for 14 days
 */


void
do_note (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], buft[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
/* char *tArg, *pt; */
  PLAYER_GAME *fch;
  NOTE_DATA *pnote;
  int vnum;
  int anum;
  int blen,cnt;
  bool room_board;
  int room_vnum;

  if (IS_NPC (ch))
    return;

  if (!ch->desc)
    {
      bug ("do_note: no descriptor", 0);
      return;
    }

  switch (ch->substate)
    {
    default:
      break;
    case SUB_RESTRICTED:
      send_to_char ("You cannot use this command from while editing something else.\n\r", ch);
      return;
      break;

    case SUB_WRITING_NOTE:
      ch->pnote->text = copy_buffer (ch);
      stop_editing (ch);
      return;
      break;
    }


  if (ch->level < 5 && ch->pcdata->reincarnation == 0)
    {
      send_to_char ("You must be at least level 5 to use the Note Boards.\n\r",
		    ch);
      return;
    }

  argument = one_argument (argument, arg);
  smash_tilde (argument);

  if (ch->in_room != NULL && IS_SET (ch->in_room->room_flags, ROOM_NOTE_BOARD)
      && ch->in_room->vnum > 0)
    {
      room_board = TRUE;
      room_vnum = ch->in_room->vnum;
    }
  else
    {
      room_board = FALSE;
      room_vnum = 0;
    }

  *buf = '\0';


  if (!strcasecmp (arg, "list") || arg[0] == '\0')
    {
      bool found_topic;

      found_topic = FALSE;
      if (room_board)
	{
	  found_topic = TRUE;
	  strcpy (buf, "{070}Note Board:\n\r");
	}
      else
       sprintf (buf, "{070}TOPIC: %s\n\r", topic_table[ch->pcdata->note_topic].name);

      if (is_number (argument))
	{
	  int curtime;

	  send_to_char_color (buf, ch);
	  curtime = current_time - (24 * 60 * 60 * atol (argument));
	  vnum = 0;
	  blen = 0;
	  *buf = '\0';
	  for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	    {
	      if (is_note_to (ch, pnote) && pnote->time >= curtime &&
		  ((!room_board && pnote->room_vnum < 1) ||
		   (room_board && pnote->room_vnum == room_vnum)))
		{
		  if (room_board ||
		      pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
		    sprintf (buft, "{070}[%3u]{030} %s to %s:{070} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
	 	  else
		    sprintf (buft, "{170}[%3u]{120}*{130}%s to %s:{170} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
		  blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
		}
	      vnum++;
	    }
	  str_apd_max (buf, "{070}The {120}*{070} denotes an unread note.\n\r", blen, MAX_STRING_LENGTH);
	  send_to_char_color (buf, ch );
	}
      else if (!strcasecmp (argument, "new"))
	{
	  int oldTopic;
	  time_t ctm_t;
	  bool new_note;

	  ctm_t = 0;
	  new_note = FALSE;
	  if (ch->pcdata->last_time > 0)
	    ctm_t = ch->pcdata->last_time;
	  vnum = 0;
	  blen = 0;
	  *buf = '\0';
	  oldTopic = ch->pcdata->note_topic;
	  for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	    {
	      ch->pcdata->note_topic = pnote->topic;
	      if (is_note_to (ch, pnote) && pnote->time >= ctm_t &&
		  ((!room_board && pnote->room_vnum < 1) ||
		   (room_board && pnote->room_vnum == room_vnum)))
		{
		  new_note = TRUE;
		  if (room_board ||
	    	      pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
		    sprintf (buft, "{070}[%3u]{030} %s to %s:{070} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
	 	  else
		    sprintf (buft, "{170}[%3u]{120}*{130}%s to %s:{170} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
		  blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
		}
	      vnum++;
	    }
	  str_apd_max (buf, "{070}The {120}*{070} denotes an unread note.\n\r", blen, MAX_STRING_LENGTH);
	  if (!new_note)
	    send_to_char ("There have been no new notes since you last logged out.\n\r", ch);
	  else
	    send_to_char_color (buf, ch );
	  ch->pcdata->note_topic = oldTopic;
	}
      else if (!strcasecmp (argument, "unread"))
	{
	  int oldTopic;
	  int unreadNum[MAX_TOPIC]={0};	
	  bool found=FALSE;
	  oldTopic = ch->pcdata->note_topic;
	  
  	  for (pnote= first_note; pnote != NULL; pnote = pnote->next)
	  {
	      ch->pcdata->note_topic = pnote->topic;
	      if (is_note_to (ch, pnote) &&
		  ((!room_board && pnote->room_vnum < 1) ||
		   (room_board && pnote->room_vnum == room_vnum)) 
		   && pnote->time > ch->pcdata->topic_stamp[pnote->topic])
		{
                   unreadNum[pnote->topic]++;
		   found=TRUE;
 		}
   	  }		
	  if (found==FALSE)
	   send_to_char( "You've read all the notes in all topics.\n\r", ch);
	  else
	  {
 	   send_to_char_color("{170}Number of unread notes in all topics:\n\r", ch);
	   for (cnt=0;cnt<MAX_TOPIC;cnt++)
 	    ch_printf_color(ch, "{030}%2d  {070}%-22s {130}%d\n\r", cnt, topic_table[cnt].name, unreadNum[cnt]);
 	  }
	  ch->pcdata->note_topic = oldTopic;
	
	}
      else if (argument[0] != '\0')
	{
	  send_to_char_color (buf, ch);
	  vnum = 0;
	  blen = 0;
	  *buf = '\0';
	  for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	    {
	      if (is_note_to (ch, pnote) &&
		  ((!room_board && pnote->room_vnum < 1) ||
		   (room_board && pnote->room_vnum == room_vnum)) &&
		  ((!str_infix (argument, pnote->sender)) ||
		   (!str_infix (argument, pnote->subject)) ||
		   (!str_infix (argument, pnote->to_list))))
		{
		  if (room_board || 
		      pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
		    sprintf (buft, "{070}[%3u]{030} %s to %s:{070} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
	 	  else
		    sprintf (buft, "{170}[%3u]{120}*{130}%s to %s:{170} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
		  blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
		}
	      vnum++;
	    }
	  str_apd_max (buf, "{070}The {120}*{070} denotes an unread note.\n\r", blen, MAX_STRING_LENGTH);
	  send_to_char_color (buf, ch );
	}
      else
	{
	  send_to_char_color (buf, ch);
	  blen = 0;
	  *buf = '\0';
	  vnum = 0;
	  for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	    {
	      if (is_note_to (ch, pnote) &&
		  ((!room_board && pnote->room_vnum < 1) ||
		   (room_board && pnote->room_vnum == room_vnum)))
		{
		  if (room_board ||
		      pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
		    sprintf (buft, "{070}[%3u]{030} %s to %s:{070} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
	 	  else
		    sprintf (buft, "{170}[%3u]{120}*{130}%s to %s:{170} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
		  blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
		}
	      vnum++;
	    } 
	  str_apd_max (buf, "{070}The {120}*{070} denotes an unread note.\n\r", blen, MAX_STRING_LENGTH);
	  send_to_char_color (buf, ch );
	}
      return;
    }

  if (!strcasecmp (arg, "next"))
    {
      vnum = 0;
	
      for (pnote = first_note; pnote != NULL; pnote = pnote->next, vnum++)
	{
	  if (pnote->time <= ch->pcdata->last_note)
	    continue;
	  if (is_note_to (ch, pnote) &&
	      ((!room_board && pnote->room_vnum < 1) ||
	       (room_board && pnote->room_vnum == room_vnum)))
	    {
	      blen = 0;
	      *buf = '\0';
	      sprintf (buft, "{170}[%3u]{130} %s:{170} %s\n\r%s\t\tTopic: %s\n\rTo: %s{070}\n\r",
		       vnum, pnote->sender,
		       pnote->subject, pnote->date,
		       room_board ? "Private Note Board" :
			topic_table[pnote->topic].name, 
		       pnote->to_list);
	      blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
	      blen = str_apd_max (buf, justify (pnote->text),
				  blen, MAX_STRING_LENGTH);
	      send_to_char_color (buf, ch );
	      if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
	        ch->pcdata->topic_stamp[ch->pcdata->note_topic] = pnote->time;
	      ch->pcdata->last_note = pnote->time;
	      return;
	    }
	}
       send_to_char ("End of notes.\n\r", ch);
      return;
    }
  if (!strcasecmp (arg, "unread"))
    {
      vnum = 0;
	
      for (pnote = first_note; pnote != NULL; pnote = pnote->next, vnum++)
	{
	  if (pnote->time <= ch->pcdata->topic_stamp[ch->pcdata->note_topic])
	    continue;
	  if (is_note_to (ch, pnote) &&
	      ((!room_board && pnote->room_vnum < 1) ||
	       (room_board && pnote->room_vnum == room_vnum)))
	    {
	      blen = 0;
	      *buf = '\0';
	      sprintf (buft, "{170}[%3u]{130} %s:{170} %s\n\r%s\t\tTopic: %s\n\rTo: %s{070}\n\r",
		       vnum, pnote->sender,
		       pnote->subject, pnote->date,
		       room_board ? "Private Note Board" :
			topic_table[pnote->topic].name, 
		       pnote->to_list);
	      blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
	      blen = str_apd_max (buf, justify (pnote->text),
				  blen, MAX_STRING_LENGTH);
	      send_to_char_color (buf, ch);
	      if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
	        ch->pcdata->topic_stamp[ch->pcdata->note_topic] = pnote->time;
	      ch->pcdata->last_note = pnote->time;
	      return;
	    }
	}
      if (ch->pcdata->note_topic <5)
      {
       ch->pcdata->note_topic++;
       ch_printf(ch, "You've read all the notes in this topic. Proceeding to %s.\n\r", topic_table[ch->pcdata->note_topic].name); 
       do_note(ch, "unread");
      }
      else
       send_to_char ("End of notes.\n\r", ch);
      return;
    }
  if (!strcasecmp (arg, "read") || is_number(arg) || !strcasecmp(arg, "all" ))
    {
      bool fAll, fAnytopic;
      int oldTopic = 0;
    fAnytopic=FALSE;

    if(is_number(arg))
      {
      fAll = FALSE;
      fAnytopic=TRUE;
      anum = atol( arg );
      }
    else if(!strcasecmp(arg,"all"))
      {
      fAll = TRUE;
      anum = 0;
      }
    else
      {
      if ( !strcasecmp( argument, "all" ) )
        {
        fAll = TRUE;
        anum = 0;
        }
      else if ( is_number( argument ) )
        {
        fAll = FALSE;
        anum = atol( argument );
        }
      else
        {
        send_to_char( "Note read which number?\n\r", ch );
        return;
        }
      }

      vnum = 0;
      for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	{
          if (fAnytopic)
            {
              oldTopic = ch->pcdata->note_topic;
              ch->pcdata->note_topic = pnote->topic;
            }

	  if (is_note_to (ch, pnote) && (vnum == anum || fAll) &&
	      ((!room_board && pnote->room_vnum < 1) ||
	       (room_board && pnote->room_vnum == room_vnum)))
	    {
	      blen = 0;
	      *buf = '\0';
	      sprintf (buft, "{170}[%3u]{130} %s:{170} %s\n\r%s\t\tTopic: %s\n\rTo: %s{070}\n\r",
		       vnum, pnote->sender,
		       pnote->subject, pnote->date,
		       room_board ? "Private Note Board" :
			topic_table[pnote->topic].name, 
		       pnote->to_list);
	      blen = str_apd_max (buf, buft, blen, MAX_STRING_LENGTH);
	      blen = str_apd_max (buf, justify (pnote->text),
				  blen, MAX_STRING_LENGTH);
	      send_to_char_color (buf, ch);
	      if (fAnytopic)
		ch->pcdata->note_topic = oldTopic;
	      ch->pcdata->last_note = pnote->time;
	      if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
	        ch->pcdata->topic_stamp[pnote->topic] = pnote->time;
	      return;
	    }
          if (fAnytopic)
            ch->pcdata->note_topic = oldTopic;
	  vnum++;
	}

      send_to_char ("No such note.\n\r", ch);
      return;
    }
  if (!strcasecmp (arg, "subject"))
    {
      note_attach (ch);
      STRFREE (ch->pnote->subject);
      ch->pnote->subject = STRALLOC (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "to"))
    {
      note_attach (ch);
      STRFREE (ch->pnote->to_list);
      ch->pnote->to_list = STRALLOC (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!strcasecmp (arg, "write"))
    {
      note_attach (ch);
      ch->substate = SUB_WRITING_NOTE;
      start_editing (ch, ch->pnote->text);
      return;
    }

  if (!strcasecmp (arg, "reply"))
    {
      NOTE_DATA *oldNote;
      int anum;

      /* find the note you are replying to */
      if (!is_number (argument))
	{
	  send_to_char ("Use 'note reply #' where # is the note number to reply to.\n\r", ch);
	  return;
	}
      vnum = atol (argument);
      anum = 0;
      for (oldNote = first_note; oldNote != NULL; oldNote = oldNote->next)
	if (anum++ == vnum)
	  break;
      if (oldNote == NULL)
	{
	  sprintf (buf, "There is no note #%u.\n\r", vnum);
	  send_to_char (buf, ch);
	  return;
	}

      if ((oldNote->room_vnum > 0 && !room_board) ||
	  (room_board && oldNote->room_vnum != ch->in_room->vnum))
	{
	  send_to_char ("There is no such note.\n\r", ch);
	  return;
	}

      note_attach (ch);
      STRFREE (ch->pnote->to_list);
      STRFREE (ch->pnote->subject);

      if (is_name ("all", oldNote->to_list) ||
	  is_name (oldNote->sender, oldNote->to_list))
	strcpy (buf, oldNote->to_list);
      else
	sprintf (buf, "%s %s", oldNote->sender, oldNote->to_list);
      ch->pnote->to_list = STRALLOC (buf);
      ch->pnote->topic   = oldNote->topic;

      if (oldNote->subject[0] == 'R' && oldNote->subject[1] == 'e' &&
	  oldNote->subject[2] == ':' && oldNote->subject[3] == ' ')
	{
	  ch->pnote->subject = STRALLOC (oldNote->subject);
	}
      else
	{
	  sprintf (buf, "Re: %s", oldNote->subject);
	  ch->pnote->subject = STRALLOC (buf);
	}

      sprintf (buf, "%s: %s\n\rTo: %s\n\r", ch->pnote->sender, ch->pnote->subject,
	       ch->pnote->to_list);
      send_to_char (buf, ch);
      return;
    }

  if (!strcasecmp (arg, "clear"))
    {
      if (ch->pnote != NULL)
	{
	  STRFREE (ch->pnote->text);
	  STRFREE (ch->pnote->subject);
	  STRFREE (ch->pnote->to_list);
	  STRFREE (ch->pnote->date);
	  STRFREE (ch->pnote->sender);
	  DISPOSE (ch->pnote);
	  ch->pnote = NULL;
	}

      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "show"))
    {
      if (ch->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}
      sprintf (buf, "%s: %s\n\rTo: %s\n\r", ch->pnote->sender, ch->pnote->subject,
	       ch->pnote->to_list);
      send_to_char (buf, ch);
      send_to_char (justify (ch->pnote->text), ch);
      return;
    }

  if (!strcasecmp (arg, "post"))
    {
      FILE *fp;
      char *strtime;
      bool saveAll = FALSE, canAnnounce=FALSE;
      NOTE_DATA *pnote_next;
      int curtime;

	  if (!IS_IMMORTAL(ch) && ch->which_god != GOD_POLICE )
	      /* && ((ch->pcdata->clan != NULL 
	      && !( !strcasecmp( ch->name, ch->pcdata->clan->leader  ) ) )
	      || ch->pcdata->clan == NULL)) */
	    canAnnounce = FALSE;
	  else
	    canAnnounce = TRUE;

      if (ch->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}

      if (!strcasecmp (ch->pnote->to_list, ""))
	{
	  send_to_char ("This note is addressed to no one!\n\r", ch);
	  return;
	}
      if (!strcasecmp (ch->pnote->subject, ""))
	{
	  send_to_char ("This note has no subject.\n\r", ch);
	  return;
	}
      if (!strcasecmp (ch->pnote->text, ""))
	{
	  send_to_char ("There is nothing written on this note.\n\r", ch);
	  return;
	}

      if ( ch->pnote->topic == -1 && !room_board 
	   && (!strcasecmp(argument, "") || !(is_number(argument))))
        {
	  send_to_char("You didn't specify which topic you wish to post this note under.\n\r", ch);
	  send_to_char ("The topics you can post under are:\n\r", ch);
	  send_to_char ("Reference Number    Topic Name:\n\r", ch);
	  for (cnt=0;cnt<MAX_TOPIC;cnt++ )
	    {
	      if (cnt == 0 && !canAnnounce)
		continue;
	      if (ch->level >= topic_table[cnt].min_level)
		{
		  sprintf (buf, "%2d                  %s\n\r", cnt, topic_table[cnt].name);
		  send_to_char (buf, ch);
		}
	    }
	  send_to_char("Syntax is: note post <topic number>.\n\r", ch);
	  return;
	}
      if ( ch->pnote->topic == -1 &&
	  !room_board && ( atol(argument)>5 || atol(argument)<0) )
        {
	  send_to_char("You specificed an invalid topic for this note to be posted under.\n\r", ch);
	  send_to_char ("The topics you can post under are:\n\r", ch);
	  send_to_char ("Reference Number    Topic Name:\n\r", ch);
	  for (cnt=0;cnt<MAX_TOPIC;cnt++ )
	    {
	      if (cnt == 0 && !canAnnounce)
		continue;
	      if (ch->level >= topic_table[cnt].min_level)
		{
		  sprintf (buf, "%2d                  %s\n\r", cnt, topic_table[cnt].name);
		  send_to_char (buf, ch);
		}
	    }
	  send_to_char("Syntax is: note post <topic number>.\n\r", ch);
	  return;
	}
      if (!room_board && ( atol(argument) == 0 || ch->pnote->topic == 0))
        {
	  if (!IS_IMMORTAL(ch) && ch->which_god != GOD_POLICE 
	      && ((ch->pcdata->clan != NULL 
	      && !( !strcasecmp( ch->name, ch->pcdata->clan->leader  ) ) )
	      || ch->pcdata->clan == NULL))
	     
           {
	    send_to_char("You are not allowed to post on the Announcements topic.\n\r", ch);
	    return;
	   }
  	}
      if (!room_board && ch->pnote->topic == -1)
        ch->pnote->topic = atol(argument);	
      if( ch->pnote->topic == 0 && !canAnnounce )
        ch->pnote->topic = 5;

      strtime = ctime (&current_time);
      strtime[strlen (strtime) - 1] = '\0';
      ch->pnote->date = STRALLOC (strtime);
      ch->pnote->time = current_time;
      if (IS_SET (ch->in_room->room_flags, ROOM_NOTE_BOARD))
      {
	ch->pnote->room_vnum = ch->in_room->vnum;
	ch->pnote->topic = 0;
      }
      else
	ch->pnote->room_vnum = 0;
      LINK (ch->pnote, first_note, last_note, next, prev);


      for (pnote = first_note; pnote->next != NULL; pnote = pnote_next)
	{
	  pnote_next = pnote->next;

	  /* do some clean-up while scanning */
	  curtime = current_time - (24 * 60 * 60 * 7);	/* one week old */
	  if (pnote->time <= curtime)
	    {
#ifdef UNLINKCHECK
  {
  NOTE_DATA *tnote;
  bool foundn;

  for( foundn=FALSE, tnote=first_note; tnote!=NULL; tnote=tnote->next)
    if( tnote==pnote )
      {
      foundn=TRUE;
      break;
      }
  if( foundn )
    UNLINK (pnote, first_note, last_note, next, prev);
  else
    bug( "UNLINK ERROR:  did not find note %s", pnote->subject );
  }
#else
    UNLINK (pnote, first_note, last_note, next, prev);
#endif
	      STRFREE (pnote->text);
	      STRFREE (pnote->subject);
	      STRFREE (pnote->to_list);
	      STRFREE (pnote->date);
	      STRFREE (pnote->sender);
	      DISPOSE (pnote);
	      saveAll = TRUE;
	    }
	}
      ch->pnote = NULL;

      /*  Added for recieving mail while on the game.  Chaos 12/15/93  */
      if (!room_board)
	for (fch = first_player; fch != NULL; fch = fch->next)
	  if (is_name (fch->ch->name, pnote->to_list))
	    {
	      if (fch->ch->ansi == 1)
		send_to_char ("\033[7;1;5mYou just received a note.\n\r", fch->ch);
	      else
		send_to_char ("*** You just received a note.\n\r", fch->ch);
	    }
      if (!saveAll)
	{
	  fclose (fpReserve);
	  fp = fopen (NOTE_FILE_T, "a");
	  if (fp == NULL)
	    {
	      perror (NOTE_FILE_T);
	    }
	  else
	    {
	      fprintf (fp, "Sender  %s~\nDate    %s~\nTime   %d\nTo      %s~\nSubject %s~\nTopic   %d\nText\n%s~\nRoom %u\n\n",
		    pnote->sender, pnote->date, pnote->time, pnote->to_list,
		       pnote->subject, pnote->topic, pnote->text,
		       pnote->room_vnum);
	      fflush (fp);
	    }
	  fpReserve = fopen (NULL_FILE, "r");
	}
      else
	{
	  fclose (fpReserve);
	  fp = fopen (NOTE_FILE_T, "w");
	  if (fp == NULL)
	    perror (NOTE_FILE_T);
	  else
	    {
	      for (pnote = first_note; pnote != NULL; pnote = pnote->next)
		{
		  fprintf (fp, "Sender  %s~\nDate    %s~\nTime   %d\nTo      %s~\nSubject %s~\nTopic   %d\nText\n%s~\nRoom %u\n\n",
			   pnote->sender, pnote->date, pnote->time,
			   pnote->to_list, pnote->subject,
			   pnote->topic, pnote->text,
			   pnote->room_vnum);
		}
	      fflush (fp);
	      fpReserve = fopen (NULL_FILE, "r");
	    }
	}
      if (!room_board)
        send_to_char ("Ok.\n\r", ch);
      else
        send_to_char ("You post your note onto the noteboard.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "remove"))
    {
      if (!is_number (argument))
	{
	  send_to_char ("Note remove which number?\n\r", ch);
	  return;
	}
      anum = atol (argument);
      vnum = 0;
      for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote) && vnum == anum)
	    {
	      note_remove (ch, pnote, FALSE);
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	  vnum++;
	}

      send_to_char ("No such note.\n\r", ch);
      return;
    }

  if (is_name (arg, "erase delete"))
    {
      if (!is_number (argument))
	{
	  send_to_char ("Note delete which number?\n\r", ch);
	  return;
	}

      anum = atol (argument);
      vnum = 0;
      for (pnote = first_note; pnote != NULL; pnote = pnote->next)
	{
	  if ((vnum == anum) &&
	      (IS_IMMORTAL(ch) || !strcmp (ch->name, pnote->sender)))
	    {
	      note_remove (ch, pnote, TRUE);
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	  vnum++;
	}

      send_to_char ("No such note.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "topic"))
    {
      if (room_board)
	{
	  send_to_char ("You are in a room containing a note board.\n\r", ch);
	  return;
	}
      if (is_number (argument))
	{
	  int num;

	  num = atol (argument);
	  if (num <0 || num >5)
	  {
	    send_to_char ("What topic was that?\n\r", ch);
            return;
	  }
	  ch->pcdata->note_topic = num;
 	  ch_printf( ch, "Note topic changed to: %s\n\r", topic_table[ch->pcdata->note_topic].name);
	  return;
	}
      else
	{
	  send_to_char_color ("{170}List of note topics:\n\r", ch);
	  send_to_char_color ("{130}Reference Number    {170}Topic Name:\n\r", ch);
	  for (cnt=0 ; cnt < MAX_TOPIC; cnt++ )
	    {
	      if (ch->level >= topic_table[cnt].min_level)
		{
		  sprintf (buf, "{030}%2d                  {070}%s\n\r", cnt, topic_table[cnt].name);
		  send_to_char_color (buf, ch);
		}
	    }
	}
      return;
    }

  send_to_char ("Huh?  Type 'help note' for usage.\n\r", ch);
  return;
}


void do_immtalk (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_STRING_LENGTH];

  if ( (which_god (ch) != GOD_POLICE && !IS_IMMORTAL(ch)) || IS_NPC(ch))
    {
      send_to_char ("You are not allowed to speak on that channel.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Immtalk what?\n\r", ch);
      return;
    }

  sprintf (buf, "%s immtalks '%s'", ch->name, argument);
  ch_printf_color (ch, "{120}%s\n\r", justify(buf));

  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (ch == fch || (which_god (fch)!=GOD_POLICE && fch->level<97 ) )
	continue;
      ch_printf_color (fch, "{120}%s\n\r", justify(buf));
    }
  return;
}

void do_reign (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_STRING_LENGTH];

  if (argument[0] == '\0')
    {
      send_to_char ("pray what?\n\r", ch);
      return;
    }

  sprintf (buf, "%s prays '%s'", ch->name, argument);
  ch_printf_color (ch, "{150}%s\n\r", justify(buf));

  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (ch == fch || (which_god (fch)!=GOD_POLICE && fch->level<96 ) )
	continue;
      ch_printf_color (fch, "{150}%s\n\r", justify(buf));
    }
  return;
}

void do_plan (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_STRING_LENGTH];
  int dest;

  if (  which_god(ch)!=GOD_CHAOS && which_god(ch)!=GOD_ORDER &&
        which_god(ch)!=GOD_POLICE )
    {
      send_to_char ("You are not a follower of a god.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Plan what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_PLAN))
    {
      send_to_char ("You decided not to plan.\n\r", ch);
      return;
    }
  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {				/*   Include no-talk for dumb creatures */
      send_to_char ("You're too dumb to talk.\n\r", ch);
      return;
    }
  MOBtrigger = FALSE;

  dest = 0;
  if (which_god(ch) == GOD_POLICE)	/* Police plan any channel - Chaos 4/26/96 */
    {
      if (*argument >= '1' && *argument <= '4')
	{
	  dest = *argument - '0';
	  argument++;
	  while (*argument == ' ')
	    argument++;
	}
    }

  if (dest == 0)
    dest = which_god (ch);


  sprintf(buf, "You plan '%s'", argument );
  ch_printf_color (ch, "{160}%s\n\r", justify(buf));

  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (IS_SET (fch->act, PLR_PLAN))
	continue;
      if (ch == fch || (dest != which_god(fch) && which_god(fch) != GOD_POLICE))
	continue;
	
      if (which_god(fch) == GOD_POLICE)
      {
	switch(dest)
	{
	case 1 : sprintf(buf, "%s order plans '%s'", ch->name, argument);
		  break;
	case 2 : sprintf(buf, "%s chaos plans '%s'", ch->name, argument);
		  break;
	case 3 : sprintf(buf, "%s demise plans '%s'", ch->name, argument);
		  break;
	case 4 : sprintf(buf, "%s sheriff plans '%s'", ch->name, argument);
		  break;
	default : sprintf(buf, "%s plans '%s'", ch->name, argument);
		  break;
	}
      }
      else
	sprintf(buf, "%s plans '%s'", ch->name, argument);
 	
     ch_printf_color (fch, "{160}%s\n\r", justify(buf));
    }

  return;
}

void do_beep (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  char buf[80];

  fch = NULL;
  sprintf (buf, "%s beeped.\007\n\r", get_name (ch));
  if (buf[0] >= 'a' && buf[0] <= 'z')
    buf[0] -= ('a' - 'A');
  if (argument[0] == '\0')	/*  Beeping Room */
    {
      for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
	if (!IS_NPC (fch) && fch->vt100 == 1 && fch != ch)
	  {
	    /* poll the block list of the victim */
	    if (blocking (fch, ch))
	      continue;
	    send_to_char (buf, fch);
	  }
      send_to_char ("You beep the room.\n\r", ch);
      return;
    }
  if ((fch = get_char_world (ch, argument)) == NULL
      || IS_NPC (fch))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  /* poll the block list of the victim */
  if (blocking (fch, ch))
    {
      act ("$E refuses to hear you.", ch, 0, fch, TO_CHAR);
      return;
    }
  send_to_char (buf, fch);
  send_to_char ("You beep.\n\r", ch);
  return;
}

void
do_chat (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_INPUT_LENGTH];
  char jbuf[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
    {
      send_to_char ("Chat what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_CHAT))
    {
      send_to_char ("You decided not to chat.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }
  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {				/*   Include no-talk for dumb creatures */
      send_to_char ("Your too dumb to talk.\n\r", ch);
      return;
    }
  MOBtrigger = FALSE;

  sprintf (jbuf, "You chat '%s'", argument);
  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r",
	  ch->pcdata->color[0][9], ch->pcdata->color[1][9], justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, ch);

  sprintf (buf, "%s chats '%s'", get_name (ch), argument);
  strcpy (jbuf, justify (buf));
  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (ch == fch || ch->race != fch->race || IS_SET (fch->act, PLR_CHAT))
	continue;
      if (fch->ansi == 1)
	sprintf (buf, "\033[1;%d;%dm%s\n\r",
		 fch->pcdata->color[0][9], fch->pcdata->color[1][9], jbuf);
      else
	sprintf (buf, "%s\n\r", jbuf);
      send_to_char (buf, fch);
    }
  return;
}


void
do_battle (char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_INPUT_LENGTH];

  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (IS_NPC (fch))
	continue;
      if (IS_SET (fch->pcdata->player2_bits, PLR2_BATTLE))
	continue;
      if (fch->ansi == 1)
	sprintf (buf, "\033[1;31m%s\n\r", justify (argument));
      else
	sprintf (buf, "%s\n\r", justify (argument));
      send_to_char (buf, fch);
    }
  return;
}

void
do_channel_talk (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_INPUT_LENGTH];
  char jbuf[MAX_INPUT_LENGTH];

  if (IS_NPC (ch))
    return;
  if (argument[0] == '\0')
    {
      send_to_char ("Talk what?\n\r", ch);
      return;
    }
  if (ch->pcdata->channel == 0)
    {
      send_to_char ("You have the channels turned off.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }
  sprintf (jbuf, "You talk '%s'", argument);
  if (ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r",
	  ch->pcdata->color[0][9], ch->pcdata->color[1][9], justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, ch);

  sprintf (buf, "%s talks '%s'", get_name (ch), argument);
  strcpy (jbuf, justify (buf));
  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (ch == fch || ch->pcdata->channel != fch->pcdata->channel)
	continue;
      if (!can_understand (fch, ch))
	if (ch->level < MAX_LEVEL - 3 && fch->level < MAX_LEVEL - 3)
	  continue;
      if (!IS_NPC (fch) && fch->ansi == 1)
	sprintf (buf, "\033[1;%d;%dm%s\n\r",
		 fch->pcdata->color[0][9], fch->pcdata->color[1][9], jbuf);
      else
	sprintf (buf, "%s\n\r", jbuf);
      send_to_char (buf, fch);
    }


  return;
}

void
do_shout (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  PLAYER_GAME *fpl;
  char buf[MAX_INPUT_LENGTH];
  char jbuf[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
    {
      send_to_char ("Shout what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }
  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {				/*   Include no-talk for dumb creatures */
      send_to_char ("You are too dumb to talk.\n\r", ch);
      return;
    }
  MOBtrigger = FALSE;
  sprintf (jbuf, "You shout '%s'", argument);
  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r",
	  ch->pcdata->color[0][9], ch->pcdata->color[1][9], justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, ch);

  sprintf (buf, "%s shouts '%s'", get_name (ch), argument);
  strcpy (jbuf, justify (buf));
  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (ch == fch || fch->in_room->area != ch->in_room->area ||
	  fch->position <= POS_SLEEPING)
	continue;
      if (blocking (fch, ch))
	continue;
      if (!can_understand (fch, ch))
	continue;
      if (!IS_NPC (fch) && fch->ansi == 1)
	sprintf (buf, "\033[1;%d;%dm%s\n\r",
		 fch->pcdata->color[0][9], fch->pcdata->color[1][9], jbuf);
      else
	sprintf (buf, "%s\n\r", jbuf);
      send_to_char (buf, fch);
    }


  /*MOBtrigger=TRUE; */
  mprog_speech_trigger (argument, ch);
  if (!IS_NPC (ch))
    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
  return;
}



void
do_buffer (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  int page, pt, cnt, ct, pages;
  char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
  bool done;

  if (IS_NPC (ch))
    return;
  d = ch->desc;
  if (d->original != NULL || d->character!=ch )
    return;
  if (argument[0] != '\0')
    pages = atol (argument);
  else
    pages = 1;
  if (pages < 0)
    pages = 0;
  done = FALSE;
  page = ch->vt100_type%100 - 7 - ch->pcdata->tactical_mode%100;
  ct = 0 - page;
  if (pages > 99)
    pages = 99;
  for (pt = ch->pcdata->scroll_end; !done; pt--)
    {
      if (ch->pcdata->scroll_buf[pt] == '\n')
	ct++;
      if (ct > page * pages)
	done = TRUE;
      if ((ch->pcdata->scroll_start == 0 && pt == 0) || pt == ch->pcdata->scroll_end + 1 ||
	  (pt == 0 && ch->pcdata->scroll_end == MAX_BUFFER_LENGTH - 1))
	{
	  send_to_char ("That is too far back.\n\r", ch);
	  return;
	}
      if (pt == 0)
	pt = MAX_BUFFER_LENGTH;
    }
  pt++;
  if (pt >= MAX_BUFFER_LENGTH)
    pt = 0;
  while (ch->pcdata->scroll_buf[pt] == '\n' || ch->pcdata->scroll_buf[pt] == '\r')
    {
      pt++;
      if (pt >= MAX_BUFFER_LENGTH)
	pt = 0;
    }

    cnt = 82;
    sprintf (buf, "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv Pages back: %2d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\r", pages);
    done = FALSE;

  for (ct = 0; !done; cnt++)
    {
    if( cnt>=MAX_STRING_LENGTH-300 )
      done=TRUE;
      buf[cnt] = ch->pcdata->scroll_buf[pt];
      pt++;
      if (pt >= MAX_BUFFER_LENGTH)
	pt = 0;
      if (buf[cnt] == '\n')
	ct++;
      if (ct >= page && buf[cnt] != '\n' && buf[cnt] != '\r')
	done = TRUE;
    }
  buf[cnt - 1] = '\0';
  if (pages > 0)
    {
      if (ch->ansi == 1)
	sprintf (buf2, "\033[1m^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Pages back: %2d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\033[m\n\r", pages);
      else
	sprintf (buf2, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Pages back: %2d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\r", pages);
      strcat (buf, buf2);
    }
  ch->pcdata->scroll_ip = 1;
  send_to_char (buf, ch);
  ch->pcdata->scroll_ip = 0;
  return;
}

void
do_grep (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  int pt, lend, lstart, ofs, ct, cnt, fnd;
  char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
  char lines[10][MAX_INPUT_LENGTH];
  bool done, found, looped;

  ofs = ('a' - 'A');
  fnd = 0;
  looped = FALSE;

  if (IS_NPC (ch))
    return;
  d = ch->desc;
  if (d->original != NULL)
    return;
  if (argument[0] == '\0')
    {
      send_to_char ("Grep what?\n\r", ch);
      return;
    }
  buf[0] = '\0';
  done = FALSE;
  for (ct = 0; ct < 10; ct++)
    lines[ct][0] = '\0';
  for (ct = 0; argument[ct] != '\0'; ct++)
    if (argument[ct] >= 'A' && argument[ct] <= 'Z')
      buf2[ct] = argument[ct];
    else if (argument[ct] >= 'a' && argument[ct] <= 'z')
      buf2[ct] = argument[ct] - ofs;
    else if (argument[ct] == ' ')
      buf2[ct] = ' ';
    else
      {
	sprintf (buf, "Grep only works with words.  '%s' is invalid.\n\r",
		 argument);
	send_to_char (buf, ch);
	return;
      }
  buf2[ct] = '\0';
  ct = 0;
  lend = -1;
  lstart = -1;
  for (pt = ch->pcdata->scroll_end; !done; pt--)
    {
      if (ch->pcdata->scroll_buf[pt] == '\n')
	{
	  if (lend != -1 && lend > lstart)
	    {
	      found = FALSE;
	      for (ct = lstart; ct != lend; ct++)
		{
		  found = TRUE;
		  for (cnt = 0; buf2[cnt] != '\0'; cnt++)
		    if (buf2[cnt] != ch->pcdata->scroll_buf[ct + cnt] &&
			buf2[cnt] != ch->pcdata->scroll_buf[ct + cnt] - ofs)
		      {
			found = FALSE;
			break;
		      }
		  if (found == TRUE)
		    {
		      pt -= 2;	/* don't grab twice */
		      for (cnt = lstart; cnt < lend + 2 && cnt < (lstart + MAX_INPUT_LENGTH); cnt++)
			lines[fnd][cnt - lstart] = ch->pcdata->scroll_buf[cnt];
		      lines[fnd][cnt - lstart] = '\0';
		      fnd++;
		      if (fnd > 9)
			{
			  looped = TRUE;
			  fnd = 0;
			}
		    }
		}
	    }
	  lend = pt;
	}
      else if (ch->pcdata->scroll_buf[pt] != '\r')
	lstart = pt;

      if (pt == ch->pcdata->scroll_end + 1 || (pt == MAX_BUFFER_LENGTH &&
				   ch->pcdata->scroll_end == 0) || fnd >= 9)
	done = TRUE;
      if (pt <= 0)
	pt = MAX_BUFFER_LENGTH;
    }
  sprintf (buf3, " Search for the word: %s ", buf2);
  buf[0] = '\0';
  for (cnt = 0; cnt < (39 - (strlen (buf3) / 2)); cnt++)
    strcat (buf, "_");
  strcat (buf, buf3);
  for (cnt = 0; cnt < (39 - (strlen (buf3) / 2)); cnt++)
    strcat (buf, "_");
  strcat (buf, "\n\r");
  if (!looped)
    {
      for (cnt = fnd - 1; cnt >= 0; cnt--)
	strcat (buf, lines[cnt]);
    }
  else
    {
      for (cnt = fnd - 1; cnt >= 0; cnt--)
	strcat (buf, lines[cnt]);
      for (cnt = 9; cnt >= fnd; cnt--)
	strcat (buf, lines[cnt]);
    }
  strcat (buf, "------------------------------------------------------------------------------\n\r");
  ch->pcdata->scroll_ip = 1;
  send_to_char (buf, ch);
  ch->pcdata->scroll_ip = 0;
  return;
}

void
do_say (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *fch;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char jbuf[MAX_STRING_LENGTH];

  if (IS_NPC (ch) && ch->pIndexData->vnum == 9900)
    ch = ch->desc->original;	/* make this the real speaker */
  if (argument[0] == '\0')
    {
      if (!IS_NPC (ch) && ch->ansi == 1)
	{
	  sprintf (buf, "\033[0;%d;%dmSay What?\n\r", ch->pcdata->color[0][9],
		   ch->pcdata->color[1][9]);
	  send_to_char (buf, ch);
	  return;
	}
      send_to_char ("Say what?\n\r", ch);
      return;
    }
  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {				/*   Include no-talk for dumb creatures */
      send_to_char ("Your too dumb to talk.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }
  MOBtrigger = FALSE;
  sprintf (jbuf, "You say '%s'", argument);

  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s", ch->pcdata->color[0][9],
	     ch->pcdata->color[1][9], justify (jbuf));
  else
    strcpy (buf, justify (jbuf));
  strcat (buf, "\n\r");
  send_to_char (buf, ch);
  if (IS_SET (ch->hook, 8))
    speech (ch, ch, argument);

  for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
    {
      if (ch == fch || fch->position <= POS_SLEEPING)
	continue;
      /* poll the block list of the fch */
      if (blocking (fch, ch))
	continue;

      if (!can_understand (fch, ch))
	{
	  if (!IS_NPC (fch) && can_see (ch, fch))
	    act ("$N does not understand $t.", ch,
		 race_table[UNSHIFT (ch->speak)].race_name, fch, TO_CHAR);
	  translate (argument, buf);
	  sprintf (buf2, "%s says '%s'", get_name (ch), buf);
	  strcpy (jbuf, justify (buf2));
	  strcat (jbuf, "\n\r");
	  send_to_char (jbuf, fch);
	  continue;
	}
      if ((IS_SET (fch->hook, 1) && IS_SET (fch->hook, 4)) || !IS_SET (fch->hook, 1))
	{
	  sprintf (jbuf, "%s says '%s'", get_name (ch), argument);
	  if (!IS_NPC (fch) && fch->ansi == 1)
	    sprintf (buf, "\033[0;%d;%dm%s",
		     fch->pcdata->color[0][9], fch->pcdata->color[1][9],
		     justify (jbuf));
	  else
	    strcpy (buf, justify (jbuf));
	  strcat (buf, "\n\r");
	  send_to_char (buf, fch);
	  if (IS_SET (fch->hook, 1))
	    speech (ch, fch, argument);
	}
    }
  if (IS_NPC (ch))
    return;
  /*MOBtrigger=TRUE; */
  mprog_speech_trigger (argument, ch);
  return;
}


void
do_dump (CHAR_DATA * ch, char *argument)
{

  if (IS_NPC (ch))
    return;

  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }

  if (ch->obj_prog_ip != 0 || ch->alias_ip != 0 ||
      ch->desc->back_buf != NULL)
    return;

  send_to_char ("Waiting for Dump.  Issue 'stop' to cancel.\n\r", ch);
  ch->pcdata->dump = TRUE;
  return;
}




void do_tell (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char jbuf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int position;

  if (ch == NULL)
    return;

  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }

  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {
      send_to_char ("Your too dumb to talk.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Tell whom what?\n\r", ch);
      return;
    }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      if ((victim = get_player_world (ch, arg)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
    }
  if (!IS_NPC (victim) && is_affected (victim, gsn_greater_stealth)
      && !(IS_SET (ch->act, PLR_HOLYLIGHT)
	   || (is_affected (ch, gsn_truesight)
	       && ch->mclass[CLASS_ILLUSIONIST] >= victim->level)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && (IS_SET (victim->pcdata->player2_bits, PLR2_AFK)))
    {
      ch_printf (ch, "%s is afk and may not see your message.\n\r", get_name (victim));
    }
  if (!IS_NPC (victim) && (!victim->desc))
    {
      ch_printf (ch, "%s is link-dead.\n\r", get_name (victim));
      return;
    }
  if (victim->desc		/* Make sure there's a desc first :) Martin */
      && victim->desc->connected == CON_EDITING
      && get_trust (ch) < LEVEL_IMMORTAL)
    {
      act ("$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR);
      return;
    }

  /* poll the block list of the victim */
  if (blocking (victim, ch))
    {
      act ("$E refuses to hear you.", ch, 0, victim, TO_CHAR);
      return;
    }

  /* Check for non-understanding    Chaos 11/29/93   */
  if (!can_understand (victim, ch))
    {
      if (!IS_NPC (victim))
	{
	  sprintf (buf, "%s does not understand %s.\n\r", get_name (victim),
		   race_table[UNSHIFT (ch->speak)].race_name);
	  if (buf[0] <= 'z' && buf[0] >= 'a')
	    buf[0] -= ('a' - 'A');
	  send_to_char (buf, ch);
	}
      else
	{
	  sprintf (buf, "%s only understands %s.\n\r", get_name (victim),
		   race_table[UNSHIFT (victim->speak)].race_name);
	  if (buf[0] <= 'z' && buf[0] >= 'a')
	    buf[0] -= ('a' - 'A');
	  send_to_char (buf, ch);
	}
      position = victim->position;
      victim->position = POS_STANDING;
      translate (argument, buf);
      sprintf (jbuf, "%s tells you '%s'\n\r", get_name (ch), buf);
      send_to_char (justify (jbuf), victim);
      victim->position = position;
      return;
    }

  MOBtrigger = FALSE;
  sprintf (jbuf, "You tell %s '%s'", get_name (victim), argument);
  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r", ch->pcdata->color[0][9],
	     ch->pcdata->color[1][9], justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, ch);
  position = victim->position;
  victim->position = POS_STANDING;
  sprintf (jbuf, "%s tells you '%s'", get_name (ch), argument);
  if (!IS_NPC (victim) && victim->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r",
	     victim->pcdata->color[0][9], victim->pcdata->color[1][9],
	     justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, victim);
  /*MOBtrigger=TRUE; */
  victim->position = position;
  victim->reply = ch;
  if (IS_SET (victim->hook, 1))
    speech (ch, victim, argument);
  if (IS_SET (ch->hook, 8))
    speech (ch, ch, argument);

  return;
}

void
do_etell (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int position, expl, blocked;

  expl = 0;
  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }

  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {
      send_to_char ("You're too dumb to talk.\n\r", ch);
      return;
    }

  /* These guys shouldn't lose experience, so don't allow etell -  
     Chaos 5/2/99  */
  if(ch->level >= 65)
    {
      send_to_char ("You are too high a level the max is level 65, try using regular tell instead.\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Tell whom what?\n\r", ch);
      return;
    }


  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ((victim = get_char_world (ch, arg)) == NULL
      || (IS_NPC (victim) && victim->in_room != ch->in_room))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  /* poll the block list of the victim, make it cost more exp */
  blocked = 1;
  if (blocking (victim, ch))
    {
      if (ch->exp < 100000)
	{
	  act ("$E refuses to hear you.", ch, 0, victim, TO_CHAR);
	  return;
	}
      else
	blocked = 10;
    }

  if (!IS_SET (ch->act, PLR_SILENCE))
    {
      MOBtrigger = FALSE;
      if (!IS_NPC (ch) && ch->ansi == 1)
	sprintf (buf, "\033[1;%d;%dmYou etell %s '%s'\n\r", ch->pcdata->color[0][9],
		 ch->pcdata->color[1][9], get_name (victim), argument);
      else
	sprintf (buf, "You etell %s '%s'\n\r", get_name (victim), argument);
      send_to_char (buf, ch);
      position = victim->position;
      victim->position = POS_STANDING;
      if (!IS_NPC (victim) && victim->ansi == 1)
	sprintf (buf, "\033[1;%d;%dmEMERGENCY: %s tells you '%s'\n\r",
		 victim->pcdata->color[0][9], victim->pcdata->color[1][9],
		 get_name (ch), argument);
      else
	sprintf (buf, "EMERGENCY: %s tells you '%s'\n\r",
		 get_name (ch), argument);
      send_to_char (buf, victim);
      /*MOBtrigger=TRUE; */
      victim->position = position;
      victim->reply = ch;
    }
  else
    send_to_char ("Good try, but you're out of luck.\n\r", ch);
  expl = ch->level * ch->level * 4 + 10 * ch->level + 100;
  expl = blocked * expl;
  sprintf (buf, "You lose %d experience for the emergency.\n\r", expl);
  send_to_char (buf, ch);
  gain_exp (ch, 0 - expl);

  /* To discourage abuse of etell to lose experience. -Presto 3/20/99 */
  if(!IS_NPC(ch) && ch->level >= 20)
    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

  return;
}


void
do_reply (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int position;

  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }

  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_SMART))
    {
      send_to_char ("Your too dumb to talk.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }

  if ((victim = ch->reply) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (!IS_NPC (victim) && (IS_SET (victim->pcdata->player2_bits, PLR2_AFK)))
    {
      ch_printf (ch, "%s is afk and may not see your message.\n\r", get_name (victim));
    }


  if (!IS_NPC (victim) && (!victim->desc))
    {
      ch_printf (ch, "%s is link-dead.\n\r", get_name (victim));
      return;
    }
  /* poll the block list of the victim */
  if (blocking (victim, ch))
    {
      act ("$E refuses to hear you.", ch, 0, victim, TO_CHAR);
      return;
    }

  /* Check for non-understanding    Chaos 11/29/93   */
  if (!can_understand (victim, ch))
    {
      if (!IS_NPC (victim))
	act ("$N does not understand $t.", ch,
	     race_table[UNSHIFT (ch->speak)].race_name, victim, TO_CHAR);
      else
	act ("$N only understands $t.", ch,
	     race_table[UNSHIFT (victim->speak)].race_name, victim, TO_CHAR);
      position = victim->position;
      victim->position = POS_STANDING;
      translate (argument, buf);
      act ("$n tells you '$t'", ch, buf, victim, TO_VICT);
      victim->position = position;
      return;
    }
  MOBtrigger = FALSE;
  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dmYou reply to %s '%s'", ch->pcdata->color[0][9],
	     ch->pcdata->color[1][9], get_name (victim), argument);
  else
    sprintf (buf, "You reply to %s '%s'", get_name (victim), argument);
  send_to_char (justify (buf), ch);
  position = victim->position;
  victim->position = POS_STANDING;
  if (!IS_NPC (victim) && victim->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s replies to you '%s'\n\r",
	     victim->pcdata->color[0][9], victim->pcdata->color[1][9],
	     get_name (ch), argument);
  else
    sprintf (buf, "%s replies to you '%s'\n\r", get_name (ch), argument);
  send_to_char (justify (buf), victim);
  /*MOBtrigger=TRUE; */
  victim->position = position;
  victim->reply = ch;

  return;
}


void
do_hook (CHAR_DATA * ch, char *argument)
{
  bool test;
  char buf[MAX_STRING_LENGTH];

  test = FALSE;

  if (IS_NPC (ch))
    return;

  if (argument[0] == 'A' || argument[0] == 'a')
    {
      if (IS_SET (ch->hook, 1))
	ch->hook -= 1;
      else
	ch->hook += 1;
      test = TRUE;
    }
  if (argument[0] == 'S' || argument[0] == 's')
    {
      if (IS_SET (ch->hook, 8))
	ch->hook -= 8;
      else
	ch->hook += 8;
      test = TRUE;
    }
  if (argument[0] == 'V' || argument[0] == 'v')
    {
      if (IS_SET (ch->hook, 2))
	ch->hook -= 2;
      else
	ch->hook += 2;
      test = TRUE;
    }
  if (argument[0] == 'E' || argument[0] == 'e')
    {
      if (IS_SET (ch->hook, 4))
	ch->hook -= 4;
      else
	ch->hook += 4;
      test = TRUE;
    }

  buf[0] = '\0';
  if (argument[0] == '\0' || test)
    {
      sprintf (buf, "Current HOOK settings:\n\r      Audio: %s\n\rscreen Echo: %s\n\r  Self echo: %s\n\r",
	       IS_SET (ch->hook, 1) ? "ON" : "OFF",
	       IS_SET (ch->hook, 4) ? "ON" : "OFF",
	       IS_SET (ch->hook, 8) ? "ON" : "OFF");
      send_to_char (buf, ch);
      return;
    }

  send_to_char ("That is not an option for HOOK.\n\r", ch);
  return;
}

void
do_emote (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char tbuf[MAX_STRING_LENGTH];
  char *plast;
  PLAYER_GAME *fpl;
  CHAR_DATA *fch;

  /*   Chaos 11/27/93   set for RACE --> LANGUAGE only    */

  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }

  if (argument[0] == '\0')
    {
      send_to_char ("Emote what?\n\r", ch);
      return;
    }

  for (plast = argument; *plast != '\0'; plast++)
    ;
  strcpy (buf, get_name (ch));
  strcat (buf, " ");
  strcat (buf, argument);
  strcpy (tbuf, justify (buf));

  MOBtrigger = FALSE;

  for (fpl = first_player; fpl != NULL; fpl = fpl->next)
    {
      fch = fpl->ch;
      if (fch->in_room == ch->in_room &&
	  fch->position > POS_SLEEPING && can_see (fch, ch) &&
	(IS_SHIFT (fch->language, ch->race) || fch->level > MAX_LEVEL - 5 ||
	 ch->level > MAX_LEVEL - 5))
	send_to_char (tbuf, fch);
    }
  /*MOBtrigger=TRUE; */
  return;
}



/*
 * All the posing stuff.
 */
struct pose_table_type
{
  char *message[2 * MAX_CLASS];
};

const struct pose_table_type pose_table[] =
{
  {
    {
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."
    }
  },

  {
    {
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."
    }
  },

  {
    {
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."
    }
  },

  {
    {
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."
    }
  },

  {
    {
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."
    }
  },

  {
    {
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."
    }
  },

  {
    {
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
      "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."
    }
  },

  {
    {
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."
    }
  },

  {
    {
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."
    }
  },

  {
    {
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."
    }
  },

  {
    {
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."
    }
  },

  {
    {
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."
    }
  },

  {
    {
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."
    }
  },

  {
    {
      "The heavens and grass change color as you smile.",
      "The heavens and grass change color as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."
    }
  },

  {
    {
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."
    }
  },

  {
    {
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."
    }
  },

  {
    {
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great god Mota gives you a staff.",
      "The great god Mota gives $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."
    }
  }
};



void
do_pose (CHAR_DATA * ch, char *argument)
{
  int level;
  int pose;
  int class;

  if (IS_NPC (ch))
    return;

  level = UMIN (ch->level, sizeof (pose_table) / sizeof (pose_table[0]) - 1);
  pose = number_range (0, level);

  class = ch->class;
  switch (class)
    {
    case CLASS_NECROMANCER:
      class = CLASS_ILLUSIONIST;
      break;
    case CLASS_MONK:
      class = CLASS_ELEMENTALIST;
      break;
    case CLASS_ASSASSIN:
      class = CLASS_ROGUE;
      break;
    default:
      break;
    }
  act (pose_table[pose].message[2 * class + 0], ch, NULL, NULL, TO_CHAR);
  act (pose_table[pose].message[2 * class + 1], ch, NULL, NULL, TO_ROOM);

  return;
}



void
do_bug (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], *strtime;

  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  strcpy (buf, strtime);
  strcat (buf, "-");
  strcat (buf, argument);
  append_file (ch, USER_BUG_FILE_T, USER_BUG_FILE, buf);
  send_to_char ("Ok.  Thanks.\n\r", ch);
  return;
}



void
do_idea (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], *strtime;

  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  strcpy (buf, strtime);
  strcat (buf, "-");
  strcat (buf, argument);
  append_file (ch, IDEA_FILE_T, IDEA_FILE, buf);
  send_to_char ("Ok.  Thanks.\n\r", ch);
  return;
}



void
do_typo (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], *strtime;

  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  strcpy (buf, strtime);
  strcat (buf, "-");
  strcat (buf, argument);
  append_file (ch, TYPO_FILE_T, TYPO_FILE, buf);
  send_to_char ("Ok.  Thanks.\n\r", ch);
  return;
}



void do_qui (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to QUIT, you have to spell it out.\n\r", ch);
  return;
}



void do_quit (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *fch, *next, *rch;
  /* bool doAnim; */

  if (IS_NPC (ch))
    {
      SET_BIT (ch->act, ACT_WILL_DIE);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      send_to_char ("No way! You are fighting.\n\r", ch);
      return;
    }

  if (ch->position < POS_STUNNED)
    {
      send_to_char ("You're not DEAD yet.\n\r", ch);
      return;
    }

  if (IS_SET (ch->in_room->room_flags, ROOM_NO_SAVE)
      && strcasecmp (argument, "arglebargle"))
    {
      send_to_char ("You can't save here, so you had better not quit.\n\r", ch);
      return;
    }

  if (ch->pnote != NULL && strcasecmp (argument, "now") &&
      strcasecmp (argument, "arglebargle"))
    {
      send_to_char ("You must specify 'quit now'.\n\rYou have a note in progress.\n\r", ch);
      return;
    }

  if (ch->level < 1 && strcasecmp (argument, "now")
      && strcasecmp (argument, "arglebargle"))
    {
      send_to_char ("You must specify 'quit now'.\n\rQuiting below 1st level will erase your character.\n\r", ch);
      return;
    }

  /* Removed by Order...this is silly.  4/03/94
     if(!ch->in_room->area->free_save)
     {
     send_to_char("Hmmm...quitting outside of a city, that'll cost you some of your experience.\n\r",ch);
     ch->exp-=2 * exp_level(CLASS_MONSTER,ch->level);
     }
   */

  if (IS_SET (ch->in_room->room_flags, ROOM_NO_SAVE))
    {
      char_from_room (ch);
      if (ch->level < 2)
	char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
      else
	switch (which_god (ch))
	  {
	  case GOD_INIT_ORDER:
	  case GOD_ORDER:
	    char_to_room (ch, get_room_index (9799));
	    break;
	  case GOD_INIT_CHAOS:
	  case GOD_CHAOS:
	    char_to_room (ch, get_room_index (9719));
	    break;
	  default:
	    char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
	    break;
	  }
      ch->recall = ch->in_room->vnum;
      return;
    }
  if (ch->master != NULL)
    stop_follower (ch);

  /* turn off their channels so they can't but in on someone elses when
     they get back on -Dug 
     Assume they will fractionate on their own - Dave   
     ch->pcdata->channel=0;  */

  /* Don't forget about bets here - chaos 4/16/96  */
  if (ch->pcdata->bet_mode == 2)
    do_resign (ch, NULL);

  save_char_obj (ch, NORMAL_SAVE);

  for (fch = first_char; fch != NULL; fch = next)
    {
      next = fch->next;
      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM))
        stop_follower (fch);
    }

  do_return (ch, NULL);		/* fix those out of body */


  if (!IS_NPC (ch) && ch->desc != NULL &&
      ch->desc->character == ch &&
      !IS_SET (ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES) &&
      IS_SET (ch->act, PLR_TERMINAL))
    {
      /* Send notice of file */
      char buf[200];

      sprintf (buf, "%c%c", 29, 'S');
      write_to_port (ch->desc);
      write_to_buffer (ch->desc, buf, 1000000);
      write_to_port (ch->desc);
    }

  /* if (!IS_NPC(ch) && ch->pcdata->beingsaved!=NULL)
     {
     pthread_join(ch->pcdata->beingsaved, NULL);
     ch->pcdata->beingsaved = NULL;
     } */
  /* doAnim=(ch->vt100!=0); */
  if (ch->vt100 != 0 && is_desc_valid (ch))
    {
      /*char buf[5];
         if( ch->ansi==1 && ch->pcdata->term_info!=0 )
         {
         sprintf(buf,"%c%c", 14, 25 );
         write_to_descriptor(ch->desc,buf,0);
         } */
      vt100off (ch);
    }
  for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room)
    if (can_see (rch, ch) && rch != ch && rch->position > POS_SLEEPING)
      ch_printf (rch, "%s has left the game.\n\r", get_name (ch));

  sprintf (log_buf, "%s has quit.", get_name (ch));
  log_string (log_buf);

  /*
   * After extract_char the ch is no longer valid!
   */
  d = NULL;
  if (ch->desc != NULL)
    {
      d = ch->desc;
      force_help (d, "goodbye");
      /*if(doAnim)
         force_help( d,"gbanim"); */
      write_to_port (d);
      *d->inbuf = '\0';
    }

  extract_char (ch, TRUE);
  if (d != NULL)
    d->character = NULL;
  /* if(d!=NULL)
     {
     SET_BIT( d->comm_flags, COMM_FLAG_DISCONNECT);
     d->connected = -1;
     } */
  if (d != NULL)
    close_socket (d, TRUE);

  return;
}



void
do_save (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *tch;
  bool in_group;

  if (ch == NULL)
    return;
  if (IS_NPC (ch))
    return;
  if (ch->in_room == NULL)
    return;

  if (!IS_NPC (ch) && get_trust (ch) < 99)
    {
      char buf[120];
      if (ch->level < 20)
	{
	  sprintf (buf, "Your character is being saved every %d minutes.\n\r",
		   SAVE_DELAY);
	  send_to_char (buf, ch);
	}
      else
	{
	  sprintf (buf, "Save vs Magic: %d\n\r", GET_SAVING_THROW (ch));
	  send_to_char (buf, ch);
	}
    }

  if (!IS_NPC (ch) && get_trust (ch) < 99)
    return;

  if (argument[0] != '\0')
    ch = lookup_char (argument);

  if (ch == NULL)
    {
      send_to_char ("You can't save that person.\n\r", ch);
      return;
    }

  if (IS_SET (ch->in_room->room_flags, ROOM_NO_SAVE) && get_trust (ch) < 99)
    {
      send_to_char ("You can't save here.\n\r", ch);
      return;
    }

  if (ch->level < 2)
    {
      send_to_char ("You must be at least second level to save.\n\r", ch);
      return;
    }

  in_group = FALSE;
  if (ch->master == NULL)
    {
      save_char_obj (ch, NORMAL_SAVE);
    }
  else
    /* save all in same group in same room */
    for (tch = ch->in_room->first_person; tch != NULL; tch = tch->next_in_room)
      if (!IS_NPC (tch)
	  && is_same_group (tch, ch))
	{
	  /* why tell them?  It confuses them.   Order 4/9/94
	     if(tch!=ch)
	     act( "Saving $n.", tch, NULL, NULL, TO_CHAR );
	   */
	  in_group = TRUE;
	  save_char_obj (tch, NORMAL_SAVE);
	}
  if (!IS_SET (ch->in_room->room_flags, ROOM_NO_SAVE))
    {				/* only tell them if it is a room they can save in */
      if (in_group)
	act ("Saving group.", ch, NULL, NULL, TO_CHAR);
      else
	act ("Saving $n.", ch, NULL, NULL, TO_CHAR);
    }
  return;
}



void
do_follow (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Follow whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM))
    return;

  if (victim == ch)
    {
      if (ch->master == NULL)
	{
	  send_to_char ("You already follow yourself.\n\r", ch);
	  return;
	}
      stop_follower (ch);
      return;
    }

  if (!IS_NPC (ch))
    {
      if ((ch->level < (((victim->level) * 3) / 4) - 2 ||
	   ch->level > (((victim->level) * 5) / 4) + 2)
	  && ch->level < 96)
	{
	  send_to_char ("You are not of the right caliber to follow.\n\r", ch);
	  return;
	}
      if (IS_AFFECTED (victim, AFF_CHARM) && IS_NPC (victim))
	{
	  send_to_char ("You may not follow a pet.\n\r", ch);
	  return;
	}
    }

  /* No mixed groups    - Chaos 2/1/97  */
  if( which_god(ch) == GOD_CHAOS || which_god(ch) == GOD_ORDER ||
      which_god(ch) == GOD_DEMISE )
    {
      CHAR_DATA *fch;

      for (fch = first_char; fch != NULL; fch = fch->next)
	if (!IS_NPC (ch) && (fch == victim ||
			   (fch->master != NULL && fch->master == victim) ||
		   (fch->master != NULL && fch->master == victim->master) ||
			 (victim->master != NULL && victim->master == fch)))
          if( which_god(fch) == GOD_CHAOS || which_god(fch) == GOD_ORDER ||
              which_god(fch) == GOD_DEMISE )
	    if(which_god(fch)!=which_god(ch))
	    {
	      send_to_char ("You cannot follow someone in a group from a different god.\n\r", ch);
	      return;
	    }
    }


  /* poll the block list of the victim */
  if (blocking (victim, ch))
    {
      act ("$E eludes you at every step.", ch, 0, victim, TO_CHAR);
      return;
    }

  if (ch->master != NULL)
    stop_follower (ch);

  add_follower (ch, victim);
  return;
}



void
add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
  if (ch->master != NULL)
    {
      bug ("Add_follower: non-null master.", 0);
      return;
    }

  ch->master = master;
  ch->leader = NULL;

  if (can_see (master, ch))
    act ("$n now follows you.", ch, NULL, master, TO_VICT);

  act ("You now follow $N.", ch, NULL, master, TO_CHAR);

  return;
}



void
stop_follower (CHAR_DATA * ch)
{
  if (ch->master == NULL)
    {
      /* bug( "Stop_follower: null master.", 0 ); */
      return;
    }

  if (can_see (ch->master, ch))
    act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
  act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);

  if (ch->leader != NULL)
    negate_mob_hit (ch->leader->pcdata->pvnum);

  ch->master = NULL;
  ch->leader = NULL;
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
    REMOVE_BIT (ch->affected_by, AFF_CHARM);
    if (IS_NPC (ch) )
      SET_BIT (ch->act, ACT_WILL_DIE);
    }

  return;
}



void
die_follower (CHAR_DATA * ch)
{
  CHAR_DATA *fch;

  ch->position = POS_DEAD;

  if (ch->master != NULL)
    stop_follower (ch);

  ch->leader = NULL;

  for (fch = first_char; fch != NULL; fch = fch->next)
    {
      if (fch->master == ch)
	stop_follower (fch);
      if (fch->leader == ch)
	fch->leader = fch;
    }

  return;
}


void
do_shadow (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], *dir, *name;
  CHAR_DATA *victim;
  int dirNum;
  ROOM_INDEX_DATA *rm;

  if (!IS_NPC (ch) && ch->pcdata->learned[gsn_shadow] == 0)
    return;

  argument = one_argument (argument, arg);
  one_argument (argument, arg2);

  if ((dirNum = getDirNumber (arg)) == -1)
    {
      if ((dirNum = getDirNumber (arg2)) == -1)
	{
	  send_to_char ("What direction?\n\r", ch);
	  return;
	}
      else
	{
	  dir = arg2;
	  name = arg;
	}
    }
  else
    {
      dir = arg;
      name = arg2;
    }
  if (name[0] == '\0')
    {
      send_to_char ("Shadow whom?\n\r", ch);
      return;
    }
  if ((ch->in_room->exit[dirNum] == NULL) ||
      ((rm = ch->in_room->exit[dirNum]->to_room) == NULL))
    {
      send_to_char ("There is no exit there.\n\r", ch);
      return;
    }

  if ((rm->first_person == NULL) ||
      ((victim = get_char_room (rm->first_person, name)) == NULL))
    {
      send_to_char ("They aren't there.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
    {
      act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      if (ch->shadowing == NULL)
	{
	  send_to_char ("You weren't shadowing anyone.\n\r", ch);
	  return;
	}
      stop_shadow (ch);
      return;
    }

  if (number_percent () > ch->pcdata->learned[gsn_shadow])
    {
      act ("You fail to shadow $m.\n\r", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->shadowed_by != NULL || victim->shadowing != NULL)
    {
      act ("You can't seem to shadow $m.\n\r", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->master != NULL)
    stop_follower (ch);
  if (ch->shadowing != NULL)
    stop_shadow (ch);

  add_shadow (ch, victim);
  return;
}

void
add_shadow (CHAR_DATA * ch, CHAR_DATA * master)
{
  if (ch->shadowing != NULL)
    {
      bug ("Add_shadow : non-null shadow.", 0);
      return;
    }
  ch->shadowing = master;
  master->shadowed_by = ch;
  act ("You are now shadowing $N.", ch, NULL, master, TO_CHAR);
  return;
}

void
stop_shadow (CHAR_DATA * ch)
{
  if (ch->shadowing == NULL)
    {
      /* bug( "Stop_shadow: null shadow.", 0 ); */
      return;
    }
  act ("You stop shadowing $N.", ch, NULL, ch->shadowing, TO_CHAR);
  ch->shadowing->shadowed_by = NULL;
  ch->shadowing = NULL;
  return;
}

void
die_shadow (CHAR_DATA * ch)
{
  if (ch->shadowing != NULL)
    stop_shadow (ch);
}

void
do_order (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *och;
  CHAR_DATA *och_next;
  bool found;
  bool fAll;

  argument = one_argument (argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Order whom to do what?\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      send_to_char ("You feel like taking, not giving, orders.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "all"))
    {
      fAll = TRUE;
      victim = NULL;
    }
  else
    {
      fAll = FALSE;
      if ((victim = get_char_room (ch, arg)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (victim == ch)
	{
	  send_to_char ("Aye aye, right away!\n\r", ch);
	  return;
	}

      if (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch)
	{
	  send_to_char ("Do it yourself!\n\r", ch);
	  return;
	}
    }

  found = FALSE;
  for (och = ch->in_room->first_person; och != NULL; och = och_next)
    {
      och_next = och->next_in_room;

      if (IS_AFFECTED (och, AFF_CHARM)
	  && och->master == ch
	  && (fAll || och == victim))
	{
	  found = TRUE;
	  act ("$n orders you to '$t'.", ch, argument, och, TO_VICT);
	  interpret (och, argument);
	}
    }

  if (found)
    {
      if ((argument[0] == 'k') || (argument[0] == 'K'))		/* to slowdown kills */
	WAIT_STATE (ch, PULSE_VIOLENCE);
      send_to_char ("Ok.\n\r", ch);
    }
  else
    send_to_char ("You have no followers here.\n\r", ch);
  return;
}

/* used in group functions to stop a bit of cheating  - Chaos 8/8/95 */
void
negate_mob_hit (int pvnum_ld)
{
  CHAR_DATA *ch;
  for (ch = first_char; ch != NULL; ch = ch->next)
    if (IS_NPC (ch) && ch->npcdata->pvnum_last_hit_leader == pvnum_ld)
      ch->npcdata->pvnum_last_hit_leader = 1;
  return;
}

void
do_group (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int t_hp, t_current_hp, t_mana, t_current_mana, t_level, t_move;
  int t_current_move;
  CHAR_DATA *victim;
  CHAR_DATA *gch;
  CHAR_DATA *leader;

  one_argument (argument, arg);
  t_hp = 0;
  t_current_hp = 0;
  t_current_mana = 0;
  t_current_move = 0;
  t_mana = 0;
  t_level = 0;
  t_move = 0;

  leader = (ch->leader != NULL) ? ch->leader : ch;

  if (arg[0] == '\0')
    {

      sprintf (buf, "%s's group:\n\r", PERS (leader, ch));
      send_to_char (buf, ch);

      for (gch = first_char; gch != NULL; gch = gch->next)
	if (is_same_group (gch, ch) && can_see (ch, gch))
	  {
	    strcpy (buf2, get_name (gch));
	    buf2[16] = '\0';
	    if (buf[0] <= 'z' && buf[0] >= 'a')
	      buf[0] -= ('a' - 'A');
	    sprintf (buf,
	     "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %9d xp\n\r",
		     gch->level,
		     IS_NPC (gch) ? "Mob" : class_table[gch->class].who_name,
		     buf2,
		     gch->hit, gch->max_hit,
		     gch->mana, gch->max_mana,
		     gch->move, gch->max_move,
		     gch->exp);
	    t_hp += gch->max_hit;
	    t_current_hp += gch->hit;
	    t_mana += gch->max_mana;
	    t_current_mana += gch->mana;
	    t_move += gch->max_move;
	    t_current_move += gch->move;
	    t_level += gch->level;
	    send_to_char (buf, ch);
	  }
      sprintf (buf,
	       "[%-6d] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv\n\r",
	       t_level, "Totals", t_current_hp, t_hp, t_current_mana, t_mana,
	       t_current_move, t_move);
      send_to_char (buf, ch);
      return;
    }


      for (gch = first_char; gch; gch = gch->next)
        if (is_same_group (ch, gch) || (ch == gch) || 
           (gch->master!=NULL && is_same_group(gch->master, ch) ) )
          if (gch->fighting != NULL || gch->position==POS_FIGHTING )
	    {
	    send_to_char ("You cannot modify the group while a groupmember is fighting.\n\r", ch);
	    return;
	    }

  if (!strcmp (arg, "disband"))
    {
      int count = 0;

      if (ch->leader || ch->master)
	{
	  send_to_char ("You cannot disband a group if you're following someone.\n\r ", ch);
	  return;
	}


      for (gch = first_char; gch; gch = gch->next)
	{
	  if (is_same_group (ch, gch)
	      && (ch != gch))
	    {
	      gch->leader = NULL;
	      /* if (!IS_NPC(gch))
	         gch->master = NULL; */
	      count++;
	      send_to_char ("Your group is disbanded.\n\r", gch);
	    }
	}

      if (count == 0)
	send_to_char ("You have no group members to disband.\n\r", ch);
      else
	send_to_char ("You disband your group.\n\r", ch);

      return;
    }
  if (!strcmp (arg, "all"))
    {
      CHAR_DATA *rch;
      int count = 0;

      for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
	{
	  if (ch != rch
	      && (!IS_NPC (rch) || (IS_NPC (rch) && IS_AFFECTED (rch, AFF_CHARM)))
	      && can_see (ch, rch)
	      && rch->master == ch
	      && !rch->fighting
              && rch->position!=POS_FIGHTING 
	      && !ch->master
	      && !ch->leader
	      && !is_same_group (rch, ch)
	    )
	    {
	      rch->leader = ch;
	      count++;
	    }
	}

      if (count == 0)
	send_to_char ("You have no eligible group members.\n\r", ch);
      else
	{
	  act ("$n groups $s followers.", ch, NULL, NULL, TO_ROOM);
	  send_to_char ("You group your followers.\n\r", ch);
	}
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("Hmmm...where did they go?\n\r", ch);
      return;
    }

  if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
      send_to_char ("But you are following someone else!\n\r", ch);
      return;
    }

  if (victim->master != ch && ch != victim)
    {
      act ("$N isn't following you.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (is_same_group (victim, ch) && ch != victim)
    {
      if (!IS_NPC (victim->leader))
	negate_mob_hit (victim->leader->pcdata->pvnum);
      victim->leader = NULL;
      /*    if (!IS_NPC(victim))
         victim->master = NULL;
       */
      act ("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
      act ("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
      act ("You remove $N from your group.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if (victim->fighting != NULL || victim->position==POS_FIGHTING )
	    {
	    send_to_char ("You cannot group someone that is fighting.\n\r", ch);
	    return;
	    }

  if (!IS_NPC (ch))
    negate_mob_hit (ch->pcdata->pvnum);

  victim->leader = ch;
  act ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
  act ("You join $n's group.", ch, NULL, victim, TO_VICT);
  act ("$N joins your group.", ch, NULL, victim, TO_CHAR);
  return;
}


/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void 
do_split (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  int members;
  int amount;
  int share;
  int extra;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Split how much?\n\r", ch);
      return;
    }

  amount = atol (arg);

  if (amount < 0)
    {
      send_to_char ("Your group wouldn't like that.\n\r", ch);
      return;
    }

  if (amount == 0)
    {
      send_to_char ("You hand out zero coins, but no one notices.\n\r", ch);
      return;
    }

  if (ch->gold < amount)
    {
      send_to_char ("You don't have that much gold.\n\r", ch);
      return;
    }

  members = 0;
  for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
    {
      if (is_same_group (gch, ch) && !IS_NPC (gch))
	members++;
    }

  if (members < 2)
    {
      send_to_char ("Just keep it all.\n\r", ch);
      return;
    }

  share = amount / members;
  extra = amount % members;

  if (share == 0)
    {
      send_to_char ("Don't even bother, cheapskate.\n\r", ch);
      return;
    }

  ch->gold -= amount;
  ch->gold += share + extra;

  sprintf (buf,
	   "You split %d gold coins.  Your share is %d gold coins.\n\r",
	   amount, share + extra);
  send_to_char (buf, ch);

  sprintf (buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	   amount, share);

  for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
    {
      if (gch != ch && is_same_group (gch, ch) && !IS_NPC (gch))
	{
	  act (buf, ch, NULL, gch, TO_VICT);
	  gch->gold += share;
	}
    }

  return;
}



void 
do_gtell (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char jbuf[MAX_STRING_LENGTH];
  CHAR_DATA *gch, *victim;
  PLAYER_GAME *gpl;
  int position;

  if (argument[0] == '\0')
    {
      send_to_char ("Tell your group what?\n\r", ch);
      return;
    }

  if (IS_SET (ch->act, PLR_NO_TELL))
    {
      send_to_char ("Your message didn't get through!\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_SILENCE))
    {
      send_to_char ("You cannot talk.\n\r", ch);
      return;
    }

  /*
   * Note use of send_to_char, so gtell works on sleepers.
   */
  /*  Chaos 1/4/94   for languages    */
  sprintf (jbuf, "You tell the group '%s'", argument);
  if (!IS_NPC (ch) && ch->ansi == 1)
    sprintf (buf, "\033[1;%d;%dm%s\n\r", ch->pcdata->color[0][9],
	     ch->pcdata->color[1][9], justify (jbuf));
  else
    sprintf (buf, "%s\n\r", justify (jbuf));
  send_to_char (buf, ch);
  for (gpl = first_player; gpl != NULL; gpl = gpl->next)
    {
      gch = gpl->ch;
      if (is_same_group (gch, ch) && ch != gch && !IS_NPC (gch))
	{
	  victim = gch;
	  if (!can_understand (victim, ch))
	    {
	      sprintf (buf, "%s does not understand %s.\n\r", get_name (victim),
		       race_table[UNSHIFT (ch->speak)].race_name);
	      if (buf[0] <= 'z' && buf[0] >= 'a')
		buf[0] -= ('a' - 'A');
	      send_to_char (buf, ch);
	      continue;
	    }

	  MOBtrigger = FALSE;
	  position = victim->position;
	  victim->position = POS_STANDING;
	  sprintf (jbuf, "%s tells the group '%s'", get_name (ch), argument);
	  if (!IS_NPC (victim) && victim->ansi == 1)
	    sprintf (buf, "\033[1;%d;%dm%s\n\r",
		   victim->pcdata->color[0][9], victim->pcdata->color[1][9],
		     justify (jbuf));
	  else
	    sprintf (buf, "%s\n\r", justify (jbuf));
	  send_to_char (buf, victim);

	  MOBtrigger = TRUE;
	  victim->position = position;
	}
    }

  return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool 
is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
  if (ach->leader != NULL)
    ach = ach->leader;
  if (bch->leader != NULL)
    bch = bch->leader;
  return ach == bch;
}

bool 
vnum_in_group (CHAR_DATA * ach, int mobvnum)
{
  CHAR_DATA *gch;
  for (gch = ach->in_room->first_person; gch != NULL; gch = gch->next_in_room)
    {
      if (IS_NPC (gch) && gch->pIndexData->vnum == mobvnum
	  && is_same_group (ach, gch))
	{
	  return TRUE;
	}
    }
  return FALSE;
}

CHAR_DATA *
get_pvnum_index (int pvnum)
{
  PLAYER_GAME *fch, *next_fch;
  for (fch = first_player; fch != NULL; fch = next_fch)
    {
      next_fch = fch->next;
      if (fch->ch->name == NULL)
	sub_player (fch->ch);
      else if (fch->ch->pcdata->pvnum == pvnum)
	return (fch->ch);
    }
  return NULL;
}

void vt100on (CHAR_DATA * ch)
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  int n1, n2, n3;
  if (!is_desc_valid (ch))
    return;
  d = ch->desc;
  if (d->original != NULL)
    return;

  write_to_port (d);

  ch->pcdata->MrTerm_Max_Hit = 0;
  ch->pcdata->MrTerm_Max_Mana = 0;
  ch->pcdata->MrTerm_Max_Move = 0;
  ch->pcdata->MrTerm_Hit = 0;
  ch->pcdata->MrTerm_Mana = 0;
  ch->pcdata->MrTerm_Move = 0;
  ch->pcdata->MrTerm_Experience = 0;
  ch->pcdata->MrTerm_Exp_Level = 0;
  ch->pcdata->MrTerm_Gold = 0;
  ch->pcdata->MrTerm_Position = 0;
  ch->pcdata->MrTerm_Full = 0;
  ch->pcdata->MrTerm_Thirst = 0;

  if (ch->pcdata->tactical == NULL)
    {
      CREATE (ch->pcdata->tactical, TACTICAL_MAP, 1);
      clear_tactical_map (ch);
    }

  write_to_port (d);

  if (ch->vt100 != 0 && ch->ansi == 1 && IS_SET (ch->act, PLR_TERMINAL))
    {
      sprintf (buf, "%c%c%c", 14, 24, 32 + ch->vt100_type % 100);
      write_to_descriptor (d, buf, 0);
    }
/*
  I don't know what this is supposed to do but it fucks up everything with
  xterms 8/2/99 Martin
  else
  if (ch->vt100 != 0 && ch->ansi == 1 && !IS_SET (ch->act, PLR_TERMINAL))
    {
      sprintf (buf, "\033c");
      write_to_descriptor (d, buf, 0);
    }
*/

  if (!IS_SET (ch->act, PLR_PROMPT))
    {
      sprintf (buf, "\033[1;%dr\033[2J", ch->vt100_type % 100);
      send_to_char (buf, ch);
      ch->vt100 = 1;
      return;
    }

  if (ch->vt100_type / 100 % 10 == 0)
    {
      n1 = 3;
      n2 = ch->vt100_type % 100 - 1;
      n3 = 2;
    }
  else
    {
      n1 = 1 + ch->pcdata->tactical_mode % 100;
      n2 = ch->vt100_type % 100 - 2;
      n3 = ch->vt100_type % 100 - 1;
    }

  if (ch->ansi == 1)
    sprintf (buf, "\033[2J\033[%d;%dr\033[0;%d;%dm\033[%d;1H", n1, n2,
	     ch->pcdata->color[0][2], ch->pcdata->color[1][2], n3);
  else
    sprintf (buf, "\033[2J\033[%d;%dr\033[7m\033[%d;1H", n1, n2, n3);
  write_to_descriptor (d, buf, 0);
  if (ch->ansi == 1)
    sprintf (buf, "\033[%d;1H\033[m\033[2K\033[1;%d;%dm", ch->vt100_type % 100,
	     ch->pcdata->color[0][6], ch->pcdata->color[1][6]);
  else
    sprintf (buf, "\033[%d;1H\033[m\033[2K\033[1m", ch->vt100_type % 100);
  write_to_descriptor (d, buf, 0);
  ch->vt100 = 1;

  clear_tactical_map (ch);

  /* Send setup info for MrTerm stuff -  Chaos 1/7/98 */
  if (ch->desc == d && IS_SET (ch->act, PLR_TERMINAL) &&
      IS_SET (ch->pcdata->player2_bits, PLR2_MRTERM_STATS))
    {
      sprintf (buf, "%cXH3h3M3m3V3v3x6X6g6d1f1p1%c", 30, 29);
      write_to_descriptor (d, buf, 0);
    }


  ch->pcdata->old_hit = -999;
  ch->pcdata->old_max_hit = -999;
  ch->pcdata->old_mana = -999;
  ch->pcdata->old_move = -999;
  ch->pcdata->old_exp = -999;
  ch->pcdata->old_gold = -999;
  ch->pcdata->old_time = -999;
  ch->pcdata->old_dir = 0;
  d->outtop = 0;

  write_to_port (d);
}

void 
vt100off (CHAR_DATA * ch)
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];

  if (!is_desc_valid (ch))
    return;
  d = ch->desc;
  if (d == NULL)
    return;
  if (d->original != NULL)
    return;
  write_to_port (d);
  if (ch->vt100 != 0 && ch->ansi == 1 && IS_SET (ch->act, PLR_TERMINAL))
    {
      sprintf (buf, "%c%c", 14, 25);
      write_to_descriptor (d, buf, 0);
    }
  sprintf (buf, "\033[1;%dr\033[0m\033[2J", ch->vt100_type % 100);
  write_to_descriptor (d, buf, 0);
  d->outtop = 0;
  ch->vt100 = 0;
  if (ch->pcdata->tactical != NULL)
    {
      DISPOSE (ch->pcdata->tactical);
      ch->pcdata->tactical = NULL;
    }
}

void 
vt100prompt (CHAR_DATA * ch)
{
  if (!is_desc_valid (ch) || ch->pcdata == NULL)
    return;
  if (ch->vt100 == 2)
    {
      vt100prompter (ch);
      return;
    }
  ch->pcdata->tactical_update = TRUE;
  return;
}

void 
vt100prompter (CHAR_DATA * ch)
{
  DESCRIPTOR_DATA *d;
  char buf2[32];
  char buf[MAX_STRING_LENGTH];
  TACTICAL_MAP *tact;
  int leng;
  bool first;
  int cnt;

  tact = NULL;
  if (!is_desc_valid (ch))
    return;
  d = ch->desc;

  open_timer (TIMER_WRITE_PROCESS);

  first = FALSE;
  if (ch->vt100 == 2)
    {
      first = TRUE;
      vt100on (ch);
    }

  if (!IS_SET (ch->act, PLR_PROMPT))
    {
      close_timer (TIMER_WRITE_PROCESS);
      return;
    }
  if (d->connected != CON_EDITING)
    write_to_buffer (d, "\033[0K", 1000000);

  tact = get_diff_tactical (ch);
  if (tact == NULL)
    {
      close_timer (TIMER_WRITE_PROCESS);
      return;
    }
  if (ch->vt100_type / 100 % 10 == 0)
    cnt = ch->vt100_type % 100 - 1;
  else
    cnt = ch->vt100_type % 100 - 2;

  leng = str_cpy_max (buf, "\0337", MAX_STRING_LENGTH);

  if (ch->wait == 0)
    ch->pcdata->old_wait = 0;
  else
    ch->pcdata->old_wait = 1;

  ch->pcdata->tactical_update = FALSE;

  leng = str_apd_max (buf, get_tactical_string (ch, tact), leng,
		      MAX_STRING_LENGTH);
  open_timer (TIMER_WRITE_PROCESS_CMP);

/* I added this in to explicitly reset cursor position and colour instead
   of relying on highly dodgy \0337 and \0338 codes...Martin */

  if (ch->pcdata->color[0] != 0 && ch->ansi == 1)
    sprintf (buf2, "\0338\033[0;%d;%dm", ch->pcdata->color[0][0],
	     ch->pcdata->color[1][0]);
  else
    sprintf (buf2, "\0338");
  leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);

  leng = str_apd_max (buf, "\033[0K", leng, MAX_STRING_LENGTH);

  write_to_buffer (d, buf, 1000000);
  if (ch->desc == d && IS_SET (ch->act, PLR_TERMINAL) &&
      IS_SET (ch->pcdata->player2_bits, PLR2_MRTERM_STATS))
    Update_MrTerm_Stats (ch);

  close_timer (TIMER_WRITE_PROCESS_CMP);
  close_timer (TIMER_WRITE_PROCESS);


  /* Fix up the character that just starts */
  if (first && ch->level == 1)
    if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_SCHOOL)
      {
	if (ch->desc != NULL)
	  write_to_port (ch->desc);
	do_help (ch, "newchar");
	do_look (ch, "");
	if (ch->desc != NULL)
	  write_to_port (ch->desc);
      }

  return;
}

void 
do_port (CHAR_DATA * ch, char *argument)
{
  char buf1[MAX_INPUT_LENGTH];
  int size;
  if (IS_NPC (ch))
    return;
  if (ch->desc->original != NULL)
    return;
  argument = one_argument (argument, buf1);
  size = atol (buf1);
  if (size == 0)
    {
      sprintf (buf1, "Current port settings:\n\rBlock size: %d\n\r",
	       ch->desc->port_size);
      send_to_char (buf1, ch);
      return;
    }
  if (size < 16 || size > 10000)
    send_to_char ("The range of sizes are 16 to 10000", ch);
  if (size < 16)
    size = 16;
  if (size > 10000)
    size = 10000;
  ch->desc->port_size = size;
  ch->pcdata->port_size = size;
  sprintf (buf1, "Port size set to %d.\n\r", size);
  send_to_char (buf1, ch);
  return;
}


void 
do_voice (CHAR_DATA * ch, char *argument)
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;

  if (IS_NPC (ch))
    {
      if (IS_AFFECTED (ch, AFF_CHARM))
	return;
    }
  else if (ch->pcdata->learned[gsn_voice] == 0)
    return;
  if (ch->in_room == NULL)
    return;
  for (vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room)
    if (!IS_NPC (vch) && vch->level >= 99)
      {
	sprintf (buf1, "You may not voice with %s in the room.\n\r",
		 capitalize (vch->name));
	send_to_char (buf1, ch);
	return;
      }
  argument = one_argument (argument, speaker);
  if (argument[0] == '\0')
    {
      send_to_char ("What do you want them to say?", ch);
      return;
    }

  sprintf (buf1, "%s says '%s'\n\r", speaker, argument);
  sprintf (buf2, "Someone say's '%s'\n\r", argument);
  buf1[0] = UPPER (buf1[0]);

  for (vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room)
    if (vch->position >= POS_RESTING)
      {
	if (IS_NPC (ch))
	  send_to_char (buf1, vch);
	else
	  {
	    if (vch->level > ch->level &&
		number_percent () > ch->pcdata->learned[gsn_voice])
	      {			/* bad voice...send "someone" */
		sprintf (buf1, "%s%s", get_color_string (vch, COLOR_SPEACH, VT102_DIM), buf2);
		send_to_char (buf1, vch);
	      }
	    else
	      {			/* good voice...send "name" */
		sprintf (buf2, "%s%s", get_color_string (vch, COLOR_SPEACH, VT102_DIM), buf1);
		send_to_char (buf2, vch);
	      }
	  }
      }

  return;
}

void 
translate (char *in, char *out)
{
  char *pName;
  int iSyl;
  int length, cnt;

  struct syl_type
    {
      char *old;
      char *new;
    };

  static const struct syl_type syl_table[] =
  {
    {
      " ", " "
    }
    ,
    {
      "ar", "ug"
    }
    ,
    {
      "au", "ja"
    }
    ,
    {
      "le", "fi"
    }
    ,
    {
      "li", "ni"
    }
    ,
    {
      "ur", "ir"
    }
    ,
    {
      "cu", "je"
    }
    ,
    {
      "de", "ca"
    }
    ,
    {
      "en", "un"
    }
    ,
    {
      "li", "fu"
    }
    ,
    {
      "lo", "hi"
    }
    ,
    {
      "mo", "za"
    }
    ,
    {
      "ma", "do"
    }
    ,
    {
      "ne", "la"
    }
    ,
    {
      "ni", "ta"
    }
    ,
    {
      "pe", "da"
    }
    ,
    {
      "ra", "ru"
    }
    ,
    {
      "re", "ca"
    }
    ,
    {
      "so", "sa"
    }
    ,
    {
      "ec", "in"
    }
    ,
    {
      "ri", "lu"
    }
    ,
    {
      "en", "of"
    }
    ,
    {
      "a", "i"
    }
    ,
    {
      "b", "t"
    }
    ,
    {
      "c", "f"
    }
    ,
    {
      "d", "p"
    }
    ,
    {
      "e", "u"
    }
    ,
    {
      "f", "l"
    }
    ,
    {
      "g", "j"
    }
    ,
    {
      "h", "s"
    }
    ,
    {
      "i", "e"
    }
    ,
    {
      "j", "n"
    }
    ,
    {
      "k", "b"
    }
    ,
    {
      "l", "k"
    }
    ,
    {
      "m", "g"
    }
    ,
    {
      "n", "r"
    }
    ,
    {
      "o", "a"
    }
    ,
    {
      "p", "y"
    }
    ,
    {
      "q", "d"
    }
    ,
    {
      "r", "m"
    }
    ,
    {
      "s", "h"
    }
    ,
    {
      "t", "w"
    }
    ,
    {
      "u", "o"
    }
    ,
    {
      "v", "x"
    }
    ,
    {
      "w", "q"
    }
    ,
    {
      "x", "z"
    }
    ,
    {
      "y", "c"
    }
    ,
    {
      "z", "v"
    }
    ,
    {
      "?", "?"
    }
    ,
    {
      "!", "!"
    }
    ,
    {
      ".", "."
    }
    ,
    {
      ")", ")"
    }
    ,
    {
      "(", "("
    }
    ,
    {
      ":", ":"
    }
    ,
    {
      "'", "'"
    }
    ,
    {
      "-", "-"
    }
    ,
    {
      "=", "="
    }
    ,
    {
      "*", "*"
    }
    ,
    {
      "%", "%"
    }
    ,
    {
      ",", ","
    }
    ,
    {
      "<", "<"
    }
    ,
    {
      ">", ">"
    }
    ,
    {
      "", ""
    }
  };

  out[0] = '\0';
  for (pName = in; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      strcat (out, syl_table[iSyl].new);
	      break;
	    }

	}
      if (length == 0)
	length = 1;
    }
  for (cnt = 0; in[cnt] != '\0'; cnt++)
    if (in[cnt] >= 'A' && in[cnt] <= 'Z')
      out[cnt] -= ('a' - 'A');

  return;
}

void 
send_string (CHAR_DATA * ch, int id, char *val)
{
  char buf[2 * MAX_INPUT_LENGTH];

  if (!is_desc_valid (ch))
    return;

  sprintf (buf, "%c%c%s%c", 28, id, val, 29);
  write_to_buffer (ch->desc, buf, 1000000);
  return;
}


void 
send_1_byte (CHAR_DATA * ch, int id, sh_int val)
{
  char buf[MAX_INPUT_LENGTH];

  if (!is_desc_valid (ch) || val < 0 || val > 255 || IS_NPC (ch))
    return;

  sprintf (buf, "%c%c%c", 25, id, val);
  write_to_buffer (ch->desc, buf, 1000000);
  return;
}

void 
send_2_byte (CHAR_DATA * ch, int id, sh_int val)
{
  char buf[MAX_INPUT_LENGTH];
  char *v1, *v2;

  if (!is_desc_valid (ch) || IS_NPC (ch))
    return;

  v1 = (char *) &val;
  v2 = v1 + 1;

  sprintf (buf, "%c%c%c%c", 26, id, *v1, *v2);
  write_to_buffer (ch->desc, buf, 1000000);
  return;
}

void 
send_4_byte (CHAR_DATA * ch, int id, int val)
{
  char buf[MAX_INPUT_LENGTH];
  char *v1, *v2, *v3, *v4;

  if (!is_desc_valid (ch) || IS_NPC (ch))
    return;

  v1 = (char *) &val;
  v2 = v1 + 1;
  v3 = v2 + 1;
  v4 = v3 + 1;

  sprintf (buf, "%c%c%c%c%c%c", 22, id, *v1, *v2, *v3, *v4);
  write_to_buffer (ch->desc, buf, 1000000);
  return;
}

void 
send_packet (CHAR_DATA * ch, int id, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int packet_num, packet_bit, packet_vector;

  if (!is_desc_valid (ch) || IS_NPC (ch))
    return;

  packet_num = id / 8;
  packet_bit = id % 8;
  packet_vector = 1 << packet_bit;

  if (IS_SET (ch->pcdata->packet[packet_num], packet_vector))
    {
      sprintf (buf, "%c%c", 24, id);
      write_to_buffer (ch->desc, buf, 1000000);
      return;
    }


  sprintf (buf, "%c%c%s%c", 23, id, arg, 29);
  write_to_buffer (ch->desc, buf, 1000000);
  SET_BIT (ch->pcdata->packet[packet_num], packet_vector);
  return;
}

void 
speech (CHAR_DATA * ch, CHAR_DATA * vi, char *arg)
{
  char buf[2 * MAX_INPUT_LENGTH], buf2[2 * MAX_INPUT_LENGTH];

  if (!is_desc_valid (vi) || IS_NPC (vi) || !IS_SET (ch->act, PLR_TERMINAL))
    return;
  /* Max ARG length of 1200 chars */
  strcpy (buf2, arg);
  buf2[4000] = '\0';
  sprintf (buf, "%cT%c%c%s%c",
	   30,			/* Header for AUDIO hook */
	   32 + (ch->race % 8) * 2 + ch->sex,	/* Voice style */
	   !IS_NPC (ch) ?
	   (ch->pcdata->pvnum % 63) + 32 :
	   (ch->npcdata->mob_number % 63) + 32,		/* Unique value */
	   buf2,		/* Actual string */
	   29);			/* Ender for AUDIO hook */

  write_to_buffer (vi->desc, buf, 1000000);
  return;
}

bool 
pvnum_in_group (CHAR_DATA * ch, int pvnum)
{
  CHAR_DATA *fch;

  fch = get_pvnum_index (pvnum);
  if (fch == NULL)
    return FALSE;
  return (is_same_group (fch, ch));
}


void 
do_color (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  int col1, col2, col3, cnt;


  if (IS_NPC (ch) || !is_desc_valid (ch))
    return;

  if (!strcasecmp ("on", argument))
    {
      int count, cnt;
      if (ch->ansi == 1)
	{
	  send_to_char ("You already have color on.\n\r", ch);
	  return;
	}
      ch->ansi = 1;
      count = 0;
      for (cnt = 0; cnt < COLOR_MAX; cnt++)
	count += ch->pcdata->color[0][cnt];
      if (count == 0)
	reset_color (ch);
      do_refresh (ch, "");
      sprintf (buf, "\033[1;%dmAnsi color is ON!\n\r", ch->pcdata->color[0][0]);
      send_to_char (buf, ch);
      return;
    }
  if (ch->ansi == 0)
    {
      send_to_char ("You have your color turned off.\n\r", ch);
      return;
    }
  if (!strcasecmp ("reset", argument))
    {				/* Chaos's favorite colors */
      reset_color (ch);
      do_refresh (ch, "");
      send_to_char ("Color reset to defaults.\n\r", ch);
      return;
    }
  if (!strcasecmp ("off", argument))
    {
      ch->ansi = 0;
      do_refresh (ch, "");
      send_to_char ("Ansi color is OFF.\n\r", ch);
      return;
    }

  if (!strcasecmp ("", argument))
    {
      send_to_char (
		     "       0 - Black        1 - Red         2 - Green       3 - Yellow", ch);
      send_to_char (
		     "       4 - Blue         5 - Purple      6 - Cyan        7 - Grey", ch);


      for (cnt = 0; cnt < COLOR_MAX; cnt++)
	{
	  char buft[100];
	  switch (cnt)
	    {
	    case 0:
	      strcpy (buft, " 0 - Standard Text                ");
	      break;
	    case 1:
	      strcpy (buft, " 1 - Top Status Bar               ");
	      break;
	    case 2:
	      strcpy (buft, " 2 - Bottom Status Bar            ");
	      break;
	    case 3:
	      strcpy (buft, " 3 - Score, Status, and Affects   ");
	      break;
	    case 4:
	      strcpy (buft, " 4 - You are hit                  ");
	      break;
	    case 5:
	      strcpy (buft, " 5 - You hit others               ");
	      break;
	    case 6:
	      strcpy (buft, " 6 - Prompt line                  ");
	      break;
	    case 7:
	      strcpy (buft, " 7 - Exits                        ");
	      break;
	    case 8:
	      strcpy (buft, " 8 - Party member is hit          ");
	      break;
	    case 9:
	      strcpy (buft, " 9 - Speech commands              ");
	      break;
	    case 10:
	      strcpy (buft, "10 - Objects                      ");
	      break;
	    case 11:
	      strcpy (buft, "11 - Living things                ");
	      break;
	    case 12:
	      strcpy (buft, "12 - General Tactical Map         ");
	      break;
	    case 13:
	      strcpy (buft, "13 - Party members in Tactical    ");
	      break;
	    case 14:
	      strcpy (buft, "14 - Enemies in Tactical          ");
	      break;
	    case 15:
	      strcpy (buft, "15 - Neutrals in Tactical         ");
	      break;
	    }

	  sprintf (buf, "%s%s foreground %2d, background %2d.\n\r",
		   get_color_string (ch, cnt, VT102_DIM), buft,
	    ch->pcdata->color[0][cnt] - 30, ch->pcdata->color[1][cnt] - 40);
	  send_to_char (buf, ch);
	}
      send_to_char ("Syntax:  COLOR <field number> <forground number> <background number>\n\r", ch);
      return;
    }
  argument = one_argument (argument, buf);
  argument = one_argument (argument, buf2);
  if (buf[0] < '0' || buf[0] > '9')
    {
      send_to_char ("Try using HELP COLOR.\n\r", ch);
      return;
    }
  col1 = atol (buf) % COLOR_MAX;
  col2 = atol (buf2) % 8;
  col3 = atol (argument) % 8;
  ch->pcdata->color[0][col1] = col2 + 30;
  ch->pcdata->color[1][col1] = col3 + 40;
  do_refresh (ch, "");
  sprintf (buf, "\033[1;%d;%dmColor field #%d set to color forground %d, background %d.\n\r",
  ch->pcdata->color[0][col1], ch->pcdata->color[1][col1], col1, col2, col3);
  send_to_char (buf, ch);
  return;
}

void 
reset_color (CHAR_DATA * ch)
{

  if (IS_NPC (ch))
    return;
  if (ch->ansi > 0)
    {
      ch->pcdata->color[0][0] = 36;
      ch->pcdata->color[1][0] = 40;
      ch->pcdata->color[0][1] = 37;
      ch->pcdata->color[1][1] = 41;
      ch->pcdata->color[0][2] = 33;
      ch->pcdata->color[1][2] = 44;
      ch->pcdata->color[0][3] = 34;
      ch->pcdata->color[1][3] = 40;
      ch->pcdata->color[0][4] = 31;
      ch->pcdata->color[1][4] = 40;
      ch->pcdata->color[0][5] = 32;
      ch->pcdata->color[1][5] = 40;
      ch->pcdata->color[0][6] = 37;
      ch->pcdata->color[1][6] = 40;
      ch->pcdata->color[0][7] = 35;
      ch->pcdata->color[1][7] = 40;
      ch->pcdata->color[0][8] = 34;
      ch->pcdata->color[1][8] = 40;
      ch->pcdata->color[0][9] = 33;
      ch->pcdata->color[1][9] = 40;
      ch->pcdata->color[0][10] = 32;
      ch->pcdata->color[1][10] = 40;
      ch->pcdata->color[0][11] = 35;
      ch->pcdata->color[1][11] = 40;
      ch->pcdata->color[0][12] = 37;
      ch->pcdata->color[1][12] = 44;
      ch->pcdata->color[0][13] = 33;
      ch->pcdata->color[1][13] = 44;
      ch->pcdata->color[0][14] = 31;
      ch->pcdata->color[1][14] = 44;
      ch->pcdata->color[0][15] = 35;
      ch->pcdata->color[1][15] = 44;
    }
  else
    {
      ch->pcdata->color[0][0] = 30;
      ch->pcdata->color[1][0] = 40;
      ch->pcdata->color[0][1] = 31;
      ch->pcdata->color[1][1] = 41;
      ch->pcdata->color[0][2] = 31;
      ch->pcdata->color[1][2] = 41;
      ch->pcdata->color[0][3] = 30;
      ch->pcdata->color[1][3] = 40;
      ch->pcdata->color[0][4] = 30;
      ch->pcdata->color[1][4] = 40;
      ch->pcdata->color[0][5] = 30;
      ch->pcdata->color[1][5] = 40;
      ch->pcdata->color[0][6] = 30;
      ch->pcdata->color[1][6] = 40;
      ch->pcdata->color[0][7] = 30;
      ch->pcdata->color[1][7] = 40;
      ch->pcdata->color[0][8] = 30;
      ch->pcdata->color[1][8] = 40;
      ch->pcdata->color[0][9] = 30;
      ch->pcdata->color[1][9] = 40;
      ch->pcdata->color[0][10] = 30;
      ch->pcdata->color[1][10] = 40;
      ch->pcdata->color[0][11] = 30;
      ch->pcdata->color[1][11] = 40;
      ch->pcdata->color[0][12] = 31;
      ch->pcdata->color[1][12] = 41;
      ch->pcdata->color[0][13] = 31;
      ch->pcdata->color[1][13] = 41;
      ch->pcdata->color[0][14] = 31;
      ch->pcdata->color[1][14] = 41;
      ch->pcdata->color[0][15] = 31;
      ch->pcdata->color[1][15] = 41;
    }
  return;
}

void 
do_repeat (CHAR_DATA * ch, char *argument)
{
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  int amount, cnt;
  char bbuf[MAX_INPUT_LENGTH];
  char *pto, *pti;

  if (IS_NPC (ch))
    return;
  if (ch->desc == NULL)
    return;
  if (ch->desc->original != NULL || ch->desc->character != ch)
    return;
  argument = one_argument (argument, buf1);

  amount = atol (buf1);

  if (amount < 2 || amount > 40)
    {
      send_to_char ("Syntax:  REPEAT <amount> <command string>\n\r", ch);
      send_to_char ("Amount range is 2 to 40, or whatever the input line can hold.\n\r", ch);
      return;
    }

  str_cpy_max (buf2, argument, MAX_INPUT_LENGTH - 6);
  strcat (buf2, "\r");
  for (cnt = 0, pto = bbuf, pti = ch->desc->inbuf; cnt < ch->desc->intop;
       pto++, pti++, cnt++)
    *pto = *pti;
  *pto = '\0';

  ch->desc->intop = 0;
  *ch->desc->inbuf = '\0';

  for (cnt = 0; cnt < amount && ch->desc->intop < MAX_INPUT_LENGTH - 6; cnt++)
    ch->desc->intop = str_apd_max (ch->desc->inbuf, buf2, ch->desc->intop,
				   MAX_INPUT_LENGTH - 5);
  ch->desc->intop = str_apd_max (ch->desc->inbuf, bbuf, ch->desc->intop,
				 MAX_INPUT_LENGTH - 6);
  return;
}

void 
do_request (CHAR_DATA * ch, char *argument)
{

  if (!is_desc_valid (ch) || IS_NPC (ch))
    return;

  if (argument[0] == 'v' || argument[0] == 'V')
    {
      if (argument[1] == '+' && IS_SET (ch->pcdata->request, REQUEST_VT_SAVE_ON))
	{
	  if (ch->vt100 == 1)
	    return;
	  REMOVE_BIT (ch->pcdata->request, REQUEST_VT_SAVE_ON);
	  vt100on (ch);
	  return;
	}
      if (argument[1] == '-' && !IS_SET (ch->pcdata->request, REQUEST_VT_SAVE_ON))
	{
	  if (ch->vt100 == 0)
	    return;
	  SET_BIT (ch->pcdata->request, REQUEST_VT_SAVE_ON);
	  vt100off (ch);
	  return;
	}
      return;
    }

  if (argument[0] == 'l' || argument[0] == 'L')
    {
      if (argument[1] == '+' && IS_SET (ch->pcdata->request, REQUEST_PROMPT_SAVE_ON))
	{
	  if (IS_SET (ch->act, PLR_PROMPT))
	    return;
	  REMOVE_BIT (ch->pcdata->request, REQUEST_PROMPT_SAVE_ON);
	  SET_BIT (ch->act, PLR_PROMPT);
	  do_refresh (ch, "");
	  return;
	}
      if (argument[1] == '-' && !IS_SET (ch->pcdata->request, REQUEST_PROMPT_SAVE_ON))
	{
	  if (!IS_SET (ch->act, PLR_PROMPT))
	    return;
	  SET_BIT (ch->pcdata->request, REQUEST_PROMPT_SAVE_ON);
	  REMOVE_BIT (ch->act, PLR_PROMPT);
	  do_refresh (ch, "");
	  return;
	}
      return;
    }

  return;
}

void 
do_block (CHAR_DATA * ch, char *argument)
{
  char buf1[MAX_INPUT_LENGTH];

  if (!is_desc_valid (ch) || IS_NPC (ch))
    return;

  argument = one_argument (argument, buf1);
  if ((buf1 == NULL) || buf1[0] == '\0')
    {				/* send them the current block list */
      if (ch->pcdata->block_list == NULL)
	ch->pcdata->block_list = STRALLOC ("");
      sprintf (buf1, "Your block-list is:\n\r > %s\n\r", ch->pcdata->block_list);
      send_to_char (buf1, ch);
      return;
    }
  if (!strcasecmp ("clear", buf1))
    {
      STRFREE (ch->pcdata->block_list);
      ch->pcdata->block_list = NULL;
      send_to_char ("Your block-list is now empty.\n\r", ch);
      return;
    }
  if (!strcasecmp ("del", buf1))
    {
      if (ch->pcdata->block_list != NULL)
	{
	  char *block_list, block_names[MAX_INPUT_LENGTH], name_to_delete[MAX_INPUT_LENGTH],
	    cur_name[MAX_INPUT_LENGTH], new_block[MAX_INPUT_LENGTH];

	  new_block[0] = '\0';
	  argument = one_argument (argument, name_to_delete);
	  strcpy (block_names, ch->pcdata->block_list);
	  block_list = &(block_names[0]);
	  for (block_list = one_argument (block_list, cur_name);
	       cur_name == NULL || cur_name[0] != '\0';
	       block_list = one_argument (block_list, cur_name))
	    if (!strcasecmp (cur_name, name_to_delete))
	      {			/* now delete the old string and create the new one */
		strcat (new_block, block_list);
		STRALLOC (ch->pcdata->block_list);
		ch->pcdata->block_list = STRALLOC (new_block);
		sprintf (buf1, "Deleted '%s' from your block-list.\n\r", name_to_delete);
		send_to_char (buf1, ch);
		sprintf (buf1, "Your block-list is now:\n\r > %s\n\r",
			 ch->pcdata->block_list);
		send_to_char (buf1, ch);
		return;
	      }
	    else
	      {			/* add the name to the new string */
		strcat (cur_name, " ");
		strcat (new_block, cur_name);
	      }
	  sprintf (buf1, "I was unabled to find '%s' in your block-list.\n\r",
		   name_to_delete);
	}
      else
	sprintf (buf1, "You have NO block-list!\n\r");
      send_to_char (buf1, ch);
      return;
    }
  if (!strcasecmp ("add", buf1))
    {
      if (ch->pcdata->block_list != NULL)
	{
	  if (strlen (ch->pcdata->block_list) > 128)
	    {
	      send_to_char ("Your block-list is too long to add a new name.\n\r", ch);
	      return;
	    }
	  strcpy (buf1, ch->pcdata->block_list);
	  STRFREE (ch->pcdata->block_list);
	  if (buf1[0] != '\0')
	    strcat (buf1, " ");
	}
      else
	buf1[0] = '\0';
      strcat (buf1, argument);
      ch->pcdata->block_list = STRALLOC (buf1);
      sprintf (buf1, "Your block-list is now:\n\r > %s\n\r",
	       ch->pcdata->block_list);
      send_to_char (buf1, ch);
      return;
    }
  send_to_char ("Incorrect blocking format.\n\rUse: block [<add/clear/del>] [namelist]\n\r", ch);
  return;
}

void 
do_death (CHAR_DATA * ch, char *arg)
{
  if (IS_NPC (ch))
    return;
  switch (which_god (ch))
    {
    case GOD_INIT_ORDER:
    case GOD_ORDER:
      if (!IS_SET (ch->in_room->room_flags, ROOM_GOD_LO)
	  && IS_SET (ch->in_room->room_flags, ROOM_GOD_HI))
	{
	  ch->pcdata->death_room = ch->in_room->vnum;
	  send_to_char ("Death room set.\n\r", ch);
	  return;
	}
      break;
    case GOD_INIT_CHAOS:
    case GOD_CHAOS:
      if (!IS_SET (ch->in_room->room_flags, ROOM_GOD_HI)
	  && IS_SET (ch->in_room->room_flags, ROOM_GOD_LO))
	{
	  ch->pcdata->death_room = ch->in_room->vnum;
	  send_to_char ("Death room set.\n\r", ch);
	  return;
	}
      break;
    default:
      if (IS_SET (ch->in_room->room_flags, ROOM_GOD_LO)
	  && IS_SET (ch->in_room->room_flags, ROOM_GOD_HI))
	{
	  ch->pcdata->death_room = ch->in_room->vnum;
	  send_to_char ("Death room set.\n\r", ch);
	  return;
	}
      break;
    }
  send_to_char ("You may not set your death room here.\n\r", ch);
  send_to_char ("Your death room is currently set at:\n\r", ch);
  send_to_char (room_index[ch->pcdata->death_room]->name, ch);
  return;
}

char *
justify (char *inp)
{
  char *pti, *pto, *last_space, *fp, *tp;
  int cnt;

  pto = (char *) justified_buf;
  pti = inp;
  last_space = pto;
  cnt = 0;

  while (TRUE)
    {
      *pto = *pti;
      if (*pto == '\0')
	return ((char *) justified_buf);
      if (isspace ((int) *pto))
	last_space = pto;
      if ((*pto == '\n') || (*pto == '\r'))
	cnt = 0;
      pto++;
      pti++;
      cnt++;
      if (cnt == 80)
	{
	  if (pto - last_space > 20)
	    {
	      *(pto - 1) = '\n';
	      *pto = '\r';
	      cnt = 0;
	    }
	  else
	    {
	      *last_space = '\n';
	      for (fp = pto - 1, tp = pto; fp > last_space; fp--, tp--)
		*tp = *fp;
	      *(last_space + 1) = '\r';
	      cnt = pto - last_space - 1;
	    }
	  pto++;
	}
    }
}

  /*  Can Victim understand Char  */
bool 
can_understand (CHAR_DATA * victim, CHAR_DATA * ch)
{
  if (ch->in_room == get_room_index (ROOM_VNUM_MEETING))
    if (victim->in_room == get_room_index (ROOM_VNUM_MEETING))
      return (TRUE);
  if (!IS_SET (victim->language, ch->speak))
    if (!IS_AFFECTED (victim, AFF_UNDERSTAND) && !IS_AFFECTED (ch, AFF_TONGUES))
      if (ch->level < 90)
	return (FALSE);
  return (TRUE);
}


void 
do_tactical (CHAR_DATA * ch, char *arg)
{
  int val;
  int max;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC (ch))
    return;

  if (!strcasecmp (arg, "dump"))
    {
      char buf[MAX_STRING_LENGTH];
      char tbuf[40];
      int cnt, lcnt;

      buf[0] = '\0';
      for (lcnt = 0; lcnt < (ch->pcdata->tactical_mode % 100 + 1); lcnt++)
	for (cnt = 0; cnt < 80; cnt++)
	  {
	    sprintf (tbuf, "\033[%d;%d;%dm%c",
		     ch->pcdata->tactical->map[lcnt][cnt] > 128 ? 1 : 0,
		     (ch->pcdata->tactical->color[lcnt][cnt] % 8) + 30,
		     (ch->pcdata->tactical->color[lcnt][cnt] / 8) + 40,
		     ch->pcdata->tactical->map[lcnt][cnt] % 128);
	    strcat (buf, tbuf);
	  }
      send_to_char (buf, ch);
      return;
    }

  if (!strcasecmp (arg, "top"))
    {
      if ((ch->pcdata->tactical_mode / 1000) % 10 == 0)
	{
	  send_to_char ("Your tactical is on top.\n\r", ch);
	  return;
	}
      ch->pcdata->tactical_mode -= 1000;
      send_to_char ("Your tactical is set to the top.\n\r", ch);
      return;
    }

  if (!strcasecmp (arg, "bottom"))
    {
      if ((ch->pcdata->tactical_mode / 1000) % 10 == 1)
	{
	  send_to_char ("Your tactical is on bottom.\n\r", ch);
	  return;
	}
      ch->pcdata->tactical_mode += 1000;
      send_to_char ("Your tactical is set to the bottom.\n\r", ch);
      return;
    }

  if (arg[0] == 'n' || arg[0] == 'N')
    {
      if ((ch->pcdata->tactical_mode / 100) % 10 == 0)
	{
	  send_to_char ("You turn the tactical numbers off.\n\r", ch);
	  ch->pcdata->tactical_mode += 100;
	}
      else
	{
	  send_to_char ("You turn the tactical numbers on.\n\r", ch);
	  ch->pcdata->tactical_mode -= 100;
	}
      return;
    }


  if (arg[0] == 'i' || arg[0] == 'I')
    {
      char bufs[10];

      arg = one_argument (arg, buf);

      if (arg[0] <= ' ' || arg[0] > 'z' || arg[1] <= ' ' || arg[1] > 'z')
	{
	  send_to_char ("You must specify a two character string for an index.\n\r",
			ch);
	  return;
	}

      if (arg[0] == '~')
	arg[0] = '-';
      if (arg[1] == '~')
	arg[1] = '-';

      bufs[0] = arg[0];
      bufs[1] = arg[1];
      bufs[2] = '\0';

      STRFREE (ch->pcdata->tactical_index);
      ch->pcdata->tactical_index = str_dup (bufs);

      send_to_char ("Index set.\n\r", ch);

      return;
    }


  if (arg[0] == 'c' || arg[0] == 'C')
    {
      char bufs[100];
      int cnt, col;

      arg = one_argument (arg, buf);

      if (arg[0] == '\0')
	{
	  send_to_char ("The following is the tactical compass colors and their Sector Types.\n\r", ch);
	  for (cnt = 0; cnt < SECT_MAX; cnt++)
	    {
	      col = get_sector_color (ch, cnt);
	      switch (cnt)
		{
		case 0:
		  strcpy (bufs, "Inside        ");
		  break;
		case 1:
		  strcpy (bufs, "City          ");
		  break;
		case 2:
		  strcpy (bufs, "Field         ");
		  break;
		case 3:
		  strcpy (bufs, "Forest        ");
		  break;
		case 4:
		  strcpy (bufs, "Hills         ");
		  break;
		case 5:
		  strcpy (bufs, "Mountain      ");
		  break;
		case 6:
		  strcpy (bufs, "Shallow Water ");
		  break;
		case 7:
		  strcpy (bufs, "Deep Water    ");
		  break;
		case 9:
		  strcpy (bufs, "Air           ");
		  break;
		case 10:
		  strcpy (bufs, "Desert        ");
		  break;
		case 11:
		  strcpy (bufs, "Lava          ");
		  break;
		case 12:
		  strcpy (bufs, "Inn           ");
		  break;
		case 13:
		  strcpy (bufs, "Ethereal Plane");
		  break;
		case 14:
		  strcpy (bufs, "Astral Plane  ");
		  break;
		case 15:
		  strcpy (bufs, "Under Water   ");
		  break;
		case 16:
		  strcpy (bufs, "Under Ground  ");
		  break;
		case 17:
		  strcpy (bufs, "Deep Earth    ");
		  break;
		default:
		  strcpy (bufs, "Unknown       ");
		  break;
		}
	      sprintf (buf, "%s%s\n\r", vt_command_3 (ch, 0, col % 8 + 30,
						  col / 8 + 40, 'm'), bufs);
	      send_to_char (buf, ch);
	    }
	  return;
	}

      val = atol (arg);
      if (val < 4)
	{
	  send_to_char ("The tactical compass is turned off.\n\r", ch);
	  ch->pcdata->compass_width = 0;
	  return;
	}
      if (val > 16)
	{
	  send_to_char ("The tactical compass size must be from 4 to 16, or 0 to turn off.\n\r", ch);
	  return;
	}

      ch->pcdata->compass_width = val;
      send_to_char ("The tactical compass size is set.\n\r", ch);
      return;
    }


  max = MAX_TACTICAL - 1;
  if ((ch->vt100_type % 100) - 10 < MAX_TACTICAL - 1)
    max = (ch->vt100_type % 100) - 10;
  if (max < 3)
    max = 1;
  val = atol (arg);
  if (val < 1 || val > max)
    {
      sprintf (buf, "The tactical size must be from 1 to %d.\n\r", max);
      send_to_char (buf, ch);
      return;
    }

  ch->pcdata->tactical_mode = ch->pcdata->tactical_mode -
    ch->pcdata->tactical_mode % 100 + val;

  do_refresh (ch, "");

  return;
}


void 
clear_tactical_map (CHAR_DATA * ch)
{
  register int cnt, lcnt;
  register unsigned char *cm, *cc;

  /* Assume everything is linear in memory by bytes - Chaos 4/1/96  */
  cm = (unsigned char *) ch->pcdata->tactical->map;
  cc = (unsigned char *) ch->pcdata->tactical->color;

  for (lcnt = 0; lcnt < (ch->pcdata->tactical_mode % 100 + 1); lcnt++)
    for (cnt = 0; cnt < 80; cnt++, cm++, cc++)
      {
	/*ch->pcdata->tactical->map[lcnt][cnt]='~';
	   ch->pcdata->tactical->color[lcnt][cnt]=0; */
	*cm = '~';
	*cc = 0;
      }
  /* A basic invalid character so replacement is guaranteed */

  return;
}

 /* Chaos -   3/1/95  */
char *
get_tactical_string (CHAR_DATA * ch, TACTICAL_MAP * tact)
{
  char *str;
  char tbuf[50];
  int cnt, lcnt;
  bool old_bold;		/* Last bold change */
  int old_for, old_bak;		/* last color selected */
  int new_for, new_bak;
  int prev_h, prev_v;		/* last known change in text */
  int ccnt;
  int stbar;
  bool new_bold;
  bool any_diff;
  int leng;
  unsigned char *tm, *tc, *cm, *cc;	/* Pointers to tacticals */

  old_bold = FALSE;
  any_diff = FALSE;

  if (IS_NPC (ch) || ch->pcdata->tactical == NULL)
    return (NULL);

  open_timer (TIMER_WRITE_PROCESS_STR);


  prev_h = -10;
  prev_v = -10;
  old_for = ch->pcdata->color[0][COLOR_TEXT] - 30;
  old_bak = ch->pcdata->color[1][COLOR_TEXT] - 40;

  str = (char *) get_tactical_string_string;	/* Let's shorten this up */

  tbuf[1] = '\0';
  *str = '\0';
  leng = 0;

  if (ch->vt100_type / 100 % 10 == 0)
    stbar = 2;
  else
    stbar = ch->vt100_type % 100 - 1;

  /* Assume everything is linear in memory by bytes - Chaos 4/1/96  */
  tm = (unsigned char *) tact->map;
  tc = (unsigned char *) tact->color;
  cm = (unsigned char *) ch->pcdata->tactical->map;
  cc = (unsigned char *) ch->pcdata->tactical->color;

  for (lcnt = 0; lcnt < (ch->pcdata->tactical_mode % 100 + 1); lcnt++)
    for (cnt = 0; cnt < 80; cnt++, tm++, tc++, cm++, cc++)
      if (tact->map[lcnt][cnt] != ch->pcdata->tactical->map[lcnt][cnt] ||
	  (tact->color[lcnt][cnt] != ch->pcdata->tactical->color[lcnt][cnt]))
	if (*tm != *cm || *tc != *cc)
	  {
	    /* if( tact->map[lcnt][cnt] >= 128 ) */
	    if (*tm >= 128)
	      new_bold = TRUE;
	    else
	      new_bold = FALSE;

	    /* tbuf[0] = tact->map[lcnt][cnt] % 128; */
	    tbuf[0] = *tm % 128;

	    if (ch->vt100 > 0 && ch->ansi == 0)
	      {
		new_for = 0;
		new_bak = 1;
	      }
	    else
	      {
		/* new_for = tact->color[lcnt][cnt] % 8;
		   new_bak = (tact->color[lcnt][cnt] / 8) % 8; */
		new_for = *tc % 8;
		new_bak = (*tc / 8) % 8;
	      }

	    /* Get color shifting */
	    leng = str_apd_max (str, get_color_diff (ch, old_for, old_bak, old_bold,
		      new_for, new_bak, new_bold), leng, MAX_STRING_LENGTH);

	    if (lcnt == ch->pcdata->tactical_mode % 100)
	      ccnt = stbar - 1;
	    else
	      ccnt = lcnt;

	    if (prev_v == ccnt + 1 && prev_h == cnt)
	      leng = str_apd_max (str, tbuf, leng, MAX_STRING_LENGTH);
	    else
	      {

		/* Check for slow VT102 processing */
		if ((ch->vt100_type / 10000) % 10 == 1)
		  {
		    /* sprintf( tbuf, "%s%c", vt_command_2(ch,ccnt+1, cnt+1,'H'),
		       tact->map[lcnt][cnt] % 128 ); */
		    sprintf (tbuf, "%s%c", vt_command_2 (ch, ccnt + 1, cnt + 1, 'H'),
			     *tm % 128);
		  }
		else
		  {

		    if (cnt == 0)	/* Left column optimize */
		      sprintf (tbuf, "%s%c", vt_command_1 (ch, ccnt + 1, 'H'),
			       *tm % 128);
		    /* tact->map[lcnt][cnt] % 128 ); */
		    else if (ccnt + 1 == prev_v)	/* Same row optimize */
		      sprintf (tbuf, "%s%c", vt_command_1 (ch, cnt - prev_h, 'C'),
			       *tm % 128);
		    /* tact->map[lcnt][cnt] % 128 ); */
		    else if (ccnt + 1 - 1 == prev_v && cnt == prev_h)	/* CR optimization */
		      sprintf (tbuf, "\013%c", *tm % 128);
		    /* sprintf( tbuf, "\013%c",tact->map[lcnt][cnt] % 128 ); */
		    /* R optimization */
		    /* L optimization */

		    else	/* Direct cursor positioning */
		      sprintf (tbuf, "%s%c", vt_command_2 (ch, ccnt + 1, cnt + 1, 'H'),
			       *tm % 128);
		    /* tact->map[lcnt][cnt] % 128 ); */

		  }

		leng = str_apd_max (str, tbuf, leng, MAX_STRING_LENGTH);
		tbuf[1] = '\0';
	      }

	    prev_v = ccnt + 1;
	    prev_h = cnt + 1;
	    old_for = new_for;
	    old_bak = new_bak;
	    old_bold = new_bold;
	  }

  close_timer (TIMER_WRITE_PROCESS_STR);
  open_timer (TIMER_WRITE_PROCESS_UPD);

  /* Update the new map for player */
  tm = (unsigned char *) tact->map;
  tc = (unsigned char *) tact->color;
  cm = (unsigned char *) ch->pcdata->tactical->map;
  cc = (unsigned char *) ch->pcdata->tactical->color;
  for (lcnt = 0; lcnt < (ch->pcdata->tactical_mode % 100 + 1); lcnt++)
    for (cnt = 0; cnt < 80; cnt++, tm++, tc++, cm++, cc++)
      {
	*cm = *tm;
	*cc = *tc;
	/* ch->pcdata->tactical->map[lcnt][cnt]=tact->map[lcnt][cnt];
	   ch->pcdata->tactical->color[lcnt][cnt]=tact->color[lcnt][cnt]; */
      }


  close_timer (TIMER_WRITE_PROCESS_UPD);

  return (str);
}

TACTICAL_MAP *
get_tactical_map (CHAR_DATA * ch)
{
  TACTICAL_MAP *tact;
  register int val;
  int cnt, lcnt;
  register char *pti, *pto;
  register char *ptoc;
  AFFECT_DATA *paf;
  PC_DATA *pcd;			/* Shortcutt one reference */
  int size_v;
  int cuc;
  char buf[MAX_STRING_LENGTH];
  char buf3[MAX_INPUT_LENGTH];
  char buf4[MAX_INPUT_LENGTH];
  char *ptb;
  unsigned char color1, color2, color3;
  int door;
  int tim, hr;
  struct tm clk;
  unsigned char *tm, *tc;
  EXIT_DATA *pexit;
  bool found;
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *fch;
  int col, row;
  int len;
  char cval;
  int bld;
  int colors;
  int max_width;
  int offset;
  int vo;
  int hour;
  int cw;

  tact = cont_tact;
  pcd = ch->pcdata;

  clk = *localtime (&current_time);
  tim = 100 * clk.tm_hour + clk.tm_min;

  tm = (unsigned char *) tact->map;
  tc = (unsigned char *) tact->color;
  color1 = pcd->color[0][COLOR_TOP_STAT] - 30 +
    (pcd->color[1][COLOR_TOP_STAT] - 40) * 8;
  color2 = pcd->color[0][COLOR_TACTICAL] - 30 +
    (pcd->color[1][COLOR_TACTICAL] - 40) * 8;
  color3 = pcd->color[0][COLOR_BOT_STAT] - 30 +
    (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  val = pcd->tactical_mode;
  size_v = val % 100 + 1;
  for (lcnt = 0; lcnt < size_v; lcnt++)
    for (cnt = 0; cnt < 80; cnt++, tm++, tc++)
      {
	*tm = ' ';
	if ((lcnt == 0 && (val / 1000) % 10 == 0) ||
	    (lcnt == val % 100 - 1 &&
	     (val / 1000) % 10 != 0))
	  *tc = color1;
	else if (lcnt < val % 100)
	  *tc = color2;
	else
	  *tc = color3;
      }

  /* Let's use thses as a index pointer for speed */
  tm = (unsigned char *) tact->map;
  tc = (unsigned char *) tact->color;

  if ((val / 1000) % 10 == 0)
    pto = tm;
  else
    pto = tm + 80 * (size_v - 2);

  for (cnt = 0, pti = ch->name; *pti != '\0'; pti++, pto++, cnt++)
    *pto = *pti;
  for (pti = pcd->title; *pti != '\0' && cnt < 73; pti++, pto++, cnt++)
    *pto = *pti;
  for (; cnt < 80; cnt++, pto++)
    *pto = ' ';

  pto--;
  *pto = ch->level % 10 + 128 + '0';
  if (ch->level >= 10)
    {
      pto--;
      *pto = ch->level / 10 + 128 + '0';
    }
  pto--;
  *pto = ':';
  pto--;
  *pto = 'v';
  pto--;
  *pto = 'e';
  pto--;
  *pto = 'L';
  pto--;
  *pto = ' ';

  if (ch->level > 17)
    {
      val = abs (ch->alignment);
      if (val < 10)
	{
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = val % 10 + 128 + '0';
	}
      else if (val < 100)
	{
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = val % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 10) % 10 + 128 + '0';
	}
      else if (val < 1000)
	{
	  pto--;
	  *pto = ' ';
	  pto--;
	  *pto = val % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 10) % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 100) % 10 + 128 + '0';
	}
      else
	{
	  pto--;
	  *pto = val % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 10) % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 100) % 10 + 128 + '0';
	  pto--;
	  *pto = (val / 1000) % 10 + 128 + '0';
	}
      if (ch->alignment < 0)
	{
	  pto--;
	  *pto = '-' + 128;
	}
      else
	{
	  pto--;
	  *pto = ' ';
	}
      pto--;
      *pto = ':';
      pto--;
      *pto = 'l';
      pto--;
      *pto = 'A';
      pto--;
      *pto = ' ';
    }

  val = ch->speed;
  if (val == 0)
    {
      pto--;
      *pto = 'W';
    }
  else if (val == 1)
    {
      pto--;
      *pto = 'N';
    }
  else if (val == 2)
    {
      pto--;
      *pto = 'J';
    }
  else if (val == 3)
    {
      pto--;
      *pto = 'R';
    }
  else if (val == 4)
    {
      pto--;
      *pto = 'H';
    }
  pto--;
  *pto = ' ';

  if (IS_SET (ch->act, PLR_HOLYLIGHT))
    {
      pto--;
      *pto = 'L' + 128;
      pto--;
      *pto = 'H' + 128;
    }
  if (IS_SET (ch->act, PLR_WIZINVIS))
    {
      pto--;
      *pto = 'I' + 128;
      pto--;
      *pto = 'W' + 128;
    }
  if (ch->first_affect != NULL)
    for (paf = ch->first_affect; paf != NULL; paf = paf->next)
      {
	pto--;
	*pto = skill_table[paf->type].name[1] + 128;
	pto--;
	*pto = skill_table[paf->type].name[0] - 32 + 128;
      }
  if (ch->first_affect == NULL && !IS_SET (ch->act, PLR_WIZINVIS) &&
      !IS_SET (ch->act, PLR_HOLYLIGHT))
    {
      pto--;
      *pto = 'e';
      pto--;
      *pto = 'n';
      pto--;
      *pto = 'o';
      pto--;
      *pto = 'n';
    }
  pto--;
  *pto = ':';
  pto--;
  *pto = 'X';
  pto--;
  *pto = 'F';
  pto--;
  *pto = ' ';

  cnt = strlen (race_table[UNSHIFT (ch->speak)].race_name);
  pti = (char *) race_table[UNSHIFT (ch->speak)].race_name + cnt - 1;
  for (pto--; cnt > 0; cnt--, pto--, pti--)
    *pto = *pti + 128;
  *pto = ':';
  pto--;
  *pto = 'p';
  pto--;
  *pto = 'S';
  pto--;
  *pto = ' ';


  {				/* Add a game clock */
    hour = time_info.hour % 12;

    pto--;
    *pto = 'm' + 128;
    if (time_info.hour >= 12)
      {
	pto--;
	*pto = 'p' + 128;
      }
    else
      {
	pto--;
	*pto = 'a' + 128;
      }

    if (hour == 0)
      hour = 12;
    pto--;
    *pto = '0' + (hour % 10) + 128;
    if (hour >= 10)
      {
	pto--;
	*pto = '1' + 128;
      }

    pto--;
    *pto = ' ';
  }

  /* Add the bottom stat bar */
  pto = tm + (80 * (size_v - 1));
  ptoc = tc + (80 * (size_v - 1));

  cuc = color3;
  if ( IS_IMMORTAL(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
  &&   ch->pcdata->subprompt[0] != '\0' )
  {
   sprintf (buf, "%-61s", ch->pcdata->subprompt);
   for (ptb = buf; *ptb != '\0'; ptb++)
    {
      *pto = *ptb + 128;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  }
  else
  {

  *pto = ' ';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'H';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;

  val = ch->hit;
  if (ch->max_hit == 0)
    cnt = 9;
  else
    cnt = val * 10 / ch->max_hit;
  if (cnt < 3 || val < 0)
    cuc = 1 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else if (cnt < 7)
    cuc = 3 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else
    cuc = 2 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;

  if (val >= 1000)
    {
      *pto = '0' + 128 + val / 1000;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      if (val < 0)
	{
	  *pto = 128 + '-';
	  pto++;
	  *ptoc = cuc;
	  ptoc++;
	}
      else
	{
	  *pto = 128 + ' ';
	  pto++;
	  *ptoc = cuc;
	  ptoc++;
	}
    }

  if (abs (val) >= 100)
    {
      *pto = 128 + '0' + (abs (val) % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 128 + ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  if (val >= 10)
    {
      *pto = 128 + '0' + (abs (val) % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 128 + ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  *pto = 128 + '0' + (abs (val) % 10);
  pto++;
  *ptoc = cuc;
  ptoc++;

  cuc = color3;

  *pto = '/';
  pto++;
  *ptoc = cuc;
  ptoc++;
  val = abs (ch->max_hit);
  if (val >= 1000)
    {
      *pto = 128 + '0' + val / 1000;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 100)
    {
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 10)
    {
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  *pto = ' ';
  pto++;
  *ptoc = cuc;
  ptoc++;

  *pto = 'M';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'n';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;

  val = abs (ch->mana);
  if (ch->max_mana == 0)
    cnt = 9;
  else
    cnt = val * 10 / ch->max_mana;
  if (cnt < 3 || ch->hit < 0)
    cuc = 1 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else if (cnt < 7)
    cuc = 3 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else
    cuc = 2 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;

  if (val >= 1000)
    {
      *pto = 128 + '0' + val / 1000;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 100)
    {
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 10)
    {
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  cuc = color3;

  *pto = ' ';
  pto++;
  *ptoc = cuc;
  ptoc++;

  *pto = 'M';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'v';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;

  val = abs (ch->move);
  if (ch->max_move == 0)
    cnt = 9;
  else
    cnt = val * 10 / ch->max_move;
  if (cnt < 3 || ch->move < 0)
    cuc = 1 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else if (cnt < 7)
    cuc = 3 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;
  else
    cuc = 2 + (pcd->color[1][COLOR_BOT_STAT] - 40) * 8;

  if (val >= 1000)
    {
      *pto = 128 + '0' + val / 1000;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 100)
    {
      *pto = 128 + '0' + (val % 1000) / 100;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else if (val >= 10)
    {
      *pto = 128 + '0' + (val % 100) / 10;
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 128 + '0' + (val % 10);
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  cuc = color3;

  *pto = ' ';
  pto++;
  *ptoc = cuc;
  ptoc++;

  if (IS_SET (ch->pcdata->player2_bits, PLR2_EXP_TO_LEVEL))
    {
      *pto = 'e';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 'X';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  else
    {
      *pto = 'E';
      pto++;
      *ptoc = cuc;
      ptoc++;
      *pto = 'x';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;

  if (IS_SET (ch->pcdata->player2_bits, PLR2_EXP_TO_LEVEL))
    {
      if (ch->level >= 95)
	strcpy (buf, " max         ");
      else
	sprintf (buf, "%-13d", exp_level (ch->class, ch->level) - ch->exp);
    }
  else
    sprintf (buf, "%-13d", ch->exp);
  for (ptb = buf; *ptb != '\0'; ptb++)
    {
      *pto = *ptb + 128;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  *pto = 'G';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'd';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;

  sprintf (buf, "%-12d", ch->gold);
  for (ptb = buf; *ptb != '\0'; ptb++)
    {
      *pto = *ptb + 128;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }
 }
  
  cuc = color3;

  *pto = 'E';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'x';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 'i';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = 't';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ':';
  pto++;
  *ptoc = cuc;
  ptoc++;
  *pto = ' ';
  pto++;
  *ptoc = cuc;
  ptoc++;

  cnt = 0;
  if (IS_SET (ch->act, PLR_HOLYLIGHT) || !IS_AFFECTED (ch, AFF_BLIND))
    for (door = 0; door < 6; door++)
      {
	ROOM_INDEX_DATA *room;
	if (IS_SET (ch->in_room->room_flags, ROOM_HALLUCINATE) &&
	!is_affected (ch, gsn_truesight) && !IS_SET (ch->act, PLR_HOLYLIGHT))
	  room = ch->in_room->hallucinate_room;
	else
	  room = ch->in_room;

	if ((pexit = room->exit[door]) != NULL
	  && pexit->to_room != NULL && !IS_SET (pexit->exit_info, EX_CLOSED)
	    && (IS_NPC (ch) || (!IS_SET (pexit->exit_info, EX_HIDDEN)) ||
		IS_AFFECTED (ch, AFF_DETECT_HIDDEN))
	    && (!IS_SET (pexit->to_room->room_flags, ROOM_IS_CASTLE)
		|| IS_NPC (ch) || ch->pcdata == NULL
		|| !IS_SET (ch->pcdata->player2_bits, PLR2_CASTLES))
	    && (IS_NPC (ch) || can_use_exit (ch, room, door)))
	  {
	    cnt++;
	    switch (door)
	      {
	      case 0:
		*pto = 'N' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      case 1:
		*pto = 'E' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      case 2:
		*pto = 'S' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      case 3:
		*pto = 'W' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      case 4:
		*pto = 'U' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      case 5:
		*pto = 'D' + 128;
		*ptoc = cuc;
		pto++;
		ptoc++;
		break;
	      }
	  }
      }

  for (; cnt < 6; cnt++)
    {
      *pto = ' ';
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  if (ch->wait != 0)
    {
      *pto = '*' + 128;
      pto++;
      *ptoc = (cuc & 0xf8) + 7;
      ptoc++;
    }
  else
    {
      *pto = ' ' + 128;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  buf3[0] = (tim % 100) / 10 + '0';
  buf3[1] = (tim % 10) + '0';
  buf3[2] = '\0';
  hr = tim / 100 + ch->clock % 100;
  hr = hr % 24;
  if (ch->clock > 99)
    {
      buf[0] = (hr / 10) + '0';
      buf[1] = (hr % 10) + '0';
      buf[2] = ':';
      buf[3] = buf3[0];
      buf[4] = buf3[1];
      buf[5] = ' ';
      buf[6] = ' ';
      buf[7] = '\0';
      /*sprintf(buf,"%2d:%s  ",hr,buf3); */
    }
  else
    {
      if (hr > 11)
	{
	  hr -= 12;
	  strcpy (buf4, "p");
	}
      else
	strcpy (buf4, "a");
      if (hr == 0)
	hr = 12;
      if (hr >= 20)
	buf[0] = '2';
      else if (hr >= 10)
	buf[0] = '1';
      else
	buf[0] = ' ';
      buf[1] = (hr % 10) + '0';
      buf[2] = ':';
      buf[3] = buf3[0];
      buf[4] = buf3[1];
      buf[5] = buf4[0];
      buf[6] = 'm';
      buf[7] = '\0';
      /* sprintf(buf,"%2d:%s%sm",hr,buf3,buf4); */
    }

  for (ptb = buf; *ptb != '\0'; ptb++)
    {
      *pto = *ptb + 128;
      pto++;
      *ptoc = cuc;
      ptoc++;
    }

  /* Do the room mobiles  */

  if (size_v - 1 > 1)
    {


      colors = 7;

      cw = pcd->compass_width;

      if (cw == 0)
	max_width = 80 - 17;
      else
	max_width = 80 - 17 - 3 - 3 * cw;

      col = 2;
      /*  + ( (ch->pcdata->tactical_mode % 100) - 1 ) * 2;   Room for map  */
      row = 1;


      if (!IS_AFFECTED (ch, AFF_BLIND) && can_see_in_room (ch, ch->in_room) &&
	  ch->position > POS_SLEEPING)
	for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
	  if (col < max_width && can_see (ch, fch) &&
	  !(IS_AFFECTED (fch, AFF_HIDE) || IS_AFFECTED (fch, AFF_STEALTH) ||
	    IS_AFFECTED (fch, AFF_ETHEREAL)))
	    {
	      if (fch->name == NULL || fch->max_hit == 0)
		continue;
	      if (IS_NPC (fch))
		pti = capitalize (fch->short_descr);	/* Don't use again in loop */
	      else
		pti = fch->name;

	      if (fch->fighting != NULL && 
		  (fch->fighting->who->name == NULL ||
		  fch->fighting->who->in_room != fch->in_room))
		stop_fighting (fch, FALSE);

	      if (fch->fighting != NULL)	/* Bold for those fighting */
		bld = 128;
	      else
		bld = 0;

	      if (fch->hit >= fch->max_hit)
		cval = 'F';
	      else
		cval = '0' + (10 * fch->hit / fch->max_hit);

	      len = strlen (pti);

	      if ((pcd->tactical_mode / 1000) % 10 == 0)
		{
		  pto = tm + 80 * row + col;
		  ptoc = tc + 80 * row + col;
		}
	      else
		{
		  pto = tm + 80 * (row - 1) + col;
		  ptoc = tc + 80 * (row - 1) + col;
		}
	      cnt = 0;

	      *pto = cval;
	      pto++;
	      if ((10 * fch->hit / fch->max_hit) > 7)
		*ptoc = 2 + (pcd->color[1][COLOR_TACTICAL] - 40) * 8;
	      else if ((10 * fch->hit / fch->max_hit) > 3)
		*ptoc = 3 + (pcd->color[1][COLOR_TACTICAL] - 40) * 8;
	      else
		*ptoc = 1 + (pcd->color[1][COLOR_TACTICAL] - 40) * 8;
	      ptoc++;

	      if ((pcd->tactical_mode / 100) % 10 == 0)
		{
		  if (IS_NPC (fch))
		    {

		      *pto = *fch->name;
		      if (*pto >= 'a' && *pto <= 'z')
			*pto -= ('a' - 'A');
		      pto++;
		      ptoc++;
		      *pto = 'a' + (fch->npcdata->mob_number % 26);
		      pto++;
		      ptoc++;

/*      *pto = '0' + ((fch->npcdata->mob_number % 100 ) / 10 );
   pto ++;
   ptoc++;
   *pto = '0' + (fch->npcdata->mob_number % 10 );
   pto ++;
   ptoc++; */
		    }
		  else if (fch->pcdata->tactical_index[0] == '\0')
		    {
		      *pto = '0' + ((fch->pcdata->pvnum % 100) / 10);
		      pto++;
		      ptoc++;
		      *pto = '0' + (fch->pcdata->pvnum % 10);
		      pto++;
		      ptoc++;
		    }
		  else
		    {
		      *pto = fch->pcdata->tactical_index[0];
		      pto++;
		      ptoc++;
		      *pto = fch->pcdata->tactical_index[1];
		      pto++;
		      ptoc++;
		    }
		}
	      else
		{
		  *pto = '-';
		  pto++;
		  ptoc++;
		}


	      if (fch->fighting != NULL && is_same_group (fch->fighting->who, ch))
		colors = pcd->color[0][COLOR_TACT_ENEMY] - 30 +
		  (pcd->color[1][COLOR_TACT_ENEMY] - 40) * 8;
	      else if (is_same_group (fch, ch))
		colors = pcd->color[0][COLOR_TACT_PARTY] - 30 +
		  (pcd->color[1][COLOR_TACT_PARTY] - 40) * 8;
	      else
		colors = pcd->color[0][COLOR_TACT_NEUTRAL] - 30 +
		  (pcd->color[1][COLOR_TACT_NEUTRAL] - 40) * 8;

	      if (fch->fighting == NULL || fch->position != POS_FIGHTING ||
		  (pcd->tactical_mode / 100) % 10 == 1)
		{
		  for (; cnt < 12 && *pti != '\0'; cnt++, pti++, pto++, ptoc++)
		    {
		      *pto = *pti + bld;
		      *ptoc = colors;
		    }
		}
	      else
		{
		  for (; cnt < 9 && *pti != '\0'; cnt++, pti++, pto++, ptoc++)
		    {
		      *pto = *pti + bld;
		      *ptoc = colors;
		    }
		  for (; cnt < 9; cnt++, pto++, ptoc++)
		    {
		      *pto = '-' + bld;
		      *ptoc = colors;
		    }
		  *pto = '>';
		  pto++;
		  ptoc++;

		  if (fch->fighting && IS_NPC (fch->fighting->who))
		    {
		      if (fch->fighting->who->npcdata == NULL)
			bug ("Hey, an npc with an npcdata", 0);
		      else
			{

			  *pto = *fch->fighting->who->name;
			  if (*pto >= 'a' && *pto <= 'z')
			    *pto -= ('a' - 'A');
			  pto++;
			  ptoc++;
			  *pto = 'a' + (fch->fighting->who->npcdata->mob_number % 26);
			  pto++;
			  ptoc++;

/*          *pto = '0' + ((fch->fighting->npcdata->mob_number % 100 ) / 10 );
   pto ++;
   ptoc++;
   *pto = '0' + (fch->fighting->npcdata->mob_number % 10 );
   pto ++;
   ptoc++;  */
			}
		    }
		  else if (fch->fighting->who->pcdata->tactical_index[0] == '\0')
		    {
		      *pto = '0' + ((fch->fighting->who->pcdata->pvnum % 100) / 10);
		      pto++;
		      ptoc++;
		      *pto = '0' + (fch->fighting->who->pcdata->pvnum % 10);
		      pto++;
		      ptoc++;
		    }
		  else
		    {
		      *pto = fch->fighting->who->pcdata->tactical_index[0];
		      pto++;
		      ptoc++;
		      *pto = fch->fighting->who->pcdata->tactical_index[1];
		      pto++;
		      ptoc++;
		    }
		}

	      row++;
	      if (row > size_v - 2)
		{
		  row = 1;
		  col += 16;
		}
	    }

      /* Compass code  */
      if (IS_SET (ch->act, PLR_HOLYLIGHT) || !IS_AFFECTED (ch, AFF_BLIND))
	if (size_v - 1 > 3 && cw > 0)
	  {
	    ROOM_INDEX_DATA *room;
	    if (IS_SET (ch->in_room->room_flags, ROOM_HALLUCINATE) &&
		!is_affected (ch, gsn_truesight) && !IS_SET (ch->act, PLR_HOLYLIGHT))
	      room = ch->in_room->hallucinate_room;
	    else
	      room = ch->in_room;

	    for (door = -1; door <= 5; door++)
	      {
		found = FALSE;
		if (door == -1)
		  {
		    found = TRUE;
		    if (!can_see_in_room (ch, room))
		      strcpy (buf3, "Too dark to tell");
		    else
		      strcpy (buf3, room->name);
		    colors = get_sector_color (ch, room->sector_type);
		  }
		else if ((pexit = room->exit[door]) != NULL
			 && pexit->to_room != NULL
			 && ((!IS_SET (pexit->exit_info, EX_HIDDEN)) ||
			     IS_AFFECTED (ch, AFF_DETECT_HIDDEN))
		    && (!IS_SET (pexit->to_room->room_flags, ROOM_IS_CASTLE)
			|| IS_NPC (ch) || ch->pcdata == NULL
			|| !IS_SET (ch->pcdata->player2_bits, PLR2_CASTLES))
			 && (IS_NPC (ch) || can_use_exit (ch, room, door)))
		  {
		    found = TRUE;
		    colors = get_sector_color (ch, pexit->to_room->sector_type);
		    if (!IS_SET (pexit->exit_info, EX_CLOSED))
		      {
			if (!can_see_in_room (ch, pexit->to_room))
			  strcpy (buf3, "Too dark to tell");
			else
			  strcpy (buf3, pexit->to_room->name);
		      }
		    else
		      {
			if (pexit->description != NULL && pexit->description[0] != '\0')
			  strcpy (buf3, pexit->description);
			else
			  strcpy (buf3, "You see nothing special.");
		      }
		  }

		if (found)
		  {

		    if ((ch->pcdata->tactical_mode / 1000) % 10 == 0)
		      vo = 0;
		    else
		      vo = -1;

		    offset = 0;

		    if (buf3[0] == 't' || buf3[0] == 'T')
		      if (buf3[1] == 'h' || buf3[1] == 'H')
			if (buf3[2] == 'e' || buf3[2] == 'E')
			  if (buf3[3] == ' ')
			    offset = 4;

		    if (buf3[0] == 'a' || buf3[0] == 'A')
		      if (buf3[1] == ' ')
			offset = 2;

		    str_cpy_max (buf2, capitalize (buf3 + offset),
				 cw + 1);

		    if (size_v - 1 > 5)
		      {
			switch (door)
			  {
			  case -1:
			    pto = tm + 80 * (3 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (3 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 0:
			    pto = tm + 80 * (2 + vo) + 80 - 1 - 3 * cw / 2;
			    *pto = '|';
			    pto = tm + 80 * (1 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (1 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 1:
			    pto = tm + 80 * (3 + vo) + 80 - 1 - cw;
			    *pto = '-';
			    pto = tm + 80 * (3 + vo) + 80 - cw;
			    ptoc = tc + 80 * (3 + vo) + 80 - cw;
			    break;
			  case 2:
			    pto = tm + 80 * (4 + vo) + 80 - 1 - 3 * cw / 2;
			    *pto = '|';
			    pto = tm + 80 * (5 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (5 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 3:
			    pto = tm + 80 * (3 + vo) + 80 - 2 - 2 * cw;
			    *pto = '-';
			    pto = tm + 80 * (3 + vo) + 80 - 2 - 3 * cw;
			    ptoc = tc + 80 * (3 + vo) + 80 - 2 - 3 * cw;
			    break;
			  case 4:
			    pto = tm + 80 * (2 + vo) + 80 - 1 - cw;
			    *pto = '/';
			    pto = tm + 80 * (1 + vo) + 80 - cw;
			    ptoc = tc + 80 * (1 + vo) + 80 - cw;
			    break;
			  case 5:
			    pto = tm + 80 * (4 + vo) + 80 - 2 - 2 * cw;
			    *pto = '/';
			    pto = tm + 80 * (5 + vo) + 80 - 2 - 3 * cw;
			    ptoc = tc + 80 * (5 + vo) + 80 - 2 - 3 * cw;
			    break;
			  }
			for (cnt = 0, found = FALSE, pti = buf2; cnt < cw; ptoc++, pto++, cnt++)
			  {
			    if (found)
			      {
				*pto = ' ';
				*ptoc = (char) colors;
			      }
			    else if (*pti == '\n' || *pti == '\r' || *pti == '\0')
			      {
				found = TRUE;
				*pto = ' ';
				*ptoc = (char) colors;
			      }
			    else
			      {
				*pto = *pti;
				*ptoc = (char) colors;
				pti++;
			      }
			  }
		      }
		    else
		      {
			switch (door)
			  {
			  case -1:
			    pto = tm + 80 * (2 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (2 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 0:
			    pto = tm + 80 * (1 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (1 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 1:
			    pto = tm + 80 * (2 + vo) + 80 - 1 - cw;
			    *pto = '-';
			    pto = tm + 80 * (2 + vo) + 80 - cw;
			    ptoc = tc + 80 * (2 + vo) + 80 - cw;
			    break;
			  case 2:
			    pto = tm + 80 * (3 + vo) + 80 - 1 - 2 * cw;
			    ptoc = tc + 80 * (3 + vo) + 80 - 1 - 2 * cw;
			    break;
			  case 3:
			    pto = tm + 80 * (2 + vo) + 80 - 2 - 2 * cw;
			    *pto = '-';
			    pto = tm + 80 * (2 + vo) + 80 - 2 - 3 * cw;
			    ptoc = tc + 80 * (2 + vo) + 80 - 2 - 3 * cw;
			    break;
			  case 4:
			    pto = tm + 80 * (1 + vo) + 80 - 1 - cw;
			    *pto = '/';
			    pto = tm + 80 * (1 + vo) + 80 - cw;
			    ptoc = tc + 80 * (1 + vo) + 80 - cw;
			    break;
			  case 5:
			    pto = tm + 80 * (3 + vo) + 80 - 2 - 2 * cw;
			    *pto = '/';
			    pto = tm + 80 * (3 + vo) + 80 - 2 - 3 * cw;
			    ptoc = tc + 80 * (3 + vo) + 80 - 2 - 3 * cw;
			    break;
			  }
			for (cnt = 0, found = FALSE, pti = buf2; cnt < cw;
			     ptoc++, pto++, cnt++)
			  {
			    if (found)
			      {
				*pto = ' ';
				*ptoc = (char) colors;
			      }
			    else if (*pti == '\n' || *pti == '\r' || *pti == '\0')
			      {
				found = TRUE;
				*pto = ' ';
				*ptoc = (char) colors;
			      }
			    else
			      {
				*pto = *pti;
				*ptoc = (char) colors;
				pti++;
			      }
			  }
		      }
		  }

	      }

	  }



    }

  return (tact);
}

TACTICAL_MAP *
get_diff_tactical (CHAR_DATA * ch)
{
  TACTICAL_MAP *tact;
  int cnt, lcnt;
  bool any_diff;
  unsigned char *tm, *tc, *cm, *cc;
  int size_v;

  any_diff = FALSE;
  size_v = ch->pcdata->tactical_mode % 100 + 1;


  tact = get_tactical_map (ch);

  if (ch->pcdata->tactical == NULL)
    return (tact);

  tm = (unsigned char *) tact->map;
  tc = (unsigned char *) tact->color;
  cm = (unsigned char *) ch->pcdata->tactical->map;
  cc = (unsigned char *) ch->pcdata->tactical->color;

  for (lcnt = 0; !any_diff && lcnt < size_v; lcnt++)
    for (cnt = 0; !any_diff && cnt < 80; cnt++, tm++, tc++, cm++, cc++)
      if (*tm != *cm || *tc != *cc)
	any_diff = TRUE;

  if (any_diff)
    return (tact);
  else
    {
      return (NULL);
    }
}




char *
get_color_diff (CHAR_DATA * ch, int old_for, int old_bak, bool old_bold,
		int new_for, int new_bak, bool new_bold)
{
  char *ptr;
  ptr = (char *) get_color_diff_string;		/* Shorten that name up */

  strcpy (ptr, "");

  if (IS_NPC (ch) || ch->vt100 == 0)
    return (ptr);
  /*if( ch->ansi != 1)
     return( ptr ); */
  if (old_for == new_for && old_bak == new_bak && old_bold == new_bold)
    return (ptr);


  /*   compressed terminal codes   */
  if (ch->ansi != 0 && ch->pcdata->term_info != 0)
    {
      int num;
      num = 128 + (new_bold ? 64 : 0) + (int) new_for + (int) new_bak *8;
      *ptr = 14;
      *(ptr + 1) = num;
      *(ptr + 2) = '\0';
      return (ptr);
    }

  /* No ansi color */
  if (ch->ansi == 0)
    {
      if (new_bak > 0)
	{
	  if (new_bak == old_bak)
	    return (ptr);
	  strcpy (ptr, "\033[7m");
	  return (ptr);
	}

      if (new_bold == old_bold)
	return (ptr);
      if (new_bold)
	{
	  strcpy (ptr, "\033[1m");
	  return (ptr);
	}
      strcpy (ptr, "\033[m");
      return (ptr);
    }

  /* Ansi color only section */

  if (new_bold)
    {
      sprintf (ptr, "\033[1;%d;%dm", new_for + 30, new_bak + 40);
      return (ptr);
    }
  sprintf (ptr, "\033[;%d;%dm", new_for + 30, new_bak + 40);
  return (ptr);

  /* Change in bold only 
     if( new_for == old_for && new_bak == old_bak )
     {
     if( new_bold == old_bold )
     return( ptr );
     if( new_bold )
     {
     strcpy( ptr, "\033[1m" );
     return( ptr );
     }
     sprintf( ptr, "\033[%d;%dm", new_for+30, new_bak+40 );
     return( ptr );
     }  */

  /* Foreground color change 
     if( new_for != old_for && new_bak == old_bak )
     {  
     if( new_bold == old_bold )
     {
     sprintf( ptr, "\033[%dm", new_for+30 );
     return( ptr );
     }
     if( new_bold )
     {
     sprintf( ptr, "\033[1;%dm", new_for+30 );
     return( ptr );
     }
     sprintf( ptr, "\033[;%d;%dm", new_for+30, new_bak+40 );
     return( ptr );
     }

     if( new_for == old_for && new_bak != old_bak )
     {  
     if( new_bold == old_bold )
     {
     sprintf( ptr, "\033[%dm", new_bak+40 );
     return( ptr );
     }
     if( new_bold )
     {
     sprintf( ptr, "\033[1;%dm", new_bak+40 );
     return( ptr );
     }
     sprintf( ptr, "\033[;%d;%dm", new_for+30, new_bak+40 );
     return( ptr );
     }

     if( new_bold == old_bold )
     {
     sprintf( ptr, "\033[%d;%dm", new_for+30, new_bak+40 );
     return( ptr );
     }
     if( new_bold )
     {
     sprintf( ptr, "\033[1;%d;%dm", new_for+30, new_bak+40 );
     return( ptr );
     }
     sprintf( ptr, "\033[;%d;%dm", new_for+30, new_bak+40 );
     return( ptr );  */
}


void 
do_terminal (CHAR_DATA * ch, char *arg)
{
  if (IS_NPC (ch))
    return;
  if (ch->ansi == 0)
    {
      send_to_char ("You do not have color installed.\n\r", ch);
      return;
    }
  if (!strcasecmp (arg, "on"))
    {
      ch->pcdata->term_info = 1;
      /* sprintf(buf,"%c%c%c", 14, 24, 32 + ch->vt100_type %100 );
         send_to_char(buf,ch); */
    }
  else
    ch->pcdata->term_info = 0;

  return;
}

void 
do_termlist (CHAR_DATA * ch, char *arg)
{
  PLAYER_GAME *gpl;
  int clcnt, vtcnt, mtcnt, prcnt;
  char buf[100];
  char tbuf[50];

  vtcnt = 0;
  clcnt = 0;
  mtcnt = 0;
  prcnt = 0;
  for (gpl = first_player; gpl != NULL; gpl = gpl->next)
    {
      if (IS_SET (gpl->ch->act, PLR_TERMINAL))
	{
	  mtcnt++;
	  if (IS_SET (gpl->ch->act, PLR_PROMPT))
	    sprintf (tbuf, "MrTerm %d", gpl->ch->vt100_type % 100);
	  else
	    strcpy (tbuf, "MrTerm prompt");
	}
      else if (gpl->ch->ansi != 0)
	{
	  /*if( IS_SET(gpl->ch->act, PLR_PROMPT ) )
	     sprintf( tbuf, "ANSI %d", gpl->ch->vt100_type % 100 );
	     else */
	  sprintf (tbuf, "ANSI prompt");
	  clcnt++;
	}
      else if (gpl->ch->vt100 != 0)
	{
	  if (IS_SET (gpl->ch->act, PLR_PROMPT))
	    sprintf (tbuf, "VT %d", gpl->ch->vt100_type % 100);
	  else
	    sprintf (tbuf, "VT prompt");
	  vtcnt++;
	}
      else
	{
	  strcpy (tbuf, "TTY prompt");
	  prcnt++;
	}
      sprintf (buf, "%15s  %s\n\r", capitalize (gpl->ch->name), tbuf);
      send_to_char (buf, ch);
    }

  sprintf (buf, "MrTerm: %d    ANSI: %d    VT: %d    TTY: %d\n\r",
	   mtcnt, clcnt, vtcnt, prcnt);
  send_to_char (buf, ch);
  return;
}

char *
vt_command_0 (CHAR_DATA * ch, char option)
{
  if (ch->pcdata->term_info != 0)
    {
      sprintf ((char *) vt_command_0_string, "%c%c", 15, option);
      return ((char *) vt_command_0_string);
    }

  sprintf ((char *) vt_command_0_string, "\033[%c", option);
  return ((char *) vt_command_0_string);
}

char *
vt_command_1 (CHAR_DATA * ch, int val1, char option)
{
  if (ch->pcdata->term_info != 0)
    {
      sprintf ((char *) vt_command_1_string, "%c%c%c", 16, val1 + 32, option);
      return ((char *) vt_command_1_string);
    }

  sprintf ((char *) vt_command_1_string, "\033[%d%c", val1, option);
  return ((char *) vt_command_1_string);
}

char *
vt_command_2 (CHAR_DATA * ch, int val1, int val2, char option)
{
  if (ch->pcdata->term_info != 0)
    {
      sprintf ((char *) vt_command_2_string, "%c%c%c%c", 20, val1 + 32, val2 + 32, option);
      return ((char *) vt_command_2_string);
    }

  sprintf ((char *) vt_command_2_string, "\033[%d;%d%c", val1, val2, option);
  return ((char *) vt_command_2_string);
}

char * vt_command_3 (CHAR_DATA * ch, int val1, int val2, int val3, char option)
{
  if (ch->pcdata->term_info != 0)
    {
      sprintf ((char *) vt_command_3_string, "%c%c%c%c%c", 18, 32 + val1, 32 + val2,
	       32 + val3, option);
      return ((char *) vt_command_3_string);
    }

  sprintf ((char *) vt_command_3_string, "\033[%d;%d;%d%c", val1, val2, val3, option);
  return ((char *) vt_command_3_string);
}

int get_sector_color (CHAR_DATA * ch, int sector_type)
{
  int col;
  switch (sector_type)
    {
    case SECT_INSIDE:
      col = 7 + 0 * 8;
      break;
    case SECT_CITY:
      col = 5 + 7 * 8;
      break;
    case SECT_FIELD:
      col = 0 + 3 * 8;
      break;
    case SECT_FOREST:
      col = 1 + 2 * 8;
      break;
    case SECT_HILLS:
      col = 0 + 7 * 8;
      break;
    case SECT_MOUNTAIN:
      col = 4 + 7 * 8;
      break;
    case SECT_WATER_SWIM:
      col = 4 + 6 * 8;
      break;
    case SECT_WATER_NOSWIM:
      col = 6 + 4 * 8;
      break;
    case SECT_AIR:
      col = 0 + 6 * 8;
      break;
    case SECT_DESERT:
      col = 1 + 3 * 8;
      break;
    case SECT_LAVA:
      col = 3 + 1 * 8;
      break;
    case SECT_INN:
      col = 2 + 5 * 8;
      break;
    case SECT_ASTRAL:
      col = 3;
      break;
    case SECT_ETHEREAL:
      col = 2;
      break;
    case SECT_UNDER_WATER:
      col = 4;
      break;
    case SECT_UNDER_GROUND:
      col = 5;
      break;
    case SECT_DEEP_EARTH:
      col = 1;
      break;
    default:
      col = ch->pcdata->color[0][COLOR_TACTICAL] - 30 +
	(ch->pcdata->color[1][COLOR_TACTICAL] - 40) * 8;
    }

  return (col);
}

void do_reincarnate (CHAR_DATA * ch, char *arg)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  sh_int future_reincarnate;
  int cnt, sn, future_class, future_race;
  int future_hours;
  bool was_95;
  OBJ_DATA *obj, *obj_next;
  CHAR_DATA *fch;

  DISALLOW_SNOOP = TRUE;

  if (IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM))
    {
      wipe_string (arg);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  reincarnate <class> <race> <password>\n\r", ch);
      return;
    }
  if (ch->level == 95 && ch->pcdata->reincarnation == 5)
    {
      send_to_char ("You have reached the pinnacle of mortality. Rejoice and be at peace.\n\r", ch);
      return;
    }

  if (ch->level != 95)
    {
      if (ch->pcdata->allow_reincarnate != 1 || ch->pcdata->reincarnation == 0)
	{
	  send_to_char ("You may only reincarnate yourself at 95th level or as a new reincarnate.\n\r", ch);
	  wipe_string (arg);
	  return;
	}
      was_95 = FALSE;
    }
  else
    was_95 = TRUE;

  arg = one_argument (arg, arg1);
  arg = one_argument (arg, arg2);

  future_class = -1;
  future_race = -1;
  for (cnt = 0; cnt < MAX_CLASS; cnt++)
    {
      if (is_name_short (arg1, (char *) class_table[cnt].who_name_long))
	{
	  future_class = cnt;
	  break;
	}
    }
  for (cnt = 0; cnt < MAX_RACE; cnt++)
    {
      if (is_name_short (arg2, (char *) race_table[cnt].race_name))
	{
	  future_race = cnt;
	  break;
	}
    }
  if (future_class == -1)
    {
      send_to_char ("You must select a valid class to reincarnate.\n\r", ch);
      wipe_string (arg);
      return;
    }
  if (future_race == -1)
    {
      send_to_char ("You must select a valid race to reincarnate.\n\r", ch);
      wipe_string (arg);
      return;
    }
  if (race_table[future_race].race_class[future_class] == 1)
    {
      send_to_char ("That combination of class and race is not allowed.\n\r", ch);
      wipe_string (arg);
      return;
    }
  if (strcasecmp (crypt (arg, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      send_to_char ("You must include your password to reincarnate.\n\r", ch);
      wipe_string (arg);
      return;
    }

  if (was_95)
    {
      future_reincarnate = ch->pcdata->reincarnation + 1;
      future_hours = ch->played / 3600;
      ch->played = 0;
    }
  else
    {
      future_reincarnate = ch->pcdata->reincarnation;
      future_hours = 0;
    }


  /* Reset all the character stats here */

  stop_follower (ch);

  for (fch = first_char; fch != NULL; fch = fch->next)
    if (fch->master == ch || fch->leader == ch)
      {
	fch->master = NULL;
	fch->leader = NULL;
      }

  stop_fighting (ch, TRUE);
  char_from_room (ch);
  char_to_room (ch, room_index[ROOM_VNUM_TEMPLE]);
  for (obj = ch->first_carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      extract_obj (obj);
    }

  ch->act = PLR_COMBINE | PLR_PROMPT | PLR_AUTOEXIT | PLR_AUTOSAC;
  ch->level = 2;
  ch->class = future_class;
  ch->race = future_race;
  ch->speak = 0;
  ch->language = 0;
  ch->rank = 0;
  ch->leader = NULL;
  if (was_95)
    ch->played = FALSE;

  ch->hit = future_reincarnate * 15 + 30;
  ch->max_hit = future_reincarnate * 15 + 30;
  ch->actual_max_hit = future_reincarnate * 15 + 30;
  ch->mana = future_reincarnate * 15 + 100;
  ch->max_mana = future_reincarnate * 15 + 100;
  ch->actual_max_mana = future_reincarnate * 15 + 100;
  ch->move = future_reincarnate * 15 + 100;
  ch->max_move = future_reincarnate * 15 + 100;
  ch->actual_max_move = future_reincarnate * 15 + 100;

  ch->gold = 0;
  ch->exp = exp_level (future_class, 1) + 1;
  ch->pcdata->condition[COND_THIRST] = 48;
  ch->pcdata->condition[COND_FULL] = 48;
  ch->practice = 10 + 3 * future_reincarnate;
  ch->carry_weight = 0;
  ch->carry_number = 0;
  ch->saving_throw = 0;
  ch->pcdata->eqsaves = 0;
  ch->pcdata->eqhitroll = 0;
  ch->pcdata->eqdamroll = 0;
  ch->alignment = 0;
  ch->wimpy = 10;

  ch->which_god = GOD_NEUTRAL;
  ch->exp_lost = 0;
  ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
  ch->recall = ROOM_VNUM_TEMPLE;
  ch->speed = 1;
  roll_race (ch);		/* Roll them stats */

  ch->npcdata = NULL;

  for (cnt = 0; cnt < MAX_CLASS; cnt++)
    if (future_class != cnt)
      ch->mclass[cnt] = 0;
    else
      ch->mclass[cnt] = 2;
  for (cnt = 0; cnt < MAX_AREA; cnt++)
    if( ch->pcdata->quest[cnt] != NULL )
      {
      DISPOSE( ch->pcdata->quest[cnt] );
      ch->pcdata->quest[cnt]=NULL;
      }
  ch->pcdata->corpse_room = 0;
  ch->pcdata->corpse = NULL;
  ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
  ch->pcdata->account = 0;
  ch->mclass_switched = 0;
  ch->obj_with_prog = NULL;
  ch->asn_obj = NULL;
  ch->pcdata->army_status = 0;

  for (sn = 0; sn < MAX_SKILL; sn++)
    ch->pcdata->learned[sn] = 0;

  stop_shadow (ch);
  sub_player (ch);
  add_player (ch);

  char_reset (ch);

  send_to_char ("You are reincarnated.\n\r", ch);
  ch->pcdata->reincarnation = future_reincarnate;
  ch->pcdata->allow_reincarnate = 1;	/* Allow multiple for rerolls */
  ch->pcdata->previous_hours += future_hours;

  do_status (ch, "reincarnate");

  wipe_string (arg);
  return;
}

void do_download (CHAR_DATA * ch, char *argument)
{
  unsigned char bufo[33000];
  unsigned char bufi[MAX_INPUT_LENGTH];
  FILE *fp;
  int cnt, foe;
  unsigned char *pt, bt;
  unsigned char *pto, *ptc;
  int checksum;
  int port_size;
  int send_offset;
  int file_len;

  if (IS_NPC (ch) || ch->desc == NULL || ch->desc->character != ch)
    return;
  if (IS_SET (ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES) ||
      !IS_SET (ch->act, PLR_TERMINAL))
    return;

  port_size = ch->desc->port_size;

  argument = one_argument (argument, bufi);
  send_offset = atoi (bufi);

  while (*argument != '\0' && *argument < 33)
    argument++;

  for (pt = argument; *pt > 32 && *pt < 128; pt++);
  *pt = '\0';

  /* sprintf( bufo, "You want to download '%s' with offset %d.\n\r", argument,
     send_offset );
     send_to_char( bufo, ch ); */

  if (send_offset > 31001)
    return;
  if (send_offset < 0)
    return;

  sprintf (bufo, "data/%s", argument);

  fp = fopen (bufo, "rb");
  if (fp == NULL)
    {
      sprintf (bufo, "The file '%s' could not be found.\n\r", argument);
      send_to_char (bufo, ch);
      return;
    }


  /* foe=0;
     cnt=0;
     while( foe==0 && cnt<32000 && cnt<send_offset)
     {
     bt = fgetc( fp );
     cnt++;
     foe=feof(fp);
     if( foe!=0 )
     return;
     } */

  fseek (fp, 0, SEEK_END);
  file_len = ftell (fp);

  if (file_len > 32000)
    {
      sprintf (bufo, "The file '%s' is too long.\n\r", argument);
      send_to_char (bufo, ch);
      return;
    }

  if (send_offset > file_len)
    return;

  fseek (fp, send_offset, SEEK_SET);
  if (feof (fp) != 0)
    return;

  if (!IS_SET (ch->pcdata->player2_bits, PLR2_EXTERNAL_METER))
    {
      sprintf (bufo, "Downloading '%s' %d%%\n\r", argument,
	       100 * send_offset / file_len);
      send_to_char (bufo, ch);
    }

  pto = bufo;
  cnt = 0;
  foe = 0;
  *pto = 30;
  pto++;
  *pto = 'F';
  pto++;
  for (pt = argument; *pt != '\0'; pt++, pto++)
    *pto = *pt;
  *pto = 29;
  pto++;
  ptc = pto;
  pto += 2;			/* Skip checksum */
  *pto = 'a' + (unsigned char) (send_offset % 16);
  pto++;
  *pto = 'A' + (unsigned char) ((send_offset / 16) % 16);
  pto++;
  *pto = 'a' + (unsigned char) ((send_offset / 256) % 16);
  pto++;
  *pto = 'A' + (unsigned char) ((send_offset / 256 / 16) % 16);
  pto++;
  checksum = 0;
  while (foe == 0 && cnt < 500)
    {
      bt = fgetc (fp);
      cnt++;
      foe = feof (fp);
      if (foe == 0)
	{
	  *pto = 'a' + bt % 16;
	  pto++;
	  *pto = 'A' + (bt / 16) % 16;
	  pto++;
	  checksum += bt;
	  checksum = checksum % 256;
	}
    }
  if (foe == 0)
    *pto = 29;
  else
    *pto = 30;
  pto++;
  *pto = '\0';
  pto = bufo;

  *ptc = 'a' + checksum % 16;
  ptc++;
  *ptc = 'A' + (checksum / 16) % 16;

  fclose (fp);

  /*sprintf( bufi, "Sending %d bytes. Checksum %d.  Stringlength %d.\n\r", 
     cnt-1, checksum, strlen(bufo) );
     send_to_char( bufi, ch ); */

  ch->desc->port_size = 64080;
  write_to_port (ch->desc);	/*Flush port */
  write_to_buffer (ch->desc, bufo, 1000000);
  ch->desc->port_size = port_size;

  /*send_to_char( "Sent.\n\r", ch ); */
  return;
}

void do_external (CHAR_DATA * ch, char *argument)
{
  FILE *fp;
  unsigned char *pt;
  char bufo[MAX_INPUT_LENGTH];

  if (argument == NULL || *argument == '\0')
    {
      send_to_char ("That is not a file name.\n\r", ch);
      return;
    }

  if (!IS_NPC (ch) && ch->desc != NULL &&
      ch->desc->character == ch &&
      !IS_SET (ch->pcdata->player2_bits, PLR2_EXTERNAL_FILES) &&
      IS_SET (ch->act, PLR_TERMINAL))
    {
      /* Send notice of file */

      while (*argument != '\0' && *argument < 33)
	argument++;

      for (pt = argument; *pt > 32 && *pt < 128; pt++);
      *pt = '\0';
      for (pt = argument; *pt != '\0'; pt++)
	if (*pt == '\\' || *pt == '/')
	  *pt = '-';

      sprintf (bufo, "data/%s", argument);

      fp = fopen (bufo, "rb");
      if (fp == NULL)
	{
	  sprintf (bufo, "The file '%s' could not be found.\n\r", argument);
	  send_to_char (bufo, ch);
	  return;
	}
      fclose (fp);

      sprintf (bufo, "%c%c%s%c", 30, 'E', argument, 29);

      write_to_port (ch->desc);
      write_to_buffer (ch->desc, bufo, 1000000);
    }

  return;
}

void Update_MrTerm_Stats (CHAR_DATA * ch)
{
  char buf[MAX_INPUT_LENGTH];
  int cnt;
  int blen;

  blen = 0;
  *buf = 30;
  blen++;
  *(buf + blen) = 'Y';
  blen++;

  if (ch->max_mana != ch->pcdata->MrTerm_Max_Mana)
    {
      *(buf + blen) = 'M';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->max_mana, 3);
      ch->pcdata->MrTerm_Max_Mana = ch->max_mana;
    }

  if (ch->max_move != ch->pcdata->MrTerm_Max_Move)
    {
      *(buf + blen) = 'V';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->max_move, 3);
      ch->pcdata->MrTerm_Max_Move = ch->max_move;
    }

  if (ch->max_hit != ch->pcdata->MrTerm_Max_Hit)
    {
      *(buf + blen) = 'H';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->max_hit, 3);
      ch->pcdata->MrTerm_Max_Hit = ch->max_hit;
    }

  if (ch->move != ch->pcdata->MrTerm_Move)
    {
      *(buf + blen) = 'v';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->move, 3);
      ch->pcdata->MrTerm_Move = ch->move;
    }

  if (ch->mana != ch->pcdata->MrTerm_Mana)
    {
      *(buf + blen) = 'm';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->mana, 3);
      ch->pcdata->MrTerm_Mana = ch->mana;
    }

  if (ch->hit != ch->pcdata->MrTerm_Hit)
    {
      *(buf + blen) = 'h';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->hit, 3);
      ch->pcdata->MrTerm_Hit = ch->hit;
    }

  if (ch->exp != ch->pcdata->MrTerm_Experience)
    {
      *(buf + blen) = 'x';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->exp, 6);
      ch->pcdata->MrTerm_Experience = ch->exp;
    }

  cnt = exp_level (ch->class, ch->level) - ch->exp;
  if (cnt != ch->pcdata->MrTerm_Exp_Level)
    {
      *(buf + blen) = 'X';
      blen++;
      blen = MrTerm_number_append (buf, blen, cnt, 6);
      ch->pcdata->MrTerm_Exp_Level = cnt;
    }

  if (ch->gold != ch->pcdata->MrTerm_Gold)
    {
      *(buf + blen) = 'g';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->gold, 6);
      ch->pcdata->MrTerm_Gold = ch->gold;
    }

  if (ch->position != ch->pcdata->MrTerm_Position)
    {
      *(buf + blen) = 'p';
      blen++;
      blen = MrTerm_number_append (buf, blen, ch->position, 1);
      ch->pcdata->MrTerm_Position = ch->position;
    }

  cnt = ch->pcdata->condition[COND_THIRST];
  if (cnt != 0)
    cnt = 1;
  if (cnt != ch->pcdata->MrTerm_Thirst)
    {
      *(buf + blen) = 'd';
      blen++;
      blen = MrTerm_number_append (buf, blen, cnt, 1);
      ch->pcdata->MrTerm_Thirst = cnt;
    }

  cnt = ch->pcdata->condition[COND_FULL];
  if (cnt != 0)
    cnt = 1;
  if (cnt != ch->pcdata->MrTerm_Full)
    {
      *(buf + blen) = 'f';
      blen++;
      blen = MrTerm_number_append (buf, blen, cnt, 1);
      ch->pcdata->MrTerm_Full = cnt;
    }

  if (blen > 2)
    {
      *(buf + blen) = 29;
      blen++;
      *(buf + blen) = '\0';

      write_to_descriptor (ch->desc, buf, 0);
    }

  return;
}

int MrTerm_number_append (char *buf, int blen, int val, int d64)
{
  int cnt, vo, aval;
  bool sign;

  aval = val;
  if (aval < 0)
    {
      aval = 0 - aval;
      sign = TRUE;
    }
  else
    sign = FALSE;

  for (vo = 0, cnt = 0; cnt < d64; cnt++)
    {
      if (vo == 0)
	{
	  vo = (aval % 32) + 32;
	  if (sign)
	    vo += 32;
	  aval /= 32;
	}
      else
	{
	  vo = (aval % 64) + 32;
	  aval /= 64;
	}

      *(buf + blen) = vo;
      blen++;
    }

  return (blen);
}
