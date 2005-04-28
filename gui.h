
#ifndef __GUI_H__
#define __GUI_H__

#include <decoder.h>
#include <utils.h>

#include <gtk/gtk.h>

class Gui {
 public:
  Gui();
  ~Gui();
  
  bool init(vector<Decoder*> *devices);
  void start();
  
 private:
  
};

#endif
