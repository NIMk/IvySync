/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2008 Denis Roio aka jaromil <jaromil@dyne.org>
 *                          Nederlands Instituut voor Mediakunst
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
 *
 * to compile this sourcecode: gcc -o ivysync ivysync.c -lpthreads
 * it should work on any POSIX system, including embedded hardware
 * wherever the IvyTV drivers can also run (see http://ivtv.sf.net)
 *
 */

#ifndef __UDPLITESERVER_H__
#define __UDPLITESERVER_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>                     /* getaddrinfo() */
#include <arpa/inet.h>                 /* inet_ntop()   */
#include <netinet/in.h>                /* struct sockaddr_in|6  */


#include <thread.h>


/* CONSTANTS */
#define CS_COV              8          /* UDP-Lite checksum coverage */
#define LOCAL_PORT          2000       /* client (sender) port       */
#define SERVER_PORT         15500      /* remote service port        */
/* the following determines the size of packets read from the network */
#define READSIZE            BUFSIZ

#define  TS_LEN 	CMSG_SPACE(sizeof(struct timeval))

// if you comment out the following line, you will get a UDP application
#undef IPPROTO_UDPLITE

#ifdef IPPROTO_UDPLITE
#include <netinet/udplite.h>           /* definitions for UDP-Lite   */
#else
#warning "No UDP-Lite support, using UDP instead"
#define  IPPROTO_UDPLITE 0
#endif


class UdpLiteSrv : public Thread {

 public:
  UdpLiteSrv();
  ~UdpLiteSrv();

  void run();

  void init(bool *syncbit);

  bool *syncstart;

 private:
  struct sockaddr_storage cliaddr;               /* address of connecting host */
  socklen_t               addrlen; //  = sizeof(struct sockaddr_storage);

  struct addrinfo         hints,                 /* address resolution aid     */
                         *res;                   /* resolution results         */
  struct iovec            iov[1];                /* scatter/gather array       */
  struct msghdr           msg;                   /* for recvmsg(), see below   */
  struct cmsghdr         *cmsg;			 /* for the timestamp          */
  struct timeval         *stamp;
  char                    buf[READSIZE],         /* read buffer                */
                          hbuf[NI_MAXHOST],      /* address as string          */
                          sbuf[NI_MAXSERV],      /* service port as string     */
			  tsbuf[TS_LEN];	 /* to contain timestamp       */
  int                     sd,                    /* socket descriptor          */
                          rc, option, n;
  enum {v6andv4, v4only, v6only} type; // = v6andv4; /* IP service type            */


};

#endif
