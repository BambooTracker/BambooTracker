# C11 required
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

# C++14 required
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (BT_WARNFLAGS)
if (MSVC)
	list (APPEND BT_WARNFLAGS /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
	list (APPEND BT_WARNFLAGS -Wall -Wextra -pedantic)
endif()

option (WARNINGS_ARE_ERRORS "Treat any warnings in BambooTracker's code as errors" ON)
if (WARNINGS_ARE_ERRORS)
	if (MSVC)
		list (APPEND BT_WARNFLAGS /WX)
	else()
		list (APPEND BT_WARNFLAGS -Werror -pedantic-errors)
	endif()
endif()

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

# Identify Qt version we're using
message (STATUS "Attempting to identify Qt version.")
find_package (Qt6 COMPONENTS Core)
if (Qt6_FOUND)
	set (QT_VERSION 6)
else()
	find_package (Qt5 COMPONENTS Core)
	if (Qt5_FOUND)
		set (QT_VERSION 5)
	else()
		message (FATAL_ERROR "Unable to locate either Qt5 or Qt6!")
	endif()
endif()
message(STATUS "Found Qt${QT_VERSION}")

set (QT_COMPONENTS Core Gui Widgets LinguistTools)
if (QT_VERSION EQUAL 6)
	list (APPEND QT_COMPONENTS Core5Compat)
endif (QT_VERSION EQUAL 6)
find_package ("Qt${QT_VERSION}" COMPONENTS ${QT_COMPONENTS} REQUIRED)

# C/C++ & qrc Qt Resource files
set (BT_SOURCES
	audio/audio_stream.cpp
	audio/audio_stream_rtaudio.cpp
	bamboo_tracker.cpp
	chip/chip.cpp
	chip/mame/fmopn.c
	chip/mame/mame_2608.cpp
	chip/mame/ymdeltat.c
	chip/nuked/nuked_2608.cpp
	chip/nuked/ym3438.c
	chip/opna.cpp
	chip/register_write_logger.cpp
	chip/ymfm/ymfm_2608.cpp
	chip/ymfm/ymfm_adpcm.cpp
	chip/ymfm/ymfm_opn.cpp
	chip/ymfm/ymfm_ssg.cpp
	chip/resampler.cpp
	command/command_manager.cpp
	command/instrument/add_instrument_command.cpp
	command/instrument/change_instrument_name_command.cpp
	command/instrument/clone_instrument_command.cpp
	command/instrument/deep_clone_instrument_command.cpp
	command/instrument/remove_instrument_command.cpp
	command/instrument/swap_instruments_command.cpp
	command/order/clone_order_command.cpp
	command/order/clone_patterns_command.cpp
	command/order/delete_order_command.cpp
	command/order/duplicate_order_command.cpp
	command/order/insert_order_below_command.cpp
	command/order/move_order_command.cpp
	command/order/paste_copied_data_to_order_command.cpp
	command/order/set_pattern_to_order_command.cpp
	command/pattern/change_values_in_pattern_command.cpp
	command/pattern/delete_previous_step_command.cpp
	command/pattern/erase_cells_in_pattern_command.cpp
	command/pattern/erase_effect_in_step_command.cpp
	command/pattern/erase_effect_value_in_step_command.cpp
	command/pattern/erase_instrument_in_step_command.cpp
	command/pattern/erase_step_command.cpp
	command/pattern/erase_volume_in_step_command.cpp
	command/pattern/expand_pattern_command.cpp
	command/pattern/insert_step_command.cpp
	command/pattern/interpolate_pattern_command.cpp
	command/pattern/paste_copied_data_to_pattern_command.cpp
	command/pattern/paste_insert_copied_data_to_pattern_command.cpp
	command/pattern/paste_mix_copied_data_to_pattern_command.cpp
	command/pattern/paste_overwrite_copied_data_to_pattern_command.cpp
	command/pattern/pattern_command_utils.cpp
	command/pattern/replace_instrument_in_pattern_command.cpp
	command/pattern/reverse_pattern_command.cpp
	command/pattern/set_echo_buffer_access_command.cpp
	command/pattern/set_effect_id_to_step_command.cpp
	command/pattern/set_effect_value_to_step_command.cpp
	command/pattern/set_instrument_to_step_command.cpp
	command/pattern/set_key_off_to_step_command.cpp
	command/pattern/set_key_on_to_step_command.cpp
	command/pattern/set_volume_to_step_command.cpp
	command/pattern/shrink_pattern_command.cpp
	command/pattern/transpose_note_in_pattern_command.cpp
	configuration.cpp
	format/wopn_file.c
	gui/bookmark_manager_form.cpp
	gui/color_palette.cpp
	gui/command/instrument/add_instrument_qt_command.cpp
	gui/command/instrument/change_instrument_name_qt_command.cpp
	gui/command/instrument/clone_instrument_qt_command.cpp
	gui/command/instrument/deep_clone_instrument_qt_command.cpp
	gui/command/instrument/instrument_command_qt_utils.cpp
	gui/command/instrument/remove_instrument_qt_command.cpp
	gui/command/instrument/swap_instruments_qt_command.cpp
	gui/command/order/order_list_common_qt_command.cpp
	gui/command/pattern/pattern_editor_common_qt_command.cpp
	gui/comment_edit_dialog.cpp
	gui/configuration_dialog.cpp
	gui/configuration_handler.cpp
	gui/drop_detect_list_widget.cpp
	gui/effect_description.cpp
	gui/effect_list_dialog.cpp
	gui/event_guard.cpp
	gui/file_history.cpp
	gui/file_io_error_message_box.cpp
	gui/font_info_widget.cpp
	gui/fm_envelope_set_edit_dialog.cpp
	gui/go_to_dialog.cpp
	gui/groove_settings_dialog.cpp
	gui/gui_utils.cpp
	gui/hide_tracks_dialog.cpp
	gui/instrument_editor/adpcm_drumkit_editor.cpp
	gui/instrument_editor/adpcm_instrument_editor.cpp
	gui/instrument_editor/adpcm_sample_editor.cpp
	gui/instrument_editor/arpeggio_macro_editor.cpp
	gui/instrument_editor/fm_instrument_editor.cpp
	gui/instrument_editor/fm_operator_table.cpp
	gui/instrument_editor/grid_settings_dialog.cpp
	gui/instrument_editor/instrument_editor.cpp
	gui/instrument_editor/instrument_editor_manager.cpp
	gui/instrument_editor/instrument_editor_utils.cpp
	gui/instrument_editor/pan_macro_editor.cpp
	gui/instrument_editor/sample_length_dialog.cpp
	gui/instrument_editor/ssg_instrument_editor.cpp
	gui/instrument_editor/tone_noise_macro_editor.cpp
	gui/instrument_editor/visualized_instrument_macro_editor.cpp
	gui/instrument_selection_dialog.cpp
	gui/keyboard_shortcut_list_dialog.cpp
	gui/key_signature_manager_form.cpp
	gui/labeled_horizontal_slider.cpp
	gui/labeled_vertical_slider.cpp
	gui/mainwindow.cpp
	gui/module_properties_dialog.cpp
	gui/note_name_manager.cpp
	gui/order_list_editor/order_list_editor.cpp
	gui/order_list_editor/order_list_panel.cpp
	gui/pattern_editor/pattern_editor.cpp
	gui/pattern_editor/pattern_editor_panel.cpp
	gui/q_application_wrapper.cpp
	gui/s98_export_settings_dialog.cpp
	gui/slider_style.cpp
	gui/swap_tracks_dialog.cpp
	gui/transpose_song_dialog.cpp
	gui/vgm_export_settings_dialog.cpp
	gui/wave_export_settings_dialog.cpp
	gui/wave_visual.cpp
	gui/wheel_spin_box.cpp
	instrument/abstract_instrument_property.cpp
	instrument/bank.cpp
	instrument/effect_iterator.cpp
	instrument/envelope_fm.cpp
	instrument/instrument.cpp
	instrument/instruments_manager.cpp
	instrument/lfo_fm.cpp
	instrument/sample_adpcm.cpp
	instrument/sequence_property.cpp
	io/bank_io.cpp
	io/binary_container.cpp
	io/btb_io.cpp
	io/bti_io.cpp
	io/btm_io.cpp
	io/dat_io.cpp
	io/dmp_io.cpp
	io/export_io.cpp
	io/ff_io.cpp
	io/instrument_io.cpp
	io/ins_io.cpp
	io/io_utils.cpp
	io/module_io.cpp
	io/opni_io.cpp
	io/p86_io.cpp
	io/ppc_io.cpp
	io/pps_io.cpp
	io/pvi_io.cpp
	io/pzi_io.cpp
	io/raw_adpcm_io.cpp
	io/tfi_io.cpp
	io/vgi_io.cpp
	io/wav_container.cpp
	io/wopn_io.cpp
	io/y12_io.cpp
	jamming.cpp
	main.cpp
	midi/midi.cpp
	module/effect.cpp
	module/module.cpp
	module/pattern.cpp
	module/song.cpp
	module/step.cpp
	module/track.cpp
	note.cpp
	opna_controller.cpp
	playback.cpp
	precise_timer.cpp
	song_length_calculator.cpp
	tick_counter.cpp

	resources/doc/doc.qrc
	resources/icon/icon.qrc
)

# UI forms
set (BT_FORMS
	gui/bookmark_manager_form.ui
	gui/comment_edit_dialog.ui
	gui/configuration_dialog.ui
	gui/effect_list_dialog.ui
	gui/font_info_widget.ui
	gui/fm_envelope_set_edit_dialog.ui
	gui/go_to_dialog.ui
	gui/groove_settings_dialog.ui
	gui/hide_tracks_dialog.ui
	gui/instrument_editor/adpcm_drumkit_editor.ui
	gui/instrument_editor/adpcm_instrument_editor.ui
	gui/instrument_editor/adpcm_sample_editor.ui
	gui/instrument_editor/fm_instrument_editor.ui
	gui/instrument_editor/fm_operator_table.ui
	gui/instrument_editor/grid_settings_dialog.ui
	gui/instrument_editor/sample_length_dialog.ui
	gui/instrument_editor/ssg_instrument_editor.ui
	gui/instrument_editor/visualized_instrument_macro_editor.ui
	gui/instrument_selection_dialog.ui
	gui/keyboard_shortcut_list_dialog.ui
	gui/key_signature_manager_form.ui
	gui/labeled_horizontal_slider.ui
	gui/labeled_vertical_slider.ui
	gui/mainwindow.ui
	gui/module_properties_dialog.ui
	gui/order_list_editor/order_list_editor.ui
	gui/pattern_editor/pattern_editor.ui
	gui/s98_export_settings_dialog.ui
	gui/swap_tracks_dialog.ui
	gui/transpose_song_dialog.ui
	gui/vgm_export_settings_dialog.ui
	gui/wave_export_settings_dialog.ui
)

set (BT_INCLUDEPATHS
	"${CMAKE_CURRENT_SOURCE_DIR}"
	instrument
	module
)

option (REAL_CHIP "Compile with support for SCCI and C86CTL interfaces to a real OPNA chip" ${WIN32})

if (REAL_CHIP)
	list (APPEND BT_SOURCES
		chip/c86ctl/c86ctl_wrapper.cpp
		chip/scci/scci_wrapper.cpp
	)
	list (APPEND BT_INCLUDEPATHS
		chip/c86ctl
		chip/scci
	)
endif (REAL_CHIP)

if (APPLE)
	set (MACOSX_BUNDLE_ICON_FILE BambooTracker.icns)
	set (MACOSX_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/${MACOSX_BUNDLE_ICON_FILE}")
	set_source_files_properties (${MACOSX_ICNS}
		PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
	)
endif()
add_executable (BambooTracker WIN32 MACOSX_BUNDLE
	${BT_SOURCES} ${BT_FORMS}
	${MACOSX_ICNS}
)

target_include_directories (BambooTracker PRIVATE ${BT_INCLUDEPATHS})
target_compile_options (BambooTracker PRIVATE ${BT_WARNFLAGS})

if (REAL_CHIP)
	target_compile_definitions (BambooTracker PRIVATE USE_REAL_CHIP)
endif (REAL_CHIP)

if (QT_VERSION EQUAL 6)
	set (QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat)
else()
	set (QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
target_link_libraries (BambooTracker PUBLIC ${QT_LIBRARIES})

# Dependencies
set (THREADS_PREFER_PTHREAD_FLAG ON)
include (FindThreads REQUIRED)
target_include_directories (BambooTracker SYSTEM PRIVATE ${EMU2149_INCLUDE_DIRS} ${RTAUDIO_INCLUDE_DIRS} ${RTMIDI_INCLUDE_DIRS})
target_compile_options (BambooTracker PRIVATE ${EMU2149_COMPILE_OPTIONS} ${RTAUDIO_COMPILE_OPTIONS} ${RTMIDI_COMPILE_OPTIONS})
if ("${CMAKE_VERSION}" VERSION_LESS "3.13")
	message (WARNING
		"CMake version is <3.13, using old pkg-config LDFLAGS. "
		"You may encounter linking problems with these!"
	)
	target_link_libraries (BambooTracker PRIVATE ${EMU2149_LDFLAGS_LEGACY} ${RTAUDIO_LDFLAGS_LEGACY} ${RTMIDI_LDFLAGS_LEGACY} Threads::Threads)
else()
	target_link_libraries (BambooTracker PRIVATE ${EMU2149_LIBRARIES} ${RTAUDIO_LIBRARIES} ${RTMIDI_LIBRARIES} Threads::Threads)
	target_link_directories (BambooTracker PRIVATE ${EMU2149_LINK_DIRS} ${RTAUDIO_LINK_DIRS} ${RTMIDI_LINK_DIRS})
	target_link_options (BambooTracker PRIVATE ${EMU2149_LINK_OPTIONS} ${RTAUDIO_LINK_OPTIONS} ${RTMIDI_LINK_OPTIONS})
endif()

install (TARGETS BambooTracker DESTINATION "${CMAKE_INSTALL_BINDIR}")

add_subdirectory (lang)
