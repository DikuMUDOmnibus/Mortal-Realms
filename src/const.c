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
#include <stdio.h>
#include <time.h>
#include "merc.h"

/*
 *     List of descriptions for timers
 */

const char timer_strings[ MAX_TIMERS ][80] =
       { "Average Update Time:              ",
         "    Average Char Update Time:     ",
         "    Average Mobile Update Time:   ",
         "    Average Object Update Time:   ",
         "    Average Shop Update Time:     ",
         "    Average Aggr Update Time:     ",
         "    Average Violence Update Time: ",
         "    Average Area Update Time:     ",
         "Average Save Time:                ",
         "Average Load Time:                ",
         "Average Interp Command Time:      ",
         "Average Castle Save Time:         ",
         "Average hit_gain:                 " ,
         "Average GetHostAddress Lookup:    " ,
         "Average MobProg:                  " ,
         "Average ObjProg:                  " ,
         "Average Write to Descriptor:      " ,
         "Average Read From Descriptor:     " ,
         "Average Scan Descriptor:          " ,
         "    Average Weather Update Time:  " ,
         "    Average Sector Update Time:   " ,
         "  Average Write Pager:            " ,
         "  Average Write Scroller:         " ,
         "  Average Write Append:           " ,
         "  Average Total Tactical:         " ,
         "  Average Write Process Tact STR: " ,
         "  Average Write Process Tact CMP: " ,
         "  Average Write Process Tact OUT: " ,
         "  		Average Clan Save : ", 
         "  		Average CREATE    : ",
         "  		Average DISPOSE   : "
};


/*
 *    Race table.
      Copy of structure  
struct  race_type
{
   char         race_name[10];
   sh_int       race_mod[5];
   sh_int       race_class[MAX_CLASS];
   sh_int       max_speed;
   sh_int       move_rate;
   sh_int       vision;
   sh_int       hp_mod;
   sh_int       mana_mod;
   sh_int       move_mod;
   char         race_special[80];
};
*/

const   struct  race_type       race_table      [MAX_RACE]      =
{
    {  "Human",   { 0, 0, 0, 0, 0}, { 1, 0, 0, 0, 0, 0, 1}, 2, 2, 0,  0, 0, 0,
         "Enhanced healing indoors and in cities." ,
	 "Humans are the most numerous and the plainest race in the realms. Although possessing no special abilities, they are the most adaptable and flexible race." },

    {  "Halfling",{-1,+1,+1,+1,-2}, { 0, 0, 0, 1, 0, 1, 0}, 1, 2, 0, -1, 2,-2,
         "Can see the presense of aggressive creatures.",
	"The members of this stout and round, open-minded race that venture forth from the comforts of home are served well by their keen intelligence and dexterity." },

    {  "Elf",     {-1,+2,+1,-1,-1}, { 0, 1, 0, 0, 1, 0, 0}, 3, 3, 1, -1, 3,-1,
         "Enhanced mana gain while outdoors.",
	"Tall, slender creatures with an affinity for the woods and nature, Elves may lack physical stature, but have superior agility, both in thought and movement." },

    {  "Drow",    {-1,+2,-1,+1,-1}, { 0, 0, 0, 1, 0, 1, 0}, 3, 4, 2, -1, 1,-1,
         "Know alignment.",
	"The dark cousin of the elf, the Drow live underground in a vast network of caverns and cities. They are lightning fast and possess immense magical power." },

    {  "Dwarf",   {+1,-1,-1,-1,+2}, { 0, 0, 1, 0, 1, 0, 0}, 1, 2, 2,  2,-1, 0,
         "Resistance to magic.",
	"Dwarves are relatively short, stocky and powerfully built creatures. Long lived and extremely sturdy, they are known famed for their hardiness." },

    {  "Gnome",   { 0,-1,+1,+1, 0}, { 0, 0, 1, 0, 0, 0, 1}, 1, 1, 0, -1, 1,-1,
         "Forage for food.",
	"A short and quirky race, gnomes are not the mightiest of races, though they are well versed in their studies of knowledges, magics and religions." },

    {  "Orc",     {+1,+1,-2,-1,+1}, { 1, 0, 0, 0, 1, 0, 0}, 2, 3, 1,  3,-1,-1,
         "Dirty fighting techniques with knees and elbows.",
	"Large and dull of wit, with short but broad bodies, their poor mental dexterity is offset by their iron constitution and impressive physical strength." },

    {  "Ogre",    {+2,+1,-2,-3,+2}, { 0, 1, 0, 0, 0, 0, 1}, 2, 2, 0,  5,-2,-1,
         "Intimidate opponents into forgetting to defend.",
	"Ogres and their muscle packed bodies can weigh between three and four hundred pounds. Their bodies are covered with small boney growths." },

    {  "Aviaran",  {-3,+1,+1,+2, 0}, { 1, 0, 0, 1, 0, 0, 0}, 2, 3, 0, -2,+2,-1,
         "Natural ability to fly.",
	"Aviarans are a race of intelligent bird-men that come from the peaks of the highest mountains.  They are about 5' tall with 20' wingspans." },

    {  "Centaur",  {+2,-1, 0,-1, 0}, { 0, 0, 0, 0, 1, 0, 1}, 4, 1, 0, +3,-3,+2,
         "Extra melee attack with their hooves.",
	"Half human, half horse, the Centaurs are quite at home in the wilderness.  With four legs, they move much faster than the other races of the realm." },

    {  "Gith",  {+1,-2,+1,+2,-1}, { 0, 1, 0, 0, 0, 1, 0}, 2, 1, 1, -2,+2,-2,
         "Natural astral projection.",
	"An ancient race of enslaved humans who broke free from their mindflayer captors, the Gith are beings who dwell naturally in the astral plane." },

    {  "Tsarian",  {-1,+4,-1,-1,-1}, { 0, 0, 0, 1, 1, 0, 0}, 3, 2, 2, +1,-2,+2,
         "Natural ability to sneak.",
	"Tsarians are a race of cat-people.  They make excellent hunters with their quick speed and amazing dexterity." },

};

/*
 * Class table.
struct	class_type
{
    char 	who_name	[4];
    char        who_name_long   [40];
    sh_int	attr_prime;
    sh_int	weapon; 
    sh_int	guild;
    sh_int	skill_adept;
    sh_int	thac0_00;
    sh_int	thac0_32;
    sh_int	hp_min;
    sh_int	hp_max;
    sh_int	mana_min;
    sh_int	mana_max;
    char        desc           [80];
};
 */

const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"Ill", "Illusionist",  APPLY_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	9809,  92,  18, 10,  6,  8, 7, 10,
        "These tricksters of the magic world can conjure up the most fiercesome destructive magic known to any magician, living or dead..."
    },

    {
	"Ele",  "Elementalist", APPLY_WIS,  OBJ_VNUM_SCHOOL_MACE,
	9766,  95,  18, 12,  7, 10, 6,9,
        "Elementalists can tap the awesome powers of Air, Fire, Water and Earth in a school of magic that is almost druidic in nature."
    },

    {
	"Rog",  "Rogue", APPLY_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	9813,  97,  18,  8,  8, 13, 0,3,
        "Rogues are characters which make use of many skills deemed improper by the rest of society. They are useful when a lock needs to be \"fixed\" or some \"lost\" items need to be recovered." 
    },

    {
	"Ran",  "Ranger", APPLY_CON,  OBJ_VNUM_SCHOOL_SWORD,
	9782,  90,  18,  6,  11, 16, 0,2,
        "A ranger is the outdoorsman and adventuresome free spirit of the times. Rangers possess the skill at arms and mighty strength to be able to survive and adapt to almost any locale."
    },

    {
	"Nec",  "Necromancer", APPLY_WIS,  OBJ_VNUM_SCHOOL_DAGGER,
	9801,  99,  18, 10,   5,  8, 7, 11 ,
        "Necromancers deal with death and powerful magic. They are more scholarly than their colleagues in the other schools of magic, preferring to study than seek adventure."
    },

    {
	"Mon",  "Monk", APPLY_STR,  0,
	9796,  90,  18,  6,   8, 12, 6,9 ,
        "A monk is a mighty healer and champion of the weak. Having spent many years in strict discipline and rigourous training, they have mastered both the arts of healing and the arts of unarmed combat."
    },

    {
	"Asn",  "Assassin", APPLY_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	9807,  93,  17,  7,   8, 12, 0,2,
        "Assassins deal with the darkest side of human nature, the desire for profit and vengeance. They are generally unliked and regarded with suspicion by the rest of society, they are generally unseen until they try to kill you."
    }
};



/*
 * Titles.
 */
/* these are cool, but big trouble thinking them up -dug
char *	const			title_table	[MAX_CLASS][MAX_LEVEL+1][2] =
{
    {
	{ "Man",			"Woman"				},

	{ "Apprentice of Magic",	"Apprentice of Magic"		},
	{ "Spell Student",		"Spell Student"			},
	{ "Scholar of Magic",		"Scholar of Magic"		},
	{ "Delver in Spells",		"Delveress in Spells"		},
	{ "Medium of Magic",		"Medium of Magic"		},

	{ "Scribe of Magic",		"Scribess of Magic"		},
	{ "Seer",			"Seeress"			},
	{ "Sage",			"Sage"				},
	{ "Illusionist",		"Illusionist"			},
	{ "Abjurer",			"Abjuress"			},

	{ "Invoker",			"Invoker"			},
	{ "Enchanter",			"Enchantress"			},
	{ "Conjurer",			"Conjuress"			},
	{ "Magician",			"Witch"				},
	{ "Creator",			"Creator"			},

	{ "Savant",			"Savant"			},
	{ "Magus",			"Craftess"			},
	{ "Wizard",			"Wizard"			},
	{ "Warlock",			"War Witch"			},
	{ "Sorcerer",			"Sorceress"			},

	{ "Elder Sorcerer",		"Elder Sorceress"		},
	{ "Grand Sorcerer",		"Grand Sorceress"		},
	{ "Great Sorcerer",		"Great Sorceress"		},
	{ "Golem Maker",		"Golem Maker"			},
	{ "Greater Golem Maker",	"Greater Golem Maker"		},

	{ "Maker of Stones",		"Maker of Stones",		},
	{ "Maker of Potions",		"Maker of Potions",		},
	{ "Maker of Scrolls",		"Maker of Scrolls",		},
	{ "Maker of Wands",		"Maker of Wands",		},
	{ "Maker of Staves",		"Maker of Staves",		},

	{ "Demon Summoner",		"Demon Summoner"		},
	{ "Greater Demon Summoner",	"Greater Demon Summoner"	},
	{ "Dragon Charmer",		"Dragon Charmer"		},
	{ "Greater Dragon Charmer",	"Greater Dragon Charmer"	},
	{ "Master of all Magic",	"Master of all Magic"		},

	{ "Mage Hero",			"Mage Heroine"			},
	{ "Angel of Magic",		"Angel of Magic"		},
	{ "Deity of Magic",		"Deity of Magic"		},
	{ "Supremity of Magic",		"Supremity of Magic"		},
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Believer",			"Believer"			},
	{ "Attendant",			"Attendant"			},
	{ "Acolyte",			"Acolyte"			},
	{ "Novice",			"Novice"			},
	{ "Missionary",			"Missionary"			},

	{ "Adept",			"Adept"				},
	{ "Deacon",			"Deaconess"			},
	{ "Vicar",			"Vicaress"			},
	{ "Priest",			"Priestess"			},
	{ "Minister",			"Lady Minister"			},

	{ "Canon",			"Canon"				},
	{ "Levite",			"Levitess"			},
	{ "Curate",			"Curess"			},
	{ "Monk",			"Nun"				},
	{ "Healer",			"Healess"			},

	{ "Chaplain",			"Chaplain"			},
	{ "Expositor",			"Expositress"			},
	{ "Bishop",			"Bishop"			},
	{ "Arch Bishop",		"Arch Lady of the Church"	},
	{ "Patriarch",			"Matriarch"			},

	{ "Elder Patriarch",		"Elder Matriarch"		},
	{ "Grand Patriarch",		"Grand Matriarch"		},
	{ "Great Patriarch",		"Great Matriarch"		},
	{ "Demon Killer",		"Demon Killer"			},
	{ "Greater Demon Killer",	"Greater Demon Killer"		},

	{ "Cardinal of the Sea",	"Cardinal of the Sea"		},
	{ "Cardinal of the Earth",	"Cardinal of the Earth"		},
	{ "Cardinal of the Air",	"Cardinal of the Air"		},
	{ "Cardinal of the Ether",	"Cardinal of the Ether"		},
	{ "Cardinal of the Heavens",	"Cardinal of the Heavens"	},

	{ "Avatar of an Immortal",	"Avatar of an Immortal"		},
	{ "Avatar of a Deity",		"Avatar of a Deity"		},
	{ "Avatar of a Supremity",	"Avatar of a Supremity"		},
	{ "Avatar of an Implementor",	"Avatar of an Implementor"	},
	{ "Master of all Divinity",	"Mistress of all Divinity"	},

	{ "Holy Hero",			"Holy Heroine"			},
	{ "Angel",			"Angel"				},
	{ "Deity",			"Deity"				},
	{ "Supreme Master",		"Supreme Mistress"		},
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Pilferer",			"Pilferess"			},
	{ "Footpad",			"Footpad"			},
	{ "Filcher",			"Filcheress"			},
	{ "Pick-Pocket",		"Pick-Pocket"			},
	{ "Sneak",			"Sneak"				},

	{ "Pincher",			"Pincheress"			},
	{ "Cut-Purse",			"Cut-Purse"			},
	{ "Snatcher",			"Snatcheress"			},
	{ "Sharper",			"Sharpress"			},
	{ "Rogue",			"Rogue"				},

	{ "Robber",			"Robber"			},
	{ "Magsman",			"Magswoman"			},
	{ "Highwayman",			"Highwaywoman"			},
	{ "Burglar",			"Burglaress"			},
	{ "Thief",			"Thief"				},

	{ "Knifer",			"Knifer"			},
	{ "Quick-Blade",		"Quick-Blade"			},
	{ "Killer",			"Murderess"			},
	{ "Brigand",			"Brigand"			},
	{ "Cut-Throat",			"Cut-Throat"			},

	{ "Spy",			"Spy"				},
	{ "Grand Spy",			"Grand Spy"			},
	{ "Master Spy",			"Master Spy"			},
	{ "Assassin",			"Assassin"			},
	{ "Greater Assassin",		"Greater Assassin"		},

	{ "Master of Vision",		"Mistress of Vision"		},
	{ "Master of Hearing",		"Mistress of Hearing"		},
	{ "Master of Smell",		"Mistress of Smell"		},
	{ "Master of Taste",		"Mistress of Taste"		},
	{ "Master of Touch",		"Mistress of Touch"		},

	{ "Crime Lord",			"Crime Mistress"		},
	{ "Infamous Crime Lord",	"Infamous Crime Mistress"	},
	{ "Greater Crime Lord",		"Greater Crime Mistress"	},
	{ "Master Crime Lord",		"Master Crime Mistress"		},
	{ "Godfather",			"Godmother"			},

	{ "Assassin Hero",		"Assassin Heroine"		},
	{ "Angel of Death",		"Angel of Death"		},
	{ "Deity of Assassins",		"Deity of Assassins"		},
	{ "Supreme Master",		"Supreme Mistress"		},
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Swordpupil",			"Swordpupil"			},
	{ "Recruit",			"Recruit"			},
	{ "Sentry",			"Sentress"			},
	{ "Fighter",			"Fighter"			},
	{ "Soldier",			"Soldier"			},

	{ "Warrior",			"Warrior"			},
	{ "Veteran",			"Veteran"			},
	{ "Swordsman",			"Swordswoman"			},
	{ "Fencer",			"Fenceress"			},
	{ "Combatant",			"Combatess"			},

	{ "Hero",			"Heroine"			},
	{ "Myrmidon",			"Myrmidon"			},
	{ "Swashbuckler",		"Swashbuckleress"		},
	{ "Mercenary",			"Mercenaress"			},
	{ "Swordmaster",		"Swordmistress"			},

	{ "Lieutenant",			"Lieutenant"			},
	{ "Champion",			"Lady Champion"			},
	{ "Dragoon",			"Lady Dragoon"			},
	{ "Cavalier",			"Lady Cavalier"			},
	{ "Knight",			"Lady Knight"			},

	{ "Grand Knight",		"Grand Knight"			},
	{ "Master Knight",		"Master Knight"			},
	{ "Paladin",			"Paladin"			},
	{ "Grand Paladin",		"Grand Paladin"			},
	{ "Demon Slayer",		"Demon Slayer"			},

	{ "Greater Demon Slayer",	"Greater Demon Slayer"		},
	{ "Dragon Slayer",		"Dragon Slayer"			},
	{ "Greater Dragon Slayer",	"Greater Dragon Slayer"		},
	{ "Underlord",			"Underlord"			},
	{ "Overlord",			"Overlord"			},

	{ "Baron of Thunder",		"Baroness of Thunder"		},
	{ "Baron of Storms",		"Baroness of Storms"		},
	{ "Baron of Tornadoes",		"Baroness of Tornadoes"		},
	{ "Baron of Hurricanes",	"Baroness of Hurricanes"	},
	{ "Baron of Meteors",		"Baroness of Meteors"		},

	{ "Knight Hero",		"Knight Heroine"		},
	{ "Angel of War",		"Angel of War"			},
	{ "Deity of War",		"Deity of War"			},
	{ "Supreme Master of War",	"Supreme Mistress of War"	},
	{ "Implementor",		"Implementress"			}
    }
};
*/



/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[36]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 140, 12 },
    {  0,  0, 140, 13 }, /* 13  */
    {  0,  1, 170, 14 },
    {  1,  1, 170, 15 }, /* 15  */
    {  1,  2, 195, 16 },
    {  2,  3, 220, 18 },
    {  2,  4, 250, 20 }, /* 18  */
    {  3,  5, 400, 22 },
    {  3,  6, 500, 24 }, /* 20  */
    {  4,  7, 600, 26 },
    {  5,  7, 700, 28 },
    {  6,  8, 800, 30 },
    {  7,  8,1000, 33 },
    {  8,  9,1200, 36 }, /* 25   */
    {  9,  9,1400, 39 },
    { 10, 10,1600, 42 },
    { 11, 10,1800, 45 },
    { 12, 11,2000, 48 },
    { 13, 11,2300, 52 }, /* 30 */
    { 14, 12,2600, 56 },
    { 15, 12,2900, 60 },
    { 16, 13,3200, 64 },
    { 17, 13,3500, 68 }, 
    { 18, 14,3800, 72 }  /* 35  */
};



const	struct	int_app_type	int_app		[36]		=
{
    {  3, -5 },	/*  0 */
    {  5, -5 },	/*  1 */
    {  7, -4 },
    {  8, -4 },	/*  3 */
    {  9, -3 },
    { 10, -3 },	/*  5 */
    { 11, -2 },
    { 12, -2 },
    { 13, -1 },
    { 15, -1 },
    { 17, 0 },	/* 10 */
    { 19, 0 },
    { 22, 0 },
    { 25, 0 },
    { 28, 0 },
    { 31, 0 },	/* 15 */
    { 34, 0 },
    { 37, 0 },
    { 40, 0 },	/* 18 */
    { 44, 0 },
    { 49, 0 },	/* 20 */
    { 55, 0 },
    { 60, 0 },
    { 70, 0 },
    { 85, 0 },
    {105, 1 },
    {125, 1 },
    {150, 1 },
    {180, 2 },
    {215, 2 },
    {255, 2 },    /* 30 */
    {300, 2 },
    {345, 3 },
    {390, 3 },
    {445, 4 },
    {500, 5 }     /* 35 */
};



const	struct	wis_app_type	wis_app		[36]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 2 },
    { 2 },	/* 10 */
    { 2 },
    { 2 },
    { 2 },
    { 2 },
    { 3 },	/* 15 */
    { 3 },
    { 4 },
    { 4 },	/* 18 */
    { 5 },
    { 5 },	/* 20 */
    { 6 },
    { 6 },
    { 7 },
    { 7 },
    { 8 },
    { 8 },
    { 9 },
    { 9 },
    {10 },
    {10 },     /* 30 */
    {11 },
    {11 },
    {12 },
    {12 },
    {13 }      /* 35 */
};



const	struct	dex_app_type	dex_app		[36]		=
{
    {   60 },   /* 0 */
    {   55 },   /* 1 */
    {   50 },
    {   45 },
    {   40 },
    {   35 },   /* 5 */
    {   30 },
    {   25 },
    {   20 },
    {   15 },
    {   10 },   /* 10 */
    {    5 },
    {    0 },
    {    0 },
    {  - 5 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 25 },
    { - 30 },
    { - 35 },   /* 20 */
    { - 40 },
    { - 45 },
    { - 50 },
    { - 55 },
    { - 60 },
    { - 65 },
    { - 70 },
    { - 75 },
    { - 80 },
    { - 85 },   /* 30 */
    { - 90 },
    { - 95 },
    { - 100},
    { - 105},
    { - 110}    /* 35 */
};



const	struct	con_app_type	con_app		[36]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  5, 99 },
    {  6, 99 },
    {  6, 99 },
    {  7, 99 },
    {  7, 99 },
    {  8, 99 },
    {  8, 99 },
    {  9, 99 },     /* 30 */
    {  9, 99 },
    { 10, 99 },
    { 10, 99 },
    { 11, 99 },
    { 12, 99 }      /* 35 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",			"clear",	{  0, 0, 10 }	},  /*  0 */
    { "beer",			"amber",	{  3, 0,  5 }	},
    { "wine",			"rose",		{  5, 0,  5 }	},
    { "ale",			"brown",	{  2, 1,  5 }	},
    { "dark ale",		"dark",		{  1, 1,  5 }	},

    { "whisky",			"golden",	{  6, 0,  4 }	},  /*  5 */
    { "lemonade",		"pink",		{  0, 0,  8 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0 }	},
    { "local specialty",	"everclear",	{  3, 0,  3 }	},
    { "slime mold juice",	"green",	{  0, 1, -8 }	},

    { "milk",			"white",	{  0, 1,  6 }	},  /* 10 */
    { "tea",			"tan",		{  0, 0,  6 }	},
    { "coffee",			"black",	{  0, 0,  6 }	},
    { "blood",			"red",		{  0, 2, -1 }	},
    { "salt water",		"clear",	{  0, 0, -2 }	},

    { "cola",			"cherry",	{  4, 0,  5 }	}   /* 15 */
};     /*  Chaos changed water, etc. to not fill up, & cola to get drunk */


char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP
};

const sh_int monthdays[] =
{
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	body_type	body_table	[MAX_BODY]	=
 {
    { "head", "%s's head", 
      "%s's head lies here.",
      "%s's head is slowly staring into space.",
      "%s's head butt" ,
      "$n's head slowly falls off." },

    { "jaw", "%s's jaw bone", 
      "%s's jaw bone lies here.",
      "%s's jaw bone is has a few scraps of meat left on it.",
      "%s's bite" ,
      "$n's jaw drops to the floor." },

    { "eye eyeball", "%s's eye", 
      "%s's eyeball lies here.",
      "%s's eyeball looks very peircing.",
      "%s's evil-eye" ,
      "$n's eyeball pops out." },

    { "torso", "%s's torso", 
      "%s's torso rots here.",
      "%s's torso does not have too many maggots on it.",
      "%s's shoulder slam" ,
      "$n is broken into pieces." },

    { "hip", "%s's hip", 
      "%s's hip is reeking here.",
      "%s's hip has some meat left on it.",
      "%s's hip slam" ,
      "$n falls to pieces." },

    { "leg", "%s's leg", 
      "%s's bloody leg rests on the floor.",
      "%s's leg is black, blue, and red.",
      "%s's kick" ,
      "$n kicks $s leg off." },

    { "arm", "%s's arm", 
      "%s's arm is resting on the floor.",
      "%s's arm looks very strong.",
      "%s's punch" ,
      "$n's arm seems to have been unattached." },

    { "wing", "%s's wing", 
      "%s's wing rests here.",
      "%s's wing has a few cuts in it.",
      "%s's flap" ,
      "$n is flying nowhere." },

    { "tail", "%s's tail", 
      "%s's tail is laying on the floor.",
      "%s's tail looks only slightly edible.",
      "%s's tail slap" ,
      "$n can no longer wag $s tail." },

    { "tenticle", "%s's tenticle", 
      "%s's tenticle rests here.",
      "%s's tenticle looks very slimy.",
      "%s's tenticle whip" ,
      "$n's tenticle gushes bloody." },

    { "horn", "%s's horn", 
      "%s's horn rests here.",
      "%s's horn has a bit of meat stuck to it.",
      "%s's horn jab" ,
      "$n's horn bumps you as it falls to the floor." },

    { "claw", "%s's claw", 
      "%s's claw is here.",
      "%s's claw seems to have gotten quite a bit of use.",
      "%s claws",
      "The claw of $n gets clipped." },

    { "hand", "%s's hand", 
      "%s's hand is sits on the ground.",
      "%s's hand is in no shape to be reattached.",
      "%s's punch" ,
      "$n gives you the finger, right before losing $s hand." },

    { "foot", "%s's foot", 
      "%s's foot stands here.",
      "%s's foot is only slightly smelly.",
      "%s's kick" ,
      "$n stumbles, as $s foot falls off." },

 };

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

    {
	"reserved",		{ 99, 99, 99, 99, 99, 99, 99 },
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			""
    },

    {
	"acid blast",		{ 34, 99, 99, 99, 99, 99, 99 },
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	20,	16,
	"acid blast",		"!Acid Blast!"
    },

    {
	"armor",		{ 99,  1, 99, 99, 99, 99, 99 },
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 5,	16,
	"",			"Your armor returns to its mundane value."
    },

    {
	"bless",		{ 99, 99, 99, 99, 99,  4, 99 },
	spell_bless,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 3),	 15,	16,
	"",			"Your blessing fades away."
    },

    {
	"blindness",		{ 19, 99, 99, 99, 99, 99, 99 },
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	16,
	"",			"You can see again."
    },

    {
	"burning hands",	{  7, 99, 99, 99, 99, 99, 99 },
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 5),	15,	16,
	"burning hands",	"!Burning Hands!"
    },

    {
	"call lightning",	{ 99, 14, 99, 99, 99, 99, 99 },
	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	35,	16,
	"lightning bolt",	"!Call Lightning!"
    },

    {
	"cause critical",	{ 99, 99, 99, 99, 99, 20, 99 },
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	20,	16,
	"spell",		"!Cause Critical!"
    },

    {
	"cause light",		{ 99, 99, 99, 99, 99,  1, 99 },
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	15,	16,
	"spell",		"!Cause Light!"
    },

    {
	"cause serious",	{ 99, 99, 99, 99, 99, 11, 99 },
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	17,	16,
	"spell",		"!Cause Serious!"
    },

    {
	"change sex",		{ 11, 99, 99, 99, 99, 99, 99 },
	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(82),	15,	16,
	"",			"Your body feels familiar again."
    },

    {
	"charm person",		{ 99, 99, 99, 99, 24, 99, 99 },
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	16,
	"",			"You feel more self-confident."
    },

    {
	"chill touch",		{ 99,  4, 99, 99, 99, 99, 99 },
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 8),	15,	16,
	"chilling touch",	"You feel less cold."
    },

    {
	"color spray",		{ 25, 99, 99, 99, 99, 99, 99 },
	spell_color_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	16,
	"color spray",		"!Color Spray!"
    },

    {
	"continual light",	{ 12, 99, 99, 99, 99, 99, 99 },
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 7,	16,
	"",			"!Continual Light!"
    },

    {
	"control weather",	{ 99, 15, 99, 99, 99, 99, 99 },
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	16,
	"",			"!Control Weather!"
    },

    {
	"create food",		{ 99, 99, 99, 99, 99,  2, 99 },
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 5,	16,
	"",			"!Create Food!"
    },

    {
	"block area",		{ 99, 23, 99, 99, 99, 99, 99 },
	spell_block_area,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(601),	25,	16,
	"",			"!block area!"
    },

    {
	"create spring",	{ 99, 11, 99, 99, 99, 99, 99 },
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	16,
	"",			"!Create Spring!"
    },

    {
	"create water",		{ 99,  2, 99, 99, 99, 99, 99 },
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 5,	16,
	"",			"!Create Water!"
    },

    {
	"cure blindness",	{ 99, 99, 99, 99, 99,  5, 99 },
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	16,
	"",			"!Cure Blindness!"
    },

    {
	"cure critical",	{ 99, 99, 99, 99, 99, 12, 99 },
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	20,	16,
	"",			"!Cure Critical!"
    },

    {
	"cure light",		{ 96, 96, 96, 96, 96,  3, 96 },
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	10,	16,
	"",			"!Cure Light!"
    },

    {
	"cure poison",		{ 99, 99, 99, 99, 99, 15, 99 },
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(43),	 5,	16,
	"",			"!Cure Poison!"
    },

    {
	"cure serious",		{ 99, 99, 99, 99, 99,  7, 99 },
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	15,	16,
	"",			"!Cure Serious!"
    },

    {
	"curse",		{ 99, 99, 99, 99, 11, 99, 99 },
	spell_curse,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	20,	16,
	"curse",		"The curse wears off."
    },

    {
	"detect evil",		{ 99, 99, 99, 99, 99,  6, 99 },
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 5,	16,
	"",			"The red outlines fade from your vision."
    },

    {
	"detect hidden",	{  8, 99, 99, 99, 99, 99, 99 },
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(44),	 5,	16,
	"",			"You feel less aware of your surroundings."
    },

    {
	"detect invis",		{  3,  99, 99, 99, 99, 99, 99 },
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	16,
	"",			"You no longer see invisible objects."
    },

    {
	"detect magic",		{ 99, 99, 99, 99,  1, 99, 99 },
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 5,	16,
	"",			"The blue outlines disappear from your vision."
    },

    {
	"detect poison",	{ 99, 99, 99, 99, 99,  8, 99 },
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 5,	16,
	"",			"!Detect Poison!"
    },

    {
	"dispel evil",		{ 99, 99, 99, 99, 99, 16, 99 },
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	15,	16,
	"dispel evil",		"!Dispel Evil!"
    },

    {
	"dispel magic",		{ 99, 99, 99, 99,  9, 99, 99 },
	spell_dispel_magic,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	15,	16,
	"",			"!Dispel Magic!"
    },

    {
	"earthquake",		{ 99, 13, 99, 99, 99, 99, 99 },
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	15,	16,
	"earthquake",		"!Earthquake!"
    },

    {
	"enchant weapon",	{ 99, 99, 99, 99, 12, 99, 99 },
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	32,
	"",			"!Enchant Weapon!"
    },

    {
        "object invis",         { 15, 99, 99, 99, 99, 99, 99 },
        spell_invis_obj,        TAR_OBJ_INV,            POS_STANDING,
        NULL,                   SLOT(612),      100,    32,
        "",                     "!Object Invis!"
    },

    {
	"energy drain",		{ 99, 99, 99, 99, 19, 99, 99 },
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	16,
	"energy drain",		"!Energy Drain!"
    },

    {
	"faerie fire",		{  4, 99, 99, 99, 99, 99, 99 },
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	16,
	"faerie fire",		"The pink aura around you fades away."
    },

    {
	"faerie fog",		{ 16, 99, 99, 99, 99, 99, 99 },
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	12,	16,
	"faerie fog",		"!Faerie Fog!"
    },

    {
	"breath water",		{ 99, 38, 99, 99, 99, 99, 99 },
	spell_breath_water,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(635),	15,	16,
	"breath water",		"You can no longer breath water."
    },

    {
	"mage blast",		{ 99, 99, 99, 99, 43, 99, 99 },
	spell_mage_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_mage_blast,	SLOT(636),	45,	12,
	"mage blast",		"You feel less vulnerable."
    },

    {
	"fireball",		{ 99, 21, 99, 99, 99, 99, 99 },
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	16,
	"fireball",		"!Fireball!"
    },

    {
	"flamestrike",		{ 99, 99, 99, 99, 99, 23, 99 },
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	16,
	"flamestrike",		"!Flamestrike!"
    },

    {
	"fly",			{ 99,  7, 99, 99, 99, 99, 99 },
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	10,	24,
	"",			"You slowly float to the ground."
    },

    {
	"gate",			{ 99, 99, 99, 99, 99, 99, 99 },
	spell_gate,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(83),	50,	16,
	"",			"!Gate!"
    },

    {
	"giant strength",	{ 99, 99, 99, 99, 99, 14, 99 },
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(39),	20,	16,
	"",			"You feel weaker."
    },

    {
	"harm",			{ 99, 99, 99, 99, 99, 29, 99 },
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	35,	16,
	"harm spell",		"!Harm!"
    },

    {
	"heal",			{ 99, 99, 99, 99, 99, 21, 99 },
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	50,	16,
	"",			"!Heal!"
    },

    {
	"homonculous",		{ 99, 99, 99, 99, 18, 99, 99 },
	spell_homonculous,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(603),	90,	32,
	"",			"!homonculous!"
    },

    {
	"identify",		{ 99, 99, 99, 99, 10, 99, 99 },
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	12,	32,
	"",			"!Identify!"
    },

    {
	"infravision",		{ 99, 19, 99, 99, 99, 99, 99 },
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	24,
	"",			"You no longer see in the dark."
    },

    {
	"invis",		{ 13, 99, 99, 99, 99, 99, 99 },
	spell_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 5,	16,
	"",			"You are no longer invisible."
    },

    {
	"know alignment",	{ 99, 99, 99, 99, 99,  9, 99 },
	spell_know_alignment,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(58),	 9,	16,
	"",			"!Know Alignment!"
    },

    {
	"lightning bolt",	{ 17, 99, 99, 99, 99, 99, 99 },
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_zap,		SLOT(30),	15,	16,
	"lightning bolt",	"!Lightning Bolt!"
    },

    {
	"locate object",	{ 14, 99, 99, 99, 99, 99, 99 },
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	20,	24,
	"",			"!Locate Object!"
    },

    {
	"magic missile",	{  1, 99, 99, 99, 99, 99, 99 },
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(32),	15,	16,
	"magic missile",	"!Magic Missile!"
    },

    {
	"mass invis",		{ 20, 99, 99, 99, 99, 99, 99 },
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	20,	36,
	"",			"!Mass Invis!"
    },

    {
	"mass",		        { 46, 99, 99, 99, 99, 46, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_mass,		SLOT( 0),	 0,	 0,
	"",			"!mass!"
    },

    {
	"pass door",		{ 99, 24, 99, 99, 99, 99, 99 },
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(74),	20,	16,
	"",			"You feel solid again."
    },

    {
	"poison",		{ 99, 99, 99, 99,  6, 99, 99 },
	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	10,	16,
	"poison",		"You feel less sick."
    },

    {
	"protection from evil",	{ 99, 99, 99, 99, 99,  9, 99 },
	spell_protection_fe,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34),	 5,	16,
	"",			"You feel less protected from evil."
    },

    {
	"refresh",		{ 99, 99, 99, 99, 99,  8, 99 },
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	24,
	"refresh",		"!Refresh!"
    },

    {
	"remove curse",		{ 99, 99, 99, 99, 99, 13, 99 },
	spell_remove_curse,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(35),	 5,	16,
	"",			"!Remove Curse!"
    },

    {
	"rift",			{ 99, 99, 99, 99, 27, 99, 99 },
	spell_rift,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(608),	90,	50,
	"",			"!rift!"
    },

    {
	"rip",			{ 99, 99, 99, 99, 20, 99, 99 },
	spell_rip,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(609),	75,	25,
	"",			"!rip!"
    },

    {
	"sanctuary",		{ 99, 99, 99, 99, 99, 27, 99 },
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(36),	75,	16,
	"",			"The white aura around your body fades."
    },

    {
	"sanctify",		{ 99, 99, 99, 99, 99, 53, 99 },
	spell_sanctify,	        TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(638),	75,	16,
	"",			""
    },


    {
	"shield",		{ 99, 99, 99, 99, 15, 99, 99 },
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(67),	12,	24,
	"",			"Your force shield shimmers then fades away."
    },

    {
	"shocking grasp",	{ 10, 99, 99, 99, 99, 99, 99 },
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(37),	15,	16,
	"shocking grasp",	"!Shocking Grasp!"
    },

    {
	"sleep",		{ 99, 99, 99, 99, 16, 99, 99 },
	spell_sleep,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	16,
	"",			"You feel less tired."
    },

    {
	"stone skin",		{ 99, 25, 99, 99 , 99, 99, 99},
	spell_stone_skin,	TAR_CHAR_DEFENSIVE,		POS_STANDING,
	NULL,			SLOT(66),	12,	24,
	"",			"Your skin feels soft again."
    },

    {
	"summon",		{ 99, 99, 99, 99,  8, 99, 99 },
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(76),	50,	16,
	"",			"!Summon!"
    },

    {
        "demon",                { 99, 99, 99, 99, 13, 99, 99 },
        spell_demon,            TAR_IGNORE,         POS_STANDING,
        NULL,                   SLOT(604),       90,     16,
        "",                     "!Demon!"
    },

    {
        "tongues",              { 99,  5, 99, 99, 99, 99, 99 },
        spell_tongues,          TAR_CHAR_DEFENSIVE,           POS_STANDING,
        NULL,                   SLOT(610),      10,      8,
        "",                     "You no longer have a tongue!"
    },

    {
        "understand",           { 99, 99, 99, 99,  5, 99, 99 },
        spell_understand,       TAR_CHAR_DEFENSIVE,        POS_STANDING,
        NULL,                   SLOT(611),      10,      8,
        "",                     "You lose all understanding!"
    },

    {
        "beast",                { 14, 99, 99, 99, 99, 99, 99 },
        spell_beast,            TAR_IGNORE,           POS_STANDING,
        NULL,                   SLOT(605),       20,    16,
        "",                     "!Beast!"
    },

    {
        "shade",               { 18, 99, 99, 99, 99, 99, 99 },
        spell_shade,           TAR_IGNORE,           POS_STANDING,
        NULL,                  SLOT(606),        50,    16,
        "",                    "!Shade!"
    },

    {
        "phantasm",            { 23, 99, 99, 99, 99, 99, 99 },
        spell_phantasm,        TAR_IGNORE,          POS_STANDING,
        NULL,                  SLOT(607),       30,     16,
        "",                    "!Phantasm!"
    },

    {
	"teleport",		{ 99, 16, 99, 99, 99, 99, 99 },
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 2),	35,	16,
	"",			"!Teleport!"
    },

    {
	"ventriloquate",	{  2, 99, 99, 99, 99, 99, 99 },
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	16,
	"",			"!Ventriloquate!"
    },

    {
	"weaken",		{ 99, 99, 99, 99,  4, 99, 99 },
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	20,	16,
	"spell",		"You feel stronger."
    },

    {
	"word of recall",	{ 99, 99, 99, 99, 99, 99, 99 },
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 5,	16,
	"",			"!Word of Recall!"
    },

    {
	"write spell",		{ 99, 99, 99, 99, 17, 99, 99 },
	spell_write_spell,	TAR_IGNORE,		POS_RESTING,
	&gsn_write_spell,	SLOT(602),	 5,	16,
	"",			"!write spell!"
    },

    {
	"stability",		{ 99, 17, 99, 99, 99, 99, 99 },
	spell_stability,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_stability,		SLOT(632),	 10,	16,
	"",			"You no longer feel the strength of the earth."
    },

/*
 * Dragon breath
 */
    {
	"acid breath",		{ 99, 90, 99, 99, 99, 99, 99 },
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(200),	 30,	 16,
	"blast of acid",	"!Acid Breath!"
    },

    {
	"fire breath",		{ 99, 99, 99, 99, 99, 99, 99 },
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(201),	 0,	 6,
	"blast of flame",	"!Fire Breath!"
    },

    {
	"frost breath",		{ 99, 99, 99, 99, 99, 99, 99 },
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	 0,	 6,
	"blast of frost",	"!Frost Breath!"
    },

    {
	"gas breath",		{ 99, 89, 99, 99, 99, 99, 99 },
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(203),	 30,	 16,
	"blast of gas",		"!Gas Breath!"
    },

    {
	"lightning breath",	{ 99, 99, 99, 99, 99, 99, 99 },
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(204),	 0,	 6,
	"blast of lightning",	"!Lightning Breath!"
    },

/*
 * Fighter and thief skills.
 */
    {
	"backstab",		{ 99, 99,  3, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_backstab,		SLOT( 0),	32,	32,
	"backstab",		"!Backstab!", "backstab"
    },

    {
	"disarm",		{ 99, 99,  9, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_disarm,		SLOT( 0),	64,	32,
	"",			"!Disarm!", "disarm"
    },

    {
	"dodge",		{ 99, 99,  6, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_dodge,		SLOT( 0),	 0,	 0,
	"",			"!Dodge!"
    },

    {
	"enhanced damage",	{ 99, 99, 99,  9, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_enhanced_damage,	SLOT( 0),	 0,	 0,
	"",			"!Enhanced Damage!"
    },

    {
	"hide",			{ 99, 99, 99, 99, 99, 99, 1 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	16,	16,
	"",			"!Hide!", "hide"
    },

    {
	"martial arts",		{ 99, 99, 99, 41, 99, 99, 99 },
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_martial_arts,	SLOT( 0),	 24,	20,
	"martial arts",		"!martial arts!", "martial arts"
    },

    {
	"kick",			{ 99, 99, 99,  1, 99, 99, 99 },
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_kick,		SLOT( 0),	 24,	32,
	"kick",			"!Kick!", "kick"
    },

    {
	"parry",		{ 99, 99, 99,  3, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_parry,		SLOT( 0),	 0,	 0,
	"",			"!Parry!"
    },

    {
	"peek",			{ 99, 99,  7, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!"
    },

    {
	"pick lock",		{ 99, 99,  1, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 8,	16,
	"",			"!Pick!", "pick"
    },

    {
	"rescue",		{ 99, 99, 99,  4, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_rescue,		SLOT( 0),	 48,	16,
	"",			"!Rescue!", "rescue"
    },

    {
	"second attack",	{ 99, 99,  8,  2, 99, 99, 15 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_second_attack,	SLOT( 0),	 0,	 0,
	"",			"!Second Attack!"
    },

    {
	"fourth attack",	{ 99, 99,  99,  81, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_fourth_attack,	SLOT( 0),	 0,	 0,
	"",			"!fourth Attack!"
    },

    {
	"sneak",		{ 99, 99, 99, 99, 99, 99,  3 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 8,	16,
	"",			NULL, "sneak"
    },

    {
	"climb",		{ 99, 99, 33, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_climb,		SLOT( 0),	 5,	5,
	"",			"!Climb!", "climb"
    },

    {
	"steal",		{ 99, 99,  2, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 24,	32,
	"",			"!Steal!", "steal"
    },

    {
	"third attack",		{ 99, 99, 99,  7, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_third_attack,	SLOT( 0),	 0,	 0,
	"",			"!Third Attack!"
    },

    {
	"shoot",		{ 99, 99, 99, 99, 99, 99,  7 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_shoot,		SLOT( 0),	16,	 32,
	"shot",  		"!shoot!", "shoot"
    },

    {
        "track",                { 99, 99, 99, 14, 99, 99, 99 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_track,             SLOT(0),         8,      32,
        "track",                "!track!", "track"
    },

    {
        "notice",                { 99, 99, 99, 17, 99, 99, 99 },
        spell_null,             TAR_IGNORE,           POS_STANDING,
        &gsn_notice,            SLOT(0),        8,      32,
        "notice",               "!notice!", "notice"
    },

    {
	"throw",		{ 99, 99,  4, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_throw,		SLOT( 0),	16,	 32,
	"throw",  		"!throw!", "throw"
    },

    {
	"short range",		{  5,  6,  5,  5,  3,  5,  5 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_short_range,	SLOT( 0),	 0,	 0,
	"",			"!srange!"
    },

    {
	"medium range",		{ 15, 12, 12, 12, 14, 18, 13},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_med_range,		SLOT( 0),	 0,	 0,
	"",			"!mrange!"
    },

    {
	"long range",		{ 21, 22, 21, 21, 21, 25, 21 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_long_range,	SLOT( 0),	 0,	 0,
	"",			"!lrange!"
    },

   {
        "rearm",               { 99, 99, 99, 11, 99, 99, 99 },
        spell_null,             TAR_IGNORE,            POS_STANDING,
        &gsn_rearm,             SLOT(0),        0,      0,
        "",                     "!rearm!"
   },

    {
	"shadow",		{ 99, 99, 99, 99, 99, 99,  9 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_shadow,		SLOT( 0),	 8,	 0,
	"",			"!shadow!", "shadow"
    },

    {
	"divert",		{ 99, 99, 99, 99, 99, 99, 11 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_divert,		SLOT( 0),	 48,	 12,
	"",			"!divert!", "divert"
    },

    {
	"voice",		{ 99, 99, 99, 99, 99, 99, 12 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_voice,		SLOT( 0),	 0,	 0,
	"",			"!voice!"
    },

    {
	"bargain",		{ 99, 99, 11, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_bargain,		SLOT( 0),	 0,	 0,
	"",			"!bargain!"
    },

    {
	"lock lock",		{ 99, 99, 13, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_lock,		SLOT( 0),	 0,	 0,
	"",			"!lock lock!"
    },

    {
	"knife",		{ 99, 99, 99, 99, 99, 99, 15 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_knife,		SLOT( 0),	24,	40,
	"knife",		"!knife!", "knife"
    },

    {
	"greater hide",		{ 99, 99, 99, 99, 99, 99, 17 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_hide,	SLOT( 0),	 0,	 0,
	"",			"!greater hide!"
    },

    {
	"greater sneak",	{ 99, 99, 99, 99, 99, 99, 19 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_sneak,		SLOT( 0),	 0,	 0,
	"",			"!greater sneak!"
    },

    {
	"makepoison",		{ 99, 99, 99, 99, 99, 99, 33 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_make_poison,	SLOT( 0),	 16,	 32,
	"poison",               "!Poison !"
    },

    {
	"stealth",		{ 99, 99, 99, 99, 99, 99, 21 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_stealth,		SLOT( 0),	 16,	 32,
	"",			NULL, "stealth"
    },

    {
	"disguise",		{ 99, 99, 99, 99, 99, 99, 26 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_disguise,		SLOT( 0),	 0,	 0,
	"",			"!disguise!"
    },

    {
	"spy",			{ 99, 99, 99, 99, 99, 99, 29 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_spy,		SLOT( 0),	 8,	 16,
	"",			"!spy!", "spy"
    },

    {
	"attack",		{ 99, 99, 14, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_attack,		SLOT( 0),	 0,	 0,
	"",			"!attack!"
    },

    {
	"armor usage",		{ 99, 99, 16, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_armor_usage,	SLOT( 0),	 0,	 0,
	"",			"!armor usage!"
    },

    {
	"distract",		{ 99, 99, 19, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_distract,		SLOT( 0),	 24,	 8,
	"",			"!distract!", "distract"
    },

    {
	"greater peek",		{ 99, 99, 21, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_peek,	SLOT( 0),	 0,	 0,
	"",			"!greater peek!"
    },

    {
	"snatch",		{ 99, 99, 23, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_snatch,		SLOT( 0),	 4,	 8,
	"",			"!snatch!", "snatch"
    },

    {
	"greater pick",		{ 99, 99, 25, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_pick,	SLOT( 0),	 0,	 0,
	"",			"!greater pick!"
    },

    {
        "clear path",           { 99, 99, 99, 13, 99, 99, 99 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_clear_path,        SLOT( 0),        24,       32,
       "",                      "You stop clearing paths.", "clear"
    },
 
    {
        "hunt",                 { 99, 99, 99,  6, 99, 99, 99 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_hunt,              SLOT( 0),        16,       32,
        "",                     "You stop hunting for today.", "hunt"
    },

    {
        "haste",                 { 99, 13, 99, 99, 99, 99, 99 },
	spell_haste       ,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(613),	 10,	32,
	"",			"You slow down."
    },

    {
	"enhanced rest",	{ 99, 35, 99, 99, 99, 99, 99 },
	spell_enhanced_rest,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(614),	10,	32,
	"",			"You no longer rest easily."
    },

    {
	"enhanced heal",	{ 96, 96, 96, 96, 96, 35, 96 },
	spell_enhanced_heal,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(615),	10,	32,
	"",			"You no longer heal easily."
    },

    {
	"enhanced revive",	{ 99, 99, 99, 99, 35, 99, 99 },
	spell_enhanced_revive,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(616),	10,	32,
	"",			"You no longer revive easily."
    },

    {
	"animate dead",	{ 99, 99, 99, 99, 38, 99, 99 },
	spell_animate_dead,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(617),	20,	16,
	"",			""
    },

    {
	"banish",	{ 99, 99, 99, 99, 31, 99, 99 },
	spell_banish,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(618),	80,	32,
	"",			""
    },

    {
	"enhance object",	{ 99, 99, 99, 99, 25, 99, 99 },
	spell_enhance_object,	TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(619),	80,	16,
	"",			""
    },

    {
	"mage shield",	{ 99, 99, 99, 99, 23, 99, 99 },
	spell_mage_shield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(620),	80,	16,
	"",		"The shimmering glow fades from about your body."
    },

    {
	"dispel undead",	{ 99, 99, 99, 99, 99, 17, 99 },
	spell_dispel_undead,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(621),	20,	10,
	"dispel",			"!Dispel Undead!"
    },

    {
	"dispel good",	{ 99, 99, 99, 99, 99, 37, 99 },
	spell_dispel_good,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(622),	20,	10,
	"dispel",			"!Dispel Good!"
    },

    {
	"remove fear",	{ 99, 99, 99, 99, 99, 24, 99 },
	spell_remove_fear,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(623),	40,	20,
	"",			""
    },

    {
	"feast",	{ 96, 96, 96, 96, 96, 32, 96 },
	spell_feast,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(624),	20,	10,
	"",			""
    },

    {
	"restore",	{ 99, 99, 99, 99, 99, 45, 99 },
	spell_restore,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(625),	20,	20,
	"",			""
    },

    {
	"tremor",		{ 99, 43, 99, 99, 99, 99, 99 },
	spell_tremor,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(626),	45,	24,
	"Tremor",		"!Tremor!"
    },

    {
	"energy shift",	{ 99, 99, 99, 99, 33, 99, 99 },
	spell_energy_shift,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(627),	20,	20,
	"",			""
    },

    {
        "induction",                 { 99, 32, 99, 99, 99, 99, 99 },
	spell_induction   ,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(628),	 20,	 1,
	"",			""
    },

    {
	"assassinate",		{ 99, 99, 99, 99, 99, 99, 36 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_assassinate,       SLOT( 0),	24,	40,
	"",                     "!assassinate!", "assassinate"
    },

    {
	"glance",		{ 99, 99, 99, 99, 99, 99, 27 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_glance,		SLOT( 0),	 0,	 0,
	"",			"!glance!"
    },

    {
	"guard",		{ 99, 99, 99, 99, 99, 99, 31 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_guard,		SLOT( 0),	 0,	 0,
	"",			"!guard!"
    },

    {
	"repair",		{ 99, 99, 81, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_repair,		SLOT( 0),	 0,     50,
	"",			"!repair!"
    },

    {
	"forge",		{ 99, 99, 36, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_forge,		SLOT( 0),	 0,     150,
	"",			"!forge!"
    },

    {
	"detect forgery",	{ 99, 99, 27, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_detect_forgery,	SLOT( 0),	 0,	 0,
	"",			"!detect forgery!"
    },

    {
	"plant",		{ 99, 99, 31, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_plant,		SLOT( 0),	 0,	 0,
	"",			"!plant!"
    },

    {
	"camp",		        { 99, 99, 99, 31, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_camp,		SLOT( 0),	 50,	 64,
	"",			"!camp!"
    },

    {
	"illusion",		{ 36, 99, 99, 99, 99, 99, 99 },
	spell_illusion,	        TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(629),          75,     25,
	"illusion",		"!illusion!"
    },

    {
	"mirror image",		{ 27, 99, 99, 99, 99, 99, 99 },
	spell_mirror_image,     TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(630),          80,     32,
	"mirror image",		"Your images fold back into you."
    },

    {
	"confusion",		{ 42, 99, 99, 99, 99, 99, 99 },
	spell_confusion,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(637),          30,     45,
	"confusion",		"Your eyes refocus."
    },

    {
	"hallucinate",		{ 31, 99, 99, 99, 99, 99, 99 },
	spell_hallucinate,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(631),          25,     25,
	"hallucinate",		"Reality shifts back into focus."
    },

    {
	"ethereal travel",	{ 99, 99, 99, 99, 29, 99, 99 },
	spell_ethereal,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(633),          35,     25,
	"",		"You become solid."
    },

    {
	"astral projection",	{ 29, 99, 99, 99, 99, 99, 99 },
	spell_astral,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(634),          45,     25,
	"",		"You step back into your body."
    },

    {
	"berserk",	        { 99, 99, 99, 51, 99, 99, 99 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_berserk,		SLOT( 0),	 24,	32,
	"berserk",	        "You start to feel more rational.", "berserk"
    },

    {
	"critical hit",	        { 99, 99, 99, 99, 99, 99, 41 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_critical_hit,	SLOT( 0),	 24,	32,
	"critical hit",	   "Your wounds start to look better.","critical hit"
    },

/* New skills added by Martin 4/8/98 */

    {
	"greater backstab",     { 99, 99, 71, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_backstab,	SLOT( 0),	 0,	 0,
	"",			"!greater backstab!"
    },

    {
	"gouge",		{ 99, 99, 46, 99, 99, 99, 99 },
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_gouge,		SLOT( 0),	20,	40,
	"gouge",		"!gouge!", "gouge"
    },

    {
	"circle",		{ 99, 99, 91, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_circle,		SLOT( 0),	90,	55,
	"circle",		"!circle!", "circle"
    },

    {
	"trip",			{ 99, 99, 22, 99, 99, 99, 99 },
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	 60,	40,
	"trip",			"!trip!", "trip"
    },

    {
	"anatomy",		{ 99, 99, 41, 99, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_anatomy,		SLOT( 0),	 0,	 0,
	"",			"!anatomy!"
    },

/* New skills added by Martin 5/8/98 */

    {
	"soothing touch",	{ 99, 99, 99, 99, 99, 81, 99 },
	spell_soothing_touch,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(639),	150,	16,
	"",			"!Soothing Touch!"
    },

    {
	"benediction",		{ 99, 99, 99, 99, 99,  61, 99 },
	spell_benediction,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 640),	 45,	16,
	"",			"The powerful blessing of the gods fades away."
    },

    {
	"righteous fury",	{ 99, 99, 99, 99, 99,  31, 99 },
	spell_righteous_fury,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 641),	 75,	16,
	"",			"Your righteous fury abruptly leaves you, you feel drained and weak."
    },

    {
	"farheal",	        { 99, 99, 99, 99, 99, 91, 99 },
	spell_farheal,		TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(642),	200,	65,
	"",			"!Far Heal!"
    },


/* Leaving these uncoded until another day...

Basic Idea:

The target is first_affect for small number of hours, like 4/5 when the monk is 
95th level. Holy word increases the affect of cure light/cure serious/cure
critical by 50% while unholy word increases the affect of cause light/cause
serious/cause critical

Martin
*/
    {

	"unholy word",	 	{ 98, 98, 98, 98, 98, 98, 98 },
	spell_unholy_word,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(643),	17,	16,
	"",			"!Unholy Word!"
    },

    {
	"holy word",	 	{ 98, 98, 98, 98, 98, 98, 98 },
	spell_holy_word,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(644),	17,	16,
	"",		 	"!Holy Word!"
    },

    {
	"invigorate",		{ 99, 99, 99, 99, 99,  41, 99 },
	spell_invigorate,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(645),	48,	32,
	"invigorate",		"!Invigorate!"
    },

/* New Assassin skills 6/8/98 Martin */

    {
	"greater stealth",	{ 99, 99, 99, 99, 99, 99, 91 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_stealth,	SLOT( 0),	 0,	 0,
	"",			NULL,
    },

    {
        "quick draw",           { 99, 99, 99, 99, 99, 99, 56 },
        spell_null,             TAR_IGNORE,        POS_STANDING,
        &gsn_quick_draw,        SLOT(0),        0,      0,
        "",                     "!quick draw!"
    },

    {
	"flash powder",	 	{ 99, 99, 99, 99, 99, 99, 61 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_flash_powder,	SLOT( 0),	 0,	 0,
	"",		 	NULL	
    },

    {
	"muffle",	 	{ 99, 99, 99, 99, 99, 99, 39 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_muffle,		SLOT( 0),	 0,	 0,
	"",		 	NULL	
    },
    {
	"pass without trace",	{ 99, 99, 99, 99, 99, 99, 71 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pass_without_trace, SLOT( 0),	 0,	 0,
	"",			NULL
    },

/* New Illusionist skills 6/8/98 */
    {
	"improved invis",	{ 51, 99, 99, 99, 99, 99, 99 },
	spell_improved_invis,	TAR_CHAR_SELF,	POS_STANDING,
	&gsn_improved_invis,	SLOT(646),	45,	24,
	"",			"The shroud of invisibility fades from about your body."
    },

    {
	"truesight",		{  71,  99, 99, 99, 99, 99, 99 },
	spell_truesight,	TAR_CHAR_SELF,	POS_STANDING,
	&gsn_truesight,		SLOT(647),	 50,	24,
	"",			"The clarity in your vision fades away."
    },

/* New Necromancer spells 7/8/98 */
    {
	"recharge",		{ 99, 99, 99, 99,  61, 99, 99 },
	spell_recharge,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(648),	50,	16,
	"",			"!recharge!"
    },

    {
	"transport",		{ 99, 99, 99, 99,  76, 99, 99 },
	spell_transport,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(649),	50,	16,
	"",			"!transport!"
    },

    {
	"anti-magic shell",	{ 99, 99, 99, 99, 91, 99, 99 },
	spell_anti_magic_shell,	TAR_CHAR_SELF,	POS_STANDING,
	&gsn_anti_magic_shell,	SLOT(650),	80,	16,
	"",		"The shimmering glow fades from about your body."
    },

    {
	"possess",		{ 99, 99, 99, 99, 81, 99, 99 },
	spell_possess,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(651),          25,     25,
	"",		        "Your mind is wrenched back to it's rightful home."
    },
    {
	"hallucinatory terrain",	{ 81, 99, 99, 99,  99, 99, 99 },
	spell_hallucinatory_terrain,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(652),	120,	16,
	"spell",		NULL
    },
    {
	"nightmare",		{ 91, 99, 99, 99,  99, 99, 99 },
	spell_nightmare,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_nightmare,		SLOT(653),	20,	16,
	"spell",		"Your panic recedes and your breathing returns to normal."
    },

    {
	"smoke",		{ 61, 99, 99, 99, 99, 99, 99 },
	spell_smoke,	 	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(654),	80,	16,
	"smoke",		"!Smoke!"
    },





/* New ele spells 18/8/98 Martin */

    {
        "slow",                 { 99, 41, 99, 99, 99, 99, 99 },
	spell_slow       ,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	&gsn_slow,		SLOT(655),	 10,	32,
	"",			"You speed up."
    },

    {
        "brew potion",          { 99, 46, 99, 99, 99, 99, 99 },
	spell_brew_potion,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(656),	 10,	32,
	"",			NULL
    },

    {
        "elemental",          { 99, 61, 99, 99, 99, 99, 99 },
	spell_elemental,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(657),	 10,	32,
	"",			NULL
    },

    {
        "fireshield",     	{ 99, 91, 99, 99, 99, 99, 99 },
	spell_fire_shield,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_fire_shield,	SLOT(658),	 10,	32,
	"fireshield",		"The flames around your body go out."
    },
    {
        "unbarring ways",     { 99, 71, 99, 99, 99, 99, 99 },
	spell_unbarring_ways,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(659),	 10,	32,
	"",			NULL
    },

    {
	"vampiric touch",	{ 99, 99, 99, 99, 52, 99, 99 },
	spell_vampiric_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(660),	35,	20,
	"vampiric touch",	"!Vampiric Touch!"
    },

    {
	"bash",			{ 99, 99, 99,  71, 99, 99, 99 },
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_bash,		SLOT( 0),	 50,	32,
	"bash",			"!bash!", "bash"
    },

    {
	"doorbash",		{ 99, 99,  99, 36, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_bashdoor,		SLOT( 0),	 60,	24,
	"bashdoor",		"!door bash!", "bashdoor"
    },

    {
	"greater hunt",		{ 99, 99,  99, 61, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_hunt,	SLOT( 0),	 0,	0,
	"",			"!greater hunt!"
    },

    {
	"greater track",	{ 99, 99, 99, 46, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_greater_track,	SLOT( 0),	 0,	 0,
	"",			"!greater track!"
    },

    {
	"dual wield",		{ 99, 99, 99, 91, 99, 99, 99 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_dual_wield,	SLOT( 0),	 0,	 0,
	"",			"!dual wield!"
    },

    {
	"protection from good",	{ 99, 99, 99, 99, 99, 30, 99 },
	spell_protection_fg,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(661),	 12,	16,
	"",			"You feel less protected from good."
    },
/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
	"general purpose",	{ 99, 99, 99, 99, 99, 99, 99 },
	spell_general_purpose,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(501),	0,	16,
	"general purpose ammo",	"!General Purpose Ammo!"
    },

    {
	"high explosive",	{ 99, 99, 99, 99, 99, 99, 99 },
	spell_high_explosive,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(502),	0,	16,
	"high explosive ammo",	"!High Explosive Ammo!"
    },

/* THIS IS JUST TO GET SLOT( -1) ...IGNORE IT OTHERWISE */
    {
	"slot -1",		{ 99, 99, 99, 99, 99, 99, 99 },
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( -1),	 0,	 0,
	"",			""
    }


};
const	struct	topic_data topic_table	[MAX_TOPIC]	=
{
 {"Announcements", 		1},
 {"Game Changes and Ideas", 	5},
 {"Area Creators", 		5},
 {"Game Bugs", 			5},
 {"Complaints", 		10},
 {"Chat", 			5}
};

const	struct	bitvector_type bitvector_table	[MAX_BITVECTOR]	=
{
   { "FALSE",		0	},
   { "TRUE",		1	},
   { "NOT_POISONED",		0	},
   { "POISONED",		1	},
   { "GOD_NEUTRAL",		0	},
   { "GOD_ORDER",		1	},
   { "GOD_CHAOS",		2	},
   { "GOD_DEMISE",		3	},
   { "GOD_POLICE",		4	},
   { "GOD_CREATOR",		5	},
   { "GOD_INIT_ORDER",		6	},
   { "GOD_INIT_CHAOS",		7	},
   { "AFLAG_NODEBUG",		1	},
   { "AFLAG_NOTELEPORT",		2	},
   { "AFLAG_NOGOHOME",		4	},
   { "AFLAG_NORECALL",		8	},
   { "AFLAG_NOCASTLE",		16	},
   { "AFLAG_NORIP",		32	},
   { "AFLAG_FREEQUIT",		64	},
   { "ACT_IS_NPC",		1	},
   { "ACT_SENTINEL",		2	},
   { "ACT_SCAVENGER",		4	},
   { "ACT_AGGRESSIVE",		32	},
   { "ACT_STAY_AREA",		64	},
   { "ACT_WIMPY",		128	},
   { "ACT_PET",		256	},
   { "ACT_TRAIN",		512	},
   { "ACT_PRACTICE",		1024	},
   { "ACT_WEAK",		2048	},
   { "ACT_SMART",		4096	},
   { "ACT_ONE_FIGHT",		8192	},
   { "ACT_NO_ORDER",		16384	},
   { "ACT_RIDE",		32768	},
   { "ACT_BODY",		65536	},
   { "ACT_RACE",		131072	},
   { "ACT_UNDEAD",		262144	},
   { "ACT_ELEMENTAL",		524288	},
   { "ACT_CLAN_GUARD",		1048576	},
   { "ACT_CLAN_HEALER",		2097152	},
   { "ACT_PROTOTYPE",		4194304	},
   { "ACT_WILL_DIE",		16777216	},
   { "AFF_BLIND",		1	},
   { "AFF_INVISIBLE",		2	},
   { "AFF_DETECT_EVIL",		4	},
   { "AFF_DETECT_INVIS",		8	},
   { "AFF_DETECT_MAGIC",		16	},
   { "AFF_DETECT_HIDDEN",		32	},
   { "AFF_HOLD",		64	},
   { "AFF_SANCTUARY",		128	},
   { "AFF_FAERIE_FIRE",		256	},
   { "AFF_INFRARED",		512	},
   { "AFF_CURSE",		1024	},
   { "AFF_UNDERSTAND",		2048	},
   { "AFF_POISON",		4096	},
   { "AFF_PROTECT_EVIL",		8192	},
   { "AFF_PROTECT_GOOD",		16384	},
   { "AFF_SNEAK",		32768	},
   { "AFF_HIDE",		65536	},
   { "AFF_SLEEP",		131072	},
   { "AFF_CHARM",		262144	},
   { "AFF_FLYING",		524288	},
   { "AFF_PASS_DOOR",		1048576	},
   { "AFF_STEALTH",		2097152	},
   { "AFF_CLEAR",		4194304	},
   { "AFF_HUNT",		8388608	},
   { "AFF_TONGUES",		16777216	},
   { "AFF_ETHEREAL",		33554432	},
   { "SEX_NEUTRAL",		0	},
   { "SEX_MALE",		1	},
   { "SEX_FEMALE",		2	},
   { "ITEM_TYPE_NOTHING",		0	},
   { "ITEM_TYPE_LIGHT",		1	},
   { "ITEM_TYPE_SCROLL",		2	},
   { "ITEM_TYPE_WAND",		3	},
   { "ITEM_TYPE_STAFF",		4	},
   { "ITEM_TYPE_WEAPON",		5	},
   { "ITEM_TYPE_TREASURE",		8	},
   { "ITEM_TYPE_ARMOR",		9	},
   { "ITEM_TYPE_POTION",		10	},
   { "ITEM_TYPE_FURNITURE",		12	},
   { "ITEM_TYPE_TRASH",		13	},
   { "ITEM_TYPE_CONTAINER",		15	},
   { "ITEM_TYPE_DRINK_CON",		17	},
   { "ITEM_TYPE_KEY",		18	},
   { "ITEM_TYPE_FOOD",		19	},
   { "ITEM_TYPE_MONEY",		20	},
   { "ITEM_TYPE_BOAT",		22	},
   { "ITEM_TYPE_CORPSE_NPC",		23	},
   { "ITEM_TYPE_CORPSE_PC",		24	},
   { "ITEM_TYPE_FOUNTAIN",		25	},
   { "ITEM_TYPE_PILL",		26	},
   { "ITEM_TYPE_AMMO",		30	},
   { "ITEM_FLAG_GLOW",		1	},
   { "ITEM_FLAG_HUM",		2	},
   { "ITEM_FLAG_DARK",		4	},
   { "ITEM_FLAG_LOCK",		8	},
   { "ITEM_FLAG_EVIL",		16	},
   { "ITEM_FLAG_INVIS",		32	},
   { "ITEM_FLAG_MAGIC",		64	},
   { "ITEM_FLAG_NODROP",		128	},
   { "ITEM_FLAG_BLESS",		256	},
   { "ITEM_FLAG_ANTI_GOOD",		512	},
   { "ITEM_FLAG_ANTI_EVIL",		1024	},
   { "ITEM_FLAG_ANTI_NEUTRAL",		2048	},
   { "ITEM_FLAG_NOREMOVE",		4096	},
   { "ITEM_FLAG_INVENTORY",		8192	},
   { "ITEM_FLAG_LEVEL",		16384	},
   { "ITEM_FLAG_NOT_VALID",		32768	},
   { "ITEM_FLAG_AUTO_ENGRAVE",		65536	},
   { "ITEM_PROTOTYPE",		262144	},
   { "ITEM_FLAG_QUEST",		524288	},
   { "ITEM_WEAR_TAKE",		1	},
   { "ITEM_WEAR_FINGER",		2	},
   { "ITEM_WEAR_NECK",		4	},
   { "ITEM_WEAR_BODY",		8	},
   { "ITEM_WEAR_HEAD",		16	},
   { "ITEM_WEAR_LEGS",		32	},
   { "ITEM_WEAR_FEET",		64	},
   { "ITEM_WEAR_HANDS",		128	},
   { "ITEM_WEAR_ARMS",		256	},
   { "ITEM_WEAR_SHIELD",		512	},
   { "ITEM_WEAR_ABOUT",		1024	},
   { "ITEM_WEAR_WAIST",		2048	},
   { "ITEM_WEAR_WRIST",		4096	},
   { "ITEM_WEAR_WIELD",		8192	},
   { "ITEM_WEAR_HOLD",		16384	},
   { "APPLY_NONE",		0	},
   { "APPLY_STR",		1	},
   { "APPLY_DEX",		2	},
   { "APPLY_INT",		3	},
   { "APPLY_WIS",		4	},
   { "APPLY_CON",		5	},
   { "APPLY_SEX",		6	},
   { "APPLY_CLASS",		7	},
   { "APPLY_LEVEL",		8	},
   { "APPLY_AGE",		9	},
   { "APPLY_HEIGHT",		10	},
   { "APPLY_WEIGHT",		11	},
   { "APPLY_MANA",		12	},
   { "APPLY_HIT",		13	},
   { "APPLY_MOVE",		14	},
   { "APPLY_GOLD",		15	},
   { "APPLY_EXP",		16	},
   { "APPLY_AC",		17	},
   { "APPLY_HITROLL",		18	},
   { "APPLY_DAMROLL",		19	},
   { "APPLY_SAVING_PARA",		20	},
   { "APPLY_SAVING_ROD",		21	},
   { "APPLY_SAVING_PETRI",		22	},
   { "APPLY_SAVING_BREATH",		23	},
   { "APPLY_SAVING_SPELL",		24	},
   { "CONT_CLOSEABLE",		1	},
   { "CONT_PICKPROOF",		2	},
   { "CONT_CLOSED",		4	},
   { "CONT_LOCKED",		8	},
   { "ROOM_DARK",		1	},
   { "ROOM_SMOKE",		2	},
   { "ROOM_NO_MOB",		4	},
   { "ROOM_INDOORS",		8	},
   { "ROOM_HALLUCINATE",		16	},
   { "ROOM_NO_GOHOME",		32	},
   { "ROOM_CLAN_DONATION",		64	},
   { "ROOM_BANK",		256	},
   { "ROOM_PRIVATE",		512	},
   { "ROOM_SAFE",		1024	},
   { "ROOM_SOLITARY",		2048	},
   { "ROOM_PET_SHOP",		4096	},
   { "ROOM_NO_RECALL",		8192	},
   { "ROOM_RIP",		16384	},
   { "ROOM_BLOCK",		32768	},
   { "ROOM_NO_SAVE",		65536	},
   { "ROOM_MORGUE",		131072	},
   { "ROOM_TEMPLE",		262144	},
   { "ROOM_ALTAR_N",		1572864	},
   { "ROOM_ALTAR_C",		524288	},
   { "ROOM_ALTAR_O",		1048576	},
   { "ROOM_ALLOW_ILL",		2097152	},
   { "ROOM_ALLOW_ELE",		4194304	},
   { "ROOM_ALLOW_ROG",		6291456	},
   { "ROOM_ALLOW_RAN",		8388608	},
   { "ROOM_ALLOW_NEC",		10485760	},
   { "ROOM_ALLOW_MON",		12582912	},
   { "ROOM_ALLOW_ASN",		14680064	},
   { "ROOM_IS_CASTLE",		16777216	},
   { "ROOM_IS_ENTRANCE",		33554432	},
   { "ROOM_PROTOTYPE",		67108864	},
   { "ROOM_NOTE_BOARD",		134217728	},
   { "ROOM_NO_CASTLE",		268435456	},
   { "ROOM_NO_RIP",		536870912	},
   { "DIR_NORTH",		0	},
   { "DIR_EAST",		1	},
   { "DIR_SOUTH",		2	},
   { "DIR_WEST",		3	},
   { "DIR_UP",		4	},
   { "DIR_DOWN",		5	},
   { "EX_ISDOOR",		1	},
   { "EX_CLOSED",		2	},
   { "EX_LOCKED",		4	},
   { "EX_HIDDEN",		8	},
   { "EX_RIP",		16	},
   { "EX_PICKPROOF",		32	},
   { "EX_BASHPROOF",		64	},
   { "EX_MAGIC_PROOF",		128	},
   { "EX_BASHED",		256	},
   { "EX_UNBARRED",		512	},
   { "EX_BACKDOOR",		1024	},
   { "EX_CLAN_BACKDOOR",		2048	},
   { "SECT_INSIDE",		0	},
   { "SECT_CITY",		1	},
   { "SECT_FIELD",		2	},
   { "SECT_FOREST",		3	},
   { "SECT_HILLS",		4	},
   { "SECT_MOUNTAIN",		5	},
   { "SECT_WATER_SWIM",		6	},
   { "SECT_WATER_NOSWIM",		7	},
   { "SECT_UNUSED",		8	},
   { "SECT_AIR",		9	},
   { "SECT_DESERT",		10	},
   { "SECT_LAVA",		11	},
   { "SECT_INN",		12	},
   { "SECT_ETHEREAL",		13	},
   { "SECT_ASTRAL",		14	},
   { "SECT_UNDER_WATER",		15	},
   { "SECT_UNDER_GROUND",		16	},
   { "SECT_DEEP_EARTH",		17	},
   { "SECT_MAX",		18	},
   { "POS_DEAD",		0	},
   { "POS_MORTAL",		1	},
   { "POS_INCAP",		2	},
   { "POS_STUNNED",		3	},
   { "POS_SLEEPING",		4	},
   { "POS_RESTING",		5	},
   { "POS_FIGHTING",		6	},
   { "POS_STANDING",		7	},
   { "PLR_IS_NPC",		1	},
   { "PLR_BOUGHT_PET",		2	},
   { "PLR_AUTOEXIT",		8	},
   { "PLR_AUTOLOOT",		16	},
   { "PLR_AUTOSAC",		32	},
   { "PLR_BLANK",		64	},
   { "PLR_BRIEF",		128	},
   { "PLR_COMBINE",		512	},
   { "PLR_PROMPT",		1024	},
   { "PLR_TELNET_GA",		2048	},
   { "PLR_HOLYLIGHT",		4096	},
   { "PLR_WIZINVIS",		8192	},
   { "PLR_WIZTIME",		16384	},
   { "PLR_SILENCE",		32768	},
   { "PLR_NO_EMOTE",		65536	},
   { "PLR_NO_TELL",		262144	},
   { "PLR_LOG",		524288	},
   { "PLR_DENY",		1048576	},
   { "PLR_FREEZE",		2097152	},
   { "PLR_THIEF",		4194304	},
   { "PLR_KILLER",		8388608	},
   { "PLR_DAMAGE",		16777216	},
   { "PLR_AUTO_SPLIT",		33554432	},
   { "PLR_QUIET",		67108864	},
   { "PLR_PAGER",		134217728	},
   { "PLR_CHAT",		268435456	},
   { "PLR_PLAN",		536870912	},
   { "CLASS_ILLUSIONIST",		0	},
   { "CLASS_ELEMENTALIST",		1	},
   { "CLASS_ROGUE",		2	},
   { "CLASS_RANGER",		3	},
   { "CLASS_NECROMANCER",		4	},
   { "CLASS_MONK",		5	},
   { "CLASS_ASSASSIN",		6	},
   { "CLASS_MONSTER",		99	},
   { "RACE_HUMAN",		0	},
   { "RACE_HALFLING",		1	},
   { "RACE_ELF",		2	},
   { "RACE_DROW",		3	},
   { "RACE_DWARF",		4	},
   { "RACE_GNOME",		5	},
   { "RACE_ORC",		6	},
   { "RACE_OGRE",		7	},
   { "RACE_AVIARAN",		8	},
   { "RACE_CENTAUR",		9	},
   { "RACE_GITH",		10	},
   { "RACE_TSARIAN",		11	},
   { "BODY_HEAD",		1	},
   { "BODY_MOUTH",		2	},
   { "BODY_EYE",		4	},
   { "BODY_TORSO",		8	},
   { "BODY_HIP",		16	},
   { "BODY_LEG",		32	},
   { "BODY_ARM",		64	},
   { "BODY_WING",		128	},
   { "BODY_TAIL",		256	},
   { "BODY_TENTICLE",		512	},
   { "BODY_HORN",		1024	},
   { "BODY_CLAW",		2048	},
   { "BODY_HAND",		4096	},
   { "BODY_FOOT",		8192	},
   { "WEAPON_SLICE",		1	},
   { "WEAPON_STAB",		2	},
   { "WEAPON_SLASH",		3	},
   { "WEAPON_WHIP",		4	},
   { "WEAPON_CLAW",		5	},
   { "WEAPON_BLAST",		6	},
   { "WEAPON_POUND",		7	},
   { "WEAPON_CRUSH",		8	},
   { "WEAPON_GREP",		9	},
   { "WEAPON_BITE",		10	},
   { "WEAPON_PIERCE",		11	},
   { "SPELL_NONE",	        -1	},
   { "SPELL_NONE",		0	},
   { "SPELL_ARMOR",		1	},
   { "SPELL_TELEPORT",		2	},
   { "SPELL_BLESS",		3	},
   { "SPELL_BLINDNESS",		4	},
   { "SPELL_BURNING_HANDS",		5	},
   { "SPELL_CALL_LIGHTNING",		6	},
   { "SPELL_CHARM_PERSON",		7	},
   { "SPELL_CHILL_TOUCH",		8	},
   { "SPELL_COLOUR_SPRAY",		10	},
   { "SPELL_CONTROL_WEATHER",		11	},
   { "SPELL_CREATE_FOOD",		12	},
   { "SPELL_CREATE_WATER",		13	},
   { "SPELL_CURE_BLINDNESS",		14	},
   { "SPELL_CURE_CRITICAL",		15	},
   { "SPELL_CURE_LIGHT",		16	},
   { "SPELL_CURSE",		17	},
   { "SPELL_DETECT_EVIL",		18	},
   { "SPELL_DETECT_INVIS",		19	},
   { "SPELL_DETECT_MAGIC",		20	},
   { "SPELL_DETECT_POISON",		21	},
   { "SPELL_DISPEL_EVIL",		22	},
   { "SPELL_EARTHQUAKE",		23	},
   { "SPELL_ENCHANT_WEAPON",		24	},
   { "SPELL_ENERGY_DRAIN",		25	},
   { "SPELL_FIREBALL",		26	},
   { "SPELL_HARM",		27	},
   { "SPELL_HEAL",		28	},
   { "SPELL_INVIS",		29	},
   { "SPELL_LIGHTNING_BOLT",		30	},
   { "SPELL_LOCATE_OBJECT",		31	},
   { "SPELL_MAGIC_MISSILE",		32	},
   { "SPELL_POISON",		33	},
   { "SPELL_PROTECTION_EVIL",		34	},
   { "SPELL_REMOVE_CURSE",		35	},
   { "SPELL_SANCTUARY",		36	},
   { "SPELL_SHOCKING_GRASP",		37	},
   { "SPELL_SLEEP",		38	},
   { "SPELL_GIANT_STRENGTH",		39	},
   { "SPELL_VENTRILOQUATE",		41	},
   { "SPELL_WORD_OF_RECALL",		42	},
   { "SPELL_CURE_POISON",		43	},
   { "SPELL_DETECT_HIDDEN",		44	},
   { "SPELL_IDENTIFY",		53	},
   { "SPELL_FLY",		56	},
   { "SPELL_CONTINUAL_LIGHT",		57	},
   { "SPELL_KNOW_ALIGNMENT",		58	},
   { "SPELL_DISPEL_MAGIC",		59	},
   { "SPELL_CURE_SERIOUS",		61	},
   { "SPELL_CAUSE_LIGHT",		62	},
   { "SPELL_CAUSE_CRITICAL",		63	},
   { "SPELL_CAUSE_SERIOUS",		64	},
   { "SPELL_FLAMESTRIKE",		65	},
   { "SPELL_STONE_SKIN",		66	},
   { "SPELL_SHIELD",		67	},
   { "SPELL_WEAKEN",		68	},
   { "SPELL_MASS_INVIS",		69	},
   { "SPELL_ACID_BLAST",		70	},
   { "SPELL_FAERIE_FIRE",		72	},
   { "SPELL_FAERIE_FOG",		73	},
   { "SPELL_PASS_DOOR",		74	},
   { "SPELL_SUMMON",		76	},
   { "SPELL_INFRAVISION",		77	},
   { "SPELL_CREATE_SPRING",		80	},
   { "SPELL_REFRESH",		81	},
   { "SPELL_CHANGE_SEX",		82	},
   { "SPELL_GATE",		83	},
   { "SPELL_ACID_BREATH",		200	},
   { "SPELL_FIRE_BREATH",		201	},
   { "SPELL_FROST_BREATH",		202	},
   { "SPELL_GAS_BREATH",		203	},
   { "SPELL_LIGHTNING_BREATH",		204	},
   { "SPELL_GENERAL_PURPOSE",		501	},
   { "SPELL_HIGH_EXPLOSIVE",		502	},
   { "SPELL_BLOCK_AREA",		601	},
   { "SPELL_WRITE_SPELL",		602	},
   { "SPELL_HOMONCULOUS",		603	},
   { "SPELL_DEMON",		604	},
   { "SPELL_SHADOW",		605	},
   { "SPELL_SHADE",		606	},
   { "SPELL_PHANTASM",		607	},
   { "SPELL_RIFT",		608	},
   { "SPELL_RIP",		609	},
   { "SPELL_TONGUES",		610	},
   { "SPELL_UNDERSTAND",		611	},
   { "SPELL_OBJECT_INVIS",		612	},
   { "SPELL_HASTE",		613	},
   { "SPELL_ENHANCED_REST",		614	},
   { "SPELL_ENHANCED_HEAL",		615	},
   { "SPELL_ENHANCED_REVIVE",		616	},
   { "SPELL_ANIMATE_DEAD",		617	},
   { "SPELL_BANISH",		618	},
   { "SPELL_ENHANCE_OBJECT",		619	},
   { "SPELL_MAGE_SHIELD",		620	},
   { "SPELL_DISPEL_UNDEAD",		621	},
   { "SPELL_DISPEL_GOOD",		622	},
   { "SPELL_REMOVE_FEAR",		623	},
   { "SPELL_FEAST",		624	},
   { "SPELL_RESTORE",		625	},
   { "SPELL_TREMOR",		626	},
   { "SPELL_ENERGY_SHIFT",		627	},
   { "SPELL_INDUCTION",		628	},
   { "SPELL_ILLUSION",		629	},
   { "SPELL_MIRROR_IMAGE",		630	},
   { "SPELL_HALLUCINATE",		631	},
   { "SPELL_STABILITY",		632	},
   { "SPELL_ETHEREAL_TRAVEL",		633	},
   { "SPELL_ASTRAL_PROJECTION",		634	},
   { "SPELL_BREATH_WATER",		635	},
   { "SPELL_MAGE_BLAST",		636	},
   { "SPELL_CONFUSION",		637	},
   { "SPELL_SANCTIFY",		638	},
   { "SPELL_SOUTHING_TOUCH",		639	},
   { "SPELL_BENEDICTION",		640	},
   { "SPELL_RIGHTEOUS_FURY",		641	},
   { "SPELL_FARHEAL",		642	},
   { "SPELL_UNHOLY_WORD",		643	},
   { "SPELL_HOLY_WORD",		644	},
   { "SPELL_INVIGORATE",		645	},
   { "SPELL_IMPROVED_INVIS",		646	},
   { "SPELL_TRUESIGHT",		647	},
   { "SPELL_RECHARGE",		648	},
   { "SPELL_TRANSPORT",		649	},
   { "SPELL_ANTI_MAGIC_SHELL",		650	},
   { "SPELL_POSSESS",		651	},
   { "SPELL_HALLUCINATORY_TERRAIN",		652	},
   { "SPELL_NIGHTMARE",		653	},
   { "SPELL_SMOKE",		654	},
   { "SPELL_SLOW",		655	},
   { "SPELL_BREW_POTION",		656	},
   { "SPELL_ELEMENTAL",		657	},
   { "SPELL_FIRESHIELD",		658	},
   { "SPELL_UNBARRING_WAYS",		659	},
   { "SPELL_VAMPIRIC_TOUCH",		660	},
   { "SPELL_PROTECTION_GOOD",		661	},
   { "LIQ_WATER",		0	},
   { "LIQ_BEER",		1	},
   { "LIQ_WINE",		2	},
   { "LIQ_ALE",		3	},
   { "LIQ_DARKALE",		4	},
   { "LIQ_WHISKY",		5	},
   { "LIQ_LEMONADE",		6	},
   { "LIQ_FIREBRT",		7	},
   { "LIQ_LOCALSPC",		8	},
   { "LIQ_SLIME",		9	},
   { "LIQ_MILK",		10	},
   { "LIQ_TEA",			11	},
   { "LIQ_COFFE",		12	},
   { "LIQ_BLOOD",		13	},
   { "LIQ_SALTWATER",		14	},
   { "LIQ_COKE",		15	},
   { "WEAR_NONE",	        -1	},
   { "WEAR_LIGHT",		0	},
   { "WEAR_FINGER_L",		1	},
   { "WEAR_FINGER_R",		2	},
   { "WEAR_NECK_A",		3	},
   { "WEAR_NECK_B",		4	},
   { "WEAR_BODY",		5	},
   { "WEAR_HEAD",		6	},
   { "WEAR_LEGS",		7	},
   { "WEAR_FEET",		8	},
   { "WEAR_HANDS",		9	},
   { "WEAR_ARMS",		10	},
   { "WEAR_SHIELD",		11	},
   { "WEAR_ABOUT",		12	},
   { "WEAR_WAIST",		13	},
   { "WEAR_WRIST_L",		14	},
   { "WEAR_WRIST_R",		15	},
   { "WEAR_WIELD",		16	},
   { "WEAR_HOLD",		17	},
   { "DOOR_OPEN",		0	},
   { "DOOR_CLOSED",		1	},
   { "DOOR_CLOSED_LOCKED",		2	},
   { "TRIG_COMMAND",		67	},
   { "TRIG_UNKNOWN",		85	},
   { "TRIG_TICK",		84	},
   { "TRIG_VOID",		88	},
   { "TRIG_HIT",		72	},
   { "TRIG_DAMAGE",		68	},
   { "TRIG_WEAR",		87	},
   { "TRIG_REMOVE",		82	},
   { "OPROG_APPLY",		65	},
   { "OPROG_ECHO",		69	},
   { "OPROG_GOD_COMMAND",		67	},
   { "OPROG_GOD_ARGUMENT",		71	},
   { "OPROG_IF_HAS_OBJECT",		72	},
   { "OPROG_IF",		73	},
   { "OPROG_JUNK",		74	},
   { "OPROG_QUEST_SET",		83	},
   { "OPROG_QUEST_ADD",		68	},
   { "OPROG_OBJECT_QUEST_IF",		81	},
   { "OPROG_PLAYER_QUEST_IF",		80	},
   { "OIF_WEAR_LOC",		119	},
   { "OIF_USER_PERCENT_DAM",		100	},
   { "OIF_USER_CLASS",		99	},
   { "OIF_USER_POSITION",		111	},
   { "OIF_USER_GOLD",		98	},
   { "OIF_USER_ROOM_NUM",		117	},
   { "OIF_RANDOM_PERCENT",		112	},
   { "OIF_USER_WHICH_GOD",		103	},
   { "OIF_USER_ALIGNMENT",		97	},
   { "OIF_USER_LEVEL",		108	},
   { "OIF_USER_RACE",		114	},
   { "OIF_USER_SEX",		115	},
   { "OIF_USER_AREA",		105	},
   { "OIF_USER_PERCENT_MOVE",		118	},
   { "OIF_USER_PERCENT_HITPT",		104	},
   { "OIF_USER_PERCENT_MANA",		109	},
   { "OIF_USER_PK_STATUS",		102	},
   { "OIF_ARMYSTATUS",		74	},
   { "OIF_MCLASS_ILLUSIONIST",		64	},
   { "OIF_MCLASS_ELEMENTALIST",		65	},
   { "OIF_MCLASS_ROGUE",		66	},
   { "OIF_MCLASS_RANGER",		67	},
   { "OIF_MCLASS_NECROMANCER",		68	},
   { "OIF_MCLASS_MONK",		69	},
   { "OIF_MCLASS_ASSASSIN",		70	},
   { "OIF_USER_SECTOR",		71	},
   { "OIF_TIME_OF_DAY",		72	},
   { "OIF_WEATHER",		73	},
   { "OAPPLY_HIT",		1	},
   { "OAPPLY_MOVE",		2	},
   { "OAPPLY_MANA",		3	},
   { "OAPPLY_ALIGNMENT",		4	},
   { "FLAG_CLASS_ILLUSIONIST",		1	},
   { "FLAG_CLASS_ELEMENTALIST",		2	},
   { "FLAG_CLASS_ROGUE",		4	},
   { "FLAG_CLASS_RANGER",		8	},
   { "FLAG_CLASS_NECROMANCER",		16	},
   { "FLAG_CLASS_MONK",		32	},
   { "FLAG_CLASS_ASSASSIN",		64	},
   { "FLAG_CLASS_MONSTER",		128	},
   { "CROOM_DARK",		1	},
   { "CROOM_NO_MOB",		4	},
   { "CROOM_INDOORS",		8	},
   { "CROOM_PRIVATE",		512	},
   { "CROOM_SAFE",		1024	},
   { "CROOM_SOLITARY",		2048	},
   { "CROOM_NO_RECALL",		8192	},
   { "CROOM_BLOCK",		32768	},
   { "CEX_CANCLOSE",		1	},
   { "CEX_HIDDEN",		8	},
   { "CEX_PICKPROOF",		32	},
   { "CACT_SENTINEL",		2	},
   { "CACT_SCAVENGER",		4	},
   { "CACT_AGGRESSIVE",		32	},
   { "CACT_WIMPY",		128	},
   { "CACT_WEAK",		2048	},
   { "CACT_SMART",		4096	},
   { "CACT_NO_ORDER",		16384	}
};
