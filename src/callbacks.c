#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "inotify.h"
#include "inotify-syscalls.h"

#define BUF_LEN    1024

#define FILENAME ".sidconf"
#define TARGET_MNT_POINT "/media/hdinstall"

#define HD_SCAN                   "fdisk -l | grep \"Disk /dev\" | cut -d: -f1 | cut -d\" \" -f2 > "
#define SCANPARTITIONS            "fll_fshelper --install-gui 2>/dev/null > "

#define INSTALL_SH                ". /etc/default/distro; [ \"$FLL_DISTRO_MODE\" = live ] && fll-installer installer"
#define INSTALL_SH_WITH_TERMINAL  ". /etc/default/distro; [ \"$FLL_DISTRO_MODE\" = live ] && x-terminal-emulator --noclose -e fll-installer installer &"

#define LANG_SH "/etc/init.d/fll-locales list | sed 's|\t|, |' > "
//${LANGUAGE} is set in /etc/default/fll-locales
#define LANG_CUR ". /etc/default/fll-locales; printf \"DEFAULT_LANG:${LANGUAGE}\n\";sed -ie \"s/^${LANGUAGE},/DEFAULT_${LANGUAGE},/\" "



char scanparttmp[80];
char systemcallstr[BUF_LEN];
char mountpoints_config[512];
char rootpw[21], rootpw_a[21], pw[21], pw_a[21], nname[80], uname[80], lang_default[80], progressclock[80], install_call_tmp[80];
int  counter, leaved_user_page, i = 0, partitions_counter = 0;
//int do_it_at_first_time = 0;
GtkWidget *label_changed, *install_progressbar, *window_main;
 

// progressbar
char FILE_NAME[256];
GtkWidget *pprogres, *pprogres2, *label, *label_generally, *label_clock;
static gint fd, hour = 0, min = 0, sec = 0;


enum
{
  COL_DEVICE,
  COL_FS,
  COL_MOUNTP
} ;


void
is_the_device_a_usbdevice (GtkComboBox     *combobox)
{

 if ( partitions_counter > 0 ) {

  // is the selected install device a usb device, then only grub to partition
   char device[80], usbdevicetmp[80];
   char *entry1, *entry2 = "";
   int fd, len;


   gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (combobox), "rootpartcombo")));
   if( hd_choice == NULL ) {
      return;
   }

   if( strlen(hd_choice) > 5 ) { 

      entry1 = strtok(hd_choice, "/");
      entry2 = strtok(NULL, "/");

      strcpy(usbdevicetmp, "/tmp/usbdevice.XXXXXX");
      fd = mkstemp(usbdevicetmp);  // make a tempfile

      if( fd ) {
               // create the shell system command (scanpartitions)
              strcpy(device, "ls -l /sys/block/");
              strncat(device, entry2, 3);
              strcat(device, "/device | grep usb > ");
              strcat(device, usbdevicetmp);

              system(device);

      }
      else  {
               perror("mkstemp usb device");
      }


      // clear the combo_box before
      GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
      gtk_list_store_clear(GTK_LIST_STORE(model));


      len = lseek(fd, 0, SEEK_END);

      if( len == 0 ) {
          // no usb device found
          gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "mbr");
          gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "partition");
      }
      else {
          gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "partition");
      }
      gtk_combo_box_set_active( GTK_COMBO_BOX(combobox),0);
      close(fd);

      /* remove the tempfile */
      unlink(usbdevicetmp);
   }
 }

}


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
        strcmp(new_text, "/boot") == 0 ||
        strcmp(new_text, "/home") == 0 ||
        strcmp(new_text, "/opt") == 0  ||
        strcmp(new_text, "/root") == 0 ||
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
   FILE* fp;

   char partition[80];
   char *ptr_dev, *ptr_fs;
   int fd;


   strcpy(scanparttmp, "/tmp/scanpartitions-gui.XXXXXX");
   fd = mkstemp(scanparttmp);  // make a tempfile

   if( fd ) {
            // create the shell system command (scanpartitions)
            strncpy(systemcallstr, SCANPARTITIONS, BUF_LEN);
            strncat(systemcallstr, scanparttmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"======= scan partitions =======\n\";printf \"", BUF_LEN);
            strncat(systemcallstr, scanparttmp, BUF_LEN);
            strncat(systemcallstr, "\n\"; printf \"__________________________________\n\"; cat ", BUF_LEN);
            strncat(systemcallstr, scanparttmp, BUF_LEN);
            //strncat(systemcallstr, "; printf \"====================================\n\"", BUF_LEN);

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
       partitions_counter = 0;

       fseek( fp, 0L, SEEK_SET );
       while (fscanf(fp, "%s", partition) != EOF) {

          //printf("%s %s\n", "combobox setzen, partition");

          // example of partition  /dev/hda1-ext3
          ptr_dev = strtok(partition, "-");  // ptr_dev is /dev/hda1
          ptr_fs = strtok(NULL, "-");        // ptr_fs is ext3

          if( strncmp(ptr_fs, "reiser", 6) == 0 ||
              strncmp(ptr_fs, "ext", 3) == 0 ||
              strcmp(ptr_fs, "jfs") == 0 ) {

              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ptr_dev);
              gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),0);

              partitions_counter++;
          }
     }

     fclose(fp);

    /* ============================================================= *
     *                   fill the combobox_installplace              *
     * ============================================================= */
     GtkWidget *combobox_installplace = lookup_widget (GTK_WIDGET (combobox), "combobox_installplace");
     is_the_device_a_usbdevice ( GTK_COMBO_BOX (combobox_installplace));

   }

}


int
password_check(GtkWidget     *button) 
{

      GtkWidget *entry, *mainW, *dialog;

      // Root Password check
      entry = lookup_widget(GTK_WIDGET(button), "entry_rootpw");
      strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

      entry = lookup_widget(GTK_WIDGET(button), "entry_rootpw_again");
      strcpy(rootpw_a, gtk_entry_get_text(GTK_ENTRY(entry)));



      //  Message Dialog Root Password different
      if( strcmp( rootpw, rootpw_a ) != 0 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Root-Password different!"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);

          return 0;
      }

      //  Message Dialog Root Password too short
      if( strlen( rootpw ) < 6 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Root-Password too short!"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }


      //  Message Dialog RealName empty
      entry = lookup_widget(GTK_WIDGET(button), "entry_realname");
      strcpy(nname, gtk_entry_get_text(GTK_ENTRY(entry)));

      if( strlen( nname ) < 1 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Realname empty!"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }


      //  Message Dialog UserName empty
      entry = lookup_widget(GTK_WIDGET(button), "entry_username");
      strcpy(uname, gtk_entry_get_text(GTK_ENTRY(entry)));

      if( strlen( uname ) < 1 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Username empty!"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }

      // Password check
      entry = lookup_widget(GTK_WIDGET(button), "entry_pw");
      strcpy(pw, gtk_entry_get_text(GTK_ENTRY(entry)));

      entry = lookup_widget(GTK_WIDGET(button), "entry_pw_again");
      strcpy(pw_a, gtk_entry_get_text(GTK_ENTRY(entry)));


      //  Message Dialog Password different
      if( strcmp( pw, pw_a ) != 0 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Password different!"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }

      //  Message Dialog Password too short
      if( strlen( pw ) < 6 ) {
          mainW = lookup_widget (GTK_WIDGET (button), "window_main");
          dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("Root-Password too short!") );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }

   return 1;
}


void
on_rootpartcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
   // if the Root-Partition Combo Box has changed

   GtkWidget *rootpartcombo;

   rootpartcombo = lookup_widget (GTK_WIDGET (combobox), "rootpartcombo");
   //rootpart = gtk_combo_box_get_active_text(GTK_COMBO_BOX (rootpartcombo));


   // fill the combobox_installplace
   GtkWidget *combobox_installplace = lookup_widget (GTK_WIDGET (combobox), "combobox_installplace");
   is_the_device_a_usbdevice ( GTK_COMBO_BOX (combobox_installplace));

   // change the label-rootpart-warning
   GtkWidget* label_rootpart_warning = lookup_widget( GTK_WIDGET( combobox ), "label_rootpart_warning" );
   gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (
                     lookup_widget (GTK_WIDGET ( combobox ), "rootpartcombo")));
   gtk_label_set_text( GTK_LABEL ( label_rootpart_warning ), hd_choice );


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
   FILE* fp;

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
on_window_main_delete_event                (GtkWidget       *widget,
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
start_install_meta()

{
   char sh_call[BUF_LEN];

   // >>>>>>>>>>>>>>>   installation done   <<<<<<<<<<<<<<<<<<
   //           start install meta
   strncpy(sh_call, "source ${HOME}/", BUF_LEN);
   strncat(sh_call, FILENAME, BUF_LEN);
   strncat(sh_call, "\nif [ \"$INSTALL_META\" = yes -a -n \"$DISPLAY\" ]\n", BUF_LEN);
   strncat(sh_call, "then\n", BUF_LEN);
   strncat(sh_call, "   if [ -x /usr/bin/install-meta ]; then\n", BUF_LEN);
   strncat(sh_call, "       exec /usr/bin/install-meta --chroot=", BUF_LEN);
   strncat(sh_call, TARGET_MNT_POINT, BUF_LEN);
   strncat(sh_call, " &\n   else\n", BUF_LEN);
   strncat(sh_call, "       echo \"install-meta is not available\" 1>&2\n", BUF_LEN);
   strncat(sh_call, "  fi\n", BUF_LEN);
   strncat(sh_call, "fi\n", BUF_LEN);

   system(sh_call);
}


void
on_success_exit_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
   // start install-meta
   start_install_meta ();

   // remove the tempfile
   unlink(scanparttmp);

   //remove the initofy tempfile
   unlink(install_call_tmp);

   gtk_main_quit();
}


gboolean
on_dialog_end_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   // start install-meta
   start_install_meta ();

   // remove the tempfile
   unlink(scanparttmp);

   //remove the initofy tempfile
   unlink(install_call_tmp);

   gtk_main_quit();

   return FALSE;
}


gboolean 
rootpart_warning  ( gpointer user_data )
{
    //GtkWidget *rootpartcombo = lookup_widget ( GTK_WIDGET (label_changed), "rootpartcombo" );


    if ( i == 0 ) {
          gtk_label_set_markup ( GTK_LABEL ( label_changed ), 
                "<span foreground=\"red\" font_desc=\"Sans Bold 12\">Root Partition changed !!!</span>" );

          i = 1;
    }
    else {
          gtk_label_set_text( GTK_LABEL ( label_changed ), "" );

          i = 0;
    }

    return(TRUE);
}


void
on_button_gparted_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    FILE *stream;
    char sh_command[80];
    char kdeconfdir[256];

   // hide the main window after gparted has done
   //GtkWidget *window_main = lookup_widget(GTK_WIDGET(button),"window_main");
   gtk_widget_hide ( GTK_WIDGET (window_main) );
   while (gtk_events_pending ())
          gtk_main_iteration ();

   //disable kde automount
   strncpy(kdeconfdir, getenv("HOME"), 256);
   strncat(kdeconfdir, "/.kde/share/config/", 256);

   if (chdir(kdeconfdir) < 0) {
         printf("failed change to $HOME/.kde/share/config/\n");
   }
   else {
      stream = fopen( "medianotifierrc", "w+" );
      if( stream == NULL )
         printf( "The file medianotifierrc was not opened\n");
      else
      {
         fprintf( stream, "%s\n%s\n%s\n%s\n", 
"[Auto Actions]",
"media/cdwriter_unmounted=#NothinAction",
"media/hdd_unmounted=#NothinAction",
"media/removable_unmounted=#NothinAction"
          );
          fclose( stream );
      }
   }


   GtkWidget *combobox = lookup_widget (GTK_WIDGET (button), "combobox_hd");
   gchar *hd = gtk_combo_box_get_active_text(GTK_COMBO_BOX (combobox));


   // start prtition manager
   GtkToggleButton *radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton_part1"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {

       //start gparted
       if (hd != NULL) {
           strncpy(sh_command, "gparted ", 80);
           strncat(sh_command, hd, 80);

           system(sh_command);
       }
       else {
           system("gparted");
       }
   }

   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton_part2"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
       //start cfdisk
       if (hd != NULL) {
           strncpy(sh_command, "x-terminal-emulator -e cfdisk ", 80);
           strncat(sh_command, hd, 80);

           system(sh_command);
       }
       else {
           system("gparted");
       }

   }

   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton_part3"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
       //start fdisk
       if (hd != NULL) {
           strncpy(sh_command, "x-terminal-emulator -e fdisk ", 80);
           strncat(sh_command, hd, 80);

           system(sh_command);
       }
       else {
           system("gparted");
       }
   }

   // update disk name/symlinks after partitioning/reformatting
   system("udevtrigger");


   system("rm -f ${HOME}/.kde/share/config/medianotifierrc;printf \"\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\ncreate fstab\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\";fll_fshelper --write-fstab --make-mountpoints");  // create the fstab and start kde automount again


   /* remove the tempfile */
   unlink(scanparttmp);



   // has the rootpartcombo changed
   // get combo box
   GtkWidget *rootpartcombo = lookup_widget (GTK_WIDGET (button), "rootpartcombo");
   gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));


   // rebuild the partitions in rootpartcombo and treeview1
   read_partitions( GTK_COMBO_BOX (rootpartcombo) );


   gchar *hd_choice_post = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));


   if ( partitions_counter > 0 && hd_choice != NULL) {

      if ( strcmp(hd_choice, hd_choice_post) != 0 ) {

          label_changed = lookup_widget( GTK_WIDGET (button), "label_changed" );

          g_timeout_add( 1000, rootpart_warning, label_changed );
      }
   }

   // show the main window
   gtk_widget_show ( GTK_WIDGET (window_main) );

}


void
save_config            (GtkButton       *button)
{
      GtkToggleButton *checkbutton;
      char systemcall[256], services[17]; //, *default_lang, *default_country;
      FILE *stream;


      gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));

      // read the treeview1 (mountpoint) list
      GtkWidget *treeview1 = lookup_widget (GTK_WIDGET (button), "treeview1");
      GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview1));

      counter=0;
      strcpy(mountpoints_config, "HD_MAP=\'");
      gtk_tree_model_foreach(GTK_TREE_MODEL(model), foreach_func, NULL);  // add the mountpoints to mountpoints_config
      strcat(mountpoints_config, "\'");


      // start services
      strncpy(services, "'", 17);

      checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_printsystem"));
      if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
         strncat(services, "cupsys", 17);
      }

      checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_ssh"));
      if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {

         if( strlen( services ) > 1 ) {
             strncat(services, " ", 17);
         }

         strncat(services, "ssh", 17);
      }
      strncat(services, "'", 17);


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
         fprintf( stream, "%s\n\n%s\n%s\n%s\n%s\n%s\n%s", 
"REGISTERED=' SYSTEM_MODULE HD_MODULE HD_FORMAT HD_FSTYPE HD_CHOICE HD_MAP HD_IGNORECHECK SWAP_MODULE SWAP_AUTODETECT SWAP_CHOICES NAME_MODULE NAME_NAME USER_MODULE USER_NAME USERPASS_MODULE USERPASS_CRYPT ROOTPASS_MODULE ROOTPASS_CRYPT HOST_MODULE HOST_NAME SERVICES_MODULE SERVICES_START BOOT_MODULE BOOT_LOADER BOOT_DISK BOOT_WHERE AUTOLOGIN_MODULE INSTALL_READY'", 

"SYSTEM_MODULE='configured'",
"HD_MODULE='configured'",

"\n# Determines if the HD should be formatted. (mkfs.*)",
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

         fprintf( stream, "\n%s\n%s\n%s\n%s", 
"# Sets the Filesystem type.",
"# Possible are: ext3|ext2|reiserfs|jfs",
"# Default value is: reiserfs",
"HD_FSTYPE='");
 
        gchar *hd_fstyp = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "format_combo")));
        fprintf( stream, "%s'\n%s\n%s\n%s", 
hd_fstyp,
"\n# Here the sidux-System will be installed",
"# This value will be checked by function module_hd_check",
"HD_CHOICE='");

        fprintf( stream, "%s'\n%s\n", 
hd_choice,
"\n# Here you can give additional mappings. (Experimental) You need to have the partitions formatted yourself and give the correct mappings like: \"/dev/hda4:/boot /dev/hda5:/var /dev/hda6:/tmp\"");

        fprintf( stream, "%s\n%s\n%s\n%s\n%s", 
mountpoints_config,
"\n# If set to yes, the program will NOT check if there is enough space to install sidux on the selected partition(s). Use at your own risk! Useful for example with HD_MAP if you only have a small root partition.",
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
%s\n%s%s'\n\n%s\n%s\n%s\n%s%s\n\n%s\n%s\n%s\n%s\n%s%s'\n%s\n%s\n%s\n%s", 

"SWAP_MODULE='configured'",
"# If set to yes, the swap partitions will be autodetected.",
"# Possible are: yes|no",
"# Default value is: yes",
"SWAP_AUTODETECT='yes'",

"\n# The swap partitions to be used by the installed sidux.",
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
"# Possible services are for now: cupsys smail ssh samba",
"# Default value is: cupsys",
"SERVICES_START=",
services,
"BOOT_MODULE='configured'",
"# Chooses the Boot-Loader",
"# Possible are: grub",
"# Default value is: grub",
"BOOT_LOADER='",
gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_bootmanager"))),
"\n# If set to 'yes' a boot disk will be created!",
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

       fprintf( stream, "%s\n%s\n%s\n%s%s'\n\n%s\n%s\n\n%s\n%s", 
"\n# Where the Boot-Loader will be installed",
"# Possible are: mbr|partition",
"# Default value is: mbr",
"BOOT_WHERE='",
gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_installplace"))),
"AUTOLOGIN_MODULE='configured'",
"INSTALL_READY='yes'",
"# Install aditional metapackages. Default value is: yes",
"INSTALL_META='"
         );

         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_metapackages"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }

       fprintf( stream, "\n%s\n%s", 
"# mount partitions on boot. Default value is: yes",
"HD_AUTO='"
         );

         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_automount"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }


        // Language 
/*        default_lang = strtok(gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_lang"))), ",");
        default_country = strtok(NULL, ",");

        fprintf( stream, "\n%s\n%s%s'\n%s%s'\n", 
"# locale",
"HD_LANG='",
        default_lang,
"HD_LANG_COUNTRY='",
        default_country
       );
*/

        fclose( stream );
      }


     /* ==================================================================== *
      * change the __.......__ entries in the .sidconf file with system calls *
      * ==================================================================== */

      //__userpass_crypt__
      strcpy(systemcall, "sed -ie \"s%__userpass_crypt__%$(mkpasswd --hash=md5 \"");
      strcat(systemcall, pw);
      strcat(systemcall, "\")%\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

      //__rootpass_crypt__
      strcpy(systemcall, "sed -ie \"s%__rootpass_crypt__%$(mkpasswd --hash=md5 \"");
      strcat(systemcall, rootpw);
      strcat(systemcall, "\")%\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

      //  change $ to \$
      strcpy(systemcall, "sed -ie 's%\\$%\\\\\\$%g' $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);

      //__swapchoices__
      //awk '/^\/dev\//{print $1}' /proc/swaps
      strcpy(systemcall, "sed -ie \"s#__swapchoices__#$(awk '/^\\/dev\\//{print $1}' /proc/swaps | head -1)#\" $HOME/");
      strcat(systemcall, FILENAME);
      //printf("%s\n", systemcall);
      system(systemcall);
}


void
on_button_install_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  /* ======================================================== *
   *                      read the widgets                    *
   * ======================================================== */
   GtkToggleButton *radiobutton;

   //password_check
   if( password_check(GTK_WIDGET (button)) < 1 ) {

           GtkWidget *notebook1 = lookup_widget (GTK_WIDGET (button), "notebook1");
           gtk_notebook_set_current_page( GTK_NOTEBOOK(notebook1), 3 );

           return;
   }

   // root partition check
   gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));
   if( hd_choice == NULL ) {
           GtkWidget *mainW, *dialog;

           // Message Dialog root partition empty
           mainW = lookup_widget (GTK_WIDGET (button), "window_main");
           dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s\n%s\n\n%s\n%s\n%s\n%s", "Rootpartition empty!", "Please create a linux partition", 
                                                  "Note: VMmware with SCSI virtual disc:",
                                                  "-------------------------------------",
                                                  "You must use \"Linux/Other Linux\",",
                                                  "	    NOT \"Other Linux 2.6x kernel\"");
           gtk_dialog_run (GTK_DIALOG (dialog));
           gtk_widget_destroy (dialog);

           return;
   }


  /* ======================================================== *
   *                   start the install_window               *
   * ======================================================== */
   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton1"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {


         // save config if not available
         save_config ( GTK_BUTTON (button) );

         // hide the main window
         GtkWidget *window_main = lookup_widget(GTK_WIDGET(button),"window_main");
         gtk_widget_hide ( window_main );

         // open install window
         GtkWidget *install_window = create_install_window ();
         gtk_widget_show (install_window);

   }


  /* ======================================================== *
   *                   save config file .sidconf              *
   * ======================================================== */
/*   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton2"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {

         save_config ( GTK_BUTTON (button) );

   }
*/

}


void
on_button_edit_configuration_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
  /* ======================================================== *
   *            edit config file .sidconf              *
   * ======================================================== */
    char syscall[BUF_LEN], fname[BUF_LEN];

    strncpy( fname, getenv("HOME"), BUF_LEN);
    strncat( fname, "/", BUF_LEN);
    strncat( fname, FILENAME, BUF_LEN);

/*
    // set the config filename
    // save config if not available
    FILE* fp=fopen(fname, "r");
    if( fp == NULL ) {
        save_config ( GTK_BUTTON (button) );
    }
    else {
         fclose(fp);
    }
*/

     strncpy( syscall, "#!/bin/bash\n", BUF_LEN);
     strncat( syscall, "EDITOR=$(which kwrite) || $(which gedit)", BUF_LEN);
     strncat( syscall, "\n$EDITOR ", BUF_LEN);
     strncat( syscall, fname, BUF_LEN);
     strncat( syscall, " 2> /dev/null", BUF_LEN);

     printf("edit config: %s\n", syscall);
     system( syscall);

}


void
on_button_install_now_back_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
    // close install window and show install-gui window (back button in install_window)
    // hide install window
    gtk_widget_show ( window_main );

    GtkWidget *install_window = lookup_widget(GTK_WIDGET(button),"install_window");
    gtk_widget_destroy ( install_window );

}


void
on_button_install_now_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    // hide install window
    GtkWidget *install_window = lookup_widget(GTK_WIDGET(button),"install_window");
    gtk_widget_hide ( install_window );

    // start install with progressbar
    GtkWidget* radiobutton = lookup_widget( GTK_WIDGET(button),"radiobutton_install_now1");
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {

          install_progressbar = create_install_progressbar ();
          gtk_widget_show (install_progressbar);
     }

     // start install with terminal
     else {
           system(INSTALL_SH_WITH_TERMINAL);

           // remove the tempfile
           unlink(scanparttmp);

           gtk_main_quit ();

     }


    // remove the tempfile
    unlink(scanparttmp);
}


void
on_notebook1_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{

   int password_failed;
   //int leaved_page;

   GtkWidget *notebook1;

   notebook1 = lookup_widget (GTK_WIDGET (notebook), "notebook1");

     // ==================================================================== 
     //          Hide the Next Button if Page Install is selected
     // ==================================================================== 

      GtkWidget *button_next = lookup_widget(GTK_WIDGET (notebook),"next");

      if( page_num < 5 )
          gtk_widget_show ( GTK_WIDGET (button_next) );
      else
          gtk_widget_hide ( GTK_WIDGET (button_next) );



      // password_check
      if(page_num == 3) {
            leaved_user_page = 1;
      }

      if(page_num != 3 && leaved_user_page == 1) {
          password_failed = password_check(GTK_WIDGET (notebook));
          leaved_user_page = 0;

          if( password_failed == 0) {
                 gtk_notebook_set_current_page( GTK_NOTEBOOK(notebook1), 3 );
          }
      }

}


void
on_radiobutton1_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   // Installation

  GtkWidget* label_install_button = lookup_widget( GTK_WIDGET(togglebutton), "label_install_button" );

  GtkWidget* radiobutton = lookup_widget( GTK_WIDGET(togglebutton),"radiobutton1");
  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
      gtk_label_set_text( GTK_LABEL ( label_install_button ), "Installation" );
  }
}


void
on_radiobutton2_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   // save .sidconf

  GtkWidget* label_install_button = lookup_widget( GTK_WIDGET(togglebutton), "label_install_button" );

  GtkWidget* radiobutton = lookup_widget( GTK_WIDGET(togglebutton),"radiobutton2");
  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
      gtk_label_set_text( GTK_LABEL ( label_install_button ), "Save" );
  }
}


void
on_radiobutton3_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   // Save and edit .sidconf
  GtkWidget* label_install_button = lookup_widget( GTK_WIDGET(togglebutton), "label_install_button" );

  GtkWidget* radiobutton = lookup_widget( GTK_WIDGET(togglebutton),"radiobutton3");
  if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
      gtk_label_set_text( GTK_LABEL ( label_install_button ), "Edit" );
  }
}


void
timezone_read (GtkWidget       *widget)
{
   FILE *tz_file;
   char tz[256], tz_markup[256];

   // read the timezone file
   tz_file = fopen( "/etc/timezone", "r" );
   if( tz_file == NULL ) {
       printf( "The file /etc/timezone was not opened\n");
   }
   else {
      fseek(  tz_file, 0L, SEEK_SET );
      fscanf( tz_file, "%[^\n]\n", tz);

      strncpy( tz_markup, "<span foreground=\"#A4A58B\" font_desc=\"Sans Bold 12\">", 256);
      strncat( tz_markup, tz, 256);
      strncat( tz_markup, "</span>", 256);

      GtkWidget *label_tz = lookup_widget (GTK_WIDGET (widget), "label_tz");
      gtk_label_set_markup( GTK_LABEL ( label_tz ), tz_markup );

      fclose( tz_file );
   }
}


void
on_button_tz_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
   // change timezone
   char tzsh[512];

   //hide the main window
   //GtkWidget *window_main = lookup_widget(GTK_WIDGET(button),"window_main");
   gtk_widget_hide ( GTK_WIDGET (window_main) );
   while (gtk_events_pending ())
          gtk_main_iteration ();


   // system call
   strncpy(tzsh, "#!/bin/bash\n", 512);
   strncat(tzsh, "close_me=0\n", 512);

   strncat(tzsh, "[ -n \"$(dpkg -l | grep libqt-perl)\" ] && close_me=1 && \
                       DEBIAN_FRONTEND=kde dpkg-reconfigure tzdata\n", 512);
   strncat(tzsh, "[ \"${close_me}\" = 1 ] && exit\n", 512);

   strncat(tzsh, "[ -n \"$(dpkg -l | grep libgnome2-perl)\" ] && close_me=1 && \
                       DEBIAN_FRONTEND=gnome dpkg-reconfigure tzdata\n", 512);
   strncat(tzsh, "[ \"${close_me}\" = 1 ] && exit\n", 512);

   strncat(tzsh, "x-terminal-emulator -e dpkg-reconfigure tzdata", 512);


   system(tzsh);


   timezone_read (GTK_WIDGET (button));


   //show the main window
   gtk_widget_show ( GTK_WIDGET (window_main) );
}


void
combobox_hd_read (GtkWidget       *widget)
{

   FILE* fp;

   char partition[80], hd_tmp[80];
   int fd;

   GtkWidget *combobox = lookup_widget (GTK_WIDGET (widget), "combobox_hd");

   strcpy(hd_tmp, "/tmp/harddisk.XXXXXX");
   fd = mkstemp(hd_tmp);  // make a tempfile

   if( fd ) {
            // create the shell system command
            strncpy(systemcallstr, HD_SCAN, BUF_LEN);
            strncat(systemcallstr, hd_tmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"======= harddisk call =======\n\";printf \"", BUF_LEN);
            strncat(systemcallstr, hd_tmp, BUF_LEN);
            strncat(systemcallstr, "\n\"; printf \"__________________________________\n\"; cat ", BUF_LEN);
            strncat(systemcallstr, hd_tmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"====================================\n\"", BUF_LEN);

            system(systemcallstr);  // write the harddisktable to the tempfile
            close(fd);
   }
   else  {
            perror("mkstemp(hd_tmp)");
   }

   // read the scanpartition temp file
   fp=fopen(hd_tmp, "r");
   if( fp == NULL ) {
       strncpy(partition, "tmp file error", BUF_LEN);
       gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), partition);
   }
   else {

       // append to combo_box
       fseek( fp, 0L, SEEK_SET );
       while (fscanf(fp, "%s", partition) != EOF) {

          //printf("%s %s\n", "combobox setzen, partition");
          gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), partition);
          gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),0);
       }
    }
    fclose(fp);
    unlink(hd_tmp);
}


void read_language(GtkComboBox     *combobox) 
{
  /* ======================================================= *
   * read the language codes from /etc/init.d/fll-locales    *
   * and put it in the combo box                             *
   * ======================================================= */

/*
   FILE* fp;

   char langcode[80], langtmp[80], *default_lang;
   int fd, z = 0, l = 0;


   strcpy(langtmp, "/tmp/languagetmp.XXXXXX");
   fd = mkstemp(langtmp);  // make a tempfile

   if( fd ) {
            // create the shell system command (fll_fshelper)
            strncpy(systemcallstr, LANG_SH, BUF_LEN);
            strncat(systemcallstr, langtmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"======= language call =======\n\";printf \"", BUF_LEN);
            strncat(systemcallstr, langtmp, BUF_LEN);
            strncat(systemcallstr, "\n\"; printf \"__________________________________\n\"; cat ", BUF_LEN);
            strncat(systemcallstr, langtmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"======= default language =======\n\";", BUF_LEN);
            strncat(systemcallstr, LANG_CUR, BUF_LEN);
            strncat(systemcallstr, langtmp, BUF_LEN);

            system(systemcallstr);  // write the partitiontable to the tempfile
            close(fd);
   }
   else  {
            perror("mkstemp(langtmp)");
   }


   // read the scanpartition temp file
   fp=fopen(langtmp, "r");
   if( fp == NULL ) {
       strcpy(langcode, "tmp file error");
       gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), langcode);
   }
   else {

       // clear the combo_box before
       GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
       gtk_list_store_clear(GTK_LIST_STORE(model));

       // appand to combo_box
       partitions_counter = 0;

       fseek( fp, 0L, SEEK_SET );
       while (fscanf(fp, "%[^\n]\n", langcode) != EOF) {

          //set default language
          if( strncmp ( langcode, "DEFAULT_", 8 ) == 0 ) {
              l=z;

              default_lang = strtok(langcode, "_");
              default_lang = strtok(NULL, "_");

              strncpy(lang_default, default_lang, 80);

              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), default_lang);
          }
          else {
              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), langcode);
          }

          z++;
      }

      gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), l);

      fclose(fp);

   }
*/
}


void
on_combobox_lang_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data)
{

     char *selected_lang = gtk_combo_box_get_active_text(GTK_COMBO_BOX ( combobox ));

     if ( strcmp(lang_default, selected_lang) != 0 ) {
            GtkWidget *dialog = create_dialog_keyb_change ();
            gtk_widget_show (dialog);

     }

}


void
on_okbutton2_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
          GtkWidget *dialog = lookup_widget (GTK_WIDGET (button), "dialog_keyb_change");
          gtk_widget_destroy (dialog);
}


void
on_button_usb_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	system("USBGUI=$(which install-usb-gui.bash); $USBGUI &");
}




void
on_window_main_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
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
   GtkListStore *comboList;
   GtkTreeIter iter;

   printf("\n========================\non_window_main_realize\n========================\n");


    window_main = lookup_widget (GTK_WIDGET (widget), "window_main");

   /* treeview, other Mountpoints */
   treeview1   = lookup_widget (GTK_WIDGET (widget), "treeview1");
   model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, GTK_TYPE_STRING);



   gtk_tree_view_set_model(GTK_TREE_VIEW(treeview1), GTK_TREE_MODEL (model));
   cell = gtk_cell_renderer_text_new();
   cell_editable = gtk_cell_renderer_combo_new ();

   comboList = gtk_list_store_new (1, G_TYPE_STRING);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/boot", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/home", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/opt", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/root", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/tmp", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/usr", -1);
   gtk_list_store_append (comboList, &iter);
   gtk_list_store_set (comboList, &iter, 0, "/var", -1);

   g_object_set(G_OBJECT(cell_editable), "model", comboList, NULL);
   g_object_set(G_OBJECT(cell_editable), "has-entry", FALSE, NULL);
   g_object_set(G_OBJECT(cell_editable), "editable", TRUE, NULL);
   g_object_set(G_OBJECT(cell_editable), "text-column", 0, NULL);
   g_object_set(G_OBJECT(cell_editable), "foreground", "red", NULL);
   g_object_set(G_OBJECT(cell_editable), "background", "gray", NULL);

   device     = gtk_tree_view_column_new_with_attributes("Device", cell, "text", 0, NULL);
   fs         = gtk_tree_view_column_new_with_attributes("FS", cell, "text", 1, NULL);
   mointpoint = gtk_tree_view_column_new_with_attributes ("Mountpoint", cell_editable, "text", 2, NULL);

   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), GTK_TREE_VIEW_COLUMN(device));
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), GTK_TREE_VIEW_COLUMN(fs));
   gtk_tree_view_append_column (GTK_TREE_VIEW(treeview1), mointpoint);

   // when the cell was edited store the new entry
   g_signal_connect (G_OBJECT(cell_editable), "edited",
		    G_CALLBACK(cell_edit_cb),
		    model);


  /* ============================================================ *
   *                   fill the root partition combo box          *
   * ============================================================ */
   rootpartcombo = lookup_widget (GTK_WIDGET (widget), "rootpartcombo");
   read_partitions( GTK_COMBO_BOX (rootpartcombo) );


  /* ============================================================ *
   *                   fill the language combobox                 *
   * ============================================================ */
   GtkWidget *combobox_lang = lookup_widget (GTK_WIDGET (widget), "combobox_lang");
   read_language( GTK_COMBO_BOX (combobox_lang) );


  /* ============================================================= *
   *                   fill the format_combo_box                   *
   * ============================================================= */

   GtkWidget *format_combo = lookup_widget (GTK_WIDGET (widget), "format_combo");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext3");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext2");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "reiserfs");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "jfs");

   gtk_combo_box_set_active( GTK_COMBO_BOX(format_combo),0);


  /* ============================================================= *
   *                   fill the combobox_bootmanager               *
   * ============================================================= */
   GtkWidget *combobox_bootmanager = lookup_widget (GTK_WIDGET (widget), "combobox_bootmanager");
   gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_bootmanager), "grub");
   //gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_bootmanager), "lilo");

   gtk_combo_box_set_active( GTK_COMBO_BOX(combobox_bootmanager),0);


  /* ============================================================= *
   *                      fill the label_tz                        *
   * ============================================================= */
   timezone_read (GTK_WIDGET (widget));


  /* ============================================================= *
   *             fill the combobox_hd  (harddisc)                  *
   * ============================================================= */
   combobox_hd_read (GTK_WIDGET (widget));


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

   label = lookup_widget ( GTK_WIDGET (widget), "checkbutton_metapackages");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "checkbutton_automount");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_rootpart_warning");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("18");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);
}


/*
void
on_window_main_show                    (GtkWidget       *widget,
                                        gpointer         user_data)
{

 if( do_it_at_first_time < 1 ) {

   do_it_at_first_time = 1;  // only at start


 }

}
*/



/**************************************************************************
***************************************************************************
*                            progress-bar                                 *
***************************************************************************
***************************************************************************/
static gboolean
f_notify(GIOChannel    *source, 
	 GIOCondition  condition,
	 gpointer      data)
{

   gchar column[BUF_LEN], text_current[BUF_LEN];
   char buf[BUF_LEN], *assign;
   int len, i;
   FILE *watched_file;


   while (gtk_events_pending ())
          gtk_main_iteration ();

   // stop neverending loop
   len = read (fd, buf, BUF_LEN);


   // read the changed file
   watched_file = fopen( FILE_NAME, "r" );
   if( watched_file== NULL ) {
      printf( "inotify watch file was not opened\n" );
   }
   else {
       i = 0;
       strncpy( text_current, "", BUF_LEN );
       fseek( watched_file, 0L, SEEK_SET );

       while ( fgets( column, BUF_LEN, watched_file )  != NULL ) {

             // exit main
             if( strncmp ( column, "end", 3 ) == 0 ) {

                  //hide the progressbar window
                  gtk_widget_hide ( install_progressbar );

                  // start dialog_end
                  GtkWidget* dialog_end = create_dialog_end  ();
                  gtk_widget_show ( dialog_end );

                  GtkWidget* label46 = lookup_widget( GTK_WIDGET ( dialog_end ), "label46" );
                  gtk_label_set_text ( GTK_LABEL( label46 ), progressclock );

             }


             assign = strtok( column, "=");

             if ( strcmp ( assign, "PERC") == 0 ) {
                 gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( pprogres ), strtod( strtok( NULL, "="), NULL) );
             }

             if ( strcmp ( assign, "CURRENT") == 0 ) {
                 gtk_label_set_markup ( GTK_LABEL ( label ), strtok( NULL, "=") );
             }

             if ( strcmp ( assign, "COMPLETE") == 0 ) {
                 gtk_progress_bar_set_text ( GTK_PROGRESS_BAR( pprogres ), strtok( NULL, "=") );
             }

       }


    }
    fclose( watched_file );

    //gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( pprogres2 ), 0 );

    return(TRUE);
}


gboolean up (gpointer user_data)
{
  /********************************************
   *        progresbar2 pulse                 *
   ********************************************/
   gtk_progress_bar_pulse  ( GTK_PROGRESS_BAR( pprogres2 ) );
   return(TRUE);
}


gboolean zeit (gpointer user_data)
{
  /********************************************
   *                time counter              *
   ********************************************/
   char min0[2] = "0", sec0[2] = "0";

   sec++;
   if ( sec == 60 ) {
        sec = 0;
        min++;
   }

   if ( sec < 10)
        strncpy( sec0, "0", 2);
   else
        strncpy( sec0, "", 2);


   if ( min == 60 ) {
        min = 0;
        hour++;
   }

   if ( min < 10)
        strncpy( min0, "0", 2);
   else
        strncpy( min0, "", 2);

   sprintf (progressclock, "0%d:%s%d:%s%d", hour, min0, min, sec0, sec);
   gtk_label_set_text ( GTK_LABEL( label_clock ), progressclock );


   return(TRUE);
}


gboolean
on_install_progressbar_delete_event    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  /* remove the tempfile */
  unlink(scanparttmp);

  gtk_main_quit();

  return FALSE;
}


void
on_install_progressbar_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
   GdkColor color;
   PangoFontDescription *font_desc;
   GIOChannel *ioc;
   int wd;
   char install_call[256];



   //  inotify tem file for fll-installer
   strcpy( install_call_tmp, "/tmp/INSTALL_INOTIFY.XXXXXX");
   fd = mkstemp( install_call_tmp );  // make a tempfile
   strncpy(FILE_NAME, install_call_tmp, 80);

   if( fd ) {
         close ( fd );
   }
   else {
         strncpy( install_call_tmp, "/tmp/fifo_inst", 80 );
         perror("mkstemp INSTALL_INOTIFY,");
   }


  /********************************************
   *           PROGRESS BAR PART              *
   ********************************************/
   label = lookup_widget ( GTK_WIDGET (widget), "label1");

   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label2");
   //set color of label
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);



   label = lookup_widget ( GTK_WIDGET (widget), "label_fifo");

   pprogres = lookup_widget(GTK_WIDGET(widget), "progressbar1");
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(pprogres), font_desc);
   pango_font_description_free (font_desc);
   // set color of ProgressBar
   gdk_color_parse ("gray70", &color);
   gtk_widget_modify_bg (pprogres, GTK_STATE_NORMAL, &color);
   //gdk_color_parse ("IndianRed4", &color);
   gdk_color_parse ("#A4A58B", &color);
   gtk_widget_modify_bg (pprogres, GTK_STATE_PRELIGHT, &color);

   pprogres2 = lookup_widget(GTK_WIDGET(widget), "progressbar2");
   gtk_progress_bar_set_pulse_step ( GTK_PROGRESS_BAR( pprogres2 ), 0.01 );

   // set color of ProgressBar
   gdk_color_parse ("gray70", &color);
   gtk_widget_modify_bg (pprogres2, GTK_STATE_NORMAL, &color);
   gdk_color_parse ("#A4A58B", &color);
   gtk_widget_modify_bg (pprogres2, GTK_STATE_PRELIGHT, &color);

   // label_clock
   label_clock = lookup_widget ( GTK_WIDGET (widget), "label_clock");
   font_desc = pango_font_description_from_string ("Bold 12");
   gtk_widget_modify_font ( GTK_WIDGET(label_clock), font_desc);
   pango_font_description_free (font_desc);


   //  Initialize, inotify!
   fd = inotify_init();
   if (fd < 0)  perror ("inotify_init");

   //  Adding Watches
   wd = inotify_add_watch (fd, FILE_NAME, IN_MODIFY | IN_CREATE );
   if (wd < 0)  perror ("inotify_add_watch");

   ioc=g_io_channel_unix_new(fd);
   g_io_add_watch(ioc,G_IO_IN,(GIOFunc) f_notify, NULL);


   while (gtk_events_pending ())
	  gtk_main_iteration ();


  /********************************************
   *        progresbar2 pulse                 *
   ********************************************/
   g_timeout_add( 20, up, pprogres2 );
   g_timeout_add( 1000, zeit, pprogres2 );



   // start fll-installer
   strncpy( install_call, INSTALL_SH, 256 );
   strncat( install_call, " ", 256 );
   strncat( install_call, install_call_tmp, 256 );
   strncat( install_call, " &", 256 );
   printf("installer call: %s\n", install_call);
   system( install_call );

}

