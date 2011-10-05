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

#include "cookie.h"
#include "window.h"
#include "mpdwrapper.h"

void *playlist_changed(MpdObj * mi, int old_id, int new_id, void *cookie)
{

    cookie_refresh_playlist((struct cookie_main *)cookie);
    cookie_song_changed(mi, -1, -1, cookie);

    return NULL;
}

int main(int argc, char *argv[])
{
    struct cookie_main *cookie = calloc(1, sizeof(struct cookie_main));
    int i, port = 6600;
    char *host = strdup("localhost");
    char *pass = NULL;

    if (NULL == cookie) {
        fprintf(stderr, _("error allocating memory for main object!\n"));
        _exit(1);
    }

    /* Initialise our internationalisation features. */
    setlocale(LC_ALL, "");
    if (getenv("COOKIE_I18N_LOCALEDIR"))
        /* Getting the localedir through an envvar is good for
         * debugging/devel-builds. */
        bindtextdomain(COOKIE_I18N_PKG, getenv("COOKIE_I18N_LOCALEDIR"));
    else
        bindtextdomain(COOKIE_I18N_PKG, COOKIE_I18N_LOCALEDIR);
    textdomain(COOKIE_I18N_PKG);


    if (getenv("MPD_HOST")) {
        free(host);
        host = strdup(getenv("MPD_HOST"));
    }
    if (getenv("MPD_PORT")) {
        port = atoi(getenv("MPD_PORT"));
    }

    gtk_init(&argc, &argv);

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (!strncmp(argv[i], "--host", 6)) {
                if ((i + 1) >= argc) {
                    break;
                }
                host = strdup(argv[i + 1]);
                i++;
            } else if (!strncmp(argv[i], "--port", 6)) {
                if ((i + 1) >= argc) {
                    break;
                }
                port = atoi(argv[i + 1]);
                i++;
            } else if (!strncmp(argv[i], "--password", 10)) {
                if ((i + 1) >= argc) {
                    break;
                }
                pass = strdup(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, _("Invalid argument!\n"));
                i++;
            }
        }
    }

    cookie->mpd = mpd_ob_new(host, port, pass);
    while (cookie_connect(cookie))
        sleep(5);
    cookie_main_window_new(cookie);

    gtk_adjustment_set_value(GTK_ADJUSTMENT(cookie->volume->adj),
        mpd_ob_status_get_volume(cookie->mpd));
    gtk_widget_show(cookie->main);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookie->showlist->
            button), TRUE);

    if (!mpd_ob_check_connected(cookie->mpd)) {
        fprintf(stderr, _("Not really connected. Shutting down.\n"));
        cookies_quit(cookie);
    }

    mpd_ob_signal_set_playlist_changed(cookie->mpd,
        playlist_changed, (struct cookie_main *)cookie);

    mpd_ob_signal_set_song_changed(cookie->mpd,
        cookie_song_changed, (struct cookie_main *)cookie);

    cookie_init(cookie);

    GTK_WIDGET_SET_FLAGS(cookie->view, GTK_CAN_DEFAULT);
    g_object_set(cookie->view, "is-focus", TRUE,
        "has-default", TRUE, "has-focus", TRUE, NULL);

    cookie->timeout_id =
        g_timeout_add(500, (GSourceFunc) cookie_update_info,
        (struct cookie_main *)cookie);
    g_timeout_add(500, (GSourceFunc) cookie_snap_playlist,
        (struct cookie_main *)cookie);

    gtk_main();

    if (host)
        free(host);
    if (pass)
        free(pass);
    cookies_quit(cookie);
    return 0;
}
