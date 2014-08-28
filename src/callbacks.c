#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "inotify.h"
#include "inotify-syscalls.h"

#define COLOR    "#455459"

#define BUF_LEN    1024

#define FILENAME ".sidconf"
#define TARGET_MNT_POINT "/fll/hdinstall"

//#define HD_SCAN                   "LANG=C fdisk -l | grep \"Disk /dev\" | cut -d: -f1 | cut -d\" \" -f2 > "
//#define HD_SCAN_NO_USB            "for hd in $( LANG=C fdisk -l | grep \"Disk /dev\" | cut -d: -f1 | cut -d\" \" -f2) ; do drive=$(echo $hd|cut -d / -f3); if readlink -f /sys/block/$drive/device |grep -q -v usb; then echo $hd; fi; done > "  // without usb devices

#define HD_SCAN                    "/usr/share/fll-installer-common/disk -d  > "
#define HD_SCAN_NO_USB             "/usr/share/fll-installer-common/disk -n  > "    // without usb devices
#define HD_SCAN_USB                "/usr/share/fll-installer-common/disk -u "
#define SCANPARTITIONS             "/usr/share/fll-installer-common/disk -p  > "

#define INSTALL_SH                ". /etc/default/distro; [ \"$FLL_DISTRO_MODE\" = live ] && fll-installer installer"
#define INSTALL_SH_WITH_TERMINAL  ". /etc/default/distro; [ \"$FLL_DISTRO_MODE\" = live ] && x-terminal-emulator --noclose -e fll-installer installer &"
#define INSTALL_FIRMWARE_BASH     "/usr/share/fll-installer-common/fw-install"

#define UM_SCRIPT_CHECK "/usr/share/fll-installer-common/um_all check"
#define UM_SCRIPT "/usr/share/fll-installer-common/um_all"
#define CHECK_EXISTING_HOME "/usr/share/fll-installer-common/check_existing_home"

// Abort message from backend
#define ABORT_MESSAGE             "Abort:"

#define LANG_SH "/etc/init.d/fll-locales list | sed 's|\t|, |' > "
//${LANG} is set in /etc/default/fll-locales
//#define LANG_CUR ". /etc/default/fll-locales; printf \"DEFAULT_LANG:${LANG}\n\";sed -ie \"s/^${LANG},/DEFAULT_${LANG},/\" "

#define HOSTNAME_ALLOWED_CHAR_0      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define HOSTNAME_ALLOWED_CHAR_OTHERS "0123456789-."
#define USERNAME_ALLOWED_CHAR        "abcdefghijklmnopqrstuvwxyz0123456789-."
#define NAME_NAME_NOT_ALLOWED_CHARS  "^°!\"§$%&/(){}[]=?`+*~#;:=,><|-_\\"


char scanparttmp[80], hd_tmp[80];
char systemcallstr[BUF_LEN];
char mountpoints_config[512];
char rootpw[21], rootpw_a[21], pw[21], pw_a[21], nname[80], uname[80], lang_default[80], progressclock[80], install_call_tmp[80];
int  counter, leaved_user_page, i = 0, partitions_counter = 0, hostname_ok = 1, longname_ok = 1, username_ok = 1;
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


// help function to escape special shell chars

char * escape_chars(char * pwd, char * new_str)
{
   char ch;
   int i, j=0;

   for ( i= 0; i< strlen(pwd); i++) {
      ch = pwd[i];
      if ( ch == '!'  || ch == '\"'  || ch == '$'   || 
	   ch == '%'  || ch == '&'   || ch == '{'   || 
	   ch == '('  || ch == '['   || ch == ')'   ||
	   ch == ']'  || ch == '='   || ch == '}'   || 
	   ch == '?'  || ch == '\\'  || ch == '*'   || 
	   ch == '+'  || ch == '~'   || ch == '\''  || 
	   ch == '#'  || ch == '>'   || ch == '<'   || ch == '`' ||
	   ch == '|'  || ch == ';'   || ch == '-'   || ch == ' '   
         ) {
	 new_str[j++] = '\\';
         new_str[j++] = ch;     
      }
      else {
         new_str[j++] = ch;
      }
   }
   new_str[j] = '\0';
   return (new_str);
}


void
combobox_hd_read (GtkWidget       *widget,
                  const gchar     *combobox_name)
{

   int fd;

   printf("--> in combobox_hd_read\n");

   strcpy(hd_tmp, "/tmp/harddisk.XXXXXX");
   fd = mkstemp(hd_tmp);  // make a tempfile

   if( fd ) {
            // create the shell system command
            if( strcmp(combobox_name, "combobox_hd") == 0 ) {  // devices with usbdevices (combobox_hd)
                  strncpy(systemcallstr, HD_SCAN, BUF_LEN);
            }
            else {   // devices without usbdevices (combobox_installplace)
                  strncpy(systemcallstr, HD_SCAN_NO_USB, BUF_LEN);
            }

            strncat(systemcallstr, hd_tmp, BUF_LEN);
            strncat(systemcallstr, "; printf \"======= harddisk call =======\n\";printf \"", BUF_LEN);
            strncat(systemcallstr, hd_tmp, BUF_LEN);
            strncat(systemcallstr, "\n\"; printf \"__________________________________\n\"; cat ", BUF_LEN);
            strncat(systemcallstr, hd_tmp, BUF_LEN);
            //strncat(systemcallstr, "; printf \"====================================\n\"", BUF_LEN);

            system(systemcallstr);  // write the harddisktable to the tempfile
            close(fd);
   }
   else  {
            perror("mkstemp(hd_tmp)");
   }

}


void
combobox_hd_set  (GtkWidget       *widget,
                  const gchar     *combobox_name)
{
   FILE* fp;

   char partition[BUF_LEN];

   GtkWidget *combobox = lookup_widget (GTK_WIDGET (widget), combobox_name);

   printf("--> in combobox_hd_set\n");

   // create the tempfile icludes name of devices  /dev/sda, /dev/sdb, ...
   combobox_hd_read (GTK_WIDGET (widget), combobox_name);

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

    //remove the tempfile
    unlink( hd_tmp );

}


void
is_the_device_a_usbdevice (GtkComboBox     *combobox)
{

 printf("--> in is_the_device_a_usbdevice\n");
 
 if ( partitions_counter > 0 ) {

  // is the selected install device a usb device, then only grub to partition
   char device[BUF_LEN], usbdevicetmp[BUF_LEN];
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
              strcpy(device, HD_SCAN_USB);
              strncat(device, entry2, 3);
              strcat(device, " > ");
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
          struct stat st;
          // this needs to check efisysdir present also
          if(stat("/sys/firmware/efi",&st)==0) {
              // efi dir present => efi install
              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "efi");
          }
          else {
              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "mbr");
              gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "partition");
              // add values from combobox hd (see xparted) to combobox_installplace
              combobox_hd_set  (GTK_WIDGET (combobox), "combobox_installplace");
          }
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

   char partition[BUF_LEN];
   char *ptr_dev, *ptr_fs;
   int fd;

   //printf("--> in read_partitions\n");

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
            strncat(systemcallstr, "; printf \"====================================\n\"", BUF_LEN);

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

          // example of partition  /dev/dm-0,ext3
          // example of partition  /dev/sda1,ext3
          ptr_dev = strtok(partition, ",");  // ptr_dev is /dev/sda1
          ptr_fs = strtok(NULL, ",");        // ptr_fs is ext3

          gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ptr_dev);
          //gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

          if( strncmp(ptr_fs, "reiser", 6) == 0 ||
              strncmp(ptr_fs, "ext", 3) == 0 ||
              strcmp(ptr_fs, "jfs") == 0 ) {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), partitions_counter);
          }
          partitions_counter++;
     }


     fclose(fp);

    /* ============================================================= *
     *                   fill the combobox_installplace              *
     * ============================================================= */
     GtkWidget *combobox_installplace = lookup_widget (GTK_WIDGET (combobox), "combobox_installplace");
     is_the_device_a_usbdevice ( GTK_COMBO_BOX (combobox_installplace));
     // add values from combobox hd (see xparted) to combobox_installplace
     //combobox_hd_set  (GTK_WIDGET (combobox_installplace), "combobox_installplace");
   }

}


void
on_entry_rootpw_changed                (GtkEditable     *editable,
                                        gpointer         user_data)
{

   GtkWidget* entry = lookup_widget ( GTK_WIDGET (window_main), "entry_rootpw");
   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_root_pw");
   GtkWidget* image_again = lookup_widget ( GTK_WIDGET (window_main), "image_rootpw_again");

   gtk_image_set_from_stock ( GTK_IMAGE(image_again), "gtk-cancel", GTK_ICON_SIZE_BUTTON);

   strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

   // Root Password too short
   if( strlen( rootpw ) < 6 ) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       // password cant start with '-' . mkpasswd limitation
	if( rootpw[0] == '-' ) {
	    gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
	}
	else {
	    gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
	}
//       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   }
}


void
on_entry_rootpw_again_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_rootpw_again");

   GtkWidget* entry = lookup_widget(GTK_WIDGET(window_main), "entry_rootpw");
   strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

   entry = lookup_widget(GTK_WIDGET(window_main), "entry_rootpw_again");
   strcpy(rootpw_a, gtk_entry_get_text(GTK_ENTRY(entry)));

   if( strcmp( rootpw, rootpw_a ) != 0 ) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   }
}


void
on_entry_realname_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
   char longname[BUF_LEN];

   GtkWidget* entry = lookup_widget ( GTK_WIDGET (window_main), "entry_realname");
   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_realname");

   strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

   /*   if( strlen( rootpw ) < 1 ) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   }
   */
   // longname empty?
   strncpy(longname, gtk_entry_get_text(GTK_ENTRY(entry)), BUF_LEN);
   if( strlen( longname ) < 1 ) {
       longname_ok = 0;
   }
   else {
       if (strpbrk(longname,NAME_NAME_NOT_ALLOWED_CHARS) == NULL) {
	 longname_ok = 1;
       }
       else 
	 longname_ok = 0;
   }

   if( longname_ok < 1) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);     
   }
}


void
on_entry_username_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
    char username[BUF_LEN];
    int  pos;

    GtkWidget* entry = lookup_widget ( GTK_WIDGET (window_main), "entry_username");
    GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_username");

    // username empty?
    strncpy(username, gtk_entry_get_text(GTK_ENTRY(entry)), BUF_LEN);


    if( strlen( username ) < 1 ) {
        username_ok = 0;
    }
    else {
        username_ok = 1;
    }


    // allowed char in Username
    if( username_ok > 0 ) {
        // check characters from usertname

        pos = strspn(username, USERNAME_ALLOWED_CHAR);
        if ( pos < strlen(username) ) {
            printf("Username ERROR, not allowed characters, use only a-z0-9.-\n");
            username_ok = 0;
        }
        else {
            username_ok = 1;
        }
    }

    // set image
    if( username_ok < 1 ) {
        gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
    }
    else {
        gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
    }
}


void
on_entry_pw_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget* entry = lookup_widget ( GTK_WIDGET (window_main), "entry_pw");
   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_pw");
   GtkWidget* image_again = lookup_widget ( GTK_WIDGET (window_main), "image_pw_again");

   gtk_image_set_from_stock ( GTK_IMAGE(image_again), "gtk-cancel", GTK_ICON_SIZE_BUTTON);

   strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));


   if( strlen( rootpw ) < 6 ) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       	if( rootpw[0] == '-' ) {
	    gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
	}
	else {
	    gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
	}
 //      gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   }
}


void
on_entry_pw_again_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_pw_again");

   GtkWidget* entry = lookup_widget(GTK_WIDGET(window_main), "entry_pw");
   strcpy(rootpw, gtk_entry_get_text(GTK_ENTRY(entry)));

   entry = lookup_widget(GTK_WIDGET(window_main), "entry_pw_again");
   strcpy(rootpw_a, gtk_entry_get_text(GTK_ENTRY(entry)));

   if( strcmp( rootpw, rootpw_a ) != 0 ) {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);
   }
   else {
       gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   }
}


void
on_hostname_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
   char hostname[BUF_LEN], hostname_allowed[BUF_LEN], hostname_first_char[1];
   int pos;

   GtkWidget* image = lookup_widget ( GTK_WIDGET (window_main), "image_hostname");
   GtkWidget* label_hostname_error = lookup_widget( GTK_WIDGET ( window_main ), "label_hostname_error" );

   GtkWidget* hostname_entry = lookup_widget ( GTK_WIDGET (window_main), "hostname");
   strcpy(hostname, gtk_entry_get_text(GTK_ENTRY(hostname_entry)));

   strncpy ( hostname_allowed, HOSTNAME_ALLOWED_CHAR_0, BUF_LEN);
   strncat ( hostname_allowed, HOSTNAME_ALLOWED_CHAR_OTHERS, BUF_LEN);


   // check 1. character from hostname
   strncpy ( hostname_first_char, hostname, 1);
   pos = strspn(hostname_first_char, HOSTNAME_ALLOWED_CHAR_0);
   if ( pos < 1 && strlen( hostname ) > 0 ) {
     gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);

     printf("Hostname Error position 1 !\n");
     gtk_label_set_markup ( GTK_LABEL( label_hostname_error ), "<span foreground=\"red\">ERROR: A-Za-z</span>" );

     hostname_ok = 0;
     return;
   }

   // check other characters from hostname
   pos = strspn(hostname, hostname_allowed);
   if ( pos < strlen(hostname) ) {
     gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);

     printf("Hostname ERROR position %d !\n", pos);
     gtk_label_set_markup ( GTK_LABEL( label_hostname_error ), "<span foreground=\"red\">ERROR: A-Za-z0-9.-</span>" );

     hostname_ok = 0;
     return;
   }

   // hostname empty?
   if ( strlen(hostname) < 1) {
     gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-cancel", GTK_ICON_SIZE_BUTTON);

     printf("Hostname empty !\n");
     gtk_label_set_markup ( GTK_LABEL( label_hostname_error ), "<span foreground=\"red\">ERROR: empty</span>" );

     hostname_ok = 0;
     return;
   }

   gtk_image_set_from_stock ( GTK_IMAGE(image), "gtk-apply", GTK_ICON_SIZE_BUTTON);
   gtk_label_set_text ( GTK_LABEL( label_hostname_error ), "" );

   hostname_ok = 1;
}


int
mount_check(GtkWidget     *button)
{
    // x-un-i's mount check ;-)
    GtkWidget *mainW, *dialog;
    gint rc, response;

    rc = system (UM_SCRIPT_CHECK);
    if (WIFEXITED(rc)) {

        if (WEXITSTATUS(rc) == 1) {    // partitions mounted
            printf ("mount_check exitcode: %d\n", WEXITSTATUS(rc));

            // Message Dialog root partition empty
            mainW = lookup_widget (GTK_WIDGET (button), "window_main");
            dialog = gtk_message_dialog_new( GTK_WINDOW( mainW ),
                    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
                    GTK_BUTTONS_YES_NO, "umount all partitions?");
            gtk_window_set_title(GTK_WINDOW(dialog), "Partition mounted");
            response = gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);

            if (response == GTK_RESPONSE_YES) {
                // user input -> yes, umount
                rc = system( UM_SCRIPT );
                if (WIFEXITED(rc)) {
                    if (WEXITSTATUS(rc) > 0) {
                        printf ("umount problem, exitcode: %d\n", WEXITSTATUS(rc));

                        // Message Dialog root partition empty
                        dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                            GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE, "%s\n", "device is busy, can't umount :-(");
                        gtk_dialog_run (GTK_DIALOG (dialog));
                        gtk_widget_destroy (dialog);

                        // skip install
                        return 0;
                    }
                }
            }
            else 
            {
                // user input -> no, skip install
                return 0;
            }
        }
    }

    // start install
    return 1;
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

    if( rootpw[0] == '-' ){
	mainW = lookup_widget (GTK_WIDGET (button), "window_main");
	dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("The root password is not allowed to start with a minus (-) due to some limitations of mkpasswd!"));
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
                                  "%s", gettext("Password too short!") );
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
          return 0;
      }
      
    if( pw[0] == '-' ){
	mainW = lookup_widget (GTK_WIDGET (button), "window_main");
	dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s", gettext("The password is not allowed to start with a minus (-) due to some limitations of mkpasswd!"));
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
   /* g_signal_connect ((gpointer) rootpartcombo, "changed",
                    G_CALLBACK (on_checkbutton_mountpoints_toggled),
                    NULL);  */

   // change also the  / (rootpartition) entry in the treeview, set the treeview new
   GtkWidget *toggle = lookup_widget ( GTK_WIDGET (combobox), "checkbutton_mountpoints");
   on_checkbutton_mountpoints_toggled (GTK_TOGGLE_BUTTON (toggle), NULL);

}


void
on_checkbutton_mountpoints_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   // GtkWidget *toggle;
   FILE* fp;

   GtkTreeIter iter_tb;

   char partition[80];
   char slash[80];
   char *ptr_dev, *ptr_fs;


   GtkWidget *toggle = lookup_widget ( GTK_WIDGET (togglebutton), "checkbutton_mountpoints");
   GtkWidget *treeview1 = lookup_widget (GTK_WIDGET (togglebutton), "treeview1");
   GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview1));

   gboolean enabled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));

   if (enabled == TRUE) {

        gtk_list_store_clear ( GTK_LIST_STORE (model) );  //make treeview empty


        // get the partitiontable from scanpart tempfile 
        fp=fopen(scanparttmp, "r");
        if( fp == NULL ) {
           strcpy(partition, "tmp file error");
        }
       else {
            gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (togglebutton), "rootpartcombo")));

            fseek( fp, 0L, SEEK_SET );
            while (fscanf(fp, "%s", partition) != EOF) {

                // Append a row and fill in some data
                gtk_list_store_append ( GTK_LIST_STORE (model), &iter_tb);

                // example of partition  /dev/sda1-ext3
                ptr_dev = strtok(partition, ",");  // ptr_dev is /dev/sda1
                ptr_fs = strtok(NULL, ",");        // ptr_fs is ext3

                if( hd_choice == NULL ) {
                    strcpy(slash, "");
                    printf("rootpartcombo = NULL\n");
                }
                else {
                    if( strcmp(ptr_dev, hd_choice) == 0 ) {
                        strcpy(slash, "/");
                    }
                    else {
                        strcpy(slash, "");
                    }
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
on_success_exit_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
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
                "<span foreground=\"red\" font_desc=\"Sans 12\">Root Partition changed !!!</span>" );

          i = 1;
    }
    else {
          gtk_label_set_text( GTK_LABEL ( label_changed ), "" );

          i = 0;
    }

    return(TRUE);
}


void
on_button_xparted_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    //FILE *stream;
    char sh_command[256];

    //start x-un-i mount_check
    if( mount_check(GTK_WIDGET (button)) < 1 ) {
            return;
    }


   // hide the main window after xparted has done
   //GtkWidget *window_main = lookup_widget(GTK_WIDGET(button),"window_main");
   gtk_widget_hide ( GTK_WIDGET (window_main) );
   while (gtk_events_pending ())
          gtk_main_iteration ();

   GtkWidget *combobox = lookup_widget (GTK_WIDGET (button), "combobox_hd");
   gchar *hd = gtk_combo_box_get_active_text(GTK_COMBO_BOX (combobox));


   // start prtition manager
   GtkToggleButton *radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton_part1"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {

       //start xparted
       if (access("/usr/bin/partitionmanager", X_OK) == 0) {
           system("partitionmanager");
       }
       else if ((hd != NULL) && (access("/usr/sbin/gparted", X_OK) == 0)) {
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
           strncpy(sh_command, "x-terminal-emulator -e /usr/share/fll-installer-common/cfdisk_wrapper cfdisk ", 256);
           strncat(sh_command, hd, 256);

           system(sh_command);
       }
       else if (access("/usr/bin/partitionmanager", X_OK) == 0) {
           system("partitionmanager");
       }
       else {
           system("gparted");
       }
   }

   radiobutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"radiobutton_part3"));
   if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {
       //start fdisk
       if (hd != NULL) {
           strncpy(sh_command, "x-terminal-emulator -e /usr/share/fll-installer-common/cfdisk_wrapper fdisk ", 256);
           strncat(sh_command, hd, 256);

           system(sh_command);
       }
       else if (access("/usr/bin/partitionmanager", X_OK) == 0) {
           system("partitionmanager");
       }
       else {
           system("gparted");
       }
   }

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
         strncat(services, "cups", 17);
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
"REGISTERED=' SYSTEM_MODULE HD_MODULE HD_FORMAT HD_FSTYPE HD_CHOICE HD_MAP HD_IGNORECHECK SWAP_MODULE SWAP_AUTODETECT SWAP_CHOICES NAME_MODULE NAME_NAME USER_MODULE USER_NAME USERPASS_MODULE USERPASS_CRYPT ROOTPASS_MODULE ROOTPASS_CRYPT HOST_MODULE HOST_NAME SERVICES_MODULE SERVICES_START BOOT_MODULE BOOT_LOADER BOOT_DISK BOOT_WHERE AUTOLOGIN_MODULE INSTALL_READY HD_AUTO'", 

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
"# Possible are: ext4|ext3|ext2|reiserfs|jfs",
"# Default value is: ext4",
"HD_FSTYPE='");
 
        gchar *hd_fstyp = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "format_combo")));
        fprintf( stream, "%s'\n%s\n%s\n%s", 
hd_fstyp,
"\n# Here the aptosid-System will be installed",
"# This value will be checked by function module_hd_check",
"HD_CHOICE='");

        fprintf( stream, "%s'\n%s\n", 
hd_choice,
"\n# Here you can give additional mappings. You need to have the partitions formatted yourself and give the correct mappings like: \"/dev/sda4:/boot /dev/sda5:/var /dev/sda6:/tmp\"");

        fprintf( stream, "%s\n%s\n%s\n%s\n%s", 
mountpoints_config,
"\n# If set to yes, the program will NOT check if there is enough space to install aptosid on the selected partition(s). Use at your own risk! Useful for example with HD_MAP if you only have a small root partition.",
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
      char * new_nname = (char *) malloc(2 * strlen((char *)nname) * sizeof(char));	 
      fprintf( stream, 
"\n%s\n%s\n%s\n%s\n%s\n%s\n\%s\n%s\n\n%s\n%s%s\n\n%s\n%s%s'\n\n%s\n%s\n\n%s\n%s\n\n\
%s\n%s%s'\n\n%s\n%s\n%s\n%s%s\n\n%s\n%s\n%s\n%s\n%s%s'\n%s\n%s\n%s\n%s", 

"SWAP_MODULE='configured'",
"# If set to yes, the swap partitions will be autodetected.",
"# Possible are: yes|no",
"# Default value is: yes",
"SWAP_AUTODETECT='yes'",

"\n# The swap partitions to be used by the installed aptosid.",
"# This value will be checked by function module_swap_check",
"SWAP_CHOICES='__swapchoices__'",
"NAME_MODULE='configured'",
"NAME_NAME=",
	       escape_chars(nname, new_nname),
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
"# Possible services are for now: cups smail ssh samba",
"# Default value is: cups",
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
      free (new_nname);
         checkbutton = GTK_TOGGLE_BUTTON(lookup_widget( GTK_WIDGET(button),"checkbutton_bootdisk"));
         if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON( checkbutton )) == TRUE ) {
             fprintf( stream, "yes'\n");
         }
         else {
             fprintf( stream, "no'\n");
         }

       fprintf( stream, "%s\n%s\n%s\n%s%s'\n\n%s\n%s\n\n", 
"\n# Where the Boot-Loader will be installed",
"# Possible are: mbr|partition|efi",
"# Default value is: mbr",
"BOOT_WHERE='",
gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "combobox_installplace"))),
"AUTOLOGIN_MODULE='configured'",
"INSTALL_READY='yes'"
         );


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


        fclose( stream );
      }


     /* ==================================================================== *
      * change the __.......__ entries in the .sidconf file with system calls *
      * ==================================================================== */

      //__userpass_crypt__
      strcpy(systemcall, "sed -i \"s%__userpass_crypt__%$(mkpasswd --method=sha-256 ");
      
      char *new_pw = (char *) malloc(2 * strlen((char *)pw) * sizeof(char));
      strcat(systemcall, escape_chars(pw, new_pw));
      free(new_pw);

      strcat(systemcall, ")%\" $HOME/");
      strcat(systemcall, FILENAME);
      // printf("%s\n", systemcall);
      system(systemcall);

      //__rootpass_crypt__
      strcpy(systemcall, "sed -i \"s%__rootpass_crypt__%$(mkpasswd --method=sha-256 ");

      char *new_rootpw = (char *) malloc(2 * strlen((char *)rootpw) * sizeof(char));
      strcat(systemcall, escape_chars(rootpw, new_rootpw));
      free (new_rootpw);

      strcat(systemcall, ")%\" $HOME/");
      strcat(systemcall, FILENAME);
      // printf("%s\n", systemcall);
      system(systemcall);

      //  change $ to \$
      strcpy(systemcall, "sed -i 's%\\$%\\\\\\$%g' $HOME/");
      strcat(systemcall, FILENAME);
      // printf("%s\n", systemcall);
      system(systemcall);

      //__swapchoices__
      //awk '/^\/dev\//{print $1}' /proc/swaps
      strcpy(systemcall, "sed -i \"s#__swapchoices__#$(awk '/^\\/dev\\//{print $1}' /proc/swaps | head -1)#\" $HOME/");
      strcat(systemcall, FILENAME);
      //("%s\n", systemcall);
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
   GtkWidget *mainW, *dialog;

   //start x-un-i mount_check
   if( mount_check(GTK_WIDGET (button)) < 1 ) {
           return;
   }

   //password_check
   if( password_check(GTK_WIDGET (button)) < 1 ) {

           GtkWidget *notebook1 = lookup_widget (GTK_WIDGET (button), "notebook1");
           gtk_notebook_set_current_page( GTK_NOTEBOOK(notebook1), 3 );

           return;
   }

   // hostname check
   if( hostname_ok < 1 ) {

           // Message Dialog root partition empty
           mainW = lookup_widget (GTK_WIDGET (button), "window_main");
           dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s\n", "Hostname wrong!");
           gtk_dialog_run (GTK_DIALOG (dialog));
           gtk_widget_destroy (dialog);

           return;
   }

   // username check
   if( username_ok < 1 ) {

           // Message Dialog root partition empty
           mainW = lookup_widget (GTK_WIDGET (button), "window_main");
           dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s\n", "Username wrong, use only a-z0-9.-");
           gtk_dialog_run (GTK_DIALOG (dialog));
           gtk_widget_destroy (dialog);

           return;
   }
   
    // real name check
    if( longname_ok < 1 ) {
       
	mainW = lookup_widget (GTK_WIDGET (button), "window_main");
	dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
					  GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_CLOSE,
					  "%s\n", "Real name wrong, do not use ^°!\"§$%&/(){}[]=?`+*~#;:=,><|-_\\");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	return;
    }

   // root partition check
   gchar *hd_choice = gtk_combo_box_get_active_text(GTK_COMBO_BOX (lookup_widget (GTK_WIDGET (button), "rootpartcombo")));
   if( hd_choice == NULL ) {

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
    
    // check if /home/username already exists on separate /home
    GtkWidget *treeview1 = lookup_widget (GTK_WIDGET (button), "treeview1");
    GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview1));
    GtkTreeIter iter;
    GValue mount_point = {0};
    GValue home_dev = {0};

    gboolean next = gtk_tree_model_get_iter_first(model, &iter);
    gboolean home_exists = FALSE;
    while( next ) {
	    gtk_tree_model_get_value(model, &iter, 2, &mount_point);
	    if( strcmp("/home", g_value_get_string(&mount_point)) == 0 ) {
		    gtk_tree_model_get_value(model, &iter, 0, &home_dev);
		    home_exists = TRUE;
		    break;
	    }
	    g_value_unset(&mount_point);
	    next = gtk_tree_model_iter_next(model, &iter);
    }
    

    
    if( home_exists ) {
	    char syscall[BUF_LEN];
	    char username[BUF_LEN];
	    GtkWidget* entry = lookup_widget(GTK_WIDGET(window_main), "entry_username");
	    strncpy(username, gtk_entry_get_text(GTK_ENTRY(entry)), BUF_LEN);

	    strncpy(syscall, CHECK_EXISTING_HOME, BUF_LEN);
	    strncat(syscall, " ", BUF_LEN);
	    strncat(syscall, g_value_get_string(&home_dev), BUF_LEN);
	    strncat(syscall, " ", BUF_LEN);
	    strncat(syscall, username, BUF_LEN);
	    
	    g_value_unset(&home_dev);
	    
	    int return_status = system(syscall);
	    if( ! WIFEXITED(return_status) )
		    return_status = 1;
	    else
		    return_status = WEXITSTATUS( return_status );
	    
	    switch( return_status ) {
		case 0:
		    break;
		    
		case 2:
		    mainW = lookup_widget(GTK_WIDGET(button), "window_main");
		    dialog = gtk_message_dialog_new(GTK_WINDOW(mainW),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_CLOSE,
					"%s%s%s\n\n%s\n",
					"There already is a directory named ",
					username,
					" on your /home.",
					"INFO: Mount /home later or choose another username.");
		    gtk_dialog_run(GTK_DIALOG(dialog));
		    gtk_widget_destroy(dialog);
		    return;

		default:
		    mainW = lookup_widget(GTK_WIDGET(button), "window_main");
		    dialog = gtk_message_dialog_new(GTK_WINDOW(mainW),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_CLOSE,
					"%s",
					"Error on mounting /home.");
		    gtk_dialog_run(GTK_DIALOG (dialog));
		    gtk_widget_destroy(dialog);
		    return;
	    } /* switch system(call) */
	    
    } /* if (home_exists) */
        

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
     strncat( syscall, "EDITOR=$(which kwrite)\n", BUF_LEN);
     strncat( syscall, "[ -z \"$EDITOR\" ] && EDITOR=$(which gedit)\n", BUF_LEN);
     strncat( syscall, "[ -z \"$EDITOR\" ] && EDITOR=$(which mousepad)\n", BUF_LEN);
     strncat( syscall, "[ -z \"$EDITOR\" ] && EDITOR=x-terminal-emulator -e vi \n", BUF_LEN);
     strncat( syscall, "$EDITOR ", BUF_LEN);
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

    //start x-un-i mount_check
    if( mount_check(GTK_WIDGET (button)) < 1 ) {
            return;
    }

    // hide install window
    GtkWidget *install_window = lookup_widget(GTK_WIDGET(button),"install_window");
    gtk_widget_hide ( install_window );

    // start install with progressbar
    //GtkWidget* radiobutton = lookup_widget( GTK_WIDGET(button),"radiobutton_install_now1");
    //if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)) ) {

    install_progressbar = create_install_progressbar ();
    gtk_widget_show (install_progressbar);
     //}

     // start install with terminal
     /*
     else {
           system(INSTALL_SH_WITH_TERMINAL);

           // remove the tempfile
           unlink(scanparttmp);

           gtk_main_quit ();

     }
     */

    // remove the tempfile
    unlink(scanparttmp);
}


void
on_notebook1_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{

   int pages;
   //int password_failed;
   //int leaved_page;

   GtkWidget *notebook1;

   notebook1 = lookup_widget (GTK_WIDGET (notebook), "notebook1");

     // ==================================================================== 
     //          Hide the Next Button if Page Install is selected
     // ==================================================================== 

      GtkWidget *button_next = lookup_widget(GTK_WIDGET (notebook),"next");

      pages = gtk_notebook_get_n_pages ( GTK_NOTEBOOK (notebook1));


      if( page_num < --pages )
          gtk_widget_show ( GTK_WIDGET (button_next) );
      else
          gtk_widget_hide ( GTK_WIDGET (button_next) );



      // password_check
/*      if(page_num == 3) {
            leaved_user_page = 1;
      }

      if(page_num != 3 && leaved_user_page == 1) {
          password_failed = password_check(GTK_WIDGET (notebook));
          leaved_user_page = 0;

          if( password_failed == 0) {
                 gtk_notebook_set_current_page( GTK_NOTEBOOK(notebook1), 3 );
          }
      }
*/
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

      strncpy( tz_markup, "<span foreground=\"", 256);
      strncat( tz_markup, COLOR, 256);
      strncat( tz_markup, "\" font_desc=\"12\">", 256);
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
on_button_installfw_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	system(INSTALL_FIRMWARE_BASH);
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


   // ############################################################################
   // #                            firmware needeed?                             #
   // ############################################################################
   if( strncmp( getenv("FLL_FIRMWARE"), "", BUF_LEN ) == 0 ) {  // NO firmware
	GtkWidget *notebook1 = lookup_widget (GTK_WIDGET (widget), "notebook1");
	gtk_notebook_remove_page( GTK_NOTEBOOK(notebook1), 5 );
   }
   else {
	GtkWidget* label_firmware = lookup_widget( GTK_WIDGET ( widget ), "label_firmware" );
	gtk_label_set_markup ( GTK_LABEL( label_firmware ), getenv("FLL_FIRMWARE") );

	// installable firmware?
	
	if (!getenv("FLL_FIRMWARE_INSTALL")) {
		fprintf(stderr, "install-gui.bash: FLL_FIRMWARE_INSTALL isn't defined\n");
	}
	else if( strlen(getenv("FLL_FIRMWARE_INSTALL")) < 1 ) { 
		GtkWidget *button_installfw = lookup_widget(GTK_WIDGET (widget),"button_installfw");
		GtkWidget *label_fw = lookup_widget(GTK_WIDGET (widget),"label_fw");
		gtk_widget_hide ( GTK_WIDGET (button_installfw) );
		gtk_widget_hide ( GTK_WIDGET (label_fw) );
	}
   }


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

  /* ============================================================= *
   *             fill the combobox_hd  (harddisc)                  *
   * ============================================================= */
   combobox_hd_set  (GTK_WIDGET (widget), "combobox_hd");


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
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext4");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext3");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "ext2");
   gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "reiserfs");
   //gtk_combo_box_append_text (GTK_COMBO_BOX (format_combo), "jfs");

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


   // set the treeview on start
   GtkWidget *toggle = lookup_widget ( GTK_WIDGET (widget), "checkbutton_mountpoints");
   on_checkbutton_mountpoints_toggled (GTK_TOGGLE_BUTTON (toggle), NULL);



  /* ============================================================= *
   *           Label sets, font, color, etc.                       *
   * ============================================================= */
   label = lookup_widget (GTK_WIDGET (widget), "label_wellcome");

   font_desc = pango_font_description_from_string ("16");

   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_install");
   gdk_color_parse (COLOR, &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("12");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_red");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("10");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_wellcome_2");
   font_desc = pango_font_description_from_string ("10");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "checkbutton_metapackages");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("10");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "checkbutton_automount");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("10");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);

   label = lookup_widget ( GTK_WIDGET (widget), "label_rootpart_warning");
   gdk_color_parse ("red", &color);
   gtk_widget_modify_fg ( GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
   font_desc = pango_font_description_from_string ("14");
   gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   pango_font_description_free (font_desc);
}


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
   GtkWidget *mainW, *dialog;


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

             // Abort from backend
             if( strncmp ( column, ABORT_MESSAGE, 6 ) == 0 ) {

                  //hide the progressbar window
                  gtk_widget_hide ( install_progressbar );

                  // start dialog_abort
                  mainW = lookup_widget (GTK_WIDGET (pprogres), "window_main");
                  dialog = gtk_message_dialog_new ( GTK_WINDOW( mainW ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "%s\n", column);
                  gtk_dialog_run (GTK_DIALOG (dialog));
                  gtk_widget_destroy (dialog);

                  gtk_main_quit();

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
   //label = lookup_widget ( GTK_WIDGET (widget), "label1");
   //font_desc = pango_font_description_from_string ("Bold");
   //gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   //pango_font_description_free (font_desc);

   //label = lookup_widget ( GTK_WIDGET (widget), "label2");
   //set color of label
   //font_desc = pango_font_description_from_string ("Bold");
   //gtk_widget_modify_font ( GTK_WIDGET(label), font_desc);
   //pango_font_description_free (font_desc);



   label = lookup_widget ( GTK_WIDGET (widget), "label_fifo");

   pprogres = lookup_widget(GTK_WIDGET(widget), "progressbar1");
   font_desc = pango_font_description_from_string ("Bold");
   gtk_widget_modify_font ( GTK_WIDGET(pprogres), font_desc);
   pango_font_description_free (font_desc);
   // set color of ProgressBar
   gdk_color_parse ("gray70", &color);
   gtk_widget_modify_bg (pprogres, GTK_STATE_NORMAL, &color);
   //gdk_color_parse ("IndianRed4", &color);
   gdk_color_parse (COLOR, &color);
   gtk_widget_modify_bg (pprogres, GTK_STATE_PRELIGHT, &color);

   pprogres2 = lookup_widget(GTK_WIDGET(widget), "progressbar2");
   gtk_progress_bar_set_pulse_step ( GTK_PROGRESS_BAR( pprogres2 ), 0.01 );

   // set color of ProgressBar
   gdk_color_parse ("gray70", &color);
   gtk_widget_modify_bg (pprogres2, GTK_STATE_NORMAL, &color);
   gdk_color_parse (COLOR, &color);
   gtk_widget_modify_bg (pprogres2, GTK_STATE_PRELIGHT, &color);

   // label_clock
   label_clock = lookup_widget ( GTK_WIDGET (widget), "label_clock");
   font_desc = pango_font_description_from_string ("Bold");
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



