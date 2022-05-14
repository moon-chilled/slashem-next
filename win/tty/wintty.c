/*	SCCS Id: @(#)wintty.c	3.4	2002/09/27	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Neither a standard out nor character-based control codes should be
 * part of the "tty look" windowing implementation.
 * h+ 930227
 */

#include "hack.h"
#include "dlb.h"

#include "patchlevel.h"

#ifdef TTY_GRAPHICS

#include "tcap.h"

#include "wintty.h"

#include <sys/ioctl.h>
#include <poll.h>

#if defined(BSD)
#include <signal.h>
#endif

extern char mapped_menu_cmds[]; /* from options.c */

int tty_kbhit(void);
int tty_kbpoll(int);

/* Interface definition, for windows.c */
struct window_procs tty_procs = {
	"tty",
#if 0 // TODO: mouse support
	WC_MOUSE_SUPPORT |
#endif
		WC_COLOR | WC_HILITE_PET | WC_INVERSE,
	0,
	tty_init_nhwindows,
	tty_player_selection,
	tty_askname,
	tty_get_nh_event,
	tty_exit_nhwindows,
	tty_suspend_nhwindows,
	tty_resume_nhwindows,
	tty_create_nhwindow,
	tty_clear_nhwindow,
	tty_display_nhwindow,
	tty_destroy_nhwindow,
	tty_curs,
	tty_putstr,
	tty_putnstr,
	tty_display_file,
	tty_start_menu,
	tty_add_menu,
	tty_end_menu,
	tty_select_menu,
	tty_message_menu,
	tty_update_inventory,
	tty_mark_synch,
	tty_wait_synch,
	tty_cliparound,
#ifdef POSITIONBAR
	tty_update_positionbar,
#endif
	tty_print_glyph,
	tty_raw_print,
	tty_raw_print_bold,
	tty_nhgetch,
	tty_nh_poskey,
	tty_nhbell,
	tty_doprev_message,
	tty_yn_function,
	tty_getlin,
	tty_instant_getlin,
	tty_get_ext_cmd,
	tty_number_pad,
	tty_delay_output,
#ifdef CHANGE_COLOR /* the Mac uses a palette device */
	tty_change_color,
#ifdef MAC
	tty_change_background,
	set_tty_font_name,
#endif
	tty_get_color_string,
#endif

	/* other defs that really should go away (they're tty specific) */
	tty_start_screen,
	tty_end_screen,
	genl_outrip,
	genl_preference_update,
	tty_kbhit,
};

static int maxwin = 0; /* number of windows in use */
winid BASE_WINDOW;
struct WinDesc *wins[MAXWIN];
struct DisplayDesc *ttyDisplay; /* the tty display descriptor */

extern void cmov(int, int);   /*    from       */
extern void nocmov(int, int); /*   termcap.c   */
#ifdef UNIX
static char obuf[BUFSIZ]; /* BUFSIZ is defined in stdio.h */
#endif

static char winpanicstr[] = "Bad window id %d";
char defmorestr[] = "--More--";

extern struct menucoloring *menu_colorings;

static bool clipping = false; /* clipping on? */
static int clipx = 0, clipxmax = 0;
static int clipy = 0, clipymax = 0;

bool GFlag = false;

static void getret(void);
static void erase_menu_or_text(winid, struct WinDesc *, bool);
static void free_window_info(struct WinDesc *, bool);
static void dmore(struct WinDesc *, const char *);
static void set_item_state(winid, int, tty_menu_item *);
static void set_all_on_page(winid, tty_menu_item *, tty_menu_item *);
static void unset_all_on_page(winid, tty_menu_item *, tty_menu_item *);
static void invert_all_on_page(winid, tty_menu_item *, tty_menu_item *, char);
static void invert_all(winid, tty_menu_item *, tty_menu_item *, char);
static void process_menu_window(winid, struct WinDesc *);
static void process_text_window(winid, struct WinDesc *);
static tty_menu_item *reverse(tty_menu_item *);
const char *compress_str(const char *);
static void tty_putsym(winid window, int x, int y, glyph_t ch);
static char *copy_of(const char *);
static void bail(const char *); /* __attribute__((noreturn)) */
static int tty_role_select(char *, char *);
static int tty_race_select(char *, char *);

/*
 * A string containing all the default commands -- to add to a list
 * of acceptable inputs.
 */
static const char default_menu_cmds[] = {
	MENU_FIRST_PAGE,
	MENU_LAST_PAGE,
	MENU_NEXT_PAGE,
	MENU_PREVIOUS_PAGE,
	MENU_SELECT_ALL,
	MENU_UNSELECT_ALL,
	MENU_INVERT_ALL,
	MENU_SELECT_PAGE,
	MENU_UNSELECT_PAGE,
	MENU_INVERT_PAGE,
	0 /* null terminator */
};

/* clean up and quit */
static void bail(const char *mesg) {
	clearlocks();
	tty_exit_nhwindows(mesg);
	terminate(EXIT_SUCCESS);
	/*NOTREACHED*/
}

#ifdef SIGWINCH
static void winch(int sig) {
	int oldLI = LI, oldCO = CO, i;
	struct WinDesc *cw;

	getwindowsz();
	if ((oldLI != LI || oldCO != CO) && ttyDisplay) {
		ttyDisplay->rows = LI;
		ttyDisplay->cols = CO;

		cw = wins[BASE_WINDOW];
		cw->rows = ttyDisplay->rows;
		cw->cols = ttyDisplay->cols;

		if (iflags.window_inited) {
			cw = wins[WIN_MESSAGE];
			cw->curx = cw->cury = 0;

			tty_destroy_nhwindow(WIN_STATUS);
			WIN_STATUS = tty_create_nhwindow(NHW_STATUS);

			if (u.ux) {
				if (CO < COLNO || LI < ROWNO + 3) {
					setclipped();
					tty_cliparound(u.ux, u.uy);
				} else {
					clipping = false;
					clipx = clipy = 0;
				}

				i = ttyDisplay->toplin;
				ttyDisplay->toplin = 0;
				docrt();
				bot();
				ttyDisplay->toplin = i;
				flush_screen(1);
				if (i) {
					addtopl(toplines);
				} else
					for (i = WIN_INVEN; i < MAXWIN; i++)
						if (wins[i] && wins[i]->active) {
							/* cop-out */
							addtopl(nhsdupz("Press Return to continue: "));
							break;
						}
				fflush(stdout);
				if (i < 2) flush_screen(1);
			}
		}
	}
}
#endif

/*ARGSUSED*/
void tty_init_nhwindows(int *argcp, char **argv) {
#if defined(MAC_MPW)
#pragma unused(argcp, argv)
#endif
	int wid, hgt;

	/*
	 *  Remember tty modes, to be restored on exit.
	 *
	 *  gettty() must be called before tty_startup()
	 *    due to ordering of LI/CO settings
	 *  tty_startup() must be called before initoptions()
	 *    due to ordering of graphics settings
	 */
#ifdef UNIX
	setbuf(stdout, obuf);
#endif
	gettty();

	/* to port dependant tty setup */
	tty_startup(&wid, &hgt);
	setftty(); /* calls start_screen */

	/* set up tty descriptor */
	ttyDisplay = alloc(sizeof(struct DisplayDesc));
	ttyDisplay->toplin = 0;
	ttyDisplay->rows = hgt;
	ttyDisplay->cols = wid;
	ttyDisplay->curx = ttyDisplay->cury = 0;
	ttyDisplay->inmore = ttyDisplay->inread = ttyDisplay->intr = 0;
	ttyDisplay->dismiss_more = 0;
	ttyDisplay->color = NO_COLOR;
	ttyDisplay->attrs = 0;

	/* set up the default windows */
	BASE_WINDOW = tty_create_nhwindow(NHW_BASE);
	wins[BASE_WINDOW]->active = 1;

	ttyDisplay->lastwin = WIN_ERR;

#ifdef SIGWINCH
	signal(SIGWINCH, winch);
#endif

	/* add one a space forward menu command alias */
	add_menu_cmd_alias(' ', MENU_NEXT_PAGE);

	tty_clear_nhwindow(BASE_WINDOW);

	tty_putstr(BASE_WINDOW, 0, "");
	tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_A);
	tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_B);
	tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_C);
	tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_D);
	tty_putstr(BASE_WINDOW, 0, "");
	tty_display_nhwindow(BASE_WINDOW, false);
}

void tty_player_selection(void) {
	int i, k, n;
	char pick4u = 'n';
	char pbuf[BUFSZ], plbuf[QBUFSZ];
	winid win;
	anything any;
	menu_item *selected = 0;

	/* prevent an unnecessary prompt */
	rigid_role_checks();

	/* Should we randomly pick for the player? */
	if (!flags.randomall &&
	    (flags.initrole == ROLE_NONE || flags.initrace == ROLE_NONE ||
	     flags.initgend == ROLE_NONE || flags.initalign == ROLE_NONE)) {
		int echoline;
		char *prompt = build_plselection_prompt(pbuf, BUFSZ, flags.initrole, flags.initrace, flags.initgend, flags.initalign);

		tty_putstr(BASE_WINDOW, 0, "");
		echoline = wins[BASE_WINDOW]->cury;
		tty_putstr(BASE_WINDOW, 0, prompt);
		do {
			pick4u = lowc(readchar());
			if (index(quitchars, pick4u)) pick4u = 'y';
		} while (!index(ynqchars, pick4u));
		if ((int)strlen(prompt) + 1 < CO) {
			/* Echo choice and move back down line */
			tty_putsym(BASE_WINDOW, (int)strlen(prompt) + 1, echoline, pick4u);
			tty_putstr(BASE_WINDOW, 0, "");
		} else
			/* Otherwise it's hard to tell where to echo, and things are
			 * wrapping a bit messily anyway, so (try to) make sure the next
			 * question shows up well and doesn't get wrapped at the
			 * bottom of the window.
			 */
			tty_clear_nhwindow(BASE_WINDOW);

		if (pick4u != 'y' && pick4u != 'n') {
		give_up: /* Quit */
			if (selected) free(selected);
			bail(NULL);
			/*NOTREACHED*/
			return;
		}
	}

	root_plselection_prompt(plbuf, QBUFSZ - 1,
				flags.initrole, flags.initrace, flags.initgend, flags.initalign);

	/* Select a role, if necessary */
	/* we'll try to be compatible with pre-selected race/gender/alignment,
	 * but may not succeed */
	if (flags.initrole < 0) {
		/* Process the choice */
		if (pick4u == 'y' || flags.initrole == ROLE_RANDOM || flags.randomall) {
			/* Pick a random role */
			flags.initrole = pick_role(flags.initrace, flags.initgend,
						   flags.initalign, PICK_RANDOM);
			if (flags.initrole < 0) {
				tty_putstr(BASE_WINDOW, 0, "Incompatible role!");
				flags.initrole = randrole();
			}
		} else {
			if (tty_role_select(pbuf, plbuf) < 0) goto give_up;
		}
		root_plselection_prompt(plbuf, QBUFSZ - 1,
					flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}

	/* Select a race, if necessary */
	/* force compatibility with role, try for compatibility with
	 * pre-selected gender/alignment */
	if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
		/* pre-selected race not valid */
		if (pick4u == 'y' || flags.initrace == ROLE_RANDOM || flags.randomall) {
			flags.initrace = pick_race(flags.initrole, flags.initgend,
						   flags.initalign, PICK_RANDOM);
			if (flags.initrace < 0) {
				tty_putstr(BASE_WINDOW, 0, "Incompatible race!");
				flags.initrace = randrace(flags.initrole);
			}
		} else { /* pick4u == 'n' */
			if (tty_race_select(pbuf, plbuf) < 0) goto give_up;
		}
		root_plselection_prompt(plbuf, QBUFSZ - 1,
					flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}

	/* Select a gender, if necessary */
	/* force compatibility with role/race, try for compatibility with
	 * pre-selected alignment */
	if (flags.initgend < 0 || !validgend(flags.initrole, flags.initrace,
					     flags.initgend)) {
		/* pre-selected gender not valid */
		if (pick4u == 'y' || flags.initgend == ROLE_RANDOM || flags.randomall) {
			flags.initgend = pick_gend(flags.initrole, flags.initrace,
						   flags.initalign, PICK_RANDOM);
			if (flags.initgend < 0) {
				tty_putstr(BASE_WINDOW, 0, "Incompatible gender!");
				flags.initgend = randgend(flags.initrole, flags.initrace);
			}
		} else { /* pick4u == 'n' */
			/* Count the number of valid genders */
			n = 0; /* number valid */
			k = 0; /* valid gender */
			for (i = 0; i < ROLE_GENDERS; i++) {
				if (ok_gend(flags.initrole, flags.initrace, i,
					    flags.initalign)) {
					n++;
					k = i;
				}
			}
			if (n == 0) {
				for (i = 0; i < ROLE_GENDERS; i++) {
					if (validgend(flags.initrole, flags.initrace, i)) {
						n++;
						k = i;
					}
				}
			}

			/* Permit the user to pick, if there is more than one */
			if (n > 1) {
				tty_clear_nhwindow(BASE_WINDOW);
				tty_putstr(BASE_WINDOW, 0, "Choosing Gender");
				win = create_nhwindow(NHW_MENU);
				start_menu(win);
				any.a_void = 0; /* zero out all bits */
				for (i = 0; i < ROLE_GENDERS; i++)
					if (ok_gend(flags.initrole, flags.initrace, i,
						    flags.initalign)) {
						any.a_int = i + 1;
						add_menu(win, NO_GLYPH, &any, genders[i].adj[0],
							 0, ATR_NONE, genders[i].adj, MENU_UNSELECTED);
					}
				any.a_int = pick_gend(flags.initrole, flags.initrace,
						      flags.initalign, PICK_RANDOM) +
					    1;
				if (any.a_int == 0) /* must be non-zero */
					any.a_int = randgend(flags.initrole, flags.initrace) + 1;
				add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE,
					 "Random", MENU_UNSELECTED);
				any.a_int = i + 1; /* must be non-zero */
				add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
					 "Quit", MENU_UNSELECTED);
				sprintf(pbuf, "Pick the gender of your %s", plbuf);
				end_menu(win, pbuf);
				n = select_menu(win, PICK_ONE, &selected);
				destroy_nhwindow(win);
				if (n != 1 || selected[0].item.a_int == any.a_int)
					goto give_up; /* Selected quit */

				k = selected[0].item.a_int - 1;
				free(selected), selected = 0;
			}
			flags.initgend = k;
		}
		root_plselection_prompt(plbuf, QBUFSZ - 1,
					flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}

	/* Select an alignment, if necessary */
	/* force compatibility with role/race/gender */
	if (flags.initalign < 0 || !validalign(flags.initrole, flags.initrace,
					       flags.initalign)) {
		/* pre-selected alignment not valid */
		if (pick4u == 'y' || flags.initalign == ROLE_RANDOM || flags.randomall) {
			flags.initalign = pick_align(flags.initrole, flags.initrace,
						     flags.initgend, PICK_RANDOM);
			if (flags.initalign < 0) {
				tty_putstr(BASE_WINDOW, 0, "Incompatible alignment!");
				flags.initalign = randalign(flags.initrole, flags.initrace);
			}
		} else { /* pick4u == 'n' */
			/* Count the number of valid alignments */
			n = 0; /* number valid */
			k = 0; /* valid alignment */
			for (i = 0; i < ROLE_ALIGNS; i++) {
				if (ok_align(flags.initrole, flags.initrace, flags.initgend,
					     i)) {
					n++;
					k = i;
				}
			}
			if (n == 0) {
				for (i = 0; i < ROLE_ALIGNS; i++) {
					if (validalign(flags.initrole, flags.initrace, i)) {
						n++;
						k = i;
					}
				}
			}

			/* Permit the user to pick, if there is more than one */
			if (n > 1) {
				tty_clear_nhwindow(BASE_WINDOW);
				tty_putstr(BASE_WINDOW, 0, "Choosing Alignment");
				win = create_nhwindow(NHW_MENU);
				start_menu(win);
				any.a_void = 0; /* zero out all bits */
				for (i = 0; i < ROLE_ALIGNS; i++)
					if (ok_align(flags.initrole, flags.initrace,
						     flags.initgend, i)) {
						any.a_int = i + 1;
						add_menu(win, NO_GLYPH, &any, aligns[i].adj[0],
							 0, ATR_NONE, aligns[i].adj, MENU_UNSELECTED);
					}
				any.a_int = pick_align(flags.initrole, flags.initrace,
						       flags.initgend, PICK_RANDOM) +
					    1;
				if (any.a_int == 0) /* must be non-zero */
					any.a_int = randalign(flags.initrole, flags.initrace) + 1;
				add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE,
					 "Random", MENU_UNSELECTED);
				any.a_int = i + 1; /* must be non-zero */
				add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
					 "Quit", MENU_UNSELECTED);
				sprintf(pbuf, "Pick the alignment of your %s", plbuf);
				end_menu(win, pbuf);
				n = select_menu(win, PICK_ONE, &selected);
				destroy_nhwindow(win);
				if (n != 1 || selected[0].item.a_int == any.a_int)
					goto give_up; /* Selected quit */

				k = selected[0].item.a_int - 1;
				free(selected), selected = 0;
			}
			flags.initalign = k;
		}
	}
	/* Success! */
	tty_display_nhwindow(BASE_WINDOW, false);
}

static int tty_role_select(char *pbuf, char *plbuf) {
	int i, n;
	char thisch, lastch = 0;
	char rolenamebuf[QBUFSZ];
	winid win;
	anything any;
	menu_item *selected = 0;

	tty_clear_nhwindow(BASE_WINDOW);
	tty_putstr(BASE_WINDOW, 0, "Choosing Character's Role");

	/* Prompt for a role */
	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	any.a_void = 0; /* zero out all bits */
	for (i = 0; roles[i].name.m; i++) {
		if (ok_role(i, flags.initrace, flags.initgend,
			    flags.initalign)) {
			any.a_int = i + 1; /* must be non-zero */
			thisch = lowc(roles[i].name.m[0]);
			if (thisch == lastch) thisch = highc(thisch);
			if (flags.initgend != ROLE_NONE && flags.initgend != ROLE_RANDOM) {
				if (flags.initgend == 1 && roles[i].name.f)
					strcpy(rolenamebuf, roles[i].name.f);
				else
					strcpy(rolenamebuf, roles[i].name.m);
			} else {
				if (roles[i].name.f) {
					strcpy(rolenamebuf, roles[i].name.m);
					strcat(rolenamebuf, "/");
					strcat(rolenamebuf, roles[i].name.f);
				} else
					strcpy(rolenamebuf, roles[i].name.m);
			}
			add_menu(win, NO_GLYPH, &any, thisch,
				 0, ATR_NONE, an(rolenamebuf), MENU_UNSELECTED);
			lastch = thisch;
		}
	}
	any.a_int = pick_role(flags.initrace, flags.initgend,
			      flags.initalign, PICK_RANDOM) +
		    1;
	if (any.a_int == 0) /* must be non-zero */
		any.a_int = randrole() + 1;
	add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE,
		 "Random", MENU_UNSELECTED);
	any.a_int = i + 1; /* must be non-zero */
	add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
		 "Quit", MENU_UNSELECTED);
	sprintf(pbuf, "Pick a role for your %s", plbuf);
	end_menu(win, pbuf);
	n = select_menu(win, PICK_ONE, &selected);
	destroy_nhwindow(win);

	/* Process the choice */
	if (n != 1 || selected[0].item.a_int == any.a_int) {
		free(selected), selected = 0;
		return -1; /* Selected quit */
	}

	flags.initrole = selected[0].item.a_int - 1;

	free(selected);
	selected = NULL;

	return flags.initrole;
}

static int tty_race_select(char *pbuf, char *plbuf) {
	int i, k, n;
	char thisch, lastch = 0;
	winid win;
	anything any;
	menu_item *selected = 0;

	/* Count the number of valid races */
	n = 0; /* number valid */
	k = 0; /* valid race */
	for (i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend,
			    flags.initalign)) {
			n++;
			k = i;
		}
	}
	if (n == 0) {
		for (i = 0; races[i].noun; i++) {
			if (validrace(flags.initrole, i)) {
				n++;
				k = i;
			}
		}
	}

	/* Permit the user to pick, if there is more than one */
	if (n > 1) {
		tty_clear_nhwindow(BASE_WINDOW);
		tty_putstr(BASE_WINDOW, 0, "Choosing Race");
		win = create_nhwindow(NHW_MENU);
		start_menu(win);
		any.a_void = 0; /* zero out all bits */
		for (i = 0; races[i].noun; i++)
			if (ok_race(flags.initrole, i, flags.initgend,
				    flags.initalign)) {
				any.a_int = i + 1; /* must be non-zero */
				thisch = lowc(races[i].noun[0]);
				if (thisch == lastch) thisch = highc(thisch);
				add_menu(win, NO_GLYPH, &any, thisch,
					 0, ATR_NONE, races[i].noun, MENU_UNSELECTED);
				lastch = thisch;
			}
		any.a_int = pick_race(flags.initrole, flags.initgend,
				      flags.initalign, PICK_RANDOM) + 1;
		if (any.a_int == 0) /* must be non-zero */
			any.a_int = randrace(flags.initrole) + 1;
		add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE,
			 "Random", MENU_UNSELECTED);
		any.a_int = i + 1; /* must be non-zero */
		add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
			 "Quit", MENU_UNSELECTED);
		sprintf(pbuf, "Pick the race of your %s", plbuf);
		end_menu(win, pbuf);
		n = select_menu(win, PICK_ONE, &selected);
		destroy_nhwindow(win);
		if (n != 1 || selected[0].item.a_int == any.a_int) {
			free(selected);
			return -1; /* Selected quit */
		}

		k = selected[0].item.a_int - 1;
		free(selected);
	}

	flags.initrace = k;
	return k;

#if 0 /* This version deals with more than 2 races per letter */
	int i, k, n, choicelet = 0;
	char thisch;
	char choicestr[3];
	winid win;
	anything any;
	menu_item *selected = 0;
	char pbuf[QBUFSZ];

	/* Count the number of valid races */
	n = 0;	/* number valid */
	k = 0;	/* valid race */
	for (i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend,
		                flags.initalign)) {
			n++;
			k = i;
		}
	}
	if (n == 0) {
		for (i = 0; races[i].noun; i++) {
			if (validrace(flags.initrole, i)) {
				n++;
				k = i;
			}
		}
	}

	/* Permit the user to pick, if there is more than one */
	if (n > 1) do {
			win = create_nhwindow(NHW_MENU);
			start_menu(win);
			any.a_void = 0;         /* zero out all bits */
			for (i = 0; races[i].noun; i++)
				if (ok_race(flags.initrole, i, flags.initgend,
				                flags.initalign)
				                && (!choicelet || !strncmpi(races[i].noun,
				                                choicestr, choicelet))) {

					thisch = lowc(races[i].noun[choicelet]);
					any.a_int = i+1;	/* must be non-zero */
					add_menu(win, NO_GLYPH, &any, thisch,
					         0, ATR_NONE, races[i].noun, MENU_UNSELECTED);
				}
			any.a_int = pick_race(flags.initrole, flags.initgend,
			                      flags.initalign)+1;
			if (any.a_int == 0)	/* must be non-zero */
				any.a_int = randrace(flags.initrole)+1;
			add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE,
			         "Random", MENU_UNSELECTED);
			any.a_int = i+1;	/* must be non-zero */
			add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE,
			         "Quit", MENU_UNSELECTED);
			sprintf(pbuf, "Pick the race of your %s",
			        roles[flags.initrole].name.m);
			end_menu(win, pbuf);
			n = select_menu(win, PICK_ONE, &selected);
			destroy_nhwindow(win);


			if (n != 1 || selected[0].item.a_int == any.a_int) {
				free(selected),	selected = 0;
				if (!choicelet) {
					return -1;		/* Selected quit */
				} else {
					choicelet--;
					n = 2; /* there are at least 2 */
					continue;
				}
			} else {
				k = selected[0].item.a_int - 1;
				free(selected),	selected = 0;
				choicestr[choicelet] = races[k].noun[choicelet];
				choicelet++;
			}

			/* Check whether there are at least 2 choices left */
			n = 0;
			for (i = 0; (races[i].noun && (n <= 1)); i++)
				if (ok_race(flags.initrole, i, flags.initgend,
				                flags.initalign)
				                && (!choicelet || !strncmpi(races[i].noun,
				                                choicestr, choicelet)))
					n++;
		} while (n > 1);

	flags.initrace = k;
	return k;
#endif
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting the role, etc.
 * Always called after init_nhwindows() and before display_gamewindows().
 */
void tty_askname(void) {
	static char who_are_you[] = "Who are you? ";
	int c, ct, tryct = 0;

	tty_putstr(BASE_WINDOW, 0, "");
	do {
		if (++tryct > 1) {
			if (tryct > 10) bail("Giving up after 10 tries.\n");
			tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury - 1);
			tty_putstr(BASE_WINDOW, 0, "Enter a name for your character...");
			/* erase previous prompt (in case of ESC after partial response) */
			tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury), cl_end();
		}
		tty_putstr(BASE_WINDOW, 0, who_are_you);
		tty_curs(BASE_WINDOW, (int)(sizeof who_are_you),
			 wins[BASE_WINDOW]->cury - 1);
		ct = 0;
		while ((c = nhgetch()) != '\n') {
			if (c == EOF) error("End of input\n");
			if (c == '\033') {
				ct = 0; /* continue outer loop */
				break;
			}

			/* some people get confused when their erase char is not ^H */
			if (c == '\b' || c == '\177') {
				if (ct) {
					ct--;
					putchar('\b');
					putchar(' ');
					putchar('\b');
				}
				continue;
			}
#ifdef UNIX
			if (c != '-' && c != '@')
				if (c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
#endif
			if (ct < (int)(sizeof plname) - 1) {
				putchar(c);
				plname[ct++] = c;
			}
		}
		plname[ct] = 0;
	} while (ct == 0);

	/* move to next line to simulate echo of user's <return> */
	tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury + 1);
}

void tty_get_nh_event(void) {
	return;
}

static void getret(void) {
	xputs("\n");
	if (flags.standout)
		standoutbeg();
	xputs("Hit ");
	xputs(iflags.cbreak ? "space" : "return");
	xputs(" to continue: ");
	if (flags.standout)
		standoutend();
	xwaitforspace(" ");
}

void tty_suspend_nhwindows(const char *str) {
	settty(str);		     /* calls end_screen, perhaps raw_print */
	if (!str) tty_raw_print(""); /* calls fflush(stdout) */
}

void tty_resume_nhwindows(void) {
	gettty();
	setftty(); /* calls start_screen */
	docrt();
}

void tty_exit_nhwindows(const char *str) {
	winid i;

	tty_suspend_nhwindows(str);
	/* Just forget any windows existed, since we're about to exit anyway.
	 * Disable windows to avoid calls to window routines.
	 */
	for (i = 0; i < MAXWIN; i++)
		if (wins[i] && (i != BASE_WINDOW)) {
			free_window_info(wins[i], true);
			free(wins[i]);
			wins[i] = 0;
		}
	tty_shutdown(); /* cleanup termcap/terminfo/whatever */

	iflags.window_inited = 0;
}

winid tty_create_nhwindow(int type) {
	struct WinDesc *newwin;
	int i;
	int newid;

	if (maxwin == MAXWIN)
		return WIN_ERR;

	newwin = alloc(sizeof(struct WinDesc));
	newwin->type = type;
	newwin->flags = 0;
	newwin->active = false;
	newwin->curx = newwin->cury = 0;
	newwin->morestr = 0;
	newwin->mlist = NULL;
	newwin->plist = NULL;
	newwin->npages = newwin->plist_size = newwin->nitems = newwin->how = 0;
	switch (type) {
		case NHW_BASE:
			/* base window, used for absolute movement on the screen */
			newwin->offx = newwin->offy = 0;
			newwin->rows = ttyDisplay->rows;
			newwin->cols = ttyDisplay->cols;
			newwin->maxrow = newwin->maxcol = 0;
			break;
		case NHW_MESSAGE:
			/* message window, 1 line long, very wide, top of screen */
			newwin->offx = newwin->offy = 0;
			/* sanity check */
			if (iflags.msg_history < 20)
				iflags.msg_history = 20;
			else if (iflags.msg_history > 60)
				iflags.msg_history = 60;
			newwin->maxrow = newwin->rows = iflags.msg_history;
			newwin->maxcol = newwin->cols = 0;
			break;
		case NHW_STATUS:
			/* status window, 2 lines long, full width, bottom of screen */
			/* WAC make it a variable lines long */
			newwin->offx = 0;
			newwin->offy = min((int)ttyDisplay->rows - 2, ROWNO + 1);
			/* newwin->rows = newwin->maxrow = 2; */
			newwin->rows = newwin->maxrow =
				((ttyDisplay->rows - newwin->offy) > 2) ? 3 : 2;
			newwin->cols = newwin->maxcol = min(ttyDisplay->cols, MAXCO);
			break;
		case NHW_MAP:
			/* map window, ROWNO lines long, full width, below message window */
			newwin->offx = 0;
			newwin->offy = 1;
			newwin->rows = ROWNO;
			newwin->cols = COLNO;
			newwin->maxrow = 0; /* no buffering done -- let gbuf do it */
			newwin->maxcol = 0;
			break;
		case NHW_MENU:
		case NHW_TEXT:
			/* inventory/menu window, variable length, full width, top of screen */
			/* help window, the same, different semantics for display, etc */
			newwin->offx = newwin->offy = 0;
			newwin->rows = 0;
			newwin->cols = ttyDisplay->cols;
			newwin->maxrow = newwin->maxcol = 0;
			break;
		default:
			panic("Tried to create window type %d\n", (int)type);
			return WIN_ERR;
	}

	for (newid = 0; newid < MAXWIN; newid++) {
		if (wins[newid] == 0) {
			wins[newid] = newwin;
			break;
		}
	}
	if (newid == MAXWIN) {
		panic("No window slots!");
		return WIN_ERR;
	}

	if (newwin->maxrow) {
		newwin->data = alloc(sizeof(char *) * (unsigned)newwin->maxrow);
		newwin->style_data = alloc(sizeof(nhstyle*) * newwin->maxrow);
		newwin->datlen = alloc(sizeof(short) * (unsigned)newwin->maxrow);
		if (newwin->maxcol) {
			for (i = 0; i < newwin->maxrow; i++) {
				newwin->data[i] = alloc((unsigned)newwin->maxcol);
				newwin->style_data[i] = alloc(newwin->maxcol * sizeof(nhstyle));
				newwin->datlen[i] = newwin->maxcol;
			}
		} else {
			for (i = 0; i < newwin->maxrow; i++) {
				newwin->data[i] = NULL;
				newwin->style_data[i] = NULL;
				newwin->datlen[i] = 0;
			}
		}
		if (newwin->type == NHW_MESSAGE)
			newwin->maxrow = 0;
	} else {
		newwin->data = NULL;
		newwin->datlen = NULL;
	}

	return newid;
}

static void erase_menu_or_text(winid window, struct WinDesc *cw, bool clear) {
	if (cw->offx == 0)
		if (cw->offy) {
			tty_curs(window, 1, 0);
			cl_eos();
		} else if (clear)
			clear_screen();
		else
			docrt();
	else
		docorner((int)cw->offx, cw->maxrow + 1);
}

static void free_window_info(struct WinDesc *cw, bool free_data) {
	int i;

	if (cw->data) {
		if (cw == wins[WIN_MESSAGE] && cw->rows > cw->maxrow)
			cw->maxrow = cw->rows; /* topl data */
		for (i = 0; i < cw->maxrow; i++) {
			if (cw->data[i]) {
				free(cw->data[i]);
				cw->data[i] = NULL;
			}
			if (cw->style_data[i]) {
				free(cw->style_data[i]);
				cw->style_data[i] = NULL;
			}
			if (cw->datlen) cw->datlen[i] = 0;
		}
		if (free_data) {
			free(cw->data);
			cw->data = NULL;
			if (cw->datlen) free(cw->datlen);
			free(cw->style_data);
			cw->style_data = NULL;
			cw->datlen = NULL;
			cw->rows = 0;
		}
	}
	cw->maxrow = cw->maxcol = 0;
	if (cw->mlist) {
		tty_menu_item *temp;
		while ((temp = cw->mlist) != 0) {
			cw->mlist = cw->mlist->next;
			if (temp->str) free(temp->str);
			free(temp);
		}
	}
	if (cw->plist) {
		free(cw->plist);
		cw->plist = 0;
	}
	cw->plist_size = cw->npages = cw->nitems = cw->how = 0;
	if (cw->morestr) {
		free(cw->morestr);
		cw->morestr = 0;
	}
}

void tty_clear_nhwindow(winid window) {
	struct WinDesc *cw = 0;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);
	ttyDisplay->lastwin = window;

	switch (cw->type) {
		case NHW_MESSAGE:
			if (ttyDisplay->toplin) {
				home();
				cl_end();
				if (cw->cury)
					docorner(1, cw->cury + 1);
				ttyDisplay->toplin = 0;
			}
			break;
		case NHW_STATUS:
			tty_curs(window, 1, 0);
			cl_end();
			tty_curs(window, 1, 1);
			cl_end();
			break;
		case NHW_MAP:
			// cheap -- clear the whole thing and tell nethack to redraw botl
			context.botlx = 1;
		fallthru;
		case NHW_BASE:
			clear_screen();
			break;
		case NHW_MENU:
		case NHW_TEXT:
			if (cw->active)
				erase_menu_or_text(window, cw, true);
			free_window_info(cw, false);
			break;
	}
	cw->curx = cw->cury = 0;
}

static void dmore(struct WinDesc *cw, const char *valid_responses) {
	const char *prompt = cw->morestr ? cw->morestr : defmorestr;
	int offset = (cw->type == NHW_TEXT) ? 1 : 2;

	tty_curs(BASE_WINDOW, (int)ttyDisplay->curx + offset, (int)ttyDisplay->cury);
	if (flags.standout)
		standoutbeg();
	xputs(prompt);
	ttyDisplay->curx += strlen(prompt);
	if (flags.standout)
		standoutend();

	xwaitforspace(valid_responses);
}

static void set_item_state(winid window, int lineno, tty_menu_item *item) {
	char ch = item->selected ? (item->count == -1L ? '+' : '#') : '-';
	tty_curs(window, 4, lineno);
	term_start_attr(item->attr);
	putchar(ch);
	ttyDisplay->curx++;
	term_end_attr(item->attr);
}

static void set_all_on_page(winid window, tty_menu_item *page_start, tty_menu_item *page_end) {
	tty_menu_item *curr;
	int n;

	for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
		if (curr->identifier.a_void && !curr->selected) {
			curr->selected = true;
			set_item_state(window, n, curr);
		}
}

static void unset_all_on_page(winid window, tty_menu_item *page_start, tty_menu_item *page_end) {
	tty_menu_item *curr;
	int n;

	for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
		if (curr->identifier.a_void && curr->selected) {
			curr->selected = false;
			curr->count = -1L;
			set_item_state(window, n, curr);
		}
}

static void invert_all_on_page(winid window, tty_menu_item *page_start, tty_menu_item *page_end, char acc) {
	tty_menu_item *curr;
	int n;

	for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
		if (curr->identifier.a_void && (acc == 0 || curr->gselector == acc)) {
			if (curr->selected) {
				curr->selected = false;
				curr->count = -1L;
			} else
				curr->selected = true;
			set_item_state(window, n, curr);
		}
}

/*
 * Invert all entries that match the give group accelerator (or all if
 * zero).
 */
static void invert_all(winid window, tty_menu_item *page_start, tty_menu_item *page_end, char acc) {
	tty_menu_item *curr;
	bool on_curr_page;
	struct WinDesc *cw = wins[window];

	invert_all_on_page(window, page_start, page_end, acc);

	/* invert the rest */
	for (on_curr_page = false, curr = cw->mlist; curr; curr = curr->next) {
		if (curr == page_start)
			on_curr_page = true;
		else if (curr == page_end)
			on_curr_page = false;

		if (!on_curr_page && curr->identifier.a_void && (acc == 0 || curr->gselector == acc)) {
			if (curr->selected) {
				curr->selected = false;
				curr->count = -1;
			} else
				curr->selected = true;
		}
	}
}

static bool get_menu_coloring(char *str, int *color, int *attr) {
	struct menucoloring *tmpmc;
	if (iflags.use_menu_color) {
		for (tmpmc = menu_colorings; tmpmc; tmpmc = tmpmc->next) {
			if (tre_regexec(&tmpmc->match, str, 0, NULL, 0) == 0) {
				*color = tmpmc->color;
				*attr = tmpmc->attr;
				return true;
			}
		}
	}

	return false;
}

static void process_menu_window(winid window, struct WinDesc *cw) {
	tty_menu_item *page_start, *page_end, *curr;
	long count;
	int n, curr_page, page_lines;
	bool finished, counting, reset_count;
	char *cp, *rp, resp[QBUFSZ], gacc[QBUFSZ],
		*msave, *morestr;

	curr_page = page_lines = 0;
	page_start = page_end = 0;
	msave = cw->morestr; /* save the morestr */
	cw->morestr = morestr = alloc((unsigned)QBUFSZ);
	counting = false;
	count = 0L;
	reset_count = true;
	finished = false;

	/* collect group accelerators; for PICK_NONE, they're ignored;
	   for PICK_ONE, only those which match exactly one entry will be
	   accepted; for PICK_ANY, those which match any entry are okay */
	gacc[0] = '\0';
	if (cw->how != PICK_NONE) {
		int i, gcnt[128];
#define GSELIDX(c) (c & 127) /* guard against `signed char' */

		for (i = 0; i < SIZE(gcnt); i++)
			gcnt[i] = 0;
		for (n = 0, curr = cw->mlist; curr; curr = curr->next)
			if (curr->gselector && curr->gselector != curr->selector) {
				++n;
				++gcnt[GSELIDX(curr->gselector)];
			}

		if (n > 0) /* at least one group accelerator found */
			for (rp = gacc, curr = cw->mlist; curr; curr = curr->next)
				if (curr->gselector && !index(gacc, curr->gselector) &&
				    (cw->how == PICK_ANY ||
				     gcnt[GSELIDX(curr->gselector)] == 1)) {
					*rp++ = curr->gselector;
					*rp = '\0'; /* re-terminate for index() */
				}
	}

	/* loop until finished */
	while (!finished) {
		if (reset_count) {
			counting = false;
			count = 0;
		} else
			reset_count = true;

		if (!page_start) {
			/* new page to be displayed */
			if (curr_page < 0 || (cw->npages > 0 && curr_page >= cw->npages))
				panic("bad menu screen page #%d", curr_page);

			/* clear screen */
			if (!cw->offx) { /* if not corner, do clearscreen */
				if (cw->offy) {
					tty_curs(window, 1, 0);
					cl_eos();
				} else
					clear_screen();
			}

			rp = resp;
			if (cw->npages > 0) {
				/* collect accelerators */
				page_start = cw->plist[curr_page];
				page_end = cw->plist[curr_page + 1];
				for (page_lines = 0, curr = page_start;
				     curr != page_end;
				     page_lines++, curr = curr->next) {
					int color = NO_COLOR, attr = ATR_NONE;
					bool menucolr = false;

					if (curr->selector)
						*rp++ = curr->selector;

					tty_curs(window, 1, page_lines);
					if (cw->offx) cl_end();

					putchar(' ');
					++ttyDisplay->curx;
					/*
					 * Don't use xputs() because (1) under unix it calls
					 * tputstr() which will interpret a '*' as some kind
					 * of padding information and (2) it calls xputc to
					 * actually output the character.  We're faster doing
					 * this.
					 */
					/* add selector for display */
					if (curr->selector) {
						/* because WIN32CON this must be done in
						 * a brain-dead way */
						// now WIN32CON is dead can we do it in a non-braindead way? -MC
						putchar(curr->selector);
						ttyDisplay->curx++;
						putchar(' ');
						ttyDisplay->curx++;
						/* set item state */
						if (curr->identifier.a_void != 0 && curr->selected) {
							if (curr->count == -1L)
								putchar('+'); /* all selected */
							else
								putchar('#'); /* count selected */
						} else {
							putchar('-');
						}
						ttyDisplay->curx++;
						putchar(' ');
						ttyDisplay->curx++;
					}

					if (curr->glyph != NO_GLYPH && iflags.use_menu_glyphs) {
						int glyph_color = NO_COLOR;
						glyph_t character;
						unsigned special; /* unused */
						/* map glyph to character and color */
						mapglyph(curr->glyph, &character, &glyph_color, &special, 0, 0);

						if (glyph_color != NO_COLOR) term_start_color(glyph_color);
						pututf8char(character);
						if (glyph_color != NO_COLOR) term_end_color();
						putchar(' ');
						ttyDisplay->curx += 2;
					}

					if (iflags.use_menu_color &&
					    (menucolr = get_menu_coloring(curr->str, &color, &attr))) {
						term_start_attr(attr);
						if (color != NO_COLOR) term_start_color(color);
					} else
						term_start_attr(curr->attr);
					for (n = 0, cp = curr->str; *cp && (int)++ttyDisplay->curx < (int)ttyDisplay->cols; cp++, n++)
						pututf8char(*cp);

					if (iflags.use_menu_color && menucolr) {
						if (color != NO_COLOR) term_end_color();
						term_end_attr(attr);
					} else
						term_end_attr(curr->attr);
				}
			} else {
				page_start = 0;
				page_end = 0;
				page_lines = 0;
			}
			*rp = 0;

			/* corner window - clear extra lines from last page */
			if (cw->offx) {
				for (n = page_lines + 1; n < cw->maxrow; n++) {
					tty_curs(window, 1, n);
					cl_end();
				}
			}

			/* set extra chars.. */
			strcat(resp, default_menu_cmds);
			strcat(resp, "0123456789\033\n\r"); /* counts, quit */
			strcat(resp, gacc);		    /* group accelerators */
			strcat(resp, mapped_menu_cmds);

			if (cw->npages > 1)
				sprintf(cw->morestr, "(%d of %d)",
					curr_page + 1, (int)cw->npages);
			else if (msave)
				strcpy(cw->morestr, msave);
			else
				strcpy(cw->morestr, defmorestr);

			tty_curs(window, 1, page_lines);
			cl_end();
			dmore(cw, resp);
		} else {
			/* just put the cursor back... */
			tty_curs(window, (int)strlen(cw->morestr) + 2, page_lines);
			xwaitforspace(resp);
		}

		morc = map_menu_cmd(morc);
		switch (morc) {
			case '0':
				/* special case: '0' is also the default ball class */
				if (!counting && index(gacc, morc)) goto group_accel;
			fallthru;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				count = (count * 10L) + (long)(morc - '0');
				/*
			 * It is debatable whether we should allow 0 to
			 * start a count.  There is no difference if the
			 * item is selected.  If not selected, then
			 * "0b" could mean:
			 *
			 *	count starting zero:	"zero b's"
			 *	ignore starting zero:	"select b"
			 *
			 * At present I don't know which is better.
			 */
				if (count != 0L) { /* ignore leading zeros */
					counting = true;
					reset_count = false;
				}
				break;
			case '\033': /* cancel - from counting or loop */
				if (!counting) {
					/* deselect everything */
					for (curr = cw->mlist; curr; curr = curr->next) {
						curr->selected = false;
						curr->count = -1L;
					}
					cw->flags |= WIN_CANCELLED;
					finished = true;
				}
				/* else only stop count */
				break;
			case '\0': /* finished (commit) */
			case '\n':
			case '\r':
				/* only finished if we are actually picking something */
				if (cw->how != PICK_NONE) {
					finished = true;
					break;
				} else fallthru;
			case MENU_NEXT_PAGE:
				if (cw->npages > 0 && curr_page != cw->npages - 1) {
					curr_page++;
					page_start = 0;
				} else
					finished = true; /* questionable behavior */
				break;
			case MENU_PREVIOUS_PAGE:
				if (cw->npages > 0 && curr_page != 0) {
					--curr_page;
					page_start = 0;
				}
				break;
			case MENU_FIRST_PAGE:
				if (cw->npages > 0 && curr_page != 0) {
					page_start = 0;
					curr_page = 0;
				}
				break;
			case MENU_LAST_PAGE:
				if (cw->npages > 0 && curr_page != cw->npages - 1) {
					page_start = 0;
					curr_page = cw->npages - 1;
				}
				break;
			case MENU_SELECT_PAGE:
				if (cw->how == PICK_ANY)
					set_all_on_page(window, page_start, page_end);
				break;
			case MENU_UNSELECT_PAGE:
				unset_all_on_page(window, page_start, page_end);
				break;
			case MENU_INVERT_PAGE:
				if (cw->how == PICK_ANY)
					invert_all_on_page(window, page_start, page_end, 0);
				break;
			case MENU_SELECT_ALL:
				if (cw->how == PICK_ANY) {
					set_all_on_page(window, page_start, page_end);
					/* set the rest */
					for (curr = cw->mlist; curr; curr = curr->next)
						if (curr->identifier.a_void && !curr->selected)
							curr->selected = true;
				}
				break;
			case MENU_UNSELECT_ALL:
				unset_all_on_page(window, page_start, page_end);
				/* unset the rest */
				for (curr = cw->mlist; curr; curr = curr->next)
					if (curr->identifier.a_void && curr->selected) {
						curr->selected = false;
						curr->count = -1;
					}
				break;
			case MENU_INVERT_ALL:
				if (cw->how == PICK_ANY)
					invert_all(window, page_start, page_end, 0);
				break;
			default:
				if (cw->how == PICK_NONE || !index(resp, morc)) {
					/* unacceptable input received */
					tty_nhbell();
					break;
				} else if (index(gacc, morc)) {
				group_accel:
					/* group accelerator; for the PICK_ONE case, we know that
				   it matches exactly one item in order to be in gacc[] */
					invert_all(window, page_start, page_end, morc);
					if (cw->how == PICK_ONE) finished = true;
					break;
				}
				/* find, toggle, and possibly update */
				for (n = 0, curr = page_start;
				     curr != page_end;
				     n++, curr = curr->next)
					if (morc == curr->selector) {
						if (curr->selected) {
							if (counting && count > 0) {
								curr->count = count;
								set_item_state(window, n, curr);
							} else { /* change state */
								curr->selected = false;
								curr->count = -1L;
								set_item_state(window, n, curr);
							}
						} else { /* !selected */
							if (counting && count > 0) {
								curr->count = count;
								curr->selected = true;
								set_item_state(window, n, curr);
							} else if (!counting) {
								curr->selected = true;
								set_item_state(window, n, curr);
							}
							/* do nothing counting&&count==0 */
						}

						if (cw->how == PICK_ONE) finished = true;
						break; /* from `for' loop */
					}
				break;
		}

	} /* while */
	cw->morestr = msave;
	free(morestr);
}

static void process_text_window(winid window, struct WinDesc *cw) {
	int i, n, attr;
	char *cp;

	for (n = 0, i = 0; i < cw->maxrow; i++) {
		if (!cw->offx && (n + cw->offy == ttyDisplay->rows - 1)) {
			tty_curs(window, 1, n);
			cl_end();
			dmore(cw, quitchars);
			if (morc == '\033') {
				cw->flags |= WIN_CANCELLED;
				break;
			}
			if (cw->offy) {
				tty_curs(window, 1, 0);
				cl_eos();
			} else
				clear_screen();
			n = 0;
		}
		tty_curs(window, 1, n++);
		if (cw->offx) cl_end();
		if (cw->data[i]) {
			attr = cw->data[i][0] - 1;
			if (cw->offx) {
				putchar(' ');
				++ttyDisplay->curx;
			}
			term_start_attr(attr);
			for (cp = &cw->data[i][1]; *cp && (int)++ttyDisplay->curx < (int)ttyDisplay->cols; cp++)
				pututf8char(*cp);
			term_end_attr(attr);
		}
	}
	if (i == cw->maxrow) {
		tty_curs(BASE_WINDOW, (int)cw->offx + 1,
			 (cw->type == NHW_TEXT) ? (int)ttyDisplay->rows - 1 : n);
		cl_end();
		dmore(cw, quitchars);
		if (morc == '\033')
			cw->flags |= WIN_CANCELLED;
	}
}

/*ARGSUSED*/
/* with ttys, all windows are blocking, so 'blocking' is ignored */
void tty_display_nhwindow(winid window, bool blocking) {
	struct WinDesc *cw = 0;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);
	if (cw->flags & WIN_CANCELLED)
		return;
	ttyDisplay->lastwin = window;
	ttyDisplay->rawprint = 0;

	switch (cw->type) {
		case NHW_MESSAGE:
			if (ttyDisplay->toplin == 1) {
				more();
				ttyDisplay->toplin = 1; /* more resets this */
				tty_clear_nhwindow(window);
			} else
				ttyDisplay->toplin = 0;
			cw->curx = cw->cury = 0;
			if (!cw->active)
				iflags.window_inited = true;
			break;
		case NHW_MAP:
			end_glyphout();
			if (blocking) {
				if (!ttyDisplay->toplin) ttyDisplay->toplin = 1;
				tty_display_nhwindow(WIN_MESSAGE, true);
				return;
			}
			fflush(stdout);
			break;
		case NHW_BASE:
			fflush(stdout);
			break;
		case NHW_TEXT:
			cw->maxcol = ttyDisplay->cols; /* force full-screen mode */
		fallthru;
		case NHW_MENU:
			cw->active = 1;
			/* avoid converting to uchar before calculations are finished */
			cw->offx = (uchar)(int)
				max((int)10, (int)(ttyDisplay->cols - cw->maxcol - 1));
			if (cw->type == NHW_MENU)
				cw->offy = 0;
			if (ttyDisplay->toplin == 1)
				tty_display_nhwindow(WIN_MESSAGE, true);
			if (cw->offx == 10 || cw->maxrow >= (int)ttyDisplay->rows) {
				cw->offx = 0;
				if (cw->offy) {
					tty_curs(window, 1, 0);
					cl_eos();
				} else
					clear_screen();
				ttyDisplay->toplin = 0;
			} else
				tty_clear_nhwindow(WIN_MESSAGE);

			if (cw->data || !cw->maxrow)
				process_text_window(window, cw);
			else
				process_menu_window(window, cw);
			break;
	}
	cw->active = 1;
}

void tty_dismiss_nhwindow(winid window) {
	struct WinDesc *cw = 0;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);

	switch (cw->type) {
		case NHW_MESSAGE:
			if (ttyDisplay->toplin)
				tty_display_nhwindow(WIN_MESSAGE, true);
		fallthru;
		case NHW_STATUS:
		case NHW_BASE:
		case NHW_MAP:
			/*
			 * these should only get dismissed when the game is going away
			 * or suspending
			 */
			tty_curs(BASE_WINDOW, 1, (int)ttyDisplay->rows - 1);
			cw->active = 0;
			break;
		case NHW_MENU:
		case NHW_TEXT:
			if (cw->active) {
				if (iflags.window_inited) {
					/* otherwise dismissing the text endwin after other windows
				 * are dismissed tries to redraw the map and panics.  since
				 * the whole reason for dismissing the other windows was to
				 * leave the ending window on the screen, we don't want to
				 * erase it anyway.
				 */
					erase_menu_or_text(window, cw, false);
				}
				cw->active = 0;
			}
			break;
	}
	cw->flags = 0;
}

void tty_destroy_nhwindow(winid window) {
	struct WinDesc *cw = 0;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);

	if (cw->active)
		tty_dismiss_nhwindow(window);
	if (cw->type == NHW_MESSAGE)
		iflags.window_inited = 0;
	if (cw->type == NHW_MAP)
		clear_screen();

	free_window_info(cw, true);
	free(cw);
	wins[window] = 0;
}

void tty_curs(winid window, int x, int y) {
	struct WinDesc *cw = 0;
	int cx = ttyDisplay->curx;
	int cy = ttyDisplay->cury;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);
	ttyDisplay->lastwin = window;

	cw->curx = --x; /* column 0 is never used */
	cw->cury = y;
#ifdef DEBUG
	if (x < 0 || y < 0 || y >= cw->rows || x > cw->cols) {
		const char *s = "[unknown type]";
		switch (cw->type) {
			case NHW_MESSAGE:
				s = "[topl window]";
				break;
			case NHW_STATUS:
				s = "[status window]";
				break;
			case NHW_MAP:
				s = "[map window]";
				break;
			case NHW_MENU:
				s = "[corner window]";
				break;
			case NHW_TEXT:
				s = "[text window]";
				break;
			case NHW_BASE:
				s = "[base window]";
				break;
		}
		impossible("bad curs positioning win %d %s (%d,%d)", window, s, x, y);
		return;
	}
#endif
	x += cw->offx;
	y += cw->offy;

	if (clipping && window == WIN_MAP) {
		x -= clipx;
		y -= clipy;
	}

	if (y == cy && x == cx)
		return;

	if (cw->type == NHW_MAP)
		end_glyphout();

	if (!nh_ND && (cx != x || x <= 3)) { /* Extremely primitive */
		cmov(x, y);		     /* bunker!wtm */
		return;
	}

	if ((cy -= y) < 0) cy = -cy;
	if ((cx -= x) < 0) cx = -cx;
	if (cy <= 3 && cx <= 3) {
		nocmov(x, y);
	} else if ((x <= 3 && cy <= 3) || (!nh_CM && x < cx)) {
		putchar('\r');
		ttyDisplay->curx = 0;
		nocmov(x, y);
	} else if (!nh_CM) {
		nocmov(x, y);
	} else
		cmov(x, y);

	ttyDisplay->curx = x;
	ttyDisplay->cury = y;
}

static void tty_putsym(winid window, int x, int y, glyph_t ch) {
	struct WinDesc *cw = 0;

	if (window == WIN_ERR || (cw = wins[window]) == NULL)
		panic(winpanicstr, window);

	switch (cw->type) {
		case NHW_STATUS:
		case NHW_MAP:
		case NHW_BASE:
			tty_curs(window, x, y);
			pututf8char(ch);

			ttyDisplay->curx++;
			cw->curx++;
			break;
		case NHW_MESSAGE:
		case NHW_MENU:
		case NHW_TEXT:
			impossible("Can't putsym to window type %d", cw->type);
			break;
	}
}

const char *compress_str(const char *str) {
	static char cbuf[BUFSZ];
	/* compress in case line too long */
	if ((int)strlen(str) >= CO) {
		const char *bp0 = str;
		char *bp1 = cbuf;

		do {
			if (*bp0 != ' ' || bp0[1] != ' ')
				*bp1++ = *bp0;
		} while (*bp0++);
	} else {
		return str;
	}

	return cbuf;
}

void tty_putnstr(winid window, int attr, nhstr str) {
	struct WinDesc *cw = 0;

	/* Assume there's a real problem if the window is missing --
	 * probably a panic message
	 */
	if (window == WIN_ERR || (cw = wins[window]) == NULL) {
		tty_raw_print(nhs2cstr_trunc(str));
		return;
	}

	if (!str.len || ((cw->flags & WIN_CANCELLED) && (cw->type != NHW_MESSAGE)))
		return;

	/*
	if(cw->type != NHW_MESSAGE)
		nb = compress_str(nb);
		*/
	nhstr nb = nhsdup(str);

	ttyDisplay->lastwin = window;

	switch (cw->type) {
		case NHW_MESSAGE:
			update_topl(str);
			break;

		case NHW_STATUS: {
			int stash_curx = cw->curx;
			int j = cw->curx;
			nhstyle old_style = nhstyle_default();

			for (int i = cw->curx, fx = i + 1; true; i++, fx++) {
				if (i >= nb.len) {
					if (cw->data[cw->cury][j] || context.botlx) {
						if (!nhstyle_eq(old_style, nhstyle_default())) term_end_color();

						/* last char printed may be in middle of line */
						tty_curs(WIN_STATUS, fx, cw->cury);
						cl_end();
					}
					break;
				}

				// just redraw the whole line every time
				if (true || context.botlx || (cw->data[cw->cury][j] != nb.str[i]) || !nhstyle_eq(cw->style_data[cw->cury][j], nb.style[i])) {
					if (!nhstyle_eq(nb.style[i], old_style)) {
						term_end_color();
						tty_style_start(nb.style[i]);
					}
					tty_putsym(WIN_STATUS, fx, cw->cury, nb.str[i]);
				}

				old_style = nb.style[i];
				if (cw->data[cw->cury][j]) j++;

				// EOL
				if (fx >= cw->cols) {
					// out of lines?
					if (cw->cury >= cw->rows - 1) {
						// truncate
						break;
					}

					// otherwise, wrap
					fx = 0;
					cw->cury++;
				}
			}
			if (!nhstyle_eq(old_style, nhstyle_default())) term_end_color();

			strncpy(&cw->data[cw->cury][stash_curx], nhs2cstr_trunc(nb), min(cw->cols - stash_curx - 1, nb.len));
			memcpy(&cw->style_data[cw->cury][stash_curx], nb.style, min(cw->cols - stash_curx - 1, nb.len) * sizeof(nhstyle));
			cw->data[cw->cury][cw->cols - 1] = '\0'; /* null terminate */
			/* ALI - Clear third line if present and unused */
			if (cw->cury == 1 && cw->cury < (cw->maxrow - 1)) {
				cw->data[cw->cury + 1][0] = '\0';
				tty_curs(WIN_STATUS, 1, cw->cury + 1);
				cl_end();
			}
			cw->cury = (cw->cury + 1) % 2;
			cw->curx = 0;
			break;
		}

		default:
			return tty_putstr(window, attr, nhs2cstr(str));
	}
}

void tty_putstr(winid window, int attr, const char *str) {
	struct WinDesc *cw = 0;
	char *ob;
	int i, n0;

	/* Assume there's a real problem if the window is missing --
	 * probably a panic message
	 */
	if (window == WIN_ERR || (cw = wins[window]) == NULL) {
		tty_raw_print(str);
		return;
	}

	if (str == NULL ||
	    ((cw->flags & WIN_CANCELLED) && (cw->type != NHW_MESSAGE)))
		return;
	if (cw->type != NHW_MESSAGE)
		str = compress_str(str);

	ttyDisplay->lastwin = window;

	switch (cw->type) {
		case NHW_MESSAGE:
			update_topl(nhsdupz(str));
			break;

		case NHW_STATUS:
			impossible("Tried to putstr() on a status window");
			break;
		case NHW_MAP:
			tty_curs(window, cw->curx + 1, cw->cury);
			term_start_attr(attr);
			while (*str && (int)ttyDisplay->curx < (int)ttyDisplay->cols - 1) {
				putchar(*str);
				str++;
				ttyDisplay->curx++;
			}
			cw->curx = 0;
			cw->cury++;
			term_end_attr(attr);
			break;
		case NHW_BASE:
			tty_curs(window, cw->curx + 1, cw->cury);
			term_start_attr(attr);
			while (*str) {
				if ((int)ttyDisplay->curx >= (int)ttyDisplay->cols - 1) {
					cw->curx = 0;
					cw->cury++;
					tty_curs(window, cw->curx + 1, cw->cury);
				}
				putchar(*str);
				str++;
				ttyDisplay->curx++;
			}
			cw->curx = 0;
			cw->cury++;
			term_end_attr(attr);
			break;
		case NHW_MENU:
		case NHW_TEXT:
			if (cw->type == NHW_TEXT && cw->cury == ttyDisplay->rows - 1) {
				/* not a menu, so save memory and output 1 page at a time */
				cw->maxcol = ttyDisplay->cols; /* force full-screen mode */
				tty_display_nhwindow(window, true);
				for (i = 0; i < cw->maxrow; i++) {
					if (cw->data[i]) {
						free(cw->data[i]);
						cw->data[i] = NULL;
					}
					if (cw->style_data[i]) {
						free(cw->style_data[i]);
						cw->style_data[i] = NULL;
					}
				}
				cw->maxrow = cw->cury = 0;
			}
			/* always grows one at a time, but alloc 12 at a time */
			if (cw->cury >= cw->rows) {
				char **tmp;
				nhstyle **stmp;

				cw->rows += 12;
				tmp = new(char*, (unsigned)cw->rows);
				stmp = new(nhstyle*, (unsigned)cw->rows);
				for (i = 0; i < cw->maxrow; i++) {
					tmp[i] = cw->data[i];
					stmp[i] = cw->style_data[i];
				}
				if (cw->data)
					free(cw->data);
				if (cw->style_data)
					free(cw->style_data);
				cw->data = tmp;
				cw->style_data = stmp;

				for (i = cw->maxrow; i < cw->rows; i++) {
					cw->data[i] = NULL;
					cw->style_data[i] = NULL;
				}
			}
			if (cw->data[cw->cury])
				free(cw->data[cw->cury]);
			if (cw->style_data[cw->cury])
				free(cw->style_data[cw->cury]);

			n0 = strlen(str) + 1;
			ob = cw->data[cw->cury] = new(char, n0 + 1);
			cw->style_data[cw->cury] = new(nhstyle, n0 + 1);
			*ob++ = attr + 1; /* avoid nuls, for convenience */
			strcpy(ob, str);

			if (n0 > cw->maxcol)
				cw->maxcol = n0;
			if (++cw->cury > cw->maxrow)
				cw->maxrow = cw->cury;
			if (n0 > CO) {
				/* attempt to break the line */
				for (i = CO - 1; i && str[i] != ' ' && str[i] != '\n';)
					i--;
				if (i) {
					cw->data[cw->cury - 1][++i] = '\0';
					tty_putstr(window, attr, &str[i]);
				}
			}
			break;
	}
}

void tty_display_file(const char *fname, bool complain) {
	dlb *f;
	char buf[BUFSZ];
	char *cr;

	tty_clear_nhwindow(WIN_MESSAGE);
	f = dlb_fopen(fname, "r");

	if (!f) {
		if (complain) {
			home();
			tty_mark_synch();
			tty_raw_print("");
			perror(fname);
			tty_wait_synch();
			pline("Cannot open \"%s\".", fname);
		} else if (u.ux)
			docrt();
	} else {
		winid datawin = tty_create_nhwindow(NHW_TEXT);
		bool empty = true;

		if (complain && nh_CD) {
			/* attempt to scroll text below map window if there's room */
			wins[datawin]->offy = wins[WIN_STATUS]->offy + 3;
			if ((int)wins[datawin]->offy + 12 > (int)ttyDisplay->rows)
				wins[datawin]->offy = 0;
		}
		while (dlb_fgets(buf, BUFSZ, f)) {
			if ((cr = index(buf, '\n')) != 0) *cr = 0;
			if (index(buf, '\t') != 0) tabexpand(buf);
			empty = false;
			tty_putstr(datawin, 0, buf);
			if (wins[datawin]->flags & WIN_CANCELLED)
				break;
		}
		if (!empty) tty_display_nhwindow(datawin, false);
		tty_destroy_nhwindow(datawin);
		dlb_fclose(f);
	}
}

void tty_start_menu(winid window) {
	tty_clear_nhwindow(window);
	return;
}

/*ARGSUSED*/
/*
 * Add a menu item to the beginning of the menu list.  This list is reversed
 * later.
 */
void tty_add_menu(
	winid window,		    /* window to use, must be of type NHW_MENU */
	int glyph,		    /* glyph to display with item */
	const anything *identifier, /* what to return if selected */
	char ch,		    /* keyboard accelerator (0 = pick our own) */
	char gch,		    /* group accelerator (0 = no group) */
	int attr,		    /* attribute for string (like tty_putstr()) */
	const char *str,	    /* menu string */
	bool preselected	    /* item is marked as selected */
) {
#if defined(MAC_MPW)
#pragma unused(glyph)
#endif
	struct WinDesc *cw = 0;
	tty_menu_item *item;

	if (str == NULL)
		return;

	if (window == WIN_ERR || (cw = wins[window]) == NULL || cw->type != NHW_MENU)
		panic(winpanicstr, window);

	cw->nitems++;

	item = alloc(sizeof(tty_menu_item));
	item->identifier = *identifier;
	item->count = -1L;
	item->selected = preselected;
	item->selector = ch;
	item->gselector = gch;
	item->attr = attr;
	item->str = copy_of(str);
	item->glyph = glyph;

	item->next = cw->mlist;
	cw->mlist = item;
}

/* Invert the given list, can handle NULL as an input. */
static tty_menu_item *reverse(tty_menu_item *curr) {
	tty_menu_item *next, *head = 0;

	while (curr) {
		next = curr->next;
		curr->next = head;
		head = curr;
		curr = next;
	}
	return head;
}

/*
 * End a menu in this window, window must a type NHW_MENU.  This routine
 * processes the string list.  We calculate the # of pages, then assign
 * keyboard accelerators as needed.  Finally we decide on the width and
 * height of the window.
 */
void tty_end_menu(winid window, const char *prompt) {
	struct WinDesc *cw = 0;
	tty_menu_item *curr;
	short len;
	int lmax, n;
	char menu_ch;

	if (window == WIN_ERR || (cw = wins[window]) == NULL ||
	    cw->type != NHW_MENU)
		panic(winpanicstr, window);

	/* Reverse the list so that items are in correct order. */
	cw->mlist = reverse(cw->mlist);

	/* Put the promt at the beginning of the menu. */
	if (prompt) {
		anything any;

		any.a_void = 0; /* not selectable */
		tty_add_menu(window, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
		tty_add_menu(window, NO_GLYPH, &any, 0, 0, ATR_NONE, prompt, MENU_UNSELECTED);
	}

	lmax = min(52, (int)ttyDisplay->rows - 1);     /* # lines per page */
	cw->npages = (cw->nitems + (lmax - 1)) / lmax; /* # of pages */

	/* make sure page list is large enough */
	if (cw->plist_size < cw->npages + 1 /*need 1 slot beyond last*/) {
		if (cw->plist) free(cw->plist);
		cw->plist_size = cw->npages + 1;
		cw->plist = (tty_menu_item **)
			alloc(cw->plist_size * sizeof(tty_menu_item *));
	}

	cw->cols = 0;  /* cols is set when the win is initialized... (why?) */
	menu_ch = '?'; /* lint suppression */
	for (n = 0, curr = cw->mlist; curr; n++, curr = curr->next) {
		/* set page boundaries and character accelerators */
		if ((n % lmax) == 0) {
			menu_ch = 'a';
			cw->plist[n / lmax] = curr;
		}
		if (curr->identifier.a_void && !curr->selector) {
			curr->selector = menu_ch;
			if (menu_ch++ == 'z') menu_ch = 'A';
		}

		/* cut off any lines that are too long */
		len = strlen(curr->str) + 2; /* extra space at beg & end */

		if (curr->selector) {
			/* extra space for keyboard accelerator */
			len += 4;
			if (curr->glyph != NO_GLYPH && iflags.use_menu_glyphs) {
				/* extra space for glyph */
				len += 2;
			}
		}

		if (len > (int)ttyDisplay->cols) {
			// reduce the string by the amount len exceeds cols
			curr->str[strlen(curr->str) - (len - ttyDisplay->cols)] = 0;
			len = ttyDisplay->cols;
		}
		if (len > cw->cols) cw->cols = len;
	}
	cw->plist[cw->npages] = 0; /* plist terminator */

	/*
	 * If greater than 1 page, morestr is "(x of y) " otherwise, "(end) "
	 */
	if (cw->npages > 1) {
		char buf[QBUFSZ];
		/* produce the largest demo string */
		sprintf(buf, "(%d of %d) ", cw->npages, cw->npages);
		len = strlen(buf);
		cw->morestr = copy_of("");
	} else {
		cw->morestr = copy_of("(end) ");
		len = strlen(cw->morestr);
	}

	if (len > (int)ttyDisplay->cols) {
		/* truncate the prompt if its too long for the screen */
		if (cw->npages <= 1) /* only str in single page case */
			cw->morestr[ttyDisplay->cols] = 0;
		len = ttyDisplay->cols;
	}
	if (len > cw->cols) cw->cols = len;

	cw->maxcol = cw->cols;

	/*
	 * The number of lines in the first page plus the morestr will be the
	 * maximum size of the window.
	 */
	if (cw->npages > 1)
		cw->maxrow = cw->rows = lmax + 1;
	else
		cw->maxrow = cw->rows = cw->nitems + 1;
}

int tty_select_menu(winid window, int how, menu_item **menu_list) {
	struct WinDesc *cw = 0;
	tty_menu_item *curr;
	menu_item *mi;
	int n, cancelled;

	if (window == WIN_ERR || (cw = wins[window]) == NULL || cw->type != NHW_MENU)
		panic(winpanicstr, window);

	*menu_list = NULL;
	cw->how = (short)how;
	morc = 0;
	tty_display_nhwindow(window, true);
	cancelled = !!(cw->flags & WIN_CANCELLED);
	tty_dismiss_nhwindow(window); /* does not destroy window data */

	if (cancelled) {
		n = -1;
	} else {
		for (n = 0, curr = cw->mlist; curr; curr = curr->next)
			if (curr->selected) n++;
	}

	if (n > 0) {
		*menu_list = alloc(n * sizeof(menu_item));
		for (mi = *menu_list, curr = cw->mlist; curr; curr = curr->next)
			if (curr->selected) {
				mi->item = curr->identifier;
				mi->count = curr->count;
				mi++;
			}
	}

	return n;
}

/* special hack for treating top line --More-- as a one item menu */
char tty_message_menu(char let, int how, const char *mesg) {
	/* "menu" without selection; use ordinary pline, no more() */
	if (how == PICK_NONE) {
		pline("%s", mesg);
		return 0;
	}

	ttyDisplay->dismiss_more = let;
	morc = 0;
	/* barebones pline(); since we're only supposed to be called after
	   response to a prompt, we'll assume that the display is up to date */
	tty_putstr(WIN_MESSAGE, 0, mesg);
	/* if `mesg' didn't wrap (triggering --More--), force --More-- now */
	if (ttyDisplay->toplin == 1) {
		more();
		ttyDisplay->toplin = 1; /* more resets this */
		tty_clear_nhwindow(WIN_MESSAGE);
	}
	/* normally <ESC> means skip further messages, but in this case
	   it means cancel the current prompt; any other messages should
	   continue to be output normally */
	wins[WIN_MESSAGE]->flags &= ~WIN_CANCELLED;
	ttyDisplay->dismiss_more = 0;

	return ((how == PICK_ONE && morc == let) || morc == '\033') ? morc : '\0';
}

void tty_update_inventory(void) {
	return;
}

void tty_mark_synch(void) {
	fflush(stdout);
}

void tty_wait_synch(void) {
	/* we just need to make sure all windows are synch'd */
	if (!ttyDisplay || ttyDisplay->rawprint) {
		getret();
		if (ttyDisplay) ttyDisplay->rawprint = 0;
	} else {
		tty_display_nhwindow(WIN_MAP, false);
		if (ttyDisplay->inmore) {
			addtopl(nhsdupz("--More--"));
			fflush(stdout);
		} else if (ttyDisplay->inread > program_state.gameover) {
			/* this can only happen if we were reading and got interrupted */
			ttyDisplay->toplin = 3;
			/* do this twice; 1st time gets the Quit? message again */
			tty_doprev_message();
			tty_doprev_message();
			ttyDisplay->intr++;
			fflush(stdout);
		}
	}
}

void docorner(int xmin, int ymax) {
	int y;
	struct WinDesc *cw = wins[WIN_MAP];

#ifdef SIGWINCH
	if (ymax > LI) ymax = LI; /* can happen if window gets smaller */
#endif
	for (y = 0; y < ymax; y++) {
		tty_curs(BASE_WINDOW, xmin, y); /* move cursor */
		cl_end();			/* clear to end of line */
		if (y < (int)cw->offy || y + clipy > ROWNO)
			continue; /* only refresh board */
		row_refresh(xmin + clipx - (int)cw->offx, COLNO - 1, y + clipy - (int)cw->offy);
	}

	end_glyphout();
	if (ymax >= (int)wins[WIN_STATUS]->offy) {
		/* we have wrecked the bottom line */
		context.botlx = 1;
		bot();
	}
}

void end_glyphout(void) {
	if (GFlag) {
		GFlag = false;
		graph_off();
	}

	if (ttyDisplay->color != NO_COLOR) {
		term_end_color();
		ttyDisplay->color = NO_COLOR;
	}
}

#ifndef WIN32
void g_putch(int in_ch) {
	char ch = (char)in_ch;

	if (ch & 0x80) {
		if (!GFlag) {
			graph_on();
			GFlag = true;
		}
		putchar((ch ^ 0x80)); /* Strip 8th bit */
	} else {
		if (GFlag) {
			graph_off();
			GFlag = false;
		}
		putchar(ch);
	}
}
#endif /* !WIN32 */

void setclipped(void) {
	clipping = true;
	clipx = clipy = 0;
	clipxmax = CO;
	clipymax = LI - 3;
}

void tty_cliparound(int x, int y) {
	int oldx = clipx, oldy = clipy;

	if (!clipping) return;
	if (x < clipx + 5) {
		clipx = max(0, x - 20);
		clipxmax = clipx + CO;
	} else if (x > clipxmax - 5) {
		clipxmax = min(COLNO, clipxmax + 20);
		clipx = clipxmax - CO;
	}
	if (y < clipy + 2) {
		clipy = max(0, y - (clipymax - clipy) / 2);
		clipymax = clipy + (LI - 3);
	} else if (y > clipymax - 2) {
		clipymax = min(ROWNO, clipymax + (clipymax - clipy) / 2);
		clipy = clipymax - (LI - 3);
	}
	if (clipx != oldx || clipy != oldy) {
		if (on_level(&u.uz0, &u.uz) && !restoring)
			doredraw();
	}
}

/*
 *  tty_print_glyph
 *
 *  Print the glyph to the output device.  Don't flush the output device.
 *
 *  Since this is only called from show_glyph(), it is assumed that the
 *  position and glyph are always correct (checked there)!
 */

void tty_print_glyph(winid window, xchar x, xchar y, int glyph) {
	glyph_t ch;
	bool reverse_on = false;
	int color;
	unsigned special;

	if (clipping) {
		if (x <= clipx || y < clipy || x >= clipxmax || y >= clipymax)
			return;
	}

	/* map glyph to character and color */
	mapglyph(glyph, &ch, &color, &special, x, y);

	/* Move the cursor. */
	tty_curs(window, x, y);

	if (color != ttyDisplay->color) {
		if (ttyDisplay->color != NO_COLOR)
			term_end_color();
		ttyDisplay->color = color;
		if (color != NO_COLOR)
			term_start_color(color);
	}

	/* must be after color check; term_end_color may turn off inverse too */
	if (((special & MG_PET) && iflags.hilite_pet) || ((special & MG_DETECT) && iflags.use_inverse)) {
		term_start_attr(ATR_INVERSE);
		reverse_on = true;
	}

	if (!reverse_on && (special & (MG_STAIRS | MG_OBJPILE))) {
		if ((special & MG_STAIRS) && iflags.hilite_hidden_stairs && iflags.use_color)
			term_start_bgcolor(CLR_RED);
		else if ((special & MG_OBJPILE) && iflags.hilite_obj_piles && iflags.use_color)
			term_start_bgcolor(CLR_BLUE);
	}

	pututf8char(ch);

	if (reverse_on) {
		term_end_attr(ATR_INVERSE);
		/* turn off color as well, ATR_INVERSE may have done this already */
		if (ttyDisplay->color != NO_COLOR) {
			term_end_color();
			ttyDisplay->color = NO_COLOR;
		}
	}

	if (!reverse_on && (special & (MG_STAIRS | MG_OBJPILE))) {
		term_end_color();
		ttyDisplay->color = NO_COLOR;
	}

	wins[window]->curx++; /* one character over */
	ttyDisplay->curx++;   /* the real cursor moved too */
}

void tty_raw_print(const char *str) {
	if (ttyDisplay) ttyDisplay->rawprint++;
	puts(str);
	fflush(stdout);
}

void tty_raw_print_bold(const char *str) {
	if (ttyDisplay) ttyDisplay->rawprint++;
	term_start_raw_bold();
	fputs(str, stdout);
	term_end_raw_bold();
	puts("");
	fflush(stdout);
}

static int input_buffer[8] = {-1,-1,-1};

static void unget(int c) {
	for(int i=0;i<SIZE(input_buffer);i++)if(input_buffer[i]==-1){input_buffer[i]=c;return;}
}

int tgetch(void) {
	if (input_buffer[0] != -1) {
		int r = input_buffer[0];
		memmove(input_buffer, input_buffer+1, (SIZE(input_buffer)-1)*sizeof(*input_buffer));
		input_buffer[SIZE(input_buffer)-1] = -1;
		return r;
	}
	while(1) {
		unsigned char ret;
		if(1==read(fileno(stdin),&ret,1))return ret;
	}
}

int tty_nhgetch(void) {
	int i;

	fflush(stdout);
	/* Note: if raw_print() and wait_synch() get called to report terminal
	 * initialization problems, then wins[] and ttyDisplay might not be
	 * available yet.  Such problems will probably be fatal before we get
	 * here, but validate those pointers just in case...
	 */
	if (WIN_MESSAGE != WIN_ERR && wins[WIN_MESSAGE])
		wins[WIN_MESSAGE]->flags &= ~WIN_STOP;

	if (iflags.debug_fuzzer) {
		i = randomkey();
	} else {
		i = tgetch();
	}
	if (!i) i = '\033'; /* map NUL to ESC since nethack doesn't expect NUL */
	if (ttyDisplay && ttyDisplay->toplin == 1)
		ttyDisplay->toplin = 2;
	return i;
}

int tty_getkey(void) {
	if (iflags.debug_fuzzer) return tty_nhgetch();

	int c = tty_nhgetch();
	if (c == '\033' && tty_kbpoll(10)) {
		int n = tty_nhgetch();
		if (n == '[') {
		       	if (!tty_kbpoll(10)) return META_FLG | '[';
			switch ((n=tty_nhgetch())) {
				case 'A': return K_CURSORUP;
				case 'B': return K_CURSORDOWN;
				case 'C': return K_CURSORRIGHT;
				case 'D': return K_CURSORLEFT;
				case '3':
					if (!tty_kbpoll(10)) { unget(n); return META_FLG | '['; }
					if ((n=tty_nhgetch())=='~')return K_DELETE;
					unget('3');
					unget('~');
					return META_FLG | '[';
				default:
					unget(n);
					return META_FLG | '[';
			}
		} else {
			unget(n);
			c = tty_getkey();
			if (c < 0) c &= ~META_FLG;
			else c |= META_FLG;
		}
	}
	if (c < 0x20 && c != '\r' && c != '\n' && c != '\033') {
		return CTRL_FLG | 0x60 | c;
	}
	if (c == '\177') return '\b';
	return c;
}

/*
 * return a key, or 0, in which case a mouse button was pressed
 * mouse events should be returned as character postitions in the map window.
 * Since normal tty's don't have mice, just return a key.
 */
/*ARGSUSED*/
int tty_nh_poskey(int *x, int *y, int *mod) {
	return nhgetch();
}

// Thanks to https://stackoverflow.com/questions/29335758/using-kbhit-and-getch-on-linux and https://web.archive.org/web/20170713065718/www.flipcode.com/archives/_kbhit_for_Linux.shtml
int tty_kbhit(void) {
	int byteswaiting;
	ioctl(fileno(stdin), FIONREAD, &byteswaiting);
	return byteswaiting;
}

int tty_kbpoll(int timeout) {
	return poll(&(struct pollfd){.fd = fileno(stdin), .events = POLLIN}, 1, timeout) == 1;
}

void win_tty_init(void) {
	return;
}

#ifdef POSITIONBAR
void tty_update_positionbar(char *posbar) {
}
#endif

/*
 * Allocate a copy of the given string.  If null, return a string of
 * zero length.
 *
 * This is an exact duplicate of copy_of() in X11/winmenu.c.
 */
static char *copy_of(const char *s) {
	if (!s) s = "";
	return strcpy(alloc(strlen(s) + 1), s);
}

void tty_style_start(nhstyle s) {
	if (s.fg != NO_COLOR) term_start_color(s.fg);
	if (s.bg != NO_COLOR) term_start_bgcolor(s.bg);
	term_start_attrs(s.attr);
}

#endif /* TTY_GRAPHICS */

/*wintty.c*/
