#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T16:20:11
#
#-------------------------------------------------

QT       += core gui

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
win32|install_flat {
    target.path = $$PREFIX
}
else {
    target.path = $$PREFIX/bin
}

CONFIG += c++14

# C/C++ compiler flags
msvc {
  message("Qt configured with MSVC")
  message("Compiler is version" $$QT_MSC_FULL_VER)
  CPP_WARNING_FLAGS += /Wall /Wp64 /WX
  CPP_WARNING_FLAGS += /source-charset:utf-8
}
else:clang|if(gcc:!intel_icc) {
  # Pedantic settings, warning -> error escalation and C standard specification
  CPP_WARNING_FLAGS += -Wall -Wextra -Werror -pedantic -pedantic-errors
  QMAKE_CFLAGS += -std=gnu11

  # Get the compiler version for version-specific handling
  clang {
    defined(QMAKE_APPLE_CLANG_MAJOR_VERSION, var) {
      message("Qt configured with Apple LLVM")
      CONFIG += clang-apple
      COMPILER_MAJOR_VERSION = $$QT_APPLE_CLANG_MAJOR_VERSION
      COMPILER_MINOR_VERSION = $$QT_APPLE_CLANG_MINOR_VERSION
      COMPILER_PATCH_VERSION = $$QT_APPLE_CLANG_PATCH_VERSION
    }
    else {
      message("Qt configured with LLVM")
      CONFIG += clang-normal
      COMPILER_MAJOR_VERSION = $$QT_CLANG_MAJOR_VERSION
      COMPILER_MINOR_VERSION = $$QT_CLANG_MINOR_VERSION
      COMPILER_PATCH_VERSION = $$QT_CLANG_PATCH_VERSION
    }
  }
  else {
    message("Qt configured with GCC")
    COMPILER_MAJOR_VERSION = $$QT_GCC_MAJOR_VERSION
    COMPILER_MINOR_VERSION = $$QT_GCC_MINOR_VERSION
    COMPILER_PATCH_VERSION = $$QT_GCC_PATCH_VERSION
  }
  COMPILER_VERSION = $${COMPILER_MAJOR_VERSION}.$${COMPILER_MINOR_VERSION}.$${COMPILER_PATCH_VERSION}
  message("Compiler is version" $$COMPILER_VERSION)

  # Temporary known-error downgrades

  # RtAudio code contains VLAs for multiple APIs, needs a stronger patch in the future
  CPP_WARNING_FLAGS += -Wno-error=vla

  clang {
    # See VLA workaround, needs additional switch on Clang
    CPP_WARNING_FLAGS += -Wno-vla-extension

    # macOS 10.14 (LLVM 11.0.0) targeting gnu++1y (C++14) errors when
    # using system-installed JACK headers in RtAudio & RtMidi
    # /usr/local/Cellar/jack/0.125.0_4/include/jack/types.h:(389,411)
    use_jack {
      CPP_WARNING_FLAGS += -Wno-deprecated-register
    }

    # FreeBSD ALSA headers use zero-length array
    # /usr/local/include/alsa/pcm.h:597
    freebsd:use_alsa {
      CPP_WARNING_FLAGS += -Wno-zero-length-array
    }

    # Definition of implicit copy constructor with user-declared copy assignment operator deprecated
    # Problem in Qt5 itself, nothing we can fix about it
    # Introduced by LLVM 10, triggered by any reference to QString
    # TODO: Extend to clang-apple once LLVM 10 reaches the platform and they decide on a new custom version
    # /usr/local/include/qt5/QtCore/(qbytearray.h:586,qstring.h:1191)
    clang-normal:greaterThan(COMPILER_MAJOR_VERSION, 9) {
      CPP_WARNING_FLAGS += -Wno-deprecated-copy
    }
  }
  else {
    # Definition of implicit copy constructor with user-declared copy assignment operator deprecated
    # Problem in Qt5 itself, nothing we can fix about it
    # Introduced by GCC 9, triggered by any reference to QString
    # /usr/local/include/qt5/QtCore/(qbytearray.h:586,qstring.h:1191)
    greaterThan(COMPILER_MAJOR_VERSION, 8) {
      CPP_WARNING_FLAGS += -Wno-error=deprecated-copy
    }
  }
}
else {
  message("Unknown compiler, won't attempt to add warning & pedantic compiler switches.")
}
QMAKE_CFLAGS_WARN_ON += $$CPP_WARNING_FLAGS
QMAKE_CXXFLAGS_WARN_ON += $$CPP_WARNING_FLAGS

SOURCES += \
    chips/c86ctl/c86ctl_wrapper.cpp \
    command/instrument/swap_instruments_command.cpp \
    command/pattern/change_values_in_pattern_command.cpp \
    command/pattern/paste_insert_copied_data_to_pattern_command.cpp \
    command/pattern/pattern_command_utils.cpp \
    command/pattern/transpose_note_in_pattern_command.cpp \
    gui/bookmark_manager_form.cpp \
    gui/color_palette_handler.cpp \
    gui/command/instrument/swap_instruments_qt_command.cpp \
    gui/command/pattern/change_values_in_pattern_qt_command.cpp \
    gui/command/pattern/paste_insert_copied_data_to_pattern_qt_command.cpp \
    gui/command/pattern/transpose_note_in_pattern_qt_command.cpp \
    gui/drop_detect_list_widget.cpp \
    gui/effect_description.cpp \
    gui/effect_list_dialog.cpp \
    gui/file_io_error_message_box.cpp \
    gui/go_to_dialog.cpp \
    gui/hide_tracks_dialog.cpp \
    gui/instrument_editor/adpcm_sample_editor.cpp \
    gui/instrument_editor/arpeggio_macro_editor.cpp \
    gui/instrument_editor/grid_settings_dialog.cpp \
    gui/instrument_editor/instrument_editor_adpcm_form.cpp \
    gui/instrument_editor/instrument_editor_drumkit_form.cpp \
    gui/instrument_editor/sample_length_dialog.cpp \
    gui/instrument_editor/tone_noise_macro_editor.cpp \
    gui/keyboard_shortcut_list_dialog.cpp \
    gui/swap_tracks_dialog.cpp \
    gui/track_visibility_memory_handler.cpp \
    gui/transpose_song_dialog.cpp \
    instrument/waveform_adpcm.cpp \
    io/btb_io.cpp \
    io/bti_io.cpp \
    io/btm_io.cpp \
    io/dat_io.cpp \
    io/dmp_io.cpp \
    io/ff_io.cpp \
    io/ins_io.cpp \
    io/opni_io.cpp \
    io/ppc_io.cpp \
    io/pvi_io.cpp \
    io/tfi_io.cpp \
    io/vgi_io.cpp \
    io/wav_container.cpp \
    io/wopn_io.cpp \
    io/y12_io.cpp \
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
    song_length_calculator.cpp \
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
    chips/chip_misc.hpp \
    chips/codec/ymb_codec.hpp \
    chips/c86ctl/c86ctl.h \
    chips/c86ctl/c86ctl_wrapper.hpp \
    chips/scci/SCCIDefines.hpp \
    chips/scci/scci.hpp \
    command/command_id.hpp \
    command/instrument/swap_instruments_command.hpp \
    command/pattern/change_values_in_pattern_command.hpp \
    command/pattern/paste_insert_copied_data_to_pattern_command.hpp \
    command/pattern/pattern_command_utils.hpp \
    command/pattern/transpose_note_in_pattern_command.hpp \
    enum_hash.hpp \
    gui/bookmark_manager_form.hpp \
    gui/color_palette_handler.hpp \
    gui/command/instrument/swap_instruments_qt_command.hpp \
    gui/command/pattern/change_values_in_pattern_qt_command.hpp \
    gui/command/pattern/paste_insert_copied_data_to_pattern_qt_command.hpp \
    gui/command/pattern/transpose_note_in_pattern_qt_command.hpp \
    gui/drop_detect_list_widget.hpp \
    gui/effect_description.hpp \
    gui/effect_list_dialog.hpp \
    gui/file_io_error_message_box.hpp \
    gui/go_to_dialog.hpp \
    gui/gui_util.hpp \
    gui/hide_tracks_dialog.hpp \
    gui/instrument_editor/adpcm_sample_editor.hpp \
    gui/instrument_editor/arpeggio_macro_editor.hpp \
    gui/instrument_editor/grid_settings_dialog.hpp \
    gui/instrument_editor/instrument_editor_adpcm_form.hpp \
    gui/instrument_editor/instrument_editor_drumkit_form.hpp \
    gui/instrument_editor/instrument_editor_util.hpp \
    gui/instrument_editor/sample_length_dialog.hpp \
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
    chips/opna.hpp \
    chips/resampler.hpp \
    bamboo_tracker.hpp \
    gui/swap_tracks_dialog.hpp \
    gui/track_visibility_memory_handler.hpp \
    gui/transpose_song_dialog.hpp \
    instrument/waveform_adpcm.hpp \
    io/bt_io_defs.hpp \
    io/btb_io.hpp \
    io/bti_io.hpp \
    io/btm_io.hpp \
    io/dat_io.hpp \
    io/dmp_io.hpp \
    io/ff_io.hpp \
    io/ins_io.hpp \
    io/opni_io.hpp \
    io/ppc_io.hpp \
    io/pvi_io.hpp \
    io/tfi_io.hpp \
    io/vgi_io.hpp \
    io/wav_container.hpp \
    io/wopn_io.hpp \
    io/y12_io.hpp \
    module/effect.hpp \
    playback.hpp \
    song_length_calculator.hpp \
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
    gui/go_to_dialog.ui \
    gui/hide_tracks_dialog.ui \
    gui/instrument_editor/adpcm_sample_editor.ui \
    gui/instrument_editor/grid_settings_dialog.ui \
    gui/instrument_editor/instrument_editor_adpcm_form.ui \
    gui/instrument_editor/instrument_editor_drumkit_form.ui \
    gui/instrument_editor/sample_length_dialog.ui \
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
    gui/swap_tracks_dialog.ui \
    gui/transpose_song_dialog.ui \
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

# Linking settings
system_*:CONFIG += link_pkgconfig

system_rtaudio:PKGCONFIG += rtaudio
else:include("stream/RtAudio/RtAudio.pri")

system_rtmidi:PKGCONFIG += rtmidi
else:include("midi/RtMidi/RtMidi.pri")

include("../data/data.pri")
