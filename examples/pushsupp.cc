// -*- Mode: C++; -*-
//                            Package   : omniEvents
//   pushsupp.cc              Created on: 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2004 Alex Tingle
//
//    This file is part of the omnievents application.
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
//    Push Model supplier implementation.
//
//    Modified by REDHAWK (United States Government) - 2015

/*
  $Log: pushsupp.cc,v $
  Revision 1.10  2004/10/08 09:06:08  alextingle
  More robust exception minor code handling.

  Revision 1.9  2004/08/18 17:49:45  alextingle
  Added check for SIGPIPE before trying to use it.

  Revision 1.8  2004/08/06 16:19:23  alextingle
  -k & -K options removed.
  Naming service names may now be as complex as you like.

  Revision 1.7  2004/04/20 16:52:17  alextingle
  All examples updated for latest version on omniEvents. Server may now be
  specified as a 'corbaloc' string or IOR, instead of as naming service id/kind.

  Revision 1.6  2004/03/23 19:09:26  alextingle
  Fixed typos.

  Revision 1.5  2004/02/21 19:07:45  alextingle
  Corrected servants to use POA instead of BOA.

  Revision 1.4  2004/02/04 22:29:55  alextingle
  Reworked all C++ examples.
  Removed catch(...) as it tends to make it harder to see what's going on.
  Now uses POA instead of BOA.
  Uses omniORB4's Exception name probing.
  No longer uses 'naming.h/cc' utility code.

  Revision 1.3  2003/11/03 22:19:25  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.
  Removed stub header in order to allow makefile dependency checking to work
  correctly.
  Changed int to bool where appropriate.

  Revision 1.1.1.1.2.1  2002/09/28 22:20:51  shamus13
  Added ifdefs to enable omniEvents to compile
  with both omniORB3 and omniORB4. If __OMNIORB4__
  is defined during compilation, omniORB4 headers
  and command line option syntax is used, otherwise
  fall back to omniORB3 style.

  Revision 1.1.1.1  2002/09/25 19:00:26  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 0.14  2000/10/11 01:16:21  naderp
  *** empty log message ***

  Revision 0.13  2000/08/30 04:39:48  naderp
  Port to omniORB 3.0.1.

  Revision 0.12  2000/03/16 05:37:27  naderp
  Added stdlib.h for getopt.

  Revision 0.11  2000/03/06 13:27:10  naderp
  Using util getRootNamingContext function.
  Using stub headers.
  Fixed error messages.

  Revision 0.10  2000/03/02 03:21:20  naderp
  Added -r option to connect using nil reference.
  Added retry resiliency for handling COMM_FAUILURE exceptions.

Revision 0.9  99/11/02  13:39:17  13:39:17  naderp (Paul Nader)
Added <signal.h>

  Revision 0.8  1999/11/02 07:57:18  naderp
  Updated usage.

Revision 0.7  99/11/01  19:22:43  19:22:43  naderp (Paul Nader)
Added catch for COMM_FAILURE exception in obtain_push_consumer
and disconnect_push_consumer calls.

Revision 0.6  99/11/01  16:12:03  16:12:03  naderp (Paul Nader)
omniEvents 2.0 Release.

Revision 0.5  99/10/27  19:42:31  19:42:31  naderp (Paul Nader)
Ignoring Unix SIGPIPE signal.
Reporting sleep beforhand.

Revision 0.4  99/04/23  16:05:47  16:05:47  naderp (Paul Nader)
gcc port.

Revision 0.3  99/04/23  09:34:04  09:34:04  naderp (Paul Nader)
Windows Port.

Revision 0.2  99/04/21  18:06:26  18:06:26  naderp (Paul Nader)
*** empty log message ***

Revision 0.1.1.1  98/11/27  16:59:40  16:59:40  naderp (Paul Nader)
Added -s option to sleep after disconnecting.

Revision 0.1  98/11/25  14:08:25  14:08:25  naderp (Paul Nader)
Initial Revision

*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_GETOPT
#  include <unistd.h>
extern char* optarg;
extern int optind;
#else
#  include "getopt.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include <cstdio>

#include "CosEventComm.hh"
#include "CosEventChannelAdmin.hh"
#include "naming.h"

static void usage(int argc, char **argv);

class Supplier_i : virtual public POA_CosEventComm::PushSupplier {
public:
  Supplier_i () {};
  void disconnect_push_supplier ();
};

void
Supplier_i::disconnect_push_supplier () {
  cout << "Push Supplier: disconnected." << endl;
}

int main (int argc, char** argv)
{
  long l = 0;
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv);

  // Process Options
  int         discnum       =0;
  bool        refnil        =false;
  int         sleepInterval =0;
  const char* channelName   ="EventChannel";

  int c;
  while ((c = getopt(argc,argv,"d:rs:n:h")) != EOF)
  {
     switch (c)
     {
        case 'd': discnum = atoi(optarg);
                  break;

        case 'r': refnil = true;
                  break;

        case 's': sleepInterval = atoi(optarg);
                  break;

        case 'n': channelName = optarg;
                  break;

        case 'h':
        default : usage(argc,argv);
                  exit(-1);
                  break;
     }
  }

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
  // Ignore broken pipes
  signal(SIGPIPE, SIG_IGN);
#endif

  Supplier_i* supplier = NULL;
  CosEventChannelAdmin::EventChannel_var channel;

  const char* action=""; // Use this variable to help report errors.
  try {
    CORBA::Object_var obj;

    // A Push Supplier can be implemented as a pure client or as a mixed
    // client-server process, depending on whether it requires and is
    // prepared to service disconnect requests from the channel.
    // If it is then create the servant object and activate the POA.
    if(!refnil)
    {
      supplier=new Supplier_i();

      action="resolve initial reference 'RootPOA'";
      obj=orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var rootPoa =PortableServer::POA::_narrow(obj);
      if(CORBA::is_nil(rootPoa))
          throw CORBA::OBJECT_NOT_EXIST();

      action="activate the RootPOA's POAManager";
      PortableServer::POAManager_var pman =rootPoa->the_POAManager();
      pman->activate();
    }

    //
    // Obtain object reference to EventChannel
    // (from command-line argument or from the Naming Service).
    if(optind<argc)
    {
      action="convert URI from command line into object reference";
      obj=orb->string_to_object(argv[optind]);
    }
    else
    {
      action="resolve initial reference 'NameService'";
      obj=orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var rootContext=
        CosNaming::NamingContext::_narrow(obj);
      if(CORBA::is_nil(rootContext))
          throw CORBA::OBJECT_NOT_EXIST();

      action="find EventChannel in NameService";
      cout << action << endl;
      obj=rootContext->resolve(str2name(channelName));
    }

    action="narrow object reference to event channel";
    channel=CosEventChannelAdmin::EventChannel::_narrow(obj);
    if(CORBA::is_nil(channel))
    {
       cerr << "Failed to narrow Event Channel reference." << endl;
       exit(1);
    }

  }
  catch(CORBA::ORB::InvalidName& ex) { // resolve_initial_references
     cerr<<"Failed to "<<action<<". ORB::InvalidName"<<endl;
     exit(1);
  }
  catch(CosNaming::NamingContext::InvalidName& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::InvalidName"<<endl;
     exit(1);
  }
  catch(CosNaming::NamingContext::NotFound& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::NotFound"<<endl;
     exit(1);
  }
  catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::CannotProceed"<<endl;
     exit(1);
  }
  catch(CORBA::TRANSIENT& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". TRANSIENT"<<endl;
     exit(1);
  }
  catch(CORBA::OBJECT_NOT_EXIST& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". OBJECT_NOT_EXIST"<<endl;
     exit(1);
  }
  catch(CORBA::SystemException& ex) {
     cerr<<"Failed to "<<action<<".";
#if defined(HAVE_OMNIORB4)
     cerr<<" "<<ex._name();
     if(ex.NP_minorString())
         cerr<<" ("<<ex.NP_minorString()<<")";
#endif
     cerr<<endl;
     exit(1);
  }
  catch(CORBA::Exception& ex) {
     cerr<<"Failed to "<<action<<"."
#if defined(HAVE_OMNIORB4)
       " "<<ex._name()
#endif
       <<endl;
     exit(1);
  }

  //
  // Get Supplier Admin interface - retrying on Comms Failure.
  CosEventChannelAdmin::SupplierAdmin_var supplier_admin;
  while (1)
  {
     try {
        supplier_admin = channel->for_suppliers ();
        if (CORBA::is_nil(supplier_admin))
        {
           cerr << "Event Channel returned nil Supplier Admin!"
                << endl;
           exit(1);
        }
        break;
     }
     catch (CORBA::COMM_FAILURE& ex) {
        cerr << "Caught COMM_FAILURE Exception "
             << "obtaining Supplier Admin! Retrying..."
             << endl;
        continue;
     }
  }
  cout << "Obtained SupplierAdmin." << endl;

  while (1)
  {
     //
     // Get proxy consumer - retrying on Comms Failure.
     CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer;
     while (1)
     {
        try {
           proxy_consumer = supplier_admin->obtain_push_consumer ();
           if (CORBA::is_nil(proxy_consumer))
           {
              cerr << "Supplier Admin returned nil proxy_consumer!"<< endl;
              exit(1);
           }
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE Exception "
                << "obtaining Proxy Push Consumer! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Obtained ProxyPushConsumer." << endl;

     //
     // Connect Push Supplier - retrying on Comms Failure.
     CosEventComm::PushSupplier_var sptr =CosEventComm::PushSupplier::_nil();
     if (! refnil) {
        sptr = supplier->_this();
     }

     while (1)
     {
        try {
           proxy_consumer->connect_push_supplier(sptr.in());
           break;
        }
        catch (CORBA::BAD_PARAM& ex) {
           cerr << "Caught BAD_PARAM Exception connecting Push Supplier!"
                << endl;
           exit (1);
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
           cerr << "Proxy Push Consumer already connected!"
                << endl;
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE Exception "
                << "connecting Push Supplier! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Connected Push Supplier." << endl;

     // Push data.
     for (int i=0; (discnum == 0) || (i < discnum); i++)
     {
        CORBA::Any any;
        any <<= (CORBA::ULong) l++;
        try {
           cout << "Push Supplier: push() called. " << flush;
           proxy_consumer->push(any);
           cout << "Data : " << l-1 << endl;
        }
        catch(CosEventComm::Disconnected&) {
           cout << "Failed. Caught Disconnected Exception!" << endl;
        }
        catch(CORBA::COMM_FAILURE&) {
           cout << "Failed. Caught COMM_FAILURE Exception!" << endl;
        }
     }

     // Disconnect - retrying on Comms Failure.
     while (1)
     {
        try {
           proxy_consumer->disconnect_push_consumer();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE Exception "
                << "disconnecting Push Supplier! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "ProxyPushConsumer disconnected." << endl;

     // Yawn.
     cout << "Sleeping " << sleepInterval << " seconds." << endl;
     omni_thread::sleep(sleepInterval);
  }

  // Not Reached
  return 0;
}

static void
usage(int argc, char **argv)
{
  cerr<<
"\nCreate a PushSupplier to send events to a channel.\n"
"syntax: "<<(argc?argv[0]:"pushsupp")<<" OPTIONS [CHANNEL_URI]\n"
"\n"
"CHANNEL_URI: The event channel may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -d NUM   disconnect after sending NUM events     [0 - never disconnect]\n"
" -r       connect using a nil reference\n"
" -s SECS  sleep SECS seconds after disconnecting  [0]\n"
" -n NAME  channel name (if URI is not specified)  [\"EventChannel\"]\n"
" -h       display this help text\n" << endl;
}
