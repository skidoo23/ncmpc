/* ncmpc (Ncurses MPD Client)
 * (c) 2004-2018 The Music Player Daemon Project
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

#include "StringUTF8.hxx"
#include "ScopeExit.hxx"
#include "config.h"

#include <glib.h>

#include <assert.h>
#include <string.h>

gcc_pure
int
CollateUTF8(const char *a, const char *b)
{
	return g_utf8_collate(a, b);
}

#ifdef HAVE_CURSES_ENHANCED
gcc_const
static inline unsigned
unicode_char_width(gunichar ch)
{
	if (g_unichar_iszerowidth(ch))
		return 0;

	if (g_unichar_iswide(ch))
		return 2;

	return 1;
}
#endif /* HAVE_CURSES_ENHANCED */

unsigned
utf8_width(const char *str)
{
	assert(str != nullptr);

#if defined(ENABLE_MULTIBYTE) && !defined(HAVE_CURSES_ENHANCED)
	return g_utf8_strlen(str, -1);
#else
#ifdef HAVE_CURSES_ENHANCED
	if (g_utf8_validate(str, -1, nullptr)) {
		size_t len = g_utf8_strlen(str, -1);
		unsigned width = 0;
		gunichar c;

		while (len--) {
			c = g_utf8_get_char(str);
			width += unicode_char_width(c);
			str += g_unichar_to_utf8(c, nullptr);
		}

		return width;
	} else
#endif
		return strlen(str);
#endif
}

const char *
AtWidthUTF8(const char *p, size_t length, size_t width) noexcept
{
	assert(p != nullptr);

	const char *const end = p + length;

	while (p < end) {
		gunichar c = g_utf8_get_char(p);
		size_t char_width = g_unichar_iswide(c) ? 2 : 1;
		if (char_width > width)
			/* too wide - stop here */
			break;

		width -= char_width;
		p += g_unichar_to_utf8(c, nullptr);
	}

	return p;
}
