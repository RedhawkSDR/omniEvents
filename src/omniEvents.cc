//                            Package   : omniEvents
//  omniEvents.cc             Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2005 Alex Tingle
//
//    This file is part of the omniEvents application.
//
//    omniEvents is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniEvents is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "omniEvents.h"

#define NEED_PACKAGE_INFO
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h> // exit()
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include "defaults.h"

namespace OmniEvents {

void usage(int argc, char **argv)
{
  const char* command =(argc?argv[0]:PACKAGE_NAME);
  cout<<
  "\n"
#ifdef __WIN32__
  "just run it:         "<<command<<" [OPTIONS]\n"
  "install service:     "<<command<<" install [OPTIONS]\n"
  "uninstall service:   "<<command<<" uninstall\n"
  "set service options: "<<command<<" setoptions [OPTIONS]\n"
  "get service options: "<<command<<" getoptions\n"
#else
  "Run the " PACKAGE_NAME " daemon.\n"
  "\n"
  "cold start syntax: "<<command<<" [-pPORT] "
#  ifdef HAVE_OMNIORB4
    "[-aENDPOINT] "
#  endif
    "[OPTIONS]\n"
  "warm start syntax: "<<command<<" [OPTIONS]\n"
#endif
  "\n"
  "COLD START OPTIONS:\n"
  " -p PORT      configure server port [11169]\n"
#ifdef HAVE_OMNIORB4
  " -a ENDPOINT  set alternate endPoint for failover\n"
#endif
  "\n"
  "OPTIONS:\n"
  " -l PATH      full path to data directory* [" OMNIEVENTS_LOG_DEFAULT_LOCATION "]\n"
#ifndef __WIN32__
  " -P PIDFILE   keep track of running instance in PIDFILE.\n"
#endif
  " -N ID        factory naming service id   [\"EventChannelFactory\"]\n"
#ifndef __WIN32__
  " -f           Stay in the foreground.\n"
#endif
  " -t FILE      Send trace messages to FILE instead of syslog.\n"
  " -v           print the IOR of the new EventChannelFactory.\n"
  " -V           display version\n"
  " -h           display this help text\n"
  "\n"
  "*You can also set the environment variable "<<OMNIEVENTS_LOGDIR_ENV_VAR<<"\n"
  "to specify the directory where the data files are kept.\n" << endl;
  exit(0);
}


void insertArgs(int& argc, char**& argv, int idx, int nargs)
{
  char** newArgv = new char*[argc+nargs];
  int i;
  for (i = 0; i < idx; i++) {
    newArgv[i] = argv[i];
  }
  for (i = idx; i < argc; i++) {
    newArgv[i+nargs] = argv[i];
  }
  argv = newArgv;
  argc += nargs;
}

} // end namespace OmniEvents

