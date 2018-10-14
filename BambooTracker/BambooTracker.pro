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

CONFIG += c++14

SOURCES += \
    main.cpp \
    gui/mainwindow.cpp \
    chips/chip.cpp \
    chips/opna.cpp \
    chips/resampler.cpp \
    chips/mame/2608intf.c \
    chips/mame/emu2149.c \
    chips/mame/fm.c \
    chips/mame/ymdeltat.c \
    bamboo_tracker.cpp \
    stream/audio_stream.cpp \
    stream/audio_stream_mixier.cpp \
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
    gui/line_read_only_spin_box.cpp \
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
    instrument/abstruct_instrument_property.cpp \
    gui/instrument_editor/visualized_instrument_macro_editor.cpp \
    instrument/command_sequence.cpp \
    instrument/effect_iterator.cpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.cpp \
    gui/command/pattern/paste_mix_copied_data_to_pattern_qt_command.cpp \
    gui/command/pattern/decrease_note_key_in_pattern_qt_command.cpp \
    gui/command/pattern/increase_note_key_in_pattern_qt_command.cpp \
    gui/command/pattern/increase_note_octave_in_pattern_qt_command.cpp \
    gui/command/pattern/decrease_note_octave_in_pattern_qt_command.cpp \
    command/pattern/increase_note_key_in_pattern_command.cpp \
    command/pattern/decrease_note_key_in_pattern_command.cpp \
    command/pattern/increase_note_octave_in_pattern_command.cpp \
    command/pattern/decrease_note_octave_in_pattern_command.cpp \
    gui/module_properties_dialog.cpp \
    module/groove.cpp \
    gui/groove_settings_dialog.cpp \
    gui/configuration_dialog.cpp

HEADERS += \
    gui/mainwindow.hpp \
    chips/mame/2608intf.h \
    chips/mame/emu2149.h \
    chips/mame/emutypes.h \
    chips/mame/fm.h \
    chips/mame/mamedef.h \
    chips/mame/ymdeltat.h \
    chips/chip.hpp \
    chips/chip_misc.h \
    chips/opna.hpp \
    chips/resampler.hpp \
    bamboo_tracker.hpp \
    stream/audio_stream.hpp \
    stream/audio_stream_mixier.hpp \
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
    command/abstruct_command.hpp \
    opna_controller.hpp \
    instrument/instrument.hpp \
    instrument/envelope_fm.hpp \
    gui/event_guard.hpp \
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
    gui/line_read_only_spin_box.hpp \
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
    instrument/abstruct_instrument_property.hpp \
    gui/instrument_editor/visualized_instrument_macro_editor.hpp \
    instrument/command_sequence.hpp \
    instrument/sequence_iterator_interface.hpp \
    instrument/effect_iterator.hpp \
    command/pattern/paste_mix_copied_data_to_pattern_command.hpp \
    gui/command/pattern/paste_mix_copied_data_to_pattern_qt_command.hpp \
    gui/command/pattern/decrease_note_key_in_pattern_qt_command.hpp \
    gui/command/pattern/increase_note_key_in_pattern_qt_command.hpp \
    gui/command/pattern/increase_note_octave_in_pattern_qt_command.hpp \
    gui/command/pattern/decrease_note_octave_in_pattern_qt_command.hpp \
    command/pattern/increase_note_key_in_pattern_command.hpp \
    command/pattern/decrease_note_key_in_pattern_command.hpp \
    command/pattern/increase_note_octave_in_pattern_command.hpp \
    command/pattern/decrease_note_octave_in_pattern_command.hpp \
    gui/module_properties_dialog.hpp \
    module/groove.hpp \
    gui/groove_settings_dialog.hpp \
    gui/configuration_dialog.hpp

FORMS += \
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
    gui/configuration_dialog.ui

INCLUDEPATH += \
    $$PWD/chips \
    $$PWD/stream \
    $$PWD/instrument \
    $$PWD/command \
    $$PWD/module
