/*                            Package   : omniEvents
 * gethostname.h              Created   : 2003/10/31
 *                            Author    : Alex Tingle
 *
 *    Copyright (C) 2003 Alex Tingle.
 *
 *    This file is part of the omniEvents application.
 *
 *    omniEvents is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    omniEvents is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * Provides the function `int gethostname(char* hostname, size_t len)'
 * in a platform independent manner. The associated macro MAXHOSTNAMELEN
 * is also guaranteed to be set correctly.
 */

#ifndef __GETHOSTNAME_H
#define __GETHOSTNAME_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif

#ifdef __WIN32__
#  include <winbase.h>
#endif

#if defined(__VMS) && __CRTL_VER < 70000000
#  include <omniVMS/utsname.hxx>
#endif

#include <errno.h>

/*
 * Ensure that MAXHOSTNAMELEN is defined correctly.
 */

#if defined(__WIN32__) && !defined(MAXHOSTNAMELEN)
#  define MAXHOSTNAMELEN MAX_COMPUTERNAME_LENGTH
#elif defined(__WIN32__) && defined(MAXHOSTNAMELEN)
#  undef  MAXHOSTNAMELEN
#  define MAXHOSTNAMELEN MAX_COMPUTERNAME_LENGTH
#elif !defined(MAXHOSTNAMELEN)
#  define MAXHOSTNAMELEN 256
/* Apparently on some AIX versions, MAXHOSTNAMELEN is too small (32) to
 * reflect the true size a hostname can be. Check and fix the value. */
#elif defined(MAXHOSTNAMELEN) && (MAXHOSTNAMELEN < 64)
#  undef  MAXHOSTNAMELEN
#  define MAXHOSTNAMELEN 256
#endif


#ifndef HAVE_GETHOSTNAME
inline int
gethostname(char* hostname, size_t len)
{
  int result =-1;
  if(len<1)
  {
    errno=EINVAL;
    return result;
  }
  if(len>MAXHOSTNAMELEN)
  {
    len=MAXHOSTNAMELEN;
  }

#if defined(__WIN32__)
  DWORD dwordlen = len;
  if( GetComputerName((LPTSTR) hostname, &dwordlen) )
  {
    result=0;
  }
  else
  {
    errno=EFAULT;
  }
#else
  struct utsname un;
  if( uname(&un)==0 && strlen(un.nodename)<len)
  {
    strcpy(hostname,un.nodename);
    result=0;
  }
  else
  {
    errno=EFAULT;
  }
#endif
  return result;
}
#endif /* HAVE_GETHOSTNAME */

#endif /* __GETHOSTNAME_H */
