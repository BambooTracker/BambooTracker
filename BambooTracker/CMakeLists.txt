# C11 required
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# C++14 required
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BT_WARNFLAGS)
if(MSVC)
	list(APPEND BT_WARNFLAGS /W3 /WX /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else(MSVC)
	list(APPEND BT_WARNFLAGS -Wall -Wextra -pedantic -Werror -pedantic-errors)
endif(MSVC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Identify Qt version we're using
option(QT_VERSION "Which major version of Qt we're building against (5 or 6). Any other value -> automatic detection" OFF)
if(NOT QT_VERSION MATCHES "[56]")
	message("Attempting to identify Qt version.")
	find_package(Qt6 COMPONENTS Core)
	if(Qt6_FOUND)
		set(QT_VERSION 6)
	else(Qt6_FOUND)
		find_package(Qt5 COMPONENTS Core)
		if(Qt5_FOUND)
			set(QT_VERSION 5)
		else(Qt5_FOUND)
			message(FATAL_ERROR "Unable to locate either Qt5 or Qt6!")
		endif(Qt5_FOUND)
	endif(Qt6_FOUND)
	message("Using Qt${QT_VERSION}")
endif(NOT QT_VERSION MATCHES "[56]")

set(QT_COMPONENTS Core Gui Widgets LinguistTools)
if(QT_VERSION EQUAL 6)
	list(APPEND QT_COMPONENTS Core5Compat)
endif(QT_VERSION EQUAL 6)
find_package("Qt${QT_VERSION}" COMPONENTS ${QT_COMPONENTS} REQUIRED)

# C/C++ files
set(BT_SOURCES
	chip/mame/fmopn.c
	chip/mame/mame_2608.cpp
	chip/mame/ymdeltat.c
	chip/nuked/nuked_2608.cpp
	chip/register_write_logger.cpp
	command/instrument/swap_instruments_command.cpp
	command/pattern/change_values_in_pattern_command.cpp
	command/pattern/paste_insert_copied_data_to_pattern_command.cpp
	command/pattern/pattern_command_utils.cpp
	command/pattern/transpose_note_in_pattern_command.cpp
	gui/bookmark_manager_form.cpp
	gui/command/instrument/instrument_command_qt_utils.cpp
	gui/command/instrument/swap_instruments_qt_command.cpp
	gui/command/order/order_list_common_qt_command.cpp
	gui/command/pattern/pattern_editor_common_qt_command.cpp
	gui/drop_detect_list_widget.cpp
	gui/effect_description.cpp
	gui/effect_list_dialog.cpp
	gui/file_io_error_message_box.cpp
	gui/go_to_dialog.cpp
	gui/gui_utils.cpp
	gui/hide_tracks_dialog.cpp
	gui/instrument_editor/adpcm_sample_editor.cpp
	gui/instrument_editor/arpeggio_macro_editor.cpp
	gui/instrument_editor/grid_settings_dialog.cpp
	gui/instrument_editor/instrument_editor_adpcm_form.cpp
	gui/instrument_editor/instrument_editor_drumkit_form.cpp
	gui/instrument_editor/instrument_editor_utils.cpp
	gui/instrument_editor/pan_macro_editor.cpp
	gui/instrument_editor/sample_length_dialog.cpp
	gui/instrument_editor/tone_noise_macro_editor.cpp
	gui/key_signature_manager_form.cpp
	gui/keyboard_shortcut_list_dialog.cpp
	gui/note_name_manager.cpp
	gui/swap_tracks_dialog.cpp
	gui/transpose_song_dialog.cpp
	instrument/sample_adpcm.cpp
	instrument/sequence_property.cpp
	io/btb_io.cpp
	io/bti_io.cpp
	io/btm_io.cpp
	io/dat_io.cpp
	io/dmp_io.cpp
	io/export_io.cpp
	io/ff_io.cpp
	io/ins_io.cpp
	io/io_utils.cpp
	io/opni_io.cpp
	io/p86_io.cpp
	io/ppc_io.cpp
	io/pps_io.cpp
	io/pvi_io.cpp
	io/pzi_io.cpp
	io/tfi_io.cpp
	io/vgi_io.cpp
	io/wav_container.cpp
	io/wopn_io.cpp
	io/y12_io.cpp
	jamming.cpp
	main.cpp
	gui/mainwindow.cpp
	chip/chip.cpp
	chip/opna.cpp
	chip/resampler.cpp
	chip/nuked/ym3438.c
	bamboo_tracker.cpp
	module/effect.cpp
	note.cpp
	playback.cpp
	song_length_calculator.cpp
	audio/audio_stream.cpp
	instrument/instruments_manager.cpp
	command/command_manager.cpp
	command/instrument/add_instrument_command.cpp
	command/instrument/remove_instrument_command.cpp
	gui/command/instrument/add_instrument_qt_command.cpp
	gui/command/instrument/remove_instrument_qt_command.cpp
	gui/instrument_editor/instrument_editor_fm_form.cpp
	gui/instrument_editor/fm_operator_table.cpp
	gui/labeled_vertical_slider.cpp
	gui/labeled_horizontal_slider.cpp
	gui/slider_style.cpp
	gui/command/instrument/change_instrument_name_qt_command.cpp
	command/instrument/change_instrument_name_command.cpp
	opna_controller.cpp
	instrument/instrument.cpp
	instrument/envelope_fm.cpp
	gui/event_guard.cpp
	audio/audio_stream_rtaudio.cpp
	tick_counter.cpp
	module/module.cpp
	module/song.cpp
	module/pattern.cpp
	module/track.cpp
	module/step.cpp
	gui/order_list_editor/order_list_panel.cpp
	gui/order_list_editor/order_list_editor.cpp
	gui/pattern_editor/pattern_editor_panel.cpp
	gui/pattern_editor/pattern_editor.cpp
	gui/instrument_editor/instrument_editor_ssg_form.cpp
	command/pattern/set_key_off_to_step_command.cpp
	command/pattern/set_key_on_to_step_command.cpp
	command/pattern/set_instrument_to_step_command.cpp
	command/pattern/erase_instrument_in_step_command.cpp
	command/pattern/set_volume_to_step_command.cpp
	command/pattern/erase_volume_in_step_command.cpp
	command/pattern/set_effect_id_to_step_command.cpp
	command/pattern/erase_effect_in_step_command.cpp
	command/pattern/set_effect_value_to_step_command.cpp
	command/pattern/erase_effect_value_in_step_command.cpp
	command/pattern/insert_step_command.cpp
	command/pattern/delete_previous_step_command.cpp
	command/pattern/erase_step_command.cpp
	gui/command/instrument/deep_clone_instrument_qt_command.cpp
	command/instrument/deep_clone_instrument_command.cpp
	command/instrument/clone_instrument_command.cpp
	gui/command/instrument/clone_instrument_qt_command.cpp
	command/order/set_pattern_to_order_command.cpp
	command/order/insert_order_below_command.cpp
	command/order/delete_order_command.cpp
	command/pattern/paste_copied_data_to_pattern_command.cpp
	command/pattern/erase_cells_in_pattern_command.cpp
	command/order/paste_copied_data_to_order_command.cpp
	gui/instrument_editor/instrument_form_manager.cpp
	instrument/lfo_fm.cpp
	gui/instrument_editor/visualized_instrument_macro_editor.cpp
	instrument/effect_iterator.cpp
	command/pattern/paste_mix_copied_data_to_pattern_command.cpp
	gui/module_properties_dialog.cpp
	gui/groove_settings_dialog.cpp
	gui/configuration_dialog.cpp
	command/pattern/expand_pattern_command.cpp
	command/pattern/shrink_pattern_command.cpp
	instrument/abstract_instrument_property.cpp
	command/order/duplicate_order_command.cpp
	command/order/move_order_command.cpp
	command/order/clone_patterns_command.cpp
	command/order/clone_order_command.cpp
	command/pattern/set_echo_buffer_access_command.cpp
	gui/comment_edit_dialog.cpp
	io/binary_container.cpp
	command/pattern/interpolate_pattern_command.cpp
	command/pattern/reverse_pattern_command.cpp
	command/pattern/replace_instrument_in_pattern_command.cpp
	gui/vgm_export_settings_dialog.cpp
	gui/wave_export_settings_dialog.cpp
	configuration.cpp
	gui/configuration_handler.cpp
	gui/color_palette.cpp
	command/pattern/paste_overwrite_copied_data_to_pattern_command.cpp
	format/wopn_file.c
	instrument/bank.cpp
	gui/instrument_selection_dialog.cpp
	gui/s98_export_settings_dialog.cpp
	precise_timer.cpp
	io/module_io.cpp
	io/instrument_io.cpp
	io/bank_io.cpp
	gui/fm_envelope_set_edit_dialog.cpp
	gui/file_history.cpp
	midi/midi.cpp
	gui/q_application_wrapper.cpp
	gui/wave_visual.cpp

	resources/doc/doc.qrc
	resources/icon/icon.qrc
)

# UI forms
set(BT_FORMS
	gui/bookmark_manager_form.ui
	gui/effect_list_dialog.ui
	gui/go_to_dialog.ui
	gui/hide_tracks_dialog.ui
	gui/instrument_editor/adpcm_sample_editor.ui
	gui/instrument_editor/grid_settings_dialog.ui
	gui/instrument_editor/instrument_editor_adpcm_form.ui
	gui/instrument_editor/instrument_editor_drumkit_form.ui
	gui/instrument_editor/sample_length_dialog.ui
	gui/key_signature_manager_form.ui
	gui/keyboard_shortcut_list_dialog.ui
	gui/mainwindow.ui
	gui/instrument_editor/instrument_editor_fm_form.ui
	gui/instrument_editor/fm_operator_table.ui
	gui/labeled_vertical_slider.ui
	gui/labeled_horizontal_slider.ui
	gui/order_list_editor/order_list_editor.ui
	gui/pattern_editor/pattern_editor.ui
	gui/instrument_editor/instrument_editor_ssg_form.ui
	gui/instrument_editor/visualized_instrument_macro_editor.ui
	gui/module_properties_dialog.ui
	gui/groove_settings_dialog.ui
	gui/configuration_dialog.ui
	gui/comment_edit_dialog.ui
	gui/swap_tracks_dialog.ui
	gui/transpose_song_dialog.ui
	gui/vgm_export_settings_dialog.ui
	gui/wave_export_settings_dialog.ui
	gui/instrument_selection_dialog.ui
	gui/s98_export_settings_dialog.ui
	gui/fm_envelope_set_edit_dialog.ui
)

set(BT_INCLUDEPATHS
	"${CMAKE_CURRENT_SOURCE_DIR}"
	instrument
	module
)

# Translations
add_subdirectory(lang)

add_executable(BambooTracker
	${BT_SOURCES} ${BT_FORMS}
)

target_include_directories(BambooTracker PRIVATE ${BT_INCLUDEPATHS})

target_compile_options(BambooTracker PRIVATE ${BT_WARNFLAGS})

option(REAL_CHIP "Compile with support for SCCI and C86CTL interfaces to a real OPNA chip" ${WIN32})

if(REAL_CHIP)
	target_compile_definitions(BambooTracker PRIVATE USE_REAL_CHIP)
endif(REAL_CHIP)

if(QT_VERSION EQUAL 6)
	set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat)
else(QT_VERSION EQUAL 6)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif(QT_VERSION EQUAL 6)
target_link_libraries(BambooTracker PUBLIC ${QT_LIBRARIES})

# emu2149
add_dependencies(BambooTracker emu2149)
target_include_directories(BambooTracker PRIVATE "${PROJECT_SOURCE_DIR}/submodules/emu2149/src")
target_link_libraries(BambooTracker PRIVATE emu2149)

# RtAudio
if(RTAUDIO_FOUND)
	# target_compile_options(BambooTracker PUBLIC ${RTAUDIO_CFLAGS})
	target_include_directories(BambooTracker PUBLIC ${RTAUDIO_INCLUDE_DIRS})
	target_link_libraries(BambooTracker PUBLIC ${RTAUDIO_LDFLAGS})
else(RTAUDIO_FOUND)
	add_dependencies(BambooTracker rtaudio)
	target_include_directories(BambooTracker PRIVATE "${PROJECT_SOURCE_DIR}/submodules/RtAudio/src")
	target_link_libraries(BambooTracker PRIVATE rtaudio)
endif(RTAUDIO_FOUND)

# RtMidi
if(RTMIDI_FOUND)
	# target_compile_options(BambooTracker PUBLIC ${RTMIDI_CFLAGS})
	target_include_directories(BambooTracker PUBLIC ${RTMIDI_INCLUDE_DIRS})
	target_link_libraries(BambooTracker PUBLIC ${RTMIDI_LDFLAGS})
else(RTMIDI_FOUND)
	add_dependencies(BambooTracker rtmidi)
	target_include_directories(BambooTracker PRIVATE "${PROJECT_SOURCE_DIR}/submodules/RtMidi/src")
	target_link_libraries(BambooTracker PRIVATE rtmidi)
endif(RTMIDI_FOUND)

install(TARGETS BambooTracker DESTINATION "${CMAKE_INSTALL_BINDIR}")
