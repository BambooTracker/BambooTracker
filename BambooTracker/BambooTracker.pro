#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T16:20:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
# Porting helper Qt5 -> Qt6
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TARGET = BambooTracker
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include("../qmake/variables.pri")

INSTALLS += target
win32|install_flat {
    target.path = $$PREFIX
}
else {
    target.path = $$PREFIX/bin
}

CONFIG += c++14
clang|if(gcc:!intel_icc) {
  QMAKE_CFLAGS += -std=gnu11
}

# Temporary downgrades of problematic compiler flags here
# CPP_WARNING_FLAGS +=

QMAKE_CFLAGS_WARN_ON += $$CPP_WARNING_FLAGS
QMAKE_CXXFLAGS_WARN_ON += $$CPP_WARNING_FLAGS

SOURCES += \
    chip/blip_buf/blip_buf.c \
    chip/mame/fmopn.c \
    chip/mame/mame_2608.cpp \
    chip/mame/ymdeltat.c \
    chip/nuked/nuked_2608.cpp \
    chip/register_write_logger.cpp \
    chip/ymfm/ymfm_2608.cpp \
    chip/ymfm/ymfm_adpcm.cpp \
    chip/ymfm/ymfm_opn.cpp \
    chip/ymfm/ymfm_ssg.cpp \
    command/instrument/swap_instruments_command.cpp \
    command/pattern/change_values_in_pattern_command.cpp \
    command/pattern/paste_insert_copied_data_to_pattern_command.cpp \
    command/pattern/pattern_command_utils.cpp \
    command/pattern/set_key_cut_to_step_command.cpp \
    command/pattern/transpose_note_in_pattern_command.cpp \
    gui/bookmark_manager_form.cpp \
    gui/command/instrument/instrument_command_qt_utils.cpp \
    gui/command/instrument/swap_instruments_qt_command.cpp \
    gui/command/order/order_list_common_qt_command.cpp \
    gui/command/pattern/pattern_editor_common_qt_command.cpp \
    gui/drop_detect_list_widget.cpp \
    gui/effect_description.cpp \
    gui/effect_list_dialog.cpp \
    gui/file_io_error_message_box.cpp \
    gui/font_info_widget.cpp \
    gui/go_to_dialog.cpp \
    gui/gui_utils.cpp \
    gui/hide_tracks_dialog.cpp \
    gui/instrument_editor/adpcm_address_spin_box.cpp \
    gui/instrument_editor/adpcm_drumkit_editor.cpp \
    gui/instrument_editor/adpcm_instrument_editor.cpp \
    gui/instrument_editor/adpcm_sample_editor.cpp \
    gui/instrument_editor/arpeggio_macro_editor.cpp \
    gui/instrument_editor/fm_instrument_editor.cpp \
    gui/instrument_editor/grid_settings_dialog.cpp \
    gui/instrument_editor/instrument_editor.cpp \
    gui/instrument_editor/instrument_editor_manager.cpp \
    gui/instrument_editor/instrument_editor_utils.cpp \
    gui/instrument_editor/pan_macro_editor.cpp \
    gui/instrument_editor/sample_length_dialog.cpp \
    gui/instrument_editor/ssg_instrument_editor.cpp \
    gui/instrument_editor/tone_noise_macro_editor.cpp \
    gui/key_signature_manager_form.cpp \
    gui/keyboard_shortcut_list_dialog.cpp \
    gui/note_name_manager.cpp \
    gui/swap_tracks_dialog.cpp \
    gui/transpose_song_dialog.cpp \
    gui/wheel_spin_box.cpp \
    instrument/sample_adpcm.cpp \
    instrument/sequence_property.cpp \
    io/btb_io.cpp \
    io/bti_io.cpp \
    io/btm_io.cpp \
    io/dat_io.cpp \
    io/dmp_io.cpp \
    io/export_io.cpp \
    io/ff_io.cpp \
    io/ins_io.cpp \
    io/io_utils.cpp \
    io/opni_io.cpp \
    io/p86_io.cpp \
    io/pmb_io.cpp \
    io/ppc_io.cpp \
    io/pps_io.cpp \
    io/pvi_io.cpp \
    io/pzi_io.cpp \
    io/raw_adpcm_io.cpp \
    io/tfi_io.cpp \
    io/vgi_io.cpp \
    io/wav_container.cpp \
    io/wopn_io.cpp \
    io/y12_io.cpp \
    jamming.cpp \
    main.cpp \
    gui/mainwindow.cpp \
    chip/chip.cpp \
    chip/opna.cpp \
    chip/resampler.cpp \
    chip/nuked/ym3438.c \
    bamboo_tracker.cpp \
    module/effect.cpp \
    note.cpp \
    playback.cpp \
    song_length_calculator.cpp \
    audio/audio_stream.cpp \
    instrument/instruments_manager.cpp \
    command/command_manager.cpp \
    command/instrument/add_instrument_command.cpp \
    command/instrument/remove_instrument_command.cpp \
    gui/command/instrument/add_instrument_qt_command.cpp \
    gui/command/instrument/remove_instrument_qt_command.cpp \
    gui/instrument_editor/fm_operator_table.cpp \
    gui/labeled_vertical_slider.cpp \
    gui/labeled_horizontal_slider.cpp \
    gui/slider_style.cpp \
    gui/command/instrument/change_instrument_name_qt_command.cpp \
    command/instrument/change_instrument_name_command.cpp \
    opna_controller.cpp \
    instrument/instrument.cpp \
    instrument/envelope_fm.cpp \
    gui/event_guard.cpp \
    audio/audio_stream_rtaudio.cpp \
    tick_counter.cpp \
    module/module.cpp \
    module/song.cpp \
    module/pattern.cpp \
    module/track.cpp \
    module/step.cpp \
    gui/order_list_editor/order_list_panel.cpp \
    gui/order_list_editor/order_list_editor.cpp \
    gui/pattern_editor/pattern_editor_panel.cpp \
    gui/pattern_editor/pattern_editor.cpp \
    command/pattern/set_key_off_to_step_command.cpp \
    command/pattern/set_key_on_to_step_command.cpp \
    command/pattern/set_instrument_to_step_command.cpp \
    command/pattern/erase_instrument_in_step_command.cpp \
    command/pattern/set_volume_to_step_command.cpp \
    command/pattern/erase_volume_in_step_command.cpp \
    command/pattern/set_effect_id_to_step_command.cpp \
    command/pattern/erase_effect_in_step_command.cpp \
    command/pattern/set_effect_value_to_step_command.cpp \
    command/pattern/erase_effect_value_in_step_command.cpp \
    command/pattern/insert_step_command.cpp \
    command/pattern/delete_previous_step_command.cpp \
    command/pattern/erase_step_command.cpp \
    gui/command/instrument/deep_clone_instrument_qt_command.cpp \
    command/instrument/deep_clone_instrument_command.cpp \
    command/instrument/clone_instrument_command.cpp \
    gui/command/instrument/clone_instrument_qt_command.cpp \
    command/order/set_pattern_to_order_command.cpp \
    command/order/insert_order_below_command.cpp \
    command/order/delete_order_command.cpp \
    command/pattern/paste_copied_data_to_pattern_command.cpp \
    command/pattern/erase_cells_in_pattern_command.cpp \
    command/order/paste_copied_data_to_order_command.cpp \
    instrument/lfo_fm.cpp \
    gui/instrument_editor/visualized_instrument_macro_editor.cpp \
    instrument/effect_iterator.cpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.cpp \
    gui/module_properties_dialog.cpp \
    gui/groove_settings_dialog.cpp \
    gui/configuration_dialog.cpp \
    command/pattern/expand_pattern_command.cpp \
    command/pattern/shrink_pattern_command.cpp \
    instrument/abstract_instrument_property.cpp \
    command/order/duplicate_order_command.cpp \
    command/order/move_order_command.cpp \
    command/order/clone_patterns_command.cpp \
    command/order/clone_order_command.cpp \
    command/pattern/set_echo_buffer_access_command.cpp \
    gui/comment_edit_dialog.cpp \
    io/binary_container.cpp \
    command/pattern/interpolate_pattern_command.cpp \
    command/pattern/reverse_pattern_command.cpp \
    command/pattern/replace_instrument_in_pattern_command.cpp \
    gui/vgm_export_settings_dialog.cpp \
    gui/wave_export_settings_dialog.cpp \
    configuration.cpp \
    gui/configuration_handler.cpp \
    gui/color_palette.cpp \
    command/pattern/paste_overwrite_copied_data_to_pattern_command.cpp \
    format/wopn_file.c \
    instrument/bank.cpp \
    gui/instrument_selection_dialog.cpp \
    gui/s98_export_settings_dialog.cpp \
    precise_timer.cpp \
    io/module_io.cpp \
    io/instrument_io.cpp \
    io/bank_io.cpp \
    gui/fm_envelope_set_edit_dialog.cpp \
    gui/file_history.cpp \
    midi/midi.cpp \
    gui/q_application_wrapper.cpp \
    gui/wave_visual.cpp

HEADERS += \
    bamboo_tracker_defs.hpp \
    chip/2608_interface.hpp \
    chip/blip_buf/blip_buf.h \
    chip/chip_defs.h \
    chip/codec/ymb_codec.hpp \
    chip/mame/fmopn.h \
    chip/mame/fmopn_2608rom.h \
    chip/mame/mame_2608.hpp \
    chip/mame/mamedefs.h \
    chip/mame/ymdeltat.h \
    chip/nuked/nuked_2608.hpp \
    chip/real_chip_interface.hpp \
    chip/register_write_logger.hpp \
    chip/ymfm/ymfm.h \
    chip/ymfm/ymfm_2608.hpp \
    chip/ymfm/ymfm_adpcm.h \
    chip/ymfm/ymfm_fm.h \
    chip/ymfm/ymfm_fm.ipp \
    chip/ymfm/ymfm_opn.h \
    chip/ymfm/ymfm_ssg.h \
    command/command_id.hpp \
    command/instrument/swap_instruments_command.hpp \
    command/pattern/change_values_in_pattern_command.hpp \
    command/pattern/paste_insert_copied_data_to_pattern_command.hpp \
    command/pattern/pattern_command_utils.hpp \
    command/pattern/set_key_cut_to_step_command.hpp \
    command/pattern/transpose_note_in_pattern_command.hpp \
    echo_buffer.hpp \
    enum_hash.hpp \
    gui/bookmark_manager_form.hpp \
    gui/command/instrument/instrument_command_qt_utils.hpp \
    gui/command/instrument/instrument_commands_qt.hpp \
    gui/command/instrument/swap_instruments_qt_command.hpp \
    gui/command/order/order_commands_qt.hpp \
    gui/command/order/order_list_common_qt_command.hpp \
    gui/command/pattern/pattern_editor_common_qt_command.hpp \
    gui/dpi.hpp \
    gui/drop_detect_list_widget.hpp \
    gui/effect_description.hpp \
    gui/effect_list_dialog.hpp \
    gui/file_io_error_message_box.hpp \
    gui/font_info_widget.hpp \
    gui/go_to_dialog.hpp \
    gui/gui_utils.hpp \
    gui/hide_tracks_dialog.hpp \
    gui/instrument_editor/adpcm_address_spin_box.hpp \
    gui/instrument_editor/adpcm_drumkit_editor.hpp \
    gui/instrument_editor/adpcm_instrument_editor.hpp \
    gui/instrument_editor/adpcm_sample_editor.hpp \
    gui/instrument_editor/arpeggio_macro_editor.hpp \
    gui/instrument_editor/fm_instrument_editor.hpp \
    gui/instrument_editor/grid_settings_dialog.hpp \
    gui/instrument_editor/instrument_editor.hpp \
    gui/instrument_editor/instrument_editor_manager.hpp \
    gui/instrument_editor/instrument_editor_utils.hpp \
    gui/instrument_editor/pan_macro_editor.hpp \
    gui/instrument_editor/sample_length_dialog.hpp \
    gui/instrument_editor/ssg_instrument_editor.hpp \
    gui/instrument_editor/tone_noise_macro_editor.hpp \
    gui/jam_layout.hpp \
    gui/key_signature_manager_form.hpp \
    gui/keyboard_shortcut_list_dialog.hpp \
    gui/mainwindow.hpp \
    chip/nuked/ym3438.h \
    chip/chip.hpp \
    chip/opna.hpp \
    chip/resampler.hpp \
    bamboo_tracker.hpp \
    gui/note_name_manager.hpp \
    gui/swap_tracks_dialog.hpp \
    gui/transpose_song_dialog.hpp \
    gui/wheel_spin_box.hpp \
    instrument/instrument_property_defs.hpp \
    instrument/sample_adpcm.hpp \
    instrument/sample_repeat.hpp \
    instrument/sequence_property.hpp \
    io/btb_io.hpp \
    io/bti_io.hpp \
    io/btm_io.hpp \
    io/dat_io.hpp \
    io/dmp_io.hpp \
    io/export_io.hpp \
    io/ff_io.hpp \
    io/ins_io.hpp \
    io/io_file_type.hpp \
    io/io_utils.hpp \
    io/opni_io.hpp \
    io/p86_io.hpp \
    io/pmb_io.hpp \
    io/ppc_io.hpp \
    io/pps_io.hpp \
    io/pvi_io.hpp \
    io/pzi_io.hpp \
    io/raw_adpcm_io.hpp \
    io/tfi_io.hpp \
    io/vgi_io.hpp \
    io/wav_container.hpp \
    io/wopn_io.hpp \
    io/y12_io.hpp \
    jamming.hpp \
    module/effect.hpp \
    note.hpp \
    playback.hpp \
    song_length_calculator.hpp \
    audio/audio_stream.hpp \
    instrument/instruments_manager.hpp \
    command/command_manager.hpp \
    command/instrument/add_instrument_command.hpp \
    command/instrument/remove_instrument_command.hpp \
    command/commands.hpp \
    gui/command/instrument/add_instrument_qt_command.hpp \
    gui/command/instrument/remove_instrument_qt_command.hpp \
    gui/instrument_editor/fm_operator_table.hpp \
    gui/labeled_vertical_slider.hpp \
    gui/labeled_horizontal_slider.hpp \
    gui/slider_style.hpp \
    gui/command/instrument/change_instrument_name_qt_command.hpp \
    command/instrument/change_instrument_name_command.hpp \
    opna_controller.hpp \
    instrument/instrument.hpp \
    instrument/envelope_fm.hpp \
    gui/event_guard.hpp \
    audio/audio_stream_rtaudio.hpp \
    tick_counter.hpp \
    module/module.hpp \
    module/song.hpp \
    module/pattern.hpp \
    module/track.hpp \
    module/step.hpp \
    gui/order_list_editor/order_list_panel.hpp \
    gui/order_list_editor/order_list_editor.hpp \
    gui/pattern_editor/pattern_editor_panel.hpp \
    gui/pattern_editor/pattern_editor.hpp \
    command/pattern/set_key_off_to_step_command.hpp \
    gui/command/pattern/pattern_commands_qt.hpp \
    command/pattern/set_key_on_to_step_command.hpp \
    gui/pattern_editor/pattern_position.hpp \
    command/pattern/set_instrument_to_step_command.hpp \
    command/pattern/erase_instrument_in_step_command.hpp \
    command/pattern/set_volume_to_step_command.hpp \
    command/pattern/erase_volume_in_step_command.hpp \
    command/pattern/set_effect_id_to_step_command.hpp \
    command/pattern/erase_effect_in_step_command.hpp \
    command/pattern/set_effect_value_to_step_command.hpp \
    command/pattern/erase_effect_value_in_step_command.hpp \
    command/pattern/insert_step_command.hpp \
    command/pattern/delete_previous_step_command.hpp \
    command/pattern/erase_step_command.hpp \
    gui/command/instrument/deep_clone_instrument_qt_command.hpp \
    command/instrument/deep_clone_instrument_command.hpp \
    command/instrument/clone_instrument_command.hpp \
    gui/command/instrument/clone_instrument_qt_command.hpp \
    gui/order_list_editor/order_position.hpp \
    command/order/set_pattern_to_order_command.hpp \
    command/order/insert_order_below_command.hpp \
    command/order/delete_order_command.hpp \
    command/pattern/paste_copied_data_to_pattern_command.hpp \
    command/pattern/erase_cells_in_pattern_command.hpp \
    command/order/paste_copied_data_to_order_command.hpp \
    instrument/lfo_fm.hpp \
    gui/instrument_editor/visualized_instrument_macro_editor.hpp \
    instrument/sequence_iterator_interface.hpp \
    instrument/effect_iterator.hpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.hpp \
    gui/module_properties_dialog.hpp \
    gui/groove_settings_dialog.hpp \
    gui/configuration_dialog.hpp \
    command/pattern/expand_pattern_command.hpp \
    command/pattern/shrink_pattern_command.hpp \
    command/abstract_command.hpp \
    instrument/abstract_instrument_property.hpp \
    command/order/duplicate_order_command.hpp \
    command/order/move_order_command.hpp \
    command/order/clone_patterns_command.hpp \
    command/order/clone_order_command.hpp \
    command/pattern/set_echo_buffer_access_command.hpp \
    gui/comment_edit_dialog.hpp \
    io/binary_container.hpp \
    utils.hpp \
    version.hpp \
    command/pattern/interpolate_pattern_command.hpp \
    command/pattern/reverse_pattern_command.hpp \
    command/pattern/replace_instrument_in_pattern_command.hpp \
    gui/vgm_export_settings_dialog.hpp \
    gui/wave_export_settings_dialog.hpp \
    configuration.hpp \
    gui/configuration_handler.hpp \
    gui/color_palette.hpp \
    command/pattern/paste_overwrite_copied_data_to_pattern_command.hpp \
    io/file_io_error.hpp \
    format/wopn_file.h \
    instrument/bank.hpp \
    gui/instrument_selection_dialog.hpp \
    gui/s98_export_settings_dialog.hpp \
    precise_timer.hpp \
    io/module_io.hpp \
    io/instrument_io.hpp \
    io/bank_io.hpp \
    gui/fm_envelope_set_edit_dialog.hpp \
    gui/file_history.hpp \
    midi/midi.hpp \
    gui/q_application_wrapper.hpp \
    gui/wave_visual.hpp

FORMS += \
    gui/bookmark_manager_form.ui \
    gui/effect_list_dialog.ui \
    gui/font_info_widget.ui \
    gui/go_to_dialog.ui \
    gui/hide_tracks_dialog.ui \
    gui/instrument_editor/adpcm_drumkit_editor.ui \
    gui/instrument_editor/adpcm_instrument_editor.ui \
    gui/instrument_editor/adpcm_sample_editor.ui \
    gui/instrument_editor/fm_instrument_editor.ui \
    gui/instrument_editor/grid_settings_dialog.ui \
    gui/instrument_editor/sample_length_dialog.ui \
    gui/instrument_editor/ssg_instrument_editor.ui \
    gui/key_signature_manager_form.ui \
    gui/keyboard_shortcut_list_dialog.ui \
    gui/mainwindow.ui \
    gui/instrument_editor/fm_operator_table.ui \
    gui/labeled_vertical_slider.ui \
    gui/labeled_horizontal_slider.ui \
    gui/order_list_editor/order_list_editor.ui \
    gui/pattern_editor/pattern_editor.ui \
    gui/instrument_editor/visualized_instrument_macro_editor.ui \
    gui/module_properties_dialog.ui \
    gui/groove_settings_dialog.ui \
    gui/configuration_dialog.ui \
    gui/comment_edit_dialog.ui \
    gui/swap_tracks_dialog.ui \
    gui/transpose_song_dialog.ui \
    gui/vgm_export_settings_dialog.ui \
    gui/wave_export_settings_dialog.ui \
    gui/instrument_selection_dialog.ui \
    gui/s98_export_settings_dialog.ui \
    gui/fm_envelope_set_edit_dialog.ui

INCLUDEPATH += \
    $$PWD/instrument \
    $$PWD/module

# In-app resource bundle. Needs to be handled here because it generates an object file to link against
include("resources/resources.pri")

# App translations. lupdate requires the source code for updating these to work
include("lang/lang.pri")

CONFIG += link_prl
system_* {
  CONFIG += link_pkgconfig
}

INCLUDEPATH += $$PWD/../submodules/emu2149/src
LIBS += -L$$OUT_PWD/../submodules/emu2149
CONFIG(debug, debug|release):LIBS += -lemu2149d
else:CONFIG(release, debug|release):LIBS += -lemu2149

system_rtaudio {
  PKGCONFIG += rtaudio
}
else {
  INCLUDEPATH += $$PWD/../submodules/RtAudio/src
  LIBS += -L$$OUT_PWD/../submodules/RtAudio
  CONFIG(debug, debug|release):LIBS += -lrtaudiod
  else:CONFIG(release, debug|release):LIBS += -lrtaudio
}

system_rtmidi {
  PKGCONFIG += rtmidi
}
else {
  INCLUDEPATH += $$PWD/../submodules/RtMidi/src
  LIBS += -L$$OUT_PWD/../submodules/RtMidi
  CONFIG(debug, debug|release):LIBS += -lrtmidid
  else:CONFIG(release, debug|release):LIBS += -lrtmidi
}

win32:CONFIG += real_chip
real_chip {
  DEFINES += USE_REAL_CHIP

  SOURCES += \
    chip/c86ctl/c86ctl_wrapper.cpp \
    chip/scci/scci_wrapper.cpp

  HEADERS += \
    chip/c86ctl/c86ctl.h \
    chip/c86ctl/c86ctl_wrapper.hpp \
    chip/scci/SCCIDefines.hpp \
    chip/scci/scci.hpp \
    chip/scci/scci_wrapper.hpp
}
