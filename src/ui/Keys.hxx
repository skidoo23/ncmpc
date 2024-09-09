// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#pragma once

#include <curses.h>

#define KEY_CTL(x) ((x) & 0x1f) /* KEY_CTL(A) == ^A == \1 */

enum : int {
	KEY_BCKSPC = 8,
	TAB = 9,
};