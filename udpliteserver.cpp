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

#include <stdio.h>

#include <stdlib.h>                    /* exit()        */
#include <unistd.h>                    /* close()       */
#include <errno.h>                     /* strerror()    */
#include <string.h>                    /* memset()      */

#include <utils.h>
#include <udpliteserver.h>




UdpLiteSrv::UdpLiteSrv() {
  addrlen = sizeof(struct sockaddr_storage);
  type = v4only; //v6andv4;
  syncstart = NULL;
  
  /*
   * 	get server IP address
   */
  memset(&hints, 0, sizeof(hints));              /* set all unneeded parameters to 0 */
  sprintf(sbuf, "%d", SERVER_PORT);              /* stringify port number            */
  /*
   * bind socket to server port.
   * Setting node to NULL in getaddrinfo will result in
   *  --the unspecified (INADDR_ANY | inaddr6_any) address if AI_PASSIVE is set
   *  --the loopback    (127.0.0.1  |    ::1     ) address if AI_PASSIVE is not set
   */
  hints.ai_flags    =  AI_PASSIVE|AI_NUMERICHOST|AI_ADDRCONFIG;
  hints.ai_family   = (type == v4only)? AF_INET : AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;                  /* either UDP or UDP-Lite */

  if(getaddrinfo(NULL, sbuf, &hints, &res)  ||     /* lookup error           */
     res->ai_next    != NULL                   )   /* address ambiguity      */
    E("Can not resolve the server address.");

  /*
   * Open up the socket
   */
  sd = socket(res->ai_family, res->ai_socktype, IPPROTO_UDPLITE);
  if (sd < 0)
     E("Cannot open socket.");

#if IPPROTO_UDPLITE
  /*
   * Specify the minimum socket coverage required at the receiver.
   */
  option = CS_COV;
  rc = setsockopt(sd, SOL_UDPLITE, UDPLITE_RECV_CSCOV, &option, sizeof(int));
   if (rc < 0)
     E("Can not set receiver minimum checksum coverage to %d", option);
#endif
  /* 
   * set receive timestamps (returned as cmsg_data)
   */
  option = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_TIMESTAMP, &option, sizeof(option)) < 0)
    E("Can not set the SO_TIMESTAMP option");

  /*
   * Restrict to IPv6-only listening if option was given on the commandline
   */
  if (type == v6only) {
    option = 1;
    rc = setsockopt(sd, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option));
    if (rc < 0)
      E("Can not set the IPV6_V6ONLY option.");
  }


  /*
   * Bind socket to serving port.
   */
  rc = bind(sd, res->ai_addr, res->ai_addrlen);
  if (rc < 0)
    E("Cannot bind socket to port number %d", SERVER_PORT);


  N("Accepting %s UDP%s connections on port %d",
    (type == v4only)?       "IPv4-only" :
    (type == v6only)?       "IPv6-only" : "IPv4/IPv6",
    (IPPROTO_UDPLITE > 0)?  "-Lite"     : "", SERVER_PORT);
  
}

UdpLiteSrv::~UdpLiteSrv() {

  freeaddrinfo(res);

}

void UdpLiteSrv::init(bool *syncbit) {
  syncstart = syncbit;
}

void UdpLiteSrv::run() {
  /*
   * server endless loop
   */
  memset(buf, 0, sizeof(buf));       /* wipe old buffer contents */
  
  /* we use recvmsg to get at the flags passed by the kernel */
  iov[0].iov_base = buf;
  iov[0].iov_len  = sizeof(buf);
  msg.msg_iov		= iov;
  msg.msg_iovlen	= 1;
  msg.msg_name	= (void *)&cliaddr;
  msg.msg_namelen	= addrlen;
  msg.msg_controllen	= TS_LEN; 
  msg.msg_control	= (void *) tsbuf;
  
  n = recvmsg(sd, &msg, 0);
  
  if (msg.msg_flags & MSG_TRUNC)     /* prepend warning message */
    W("datagram truncated");
  
  if (n < 0)                        /* jump back on error      */
    E("receive problem");
  
  /* resolve timestamp */
  cmsg = CMSG_FIRSTHDR(&msg);
  stamp = (struct timeval *) CMSG_DATA(cmsg);
  if (!cmsg)
    W("Failed to get a timestamp for this reception");
  else
    A("[%-6lu.%06lu] ", stamp->tv_sec, stamp->tv_usec);
  
  /* resolve the sender of the message */
  rc = getnameinfo((struct sockaddr *)&cliaddr, addrlen,
		   hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST|NI_NUMERICSERV);
  if (rc)
    W("Can not resolve client address: %s", gai_strerror(rc));
  else
    A("from %s on port %s", hbuf, sbuf);

  if(syncstart) *syncstart = true;

}



