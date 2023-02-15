/*	SCCS Id: @(#)pickup.c	3.4	2003/07/27	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	Contains code for picking objects up, and container use.
 */

#include "hack.h"

static void simple_look(struct obj *, boolean);
static boolean query_classes(char *, boolean *, boolean *, const char *, struct obj *, boolean, int *);
static void check_here(boolean);
static bool n_or_more(struct obj *);
static bool all_but_uchain(struct obj *);
#if 0 /* not used */
static boolean allow_cat_no_uchain(struct obj *);
#endif
static int autopick(struct obj *, int, menu_item **);
static int count_categories(struct obj *, int);
static long carry_count(struct obj *, struct obj *, long, boolean, int *, int *);
static int lift_object(struct obj *, struct obj *, long *, boolean);
static int in_container(struct obj *);
static int ck_bag(struct obj *);
static int out_container(struct obj *);
static long mbag_item_gone(int, struct obj *);
static void observe_quantum_cat(struct obj *);
static int menu_loot(int, struct obj *, boolean);
static int in_or_out_menu(const char *, struct obj *, boolean, boolean);
static int container_at(int x, int y, bool countem);
static bool able_to_loot(int x, int y, bool looting);
static bool mon_beside(int, int);
void tipcontainer(struct obj *box);

/* define for query_objlist() and autopickup() */
#define FOLLOW(curr, flags) \
	(((flags)&BY_NEXTHERE) ? (curr)->nexthere : (curr)->nobj)

#define CEILDIV(x, y) (((x) + (y)-1) / (y)) /* ceil(x/y) */
/*
 *  How much the weight of the given container will change when the given
 *  object is removed from it.  This calculation must match the one used
 *  by weight() in mkobj.c.
 */
#define DELTA_CWT(cont, obj)                                         \
	((cont)->cursed ? (obj)->owt * ((cont)->oartifact ? 4 : 2) : \
			  CEILDIV((obj)->owt, ((cont)->oartifact ? 3 : 2) * ((cont)->blessed ? 2 : 1)))
#define GOLD_WT(n) (((n) + 50L) / 100L)
/* if you can figure this out, give yourself a hearty pat on the back... */
#define GOLD_CAPACITY(w, n) (((w) * -100L) - ((n) + 50L) - 1L)

/* A variable set in use_container(), to be used by the callback routines  */
/* in_container() and out_container() from askchain() and use_container(). */
/* Also used by memu_loot() and container_gone().			   */
static struct obj *current_container;
#define Icebox (current_container->otyp == ICE_BOX)

static const char moderateloadmsg[] = "You have a little trouble lifting";
static const char nearloadmsg[] = "You have much trouble lifting";
static const char overloadmsg[] = "You have extreme difficulty lifting";

/* BUG: this lets you look at cockatrice corpses while blind without
   touching them */
/* much simpler version of the look-here code; used by query_classes() */
// otmp => list of objects
// here => flag for type of obj list linkage
static void simple_look(struct obj *otmp, boolean here) {
	/* Neither of the first two cases is expected to happen, since
	 * we're only called after multiple classes of objects have been
	 * detected, hence multiple objects must be present.
	 */
	if (!otmp) {
		impossible("simple_look(null)");
	} else if (!(here ? otmp->nexthere : otmp->nobj)) {
		pline("%s", doname(otmp));
	} else {
		winid tmpwin = create_nhwindow(NHW_MENU);
		putstr(tmpwin, 0, "");
		do {
			putstr(tmpwin, 0, doname(otmp));
			otmp = here ? otmp->nexthere : otmp->nobj;
		} while (otmp);
		display_nhwindow(tmpwin, true);
		destroy_nhwindow(tmpwin);
	}
}

int collect_obj_classes(char ilets[], struct obj *otmp, boolean here, bool (*filter)(struct obj *), int *itemcount) {
	int iletct = 0;
	char c;

	*itemcount = 0;
	ilets[iletct] = '\0'; /* terminate ilets so that index() will work */
	while (otmp) {
		c = def_oc_syms[(int)otmp->oclass];
		if (!index(ilets, c) && (!filter || (*filter)(otmp)))
			ilets[iletct++] = c, ilets[iletct] = '\0';
		*itemcount += 1;
		otmp = here ? otmp->nexthere : otmp->nobj;
	}

	return iletct;
}

/*
 * Suppose some '?' and '!' objects are present, but '/' objects aren't:
 *	"a" picks all items without further prompting;
 *	"A" steps through all items, asking one by one;
 *	"?" steps through '?' items, asking, and ignores '!' ones;
 *	"/" becomes 'A', since no '/' present;
 *	"?a" or "a?" picks all '?' without further prompting;
 *	"/a" or "a/" becomes 'A' since there aren't any '/'
 *	    (bug fix:  3.1.0 thru 3.1.3 treated it as "a");
 *	"?/a" or "a?/" or "/a?",&c picks all '?' even though no '/'
 *	    (ie, treated as if it had just been "?a").
 */
static boolean query_classes(char oclasses[], boolean *one_at_a_time, boolean *everything, const char *action, struct obj *objs, boolean here, int *menu_on_demand) {
	char ilets[20], inbuf[BUFSZ];
	int iletct, oclassct;
	boolean not_everything;
	char qbuf[QBUFSZ];
	boolean m_seen;
	int itemcount;

	oclasses[oclassct = 0] = '\0';
	*one_at_a_time = *everything = m_seen = false;
	iletct = collect_obj_classes(ilets, objs, here, NULL, &itemcount);
	if (iletct == 0) {
		return false;
	} else if (iletct == 1) {
		oclasses[0] = def_char_to_objclass(ilets[0]);
		oclasses[1] = '\0';
		if (itemcount && menu_on_demand) {
			ilets[iletct++] = 'm';
			*menu_on_demand = 0;
			ilets[iletct] = '\0';
		}
	} else { /* more than one choice available */
		const char *where = 0;
		char sym, oc_of_sym, *p;
		/* additional choices */
		ilets[iletct++] = ' ';
		ilets[iletct++] = 'a';
		ilets[iletct++] = 'A';
		ilets[iletct++] = (objs == invent ? 'i' : ':');
		if (menu_on_demand) {
			ilets[iletct++] = 'm';
			*menu_on_demand = 0;
		}
		ilets[iletct] = '\0';
ask_again:
		oclasses[oclassct = 0] = '\0';
		*one_at_a_time = *everything = false;
		not_everything = false;
		sprintf(qbuf, "What kinds of thing do you want to %s? [%s]",
			action, ilets);
		getlin(qbuf, inbuf);
		if (*inbuf == '\033') return false;

		for (p = inbuf; (sym = *p++);) {
			/* new A function (selective all) added by GAN 01/09/87 */
			if (sym == ' ')
				continue;
			else if (sym == 'A')
				*one_at_a_time = true;
			else if (sym == 'a')
				*everything = true;
			else if (sym == ':') {
				simple_look(objs, here); /* dumb if objs==invent */
				/* if we just scanned the contents of a container
				 * then mark it as having known contents */
				if (objs->where == OBJ_CONTAINED)
					objs->ocontainer->cknown = 1;

				goto ask_again;
			} else if (sym == 'i') {
				display_inventory(NULL, true);
				goto ask_again;
			} else if (sym == 'm') {
				m_seen = true;
			} else {
				oc_of_sym = def_char_to_objclass(sym);
				if (index(ilets, sym)) {
					add_valid_menu_class(oc_of_sym);
					oclasses[oclassct++] = oc_of_sym;
					oclasses[oclassct] = '\0';
				} else {
					if (!where)
						where = !strcmp(action, "pick up") ? "here" :
										     !strcmp(action, "take out") ?
										     "inside" :
										     "";
					if (*where)
						pline("There are no %c's %s.", sym, where);
					else
						pline("You have no %c's.", sym);
					not_everything = true;
				}
			}
		}
		if (m_seen && menu_on_demand) {
			*menu_on_demand = (*everything || !oclassct) ? -2 : -3;
			return false;
		}
		if (!oclassct && (!*everything || not_everything)) {
			/* didn't pick anything,
			   or tried to pick something that's not present */
			*one_at_a_time = true; /* force 'A' */
			*everything = false;   /* inhibit 'a' */
		}
	}
	return true;
}

/* look at the objects at our location, unless there are too many of them */
static void check_here(boolean picked_some) {
	struct obj *obj;
	int ct = 0;

	/* count the objects here */
	for (obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere) {
		if (obj != uchain)
			ct++;
	}

	/* If there are objects here, take a look. */
	if (ct) {
		if (context.run) nomul(0);
		flush_screen(1);
		look_here(ct, picked_some);
	} else {
		sense_engr_at(u.ux, u.uy, false);
	}
}

/* Value set by query_objlist() for n_or_more(). */
static long val_for_n_or_more;

/* query_objlist callback: return true if obj's count is >= reference value */
static bool n_or_more(struct obj *obj) {
	if (obj == uchain) return false;
	return obj->quan >= val_for_n_or_more;
}

/* List of valid menu classes for query_objlist() and allow_category callback */
static char valid_menu_classes[MAXOCLASSES + 2];

void add_valid_menu_class(int c) {
	static int vmc_count = 0;

	if (c == 0) /* reset */
		vmc_count = 0;
	else
		valid_menu_classes[vmc_count++] = (char)c;
	valid_menu_classes[vmc_count] = '\0';
}

/* query_objlist callback: return true if not uchain */
static bool all_but_uchain(struct obj *obj) {
	return obj != uchain;
}

/* query_objlist callback: return true */
/*ARGSUSED*/
bool allow_all(struct obj *obj) {
	return true;
}

bool allow_category(struct obj *obj) {
	if (Role_if(PM_PRIEST)) obj->bknown = true;
	if (((index(valid_menu_classes, 'u') != NULL) && obj->unpaid) ||
	    (index(valid_menu_classes, obj->oclass) != NULL))
		return true;
	else if (((index(valid_menu_classes, 'U') != NULL) &&
		  (obj->oclass != COIN_CLASS && obj->bknown && !obj->blessed && !obj->cursed)))
		return true;
	else if (((index(valid_menu_classes, 'B') != NULL) &&
		  (obj->oclass != COIN_CLASS && obj->bknown && obj->blessed)))
		return true;
	else if (((index(valid_menu_classes, 'C') != NULL) &&
		  (obj->oclass != COIN_CLASS && obj->bknown && obj->cursed)))
		return true;
	else if (((index(valid_menu_classes, 'X') != NULL) &&
		  (obj->oclass != COIN_CLASS && !obj->bknown)))
		return true;
	else
		return false;
}

#if 0 /* not used */
/* query_objlist callback: return true if valid category (class), no uchain */
static boolean allow_cat_no_uchain(struct obj *obj) {
	if ((obj != uchain) &&
	                (((index(valid_menu_classes,'u') != NULL) && obj->unpaid) ||
	                 (index(valid_menu_classes, obj->oclass) != NULL)))
		return true;
	else
		return false;
}
#endif

/* query_objlist callback: return true if valid class and worn */
bool is_worn_by_type(struct obj *otmp) {
	return (otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL | W_WEP | W_SWAPWEP | W_QUIVER)) && (index(valid_menu_classes, otmp->oclass) != NULL);
}

/*
 * Have the hero pick things from the ground
 * or a monster's inventory if swallowed.
 *
 * Arg what:
 *	>0  autopickup
 *	=0  interactive
 *	<0  pickup count of something
 *
 * Returns 1 if tried to pick something up, whether
 * or not it succeeded.
 */

/* what should be a long */
int pickup(
	int what) {
	int i, n, res, count, n_tried = 0, n_picked = 0;
	menu_item *pick_list = NULL;
	boolean autopickup = what > 0;
	struct obj *objchain;
	int traverse_how;

	if (what < 0) /* pick N of something */
		count = -what;
	else /* pick anything */
		count = 0;

	if (!u.uswallow) {
		struct trap *ttmp = t_at(u.ux, u.uy);
		/* no auto-pick if no-pick move, nothing there, or in a pool */
		if (autopickup && (context.nopick || !OBJ_AT(u.ux, u.uy) ||
				   (is_pool(u.ux, u.uy) && !Underwater) || is_lava(u.ux, u.uy))) {
			sense_engr_at(u.ux, u.uy, false);
			return 0;
		}

		/* no pickup if levitating & not on air or water level */
		if (!can_reach_floor()) {
			if ((multi && !context.run) || (autopickup && !flags.pickup))
				sense_engr_at(u.ux, u.uy, false);
			return 0;
		}
		if (ttmp && uteetering_at_seen_pit()) {
			/* Allow pickup from holes and trap doors that you escaped
			 * from because that stuff is teetering on the edge just
			 * like you, but not pits, because there is an elevation
			 * discrepancy with stuff in pits.
			 */
			sense_engr_at(u.ux, u.uy, false);
			return 0;
		}
		/* multi && !context.run means they are in the middle of some other
		 * action, or possibly paralyzed, sleeping, etc.... and they just
		 * teleported onto the object.  They shouldn't pick it up.
		 */
		if ((multi && !context.run) || (autopickup && !flags.pickup)) {
			check_here(false);
			return 0;
		}
		if (notake(youmonst.data)) {
			if (!autopickup)
				pline("You are physically incapable of picking anything up.");
			else
				check_here(false);
			return 0;
		}

		/* if there's anything here, stop running */
		if (OBJ_AT(u.ux, u.uy) && context.run && context.run != 8 && !context.nopick) nomul(0);
	}

	add_valid_menu_class(0); /* reset */
	if (!u.uswallow) {
		objchain = level.objects[u.ux][u.uy];
		traverse_how = BY_NEXTHERE;
	} else {
		objchain = u.ustuck->minvent;
		traverse_how = 0; /* nobj */
	}
	/*
	 * Start the actual pickup process.  This is split into two main
	 * sections, the newer menu and the older "traditional" methods.
	 * Automatic pickup has been split into its own menu-style routine
	 * to make things less confusing.
	 */
	if (autopickup) {
		n = autopick(objchain, traverse_how, &pick_list);
		goto menu_pickup;
	}

	if (flags.menu_style != MENU_TRADITIONAL || iflags.menu_requested) {
		/* use menus exclusively */
		if (count) { /* looking for N of something */
			char buf[QBUFSZ];
			sprintf(buf, "Pick %d of what?", count);
			val_for_n_or_more = count; /* set up callback selector */
			n = query_objlist(buf, objchain,
					  traverse_how | AUTOSELECT_SINGLE | INVORDER_SORT,
					  &pick_list, PICK_ONE, n_or_more);
			/* correct counts, if any given */
			for (i = 0; i < n; i++)
				pick_list[i].count = count;
		} else {
			n = query_objlist("Pick up what?", objchain,
					  traverse_how | AUTOSELECT_SINGLE | INVORDER_SORT | FEEL_COCKATRICE,
					  &pick_list, PICK_ANY, all_but_uchain);
		}
menu_pickup:
		n_tried = n;
		for (n_picked = i = 0; i < n; i++) {
			res = pickup_object(pick_list[i].item.a_obj, pick_list[i].count,
					    false);
			if (res < 0) break; /* can't continue */
			n_picked += res;
		}
		if (pick_list) free(pick_list);

	} else {
		/* old style interface */
		int ct = 0;
		long lcount;
		boolean all_of_a_type, selective;
		char oclasses[MAXOCLASSES];
		struct obj *obj, *obj2;

		oclasses[0] = '\0';   /* types to consider (empty for all) */
		all_of_a_type = true; /* take all of considered types */
		selective = false;    /* ask for each item */

		/* check for more than one object */
		for (obj = objchain;
		     obj;
		     obj = (traverse_how == BY_NEXTHERE) ? obj->nexthere : obj->nobj)
			ct++;

		if (ct == 1 && count) {
			/* if only one thing, then pick it */
			obj = objchain;
			lcount = min(obj->quan, (long)count);
			n_tried++;
			if (pickup_object(obj, lcount, false) > 0)
				n_picked++; /* picked something */
			goto end_query;

		} else if (ct >= 2) {
			int via_menu = 0;

			pline("There are %s objects here.",
			      (ct <= 10) ? "several" : "many");
			if (!query_classes(oclasses, &selective, &all_of_a_type,
					   "pick up", objchain,
					   traverse_how == BY_NEXTHERE,
					   &via_menu)) {
				if (!via_menu) return 0;
				n = query_objlist("Pick up what?",
						  objchain,
						  traverse_how | (selective ? 0 : INVORDER_SORT),
						  &pick_list, PICK_ANY,
						  via_menu == -2 ? allow_all : allow_category);
				goto menu_pickup;
			}
		}

		for (obj = objchain; obj; obj = obj2) {
			if (traverse_how == BY_NEXTHERE)
				obj2 = obj->nexthere; /* perhaps obj will be picked up */
			else
				obj2 = obj->nobj;
			lcount = -1L;

			if (!selective && oclasses[0] && !index(oclasses, obj->oclass))
				continue;

			if (!all_of_a_type) {
				char qbuf[BUFSZ];
				sprintf(qbuf, "Pick up %s?",
					safe_qbuf("", sizeof("Pick up ?"), doname(obj),
						  an(simple_typename(obj->otyp)), "something"));
				switch ((obj->quan < 2L) ? ynaq(qbuf) : ynNaq(qbuf)) {
					case 'q':
						goto end_query; /* out 2 levels */
					case 'n':
						continue;
					case 'a':
						all_of_a_type = true;
						if (selective) {
							selective = false;
							oclasses[0] = obj->oclass;
							oclasses[1] = '\0';
						}
						break;
					case '#':			  /* count was entered */
						if (!yn_number) continue; /* 0 count => No */
						lcount = (long)yn_number;
						if (lcount > obj->quan) lcount = obj->quan;
					fallthru;
					default: /* 'y' */
						break;
				}
			}
			if (lcount == -1L) lcount = obj->quan;

			n_tried++;
			if ((res = pickup_object(obj, lcount, false)) < 0) break;
			n_picked += res;
		}
	end_query:; /* semicolon needed by brain-damaged compilers */
	}

	if (!u.uswallow) {
		if (!OBJ_AT(u.ux, u.uy)) u.uundetected = 0;

		/* position may need updating (invisible hero) */
		if (n_picked) newsym(u.ux, u.uy);

		/* see whether there's anything else here, after auto-pickup is done */
		if (autopickup) check_here(n_picked > 0);
	}
	return n_tried > 0;
}

/* grab => forced pickup, rather than forced leave behind? */
boolean is_autopickup_exception(struct obj *obj, boolean grab) {
	/*
	 *  Does the text description of this match an exception?
	 */
	char *objdesc = makesingular(doname(obj));
	struct autopickup_exception *ape = grab ? iflags.autopickup_exceptions[AP_GRAB] :
						  iflags.autopickup_exceptions[AP_LEAVE];
	while (ape) {
		if (tre_regexec(&ape->pattern, objdesc, 0, NULL, 0) == 0) return true;
		ape = ape->next;
	}
	return false;
}

/*
 * Pick from the given list using flags.pickup_types.  Return the number
 * of items picked (not counts).  Create an array that returns pointers
 * and counts of the items to be picked up.  If the number of items
 * picked is zero, the pickup list is left alone.  The caller of this
 * function must free the pickup list.
 */
/* follow => how to follow the object list */
static int autopick(struct obj *olist, int follow, menu_item **pick_list) {
	menu_item *pi; /* pick item */
	struct obj *curr;
	int n;
	const char *otypes = flags.pickup_types;

	/* first count the number of eligible items */
	for (n = 0, curr = olist; curr; curr = FOLLOW(curr, follow))

		if ((!*otypes || index(otypes, curr->oclass) ||
		     (flags.pickup_thrown && curr->was_thrown) ||
		     is_autopickup_exception(curr, true)) &&
		    !is_autopickup_exception(curr, false))
			n++;

	if (n) {
		*pick_list = pi = alloc(sizeof(menu_item) * n);
		for (n = 0, curr = olist; curr; curr = FOLLOW(curr, follow))
			if ((!*otypes || index(otypes, curr->oclass) ||
			     (flags.pickup_thrown && curr->was_thrown) ||
			     is_autopickup_exception(curr, true)) &&
			    !is_autopickup_exception(curr, false)) {
				pi[n].item.a_obj = curr;
				pi[n].count = curr->quan;
				n++;
			}
	}
	return n;
}

/*
 * Put up a menu using the given object list.  Only those objects on the
 * list that meet the approval of the allow function are displayed.  Return
 * a count of the number of items selected, as well as an allocated array of
 * menu_items, containing pointers to the objects selected and counts.  The
 * returned counts are guaranteed to be in bounds and non-zero.
 *
 * Query flags:
 *	BY_NEXTHERE	  - Follow object list via nexthere instead of nobj.
 *	AUTOSELECT_SINGLE - Don't ask if only 1 object qualifies - just
 *			    use it.
 *	USE_INVLET	  - Use object's invlet.
 *	INVORDER_SORT	  - Use hero's pack order.
 *	SIGNAL_NOMENU	  - Return -1 rather than 0 if nothing passes "allow".
 *	SIGNAL_CANCEL	  - Return -2 rather than 0 if player cancels.
 */
int query_objlist(const char *qstr, struct obj *olist, int qflags, menu_item **pick_list, int how, bool (*allow)(struct obj *)) {
	int n;
	winid win;
	struct obj *curr, *last;
	char *pack;
	anything any;
	boolean printed_type_name;

	*pick_list = NULL;
	if (!olist) return 0;

	/* count the number of items allowed */
	for (n = 0, last = 0, curr = olist; curr; curr = FOLLOW(curr, qflags))
		if ((*allow)(curr)) {
			last = curr;
			n++;
		}

	if (n == 0) /* nothing to pick here */
		return (qflags & SIGNAL_NOMENU) ? -1 : 0;

	if (n == 1 && (qflags & AUTOSELECT_SINGLE)) {
		*pick_list = alloc(sizeof(menu_item));
		(*pick_list)->item.a_obj = last;
		(*pick_list)->count = last->quan;
		return 1;
	}

	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	any.a_obj = NULL;

	/*
	 * Run through the list and add the objects to the menu.  If
	 * INVORDER_SORT is set, we'll run through the list once for
	 * each type so we can group them.  The allow function will only
	 * be called once per object in the list.
	 */
	pack = flags.inv_order;
	do {
		printed_type_name = false;
		for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
			if ((qflags & FEEL_COCKATRICE) && curr->otyp == CORPSE &&
			    will_feel_cockatrice(curr, false)) {
				destroy_nhwindow(win); /* stop the menu and revert */
				look_here(0, false);
				return 0;
			}
			if ((!(qflags & INVORDER_SORT) || curr->oclass == *pack) && (*allow)(curr)) {
				/* if sorting, print type name (once only) */
				if (qflags & INVORDER_SORT && !printed_type_name) {
					any.a_obj = NULL;
					add_menu(win, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
						 let_to_name(*pack, false), MENU_UNSELECTED);
					printed_type_name = true;
				}

				any.a_obj = curr;
				add_menu(win, obj_to_glyph(curr), &any,
					 qflags & USE_INVLET ? curr->invlet : 0,
					 def_oc_syms[(int)objects[curr->otyp].oc_class],
					 ATR_NONE, doname(curr), MENU_UNSELECTED);
			}
		}
		pack++;
	} while (qflags & INVORDER_SORT && *pack);

	end_menu(win, qstr);
	n = select_menu(win, how, pick_list);
	destroy_nhwindow(win);

	if (n > 0) {
		menu_item *mi;
		int i;

		/* fix up counts:  -1 means no count used => pick all */
		for (i = 0, mi = *pick_list; i < n; i++, mi++)
			if (mi->count == -1L || mi->count > mi->item.a_obj->quan)
				mi->count = mi->item.a_obj->quan;
	} else if (n < 0) {
		/* caller's don't expect -1 */
		n = (qflags & SIGNAL_CANCEL) ? -2 : 0;
	}
	return n;
}

/*
 * allow menu-based category (class) selection (for Drop,take off etc.)
 *
 */
int query_category(const char *qstr, struct obj *olist, int qflags, menu_item **pick_list, int how) {
	int n;
	winid win;
	struct obj *curr;
	char *pack;
	anything any;
	boolean collected_type_name;
	char invlet;
	int ccount;
	boolean do_unpaid = false;
	boolean do_blessed = false, do_cursed = false, do_uncursed = false,
		do_buc_unknown = false;
	int num_buc_types = 0;

	*pick_list = NULL;
	if (!olist) return 0;
	if ((qflags & UNPAID_TYPES) && count_unpaid(olist)) do_unpaid = true;
	if ((qflags & BUC_BLESSED) && count_buc(olist, BUC_BLESSED)) {
		do_blessed = true;
		num_buc_types++;
	}
	if ((qflags & BUC_CURSED) && count_buc(olist, BUC_CURSED)) {
		do_cursed = true;
		num_buc_types++;
	}
	if ((qflags & BUC_UNCURSED) && count_buc(olist, BUC_UNCURSED)) {
		do_uncursed = true;
		num_buc_types++;
	}
	if ((qflags & BUC_UNKNOWN) && count_buc(olist, BUC_UNKNOWN)) {
		do_buc_unknown = true;
		num_buc_types++;
	}

	ccount = count_categories(olist, qflags);
	/* no point in actually showing a menu for a single category */
	if (ccount == 1 && !do_unpaid && num_buc_types <= 1 && !(qflags & BILLED_TYPES)) {
		for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
			if ((qflags & WORN_TYPES) &&
			    !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL | W_WEP | W_SWAPWEP | W_QUIVER)))
				continue;
			break;
		}
		if (curr) {
			*pick_list = alloc(sizeof(menu_item));
			(*pick_list)->item.a_int = curr->oclass;
			return 1;
		} else {
#ifdef DEBUG
			impossible("query_category: no single object match");
#endif
		}
		return 0;
	}

	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	pack = flags.inv_order;
	if ((qflags & ALL_TYPES) && (ccount > 1)) {
		invlet = 'a';
		any.a_void = 0;
		any.a_int = ALL_TYPES_SELECTED;
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 (qflags & WORN_TYPES) ? "All worn types" : "All types",
			 MENU_UNSELECTED);
		invlet = 'b';
	} else
		invlet = 'a';
	do {
		collected_type_name = false;
		for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
			if (curr->oclass == *pack) {
				if ((qflags & WORN_TYPES) &&
				    !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
							 W_WEP | W_SWAPWEP | W_QUIVER)))
					continue;
				if (!collected_type_name) {
					any.a_void = 0;
					any.a_int = curr->oclass;
					add_menu(win, NO_GLYPH, &any, invlet++,
						 def_oc_syms[(int)objects[curr->otyp].oc_class],
						 ATR_NONE, let_to_name(*pack, false),
						 MENU_UNSELECTED);
					collected_type_name = true;
				}
			}
		}
		pack++;
		if (invlet >= 'u') {
			impossible("query_category: too many categories");
			return 0;
		}
	} while (*pack);
	/* unpaid items if there are any */
	if (do_unpaid) {
		invlet = 'u';
		any.a_void = 0;
		any.a_int = 'u';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Unpaid items", MENU_UNSELECTED);
	}
	/* billed items: checked by caller, so always include if BILLED_TYPES */
	if (qflags & BILLED_TYPES) {
		invlet = 'x';
		any.a_void = 0;
		any.a_int = 'x';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Unpaid items already used up", MENU_UNSELECTED);
	}
	if (qflags & CHOOSE_ALL) {
		invlet = 'A';
		any.a_void = 0;
		any.a_int = 'A';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 (qflags & WORN_TYPES) ?
				 "Auto-select every item being worn" :
				 "Auto-select every item",
			 MENU_UNSELECTED);
	}
	/* items with b/u/c/unknown if there are any */
	if (do_blessed) {
		invlet = 'B';
		any.a_void = 0;
		any.a_int = 'B';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Items known to be Blessed", MENU_UNSELECTED);
	}
	if (do_cursed) {
		invlet = 'C';
		any.a_void = 0;
		any.a_int = 'C';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Items known to be Cursed", MENU_UNSELECTED);
	}
	if (do_uncursed) {
		invlet = 'U';
		any.a_void = 0;
		any.a_int = 'U';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Items known to be Uncursed", MENU_UNSELECTED);
	}
	if (do_buc_unknown) {
		invlet = 'X';
		any.a_void = 0;
		any.a_int = 'X';
		add_menu(win, NO_GLYPH, &any, invlet, 0, ATR_NONE,
			 "Items of unknown B/C/U status",
			 MENU_UNSELECTED);
	}
	end_menu(win, qstr);
	n = select_menu(win, how, pick_list);
	destroy_nhwindow(win);
	if (n < 0)
		n = 0; /* caller's don't expect -1 */
	return n;
}

static int count_categories(struct obj *olist, int qflags) {
	char *pack;
	boolean counted_category;
	int ccount = 0;
	struct obj *curr;

	pack = flags.inv_order;
	do {
		counted_category = false;
		for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
			if (curr->oclass == *pack) {
				if ((qflags & WORN_TYPES) &&
				    !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
							 W_WEP | W_SWAPWEP | W_QUIVER)))
					continue;
				if (!counted_category) {
					ccount++;
					counted_category = true;
				}
			}
		}
		pack++;
	} while (*pack);
	return ccount;
}

/* could we carry `obj'? if not, could we carry some of it/them? */
/* obj, container: object to pick up, bag it's coming out of */
static long carry_count(struct obj *obj, struct obj *container, long count, boolean telekinesis, int *wt_before, int *wt_after) {
	boolean adjust_wt = container && carried(container),
		is_gold = obj->oclass == COIN_CLASS;
	int wt, iw, ow, oow;
	long qq, savequan;
	long umoney = money_cnt(invent);

	unsigned saveowt;
	const char *verb, *prefx1, *prefx2, *suffx;
	char obj_nambuf[BUFSZ], where[BUFSZ];

	savequan = obj->quan;
	saveowt = obj->owt;

	iw = max_capacity();

	if (count != savequan) {
		obj->quan = count;
		obj->owt = (unsigned)weight(obj);
	}
	wt = iw + (int)obj->owt;
	if (adjust_wt)
		wt -= (container->otyp == BAG_OF_HOLDING) ?
			      (int)DELTA_CWT(container, obj) :
			      (int)obj->owt;
	/* This will go with silver+copper & new gold weight */
	if (is_gold) /* merged gold might affect cumulative weight */
		wt -= (GOLD_WT(umoney) + GOLD_WT(count) - GOLD_WT(umoney + count));
	if (count != savequan) {
		obj->quan = savequan;
		obj->owt = saveowt;
	}
	*wt_before = iw;
	*wt_after = wt;

	if (wt < 0)
		return count;

	/* see how many we can lift */
	if (is_gold) {
		iw -= (int)GOLD_WT(umoney);
		if (!adjust_wt) {
			qq = GOLD_CAPACITY((long)iw, umoney);
		} else {
			oow = 0;
			qq = 50L - (umoney % 100L) - 1L;
			if (qq < 0L) qq += 100L;
			for (; qq <= count; qq += 100L) {
				obj->quan = qq;
				obj->owt = (unsigned)GOLD_WT(qq);
				ow = (int)GOLD_WT(umoney + qq);
				ow -= (container->otyp == BAG_OF_HOLDING) ?
					      (int)DELTA_CWT(container, obj) :
					      (int)obj->owt;
				if (iw + ow >= 0) break;
				oow = ow;
			}
			iw -= oow;
			qq -= 100L;
		}
		if (qq < 0L)
			qq = 0L;
		else if (qq > count)
			qq = count;
		wt = iw + (int)GOLD_WT(umoney + qq);
	} else if (count > 1 || count < obj->quan) {
		/*
		 * Ugh. Calc num to lift by changing the quan of of the
		 * object and calling weight.
		 *
		 * This works for containers only because containers
		 * don't merge.		-dean
		 */
		for (qq = 1L; qq <= count; qq++) {
			obj->quan = qq;
			obj->owt = (unsigned)(ow = weight(obj));
			if (adjust_wt)
				ow -= (container->otyp == BAG_OF_HOLDING) ?
					      (int)DELTA_CWT(container, obj) :
					      (int)obj->owt;
			if (iw + ow >= 0)
				break;
			wt = iw + ow;
		}
		--qq;
	} else {
		/* there's only one, and we can't lift it */
		qq = 0L;
	}
	obj->quan = savequan;
	obj->owt = saveowt;

	if (qq < count) {
		/* some message will be given */
		strcpy(obj_nambuf, doname(obj));
		if (container) {
			sprintf(where, "in %s", the(xname(container)));
			verb = "carry";
		} else {
			strcpy(where, "lying here");
			verb = telekinesis ? "acquire" : "lift";
		}
	} else {
		/* lint supppression */
		*obj_nambuf = *where = '\0';
		verb = "";
	}
	/* we can carry qq of them */
	if (qq > 0) {
		if (qq < count)
			pline("You can only %s %s of the %s %s.",
			      verb, (qq == 1L) ? "one" : "some", obj_nambuf, where);
		*wt_after = wt;
		return qq;
	}

	if (!container) strcpy(where, "here"); /* slightly shorter form */
	if (invent || umoney) {
		prefx1 = "you cannot ";
		prefx2 = "";
		suffx = " any more";
	} else {
		prefx1 = (obj->quan == 1L) ? "it " : "even one ";
		prefx2 = "is too heavy for you to ";
		suffx = "";
	}
	pline("There %s %s %s, but %s%s%s%s.",
	      otense(obj, "are"), obj_nambuf, where,
	      prefx1, prefx2, verb, suffx);

	/* *wt_after = iw; */
	return 0L;
}

/* determine whether character is able and player is willing to carry `obj' */
static int lift_object(struct obj *obj, struct obj *container, long *cnt_p, boolean telekinesis) {
	int result, old_wt, new_wt, prev_encumbr, next_encumbr;

	if (obj->otyp == BOULDER && In_sokoban(&u.uz)) {
		pline("You cannot get your %s around this %s.",
		      body_part(HAND), xname(obj));
		return -1;
	}
	if (obj->otyp == LOADSTONE ||
	    (obj->otyp == BOULDER && throws_rocks(youmonst.data)))
		return 1; /* lift regardless of current situation */

	*cnt_p = carry_count(obj, container, *cnt_p, telekinesis, &old_wt, &new_wt);
	if (*cnt_p < 1L) {
		result = -1; /* nothing lifted */
	} else if (inv_cnt() >= 52 && !merge_choice(invent, obj)) {
		pline("Your knapsack cannot accommodate any more items.");
		result = -1; /* nothing lifted */
	} else {
		result = 1;
		prev_encumbr = near_capacity();
		if (prev_encumbr < flags.pickup_burden)
			prev_encumbr = flags.pickup_burden;
		next_encumbr = calc_capacity(new_wt - old_wt);
		if (next_encumbr > prev_encumbr) {
			if (telekinesis) {
				result = 0; /* don't lift */
			} else {
				char qbuf[BUFSZ];
				long savequan = obj->quan;

				obj->quan = *cnt_p;
				strcpy(qbuf,
				       (next_encumbr > HVY_ENCUMBER) ? overloadmsg :
				       (next_encumbr > MOD_ENCUMBER) ? nearloadmsg :
				       moderateloadmsg);
				if (container) strsubst(qbuf, "lifting", "removing");

				sprintf(eos(qbuf), " %s. Continue?",
					safe_qbuf(qbuf, sizeof(" . Continue?"),
						  doname(obj), an(simple_typename(obj->otyp)), "something"));
				obj->quan = savequan;
				switch (ynq(qbuf)) {
					case 'q':
						result = -1;
						break;
					case 'n':
						result = 0;
						break;
					default:
						break; /* 'y' => result == 1 */
				}
				clear_nhwindow(WIN_MESSAGE);
			}
		}
	}

	if (obj->otyp == SCR_SCARE_MONSTER && result <= 0 && !container)
		obj->spe = 0;
	return result;
}

/* To prevent qbuf overflow in prompts use planA only
 * if it fits, or planB if PlanA doesn't fit,
 * finally using the fallback as a last resort.
 * last_restort is expected to be very short.
 */
const char *safe_qbuf(const char *qbuf, uint padlength, const char *planA, const char *planB, const char *last_resort) {
	/* convert size_t (or int for ancient systems) to ordinary unsigned */
	unsigned len_qbuf = (unsigned)strlen(qbuf),
		 len_planA = (unsigned)strlen(planA),
		 len_planB = (unsigned)strlen(planB),
		 len_lastR = (unsigned)strlen(last_resort);
	unsigned textleft = QBUFSZ - (len_qbuf + padlength);

	if (len_lastR >= textleft) {
		impossible("safe_qbuf: last_resort too large at %u characters.",
			   len_lastR);
		return "";
	}
	return (len_planA < textleft) ? planA :
					(len_planB < textleft) ? planB : last_resort;
}

/*
 * Pick up <count> of obj from the ground and add it to the hero's inventory.
 * Returns -1 if caller should break out of its loop, 0 if nothing picked
 * up, 1 if otherwise.
 */
/* telekinesis = not picking it up directly by hand */
int pickup_object(struct obj *obj, long count, boolean telekinesis) {
	int res, nearload;

	if (obj->quan < count) {
		impossible("pickup_object: count %ld > quan %ld?",
			   count, obj->quan);
		return 0;
	}

	/* In case of auto-pickup, where we haven't had a chance
	   to look at it yet; affects docall(SCR_SCARE_MONSTER). */
	if (!Blind)
		if (!obj->oinvis || See_invisible)
			obj->dknown = 1;

	if (obj == uchain) { /* do not pick up attached chain */
		return 0;
	} else if (obj->oartifact && !touch_artifact(obj, &youmonst)) {
		return 0;
	} else if (obj->otyp == CORPSE) {
		if ((touch_petrifies(&mons[obj->corpsenm])) && !uarmg && !Stone_resistance && !telekinesis) {
			if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
				display_nhwindow(WIN_MESSAGE, false);
			else {
				char kbuf[BUFSZ];

				strcpy(kbuf, an(corpse_xname(obj, true)));
				pline("Touching %s is a fatal mistake.", kbuf);
				strcpy(kbuf, an(killer_cxname(obj, true)));
				instapetrify(kbuf);
				return -1;
			}
		} else if (is_rider(&mons[obj->corpsenm])) {
			pline("At your %s, the corpse suddenly moves...",
			      telekinesis ? "attempted acquisition" : "touch");
			revive_corpse(obj, false);
			exercise(A_WIS, false);
			return -1;
		}
	} else if (obj->otyp == SCR_SCARE_MONSTER) {
		if (obj->blessed)
			obj->blessed = 0;
		else if (!obj->spe && !obj->cursed)
			obj->spe = 1;
		else {
			pline("The scroll%s %s to dust as you %s %s up.",
			      plur(obj->quan), otense(obj, "turn"),
			      telekinesis ? "raise" : "pick",
			      (obj->quan == 1L) ? "it" : "them");
			if (!(objects[SCR_SCARE_MONSTER].oc_name_known) &&
			    !(objects[SCR_SCARE_MONSTER].oc_uname))
				docall(obj);
			useupf(obj, obj->quan);
			return 1; /* tried to pick something up and failed, but
				   don't want to terminate pickup loop yet   */
		}
	}

	if ((res = lift_object(obj, NULL, &count, telekinesis)) <= 0)
		return res;

	/* Whats left of the special case for gold :-) */
	if (obj->oclass == COIN_CLASS) context.botl = 1;
	if (obj->quan != count && obj->otyp != LOADSTONE)
		obj = splitobj(obj, count);

	obj = pick_obj(obj);

	if (uwep && uwep == obj) mrg_to_wielded = true;
	nearload = near_capacity();
	prinv(nearload == SLT_ENCUMBER ? moderateloadmsg : NULL,
	      obj, count);
	mrg_to_wielded = false;
	return 1;
}

/*
 * Do the actual work of picking otmp from the floor or monster's interior
 * and putting it in the hero's inventory.  Take care of billing.  Return a
 * pointer to the object where otmp ends up.  This may be different
 * from otmp because of merging.
 */
struct obj *pick_obj(struct obj *otmp) {
	obj_extract_self(otmp);
	if (!u.uswallow && otmp != uball && costly_spot(otmp->ox, otmp->oy)) {
		char saveushops[5], fakeshop[2];

		/* addtobill cares about your location rather than the object's;
		   usually they'll be the same, but not when using telekinesis
		   (if ever implemented) or a grappling hook */
		strcpy(saveushops, u.ushops);
		fakeshop[0] = *in_rooms(otmp->ox, otmp->oy, SHOPBASE);
		fakeshop[1] = '\0';
		strcpy(u.ushops, fakeshop);
		/* sets obj->unpaid if necessary */
		addtobill(otmp, true, false, false);
		strcpy(u.ushops, saveushops);
		/* if you're outside the shop, make shk notice */
		if (!index(u.ushops, *fakeshop))
			remote_burglary(otmp->ox, otmp->oy);
	}
	if (otmp->no_charge) /* only applies to objects outside invent */
		otmp->no_charge = 0;
	if (otmp->was_thrown) /* likewise */
		otmp->was_thrown = 0;
	newsym(otmp->ox, otmp->oy);
	return addinv(otmp); /* might merge it with other objects */
}

/*
 * prints a message if encumbrance changed since the last check and
 * returns the new encumbrance value (from near_capacity()).
 */
int encumber_msg() {
	static int oldcap = UNENCUMBERED;
	int newcap = near_capacity();

	if (oldcap < newcap) {
		switch (newcap) {
			case 1:
				pline("Your movements are slowed slightly because of your load.");
				break;
			case 2:
				pline("You rebalance your load.  Movement is difficult.");
				break;
			case 3:
				pline("You %s under your heavy load.  Movement is very hard.",
				      stagger(youmonst.data, "stagger"));
				break;
			default:
				pline("You %s move a handspan with this load!",
				      newcap == 4 ? "can barely" : "can't even");
				break;
		}
		context.botl = 1;
	} else if (oldcap > newcap) {
		switch (newcap) {
			case 0:
				pline("Your movements are now unencumbered.");
				break;
			case 1:
				pline("Your movements are only slowed slightly by your load.");
				break;
			case 2:
				pline("You rebalance your load.  Movement is still difficult.");
				break;
			case 3:
				pline("You %s under your load.  Movement is still very hard.",
				      stagger(youmonst.data, "stagger"));
				break;
		}
		context.botl = 1;
	}

	oldcap = newcap;
	return newcap;
}

/* Is there a container at x,y. Optional: return count of containers at x,y */
static int container_at(int x, int y, bool countem) {
	struct obj *cobj, *nobj;
	int container_count = 0;

	for (cobj = level.objects[x][y]; cobj; cobj = nobj) {
		nobj = cobj->nexthere;
		if (Is_container(cobj)) {
			container_count++;
			if (!countem) break;
		}
	}
	return container_count;
}

static bool able_to_loot(int x, int y, bool looting) {
	const char *verb = looting ? "loot" : "tip";

	if (!can_reach_floor()) {
		if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
			rider_cant_reach(); /* not skilled enough to reach */
		else
			pline("You cannot reach the %s.", surface(x, y));
		return false;
	} else if ((is_pool(x, y) && (looting || !Underwater)) || is_lava(x, y)) {
		// at present, can't loot in water even when Underwater.
		// can tip underwater, but not in lava
		pline("You cannot %s things that are deep in the %s.", verb, is_lava(x, y) ? "lava" : "water");
		return false;
	} else if (nolimbs(youmonst.data)) {
		pline("Without limbs, you cannot %s anything.", verb);
		return false;
	} else if (looting && !freehand()) {
		pline("Without a free %s, you cannot %s anything.", body_part(HAND), verb);
		return false;
	}
	return true;
}

static bool mon_beside(int x, int y) {
	int i, j, nx, ny;
	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++) {
			nx = x + i;
			ny = y + j;
			if (isok(nx, ny) && MON_AT(nx, ny))
				return true;
		}
	return false;
}

/* loot a container on the floor or loot saddle from mon. */
int doloot(void) {
	struct obj *cobj, *nobj;
	int c = -1;
	int timepassed = 0;
	coord cc;
	boolean underfoot = true;
	const char *dont_find_anything = "don't find anything";
	struct monst *mtmp;
	char qbuf[BUFSZ];
	int prev_inquiry = 0;
	boolean prev_loot = false;

	if (check_capacity(NULL)) {
		/* "Can't do that while carrying so much stuff." */
		return 0;
	}
	if (nohands(youmonst.data)) {
		pline("You have no hands!"); /* not `body_part(HAND)' */
		return 0;
	}
	cc.x = u.ux;
	cc.y = u.uy;

lootcont:

	if (container_at(cc.x, cc.y, false)) {
		boolean any = false;

		if (!able_to_loot(cc.x, cc.y, true)) return 0;
		for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = nobj) {
			nobj = cobj->nexthere;

			if (Is_container(cobj)) {
				sprintf(qbuf, "There is %s here, loot it?",
					safe_qbuf("", sizeof("There is  here, loot it?"),
						  doname(cobj), an(simple_typename(cobj->otyp)),
						  "a container"));
				c = ynq(qbuf);
				if (c == 'q') return timepassed;
				if (c == 'n') continue;
				any = true;

				if (cobj->olocked) {
					pline("Hmmm, it seems to be locked.");
					cobj->lknown = true;
					continue;
				}
				cobj->lknown = true;

				if (cobj->otyp == BAG_OF_TRICKS) {
					int tmp;
					pline("You carefully open the bag...");
					pline("It develops a huge set of teeth and bites you!");
					tmp = rnd(10);
					losehp(Maybe_Half_Phys(tmp), "carnivorous bag", KILLED_BY_AN);
					makeknown(BAG_OF_TRICKS);
					timepassed = 1;
					continue;
				}

				pline("You carefully open %s...", the(xname(cobj)));
				timepassed |= use_container(&cobj, 0);
				/* might have triggered chest trap or magic bag explosion */
				if (multi < 0 || !cobj) return 1;
			}
		}
		if (any) c = 'y';
	} else if (Confusion) {
		struct obj *goldob;
		/* Find a money object to mess with */
		for (goldob = invent; goldob; goldob = goldob->nobj) {
			if (goldob->oclass == COIN_CLASS) break;
		}
		if (goldob) {
			long contribution = rnd((int)min(LARGEST_INT, goldob->quan));
			if (contribution < goldob->quan)
				goldob = splitobj(goldob, contribution);
			freeinv(goldob);
			if (IS_THRONE(levl[u.ux][u.uy].typ)) {
				struct obj *coffers;
				int pass;
				/* find the original coffers chest, or any chest */
				for (pass = 2; pass > -1; pass -= 2)
					for (coffers = fobj; coffers; coffers = coffers->nobj)
						if (coffers->otyp == CHEST && coffers->spe == pass)
							goto gotit; /* two level break */
gotit:
				if (coffers) {
					verbalize("Thank you for your contribution to reduce the debt.");
					add_to_container(coffers, goldob);
					coffers->owt = weight(coffers);
				} else {
					struct monst *mon = makemon(courtmon(),
								    u.ux, u.uy, NO_MM_FLAGS);
					if (mon) {
						add_to_minv(mon, goldob);
						pline("The exchequer accepts your contribution.");
					} else {
						dropy(goldob);
					}
				}
			} else {
				dropy(goldob);
				pline("Ok, now there is loot here.");
			}
		}
	} else if (IS_GRAVE(levl[cc.x][cc.y].typ)) {
		pline("You need to dig up the grave to effectively loot it...");
	}
	/*
	 * 3.3.1 introduced directional looting for some things.
	 */
	if (c != 'y' && mon_beside(u.ux, u.uy)) {
		if (!get_adjacent_loc("Loot in what direction?", "Invalid loot location",
				      u.ux, u.uy, &cc)) return 0;
		if (cc.x == u.ux && cc.y == u.uy) {
			underfoot = true;
			if (container_at(cc.x, cc.y, false))
				goto lootcont;
		} else
			underfoot = false;
		if (u.dz < 0) {
			pline("You %s to loot on the %s.", dont_find_anything,
			      ceiling(cc.x, cc.y));
			timepassed = 1;
			return timepassed;
		}
		mtmp = m_at(cc.x, cc.y);
		if (mtmp) timepassed = loot_mon(mtmp, &prev_inquiry, &prev_loot);

		/* Preserve pre-3.3.1 behaviour for containers.
		 * Adjust this if-block to allow container looting
		 * from one square away to change that in the future.
		 */
		if (!underfoot) {
			if (container_at(cc.x, cc.y, false)) {
				if (mtmp) {
					pline("You can't loot anything %sthere with %s in the way.",
					      prev_inquiry ? "else " : "", mon_nam(mtmp));
					return timepassed;
				} else {
					pline("You have to be at a container to loot it.");
				}
			} else {
				pline("You %s %sthere to loot.", dont_find_anything,
				      (prev_inquiry || prev_loot) ? "else " : "");
				return timepassed;
			}
		}
	} else if (c != 'y' && c != 'n') {
		pline("You %s %s to loot.", dont_find_anything,
		      underfoot ? "here" : "there");
	}
	return timepassed;
}

/* loot_mon() returns amount of time passed.
 */
int loot_mon(struct monst *mtmp, int *passed_info, boolean *prev_loot) {
	int c = -1;
	int timepassed = 0;
	struct obj *otmp;
	char qbuf[QBUFSZ];

	/* 3.3.1 introduced the ability to remove saddle from a steed             */
	/* 	*passed_info is set to true if a loot query was given.               */
	/*	*prev_loot is set to true if something was actually acquired in here. */
	if (mtmp && mtmp != u.usteed && (otmp = which_armor(mtmp, W_SADDLE))) {
		long unwornmask;
		if (passed_info) *passed_info = 1;
		sprintf(qbuf, "Do you want to remove the saddle from %s?",
			x_monnam(mtmp, ARTICLE_THE, NULL, SUPPRESS_SADDLE, false));
		if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
			if (nolimbs(youmonst.data)) {
				pline("You can't do that without limbs."); /* not body_part(HAND) */
				return 0;
			}
			if (otmp->cursed) {
				pline("You can't. The saddle seems to be stuck to %s.",
				      x_monnam(mtmp, ARTICLE_THE, NULL,
					       SUPPRESS_SADDLE, false));

				/* the attempt costs you time */
				return 1;
			}
			obj_extract_self(otmp);
			if ((unwornmask = otmp->owornmask) != 0L) {
				mtmp->misc_worn_check &= ~unwornmask;
				otmp->owornmask = 0L;
				update_mon_intrinsics(mtmp, otmp, false, false);
			}
			otmp = hold_another_object(otmp, "You drop %s!", doname(otmp),
						   NULL);
			timepassed = rnd(3);
			if (prev_loot) *prev_loot = true;
		} else if (c == 'q') {
			return 0;
		}
	}
	/* 3.4.0 introduced the ability to pick things up from within swallower's stomach */
	if (u.uswallow) {
		int count = passed_info ? *passed_info : 0;
		timepassed = pickup(count);
	}
	return timepassed;
}

/*
 * Decide whether an object being placed into a magic bag will cause
 * it to explode.  If the object is a bag itself, check recursively.
 */
boolean mbag_explodes(struct obj *obj, int depthin) {
	/* these won't cause an explosion when they're empty */
	if ((obj->otyp == WAN_CANCELLATION || obj->otyp == BAG_OF_TRICKS) &&
	    obj->spe <= 0)
		return false;

	/* odds: 1/1, 2/2, 3/4, 4/8, 5/16, 6/32, 7/64, 8/128, 9/128, 10/128,... */
	if ((Is_mbag(obj) || obj->otyp == WAN_CANCELLATION) &&
	    (rn2(1 << (depthin > 7 ? 7 : depthin)) <= depthin))
		return true;
	else if (Has_contents(obj)) {
		struct obj *otmp;

		for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
			if (mbag_explodes(otmp, depthin + 1)) return true;
	}
	return false;
}

void destroy_mbag(struct obj *bomb, boolean silent) {
	xchar x, y;
	boolean underwater;
	struct monst *mtmp = NULL;

	if (get_obj_location(bomb, &x, &y, BURIED_TOO | CONTAINED_TOO)) {
		switch (bomb->where) {
			case OBJ_MINVENT:
				mtmp = bomb->ocarry;
				if (bomb == MON_WEP(mtmp)) {
					bomb->owornmask &= ~W_WEP;
					MON_NOWEP(mtmp);
				}
				if (!silent && canseemon(mtmp))
					pline("You see %s engulfed in an explosion!", mon_nam(mtmp));
				mtmp->mhp -= d(6, 6);
				if (mtmp->mhp < 1) {
					if (!bomb->yours)
						monkilled(mtmp, silent ? "" : "explosion", AD_PHYS);
					else
						xkilled(mtmp, !silent);
				}
				break;
			case OBJ_INVENT:
				/* This shouldn't be silent! */
				pline("Something explodes inside your knapsack!");
				if (bomb == uwep) {
					uwepgone();
					stop_occupation();
				} else if (bomb == uswapwep) {
					uswapwepgone();
					stop_occupation();
				} else if (bomb == uquiver) {
					uqwepgone();
					stop_occupation();
				}
				losehp(d(6, 6), "carrying live explosives", KILLED_BY);
				break;
			case OBJ_FLOOR:
				underwater = is_pool(x, y);
				if (!silent) {
					if (x == u.ux && y == u.uy) {
						if (underwater && (Flying || Levitation))
							pline("The water boils beneath you.");
						else if (underwater && Wwalking)
							pline("The water erupts around you.");
						else
							pline("A bag explodes under your %s!",
							      makeplural(body_part(FOOT)));
					} else if (cansee(x, y))
						pline(underwater ?
							      "You see a plume of water shoot up." :
							      "You see a bag explode.");
				}
				if (underwater && (Flying || Levitation || Wwalking)) {
					if (Wwalking && x == u.ux && y == u.uy) {
						struct trap trap;
						trap.ntrap = NULL;
						trap.tx = x;
						trap.ty = y;
						trap.launch.x = -1;
						trap.launch.y = -1;
						trap.ttyp = RUST_TRAP;
						trap.tseen = 0;
						trap.once = 0;
						trap.madeby_u = 0;
						trap.dst.dnum = -1;
						trap.dst.dlevel = -1;
						dotrap(&trap, 0);
					}
					goto free_bomb;
				}
				break;
			default: /* Buried, contained, etc. */
				if (!silent)
					You_hear("a muffled explosion.");
				goto free_bomb;
				break;
		}
	}

free_bomb:
	if (Has_contents(bomb))
		delete_contents(bomb);

	obj_extract_self(bomb);
	obfree(bomb, NULL);
	newsym(x, y);
}

/* Returns: -1 to stop, 1 item was inserted, 0 item was not inserted. */
static int in_container(struct obj *obj) {
	boolean floor_container = !carried(current_container);
	boolean was_unpaid = false;
	char buf[BUFSZ];

	if (!current_container) {
		impossible("<in> no current_container?");
		return 0;
	} else if (obj == uball || obj == uchain) {
		pline("You must be kidding.");
		return 0;
	} else if (obj == current_container) {
		pline("That would be an interesting topological exercise.");
		return 0;
	} else if (obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) {
		Norep("You cannot %s something you are wearing.",
		      Icebox ? "refrigerate" : "stash");
		return 0;
	} else if ((obj->otyp == LOADSTONE) && obj->cursed) {
		obj->bknown = 1;
		pline("The stone%s won't leave your person.", plur(obj->quan));
		return 0;
	} else if (obj->otyp == AMULET_OF_YENDOR ||
		   obj->otyp == CANDELABRUM_OF_INVOCATION ||
		   obj->otyp == BELL_OF_OPENING ||
		   obj->otyp == SPE_BOOK_OF_THE_DEAD) {
		/* Prohibit Amulets in containers; if you allow it, monsters can't
		 * steal them.  It also becomes a pain to check to see if someone
		 * has the Amulet.  Ditto for the Candelabrum, the Bell and the Book.
		 */
		pline("%s cannot be confined in such trappings.", The(xname(obj)));
		return 0;
	} else if (obj->otyp == LEASH && obj->leashmon != 0) {
		pline("%s attached to your pet.", Tobjnam(obj, "are"));
		return 0;
	} else if (obj == uwep) {
		if (welded(obj)) {
			weldmsg(obj);
			return 0;
		}
		setuwep(NULL, false);
		if (uwep) return 0; /* unwielded, died, rewielded */
	} else if (obj == uswapwep) {
		setuswapwep(NULL, false);
		if (uswapwep) return 0; /* unwielded, died, rewielded */
	} else if (obj == uquiver) {
		setuqwep(NULL);
		if (uquiver) return 0; /* unwielded, died, rewielded */
	}

	if (obj->otyp == CORPSE) {
		if ((touch_petrifies(&mons[obj->corpsenm])) && !uarmg && !Stone_resistance) {
			if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
				display_nhwindow(WIN_MESSAGE, false);
			else {
				char kbuf[BUFSZ];

				strcpy(kbuf, an(corpse_xname(obj, true)));
				pline("Touching %s is a fatal mistake.", kbuf);
				strcpy(kbuf, an(killer_cxname(obj, true)));
				instapetrify(kbuf);
				return -1;
			}
		}
	}

	/* boxes, boulders, and big statues can't fit into any container */
	if (obj->otyp == ICE_BOX || Is_box(obj) || obj->otyp == BOULDER ||
	    (obj->otyp == STATUE && bigmonst(&mons[obj->corpsenm]))) {
		/*
		 *  xname() uses a static result array.  Save obj's name
		 *  before current_container's name is computed.  Don't
		 *  use the result of strcpy() --- the order
		 *  of evaluation of the parameters is undefined.
		 */
		strcpy(buf, the(xname(obj)));
		pline("You cannot fit %s into %s.", buf,
		      the(xname(current_container)));
		return 0;
	}

	freeinv(obj);

	if (obj_is_burning(obj)) /* this used to be part of freeinv() */
		snuff_lit(obj);

	if (floor_container && costly_spot(u.ux, u.uy)) {
		if (current_container->no_charge && !obj->unpaid) {
			/* don't sell when putting the item into your own container */
			obj->no_charge = 1;
		} else if (obj->oclass != COIN_CLASS) {
			/* sellobj() will take an unpaid item off the shop bill
			 * note: coins are handled later */
			was_unpaid = obj->unpaid ? true : false;
			sellobj_state(SELL_DELIBERATE);
			sellobj(obj, u.ux, u.uy);
			sellobj_state(SELL_NORMAL);
		}
	}
	if (Icebox && !age_is_relative(obj)) {
		obj->age = monstermoves - obj->age; /* actual age */
		/* stop any corpse timeouts when frozen */
		if (obj->otyp == CORPSE && obj->timed) {
			long rot_alarm = stop_timer(ROT_CORPSE, obj_to_any(obj));
			stop_timer(MOLDY_CORPSE, obj_to_any(obj));
			stop_timer(REVIVE_MON, obj_to_any(obj));
			/* mark a non-reviving corpse as such */
			if (rot_alarm) obj->norevive = 1;
		}
	} else if (Is_mbag(current_container) && mbag_explodes(obj, 0)) {
		/* explicitly mention what item is triggering the explosion */
		pline("As you put %s inside, you are blasted by a magical explosion!", doname(obj));
		if (Has_contents(obj)) {
			struct obj *otmp;
			while ((otmp = container_extract_indestructable(obj)))
				if (!flooreffects(otmp, u.ux, u.uy, "fall"))
					place_object(otmp, u.ux, u.uy);
		}
		/* did not actually insert obj yet */
		if (was_unpaid) addtobill(obj, false, false, true);
		if (Has_contents(obj))
			delete_contents(obj);
		obfree(obj, NULL);
		delete_contents(current_container);
		if (!floor_container)
			useup(current_container);
		else if (obj_here(current_container, u.ux, u.uy))
			useupf(current_container, current_container->quan);
		else
			panic("in_container:  bag not found.");

		losehp(d(6, 6), "magical explosion", KILLED_BY_AN);
		current_container = 0; /* baggone = true; */
	}

	if (current_container) {
		strcpy(buf, the(xname(current_container)));
		pline("You put %s into %s.", doname(obj), buf);

		/* gold in container always needs to be added to credit */
		if (floor_container && obj->oclass == COIN_CLASS)
			sellobj(obj, current_container->ox, current_container->oy);
		add_to_container(current_container, obj);
		current_container->owt = weight(current_container);
	}
	/* gold needs this, and freeinv() many lines above may cause
	 * the encumbrance to disappear from the status, so just always
	 * update status immediately.
	 */
	bot();

	return current_container ? 1 : -1;
}

static int ck_bag(struct obj *obj) {
	return current_container && obj != current_container;
}

/* Returns: -1 to stop, 1 item was removed, 0 item was not removed. */
static int out_container(struct obj *obj) {
	struct obj *otmp;
	boolean is_gold = (obj->oclass == COIN_CLASS);
	int res, loadlev;
	long count;

	if (!current_container) {
		impossible("<out> no current_container?");
		return -1;
	} else if (is_gold) {
		obj->owt = weight(obj);
	}

	if (obj->oartifact && !touch_artifact(obj, &youmonst)) return 0;

	if (obj->otyp == CORPSE) {
		if ((touch_petrifies(&mons[obj->corpsenm])) && !uarmg && !Stone_resistance) {
			if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
				display_nhwindow(WIN_MESSAGE, false);
			else {
				char kbuf[BUFSZ];

				strcpy(kbuf, an(corpse_xname(obj, true)));
				pline("Touching %s is a fatal mistake.", kbuf);
				strcpy(kbuf, an(killer_cxname(obj, true)));
				instapetrify(kbuf);
				return -1;
			}
		}
	}

	count = obj->quan;
	if ((res = lift_object(obj, current_container, &count, false)) <= 0)
		return res;

	if (obj->quan != count && obj->otyp != LOADSTONE)
		obj = splitobj(obj, count);

	/* Remove the object from the list. */
	obj_extract_self(obj);
	current_container->owt = weight(current_container);

	if (Icebox && !age_is_relative(obj)) {
		obj->age = monstermoves - obj->age; /* actual age */
		if (obj->otyp == CORPSE)
			start_corpse_timeout(obj);
	}
	/* simulated point of time */

	if (!obj->unpaid && !carried(current_container) &&
	    costly_spot(current_container->ox, current_container->oy)) {
		obj->ox = current_container->ox;
		obj->oy = current_container->oy;
		addtobill(obj, false, false, false);
	}
	if (is_pick(obj) && !obj->unpaid && *u.ushops && shop_keeper(*u.ushops))
		verbalize("You sneaky cad! Get out of here with that pick!");

	otmp = addinv(obj);
	loadlev = near_capacity();
	prinv(loadlev ?
		      (loadlev < MOD_ENCUMBER ?
			       "You have a little trouble removing" :
			       "You have much trouble removing") :
		      NULL,
	      otmp, count);

	if (is_gold) {
		bot(); /* update character's gold piece count immediately */
	}
	return 1;
}

/* an object inside a cursed bag of holding is being destroyed */
static long mbag_item_gone(int held, struct obj *item) {
	struct monst *shkp;
	long loss = 0L;

	if (item->dknown)
		pline("%s %s vanished!", Doname2(item), otense(item, "have"));
	else
		pline("You %s %s disappear!", Blind ? "notice" : "see", doname(item));

	if (*u.ushops && (shkp = shop_keeper(*u.ushops)) != 0) {
		if (held ? (boolean)item->unpaid : costly_spot(u.ux, u.uy))
			loss = stolen_value(item, u.ux, u.uy,
					    (boolean)shkp->mpeaceful, true, true);
	}
	/* [ALI] In Slash'EM we must delete the contents of containers before
	 * we call obj_extract_self() so that any indestructable items can
	 * migrate into the bag of holding. We are also constrained by the
	 * need to wait until after we have calculated any loss.
	 */
	if (Has_contents(item)) delete_contents(item);
	obj_extract_self(item);
	obfree(item, NULL);
	return loss;
}

static void observe_quantum_cat(struct obj *box) {
	static const char sc[] = "Schroedinger's Cat";
	struct obj *deadcat;
	struct monst *livecat;
	xchar ox, oy;

	box->spe = 0; /* box->owt will be updated below */
	if (get_obj_location(box, &ox, &oy, 0))
		box->ox = ox, box->oy = oy; /* in case it's being carried */

	/* this isn't really right, since any form of observation
	   (telepathic or monster/object/food detection) ought to
	   force the determination of alive vs dead state; but basing
	   it just on opening the box is much simpler to cope with */
	livecat = rn2(2) ? makemon(&mons[PM_HOUSECAT],
				   box->ox, box->oy, NO_MINVENT) :
			   NULL;
	if (livecat) {
		livecat->mpeaceful = 1;
		set_malign(livecat);
		if (!canspotmon(livecat))
			pline("You think something brushed your %s.", body_part(FOOT));
		else
			pline("%s inside the box is still alive!", Monnam(livecat));
		christen_monst(livecat, sc);
	} else {
		deadcat = mk_named_object(CORPSE, &mons[PM_HOUSECAT],
					  box->ox, box->oy, sc);
		if (deadcat) {
			obj_extract_self(deadcat);
			add_to_container(box, deadcat);
		}
		pline("The %s inside the box is dead!",
		      Hallucination ? rndmonnam() : "housecat");
	}
	box->owt = weight(box);
	return;
}

#undef Icebox

/* used by askchain() to check for magic bag explosion */
boolean container_gone(int (*fn)(struct obj *)) {
	/* result is only meaningful while use_container() is executing */
	return (fn == in_container || fn == out_container) && !current_container;
}

int use_container(struct obj **objp, int held) {
	struct obj *curr, *otmp, *obj = *objp;
	boolean one_by_one, allflag, quantum_cat = false,
				     loot_out = false, loot_in = false;
	char select[MAXOCLASSES + 1];
	char qbuf[BUFSZ], emptymsg[QBUFSZ], pbuf[QBUFSZ];
	long loss = 0L;
	int cnt = 0, used = 0, menu_on_request;

	emptymsg[0] = '\0';
	if (nohands(youmonst.data)) {
		pline("You have no hands!"); /* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		pline("You have no free %s.", body_part(HAND));
		return 0;
	}
	if (obj->olocked) {
		pline("%s to be locked.", Tobjnam(obj, "seem"));
		if (held) pline("You must put it down to unlock.");
		obj->lknown = true;
		return 0;
	} else if (obj->otrapped) {
		if (held) pline("You open %s...", the(xname(obj)));
		obj->lknown = true;
		chest_trap(obj, HAND, false);
		/* even if the trap fails, you've used up this turn */
		if (multi >= 0) { /* in case we didn't become paralyzed */
			nomul(-1);
			nomovemsg = "";
		}
		return 1;
	}

	obj->lknown = true;
	current_container = obj; /* for use by in/out_container */
	/* from here on out, all early returns go through containerdone */

	if (obj->spe == 1) {
		observe_quantum_cat(obj);
		used = 1;
		quantum_cat = true; /* for adjusting "it's empty" message */
	}
	/* [ALI] If a container vanishes which contains indestructible
	 * objects then these will be added to the magic bag. This makes
	 * it very hard to combine the count and vanish loops so we do
	 * them seperately.
	 */
	/* Sometimes toss objects if a cursed magic bag. */
	if (Is_mbag(obj) && obj->cursed) {
		for (curr = obj->cobj; curr; curr = otmp) {
			otmp = curr->nobj;
			if (!rn2(13) && !evades_destruction(curr)) {
				obj_extract_self(curr);
				obj->owt = weight(obj);
				loss += mbag_item_gone(held, curr);
				bot();
				used = 1;
			}
		}
	}
	encumber_msg();

	/* Count the number of contained objects. */
	for (curr = obj->cobj; curr; curr = curr->nobj)
		cnt++;

	if (loss) /* magic bag lost some shop goods */
		pline("You owe %ld %s for lost merchandise.", loss, currency(loss));

	if (!cnt) {
		sprintf(emptymsg, "%s is %sempty.", Yname2(obj),
			quantum_cat ? "now " : "");
		obj->cknown = true;
	}
	if (current_container->otyp == MEDICAL_KIT) {
		if (!cnt)
			pline("%s", emptymsg);
		else
			display_cinventory(current_container);
		return 0;
	}
	if (cnt || flags.menu_style == MENU_FULL) {
		strcpy(qbuf, "Do you want to take something out of ");
		sprintf(eos(qbuf), "%s?",
			safe_qbuf(qbuf, 1, yname(obj), ysimple_name(obj), "it"));
		if (flags.menu_style != MENU_TRADITIONAL) {
			if (flags.menu_style == MENU_FULL) {
				int t;
				char menuprompt[BUFSZ];
				boolean outokay = (cnt != 0),
					inokay = (invent != 0);

				if (!outokay && !inokay) {
					pline("%s", emptymsg);
					pline("You don't have anything to put in.");
					goto containerdone;
				}
				menuprompt[0] = '\0';
				if (!cnt) sprintf(menuprompt, "%s ", emptymsg);
				strcat(menuprompt, "Do what?");
				t = in_or_out_menu(menuprompt, current_container,
						   outokay, inokay);
				if (t <= 0) {
					//used = 0;
					goto containerdone;
				}
				loot_out = (t & 0x01) != 0;
				loot_in = (t & 0x02) != 0;
			} else { /* MENU_COMBINATION or MENU_PARTIAL */
				loot_out = (yn_function(qbuf, "ynq", 'n') == 'y');
			}
			if (loot_out) {
				add_valid_menu_class(0); /* reset */
				used |= menu_loot(0, current_container, false) > 0;
			}
		} else {
			/* traditional code */
ask_again2:
			menu_on_request = 0;
			add_valid_menu_class(0); /* reset */
			strcpy(pbuf, ":ynq");
			if (cnt) strcat(pbuf, "m");
			switch (yn_function(qbuf, pbuf, 'n')) {
				case ':':
					container_contents(current_container, false, false);
					goto ask_again2;
				case 'y':
					if (query_classes(select, &one_by_one, &allflag,
							  "take out", current_container->cobj,
							  false,
							  &menu_on_request)) {
						if (askchain((struct obj **)&current_container->cobj,
							     (one_by_one ? NULL : select),
							     allflag, out_container,
							     (int (*)(struct obj *))0,
							     0, "nodot"))
							used = 1;
					} else if (menu_on_request < 0) {
						used |= menu_loot(menu_on_request,
								  current_container, false) > 0;
					}
				fallthru;
				case 'n':
					break;
				case 'm':
					menu_on_request = -2; /* triggers ALL_CLASSES */
					used |= menu_loot(menu_on_request, current_container, false) > 0;
					break;
				case 'q':
				default:
					goto containerdone;
			}
		}
	} else {
		pline("%s", emptymsg); /* <whatever> is empty. */
	}

	if (!invent) {
		/* nothing to put in, but some feedback is necessary */
		pline("You don't have anything to put in.");
		goto containerdone;
	}
	if (flags.menu_style != MENU_FULL) {
		sprintf(qbuf, "Do you wish to put something in?");
		strcpy(pbuf, ynqchars);
		if (flags.menu_style == MENU_TRADITIONAL && invent && inv_cnt() > 0)
			strcat(pbuf, "m");
		switch (yn_function(qbuf, pbuf, 'n')) {
			case 'y':
				loot_in = true;
				break;
			case 'n':
				break;
			case 'm':
				add_valid_menu_class(0); /* reset */
				menu_on_request = -2;	 /* triggers ALL_CLASSES */
				used |= menu_loot(menu_on_request, current_container, true) > 0;
				break;
			case 'q':
			default:
				goto containerdone;
		}
	}
	/*
	 * Gone: being nice about only selecting food if we know we are
	 * putting things in an ice chest.
	 */
	if (loot_in) {
		add_valid_menu_class(0); /* reset */
		if (flags.menu_style != MENU_TRADITIONAL) {
			used |= menu_loot(0, current_container, true) > 0;
		} else {
			/* traditional code */
			menu_on_request = 0;
			if (query_classes(select, &one_by_one, &allflag, "put in", invent, false, &menu_on_request)) {
				askchain((struct obj **)&invent,
					 (one_by_one ? NULL : select), allflag,
					 in_container, ck_bag, 0, "nodot");
				used = 1;
			} else if (menu_on_request < 0) {
				used |= menu_loot(menu_on_request,
						  current_container, true) > 0;
			}
		}
	}

containerdone:
	if (used) obj->cknown = true;
	*objp = current_container; /* might have become null */
	current_container = NULL;  /* avoid hanging on to stale pointer */
	return used;
}

/* Loot a container (take things out, put things in), using a menu. */
static int menu_loot(int retry, struct obj *container, boolean put_in) {
	int n, i, n_looted = 0;
	boolean all_categories = true, loot_everything = false;
	char buf[BUFSZ];
	const char *takeout = "Take out", *putin = "Put in";
	struct obj *otmp, *otmp2;
	menu_item *pick_list;
	int mflags, res;
	long count;

	if (retry) {
		all_categories = (retry == -2);
	} else if (flags.menu_style == MENU_FULL) {
		all_categories = false;
		sprintf(buf, "%s what type of objects?", put_in ? putin : takeout);
		mflags = put_in ? ALL_TYPES | BUC_ALLBKNOWN | BUC_UNKNOWN :
				  ALL_TYPES | CHOOSE_ALL | BUC_ALLBKNOWN | BUC_UNKNOWN;
		n = query_category(buf, put_in ? invent : container->cobj,
				   mflags, &pick_list, PICK_ANY);
		if (!n) return 0;
		for (i = 0; i < n; i++) {
			if (pick_list[i].item.a_int == 'A')
				loot_everything = true;
			else if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
				all_categories = true;
			else
				add_valid_menu_class(pick_list[i].item.a_int);
		}
		free(pick_list);
	}

	if (loot_everything) {
		container->cknown = true;
		for (otmp = container->cobj; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			res = out_container(otmp);
			if (res < 0) break;
		}
	} else {
		mflags = INVORDER_SORT;
		if (put_in && flags.invlet_constant) mflags |= USE_INVLET;
		if (takeout) container->cknown = true;
		sprintf(buf, "%s what?", put_in ? putin : takeout);
		n = query_objlist(buf, put_in ? invent : container->cobj,
				  mflags, &pick_list, PICK_ANY,
				  all_categories ? allow_all : allow_category);
		if (n) {
			n_looted = n;
			for (i = 0; i < n; i++) {
				otmp = pick_list[i].item.a_obj;
				count = pick_list[i].count;
				if (count > 0 && count < otmp->quan) {
					otmp = splitobj(otmp, count);
					/* special split case also handled by askchain() */
				}
				res = put_in ? in_container(otmp) : out_container(otmp);
				if (res < 0) {
					if (!current_container) {
						/* otmp caused current_container to explode;
						   both are now gone */
						otmp = 0; /* and break loop */
					} else if (otmp && otmp != pick_list[i].item.a_obj) {
						/* split occurred, merge again */
						merged(&pick_list[i].item.a_obj, &otmp);
					}
					break;
				}
			}
			free(pick_list);
		}
	}
	return n_looted;
}

static int in_or_out_menu(const char *prompt, struct obj *obj, boolean outokay, boolean inokay) {
	winid win;
	anything any;
	menu_item *pick_list;
	char buf[BUFSZ];
	int n;
	const char *menuselector = iflags.lootabc ? "abc" : "oib";

	any.a_void = 0;
	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	if (outokay) {
		any.a_int = 1;
		sprintf(buf, "Take something out of %s", the(xname(obj)));
		add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	}
	menuselector++;
	if (inokay) {
		any.a_int = 2;
		sprintf(buf, "Put something into %s", the(xname(obj)));
		add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE, buf, MENU_UNSELECTED);
	}
	menuselector++;
	if (outokay && inokay) {
		any.a_int = 3;
		add_menu(win, NO_GLYPH, &any, *menuselector, 0, ATR_NONE,
			 "Both of the above", MENU_UNSELECTED);
	}
	end_menu(win, prompt);
	n = select_menu(win, PICK_ONE, &pick_list);
	destroy_nhwindow(win);
	if (n > 0) {
		n = pick_list[0].item.a_int;
		free(pick_list);
	}
	return n;
}

static const char tippables[] = { ALL_CLASSES, TOOL_CLASS, 0 };

/* #tip command -- empty container contents onto floor */
int dotip(void) {
	struct obj *cobj, *nobj;
	coord cc;
	int boxes;
	char c, buf[BUFSZ];
	const char *spillage = 0;

	/*
	 * doesn't require free hands;
	 * limbs are needed to tip floor containers
	 */

	/* at present, can only tip things at current spot, not adjacent ones */
	cc.x = u.ux, cc.y = u.uy;

	/* check floor container(s) first; at most one will be accessed */
	if ((boxes = container_at(cc.x, cc.y, true)) > 0) {
		if (flags.verbose)
			pline("There %s here.", (boxes > 1) ? "are containers" : "is a container");
		sprintf(buf, "You can't tip %s while carrying so much.", !flags.verbose ? "a container" : (boxes > 1) ? "one" : "it");
		if (!check_capacity(buf) && able_to_loot(cc.x, cc.y, false)) {
			for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = nobj) {
				nobj = cobj->nexthere;
				if (!Is_container(cobj)) continue;

				sprintf(buf, "There is %s here, tip it?", doname(cobj));
				c = ynq(buf);
				if (c == 'q') return 0;
				if (c == 'n') continue;

				tipcontainer(cobj);
				return 1;
			}   /* next cobj */
		}
	}

	/* either no floor container(s) or couldn't tip one or didn't tip any */
	cobj = getobj(tippables, "tip");
	if (!cobj) return 0;

	/* normal case */
	if (Is_container(cobj)) {
		tipcontainer(cobj);
		return 1;
	}
	/* assorted other cases */
	if (Is_candle(cobj) && cobj->lamplit) {
		/* note "wax" even for tallow candles to avoid giving away info */
		spillage = "wax";
	} else if ((cobj->otyp == POT_OIL && cobj->lamplit) ||
			(cobj->otyp == OIL_LAMP && cobj->age != 0L) ||
			(cobj->otyp == MAGIC_LAMP && cobj->spe != 0)) {
		spillage = "oil";
		/* todo: reduce potion's remaining burn timer or oil lamp's fuel */
	} else if (cobj->otyp == CAN_OF_GREASE && cobj->spe > 0) {
		/* charged consumed below */
		spillage = "grease";
	} else if (cobj->otyp == FOOD_RATION ||
			cobj->otyp == CRAM_RATION ||
			cobj->otyp == LEMBAS_WAFER) {
		spillage = "crumbs";
	} else if (cobj->oclass == VENOM_CLASS) {
		spillage = "venom";
	}
	if (spillage) {
		buf[0] = '\0';
		if (is_pool(u.ux, u.uy))
			sprintf(buf, " and gradually %s", vtense(spillage, "dissipate"));
		else if (is_lava(u.ux, u.uy))
			sprintf(buf, " and immediately %s away", vtense(spillage, "burn"));
		pline("Some %s %s onto the %s%s.",
				spillage, vtense(spillage, "spill"),
				surface(u.ux, u.uy), buf);
		/* shop usage message comes after the spill message */
		if (cobj->otyp == CAN_OF_GREASE && cobj->spe > 0) {
			check_unpaid(cobj);
			cobj->spe--;                /* doesn't affect cobj->owt */
		}
		/* something [useless] happened */
		return 1;
	}
	/* anything not covered yet */
	if (cobj->oclass == POTION_CLASS)  /* can't pour potions... */
		pline("The %s %s securely sealed.", xname(cobj), otense(cobj, "are"));
	else
		pline("Nothing happens.");
	return 0;
}

void tipcontainer(struct obj *box) {
	box->lknown = true;

	bool empty_it = false;

	if (box->olocked) {
		pline("It's locked.");
	} else if (box->otrapped) {
		// we're not reaching inside but we're still handling it...
		chest_trap(box, HAND, false);
		// even if the trap fails, you've used up this turn

		// in case we didn't become paralyzed
		if (multi >= 0) {
			nomul(-1);
			nomovemsg = "";
		}
	} else if (box->otyp == BAG_OF_TRICKS && box->spe > 0) {
		/* apply (not loot) this bag; uses up one charge */
		bagotricks(box);
		box->cknown = true;
	} else if (box->spe) {
		char yourbuf[BUFSZ];

		observe_quantum_cat(box);

		// if a live cat came out
		if (!Has_contents(box)) {
			// container type of 'large box' is inferred
			pline("%sbox is now empty.", Shk_Your(yourbuf, box));
		} else {
			empty_it = true;
		}

		box->cknown = true;
	} else if (!Has_contents(box)) {
		box->cknown = true;
		pline("It's empty.");
	} else {
		empty_it = true;
	}

	if (empty_it) {
		struct obj *otmp, *nobj;
		bool verbose = false,
			highdrop = !can_reach_floor(),
			altarizing = IS_ALTAR(levl[u.ux][u.uy].typ),
			cursed_mbag = (Is_mbag(box) && box->cursed);
		int held = carried(box);
		long loss = 0L;

		box->cknown = true;
		pline("%s out%c",
				box->cobj->nobj ? "Objects spill" : "An object spills",
				!(highdrop || altarizing) ? ':' : '.');

		for (otmp = box->cobj; otmp; otmp = nobj) {
			nobj = otmp->nobj;
			obj_extract_self(otmp);
			if (cursed_mbag && !rn2(13)) {
				loss += mbag_item_gone(held, otmp);
				/* abbreviated drop format is no longer appropriate */
				verbose = true;
			} else if (highdrop) {
				/* might break or fall down stairs; handles altars itself */
				hitfloor(otmp);
			} else {
				if (altarizing)
					doaltarobj(otmp);
				else if (verbose)
					pline("%s %s to the %s.", Doname2(otmp),
							otense(otmp, "drop"), surface(u.ux, u.uy));
				else
					pline("%s%c", doname(otmp), nobj ? ',' : '.');
				dropy(otmp);
			}
		}
		if (loss) // magic bag lost some shop goods
			pline("You owe %ld %s for lost merchandise.", loss, currency(loss));

		// mbag_item_gone() doesn't update this
		box->owt = weight(box);
	}
}

/*pickup.c*/
