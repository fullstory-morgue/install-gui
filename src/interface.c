/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_window1 (void)
{
  GtkWidget *window1;
  GdkPixbuf *window1_icon_pixbuf;
  GtkWidget *fixed1;
  GtkWidget *exit;
  GtkWidget *alignment2;
  GtkWidget *hbox2;
  GtkWidget *image5;
  GtkWidget *label10;
  GtkWidget *prev;
  GtkWidget *alignment3;
  GtkWidget *hbox3;
  GtkWidget *image6;
  GtkWidget *label11;
  GtkWidget *next;
  GtkWidget *alignment4;
  GtkWidget *hbox4;
  GtkWidget *image7;
  GtkWidget *label12;
  GtkWidget *label_wellcome;
  GtkWidget *image1;
  GtkWidget *notebook1;
  GtkWidget *fixed2;
  GtkWidget *hseparator2;
  GtkWidget *hseparator7;
  GtkWidget *vseparator1;
  GtkWidget *hseparator8;
  GtkWidget *vseparator2;
  GtkWidget *label_wellcome_install;
  GtkWidget *label30;
  GtkWidget *hseparator3;
  GtkWidget *image2;
  GtkWidget *label_wellcome_red;
  GtkWidget *label_wellcome_2;
  GtkWidget *label1;
  GtkWidget *fixed3;
  GtkWidget *hseparator1;
  GtkWidget *label7;
  GtkWidget *label8;
  GtkWidget *checkbutton_mountpoints;
  GtkWidget *button_gparted;
  GtkWidget *alignment1;
  GtkWidget *hbox1;
  GtkWidget *image4;
  GtkWidget *label9;
  GtkWidget *format_combo;
  GtkWidget *checkbutton_format_with;
  GtkWidget *rootpartcombo;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview1;
  GtkWidget *label35;
  GtkWidget *label2;
  GtkWidget *fixed4;
  GtkWidget *hseparator4;
  GtkWidget *checkbutton_bootdisk;
  GtkWidget *label15;
  GtkWidget *hseparator9;
  GtkWidget *vseparator3;
  GtkWidget *vseparator4;
  GtkWidget *hseparator10;
  GtkWidget *label14;
  GtkWidget *combobox_bootmanager;
  GtkWidget *combobox_installplace;
  GtkWidget *label13;
  GtkWidget *label3;
  GtkWidget *fixed5;
  GtkWidget *frame1;
  GtkWidget *alignment5;
  GtkWidget *fixed8;
  GtkWidget *entry_rootpw;
  GtkWidget *entry_rootpw_again;
  GtkWidget *entry_realname;
  GtkWidget *entry_username;
  GtkWidget *entry_pw;
  GtkWidget *entry_pw_again;
  GtkWidget *hseparator6;
  GtkWidget *label18;
  GtkWidget *label21;
  GtkWidget *label26;
  GtkWidget *label27;
  GtkWidget *label28;
  GtkWidget *label29;
  GtkWidget *label25;
  GtkWidget *hseparator5;
  GtkWidget *label16;
  GtkWidget *label4;
  GtkWidget *fixed6;
  GtkWidget *hostname;
  GtkWidget *hseparator11;
  GtkWidget *vseparator6;
  GtkWidget *hseparator12;
  GtkWidget *label32;
  GtkWidget *vseparator5;
  GtkWidget *hseparator13;
  GtkWidget *label31;
  GtkWidget *label5;
  GtkWidget *fixed7;
  GtkWidget *hseparator14;
  GtkWidget *vseparator8;
  GtkWidget *vseparator7;
  GtkWidget *hseparator15;
  GtkWidget *hseparator16;
  GtkWidget *button_install;
  GtkWidget *alignment6;
  GtkWidget *hbox5;
  GtkWidget *image8;
  GtkWidget *label34;
  GtkWidget *radiobutton1;
  GSList *radiobutton1_group = NULL;
  GtkWidget *checkbutton_force;
  GtkWidget *radiobutton3;
  GtkWidget *radiobutton2;
  GtkWidget *label33;
  GtkWidget *label6;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window1), _("Sidux HD-Installation"));
  window1_icon_pixbuf = create_pixbuf ("sidux-inst-conf.png");
  if (window1_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (window1), window1_icon_pixbuf);
      gdk_pixbuf_unref (window1_icon_pixbuf);
    }

  fixed1 = gtk_fixed_new ();
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window1), fixed1);
  gtk_container_set_border_width (GTK_CONTAINER (fixed1), 15);

  exit = gtk_button_new ();
  gtk_widget_show (exit);
  gtk_fixed_put (GTK_FIXED (fixed1), exit, 32, 480);
  gtk_widget_set_size_request (exit, 110, 28);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (exit), alignment2);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox2);

  image5 = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image5);
  gtk_box_pack_start (GTK_BOX (hbox2), image5, FALSE, FALSE, 0);

  label10 = gtk_label_new_with_mnemonic (_("exit"));
  gtk_widget_show (label10);
  gtk_box_pack_start (GTK_BOX (hbox2), label10, FALSE, FALSE, 0);

  prev = gtk_button_new ();
  gtk_widget_show (prev);
  gtk_fixed_put (GTK_FIXED (fixed1), prev, 424, 480);
  gtk_widget_set_size_request (prev, 110, 28);

  alignment3 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (prev), alignment3);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment3), hbox3);

  image6 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image6);
  gtk_box_pack_start (GTK_BOX (hbox3), image6, FALSE, FALSE, 0);

  label11 = gtk_label_new_with_mnemonic (_("Prev"));
  gtk_widget_show (label11);
  gtk_box_pack_start (GTK_BOX (hbox3), label11, FALSE, FALSE, 0);

  next = gtk_button_new ();
  gtk_widget_show (next);
  gtk_fixed_put (GTK_FIXED (fixed1), next, 552, 480);
  gtk_widget_set_size_request (next, 110, 28);

  alignment4 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment4);
  gtk_container_add (GTK_CONTAINER (next), alignment4);

  hbox4 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox4);
  gtk_container_add (GTK_CONTAINER (alignment4), hbox4);

  image7 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image7);
  gtk_box_pack_start (GTK_BOX (hbox4), image7, FALSE, FALSE, 0);

  label12 = gtk_label_new_with_mnemonic (_("Next"));
  gtk_widget_show (label12);
  gtk_box_pack_start (GTK_BOX (hbox4), label12, FALSE, FALSE, 0);

  label_wellcome = gtk_label_new (_("Welcome to the Sidux HD-Installation"));
  gtk_widget_show (label_wellcome);
  gtk_fixed_put (GTK_FIXED (fixed1), label_wellcome, 32, 8);
  gtk_widget_set_size_request (label_wellcome, 503, 39);
  gtk_misc_set_alignment (GTK_MISC (label_wellcome), 0, 0.3);

  image1 = create_pixmap (window1, "sidux-inst.png");
  gtk_widget_show (image1);
  gtk_fixed_put (GTK_FIXED (fixed1), image1, 496, 2);
  gtk_widget_set_size_request (image1, 127, 39);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_fixed_put (GTK_FIXED (fixed1), notebook1, 9, 56);
  gtk_widget_set_size_request (notebook1, 678, 407);

  fixed2 = gtk_fixed_new ();
  gtk_widget_show (fixed2);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed2);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_show (hseparator2);
  gtk_fixed_put (GTK_FIXED (fixed2), hseparator2, 24, 80);
  gtk_widget_set_size_request (hseparator2, 624, 16);

  hseparator7 = gtk_hseparator_new ();
  gtk_widget_show (hseparator7);
  gtk_fixed_put (GTK_FIXED (fixed2), hseparator7, 24, 16);
  gtk_widget_set_size_request (hseparator7, 624, 16);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_show (vseparator1);
  gtk_fixed_put (GTK_FIXED (fixed2), vseparator1, 16, 24);
  gtk_widget_set_size_request (vseparator1, 16, 328);

  hseparator8 = gtk_hseparator_new ();
  gtk_widget_show (hseparator8);
  gtk_fixed_put (GTK_FIXED (fixed2), hseparator8, 24, 344);
  gtk_widget_set_size_request (hseparator8, 624, 16);

  vseparator2 = gtk_vseparator_new ();
  gtk_widget_show (vseparator2);
  gtk_fixed_put (GTK_FIXED (fixed2), vseparator2, 640, 24);
  gtk_widget_set_size_request (vseparator2, 16, 328);

  label_wellcome_install = gtk_label_new (_("This application will install Sidux to your harddisk."));
  gtk_widget_show (label_wellcome_install);
  gtk_fixed_put (GTK_FIXED (fixed2), label_wellcome_install, 32, 32);
  gtk_widget_set_size_request (label_wellcome_install, 608, 48);
  gtk_label_set_justify (GTK_LABEL (label_wellcome_install), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label_wellcome_install), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_wellcome_install), 0, 0.4);

  label30 = gtk_label_new (_("\n"));
  gtk_widget_show (label30);
  gtk_fixed_put (GTK_FIXED (fixed2), label30, 32, 256);
  gtk_widget_set_size_request (label30, 608, 72);
  gtk_misc_set_alignment (GTK_MISC (label30), 0.42, 0.5);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_show (hseparator3);
  gtk_fixed_put (GTK_FIXED (fixed2), hseparator3, 24, 176);
  gtk_widget_set_size_request (hseparator3, 624, 16);

  image2 = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_fixed_put (GTK_FIXED (fixed2), image2, 32, 112);
  gtk_widget_set_size_request (image2, 32, 48);

  label_wellcome_red = gtk_label_new (_("Note that this version is still an very early version and under heavy development. \nThe author takes no responsibility for data loss or hardware damage."));
  gtk_widget_show (label_wellcome_red);
  gtk_fixed_put (GTK_FIXED (fixed2), label_wellcome_red, 72, 96);
  gtk_widget_set_size_request (label_wellcome_red, 568, 80);
  gtk_label_set_justify (GTK_LABEL (label_wellcome_red), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label_wellcome_red), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_wellcome_red), 0, 0.35);

  label_wellcome_2 = gtk_label_new (_("If you need any help with the installation visit the Sidux-Website, -Wiki, -Forum or -Chat!\n\nSidux-Website: http://sidux.com"));
  gtk_widget_show (label_wellcome_2);
  gtk_fixed_put (GTK_FIXED (fixed2), label_wellcome_2, 32, 224);
  gtk_widget_set_size_request (label_wellcome_2, 608, 96);
  gtk_label_set_justify (GTK_LABEL (label_wellcome_2), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label_wellcome_2), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_wellcome_2), 0, 0);

  label1 = gtk_label_new (_("Wellcome"));
  gtk_widget_show (label1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);

  fixed3 = gtk_fixed_new ();
  gtk_widget_show (fixed3);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed3);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_fixed_put (GTK_FIXED (fixed3), hseparator1, 8, 88);
  gtk_widget_set_size_request (hseparator1, 640, 16);

  label7 = gtk_label_new (_("Root-Partition"));
  gtk_widget_show (label7);
  gtk_fixed_put (GTK_FIXED (fixed3), label7, 144, 24);
  gtk_widget_set_size_request (label7, 104, 24);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  label8 = gtk_label_new (_("Mountpoints of other Partitions"));
  gtk_widget_show (label8);
  gtk_fixed_put (GTK_FIXED (fixed3), label8, 8, 168);
  gtk_widget_set_size_request (label8, 128, 40);
  gtk_label_set_line_wrap (GTK_LABEL (label8), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0);

  checkbutton_mountpoints = gtk_check_button_new_with_mnemonic (_("Set mountpoints of other Partitions\n(will not be formatted automatically)"));
  gtk_widget_show (checkbutton_mountpoints);
  gtk_fixed_put (GTK_FIXED (fixed3), checkbutton_mountpoints, 144, 112);
  gtk_widget_set_size_request (checkbutton_mountpoints, 496, 48);

  button_gparted = gtk_button_new ();
  gtk_widget_show (button_gparted);
  gtk_fixed_put (GTK_FIXED (fixed3), button_gparted, 8, 31);
  gtk_widget_set_size_request (button_gparted, 120, 41);
  gtk_tooltips_set_tip (tooltips, button_gparted, _("!!!!!! be carefull !!!!!!!\nDo it, if you don't have a linux partition.\nRead the Manual before. (see the Manual Button on the Desktop)"), NULL);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (button_gparted), alignment1);

  hbox1 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox1);

  image4 = gtk_image_new_from_stock ("gtk-harddisk", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image4);
  gtk_box_pack_start (GTK_BOX (hbox1), image4, FALSE, FALSE, 0);

  label9 = gtk_label_new_with_mnemonic (_("Start Part.-\nmanager"));
  gtk_widget_show (label9);
  gtk_box_pack_start (GTK_BOX (hbox1), label9, FALSE, FALSE, 0);

  format_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (format_combo);
  gtk_fixed_put (GTK_FIXED (fixed3), format_combo, 264, 56);
  gtk_widget_set_size_request (format_combo, 176, 24);

  checkbutton_format_with = gtk_check_button_new_with_mnemonic (_("format with"));
  gtk_widget_show (checkbutton_format_with);
  gtk_fixed_put (GTK_FIXED (fixed3), checkbutton_format_with, 144, 56);
  gtk_widget_set_size_request (checkbutton_format_with, 120, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton_format_with), TRUE);

  rootpartcombo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (rootpartcombo);
  gtk_fixed_put (GTK_FIXED (fixed3), rootpartcombo, 264, 24);
  gtk_widget_set_size_request (rootpartcombo, 384, 24);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_fixed_put (GTK_FIXED (fixed3), scrolledwindow1, 144, 168);
  gtk_widget_set_size_request (scrolledwindow1, 504, 184);

  treeview1 = gtk_tree_view_new ();
  gtk_widget_show (treeview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);
  gtk_widget_set_size_request (treeview1, 248, 136);
  gtk_tooltips_set_tip (tooltips, treeview1, _("possible are:\n========\n/bin\n/boot\n/etc\n/home\n/lib\n/opt\n/root\n/sbin\n/tmp\n/usr\n/var"), NULL);
  gtk_tree_view_set_hover_expand (GTK_TREE_VIEW (treeview1), TRUE);

  label35 = gtk_label_new (_("Example:\n------------\nA seperate\nhome-partition:\n/home"));
  gtk_widget_show (label35);
  gtk_fixed_put (GTK_FIXED (fixed3), label35, 8, 216);
  gtk_widget_set_size_request (label35, 128, 128);
  gtk_label_set_line_wrap (GTK_LABEL (label35), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label35), 0, 0);

  label2 = gtk_label_new (_("Partitioning"));
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);

  fixed4 = gtk_fixed_new ();
  gtk_widget_show (fixed4);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed4);

  hseparator4 = gtk_hseparator_new ();
  gtk_widget_show (hseparator4);
  gtk_fixed_put (GTK_FIXED (fixed4), hseparator4, 16, 112);
  gtk_widget_set_size_request (hseparator4, 640, 16);

  checkbutton_bootdisk = gtk_check_button_new_with_mnemonic (_("create a bootdisk"));
  gtk_widget_show (checkbutton_bootdisk);
  gtk_fixed_put (GTK_FIXED (fixed4), checkbutton_bootdisk, 280, 256);
  gtk_widget_set_size_request (checkbutton_bootdisk, 184, 32);

  label15 = gtk_label_new (_("Installation-place"));
  gtk_widget_show (label15);
  gtk_fixed_put (GTK_FIXED (fixed4), label15, 144, 224);
  gtk_widget_set_size_request (label15, 128, 24);
  gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);

  hseparator9 = gtk_hseparator_new ();
  gtk_widget_show (hseparator9);
  gtk_fixed_put (GTK_FIXED (fixed4), hseparator9, 104, 160);
  gtk_widget_set_size_request (hseparator9, 408, 16);

  vseparator3 = gtk_vseparator_new ();
  gtk_widget_show (vseparator3);
  gtk_fixed_put (GTK_FIXED (fixed4), vseparator3, 96, 168);
  gtk_widget_set_size_request (vseparator3, 16, 128);

  vseparator4 = gtk_vseparator_new ();
  gtk_widget_show (vseparator4);
  gtk_fixed_put (GTK_FIXED (fixed4), vseparator4, 504, 168);
  gtk_widget_set_size_request (vseparator4, 16, 128);

  hseparator10 = gtk_hseparator_new ();
  gtk_widget_show (hseparator10);
  gtk_fixed_put (GTK_FIXED (fixed4), hseparator10, 104, 288);
  gtk_widget_set_size_request (hseparator10, 408, 16);

  label14 = gtk_label_new (_("Bootmanager"));
  gtk_widget_show (label14);
  gtk_fixed_put (GTK_FIXED (fixed4), label14, 144, 184);
  gtk_widget_set_size_request (label14, 128, 24);
  gtk_misc_set_alignment (GTK_MISC (label14), 0.04, 0.5);

  combobox_bootmanager = gtk_combo_box_entry_new_text ();
  gtk_widget_show (combobox_bootmanager);
  gtk_fixed_put (GTK_FIXED (fixed4), combobox_bootmanager, 280, 184);
  gtk_widget_set_size_request (combobox_bootmanager, 183, 25);

  combobox_installplace = gtk_combo_box_entry_new_text ();
  gtk_widget_show (combobox_installplace);
  gtk_fixed_put (GTK_FIXED (fixed4), combobox_installplace, 280, 224);
  gtk_widget_set_size_request (combobox_installplace, 183, 25);

  label13 = gtk_label_new (_("Bootmanager: A bootmanager allows you to choose the OS you want to boot at startup.\n\nInstallation-place: MBR (Master Boot Record) or root-partition"));
  gtk_widget_show (label13);
  gtk_fixed_put (GTK_FIXED (fixed4), label13, 8, 8);
  gtk_widget_set_size_request (label13, 656, 96);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label13), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);

  label3 = gtk_label_new (_("Grub"));
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);

  fixed5 = gtk_fixed_new ();
  gtk_widget_show (fixed5);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed5);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_fixed_put (GTK_FIXED (fixed5), frame1, 120, 120);
  gtk_widget_set_size_request (frame1, 424, 240);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);

  alignment5 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment5);
  gtk_container_add (GTK_CONTAINER (frame1), alignment5);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment5), 0, 0, 12, 0);

  fixed8 = gtk_fixed_new ();
  gtk_widget_show (fixed8);
  gtk_container_add (GTK_CONTAINER (alignment5), fixed8);

  entry_rootpw = gtk_entry_new ();
  gtk_widget_show (entry_rootpw);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_rootpw, 168, 0);
  gtk_widget_set_size_request (entry_rootpw, 216, 24);
  gtk_entry_set_max_length (GTK_ENTRY (entry_rootpw), 20);
  gtk_entry_set_visibility (GTK_ENTRY (entry_rootpw), FALSE);

  entry_rootpw_again = gtk_entry_new ();
  gtk_widget_show (entry_rootpw_again);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_rootpw_again, 168, 32);
  gtk_widget_set_size_request (entry_rootpw_again, 216, 24);
  gtk_entry_set_max_length (GTK_ENTRY (entry_rootpw_again), 20);
  gtk_entry_set_visibility (GTK_ENTRY (entry_rootpw_again), FALSE);

  entry_realname = gtk_entry_new ();
  gtk_widget_show (entry_realname);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_realname, 168, 88);
  gtk_widget_set_size_request (entry_realname, 216, 24);

  entry_username = gtk_entry_new ();
  gtk_widget_show (entry_username);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_username, 168, 120);
  gtk_widget_set_size_request (entry_username, 216, 24);

  entry_pw = gtk_entry_new ();
  gtk_widget_show (entry_pw);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_pw, 168, 160);
  gtk_widget_set_size_request (entry_pw, 216, 24);
  gtk_entry_set_max_length (GTK_ENTRY (entry_pw), 20);
  gtk_entry_set_visibility (GTK_ENTRY (entry_pw), FALSE);

  entry_pw_again = gtk_entry_new ();
  gtk_widget_show (entry_pw_again);
  gtk_fixed_put (GTK_FIXED (fixed8), entry_pw_again, 168, 192);
  gtk_widget_set_size_request (entry_pw_again, 216, 24);
  gtk_entry_set_max_length (GTK_ENTRY (entry_pw_again), 20);
  gtk_entry_set_visibility (GTK_ENTRY (entry_pw_again), FALSE);

  hseparator6 = gtk_hseparator_new ();
  gtk_widget_show (hseparator6);
  gtk_fixed_put (GTK_FIXED (fixed8), hseparator6, 0, 64);
  gtk_widget_set_size_request (hseparator6, 384, 16);

  label18 = gtk_label_new (_("Root-password:"));
  gtk_widget_show (label18);
  gtk_fixed_put (GTK_FIXED (fixed8), label18, 0, 0);
  gtk_widget_set_size_request (label18, 160, 24);
  gtk_misc_set_alignment (GTK_MISC (label18), 0, 0.5);

  label21 = gtk_label_new (_("Root-password again:"));
  gtk_widget_show (label21);
  gtk_fixed_put (GTK_FIXED (fixed8), label21, 0, 32);
  gtk_widget_set_size_request (label21, 168, 24);
  gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

  label26 = gtk_label_new (_("Realname:"));
  gtk_widget_show (label26);
  gtk_fixed_put (GTK_FIXED (fixed8), label26, 0, 88);
  gtk_widget_set_size_request (label26, 168, 24);
  gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

  label27 = gtk_label_new (_("Username:"));
  gtk_widget_show (label27);
  gtk_fixed_put (GTK_FIXED (fixed8), label27, 0, 120);
  gtk_widget_set_size_request (label27, 168, 24);
  gtk_misc_set_alignment (GTK_MISC (label27), 0, 0.5);

  label28 = gtk_label_new (_("Password:"));
  gtk_widget_show (label28);
  gtk_fixed_put (GTK_FIXED (fixed8), label28, 0, 160);
  gtk_widget_set_size_request (label28, 168, 24);
  gtk_misc_set_alignment (GTK_MISC (label28), 0, 0.5);

  label29 = gtk_label_new (_("Password again:"));
  gtk_widget_show (label29);
  gtk_fixed_put (GTK_FIXED (fixed8), label29, 0, 192);
  gtk_widget_set_size_request (label29, 168, 24);
  gtk_misc_set_alignment (GTK_MISC (label29), 0, 0.5);

  label25 = gtk_label_new ("");
  gtk_widget_show (label25);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label25);
  gtk_label_set_use_markup (GTK_LABEL (label25), TRUE);

  hseparator5 = gtk_hseparator_new ();
  gtk_widget_show (hseparator5);
  gtk_fixed_put (GTK_FIXED (fixed5), hseparator5, 16, 96);
  gtk_widget_set_size_request (hseparator5, 640, 16);

  label16 = gtk_label_new (_("Root-password: This password will be used for the administrator. So keep it in mind!\n\nUsername: The username should consist of lower case letters (and numbers) only.\n\nImportant: All passwords have to be between 6 and 20 characters long!"));
  gtk_widget_show (label16);
  gtk_fixed_put (GTK_FIXED (fixed5), label16, 16, 8);
  gtk_widget_set_size_request (label16, 640, 88);
  gtk_label_set_justify (GTK_LABEL (label16), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label16), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  label4 = gtk_label_new (_("User"));
  gtk_widget_show (label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label4);

  fixed6 = gtk_fixed_new ();
  gtk_widget_show (fixed6);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed6);

  hostname = gtk_entry_new ();
  gtk_widget_show (hostname);
  gtk_fixed_put (GTK_FIXED (fixed6), hostname, 264, 184);
  gtk_widget_set_size_request (hostname, 232, 24);
  gtk_entry_set_text (GTK_ENTRY (hostname), _("siduxbox"));

  hseparator11 = gtk_hseparator_new ();
  gtk_widget_show (hseparator11);
  gtk_fixed_put (GTK_FIXED (fixed6), hseparator11, 16, 96);
  gtk_widget_set_size_request (hseparator11, 640, 16);

  vseparator6 = gtk_vseparator_new ();
  gtk_widget_show (vseparator6);
  gtk_fixed_put (GTK_FIXED (fixed6), vseparator6, 520, 168);
  gtk_widget_set_size_request (vseparator6, 16, 128);

  hseparator12 = gtk_hseparator_new ();
  gtk_widget_show (hseparator12);
  gtk_fixed_put (GTK_FIXED (fixed6), hseparator12, 120, 160);
  gtk_widget_set_size_request (hseparator12, 408, 16);

  label32 = gtk_label_new (_("Hostname:"));
  gtk_widget_show (label32);
  gtk_fixed_put (GTK_FIXED (fixed6), label32, 144, 184);
  gtk_widget_set_size_request (label32, 112, 24);
  gtk_misc_set_alignment (GTK_MISC (label32), 0.04, 0.5);

  vseparator5 = gtk_vseparator_new ();
  gtk_widget_show (vseparator5);
  gtk_fixed_put (GTK_FIXED (fixed6), vseparator5, 112, 168);
  gtk_widget_set_size_request (vseparator5, 16, 128);

  hseparator13 = gtk_hseparator_new ();
  gtk_widget_show (hseparator13);
  gtk_fixed_put (GTK_FIXED (fixed6), hseparator13, 120, 288);
  gtk_widget_set_size_request (hseparator13, 408, 16);

  label31 = gtk_label_new (_("Hostname: The hostname should consist of letters (and numbers) only and it must not begin with a number."));
  gtk_widget_show (label31);
  gtk_fixed_put (GTK_FIXED (fixed6), label31, 16, 8);
  gtk_widget_set_size_request (label31, 640, 88);
  gtk_label_set_justify (GTK_LABEL (label31), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label31), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label31), 0, 0.5);

  label5 = gtk_label_new (_("Network"));
  gtk_widget_show (label5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), label5);

  fixed7 = gtk_fixed_new ();
  gtk_widget_show (fixed7);
  gtk_container_add (GTK_CONTAINER (notebook1), fixed7);

  hseparator14 = gtk_hseparator_new ();
  gtk_widget_show (hseparator14);
  gtk_fixed_put (GTK_FIXED (fixed7), hseparator14, 16, 96);
  gtk_widget_set_size_request (hseparator14, 640, 16);

  vseparator8 = gtk_vseparator_new ();
  gtk_widget_show (vseparator8);
  gtk_fixed_put (GTK_FIXED (fixed7), vseparator8, 88, 136);
  gtk_widget_set_size_request (vseparator8, 16, 160);

  vseparator7 = gtk_vseparator_new ();
  gtk_widget_show (vseparator7);
  gtk_fixed_put (GTK_FIXED (fixed7), vseparator7, 560, 136);
  gtk_widget_set_size_request (vseparator7, 16, 160);

  hseparator15 = gtk_hseparator_new ();
  gtk_widget_show (hseparator15);
  gtk_fixed_put (GTK_FIXED (fixed7), hseparator15, 96, 128);
  gtk_widget_set_size_request (hseparator15, 472, 16);

  hseparator16 = gtk_hseparator_new ();
  gtk_widget_show (hseparator16);
  gtk_fixed_put (GTK_FIXED (fixed7), hseparator16, 96, 288);
  gtk_widget_set_size_request (hseparator16, 472, 16);

  button_install = gtk_button_new ();
  gtk_widget_show (button_install);
  gtk_fixed_put (GTK_FIXED (fixed7), button_install, 416, 320);
  gtk_widget_set_size_request (button_install, 152, 34);

  alignment6 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment6);
  gtk_container_add (GTK_CONTAINER (button_install), alignment6);

  hbox5 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox5);
  gtk_container_add (GTK_CONTAINER (alignment6), hbox5);

  image8 = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image8);
  gtk_box_pack_start (GTK_BOX (hbox5), image8, FALSE, FALSE, 0);

  label34 = gtk_label_new_with_mnemonic (_("Begin Installation"));
  gtk_widget_show (label34);
  gtk_box_pack_start (GTK_BOX (hbox5), label34, FALSE, FALSE, 0);

  radiobutton1 = gtk_radio_button_new_with_mnemonic (NULL, _("Save configuration\nand start installation"));
  gtk_widget_show (radiobutton1);
  gtk_fixed_put (GTK_FIXED (fixed7), radiobutton1, 120, 136);
  gtk_widget_set_size_request (radiobutton1, 208, 56);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton1), radiobutton1_group);
  radiobutton1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton1));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton1), TRUE);

  checkbutton_force = gtk_check_button_new_with_mnemonic (_("Force Installation\n(Ignore RAM-/Partition-Check, etc.)"));
  gtk_widget_show (checkbutton_force);
  gtk_fixed_put (GTK_FIXED (fixed7), checkbutton_force, 328, 136);
  gtk_widget_set_size_request (checkbutton_force, 232, 56);

  radiobutton3 = gtk_radio_button_new_with_mnemonic (NULL, _("Start Sidux-Installer-Script\nwithout configuration\n(this configuration will be lost)"));
  gtk_widget_show (radiobutton3);
  gtk_fixed_put (GTK_FIXED (fixed7), radiobutton3, 120, 232);
  gtk_widget_set_size_request (radiobutton3, 376, 56);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton3), radiobutton1_group);
  radiobutton1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton3));

  radiobutton2 = gtk_radio_button_new_with_mnemonic (NULL, _("Save configuration\nonly (~/.knofig)"));
  gtk_widget_show (radiobutton2);
  gtk_fixed_put (GTK_FIXED (fixed7), radiobutton2, 120, 184);
  gtk_widget_set_size_request (radiobutton2, 368, 56);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton2), radiobutton1_group);
  radiobutton1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton2));

  label33 = gtk_label_new (_("You provided all Information neccessary to start the Sidux-Installation.\n\nYou can check or change your Options once again by pressing the \"Prev\" Button.\n\nIf everything is correct for you, begin the installation by pressing \"Begin installation\"."));
  gtk_widget_show (label33);
  gtk_fixed_put (GTK_FIXED (fixed7), label33, 16, 8);
  gtk_widget_set_size_request (label33, 640, 88);
  gtk_label_set_justify (GTK_LABEL (label33), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (label33), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label33), 0, 0.5);

  label6 = gtk_label_new (_("Installation"));
  gtk_widget_show (label6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), label6);

  g_signal_connect ((gpointer) window1, "configure_event",
                    G_CALLBACK (on_window1_configure_event),
                    NULL);
  g_signal_connect ((gpointer) window1, "delete_event",
                    G_CALLBACK (on_window1_delete_event),
                    NULL);
  g_signal_connect ((gpointer) exit, "clicked",
                    G_CALLBACK (on_exit_clicked),
                    NULL);
  g_signal_connect ((gpointer) prev, "clicked",
                    G_CALLBACK (on_prev_clicked),
                    NULL);
  g_signal_connect ((gpointer) next, "clicked",
                    G_CALLBACK (on_next_clicked),
                    NULL);
  g_signal_connect ((gpointer) notebook1, "switch_page",
                    G_CALLBACK (on_notebook1_switch_page),
                    NULL);
  g_signal_connect ((gpointer) checkbutton_mountpoints, "toggled",
                    G_CALLBACK (on_checkbutton_mountpoints_toggled),
                    NULL);
  g_signal_connect ((gpointer) button_gparted, "clicked",
                    G_CALLBACK (on_button_gparted_clicked),
                    NULL);
  g_signal_connect ((gpointer) button_gparted, "released",
                    G_CALLBACK (on_button_gparted_released),
                    NULL);
  g_signal_connect ((gpointer) button_gparted, "pressed",
                    G_CALLBACK (on_button_gparted_pressed),
                    NULL);
  g_signal_connect ((gpointer) rootpartcombo, "changed",
                    G_CALLBACK (on_rootpartcombo_changed),
                    NULL);
  g_signal_connect ((gpointer) button_install, "clicked",
                    G_CALLBACK (on_button_install_clicked),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (window1, window1, "window1");
  GLADE_HOOKUP_OBJECT (window1, fixed1, "fixed1");
  GLADE_HOOKUP_OBJECT (window1, exit, "exit");
  GLADE_HOOKUP_OBJECT (window1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (window1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (window1, image5, "image5");
  GLADE_HOOKUP_OBJECT (window1, label10, "label10");
  GLADE_HOOKUP_OBJECT (window1, prev, "prev");
  GLADE_HOOKUP_OBJECT (window1, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (window1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (window1, image6, "image6");
  GLADE_HOOKUP_OBJECT (window1, label11, "label11");
  GLADE_HOOKUP_OBJECT (window1, next, "next");
  GLADE_HOOKUP_OBJECT (window1, alignment4, "alignment4");
  GLADE_HOOKUP_OBJECT (window1, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (window1, image7, "image7");
  GLADE_HOOKUP_OBJECT (window1, label12, "label12");
  GLADE_HOOKUP_OBJECT (window1, label_wellcome, "label_wellcome");
  GLADE_HOOKUP_OBJECT (window1, image1, "image1");
  GLADE_HOOKUP_OBJECT (window1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (window1, fixed2, "fixed2");
  GLADE_HOOKUP_OBJECT (window1, hseparator2, "hseparator2");
  GLADE_HOOKUP_OBJECT (window1, hseparator7, "hseparator7");
  GLADE_HOOKUP_OBJECT (window1, vseparator1, "vseparator1");
  GLADE_HOOKUP_OBJECT (window1, hseparator8, "hseparator8");
  GLADE_HOOKUP_OBJECT (window1, vseparator2, "vseparator2");
  GLADE_HOOKUP_OBJECT (window1, label_wellcome_install, "label_wellcome_install");
  GLADE_HOOKUP_OBJECT (window1, label30, "label30");
  GLADE_HOOKUP_OBJECT (window1, hseparator3, "hseparator3");
  GLADE_HOOKUP_OBJECT (window1, image2, "image2");
  GLADE_HOOKUP_OBJECT (window1, label_wellcome_red, "label_wellcome_red");
  GLADE_HOOKUP_OBJECT (window1, label_wellcome_2, "label_wellcome_2");
  GLADE_HOOKUP_OBJECT (window1, label1, "label1");
  GLADE_HOOKUP_OBJECT (window1, fixed3, "fixed3");
  GLADE_HOOKUP_OBJECT (window1, hseparator1, "hseparator1");
  GLADE_HOOKUP_OBJECT (window1, label7, "label7");
  GLADE_HOOKUP_OBJECT (window1, label8, "label8");
  GLADE_HOOKUP_OBJECT (window1, checkbutton_mountpoints, "checkbutton_mountpoints");
  GLADE_HOOKUP_OBJECT (window1, button_gparted, "button_gparted");
  GLADE_HOOKUP_OBJECT (window1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (window1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (window1, image4, "image4");
  GLADE_HOOKUP_OBJECT (window1, label9, "label9");
  GLADE_HOOKUP_OBJECT (window1, format_combo, "format_combo");
  GLADE_HOOKUP_OBJECT (window1, checkbutton_format_with, "checkbutton_format_with");
  GLADE_HOOKUP_OBJECT (window1, rootpartcombo, "rootpartcombo");
  GLADE_HOOKUP_OBJECT (window1, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (window1, treeview1, "treeview1");
  GLADE_HOOKUP_OBJECT (window1, label35, "label35");
  GLADE_HOOKUP_OBJECT (window1, label2, "label2");
  GLADE_HOOKUP_OBJECT (window1, fixed4, "fixed4");
  GLADE_HOOKUP_OBJECT (window1, hseparator4, "hseparator4");
  GLADE_HOOKUP_OBJECT (window1, checkbutton_bootdisk, "checkbutton_bootdisk");
  GLADE_HOOKUP_OBJECT (window1, label15, "label15");
  GLADE_HOOKUP_OBJECT (window1, hseparator9, "hseparator9");
  GLADE_HOOKUP_OBJECT (window1, vseparator3, "vseparator3");
  GLADE_HOOKUP_OBJECT (window1, vseparator4, "vseparator4");
  GLADE_HOOKUP_OBJECT (window1, hseparator10, "hseparator10");
  GLADE_HOOKUP_OBJECT (window1, label14, "label14");
  GLADE_HOOKUP_OBJECT (window1, combobox_bootmanager, "combobox_bootmanager");
  GLADE_HOOKUP_OBJECT (window1, combobox_installplace, "combobox_installplace");
  GLADE_HOOKUP_OBJECT (window1, label13, "label13");
  GLADE_HOOKUP_OBJECT (window1, label3, "label3");
  GLADE_HOOKUP_OBJECT (window1, fixed5, "fixed5");
  GLADE_HOOKUP_OBJECT (window1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (window1, alignment5, "alignment5");
  GLADE_HOOKUP_OBJECT (window1, fixed8, "fixed8");
  GLADE_HOOKUP_OBJECT (window1, entry_rootpw, "entry_rootpw");
  GLADE_HOOKUP_OBJECT (window1, entry_rootpw_again, "entry_rootpw_again");
  GLADE_HOOKUP_OBJECT (window1, entry_realname, "entry_realname");
  GLADE_HOOKUP_OBJECT (window1, entry_username, "entry_username");
  GLADE_HOOKUP_OBJECT (window1, entry_pw, "entry_pw");
  GLADE_HOOKUP_OBJECT (window1, entry_pw_again, "entry_pw_again");
  GLADE_HOOKUP_OBJECT (window1, hseparator6, "hseparator6");
  GLADE_HOOKUP_OBJECT (window1, label18, "label18");
  GLADE_HOOKUP_OBJECT (window1, label21, "label21");
  GLADE_HOOKUP_OBJECT (window1, label26, "label26");
  GLADE_HOOKUP_OBJECT (window1, label27, "label27");
  GLADE_HOOKUP_OBJECT (window1, label28, "label28");
  GLADE_HOOKUP_OBJECT (window1, label29, "label29");
  GLADE_HOOKUP_OBJECT (window1, label25, "label25");
  GLADE_HOOKUP_OBJECT (window1, hseparator5, "hseparator5");
  GLADE_HOOKUP_OBJECT (window1, label16, "label16");
  GLADE_HOOKUP_OBJECT (window1, label4, "label4");
  GLADE_HOOKUP_OBJECT (window1, fixed6, "fixed6");
  GLADE_HOOKUP_OBJECT (window1, hostname, "hostname");
  GLADE_HOOKUP_OBJECT (window1, hseparator11, "hseparator11");
  GLADE_HOOKUP_OBJECT (window1, vseparator6, "vseparator6");
  GLADE_HOOKUP_OBJECT (window1, hseparator12, "hseparator12");
  GLADE_HOOKUP_OBJECT (window1, label32, "label32");
  GLADE_HOOKUP_OBJECT (window1, vseparator5, "vseparator5");
  GLADE_HOOKUP_OBJECT (window1, hseparator13, "hseparator13");
  GLADE_HOOKUP_OBJECT (window1, label31, "label31");
  GLADE_HOOKUP_OBJECT (window1, label5, "label5");
  GLADE_HOOKUP_OBJECT (window1, fixed7, "fixed7");
  GLADE_HOOKUP_OBJECT (window1, hseparator14, "hseparator14");
  GLADE_HOOKUP_OBJECT (window1, vseparator8, "vseparator8");
  GLADE_HOOKUP_OBJECT (window1, vseparator7, "vseparator7");
  GLADE_HOOKUP_OBJECT (window1, hseparator15, "hseparator15");
  GLADE_HOOKUP_OBJECT (window1, hseparator16, "hseparator16");
  GLADE_HOOKUP_OBJECT (window1, button_install, "button_install");
  GLADE_HOOKUP_OBJECT (window1, alignment6, "alignment6");
  GLADE_HOOKUP_OBJECT (window1, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (window1, image8, "image8");
  GLADE_HOOKUP_OBJECT (window1, label34, "label34");
  GLADE_HOOKUP_OBJECT (window1, radiobutton1, "radiobutton1");
  GLADE_HOOKUP_OBJECT (window1, checkbutton_force, "checkbutton_force");
  GLADE_HOOKUP_OBJECT (window1, radiobutton3, "radiobutton3");
  GLADE_HOOKUP_OBJECT (window1, radiobutton2, "radiobutton2");
  GLADE_HOOKUP_OBJECT (window1, label33, "label33");
  GLADE_HOOKUP_OBJECT (window1, label6, "label6");
  GLADE_HOOKUP_OBJECT_NO_REF (window1, tooltips, "tooltips");

  return window1;
}

