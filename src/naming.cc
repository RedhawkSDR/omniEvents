// -*- Mode: C++; -*-
//                            Package   : omniEvents
// naming.cc                  Created   : 1/10/99
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2005 Alex Tingle.
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
//    naming Service Utility functions.
//

/*
  $Log: naming.cc,v $
  Revision 1.8.2.2  2005/05/10 14:28:11  alextingle
  Updated copyrights to 2005.

  Revision 1.8.2.1  2005/04/27 20:49:31  alextingle
  Merge across changes from HEAD branch (see CHANGES_262. Change version number ready for release 2.6.2.

  Revision 1.9  2005/04/13 14:04:02  alextingle
  Fixed bug in str2name() naming.cc, that causes a SEGV on HP-UX.

  Revision 1.8  2004/10/08 14:27:59  alextingle
  Changed local variable initialisation style back to using '=' in order to please MS VC++.

  Revision 1.7  2004/09/25 23:12:28  alextingle
  New method: Orb::reportObjectFailure() - flags unexpected failures at a higher
  priority than normal non-fatal exceptions.

  New macro: NP_MINORSTRING() - a safe interface to
  CORBA::SystemException::NP_minorString() that returns "??" when there is no
  mapping for the exception's minor code.

  Revision 1.6  2004/08/04 08:13:44  alextingle
  Unix daemon & Windows service now both working. Accessed through interface class Daemon (in daemon.h).

  Revision 1.5  2004/07/26 21:17:49  alextingle
  Added missing #include <string>

  Revision 1.4  2004/07/26 16:22:25  alextingle
  New method: str2name() parses a stringified naming service name info a CosNaming::Name.

  Revision 1.3  2004/07/02 15:20:39  alextingle
  Added daemonization, syslog & pidfile support on Unix.
  Corrected trace levels for consistency with omniORB.

  Revision 1.2  2004/04/21 10:01:42  alextingle
  Removed unused code. Now silently fails if the Orb has no naming service ref.

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.3  2003/12/01 09:03:13  alextingle
  Now reports more specific exceptions (only with omniORB4).

  Revision 1.2  2003/11/03 22:45:31  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.

  Revision 1.1.1.1  2002/09/25 19:00:35  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.3  2000/09/26 08:44:58  naderp
  Added stdlib.h include for exit function.

  Revision 1.2  2000/09/04 03:45:52  naderp
  Changed headers.

  Revision 1.1  1999/11/01 17:00:16  naderp
  Initial revision

*/

#include "naming.h"

#include <string>

#ifdef HAVE_IOMANIP
#  include <iomanip>
#else
#  include <iomanip.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h> // for exit
#endif

ostream& operator<<(ostream& os, const CosNaming::Name &n)
{
  for(CORBA::ULong i=0; i<n.length(); i++)
  {
    os<<"/"<<n[i].id.in();
    const char* kind =n[i].kind.in();
    if(kind && kind[0])
        os<<"."<<kind;
  }
  return os;
}


CosNaming::Name str2name(const char* namestr)
{
  CosNaming::Name name;
  CORBA::ULong nameLen=0;
  name.length(nameLen);

  string n =namestr;
  string::size_type pos=0;
  char last='/';
  while(true)
  {
    pos=n.find_first_not_of("/.",pos);
    if(string::npos==pos) break;
    string::size_type sep =n.find_first_of("/.",pos);
    string piece =n.substr(pos, (string::npos==sep? sep: sep-pos) );
    if(last=='/')
    {
      name.length(++nameLen);
      name[nameLen-1].id=CORBA::string_dup(piece.c_str());
    }
    else
    {
      name[nameLen-1].kind=CORBA::string_dup(piece.c_str());
    }
    if(string::npos==sep) break;
    pos=sep;
    last=n[sep];
  }
  return name;
}


int bindName2Object(
  CosNaming::NamingContext_ptr namingContext,
  const CosNaming::Name& name,
  CORBA::Object_ptr obj
)
{
  // If there is no naming service, then ignore this call.
  if(CORBA::is_nil(namingContext))
      return 1;

  try
  {

      CosNaming::Name n;
      n.length(1);
      // Drill down through contexts.
      for(CORBA::ULong i=0; i<(name.length()-1); ++i)
      {
        n[0]=name[i];
        try
        {
          namingContext=namingContext->bind_new_context(n);
        }
        catch(CosNaming::NamingContext::AlreadyBound&)
        {
          CORBA::Object_var obj2 =namingContext->resolve(n);
          namingContext=CosNaming::NamingContext::_narrow(obj2);
        }
        // One of the context names is already bound to an object. Bail out!
        if(CORBA::is_nil(namingContext))
            return 2;
      }
      // Bind the object
      n[0]=name[name.length()-1];
      try
      {
        namingContext->bind(n,obj);
      }
      catch(CosNaming::NamingContext::AlreadyBound& ex)
      {
        // overwrite previously bound object
        namingContext->rebind(n,obj);
      }
      return 0;

  }
  catch (CORBA::COMM_FAILURE& ex)
  {
     cerr << "Caught system exception COMM_FAILURE, unable to contact the "
          << "naming service." << endl;
  }
  catch (omniORB::fatalException& ex)
  {
     cerr << "Caught omniORB fatal exception binding " << name << endl;
     throw;
  }
  catch (CORBA::SystemException& ex)
  {
     const char* exName  =NULL;
     const char* exMinor =NULL;
#ifdef HAVE_OMNIORB4
     exName =ex.NP_minorString();
     exMinor=ex.NP_minorString();
#endif
     cerr<<"System exception binding "<<name;
     if(exName)
       cerr<<": "<<exName;
     if(exMinor)
       cerr<<" ("<<exMinor<<")";
     cerr<<endl;
  }
  catch (CORBA::Exception& ex)
  {
     cerr<<"CORBA exception binding "<<name
#ifdef HAVE_OMNIORB4
         <<": "<<ex._name()
#endif
         << endl;
  }
  ::exit(1);
}
