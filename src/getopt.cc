// -*- Mode: C++; -*-
//                            Package   : omniEvents
// getopt.cc                  Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader.
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
// Description:
//
//    getopt implementation for WIN32 platforms.
//

/*
  $Log: getopt.cc,v $
  Revision 1.4  2004/08/04 08:13:44  alextingle
  Unix daemon & Windows service now both working. Accessed through interface class Daemon (in daemon.h).

  Revision 1.3  2004/07/15 14:34:31  alextingle
  Global variables are now declared extern "C".

  Revision 1.2  2004/02/22 00:37:35  alextingle
  Now does nothing if it isn't needed. Fixes link errors

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.2  2003/11/03 22:46:54  alextingle
  This implementation of getopt() seems to be needed on AIX as well as
  windows. It therefore needs to be able to compile on Unix. Changed
  _tzset() to tzset() in order to achieve this.

  Revision 1.1.1.1  2002/09/25 19:00:35  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.3  1999/11/01 16:59:38  naderp
  *** empty log message ***

Revision 1.2  99/04/23  12:11:18  12:11:18  naderp (Paul Nader)
*** empty log message ***

Revision 1.1  99/04/23  09:33:35  09:33:35  naderp (Paul Nader)
Initial revision
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_GETOPT
void omniEventsDummyGetopt(){}
#else

#include "getopt.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static char* letP =NULL;    // Speichert den Ort des Zeichens der
                                // naechsten Option
static char SW ='-';       // DOS-Schalter, entweder '-' oder '/'

// -------------------------------------------------------------- exports ----

extern "C"
{
  int   optind  = 1;    // Index: welches Argument ist das naechste
  char* optarg;         // Zeiger auf das Argument der akt. Option
  int   opterr  = 1;    // erlaubt Fehlermeldungen
}

// ===========================================================================
int getopt(int argc, char *argv[], const char *optionS)
{
   unsigned char ch;
   char *optP;

   if(argc>optind)
   {
      if(letP==NULL)
      {
        // Initialize letP
         if( (letP=argv[optind])==NULL || *(letP++)!=SW )
            goto gopEOF;
         if(*letP == SW)
         {
            // "--" is end of options.
            optind++;
            goto gopEOF;
         }
      }

      if((ch=*(letP++))== '\0')
      {
         // "-" is end of options.
         optind++;
         goto gopEOF;
      }
      if(':'==ch || (optP=(char*)strchr(optionS,ch)) == NULL)
      {
         goto gopError;
      }
      // 'ch' is a valid option
      // 'optP' points to the optoin char in optionS
      if(':'==*(++optP))
      {
         // Option needs a parameter.
         optind++;
         if('\0'==*letP)
         {
            // parameter is in next argument
            if(argc <= optind)
               goto gopError;
            letP = argv[optind++];
         }
         optarg = letP;
         letP = NULL;
      }
      else
      {
         // Option needs no parameter.
         if('\0'==*letP)
         {
            // Move on to next argument.
            optind++;
            letP = NULL;
         }
         optarg = NULL;
      }
      return ch;
   }
gopEOF:
   optarg=letP=NULL;
   return EOF;
    
gopError:
   optarg = NULL;
   errno  = EINVAL;
   if(opterr)
      perror ("error in command line");
   return ('?');
}
// ===========================================================================
//                      Ende von getopt ()
// ===========================================================================

#endif // HAVE_GETOPT

