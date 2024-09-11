// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#pragma once

#include "ModalDialog.hxx"
#include "co/AwaitableHelper.hxx"

#include <cstdint>
#include <string_view>

/**
 * A #ModalDialog that asks the user to press a key.
 *
 * This dialog is supposed to be awaited from a coroutine using
 * co_await.  It suspends the caller while waiting for user input.
 */
class KeyDialog final : public ModalDialog {
	const std::string_view prompt;

	std::coroutine_handle<> continuation;

	int result = -2;

	using Awaitable = Co::AwaitableHelper<KeyDialog, false>;
	friend Awaitable;

public:
	/**
	 * @param _prompt the human-readable prompt to be displayed
	 * (including question mark if desired); the pointed-by memory
	 * is owned by the caller and must remain valid during the
	 * lifetime of this dialog
	 */
	KeyDialog(ModalDock &_dock, std::string_view _prompt) noexcept
		:ModalDialog(_dock), prompt(_prompt)
	{
		Show();
	}

	~KeyDialog() noexcept {
		Hide();
	}

	/**
	 * Await completion of this dialog.
	 *
	 * @return an ncurses key code or -1 if the dialog was
         * canceled
	 */
	Awaitable operator co_await() noexcept {
		return *this;
	}

private:
	void SetResult(int _result) noexcept {
		result = _result;

		if (continuation)
			continuation.resume();
	}

	bool IsReady() const noexcept {
		return result != -2;
	}

	int TakeValue() noexcept {
		return result;
	}

public:
	/* virtual methodds from Modal */
	void OnLeave(Window window) noexcept override;
	void OnCancel() noexcept override;
	void Paint(Window window) const noexcept override;
	bool OnKey(Window window, int key) override;
};