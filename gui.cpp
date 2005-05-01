
#include <iostream>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gui.h>
#include <utils.h>


#define PACKAGE_DATA_DIR "/usr/share/ivysync"
#define CONFIGFILE "/home/jaromil/ivysyncrc"

static GtkWindow *main_window;

// playlist model columns
enum {
  POSITION,
  FILENAME,
  COLUMNS
};

/* DND stuff ripped from wolfpack.twu.net/docs/gtkdnd */
#define DRAG_TAR_NAME_0		"text/plain"
#define DRAG_TAR_INFO_0		0

#define DRAG_TAR_NAME_1		"text/uri-list"		/* not url-list */
#define DRAG_TAR_INFO_1		1

#define DRAG_TAR_NAME_2		"STRING"
#define DRAG_TAR_INFO_2		2
/* end */


// CALLBACKS

void on_add_button(GtkWidget *widget, gpointer *data) {
  GtkWidget *dialog;
  Playlist *pl = (Playlist*)data;

  dialog = gtk_file_chooser_dialog_new ("Open File", main_window,
					GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
					GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_select_multiple((GtkFileChooser*)dialog, true);
  
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    GSList *filenames;
    filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));
    while(filenames) {
      D("file choosen: %s",filenames->data);
      pl->decoder->append((char*)filenames->data);
      g_free(filenames->data);
      filenames = filenames->next;
    }
    g_slist_free(filenames);
  }

  gtk_widget_destroy(dialog);
  pl->refresh();
}

/*
void on_playlist_select(GtkTreeSelection *sel, gpointer data) {
  GtkTreeIter treeiter;
  GtkTreeModel *model;
  int *selection;
  Playlist *pl = (Playlist*)data;

  if(gtk_tree_selection_get_selected(sel,&model,&treeiter)) {
    gtk_tree_model_get(model,&treeiter,POSITION,&selection,-1);
    D("selected file %u",*selection);
    pl->selected = *selection;
    pl->refresh();
  }
}
*/

void on_save_button(GtkWidget *widget, gpointer *data) {
  Playlist *pl = (Playlist*)data;
  pl->decoder->save();
}

void on_play_button(GtkWidget *widget, gpointer *data) {
  Playlist *pl = (Playlist*)data;
  if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    if(!pl->decoder->running) pl->decoder->launch();
    pl->decoder->play();
  } else {
    pl->decoder->stop();
  }
}

gboolean DND_data_motion(GtkWidget *w, GdkDragContext *dc, gint x, gint y,
				guint t, struct gchan *o) {
  gdk_drag_status(dc, GDK_ACTION_MOVE, t);
  D("drag_data_motion");
  return FALSE;
}

void DND_data_received(GtkWidget *w, GdkDragContext *dc, gint x, gint y,
		       GtkSelectionData *selection, guint info, guint t,
		       Playlist *pl) {
  GtkTreeIter iter, itersrc;
  GtkTreeModel *model, *modelsrc;
  GtkTreePath *path;
  GtkTreeSelection *selectsrc;
  GtkWidget *source;
  gint row=0, rowsrc=0;
  gchar *title;
  GList *rowlist, *titlelist;
  
  
  /* <federico> nightolo: if gtk_drag_get_source_widget(drag_context) returns NULL,
   * it comes from another process
   * tnx to #gtk+ :) 
   */
  D("DND_data_received");	
  if( !(source = gtk_drag_get_source_widget(dc)) ) {
    E("DND_data_received error: no source");
    return;
  }
  D("source = %p info %d", source, info);
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(w));
  modelsrc = gtk_tree_view_get_model(GTK_TREE_VIEW(source));
  selectsrc = gtk_tree_view_get_selection(GTK_TREE_VIEW(source));
  rowlist = gtk_tree_selection_get_selected_rows(selectsrc, &modelsrc);
  titlelist = NULL;
  
  rowlist = g_list_first(rowlist);
  if(rowlist->data) {
    rowsrc = gtk_tree_path_get_indices((GtkTreePath *)rowlist->data)[0];
    while(rowlist && rowlist->data) {
      gtk_tree_model_get_iter(modelsrc, &itersrc, (GtkTreePath *)rowlist->data);
      gtk_tree_model_get(modelsrc, &itersrc, FILENAME, &title, -1);
      
      titlelist = g_list_append(titlelist, (void *) title);
      rowlist = g_list_next(rowlist);
      //		gtk_tree_path_free((GtkTreePath *)rowlist->data);
      
    }
    D("DND_data_received I part -> ok");
  } else
    return;
  
  
  if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(w),
				   x, y, &path, NULL, NULL, NULL)) {
    row = gtk_tree_path_get_indices(path)[0];
    D("DND_data_received row = %d", row);
    gtk_tree_path_free(path);
  }
  
  /*	if(!source && (info == DRAG_TAR_INFO_1 || info == DRAG_TAR_INFO_0)) {
	func("I got text/uri");
	mixer->add_to_playlist(o->idx-1, title);
	} else {*/
  
  rowlist = g_list_first(rowlist);
  for(; titlelist != NULL; row++ && rowsrc++) {
    D("riga %d", row);
    gtk_list_store_insert(GTK_LIST_STORE(model), &iter, row);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
		       FILENAME, titlelist->data, -1);
    
    // insert
    pl->decoder->insert((char*)titlelist->data,row+1);
    // delete old
    pl->decoder->remove(rowsrc+1);
    
    titlelist = g_list_next(titlelist);		
  }
  //}
  pl->refresh();
  D("drag_data_received");
}

void DND_data_delete(GtkWidget *w, GdkDragContext *dc, Playlist *pl) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select;
	GtkTreePath *path;
	GtkTreeRowReference *ref;
	gint row;
	GList *pathlist, *reflist;
	
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
	pathlist = gtk_tree_selection_get_selected_rows(select, &model);
	reflist = NULL;
	
	while(pathlist) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath *)pathlist->data);
		reflist = g_list_append(reflist, (void *) ref);
		pathlist = g_list_next(pathlist);
	}
	reflist = g_list_first(reflist);
	while(reflist) {
		path = gtk_tree_row_reference_get_path((GtkTreeRowReference *)reflist->data);
		row = gtk_tree_path_get_indices(path)[0];
		if(gtk_tree_model_get_iter(model, &iter, path)) {
		  gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
		}
		gtk_tree_path_free(path);
		reflist = g_list_next(reflist);
		
	}
	
	D("drag_data_delete");
}




////////////////// PLAYLIST WIDGET CLASS

Playlist::Playlist(int num) {

  selected = 0;

  widget = gtk_vbox_new (FALSE, 0);
  snprintf(widget_name,255,"widget_%u",num);
  gtk_widget_set_name (widget, widget_name);
  gtk_widget_show (widget);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  snprintf(scrolledwindow_name,255,"scrolledwindow_%u",num);
  gtk_widget_set_name (scrolledwindow, scrolledwindow_name);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (widget), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

  treeview = gtk_tree_view_new ();
  snprintf(treeview_name,255,"treeview_%u",num);
  gtk_widget_set_name (treeview, treeview_name);
  gtk_widget_show (treeview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);

  buttonbox = gtk_hbox_new (FALSE, 0);
  snprintf(buttonbox_name,255,"buttonbox_%u",num);
  gtk_widget_set_name (buttonbox, buttonbox_name);
  gtk_widget_show (buttonbox);
  gtk_box_pack_start (GTK_BOX (widget), buttonbox, FALSE, FALSE, 0);

  add_button = gtk_button_new_from_stock ("gtk-add");
  snprintf(add_button_name,255,"add_button_%u",num);
  gtk_widget_set_name (add_button, add_button_name);
  gtk_widget_show (add_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), add_button, FALSE, FALSE, 0);
  g_signal_connect((gpointer)add_button, "pressed", G_CALLBACK(on_add_button), this);

  delete_button = gtk_button_new_from_stock ("gtk-delete");
  snprintf(delete_button_name,255,"delete_button_%u",num);
  gtk_widget_set_name (delete_button, delete_button_name);
  gtk_widget_show (delete_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), delete_button, FALSE, FALSE, 0);

  up_button = gtk_button_new ();
  snprintf(up_button_name,255,"up_button_%u",num);
  gtk_widget_set_name (up_button, up_button_name);
  gtk_widget_show (up_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), up_button, FALSE, FALSE, 0);

  image2 = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
  snprintf(image2_name,255,"image2_%u",num);
  gtk_widget_set_name (image2, image2_name);
  gtk_widget_show (image2);
  gtk_container_add (GTK_CONTAINER (up_button), image2);

  down_button = gtk_button_new ();
  snprintf(down_button_name,255,"down_button_%u",num);
  gtk_widget_set_name (down_button, down_button_name);
  gtk_widget_show (down_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), down_button, FALSE, FALSE, 0);

  image3 = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
  snprintf(image3_name,255,"image3_%u",num);
  gtk_widget_set_name (image3, image3_name);
  gtk_widget_show (image3);
  gtk_container_add (GTK_CONTAINER (down_button), image3);

  play_button = gtk_toggle_button_new_with_mnemonic ("gtk-media-play");
  gtk_button_set_use_stock (GTK_BUTTON (play_button), TRUE);
  snprintf(play_button_name,255,"play_button_%u",num);
  gtk_widget_set_name (play_button, play_button_name);
  gtk_widget_show (play_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), play_button, FALSE, FALSE, 0);
  g_signal_connect((gpointer)play_button, "pressed", G_CALLBACK(on_play_button), this);

  save_button = gtk_button_new_from_stock ("gtk-save");
  snprintf(save_button_name,255,"save_button_%u",num);
  gtk_widget_set_name (save_button, save_button_name);
  gtk_widget_show (save_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), save_button, TRUE, TRUE, 0);
  g_signal_connect((gpointer)save_button, "pressed", G_CALLBACK(on_save_button), this);
  
  statusbar = gtk_statusbar_new ();
  snprintf(statusbar_name,255,"statusbar_%u",num);
  gtk_widget_set_name (statusbar, statusbar_name);
  gtk_widget_show (statusbar);
  gtk_box_pack_start (GTK_BOX (widget), statusbar, FALSE, FALSE, 0);

  // now setup the model view for the treeview
  treestore = gtk_tree_store_new(COLUMNS,
				 G_TYPE_STRING, // position
				 G_TYPE_STRING); // name
  gtk_tree_view_set_model((GtkTreeView*)treeview,GTK_TREE_MODEL(treestore));
  
  { // then the selection handling
    GtkTreeSelection *treeselect;
    treeselect = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    gtk_tree_selection_set_mode(treeselect, GTK_SELECTION_MULTIPLE);
    //    g_signal_connect(G_OBJECT(treeselect), "changed",
    //    		     G_CALLBACK(on_playlist_select), this);
  }

  
  { // then the drag and drop stuff
    GtkTargetEntry target_entry[3];
    target_entry[0].target = DRAG_TAR_NAME_0;
    target_entry[0].flags = 0;
    target_entry[0].info = DRAG_TAR_INFO_0;
    
    target_entry[1].target = DRAG_TAR_NAME_1;
    target_entry[1].flags = 0;
    target_entry[1].info = DRAG_TAR_INFO_1;
    
    target_entry[2].target = DRAG_TAR_NAME_2;
    target_entry[2].flags = 0;
    target_entry[2].info = DRAG_TAR_INFO_2;
    gtk_drag_dest_set
      (treeview,
       (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP),
       target_entry,sizeof(target_entry) / sizeof(GtkTargetEntry),
       GDK_ACTION_MOVE);

    gtk_drag_source_set
      (treeview,
       (GdkModifierType) (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK),
       target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry),
       GDK_ACTION_MOVE);
    g_signal_connect(G_OBJECT(treeview), "drag_motion",
		     G_CALLBACK(DND_data_motion), this);
    
    //    g_signal_connect(G_OBJECT(tree), "drag_begin",
    //		     G_CALLBACK(DND_begin), this);

    //	g_signal_connect(G_OBJECT(tree), "drag_end",
    //			G_CALLBACK(DND_end), this);

    //	g_signal_connect(G_OBJECT(tree), "drag_data_get",
    //			G_CALLBACK(DND_data_get), this);

    g_signal_connect(G_OBJECT(treeview), "drag_data_received",
		     G_CALLBACK(DND_data_received), this);

    g_signal_connect(G_OBJECT(treeview), "drag_data_delete",
		     G_CALLBACK(DND_data_delete), this);
  }

  { // and finally the cell rendering
    GtkCellRenderer *rend;
    GtkTreeViewColumn *col;
    rend = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(rend),"background","orange",NULL);
    col = gtk_tree_view_column_new_with_attributes
      ("Pos",rend,"text",POSITION,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

    rend = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes
      ("Filename",rend,"text",FILENAME,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);
  }
}  

Playlist::~Playlist() {
  gtk_widget_destroy(statusbar);
  gtk_widget_destroy(save_button);
  gtk_widget_destroy(image3);
  gtk_widget_destroy(down_button);
  gtk_widget_destroy(image2);
  gtk_widget_destroy(up_button);
  gtk_widget_destroy(delete_button);
  gtk_widget_destroy(add_button);
  gtk_widget_destroy(buttonbox);
  gtk_widget_destroy(treeview);
  gtk_widget_destroy(scrolledwindow);
  
  gtk_widget_destroy(widget);
  
}

// read again the playlist from the decoder and fill it up
int Playlist::refresh() {
  GtkTreeIter iter; // static
  vector<string>::iterator pl_iter;
  string pl;
  char tmp[16];
  int c;

  gtk_tree_store_clear(treestore);

  for(c=1, pl_iter = decoder->playlist.begin();
      pl_iter != decoder->playlist.end();
      ++pl_iter, c++) {

    pl = *pl_iter;

    gtk_tree_store_append(treestore,&iter,NULL);
    
    snprintf(tmp,15,"%s%u",
	     ((decoder->position-1)==c)?"->":"  ",
	     c);
    gtk_tree_store_set(treestore,&iter,
		       POSITION, tmp,
		       FILENAME, pl.c_str(),
		       -1);

  }

  return 1;
}

/////////////////////////////////////////////////////////////////




Gui::Gui() {

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "window");
  gtk_widget_set_size_request (window, 400, 300);
  gtk_window_set_title (GTK_WINDOW (window), "IvySync Graphical Interface");
  main_window = (GtkWindow*)window; // global static pointer

  notebook = gtk_notebook_new ();
  gtk_widget_set_name (notebook, "notebook");
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (window), notebook);

  // QUAA
  syncstart = true;
}

Gui::~Gui() {
  gtk_widget_destroy(notebook);
  gtk_widget_destroy(window);
}

bool Gui::init(vector<Decoder*> *devices) {
  vector<Decoder*>::iterator dev_iter;
  Decoder *dec;
  Playlist *pl;
  int c;

  for(c=1, dev_iter = devices->begin();
      dev_iter != devices->end();
      ++dev_iter, c++) {

    dec = *dev_iter;
    dec->syncstart = &syncstart;

    pl = new Playlist(c);

    pl->decoder = dec; // store the decoder pointer in the playlist
    pl->refresh(); // refresh with the filenames
    playlist.push_back(pl); // store the playlist in the gui array
    
    gtk_container_add(GTK_CONTAINER(notebook), pl->widget);
    char tmp[256];
    snprintf(tmp,255,"/dev/video%u",dec->device_num);
    gtk_notebook_set_tab_label_text ((GtkNotebook*)notebook,
				     gtk_notebook_get_nth_page((GtkNotebook*)notebook, c-1), tmp);

  }
  return true;
}
void Gui::status(char *format, ...) {
  // TODO status bar messages
  return;
}



void Gui::start() {
  g_signal_connect((gpointer)window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show(window);
  gtk_main();
}
