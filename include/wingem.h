/*	SCCS Id: @(#)wingem.h	3.4	1999/12/10	*/
/* Copyright (c) Christian Bressler, 1999				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINGEM_H
#define WINGEM_H

#define E extern

/* menu structure */
typedef struct Gmi{
	struct Gmi *Gmi_next;
	int Gmi_glyph;
	long Gmi_identifier;
	char Gmi_accelerator, Gmi_groupacc;
	int Gmi_attr;
	char *Gmi_str;
	long Gmi_count;
	int Gmi_selected;
} Gem_menu_item;

#define MAXWIN 20		/* maximum number of windows, cop-out */

extern struct window_procs Gem_procs;

/* ### wingem1.c ### */
#ifdef CLIPPING
E void setclipped(void);
#endif
E void FDECL(docorner, (int, int));
E void end_glyphout(void);
E void FDECL(g_putch, (int));
E void win_Gem_init(void);
E int mar_gem_init(void);
E char mar_ask_class(void);
E char * mar_ask_name(void);
E int FDECL(mar_create_window, (int));
E void FDECL(mar_destroy_nhwindow, (int));
E void FDECL(mar_print_glyph, (int, int, int, int));
E void FDECL(mar_print_line, (int, int, int, char *));
E void FDECL(mar_set_message, (char *, char *, char *));
E Gem_menu_item *mar_hol_inv(void);
E void FDECL(mar_set_menu_type,(int));
E void mar_reverse_menu(void);
E void FDECL(mar_set_menu_title, (const char *));
E void mar_set_accelerators(void);
E void FDECL(mar_add_menu, (winid, Gem_menu_item *));
E void FDECL(mar_change_menu_2_text, (winid));
E void FDECL(mar_add_message, (const char *));
E void mar_status_dirty(void);
E int FDECL(mar_hol_win_type, (int));
E void mar_clear_messagewin(void);
E void FDECL(mar_set_no_glyph, (int));
E void mar_map_curs_weiter(void);

/* external declarations */
E void FDECL(Gem_init_nhwindows, (int *, char **));
E void Gem_player_selection(void);
E void Gem_askname(void);
E void Gem_get_nh_event(void);
E void FDECL(Gem_exit_nhwindows, (const char *));
E void FDECL(Gem_suspend_nhwindows, (const char *));
E void Gem_resume_nhwindows(void);
E winid FDECL(Gem_create_nhwindow, (int));
E void FDECL(Gem_clear_nhwindow, (winid));
E void FDECL(Gem_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(Gem_dismiss_nhwindow, (winid));
E void FDECL(Gem_destroy_nhwindow, (winid));
E void FDECL(Gem_curs, (winid,int,int));
E void FDECL(Gem_putstr, (winid, int, const char *));
E void FDECL(Gem_display_file, (const char *, BOOLEAN_P));
E void FDECL(Gem_start_menu, (winid));
E void FDECL(Gem_add_menu, (winid,int,const ANY_P *,
			CHAR_P,CHAR_P,int,const char *, BOOLEAN_P));
E void FDECL(Gem_end_menu, (winid, const char *));
E int FDECL(Gem_select_menu, (winid, int, MENU_ITEM_P **));
E char FDECL(Gem_message_menu, (CHAR_P,int,const char *));
E void Gem_update_inventory(void);
E void Gem_mark_synch(void);
E void Gem_wait_synch(void);
#ifdef CLIPPING
E void FDECL(Gem_cliparound, (int, int));
#endif
#ifdef POSITIONBAR
E void FDECL(Gem_update_positionbar, (char *));
#endif
E void FDECL(Gem_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(Gem_raw_print, (const char *));
E void FDECL(Gem_raw_print_bold, (const char *));
E int Gem_nhgetch(void);
E int FDECL(Gem_nh_poskey, (int *, int *, int *));
E void Gem_nhbell(void);
E int Gem_doprev_message(void);
E char FDECL(Gem_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(Gem_getlin, (const char *,char *));
E int Gem_get_ext_cmd(void);
E void FDECL(Gem_number_pad, (int));
E void Gem_delay_output(void);
#ifdef CHANGE_COLOR
E void FDECL(Gem_change_color,(int color,long rgb,int reverse));
E char * Gem_get_color_string(void);
#endif

/* other defs that really should go away (they're tty specific) */
E void Gem_start_screen(void);
E void Gem_end_screen(void);

E void FDECL(genl_outrip, (winid,int));

#undef E

#endif /* WINGEM_H */
