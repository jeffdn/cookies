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

#ifndef _TREE_H_
#    define _TREE_H_

#    include <stdio.h>
#    include <stdlib.h>
#    include <gtk/gtk.h>

#    include "cookie.h"
#    include "mpdwrapper.h"

enum {
    COL_POS,
    COL_IMG,
    COL_SONG,
    COL_TIME
};

/*
#define COL_POS  0
#define COL_SONG 1
#define COL_TIME 2
*/

void cookie_playlist_update_image(struct cookie_main *, mpd_Song *);
void cookie_playlist_add_song(struct cookie_main *, mpd_Song *);
gboolean cookie_snap_playlist(struct cookie_main *);
void cookie_create_playlist(struct cookie_main *);
void cookie_refresh_playlist(struct cookie_main *);
void cookie_search_refresh_playlist(struct cookie_main *, MpdData *);
void cookie_popup_menu_play(struct secret_args *);
void cookie_popup_menu_show(struct secret_args *);

#endif
