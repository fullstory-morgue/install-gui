#include <gtk/gtk.h>


gboolean
on_window1_configure_event             (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_rootpartcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_checkbutton_mountpoints_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_next_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_prev_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gparted_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gparted_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gparted_released             (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gparted_pressed              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_install_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_checkbutton_format_with_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_checkbutton_format_with_pressed     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_install_show                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_button_install_activate             (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_button_install_configure_event      (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_button_install_configure_event      (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_button_install_focus                (GtkWidget       *widget,
                                        GtkDirectionType  direction,
                                        gpointer         user_data);

void
on_button_install_show                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_button_install_set_focus_child      (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

void
on_notebook1_change_current_page       (GtkNotebook     *notebook,
                                        gint             offset,
                                        gpointer         user_data);

gboolean
on_notebook1_select_page               (GtkNotebook     *notebook,
                                        gboolean         move_focus,
                                        gpointer         user_data);

void
on_notebook1_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
on_radiobutton1_group_changed          (GtkRadioButton  *radiobutton,
                                        gpointer         user_data);

void
on_radiobutton1_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton2_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_radiobutton3_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_tz_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_window_main_configure_event         (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_window_main_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_window_main_show                    (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_install_progressbar_show            (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_install_progressbar_delete_event    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_install_progressbar_configure_event (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_rootpartcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_rootpartcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_label1_show                         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_combobox_lang_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_okbutton2_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_success_exit_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_dialog_end_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_button_usb_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_install_now_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_entry_rootpw_changed                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_button_install_now_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_edit_configuration_clicked   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_notebook1_select_page               (GtkNotebook     *notebook,
                                        gboolean         move_focus,
                                        gpointer         user_data);

void
on_window_main_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fixed_network_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fixed_network_show                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_label5_populate_popup               (GtkLabel        *label,
                                        GtkMenu         *menu,
                                        gpointer         user_data);

void
on_label5_move_cursor                  (GtkLabel        *label,
                                        GtkMovementStep  step,
                                        gint             count,
                                        gboolean         extend_selection,
                                        gpointer         user_data);

void
on_notebook1_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
on_button_install_now_back_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_install_progressbar_realize         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_entry_rootpw_changed                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_rootpw_again_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_realname_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_username_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_pw_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_pw_again_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_hostname_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);
