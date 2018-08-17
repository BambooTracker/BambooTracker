#pragma once

/********** Instrument edit **********/
#include "./instrument/add_instrument_command.hpp"
#include "./instrument/remove_instrument_command.hpp"
#include "./instrument/change_instrument_name_command.hpp"
#include "./instrument/paste_instrument_command.hpp"
#include "./instrument/clone_instrument_command.hpp"

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
