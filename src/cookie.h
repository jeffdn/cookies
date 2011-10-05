/*
 * cookies
 *
 * Copyright (c) 2005 Javeed Shaikh and Jeff Nettleton
 * Copyright (c) 2005 Mikkel Krautz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#ifndef _COOKIES_H_
#    define _COOKIES_H_

#    include <stdio.h>
#    include <stdlib.h>
#    include <unistd.h>
#    include <glib/gmain.h>
#    include <gtk/gtk.h>
#    include <libmpd/libmpd.h>
#    include <libintl.h>

#    include "config.h"

struct cookie_vol {
    GtkWidget *bar;
    GtkObject *adj;
};

struct cookie_button {
    GtkWidget *button;
    GtkWidget *img;
    GtkTooltips *tip;
};

struct cookie_info {
    GtkWidget *main;
    GtkWidget *vbox;

    /* label widgets */
    GtkWidget *file;
    GtkWidget *date;
    GtkWidget *album;
    GtkWidget *title_hbox;
    GtkWidget *title;
    GtkWidget *genre;
    GtkWidget *artist;
};

struct cookie_main {
    /* buttons! */
    struct cookie_button *play;
    struct cookie_button *stop;
    struct cookie_button *prev;
    struct cookie_button *next;
    struct cookie_button *random;
    struct cookie_button *repeat;
    struct cookie_button *snap;
    struct cookie_button *clear;
    struct cookie_button *showlist;
    struct cookie_button *shuffle;

    /* song info thingies */
    GtkWidget *song;
    GtkWidget *time;
    gboolean seek;

    /* window stuff! */
    GtkWidget *main;
    GdkGeometry window_hints;
    GdkGeometry playlist_hints;
    GtkWidget *progress;

    GtkWidget *main_vbox;
    GtkWidget *top_hbox;
    GtkWidget *bot_hbox;
    GtkWidget *exp_vbox;
    GtkWidget *exp_hbox;

    GtkWidget *scrolled;
    GtkObject *adj;

    struct cookie_vol *volume;
    GdkPixbuf *pause_pb;
    guint timeout_id;

    /* playlist stuff */
    GtkListStore *store;
    GtkCellRenderer *rend;
    GtkCellRenderer *rend_img;
    GtkWidget *view;
    GtkTreeViewColumn *col_img;
    GtkTreeViewColumn *col_song;
    GtkTreeViewColumn *col_time;
    GdkPixbuf *current_img;

    /* mpd stuff! */
    MpdObj *mpd;
    mpd_Song *current_song;
};

struct secret_args {
    struct cookie_main *cookie;
    GtkListStore *store;
    GtkTreeSelection *selection;
    MpdObj *mpd;
};

/* For internationalisation.
 * Use this for strings within cookies that should be translated into
 * other langauges than just english. */
#    define	_(str)	gettext(str)

#endif
