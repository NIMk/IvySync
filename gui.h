
#ifndef __GUI_H__
#define __GUI_H__

#include <decoder.h>
#include <utils.h>

#include <gtk/gtk.h>

class Playlist {
 public:
  Playlist(int num);
  ~Playlist();

  int refresh();

  GtkWidget *widget;
  
  Decoder *decoder;

  int selected;

 private:
  // rendered widget names
  char widget_name[256];
  char scrolledwindow_name[256];
  char treeview_name[256];
  char buttonbox_name[256];
  char add_button_name[256];
  char delete_button_name[256];
  char up_button_name[256];
  char down_button_name[256];
  char play_button_name[256];
  char image2_name[256];
  char image3_name[256];
  char save_button_name[256];
  char statusbar_name[256];
  // widget pointers
  GtkWidget *treeview;
  GtkTreeStore *treestore;


  GtkWidget *scrolledwindow;
  GtkWidget *buttonbox;
  GtkWidget *add_button;
  GtkWidget *delete_button;
  GtkWidget *up_button;
  GtkWidget *image2;
  GtkWidget *down_button;
  GtkWidget *play_button;
  GtkWidget *image3;
  GtkWidget *save_button;
  GtkWidget *statusbar;
  
};

class Gui {
 public:
  Gui();
  ~Gui();
  
  bool init(vector<Decoder*> *devices);
  void start();

  bool syncstart;

  vector<Playlist*> playlist;

 private:
  void status(char *format, ...);
  GtkWidget *window;
  GtkWidget *notebook;
  
};

#endif
