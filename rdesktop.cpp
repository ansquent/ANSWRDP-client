/*
   rdesktop: A Remote Desktop Protocol client.
   Entrypoint and utility functions
   Copyright (C) Matthew Chapman 1999-2003

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

#include <stdarg.h>		/* va_list va_start va_end */
//#include <unistd.h>		/* read close getuid getgid getpid getppid gethostname */
#include <fcntl.h>		/* open */
#include <errno.h>		/* save licence uses it. */
//#include <pwd.h>		/* getpwuid */
//#include <termios.h>		/* tcgetattr tcsetattr */
#include <sys/stat.h>		/* stat */
//#include <sys/time.h>		/* gettimeofday */
//#include <sys/times.h>		/* times */
#include "rdesktop.h"
#include <exception.h>
#include <cstdlib>
#include <QDebug>
#include <QRandomGenerator>
#include <QTcpSocket>

char title[32] = "";
char username[16] = "Administrator";
char hostname[16];
char keymapname[16];
int keylayout = 0x409;		/* Defaults to US keyboard layout */
int width = 800;		/* If width or height are reset to zero, the geometry will 
				   be fetched from _NET_WORKAREA */
int height = 600;
int tcp_port_rdp = TCP_PORT_RDP;
BOOL bitmap_compression = True;
BOOL sendmotion = True;
BOOL orders = True;
BOOL encryption = True;
BOOL desktop_save = True;
BOOL fullscreen = False;
BOOL grab_keyboard = True;
BOOL hide_decorations = False;
extern BOOL owncolmap;

/* Display usage information */
static void
usage(char *program)
{
	fprintf(stderr, "rdesktop: A Remote Desktop Protocol client.\n");
	fprintf(stderr, "Version " VERSION ". Copyright (C) 1999-2003 Matt Chapman.\n");
	fprintf(stderr, "See http://www.rdesktop.org/ for more information.\n\n");

	fprintf(stderr, "Usage: %s [options] server[:port]\n", program);
	fprintf(stderr, "   -u: user name\n");
	fprintf(stderr, "   -d: domain\n");
	fprintf(stderr, "   -s: shell\n");
	fprintf(stderr, "   -c: working directory\n");
	fprintf(stderr, "   -p: password (- to prompt)\n");
	fprintf(stderr, "   -n: client hostname\n");
	fprintf(stderr, "   -k: keyboard layout on terminal server (us,sv,gr,etc.)\n");
	fprintf(stderr, "   -g: desktop geometry (WxH)\n");
	fprintf(stderr, "   -f: full-screen mode\n");
	fprintf(stderr, "   -b: force bitmap updates\n");
	fprintf(stderr, "   -e: disable encryption (French TS)\n");
	fprintf(stderr, "   -m: do not send motion events\n");
	fprintf(stderr, "   -C: use private colour map\n");
	fprintf(stderr, "   -K: keep window manager key bindings\n");
	fprintf(stderr, "   -T: window title\n");
	fprintf(stderr, "   -D: hide window manager decorations\n");
}

static BOOL
read_password(char *password, int size)
{
	throw not_implemented_error();
}

/* Client program */
int
tmain(int argc, char *argv[])
{
    return 0;
}


/* Generate a 32-byte random for the secure transport code. */
void
generate_random(uint8 * random)
{
    //throw not_implemented_error();
    for (int i = 0; i < SEC_RANDOM_SIZE; ++i){
        random[i] = (uint8)(QRandomGenerator::global()->generate() % (1 << 8));
    }
}

/* malloc; exit if out of memory */
void *
xmalloc(int size)
{
	void *mem = malloc(size);
	if (mem == NULL)
	{
		error("xmalloc %d\n", size);
		exit(1);
	}
	return mem;
}

/* realloc; exit if out of memory */
void *
xrealloc(void *oldmem, int size)
{
	void *mem = realloc(oldmem, size);
	if (mem == NULL)
	{
		error("xrealloc %d\n", size);
		exit(1);
	}
	return mem;
}

/* free */
void
xfree(void *mem)
{
	free(mem);
}

/* report an error */
void
error(const char *format, ...)
{
    char buf[500]; int i;
    va_list vlist;
    va_start(vlist,format);
    i=vsprintf(buf,format,vlist);
    va_end(vlist);
    qDebug()<< " ERROR: " << buf;
    exit(-1);
}

/* report a warning */
void
warning(const char *format, ...)
{
    char buf[500]; int i;
    va_list vlist;
    va_start(vlist,format);
    i=vsprintf(buf,format,vlist);
    va_end(vlist);
    qDebug()<< " WARNING: " << buf;
}

/* report an unimplemented protocol feature */
void
unimpl(const char *format, ...)
{
    char buf[500]; int i;
    va_list vlist;
    va_start(vlist,format);
    i=vsprintf(buf,format,vlist);
    va_end(vlist);
    qDebug()<< " UNIMPL: " << buf;
}

/* report an info */
void
info(const char *format, ...)
{
    char buf[500]; int i;
    va_list vlist;
    va_start(vlist,format);
    i=vsprintf(buf,format,vlist);
    va_end(vlist);
    qDebug()<< " INFO: " << buf;
}

void info(STREAM s){
    for (int i = 0; i < (s->end - s->p);){
        QString qs;
        for (int j = 0; j < 10; ++j, ++i){
            qs += " ";
            qs += QString("0x%1").arg(QString("%1").arg(s->p[i],2,16,QLatin1Char('0')).toUpper());
        }
        qDebug() << qs;
    }
}

/* produce a hex dump */
void
hexdump(unsigned char *p, unsigned int len)
{
	unsigned char *line = p;
	unsigned int thisline, offset = 0;
	int i;

	while (offset < len)
	{
		printf("%04x ", offset);
		thisline = len - offset;
		if (thisline > 16)
			thisline = 16;

		for (i = 0; i < thisline; i++)
			printf("%02x ", line[i]);

		for (; i < 16; i++)
			printf("   ");

		for (i = 0; i < thisline; i++)
			printf("%c", (line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');

		printf("\n");
		offset += thisline;
		line += thisline;
	}
}

