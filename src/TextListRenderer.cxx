// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "TextListRenderer.hxx"
#include "ListText.hxx"
#include "paint.hxx"

#include <assert.h>

static void
list_window_paint_row(const Window window, unsigned width, bool selected,
		      const char *text) noexcept
{
	row_paint_text(window, width, Style::LIST,
		       selected, text);
}

void
TextListRenderer::PaintListItem(const Window window, unsigned i, unsigned,
				unsigned width, bool selected) const noexcept
{
	char buffer[1024];
	const char *label = text.GetListItemText(buffer, sizeof(buffer), i);
	assert(label != nullptr);

	list_window_paint_row(window, width, selected, label);
}
