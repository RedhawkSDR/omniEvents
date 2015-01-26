// -*- Mode: C++; -*-
//                            Package   : omniEvents
// getopt.h                   Created   : 1/4/98
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

/*
  $Log: getopt.h,v $
  Revision 1.2  2004/07/15 14:34:31  alextingle
  Global variables are now declared extern "C".

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.2  2003/11/03 22:36:48  alextingle
  Updated License to GNU Lesser General Public v2.1

  Revision 1.1.1.1  2002/09/25 19:00:32  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.4  2000/09/24 07:17:12  naderp
  Fixed file comment.

  Revision 1.3  1999/04/23 16:04:53  naderp
  *** empty log message ***

 * Revision 1.2  99/04/23  12:11:20  12:11:20  naderp (Paul Nader)
 * *** empty log message ***
 * 
 * Revision 1.1  99/04/23  09:36:25  09:36:25  naderp (Paul Nader)
 * Initial revision
 * 
*/

#ifndef __GETOPT_H
#define __GETOPT_H

extern "C"
{
  extern int   optind;    // Index: welches Argument ist das naechste
  extern char* optarg;    // Zeiger auf das Argument der akt. Option
  extern int   opterr;    // erlaubt Fehlermeldungen
}

int getopt(int argc, char *argv[], const char *optionS);

#endif /* __GETOPT_H */
