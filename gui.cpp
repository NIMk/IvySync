#include <gtk/gtk.h>

GtkWidget *window;
GtkWidget *notebook;

int main(int argc, char **argv) {

  gtk_set_locale();
  gtk_init(&argc, &argv);

  add_pixmap_directory(PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "window");
  gtk_window_set_title (GTK_WINDOW (window), _("IvySync Graphical Interface"));

  notebook = gtk_notebook_new ();
  gtk_widget_set_name (notebook, "notebook");
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (window), notebook);
  
  
  playlist_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (playlist_box, "playlist_box");
  gtk_widget_show (playlist_box);
  gtk_container_add (GTK_CONTAINER (notebook), playlist_box);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow, "scrolledwindow");
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (playlist_box), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

  treeview = gtk_tree_view_new ();
  gtk_widget_set_name (treeview, "treeview");
  gtk_widget_show (treeview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);

  buttonbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (buttonbox, "buttonbox");
  gtk_widget_show (buttonbox);
  gtk_box_pack_start (GTK_BOX (playlist_box), buttonbox, FALSE, FALSE, 0);

  add_button = gtk_button_new_from_stock ("gtk-add");
  gtk_widget_set_name (add_button, "add_button");
  gtk_widget_show (add_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), add_button, FALSE, FALSE, 0);

  delete_button = gtk_button_new_from_stock ("gtk-delete");
  gtk_widget_set_name (delete_button, "delete_button");
  gtk_widget_show (delete_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), delete_button, FALSE, FALSE, 0);

  up_button = gtk_button_new ();
  gtk_widget_set_name (up_button, "up_button");
  gtk_widget_show (up_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), up_button, FALSE, FALSE, 0);

  image2 = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name (image2, "image2");
  gtk_widget_show (image2);
  gtk_container_add (GTK_CONTAINER (up_button), image2);

  down_button = gtk_button_new ();
  gtk_widget_set_name (down_button, "down_button");
  gtk_widget_show (down_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), down_button, FALSE, FALSE, 0);

  image3 = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name (image3, "image3");
  gtk_widget_show (image3);
  gtk_container_add (GTK_CONTAINER (down_button), image3);

  save_button = gtk_button_new_from_stock ("gtk-save");
  gtk_widget_set_name (save_button, "save_button");
  gtk_widget_show (save_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), save_button, TRUE, TRUE, 0);

  statusbar = gtk_statusbar_new ();
  gtk_widget_set_name (statusbar, "statusbar");
  gtk_widget_show (statusbar);
  gtk_box_pack_start (GTK_BOX (playlist_box), statusbar, FALSE, FALSE, 0);


