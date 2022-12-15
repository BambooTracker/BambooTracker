/*
 * Copyright (C) 2018-2022 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/********** Instrument edit **********/
#include "./instrument/add_instrument_command.hpp"
#include "./instrument/remove_instrument_command.hpp"
#include "./instrument/change_instrument_name_command.hpp"
#include "./instrument/clone_instrument_command.hpp"
#include "./instrument/deep_clone_instrument_command.hpp"
#include "./instrument/swap_instruments_command.hpp"

/********** Pattern edit **********/
#include "./pattern/set_key_on_to_step_command.hpp"
#include "./pattern/set_key_off_to_step_command.hpp"
#include "./pattern/erase_step_command.hpp"
#include "./pattern/set_instrument_to_step_command.hpp"
#include "./pattern/erase_instrument_in_step_command.hpp"
#include "./pattern/set_volume_to_step_command.hpp"
#include "./pattern/erase_volume_in_step_command.hpp"
#include "./pattern/set_effect_id_to_step_command.hpp"
#include "./pattern/erase_effect_in_step_command.hpp"
#include "./pattern/set_effect_value_to_step_command.hpp"
#include "./pattern/erase_effect_value_in_step_command.hpp"
#include "./pattern/insert_step_command.hpp"
#include "./pattern/delete_previous_step_command.hpp"
#include "./pattern/paste_copied_data_to_pattern_command.hpp"
#include "./pattern/erase_cells_in_pattern_command.hpp"
#include "./pattern/paste_mix_copied_data_to_pattern_command.hpp"
#include "./pattern/transpose_note_in_pattern_command.hpp"
#include "./pattern/expand_pattern_command.hpp"
#include "./pattern/shrink_pattern_command.hpp"
#include "./pattern/set_echo_buffer_access_command.hpp"
#include "./pattern/interpolate_pattern_command.hpp"
#include "./pattern/reverse_pattern_command.hpp"
#include "./pattern/replace_instrument_in_pattern_command.hpp"
#include "./pattern/paste_overwrite_copied_data_to_pattern_command.hpp"
#include "./pattern/change_values_in_pattern_command.hpp"
#include "./pattern/paste_insert_copied_data_to_pattern_command.hpp"
#include "./pattern/set_key_cut_to_step_command.hpp"

/********** Order edit **********/
#include "./order/set_pattern_to_order_command.hpp"
#include "./order/insert_order_below_command.hpp"
#include "./order/delete_order_command.hpp"
#include "./order/paste_copied_data_to_order_command.hpp"
#include "./order/duplicate_order_command.hpp"
#include "./order/move_order_command.hpp"
#include "./order/clone_patterns_command.hpp"
#include "./order/clone_order_command.hpp"
