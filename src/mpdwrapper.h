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

#ifndef _MPDWRAPPER_H_
#    define _MPDWRAPPER_H_

#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#    include <unistd.h>
#    include <gtk/gtk.h>
#    include <libmpd/libmpd.h>

#    include "window.h"
#    include "cookie.h"

int cookie_connect(struct cookie_main *);
char *cookie_filename(char *);
char *cookie_song_name(mpd_Song *);
char *cookie_format_time(int);

void cookie_init(struct cookie_main *);
void *cookie_song_changed(MpdObj *, int, int, void *);

/* song control functions */
void cookie_pause_or_play(struct cookie_main *);
void cookie_stop(struct cookie_main *);
gboolean cookie_update_info(struct cookie_main *);

/* playlist related functions */
void cookie_toggle_random(struct cookie_main *);
void cookie_toggle_repeat(struct cookie_main *);
void cookie_clear_playlist(struct cookie_main *);
void cookie_shuffle_playlist(struct cookie_main *);

#endif
