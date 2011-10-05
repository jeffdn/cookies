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

#include "tree.h"

void cookie_playlist_update_image(struct cookie_main *cookie,
    mpd_Song * song)
{
    GtkTreeIter old_iter, new_iter;
    char *old_pos, *new_pos;

    if (cookie->current_song != NULL) {
        old_pos = malloc(5);
        sprintf(old_pos, "%i", cookie->current_song->pos);
        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(cookie->store),
            &old_iter, old_pos);

        free(old_pos);

        gtk_list_store_set(cookie->store, &old_iter, COL_IMG, NULL, -1);
    }

    if (song != NULL) {
        new_pos = malloc(5);
        sprintf(new_pos, "%i", song->pos);
        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(cookie->store),
            &new_iter, new_pos);

        free(new_pos);

        gtk_list_store_set(cookie->store, &new_iter, COL_IMG,
            cookie->current_img, -1);
    }
}


void cookie_playlist_change_song(struct cookie_main *cookie,
    GtkTreePath * path)
{
    GtkTreeIter iter;
    int id;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(cookie->store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(cookie->store), &iter,
        COL_POS, &id, -1);

    mpd_ob_player_play_id(cookie->mpd, id);
}

void cookie_popup_menu_show(struct secret_args *args)
{
    GtkTreeIter iter;
    int id;

    gtk_tree_selection_get_selected(GTK_TREE_SELECTION(args->selection),
        NULL, &iter);

    gtk_tree_model_get(GTK_TREE_MODEL(args->store), &iter, COL_POS, &id,
        -1);
    cookie_info_window(mpd_ob_playlist_get_song(args->mpd, id));

    free(args);
}

void cookie_popup_menu_play(struct secret_args *args)
{
    GtkTreeIter iter;
    int id;

    gtk_tree_selection_get_selected(GTK_TREE_SELECTION(args->selection),
        NULL, &iter);
    gtk_tree_model_get(GTK_TREE_MODEL(args->store), &iter, COL_POS, &id,
        -1);
    mpd_ob_player_play_id(args->mpd, id);

    free(args);
}

void cookie_popup_menu_remove(struct secret_args *args)
{
    GtkTreeIter iter;
    int id;

    gtk_tree_selection_get_selected(GTK_TREE_SELECTION(args->selection),
        NULL, &iter);
    gtk_tree_model_get(GTK_TREE_MODEL(args->store), &iter, COL_POS, &id,
        -1);
    mpd_ob_playlist_queue_delete_id(args->mpd, id);
    mpd_ob_playlist_queue_commit(args->mpd);

    args->cookie->current_song = NULL;

    free(args);
}

void view_popup_menu(struct cookie_main *cookie,
    GdkEventButton * event, GtkTreePath * path)
{
    GtkWidget *menu;
    GtkWidget *display_info;
    GtkWidget *play_song;
    GtkWidget *remove_song;
    struct secret_args *args = calloc(1, sizeof(struct secret_args));

    args->cookie = cookie;
    args->store = cookie->store;
    args->mpd = cookie->mpd;
    args->selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(cookie->view));

    menu = gtk_menu_new();

    display_info = gtk_menu_item_new_with_label(_("Display song info"));
    g_signal_connect_swapped(display_info, "activate",
        (GCallback) cookie_popup_menu_show, args);

    play_song = gtk_menu_item_new_with_label(_("Play song"));
    g_signal_connect_swapped(play_song, "activate",
        (GCallback) cookie_popup_menu_play, args);

    remove_song = gtk_menu_item_new_with_label(_("Remove from playlist"));
    g_signal_connect_swapped(remove_song, "activate",
        (GCallback) cookie_popup_menu_remove, args);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), display_info);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), play_song);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), remove_song);

    gtk_widget_show_all(menu);

    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
        (event != NULL) ? event->button : 0,
        gdk_event_get_time((GdkEvent *) event));
}

gboolean cookie_playlist_item_button_pressed(struct cookie_main *cookie,
    GdkEventButton * event, GtkTreePath * path)
{

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        view_popup_menu(cookie, event, path);
    }

    return FALSE;
}

void cookie_playlist_add_song(struct cookie_main *cookie, mpd_Song * song)
{
    GtkTreeIter iter;

    char *song_str = NULL;
    char *time_str = NULL;

    if (song == NULL) {
        printf(_("skipping a song, null value.\n"));
    } else {
        song_str = cookie_song_name(song);
        time_str = cookie_format_time(song->time);

        gtk_list_store_append(GTK_LIST_STORE(cookie->store), &iter);
        gtk_list_store_set(GTK_LIST_STORE(cookie->store), &iter,
            COL_POS, (guint) song->id,
            COL_SONG, song_str, COL_TIME, time_str, -1);

        /* we would have hardcore anal leakage without this */

        free(time_str);
        free(song_str);
    }
}

void cookie_refresh_playlist(struct cookie_main *cookie)
{
    GtkTreeIter iter;
    MpdData *data;
    mpd_Song *song;
    char *pos = malloc(5);
    int state;

    gtk_list_store_clear(GTK_LIST_STORE(cookie->store));

    data = mpd_ob_playlist_get_changes(cookie->mpd, -1);
    while (data != NULL) {
        cookie_playlist_add_song(cookie, data->value.song);
        data = data->next;
    }

    if (mpd_ob_playlist_get_playlist_length(cookie->mpd) <= 1) {
        gtk_widget_set_sensitive(cookie->shuffle->button, FALSE);
    } else {
        gtk_widget_set_sensitive(cookie->shuffle->button, TRUE);
    }

    state = mpd_ob_player_get_state(cookie->mpd);
    if (state == MPD_OB_PLAYER_PLAY || state == MPD_OB_PLAYER_PAUSE) {

        song = mpd_ob_playlist_get_current_song(cookie->mpd);
        snprintf(pos, 5, "%i", song->pos);

        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(cookie->store),
            &iter, pos);
        gtk_list_store_set(cookie->store, &iter, COL_IMG,
            cookie->current_img, -1);

        free(pos);
    }
}

gboolean cookie_snap_playlist(struct cookie_main *cookie)
{
    GtkTreePath *path;
    mpd_Song *song = mpd_ob_playlist_get_current_song(cookie->mpd);
    char *pos = malloc(5);

    if (song != NULL) {
        sprintf(pos, "%i", song->pos);
        path = gtk_tree_path_new_from_string(pos);
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cookie->view), path,
            NULL, TRUE, 0, 0);

        gtk_tree_path_free(path);
    }
    free(pos);

    return FALSE;
}

gboolean cookie_search_playlist(GtkTreeModel * model, gint column,
    const gchar * key, GtkTreeIter * iter, struct cookie_main * cookie)
{

    char *song_name;

    gtk_tree_model_get(GTK_TREE_MODEL(model), iter,
        COL_SONG, &song_name, -1);

    if (strcasestr(song_name, key) != NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}


void cookie_create_playlist(struct cookie_main *cookie)
{
    cookie->store = gtk_list_store_new(4, G_TYPE_UINT, GDK_TYPE_PIXBUF,
        G_TYPE_STRING, G_TYPE_STRING);
    cookie->view =
        gtk_tree_view_new_with_model(GTK_TREE_MODEL(cookie->store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(cookie->view), FALSE);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(cookie->view), TRUE);

    g_signal_connect_swapped(G_OBJECT(cookie->view), "row-activated",
        G_CALLBACK(cookie_playlist_change_song),
        (struct cookie_main *)cookie);

    g_signal_connect_swapped(G_OBJECT(cookie->view), "button-press-event",
        G_CALLBACK(cookie_playlist_item_button_pressed),
        (struct cookie_main *)cookie);

    cookie->rend = gtk_cell_renderer_text_new();
    g_object_set(cookie->rend, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

    cookie->rend_img = gtk_cell_renderer_pixbuf_new();

    cookie->col_img =
        gtk_tree_view_column_new_with_attributes("current",
        cookie->rend_img, "pixbuf", COL_IMG, NULL);
    cookie->col_song =
        gtk_tree_view_column_new_with_attributes("song", cookie->rend,
        "markup", COL_SONG, NULL);

    /* 
     * this will reset the text renderer, we don't want to ellipsize
     * the time column!
     */

    cookie->rend = gtk_cell_renderer_text_new();
    cookie->col_time =
        gtk_tree_view_column_new_with_attributes("time", cookie->rend,
        "text", COL_TIME, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(cookie->view),
        cookie->col_img);
    gtk_tree_view_append_column(GTK_TREE_VIEW(cookie->view),
        cookie->col_song);
    gtk_tree_view_append_column(GTK_TREE_VIEW(cookie->view),
        cookie->col_time);

    gtk_tree_view_column_set_sizing(cookie->col_img,
        GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(cookie->col_img, 18);
    /*gtk_tree_view_column_set_sizing (cookie->col_time, 
       GTK_TREE_VIEW_COLUMN_FIXED);
       gtk_tree_view_column_set_fixed_width (cookie->col_time, 45); */

    gtk_tree_view_column_set_expand(cookie->col_song, TRUE);
    gtk_tree_view_column_set_alignment(cookie->col_time, 1);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(cookie->view), COL_SONG);
    gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(cookie->view),
        (GtkTreeViewSearchEqualFunc) cookie_search_playlist,
        (struct cookie_main *)cookie, NULL);

    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(cookie->view), TRUE);
}
