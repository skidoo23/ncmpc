/* ncmpc (Ncurses MPD Client)
 * (c) 2004-2009 The Music Player Daemon Project
 * Project homepage: http://musicpd.org
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "status_bar.h"
#include "options.h"
#include "colors.h"
#include "i18n.h"
#include "charset.h"
#include "strfsong.h"
#include "player_command.h"
#include "utils.h"

#ifndef NCMPC_MINI
#include "hscroll.h"
#endif

#include <mpd/client.h>

#include <assert.h>
#include <string.h>

static gboolean
status_bar_clear_message(gpointer data)
{
	struct status_bar *p = data;
	WINDOW *w = p->window.w;

	assert(p != NULL);
	assert(p->message_source_id != 0);

	p->message_source_id = 0;

	wmove(w, 0, 0);
	wclrtoeol(w);
	wrefresh(w);

	return false;
}

void
status_bar_paint(const struct status_bar *p, const struct mpd_status *status,
		 const struct mpd_song *song)
{
	WINDOW *w = p->window.w;
	enum mpd_state state;
	int elapsedTime = 0;
#ifdef NCMPC_MINI
	static char bitrate[1];
#else
	char bitrate[16];
#endif
	const char *str = NULL;
	int x = 0;
	char buffer[p->window.cols * 4 + 1];

	if (p->message_source_id != 0)
		return;

	wmove(w, 0, 0);
	wclrtoeol(w);
	colors_use(w, COLOR_STATUS_BOLD);

	state = status == NULL ? MPD_STATE_UNKNOWN
		: mpd_status_get_state(status);

	switch (state) {
	case MPD_STATE_PLAY:
		str = _("Playing:");
		break;
	case MPD_STATE_PAUSE:
		str = _("[Paused]");
		break;
	case MPD_STATE_STOP:
	default:
		break;
	}

	if (str) {
		waddstr(w, str);
		x += utf8_width(str) + 1;
	}

	/* create time string */
	if (state == MPD_STATE_PLAY || state == MPD_STATE_PAUSE) {
		int total_time = mpd_status_get_total_time(status);
		if (total_time > 0) {
			char elapsed_string[32], duration_string[32];

			/*checks the conf to see whether to display elapsed or remaining time */
			if(!strcmp(options.timedisplay_type,"elapsed"))
				elapsedTime = mpd_status_get_elapsed_time(status);
			else if(!strcmp(options.timedisplay_type,"remaining"))
				elapsedTime = total_time -
					mpd_status_get_elapsed_time(status);

			if (song != NULL &&
			    seek_id == (int)mpd_song_get_id(song))
				elapsedTime = seek_target_time;

			/* display bitrate if visible-bitrate is true */
#ifndef NCMPC_MINI
			if (options.visible_bitrate) {
				g_snprintf(bitrate, 16,
					   " [%d kbps]",
					   mpd_status_get_kbit_rate(status));
			} else {
				bitrate[0] = '\0';
			}
#endif

			/* write out the time */
			format_duration_short(elapsed_string,
					      sizeof(elapsed_string),
					      elapsedTime);
			format_duration_short(duration_string,
					      sizeof(duration_string),
					      total_time);

			g_snprintf(buffer, sizeof(buffer), "%s [%s/%s]",
				   bitrate, elapsed_string, duration_string);
#ifndef NCMPC_MINI
		} else {
			g_snprintf(buffer, sizeof(buffer),
				   " [%d kbps]",
				   mpd_status_get_kbit_rate(status));
#endif
		}
#ifndef NCMPC_MINI
	} else {
		if (options.display_time) {
			time_t timep;

			time(&timep);
			strftime(buffer, sizeof(buffer), "%X ",localtime(&timep));
		} else
#endif
			buffer[0] = 0;
	}

	/* display song */
	if (state == MPD_STATE_PLAY || state == MPD_STATE_PAUSE) {
		char songname[p->window.cols * 4 + 1];
#ifndef NCMPC_MINI
		int width = COLS - x - utf8_width(buffer);
#endif

		if (song)
			strfsong(songname, sizeof(songname),
				 options.status_format, song);
		else
			songname[0] = '\0';

		colors_use(w, COLOR_STATUS);
		/* scroll if the song name is to long */
#ifndef NCMPC_MINI
		if (options.scroll && utf8_width(songname) > (unsigned)width) {
			static struct hscroll hscroll = { 0, 0 };
			char *tmp = strscroll(&hscroll, songname,
					      options.scroll_sep, width);

			g_strlcpy(songname, tmp, sizeof(songname));
			g_free(tmp);
		}
#endif
		//mvwaddnstr(w, 0, x, songname, width);
		mvwaddstr(w, 0, x, songname);
	}

	/* display time string */
	if (buffer[0] != 0) {
		x = p->window.cols - strlen(buffer);
		colors_use(w, COLOR_STATUS_TIME);
		mvwaddstr(w, 0, x, buffer);
	}

	wnoutrefresh(w);
}

void
status_bar_resize(struct status_bar *p, unsigned width, int y, int x)
{
	p->window.cols = width;
	wresize(p->window.w, 1, width);
	mvwin(p->window.w, y, x);
}

void
status_bar_message(struct status_bar *p, const char *msg)
{
	WINDOW *w = p->window.w;

	wmove(w, 0, 0);
	wclrtoeol(w);
	colors_use(w, COLOR_STATUS_ALERT);
	waddstr(w, msg);
	wnoutrefresh(w);

	if (p->message_source_id != 0)
		g_source_remove(p->message_source_id);
	p->message_source_id = g_timeout_add(options.status_message_time * 1000,
					     status_bar_clear_message, p);
}
