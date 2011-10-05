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

#include "mpdwrapper.h"

/* returns a song's filename, which is ripped out of
 * it's full path
 */
char *cookie_filename(char *path)
{
    char *file;
    file = strrchr(path, '/');
    return (NULL != file) ? ++file : path;
}

/* formats the time, in the format 12:34 */
char *cookie_format_time(int seconds)
{
    char *string = calloc(1, 6);
    snprintf(string, 6, "%02d:%02d", seconds / 60, seconds % 60);
    return string;
}

/* extracts the song name for use in the cookies now playing
 * label, in the format "<b>song</b>\nartist"
 */
char *cookie_song_name(mpd_Song * song)
{
    char *tmp = NULL;
    char *songstr = NULL;
    char *artist = NULL;
    char *title = NULL;
    int len;

    /* Dirty hack, but meh. */
#define UNKNOWN_STR	_("Unknown")

    if (song->artist == NULL && song->title == NULL) {
        tmp = g_markup_escape_text(cookie_filename(song->file),
            strlen(cookie_filename(song->file)));
        len = strlen(tmp);
        songstr = calloc(1, len + 9);
        snprintf(songstr, len + 9, "<b>%s</b>\n", tmp);
        free(tmp);
        return songstr;
    } else {
        if (song->artist != NULL) {
            artist = strdup(song->artist);
        } else {
            artist = strdup(UNKNOWN_STR);
        }

        if (song->title != NULL) {
            title = strdup(song->title);
        } else {
            title = strdup(UNKNOWN_STR);
        }
        len = strlen(g_markup_escape_text(title,
                strlen(title))) +
            strlen(g_markup_escape_text(artist, strlen(artist)));
        songstr = calloc(1, len + 9);
        snprintf(songstr, len + 9,
            "<b>%s</b>\n%s", g_markup_escape_text(title, strlen(title)),
            g_markup_escape_text(artist, strlen(artist)));
        free(artist);
        free(title);
        return songstr;
    }

    return _("<b>Title</b>\nArtist");
}

/* initial setup of the main cookies struct and
 * all of the information inside of its members
 */
void cookie_init(struct cookie_main *cookie)
{
    char *time = NULL;
    char *song_name_str = NULL;
    char *elapsed_str = NULL;
    char *total_str = NULL;

    int state, elapsed, total;

    cookie_refresh_playlist(cookie);

    state = mpd_ob_player_get_state(cookie->mpd);

    if (state == MPD_OB_PLAYER_PLAY || state == MPD_OB_PLAYER_PAUSE) {

        cookie->current_song =
            mpd_songDup(mpd_ob_playlist_get_current_song(cookie->mpd));

        elapsed = mpd_ob_status_get_elapsed_song_time(cookie->mpd);
        total = mpd_ob_status_get_total_song_time(cookie->mpd);

        /* avoid memory leak by pointing to this memory */

        song_name_str = cookie_song_name(cookie->current_song);

        gtk_label_set_markup(GTK_LABEL(cookie->song), song_name_str);

        /* now free the memory that cookie_song_name allocated */

        free(song_name_str);

        time = calloc(1, 13);
        if (NULL == time) {
            fprintf(stderr,
                _("error allocating memory for time string!\n"));
            _exit(1);
        }

        /* avoid leak */

        elapsed_str = cookie_format_time(elapsed);
        total_str = cookie_format_time(total);

        snprintf(time, 13, "%s/%s", elapsed_str, total_str);
        gtk_label_set_text(GTK_LABEL(cookie->time), time);

        /* free the memory that we allocated as well as that which was allocated by other funcs */

        free(time);
        free(total_str);
        free(elapsed_str);
    }
}

/* the handler for a song change, changes the name of the top
 * now playing label, and updates the time and location of the
 * currently playing song image
 */
void *cookie_song_changed(MpdObj * mpd, int old_song_id,
    int new_song_id, void *data)
{

    char *song_str = NULL;
    mpd_Song *song;
    struct cookie_main *cookie = (struct cookie_main *)data;
    int state = mpd_ob_player_get_state(cookie->mpd);

    if (state == MPD_OB_PLAYER_PLAY || state == MPD_OB_PLAYER_PAUSE) {

        song = mpd_ob_playlist_get_song(cookie->mpd, new_song_id);

        cookie_playlist_update_image(cookie, song);

        song_str = cookie_song_name(song);

        gtk_label_set_markup(GTK_LABEL(cookie->song), song_str);

        free(song_str);

        cookie->current_song = mpd_songDup(song);
    } else if (state == MPD_OB_PLAYER_STOP) {
        cookie_playlist_update_image(cookie, NULL);
        gtk_label_set_markup(GTK_LABEL(cookie->song),
            _("<b>Cookies!</b>\nStopped."));
        gtk_label_set_text(GTK_LABEL(cookie->time), "00:00/00:00");
    }

    return NULL;
}

/* updates everything, run twice a second on a timeout */
gboolean cookie_update_info(struct cookie_main * cookie)
{
    char *time = NULL;
    char *elapsed_str = NULL;
    char *total_str = NULL;
    char *song_name;

    int state, elapsed, total;
    int old, new;
    mpd_Song *song = NULL;

    mpd_ob_status_queue_update(cookie->mpd);
    state = mpd_ob_player_get_state(cookie->mpd);

    old = mpd_ob_status_get_volume(cookie->mpd);
    new = gtk_range_get_value(GTK_RANGE(cookie->volume->bar));
    if (old != new) {
        gtk_range_set_value(GTK_RANGE(cookie->volume->bar), old);
    }

    elapsed = mpd_ob_status_get_elapsed_song_time(cookie->mpd);
    total = mpd_ob_status_get_total_song_time(cookie->mpd);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cookie->random->
                button)) != mpd_ob_player_get_random(cookie->mpd)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookie->random->
                button), mpd_ob_player_get_random(cookie->mpd));
        mpd_ob_player_set_random(cookie->mpd,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cookie->random->
                    button)));
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cookie->repeat->
                button)) != mpd_ob_player_get_repeat(cookie->mpd)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookie->repeat->
                button), mpd_ob_player_get_repeat(cookie->mpd));
        mpd_ob_player_set_repeat(cookie->mpd,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cookie->repeat->
                    button)));
    }

    if (state == MPD_OB_PLAYER_PLAY || state == MPD_OB_PLAYER_PAUSE) {
        song = mpd_ob_playlist_get_current_song(cookie->mpd);

        /*if (song != NULL) {
           if (cookie->current_song == NULL) {
           gtk_label_set_markup (GTK_LABEL (cookie->song), 
           cookie_song_name (song));
           cookie->current_song = mpd_songDup (song);
           }
           else if (cookie->current_song->pos != song->pos) {
           gtk_label_set_markup (GTK_LABEL (cookie->song), 
           cookie_song_name (song));
           cookie->current_song = mpd_songDup (song);
           }
           } */

        song_name = cookie_song_name(song);

        gtk_label_set_markup(GTK_LABEL(cookie->song), song_name);

        free(song_name);

        time = calloc(1, 13);
        if (NULL == time) {
            fprintf(stderr,
                _("error allocating memory for time string!\n"));
            _exit(1);
        }
        if (!cookie->seek) {
            elapsed_str = cookie_format_time(elapsed);
            total_str = cookie_format_time(total);

            snprintf(time, 13, "%s/%s", elapsed_str, total_str);
            gtk_label_set_text(GTK_LABEL(cookie->time), time);

            free(total_str);
            free(elapsed_str);

            gtk_adjustment_set_value(GTK_ADJUSTMENT(cookie->adj),
                (float)elapsed / total * 100);

        }

        (state == MPD_OB_PLAYER_PLAY) ?
            gtk_image_set_from_stock(GTK_IMAGE(cookie->play->img),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_SMALL_TOOLBAR) :
            gtk_image_set_from_stock(GTK_IMAGE(cookie->play->img),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR);

        free(time);

    }
    /*else if (state == MPD_OB_PLAYER_STOP) {
       gtk_label_set_markup (GTK_LABEL (cookie->song), 
       "<b>Cookies!</b>\nStopped.");
       gtk_label_set_text (GTK_LABEL (cookie->time), "00:00/00:00");
       if (cookie->current_song != NULL) {
       cookie_playlist_update_image (cookie, NULL);
       cookie->current_song = NULL;
       }
       } */

    return TRUE;
}

/* toggles between pausing and playing, and swaps the image */
void cookie_pause_or_play(struct cookie_main *cookie)
{
    int state = mpd_ob_player_get_state(cookie->mpd);

    if (state == MPD_OB_PLAYER_PAUSE || state == MPD_OB_PLAYER_STOP) {
        mpd_ob_player_play(cookie->mpd);
        gtk_image_set_from_stock(GTK_IMAGE(cookie->play->img),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_SMALL_TOOLBAR);
    } else if (state == MPD_OB_PLAYER_PLAY) {
        mpd_ob_player_pause(cookie->mpd);
        gtk_image_set_from_stock(GTK_IMAGE(cookie->play->img),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR);
    }
}

/* stops play, toggles the paused/playing image */
void cookie_stop(struct cookie_main *cookie)
{
    int state = mpd_ob_player_get_state(cookie->mpd);

    if (state != MPD_OB_PLAYER_STOP) {
        mpd_ob_player_stop(cookie->mpd);
        gtk_image_set_from_stock(GTK_IMAGE(cookie->play->img),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_label_set_markup(GTK_LABEL(cookie->song),
            _("<b>Cookies!</b>\nStopped."));
        gtk_label_set_text(GTK_LABEL(cookie->time), "00:00/00:00");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(cookie->adj), 0);
    }
}

/* toggles on/off random playing of songs */
void cookie_toggle_random(struct cookie_main *cookie)
{
    if (TRUE == mpd_ob_player_get_random(cookie->mpd)) {
        mpd_ob_player_set_random(cookie->mpd, FALSE);
    } else {
        mpd_ob_player_set_random(cookie->mpd, TRUE);
    }
}

/* toggles on/off cycling through the playlist */
void cookie_toggle_repeat(struct cookie_main *cookie)
{
    if (TRUE == mpd_ob_player_get_repeat(cookie->mpd)) {
        mpd_ob_player_set_repeat(cookie->mpd, FALSE);
    } else {
        mpd_ob_player_set_repeat(cookie->mpd, TRUE);
    }
}

/* clears the playlist */
void cookie_clear_playlist(struct cookie_main *cookie)
{
    mpd_ob_playlist_clear(cookie->mpd);
}

/* shuffles the playlist */
void cookie_shuffle_playlist(struct cookie_main *cookie)
{
    mpd_ob_playlist_shuffle(cookie->mpd);
}

/* connects to mpd */
int cookie_connect(struct cookie_main *cookie)
{
    if (!mpd_ob_check_connected(cookie->mpd)) {
        return (mpd_ob_connect(cookie->mpd) == -1) ? 1 : 0;
    }
    return 0;
}
