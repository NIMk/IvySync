

#include <stdio.h>

#include <stdlib.h>                    /* exit()        */
#include <unistd.h>                    /* close()       */
#include <errno.h>                     /* strerror()    */
#include <string.h>                    /* memset()      */

#include <utils.h>
#include <udpliteserver.h>

// UDP lite server
UdpLiteSrv *udpsrv;

bool synctest;

int main(int argc, char **argv) {
  
  udpsrv = new UdpLiteSrv();
  udpsrv->init(&synctest);
  //  udpsrv->launch();
  udpsrv->run();
  

  return 1;
}
