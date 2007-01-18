#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include "callbacks.h"
#include "interface.h"
#include "support.h"


#define FILENAME ".knofig"
#define SCANPARTITIONS "$(scanpartitions 2> /dev/null | grep -v -e swap -e null | awk -F' ' '{print $1\"-\"$3}' > "
#define INSTALL_SH "knx-installer nonInteractive &"
#define INSTALL_SH_WITHOUT_CONFIG "knx-installer &"

FILE* fp;
char scanparttmp[80];
char systemcallstr[100];
char mountpoints_config[512];
int  do_it_at_first_time = 0, counter;

enum
{
  COL_DEVICE,
  COL_FS,
  COL_MOUNTP
} ;


void cell_edit_cb(GtkCellRendererText *cell,
                  const gchar *path_string,
                  const gchar *new_text,
                  gpointer data)
{

  /* ======================================================= *
   * Callback for editable Mountpoints in the treeview1      *
   * ======================================================= */
  GtkTreeModel *model;
  GtkTreePath  *path;
  GtkTreeIter iter;
  gchar *old_text;

  //printf("cell_edit_cb\n");

  model  = (GtkTreeModel *) data;
  path   = gtk_tree_path_new_from_string(path_string);

  gtk_tree_model_get_iter(model, &iter, path);

  gtk_tree_model_get(model, &iter, 2, &old_text, -1);  // 0 = first column, 1 = second column

  //printf("old:%s new:%s\n", old_text, new_text);

  if( strcmp(old_text, "/") != 0  &&      // not change the root partition in the treeview
      ( strcmp(new_text, "") == 0  ||
        strcmp(new_text, "/bin") == 0  ||
        strcmp(new_text, "/boot") == 0 ||
        strcmp(new_text, "/etc") == 0  ||
        strcmp(new_text, "/home") == 0 ||
        strcmp(new_text, "/lib") == 0  ||
        strcmp(new_text, "/opt") == 0  ||
        strcmp(new_text, "/root") == 0 ||
        strcmp(new_text, "/sbin") == 0 ||
        strcmp(new_text, "/tmp") == 0  ||
        strcmp(new_text, "/usr") == 0  ||
        strcmp(new_text, "/var") == 0 )
      )
   {

      gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                COL_MOUNTP, new_text,
                -1);
    }
}

gboolean
foreach_func (GtkTreeModel *model,
              GtkTreePath  *path,
              GtkTreeIter  *iter,
              gpointer      user_data)
{
  gchar *ptr_dev, *ptr_fs, *slash, *tree_path_str;

  /* Note: here we use 'iter' and not '&iter', because we did not allocate
   *  the iter on the stack and are already getting the pointer to a tree iter */

  gtk_tree_model_get (model, iter,
                      COL_DEVICE, &ptr_dev,
                      COL_FS,     &ptr_fs,
                      COL_MOUNTP, &slash,
                      -1);

  tree_path_str = gtk_tree_path_to_string(path);

  if( strncmp(slash, "/", 1) == 0 && strlen(slash) > 1 ) {
     if(counter++ > 0 ) {
        strcat(mountpoints_config, " ");
     }
     g_print ("%s:%s\n", ptr_dev, slash);
     strcat(mountpoints_config, ptr_dev);
     strcat(mountpoints_config, ":");
     strcat(mountpoints_config, slash);
  }

  g_free(tree_path_str);

  g_free(ptr_dev); /* gtk_tree_model_get made copies of       */
  g_free(ptr_fs);  /* the strings for us when retrieving them */
  g_free(slash);

  return FALSE; /* do not stop walking the store, call us with next row */
}


void read_partitions(GtkComboBox     *combobox) 
{
  /* ======================================================= *
   * read the partitions with the systemcall scanpartitions  *
   * and put it in the combo box                             *
   * ======================================================= */

   char partition[80];
   char *ptr_dev, *ptr_fs;
   int fd;


   strcpy(scanparttmp, "/tmp/scanpartitions-gui.XXXXXX");
   fd = mkstemp(scanparttmp);  // make a tempfile

   if( fd ) {
            // create the shell system command (scanpartitions)
            strcpy(systemcallstr, SCANPARTITIONS);
            strcat(systemcallstr, scanparttmp);
            strcat(systemcallstr, ")");

            system(systemcallstr);  // write the partitiontable to the tempfile
            close(fd);
   }
   else  {
            perror("mkstemp(scanparttmp)");
   }

   // read the scanpartition temp file
   fp=fopen(scanparttmp, "r");
   if( fp == NULL ) {
       strcpy(partition, "tmp file error");
       gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), partition);
   }
   else {

       // clear the combo_box before
       GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
       gtk_list_store_clear(GTK_LIST_STORE(model));

       // appand to combo_box
       fseek( fp, 0L, SEEK_SET );
       while (fscanf(fp, "%s", partition) != EOF) {

          //printf("%s %s\n", "combobox setzen, partition");

          // example of partition  /dev/hda1-ext3
          ptr_dev = strtok(partition, "-");  // ptr_dev is /dev/hda1
          ptr_fs = strtok(NULL, "-");        // ptr_fs is ext3

          if( strncmp(ptr_fs, "reiser", 6) == 0 ||
              strncmp(ptr_fs, "ext", 3) == 0 ||
              strcmp(ptr_fs, "jfs") == 0 ||
              strcmp(ptr_fs, "xfs") == 0 ) {

              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ptr_dev);
          }
     }
     fclose(fp);
     gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),0);
   }
}


gboolean
on_window1_configure_event             (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
 if( do_it_at_first_time < 1 ) {

   do_it_at_first_time = 1;  // only at start

  /* ==================================================
   * activate mount point of other partitions treeviev
   * ================================================== */
   GtkWidget *treeview1;
   GtkListStore *model;
   GtkCellRenderer *cell, *cell_editable;
   GtkTreeViewColumn *mointpoint, *fs, *device;
   GtkWidget *rootpartcombo;
   GtkWidget *label;
   PangoFontDescription *font_desc;
   GdkColor color;

   /* treeview, other Mountpoints */
   treeview1   = lookup_widget (GTK_WIDGET (widget), "treeview1");
   model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

   gtk_tree_view_set_model(GTK_TREE_VIEW(treeview1), GTK_TREE_MODEL (model));
   cell = gtk_cell_renderer_text_new();
   cell_editable = gtk_cell_renderer_text_new();

   // set the right cell of treeview editable and colors
   g_object_set (cell_editable,
                "editable", TRUE,
                "foreground", "red",
                "background", "gray",
                NULL);

   device     = gtk_tree_view_column_new_with_attributes("Device", cell, "text", 0, NULL);
   fs         = gtk_tree_view_column_new_with_attributes("FS", cell, "text", 1, NULL);
   mointpoint = gtk_tree_view_column_new_with_attributes("Mountpoint", cell_editable, "text", 2, NULL);

   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), GTK_TREE_VIEW_COLUMN(device));
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), GTK_TREE_VIEW_COLUMN(fs));
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), GTK_TREE_VIEW_COLUMN(mointpoint));

   // when the cell was edited store the new entry
   g_signal_connect (G_OBJECT(cell_editable), "edited",
		    G_CALLBACK(cell_edit_cb),
		    model);

   //gtk_tree_view_column_set_resizable(mointpoint,TRUE);
   //gtk_tree_view_column_set_resizable(fs,TRUE);
   //gtk_tree_view_column_set_resizable(device,TRUE);
   //gtk_tree_view_column_set_fixed_width(device, 20);
   //gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);


  /* ============================================================ *
   *                   fill the root partition combo box          *
   * ============================================================ */
   rootpartcombo = lookup_widget (GTK_WIDGET (widget), "rootpartcombo");
   read_partitions( GTK_COMBO_BOX (rootpartcombo) );


  /* ============================================================= *
   *                   fill the format_combo_box                   *
   * ============================================================= */

   GtkWidget *format_combo = lookup_widget (GTK_WIDGET (widget), "format_combo");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext3");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext2");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "reiserfs");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "jfs");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "xfs");

   gtk_combo_box_set_active( GTK_COMBO_BOX(format_combo),0);


  /* ============================================================= *
   *                   fill the combobox_bootmanager               *
   * ============================================================= */
   GtkWidget *combobox_bootmanager = lookup_widget (GTK_WIDGET (widget), "combobox_bootmanager");
   gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_bootmanager), "grub");
   gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_bootmanager), "lilo");

   gtk_combo_box_set_active( GTK_COMBO_BOX(combobox_bootmanager),0);


  /* ============================================================= *
   *                   fill the combobox_installplace              *
   * ============================================================= */
   GtkWidget *combobox_installplace = lookup_widget (GTK_WIDGET (widget), "combobox_installplace");
   gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_installplace), "mbr");
   gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_installplace), "partition");

   gtk_combo_box_set_active( GTK_COMBO_BOX(combobox_installplace),0);


  /* ============================================================= *
   *           Label sets, font, color, etc.                       *
   * ============================================================= */
   label = lookup_widget (GTK_WIDGET (widget), "label_wellcome");

   font_desc = pango_font_description_from_string ("20");

   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_install");
   gdk_color_parse ("darkblue", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("14");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_red");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_2");
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);
 }

 return FALSE;
}


void
on_rootpartcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
   // if the Root-Partition Combo Box has changed

   GtkWidget *rootpartcombo;
   //gchar *rootpart;

   rootpartcombo = lookup_widget (GTK_WIDGET (combobox), "rootpartcombo");
   //rootpart = gtk_combo_box_get_active_text(GTK_COMBO_BOX (rootpartcombo));

   // change also the  / (rootpartition) entry in the treeview, set the treeview new
   g_signal_connect ((gpointer) rootpartcombo, "changed",
                    G_CALLBACK (on_checkbutton_mountpoints_toggled),
                    NULL);

}


void
on_checkbutton_mountpoints_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  // GtkWidget *toggle;
   GtkTreeIter iter_tb;
   GtkWidget *rootpartcombo;
   gchar *rootpart;

   char partition[80];
   char slash[80];
   char *ptr_dev, *ptr_fs;


   GtkWidget *toggle = lookup_widget ( GTK_WIDGET (togglebutton), "checkbutton_mountpoints");
   GtkWidget *treeview1 = lookup_widget (GTK_WIDGET (togglebutton), "treeview1");
   GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview1));
   rootpartcombo = lookup_widget (GTK_WIDGET (togglebutton), "rootpartcombo");

   gboolean enabled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));

   if (enabled == TRUE) {

        gtk_list_store_clear ( GTK_LIST_STORE (model) );  //make treeview empty


        /* get the partitiontable from scanpart tempfile */
        fp=fopen(scanparttmp, "r");
        if( fp == NULL ) {
           strcpy(partition, "tmp file error");
        }
        else {
            rootpart = gtk_combo_box_get_active_text(GTK_COMBO_BOX (rootpartcombo));

            fseek( fp, 0L, SEEK_SET );
            while (fscanf(fp, "%s", partition) != EOF) {

                /* Append a row and fill in some data */
                gtk_list_store_append ( GTK_LIST_STORE (model), &iter_tb);

                // example of partition  /dev/hda1-ext3
                ptr_dev = strtok(partition, "-");  // ptr_dev is /dev/hda1
                ptr_fs = strtok(NULL, "-");        // ptr_fs is ext3

                if( strcmp(ptr_dev, rootpart) == 0 ) {
                        strcpy(slash, "/");
                  }
                else {
                        strcpy(slash, "");
                  }

                gtk_list_store_set ( GTK_LIST_STORE (model), &iter_tb,
                         COL_DEVICE, ptr_dev,
                         COL_FS,     ptr_fs,
                         COL_MOUNTP, slash,
                         -1);
            }
            fclose(fp);
        }


   }
   else {
        gtk_list_store_clear (GTK_LIST_STORE (model) );
   }
}


void
on_next_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *notebook1;

   notebook1 = lookup_widget (GTK_WIDGET (button), "notebook1");
   gtk_notebook_next_page ( GTK_NOTEBOOK(notebook1) );
}


void
on_prev_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *notebook1;

   notebook1 = lookup_widget (GTK_WIDGET (button), "notebook1");
   gtk_notebook_prev_page ( GTK_NOTEBOOK(notebook1) );
}


gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   /* remove the tempfile */
   unlink(scanparttmp);

   gtk_main_quit();
   return FALSE;
}


void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
   /* remove the tempfile */
   unlink(scanparttmp);

   gtk_main_quit();
}


void
on_button_gparted_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_gparted_pressed              (GtkButton       *button,
                                        gpointer         user_data)
{   // hide the main window and start gparted
   GtkWidget *window1 = lookup_widget(GTK_WIDGET(button),"window1");
   gtk_widget_hide ( GTK_WIDGET (window1) ); 
}


void
on_button_gparted_released             (GtkButton       *button,
                                        gpointer         user_data)
{
   system("gparted");
   printf("%s\n", "rebuildfstab");
   system("rebuildfstab");  // rebuild the fstab

   /* remove the tempfile */
   unlink(scanparttmp);

   // rebuild the partitions in rootpartcombo and treeview1
   GtkWidget *rootpartcombo = lookup_widget (GTK_WIDGET (button), "rootpartcombo");
   read_partitions( GTK_COMBO_BOX (rootpartcombo) );

   // show the main window after gparted has done
   GtkWidget *window1 = lookup_widget(GTK_WIDGET(button),"window1");
   gtk_widget_show ( GTK_WIDGET (window1) );
}


void
on_button_install_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  /* ======================================================== *
   *                      read the widgets                    *
   * ======================================================== */
   GtkToggleButton *radiobutton, *checkbutton;
   GtkWidget *entry, *mainW, *dialog;
   char rootpw[21], rootpw_a[21], pw[21], pw_a[21], nname[80], uname[80], systemcall[80];
   FILE *stream;



   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton3"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
     /* ======================================================== *
      *          Start sidux-Installer-Script                    *
      *          without configuration                           *
      *         (this configuration will be lost)                *
      * ======================================================== */

      /* remove the tempfile */
      unlink(scanparttmp);

      system(INSTALL_SH_WITHOUT_CONFIG);

      gtk_main_quit();
   }
   else {

      // Root Password check
      entry = lookup_widget(GTK_WIDGET(button), "entry_rootpw");
      strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

      entry = lookup_widget(GTK_WIDGET(button), "entry_rootpw_again");
      strcpy(rootpw_a, gtk_entry_get_text(GTK_ENTRY(entry)));


      //  Message Dialog Root Password different
      if( strcmp( rootpw, rootpw_a ) != 0 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Root-Password different!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);

          return;
      }

      //  Message Dialog Root Password to short
      if( strlen( rootpw ) < 6 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Root-Password to short!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return;
      }


      //  Message Dialog RealName empty
      entry = lookup_widget(GTK_WIDGET(button), "entry_realname");
      strcpy(nname, gtk_entry_get_text(GTK_ENTRY(entry)));

      if( strlen( nname ) < 1 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Realname empty!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return;
      }


      //  Message Dialog UserName empty
      entry = lookup_widget(GTK_WIDGET(button), "entry_username");
      strcpy(uname, gtk_entry_get_text(GTK_ENTRY(entry)));

      if( strlen( uname ) < 1 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Username empty!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return;
      }

      // Password check
      entry = lookup_widget(GTK_WIDGET(button), "entry_pw");
      strcpy(pw, gtk_entry_get_text(GTK_ENTRY(entry)));

      entry = lookup_widget(GTK_WIDGET(button), "entry_pw_again");
      strcpy(pw_a, gtk_entry_get_text(GTK_ENTRY(entry)));


      //  Message Dialog Password different
      if( strcmp( pw, pw_a ) != 0 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Password different!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return;
      }

      //  Message Dialog Password to short
      if( strlen( pw ) < 6 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window1");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", "Password to short!" );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return;
      }


      // read the treeview1 (mountpoint) list
      GtkWidget *treeview1 = lookup_widget (GTK_WIDGET (button), "treeview1");
      GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview1));

      counter=0;
      strcpy(mountpoints_config, "HD_MAP=\'");
      gtk_tree_model_foreach(GTK_TREE_MODEL(model), foreach_func, NULL);  // add the mountpoints to mountpoints_config
      strcat(mountpoints_config, "\'");



     /* ============================================================= *
      *                    make the config file                       *
      * ============================================================= */
      if (!getenv("HOME")) {
         fprintf(stderr, "installer-gui: HOME isn't defined\n");
         }
      else if (chdir(getenv("HOME")) < 0) {
         fprintf(stderr, "installer-gui: cd HOME error\n");
      }

      stream = fopen( FILENAME, "w+" );
      if( stream == NULL )
         printf( "The file %s was not opened\n", FILENAME);
      else
      {
         fprintf( stream, "%s\n\n%s\n\n%s\n%s\n%s\n%s\n%s", 
"REGISTERED=' SYSTEM_MODULE HD_MODULE HD_FORMAT HD_FSTYPE HD_CHOICE HD_MAP HD_IGNORECHECK SWAP_MODULE SWAP_AUTODETECT SWAP_CHOICES NAME_MODULE NAME_NAME USER_MODULE USER_NAME USERPASS_MODULE USERPASS_CRYPT ROOTPASS_MODULE ROOTPASS_CRYPT HOST_MODULE HOST_NAME SERVICES_MODULE SERVICES_START BOOT_MODULE BOOT_LOADER BOOT_DISK BOOT_WHERE AUTOLOGIN_MODULE INSTALL_READY'", 

"SYSTEM_MODULE='configured'",
"HD_MODULE='configured'",

"# Determines if the HD should be formatted. (mkfs.*)",
"# Possible are: yes|no",
"# Default value is: yes",
"HD_FORMAT='");

         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_format_with"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }

         fprintf( stream, "%s\n%s\n%s\n%s", 
"# Sets the Filesystem type.",
"# Possible are: ext3|ext2|reiserfs|xfs|jfs",
"# Default value is: reiserfs",
"HD_FSTYPE='");
 
        gchar *hd_fstyp = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "format_combo")));
        fprintf( stream, "%s'\n%s\n%s\n%s", 
hd_fstyp,
"# Here the sidux-System will be installed",
"# This value will be checked by function module_hd_check",
"HD_CHOICE='");

        gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));
        fprintf( stream, "%s'\n%s\n", 
hd_choice,
"# Here you can give additional mappings. (Experimental) You need to have the partitions formatted yourself and give the correct mappings like: \"/dev/hda4:/boot /dev/hda5:/var /dev/hda6:/tmp\"");

        fprintf( stream, "%s\n%s\n%s\n%s\n%s", 
mountpoints_config,
"# If set to yes, the program will NOT check if there is enough space to install sidux on the selected partition(s). Use at your own risk! Useful for example with HD_MAP if you only have a small root partition.",
"# Possible are: yes|no",
"# Default value is: no",
"HD_IGNORECHECK='");

         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_force"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }

      fprintf( stream, 
"\n%s\n%s\n%s\n%s\n%s\n%s\n\%s\n%s\n\n%s\n%s%s'\n\n%s\n%s%s'\n\n%s\n%s\n\n%s\n%s\n\n\
%s\n%s%s'\n\n%s\n%s\n%s\n%s\n\n%s\n%s\n%s\n%s\n%s%s'\n%s\n%s\n%s\n%s", 

"SWAP_MODULE='configured'",
"# If set to yes, the swap partitions will be autodetected.",
"# Possible are: yes|no",
"# Default value is: yes",
"SWAP_AUTODETECT='yes'",
"# The swap partitions to be used by the installed sidux.",
"# This value will be checked by function module_swap_check",
"SWAP_CHOICES='__swapchoices__'",
"NAME_MODULE='configured'",
"NAME_NAME='",
nname,
"USER_MODULE='configured'",
"USER_NAME='",
uname,
"USERPASS_MODULE='configured'",
"USERPASS_CRYPT='__userpass_crypt__'",
"ROOTPASS_MODULE='configured'",
"ROOTPASS_CRYPT='__rootpass_crypt__'",
"HOST_MODULE='configured'",
"HOST_NAME='",
gtk_entry_get_text(GTK_ENTRY( lookup_widget(GTK_WIDGET(button), "hostname") )),
"SERVICES_MODULE='configured'",
"# Possible services are for now: kdm cupsys smail ssh samba",
"# Default value is: kdm",
"SERVICES_START='kdm cupsys'",
"BOOT_MODULE='configured'",
"# Chooses the Boot-Loader",
"# Possible are: lilo|grub",
"# Default value is: grub",
"BOOT_LOADER='",
gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_bootmanager"))),
"# If set to 'yes' a boot disk will be created!",
"# Possible are: yes|no",
"# Default value is: yes",
"BOOT_DISK='");

         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_bootdisk"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }

       fprintf( stream, "%s\n%s\n%s\n%s%s'\n\n%s\n%s\n", 
"# Where the Boot-Loader will be installed",
"# Possible are: mbr|partition",
"# Default value is: mbr",
"BOOT_WHERE='",
gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_installplace"))),
"AUTOLOGIN_MODULE='configured'",
"INSTALL_READY='yes'");

        fclose( stream );
      }


     /* ==================================================================== *
      * change the __.......__ entries in the .knofig file with system calls *
      * ==================================================================== */

      //__userpass_crypt__
      strcpy(systemcall, "sed -ie \"s#__userpass_crypt__#$(mkpasswd --hash=md5 \"");
      strcat(systemcall, pw);
      strcat(systemcall, "\")#\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

      //__rootpass_crypt__
      strcpy(systemcall, "sed -ie \"s#__rootpass_crypt__#$(mkpasswd --hash=md5 \"");
      strcat(systemcall, rootpw);
      strcat(systemcall, "\")#\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

      //__swapchoices__
      //awk '/^\/dev\//{print $1}' /proc/swaps
      strcpy(systemcall, "sed -ie \"s#__swapchoices__#$(awk '/^\\/dev\\//{print $1}' /proc/swaps | head -1)#\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

     /* ======================================================== *
      *         start the knx-installer non-interactive          *
      * ======================================================== */
      radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton1"));
      if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
         system(INSTALL_SH);
      }


      /* remove the tempfile */
      unlink(scanparttmp);

      gtk_main_quit();
   }
}


void
on_notebook1_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
     /* ==================================================================== *
      *          Hide the Next Button if Page Install is selected            *
      * ==================================================================== */

      GtkWidget *button_next = lookup_widget(GTK_WIDGET(notebook),"next");

      if( page_num < 5 )
          gtk_widget_show ( GTK_WIDGET (button_next) );
      else
          gtk_widget_hide ( GTK_WIDGET (button_next) );


}

