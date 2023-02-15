/*	SCCS Id: @(#)options.c	3.4	2003/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "tcap.h"
#include <ctype.h>
#include <errno.h>

#include "filename.h"

#define WINTYPELEN 16

#ifdef DEFAULT_WC_TILED_MAP
#define PREFER_TILED true
#else
#define PREFER_TILED false
#endif

#ifdef CURSES_GRAPHICS
extern int curses_read_attrs(char *attrs);
#endif

/*
 *  NOTE:  If you add (or delete) an option, please update the short
 *  options help (option_help()), the long options help (dat/opthelp),
 *  and the current options setting display function (doset()),
 *  and also the Guidebooks.
 *
 *  The order matters.  If an option is a an initial substring of another
 *  option (e.g. time and timed_delay) the shorter one must come first.
 */

struct Bool_Opt boolopt[] = {
	{"acoustics", &flags.acoustics, true, SET_IN_GAME},
	{"autodig", &flags.autodig, false, SET_IN_GAME},
	{"autopickup", &flags.pickup, true, SET_IN_GAME},
#ifdef INSURANCE
	{"checkpoint", &flags.ins_chkpt, true, SET_IN_GAME},
#endif
	{"clicklook", &iflags.clicklook, true, SET_IN_GAME},
	{"cmdassist", &iflags.cmdassist, true, SET_IN_GAME},
	{"color", &iflags.wc_color, true, SET_IN_GAME}, /*WC*/
	{"confirm", &flags.confirm, true, SET_IN_GAME},
#ifdef CURSES_GRAPHICS
	{"classic_status", &iflags.classic_status, true, SET_IN_GAME},
#endif
#if defined(ALPHA) || defined(BETA)
	{"debug_fuzzer", &iflags.debug_fuzzer, false, SET_IN_FILE},
#endif
	{"extmenu", &iflags.extmenu, false, SET_IN_GAME},
	{"female", &flags.female, false, DISP_IN_GAME},
	{"fixinv", &flags.invlet_constant, true, SET_IN_GAME},
	{"fullscreen", &iflags.wc2_fullscreen, false, SET_IN_FILE},
	{"help", &flags.help, true, SET_IN_GAME},
	{"hilite_pet", &iflags.wc_hilite_pet, true, SET_IN_GAME},		    /*WC*/
	{"hilite_hidden_stairs", &iflags.hilite_hidden_stairs, true, SET_IN_GAME}, /*WC*/
	{"hilite_obj_piles", &iflags.hilite_obj_piles, true, SET_IN_GAME},	    /*WC*/
	{"hitpointbar", &iflags.hitpointbar, true, SET_IN_GAME},
#ifndef MAC
	{"ignintr", &flags.ignintr, false, SET_IN_GAME},
#endif
	{"invweight", &flags.invweight, false, SET_IN_GAME},
/*WAC the keep savefile option...*/
#ifdef KEEP_SAVE
	{"keep_savefile", &flags.keep_savefile, false, SET_IN_FILE},
#endif
	{"legacy", &flags.legacy, true, DISP_IN_GAME},
	{"lit_corridor", &flags.lit_corridor, true, SET_IN_GAME},
	{"lootabc", &iflags.lootabc, false, SET_IN_GAME},
#ifdef MAIL
	{"mail", &flags.biff, true, SET_IN_GAME},
#endif
	{"menucolors", &iflags.use_menu_color, true, SET_IN_GAME},
	{"menu_on_esc", &flags.menu_on_esc, true, SET_IN_GAME},
	/* for menu debugging only*/
	{"menu_tab_sep", &iflags.menu_tab_sep, false, SET_IN_GAME},
	{"menu_glyphs", &iflags.use_menu_glyphs, true, SET_IN_GAME},
	{"mouse_support", &iflags.wc_mouse_support, true, DISP_IN_GAME}, /*WC*/
#ifdef CURSES_GRAPHICS
	{"msg_is_alert", &iflags.msg_is_alert, false, SET_IN_GAME},
#endif
#ifdef NEWS
	{"news", &iflags.news, true, DISP_IN_GAME},
#endif
	{"paranoid_prompts", &iflags.paranoid_prompts, true, SET_IN_GAME},
	{"perm_invent", &flags.perm_invent, true, SET_IN_GAME},
	{"pickup_thrown", &flags.pickup_thrown, true, SET_IN_GAME},
	{"popup_dialog", &iflags.wc_popup_dialog, false, SET_IN_GAME}, /*WC*/
	{"prayconfirm", &flags.prayconfirm, true, SET_IN_GAME},
	{"preload_tiles", &iflags.wc_preload_tiles, true, DISP_IN_GAME}, /*WC*/
	{"pushweapon", &flags.pushweapon, false, SET_IN_GAME},
	{"quiver_fired", &iflags.quiver_fired, true, SET_IN_GAME},
	{"rest_on_space", &flags.rest_on_space, false, SET_IN_GAME},
	{"safe_pet", &flags.safe_dog, true, SET_IN_GAME},
	{"sanity_check", &iflags.sanity_check, false, SET_IN_GAME},
	{"showexp", &flags.showexp, false, SET_IN_GAME},
	{"showrace", &iflags.showrace, false, SET_IN_GAME},
	{"showrealtime", &iflags.showrealtime, false, SET_IN_GAME},
	{"showscore", &flags.showscore, false, SET_IN_GAME},
	/* WAC made the [ xx pts]  dmg display optional */
	{"showdmg", &flags.showdmg, false, SET_IN_GAME},
	{"showweight", &flags.showweight, true, SET_IN_GAME},
	{"silent", &flags.silent, true, SET_IN_GAME},
	{"sortpack", &flags.sortpack, true, SET_IN_GAME},
	{"sparkle", &flags.sparkle, true, SET_IN_GAME},
	{"standout", &flags.standout, false, SET_IN_GAME},
	{"statuscolors", &iflags.use_status_colors, true, SET_IN_GAME},
	{"splash_screen", &iflags.wc_splash_screen, true, DISP_IN_GAME}, /*WC*/
	{"tiled_map", &iflags.wc_tiled_map, PREFER_TILED, DISP_IN_GAME}, /*WC*/
	{"time", &flags.time, false, SET_IN_GAME},
	{"timed_delay", &flags.nap, true, SET_IN_GAME},
	{"tombstone", &flags.tombstone, true, SET_IN_GAME},
	{"toptenwin", &flags.toptenwin, true, SET_IN_GAME},
	{"use_inverse", &iflags.wc_inverse, true, SET_IN_GAME}, /*WC*/
	{"verbose", &flags.verbose, true, SET_IN_GAME},
	{"wraptext", &iflags.wc2_wraptext, false, SET_IN_GAME},
	{NULL, NULL, false, 0}};

void *nh_option_get_boolopt() {
	return (void *)boolopt;
}

/* compound options, for option_help() and external programs like Amiga
 * frontend */
static struct Comp_Opt {
	const char *name, *descr;
	int size; /* for frontends and such allocating space --
		   * usually allowed size of data in game, but
		   * occasionally maximum reasonable size for
		   * typing when game maintains information in
		   * a different format */
	int optflags;
} compopt[] = {
	{"align", "your starting alignment (lawful, neutral, or chaotic)", 8, DISP_IN_GAME},
	{"align_message", "message window alignment", 20, DISP_IN_GAME}, /*WC*/
	{"align_status", "status window alignment", 20, DISP_IN_GAME},	 /*WC*/
	{"altkeyhandler", "alternate key handler", 20, DISP_IN_GAME},
	{"catname", "the name of your (first) cat (e.g., catname:Tabby)", PL_PSIZ, DISP_IN_GAME},
	{"disclose", "the kinds of information to disclose at end of game", sizeof(flags.end_disclose) * 2, SET_IN_GAME},
	{"dogname", "the name of your (first) dog (e.g., dogname:Fang)", PL_PSIZ, DISP_IN_GAME},
	{"dungeon", "the symbols to use in drawing the dungeon map", MAXDCHARS + 1, SET_IN_FILE},
	{"effects", "the symbols to use in drawing special effects", MAXECHARS + 1, SET_IN_FILE},
	{"font_map", "the font to use in the map window", 40, DISP_IN_GAME}, /*WC*/
	{"font_menu", "the font to use in menus", 40, DISP_IN_GAME},	     /*WC*/
	{"font_message", "the font to use in the message window", 40, DISP_IN_GAME}, /*WC*/
	{"font_size_map", "the size of the map font", 20, DISP_IN_GAME},	 /*WC*/
	{"font_size_menu", "the size of the menu font", 20, DISP_IN_GAME},	 /*WC*/
	{"font_size_message", "the size of the message font", 20, DISP_IN_GAME}, /*WC*/
	{"font_size_status", "the size of the status font", 20, DISP_IN_GAME},	 /*WC*/
	{"font_size_text", "the size of the text font", 20, DISP_IN_GAME},	 /*WC*/
	{"font_status", "the font to use in status window", 40, DISP_IN_GAME},	 /*WC*/
	{"font_text", "the font to use in text windows", 40, DISP_IN_GAME},	 /*WC*/
	{"fruit", "the name of a fruit you enjoy eating", PL_FSIZ, SET_IN_GAME}, {"gender", "your starting gender (male or female)", 8, DISP_IN_GAME},
	{"ghoulname", "the name of your (first) ghoul (e.g., ghoulname:Casper)", PL_PSIZ, DISP_IN_GAME},
	{"graphics", "Graphical mode", sizeof("UTF-8 (compatible)"), SET_IN_GAME},
	{"horsename", "the name of your (first) horse (e.g., horsename:Silver)", PL_PSIZ, DISP_IN_GAME},
	{"map_mode", "map display mode under Windows", 20, DISP_IN_GAME}, /*WC*/
	{"menucolor", "set menu colors", PL_PSIZ, SET_IN_FILE},
	{"menustyle", "user interface for object selection", MENUTYPELEN, SET_IN_GAME},
	{"menu_deselect_all", "deselect all items in a menu", 4, SET_IN_FILE},
	{"menu_deselect_page", "deselect all items on this page of a menu", 4, SET_IN_FILE},
	{"menu_first_page", "jump to the first page in a menu", 4, SET_IN_FILE},
	{"menu_headings", "bold, inverse, or underline headings", 9, SET_IN_GAME},
	{"menu_invert_all", "invert all items in a menu", 4, SET_IN_FILE},
	{"menu_invert_page", "invert all items on this page of a menu", 4, SET_IN_FILE},
	{"menu_last_page", "jump to the last page in a menu", 4, SET_IN_FILE},
	{"menu_next_page", "goto the next menu page", 4, SET_IN_FILE},
	{"menu_previous_page", "goto the previous menu page", 4, SET_IN_FILE},
	{"menu_search", "search for a menu item", 4, SET_IN_FILE},
	{"menu_select_all", "select all items in a menu", 4, SET_IN_FILE},
	{"menu_select_page", "select all items on this page of a menu", 4, SET_IN_FILE},
	{"monsters", "the symbols to use for monsters", MAXMCLASSES, SET_IN_FILE},
	{"msghistory", "number of top line messages to save", 5, DISP_IN_GAME},
	{"msg_window", "the type of message window required", 1, SET_IN_GAME},
	{"name", "your character's name (e.g., name:Merlin-W)", PL_NSIZ, DISP_IN_GAME},
	{"number_pad", "use the number pad", 1, SET_IN_GAME},
	{"objects", "the symbols to use for objects", MAXOCLASSES, SET_IN_FILE},
	{"packorder", "the inventory order of the items in your pack", MAXOCLASSES, SET_IN_GAME},
#ifdef CHANGE_COLOR
	{"palette", "palette (00c/880/-fff is blue/yellow/reverse white)", 15, SET_IN_GAME},
#if defined(MAC)
	{"hicolor", "same as palette, only order is reversed", 15, SET_IN_FILE},
#endif
#endif
	{"petattr", "attributes for highlighting pets", 12, SET_IN_FILE},
	{"pettype", "your preferred initial pet type", 4, DISP_IN_GAME},
	{"pickup_burden", "maximum burden picked up before prompt", 20, SET_IN_GAME},
	{"pickup_types", "types of objects to pick up automatically", MAXOCLASSES, SET_IN_GAME},
	{"pilesize", "maximum number of items on floor to list automatically", 20, SET_IN_GAME},
	{"player_selection", "choose character via dialog or prompts", 12, DISP_IN_GAME},
	{"race", "your starting race (e.g., Human, Elf)", PL_CSIZ, DISP_IN_GAME},
	{"role", "your starting role (e.g., Barbarian, Valkyrie)", PL_CSIZ, DISP_IN_GAME},
	{"runmode", "display frequency when `running' or `travelling'", sizeof "teleport", SET_IN_GAME},
	{"scores", "the parts of the score list you wish to see", 32, SET_IN_GAME},
	{"scroll_amount", "amount to scroll map when scroll_margin is reached", 20, DISP_IN_GAME}, /*WC*/
	{"scroll_margin", "scroll map when this far from the edge", 20, DISP_IN_GAME}, /*WC*/
	{"tile_width", "width of tiles", 20, DISP_IN_GAME},   /*WC*/
	{"tile_height", "height of tiles", 20, DISP_IN_GAME}, /*WC*/
	{"tile_file", "name of tile file", 70, DISP_IN_GAME}, /*WC*/
	{"tileset", "name of predefined tileset to use", PL_PSIZ, SET_IN_GAME},
	{"traps", "the symbols to use in drawing traps", MAXTCHARS + 1, SET_IN_FILE},
	{"vary_msgcount", "show more old messages at a time", 20, DISP_IN_GAME}, /*WC*/
	{"windowcolors", "the foreground/background colors of windows", 80, DISP_IN_GAME}, /*WC*/
	{"windowtype", "windowing system to use", WINTYPELEN, DISP_IN_GAME},
	{"wolfname", "the name of your (first) wolf (e.g., wolfname:Beast)", PL_PSIZ, DISP_IN_GAME},
	{NULL, NULL, 0, 0}};

static struct Bool_Tile_Opt {
	const char *name;
	unsigned long flag;
	unsigned long initvalue;
} booltileopt[] = {
	{"transparent", TILESET_TRANSPARENT, 0},
	{"pseudo3D", TILESET_PSEUDO3D, 0},
	{NULL, 0, 0}};

static boolean need_redraw; /* for doset() */

static char def_inv_order[MAXOCLASSES] = {
	COIN_CLASS,
	AMULET_CLASS,
	WEAPON_CLASS,
	ARMOR_CLASS,
	FOOD_CLASS,
	SCROLL_CLASS,
	SPBOOK_CLASS,
	POTION_CLASS,
	RING_CLASS,
	WAND_CLASS,
	TOOL_CLASS,
	GEM_CLASS,
	ROCK_CLASS,
	BALL_CLASS,
	CHAIN_CLASS,
	0,
};

/*
 * Default menu manipulation command accelerators.  These may _not_ be:
 *
 *	+ a number - reserved for counts
 *	+ an upper or lower case US ASCII letter - used for accelerators
 *	+ ESC - reserved for escaping the menu
 *	+ NULL, CR or LF - reserved for commiting the selection(s).  NULL
 *	  is kind of odd, but the tty's xwaitforspace() will return it if
 *	  someone hits a <ret>.
 *	+ a default object class symbol - used for object class accelerators
 *
 * Standard letters (for now) are:
 *
 *		<  back 1 page
 *		>  forward 1 page
 *		^  first page
 *		|  last page
 *		:  search
 *
 *		page		all
 *		 ,    select	 .
 *		 \    deselect	 -
 *		 ~    invert	 @
 *
 * The command name list is duplicated in the compopt array.
 */
typedef struct {
	const char *name;
	char cmd;
} menu_cmd_t;

#define NUM_MENU_CMDS 11
static const menu_cmd_t default_menu_cmd_info[NUM_MENU_CMDS] = {
/* 0*/	{"menu_first_page", MENU_FIRST_PAGE},
	{"menu_last_page", MENU_LAST_PAGE},
	{"menu_next_page", MENU_NEXT_PAGE},
	{"menu_previous_page", MENU_PREVIOUS_PAGE},
	{"menu_select_all", MENU_SELECT_ALL},
/* 5*/	{"menu_deselect_all", MENU_UNSELECT_ALL},
	{"menu_invert_all", MENU_INVERT_ALL},
	{"menu_select_page", MENU_SELECT_PAGE},
	{"menu_deselect_page", MENU_UNSELECT_PAGE},
	{"menu_invert_page", MENU_INVERT_PAGE},
/*10*/	{"menu_search", MENU_SEARCH},
};

/*
 * Allow the user to map incoming characters to various menu commands.
 * The accelerator list must be a valid C string.
 */
#define MAX_MENU_MAPPED_CMDS 32			 /* some number */
char mapped_menu_cmds[MAX_MENU_MAPPED_CMDS + 1]; /* exported */
static char mapped_menu_op[MAX_MENU_MAPPED_CMDS + 1];
static short n_menu_mapped = 0;

static boolean initial, from_file;

static void doset_add_menu(winid, const char *, int);
static void nmcpy(char *, const char *, int);
static void escapes(const char *, char *);
static void rejectoption(const char *);
static void badoption(const char *);
static void badtileoption(const char *);
static char *string_for_opt(char *, boolean);
static char *string_for_tile_opt(char *, boolean);
static char *string_for_env_opt(const char *, char *, boolean);
static void bad_negation(const char *, boolean);
static int change_inv_order(char *);
static void oc_to_str(char *, char *);
static const char *get_compopt_value(const char *, char *);
static boolean special_handling(const char *, boolean, boolean);
static void duplicate_opt_detection(const char *, bool);

static void wc_set_font_name(int, char *);
static int wc_set_window_colors(char *);
static boolean is_wc_option(const char *);
static boolean wc_supported(const char *);
static boolean is_wc2_option(const char *);
static boolean wc2_supported(const char *);
static void remove_autopickup_exception(struct autopickup_exception *);
static int count_ape_maps(int *, int *);

/* check whether a user-supplied option string is a proper leading
   substring of a particular option name; option string might have
   a colon or equals sign and arbitrary value appended to it */
boolean match_optname(const char *user_string, const char *opt_name, int min_length, boolean val_allowed) {
	int len = (int)strlen(user_string);

	if (val_allowed) {
		const char *p = index(user_string, ':'),
			   *q = index(user_string, '=');

		if (!p || (q && q < p)) p = q;
		while (p && p > user_string && isspace(*(p - 1)))
			p--;
		if (p) len = (int)(p - user_string);
	}

	return (len >= min_length) && !strncmpi(opt_name, user_string, len);
}

/* most environment variables will eventually be printed in an error
 * message if they don't work, and most error message paths go through
 * BUFSZ buffers, which could be overflowed by a maliciously long
 * environment variable.  if a variable can legitimately be long, or
 * if it's put in a smaller buffer, the responsible code will have to
 * bounds-check itself.
 */
char *nh_getenv(const char *ev) {
	char *getev = getenv(ev);

	if (getev && strlen(getev) <= (BUFSZ / 2))
		return getev;
	else
		return NULL;
}

void initoptions(void) {
#ifndef MAC
	char *opts;
#endif
	int i;

	/* initialize the random number generator */
	setrandom();

	/* for detection of configfile options specified multiple times */
	iflags.opt_booldup = iflags.opt_compdup = NULL;

	for (i = 0; boolopt[i].name; i++) {
		if (boolopt[i].addr)
			*(boolopt[i].addr) = boolopt[i].initvalue;
	}
	flags.end_own = false;
	flags.end_top = 3;
	flags.end_around = 2;
	iflags.runmode = RUN_LEAP;
	iflags.msg_history = 20;
	iflags.prevmsg_window = 'f';
	iflags.menu_headings = ATR_INVERSE;
	iflags.pilesize = 5;
	iflags.autopickup_exceptions[AP_GRAB] = NULL;
	iflags.autopickup_exceptions[AP_LEAVE] = NULL;

	/* Use negative indices to indicate not yet selected */
	flags.initrole = -1;
	flags.initrace = -1;
	flags.initgend = -1;
	flags.initalign = -1;

	/* Set the default monster and object class symbols.  Don't use */
	/* memcpy() --- sizeof char != sizeof uchar on some machines.	*/
	for (i = 0; i < MAXOCLASSES; i++)
		oc_syms[i] = def_oc_syms[i];
	for (i = 0; i < MAXMCLASSES; i++)
		monsyms[i] = def_monsyms[i];

	iflags.travelcc.x = iflags.travelcc.y = -1;

	/* assert(sizeof flags.inv_order == sizeof def_inv_order); */
	memcpy(flags.inv_order, def_inv_order, sizeof flags.inv_order);
	flags.pickup_types[0] = '\0';
	flags.pickup_burden = MOD_ENCUMBER;

	for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++)
		flags.end_disclose[i] = DISCLOSE_PROMPT_DEFAULT_NO;

	switch_graphics(UTF8COMPAT_GRAPHICS);	 // set default characters
	assign_colors(NULL, 0, MAXPCHARS, 0);

	flags.menu_style = MENU_FULL;

	/* since this is done before init_objects(), do partial init here */
	objects[SLIME_MOLD].oc_name_idx = SLIME_MOLD;
	nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);
	opts = getenv(NETHACK_ENV_OPTIONS);
	if (!opts) opts = getenv("NETHACKOPTIONS");
	if (!opts) opts = getenv("HACKOPTIONS");
	if (opts) {
		if (*opts == '/' || *opts == '\\' || *opts == '@') {
			if (*opts == '@') opts++; /* @filename */
			/* looks like a filename */
			if (strlen(opts) < BUFSZ / 2)
				read_config_file(opts);
		} else {
			read_config_file(NULL);
			/* let the total length of options be long;
			 * parseoptions() will check each individually
			 */
			parseoptions(opts, true, false);
		}
	} else {
		read_config_file(NULL);
	}
	s9s7_load_options("~/.slashem9rc.scm"); //todo

	fruitadd(pl_fruit);
	/* Remove "slime mold" from list of object names; this will	*/
	/* prevent it from being wished unless it's actually present	*/
	/* as a named (or default) fruit.  Wishing for "fruit" will	*/
	/* result in the player's preferred fruit [better than "\033"].	*/
	objects[SLIME_MOLD].oc_name = "fruit";

	if (flags.lit_corridor && iflags.use_color) {
		showsyms[S_darkroom] = showsyms[S_room];
	} else {
		showsyms[S_darkroom] = showsyms[S_stone];
	}

	return;
}

static void nmcpy(char *dest, const char *src, int maxlen) {
	int count;

	for (count = 1; count < maxlen; count++) {
		if (*src == ',' || *src == '\0') break; /*exit on \0 terminator*/
		*dest++ = *src++;
	}
	*dest = 0;
}

/*
 * escapes(): escape expansion for showsyms.  C-style escapes understood
 * include \n, \b, \t, \r, \xnnn (hex), \onnn (octal), \nnn (decimal).
 * (Note: unlike in C, leading digit 0 is not used to indicate octal;
 * the letter o (either upper or lower case) is used for that.
 * The ^-prefix for control characters is also understood, and \[mM]
 * has the effect of 'meta'-ing the value which follows (so that the
 * alternate character set will be enabled).
 *
 * X     normal key X
 * ^X    control-X
 * \mX   meta-X
 *
 * For 3.4.3 and earlier, input ending with "\M", backslash, or caret
 * prior to terminating '\0' would pull that '\0' into the output and then
 * keep processing past it, potentially overflowing the output buffer.
 * Now, trailing \ or ^ will act like \\ or \^ and add '\\' or '^' to the
 * output and stop there; trailing \M will fall through to \<other> and
 * yield 'M', then stop.  Any \X or \O followed by something other than
 * an appropriate digit will also fall through to \<other> and yield 'X'
 * or 'O', plus stop if the non-digit is end-of-string.
  */

// cp might be tp; updating in place
// tp is never longer than cp
static void escapes(const char *cp, char *tp) {
	static const char oct[] = "01234567", dec[] = "0123456789",
			  hex[] = "00112233445566778899aAbBcCdDeEfF";
	const char *dp;
	int cval, meta, dcount;

	while (*cp) {
		/* \M has to be followed by something to do meta conversion,
		   otherwise it will just be \M which ultimately yields 'M' */
		meta = (*cp == '\\' && (cp[1] == 'm' || cp[1] == 'M') && cp[2]);
		if (meta)
			cp += 2;
		cval = dcount = 0; /* for decimal, octal, hexadecimal cases */
		if ((*cp != '\\' && *cp != '^') || !cp[1]) {
			// simple character, or nothing left for \ or ^ to escape
			cval = *cp++;
		} else if (*cp == '^') { // expand control-character syntax
			cval = (*++cp & 0x1f);
			++cp;

			// remaining cases are all for backslash; we know cp[1] is not \0
		} else if (index(dec, cp[1])) {
			++cp; // move past backslash to first digit
			do {
				cval = (cval * 10) + (*cp - '0');
			} while (*++cp && index(dec, *cp) && ++dcount < 3);
		} else if ((cp[1] == 'o' || cp[1] == 'O') && cp[2]
				&& index(oct, cp[2])) {
			cp += 2; // move past backslash and 'O'
			do {
				cval = (cval * 8) + (*cp - '0');
			} while (*++cp && index(oct, *cp) && ++dcount < 3);
		} else if ((cp[1] == 'x' || cp[1] == 'X') && cp[2]
				&& (dp = index(hex, cp[2])) != 0) {
			cp += 2; // move past backslash and 'X'
			do {
				cval = (cval * 16) + ((int) (dp - hex) / 2);
			} while (*++cp && (dp = index(hex, *cp)) != 0 && ++dcount < 2);
		} else { // C-style character escapes
			switch (*++cp) {
				case '\\':
					cval = '\\';
					break;
				case 'n':
					cval = '\n';
					break;
				case 't':
					cval = '\t';
					break;
				case 'b':
					cval = '\b';
					break;
				case 'r':
					cval = '\r';
					break;
				default:
					cval = *cp;
			}
			++cp;
		}

		if (meta)
			cval |= 0x80;
		*tp++ = (char)cval;
	}

	*tp = '\0';
}

static void rejectoption(const char *optname) {
	pline("%s can be set only from %s or %s.", optname, NETHACK_ENV_OPTIONS, configfile);
}

static void badoption(const char *opts) {
	if (!initial) {
		if (!strncmp(opts, "h", 1) || !strncmp(opts, "?", 1))
			option_help();
		else
			pline("Bad syntax: %s.  Enter \"?g\" for help.", opts);
		return;
	}
#ifdef MAC
	else
		return;
#endif

	if (from_file)
		raw_printf("Bad syntax in OPTIONS in %s: %s.  (Press enter to continue.)", configfile, opts);
	else
		raw_printf("Bad syntax in %s: %s.  (Press enter to continue.)", NETHACK_ENV_OPTIONS, opts);

	wait_synch();
}

static void badauthoption(const char *opts) {
	raw_printf("Bad syntax in AUTHENTICATION in %s: %s. (Press enter to continue.)", configfile, opts);
	wait_synch();
}

static void badtileoption(const char *opts) {
	raw_printf("Bad syntax in TILESET in %s: %s.", configfile, opts);
	wait_synch();
}

static char *string_for_opt(char *opts, boolean val_optional) {
	char *colon, *equals;

	colon = index(opts, ':');
	equals = index(opts, '=');
	if (!colon || (equals && equals < colon)) colon = equals;

	if (!colon || !*++colon) {
		if (!val_optional) badoption(opts);
		return NULL;
	}
	return colon;
}

static char *string_for_auth_opt(char *opts, boolean val_optional) {
	char *colon = string_for_opt(opts, true);
	if (!colon && !val_optional) badauthoption(opts);
	return colon;
}

static char *string_for_tile_opt(char *opts, boolean val_optional) {
	char *colon = string_for_opt(opts, true);
	if (!colon && !val_optional) badtileoption(opts);
	return colon;
}

static char *string_for_env_opt(const char *optname, char *opts, boolean val_optional) {
	if (!initial) {
		rejectoption(optname);
		return NULL;
	}
	return string_for_opt(opts, val_optional);
}

static void bad_negation(const char *optname, boolean with_parameter) {
	pline("The %s option may not %sbe negated.",
	      optname,
	      with_parameter ? "both have a value and " : "");
}

/*
 * Change the inventory order, using the given string as the new order.
 * Missing characters in the new order are filled in at the end from
 * the current inv_order, except for gold, which is forced to be first
 * if not explicitly present.
 *
 * This routine returns 1 unless there is a duplicate or bad char in
 * the string.
 */
static int change_inv_order(char *op) {
	int oc_sym, num;
	char *sp, buf[BUFSZ];

	num = 0;

	for (sp = op; *sp; sp++) {
		oc_sym = def_char_to_objclass(*sp);
		/* reject bad or duplicate entries */
		if (oc_sym == MAXOCLASSES ||
		    oc_sym == RANDOM_CLASS || oc_sym == ILLOBJ_CLASS ||
		    !index(flags.inv_order, oc_sym) || index(sp + 1, *sp))
			return 0;
		/* retain good ones */
		buf[num++] = (char)oc_sym;
	}
	buf[num] = '\0';

	/* fill in any omitted classes, using previous ordering */
	for (sp = flags.inv_order; *sp; sp++)
		if (!index(buf, *sp)) {
			buf[num++] = *sp;
			buf[num] = '\0'; /* explicitly terminate for next index() */
		}

	strcpy(flags.inv_order, buf);
	return 1;
}

struct name_value {
	char *name;
	int value;
};

const struct name_value status_colornames[] = {
	{"black", CLR_BLACK},
	{"red", CLR_RED},
	{"green", CLR_GREEN},
	{"brown", CLR_BROWN},
	{"blue", CLR_BLUE},
	{"magenta", CLR_MAGENTA},
	{"cyan", CLR_CYAN},
	{"gray", CLR_GRAY},
	{"orange", CLR_ORANGE},
	{"lightgreen", CLR_BRIGHT_GREEN},
	{"yellow", CLR_YELLOW},
	{"lightblue", CLR_BRIGHT_BLUE},
	{"lightmagenta", CLR_BRIGHT_MAGENTA},
	{"lightcyan", CLR_BRIGHT_CYAN},
	{"white", CLR_WHITE},
	{NULL, -1}};

const struct name_value status_attrnames[] = {
	{"none", ATR_NONE},
	{"bold", ATR_BOLD},
	{"blink", ATR_BLINK},
	{"italic", ATR_ITALIC},
	{"inverse", ATR_INVERSE},
	{"underline", ATR_UNDERLINE},
	{NULL, -1}};

int value_of_name(const char *name, const struct name_value *name_values) {
	while (name_values->name && !strstri(name_values->name, name))
		++name_values;
	return name_values->value;
}

nhstyle parse_style_option(char *start) {
	nhstyle ret = {NO_COLOR, NO_COLOR};
	char last;
	char *end;
	int attr;

	for (end = start; *end != '&' && *end != '\0'; ++end)
		;
	last = *end;
	*end = '\0';
	ret.fg = value_of_name(start, status_colornames);

	while (last == '&') {
		for (start = ++end; *end != '&' && *end != '\0'; ++end)
			;
		last = *end;
		*end = '\0';
		attr = value_of_name(start, status_attrnames);
		if (attr >= 0) ret.attr |= attr;
	}

	return ret;
}

struct percent_color_option *hp_colors = NULL;
struct percent_color_option *pw_colors = NULL;
const struct text_color_option *text_colors = NULL;

struct percent_color_option *add_percent_option(struct percent_color_option *new_option, struct percent_color_option *list_head) {
	if (list_head == NULL)
		return new_option;
	if (new_option->percentage <= list_head->percentage) {
		new_option->next = list_head;
		return new_option;
	}
	list_head->next = add_percent_option(new_option, list_head->next);
	return list_head;
}

boolean parse_status_color_option(char *start) {
	char *middle;

	while (*start && isspace(*start))
		start++;
	for (middle = start; *middle != ':' && *middle != '=' && *middle != '\0'; ++middle)
		;
	*middle++ = '\0';
	if (middle - start > 2 && start[2] == '%') {
		struct percent_color_option *percent_color_option =
			alloc(sizeof(*percent_color_option));
		percent_color_option->next = NULL;
		percent_color_option->percentage = atoi(start + 3);
		percent_color_option->style = parse_style_option(middle);
		start[2] = '\0';
		if (percent_color_option->style.fg >= 0 && percent_color_option->style.attr >= 0) {
			if (!strcmpi(start, "hp")) {
				hp_colors = add_percent_option(percent_color_option, hp_colors);
				return true;
			}
			if (!strcmpi(start, "pw")) {
				pw_colors = add_percent_option(percent_color_option, pw_colors);
				return true;
			}
		}
		free(percent_color_option);
		return false;
	} else {
		int length = strlen(start) + 1;
		struct text_color_option *text_color_option =
			alloc(sizeof(*text_color_option));
		text_color_option->next = NULL;
		text_color_option->text = alloc(length);
		memcpy((char *)text_color_option->text, start, length);
		text_color_option->style = parse_style_option(middle);
		if (text_color_option->style.fg >= 0 && text_color_option->style.attr >= 0) {
			text_color_option->next = text_colors;
			text_colors = text_color_option;
			return true;
		}
		free(text_color_option->text);
		free(text_color_option);
		return false;
	}
}

boolean parse_status_color_options(char *start) {
	char last = ',';
	char *end = start - 1;
	boolean ok = true;
	while (last == ',') {
		for (start = ++end; *end != ',' && *end != '\0'; ++end)
			;
		last = *end;
		*end = '\0';
		ok = parse_status_color_option(start) && ok;
	}
	return ok;
}

void set_duplicate_opt_detection(int on_or_off) {
	int k, *optptr;
	if (on_or_off != 0) {
		/*-- ON --*/
		if (iflags.opt_booldup)
			impossible("iflags.opt_booldup already on (memory leak)");
		iflags.opt_booldup = alloc(SIZE(boolopt) * sizeof(int));
		optptr = iflags.opt_booldup;
		for (k = 0; k < SIZE(boolopt); ++k)
			*optptr++ = 0;

		if (iflags.opt_compdup)
			impossible("iflags.opt_compdup already on (memory leak)");
		iflags.opt_compdup = alloc(SIZE(compopt) * sizeof(int));
		optptr = iflags.opt_compdup;
		for (k = 0; k < SIZE(compopt); ++k)
			*optptr++ = 0;
	} else {
		/*-- OFF --*/
		if (iflags.opt_booldup) free(iflags.opt_booldup);
		iflags.opt_booldup = NULL;
		if (iflags.opt_compdup) free(iflags.opt_compdup);
		iflags.opt_compdup = NULL;
	}
}

static void duplicate_opt_detection(const char *opts, bool is_compound_option) {
	int i, *optptr;
#if defined(MAC)
	/* the Mac has trouble dealing with the output of messages while
	 * processing the config file.  That should get fixed one day.
	 * For now just return.
	 */
	return;
#endif
	if ((!is_compound_option) && iflags.opt_booldup && initial && from_file) {
		for (i = 0; boolopt[i].name; i++) {
			if (match_optname(opts, boolopt[i].name, 3, false)) {
				optptr = iflags.opt_booldup + i;
				if (*optptr == 1) {
					raw_printf(
						"\nWarning - Boolean option specified multiple times: %s.\n",
						opts);
					wait_synch();
				}
				*optptr += 1;
				break; /* don't match multiple options */
			}
		}
	} else if ((is_compound_option) && iflags.opt_compdup && initial && from_file) {
		for (i = 0; compopt[i].name; i++) {
			if (match_optname(opts, compopt[i].name, strlen(compopt[i].name), true)) {
				optptr = iflags.opt_compdup + i;
				if (*optptr == 1) {
					raw_printf(
						"\nWarning - compound option specified multiple times: %s.\n",
						compopt[i].name);
					wait_synch();
				}
				*optptr += 1;
				break; /* don't match multiple options */
			}
		}
	}
}

extern struct menucoloring *menu_colorings;

static const struct {
	const char *name;
	const int color;
} colornames[] = {
	{"black", CLR_BLACK},
	{"red", CLR_RED},
	{"green", CLR_GREEN},
	{"brown", CLR_BROWN},
	{"blue", CLR_BLUE},
	{"magenta", CLR_MAGENTA},
	{"cyan", CLR_CYAN},
	{"gray", CLR_GRAY},
	{"orange", CLR_ORANGE},
	{"lightgreen", CLR_BRIGHT_GREEN},
	{"yellow", CLR_YELLOW},
	{"lightblue", CLR_BRIGHT_BLUE},
	{"lightmagenta", CLR_BRIGHT_MAGENTA},
	{"lightcyan", CLR_BRIGHT_CYAN},
	{"white", CLR_WHITE}};

static const struct {
	const char *name;
	const int attr;
} attrnames[] = {
	{"none", ATR_NONE},
	{"bold", ATR_BOLD},
	{"blink", ATR_BLINK},
	{"italic", ATR_ITALIC},
	{"inverse", ATR_INVERSE},
	{"underline", ATR_UNDERLINE},
};

bool read_style(const char *str, nhstyle *style) {
	int i;
	int c = NO_COLOR, a = ATR_NONE;

	while (*str && isspace(*str))
		str++;

	for (i = 0; i < SIZE(colornames); i++)
		if (strstri(str, colornames[i].name) == str) {
			c = colornames[i].color;
			break;
		}
	if ((i == SIZE(colornames)) && (*str >= '0' && *str <= '9'))
		c = atoi(str);

	if (c > 15) return false;

	str = strchr(str, '&');
	if (str) {
		str++;
		while (*str && isspace(*str))
			str++;
		for (i = 0; i < SIZE(attrnames); i++)
			if (strstri(str, attrnames[i].name) == str) {
				a = attrnames[i].attr;
				break;
			}
		if ((i == SIZE(attrnames)) && (*str >= '0' && *str <= '9'))
			a = atoi(str);
	}

	style->fg = c;
	style->attr = a;
	return true;
}


bool create_menu_coloring(const char *str, nhstyle style) {
	char errbuf[256];
	struct menucoloring *tmp = alloc(sizeof(struct menucoloring));

	int errnum = tre_regcomp(&tmp->match, str, REG_EXTENDED);
	if (errnum != 0) {
		tre_regerror(errnum, &tmp->match, errbuf, sizeof(errbuf));
		raw_printf("\nMenucolors: error in regex ‘%s’: ‘%s’\n", str, errbuf);
		wait_synch();
		free(tmp);
		return false;
	}
	tmp->next = menu_colorings;
	tmp->color = style.fg;
	tmp->attr = style.attr;
	menu_colorings = tmp;
	return true;
}

/* parse '"regex_string"=color' and add it to menucoloring */
bool add_menu_coloring(char *str) {
	char *cs = strchr(str, '=');
	nhstyle style;

	if (!cs || !str) return false;

	if (!read_style(cs+1, &style)) return false;
	*cs = '\0';


	if ((*str == '"') || (*str == '\'')) {
		cs--;
		while (isspace(*cs))
			cs--;
		if (*cs == *str) {
			*cs = '\0';
			str++;
		}
	}

	return create_menu_coloring(str, style);
}

/** Split up a string that matches name:value or 'name':value and
 * return name and value separately. */
static bool parse_extended_option(const char *str, char *option_name, char *option_value) {
	int i;
	char *tmps, *cs;
	char buf[BUFSZ];

	if (!str) return false;

	strncpy(buf, str, BUFSZ - 1);

	/* remove comment*/
	cs = strrchr(buf, '#');
	if (cs) *cs = '\0';

	/* trim whitespace at end of string */
	i = strlen(buf) - 1;
	while (i >= 0 && isspace(buf[i])) {
		buf[i--] = '\0';
	}

	/* extract value */
	cs = strchr(buf, ':');
	if (!cs) return false;

	tmps = cs;
	tmps++;
	/* skip whitespace at start of string */
	while (*tmps && isspace(*tmps))
		tmps++;

	strncpy(option_value, tmps, BUFSZ);

	/* extract option name */
	*cs = '\0';
	tmps = buf;
	if ((*tmps == '"') || (*tmps == '\'')) {
		cs--;
		while (isspace(*cs))
			cs--;
		if (*cs == *tmps) {
			*cs = '\0';
			tmps++;
		}
	}

	strncpy(option_name, tmps, BUFSZ);

	return true;
}

/** Parse a string as Unicode codepoint and return the numerical codepoint.
 * Valid codepoints are decimal numbers or U+FFFF and 0xFFFF for hexadecimal
 * values. */
static int parse_codepoint(char *codepoint) {
	char *ptr, *endptr;
	int num = 0, base;

	/* parse codepoint */
	if (!strncmpi(codepoint, "u+", 2) ||
	    !strncmpi(codepoint, "0x", 2)) {
		/* hexadecimal */
		ptr = &codepoint[2];
		base = 16;
	} else {
		/* decimal */
		ptr = &codepoint[0];
		base = 10;
	}
	errno = 0;
	num = strtol(ptr, &endptr, base);
	if (errno != 0 || *endptr != 0 || endptr == ptr) {
		return false;
	}
	return num;
}

/** Parse '"monster name":unicode_codepoint' and change symbol in
 * monster list. */
bool parse_monster_symbol(const char *str) {
	char monster[BUFSZ];
	char codepoint[BUFSZ];
	int i, num = 0;

	if (!parse_extended_option(str, monster, codepoint)) {
		return false;
	}

	num = parse_codepoint(codepoint);
	if (num < 0) {
		return false;
	}

	/* find monster */
	for (i = 0; mons[i].mlet != 0; i++) {
		if (!strcmpi(monster, mons[i].mname)) {
			permonst_unicode_codepoint[i] = num;
			return true;
		}
	}

	return false;
}

/** Parse '"object name":unicode_codepoint' and change symbol in
 * object list. */
bool parse_object_symbol(const char *str) {
	char object[BUFSZ];
	char codepoint[BUFSZ];
	int i, num = 0;

	if (!parse_extended_option(str, object, codepoint)) {
		return false;
	}

	num = parse_codepoint(codepoint);
	if (num < 0) {
		return false;
	}

	/* find object */
	for (i = 0; objects[i].oc_name || objects[i].oc_descr; i++) {
		if ((objects[i].oc_name && objects[i].oc_descr) ||
		    (objects[i].oc_descr)) {
			/* Items with both descriptive and actual name or only
			 * descriptive name. */
			if (!strcmpi(object, objects[i].oc_descr)) {
				objects[i].glyph = num;
				return true;
			}
		} else if (objects[i].oc_name) {
			/* items with only actual name like "carrot" */
			if (!strcmpi(object, objects[i].oc_name)) {
				objects[i].glyph = num;
				return true;
			}
		}
	}
	return false;
}

void assign_boolopt(struct Bool_Opt *o, bool value, bool initial) {
	/* options that don't exist */
	if (!o->addr) {
		if (!initial && value)
			pline("The '%s' option is not available.", o->name);
		return;
	}
	/* options that must come from config file */
	if (!initial && (o->optflags == SET_IN_FILE)) {
		rejectoption(o->name);
		return;
	}

	if (iflags.debug_fuzzer && !initial) {
		// don't randomly toggle this/these
		if (o->addr == &flags.silent || o->addr == &flags.menu_on_esc)
			return;
	}

	*o->addr = value;

	duplicate_opt_detection(o->name, 0);

	/* only do processing below if setting with doset() */
	if (initial) return;

	if (o->addr == &flags.time
	 || o->addr == &flags.showexp
	 || o->addr == &flags.showscore
	 || o->addr == &flags.showweight) {
		bot_reconfig();
	} else if (o->addr == &flags.invlet_constant) {
		if (flags.invlet_constant) reassign();
	}
#ifdef LAN_MAIL
	else if ((o->addr) == &flags.biff) {
		if (flags.biff)
			lan_mail_init();
		else
			lan_mail_finish();
	}
#endif
	else if (o->addr == &flags.lit_corridor) {
		/*
		 * All corridor squares seen via night vision or
		 * candles & lamps change.  Update them by calling
		 * newsym() on them.  Don't do this if we are
		 * initializing the options --- the vision system
		 * isn't set up yet.
		 */
		if (u.uz.dlevel) {
			vision_recalc(2);			  /* shut down vision */
			vision_full_recalc = 1;			  /* delayed recalc */
			if (iflags.use_color) need_redraw = true; /* darkroom refresh */
		}
	} else if (o->addr == &iflags.use_inverse
	        || o->addr == &iflags.showrace
	        || o->addr == &iflags.hilite_pet) {
		need_redraw = true;
	} else if (o->addr == &iflags.use_color) {
		need_redraw = true;
	} else if (o->addr == &flags.perm_invent)
		update_inventory();

	return;
}

void parseoptions(char *opts, boolean tinitial, boolean tfrom_file) {
	char *op;
	unsigned num;
	boolean negated;
	int i;
	const char *fullname;

	initial = tinitial;
	from_file = tfrom_file;
	if ((op = index(opts, ',')) != 0) {
		*op++ = 0;
		parseoptions(op, initial, from_file);
	}
	if (strlen(opts) > BUFSZ / 2) {
		badoption("option too long");
		return;
	}

	// strip leading and trailing white space
	opts = stripspace(opts);

	if (!*opts) return;
	negated = false;
	while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
		if (*opts == '!')
			opts++;
		else
			opts += 2;
		negated = !negated;
	}

	/* variant spelling */

	if (match_optname(opts, "colour", 5, false))
		strcpy(opts, "color"); /* fortunately this isn't longer */

	if (!match_optname(opts, "subkeyvalue", 11, true)) /* allow multiple */
		duplicate_opt_detection(opts, 1);	   /* 1 means compound opts */

	/* special boolean options */

	if (match_optname(opts, "female", 3, false)) {
		if (!initial && flags.female == negated)
			pline("That is not anatomically possible.");
		else
			flags.initgend = flags.female = !negated;
		return;
	}

	if (match_optname(opts, "male", 4, false)) {
		if (!initial && flags.female != negated)
			pline("That is not anatomically possible.");
		else
			flags.initgend = flags.female = negated;
		return;
	}

	/* compound options */
	fullname = "pettype";
	if (match_optname(opts, fullname, 3, true)) {
		if ((op = string_for_env_opt(fullname, opts, negated)) != 0) {
			if (negated)
				bad_negation(fullname, true);
			else
				switch (*op) {
					case 'd': /* dog */
					case 'D':
						preferred_pet = 'd';
						break;
					case 'c': /* cat */
					case 'C':
					case 'f': /* feline */
					case 'F':
						preferred_pet = 'c';
						break;
					case 'n': /* no pet */
					case 'N':
						preferred_pet = 'n';
						break;
					default:
						pline("Unrecognized pet type '%s'.", op);
						break;
				}
		} else if (negated)
			preferred_pet = 'n';
		return;
	}

	fullname = "ghoulname";
	if (match_optname(opts, fullname, 3, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			nmcpy(ghoulname, op, PL_PSIZ);
		return;
	}

	fullname = "wolfname";
	if (match_optname(opts, fullname, 3, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			nmcpy(wolfname, op, PL_PSIZ);
		return;
	}

	fullname = "catname";
	if (match_optname(opts, fullname, 3, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			nmcpy(catname, op, PL_PSIZ);
		return;
	}

	fullname = "dogname";
	if (match_optname(opts, fullname, 3, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			nmcpy(dogname, op, PL_PSIZ);
		return;
	}

	fullname = "horsename";
	if (match_optname(opts, fullname, 5, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			nmcpy(horsename, op, PL_PSIZ);
		return;
	}

	/* menucolor:"regex_string"=color */
	fullname = "menucolor";
	if (match_optname(opts, fullname, 9, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			if (!add_menu_coloring(op))
				badoption(opts);
		return;
	}

	fullname = "number_pad";
	if (match_optname(opts, fullname, 10, true)) {
		boolean compat = (strlen(opts) <= 10);
		op = string_for_opt(opts, (compat || !initial));
		if (!op) {
			if (compat || negated || initial) {
				/* for backwards compatibility, "number_pad" without a
				   value is a synonym for number_pad:1 */
				iflags.num_pad = !negated;
				if (iflags.num_pad) iflags.num_pad_mode = 0;
				number_pad(iflags.num_pad);
			}
			return;
		}
		if (negated) {
			bad_negation("number_pad", true);
			return;
		}
		if (*op == '1' || *op == '2') {
			iflags.num_pad = 1;
			if (*op == '2')
				iflags.num_pad_mode = 1;
			else
				iflags.num_pad_mode = 0;
			number_pad(1);
		} else if (*op == '0') {
			iflags.num_pad = 0;
			iflags.num_pad_mode = 0;
			number_pad(0);
		} else
			badoption(opts);
		return;
	}

	fullname = "runmode";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated) {
			iflags.runmode = RUN_TPORT;
		} else if ((op = string_for_opt(opts, false)) != 0) {
			if (!strncmpi(op, "teleport", strlen(op)))
				iflags.runmode = RUN_TPORT;
			else if (!strncmpi(op, "run", strlen(op)))
				iflags.runmode = RUN_LEAP;
			else if (!strncmpi(op, "walk", strlen(op)))
				iflags.runmode = RUN_STEP;
			else if (!strncmpi(op, "crawl", strlen(op)))
				iflags.runmode = RUN_CRAWL;
			else
				badoption(opts);
		}
		return;
	}

	fullname = "graphics";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated) {
			bad_negation(fullname, false);
		} else if ((op = string_for_opt(opts, false)) != 0) {
			if (!strcmpi(op, "ascii")) {
				iflags.graphics = ASCII_GRAPHICS;
			} else if (!strcmpi(op, "utf8")) {
				iflags.graphics = UTF8_GRAPHICS;
			} else if (!strcmpi(op, "utf8compat")) {
				iflags.graphics = UTF8COMPAT_GRAPHICS;
			} else {
				badoption(opts);
			}

			switch_graphics(iflags.graphics);
			doredraw();

			if (!initial && u.uz.dlevel &&
			    Is_rogue_level(&u.uz))
				assign_rogue_graphics(true);
		}

		return;
	}

	fullname = "msghistory";
	if (match_optname(opts, fullname, 3, true)) {
		op = string_for_env_opt(fullname, opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.msg_history = negated ? 0 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}

	fullname = "msg_window";
	/* msg_window:single, combo, full or reversed */
	if (match_optname(opts, fullname, 4, true)) {
		/* allow option to be silently ignored by non-tty ports */
		int tmp;
		if (!(op = string_for_opt(opts, true))) {
			tmp = negated ? 's' : 'f';
		} else {
			if (negated) {
				bad_negation(fullname, true);
				return;
			}
			tmp = tolower(*op);
		}
		switch (tmp) {
			case 's': /* single message history cycle (default if negated) */
				iflags.prevmsg_window = 's';
				break;
			case 'c': /* combination: two singles, then full page reversed */
				iflags.prevmsg_window = 'c';
				break;
			case 'f': /* full page (default if no opts) */
				iflags.prevmsg_window = 'f';
				break;
			case 'r': /* full page (reversed) */
				iflags.prevmsg_window = 'r';
				break;
			default:
				badoption(opts);
		}
		return;
	}

	/* WINCAP
	 * setting font options  */
	fullname = "font";
	if (!strncmpi(opts, fullname, 4)) {
		int wintype = -1;
		char *fontopts = opts + 4;

		if (!strncmpi(fontopts, "map", 3) ||
		    !strncmpi(fontopts, "_map", 4))
			wintype = NHW_MAP;
		else if (!strncmpi(fontopts, "message", 7) ||
			 !strncmpi(fontopts, "_message", 8))
			wintype = NHW_MESSAGE;
		else if (!strncmpi(fontopts, "text", 4) ||
			 !strncmpi(fontopts, "_text", 5))
			wintype = NHW_TEXT;
		else if (!strncmpi(fontopts, "menu", 4) ||
			 !strncmpi(fontopts, "_menu", 5))
			wintype = NHW_MENU;
		else if (!strncmpi(fontopts, "status", 6) ||
			 !strncmpi(fontopts, "_status", 7))
			wintype = NHW_STATUS;
		else if (!strncmpi(fontopts, "_size", 5)) {
			if (!strncmpi(fontopts, "_size_map", 8))
				wintype = NHW_MAP;
			else if (!strncmpi(fontopts, "_size_message", 12))
				wintype = NHW_MESSAGE;
			else if (!strncmpi(fontopts, "_size_text", 9))
				wintype = NHW_TEXT;
			else if (!strncmpi(fontopts, "_size_menu", 9))
				wintype = NHW_MENU;
			else if (!strncmpi(fontopts, "_size_status", 11))
				wintype = NHW_STATUS;
			else {
				badoption(opts);
				return;
			}
			if (wintype > 0 && !negated &&
			    (op = string_for_opt(opts, false)) != 0) {
				switch (wintype) {
					case NHW_MAP:
						iflags.wc_fontsiz_map = atoi(op);
						break;
					case NHW_MESSAGE:
						iflags.wc_fontsiz_message = atoi(op);
						break;
					case NHW_TEXT:
						iflags.wc_fontsiz_text = atoi(op);
						break;
					case NHW_MENU:
						iflags.wc_fontsiz_menu = atoi(op);
						break;
					case NHW_STATUS:
						iflags.wc_fontsiz_status = atoi(op);
						break;
				}
			}
			return;
		} else {
			badoption(opts);
		}
		if (wintype > 0 &&
		    (op = string_for_opt(opts, false)) != 0) {
			wc_set_font_name(wintype, op);
#ifdef MAC
			set_font_name(wintype, op);
#endif
			return;
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
#ifdef CHANGE_COLOR
	if (match_optname(opts, "palette", 3, true)
#ifdef MAC
	    || match_optname(opts, "hicolor", 3, true)
#endif
	) {
		int color_number, color_incr;

#ifdef MAC
		if (match_optname(opts, "hicolor", 3, true)) {
			if (negated) {
				bad_negation("hicolor", false);
				return;
			}
			color_number = CLR_MAX + 4; /* HARDCODED inverse number */
			color_incr = -1;
		} else {
#endif
			if (negated) {
				bad_negation("palette", false);
				return;
			}
			color_number = 0;
			color_incr = 1;
#ifdef MAC
		}
#endif
		if ((op = string_for_opt(opts, false)) != NULL) {
			char *pt = op;
			int cnt, tmp, reverse;
			long rgb;

			while (*pt && color_number >= 0) {
				cnt = 3;
				rgb = 0L;
				if (*pt == '-') {
					reverse = 1;
					pt++;
				} else {
					reverse = 0;
				}
				while (cnt-- > 0) {
					if (*pt && *pt != '/') {
						rgb <<= 8;
						tmp = *(pt++);
						if (isalpha(tmp)) {
							tmp = (tmp + 9) & 0xf; /* Assumes ASCII... */
						} else {
							tmp &= 0xf; /* Digits in ASCII too... */
						}
						/* Add an extra so we fill f -> ff and 0 -> 00 */
						rgb += tmp << 4;
						rgb += tmp;
					}
				}
				if (*pt == '/') {
					pt++;
				}
				change_color(color_number, rgb, reverse);
				color_number += color_incr;
			}
		}
		if (!initial) {
			need_redraw = true;
		}
		return;
	}
#endif /* CHANGE_COLOR */

	if (match_optname(opts, "fruit", 2, true)) {
		char empty_str = '\0';
		op = string_for_opt(opts, negated);
		if (negated) {
			if (op) {
				bad_negation("fruit", true);
				return;
			}
			op = &empty_str;
			goto goodfruit;
		}
		if (!op) return;
		if (!initial) {
			struct fruit *f;

			num = 0;
			for (f = ffruit; f; f = f->nextf) {
				if (!strcmp(op, f->fname)) goto goodfruit;
				num++;
			}
			if (num >= 100) {
				pline("Doing that so many times isn't very fruitful.");
				return;
			}
		}
goodfruit:
		nmcpy(pl_fruit, op, PL_FSIZ);
		/* OBJ_NAME(objects[SLIME_MOLD]) won't work after initialization */
		if (!*pl_fruit)
			nmcpy(pl_fruit, "slime mold", PL_FSIZ);
		if (!initial)
			fruitadd(pl_fruit);
		/* If initial, then initoptions is allowed to do it instead
		 * of here (initoptions always has to do it even if there's
		 * no fruit option at all.  Also, we don't want people
		 * setting multiple fruits in their options.)
		 */
		return;
	}

	/* name:string */
	fullname = "name";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
#ifdef PROXY_GRAPHICS
			/*
			 * Can't change player name if authentication required.
			 */
			if (!getenv("HACKAUTHENTICATION"))
#endif
				nmcpy(plname, op, PL_NSIZ);
		return;
	}

	/* role:string or character:string */
	fullname = "role";
	if (match_optname(opts, fullname, 4, true) ||
	    match_optname(opts, (fullname = "character"), 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
			if ((flags.initrole = str2role(op)) == ROLE_NONE)
				badoption(opts);
			else /* Backwards compatibility */
				nmcpy(pl_character, op, PL_NSIZ);
		}
		return;
	}

	/* race:string */
	fullname = "race";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
			if ((flags.initrace = str2race(op)) == ROLE_NONE)
				badoption(opts);
			else /* Backwards compatibility */
				pl_race = *op;
		}
		return;
	}

	/* gender:string */
	fullname = "gender";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
			if ((flags.initgend = str2gend(op)) == ROLE_NONE)
				badoption(opts);
			else
				flags.female = flags.initgend;
		}
		return;
	}

	/* WINCAP
	 * align_status:[left|top|right|bottom] */
	fullname = "align_status";
	if (match_optname(opts, fullname, sizeof("align_status") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (op && !negated) {
			if (!strncmpi(op, "left", sizeof("left") - 1))
				iflags.wc_align_status = ALIGN_LEFT;
			else if (!strncmpi(op, "top", sizeof("top") - 1))
				iflags.wc_align_status = ALIGN_TOP;
			else if (!strncmpi(op, "right", sizeof("right") - 1))
				iflags.wc_align_status = ALIGN_RIGHT;
			else if (!strncmpi(op, "bottom", sizeof("bottom") - 1))
				iflags.wc_align_status = ALIGN_BOTTOM;
			else
				badoption(opts);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* WINCAP
	 * align_message:[left|top|right|bottom] */
	fullname = "align_message";
	if (match_optname(opts, fullname, sizeof("align_message") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (op && !negated) {
			if (!strncmpi(op, "left", sizeof("left") - 1))
				iflags.wc_align_message = ALIGN_LEFT;
			else if (!strncmpi(op, "top", sizeof("top") - 1))
				iflags.wc_align_message = ALIGN_TOP;
			else if (!strncmpi(op, "right", sizeof("right") - 1))
				iflags.wc_align_message = ALIGN_RIGHT;
			else if (!strncmpi(op, "bottom", sizeof("bottom") - 1))
				iflags.wc_align_message = ALIGN_BOTTOM;
			else
				badoption(opts);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* align:string */
	fullname = "align";
	if (match_optname(opts, fullname, sizeof("align") - 1, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			if ((flags.initalign = str2align(op)) == ROLE_NONE)
				badoption(opts);
		return;
	}

	/* the order to list the pack */
	fullname = "packorder";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated) {
			bad_negation(fullname, false);
			return;
		} else if (!(op = string_for_opt(opts, false)))
			return;

		if (!change_inv_order(op))
			badoption(opts);
		return;
	}

	/* maximum burden picked up before prompt (Warren Cheung) */
	fullname = "pickup_burden";
	if (match_optname(opts, fullname, 8, true)) {
		if (negated) {
			bad_negation(fullname, false);
			return;
		} else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
			switch (tolower(*op)) {
				/* Unencumbered */
				case 'u':
					flags.pickup_burden = UNENCUMBERED;
					break;
				/* Burdened (slight encumbrance) */
				case 'b':
					flags.pickup_burden = SLT_ENCUMBER;
					break;
				/* streSsed (moderate encumbrance) */
				case 's':
					flags.pickup_burden = MOD_ENCUMBER;
					break;
				/* straiNed (heavy encumbrance) */
				case 'n':
					flags.pickup_burden = HVY_ENCUMBER;
					break;
				/* OverTaxed (extreme encumbrance) */
				case 'o':
				case 't':
					flags.pickup_burden = EXT_ENCUMBER;
					break;
				/* overLoaded */
				case 'l':
					flags.pickup_burden = OVERLOADED;
					break;
				default:
					badoption(opts);
			}
		}
		return;
	}

	/* types of objects to pick up automatically */
	if (match_optname(opts, "pickup_types", 8, true)) {
		char ocl[MAXOCLASSES + 1], tbuf[MAXOCLASSES + 1],
			qbuf[QBUFSZ], abuf[BUFSZ];
		int oc_sym;
		boolean badopt = false, compat = (strlen(opts) <= 6), use_menu;

		oc_to_str(flags.pickup_types, tbuf);
		flags.pickup_types[0] = '\0'; /* all */
		op = string_for_opt(opts, (compat || !initial));
		if (!op) {
			if (compat || negated || initial) {
				/* for backwards compatibility, "pickup" without a
				   value is a synonym for autopickup of all types
				   (and during initialization, we can't prompt yet) */
				flags.pickup = !negated;
				return;
			}
			oc_to_str(flags.inv_order, ocl);
			use_menu = true;
			if (flags.menu_style == MENU_TRADITIONAL ||
			    flags.menu_style == MENU_COMBINATION) {
				use_menu = false;
				sprintf(qbuf, "New pickup_types: [%s am] (%s)",
					ocl, *tbuf ? tbuf : "all");
				getlin(qbuf, abuf);
				op = mungspaces(abuf);
				if (abuf[0] == '\0' || abuf[0] == '\033')
					op = tbuf; /* restore */
				else if (abuf[0] == 'm')
					use_menu = true;
			}
			if (use_menu) {
				choose_classes_menu("Auto-Pickup what?", 1,
						    true, ocl, tbuf);
				op = tbuf;
			}
		}
		if (negated) {
			bad_negation("pickup_types", true);
			return;
		}
		while (*op == ' ')
			op++;
		if (*op != 'a' && *op != 'A') {
			num = 0;
			while (*op) {
				oc_sym = def_char_to_objclass(*op);
				/* make sure all are valid obj symbols occuring once */
				if (oc_sym != MAXOCLASSES &&
				    !index(flags.pickup_types, oc_sym)) {
					flags.pickup_types[num] = (char)oc_sym;
					flags.pickup_types[++num] = '\0';
				} else
					badopt = true;
				op++;
			}
			if (badopt) badoption(opts);
		}
		return;
	}

	fullname = "pilesize";
	if (match_optname(opts, fullname, sizeof("pilesize")-1, true)) {
		if (negated) {
			bad_negation(fullname, false);
			return;
		} else if (!(op = string_for_opt(opts, false))) return;
		iflags.pilesize = atoi(op);
		if (iflags.pilesize < 1) iflags.pilesize = 1;
		return;
	}

	/* WINCAP
	 * player_selection: dialog | prompts */
	fullname = "player_selection";
	if (match_optname(opts, fullname, sizeof("player_selection") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (op && !negated) {
			if (!strncmpi(op, "dialog", sizeof("dialog") - 1))
				iflags.wc_player_selection = VIA_DIALOG;
			else if (!strncmpi(op, "prompt", sizeof("prompt") - 1))
				iflags.wc_player_selection = VIA_PROMPTS;
			else
				badoption(opts);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}

	/* things to disclose at end of game */
	if (match_optname(opts, "disclose", 7, true)) {
		/*
		 * The order that the end_disclore options are stored:
		 * inventory, attribs, vanquished, genocided, conduct
		 * There is an array in flags:
		 *	end_disclose[NUM_DISCLOSURE_OPT];
		 * with option settings for the each of the following:
		 * iagvc [see disclosure_options in decl.c]:
		 * Legal setting values in that array are:
		 *	DISCLOSE_PROMPT_DEFAULT_YES  ask with default answer yes
		 *	DISCLOSE_PROMPT_DEFAULT_NO   ask with default answer no
		 *	DISCLOSE_YES_WITHOUT_PROMPT  always disclose and don't ask
		 *	DISCLOSE_NO_WITHOUT_PROMPT   never disclose and don't ask
		 *
		 * Those setting values can be used in the option
		 * string as a prefix to get the desired behaviour.
		 *
		 * For backward compatibility, no prefix is required,
		 * and the presence of a i,a,g,v, or c without a prefix
		 * sets the corresponding value to DISCLOSE_YES_WITHOUT_PROMPT.
		 */
		boolean badopt = false;
		int idx, prefix_val;

		op = string_for_opt(opts, true);
		if (op && negated) {
			bad_negation("disclose", true);
			return;
		}
		/* "disclose" without a value means "all with prompting"
		   and negated means "none without prompting" */
		if (!op || !strcmpi(op, "all") || !strcmpi(op, "none")) {
			if (op && !strcmpi(op, "none")) negated = true;
			for (num = 0; num < NUM_DISCLOSURE_OPTIONS; num++)
				flags.end_disclose[num] = negated ?
								  DISCLOSE_NO_WITHOUT_PROMPT :
								  DISCLOSE_PROMPT_DEFAULT_YES;
			return;
		}

		num = 0;
		prefix_val = -1;
		while (*op && num < sizeof flags.end_disclose - 1) {
			char c, *dop;
			static char valid_settings[] = {
				DISCLOSE_PROMPT_DEFAULT_YES,
				DISCLOSE_PROMPT_DEFAULT_NO,
				DISCLOSE_YES_WITHOUT_PROMPT,
				DISCLOSE_NO_WITHOUT_PROMPT,
				'\0'};
			c = lowc(*op);
			if (c == 'k') c = 'v'; /* killed -> vanquished */
			dop = index(disclosure_options, c);
			if (dop) {
				idx = dop - disclosure_options;
				if (idx < 0 || idx > NUM_DISCLOSURE_OPTIONS - 1) {
					impossible("bad disclosure index %d %c",
						   idx, c);
					continue;
				}
				if (prefix_val != -1) {
					flags.end_disclose[idx] = prefix_val;
					prefix_val = -1;
				} else
					flags.end_disclose[idx] = DISCLOSE_YES_WITHOUT_PROMPT;
			} else if (index(valid_settings, c)) {
				prefix_val = c;
			} else if (c == ' ') {
				/* do nothing */
			} else
				badopt = true;
			op++;
		}
		if (badopt) badoption(opts);
		return;
	}

	/* scores:5t[op] 5a[round] o[wn] */
	if (match_optname(opts, "scores", 4, true)) {
		if (negated) {
			bad_negation("scores", false);
			return;
		}
		if (!(op = string_for_opt(opts, false))) return;

		while (*op) {
			int inum = 1;

			if (digit(*op)) {
				inum = atoi(op);
				while (digit(*op))
					op++;
			} else if (*op == '!') {
				negated = !negated;
				op++;
			}
			while (*op == ' ')
				op++;

			switch (*op) {
				case 't':
				case 'T':
					flags.end_top = inum;
					break;
				case 'a':
				case 'A':
					flags.end_around = inum;
					break;
				case 'o':
				case 'O':
					flags.end_own = !negated;
					break;
				default:
					badoption(opts);
					return;
			}
			while (letter(*++op) || *op == ' ')
				continue;
			if (*op == '/') op++;
		}
		return;
	}

	fullname = "statuscolor";
	if (match_optname(opts, fullname, 11, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_env_opt(fullname, opts, false)) != 0)
			if (!parse_status_color_options(op))
				badoption(opts);
		return;
	}

	fullname = "tileset";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated || (op = string_for_opt(opts, true)) == 0)
			tileset[0] = '\0';
		else {
			/*
			 * The tileset may not be defined (yet) if we're
			 * in initial mode, otherwise it must exist.
			 */
			if (!initial) {
				int len = strlen(op);
				for (i = 0; i < no_tilesets; i++)
					if (len == strlen(tilesets[i].name) &&
					    !strncmpi(tilesets[i].name, op, len))
						break;
				if (i == no_tilesets) {
					pline("Tileset %s not defined.", op);
					return;
				} else /* Use canonical case */
					strcpy(tileset, tilesets[i].name);
			} else
				nmcpy(tileset, op, PL_PSIZ);
		}
		if (!initial)
			need_redraw = true;
		return;
	}

	/* WINCAP
	 * map_mode:[tiles|ascii4x6|ascii6x8|ascii8x8|ascii16x8|ascii7x12|ascii8x12|
			ascii16x12|ascii12x16|ascii10x18|fit_to_screen] */
	fullname = "map_mode";
	if (match_optname(opts, fullname, sizeof("map_mode") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (op && !negated) {
			if (!strncmpi(op, "tiles", sizeof("tiles") - 1))
				iflags.wc_map_mode = MAP_MODE_TILES;
			else if (!strncmpi(op, "ascii4x6", sizeof("ascii4x6") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII4x6;
			else if (!strncmpi(op, "ascii6x8", sizeof("ascii6x8") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII6x8;
			else if (!strncmpi(op, "ascii8x8", sizeof("ascii8x8") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII8x8;
			else if (!strncmpi(op, "ascii16x8", sizeof("ascii16x8") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII16x8;
			else if (!strncmpi(op, "ascii7x12", sizeof("ascii7x12") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII7x12;
			else if (!strncmpi(op, "ascii8x12", sizeof("ascii8x12") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII8x12;
			else if (!strncmpi(op, "ascii16x12", sizeof("ascii16x12") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII16x12;
			else if (!strncmpi(op, "ascii12x16", sizeof("ascii12x16") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII12x16;
			else if (!strncmpi(op, "ascii10x18", sizeof("ascii10x18") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII10x18;
			else if (!strncmpi(op, "fit_to_screen", sizeof("fit_to_screen") - 1))
				iflags.wc_map_mode = MAP_MODE_ASCII_FIT_TO_SCREEN;
			else
				badoption(opts);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* WINCAP
	 * scroll_amount:nn */
	fullname = "scroll_amount";
	if (match_optname(opts, fullname, sizeof("scroll_amount") - 1, true)) {
		op = string_for_opt(opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.wc_scroll_amount = negated ? 1 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* WINCAP
	 * scroll_margin:nn */
	fullname = "scroll_margin";
	if (match_optname(opts, fullname, sizeof("scroll_margin") - 1, true)) {
		op = string_for_opt(opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.wc_scroll_margin = negated ? 5 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}

	/* WINCAP
	 * tile_width:nn */
	fullname = "tile_width";
	if (match_optname(opts, fullname, sizeof("tile_width") - 1, true)) {
		op = string_for_opt(opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.wc_tile_width = negated ? 0 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* WINCAP
	 * tile_file:name */
	fullname = "tile_file";
	if (match_optname(opts, fullname, sizeof("tile_file") - 1, true)) {
		if ((op = string_for_opt(opts, false)) != 0) {
			if (iflags.wc_tile_file) free(iflags.wc_tile_file);
			iflags.wc_tile_file = alloc(strlen(op) + 1);
			strcpy(iflags.wc_tile_file, op);
		}
		return;
	}
	/* WINCAP
	 * tile_height:nn */
	fullname = "tile_height";
	if (match_optname(opts, fullname, sizeof("tile_height") - 1, true)) {
		op = string_for_opt(opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.wc_tile_height = negated ? 0 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	/* WINCAP
	 * vary_msgcount:nn */
	fullname = "vary_msgcount";
	if (match_optname(opts, fullname, sizeof("vary_msgcount") - 1, true)) {
		op = string_for_opt(opts, negated);
		if ((negated && !op) || (!negated && op)) {
			iflags.wc_vary_msgcount = negated ? 0 : atoi(op);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}
	fullname = "windowtype";
	if (match_optname(opts, fullname, 3, true)) {
		if (negated) {
			bad_negation(fullname, false);
			return;
		} else if ((op = string_for_env_opt(fullname, opts, false)) != 0) {
			char buf[WINTYPELEN];
			nmcpy(buf, op, WINTYPELEN);
			choose_windows(buf);
		}
		return;
	}

	/* WINCAP
	 * setting window colors
	 * syntax: windowcolors=menu foregrnd/backgrnd text foregrnd/backgrnd
	 */
	fullname = "windowcolors";
	if (match_optname(opts, fullname, 7, true)) {
		if ((op = string_for_opt(opts, false)) != 0) {
			if (!wc_set_window_colors(op))
				badoption(opts);
		} else if (negated)
			bad_negation(fullname, true);
		return;
	}

	/* WINCAP2
	 * term_cols:amount */
	fullname = "term_cols";
	if (match_optname(opts, fullname, sizeof("term_cols") - 1, true)) {
		op = string_for_opt(opts, negated);
		iflags.wc2_term_cols = op ? atoi(op) : 0;
		if (negated) bad_negation(fullname, false);
		return;
	}

	/* WINCAP2
	 * term_rows:amount */
	fullname = "term_rows";
	if (match_optname(opts, fullname, sizeof("term_rows") - 1, true)) {
		op = string_for_opt(opts, negated);
		iflags.wc2_term_rows = op ? atoi(op) : 0;
		if (negated) bad_negation(fullname, false);
		return;
	}

	/* WINCAP2
	 * petattr:string */
	fullname = "petattr";
	if (match_optname(opts, fullname, sizeof("petattr") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (op && negated) {
			bad_negation(fullname, true);
		} else if (op) {
#ifdef CURSES_GRAPHICS
			iflags.wc2_petattr = curses_read_attrs(op);
#else
			iflags.wc2_petattr = ATR_INVERSE;
#endif
		} else {
			iflags.wc2_petattr = ATR_NONE;
		}

		need_redraw = true;
		return;
	}

	/* WINCAP2
	 * windowborders:n */
	fullname = "windowborders";
	if (match_optname(opts, fullname, sizeof("windowborders") - 1, true)) {
		op = string_for_opt(opts, negated);
		if (negated && op)
			bad_negation(fullname, true);
		else {
			if (negated)
				iflags.wc2_windowborders = 2; /* Off */
			else if (!op)
				iflags.wc2_windowborders = 1; /* On */
			else				      /* Value supplied */
				iflags.wc2_windowborders = atoi(op);
			if ((iflags.wc2_windowborders > 3) ||
			    (iflags.wc2_windowborders < 1)) {
				iflags.wc2_windowborders = 0;
				badoption(opts);
			}
		}
		return;
	}

	/* menustyle:traditional or combo or full or partial */
	if (match_optname(opts, "menustyle", 4, true)) {
		int tmp;
		boolean val_required = (strlen(opts) > 5 && !negated);

		if (!(op = string_for_opt(opts, !val_required))) {
			if (val_required) return; /* string_for_opt gave feedback */
			tmp = negated ? 'n' : 'f';
		} else {
			tmp = tolower(*op);
		}
		switch (tmp) {
			case 'n': /* none */
			case 't': /* traditional */
				flags.menu_style = MENU_TRADITIONAL;
				break;
			case 'c': /* combo: trad.class sel+menu */
				flags.menu_style = MENU_COMBINATION;
				break;
			case 'p': /* partial: no class menu */
				flags.menu_style = MENU_PARTIAL;
				break;
			case 'f': /* full: class menu + menu */
				flags.menu_style = MENU_FULL;
				break;
			default:
				badoption(opts);
		}
		return;
	}

	fullname = "menu_headings";
	if (match_optname(opts, fullname, 12, true)) {
		if (negated) {
			bad_negation(fullname, false);
			return;
		} else if (!(opts = string_for_env_opt(fullname, opts, false))) {
			return;
		}
		if (!strcmpi(opts, "bold"))
			iflags.menu_headings = ATR_BOLD;
		else if (!strcmpi(opts, "inverse"))
			iflags.menu_headings = ATR_INVERSE;
		else if (!strcmpi(opts, "underline"))
			iflags.menu_headings = ATR_UNDERLINE;
		else
			badoption(opts);
		return;
	}

	/* check for menu command mapping */
	for (i = 0; i < NUM_MENU_CMDS; i++) {
		fullname = default_menu_cmd_info[i].name;
		if (match_optname(opts, fullname, (int)strlen(fullname), true)) {
			if (negated)
				bad_negation(fullname, false);
			else if ((op = string_for_opt(opts, false)) != 0) {
				int j;
				char c, op_buf[BUFSZ];
				boolean isbad = false;

				escapes(op, op_buf);
				c = *op_buf;

				if (c == 0 || c == '\r' || c == '\n' || c == '\033' ||
				    c == ' ' || digit(c) || (letter(c) && c != '@'))
					isbad = true;
				else /* reject default object class symbols */
					for (j = 1; j < MAXOCLASSES; j++)
						if (c == def_oc_syms[i]) {
							isbad = true;
							break;
						}

				if (isbad)
					badoption(opts);
				else
					add_menu_cmd_alias(c, default_menu_cmd_info[i].cmd);
			}
			return;
		}
	}

	/* OK, if we still haven't recognized the option, check the boolean
	 * options list
	 */
	for (i = 0; boolopt[i].name; i++) {
		if (match_optname(opts, boolopt[i].name, 3, false)) {
			assign_boolopt(boolopt + i, !negated, initial);
			return;
		}
	}

	/* out of valid options */
	badoption(opts);
}

static void parseauthopt(char *opts) {
	char *op;
	boolean negated;
	const char *fullname;

	if (strlen(opts) > BUFSZ / 2) {
		badauthoption("option too long");
		return;
	}

	/* strip leading and trailing white space */
	while (isspace((int)*opts))
		opts++;
	op = eos(opts);
	while (--op >= opts && isspace((int)*op))
		*op = '\0';

	if (!*opts) return;
	negated = false;
	while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
		if (*opts == '!')
			opts++;
		else
			opts += 2;
		negated = !negated;
	}

	/* compound options */

	fullname = "prog";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_auth_opt(opts, false)) != 0)
			nmcpy(authentication.prog, op, BUFSZ);
		return;
	}

	fullname = "args";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_auth_opt(opts, false)) != 0)
			nmcpy(authentication.args, op, BUFSZ);
		return;
	}

	/* out of valid options */
	badauthoption(opts);
}

void parseauthentication(char *opts) {
	char *op;

	/* Initial values */
	authentication.prog[0] = '\0';
	authentication.args[0] = '\0';

	while ((op = index(opts, ',')) != 0) {
		*op++ = 0;
		parseauthopt(opts);
		opts = op;
	}
	parseauthopt(opts);

	if (!authentication.prog[0] && authentication.args[0])
		badauthoption("Arguments given but no program specified.");
}

static void parsetilesetopt(char *opts) {
	char *op;
	boolean negated;
	int i;
	const char *fullname;

	if (strlen(opts) > BUFSZ / 2) {
		badtileoption("option too long");
		return;
	}

	/* strip leading and trailing white space */
	while (isspace((int)*opts))
		opts++;
	op = eos(opts);
	while (--op >= opts && isspace((int)*op))
		*op = '\0';

	if (!*opts) return;
	negated = false;
	while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
		if (*opts == '!')
			opts++;
		else
			opts += 2;
		negated = !negated;
	}

	/* compound options */

	fullname = "name";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_tile_opt(opts, false)) != 0)
			nmcpy(tilesets[no_tilesets].name, op, PL_PSIZ);
		return;
	}

	fullname = "filename";
	if (match_optname(opts, fullname, 4, true)) {
		if (negated)
			bad_negation(fullname, false);
		else if ((op = string_for_tile_opt(opts, false)) != 0)
			nmcpy(tilesets[no_tilesets].file, op,
			      TILESET_MAX_FILENAME);
		return;
	}

	/* OK, if we still haven't recognized the option, check the boolean
	 * options list
	 */
	for (i = 0; booltileopt[i].name; i++) {
		if (match_optname(opts, booltileopt[i].name, 3, false)) {
			if (negated)
				tilesets[no_tilesets].flags &= ~booltileopt[i].flag;
			else
				tilesets[no_tilesets].flags |= booltileopt[i].flag;
			return;
		}
	}

	/* out of valid options */
	badtileoption(opts);
}

void parsetileset(char *opts) {
	char *op;
	int i;

	if (no_tilesets >= MAXNOTILESETS) {
		badtileoption("too many tilesets");
		return;
	}

	/* Initial values */
	tilesets[no_tilesets].name[0] = '\0';
	tilesets[no_tilesets].file[0] = '\0';
	tilesets[no_tilesets].flags = 0;
	for (i = 0; booltileopt[i].name; i++)
		tilesets[no_tilesets].flags |= booltileopt[i].initvalue;

	while ((op = index(opts, ',')) != 0) {
		*op++ = 0;
		parsetilesetopt(opts);
		opts = op;
	}
	parsetilesetopt(opts);

	if (tilesets[no_tilesets].name[0] == '\0' ||
	    tilesets[no_tilesets].file[0] == '\0') {
		badtileoption("Incomplete tileset definition.");
	} else
		no_tilesets++;
}

static const char *menutype[] = {
	"traditional", "combination", "partial", "full"};

static const char *burdentype[] = {
	"unencumbered", "burdened", "stressed",
	"strained", "overtaxed", "overloaded"};

static const char *runmodes[] = {
	"teleport", "run", "walk", "crawl"};

static const char *graphicses[] = {
	"ascii", "UTF-8", "UTF-8 (compatible)"};

/*
 * Convert the given string of object classes to a string of default object
 * symbols.
 */
static void oc_to_str(char *src, char *dest) {
	int i;

	while ((i = (int)*src++) != 0) {
		if (i < 0 || i >= MAXOCLASSES)
			impossible("oc_to_str:  illegal object class %d", i);
		else
			*dest++ = def_oc_syms[i];
	}
	*dest = '\0';
}

/*
 * Add the given mapping to the menu command map list.  Always keep the
 * maps valid C strings.
 */
void add_menu_cmd_alias(char from_ch, char to_ch) {
	if (n_menu_mapped >= MAX_MENU_MAPPED_CMDS)
		pline("out of menu map space.");
	else {
		mapped_menu_cmds[n_menu_mapped] = from_ch;
		mapped_menu_op[n_menu_mapped] = to_ch;
		n_menu_mapped++;
		mapped_menu_cmds[n_menu_mapped] = 0;
		mapped_menu_op[n_menu_mapped] = 0;
	}
}

/*
 * Map the given character to its corresponding menu command.  If it
 * doesn't match anything, just return the original.
 */
char map_menu_cmd(char ch) {
	char *found = index(mapped_menu_cmds, ch);
	if (found) {
		int idx = found - mapped_menu_cmds;
		ch = mapped_menu_op[idx];
	}
	return ch;
}

#if defined(MAC) || defined(WIN32)
#define OPTIONS_HEADING "OPTIONS"
#else
#define OPTIONS_HEADING NETHACK_ENV_OPTIONS
#endif

static char fmtstr_doset_add_menu[] = "%s%-15s [%s]   ";
static char fmtstr_doset_add_menu_tab[] = "%s\t[%s]";

/* indexoffset is the value to add to index in compopt[], or zero
 * if option cannot be changed */
static void doset_add_menu(winid win, const char *option, int indexoffset) {
	const char *value = "unknown"; /* current value */
	char buf[BUFSZ], buf2[BUFSZ];
	anything any;
	int i;

	any.a_void = 0;
	if (indexoffset == 0) {
		any.a_int = 0;
		value = get_compopt_value(option, buf2);
	} else {
		for (i = 0; compopt[i].name; i++)
			if (strcmp(option, compopt[i].name) == 0) break;

		if (compopt[i].name) {
			any.a_int = i + 1 + indexoffset;
			value = get_compopt_value(option, buf2);
		} else {
			/* We are trying to add an option not found in compopt[].
			   This is almost certainly bad, but we'll let it through anyway
			   (with a zero value, so it can't be selected). */
			any.a_int = 0;
		}
	}
	/* "    " replaces "a - " -- assumes menus follow that style */
	if (!iflags.menu_tab_sep)
		sprintf(buf, fmtstr_doset_add_menu, any.a_int ? "" : "    ", option, value);
	else
		sprintf(buf, fmtstr_doset_add_menu_tab, option, value);
	add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
}

/* Changing options via menu by Per Liboriussen */
int doset(void) {
	char buf[BUFSZ], buf2[QBUFSZ];
	int i, pass, boolcount, pick_cnt, pick_idx, opt_indx;
	bool *bool_p;
	winid tmpwin;
	anything any;
	menu_item *pick_list;
	int indexoffset, startpass, endpass;
	boolean setinitial = false, fromfile = false;
	int biggest_name = 0;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);

	any.a_void = 0;
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
		 "Booleans (selecting will toggle value):", MENU_UNSELECTED);
	any.a_int = 0;
	/* first list any other non-modifiable booleans, then modifiable ones */
	for (pass = 0; pass <= 1; pass++)
		for (i = 0; boolopt[i].name; i++)
			if ((bool_p = boolopt[i].addr) != 0 &&
			    ((boolopt[i].optflags == DISP_IN_GAME && pass == 0) ||
			     (boolopt[i].optflags == SET_IN_GAME && pass == 1))) {
				if (bool_p == &flags.female) continue; /* obsolete */
				if (bool_p == &iflags.sanity_check && !wizard) continue;
				if (bool_p == &iflags.menu_tab_sep && !wizard) continue;
				if (is_wc_option(boolopt[i].name) &&
				    !wc_supported(boolopt[i].name)) continue;
				if (is_wc2_option(boolopt[i].name) &&
				    !wc2_supported(boolopt[i].name)) continue;
				any.a_int = (pass == 0) ? 0 : i + 1;
				if (!iflags.menu_tab_sep)
					sprintf(buf, "%s%-13s [%s]",
						pass == 0 ? "    " : "",
						boolopt[i].name, *bool_p ? "true" : "false");
				else
					sprintf(buf, "%s\t[%s]",
						boolopt[i].name, *bool_p ? "true" : "false");
				add_menu(tmpwin, NO_GLYPH, &any, 0, 0,
					 ATR_NONE, buf, MENU_UNSELECTED);
			}

	boolcount = i;
	indexoffset = boolcount;
	any.a_void = 0;
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
		 "Compounds (selecting will prompt for new value):",
		 MENU_UNSELECTED);

	startpass = DISP_IN_GAME;
	endpass = SET_IN_GAME;

	/* spin through the options to find the biggest name
	   and adjust the format string accordingly if needed */
	biggest_name = 0;
	for (i = 0; compopt[i].name; i++)
		if (compopt[i].optflags >= startpass && compopt[i].optflags <= endpass &&
		    strlen(compopt[i].name) > (unsigned)biggest_name)
			biggest_name = (int)strlen(compopt[i].name);
	if (biggest_name > 30) biggest_name = 30;
	if (!iflags.menu_tab_sep)
		sprintf(fmtstr_doset_add_menu, "%%s%%-%ds [%%s]", biggest_name);

	/* deliberately put `name', `role', `race', `gender' first */
	doset_add_menu(tmpwin, "name", 0);
	doset_add_menu(tmpwin, "role", 0);
	doset_add_menu(tmpwin, "race", 0);
	doset_add_menu(tmpwin, "gender", 0);

	for (pass = startpass; pass <= endpass; pass++)
		for (i = 0; compopt[i].name; i++)
			if (compopt[i].optflags == pass) {
				if (!strcmp(compopt[i].name, "name") ||
				    !strcmp(compopt[i].name, "role") ||
				    !strcmp(compopt[i].name, "race") ||
				    !strcmp(compopt[i].name, "gender"))
					continue;
				else if (is_wc_option(compopt[i].name) &&
					 !wc_supported(compopt[i].name))
					continue;
				else if (is_wc2_option(compopt[i].name) &&
					 !wc2_supported(compopt[i].name))
					continue;
				else
					doset_add_menu(tmpwin, compopt[i].name,
						       (pass == DISP_IN_GAME) ? 0 : indexoffset);
			}
	any.a_int = -1;
	sprintf(buf, "autopickup exceptions (%d currently set)",
		count_ape_maps(NULL, NULL));
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);

	end_menu(tmpwin, "Set what options?");
	need_redraw = false;
	if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &pick_list)) > 0) {
		/*
		 * Walk down the selection list and either invert the booleans
		 * or prompt for new values. In most cases, call parseoptions()
		 * to take care of options that require special attention, like
		 * redraws.
		 */
		for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
			opt_indx = pick_list[pick_idx].item.a_int - 1;
			if (opt_indx == -2) {
				special_handling("autopickup_exception",
						 setinitial, fromfile);
			} else if (opt_indx < boolcount) {
				/* boolean option */
				sprintf(buf, "%s%s", *boolopt[opt_indx].addr ? "!" : "",
					boolopt[opt_indx].name);
				parseoptions(buf, setinitial, fromfile);
				if (wc_supported(boolopt[opt_indx].name) ||
				    wc2_supported(boolopt[opt_indx].name))
					preference_update(boolopt[opt_indx].name);
			} else {
				/* compound option */
				opt_indx -= boolcount;

				if (!special_handling(compopt[opt_indx].name,
						      setinitial, fromfile)) {
					sprintf(buf, "Set %s to what?", compopt[opt_indx].name);
					getlin(buf, buf2);
					if (buf2[0] == '\033')
						continue;
					sprintf(buf, "%s:%s", compopt[opt_indx].name, buf2);
					/* pass the buck */
					parseoptions(buf, setinitial, fromfile);
				}
				if (wc_supported(compopt[opt_indx].name) ||
				    wc2_supported(compopt[opt_indx].name))
					preference_update(compopt[opt_indx].name);
			}
		}
		free(pick_list);
		pick_list = NULL;
	}

	destroy_nhwindow(tmpwin);

	if (need_redraw) {
		if (flags.lit_corridor && iflags.use_color) {
			showsyms[S_darkroom] = showsyms[S_room];
		} else {
			showsyms[S_darkroom] = showsyms[S_stone];
		}

		doredraw();
	}
	return 0;
}

static boolean special_handling(const char *optname, boolean setinitial, boolean setfromfile) {
	winid tmpwin;
	anything any;
	int i;
	char buf[BUFSZ];

	/* Special handling of menustyle, pickup_burden, pickup_types,
	 * disclose, runmode, msg_window, menu_headings, and number_pad options.
	 * Also takes care of interactive autopickup_exception_handling changes.
	 */
	if (!strcmp("menustyle", optname)) {
		const char *style_name;
		menu_item *style_pick = NULL;
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(menutype); i++) {
			style_name = menutype[i];
			/* note: separate `style_name' variable used
			   to avoid an optimizer bug in VAX C V2.3 */
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, *style_name, 0,
				 ATR_NONE, style_name, MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Select menustyle:");
		if (select_menu(tmpwin, PICK_ONE, &style_pick) > 0) {
			flags.menu_style = style_pick->item.a_int - 1;
			free(style_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("pickup_burden", optname)) {
		const char *burden_name, *burden_letters = "ubsntl";
		menu_item *burden_pick = NULL;
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(burdentype); i++) {
			burden_name = burdentype[i];
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, burden_letters[i], 0,
				 ATR_NONE, burden_name, MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Select encumbrance level:");
		if (select_menu(tmpwin, PICK_ONE, &burden_pick) > 0) {
			flags.pickup_burden = burden_pick->item.a_int - 1;
			free(burden_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("pickup_types", optname)) {
		/* parseoptions will prompt for the list of types */
		parseoptions(strcpy(buf, "pickup_types"), setinitial, setfromfile);
	} else if (!strcmp("disclose", optname)) {
		int pick_cnt, pick_idx, opt_idx;
		menu_item *disclosure_category_pick = NULL;
		/*
		 * The order of disclose_names[]
		 * must correspond to disclosure_options in decl.h
		 */
		static const char *disclosure_names[] = {
			"inventory", "attributes", "vanquished", "genocides", "conduct"};
		int disc_cat[NUM_DISCLOSURE_OPTIONS];
		const char *disclosure_name;

		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
			disclosure_name = disclosure_names[i];
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, disclosure_options[i], 0,
				 ATR_NONE, disclosure_name, MENU_UNSELECTED);
			disc_cat[i] = 0;
		}
		end_menu(tmpwin, "Change which disclosure options categories:");
		if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &disclosure_category_pick)) > 0) {
			for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
				opt_idx = disclosure_category_pick[pick_idx].item.a_int - 1;
				disc_cat[opt_idx] = 1;
			}
			free(disclosure_category_pick);
			disclosure_category_pick = NULL;
		}
		destroy_nhwindow(tmpwin);

		for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
			if (disc_cat[i]) {
				char dbuf[BUFSZ];
				menu_item *disclosure_option_pick = NULL;
				sprintf(dbuf, "Disclosure options for %s:", disclosure_names[i]);
				tmpwin = create_nhwindow(NHW_MENU);
				start_menu(tmpwin);
				any.a_char = DISCLOSE_NO_WITHOUT_PROMPT;
				add_menu(tmpwin, NO_GLYPH, &any, 'a', 0,
					 ATR_NONE, "Never disclose and don't prompt", MENU_UNSELECTED);
				any.a_void = 0;
				any.a_char = DISCLOSE_YES_WITHOUT_PROMPT;
				add_menu(tmpwin, NO_GLYPH, &any, 'b', 0,
					 ATR_NONE, "Always disclose and don't prompt", MENU_UNSELECTED);
				any.a_void = 0;
				any.a_char = DISCLOSE_PROMPT_DEFAULT_NO;
				add_menu(tmpwin, NO_GLYPH, &any, 'c', 0,
					 ATR_NONE, "Prompt and default answer to \"No\"", MENU_UNSELECTED);
				any.a_void = 0;
				any.a_char = DISCLOSE_PROMPT_DEFAULT_YES;
				add_menu(tmpwin, NO_GLYPH, &any, 'd', 0,
					 ATR_NONE, "Prompt and default answer to \"Yes\"", MENU_UNSELECTED);
				end_menu(tmpwin, dbuf);
				if (select_menu(tmpwin, PICK_ONE, &disclosure_option_pick) > 0) {
					flags.end_disclose[i] = disclosure_option_pick->item.a_char;
					free(disclosure_option_pick);
				}
				destroy_nhwindow(tmpwin);
			}
		}
	} else if (!strcmp("runmode", optname)) {
		const char *mode_name;
		menu_item *mode_pick = NULL;
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(runmodes); i++) {
			mode_name = runmodes[i];
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, *mode_name, 0,
				 ATR_NONE, mode_name, MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Select run/travel display mode:");
		if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
			iflags.runmode = mode_pick->item.a_int - 1;
			free(mode_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("graphics", optname)) {
		const char *mode_name;
		menu_item *mode_pick = NULL;
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(graphicses); i++) {
			mode_name = graphicses[i];
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, 'a' + i, 0,
				 ATR_NONE, mode_name, MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Select run/travel display mode:");
		if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
			iflags.graphics = mode_pick->item.a_int - 1;
			switch_graphics(iflags.graphics);
			need_redraw = true;
			free(mode_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("msg_window", optname)) {
		/* by Christian W. Cooper */
		menu_item *window_pick = NULL;
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		any.a_char = 's';
		add_menu(tmpwin, NO_GLYPH, &any, 's', 0, ATR_NONE, "single", MENU_UNSELECTED);
		any.a_char = 'c';
		add_menu(tmpwin, NO_GLYPH, &any, 'c', 0, ATR_NONE, "combination", MENU_UNSELECTED);
		any.a_char = 'f';
		add_menu(tmpwin, NO_GLYPH, &any, 'f', 0, ATR_NONE, "full", MENU_UNSELECTED);
		any.a_char = 'r';
		add_menu(tmpwin, NO_GLYPH, &any, 'r', 0, ATR_NONE, "reversed", MENU_UNSELECTED);
		end_menu(tmpwin, "Select message history display type:");
		if (select_menu(tmpwin, PICK_ONE, &window_pick) > 0) {
			iflags.prevmsg_window = window_pick->item.a_char;
			free(window_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("align_message", optname) ||
		 !strcmp("align_status", optname)) {
		menu_item *window_pick = NULL;
		char abuf[BUFSZ];
		boolean msg = (*(optname + 6) == 'm');

		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		any.a_int = ALIGN_TOP;
		add_menu(tmpwin, NO_GLYPH, &any, 't', 0,
			 ATR_NONE, "top", MENU_UNSELECTED);
		any.a_int = ALIGN_BOTTOM;
		add_menu(tmpwin, NO_GLYPH, &any, 'b', 0,
			 ATR_NONE, "bottom", MENU_UNSELECTED);
		any.a_int = ALIGN_LEFT;
		add_menu(tmpwin, NO_GLYPH, &any, 'l', 0,
			 ATR_NONE, "left", MENU_UNSELECTED);
		any.a_int = ALIGN_RIGHT;
		add_menu(tmpwin, NO_GLYPH, &any, 'r', 0,
			 ATR_NONE, "right", MENU_UNSELECTED);
		sprintf(abuf, "Select %s window placement relative to the map:",
			msg ? "message" : "status");
		end_menu(tmpwin, abuf);
		if (select_menu(tmpwin, PICK_ONE, &window_pick) > 0) {
			if (msg)
				iflags.wc_align_message = window_pick->item.a_int;
			else
				iflags.wc_align_status = window_pick->item.a_int;
			free(window_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("number_pad", optname)) {
		static const char *npchoices[3] =
			{"0 (off)", "1 (on)", "2 (on, DOS compatible)"};
		const char *npletters = "abc";
		menu_item *mode_pick = NULL;

		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(npchoices); i++) {
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, npletters[i], 0,
				 ATR_NONE, npchoices[i], MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Select number_pad mode:");
		if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
			int mode = mode_pick->item.a_int - 1;
			switch (mode) {
				case 2:
					iflags.num_pad = 1;
					iflags.num_pad_mode = 1;
					break;
				case 1:
					iflags.num_pad = 1;
					iflags.num_pad_mode = 0;
					break;
				case 0:
				default:
					iflags.num_pad = 0;
					iflags.num_pad_mode = 0;
			}
			free(mode_pick);
			number_pad(iflags.num_pad);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("menu_headings", optname)) {
		static const char *mhchoices[3] = {"bold", "inverse", "underline"};
		const char *npletters = "biu";
		menu_item *mode_pick = NULL;

		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		for (i = 0; i < SIZE(mhchoices); i++) {
			any.a_int = i + 1;
			add_menu(tmpwin, NO_GLYPH, &any, npletters[i], 0,
				 ATR_NONE, mhchoices[i], MENU_UNSELECTED);
		}
		end_menu(tmpwin, "How to highlight menu headings:");
		if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
			int mode = mode_pick->item.a_int - 1;
			switch (mode) {
				case 2:
					iflags.menu_headings = ATR_UNDERLINE;
					break;
				case 0:
					iflags.menu_headings = ATR_BOLD;
					break;
				case 1:
				default:
					iflags.menu_headings = ATR_INVERSE;
			}
			free(mode_pick);
		}
		destroy_nhwindow(tmpwin);
	} else if (!strcmp("autopickup_exception", optname)) {
		int pick_cnt, pick_idx, opt_idx, pass;
		int totalapes = 0, numapes[2] = {0, 0};
		menu_item *pick_list = NULL;
		anything any;
		char apebuf[BUFSZ+1]; // so &apebuf[1] is BUFSZ long for getlin()
		struct autopickup_exception *ape;
		static struct ape_action {
			char letr; const char *desc;
		} action_titles[] = {
			{ 'a', "add new autopickup exception"},
			{ 'l', "list autopickup exceptions" },
			{ 'r', "remove existing autopickup exception" },
			{ 'x', "exit this menu" },
		};
ape_again:
		opt_idx = 0;
		totalapes = count_ape_maps(&numapes[AP_LEAVE], &numapes[AP_GRAB]);
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);
		any.a_int = 0;
		for (i = 0; i < SIZE(action_titles); i++) {
			any.a_int++;
			// omit list and remove if there aren't any yet
			if (!totalapes && (i == 1 || i == 2)) continue;
			add_menu(tmpwin, NO_GLYPH, &any, action_titles[i].letr,
					0, ATR_NONE, action_titles[i].desc,
#if 0					// this ought to work but doesn't...
					(action_titles[i].letr == 'x') ? MENU_SELECTED :
#endif
					MENU_UNSELECTED);
		}
		end_menu(tmpwin, "Do what?");
		if ((pick_cnt = select_menu(tmpwin, PICK_ONE, &pick_list)) > 0) {
			for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
				opt_idx = pick_list[pick_idx].item.a_int - 1;
			}
			free(pick_list);
			pick_list = NULL;
		}
		destroy_nhwindow(tmpwin);

		if (pick_cnt < 1 || opt_idx == 3) {
			;       // done--fall through to function exit
		} else if (opt_idx == 0) {	// add new
			getlin("What new autopickup exception pattern?", &apebuf[1]);
			mungspaces(&apebuf[1]); /* regularize whitespace */
			if (apebuf[1] == '\033') {
				;   /* fall through to function exit */
			} else {
				if (apebuf[1]) {
					apebuf[0] = '\"';
					/* guarantee room for \" prefix and \"\0 suffix;
					   -2 is good enough for apebuf[] but -3 makes
					   sure the whole thing fits within normal BUFSZ */
					apebuf[sizeof apebuf - 3] = '\0';
					strcat(apebuf, "\"");
					add_autopickup_exception(apebuf);
				}

				goto ape_again;
			}
		} else {        /* list or remove */

			tmpwin = create_nhwindow(NHW_MENU);
			start_menu(tmpwin);
			for (pass = AP_LEAVE; pass <= AP_GRAB; ++pass) {
				if (numapes[pass] == 0) continue;
				ape = iflags.autopickup_exceptions[pass];
				any.a_ape = NULL;
				add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
					 (pass == 0) ? "Never pickup" : "Always pickup",
					 MENU_UNSELECTED);
				for (i = 0; i < numapes[pass] && ape; i++) {
					any.a_ape = (opt_idx == 1) ? NULL : ape;
					sprintf(apebuf, "\"%s\"", ape->text_pattern);
					add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, apebuf, MENU_UNSELECTED);
					ape = ape->next;
				}
			}
			sprintf(apebuf, "%s autopickup exceptions",
				(opt_idx == 1) ? "List of" : "Remove which");
			end_menu(tmpwin, apebuf);
			pick_cnt = select_menu(tmpwin,
					       (opt_idx == 1) ? PICK_NONE : PICK_ANY,
					       &pick_list);
			if (pick_cnt > 0) {
				for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx)
					remove_autopickup_exception(pick_list[pick_idx].item.a_ape);
			}
			free(pick_list);
			pick_list = NULL;
			destroy_nhwindow(tmpwin);
			if (pick_cnt >= 0) goto ape_again;
		}
	} else {
		return false;
	}

	return true;
}

#define rolestring(val, array, field) ((val >= 0) ? array[val].field : \
						    (val == ROLE_RANDOM) ? randomrole : none)

/* This is ugly. We have all the option names in the compopt[] array,
   but we need to look at each option individually to get the value. */
static const char *get_compopt_value(const char *optname, char *buf) {
	char ocl[MAXOCLASSES + 1];
	static const char none[] = "(none)", randomrole[] = "random",
			  to_be_done[] = "(to be done)",
			  defopt[] = "default",
			  defbrief[] = "def";
	int i;

	buf[0] = '\0';
	if (!strcmp(optname, "align_message"))
		sprintf(buf, "%s", iflags.wc_align_message == ALIGN_TOP ? "top" : iflags.wc_align_message == ALIGN_LEFT ? "left" : iflags.wc_align_message == ALIGN_BOTTOM ? "bottom" : iflags.wc_align_message == ALIGN_RIGHT ? "right" : defopt);
	else if (!strcmp(optname, "align_status"))
		sprintf(buf, "%s", iflags.wc_align_status == ALIGN_TOP ? "top" : iflags.wc_align_status == ALIGN_LEFT ? "left" : iflags.wc_align_status == ALIGN_BOTTOM ? "bottom" : iflags.wc_align_status == ALIGN_RIGHT ? "right" : defopt);
	else if (!strcmp(optname, "align"))
		sprintf(buf, "%s", rolestring(flags.initalign, aligns, adj));
	else if (!strcmp(optname, "catname"))
		sprintf(buf, "%s", catname[0] ? catname : none);
	else if (!strcmp(optname, "disclose")) {
		for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
			char topt[2];
			if (i) strcat(buf, " ");
			topt[1] = '\0';
			topt[0] = flags.end_disclose[i];
			strcat(buf, topt);
			topt[0] = disclosure_options[i];
			strcat(buf, topt);
		}
	} else if (!strcmp(optname, "dogname"))
		sprintf(buf, "%s", dogname[0] ? dogname : none);
	else if (!strcmp(optname, "dungeon"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "effects"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "font_map"))
		sprintf(buf, "%s", iflags.wc_font_map ? iflags.wc_font_map : defopt);
	else if (!strcmp(optname, "font_message"))
		sprintf(buf, "%s", iflags.wc_font_message ? iflags.wc_font_message : defopt);
	else if (!strcmp(optname, "font_status"))
		sprintf(buf, "%s", iflags.wc_font_status ? iflags.wc_font_status : defopt);
	else if (!strcmp(optname, "font_menu"))
		sprintf(buf, "%s", iflags.wc_font_menu ? iflags.wc_font_menu : defopt);
	else if (!strcmp(optname, "font_text"))
		sprintf(buf, "%s", iflags.wc_font_text ? iflags.wc_font_text : defopt);
	else if (!strcmp(optname, "font_size_map")) {
		if (iflags.wc_fontsiz_map)
			sprintf(buf, "%d", iflags.wc_fontsiz_map);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "font_size_message")) {
		if (iflags.wc_fontsiz_message)
			sprintf(buf, "%d",
				iflags.wc_fontsiz_message);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "font_size_status")) {
		if (iflags.wc_fontsiz_status)
			sprintf(buf, "%d", iflags.wc_fontsiz_status);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "font_size_menu")) {
		if (iflags.wc_fontsiz_menu)
			sprintf(buf, "%d", iflags.wc_fontsiz_menu);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "font_size_text")) {
		if (iflags.wc_fontsiz_text)
			sprintf(buf, "%d", iflags.wc_fontsiz_text);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "fruit")) {
		sprintf(buf, "%s", pl_fruit);
	} else if (!strcmp(optname, "gender")) {
		sprintf(buf, "%s", rolestring(flags.initgend, genders, adj));
	} else if (!strcmp(optname, "ghoulname")) {
		sprintf(buf, "%s", ghoulname[0] ? ghoulname : none);
	} else if (!strcmp(optname, "graphics")) {
		sprintf(buf, "%s", graphicses[iflags.graphics]);
	} else if (!strcmp(optname, "horsename"))
		sprintf(buf, "%s", horsename[0] ? horsename : none);
	else if (!strcmp(optname, "map_mode"))
		sprintf(buf, "%s",
			iflags.wc_map_mode == MAP_MODE_TILES ? "tiles" :
							       iflags.wc_map_mode == MAP_MODE_ASCII4x6 ? "ascii4x6" :
													 iflags.wc_map_mode == MAP_MODE_ASCII6x8 ? "ascii6x8" :
																		   iflags.wc_map_mode == MAP_MODE_ASCII8x8 ? "ascii8x8" :
																							     iflags.wc_map_mode == MAP_MODE_ASCII16x8 ? "ascii16x8" :
																													iflags.wc_map_mode == MAP_MODE_ASCII7x12 ? "ascii7x12" :
																																		   iflags.wc_map_mode == MAP_MODE_ASCII8x12 ? "ascii8x12" :
																																							      iflags.wc_map_mode == MAP_MODE_ASCII16x12 ? "ascii16x12" :
																																													  iflags.wc_map_mode == MAP_MODE_ASCII12x16 ? "ascii12x16" :
																																																		      iflags.wc_map_mode == MAP_MODE_ASCII10x18 ? "ascii10x18" :
																																																								  iflags.wc_map_mode == MAP_MODE_ASCII_FIT_TO_SCREEN ?
																																																								  "fit_to_screen" :
																																																								  defopt);
	else if (!strcmp(optname, "menustyle"))
		sprintf(buf, "%s", menutype[(int)flags.menu_style]);
	else if (!strcmp(optname, "menu_deselect_all"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_deselect_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_first_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_invert_all"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_headings")) {
		sprintf(buf, "%s", (iflags.menu_headings == ATR_BOLD) ? "bold" :
		                   (iflags.menu_headings == ATR_INVERSE) ? "inverse" :
		                   (iflags.menu_headings == ATR_UNDERLINE) ? "underline" : "unknown");
	} else if (!strcmp(optname, "menu_invert_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_last_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_next_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_previous_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_search"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_select_all"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "menu_select_page"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "monsters"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "msghistory"))
		sprintf(buf, "%u", iflags.msg_history);
	else if (!strcmp(optname, "msg_window"))
		sprintf(buf, "%s", (iflags.prevmsg_window == 's') ? "single" :
				   (iflags.prevmsg_window == 'c') ? "combination" :
				   (iflags.prevmsg_window == 'f') ? "full" : "reversed");
	else if (!strcmp(optname, "name"))
		sprintf(buf, "%s", plname);
	else if (!strcmp(optname, "number_pad"))
		sprintf(buf, "%s",
			(!iflags.num_pad) ? "0=off" :
					    (iflags.num_pad_mode) ? "2=on, DOS compatible" : "1=on");
	else if (!strcmp(optname, "objects"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "packorder")) {
		oc_to_str(flags.inv_order, ocl);
		sprintf(buf, "%s", ocl);
	}
#ifdef CHANGE_COLOR
	else if (!strcmp(optname, "palette")) sprintf(buf, "%s", get_color_string());
#endif
	else if (!strcmp(optname, "pettype"))
		sprintf(buf, "%s", (preferred_pet == 'c') ? "cat" : (preferred_pet == 'd') ? "dog" : (preferred_pet == 'n') ? "none" : "random");
	else if (!strcmp(optname, "pickup_burden")) sprintf(buf, "%s", burdentype[flags.pickup_burden]);
	else if (!strcmp(optname, "pickup_types")) {
		oc_to_str(flags.pickup_types, ocl);
		sprintf(buf, "%s", ocl[0] ? ocl : "all");
	} else if (!strcmp(optname, "pilesize")) sprintf(buf, "%d", iflags.pilesize);
	else if (!strcmp(optname, "race")) sprintf(buf, "%s", rolestring(flags.initrace, races, noun));
	else if (!strcmp(optname, "role")) sprintf(buf, "%s", rolestring(flags.initrole, roles, name.m));
	else if (!strcmp(optname, "runmode")) sprintf(buf, "%s", runmodes[iflags.runmode]);
	else if (!strcmp(optname, "scores")) {
		sprintf(buf, "%d top/%d around%s", flags.end_top,
			flags.end_around, flags.end_own ? "/own" : "");
	} else if (!strcmp(optname, "scroll_amount")) {
		if (iflags.wc_scroll_amount)
			sprintf(buf, "%d", iflags.wc_scroll_amount);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "scroll_margin")) {
		if (iflags.wc_scroll_margin)
			sprintf(buf, "%d", iflags.wc_scroll_margin);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "player_selection")) {
		sprintf(buf, "%s", iflags.wc_player_selection ? "prompts" : "dialog");
	} else if (!strcmp(optname, "term_cols")) {
		if (iflags.wc2_term_cols)
			sprintf(buf, "%d", iflags.wc2_term_cols);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "term_rows")) {
		if (iflags.wc2_term_rows)
			sprintf(buf, "%d", iflags.wc2_term_rows);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "tile_file"))
		sprintf(buf, "%s", iflags.wc_tile_file ? iflags.wc_tile_file : defopt);
	else if (!strcmp(optname, "tile_height")) {
		if (iflags.wc_tile_height)
			sprintf(buf, "%d", iflags.wc_tile_height);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "tile_width")) {
		if (iflags.wc_tile_width)
			sprintf(buf, "%d", iflags.wc_tile_width);
		else
			strcpy(buf, defopt);
	} else if (!strcmp(optname, "tileset"))
		sprintf(buf, "%s", tileset[0] ? tileset : none);
	else if (!strcmp(optname, "traps"))
		sprintf(buf, "%s", to_be_done);
	else if (!strcmp(optname, "vary_msgcount")) {
		if (iflags.wc_vary_msgcount)
			sprintf(buf, "%d", iflags.wc_vary_msgcount);
		else
			strcpy(buf, defopt);
	}
	else if (!strcmp(optname, "windowborders"))
		sprintf(buf, "%s", iflags.wc2_windowborders == 1 ? "1=on" : iflags.wc2_windowborders == 2 ? "2=off" : iflags.wc2_windowborders == 3 ? "3=auto" : defopt);
	else if (!strcmp(optname, "windowtype"))
		sprintf(buf, "%s", windowprocs.name);
	else if (!strcmp(optname, "windowcolors"))
		sprintf(buf, "%s/%s %s/%s %s/%s %s/%s",
			iflags.wc_foregrnd_menu ? iflags.wc_foregrnd_menu : defbrief,
			iflags.wc_backgrnd_menu ? iflags.wc_backgrnd_menu : defbrief,
			iflags.wc_foregrnd_message ? iflags.wc_foregrnd_message : defbrief,
			iflags.wc_backgrnd_message ? iflags.wc_backgrnd_message : defbrief,
			iflags.wc_foregrnd_status ? iflags.wc_foregrnd_status : defbrief,
			iflags.wc_backgrnd_status ? iflags.wc_backgrnd_status : defbrief,
			iflags.wc_foregrnd_text ? iflags.wc_foregrnd_text : defbrief,
			iflags.wc_backgrnd_text ? iflags.wc_backgrnd_text : defbrief);
	else if (!strcmp(optname, "wolfname"))
		sprintf(buf, "%s", wolfname[0] ? wolfname : none);

	if (buf[0])
		return buf;
	else
		return "unknown";
}

int dotogglepickup(void) {
	char buf[BUFSZ], ocl[MAXOCLASSES + 1];

	flags.pickup = !flags.pickup;
	if (flags.pickup) {
		oc_to_str(flags.pickup_types, ocl);
		sprintf(buf, "ON, for %s objects%s", ocl[0] ? ocl : "all",
			(iflags.autopickup_exceptions[AP_LEAVE] ||
			 iflags.autopickup_exceptions[AP_GRAB]) ?
				((count_ape_maps(NULL, NULL) == 1) ?
					 ", with one exception" :
					 ", with some exceptions") :
				"");
	} else {
		strcpy(buf, "OFF");
	}
	pline("Autopickup: %s.", buf);
	return 0;
}

int add_autopickup_exception(const char *mapping) {
	struct autopickup_exception *ape, **apehead;
	char text[256], *text2;
	bool grab = false;

	if (sscanf(mapping, "\"%255[^\"]\"", text) == 1) {
		text2 = &text[0];
		if (*text2 == '<') { /* force autopickup */
			grab = true;
			++text2;
		} else if (*text2 == '>') { /* default - Do not pickup */
			grab = false;
			++text2;
		}

		regex_t regex;
		int errnum = tre_regcomp(&regex, text2, REG_EXTENDED | REG_NOSUB);
		if (errnum != 0) {
			char errbuf[BUFSZ];
			tre_regerror(errnum, &regex, errbuf, sizeof(errbuf));
			raw_printf("Bad regex in AUTOPICKUP_EXCEPTION: \"%s\": \"%s\"", text2, errbuf);
			return 0;
		}

		apehead = grab ? &iflags.autopickup_exceptions[AP_GRAB] :
				 &iflags.autopickup_exceptions[AP_LEAVE];
		ape = new(struct autopickup_exception);
		ape->pattern = regex;
		ape->text_pattern = strdup(text2);
		ape->grab = grab;
		ape->next = *apehead;
		*apehead = ape;
	} else {
		raw_print("syntax error in AUTOPICKUP_EXCEPTION");
		return 0;
	}

	return 1;
}

static void remove_autopickup_exception(struct autopickup_exception *whichape) {
	struct autopickup_exception *ape, *prev = 0;
	int chain = whichape->grab ? AP_GRAB : AP_LEAVE;

	for (ape = iflags.autopickup_exceptions[chain]; ape;) {
		if (ape == whichape) {
			struct autopickup_exception *freeape = ape;
			ape = ape->next;
			if (prev)
				prev->next = ape;
			else
				iflags.autopickup_exceptions[chain] = ape;
			tre_regfree(&freeape->pattern);
			free(freeape->text_pattern);
			free(freeape);
		} else {
			prev = ape;
			ape = ape->next;
		}
	}
}

static int count_ape_maps(int *leave, int *grab) {
	struct autopickup_exception *ape;
	int pass, totalapes, numapes[2] = {0, 0};

	for (pass = AP_LEAVE; pass <= AP_GRAB; ++pass) {
		ape = iflags.autopickup_exceptions[pass];
		while (ape) {
			ape = ape->next;
			numapes[pass]++;
		}
	}
	totalapes = numapes[AP_LEAVE] + numapes[AP_GRAB];
	if (leave) *leave = numapes[AP_LEAVE];
	if (grab) *grab = numapes[AP_GRAB];
	return totalapes;
}

void free_autopickup_exceptions(void) {
	struct autopickup_exception *ape;
	int pass;

	for (pass = AP_LEAVE; pass <= AP_GRAB; ++pass) {
		while ((ape = iflags.autopickup_exceptions[pass]) != 0) {
			tre_regfree(&ape->pattern);
			free(ape->text_pattern);
			iflags.autopickup_exceptions[pass] = ape->next;
			free(ape);
		}
	}
}

/* data for option_help() */
static const char *opt_intro[] = {
	"",
	"                 SlashEM Options Help:",
	"",
#define CONFIG_SLOT 3 /* fill in next value at run-time */
	NULL,
#define ENV_SLOT 4 /* substitute variable name in next value at run-time */
	"or use `%s=\"<options>\"' in your environment",
	"(<options> is a list of options separated by commas)",
	"or press \"O\" while playing and use the menu.",
	"",
	"Boolean options (which can be negated by prefixing them with '!' or \"no\"):",
	NULL};

static const char *opt_epilog[] = {
	"",
	"Some of the options can be set only before the game is started; those",
	"items will not be selectable in the 'O' command's menu.",
	NULL};

void option_help(void) {
	char buf[BUFSZ], buf2[QBUFSZ];
	int i;
	winid datawin;

	datawin = create_nhwindow(NHW_TEXT);
	sprintf(buf, "Set options as OPTIONS=<options> in %s", configfile);
	opt_intro[CONFIG_SLOT] = (const char *)buf;
	for (i = 0; opt_intro[i]; i++)
		if (i == ENV_SLOT) {
			sprintf(buf2, opt_intro[ENV_SLOT], NETHACK_ENV_OPTIONS);
			putstr(datawin, 0, buf2);
			;
		} else
			putstr(datawin, 0, opt_intro[i]);

	/* Boolean options */
	for (i = 0; boolopt[i].name; i++) {
		if (boolopt[i].addr) {
			if (boolopt[i].addr == &iflags.sanity_check && !wizard) continue;
			if (boolopt[i].addr == &iflags.menu_tab_sep && !wizard) continue;
			next_opt(datawin, boolopt[i].name);
		}
	}
	next_opt(datawin, "");

	/* Compound options */
	putstr(datawin, 0, "Compound options:");
	for (i = 0; compopt[i].name; i++) {
		sprintf(buf2, "`%s'", compopt[i].name);
		sprintf(buf, "%-20s - %s%c", buf2, compopt[i].descr,
			compopt[i + 1].name ? ',' : '.');
		putstr(datawin, 0, buf);
	}

	for (i = 0; opt_epilog[i]; i++)
		putstr(datawin, 0, opt_epilog[i]);

	display_nhwindow(datawin, false);
	destroy_nhwindow(datawin);
	return;
}

/*
 * prints the next boolean option, on the same line if possible, on a new
 * line if not. End with next_opt("").
 */
void next_opt(winid datawin, const char *str) {
	static char *buf = 0;
	int i;
	char *s;

	if (!buf) *(buf = alloc(BUFSZ)) = '\0';

	if (!*str) {
		s = eos(buf);
		if (s > &buf[1] && s[-2] == ',')
			strcpy(s - 2, "."); /* replace last ", " */
		i = COLNO;		    /* (greater than COLNO - 2) */
	} else {
		i = strlen(buf) + strlen(str) + 2;
	}

	if (i > COLNO - 2) { /* rule of thumb */
		putstr(datawin, 0, buf);
		buf[0] = 0;
	}
	if (*str) {
		strcat(buf, str);
		strcat(buf, ", ");
	} else {
		putstr(datawin, 0, str);
		free(buf), buf = 0;
	}
	return;
}

/* Returns the fid of the fruit type; if that type already exists, it
 * returns the fid of that one; if it does not exist, it adds a new fruit
 * type to the chain and returns the new one.
 */
int fruitadd(char *str) {
	int i;
	struct fruit *f;
	struct fruit *lastf = 0;
	int highest_fruit_id = 0;
	char buf[PL_FSIZ];
	bool user_specified = (str == pl_fruit);
	/* if not user-specified, then it's a fruit name for a fruit on
	 * a bones level...
	 */

	/* Note: every fruit has an id (spe for fruit objects) of at least
	 * 1; 0 is an error.
	 */
	if (user_specified) {
		/* disallow naming after other foods (since it'd be impossible
		 * to tell the difference)
		 */

		bool found = false, numeric = false;

		for (i = bases[FOOD_CLASS]; objects[i].oc_class == FOOD_CLASS;
		     i++) {
			if (!strcmp(OBJ_NAME(objects[i]), pl_fruit)) {
				found = true;
				break;
			}
		}
		{
			char *c;

			c = pl_fruit;

			for (c = pl_fruit; *c >= '0' && *c <= '9'; c++)
				;
			if (isspace(*c) || *c == 0) numeric = true;
		}
		if (found || numeric ||
		    !strncmp(str, "cursed ", 7) ||
		    !strncmp(str, "uncursed ", 9) ||
		    !strncmp(str, "blessed ", 8) ||
		    !strncmp(str, "partly eaten ", 13) ||
		    (!strncmp(str, "tin of ", 7) &&
		     (!strcmp(str + 7, "spinach") ||
		      name_to_mon(str + 7) >= LOW_PM)) ||
		    !strcmp(str, "empty tin") ||
		    ((str_end_is(str, " corpse") ||
		      str_end_is(str, " egg")) &&
		     name_to_mon(str) >= LOW_PM)) {
			strcpy(buf, pl_fruit);
			strcpy(pl_fruit, "candied ");
			nmcpy(pl_fruit + 8, buf, PL_FSIZ - 8);
		}
	}
	for (f = ffruit; f; f = f->nextf) {
		lastf = f;
		if (f->fid > highest_fruit_id) highest_fruit_id = f->fid;
		if (!strncmp(str, f->fname, PL_FSIZ))
			goto nonew;
	}
	/* if adding another fruit would overflow spe, use a random
	   fruit instead... we've got a lot to choose from. */
	if (highest_fruit_id >= 127) return rnd(127);
	highest_fruit_id++;
	f = newfruit();
	if (ffruit)
		lastf->nextf = f;
	else
		ffruit = f;
	strcpy(f->fname, str);
	f->fid = highest_fruit_id;
	f->nextf = 0;
nonew:
	if (user_specified) current_fruit = highest_fruit_id;
	return f->fid;
}

/*
 * This is a somewhat generic menu for taking a list of NetHack style
 * class choices and presenting them via a description
 * rather than the traditional NetHack characters.
 * (Benefits users whose first exposure to NetHack is via tiles).
 *
 * prompt
 *	     The title at the top of the menu.
 *
 * category: 0 = monster class
 *           1 = object  class
 *
 * way
 *	     false = PICK_ONE, true = PICK_ANY
 *
 * class_list
 *	     a null terminated string containing the list of choices.
 *
 * class_selection
 *	     a null terminated string containing the selected characters.
 *
 * Returns number selected.
 */
int choose_classes_menu(const char *prompt, int category, boolean way, char *class_list, char *class_select) {
	menu_item *pick_list = NULL;
	winid win;
	anything any;
	char buf[BUFSZ];
	int i, n;
	int ret;
	int next_accelerator, accelerator;

	if (class_list == NULL || class_select == NULL) return 0;
	accelerator = 0;
	next_accelerator = 'a';
	any.a_void = NULL;
	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	while (*class_list) {
		const char *text;
		boolean selected;

		text = NULL;
		selected = false;
		switch (category) {
			case 0:
				text = monexplain[def_char_to_monclass(*class_list)];
				accelerator = *class_list;
				sprintf(buf, "%s", text);
				break;
			case 1:
				text = objexplain[def_char_to_objclass(*class_list)];
				accelerator = next_accelerator;
				sprintf(buf, "%c  %s", *class_list, text);
				break;
			default:
				impossible("choose_classes_menu: invalid category %d",
					   category);
		}
		if (way && *class_select) { /* Selections there already */
			if (index(class_select, *class_list)) {
				selected = true;
			}
		}
		any.a_int = *class_list;
		add_menu(win, NO_GLYPH, &any, accelerator,
			 category ? *class_list : 0,
			 ATR_NONE, buf, selected);
		++class_list;
		if (category > 0) {
			++next_accelerator;
			if (next_accelerator == ('z' + 1)) next_accelerator = 'A';
			if (next_accelerator == ('Z' + 1)) break;
		}
	}
	end_menu(win, prompt);
	n = select_menu(win, way ? PICK_ANY : PICK_ONE, &pick_list);
	destroy_nhwindow(win);
	if (n > 0) {
		for (i = 0; i < n; ++i)
			*class_select++ = (char)pick_list[i].item.a_int;
		free(pick_list);
		ret = n;
	} else if (n == -1) {
		class_select = eos(class_select);
		ret = -1;
	} else
		ret = 0;
	*class_select = '\0';
	return ret;
}

struct wc_Opt wc_options[] = {
	{"color", WC_COLOR},
	{"hilite_pet", WC_HILITE_PET},
	{"popup_dialog", WC_POPUP_DIALOG},
	{"player_selection", WC_PLAYER_SELECTION},
	{"preload_tiles", WC_PRELOAD_TILES},
	{"tiled_map", WC_TILED_MAP},
	{"tile_file", WC_TILE_FILE},
	{"tile_width", WC_TILE_WIDTH},
	{"tile_height", WC_TILE_HEIGHT},
	{"use_inverse", WC_INVERSE},
	{"align_message", WC_ALIGN_MESSAGE},
	{"align_status", WC_ALIGN_STATUS},
	{"font_map", WC_FONT_MAP},
	{"font_menu", WC_FONT_MENU},
	{"font_message", WC_FONT_MESSAGE},
#if 0
	{"perm_invent",WC_PERM_INVENT},
#endif
	{"font_size_map", WC_FONTSIZ_MAP},
	{"font_size_menu", WC_FONTSIZ_MENU},
	{"font_size_message", WC_FONTSIZ_MESSAGE},
	{"font_size_status", WC_FONTSIZ_STATUS},
	{"font_size_text", WC_FONTSIZ_TEXT},
	{"font_status", WC_FONT_STATUS},
	{"font_text", WC_FONT_TEXT},
	{"map_mode", WC_MAP_MODE},
	{"scroll_amount", WC_SCROLL_AMOUNT},
	{"scroll_margin", WC_SCROLL_MARGIN},
	{"splash_screen", WC_SPLASH_SCREEN},
	{"vary_msgcount", WC_VARY_MSGCOUNT},
	{"windowcolors", WC_WINDOWCOLORS},
	{"mouse_support", WC_MOUSE_SUPPORT},
	{NULL, 0L}};

struct wc_Opt wc2_options[] = {
	{"fullscreen", WC2_FULLSCREEN},
	{"wraptext", WC2_WRAPTEXT},
	{"term_cols", WC2_TERM_COLS},
	{"term_rows", WC2_TERM_ROWS},
	{"windowborders", WC2_WINDOWBORDERS},
	{"petattr", WC2_PETATTR},
	{NULL, 0L}};

/*
 * If a port wants to change or ensure that the
 * SET_IN_FILE, DISP_IN_GAME, or SET_IN_GAME status of an option is
 * correct (for controlling its display in the option menu) call
 * set_option_mod_status()
 * with the second argument of 0,2, or 3 respectively.
 */
void set_option_mod_status(const char *optnam, int status) {
	int k;
	if (status < SET_IN_FILE || status > SET_IN_GAME) {
		impossible("set_option_mod_status: status out of range %d.",
			   status);
		return;
	}
	for (k = 0; boolopt[k].name; k++) {
		if (!strncmpi(boolopt[k].name, optnam, strlen(optnam))) {
			boolopt[k].optflags = status;
			return;
		}
	}
	for (k = 0; compopt[k].name; k++) {
		if (!strncmpi(compopt[k].name, optnam, strlen(optnam))) {
			compopt[k].optflags = status;
			return;
		}
	}
}

/*
 * You can set several wc_options in one call to
 * set_wc_option_mod_status() by setting
 * the appropriate bits for each option that you
 * are setting in the optmask argument
 * prior to calling.
 *    example: set_wc_option_mod_status(WC_COLOR|WC_SCROLL_MARGIN, SET_IN_GAME);
 */
void set_wc_option_mod_status(unsigned long optmask, int status) {
	int k = 0;
	if (status < SET_IN_FILE || status > SET_IN_GAME) {
		impossible("set_wc_option_mod_status: status out of range %d.",
			   status);
		return;
	}
	while (wc_options[k].wc_name) {
		if (optmask & wc_options[k].wc_bit) {
			set_option_mod_status(wc_options[k].wc_name, status);
		}
		k++;
	}
}

static boolean is_wc_option(const char *optnam) {
	int k = 0;
	while (wc_options[k].wc_name) {
		if (strcmp(wc_options[k].wc_name, optnam) == 0)
			return true;
		k++;
	}
	return false;
}

static boolean wc_supported(const char *optnam) {
	int k = 0;
	while (wc_options[k].wc_name) {
		if (!strcmp(wc_options[k].wc_name, optnam) &&
		    (windowprocs.wincap & wc_options[k].wc_bit))
			return true;
		k++;
	}
	return false;
}

/*
 * You can set several wc2_options in one call to
 * set_wc2_option_mod_status() by setting
 * the appropriate bits for each option that you
 * are setting in the optmask argument
 * prior to calling.
 *    example: set_wc2_option_mod_status(WC2_FULLSCREEN|WC2_SOFTKEYBOARD|WC2_WRAPTEXT, SET_IN_FILE);
 */

void set_wc2_option_mod_status(unsigned long optmask, int status) {
	int k = 0;
	if (status < SET_IN_FILE || status > SET_IN_GAME) {
		impossible("set_wc2_option_mod_status: status out of range %d.",
			   status);
		return;
	}
	while (wc2_options[k].wc_name) {
		if (optmask & wc2_options[k].wc_bit) {
			set_option_mod_status(wc2_options[k].wc_name, status);
		}
		k++;
	}
}

static boolean is_wc2_option(const char *optnam) {
	int k = 0;
	while (wc2_options[k].wc_name) {
		if (strcmp(wc2_options[k].wc_name, optnam) == 0)
			return true;
		k++;
	}
	return false;
}

static boolean wc2_supported(const char *optnam) {
	int k = 0;
	while (wc2_options[k].wc_name) {
		if (!strcmp(wc2_options[k].wc_name, optnam) &&
		    (windowprocs.wincap2 & wc2_options[k].wc_bit))
			return true;
		k++;
	}
	return false;
}

static void wc_set_font_name(int wtype, char *fontname) {
	char **fn = NULL;
	if (!fontname) return;
	switch (wtype) {
		case NHW_MAP:
			fn = &iflags.wc_font_map;
			break;
		case NHW_MESSAGE:
			fn = &iflags.wc_font_message;
			break;
		case NHW_TEXT:
			fn = &iflags.wc_font_text;
			break;
		case NHW_MENU:
			fn = &iflags.wc_font_menu;
			break;
		case NHW_STATUS:
			fn = &iflags.wc_font_status;
			break;
		default:
			return;
	}
	if (fn) {
		if (*fn) free(*fn);
		*fn = alloc(strlen(fontname) + 1);
		strcpy(*fn, fontname);
	}
	return;
}

static int wc_set_window_colors(char *op) {
	/* syntax:
	 *  menu white/black message green/yellow status white/blue text white/black
	 */

	int j;
	char buf[BUFSZ];
	char *wn, *tfg, *tbg, *newop;
	static const char *wnames[] = {"menu", "message", "status", "text"};
	static const char *shortnames[] = {"mnu", "msg", "sts", "txt"};
	static char **fgp[] = {
		&iflags.wc_foregrnd_menu,
		&iflags.wc_foregrnd_message,
		&iflags.wc_foregrnd_status,
		&iflags.wc_foregrnd_text};
	static char **bgp[] = {
		&iflags.wc_backgrnd_menu,
		&iflags.wc_backgrnd_message,
		&iflags.wc_backgrnd_status,
		&iflags.wc_backgrnd_text};

	strcpy(buf, op);
	newop = mungspaces(buf);
	while (newop && *newop) {
		wn = tfg = tbg = NULL;

		/* until first non-space in case there's leading spaces - before colorname*/
		while (*newop && isspace(*newop))
			newop++;
		if (*newop)
			wn = newop;
		else
			return 0;

		/* until first space - colorname*/
		while (*newop && !isspace(*newop))
			newop++;
		if (*newop)
			*newop = '\0';
		else
			return 0;
		newop++;

		/* until first non-space - before foreground*/
		while (*newop && isspace(*newop))
			newop++;
		if (*newop)
			tfg = newop;
		else
			return 0;

		/* until slash - foreground */
		while (*newop && *newop != '/')
			newop++;
		if (*newop)
			*newop = '\0';
		else
			return 0;
		newop++;

		/* until first non-space (in case there's leading space after slash) - before background */
		while (*newop && isspace(*newop))
			newop++;
		if (*newop)
			tbg = newop;
		else
			return 0;

		/* until first space - background */
		while (*newop && !isspace(*newop))
			newop++;
		if (*newop) *newop++ = '\0';

		for (j = 0; j < 4; ++j) {
			if (!strcmpi(wn, wnames[j]) ||
			    !strcmpi(wn, shortnames[j])) {
				if (tfg && !strstri(tfg, " ")) {
					if (*fgp[j]) free(*fgp[j]);
					*fgp[j] = alloc(strlen(tfg) + 1);
					strcpy(*fgp[j], tfg);
				}
				if (tbg && !strstri(tbg, " ")) {
					if (*bgp[j]) free(*bgp[j]);
					*bgp[j] = alloc(strlen(tbg) + 1);
					strcpy(*bgp[j], tbg);
				}
				break;
			}
		}
	}
	return 1;
}
/*options.c*/
