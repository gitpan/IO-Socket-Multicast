/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.19 from the
 * contents of Multicast.xs. Do not edit this file, edit Multicast.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "Multicast.xs"
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include <sys/socket.h>

#ifdef PerlIO
typedef PerlIO * InputStream;
#else
#define PERLIO_IS_STDIO 1
typedef FILE * InputStream;
#define PerlIO_fileno(f) fileno(f)
#endif

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

/* Recent versions of Win32 platforms are confused about these constants due to 
 problems in the order of socket header file importation 

#ifdef WIN32
#if (PERL_REVISION >=5) && (PERL_VERSION >= 8) && (PERL_SUBVERSION >= 6)
#undef  IP_OPTIONS
#undef  IP_HDRINCL
#undef  IP_TOS
#undef  IP_TTL
#undef  IP_MULTICAST_IF
#undef  IP_MULTICAST_TTL
#undef  IP_MULTICAST_LOOP
#undef  IP_ADD_MEMBERSHIP
#undef  IP_DROP_MEMBERSHIP
#undef  IP_DONTFRAGMENT

#define IP_OPTIONS          1 
#define IP_HDRINCL          2
#define IP_TOS              3
#define IP_TTL              4
#define IP_MULTICAST_IF     9 
#define IP_MULTICAST_TTL   10
#define IP_MULTICAST_LOOP  11
#define IP_ADD_MEMBERSHIP  12
#define IP_DROP_MEMBERSHIP 13
#define IP_DONTFRAGMENT    14
#endif
#endif

*/

#ifndef HAS_INET_ATON
static int
my_inet_aton(register const char *cp, struct in_addr *addr)
{
	dTHX;
	register U32 val;
	register int base;
	register char c;
	int nparts;
	const char *s;
	unsigned int parts[4];
	register unsigned int *pp = parts;

       if (!cp || !*cp)
		return 0;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, other=decimal.
		 */
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isDIGIT(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && (s=strchr(PL_hexdigit,c))) {
				val = (val << 4) +
					((s - PL_hexdigit) & 15);
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16-bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return 0;
			*pp++ = val, cp++;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && !isSPACE(*cp))
		return 0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	nparts = pp - parts + 1;	/* force to an int for switch() */
	switch (nparts) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return 0;
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	addr->s_addr = htonl(val);
	return 1;
}

#undef inet_aton
#define inet_aton my_inet_aton

#endif


#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#line 175 "Multicast.c"

XS(XS_IO__Socket__Multicast__mcast_add); /* prototype to pass -Wmissing-prototypes */
XS(XS_IO__Socket__Multicast__mcast_add)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       Perl_croak(aTHX_ "Usage: %s(%s)", "IO::Socket::Multicast::_mcast_add", "sock, mcast_group, interface_addr=\"\"");
    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	InputStream	sock = IoIFP(sv_2io(ST(0)));
	char*	mcast_group = (char *)SvPV_nolen(ST(1));
	char*	interface_addr;
#line 171 "Multicast.xs"
     int fd;
     struct ip_mreq mreq;
#line 197 "Multicast.c"

	if (items < 3)
	    interface_addr = "";
	else {
	    interface_addr = (char *)SvPV_nolen(ST(2));
	}
#line 174 "Multicast.xs"
     {
       fd = PerlIO_fileno(sock);
       if (!inet_aton(mcast_group,&mreq.imr_multiaddr))
         croak("Invalid address used for mcast group");
       if ((strlen(interface_addr) > 0)) {
	 if (!inet_aton(interface_addr,&mreq.imr_interface))
	   croak("Invalid address used for local interface");
       } else {
	 mreq.imr_interface.s_addr = INADDR_ANY;
       }
       if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(void*) &mreq,sizeof(mreq)) < 0)
	 XSRETURN_EMPTY;
       else
	 XSRETURN_YES;
     }
#line 220 "Multicast.c"
	PUTBACK;
	return;
    }
}


XS(XS_IO__Socket__Multicast__mcast_drop); /* prototype to pass -Wmissing-prototypes */
XS(XS_IO__Socket__Multicast__mcast_drop)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       Perl_croak(aTHX_ "Usage: %s(%s)", "IO::Socket::Multicast::_mcast_drop", "sock, mcast_group, interface_addr=\"\"");
    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	InputStream	sock = IoIFP(sv_2io(ST(0)));
	char*	mcast_group = (char *)SvPV_nolen(ST(1));
	char*	interface_addr;
#line 197 "Multicast.xs"
     int fd;
     struct ip_mreq mreq;
#line 247 "Multicast.c"

	if (items < 3)
	    interface_addr = "";
	else {
	    interface_addr = (char *)SvPV_nolen(ST(2));
	}
#line 200 "Multicast.xs"
     {
       fd = PerlIO_fileno(sock);
       if (!inet_aton(mcast_group,&mreq.imr_multiaddr))
         croak("Invalid address used for mcast group");
       if ((strlen(interface_addr) > 0)) {
	 if (!inet_aton(interface_addr,&mreq.imr_interface))
	   croak("Invalid address used for local interface");
       } else {
	 mreq.imr_interface.s_addr = htonl(INADDR_ANY);
       }
       if (setsockopt(fd,IPPROTO_IP,IP_DROP_MEMBERSHIP,(void*)&mreq,sizeof(mreq)) < 0)
	 XSRETURN_EMPTY;
       else
	 XSRETURN_YES;
     }
#line 270 "Multicast.c"
	PUTBACK;
	return;
    }
}


XS(XS_IO__Socket__Multicast_mcast_loopback); /* prototype to pass -Wmissing-prototypes */
XS(XS_IO__Socket__Multicast_mcast_loopback)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1)
       Perl_croak(aTHX_ "Usage: %s(%s)", "IO::Socket::Multicast::mcast_loopback", "sock, ...");
    PERL_UNUSED_VAR(cv); /* -W */
    {
	InputStream	sock = IoIFP(sv_2io(ST(0)));
#line 221 "Multicast.xs"
     int fd;
     int len;
     char previous,loopback;
#line 294 "Multicast.c"
	int	RETVAL;
	dXSTARG;
#line 225 "Multicast.xs"
     {
       fd = PerlIO_fileno(sock);
       /* get previous value of flag */
       len = sizeof(previous);
       if (getsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP,(void*)&previous,&len) < 0)
	 XSRETURN_UNDEF;

       if (items > 1) { /* set value */
	 loopback = SvIV(ST(1));
	 if (setsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP,(void*)&loopback,sizeof(loopback)) < 0)
	   XSRETURN_UNDEF;
       }
       RETVAL = previous;
     }
#line 312 "Multicast.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_IO__Socket__Multicast_mcast_ttl); /* prototype to pass -Wmissing-prototypes */
XS(XS_IO__Socket__Multicast_mcast_ttl)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1)
       Perl_croak(aTHX_ "Usage: %s(%s)", "IO::Socket::Multicast::mcast_ttl", "sock, ...");
    PERL_UNUSED_VAR(cv); /* -W */
    {
	InputStream	sock = IoIFP(sv_2io(ST(0)));
#line 247 "Multicast.xs"
     int fd;
     int len;
     char previous,ttl;
#line 336 "Multicast.c"
	int	RETVAL;
	dXSTARG;
#line 251 "Multicast.xs"
     {
       fd = PerlIO_fileno(sock);
       /* get previous value of flag */
       len = sizeof(previous);
       if (getsockopt(fd,IPPROTO_IP,IP_MULTICAST_TTL,(void*)&previous,&len) < 0)
	 XSRETURN_UNDEF;

       if (items > 1) { /* set value */
	 ttl = SvIV(ST(1));
	 if (setsockopt(fd,IPPROTO_IP,IP_MULTICAST_TTL,(void*)&ttl,sizeof(ttl)) < 0)
	   XSRETURN_UNDEF;
       }
       RETVAL = previous;
     }
#line 354 "Multicast.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_IO__Socket__Multicast__mcast_if); /* prototype to pass -Wmissing-prototypes */
XS(XS_IO__Socket__Multicast__mcast_if)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1)
       Perl_croak(aTHX_ "Usage: %s(%s)", "IO::Socket::Multicast::_mcast_if", "sock, ...");
    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	InputStream	sock = IoIFP(sv_2io(ST(0)));
#line 273 "Multicast.xs"
     int                fd,len;
     STRLEN             slen;
     char*              addr;
     struct in_addr     ifaddr;
     struct ip_mreq     mreq;
#line 382 "Multicast.c"
#line 279 "Multicast.xs"
     {
       fd = PerlIO_fileno(sock);
       if (items > 1) { /* setting interface */
	 addr = SvPV(ST(1),slen);
	 if (inet_aton(addr,&ifaddr) == 0 )
	   XSRETURN_EMPTY;
	 if (setsockopt(fd,IPPROTO_IP,IP_MULTICAST_IF,(void*)&ifaddr,sizeof(ifaddr)) == 0)
	   XSRETURN_YES;
	 else
	   XSRETURN_NO;
       } else {  /* getting interface address */

	 /* freakin' bug in Linux -- IP_MULTICAST_IF returns a struct mreqn rather than
	    an in_addr (contrary to Stevens and the setsockopt()!  
	    We work around this by looking at size of returned thing and doing a 
	    ugly cast */

	 len = sizeof(mreq);
	 if (getsockopt(fd,IPPROTO_IP,IP_MULTICAST_IF,(void*) &mreq,&len) != 0)
	   XSRETURN_EMPTY;

	 if (len == sizeof(mreq)) {
	   XPUSHs(sv_2mortal(newSVpv(inet_ntoa(mreq.imr_interface),0)));
	 } else if (len == sizeof (struct in_addr)) {
	   XPUSHs(sv_2mortal(newSVpv(inet_ntoa(*(struct in_addr*)&mreq),0)));
	 } else {
	   croak("getsockopt() returned a data type I don't understand");
	 }

       }
     }
#line 415 "Multicast.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_IO__Socket__Multicast); /* prototype to pass -Wmissing-prototypes */
XS(boot_IO__Socket__Multicast)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    char* file = __FILE__;

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
    XS_VERSION_BOOTCHECK ;

        newXSproto("IO::Socket::Multicast::_mcast_add", XS_IO__Socket__Multicast__mcast_add, file, "$$;$");
        newXSproto("IO::Socket::Multicast::_mcast_drop", XS_IO__Socket__Multicast__mcast_drop, file, "$$;$");
        newXSproto("IO::Socket::Multicast::mcast_loopback", XS_IO__Socket__Multicast_mcast_loopback, file, "$;$");
        newXSproto("IO::Socket::Multicast::mcast_ttl", XS_IO__Socket__Multicast_mcast_ttl, file, "$;$");
        newXSproto("IO::Socket::Multicast::_mcast_if", XS_IO__Socket__Multicast__mcast_if, file, "$;$");
    if (PL_unitcheckav)
         call_list(PL_scopestack_ix, PL_unitcheckav);
    XSRETURN_YES;
}

