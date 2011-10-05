/*
 * cookies
 *
 * Copyright (c) 2005 Javeed Shaikh and Jeff Nettleton
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

#ifndef _WINDOW_H_
#    define _WINDOW_H_

#    include <stdio.h>
#    include <stdlib.h>
#    include <unistd.h>
#    include <gtk/gtk.h>

#    include "cookie.h"
#    include "mpdwrapper.h"
#    include "tree.h"

void cookie_seek(struct cookie_main *);
gboolean cookie_seek_start(struct cookie_main *);
gboolean cookie_seek_end(struct cookie_main *);
gboolean cookie_seek_time_update(struct cookie_main *);
gboolean cookie_volume_update(struct cookie_main *);
void cookie_main_window_new(struct cookie_main *);
void cookie_destroy_button(struct cookie_button *);
struct cookie_button *cookie_create_button(char *, int, int,
    GCallback, gpointer data, char *);
void cookie_playlist_toggle(struct cookie_main *);
void cookies_quit(struct cookie_main *);
struct cookie_vol *cookie_volume_set(void);
GtkWidget *cookie_info_label_new(void);
void cookie_info_destroy(struct cookie_info *);
void cookie_info_window(mpd_Song *);

#endif
