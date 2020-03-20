#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T16:20:11
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

# This produces the installation rule for the program and resources.
# Use a default destination prefix if none is given.
isEmpty(PREFIX) {
    win32:PREFIX = C:/BambooTracker
    else:PREFIX = /usr/local
}
INSTALLS += target
win32 {
    QM_FILES_INSTALL_PATH = $$PREFIX/lang
    target.path = $$PREFIX
}
else {
    QM_FILES_INSTALL_PATH = $$PREFIX/share/BambooTracker/lang
    target.path = $$PREFIX/bin
}

CONFIG += c++14

SOURCES += \
    chips/c86ctl/c86ctl_wrapper.cpp \
    command/pattern/change_values_in_pattern_command.cpp \
    command/pattern/pattern_command_utils.cpp \
    command/pattern/transpose_note_in_pattern_command.cpp \
    gui/bookmark_manager_form.cpp \
    gui/color_palette_handler.cpp \
    gui/command/pattern/change_values_in_pattern_qt_command.cpp \
    gui/command/pattern/transpose_note_in_pattern_qt_command.cpp \
    gui/effect_description.cpp \
    gui/effect_list_dialog.cpp \
    gui/instrument_editor/arpeggio_macro_editor.cpp \
    gui/instrument_editor/instrument_editor_adpcm_form.cpp \
    gui/instrument_editor/tone_noise_macro_editor.cpp \
    gui/keyboard_shortcut_list_dialog.cpp \
    instrument/waveform_adpcm.cpp \
    io/wav_container.cpp \
    main.cpp \
    gui/mainwindow.cpp \
    chips/chip.cpp \
    chips/opna.cpp \
    chips/resampler.cpp \
    chips/mame/2608intf.c \
    chips/mame/emu2149.c \
    chips/mame/fm.c \
    chips/mame/ymdeltat.c \
    chips/nuked/nuke2608intf.c \
    chips/nuked/ym3438.c \
    bamboo_tracker.cpp \
    module/effect.cpp \
    playback.cpp \
    stream/audio_stream.cpp \
    jam_manager.cpp \
    pitch_converter.cpp \
    instrument/instruments_manager.cpp \
    command/command_manager.cpp \
    command/instrument/add_instrument_command.cpp \
    command/instrument/remove_instrument_command.cpp \
    gui/command/instrument/add_instrument_qt_command.cpp \
    gui/command/instrument/remove_instrument_qt_command.cpp \
    gui/instrument_editor/instrument_editor_fm_form.cpp \
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
    stream/audio_stream_rtaudio.cpp \
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
    gui/instrument_editor/instrument_editor_ssg_form.cpp \
    gui/command/pattern/set_key_off_to_step_qt_command.cpp \
    command/pattern/set_key_off_to_step_command.cpp \
    command/pattern/set_key_on_to_step_command.cpp \
    gui/command/pattern/set_key_on_to_step_qt_command.cpp \
    gui/command/pattern/set_instrument_to_step_qt_command.cpp \
    command/pattern/set_instrument_to_step_command.cpp \
    gui/command/pattern/erase_instrument_in_step_qt_command.cpp \
    command/pattern/erase_instrument_in_step_command.cpp \
    gui/command/pattern/set_volume_to_step_qt_command.cpp \
    command/pattern/set_volume_to_step_command.cpp \
    gui/command/pattern/erase_volume_in_step_qt_command.cpp \
    command/pattern/erase_volume_in_step_command.cpp \
    command/pattern/set_effect_id_to_step_command.cpp \
    gui/command/pattern/set_effect_id_to_step_qt_command.cpp \
    command/pattern/erase_effect_in_step_command.cpp \
    gui/command/pattern/erase_effect_in_step_qt_command.cpp \
    command/pattern/set_effect_value_to_step_command.cpp \
    gui/command/pattern/set_effect_value_to_step_qt_command.cpp \
    command/pattern/erase_effect_value_in_step_command.cpp \
    gui/command/pattern/erase_effect_value_in_step_qt_command.cpp \
    command/pattern/insert_step_command.cpp \
    command/pattern/delete_previous_step_command.cpp \
    gui/command/pattern/insert_step_qt_command.cpp \
    gui/command/pattern/delete_previous_step_qt_command.cpp \
    gui/command/pattern/erase_step_qt_command.cpp \
    command/pattern/erase_step_command.cpp \
    gui/command/instrument/deep_clone_instrument_qt_command.cpp \
    command/instrument/deep_clone_instrument_command.cpp \
    command/instrument/clone_instrument_command.cpp \
    gui/command/instrument/clone_instrument_qt_command.cpp \
    command/order/set_pattern_to_order_command.cpp \
    gui/command/order/set_pattern_to_order_qt_command.cpp \
    command/order/insert_order_below_command.cpp \
    command/order/delete_order_command.cpp \
    gui/command/order/insert_order_below_qt_command.cpp \
    gui/command/order/delete_order_qt_command.cpp \
    command/pattern/paste_copied_data_to_pattern_command.cpp \
    gui/command/pattern/paste_copied_data_to_pattern_qt_command.cpp \
    command/pattern/erase_cells_in_pattern_command.cpp \
    gui/command/pattern/erase_cells_in_pattern_qt_command.cpp \
    command/order/paste_copied_data_to_order_command.cpp \
    gui/command/order/paste_copied_data_to_order_qt_command.cpp \
    gui/instrument_editor/instrument_form_manager.cpp \
    instrument/lfo_fm.cpp \
    gui/instrument_editor/visualized_instrument_macro_editor.cpp \
    instrument/command_sequence.cpp \
    instrument/effect_iterator.cpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.cpp \
    gui/command/pattern/paste_mix_copied_data_to_pattern_qt_command.cpp \
    gui/module_properties_dialog.cpp \
    module/groove.cpp \
    gui/groove_settings_dialog.cpp \
    gui/configuration_dialog.cpp \
    command/pattern/expand_pattern_command.cpp \
    gui/command/pattern/expand_pattern_qt_command.cpp \
    command/pattern/shrink_pattern_command.cpp \
    gui/command/pattern/shrink_pattern_qt_command.cpp \
    instrument/abstract_instrument_property.cpp \
    command/order/duplicate_order_command.cpp \
    command/order/move_order_command.cpp \
    command/order/clone_patterns_command.cpp \
    command/order/clone_order_command.cpp \
    gui/command/order/duplicate_order_qt_command.cpp \
    gui/command/order/move_order_qt_command.cpp \
    gui/command/order/clone_patterns_qt_command.cpp \
    gui/command/order/clone_order_qt_command.cpp \
    gui/command/pattern/set_echo_buffer_access_qt_command.cpp \
    command/pattern/set_echo_buffer_access_command.cpp \
    gui/comment_edit_dialog.cpp \
    io/file_io.cpp \
    io/binary_container.cpp \
    gui/command/pattern/interpolate_pattern_qt_command.cpp \
    command/pattern/interpolate_pattern_command.cpp \
    gui/command/pattern/reverse_pattern_qt_command.cpp \
    command/pattern/reverse_pattern_command.cpp \
    gui/command/pattern/replace_instrument_in_pattern_qt_command.cpp \
    command/pattern/replace_instrument_in_pattern_command.cpp \
    chips/export_container.cpp \
    gui/vgm_export_settings_dialog.cpp \
    gui/wave_export_settings_dialog.cpp \
    configuration.cpp \
    gui/configuration_handler.cpp \
    gui/color_palette.cpp \
    gui/command/pattern/paste_overwrite_copied_data_to_pattern_qt_command.cpp \
    command/pattern/paste_overwrite_copied_data_to_pattern_command.cpp \
    io/file_io_error.cpp \
    format/wopn_file.c \
    instrument/bank.cpp \
    gui/instrument_selection_dialog.cpp \
    gui/s98_export_settings_dialog.cpp \
    stream/timer.cpp \
    io/module_io.cpp \
    io/export_handler.cpp \
    io/instrument_io.cpp \
    io/bank_io.cpp \
    gui/fm_envelope_set_edit_dialog.cpp \
    gui/file_history_handler.cpp \
    gui/file_history.cpp \
    midi/midi.cpp \
    gui/q_application_wrapper.cpp \
    gui/wave_visual.cpp

HEADERS += \
    chips/codec/ymb_codec.hpp \
    chips/c86ctl/c86ctl.h \
    chips/c86ctl/c86ctl_wrapper.hpp \
    command/command_id.hpp \
    command/pattern/change_values_in_pattern_command.hpp \
    command/pattern/pattern_command_utils.hpp \
    command/pattern/transpose_note_in_pattern_command.hpp \
    enum_hash.hpp \
    gui/bookmark_manager_form.hpp \
    gui/color_palette_handler.hpp \
    gui/command/pattern/change_values_in_pattern_qt_command.hpp \
    gui/command/pattern/transpose_note_in_pattern_qt_command.hpp \
    gui/effect_description.hpp \
    gui/effect_list_dialog.hpp \
    gui/instrument_editor/arpeggio_macro_editor.hpp \
    gui/instrument_editor/instrument_editor_adpcm_form.hpp \
    gui/instrument_editor/tone_noise_macro_editor.hpp \
    gui/instrument_list_misc.hpp \
    gui/jam_layout.hpp \
    gui/keyboard_shortcut_list_dialog.hpp \
    gui/mainwindow.hpp \
    chips/mame/2608intf.h \
    chips/mame/emu2149.h \
    chips/mame/emutypes.h \
    chips/mame/fm.h \
    chips/mame/mamedef.h \
    chips/mame/ymdeltat.h \
    chips/nuked/nuke2608intf.h \
    chips/nuked/ym3438.h \
    chips/chip.hpp \
    chips/chip_misc.h \
    chips/opna.hpp \
    chips/resampler.hpp \
    bamboo_tracker.hpp \
    gui/shortcut_util.hpp \
    instrument/waveform_adpcm.hpp \
    io/wav_container.hpp \
    module/effect.hpp \
    playback.hpp \
    stream/audio_stream.hpp \
    chips/chip_def.h \
    jam_manager.hpp \
    misc.hpp \
    pitch_converter.hpp \
    instrument/instruments_manager.hpp \
    command/command_manager.hpp \
    command/instrument/add_instrument_command.hpp \
    command/instrument/remove_instrument_command.hpp \
    command/commands.hpp \
    gui/command/instrument/add_instrument_qt_command.hpp \
    gui/command/commands_qt.hpp \
    gui/command/instrument/remove_instrument_qt_command.hpp \
    gui/instrument_editor/instrument_editor_fm_form.hpp \
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
    stream/audio_stream_rtaudio.hpp \
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
    gui/instrument_editor/instrument_editor_ssg_form.hpp \
    gui/command/pattern/set_key_off_to_step_qt_command.hpp \
    command/pattern/set_key_off_to_step_command.hpp \
    gui/command/pattern/pattern_commands_qt.hpp \
    command/pattern/set_key_on_to_step_command.hpp \
    gui/command/pattern/set_key_on_to_step_qt_command.hpp \
    gui/pattern_editor/pattern_position.hpp \
    gui/command/pattern/set_instrument_to_step_qt_command.hpp \
    command/pattern/set_instrument_to_step_command.hpp \
    gui/command/pattern/erase_instrument_in_step_qt_command.hpp \
    command/pattern/erase_instrument_in_step_command.hpp \
    gui/command/pattern/set_volume_to_step_qt_command.hpp \
    command/pattern/set_volume_to_step_command.hpp \
    gui/command/pattern/erase_volume_in_step_qt_command.hpp \
    command/pattern/erase_volume_in_step_command.hpp \
    command/pattern/set_effect_id_to_step_command.hpp \
    gui/command/pattern/set_effect_id_to_step_qt_command.hpp \
    command/pattern/erase_effect_in_step_command.hpp \
    gui/command/pattern/erase_effect_in_step_qt_command.hpp \
    command/pattern/set_effect_value_to_step_command.hpp \
    gui/command/pattern/set_effect_value_to_step_qt_command.hpp \
    command/pattern/erase_effect_value_in_step_command.hpp \
    gui/command/pattern/erase_effect_value_in_step_qt_command.hpp \
    command/pattern/insert_step_command.hpp \
    command/pattern/delete_previous_step_command.hpp \
    gui/command/pattern/insert_step_qt_command.hpp \
    gui/command/pattern/delete_previous_step_qt_command.hpp \
    gui/command/pattern/erase_step_qt_command.hpp \
    command/pattern/erase_step_command.hpp \
    gui/command/instrument/deep_clone_instrument_qt_command.hpp \
    command/instrument/deep_clone_instrument_command.hpp \
    command/instrument/clone_instrument_command.hpp \
    gui/command/instrument/clone_instrument_qt_command.hpp \
    gui/order_list_editor/order_position.hpp \
    command/order/set_pattern_to_order_command.hpp \
    gui/command/order/set_pattern_to_order_qt_command.hpp \
    gui/command/order/order_commands.hpp \
    command/order/insert_order_below_command.hpp \
    command/order/delete_order_command.hpp \
    gui/command/order/insert_order_below_qt_command.hpp \
    gui/command/order/delete_order_qt_command.hpp \
    command/pattern/paste_copied_data_to_pattern_command.hpp \
    gui/command/pattern/paste_copied_data_to_pattern_qt_command.hpp \
    command/pattern/erase_cells_in_pattern_command.hpp \
    gui/command/pattern/erase_cells_in_pattern_qt_command.hpp \
    command/order/paste_copied_data_to_order_command.hpp \
    gui/command/order/paste_copied_data_to_order_qt_command.hpp \
    gui/instrument_editor/instrument_form_manager.hpp \
    instrument/lfo_fm.hpp \
    gui/instrument_editor/visualized_instrument_macro_editor.hpp \
    instrument/command_sequence.hpp \
    instrument/sequence_iterator_interface.hpp \
    instrument/effect_iterator.hpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.hpp \
    gui/command/pattern/paste_mix_copied_data_to_pattern_qt_command.hpp \
    gui/module_properties_dialog.hpp \
    module/groove.hpp \
    gui/groove_settings_dialog.hpp \
    gui/configuration_dialog.hpp \
    command/pattern/expand_pattern_command.hpp \
    gui/command/pattern/expand_pattern_qt_command.hpp \
    command/pattern/shrink_pattern_command.hpp \
    gui/command/pattern/shrink_pattern_qt_command.hpp \
    command/abstract_command.hpp \
    instrument/abstract_instrument_property.hpp \
    command/order/duplicate_order_command.hpp \
    command/order/move_order_command.hpp \
    command/order/clone_patterns_command.hpp \
    command/order/clone_order_command.hpp \
    gui/command/order/duplicate_order_qt_command.hpp \
    gui/command/order/move_order_qt_command.hpp \
    gui/command/order/clone_patterns_qt_command.hpp \
    gui/command/order/clone_order_qt_command.hpp \
    gui/command/pattern/set_echo_buffer_access_qt_command.hpp \
    command/pattern/set_echo_buffer_access_command.hpp \
    gui/comment_edit_dialog.hpp \
    io/file_io.hpp \
    io/binary_container.hpp \
    version.hpp \
    gui/command/pattern/interpolate_pattern_qt_command.hpp \
    command/pattern/interpolate_pattern_command.hpp \
    gui/command/pattern/reverse_pattern_qt_command.hpp \
    command/pattern/reverse_pattern_command.hpp \
    gui/command/pattern/replace_instrument_in_pattern_qt_command.hpp \
    command/pattern/replace_instrument_in_pattern_command.hpp \
    chips/export_container.hpp \
    gui/vgm_export_settings_dialog.hpp \
    gui/wave_export_settings_dialog.hpp \
    io/gd3_tag.hpp \
    configuration.hpp \
    gui/configuration_handler.hpp \
    gui/color_palette.hpp \
    gui/command/pattern/paste_overwrite_copied_data_to_pattern_qt_command.hpp \
    command/pattern/paste_overwrite_copied_data_to_pattern_command.hpp \
    io/file_io_error.hpp \
    format/wopn_file.h \
    instrument/bank.hpp \
    gui/instrument_selection_dialog.hpp \
    io/s98_tag.hpp \
    gui/s98_export_settings_dialog.hpp \
    chips/scci/scci.h \
    chips/scci/SCCIDefines.h \
    stream/timer.hpp \
    io/module_io.hpp \
    io/io_handlers.hpp \
    io/export_handler.hpp \
    io/instrument_io.hpp \
    io/bank_io.hpp \
    gui/fm_envelope_set_edit_dialog.hpp \
    gui/file_history_handler.hpp \
    gui/file_history.hpp \
    midi/midi.hpp \
    midi/midi_def.h \
    gui/q_application_wrapper.hpp \
    gui/wave_visual.hpp

FORMS += \
    gui/bookmark_manager_form.ui \
    gui/effect_list_dialog.ui \
    gui/instrument_editor/instrument_editor_adpcm_form.ui \
    gui/keyboard_shortcut_list_dialog.ui \
    gui/mainwindow.ui \
    gui/instrument_editor/instrument_editor_fm_form.ui \
    gui/instrument_editor/fm_operator_table.ui \
    gui/labeled_vertical_slider.ui \
    gui/labeled_horizontal_slider.ui \
    gui/order_list_editor/order_list_editor.ui \
    gui/pattern_editor/pattern_editor.ui \
    gui/instrument_editor/instrument_editor_ssg_form.ui \
    gui/instrument_editor/visualized_instrument_macro_editor.ui \
    gui/module_properties_dialog.ui \
    gui/groove_settings_dialog.ui \
    gui/configuration_dialog.ui \
    gui/comment_edit_dialog.ui \
    gui/vgm_export_settings_dialog.ui \
    gui/wave_export_settings_dialog.ui \
    gui/instrument_selection_dialog.ui \
    gui/s98_export_settings_dialog.ui \
    gui/fm_envelope_set_edit_dialog.ui

INCLUDEPATH += \
    $$PWD/chips \
    $$PWD/stream \
    $$PWD/instrument \
    $$PWD/command \
    $$PWD/module \
    $$PWD/io

RESOURCES += \
    bamboo_tracker.qrc

TRANSLATIONS += \
    res/lang/bamboo_tracker_fr.ts \
    res/lang/bamboo_tracker_ja.ts

include("stream/RtAudio/RtAudio.pri")
include("midi/RtMidi/RtMidi.pri")

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
    message(using a workaround for missing 'lrelease' option in Qt <5.12...)

    for(tsfile, TRANSLATIONS) {
      qmfile   = $$tsfile
      qmfile  ~= s/.ts$/.qm/
      qmfile  ~= s,^res/lang,.qm,
               thisqmcom  = $${qmfile}.commands
      win32:$$thisqmcom  = mkdir .qm;
       else:$$thisqmcom  = test -d .qm || mkdir -p .qm;
            $$thisqmcom += lrelease -qm $$qmfile $$PWD/$$tsfile
        thisqmdep  = $${qmfile}.depends
      $$thisqmdep  = $$PWD/$${tsfile}

      PRE_TARGETDEPS      += $${qmfile}
      QMAKE_EXTRA_TARGETS += $${qmfile}


        thisinst    = translations_$${qmfile}
        thisinstdep = $${thisinst}.depends
      $$thisinstdep = $$qmfile

        thisinstcfg = $${thisinst}.CONFIG
      $$thisinstcfg = no_check_exist

        thisinstfil = $${thisinst}.files
      $$thisinstfil = $$PWD/$$qmfile

        thisinstpat = $${thisinst}.path
      $$thisinstpat = $$QM_FILES_INSTALL_PATH

      INSTALLS += $$thisinst
    }
}
else {
    CONFIG += lrelease
}

win32 {
    RC_ICONS = res/icon/BambooTracker.ico
}
