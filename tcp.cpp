/*
   rdesktop: A Remote Desktop Protocol client.
   Protocol services - TCP layer
   Copyright (C) Matthew Chapman 1999-2002
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// #include <unistd.h>		/* select read write close */
// #include <sys/socket.h>		/* socket connect setsockopt */
// #include <sys/time.h>		/* timeval */
// #include <netdb.h>		/* gethostbyname */
// #include <netinet/in.h>		/* sockaddr_in */
// #include <netinet/tcp.h>	/* TCP_NODELAY */
// #include <arpa/inet.h>		/* inet_addr */
#include <errno.h>		/* errno */
#include "rdesktop.h"
#include <exception.h>

static int sock;
static struct stream in;
static struct stream out;
extern int tcp_port_rdp;

/* Initialise TCP transport data packet */
STREAM
tcp_init(int maxlen)
{
	throw not_implemented_error();
}

/* Send TCP transport data packet */
void
tcp_send(STREAM s)
{
	throw not_implemented_error();
}

/* Receive a message on the TCP layer */
STREAM
tcp_recv(int length)
{
	throw not_implemented_error();
}

/* Establish a connection on the TCP layer */
BOOL
tcp_connect(char *server)
{
	throw not_implemented_error();
}

/* Disconnect on the TCP layer */
void
tcp_disconnect(void)
{
	throw not_implemented_error();
}
