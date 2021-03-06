/*
   Copyright (C) 2008 - 2014 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Copyright (C) 2012 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/mp_host_game_prompt.hpp"

#include "game_preferences.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_host_game_prompt
 *
 * == Host Networked Game prompt ==
 *
 * This shows the dialog to confirm deleting a savegame file.
 *
 * @begin{table}{dialog_widgets}
 *
 * do_not_show_again & & boolean_selector & m &
 *        A checkbox to not show this dialog again. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_host_game_prompt)

/**
 * Helper for @ref preferences::ask_delete_saves.
 */
static bool get_do_not_show_again()
{
	return preferences::mp_server_warning_disabled() != 1;
}

/**
 * Helper for @ref preferences::set_ask_delete_saves.
 */
static void set_do_not_show_again(const bool do_not_show_again)
{
	preferences::set_mp_server_warning_disabled(do_not_show_again ? 2 : 1);
}

tmp_host_game_prompt::tmp_host_game_prompt()
{
	register_bool("do_not_show_again"
			, true
			, &get_do_not_show_again
			, &set_do_not_show_again);
}

} // namespace gui2

