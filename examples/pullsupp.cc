// -*- Mode: C++; -*-
//                            Package   : omniEvents
//   pullsupp.cc              Created   : 1/4/98
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2003-2004 Alex Tingle
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
//    Pull Model supplier implementation.
//	

/*
  $Log: pullsupp.cc,v $
  Revision 1.9  2004/10/08 09:06:08  alextingle
  More robust exception minor code handling.

  Revision 1.8  2004/08/18 17:49:45  alextingle
  Added check for SIGPIPE before trying to use it.

  Revision 1.7  2004/08/06 16:19:23  alextingle
  -k & -K options removed.
  Naming service names may now be as complex as you like.

  Revision 1.6  2004/04/20 16:52:05  alextingle
  All examples updated for latest version on omniEvents. Server may now be
  specified as a 'corbaloc' string or IOR, instead of as naming service id/kind.

  Revision 1.5  2004/02/04 22:29:55  alextingle
  Reworked all C++ examples.
  Removed catch(...) as it tends to make it harder to see what's going on.
  Now uses POA instead of BOA.
  Uses omniORB4's Exception name probing.
  No longer uses 'naming.h/cc' utility code.

  Revision 1.4  2003/11/17 08:47:09  alextingle
  Corrected typo.

  Revision 1.2  2003/11/16 23:24:22  alex
  Corrected typo.

  Revision 1.1.1.1  2003/11/11 22:28:56  alex
  Import of testing 2.3.0

  Revision 1.3  2003/11/03 22:20:26  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.
  Removed stub header in order to allow makefile dependency checking to work
  correctly.

  Revision 1.1.1.1.2.1  2002/09/28 22:20:51  shamus13
  Added ifdefs to enable omniEvents to compile
  with both omniORB3 and omniORB4. If __OMNIORB4__
  is defined during compilation, omniORB4 headers
  and command line option syntax is used, otherwise
  fall back to omniORB3 style.

  Revision 1.1.1.1  2002/09/25 19:00:26  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 0.11  2000/08/30 04:39:48  naderp
  Port to omniORB 3.0.1.

  Revision 0.10  2000/03/16 05:37:27  naderp
  Added stdlib.h for getopt.

  Revision 0.9  2000/03/06 13:26:32  naderp
  Using util getRootNamingContext function.
  Using stub headers.
  Fixed error messages.

  Revision 0.8  2000/03/02 03:16:26  naderp
  Added retry resiliency for handling COMM_FAUILURE exceptions.
  Replaced condition variable by counting semaphore.

  Revision 0.7  1999/11/02 13:39:13  naderp
  Added <signal.h>

  Revision 0.6  1999/11/02 07:57:22  naderp
  Updated usage.

Revision 0.5  99/11/01  16:10:12  16:10:12  naderp (Paul Nader)
omniEvents 2.0 Release.
Ignoring SIGPIPE for UNIX platforms.

Revision 0.4  99/04/23  16:05:44  16:05:44  naderp (Paul Nader)
gcc port.

Revision 0.3  99/04/23  09:34:01  09:34:01  naderp (Paul Nader)
Windows Port.

Revision 0.2  99/04/21  18:06:25  18:06:25  naderp (Paul Nader)
*** empty log message ***

Revision 0.1.1.1  98/11/27  16:59:33  16:59:33  naderp (Paul Nader)
Added -s option to sleep after disconnecting.

Revision 0.1  98/11/25  14:08:07  14:08:07  naderp (Paul Nader)
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

static omni_semaphore connect_cond(0);
static void usage(int argc, char **argv);

class Supplier_i : virtual public POA_CosEventComm::PullSupplier {
public:
  Supplier_i (long disconnect = 0) : i(0), _disconnect(disconnect), l(0) {};
  CORBA::Any *pull();
  CORBA::Any *try_pull(CORBA::Boolean &has_event);
  void disconnect_pull_supplier ();

private:
  long i;
  long _disconnect;
  CORBA::ULong l;
};

void
Supplier_i::disconnect_pull_supplier () {
  cout << "Pull Supplier: disconnected by channel." << endl;
}

CORBA::Any *
Supplier_i::pull() {
   cout << "Pull Supplier: pull() called. Data : ";
   CORBA::Any *any = new CORBA::Any();
   *any <<= l++;
   cout << l-1 << endl;

   // Exercise Disconnect
   if ((_disconnect > 0) && (i == _disconnect)) {
      i = 0;
      // Signal main thread to disconnect and re-connect.
      connect_cond.post();
   }
   i++;
   return (any);
}

CORBA::Any *
Supplier_i::try_pull(CORBA::Boolean &has_event)
{
   cout << "Pull Supplier: try_pull() called. Data : ";
   CORBA::Any *any = new CORBA::Any();
   *any <<= l++;
   cout << l-1 << endl;
   has_event = 1;

   // Exercise Disconnect
   if ((_disconnect > 0) && (i == _disconnect)) {
      i = 0;
      // Signal main thread to disconnect and re-connect.
      connect_cond.post();
   }
   i++;
   return (any);
}


int
main (int argc, char** argv)
{
#if defined(HAVE_OMNIORB4)
  CORBA::ORB_var orb =CORBA::ORB_init(argc,argv,"omniORB4");
#else
  CORBA::ORB_var orb =CORBA::ORB_init(argc,argv,"omniORB3");
#endif

  // Process Options
  int         discnum       =0;
  int         sleepInterval =0;
  const char* channelName   ="EventChannel";

  int c;
  while ((c = getopt(argc,argv,"d:s:n:h")) != EOF)
  {
     switch (c)
     {
        case 'd': discnum = atoi(optarg);
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

  Supplier_i* supplier = new Supplier_i (discnum);
  CosEventChannelAdmin::EventChannel_var channel;

  const char* action=""; // Use this variable to help report errors.
  try {
    CORBA::Object_var obj;

    action="resolve initial reference 'RootPOA'";
    obj=orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var rootPoa =PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(rootPoa))
        throw CORBA::OBJECT_NOT_EXIST();

    action="activate the RootPOA's POAManager";
    PortableServer::POAManager_var pman =rootPoa->the_POAManager();
    pman->activate();

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
        cerr << "Caught COMM_FAILURE exception "
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
     CosEventChannelAdmin::ProxyPullConsumer_var proxy_consumer;
     while (1)
     {
        try {
           proxy_consumer = supplier_admin->obtain_pull_consumer ();
           if (CORBA::is_nil(proxy_consumer))
           {
              cerr << "Supplier Admin returned nil proxy_consumer!"
                   << endl;
              exit(1);
           }
	   break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE exception "
                << "obtaining Proxy Pull Consumer! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Obtained ProxyPullConsumer." << endl;
   
     // Connect Pull Supplier - retrying on Comms Failure.
     CosEventComm::PullSupplier_var supplierRef =supplier->_this();
     while (1)
     {
        try {
           proxy_consumer->connect_pull_supplier(supplierRef.in());
           break;
        }
        catch (CORBA::BAD_PARAM& ex) {
           cerr<<"Caught BAD_PARAM Exception connecting Pull Supplier!"<<endl;
           exit(1);
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
           cerr << "Pull Supplier already connected!"
                << endl;
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE exception "
                << "connecting Pull Supplier! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Connected Pull Supplier." << endl;

     // Wait for indication to disconnect before re-connecting.
     connect_cond.wait();

     // Disconnect - retrying on Comms Failure.
     while (1)
     {
        try {
           proxy_consumer->disconnect_pull_consumer();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE exception "
                << "disconnecting Pull Supplier! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Disconnected Pull Supplier." << endl;

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
"\nCreate a PullSupplier to send events to a channel.\n"
"syntax: "<<(argc?argv[0]:"pullsupp")<<" OPTIONS [CHANNEL_URI]\n"
"\n"
"CHANNEL_URI: The event channel may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -d NUM   disconnect after sending NUM events     [0 - never disconnect]\n"
" -s SECS  sleep SECS seconds after disconnecting  [0]\n"
" -n NAME  channel name (if URI is not specified)  [\"EventChannel\"]\n"
" -h       display this help text\n" << endl;
}
