#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursinit.h"
#include "patchlevel.h"

#include <ctype.h>

/* Initialization and startup functions for curses interface */

static void set_window_position(int *, int *, int *, int *, int,
				int *, int *, int *, int *, int,
				int, int);

/* array to save initial terminal colors for later restoration */

typedef struct nhrgb_type {
	short r;
	short g;
	short b;
} nhrgb;

nhrgb orig_yellow;
nhrgb orig_white;
nhrgb orig_darkgray;
nhrgb orig_hired;
nhrgb orig_higreen;
nhrgb orig_hiyellow;
nhrgb orig_hiblue;
nhrgb orig_himagenta;
nhrgb orig_hicyan;
nhrgb orig_hiwhite;

/* Banners used for an ASCII splash screen */
static const char *fancy_splash_text[] = {
	"     ▄▄▄▄    ▄▄▄▄                          ▄▄         ▄▄   ▄▄▄▄▄▄▄▄  ▄▄▄  ▄▄▄",
	"   ▄█▀▀▀▀█   ▀▀██                          ██         ██   ██▀▀▀▀▀▀  ███  ███",
	"   ██▄         ██       ▄█████▄  ▄▄█████▄  ██▄████▄   ▀▀   ██        ████████",
	"    ▀████▄     ██       ▀ ▄▄▄██  ██▄▄▄▄ ▀  ██▀   ██        ███████   ██ ██ ██",
	"        ▀██    ██      ▄██▀▀▀██   ▀▀▀▀██▄  ██    ██        ██        ██ ▀▀ ██",
	"   █▄▄▄▄▄█▀    ██▄▄▄   ██▄▄▄███  █▄▄▄▄▄██  ██    ██        ██▄▄▄▄▄▄  ██    ██",
	"    ▀▀▀▀▀       ▀▀▀▀    ▀▀▀▀ ▀▀   ▀▀▀▀▀▀   ▀▀    ▀▀        ▀▀▀▀▀▀▀▀  ▀▀    ▀▀",
	"   ▄▄▄   ▄▄",
	"   ███   ██                        ██",
	"   ██▀█  ██   ▄████▄   ▀██  ██▀  ███████",
	"   ██ ██ ██  ██▄▄▄▄██    ████      ██",
	"   ██  █▄██  ██▀▀▀▀▀▀    ▄██▄      ██",
	"   ██   ███  ▀██▄▄▄▄█   ▄█▀▀█▄     ██▄▄▄",
	"   ▀▀   ▀▀▀    ▀▀▀▀▀   ▀▀▀  ▀▀▀     ▀▀▀▀"};

/*
   ▄▄▄▄    ▄▄▄▄                          ▄▄           ▄▄     ▄▄▄▄▄▄▄▄  ▄▄▄  ▄▄▄
 ▄█▀▀▀▀█   ▀▀██                          ██           ██     ██▀▀▀▀▀▀  ███  ███
 ██▄         ██       ▄█████▄  ▄▄█████▄  ██▄████▄     ▀▀     ██        ████████
  ▀████▄     ██       ▀ ▄▄▄██  ██▄▄▄▄ ▀  ██▀   ██            ███████   ██ ██ ██
      ▀██    ██      ▄██▀▀▀██   ▀▀▀▀██▄  ██    ██            ██        ██ ▀▀ ██
 █▄▄▄▄▄█▀    ██▄▄▄   ██▄▄▄███  █▄▄▄▄▄██  ██    ██            ██▄▄▄▄▄▄  ██    ██
  ▀▀▀▀▀       ▀▀▀▀    ▀▀▀▀ ▀▀   ▀▀▀▀▀▀   ▀▀    ▀▀            ▀▀▀▀▀▀▀▀  ▀▀    ▀▀
 ▄▄▄   ▄▄
 ███   ██                        ██
 ██▀█  ██   ▄████▄   ▀██  ██▀  ███████
 ██ ██ ██  ██▄▄▄▄██    ████      ██
 ██  █▄██  ██▀▀▀▀▀▀    ▄██▄      ██
 ██   ███  ▀██▄▄▄▄█   ▄█▀▀█▄     ██▄▄▄
 ▀▀   ▀▀▀    ▀▀▀▀▀   ▀▀▀  ▀▀▀     ▀▀▀▀
*/

/* win* is size and placement of window to change, x/y/w/h is baseline which can
   decrease depending on alignment of win* in orientation.
   Negative minh/minw: as much as possible, but at least as much as specified. */
static void
set_window_position(int *winx, int *winy, int *winw, int *winh, int orientation,
		    int *x, int *y, int *w, int *h, int border_space,
		    int minh, int minw) {
	*winw = *w;
	*winh = *h;

	/* Set window height/width */
	if (orientation == ALIGN_TOP || orientation == ALIGN_BOTTOM) {
		if (minh < 0) {
			*winh = (*h - ROWNO - border_space);
			if (-minh > *winh)
				*winh = -minh;
		} else
			*winh = minh;
		*h -= (*winh + border_space);
	} else {
		if (minw < 0) {
			*winw = (*w - COLNO - border_space);
			if (-minw > *winw)
				*winw = -minw;
		} else
			*winw = minw;
		*w -= (*winw + border_space);
	}

	*winx = *w + border_space + *x;
	*winy = *h + border_space + *y;

	/* Set window position */
	if (orientation != ALIGN_RIGHT) {
		*winx = *x;
		if (orientation == ALIGN_LEFT)
			*x += *winw + border_space;
	}
	if (orientation != ALIGN_BOTTOM) {
		*winy = *y;
		if (orientation == ALIGN_TOP)
			*y += *winh + border_space;
	}
}

/* Create the "main" nonvolitile windows used by nethack */

void curses_create_main_windows() {
	int message_orientation = 0;
	int status_orientation = 0;
	int border_space = 0;
	int hspace = term_cols - 80;
	bool borders = false;

	switch (iflags.wc2_windowborders) {
		case 1: /* On */
			borders = true;
			break;
		case 2: /* Off */
			borders = false;
			break;
		case 3: /* Auto */
			if ((term_cols > 81) && (term_rows > 25)) {
				borders = true;
			}
			break;
		default:
			borders = false;
	}

	if (borders) {
		border_space = 2;
		hspace -= border_space;
	}

	/* Determine status window orientation */
	if (!iflags.wc_align_status || (iflags.wc_align_status == ALIGN_TOP) || (iflags.wc_align_status == ALIGN_BOTTOM)) {
		if (!iflags.wc_align_status) {
			iflags.wc_align_status = ALIGN_BOTTOM;
		}
		status_orientation = iflags.wc_align_status;
	} else { /* left or right alignment */

		/* Max space for player name and title horizontally */
		if ((hspace >= 26) && (term_rows >= 24)) {
			status_orientation = iflags.wc_align_status;
			hspace -= (26 + border_space);
		} else {
			status_orientation = ALIGN_BOTTOM;
		}
	}

	/* Determine message window orientation */
	if (!iflags.wc_align_message || (iflags.wc_align_message == ALIGN_TOP) || (iflags.wc_align_message == ALIGN_BOTTOM)) {
		if (!iflags.wc_align_message) {
			iflags.wc_align_message = ALIGN_TOP;
		}
		message_orientation = iflags.wc_align_message;
	} else { /* left or right alignment */

		if ((hspace - border_space) >= 25) { /* Arbitrary */
			message_orientation = iflags.wc_align_message;
		} else {
			message_orientation = ALIGN_TOP;
		}
	}

	/* Figure out window positions and placements. Status and message area can be aligned
       based on configuration. The priority alignment-wise is: status > msgarea > game.
       Define everything as taking as much space as possible and shrink/move based on
       alignment positions. */
	int message_x = 0;
	int message_y = 0;
	int status_x = 0;
	int status_y = 0;
	int inv_x = 0;
	int inv_y = 0;
	int map_x = 0;
	int map_y = 0;

	int message_height = 0;
	int message_width = 0;
	int status_height = 0;
	int status_width = 0;
	int inv_height = 0;
	int inv_width = 0;
	int map_height = (term_rows - border_space);
	int map_width = (term_cols - border_space);

	bool status_vertical = false;
	bool msg_vertical = false;
	if (status_orientation == ALIGN_LEFT ||
	    status_orientation == ALIGN_RIGHT)
		status_vertical = true;
	if (message_orientation == ALIGN_LEFT ||
	    message_orientation == ALIGN_RIGHT)
		msg_vertical = true;

	int statusheight = 3;
	if (iflags.classic_status)
		statusheight = 2;

	/* Vertical windows have priority. Otherwise, priotity is:
       status > inv > msg */
	if (status_vertical)
		set_window_position(&status_x, &status_y, &status_width, &status_height,
				    status_orientation, &map_x, &map_y, &map_width, &map_height,
				    border_space, statusheight, 26);

	if (flags.perm_invent) {
		/* Take up all width unless msgbar is also vertical. */
		int width = -25;
		if (msg_vertical)
			width = 25;

		set_window_position(&inv_x, &inv_y, &inv_width, &inv_height,
				    ALIGN_RIGHT, &map_x, &map_y, &map_width, &map_height,
				    border_space, -1, width);
	}

	if (msg_vertical)
		set_window_position(&message_x, &message_y, &message_width, &message_height,
				    message_orientation, &map_x, &map_y, &map_width, &map_height,
				    border_space, -1, -25);

	/* Now draw horizontal windows */
	if (!status_vertical)
		set_window_position(&status_x, &status_y, &status_width, &status_height,
				    status_orientation, &map_x, &map_y, &map_width, &map_height,
				    border_space, statusheight, 26);

	if (!msg_vertical)
		set_window_position(&message_x, &message_y, &message_width, &message_height,
				    message_orientation, &map_x, &map_y, &map_width, &map_height,
				    border_space, -1, -25);

	if (map_width > COLNO)
		map_width = COLNO;

	if (map_height > ROWNO)
		map_height = ROWNO;

	if (curses_get_nhwin(STATUS_WIN)) {
		curses_del_nhwin(STATUS_WIN);
		/* 'count window' overlays last line of mesg win;
           asking it to display a Null string removes it */
		curses_count_window(NULL);

		curses_del_nhwin(MESSAGE_WIN);
		curses_del_nhwin(MAP_WIN);
		curses_del_nhwin(INV_WIN);

		clear();
	}

	curses_add_nhwin(STATUS_WIN, status_height, status_width, status_y,
			 status_x, status_orientation, borders);

	curses_add_nhwin(MESSAGE_WIN, message_height, message_width, message_y,
			 message_x, message_orientation, borders);

	if (flags.perm_invent)
		curses_add_nhwin(INV_WIN, inv_height, inv_width, inv_y, inv_x,
				 ALIGN_RIGHT, borders);

	curses_add_nhwin(MAP_WIN, map_height, map_width, map_y, map_x, 0, borders);

	refresh();

	curses_refresh_nethack_windows();

	if (iflags.window_inited) {
		curses_update_stats();
		if (flags.perm_invent)
			curses_update_inventory();
	} else {
		iflags.window_inited = true;
	}
}

/* Initialize curses colors to colors used by NetHack */

void curses_init_nhcolors() {
	if (has_colors()) {
		COLORS = 8;
		// force the rest of the library to assume only 8 colours are
		// available, so it uses bold properly.

		use_default_colors();
		init_pair(1, COLOR_BLACK, -1);
		init_pair(2, COLOR_RED, -1);
		init_pair(3, COLOR_GREEN, -1);
		init_pair(4, COLOR_YELLOW, -1);
		init_pair(5, COLOR_BLUE, -1);
		init_pair(6, COLOR_MAGENTA, -1);
		init_pair(7, COLOR_CYAN, -1);
		init_pair(8, -1, -1);

		{
			int i;

			int clr_remap[16] = {
				COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
				COLOR_BLUE,
				COLOR_MAGENTA, COLOR_CYAN, -1, COLOR_WHITE,
				COLOR_RED + 8, COLOR_GREEN + 8, COLOR_YELLOW + 8,
				COLOR_BLUE + 8,
				COLOR_MAGENTA + 8, COLOR_CYAN + 8, COLOR_WHITE + 8};

			for (i = 0; i < (COLORS >= 16 ? 16 : 8); i++) {
				init_pair(17 + (i * 2) + 0, clr_remap[i], COLOR_RED);
				init_pair(17 + (i * 2) + 1, clr_remap[i], COLOR_BLUE);
			}

			bool hicolor = false;
			if (COLORS >= 16)
				hicolor = true;

			/* Work around the crazy definitions above for more background colors... */
			for (i = 0; i < (COLORS >= 16 ? 16 : 8); i++) {
				init_pair((hicolor ? 49 : 9) + i, clr_remap[i], COLOR_GREEN);
				init_pair((hicolor ? 65 : 33) + i, clr_remap[i], COLOR_YELLOW);
				init_pair((hicolor ? 81 : 41) + i, clr_remap[i], COLOR_MAGENTA);
				init_pair((hicolor ? 97 : 49) + i, clr_remap[i], COLOR_CYAN);
				init_pair((hicolor ? 113 : 57) + i, clr_remap[i], COLOR_WHITE);
			}
		}

		if (COLORS >= 16) {
			init_pair(9, COLOR_WHITE, -1);
			init_pair(10, COLOR_RED + 8, -1);
			init_pair(11, COLOR_GREEN + 8, -1);
			init_pair(12, COLOR_YELLOW + 8, -1);
			init_pair(13, COLOR_BLUE + 8, -1);
			init_pair(14, COLOR_MAGENTA + 8, -1);
			init_pair(15, COLOR_CYAN + 8, -1);
			init_pair(16, COLOR_WHITE + 8, -1);
		}

		if (can_change_color()) {
			/* Preserve initial terminal colors */
			color_content(COLOR_YELLOW, &orig_yellow.r, &orig_yellow.g,
				      &orig_yellow.b);
			color_content(COLOR_WHITE, &orig_white.r, &orig_white.g,
				      &orig_white.b);

			/* Set colors to appear as NetHack expects */
			init_color(COLOR_YELLOW, 500, 300, 0);
			init_color(COLOR_WHITE, 600, 600, 600);
			if (COLORS >= 16) {
				/* Preserve initial terminal colors */
				color_content(COLOR_RED + 8, &orig_hired.r,
					      &orig_hired.g, &orig_hired.b);
				color_content(COLOR_GREEN + 8, &orig_higreen.r,
					      &orig_higreen.g, &orig_higreen.b);
				color_content(COLOR_YELLOW + 8, &orig_hiyellow.r,
					      &orig_hiyellow.g, &orig_hiyellow.b);
				color_content(COLOR_BLUE + 8, &orig_hiblue.r,
					      &orig_hiblue.g, &orig_hiblue.b);
				color_content(COLOR_MAGENTA + 8, &orig_himagenta.r,
					      &orig_himagenta.g, &orig_himagenta.b);
				color_content(COLOR_CYAN + 8, &orig_hicyan.r,
					      &orig_hicyan.g, &orig_hicyan.b);
				color_content(COLOR_WHITE + 8, &orig_hiwhite.r,
					      &orig_hiwhite.g, &orig_hiwhite.b);

				/* Set colors to appear as NetHack expects */
				init_color(COLOR_RED + 8, 1000, 500, 0);
				init_color(COLOR_GREEN + 8, 0, 1000, 0);
				init_color(COLOR_YELLOW + 8, 1000, 1000, 0);
				init_color(COLOR_BLUE + 8, 0, 0, 1000);
				init_color(COLOR_MAGENTA + 8, 1000, 0, 1000);
				init_color(COLOR_CYAN + 8, 0, 1000, 1000);
				init_color(COLOR_WHITE + 8, 1000, 1000, 1000);
				if (COLORS > 16) {
					color_content(CURSES_DARK_GRAY, &orig_darkgray.r,
						      &orig_darkgray.g, &orig_darkgray.b);
					init_color(CURSES_DARK_GRAY, 300, 300, 300);
					/* just override black colorpair entry here */
					init_pair(1, CURSES_DARK_GRAY, -1);
				}
			} else {
				/* Set flag to use bold for bright colors */
			}
		}
	}
}

/* Allow player to pick character's role, race, gender, and alignment.
Borrowed from the Gnome window port. */

void curses_choose_character() {
	int n, i, sel, count_off, pick4u;
	int count = 0;
	int cur_character = 0;
	const char **choices;
	int *pickmap;
	char *prompt;
	char pbuf[QBUFSZ];
	char choice[BUFSZ];
	char tmpchoice[QBUFSZ];

#ifdef TUTORIAL_MODE
	winid win;
	anything any;
	menu_item *selected = 0;
#endif

	prompt = build_plselection_prompt(pbuf, QBUFSZ, flags.initrole,
					  flags.initrace, flags.initgend,
					  flags.initalign);

	/* This part is irritating: we have to strip the choices off of
       the string and put them in a separate string in order to use
       curses_character_input_dialog for this prompt. */

	while (cur_character != '[') {
		cur_character = prompt[count];
		count++;
	}

	count_off = count;

	while (cur_character != ']') {
		tmpchoice[count - count_off] = prompt[count];
		count++;
		cur_character = prompt[count];
	}

	tmpchoice[count - count_off] = '\0';
	lcase(tmpchoice);

	while (!isspace(prompt[count_off])) {
		count_off--;
	}

	prompt[count_off] = '\0';
	sprintf(choice, "%s%c", tmpchoice, '\033');
	if (strchr(tmpchoice, 't')) { /* Tutorial mode */
		mvaddstr(0, 1, "New? Press t to enter a tutorial.");
	}

	/* Add capital letters as choices that aren't displayed */

	for (count = 0; tmpchoice[count]; count++) {
		tmpchoice[count] = toupper(tmpchoice[count]);
	}

	strcat(choice, tmpchoice);

	/* prevent an unnecessary prompt */
	rigid_role_checks();

	if (!flags.randomall &&
	    (flags.initrole == ROLE_NONE || flags.initrace == ROLE_NONE ||
	     flags.initgend == ROLE_NONE || flags.initalign == ROLE_NONE)) {
		pick4u = tolower(curses_character_input_dialog(prompt, choice, 'y'));
	} else {
		pick4u = 'y';
	}

	if (pick4u == 'q') { /* Quit or cancelled */
		clearlocks();
		curses_bail(0);
	}

	if (pick4u == 'y') {
		flags.randomall = true;
	}
#ifdef TUTORIAL_MODE
	else if (pick4u == 't') { /* Tutorial mode in UnNetHack */
		clear();
		mvaddstr(0, 1, "Choose a character");
		refresh();
		win = curses_get_wid(NHW_MENU);
		curses_create_nhmenu(win);
		any.a_int = 1;
		curses_add_menu(win, NO_GLYPH, &any, 'v', 0, ATR_NONE,
				"lawful female dwarf Valkyrie (uses melee and thrown weapons)",
				MENU_UNSELECTED);
		any.a_int = 2;
		curses_add_menu(win, NO_GLYPH, &any, 'w', 0, ATR_NONE,
				"chaotic male elf Wizard (relies mostly on spells)",
				MENU_UNSELECTED);
		any.a_int = 3;
		curses_add_menu(win, NO_GLYPH, &any, 'R', 0, ATR_NONE,
				"neutral female human Ranger (good with ranged combat)",
				MENU_UNSELECTED);
		any.a_int = 4;
		curses_add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
				"quit", MENU_UNSELECTED);
		curses_end_menu(win, "What character do you want to try?");
		n = curses_select_menu(win, PICK_ONE, &selected);
		destroy_nhwindow(win);
		if (n != 1 || selected[0].item.a_int == 4) {
			clearlocks();
			curses_bail(0);
		}
		switch (selected[0].item.a_int) {
			case 1:
				flags.initrole = str2role("Valkyrie");
				flags.initrace = str2race("dwarf");
				flags.initgend = str2gend("female");
				flags.initalign = str2align("lawful");
				break;
			case 2:
				flags.initrole = str2role("Wizard");
				flags.initrace = str2race("elf");
				flags.initgend = str2gend("male");
				flags.initalign = str2align("chaotic");
				break;
			case 3:
				flags.initrole = str2role("Ranger");
				flags.initrace = str2race("human");
				flags.initgend = str2gend("female");
				flags.initalign = str2align("neutral");
				break;
			default:
				panic("Impossible menu selection");
				break;
		}
		free(selected);
		selected = 0;
		flags.tutorial = 1;
	}
#endif

	clear();
	refresh();

	if (!flags.randomall && flags.initrole < 0) {
		/* select a role */
		for (n = 0; roles[n].name.m; n++)
			continue;
		choices = alloc(sizeof(char *) * (n + 1));
		pickmap = alloc(sizeof(int) * (n + 1));
		for (;;) {
			for (n = 0, i = 0; roles[i].name.m; i++) {
				if (ok_role(i, flags.initrace, flags.initgend, flags.initalign)) {
					if (flags.initgend >= 0 && flags.female && roles[i].name.f)
						choices[n] = roles[i].name.f;
					else
						choices[n] = roles[i].name.m;
					pickmap[n++] = i;
				}
			}
			if (n > 0)
				break;
			else if (flags.initalign >= 0)
				flags.initalign = -1; /* reset */
			else if (flags.initgend >= 0)
				flags.initgend = -1;
			else if (flags.initrace >= 0)
				flags.initrace = -1;
			else
				panic("no available ROLE+race+gender+alignment combinations");
		}
		choices[n] = NULL;
		if (n > 1)
			sel =
				curses_character_dialog(choices, "Choose one of the following roles:");
		else
			sel = 0;
		if (sel >= 0)
			sel = pickmap[sel];
		else if (sel == ROLE_NONE) { /* Quit */
			clearlocks();
			curses_bail(0);
		}
		free(choices);
		free(pickmap);
	} else if (flags.initrole < 0)
		sel = ROLE_RANDOM;
	else
		sel = flags.initrole;

	if (sel == ROLE_RANDOM) { /* Random role */
		sel = pick_role(flags.initrace, flags.initgend,
				flags.initalign, PICK_RANDOM);
		if (sel < 0)
			sel = randrole();
	}

	flags.initrole = sel;

	/* Select a race, if necessary */
	/* force compatibility with role, try for compatibility with
     * pre-selected gender/alignment */
	if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
		if (flags.initrace == ROLE_RANDOM || flags.randomall) {
			flags.initrace = pick_race(flags.initrole, flags.initgend,
						   flags.initalign, PICK_RANDOM);
			if (flags.initrace < 0)
				flags.initrace = randrace(flags.initrole);
		} else {
			/* Count the number of valid races */
			n = 0; /* number valid */
			for (i = 0; races[i].noun; i++) {
				if (ok_race(flags.initrole, i, flags.initgend, flags.initalign))
					n++;
			}
			if (n == 0) {
				for (i = 0; races[i].noun; i++) {
					if (validrace(flags.initrole, i))
						n++;
				}
			}

			choices = alloc(sizeof(char *) * (n + 1));
			pickmap = alloc(sizeof(int) * (n + 1));
			for (n = 0, i = 0; races[i].noun; i++) {
				if (ok_race(flags.initrole, i, flags.initgend, flags.initalign)) {
					choices[n] = races[i].noun;
					pickmap[n++] = i;
				}
			}
			choices[n] = NULL;
			/* Permit the user to pick, if there is more than one */
			if (n > 1)
				sel = curses_character_dialog(choices, "Choose one of the following races:");
			else
				sel = 0;
			if (sel >= 0)
				sel = pickmap[sel];
			else if (sel == ROLE_NONE) { /* Quit */
				clearlocks();
				curses_bail(0);
			}
			flags.initrace = sel;
			free(choices);
			free(pickmap);
		}
		if (flags.initrace == ROLE_RANDOM) { /* Random role */
			sel = pick_race(flags.initrole, flags.initgend,
					flags.initalign, PICK_RANDOM);
			if (sel < 0)
				sel = randrace(flags.initrole);
			flags.initrace = sel;
		}
	}

	/* Select a gender, if necessary */
	/* force compatibility with role/race, try for compatibility with
     * pre-selected alignment */
	if (flags.initgend < 0 ||
	    !validgend(flags.initrole, flags.initrace, flags.initgend)) {
		if (flags.initgend == ROLE_RANDOM || flags.randomall) {
			flags.initgend = pick_gend(flags.initrole, flags.initrace,
						   flags.initalign, PICK_RANDOM);
			if (flags.initgend < 0)
				flags.initgend = randgend(flags.initrole, flags.initrace);
		} else {
			/* Count the number of valid genders */
			n = 0; /* number valid */
			for (i = 0; i < ROLE_GENDERS; i++) {
				if (ok_gend(flags.initrole, flags.initrace, i, flags.initalign))
					n++;
			}
			if (n == 0) {
				for (i = 0; i < ROLE_GENDERS; i++) {
					if (validgend(flags.initrole, flags.initrace, i))
						n++;
				}
			}

			choices = alloc(sizeof(char *) * (n + 1));
			pickmap = alloc(sizeof(int) * (n + 1));
			for (n = 0, i = 0; i < ROLE_GENDERS; i++) {
				if (ok_gend(flags.initrole, flags.initrace, i, flags.initalign)) {
					choices[n] = genders[i].adj;
					pickmap[n++] = i;
				}
			}
			choices[n] = NULL;
			/* Permit the user to pick, if there is more than one */
			if (n > 1)
				sel = curses_character_dialog(choices, "Choose one of the following genders:");
			else
				sel = 0;
			if (sel >= 0)
				sel = pickmap[sel];
			else if (sel == ROLE_NONE) { /* Quit */
				clearlocks();
				curses_bail(0);
			}
			flags.initgend = sel;
			free(choices);
			free(pickmap);
		}
		if (flags.initgend == ROLE_RANDOM) { /* Random gender */
			sel = pick_gend(flags.initrole, flags.initrace,
					flags.initalign, PICK_RANDOM);
			if (sel < 0)
				sel = randgend(flags.initrole, flags.initrace);
			flags.initgend = sel;
		}
	}

	/* Select an alignment, if necessary */
	/* force compatibility with role/race/gender */
	if (flags.initalign < 0 ||
	    !validalign(flags.initrole, flags.initrace, flags.initalign)) {
		if (flags.initalign == ROLE_RANDOM || flags.randomall) {
			flags.initalign = pick_align(flags.initrole, flags.initrace,
						     flags.initgend, PICK_RANDOM);
			if (flags.initalign < 0)
				flags.initalign = randalign(flags.initrole, flags.initrace);
		} else {
			/* Count the number of valid alignments */
			n = 0; /* number valid */
			for (i = 0; i < ROLE_ALIGNS; i++) {
				if (ok_align(flags.initrole, flags.initrace, flags.initgend, i))
					n++;
			}
			if (n == 0) {
				for (i = 0; i < ROLE_ALIGNS; i++)
					if (validalign(flags.initrole, flags.initrace, i))
						n++;
			}

			choices = alloc(sizeof(char *) * (n + 1));
			pickmap = alloc(sizeof(int) * (n + 1));
			for (n = 0, i = 0; i < ROLE_ALIGNS; i++) {
				if (ok_align(flags.initrole, flags.initrace, flags.initgend, i)) {
					choices[n] = aligns[i].adj;
					pickmap[n++] = i;
				}
			}
			choices[n] = NULL;
			/* Permit the user to pick, if there is more than one */
			if (n > 1)
				sel =
					curses_character_dialog(choices,
								"Choose one of the following alignments:");
			else
				sel = 0;
			if (sel >= 0)
				sel = pickmap[sel];
			else if (sel == ROLE_NONE) { /* Quit */
				clearlocks();
				curses_bail(0);
			}
			flags.initalign = sel;
			free(choices);
			free(pickmap);
		}
		if (flags.initalign == ROLE_RANDOM) {
			sel = pick_align(flags.initrole, flags.initrace,
					 flags.initgend, PICK_RANDOM);
			if (sel < 0)
				sel = randalign(flags.initrole, flags.initrace);
			flags.initalign = sel;
		}
	}
}

/* Prompt user for character race, role, alignment, or gender */

int curses_character_dialog(const char **choices, const char *prompt) {
	int count, count2, ret, curletter;
	char used_letters[52];
	anything identifier;
	menu_item *selected = NULL;
	winid wid = curses_get_wid(NHW_MENU);

	identifier.a_void = 0;
	curses_start_menu(wid);

	for (count = 0; choices[count]; count++) {
		curletter = tolower(choices[count][0]);
		for (count2 = 0; count2 < count; count2++) {
			if (curletter == used_letters[count2]) {
				curletter = toupper(curletter);
			}
		}

		identifier.a_int = (count + 1); /* Must be non-zero */
		curses_add_menu(wid, NO_GLYPH, &identifier, curletter, 0,
				A_NORMAL, choices[count], false);
		used_letters[count] = curletter;
	}

	/* Random Selection */
	identifier.a_int = ROLE_RANDOM;
	curses_add_menu(wid, NO_GLYPH, &identifier, '*', 0, A_NORMAL, "Random",
			false);

	/* Quit prompt */
	identifier.a_int = ROLE_NONE;
	curses_add_menu(wid, NO_GLYPH, &identifier, 'q', 0, A_NORMAL, "Quit",
			false);
	curses_end_menu(wid, prompt);
	ret = curses_select_menu(wid, PICK_ONE, &selected);
	if (ret == 1) {
		ret = (selected->item.a_int);
	} else { /* Cancelled selection */

		ret = ROLE_NONE;
	}

	if (ret > 0) {
		ret--;
	}

	free(selected);
	return ret;
}

/* Initialize and display options appropriately */

void curses_init_options() {
	set_wc_option_mod_status(WC_ALIGN_MESSAGE | WC_ALIGN_STATUS | WC_COLOR |
					 WC_HILITE_PET | WC_POPUP_DIALOG,
				 SET_IN_GAME);

	/* Add those that are */
	set_option_mod_status("classic_status", SET_IN_GAME);

#ifdef PDCURSES
	/* PDCurses for SDL, win32 and OS/2 has the ability to set the
       terminal size programatically.  If the user does not specify a
       size in the config file, we will set it to a nice big 110x32 to
       take advantage of some of the nice features of this windowport. */
	if (iflags.wc2_term_cols == 0) {
		iflags.wc2_term_cols = 110;
	}

	if (iflags.wc2_term_rows == 0) {
		iflags.wc2_term_rows = 32;
	}

	resize_term(iflags.wc2_term_rows, iflags.wc2_term_cols);
	getmaxyx(base_term, term_rows, term_cols);
#endif /* PDCURSES */
	if (!iflags.wc2_windowborders) {
		iflags.wc2_windowborders = 3; /* Set to auto if not specified */
	}

	if (!iflags.wc2_petattr) {
		iflags.wc2_petattr = A_REVERSE;
	} else { /* Pet attribute specified, so hilite_pet should be true */

		iflags.hilite_pet = true;
	}

#ifdef NCURSES_MOUSE_VERSION
	if (iflags.wc_mouse_support) {
		mousemask(BUTTON1_CLICKED, NULL);
	}
#endif
}

/* Display an ASCII splash screen if the splash_screen option is set */

void curses_display_splash_window() {
	int x_start;
	int y_start;
	curses_get_window_xy(MAP_WIN, &x_start, &y_start);
	y_start++;

	if ((term_cols < 79) || (term_rows < 23)) {
		iflags.wc_splash_screen = false; /* No room for s.s. */
	}

	curses_toggle_color_attr(stdscr, CLR_MAGENTA, A_NORMAL, ON);
	if (iflags.wc_splash_screen) {
		for (usize i = 0; i < SIZE(fancy_splash_text); i++) {
			mvaddstr(y_start++, x_start, fancy_splash_text[i]);
		}
		y_start++;
	}

	curses_toggle_color_attr(stdscr, CLR_GREEN, A_NORMAL, ON);
	mvaddstr(y_start++, x_start, COPYRIGHT_BANNER_A);
	mvaddstr(y_start++, x_start, COPYRIGHT_BANNER_B);
	mvaddstr(y_start++, x_start, COPYRIGHT_BANNER_C);
	mvaddstr(y_start++, x_start, COPYRIGHT_BANNER_D);

	refresh();
}

/* Resore colors and cursor state before exiting */

void curses_cleanup() {
	if (has_colors() && can_change_color()) {
		init_color(COLOR_YELLOW, orig_yellow.r, orig_yellow.g, orig_yellow.b);
		init_color(COLOR_WHITE, orig_white.r, orig_white.g, orig_white.b);

		if (COLORS >= 16) {
			init_color(COLOR_RED + 8, orig_hired.r, orig_hired.g, orig_hired.b);
			init_color(COLOR_GREEN + 8, orig_higreen.r, orig_higreen.g,
				   orig_higreen.b);
			init_color(COLOR_YELLOW + 8, orig_hiyellow.r,
				   orig_hiyellow.g, orig_hiyellow.b);
			init_color(COLOR_BLUE + 8, orig_hiblue.r, orig_hiblue.g,
				   orig_hiblue.b);
			init_color(COLOR_MAGENTA + 8, orig_himagenta.r,
				   orig_himagenta.g, orig_himagenta.b);
			init_color(COLOR_CYAN + 8, orig_hicyan.r, orig_hicyan.g,
				   orig_hicyan.b);
			init_color(COLOR_WHITE + 8, orig_hiwhite.r, orig_hiwhite.g,
				   orig_hiwhite.b);
			if (COLORS > 16) {
				init_color(CURSES_DARK_GRAY, orig_darkgray.r,
					   orig_darkgray.g, orig_darkgray.b);
			}
		}
	}
}
