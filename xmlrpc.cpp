/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004 - 2006 Denis Rojo <jaromil@dyne.org>
 *                     Nederlands Instituut voor Mediakunst
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <decoder.h>
#include <xmlrpc.h>
#include <utils.h>

static bool quit;

IvySyncDaemon::IvySyncDaemon(XmlRpcServer *srv) {

  xmlrpc = srv;
  xmlrpc->_ssl = false;
  xmlrpc->_ssl_ssl = NULL;

  quit = false;

}

bool IvySyncDaemon::init(int port) {
  // guess where this number comes from ? ;)
  if( ! xmlrpc->bindAndListen(port) ) return false;
  
  // let's be introspective so we list our own methods
  xmlrpc->enableIntrospection(true);
  
  return true;
}

void IvySyncDaemon::run(double mstime) {
  //  running = true;
  //  unlock();

  if(::quit) {
    quit = true;
    return;
  }

  // run for amount of milliseconds (-1.0 for infinite)
  xmlrpc->work( mstime );

}


Play::Play(XmlRpcServer* srv, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("Play", srv),
    IvySyncPublicMethod(decoders)
{ }

Stop::Stop(XmlRpcServer* srv, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("Stop", srv),
    IvySyncPublicMethod(decoders)
{ }

Pause::Pause(XmlRpcServer* srv, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("Pause", srv),
    IvySyncPublicMethod(decoders)
{ }

GetPos::GetPos(XmlRpcServer* srv, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("GetPos", srv),
    IvySyncPublicMethod(decoders)
{ }

SetPos::SetPos(XmlRpcServer* srv, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("SetPos", srv),
    IvySyncPublicMethod(decoders)
{ }

Open::Open(XmlRpcServer* src, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("Open", src),
    IvySyncPublicMethod(decoders)
{ }

Quit::Quit(XmlRpcServer* src, vector<Decoder*> *decoders)
  : XmlRpcServerMethod("Quit", src),
    IvySyncPublicMethod(decoders)
{ }

void Quit::execute(XmlRpcValue &params, XmlRpcValue &result) {
  Decoder *dec;
  vector<Decoder*>::iterator iter;

  for(iter = decoders->begin();
      iter != decoders->end();
      ++iter) {
    
    dec = *iter;
    dec->stop();
    dec->close();

  }

  result = 1.0;

  ::quit = true;
}
  
void Open::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;
  char *path;

  if( params.size() < 2) {
    E("XMLRPC: Open called with invalid number of arguments(%u)",
      params.size() );
    result = 0.0;
    return;
  }
  
  // get out the decoder parameter
  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }
  
  // get out the path to the file to be opened
  path = (char*) (std::string(params[1])).c_str();
  D("XMLRPC: Open decoder %u file %s", decnum+1, path);

  FILE *fd;
  fd = fopen(path, "r");
  if(!fd) {
    result = 0.0;
    return;
  } else fclose(fd);
  
  dec->empty();
  dec->append(path);
  result = 1.0;

}

void Play::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;

  if( params.size() < 1) {
    E("XMLRPC: Play called with invalid number of arguments (%u)",
      params.size() );
    result = 0.0;
    return;
  }

  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }

  D("XMLRPC: Play decoder %u", decnum+1 );
  result = (double) dec->play();
}



void Stop::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;

  if( params.size() < 1) {
    E("XMLRPC: Stop called with invalid number of arguments (%u)",
      params.size() );
    result = 0.0;
    return;
  }

  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }

  D("XMLRPC: Stop decoder %u", decnum+1);
  result = (double) dec->stop();
}

void Pause::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;
  
  if( params.size() < 1) {
    E("XMLRPC: Pause called with invalid number of arguments (%u)",
      params.size() );
    result = 0.0;
    return;
  }

  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }

  D("XMLRPC: Pause decoder %u", decnum+1);
  result = (double) dec->pause();
}


void GetPos::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;
  double pos;

  if( params.size() < 1) {
    E("XMLRPC: GetPos called with invalid number of arguments (%u)",
      params.size() );
    return;
  }

  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }

  pos = (double) dec->getpos();
  result = pos;
  D("XMLRPC: GetPos decoder %u returns %d", decnum+1, pos);

}

void SetPos::execute(XmlRpcValue &params, XmlRpcValue &result) {
  int decnum;

  if( params.size() < 2) {
    E("XMLRPC: SetPos called with invalid number of arguments (%u)",
      params.size() );
    return;
  }

  decnum = (int) params[0] -1;
  Decoder *dec = get_decoder( decnum );
  if(!dec) { result = 0.0; return; }

  D("XMLRPC: SetPos decoder %u", decnum+1);
  dec->setpos( (int) params[1] );
}

