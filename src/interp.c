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
/* #include <strings.h> */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "merc.h"

/*int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) ); */


bool    check_social    args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );


void    process_command( CHAR_DATA *, char *, bool);
void preprocess_command( CHAR_DATA *, char *, bool);

/*
 * Command logging types.
 */
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

#define CENSOR          1


/*
 * Log-all switch.
 */
bool                            fLogAll         = FALSE;



/*
 * Command table.
 */
sh_int cmd_gsn[512];


const struct    cmd_type        cmd_table       [] =
{
    /*
     * Common movement commands.
     */
    { "north",          do_north,       POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "east",           do_east,        POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "south",          do_south,       POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "west",           do_west,        POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "up",             do_up,          POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "down",           do_down,        POS_STANDING,    0,  LOG_NORMAL ,NULL},

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "buy",            do_buy,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "banish",         do_ban,         POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "bank",           do_bank,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "bounty",         do_bounty,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "cast",           do_cast,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "mana",           do_mana,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "mass",           do_mass,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "move",           do_move,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "rcast",          do_rcast,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "exits",          do_exits,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "get",            do_get,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "inventory",      do_inventory,   POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "identify",       do_identify,    POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "kill",           do_kill,   POS_FIGHTING,  0,  LOG_NORMAL , &cmd_kill},
    { "rank",           do_rank,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "look",           do_look,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "know",           do_know,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "glance",         do_glance,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "order",          do_order,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "rest",           do_rest,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "stand",          do_stand,       POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "tell",           do_tell,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "wield",          do_wear,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "wizhelp",        do_wizhelp,     POS_DEAD,       96,  LOG_NORMAL ,NULL},
    { "etell",          do_etell,       POS_DEAD,        0,  LOG_NORMAL ,NULL},

    /*
     * Informational commands.
     */
    { "a",              do_a,           POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "areas",          do_areas,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "afk",          do_afk,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "bug",            do_bug,         POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "commands",       do_commands,    POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "skills",         do_skills,      POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "compare",        do_compare,     POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "consider",       do_consider,    POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "credits",        do_credits,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "equipment",      do_equipment,   POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "examine",        do_examine,     POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "help",           do_help,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "display",        do_display,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "idea",           do_idea,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "report",         do_report,      POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "score",          do_score,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "status",         do_status,      POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "affects",        do_affects,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "socials",        do_socials,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "time",           do_time,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "typo",           do_typo,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "weather",        do_weather,     POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "who",            do_who,         POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "language",       do_language,    POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "level",          do_level,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "vt102",          do_vt100,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "color",          do_color,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "buffer",         do_buffer,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "grep",           do_grep,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "port",           do_port,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "?",              do_grep,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "hoo",            do_a,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "hook",           do_hook,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "history",        do_history,     POS_DEAD,        0,  LOG_NORMAL,NULL},
    { "refresh",        do_refresh,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "clock",          do_clock,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "class",          do_class,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "multiclass",     do_multi,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "alias",          do_alias,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "unalias",        do_unalias,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "block",          do_block,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "speed",          do_speed,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "castle",         do_castle,      POS_STANDING,    0,  LOG_ALWAYS ,NULL},

    /*
     * Configuration commands.
     */
    { "spam",           do_spam,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "channel",        do_channel,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "config",         do_config,      POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "configb",        do_config2,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "description",    do_description, POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "password",       do_password,    POS_DEAD,        0,  LOG_NEVER  ,NULL},
    { "title",          do_title,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "wimpy",          do_wimpy,       POS_DEAD,        0,  LOG_NORMAL ,NULL},

    /*
     * Communication commands.
     */
    { "chat",           do_chat,        POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "beep",           do_beep,        POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "plan",           do_plan,        POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { ".",              do_chat,        POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "emote",          do_emote,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { ",",              do_emote,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "gtell",          do_gtell,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { ";",              do_gtell,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "note",           do_note,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "pose",           do_pose,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "reply",          do_reply,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "say",            do_say,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "'",              do_say,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "shout",          do_shout,       POS_RESTING,     3,  LOG_NORMAL ,NULL},
    { "voice",          do_voice,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { ":",              do_channel_talk,     POS_DEAD,   0,  LOG_NORMAL,NULL},
    { "talk",           do_channel_talk,     POS_DEAD,   0,  LOG_NORMAL,NULL},
    { "immtalk",        do_immtalk,     POS_SLEEPING,    97,  LOG_NORMAL ,NULL},
    { "invoke",        do_reign,     POS_SLEEPING,    96,  LOG_NORMAL ,NULL},
    { "reign",        do_reign,     POS_SLEEPING,    97,  LOG_NORMAL ,NULL},
    { "]",              do_immtalk,     POS_SLEEPING,    97,  LOG_NORMAL ,NULL},

    /*
     * Object manipulation commands.
     */
    { "brandish",       do_brandish,    POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "close",          do_close,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "drink",          do_drink,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "drop",           do_drop,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "eat",            do_eat,         POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "forage",         do_forage,      POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "fill",           do_fill,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "give",           do_give,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "hold",           do_wear,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "list",           do_list,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "lock",           do_lock,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "open",           do_open,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "pick",           do_pick,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "put",            do_put,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "quaff",          do_quaff,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "recite",         do_recite,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "remove",         do_remove,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "sell",           do_sell,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "take",           do_get,         POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "sacrifice",      do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "value",          do_value,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "evaluate",       do_evaluate,    POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "wear",           do_wear,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "zap",            do_zap,         POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "plant",          do_plant,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "forge",          do_forge,       POS_RESTING,     0,  LOG_NORMAL ,NULL},

    /*
     * Combat commands.
     */
    { "backstab",       do_backstab,    POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "bs",             do_backstab,    POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "disarm",         do_disarm,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "flee",           do_flee,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "kick",           do_kick,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "bash",           do_bash,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "doorbash",       do_bashdoor,    POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "martialarts",    do_martial_arts,POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "murde",          do_murde,       POS_FIGHTING,    5,  LOG_NORMAL ,NULL},
    { "murder",         do_murder,      POS_FIGHTING,    5,  LOG_ALWAYS ,NULL},
    { "assassin",       do_assassin,    POS_FIGHTING,    0,  LOG_ALWAYS ,NULL},
    { "assassinate",    do_assassinate, POS_FIGHTING,    0,  LOG_ALWAYS ,NULL},
    { "rescue",         do_rescue,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "shoot",          do_shoot,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "notice",         do_notice,      POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "speak",          do_speak,       POS_DEAD,    	 0,  LOG_NORMAL ,NULL},
    { "delete",         do_delete,      POS_DEAD,        0,  LOG_NEVER  ,NULL},
    { "protect",        do_protect,     POS_STANDING,    25, LOG_NEVER  ,NULL},
    { "suicide",        do_suicide,     POS_DEAD,        0,  LOG_NEVER  ,NULL},
    { "throw",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "divert",         do_divert,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "knife",          do_knife,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "disguise",       do_disguise,    POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "spy",            do_spy,         POS_STANDING,    0,  LOG_ALWAYS ,NULL},
    { "distract",       do_distract,    POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "snatch",         do_snatch,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "berserk",        do_berserk,     POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "trip",           do_trip,        POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "gouge",          do_gouge,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "circle",         do_circle,       POS_FIGHTING,    0,  LOG_NORMAL ,NULL},

    /*
     * Miscellaneous commands.
     */
    { "follow",         do_follow,      POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "shadow",         do_shadow,      POS_RESTING,     0,  LOG_ALWAYS ,NULL},
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "hide",           do_hide,        POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "practice",       do_practice,    POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "qui",            do_qui,         POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "quit",           do_quit,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "recall",         do_recall,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "death",          do_death,       POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "/",              do_recall,      POS_FIGHTING,    0,  LOG_NORMAL ,NULL},
    { "rent",           do_rent,        POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "save",           do_save,        POS_DEAD,        0,  LOG_NORMAL ,NULL}, 
    { "sleep",          do_sleep,       POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "sneak",          do_sneak,       POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "split",          do_split,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "steal",          do_steal,       POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "stealth",        do_stealth,     POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "makepoison",     do_make_poison, POS_STANDING,    33,  LOG_NORMAL ,NULL},
    { "makeflash",      do_make_flash,  POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "train",          do_train,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "travel",         do_travel,      POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "visible",        do_visible,     POS_DEAD  , 0,   LOG_NORMAL ,&cmd_stop},
    { "wake",           do_wake,        POS_SLEEPING,    0,  LOG_NORMAL ,NULL},
    { "where",          do_where,       POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "usage",          do_usage,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "bet",            do_bet,         POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "accept",         do_accept,      POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "resign",         do_resign,      POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "request",        do_request,     POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "clear",          do_clear_path,  POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "hunt",           do_hunt,        POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "stop",           do_visible,     POS_DEAD    , 0,LOG_NORMAL,&cmd_stop},
    { "grant",          do_grant,       POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "decline",        do_decline,     POS_RESTING,    0,  LOG_NORMAL ,NULL},
    { "camp",           do_camp,        POS_RESTING,     0,  LOG_NORMAL ,NULL},

    /*
     * MOBprogram commands.
     */
    { "mpstat",         do_mpstat,      POS_DEAD,       97,  LOG_NORMAL ,NULL},
    { "mpasound",       do_mpasound,    POS_DEAD,       99,  LOG_NORMAL ,NULL},
    { "mpareaecho",     do_mpareaecho,  POS_DEAD,       97,  LOG_NORMAL ,NULL},
    { "mpjunk",         do_mpjunk,      POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpjunkperson",   do_mpjunk_person,POS_DEAD,      98,  LOG_NORMAL ,NULL},
    { "mpecho",         do_mpecho,      POS_DEAD,       97,  LOG_NORMAL ,NULL},
    { "mpechoat",       do_mpechoat,    POS_DEAD,       97,  LOG_NORMAL ,NULL},
    { "mpechoaround",   do_mpechoaround,POS_DEAD,       97,  LOG_NORMAL ,NULL},
    { "mpkill",         do_mpkill      ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpmload",        do_mpmload     ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpoload",        do_mpoload     ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mppurge",        do_mppurge     ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpquiet",        do_mpquiet     ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpgoto",         do_mpgoto      ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpat",           do_mpat        ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mptransfer",     do_mptransfer  ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpforce",        do_mpforce     ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpmset",         do_mpmset,      POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mposet",         do_mposet,      POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpmadd",         do_mpmadd,      POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpoadd",         do_mpoadd      ,POS_DEAD,       98,  LOG_NORMAL ,NULL},
    { "mpgorand",       do_mpgorand    ,POS_DEAD,       98,  LOG_NORMAL ,NULL},

    /*
     * Immortal commands.
     */
    { "advance",        do_advance,     POS_DEAD,MAX_LEVEL,  LOG_ALWAYS ,NULL},
    { "trust",          do_trust,       POS_DEAD,MAX_LEVEL,  LOG_ALWAYS ,NULL},
    { "allow",          do_allow,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "arrest",         do_arrest,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "copyover",       do_copyover,    POS_DEAD,MAX_LEVEL-2,  LOG_ALWAYS,NULL},
    { "deny",           do_deny,        POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "disconnect",     do_disconnect,  POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "freeze",         do_freeze,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "reboo",          do_reboo,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "reboot",         do_reboot,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "rename",         do_rename,      POS_DEAD,MAX_LEVEL-1,LOG_ALWAYS,NULL},
    { "release",        do_release,     POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "shutdow",        do_shutdow,     POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "shutdown",       do_shutdown,    POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "users",          do_users,       POS_DEAD,MAX_LEVEL-2,LOG_NORMAL,NULL},
    { "wizlock",        do_wizlock,     POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "force",          do_force,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "notell",         do_notell,      POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "mload",          do_mload,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "oload",          do_oload,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "pardon",         do_pardon,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "purge",          do_purge,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "restore",        do_restore,     POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "revert",         do_revert,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "silence",        do_silence,     POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "sla",            do_sla,         POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "slay",           do_slay,        POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "slaughter",      do_slaughter,   POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "sset",           do_sset,        POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "transfer",       do_transfer,    POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "retran",   	do_retran,      POS_DEAD,MAX_LEVEL-2,LOG_NORMAL,NULL},
    { "at",             do_at,          POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "attack",         do_attack,      POS_STANDING,    0,  LOG_NORMAL ,NULL},
    { "bamfin",         do_bamfin,      POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "bamfout",        do_bamfout,     POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "echo",           do_echo,        POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "holylight",      do_holylight,   POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "timemode",       do_timemode,    POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "maze",   	do_maze,        POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "giveprac",   	do_giveprac,    POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "download",   	do_download,    POS_DEAD,0,          LOG_NORMAL,NULL},
    { "external",   	do_external,    POS_DEAD,0,          LOG_NORMAL,NULL},
    { "pquit",   	do_pquit,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "pload",   	do_pload,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "army",   	do_army,        POS_DEAD,0,          LOG_NORMAL,NULL},
    { "victory",   	do_victory_list,POS_DEAD,0,          LOG_NORMAL,NULL},
    { "war",   		do_war,         POS_DEAD,0,          LOG_NORMAL,NULL},
    { "roomfragment",   do_roomfragment,POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "most",   	do_most,        POS_DEAD,0,          LOG_NORMAL,NULL},
    { "dump",   	do_dump,        POS_DEAD,0,          LOG_NORMAL,NULL},
    { "llog",   	do_llog,        POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "map",   		do_map,         POS_DEAD,10,         LOG_NORMAL,NULL},
    { "prompt",   	do_prompt,      POS_DEAD,0,          LOG_NORMAL,NULL},
    { "reincarnate",  	do_reincarnate, POS_STANDING, 0 ,    LOG_NORMAL,NULL},
    { "reign",   	do_reign,       POS_STANDING,97,     LOG_NORMAL,NULL},
    { "tactical",   	do_tactical,    POS_DEAD,0,          LOG_NORMAL,NULL},
    { "savearea",       do_savearea,    POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "lookup", 	do_lookup,      POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "auto",   	do_auto,        POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "repeat", 	do_repeat,      POS_DEAD,  0,        LOG_NORMAL,NULL},
    { "finger", 	do_finger,      POS_DEAD,  0,  	     LOG_NORMAL,NULL},
    { "engrave",        do_engrave,     POS_STANDING,  0,    LOG_NORMAL,NULL},
    { "repair",         do_repair,      POS_STANDING,  0,    LOG_NORMAL,NULL},
    { "key",    	do_key, 	POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "rescale",        do_rescale,     POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "connect",        do_connect,     POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "mprogram",  	do_mprogram,    POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "oprogram",  	do_oprogram,    POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "shutoff",  	do_shutoff,     POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "undeny",  	do_undeny,      POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "fixpass",  	do_fixpass,     POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "test1",  	do_test1,       POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "test2",  	do_test2,       POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "invis",          do_invis,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "log",            do_log,         POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "memory",         do_memory,      POS_DEAD, MAX_LEVEL-2, LOG_NORMAL,NULL},
    { "cpu",            do_cpu,         POS_DEAD, MAX_LEVEL-2, LOG_NORMAL,NULL},
    { "strength",       do_strength,    POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "intelligence",   do_intelligence,POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "wisdom",         do_wisdom,      POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "constitution",   do_constitution,POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "dexterity",      do_dexterity,   POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "email",          do_email,       POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "mail",           do_email,       POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "html",           do_html,        POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "create",         do_create,      POS_DEAD, 0,         LOG_NORMAL,NULL},
    { "mfind",          do_mfind,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "mstat",          do_mstat,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "mlist",          do_mlist,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "mwhere",         do_mwhere,      POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "owhere",         do_owhere,      POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "ofind",          do_ofind,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "ostat",          do_ostat,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "olist",          do_olist,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "peace",          do_peace,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "recho",          do_recho,       POS_DEAD,MAX_LEVEL-3,  LOG_ALWAYS,NULL},
    { "return",         do_return,      POS_DEAD,1,          LOG_NORMAL,NULL},
    { "rstat",          do_rstat,       POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "slookup",        do_slookup,     POS_DEAD,MAX_LEVEL-3,  LOG_ALWAYS,NULL},
    { "snoop",          do_snoop,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "switch",         do_switch,      POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "tick",           do_tick,        POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "terminal",       do_terminal,    POS_DEAD,0        ,  LOG_NORMAL,NULL},
    { "termlist",       do_termlist,    POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},
    { "delcastle",      do_delcastle,   POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "hearlog",        do_hearlog,     POS_DEAD,MAX_LEVEL-2,  LOG_NORMAL,NULL},

    /* Clan commands 4/12/98 Martin */

    { "clans",          do_clans,       POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "orders",         do_orders,      POS_DEAD,        0,  LOG_NORMAL ,NULL},
    { "clanmessage",    do_send_clan_message,POS_DEAD,MAX_LEVEL-2,  LOG_ALWAYS,NULL},
    { "initiate",       do_initiate,    POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "outcast",        do_outcast,     POS_RESTING,     0,  LOG_NORMAL ,NULL},
    { "gohome",         do_gohome,      POS_RESTING,    20,LOG_NORMAL ,NULL},
    { "makeclan",       do_makeclan,    POS_RESTING,  50,LOG_ALWAYS ,NULL},
    { "setclan",        do_setclan,     POS_RESTING, 0,  LOG_NORMAL ,NULL},
    { "coffer",         do_coffer,      POS_RESTING,     20,  LOG_ALWAYS ,NULL},
    { "pledge",         do_pledge,      POS_RESTING,     20,  LOG_NORMAL ,NULL},
    { "clanwhere",      do_clanwhere,  POS_RESTING,        0,  LOG_NORMAL ,NULL},
    { "forceren",      do_forceren,  POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "forcerent",     do_forcerent, POS_DEAD,MAX_LEVEL,  LOG_ALWAYS,NULL},
    { "renounce",      do_renounce,  POS_RESTING,        20,  LOG_NORMAL ,NULL},
    { "nominate",      do_nominate,  POS_RESTING,        20,  LOG_NORMAL ,NULL},
    { "destroyclan",   do_destroy_clan,    POS_RESTING,  MAX_LEVEL-1,LOG_ALWAYS ,NULL},
    { "donate",        do_donate,    POS_STANDING,  20,LOG_NORMAL ,NULL},
    { "heal",          do_heal,      POS_DEAD,        20,  LOG_NORMAL ,NULL},

    /* OLC stuff 28/11/98  Martin*/
    { "mcreate",      do_mcreate,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "ocreate",      do_ocreate,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "mdelete",      do_mdelete,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "odelete",      do_odelete,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "rdelete",      do_rdelete,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "massign",      do_massign,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "oassign",      do_oassign,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "rassign",      do_rassign,  POS_RESTING, MAX_LEVEL-1,  LOG_NORMAL ,NULL},
    { "mset",         do_mset,        POS_DEAD, MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "oset",         do_oset,        POS_DEAD, MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "rset",         do_rset,        POS_DEAD, MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "door",         do_door,        POS_DEAD, MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "doorset",      do_door,        POS_DEAD, MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "goto",         do_goto,        POS_DEAD, MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "regoto",       do_regoto,      POS_DEAD, MAX_LEVEL-1,LOG_NORMAL,NULL},

    { "aset",  	      do_aset,        POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "astat",        do_astat,        POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},

    { "medit",        do_medit,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "oedit",        do_oedit,       POS_DEAD,MAX_LEVEL-1,  LOG_ALWAYS,NULL},
    { "redit",        do_redit,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "hedit",        do_hedit,       POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "mpedit",       do_mpedit,       POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "reset",        do_reset,   POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "rreset",       do_rreset,   POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "resetarea",  	do_resetarea,   POS_DEAD,MAX_LEVEL-1,  LOG_NORMAL,NULL},
    { "resetquest",   do_resetquest,  POS_DEAD,MAX_LEVEL-3,  LOG_ALWAYS,NULL},
    { "instaroom",    do_instaroom,   POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    { "instazone",    do_instazone,   POS_DEAD,MAX_LEVEL,  LOG_NORMAL,NULL},
    /*
     * End of list.
     */
    { "",               0,              POS_DEAD,        0,  LOG_NORMAL ,NULL}
};



/*
 * The social table.
 * Add new socials here.
 * Alphabetical order is not required.
 */
const   struct  social_type     social_table [] =
{
    {
	"accuse",
	"Accuse whom?",
	"$n is in an accusing mood.",
	"You look accusingly at $M.",
	"$n looks accusingly at $N.",
	"$n looks accusingly at you.",
	"You accuse yourself.",
	"$n seems to have a bad conscience."
    },

    {
	"ack",
	"You gasp and say 'ACK!' at your mistake.",
	"$n ACKS at $s big mistake.",
	"You ACK $M.",
	"$n ACKS $N.",
	"$n ACKS you.",
	"You ACK yourself.",
	"$n ACKS $mself.  Must be a bad day."
    },


    {
        "addict",
        "You stand and admit to all in the room, 'Hi, I'm $n, and I'm a 
mud addict.'",
        "$n stands and says, 'Hi, I'm $n, and I'm a mud addict.'",
        "You tell $N that you are addicted to $S love.",
        "$n tells $N that $e is addicted to $S love.",
        "$n tells you that $e is addicted to your love.",
        "You stand and admit to yourself that you are a mud addict. Maybe you 
should tell everyone else now?",
        "$n is mumbling to himself, $e's been mudding to long probably."
    },

    {
	"applaud",
	"Clap, clap, clap.",	 
	"$n gives a round of applause.",
	"You clap at $S actions.",
	"$n claps at $N's actions.",
	"$n gives you a round of applause.  You MUST'VE done something good!",
	"You applaud at yourself.  Boy, are we conceited!",
	"$n applauds at $mself.  Boy, are we conceited!"
    },

    {
	"backpat",
	"Huh?",
	NULL,
	"You pat $M on the back for a job well done.",
	"$n pats $N on the back.",
	"$n pats you on the back for a job well done.",
	"You pat yourself on the back vainly.",
	"$n vainly pats $mself on the back."
     },

    {
	"bark",
	"Woof!  Woof!",
	"$n barks like a dog.",
	"You bark at $M.",
	"$n barks at $N.",
	"$n barks at you.",
	"You bark at yourself.  Woof!  Woof!",
	"$n barks at $mself.  Woof!  Woof!"
    },

    {
	"beer",
	"You down a cold, frosty beer.",
	"$n downs a cold, frosty beer.",
	"You draw a cold, frosty beer for $N.",
	"$n draws a cold, frosty beer for $N.",
	"$n draws a cold, frosty beer for you.",
	"You draw yourself a beer.",
	"$n draws $mself a beer."
    },

    {
        "bleed",
        "You bleed all over the room!",
        "$n bleeds all over the room!  Get out of $s way!",
        "You bleed all over $M!",
        "$n bleeds all over $N.  Better leave, you may be next!",
        "$n bleeds all over you!  YUCK!",
        "You bleed all over yourself!",
        "$n bleeds all over $mself."
    },

    {
	"beg",
	"You beg the gods for mercy.",
	"The gods fall down laughing at $n's request for mercy.",
	"You desperately beg $M for help.",
	"$n begs $N for assistance!",
	"$n begs you for help.",
	"Begging yourself for help wouldn't be very smart.",
	"$n begs $mself for help, how ingenious."
    },

    {
	"blink",
	"You blink in utter disbelief.",
	"$n blinks in utter disbelief.",
	"You blink at $M in confusion.",
	"$n blinks at $N in confusion.",
	"$n blinks at you in confusion.",
	"You are sooooooooooooo confused.",
	"$n blinks at $mself in complete confusion."
    },

    {
	"blush",
	"Your cheeks are burning.",
	"$n blushes.",
	"Your cheecks turn red seeing $M here.",
	"$n blushes as $e sees $N here.",
	"$n blushes as $e sees you here.  Such an effect on people!",
	"You blush at your actions.",
	"$n blushes at $e's actions."
    },

    {
    "bong",
    "Are you just gonna stare at it?",
    NULL,
    "You pass the bong to $M.",
    "$n passes the bong to $N.",
    "$n passes you the bong.",
    "You light a bong and take a hit.",
    "$n lights a bong and takes a hit."
    },

    {
	"bounce",
	"BOIINNNNNNGG!",
	"$n bounces around happily.",
	"You bounce onto $S lap.",
	"$n bounces onto $N's lap.",
	"$n bounces onto your lap.",
	"You bounce your head like a basketball.",
	"$n plays basketball with $s head."
    },

    {
	"bow",
	"You bow deeply.",
	"$n bows deeply flourishing $s cape.",
	"You bow before $M.",
	"$n bows before $N.",
	"$n bows before you, flourishing $s cape.",
	"You kiss your toes; how flexable.",
	"$n folds up like a jack knife and kisses $s own toes."
    },

    {
        "bungy",
        "You tie a bungy cord to the mud and jump into the internet.",
        "$n ties a bungy cord to the mud and jumps into the internet.",
        "You tie a bungy cord to $N and throw $M off the mud .",
        "$n ties a bungy cord to $N and throws $M off the mud.",
        "$n ties a bungy cord to you and throws you off the mud.",
        "You tie a bungy cord to yourself and jump off the mud .",
        "$n ties a bungy cord to $mself and jumps off the mud."
    },

    {
	"burp",
	"Excuse yourself.",
	"$n burps loudly.",
	"You burp loudly to $M in response.",
	"$n burps loudly in response to $N's remark.",
	"$n burps loudly in response to your remark.",
	"You burp at yourself.",
	"$n burps at $mself. How rude."
    },

    {
        "bye",
        "You say goodbye to all in the room.",
        "$n says goodbye to everyone in the room, including you!",
        "You say goodbye to all in the room, and especially to $N.",
        "$n says goodbye to everyone in the room, including you!",
        "$n says goodbye to everyone in the room, and especially you!",
        "You say goodbye to all in the room.",
        "$n says goodbye to everyone in the room, including you!"
    },

    {
	"cackle",
	"You throw back your head and cackle with insane glee!",
	"$n throws back $s head and cackles with insane glee!",
	"You cackle gleefully at $N",
	"$n cackles gleefully at $N.",
	"$n cackles gleefully at you.  Better keep your distance from $m.",
	"You cackle at yourself.  Now, THAT'S strange!",
	"$n is really crazy now!  $e cackles at $mself."
    },
   
    {
	"cap",
	"You tip your cap gallantly.",
	"$n tips $s cap gallantly.",
	"You tip your cap to $N gallantly.",
	"$n tips $s cap gallantly for $N.",
	"$n tips $s cap to you gallantly.",
	"You tip your cap over your head for a nap.",
	"$n tips $s cap over $s head for a nap."
    },

    {
	"cheek",
	"Kiss whose cheek?",
	NULL,
	"You lean forward and kiss $M gently on the cheek.",
	"$n leans forward and kisses $N gently on the cheek.",
	"$n leans forward and kisses you gently on the cheek.",
	NULL,
	NULL
    },
    
    {
	"cheer",
	"You cheer with great gusto.",
	"$n cheers with great gusto.",
	"You cheer at $M with great gusto.",
	"$n cheers for $N with great gusto.",
	"$n cheers for you with great gusto.",
	"You cheer for yourself since no one else will.",
	"$n cheers for $mself since you won't."
    },

    {
	"choke",
	"Choke who?",
	NULL,
	"You grab $M by the neck and shake violently.",
	"$n grabs $N by the neck and shakes violently.",
	"$n grabs YOU by the neck and shakes violently.  Ouch!",
	"You grab your own neck, and start shaking violently.",
	"$n grabs $s own neck and shakes violently."
    }, 

    {
	"chuckle",
	"You chuckle politely.",
	"$n chuckles politely.",
	"You chuckle at $S actions.",
	"$n chuckles at $N's actions.",
	"$n chuckles at your actions.",
	"You chuckle at your own joke, since no one else would.",
	"$n chuckles at $s own joke, since none of you would."
    },

    {
	"clap",
	"You clap your hands together.",
	"$n shows $s approval by clapping $s hands together.",
	"You clap at $S performance.",
	"$n claps at $N's performance.",
	"$n claps at your performance.",
	"You clap at your own performance.",
	"$n claps at $s own performance."
    },

    {
        "clueless",
        "You chastise everyone in the room for being clueless.",
        "$n chastises everyone for being clueless.",
        "You chastise $M for being clueless.",
        "$n chastises $N for being clueless.",
        "$n chastises you for being clueless.",
        "You chastise yourself for being clueless.  What a dweeb you are!",
        "$n chastises $mslef for being clueless.  What a dweeb!"
    },

    {
	"comb",
	"You comb your hair - perfect.",
	"$n combs $s hair, how dashing!",
	"You comb $S's hair and makes it shine.",
	"$n patiently combs $N's hair and makes it shine.",
	"$n combs your hair and makes it shine.",
	"You pull your hair attempting to comb it.",
	"$n tries to comb $s tangled hair."
    },

    {
	"comfort",
	"Aww, what's wrong?",
	"$n needs someone to comfort $m.",
	"You comfort $M.",
	"$n comforts $N.",
	"$n comforts you.",
	"You make a vain attempt to comfort yourself.",
	"$n has no one to comfort $m but $mself."
    },
    {
	"cough",
	"You cough to clear your throat and eyes and nose and....",
	"$n coughs loudly.",
	"You cough loudly.  It must be $S fault, $E gave you this cold.",
	"$n coughs loudly, and glares at $N, like it is $S fault.",
	"$n coughs loudly, and glares at you.  Did you give $M that cold?",
	"You cough loudly.  Why don't you take better care of yourself?",
	"$n coughs loudly.  $n should take better care of $mself."
    },

    {
	"cringe",
	"You cringe in terror.",
	"$n cringes in terror!",
	"You cringe away from $M.",
	"$n cringes away from $N in mortal terror.",
	"$n cringes away from you.",
	"I beg your pardon?",
	NULL
    },

    {
	"cry",
	"Poor baby.",
	"$n bursts into tears.",
	"You cry on $S shoulder.",
	"$n cries on $N's shoulder.",
	"$n cries on your shoulder.",
	"You cry to yourself.",
	"$n sobs quietly to $mself."
    },

    {
	"cuddle",
	"Whom do you feel like cuddling today?",
	NULL,
	"You cuddle $M warmly.",
	"$n cuddles $N warmly, you wish someone would cuddle you.",
	"$n cuddles you warmly.",
	NULL,
	NULL
    },

    {
	"curse",
	"You swear loudly.",
	"$n swears: @*&^%@*&!",
	"You swear at $M.",
	"$n swears at $N.",
	"$n swears at you! What did you do?",
	"You swear at your own mistakes.",
	"$n starts swearing at $mself for making a mistake."
    },

    {
	"curtsey",
	"You curtsey to your audience.",
	"$n curtseys gracefully.",
	"You curtsey to $M.",
	"$n curtseys gracefully to $N.",
	"$n curtseys gracefully for you.",
	"You curtsey to your audience (yourself).",
	"$n curtseys to $mself, since no one is paying attention to $m."
    },

    {
	"dance",
	"Feels silly, doesn't it?",
	"$n tries to break dance, but nearly breaks $s neck!",
	"You stick a rose between your teeth and sweep $M into a tango.",
	"$n sticks a rose between $s teeth and sweeps $N into a tango.",
	"$n sticks a rose between $s teeth and sweeps you into a tangos.",
	"You skip and dance around by yourself.",
	"$n dances a pas-de-une."
    },

  {
	"doh",
	"You say, 'Doh!!'  and slap yourself.  What an idiot you are!",
	"$n slaps $mself and says, 'Doh!!!'",
	"You say, 'Doh!!'  and slap yourself.  What an idiot you are!",
	"$n slaps $mself and says, 'Doh!!!'",
	"$n slaps $mself and says, 'Doh!!!'  You made $m feel dumb.  Good job!",
	"You slap yourself and say, 'Doh!!!'",
	"$n slaps $mself and says, 'Doh!!!'"
    },

    {
	"drool",
	"You drool on yourself.",
	"$n drools on $mself.",
	"You drool all over $N.",
	"$n drools all over $N.",
	"$n drools all over you.",
	"You drool on yourself.",
	"$n drools on $mself."
    },

    {
       "embrace",
        "Who do you want to hold?",
        "$n looks around for someone to hold close to $m.",
        "You hold $N in a warm and loving embrace.",
        "$n holds $N in a warm and loving embrace.",
        "$n holds you in a warm and loving embrace.",
        "You hold yourself in a warm and loving embrace.  Feels silly 
doesn't it?",
        "$n holds $mself in a warm and loving embrace ($e looks pretty 
silly, doesn't $e?)"
    },

    {
        "eskimo",
        "Who do you want to eskimo kiss with?",
        "$n is looking for someone to rub noses with.  Any volunteers?",
        "You rub noses with $M.",
        "$n rubs noses with $N.  You didn't know they were eskimos!",
        "$n rubs noses with you.",
        "You become cross-eyed from your attempts at rubbing your nose on 
yourself!",
        "$n is trying to rub $s nose on $mself.  That must hurt!"
    },

    {
        "fakerep",
        "You report: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "$n reports: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "You report: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "$n reports: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "$n reports: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "You report: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
        "$n reports: 7367/7367 hp 1049/1049 mana 2595/2595 mv 497189537 xp.",
    },

    {
	"fart",
	"Where are your manners?",
	"$n lets off a real rip-roarer ... a greenish cloud envelops $n!",
	"You fart at $M.  Boy, you are sick.",
	"$n farts in $N's direction.  Better flee before $e turns to you!",
	"$n farts in your direction.  You gasp for air.",
	"You fart at yourself.  You deserve it.",
	"$n farts at $mself.  Better $m than you."
    },

    {
	"flip",
	"You flip head over heels.",
	"$n flips head over heels.",
	"You flip $M over your shoulder.",
	"$n flips $N over $s shoulder.",
	"$n flips you over $s shoulder.  Hmmmm.",
	"You tumble all over the room.",
	"$n does some nice tumbling and gymnastics."
    },

    {
	"flirt",
	"Wink wink!",
	"$n flirts -- probably needs a date, huh?",
	"You flirt with $M.",
	"$n flirts with $N.",
	"$n wants you to show some interest and is flirting with you.",
	"You flirt with yourself.",
	"$n flirts with $mself.  Hoo boy."
    },

    {
	"fondle",
	"Who needs to be fondled?",
	NULL,
	"You fondly fondle $M.",
	"$n fondly fondles $N.",
	"$n fondly fondles you.",
	"You fondly fondle yourself, feels funny doesn't it ?",
	"$n fondly fondles $mself - this is going too far !!"
    },

    {
	"french",
	"Kiss whom?",
	NULL,
	"You give $N a long and passionate kiss.",
	"$n kisses $N passionately.",
	"$n gives you a long and passionate kiss.",
	"You gather yourself in your arms and try to kiss yourself.",
	"$n makes an attempt at kissing $mself."
    },

    {
	"frown",
	"What's bothering you ?",
	"$n frowns.",
	"You frown at what $E did.",
	"$n frowns at what $N did.",
	"$n frowns at what you did.",
	"You frown at yourself.  Poor baby.",
	"$n frowns at $mself.  Poor baby."
    },

    {
	"fume",
	"You grit your teeth and fume with rage.",
	"$n grits $s teeth and fumes with rage.",
	"You stare at $M, fuming.",
	"$n stares at $N, fuming with rage.",
	"$n stares at you, fuming with rage!",
	"That's right - hate yourself!",
	"$n clenches $s fists and stomps his feet, fuming with anger."
    },

    {
	"gasp",
	"You gasp in astonishment.",
	"$n gasps in astonishment.",
	"You gasp as you realize what $e did.",
	"$n gasps as $e realizes what $N did.",
	"$n gasps as $e realizes what you did.",
	"You look at yourself and gasp!",
	"$n takes one look at $mself and gasps in astonishment!"
    },

    {
	"giggle",
	"You giggle.",
	"$n giggles.",
	"You giggle in $N's presence.",
	"$n giggles at $N's actions.",
	"$n giggles at you.  Hope it's not contagious!",
	"You giggle at yourself.  You must be nervous or something.",
	"$n giggles at $mself looking rather jittery."
    },

    {
	"glare",
	"You glare at nothing in particular.",
	"$n glares around $m.",
	"You glare icily at $M.",
	"$n glares at $N.",
	"$n glares icily at you, you feel cold to your bones.",
	"You glare icily at your feet, they are suddenly very cold.",
	"$n glares at $s feet, what is bothering $m?"
    },

    {
	"girn",
	"You try to grin, but somehow get it slightly wrong.",
	"$n tries to grin, but somehow gets it slightly wrong.",
	"You try to grin at $N, but $E gives you a funny look.",
	"$n tries to grin at $N, but screws it up badly.",
	"$n turns $s lips in a sad attempt at a lopsided grin.",
	"Your face becomes a ghastly mask as you fail to grin.",
	"$n's face becomes a strange death mask as $s tries to grin."
    },

    {
	"grin",
	"You grin evilly.",
	"$n grins evilly.",
	"You grin evilly at $M.",
	"$n grins evilly at $N.",
	"$n grins evilly at you with a twinkle in $s eyes.",
	"You grin at yourself.  You must be getting very bad thoughts.",
	"$n grins at $mself.  You must wonder what $e is thinking."
    },

    {
	"groan",
	"You groan loudly.",
	"$n groans loudly.",
	"You groan at the sight of $M.",
	"$n groans at the sight of $N.",
	"$n groans at the sight of you.",
	"You groan as you realize what you have done.",
	"$n groans as $e realizes what $e has done."
    },

    {
	"grope",
	"Whom do you wish to grope?",
	NULL,
	"Well, what sort of noise do you expect here?",
	"$n gropes $N.",
	"$n gropes you.",
	"You grope yourself - YUCK.",
	"$n gropes $mself - YUCK."
    },

    {
	"grovel",
	"You grovel in the dirt.",
	"$n grovels in the dirt.",
	"You grovel before $M.",
	"$n grovels at $N's feet.",
	"$n grovels in the dirt at your feet.",
	"That seems a little silly to me.",
	NULL
    },

    {
	"growl",
	"Grrrrrrrrrr ...",
	"$n growls.",
	"Grrrrrrrrrr ... take that, $N!",
	"$n growls at $N.  Better leave the room before the fighting starts.",
	"$n growls at you.  Hey, two can play it that way!",
	"You growl at yourself.  Boy, do you feel bitter!",
	"$n growls at $mself.  This could get interesting..."
    },

    {
	"grumble",
	"You grumble.",
	"$n grumbles.",
	"You grumble to $M.",
	"$n grumbles to $N.",
	"$n grumbles to you.",
	"You grumble under your breath.",
	"$n grumbles under $s breath."
    },

    {
	"grunt",
	"Oink, oink.",
	"$n grunts like a pig.",
	"Oink, oink.",
	"$n grunts to $N.  What a pig!",
	"$n grunts to you.  What a pig!",
	"Oink, oink.",
	"$n grunts to nobody in particular.  What a pig!"
    },

    {
        "gulp",
        "ggggggguuuuuuulllllppppp!!!!!",
        "$n says 'That drink was nice.'",
        "$N thanks you for the drink.",
        "$N comsumes a drink from $n.",
        "$n gives you a refreshing drink which you gulp down.",
        "You gulp down a refreshing drink.",
        "$n gulps down a refreshing looking drink."
    },

    {
	"hand",
	"Kiss whose hand?",
	NULL,
	"You kiss $S hand.",
	"$n kisses $N's hand.  How sweet!",
	"$n kisses your hand.  What a sweetie!",
	"You kiss your own hand.",
	"$n kisses $s own hand."
    },

    {
	"hello",
	"You say hello to everyone in the room.",
	"$n says hello to everyone in the room.",
	"You say tell $M how truly glad you are to see $M.",
	"$n tells $N 'Hi!'",
	"$n tells you how truly glad $e is you are here.",
	"You greet yourself enthusiastically.",
	"$n greets $mself enthusiastically.  How odd."
    },

   {
	"hangover",
	"You pop a few aspirin and put on your sunglasses.  Ow, your head hurts!
",
	"$n holds $s head and says 'Quit breathing so loud!'",
	"Won't $N be quiet?  Your head is gonna split in two if $E keeps talking
!",
	"$n complains to $N 'Be quiet!  I have a hangover!",
	"$n complains to you 'Be quiet!  I have a hangover!",
	"You shoosh yourself.  Be quiet!  You have a hangover!",
	"$n shooshes $mself.  Wow, what a hangover that must be!"
     },

     {
        "highfive",
        "You jump in the air...oops, better get someone else to join you.",
        "$n jumps in the air by $mself.  Is $e a cheerleader, or just daft?",
        "You jump in the air and give $M a big highfive!",
        "$n jumps in the air and gives $N a big highfive!",
        "$n jumps in the air and gives you a big highfive!",
        "You jump in the air and congratulate yourself!",
        "$n jumps in the air and gives himself a big highfive!  Wonder 
what he did?"
    },

    {
	"hooter",
	"You light up a big, fat hooter.",
	"$n lights up a big, fat hooter.",
        "You roll a big, fat hooter for $N.",
        "$n rolls a big, fat hooter for $N.",
        "$n rolls a big, fat hooter for YOU!",
        "You roll yourself a big, fat hooter.",
        "$n rolls $mself a big, fat hooter."
    },

    {
	"hop",
	"You hop around like a little kid.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
    },

    {
	"hug",
	"Hug whom?",
	NULL,
	"You hug $M tightly.",
	"$n hugs $N tightly.",
	"$n hugs you tightly.",
	"You hug yourself.",
	"$n hugs $mself in a vain attempt to get friendship."
    },

    {
	"huggle",
	"Huggle whom?",
	NULL,
	"You huggle $M.",
	"The world seems to brighten as $n huggles $N.",
	"$n mesmerises you with love as $e envelopes you in a huggle.",
	NULL,
	NULL
    },

    {
        "hum",
        "Hmm Hmm Hmm Hmmmmmmm.",
        "$n hums like a bee with a chest cold.",
        "You hum a little ditty for $M.  Hmm Hmm Hmm Hmmmmmm.",
        "$n hums a little ditty for $N.  Hmm Hmm Hmm Hmmmmmm.",
        "$n hums a little ditty for you.  Hmm Hmm Hmm Hmmmmmm.",
        "Hmm Hmm Hmmmmmmm.",
        "$n hums like a bee with a chest cold."
    },

    {
	"kiss",
	"Isn't there someone you want to kiss?",
	NULL,
	"You kiss $M.",
	"$n kisses $N.",
	"$n kisses you.",
	"All the lonely people :(",
	NULL
    },

    {
	"laugh",
	"You laugh.",
	"$n laughs.",
	"You laugh at $N mercilessly.",
	"$n laughs at $N mercilessly.",
	"$n laughs at you mercilessly.  Hmmmmph.",
	"You laugh at yourself.  I would, too.",
	"$n laughs at $mself.  Let's all join in!!!"
    },

    {
	"lag",
	"You complain about the terrible lag.",
	"$n starts complaining about the terrible lag.",
	"You complain to $N about the terrible lag.",
	"$n complains to $N about the terrible lag.",
	"$n complains to you about the terrible lag.",
	"You start muttering about the awful lag.",
	"$n starts muttering about the awful lag."
    },

    {
	"lick",
	"You lick your lips and smile.",
	"$n licks $s lips and smiles.",
	"You lick $M.",
	"$n licks $N.",
	"$n licks you.",
	"You lick yourself.",
	"$n licks $mself - YUCK."
    },

    {
	"love",
	"You love the whole world.",
	"$n loves everybody in the world.",
	"You tell your true feelings to $N.",
	"$n whispers softly to $N.",
	"$n whispers to you sweet words of love.",
	"Well, we already know you love yourself (lucky someone does!)",
	"$n loves $mself, can you believe it ?"
    },

    {
        "maim",
        "Who do you want to maim?",
        "$n is looking for someone to maim.",
        "You maim $N with your dull fingernails.",
        "$n raises $s hand and tries to maim $N to pieces.",
        "$n raises $s hand and paws at you.  You've been maimed!",
        "You maim yourself with your dull fingernails.",
        "$n raises $s hand and maims $mself to pieces."
    },

    {
	"massage",
	"Massage what?  Thin air?",
	NULL,
	"You gently massage $N's shoulders.",
	"$n massages $N's shoulders.",
	"$n gently massages your shoulders.",
	"You practice yoga as you try to massage yourself.",
	"$n gives a show on yoga positions, trying to massage $mself."
    },

    {
	"moan",
	"You start to moan.",
	"$n starts moaning.",
	"You moan for the loss of $m.",
	"$n moans for the loss of $N.",
	"$n moans at the sight of you.  Hmmmm.",
	"You moan at yourself.",
	"$n makes $mself moan."
    },

    {
    "muhaha",
    "MUHAHAHAHAHA",
    "$n laughs diabolically.  MUHAHAHAHAHA!",
    "You laugh at $M diabolically.",
    "$n laughs at $N diabolically.  MUHAHAHAHAHA!",
    "$n laughs at you diabolically.  MUHAHAHAHAHA!",
    NULL,
    NULL
    },

    {
	"nibble",
	"Nibble on whom?",
	NULL,
	"You nibble on $N's ear.",
	"$n nibbles on $N's ear.",
	"$n nibbles on your ear.",
	"You nibble on your OWN ear.",
	"$n nibbles on $s OWN ear."
    },

    {
	"nod",
	"You nod in approval.",
	"$n nods $s head in approval.",
	"You nod in recognition to $M.",
	"$n nods in recognition to $N.",
	"$n nods in recognition to you.  You DO know $m, right?",
	"You nod at yourself.  Are you getting senile?",
	"$n nods at $mself.  $e must be getting senile."
    },

    {
	"nudge",
	"Nudge whom?",
	NULL,
	"You nudge $M.",
	"$n nudges $N.",
	"$n nudges you.",
	"You nudge yourself, for some strange reason.",
	"$n nudges $mself, to keep $mself awake."
    },

    {
	"nuzzle",
	"Nuzzle whom?",
	NULL,
	"You nuzzle $S neck softly.",
	"$n softly nuzzles $N's neck.",
	"$n softly nuzzles your neck.",
	"I'm sorry, friend, but that's impossible.",
	NULL
    },

    {
	"ogle",
	"Whom do you want to ogle?",
	NULL,
	"You ogle $M like $E was a piece of meat.",
	"$n ogles $N.  Maybe you should leave them alone for awhile?",
	"$n ogles you.  Guess what $e is thinking about?",
	"You ogle yourself.  You may just be too wierd for this mud.",
	"$n ogles $mself.  Better hope that $e stops there."
    },

    {
        "ohno",
        "Oh no!  You did it again!",
        "Oh no!  $n did it again!",
        "You exclaim, 'Oh no!  I did it again!'",
        "$n exclaims to $N, 'Oh no!  I did it again!'",
        "$n exclaims to you, 'Oh no!  I did it again!'",
        "You exclaim to yourself, 'Oh no!  I did it again!'",
        "$n mumbles to himself, 'Oh no!  I did it again!'"
    },

    {
	"pat",
	"Pat whom?",
	NULL,
	"You pat $N on $S head.",
	"$n pats $N on $S head.",
	"$n pats you on your head.",
	"You pat yourself on your back, great job!",
	"$n pats $mself on the back for $s accomplishment."
    },

    {
	"peer",
	"You peer intently about your surroundings.",
	"$n peers intently about the area, looking for thieves no doubt.",
	"You peer at $M quizzically.",
	"$n peers at $N quizzically.",
	"$n peers at you quizzically.",
	"You peer intently about your surroundings.",
	"$n peers intently about the area, looking for thieves no doubt."
    },

    {
	"point",
	"Point at whom?",
	NULL,
	"You point at $M accusingly.",
	"$n points at $N accusingly.",
	"$n points at you accusingly.",
	"You point proudly at yourself.",
	"$n points proudly at $mself."
    },

    {
	"poke",
	"Poke whom?",
	NULL,
	"You poke $M in the ribs.",
	"$n pokes $N in the ribs.",
	"$n pokes you in the ribs.",
	"You poke yourself in the ribs, feeling very silly.",
	"$n pokes $mself in the ribs, looking very sheepish."
    },

    {
	"ponder",
	"You ponder the question.",
	"$n sits down and thinks deeply.",
	"You ponder what $E is thinking.",
	"$n tries to ponder what $N is thinking.",
	"$n ponders what you are thinking, want to help?",
	"If you don't know, who does?",
	NULL
    },
    {
	"possum",
	"You do your best imitation of a corpse.",
	"$n hits the ground... DEAD.",
	"You do your best imitation of a corpse.",
	"$n hits the ground... DEAD.",
	"$n hits the ground... DEAD.",
	"You do your best imitation of a corpse.",
	"$n hits the ground... DEAD."
    },


    {
	"pout",
	"Ah, don't take it so hard.",
	"$n pouts.",
	"You pout at $M through lowered lashes.",
	"$n pouts at $N.",
	"$n pouts at you through lowered lashes.",
	NULL,
	NULL
    },

    {
	"pray",
	"You feel righteous, and maybe a little foolish.",
	"$n begs and grovels to the powers that be.",
	"You crawl in the dust before $M.",
	"$n falls down and grovels in the dirt before $N.",
	"$n kisses the dirt at your feet.",
	"Talk about narcissism ...",
	"$n mumbles a prayer to $mself."
    },
  
    {
	"pretend",
	"You pretend you are the Imp, and slay everyone in sight!",
	"$n is pretending $e is the implementor again.  *sigh*",
	"You pretend you are Zolstead, and you demote $N to level 1.",
	"$n pretends $e is Zolstead, and says, '$N, you're demoted to level 1!'",
	"$n pretends $e is Zolstead, and says, 'You are demoted to level 1!'",
	"You pretend you are the implementor, and you demote yourself to level 1
.",
	"$n pretends $e is Zolstead, and demotes $mself to level 1."
    },


    {
	"puke",
	"You puke ... chunks everywhere!",
	"$n pukes.",
	"You puke on $M.",
	"$n pukes on $N.",
	"$n spews vomit and pukes all over your clothing!",
	"You puke on yourself.",
	"$n pukes on $s clothes."
    },

    {
	"punch",
	"Punch whom?",
	NULL,
	"You punch $M playfully.",
	"$n punches $N playfully.",
	"$n punches you playfully.  OUCH!",
	"You punch yourself.  You deserve it.",
	"$n punches $mself.  Why don't you join in?"
    },

    {
	"purr",
	"MMMMEEEEEEEEOOOOOOOOOWWWWWWWWWWWW.",
	"$n purrs contentedly.",
	"You purr contentedly rubbing against $N's leg.",
	"$n purrs contentedly brushing against $N's leg.",
	"$n purrs contentedly rubbing against your leg.",
	"You purr at yourself.",
	"$n purrs at $mself.  Must be a cat thing."
    },

  {
	"renandstimpy",
	"You say, 'Oh Happy Happy, Joy Joy!'",
	"$n exclaims, 'Oh Happy Happy, Joy Joy!'",
	"You exclaim, 'Oh Happy Happy, Joy Joy!' at the mere thought of $M.",
	"$n exclaims, 'Oh Happy Happy, Joy Joy!' as $e sees $N enter the room.",
	"$n exclaims, 'Oh Happy Happy, Joy Joy!' when $e sees you approach.",
	"You exclaim, 'Oh Happy Happy, Joy Joy!' at the thought of yourself.",
	"$n exclaims, 'Oh Happy Happy, Joy Joy!' at the thought of $mself."
    },

  {
	"rofl",
	"You roll around on the floor laughing hysterically.",
	"$n rolls around on the floor laughing hysterically.",
	"You roll around on the floor laughing hysterically in front of $M.",
	"$n rolls around on the floor laughing hysterically in front of $N.",
	"$n rolls around on the floor laughing hysterically in front of you.",
	"You roll around on the floor laughing hysterically.",
	"$n rolls around on the floor laughing hysterically."
    },

  {
	"roll",
	"You roll your eyes.",
	"$n rolls $s eyes.",
	"You roll your eyes at $M.",
	"$n rolls $s eyes at $N.",
	"$n rolls $s eyes at you.",
	"You roll your eyes at yourself.",
	"$n rolls $s eyes at $mself."
    },

  {
	"rose",
	"Who would you like to give a rose to ?",
	"$n is looking for someone to give a rose to.",
	"You give $M a pretty rose @}-,-`-",
	"$n gives $N a pretty rose @}-,-`-",
	"$n gives you a pretty rose @}-,-`-",
	"You lament that no-one has given you any roses recently.",
	"$n is hinting that $E would like to be given a rose."
    },

    {
        "rub",
        "You rub your eyes.  How long have you been at this?",
        "$n rubs $s eyes.  $n must have been playing all day.",
        "You rub your eyes.  Has $N been playing as long as you have?",
        "$n rubs $s eyes.  $n must have been playing all day.",
        "$n rubs $s eyes.  Have you been playing as long as $m?",
        "You rub your eyes.  How long have you been at this?",
        "$n rubs $s eyes.  $n must have been playing all day."
    },

    {
	"ruffle",
	"You've got to ruffle SOMEONE.",
	NULL,
	"You ruffle $N's hair playfully.",
	"$n ruffles $N's hair playfully.",
	"$n ruffles your hair playfully.",
	"You ruffle your hair.",
	"$n ruffles $s hair."
    },

    {
	"scream",
	"ARRRRRRRRRRGH!!!!!",
	"$n screams loudly!",
	"ARRRRRRRRRRGH!!!!!  Yes, it MUST have been $S fault!!!",
	"$n screams loudly at $N.  Better leave before $n blames you, too!!!",
	"$n screams at you!  That's not nice!  *sniff*",
	"You scream at yourself.  Yes, that's ONE way of relieving tension!",
	"$n screams loudly at $mself!  Is there a full moon up?"
    },

    {
	"seduce",
	"Whom did you wish to seduce?",
	NULL,
	"Well, it's worth a try...",
	"$n tries to seduce $N with a smile.",
	"$n smiles seductively at you with darkened eyes.",
	"Umm, are you sure about that?",
	NULL
    },

    {
	"shake",
	"You shake your head.",
	"$n shakes $s head.",
	"You shake $S hand.",
	"$n shakes $N's hand.",
	"$n shakes your hand.",
	"You are shaken by yourself.",
	"$n shakes and quivers like a bowl full of jelly."
    },

    {
	"shiver",
	"Brrrrrrrrr.",
	"$n shiver contentedly.",
	"You shiver at the thought of fighting $M.",
	"$n shivers at the thought of fighting $N.",
	"$n shivers at the suicidal thought of fighting you.",
	"You shiver to yourself?",
	"$n shivers because $e's cold."
    },

    {
	"shrug",
	"You shrug.",
	"$n shrugs helplessly.",
	"You shrug in response to $s question.",
	"$n shrugs in response to $N's question.",
	"$n shrugs in response to your question.",
	"You shrug to yourself.",
	"$n shrugs to $mself.  What a strange person."
    },

    {
	"sigh",
	"You sigh.",
	"$n sighs loudly.",
	"You sigh as you think of $M.",
	"$n sighs at the sight of $N.",
	"$n sighs as $e thinks of you.  Touching, huh?",
	"You sigh at yourself.  You MUST be lonely.",
	"$n sighs at $mself.  What a sorry sight."
    },

    {
	"sing",
	"You raise your clear voice towards the sky.",
	"$n has begun to sing a bit offkey.",
	"You sing a ballad to $M.",
	"$n sings a ballad to $N.",
	"$n sings a ballad to you!  How sweet!",
	"You sing a little ditty to yourself.",
	"$n sings a little ditty to $mself."
    },

    {
        "slap",
        "Slap whom?",
        NULL,
        "You slap $N on the face.",
        "$n slaps $N on the face.",
        "$n slaps you on your face.",
        "You slap yourself HARD!",
        "$n slaps $mself repeatedly, $e seems to enjoy it!"
    },

    {
        "smack",
        "Smack whom?",
        NULL,
        "You smack $N upside the head.",
        "$n smacks $N upside the head.",
        "$n smacks you upside the head.",
        "You smack yourself HARD!",
        "$n smacks $mself upside the head, what an idiot!"
    },

    {
	"smile",
	"You smile happily.",
	"$n smiles happily.",
	"You smile at $M.",
	"$n beams a smile at $N.",
	"$n smiles at you.",
	"You smile at yourself.",
	"$n smiles at $mself."
    },

    {
	"smirk",
	"You smirk.",
	"$n smirks.",
	"You smirk at $S saying.",
	"$n smirks at $N's saying.",
	"$n smirks at your saying.",
	"You smirk at yourself.  Okay ...",
	"$n smirks at $s own 'wisdom'."
    },

    {
	"snap",
	"PRONTO ! You snap your fingers.",
	"$n snaps $s fingers.",
	"You snap you fingers at $M.",
	"$n snaps $s fingers at $N impatiently.",
	"$n snaps $s fingers at you impatiently. C'mon already!",
	"You snap yourself to attention.",
	"$n snaps $mself to attention."
    },

    {
	"snarl",
	"You grizzle your teeth and look mean.",
	"$n snarls angrily.",
	"You snarl at $M.",
	"$n snarls at $N.",
	"$n snarls at you in anger.",
	"You snarl at yourself.",
	"$n snarls at $mself."
    },

    {
	"sneeze",
	"Gesundheit!",
	"$n sneezes.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
    },

    {
	"snicker",
	"You snicker softly.",
	"$n snickers softly.",
	"You snicker with $M about your shared secret.",
	"$n snickers with $N about their shared secret.",
	"$n snickers with you about your shared secret.",
	"You snicker at your own evil thoughts.",
	"$n snickers at $s own evil thoughts."
    },

    {
	"sniff",
	"You sniff sadly. *SNIFF*",
	"$n sniffs sadly.",
	"You sniff sadly at the way $E is treating you.",
	"$n sniffs sadly at the way $N is treating $m.",
	"$n sniffs sadly at the way you are treating $m.",
	"You sniff sadly at your lost opportunities.",
	"$n sniffs sadly at $mself.  Something MUST be bothering $m."
    },

    {
	"snore",
	"Zzzzzzzzzzzzzzzzz.",
	"$n snores loudly.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
    },

    {
	"snowball",
	"Whom do you want to throw a snowball at?",
	NULL,
	"You throw a snowball in $N's face.",
	"$n throws a snowball at $N.",
	"$n throws a snowball at you.  Splat!",
	"You throw a snowball at yourself.",
	"$n throws a snowball at $mself."
    },

    {
	"snuggle",
	"Who?",
	NULL,
	"You snuggle $M.",
	"$n snuggles up to $N.",
	"$n snuggles up to you affectionately.",
	"You snuggle up, getting ready to sleep.",
	"$n snuggles up, getting ready to sleep."
    },

    {
	"spank",
	"Spank whom?",
	NULL,
	"You spank $M playfully.",
	"$n spanks $N playfully.",
	"$n spanks you playfully.  OUCH!",
	"You spank yourself.  Kinky!",
	"$n spanks $mself.  Kinky!"
    },

    {
	"spit",
	"You spit.",
	"$n spits.",
	"You spit on $N.",
	"$n spits on $N.",
	"$n spits on you.",
	"You spit on yourself.  Always knew you were a drooler.",
	"$n spits on $mself."
    },


    {
	"squeeze",
	"Where, what, how, whom?",
	NULL,
	"You squeeze $M fondly.",
	"$n squeezes $N fondly.",
	"$n squeezes you fondly.",
	"You squeeze yourself - try to relax a little!",
	"$n squeezes $mself."
    },

    {
	"stare",
	"You stare at the sky.",
	"$n stares at the sky.",
	"You stare dreamily at $N, completely lost in $S eyes..",
	"$n stares dreamily at $N.",
	"$n stares dreamily at you, completely lost in your eyes.",
	"You stare dreamily at yourself - enough narcissism for now.",
	"$n stares dreamily at $mself - NARCISSIST!"
    },

    {
	"strut",
	"Strut your stuff.",
	"$n struts proudly.",
	"You strut to get $S attention.",
	"$n struts, hoping to get $N's attention.",
	"$n struts, hoping to get your attention.",
	"You strut to yourself, lost in your own world.",
	"$n struts to $mself, lost in $s own world."
    },

    {
        "suffer",
        "No xp again?  You suffer at the hands of fate.",
        "$n is suffering.  Looks like $e can't seem to level.",
        "You tell $N how you suffer whenever you're away from $M.",
        "$n tells $N that $e suffers whenever they're apart.",
        "$n tells you that $e suffers whenever you're apart.",
        "No xp again?  You suffer at the hands of fate.",
        "$n is suffering.  Looks like $e can't seem to level."
    },

    {
	"sulk",
	"You sulk.",
	"$n sulks in the corner.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
    },

   {
	"tap",
	"You tap your foot impatiently.",
	"$n taps $s foot impatiently.",
	"You tap your foot impatiently.  Will $N ever be ready?",
	"$n taps $s foot impatiently as $e waits for $N.",
	"$n taps $s foot impatiently as $e waits for you.",
	"You tap yourself on the head.  Ouch!",
	"$n taps $mself on the head."
    },


    {
	"thank",
	"Thank you too.",
	NULL,
	"You thank $N heartily.",
	"$n thanks $N heartily.",
	"$n thanks you heartily.",
	"You thank yourself since nobody else wants to !",
	"$n thanks $mself since you won't."
    },

    {
        "tender",
        "You will enjoy it more if you choose someone to kiss.",
        NULL,
        "You give $M a soft, tender kiss.",
        "$n gives $N a soft, tender kiss.",
        "$n gives you a soft, tender kiss.",
        "You'd better not, people may start to talk!",
        NULL
     },

    {
	"tickle",
	"Whom do you want to tickle?",
	NULL,
	"You tickle $N.",
	"$n tickles $N.",
	"$n tickles you - hee hee hee.",
	"You tickle yourself, how funny!",
	"$n tickles $mself."
    },

    {
        "tummy",
        "You rub your tummy and wish you'd bought a pie at the bakery.",
        "$n rubs $s tummy and wishes $e'd bought a pie at the bakery.",
        "You rub your tummy and ask $M for some food.",
        "$n rubs $s tummy and asks $N for some food.",
        "$n rubs $s tummy and asks you for some food.  Please?",
        "You rub your tummy and wish you'd bought a pie at the bakery.",
        "$n rubs $s tummy and wishes $e'd bought a pie at the bakery."
    },

    {
	"twiddle",
	"You patiently twiddle your thumbs.",
	"$n patiently twiddles $s thumbs.",
	"You twiddle $S ears.",
	"$n twiddles $N's ears.",
	"$n twiddles your ears.",
	"You twiddle your ears like Dumbo.",
	"$n twiddles $s own ears like Dumbo."
    },

    {
	"wave",
	"You wave.",
	"$n waves happily.",
	"You wave goodbye to $N.",
	"$n waves goodbye to $N.",
	"$n waves goodbye to you.  Have a good journey.",
	"Are you going on adventures as well?",
	"$n waves goodbye to $mself."
    },

    {
	"whistle",
	"You whistle appreciatively.",
	"$n whistles appreciatively.",
	"You whistle at the sight of $M.",
	"$n whistles at the sight of $N.",
	"$n whistles at the sight of you.",
	"You whistle a little tune to yourself.",
	"$n whistles a little tune to $mself."
    },

    {
        "whew",
        "What a relief.",
        "$n wipes $e brow and exclaims, 'Whew, that was close.'",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    {
	"wiggle",
	"Your wiggle your bottom.",
	"$n wiggles $s bottom.",
	"You wiggle your bottom toward $M.",
	"$n wiggles $s bottom toward $N.",
	"$n wiggles his bottom toward you.",
	"You wiggle about like a fish.",
	"$n wiggles about like a fish."
    },

    {
	"wince",
	"You wince.  Ouch!",
	"$n winces.  Ouch!",
	"You wince at $M.",
	"$n winces at $N.",
	"$n winces at you.",
	"You wince at yourself.  Ouch!",
	"$n winces at $mself.  Ouch!"
    },

    {
	"wink",
	"You wink suggestively.",
	"$n winks suggestively.",
	"You wink suggestively at $N.",
	"$n winks at $N.",
	"$n winks suggestively at you.",
	"You wink at yourself ?? - what are you up to ?",
	"$n winks at $mself - something strange is going on..."
    },

    {
	"yawn",
	"You must be tired.",
	"$n yawns tiredly.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
    },


    {
	"",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
};

int get_game_realtime( void )
   {
    struct timeval last_time;

    last_time.tv_usec=0;
    last_time.tv_sec=0;
    gettimeofday( &last_time, NULL );
    return((int) ( last_time.tv_sec % (60*60*12) ));
    }
  

int get_game_usec( void )
   {
    struct timeval last_time;

    last_time.tv_usec=0;
    last_time.tv_sec=0;
    gettimeofday( &last_time, NULL );
    return( last_time.tv_usec + 1000000 * last_time.tv_sec );
    }

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */

extern char lastplayercmd[MAX_INPUT_LENGTH*2];

void interpret( CHAR_DATA *ch, char *argue )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char pag_buf[ MAX_STRING_LENGTH ];
    char buf[MAX_STRING_LENGTH];
    char argument_buf[MAX_INPUT_LENGTH];
    char *argument;
    int cmd = -1, delta;
    int leng;
    int trust;
    bool found;
    int last_time;
    bool QUIT_CMD;
    int largument;
    char *pt, *pti, *pto;
    bool auto_command;

    DISALLOW_SNOOP = FALSE;
    if( ch == NULL )
      {
      bug( "Interp: Null Character.",0);
      return;
      }

    if( ch->in_room == NULL )
      {
      sprintf(buf,"Interp: Null room for %s. Removing.",ch->name);
      bug(buf,0);
      if( IS_NPC( ch ) )
        SET_BIT( ch->act, ACT_WILL_DIE);
      else
        {
        send_to_char( "By the will of the gods, you are in Chakkor.\n\r", ch);
        transference( ch, room_index[ ROOM_VNUM_TEMPLE ] );
        }
      return;
      }
     last_time = get_game_usec();

    auto_command = FALSE;
    while( *argue == 27 )
      {
      argue++;
      auto_command=TRUE;
      }
   if (ch!= NULL && argue != NULL)
    strcpy( argument_buf, argue );
    /* Strip off non printables  - Chaos 5/13/96 */
  for( pt=argument_buf; *pt!= '\0' ; pt++)
    if( *pt != '\r' && *pt!='\n' && *pt!=27 && ( *pt < 32 || *pt > 126 ) )
      *pt = ' ';
    argument = argument_buf;
/*  This is OLC stuff so you can shortcut some things while editing
    Martin 22/2/99 */
    if ( ch->substate == SUB_REPEATCMD )
    {
        DO_FUN *fun;

        if ( (fun=ch->last_cmd) == NULL )
        {
            ch->substate = SUB_NONE;
            bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
            return;
        }
        else
        {
	    for ( cmd = 0; *cmd_table[cmd].name != '\0'; cmd++ )
    	    {
             	  if(cmd_table[cmd].do_fun == fun )
        	  {
            	   found = TRUE;
            	   break;
        	  }
            }

            if ( !found )
            {
                cmd = 0;
                bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
                return;
            }
            sprintf( logline, "(%s) %s", cmd_table[cmd].name, argument );
	}
    }



   if (cmd == -1)
   {
       /* Let's not repeat commands in obj_progs, and doesn't hurt either */
    if( !IS_NPC( ch ) && ch->desc != NULL && ch->desc->character == ch )
      *ch->desc->incomm = '\0';


    if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_REPEAT ) )
      {
      if( ch->vt100 == 0 )
        sprintf( buf, "[%s]\n\r", argument );
      else
        sprintf( buf, "\033[;7m%s\033[m\n\r", argument );
      send_to_char( buf, ch );
      }


    /* Dump Command */
    if( !IS_NPC( ch ) && ch->pcdata->dump )
      {
      char jbuf[MAX_STRING_LENGTH];
      CHAR_DATA *fch;

      if( !strcasecmp( argument, "stop" ) )
        {
        send_to_char( "Dump stopped.\n\r", ch );
        ch->pcdata->dump = FALSE;
        return;
        }
    for(fch=ch->in_room->first_person ; fch!=NULL ; fch=fch->next_in_room )
       {
       if( fch->position<=POS_SLEEPING)
	 continue;
/* poll the block list of the fch */
       if(ch!=fch && blocking(fch,ch))
         continue;

       if(!can_understand(fch,ch))
	     continue;
	      
       sprintf( jbuf, "%s dumps '%s'", get_name( ch ), argument);
       if((IS_SET(fch->hook,1) && IS_SET(fch->hook,4)) || !IS_SET(fch->hook,1))
         {
         if( !IS_NPC(fch) && fch->ansi==1)
           sprintf( buf, "\033[0;%d;%dm%s", 
               fch->pcdata->color[0][9], fch->pcdata->color[1][9],
               justify( jbuf));
         else
           strcpy( buf, justify( jbuf));
         strcat( buf, "\n\r" );
         send_to_char( buf, fch);
         }
      }
    return;
    }


    /*
     * Strip leading spaces.
     */

	 /* Remove starting and ending spaces */
    while ( isspace((int)*argument) && *argument!='\0')
	    argument++;
    largument = strlen( argument );
    pt = argument + largument - 1;
    while ( *pt==' ' && largument > 0 )
        {
	*pt='\0';
        pt--;
        largument--;
        }

        /*  Send the scroll prompter out only if no inputs  -  Chaos 8/22/97 */
    if( !IS_NPC(ch) && ch->pcdata->page_mode!=0 && ch->pcdata->page_buf != NULL)
       {
       ch->pcdata->page_mode=0;
       leng=str_cpy_max( pag_buf, ansi_translate_text( ch, "{300}" ), 
                MAX_BUFFER_LENGTH );
       leng=str_apd_max( pag_buf, ch->pcdata->page_buf,leng,MAX_BUFFER_LENGTH);
       STRFREE (ch->pcdata->page_buf );
       if( *argument=='\0' )
         send_to_char(pag_buf, ch);
       else
         send_to_char((const char*)ansi_translate_text(ch,
              "{120}----------------------------[Page Buffer Canceled]-----------------------------\n\r")
            , ch);
       }

      if ( *argument == '\0' )
       {
       return;
       }

	 /* Add links to hyper-text helps */
    if( !IS_NPC(ch) && ch->pcdata->help_mode )
       {
       ch->pcdata->help_mode=FALSE;
       if(argument[1]=='\0' && argument[0]!='\0')
	 {
	 HELP_MENU_DATA *menu;
	 HELP_MENU_DATA *prev_menu=NULL;
	 HELP_MENU_DATA *menu2;
	 HELP_DATA      *help;
         bool   foundh, foundh2;
           foundh=FALSE;
	   help=ch->pcdata->last_help;
	   for(menu=help->menu; menu!=NULL; menu=menu->next)
             {
	     if( menu->option == '-' )
               foundh=TRUE;
             prev_menu=menu;
             if(prev_menu==NULL)
               break;
             }
         if( argument[0] == '-' && !foundh && ch->pcdata->prev_help != NULL )
             {
	     send_to_char_color( ch->pcdata->prev_help->text,ch);
	     ch->pcdata->last_help=ch->pcdata->prev_help;
	     ch->pcdata->help_mode=TRUE;
             ch->pcdata->prev_help=NULL;
	     return;
	     }
	 for(menu=help->menu; menu!=NULL; menu=menu->next)
	   {
	   if( argument[0]==menu->option )
	     {
	     send_to_char_color( menu->help->text, ch);
             foundh2= FALSE;
	     for(menu2=menu->help->menu; menu2!=NULL; menu2=menu2->next)
	       if( menu2->option == '-' )
                 foundh2=TRUE;
             if( !foundh2 && argument[0] != '-' )
               send_to_char( "  (-)  Return\n\r", ch );
            if( argument[0] == '-' )
               ch->pcdata->prev_help=NULL;
             else
	       ch->pcdata->prev_help=ch->pcdata->last_help;
	     ch->pcdata->last_help=menu->help;
	     ch->pcdata->help_mode=TRUE;
	     return;
	     }
	   }
	/* assume a menu option not listed is normal command and continue */
	 }
       }

    /* Last Command  - Chaos 5/6/99 */
    if( !IS_NPC( ch ) && ch->pcdata->last_command != NULL )
      {
      char jbuf[MAX_STRING_LENGTH];

      if( !strcasecmp( argument, "stop" ) || !strcasecmp( argument, "quit") )
        {
        send_to_char( "Command stopped.\n\r", ch );
        STRFREE( ch->pcdata->last_command );
        ch->pcdata->last_command = NULL;
        return;
        }
     strcpy( jbuf, ch->pcdata->last_command);
     strcat( jbuf, argument );
     strcpy( argument, jbuf );
     STRFREE( ch->pcdata->last_command );
     ch->pcdata->last_command = NULL;
     }

    /* Any command starting with numbers is a repeat command */
    if( *argument>='0' && *argument<='9')
      {
      do_repeat( ch, argument);
      return;
      }


    /* Added the '&' to multi-line commands  Chaos 12/7/93  */
    pti = argument;
    pto = command;
    for(pti=argument, pto=command;*pti!=' ' && *pti!='\0'; pto++, pti++)
       *pto = *pti;
    *pto='\0';

   if(!IS_NPC(ch) && ch->desc!=NULL && 
      ch->desc->intop<MAX_INPUT_LENGTH-2)
     if(strchr(argument,'&')!=NULL && strcasecmp(command,"alias"))
       {
       process_command( ch, argument, auto_command);
       return;
       }

    /*
     * No hiding.
     */
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha((int)*argument) && !isdigit((int)*argument) )
    {
	*command = *argument;
	*(command+1) = '\0';
	argument++;
	while ( isspace((int)*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */

    found = FALSE;
    trust = get_trust( ch );

  /*  Look for alias first - Chaos 10/11/93      */
   if(!IS_NPC(ch) && ch->obj_prog_ip == 0)
    if( !auto_command )
    if(ch->alias_ip==0 && ch->desc!=NULL && 
	   ch->desc->intop<MAX_INPUT_LENGTH-2)
     for(cmd = 0; cmd<MAX_ALIAS; cmd++)
      if(ch->pcdata->alias[cmd]!=NULL && ch->pcdata->alias[cmd][0]!='\0')
      if(!strcasecmp(ch->pcdata->alias_c[cmd],command))
	{
	ch->alias_ip=1;
        buf[0]='\0';
        
        pti = ch->pcdata->alias[cmd];
/*
I completely rewrote this section a fair bit to stop it over-running 
MAX_INPUT_LENGTH and crashing the mud. Forgive the dodgy pointer operations
but it's blindingly fast :) - Martin */
    
	for(pto=buf; *pti!='\0'; pti++)
	  {
	  if(*pti=='%')
           {
            pto = &buf[0] + str_cat_max(buf, argument, MAX_INPUT_LENGTH);
	   }
	  else
	    {
 	    if ((pto-&buf[0]) < MAX_INPUT_LENGTH-1)
             {
             *pto = (char) (*pti); 
             *(++pto)= '\0';
             }
	    }
	  }
        str_cat_max(buf, "\0", MAX_INPUT_LENGTH);
        /**pto = '\0';*/
        if( ch->desc != NULL && ch->desc->character == ch )
          {
          ch->pcdata->dump = FALSE;
          if( ch->desc->back_buf != NULL )
            {
            char buf2[MAX_INPUT_LENGTH];

            str_cpy_max( buf2, ch->desc->back_buf, MAX_INPUT_LENGTH );
            STRFREE(ch->desc->back_buf );
            str_cat_max( buf2, ch->desc->inbuf, MAX_INPUT_LENGTH );
            ch->desc->back_buf = str_dup( buf2 );
            }
          else
            ch->desc->back_buf = str_dup( ch->desc->inbuf );
          ch->desc->intop = 0;
          *ch->desc->inbuf = '\0';
          }
	preprocess_command( ch, buf, auto_command);
	return;
	}

    for ( cmd = 0; *cmd_table[cmd].name != '\0'; cmd++ )
    {
	if ( *command == *cmd_table[cmd].name)
	  if(!str_prefix( command, cmd_table[cmd].name ))
	    if(cmd_table[cmd].level <= trust )
	{
	    found = TRUE;
	    break;
	}
    }

    /*
     * Berserk players can only "kill" or "murder"
     */
    if(IS_AFFECTED(ch,AFF2_BERSERK) &&  ch->level != MAX_LEVEL &&
       ((found && 
         cmd_kill != cmd &&
         strcasecmp(cmd_table[cmd].name,"murder") &&
         strcasecmp(cmd_table[cmd].name,"kick") &&
         strcasecmp(cmd_table[cmd].name,"north") &&
         strcasecmp(cmd_table[cmd].name,"south") &&
         strcasecmp(cmd_table[cmd].name,"east") &&
         strcasecmp(cmd_table[cmd].name,"west") &&
         strcasecmp(cmd_table[cmd].name,"up") &&
         strcasecmp(cmd_table[cmd].name,"down") &&
         strcasecmp(cmd_table[cmd].name,"look") &&
         strcasecmp(cmd_table[cmd].name,"affects") &&
         strcasecmp(cmd_table[cmd].name,"score") &&
         strcasecmp(cmd_table[cmd].name,"quit") &&
         strcasecmp(cmd_table[cmd].name,"status") &&
         strcasecmp(cmd_table[cmd].name,"refresh"))||!found))
      {
      send_to_char("You're Berserk!  Kill!  Kill!  Kill!\n\r",ch);
      return;
      }

    /*
     * Arrested players are very limited as well. (Presto 02/97)
     */
    if(!IS_NPC(ch))
      if((ch->pcdata->arrested == TRUE) &&
	 ((found && 
	   strcasecmp(cmd_table[cmd].name,"say") &&
	   strcasecmp(cmd_table[cmd].name,"look") &&
	   strcasecmp(cmd_table[cmd].name,"affects") &&
	   strcasecmp(cmd_table[cmd].name,"score") &&
	   strcasecmp(cmd_table[cmd].name,"quit") &&
	   strcasecmp(cmd_table[cmd].name,"status") &&
	   strcasecmp(cmd_table[cmd].name,"refresh")) ||
	  !found))
	{
      send_to_char("You're in jail!\n\r",ch);
      return;
      }
      if(!IS_NPC(ch))
      if ( IS_SET ( ch->pcdata->player2_bits, PLR2_AFK) ) 
      {
          REMOVE_BIT( ch->pcdata->player2_bits, PLR2_AFK );
          act( "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
      }
    }
    /*
     * Log and snoop.
     */
    sprintf( lastplayercmd, "** %s: %s", ch->name, logline );


    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
  if( !IS_NPC( ch))
	  sprintf( log_buf, "Log %s: %s", ch->name, logline );
  else
	  sprintf( log_buf, "Log %s: %s", ch->short_descr, logline );
	log_string( log_buf );
    }


  if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
    OBJ_DATA *obj;
    if(ch->in_room!=NULL)
      for(obj=ch->in_room->first_content;obj!=NULL;obj=obj->next_content)
	if ( obj->pIndexData->obj_fun != 0 )
	  if ( !CAN_WEAR(obj,ITEM_TAKE) )
            {
            DISALLOW_SNOOP = FALSE;
	    if ( (*obj->pIndexData->obj_fun) (obj,OBJ_UNKNOWN,(void *)command,(void *)argument,ch))
	      {
	      tail_chain();
	      return;
	      }
	    tail_chain();
            }

  /* Check for object programs of unknown commands*/
  if( !IS_NPC( ch ) && ch->desc != NULL && ch->obj_prog_ip==0 )
    {
    OBJ_DATA *obj;
    OBJ_PROG *prg;
    OBJ_WITH_PROG *with;
    bool fcom;

    
    fcom=FALSE;
    for(with=ch->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if( obj->pIndexData->obj_prog!=NULL)
	for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
	  if( prg->cmd == -2  && !str_prefix( command, prg->unknown))
	    if( number_percent() < prg->percentage )
	    {
            open_timer( TIMER_OBJ_PROG );
	    start_object_program( ch, obj, prg, argument);
            close_timer( TIMER_OBJ_PROG );
	    tail_chain();
            fcom=TRUE;
	    }
      }
    if( fcom )
      return;
    for(with=ch->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if ( obj->pIndexData->obj_fun != 0 )
        {
        DISALLOW_SNOOP = FALSE;
	if ( (*obj->pIndexData->obj_fun) (obj,OBJ_UNKNOWN,(void *)command,(void *)argument,ch) )
	  {
	  tail_chain();
	  return;
	  }
        else
	  tail_chain();
        }
      }
    }

    if ( !check_social( ch, command, argument ) && strcasecmp( command, "!") &&
	  strcasecmp( command, "."))
      {
      sprintf( buf, "Huh?  '%s' is not a command.\n\r", command);
	    send_to_char( buf, ch );
      }

	  return;
    }

    /* Check for invalid combat */
    if( ch->fighting != NULL )
      if( !is_in_room( ch->fighting->who, ch->in_room ) )
        {
        stop_fighting(ch,FALSE);
        update_pos( ch );
        }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

      /*  Pets limitations */
   if( IS_NPC( ch) && IS_AFFECTED( ch, AFF_CHARM))
	    if( !(!strcasecmp(cmd_table[cmd].name, "north"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "south"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "east"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "west"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "up"       ) ||
		  !strcasecmp(cmd_table[cmd].name, "down"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "look"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "return"   ) ||
		  !strcasecmp(cmd_table[cmd].name, "kill"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "flee"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "exits"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "open"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "say"      ) ||
		  !strcasecmp(cmd_table[cmd].name, "get"      ) ||
		  !strcasecmp(cmd_table[cmd].name, "drop"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "wear"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "remove"   ) ||
		  !strcasecmp(cmd_table[cmd].name, "rest"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "stand"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "wake"     ) ||
		  !strcasecmp(cmd_table[cmd].name, "sleep"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "wield"    ) ||
		  !strcasecmp(cmd_table[cmd].name, "follow"   ) ||
		  !strcasecmp(cmd_table[cmd].name, "'"        ) ||
		  !strcasecmp(cmd_table[cmd].name, "close"    )   ))
	    {
	    send_to_char( "Your pet can't do that.\n\r", ch->master);
	    return;
	    }

      /*  Check for valid Homonculous commands   of:
	   n,s,e,w,u,d,look,return,kill,flee,exits,open,close */
    if(IS_NPC(ch) && (ch->pIndexData->vnum==9900 || ch->desc != NULL 
        || IS_AFFECTED(ch, AFF2_POSSESS)))
      {
	      if(!(!strcasecmp(cmd_table[cmd].name, "north"    ) ||
		   !strcasecmp(cmd_table[cmd].name, "south"    ) ||
		   !strcasecmp(cmd_table[cmd].name, "east"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "west"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "up"       ) ||
		   !strcasecmp(cmd_table[cmd].name, "down"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "look"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "return"   ) ||
		   !strcasecmp(cmd_table[cmd].name, "kill"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "flee"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "exits"    ) ||
		   !strcasecmp(cmd_table[cmd].name, "open"     ) ||
		   !strcasecmp(cmd_table[cmd].name, "close"    ) ||
		   !strcasecmp(cmd_table[cmd].name, "say"      ) ||
		   !strcasecmp(cmd_table[cmd].name, "'"        ) ))
	    {
            if(ch->pIndexData->vnum==9900 )
             send_to_char( "The homonculous can't do that.\n\r", ch);
	    else
             send_to_char( "You are having enough difficulty keeping control of this body to do that!\n\r", ch);
	    return;
	    }
      }


    /*
     * Check for objects to respond to command in room
     */
  if( !IS_NPC( ch ) && ch->desc != NULL && *ch->desc->inbuf=='\0')
    {
    OBJ_DATA *obj;

    if(ch->in_room!=NULL)
      for(obj=ch->in_room->first_content;obj!=NULL;obj=obj->next_content)
	if ( obj->pIndexData->obj_fun != 0 )
	  if ( !CAN_WEAR(obj,ITEM_TAKE) )
            {
            DISALLOW_SNOOP = FALSE;
	    if ( (*obj->pIndexData->obj_fun) (obj,OBJ_COMMAND,(void *)&cmd,(void *)argument,ch))
	      {
	      tail_chain();
	      return;
	      }
            else
              tail_chain();
            }
    }

    /*
     * Decrement movement for exertion cmds
     */
    if(cmd_gsn[cmd]!=-1)
      { 
      if(ch->move<skill_table[cmd_gsn[cmd]].min_mana)
	{
		    send_to_char( "You are too tired to do that!\n\r", ch );
		    return;
	}
      else
	ch->move-=skill_table[cmd_gsn[cmd]].min_mana;
      }

  /* Check for object programs */
  if( !IS_NPC( ch ) && ch->obj_prog_ip==0)
   if( ch->desc != NULL )
    {
    OBJ_DATA *obj;
    OBJ_PROG *prg;
    OBJ_WITH_PROG *with;
    bool fcom;

    fcom = FALSE;

    for(with=ch->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if( obj->pIndexData->obj_prog!=NULL)
	for( prg = obj->pIndexData->obj_prog; prg!=NULL ; prg=prg->next)
	  if( prg->cmd == cmd )
	    if( number_percent() < prg->percentage )
	    {
            open_timer( TIMER_OBJ_PROG );
	    start_object_program( ch, obj, prg, argument);
            close_timer( TIMER_OBJ_PROG );
	    tail_chain();
	    fcom=TRUE;
	    }
        }
    if( fcom )
      return;
    for(with=ch->obj_with_prog;with!=NULL;with=with->next)
      {
      if( (obj = with->obj ) == NULL || obj->pIndexData == NULL )
        continue;
      if( obj->pIndexData->obj_fun!=NULL)
      {
            DISALLOW_SNOOP = FALSE;
	if ( (*obj->pIndexData->obj_fun) (obj,OBJ_COMMAND,(void *)&cmd,(void *)argument,ch) )
	  {
	  tail_chain();
	  return;
	  }
            else
              tail_chain();
       }
       }
     }
    
	/* check for those nasty area creators that want to make gods */
    if(  ch->obj_prog_ip!=0 &&
	 (!strcasecmp(cmd_table[cmd].name, "quit" ) ||
	 !strcasecmp(cmd_table[cmd].name, "save" ) ) )
      return;

    if (!strcasecmp(cmd_table[cmd].name, "quit" ) )
       QUIT_CMD = TRUE;
    else
       QUIT_CMD = FALSE;

    open_timer( TIMER_INTERP );

    /*
     * Dispatch the command.
     */
    DISALLOW_SNOOP = FALSE;
    ch->last_cmd = *cmd_table[cmd].do_fun;
    (*cmd_table[cmd].do_fun) ( ch, argument );
    if( DISALLOW_SNOOP )
      wipe_string( argument );

    tail_chain( );


    delta = get_game_usec() - last_time;
    if( !QUIT_CMD  && ch!=NULL && !IS_NPC( ch ))
      {
      if( IS_SET( ch->act, PLR_WIZTIME ))
	{
	sprintf( buf, "(%d usec)\n\r", delta);
	send_to_char( buf, ch);
	}
      }

    close_timer( TIMER_INTERP );

    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; *social_table[cmd].name != '\0'; cmd++ )
    {
      if(!str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    return( check_social_fast( ch, cmd, argument ) );
    }

bool check_social_fast( CHAR_DATA *ch, int cmd, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !strcasecmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    if ( argument == NULL)
    {
	act( social_table[cmd].others_no_arg, ch, NULL, NULL, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, NULL, TO_CHAR    );
    return TRUE;
    }

    one_argument( argument, arg );
    victim = NULL;
  
    if ( arg == NULL || arg[0]=='\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    return TRUE;
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

    GLOBAL_SOCIAL_FLAG = FALSE;
    mprog_social_trigger( social_table[cmd].name, victim, ch );
    if( GLOBAL_SOCIAL_FLAG == TRUE )
      return( TRUE );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:     /*   Chaos removed  12/6/93
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;   */

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    if ( *arg == '\0' )
	return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit((int)*arg) && (*arg!='-') && (*arg!='+') )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atol( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
  {
  char cEnd;

  while ( *argument==' ' || *argument=='\n' || *argument=='\r' || *argument==27)
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' )
    {
    if ( *argument == cEnd )
      {
      argument++;
      break;
      }
    *arg_first = LOWER(*argument);
    arg_first++;
    argument++;
    }
  *arg_first = '\0';

  while ( *argument==' ' || *argument=='\n' || *argument=='\r' || *argument==27)
    argument++;

  return argument;
  }

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Don't lower case it.
 */
char *one_argument_nolower( char *argument, char *arg_first )
  {
  char cEnd;

  while ( isspace((int)*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' )
    {
    if ( *argument == cEnd )
      {
      argument++;
      break;
      }
    *arg_first = *argument;
    arg_first++;
    argument++;
    }
  *arg_first = '\0';

  while ( isspace((int)*argument) )
    argument++;

  return argument;
  }

void process_command( CHAR_DATA *ch, char *argument, bool auto_command)
  {
  char buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
  char *pti, *pto;
  bool found;

  found = FALSE;
  for( pti=argument, pto=buf3; *pti != '\0'; pti++, pto++)
    {
    if( *pti == '&' )
      {
      if( !found )
        {
        *pto = '\0';
        pto = buf2-1;   /* Switch recording medium */
        found = TRUE;
        }
      else
        *pto = '\r';
      }
    else
      *pto = *pti;
    }
  if( found )
    {
    *pto = '\r';
    pto++;
    }
  *pto = '\0';

  if( found )
   {


    /* Add buf2 to the inbuf */
 if( !auto_command )
  {
  if(*ch->desc->inbuf=='\0')
    ch->desc->intop = str_cpy_max( ch->desc->inbuf, buf2, MAX_INPUT_LENGTH);
  else
    {
    if(ch->desc->inbuf[ch->desc->intop-1]!='\r' &&
      ch->desc->inbuf[ch->desc->intop-1]!='\n')
      {
      ch->desc->intop = str_apd_max(ch->desc->inbuf, "\r", ch->desc->intop,
                               MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      }
    else
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop,
                               MAX_INPUT_LENGTH);
    }
  }
 else
  {
  *buf3=27;
  *(buf3+1)=0;
  if(*ch->desc->inbuf=='\0')
      {
      ch->desc->intop = str_cpy_max(ch->desc->inbuf, buf3, MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      }
  else
    {
    if(ch->desc->inbuf[ch->desc->intop-1]!='\r' &&
      ch->desc->inbuf[ch->desc->intop-1]!='\n')
      {
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf3, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, "\r", ch->desc->intop,
                               MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      }
    else
      {
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf3, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop,
                               MAX_INPUT_LENGTH);
      }
    }
   }
  }
  if (ch->desc && ch->desc->connected==CON_EDITING)
  {
   ch->desc->connected=CON_PLAYING;
   interpret(ch, buf3);
   ch->desc->connected=CON_EDITING;
  }
  else
   interpret(ch, buf3);
   
  tail_chain();
  return;
  } 

void preprocess_command( CHAR_DATA *ch, char *argument, bool auto_command)
  {
  char buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  char *pti, *pto;
  bool found;

  found = FALSE;
  for( pti=argument, pto=buf3; *pti != '\0'; pti++, pto++)
    {
    if( *pti == '&' )
      {
      if( !found )
        {
        *pto = '\0';
        pto = buf2-1;   /* Switch recording medium */
        found = TRUE;
        }
      else
        *pto = '\r';
      }
    else
      *pto = *pti;
    }
  if( found )
    {
    *pto = '\r';
    pto++;
    }
  *pto = '\0';

  if( found )
   {

  if(*ch->desc->inbuf=='\0')
    ch->desc->intop = str_cpy_max( ch->desc->inbuf, buf2, MAX_INPUT_LENGTH);
  else
      {
      strcpy( buf1, ch->desc->inbuf);
      ch->desc->intop = str_cpy_max( ch->desc->inbuf, buf2, MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, "\r", ch->desc->intop,
                                     MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max(ch->desc->inbuf, buf1, ch->desc->intop,
                                     MAX_INPUT_LENGTH);
      }
   }
  if (ch->desc && ch->desc->connected==CON_EDITING)
  {
   ch->desc->connected=CON_PLAYING;
   interpret(ch, buf3);
   ch->desc->connected=CON_EDITING;
  }
  else
   interpret(ch, buf3);
  tail_chain();
  return;
}


void start_object_program ( CHAR_DATA * ch, OBJ_DATA *obj, OBJ_PROG *prg,
	      char *argument)
  {
  char buf[MAX_INPUT_LENGTH];
  int bufl;
  char *pt;

  if( ch->desc == NULL || ch->desc->character != ch || IS_NPC( ch ) )
    return;

    /* No lock-ups here including wait oriented char lock-ups */
  if( ch->pcdata->oprog_started || (ch->desc->back_buf != NULL &&
       *ch->desc->back_buf != '\0' ) ||
      ch->wait > 0 )
    return;

  /* Dont want this messing with nice inbuf and incomm while editing 
     - Martin */

  if (ch->desc->connected == CON_EDITING)
    return;

  if( argument != NULL )
    for( pt=argument; *pt!='\0'; pt++)
      if( *pt == '&' )
        *pt = '-';

  ch->pcdata->dump = FALSE;

  if( *ch->desc->incomm != '\0' )
    {
    bufl = str_cpy_max( buf, ch->desc->incomm, MAX_INPUT_LENGTH - 10 );
    bufl = str_apd_max( buf, "\r", bufl, MAX_INPUT_LENGTH - 10 );
    }
  else
    {
    bufl = 0;
    *buf = '\0';
    }
    
  bufl = str_apd_max( buf, ch->desc->inbuf, bufl, MAX_INPUT_LENGTH-10);

  if( ch->desc->back_buf != NULL && *ch->desc->back_buf != '\0' )
    {
    if( *buf != '\0' )
      bufl = str_apd_max( buf, "\r", bufl, MAX_INPUT_LENGTH - 10 );
    bufl = str_apd_max( buf, ch->desc->back_buf, bufl, MAX_INPUT_LENGTH - 10 );
    STRFREE(ch->desc->back_buf );
    }

     /* This could cause problems if left alone */
  if( bufl == 0 )
    bufl=str_cpy_max( buf, "\r", MAX_INPUT_LENGTH-10 );

  ch->desc->back_buf = str_dup( buf );

  *ch->desc->incomm = '\0';
  *ch->desc->inbuf = '\0';
   ch->desc->intop = 0;

  ch->pcdata->oprog_started = TRUE;
  
  object_program( ch, obj, prg, argument );

  ch->pcdata->oprog_started = FALSE;

  return;
  }


void object_program ( CHAR_DATA * ch, OBJ_DATA *obj, OBJ_PROG *prg,
	      char *argument)
  {
  int if_val;
  int cnt;
  char buf[MAX_STRING_LENGTH];
  char *pt, pb[2];
  if( obj == NULL || ch == NULL || obj->carried_by != ch )
    return;

  if( ch->desc == NULL || ch->desc->character != ch )
    return;

  if_val = 0;
  switch( prg->obj_command )
    {
    case 'E':  /* Echo to user */
      send_to_char( prg->argument , ch);
      if( prg->true != NULL )
	object_program( ch, obj, prg->true , argument);
      return;

    case 'H':  /* has object check */
      {
      OBJ_DATA *obj;
      if_val = 0;
      for( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
        if( obj->pIndexData->vnum == prg->if_value )
          {
          if_val = 1;
          break;
          }
      }
	  if( if_val == 1 )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;

    case 'J':  /* Junk the item */
      obj_from_char( obj );
      extract_obj( obj);
      return;

    case 'C':   /* user command at level 99 multi-line no argument */
      pb[1]='\0';
      buf[0]=27;
      buf[1]='\0';

      for( pt = prg->argument; *pt!='\0'; pt++)
        if( *pt != '$' )
          {
          *pb=*pt;
          strcat( buf, pb );
          }
      else
          strcat( buf, capitalize(ch->name) );
  
      ch->desc->intop = str_cpy_max( ch->desc->inbuf, buf, MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max( ch->desc->inbuf, "\r", ch->desc->intop,
                                        MAX_INPUT_LENGTH);
      ch->trust = 99;  /* This is very dangerous, but required */
      ch->obj_prog_ip = 1;
      ch->alias_ip = 1;
      if( prg->true != NULL )
	object_program( ch, obj, prg->true , argument);
      return;

    case 'G':   /* single user command at level 99 with argument */
      buf[0]=27;
      buf[1]='\0';
      for( cnt=0; argument[cnt]!='\0' && argument[cnt]!='\r' &&
		  argument[cnt]!='&' ; cnt++);
      argument[cnt]='\0';
      ch->desc->intop = str_cpy_max( ch->desc->inbuf, buf, MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max( ch->desc->inbuf, prg->argument, 
                         ch->desc->intop, MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max( ch->desc->inbuf, " ", ch->desc->intop,
                         MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max( ch->desc->inbuf, argument, ch->desc->intop,
                         MAX_INPUT_LENGTH);
      ch->desc->intop = str_apd_max( ch->desc->inbuf, "\r", ch->desc->intop,
                         MAX_INPUT_LENGTH);
      ch->trust = 99;  /* This is very dangerous, but required */
      ch->obj_prog_ip = 1;
      ch->alias_ip = 1;
      if( prg->true != NULL )
	object_program( ch, obj, prg->true, argument );
      return;

    case 'A':  /* Apply to temp stats  */
      switch( prg->if_check )
	{
	case 1:
	  ch->hit+=prg->if_value;
	  if( ch->hit>ch->max_hit)
	    ch->hit = ch->max_hit;
	  if( ch->hit<0)
            {
            ch->trust = ch->level;
		
            raw_kill( ch );
            return;
            }
	  break;
	case 2:
	  ch->move+=prg->if_value;
	  if( ch->move>ch->max_move)
	    ch->move= ch->max_move;
	  if( ch->move<0)
	    ch->move = 0;
	  break;
	case 3:
	  ch->mana+=prg->if_value;
	  if( ch->mana>ch->max_mana)
	    ch->mana = ch->max_mana;
	  if( ch->mana<0)
	    ch->mana = 0;
	  break;
	case 4:
	  ch->alignment+=prg->if_value;
	  if( ch->alignment>1000)
	    ch->alignment = 1000;
	  if( ch->alignment<-1000)
	    ch->alignment = -1000;
	  break;
	}
      if( prg->true != NULL )
	object_program( ch, obj, prg->true, argument );
      break;

    case 'D':   /* Add to quest bits with if_value */
      
      cnt=get_quest_bits( obj->obj_quest, prg->quest_offset, prg->quest_bits);
      cnt+=prg->if_value;
      set_quest_bits( &obj->obj_quest, prg->quest_offset, prg->quest_bits,
        cnt);

      if( prg->true != NULL )
	object_program( ch, obj, prg->true, argument );
      break;

    case 'S':   /* Set quest bit to if_value */
      set_quest_bits( &obj->obj_quest, prg->quest_offset, prg->quest_bits,
        prg->if_value);
      
      if( prg->true != NULL )
	object_program( ch, obj, prg->true, argument );
      break;

    case 'I':  /* if check */
      switch( prg->if_check )
	{
        case 'f': /* PK status */
          if (ch->fighting == NULL) if_val = 0;
           else
          {
           if (!IS_NPC(ch->fighting->who))
            {
              if (which_god( ch)!=0 && which_god(ch->fighting->who)!=0 &&
                   which_god(ch)!=which_god(ch->fighting->who))
                 if_val=1;
               else
                 if_val=2;
            }
          else if_val = 3;
          }
          break;

	case 'c':   /* user class */
	  if_val = ch->class;
	  break;
	case 'o':   /* user position */
	  if_val = ch->position;
	  break;
	case 'b':   /* user gold */
	  if_val = ch->gold;
	  break;
	case 'i':   /* user area first room number in */
	  if_val = (int)ch->in_room->area->low_r_vnum;
	  break;
	case 'u':   /* user room number in */
	  if_val = ch->in_room->vnum;
	  break;
	case 'p':   /* random percent */
	  if_val = number_percent();
	  break;
	case 'g':   /* user god */
	  if_val = which_god(ch);
	  break;
	case 'a':   /* user alignment */
	  if_val = ch->alignment;
	  break;
	case 'l':   /* user level */
	  if_val = ch->level;
	  break;
	case 'r':   /* user race */
	  if_val = ch->race;
	  break;
	case 's':   /* user sex */
          if_val = ch->sex;
	  break;
	case 'w':   /* wear location */
          if_val = obj->wear_loc;
	  break;
	case 'v':   /* user move percent */
          if( ch->max_move == 0 )
            if_val = 0;
          else
	    if_val = ch->move*100/ch->max_move;
	  break;
	case 'h':   /* user hitpoints percent */
          if( ch->max_hit == 0 )
            if_val = 0;
          else
	    if_val = ch->hit*100/ch->max_hit;
	  break;
	case 'm':   /* user mana percent */
          if( ch->max_mana == 0 )
            if_val = 0;
          else
	    if_val = ch->mana*100/ch->max_mana;
	  break;
	case 'd':   /* damage to character percent */
          if( ch->max_hit == 0 )
            if_val = 0;
          else
	    if_val = (ch->max_hit - ch->hit)*100/ch->max_hit;
	  break;
	case 64:   /* Multi-class level of Ill */
	  if_val = ch->mclass[0];
	  break;
	case 65:   /* Multi-class level of Ill */
	  if_val = ch->mclass[1];
	  break;
	case 66:   /* Multi-class level of Ill */
	  if_val = ch->mclass[2];
	  break;
	case 67:   /* Multi-class level of Ill */
	  if_val = ch->mclass[3];
	  break;
	case 68:   /* Multi-class level of Ill */
	  if_val = ch->mclass[4];
	  break;
	case 69:   /* Multi-class level of Ill */
	  if_val = ch->mclass[5];
	  break;
	case 70:   /* Multi-class level of Ill */
	  if_val = ch->mclass[6];
	  break;
	case 71:   /* Sector Type */
	  if_val = ch->in_room->sector_type;
	  break;
        case 72:  /* Time of day */
	  if_val = time_info.hour;
	  break;
	case 73:   /* Weather */
	  if_val = ch->in_room->area->weather_info.sky;
	  break;
	case 74:   /* Army 0=No, 1=Yes, 2=Retired */
	  if_val = ch->pcdata->army_status;
	  break;
	}
      switch( prg->if_symbol )
	{
	case '<':
	  if( if_val < prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '>':
	  if( if_val > prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '=':
	  if( if_val == prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '!':
	  if( if_val != prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
       }
	break;

   case 'Q':  /* Object quest bits If check */
      if_val = get_quest_bits( obj->obj_quest, prg->quest_offset,
           prg->quest_bits );
     
      switch( prg->if_symbol )
	{
	case '<':
	  if( if_val < prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '>':
	  if( if_val > prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '=':
	  if( if_val == prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '!':
	  if( if_val != prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
       }
	break;
      
   case 'P':  /* Player quest bits If check */
     if_val= get_quest_bits ( 
        ch->pcdata->quest[obj->pIndexData->area->low_r_vnum/100], 
        prg->quest_offset, prg->quest_bits );
    	

      switch( prg->if_symbol )
	{
	case '<':
	  if( if_val < prg->if_value )
	    {
	    if( prg->true != NULL )
	    {	
	      object_program( ch, obj, prg->true, argument );
             }
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '>':
	  if( if_val > prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '=':
	  if( if_val == prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
	case '!':
	  if( if_val != prg->if_value )
	    {
	    if( prg->true != NULL )
	      object_program( ch, obj, prg->true, argument );
	    }
	  else
	    {
	    if( prg->false != NULL )
	      object_program( ch, obj, prg->false , argument);
	    }
	  return;
       }
	break;
      
      
    default:
      break;
    }
  return;
  }

void do_a( CHAR_DATA *ch, char *argument )
  {
  send_to_char( "Ah, what?\n\r", ch );
  return;
  }

/* Cut down on the swearing.  Presto 07/98 */
int has_word(char *word, char *input_string)
{
  int i, j, offset, word_len, span;

  word_len=strlen(word);
  span = strlen(input_string) - word_len + 1;

  for(i=0; i<span; i++)
    if(word[0] == input_string[i])
    {
      offset = i;
      for(j=0; j<word_len; j++)
        if(word[j] != input_string[(offset+j)])
          break;
      if(j == word_len)
        return offset;
    }

  return -1;
}

void censorword(char *word, char *lower_string, char *input_string)
{
  int i, at, word_len;

  word_len=strlen(word);
  while((at=has_word(word, lower_string)) >= 0)
    for(i=0; i<word_len; i++)
    {
      lower_string[(i+at)] = '*';
      input_string[(i+at)] = '*';
    }
  return;
}

void censorstring(char *input_string)
{
  SWEAR_DATA *swear;
  char lower_string[MAX_INPUT_LENGTH];
  int i, string_len;

  string_len = strlen(input_string) + 1;
  for(i=0; i<string_len; i++)
    lower_string[i] = tolower(input_string[i]);

  for(swear=swear_list; swear; swear=swear->next)
  {
    censorword(swear->word, lower_string, input_string);
  }
}

