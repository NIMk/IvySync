#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <decoder.h>
#include <utils.h>
#include <fakiir.h>

class Daemon {
 public:
  Daemon();
  ~Daemon();
  
  bool init(vector<Decoder*> *devices);
  void launch();
  void quit();


  /// fakiir external api
  int get_num_decoders();

  int play(int dec, char *path);
  int stop();

  float get_position(int dec);
  int set_position(float pos);
  ///

  
 private:
  vector<Decoder*> *decoders;
};

#endif
