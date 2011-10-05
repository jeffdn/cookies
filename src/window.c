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

#include "window.h"

/* seeks through a song */
void cookie_seek(struct cookie_main *cookie)
{
    int state;
    gdouble value;

    state = mpd_ob_player_get_state(cookie->mpd);

    if (state == MPD_OB_PLAYER_PLAY || state == MPD_OB_PLAYER_PAUSE) {

        value = gtk_range_get_value(GTK_RANGE(cookie->progress));

        mpd_ob_player_seek(cookie->mpd,
            (int)((value / 100) * cookie->current_song->time));
    }

}

/* stops the time from changing, because that causes a 
 * segfault mid-seek
 */
gboolean cookie_seek_start(struct cookie_main *cookie)
{
    cookie->seek = TRUE;
    return FALSE;
}

/* restarts the time ticking */
gboolean cookie_seek_end(struct cookie_main * cookie)
{
    cookie_seek(cookie);
    cookie->seek = FALSE;
    return FALSE;
}

/* updates the time while the song is being seeked through */
gboolean cookie_seek_time_update(struct cookie_main * cookie)
{
    char *time = NULL;
    char *elapsed_str = NULL;
    char *total_str = NULL;

    int elapsed, total;

    elapsed = mpd_ob_status_get_elapsed_song_time(cookie->mpd);
    total = mpd_ob_status_get_total_song_time(cookie->mpd);

    if (cookie->seek) {
        time = calloc(1, 13);

        elapsed_str =
            cookie_format_time((int)(total *
                (gtk_range_get_value(GTK_RANGE(cookie->progress)) / 100)));
        total_str = cookie_format_time(total);

        snprintf(time, 13, "%s/%s", elapsed_str, total_str);
        gtk_label_set_text(GTK_LABEL(cookie->time), time);

        free(time);
        free(total_str);
        free(elapsed_str);
    }
    return FALSE;
}

/* updates the volume as the bar moves */
gboolean cookie_volume_update(struct cookie_main * cookie)
{
    mpd_ob_status_set_volume(cookie->mpd,
        gtk_range_get_value(GTK_RANGE(cookie->volume->bar)));
    return FALSE;
}

/* creates the main window */
void cookie_main_window_new(struct cookie_main *cookie)
{
    GError **error = NULL;

    /* create the window, don't allow resizing. */
    cookie->main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(cookie->main), FALSE);

    gtk_container_set_border_width(GTK_CONTAINER(cookie->main), 5);
    g_signal_connect(G_OBJECT(cookie->main), "destroy",
        G_CALLBACK(cookies_quit), (struct cookie_main *)cookie);
    gtk_window_set_title(GTK_WINDOW(cookie->main), "cookies");
    gtk_window_set_icon_from_file((GtkWindow *) cookie->main,
        COOKIE_WINDOW_IMG, error);

    /* set up the volume thing and playlist */
    cookie->volume = cookie_volume_set();

    /* create the main storage container for all our boxes. */
    cookie->main_vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(cookie->main), cookie->main_vbox);
    gtk_widget_show(cookie->main_vbox);

    /* the box that stores the artist, title, and time 
     * elapsed widgets. */
    cookie->top_hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(cookie->main_vbox), cookie->top_hbox,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->top_hbox);

    /* create the title/artist display label. */
    cookie->song = gtk_label_new("");
    gtk_label_set_width_chars(GTK_LABEL(cookie->song), 40);
    gtk_misc_set_alignment(GTK_MISC(cookie->song), 0, .5);
    gtk_label_set_ellipsize(GTK_LABEL(cookie->song), PANGO_ELLIPSIZE_END);
    gtk_label_set_markup(GTK_LABEL(cookie->song),
        _("<b>Cookies!</b>\nStopped."));
    gtk_box_pack_start(GTK_BOX(cookie->top_hbox), cookie->song,
        TRUE, TRUE, 0);
    gtk_widget_show(cookie->song);

    /* create the time display label. */
    cookie->time = gtk_label_new("00:00/00:00");
    gtk_box_pack_end(GTK_BOX(cookie->top_hbox), cookie->time,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->time);

    /* the adjustment widget for the progress bar, as well
     * as the progress bar widget. */
    cookie->adj = gtk_adjustment_new(0, 0.0, 101.0, 0.1, 0, 0);
    cookie->progress = gtk_hscale_new(GTK_ADJUSTMENT(cookie->adj));
    gtk_widget_set_size_request(GTK_WIDGET(cookie->progress), 350, -1);
    gtk_range_set_update_policy(GTK_RANGE(cookie->progress),
        GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits(GTK_SCALE(cookie->progress), 0);
    gtk_scale_set_draw_value(GTK_SCALE(cookie->progress), FALSE);
    gtk_box_pack_start(GTK_BOX(cookie->main_vbox), cookie->progress,
        FALSE, FALSE, 0);

    g_signal_connect_swapped(G_OBJECT(cookie->progress),
        "button_press_event", G_CALLBACK(cookie_seek_start),
        (struct cookie_main *)cookie);

    g_signal_connect_swapped(G_OBJECT(cookie->progress),
        "button_release_event", G_CALLBACK(cookie_seek_end),
        (struct cookie_main *)cookie);

    /* for max smoothfarmance */

    g_signal_connect_swapped(G_OBJECT(cookie->progress), "value_changed",
        G_CALLBACK(cookie_seek_time_update), (struct cookie_main *)cookie);
    gtk_widget_show(cookie->progress);

    g_object_set(cookie->progress, "can-focus", FALSE, NULL);

    /* the box with all the buttons */
    cookie->bot_hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(cookie->main_vbox), cookie->bot_hbox,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->bot_hbox);

    /* XXX: BUTTONS NOTICE!!! :XXX */
    /* I'm temporarily using the stock icons, because using
     * the current temporary icon is fugly. -jeff */

    /* the previous song button. */
    cookie->prev =
        cookie_create_button(GTK_STOCK_MEDIA_PREVIOUS, TRUE, FALSE,
        G_CALLBACK(mpd_ob_player_prev), (MpdObj *) cookie->mpd,
        _("change to the previous song"));
    gtk_box_pack_start(GTK_BOX(cookie->bot_hbox), cookie->prev->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->prev->button);

    /* the play or pause button. */
    cookie->play = cookie_create_button(GTK_STOCK_MEDIA_PLAY, TRUE, FALSE,
        G_CALLBACK(cookie_pause_or_play), (struct cookie_main *)cookie,
        _("toggle paused or playing mode"));
    gtk_box_pack_start(GTK_BOX(cookie->bot_hbox), cookie->play->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->play->button);

    /* the stop button. */
    cookie->stop = cookie_create_button(GTK_STOCK_MEDIA_STOP, TRUE, FALSE,
        G_CALLBACK(cookie_stop), (struct cookie_main *)cookie,
        _("stop the currently playing song"));
    gtk_box_pack_start(GTK_BOX(cookie->bot_hbox), cookie->stop->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->stop->button);

    /* the next song button. */
    cookie->next = cookie_create_button(GTK_STOCK_MEDIA_NEXT, TRUE, FALSE,
        G_CALLBACK(mpd_ob_player_next), (MpdObj *) cookie->mpd,
        _("change to the next song"));
    gtk_box_pack_start(GTK_BOX(cookie->bot_hbox), cookie->next->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->next->button);

    /* volume adjuster */
    cookie->volume->adj = gtk_adjustment_new(0, 0.0, 100.0, 0.1, 0, 0);
    cookie->volume->bar =
        gtk_hscale_new(GTK_ADJUSTMENT(cookie->volume->adj));
    gtk_widget_set_size_request(GTK_WIDGET(cookie->volume->bar), 125, -1);
    gtk_range_set_update_policy(GTK_RANGE(cookie->volume->bar),
        GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_draw_value(GTK_SCALE(cookie->volume->bar), FALSE);
    gtk_box_pack_end(GTK_BOX(cookie->bot_hbox), cookie->volume->bar,
        FALSE, FALSE, 1);
    g_signal_connect_swapped(G_OBJECT(cookie->volume->bar),
        "value_changed", G_CALLBACK(cookie_volume_update),
        (struct cookie_main *)cookie);
    gtk_widget_show(cookie->volume->bar);

    g_object_set(cookie->volume->bar, "can-focus", FALSE, NULL);

    cookie->showlist =
        cookie_create_button(GTK_STOCK_JUSTIFY_FILL, TRUE, TRUE,
        G_CALLBACK(cookie_playlist_toggle), (struct cookie_main *)cookie,
        _("show the playlist"));
    gtk_box_pack_start(GTK_BOX(cookie->bot_hbox), cookie->showlist->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->showlist->button);

    gtk_window_get_size(GTK_WINDOW(cookie->main),
        &cookie->window_hints.max_width, &cookie->window_hints.max_height);

/*	gtk_window_set_geometry_hints (GTK_WINDOW (cookie->main), cookie->main, &cookie->window_hints, GDK_HINT_MAX_SIZE);*/

    /* vbox for the playlist expander */
    cookie->exp_vbox = gtk_vbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(cookie->main_vbox), cookie->exp_vbox,
        TRUE, TRUE, 0);
    gtk_widget_show(cookie->exp_vbox);

    /* playlist shit */
    cookie->scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(cookie->exp_vbox),
        cookie->scrolled, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(cookie->scrolled),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(cookie->
            scrolled), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_size_request(cookie->scrolled, 340, 320);
    gtk_widget_show(cookie->scrolled);

    /* the view! */
    cookie_create_playlist(cookie);
    gtk_container_add(GTK_CONTAINER(cookie->scrolled), cookie->view);
    cookie->current_img =
        gdk_pixbuf_new_from_file(COOKIE_CURRENT_IMG, error);
    gtk_widget_show(cookie->view);
    gtk_widget_grab_focus(cookie->view);

    /* repeat and random buttons + hbox */
    cookie->exp_hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_end(GTK_BOX(cookie->exp_vbox), cookie->exp_hbox,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->exp_hbox);

    cookie->repeat = cookie_create_button(GTK_STOCK_REFRESH, TRUE, TRUE,
        G_CALLBACK(cookie_toggle_repeat), (struct cookie_main *)cookie,
        _("toggle repeat"));
    gtk_box_pack_end(GTK_BOX(cookie->exp_hbox), cookie->repeat->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->repeat->button);

    cookie->random = cookie_create_button(GTK_STOCK_INDEX, TRUE, TRUE,
        G_CALLBACK(cookie_toggle_random), (struct cookie_main *)cookie,
        _("toggle random song playing"));
    gtk_box_pack_end(GTK_BOX(cookie->exp_hbox), cookie->random->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->random->button);

    cookie->snap = cookie_create_button(GTK_STOCK_ZOOM_FIT, TRUE, FALSE,
        G_CALLBACK(cookie_snap_playlist), (struct cookie_main *)cookie,
        _("jump to current song"));
    gtk_box_pack_start(GTK_BOX(cookie->exp_hbox), cookie->snap->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->snap->button);

    cookie->clear = cookie_create_button(GTK_STOCK_STOP, TRUE, FALSE,
        G_CALLBACK(cookie_clear_playlist), (struct cookie_main *)cookie,
        _("clear the playlist"));
    gtk_box_pack_start(GTK_BOX(cookie->exp_hbox), cookie->clear->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->clear->button);

    cookie->shuffle = cookie_create_button(GTK_STOCK_INDENT, TRUE, FALSE,
        G_CALLBACK(cookie_shuffle_playlist), (struct cookie_main *)cookie,
        _("shuffle the playlist"));
    gtk_box_pack_start(GTK_BOX(cookie->exp_hbox), cookie->shuffle->button,
        FALSE, FALSE, 0);
    gtk_widget_show(cookie->shuffle->button);
}

/* the quitting function, needs alot of work */
void cookies_quit(struct cookie_main *cookie)
{
    /* XXX: There are tons of segfaults here, owned. */

    /* causing a damn segmentation fault, we'll have to fix
     * if (cookie->mpd != NULL) mpd_ob_free (cookie->mpd);
     */

    /*g_source_remove (cookie->timeout_id); */

    /* destroy the buttons */
    cookie_destroy_button(cookie->play);
    cookie_destroy_button(cookie->stop);
    cookie_destroy_button(cookie->next);
    cookie_destroy_button(cookie->prev);
    cookie_destroy_button(cookie->snap);
    cookie_destroy_button(cookie->clear);
    cookie_destroy_button(cookie->random);
    cookie_destroy_button(cookie->repeat);
    cookie_destroy_button(cookie->showlist);

    /* destroy the current song */
    if (cookie->current_song)
        mpd_freeSong(cookie->current_song);

    free(cookie);
    _exit(0);
}

/* creates a volume setter */
struct cookie_vol *cookie_volume_set(void)
{
    struct cookie_vol *volume = calloc(1, sizeof(struct cookie_vol));

    if (NULL == volume) {
        fprintf(stderr, _("error allocating memory for volume window\n"));
        _exit(1);
    }

    return volume;
}

/* a simple button maker, for less typing and more sex! */
struct cookie_button *cookie_create_button(char *label, int img, int tog,
    GCallback callback, gpointer data, char *tip)
{
    struct cookie_button *button =
        calloc(1, sizeof(struct cookie_button));;

    if (NULL == button) {
        fprintf(stderr, _("error allocating memory for new button!\n"));
        _exit(1);
    }

    if (!img) {
        if (!tog) {
            button->button = gtk_button_new_with_label(label);
            g_signal_connect_swapped(G_OBJECT(button->button), "clicked",
                callback, data);
        } else {
            button->button = gtk_toggle_button_new_with_label(label);
            g_signal_connect_swapped(G_OBJECT(button->button), "toggled",
                callback, data);
        }
        gtk_button_set_relief(GTK_BUTTON(button->button), GTK_RELIEF_NONE);
    } else {
        if (!tog) {
            button->button = gtk_button_new();
            g_signal_connect_swapped(G_OBJECT(button->button), "clicked",
                callback, data);
        } else {
            button->button = gtk_toggle_button_new();
            g_signal_connect_swapped(G_OBJECT(button->button), "toggled",
                callback, data);
        }
        button->img = gtk_image_new_from_stock((const gchar *)label,
            GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_button_set_relief(GTK_BUTTON(button->button), GTK_RELIEF_NONE);
        gtk_container_add(GTK_CONTAINER(button->button), button->img);
        gtk_widget_show(button->img);
    }

    if (tip != NULL) {
        button->tip = gtk_tooltips_new();
        gtk_tooltips_enable(button->tip);
        gtk_tooltips_set_tip(button->tip, button->button,
            (const gchar *)tip, NULL);
    }

    g_object_set(button->button, "can-focus", FALSE, NULL);

    return button;
}

/* TODO: destroy the buttons! */
void cookie_destroy_button(struct cookie_button *button)
{
    if (button) {
        /*
         * gtk_widget_destroy (button->img);
         * gtk_widget_destroy (button->button);
         */
    }
}

/* toggles opening/closing the playlist */
void cookie_playlist_toggle(struct cookie_main *cookie)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cookie->showlist->
                button))) {
        gtk_widget_show(cookie->exp_vbox);
        cookie->playlist_hints.max_height = 9000;
        cookie->playlist_hints.max_width = 9000;

        gtk_window_set_geometry_hints(GTK_WINDOW(cookie->main),
            cookie->exp_vbox, &cookie->playlist_hints, GDK_HINT_MAX_SIZE);

        if (cookie->playlist_hints.base_width > 0 &&
            cookie->playlist_hints.base_height > 0) {
            gtk_window_resize(GTK_WINDOW(cookie->main),
                cookie->playlist_hints.base_width,
                cookie->playlist_hints.base_height);
        }

        gtk_widget_grab_focus(cookie->view);

    } else {
        gtk_window_get_size(GTK_WINDOW(cookie->main),
            &cookie->playlist_hints.base_width,
            &cookie->playlist_hints.base_height);

        gtk_window_resize(GTK_WINDOW(cookie->main), 350,
            cookie->window_hints.max_height);

        cookie->window_hints.max_width = 340;
        gtk_window_set_geometry_hints(GTK_WINDOW(cookie->main),
            cookie->exp_vbox, &cookie->window_hints, GDK_HINT_MAX_SIZE);

        gtk_widget_hide(cookie->exp_vbox);
    }
}

/* creates a new info label */
GtkWidget *cookie_info_label_new(void)
{
    GtkWidget *label;

    label = gtk_label_new("");
    gtk_label_set_width_chars(GTK_LABEL(label), 45);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

    return label;
}

/* destroys an info window */
void cookie_info_destroy(struct cookie_info *info)
{
    /* omgwtf broken */
    gtk_widget_destroy(info->main);
    free(info);
}

/* a song information window, displays the song's artist,
 * title, album, genre, and path
 */
void cookie_info_window(mpd_Song * song)
{
    struct cookie_info *info = calloc(1, sizeof(struct cookie_info));
    char *label;
    GError **error = NULL;

    if (NULL == info) {
        fprintf(stderr,
            _("Error allocating memory for cookie_info struct!\n"));
        _exit(1);
    }

    info->main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(info->main), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(info->main), 5);
    g_signal_connect(G_OBJECT(info->main), "destroy",
        G_CALLBACK(gtk_widget_destroy), (GtkWidget *) info->main);
    gtk_window_set_title(GTK_WINDOW(info->main), _("song information"));
    gtk_window_set_icon_from_file((GtkWindow *) info->main,
        COOKIE_WINDOW_IMG, error);

    info->vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(info->main), info->vbox);
    gtk_widget_show(info->vbox);

    info->title = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->title, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->title), 0, 0);
    gtk_widget_show(info->title);

    info->artist = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->artist, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->artist), 0, 0);
    gtk_widget_show(info->artist);

    info->album = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->album, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->album), 0, 0);
    gtk_widget_show(info->album);

    info->genre = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->genre, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->genre), 0, 0);
    gtk_widget_show(info->genre);

    info->date = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->date, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->date), 0, 0);
    gtk_widget_show(info->date);

    info->file = cookie_info_label_new();
    gtk_box_pack_start(GTK_BOX(info->vbox), info->file, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(info->file), 0, 0);
    gtk_widget_show(info->file);


    if (NULL != song->file) {
        label = malloc(18 + strlen(cookie_filename(song->file)));
        snprintf(label, 18 + strlen(cookie_filename(song->file)),
            _("<b>Filename:</b> %s"), cookie_filename(song->file));
        gtk_label_set_markup(GTK_LABEL(info->file), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->file), _("<b>Filename:</b>"));
    }

    if (NULL != song->artist) {
        label =
            malloc(16 + strlen(g_markup_escape_text(song->artist,
                    strlen(song->artist))));
        snprintf(label, 16 + strlen(g_markup_escape_text(song->artist,
                    strlen(song->artist))), _("<b>Artist:</b> %s"),
            g_markup_escape_text(song->artist, strlen(song->artist)));
        gtk_label_set_markup(GTK_LABEL(info->artist), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->artist), _("<b>Artist:</b>"));
    }

    if (NULL != song->title) {
        label =
            malloc(15 + strlen(g_markup_escape_text(song->title,
                    strlen(song->title))));
        snprintf(label, 15 + strlen(g_markup_escape_text(song->title,
                    strlen(song->title))), _("<b>Title:</b> %s"),
            g_markup_escape_text(song->title, strlen(song->title)));
        gtk_label_set_markup(GTK_LABEL(info->title), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->title), _("<b>Title:</b>"));
    }

    if (NULL != song->date) {
        label = malloc(14 + strlen(song->date));
        snprintf(label, 14 + strlen(song->date),
            _("<b>Date:</b> %s"), song->date);
        gtk_label_set_markup(GTK_LABEL(info->date), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->date), _("<b>Date:</b>"));
    }

    if (NULL != song->album) {
        label =
            malloc(15 + strlen(g_markup_escape_text(song->album,
                    strlen(song->album))));
        snprintf(label, 15 + strlen(g_markup_escape_text(song->album,
                    strlen(song->album))), _("<b>Album:</b> %s"),
            g_markup_escape_text(song->album, strlen(song->album)));
        gtk_label_set_markup(GTK_LABEL(info->album), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->album), _("<b>Album:</b>"));
    }

    if (NULL != song->genre) {
        label = malloc(15 + strlen(song->genre));
        snprintf(label, 15 + strlen(song->genre),
            _("<b>Genre: %s</b>"), song->genre);
        gtk_label_set_markup(GTK_LABEL(info->genre), label);
        free(label);
    } else {
        gtk_label_set_markup(GTK_LABEL(info->genre), _("<b>Genre:</b>"));
    }

    gtk_widget_show(info->main);
}
