/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004 - 2005 Denis Rojo <jaromil@dyne.org>
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

#ifndef __XMLRPC_H__
#define __XMLRPC_H__


#include <XmlRpc.h>

#include <iostream>
#include <stdlib.h>

#include <thread.h>

using namespace XmlRpc;

class IvySyncDaemon : public Thread {
public:
  IvySyncDaemon(XmlRpcServer *srv);
  ~IvySyncDaemon() { };
  
  void run();
  
private:
  XmlRpcServer *xmlrpc;
};

class IvySyncPublicMethod {
public:
  
  IvySyncPublicMethod(vector<Decoder*> *decs) {
    decoders = decs;
  }

  ~IvySyncPublicMethod() { };

  Decoder *get_decoder(int num) {
    vector<Decoder*>::iterator dec_iter;
    dec_iter = decoders->begin();
    dec_iter += num;
    return *dec_iter;
  }
  
 private:
  vector<Decoder*> *decoders;

};

//RPC(Stop,"Stop playing a channel");
//RPC(PlaylistAppend,"Append a file to a channel's playlist");

class GetPos : public XmlRpcServerMethod, IvySyncPublicMethod {
public:
  
  GetPos(XmlRpcServer* srv, vector<Decoder*> *decoders);
  
  ~GetPos() { };
  
  void execute(XmlRpcValue &params, XmlRpcValue &result);

  std::string help() { 
    return std::string("Return position of channel in percentage"); }
  
};


class SetPos : public XmlRpcServerMethod, IvySyncPublicMethod {
 public:

  SetPos(XmlRpcServer* srv, vector<Decoder*> *decoders);

  ~SetPos() { };

  void execute(XmlRpcValue &params, XmlRpcValue &result);
  
  std::string help() { 
    return std::string("Skip channel to position in percentage"); }

};


#endif
