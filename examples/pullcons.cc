// -*- Mode: C++; -*-
//                            Package   : omniEvents
// pullcons.cc                Created on: 1/4/98
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
//    Pull Model consumer implementation.
//	

/*
  $Log: pullcons.cc,v $
  Revision 1.12  2004/10/08 09:06:08  alextingle
  More robust exception minor code handling.

  Revision 1.11  2004/08/18 17:49:45  alextingle
  Added check for SIGPIPE before trying to use it.

  Revision 1.10  2004/08/06 16:19:23  alextingle
  -k & -K options removed.
  Naming service names may now be as complex as you like.

  Revision 1.9  2004/04/20 16:52:02  alextingle
  All examples updated for latest version on omniEvents. Server may now be
  specified as a 'corbaloc' string or IOR, instead of as naming service id/kind.

  Revision 1.8  2004/03/08 18:00:13  alextingle
  One too many newlines.

  Revision 1.7  2004/03/08 17:48:25  alextingle
  Added newlines to cirrectly format output when exceptions occur.

  Revision 1.6  2004/02/21 19:07:45  alextingle
  Corrected servants to use POA instead of BOA.

  Revision 1.5  2004/02/04 22:29:55  alextingle
  Reworked all C++ examples.
  Removed catch(...) as it tends to make it harder to see what's going on.
  Now uses POA instead of BOA.
  Uses omniORB4's Exception name probing.
  No longer uses 'naming.h/cc' utility code.

  Revision 1.4  2004/01/11 16:59:28  alextingle
  Added more helpful exception handling error messages to pullcons.cc.

  Revision 1.3  2003/11/03 22:20:54  alextingle
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

  Revision 1.1.1.1  2002/09/25 19:00:25  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 0.11  2000/10/11 01:16:21  naderp
  *** empty log message ***

  Revision 0.10  2000/08/30 04:39:48  naderp
  Port to omniORB 3.0.1.

  Revision 0.9  2000/03/16 05:37:27  naderp
  Added stdlib.h for getopt.

  Revision 0.8  2000/03/06 13:25:44  naderp
  Using util getRootNamingContext function.
  Using stub headers.
  Fixed error messages.

  Revision 0.7  2000/03/02 02:11:27  naderp
  Added -r option to connect using nil reference.
  Added retry resiliency for handling COMM_FAUILURE exceptions.

  Revision 0.6  1999/11/02 13:38:57  naderp
  Added <signal.h>

  Revision 0.5  1999/11/01 15:55:11  naderp
  omniEvents 2.0 Release.
  Ignoring SIGPIPE for UNIX platforms.

Revision 0.4  99/04/23  16:05:38  16:05:38  naderp (Paul Nader)
gcc port.

Revision 0.3  99/04/23  09:33:40  09:33:40  naderp (Paul Nader)
Windows Port.

Revision 0.2  99/04/21  18:06:25  18:06:25  naderp (Paul Nader)
*** empty log message ***

Revision 0.1.1.1  98/11/27  16:59:07  16:59:07  naderp (Paul Nader)
Added -s option to sleep after disconnecting.

Revision 0.1  98/11/25  14:08:04  14:08:04  naderp (Paul Nader)
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

class Consumer_i : virtual public POA_CosEventComm::PullConsumer {
public:
  Consumer_i () {};
  void disconnect_pull_consumer ();
};

void Consumer_i::disconnect_pull_consumer () {
  cout << "Pull Consumer: disconnected." << endl;
}

int
main(int argc, char **argv)
{
  //
  // Start orb.
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv);

  // Process Options
  bool        trymode       =false;
  int         discnum       =0;
  bool        refnil        =false;
  int         sleepInterval =0;
  const char* channelName   ="EventChannel";

  int c;
  while ((c = getopt(argc,argv,"td:rs:n:h")) != EOF)
  {
     switch (c)
     {
        case 't': trymode = true;
                  break;

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

  Consumer_i* consumer =NULL;
  CosEventChannelAdmin::EventChannel_var channel;

  const char* action=""; // Use this variable to help report errors.
  try {
    CORBA::Object_var obj;

    // A Pull Consumer can be implemented as a pure client or as a mixed
    // client-server process, depending on whether it requires and is
    // prepared to service disconnect requests from the channel.
    // If it is, then create the servant object and activate the POA.
    if (! refnil)
    {
       consumer=new Consumer_i();

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
  // Get Consumer admin interface - retrying on Comms Failure.
  CosEventChannelAdmin::ConsumerAdmin_var consumer_admin;
  while (1)
  {
     try {
        consumer_admin = channel->for_consumers ();
        if (CORBA::is_nil (consumer_admin))
        {
           cerr << "Event Channel returned nil Consumer Admin!" << endl;
           exit (1);
        }
        break;
     }
     catch (CORBA::COMM_FAILURE& ex) {
        cerr << "Caught COMM_FAILURE exception "
             << "obtaining Consumer Admin! Retrying..."
             << endl;
        continue;
     }
  }
  cout << "Obtained Consumer Admin." << endl;

  while (1)
  {
     //
     // Get proxy supplier - retrying on Comms Failure.
     CosEventChannelAdmin::ProxyPullSupplier_var proxy_supplier;
     while (1)
     {
        try {
           proxy_supplier = consumer_admin->obtain_pull_supplier ();
           if (CORBA::is_nil (proxy_supplier))
           {
              cerr << "Consumer Admin returned nil Proxy Supplier!" << endl;
              exit (1);
           }
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE Exception "
                << "obtaining Pull Supplier! Retrying..."
                << endl;
           continue;
        }
     }
     cout << "Obtained ProxyPullSupplier." << endl;
   
     //
     // Connect Pull Consumer - retrying on Comms Failure.
     CosEventComm::PullConsumer_ptr cptr =CosEventComm::PullConsumer::_nil();
     if (! refnil) {
        cptr=consumer->_this();
     }

     while (1)
     {
        try {
           proxy_supplier->connect_pull_consumer(cptr);
           break;
        }
        catch (CORBA::BAD_PARAM& ex) {
           cerr << "Caught BAD_PARAM exception connecting Pull Consumer!"<<endl;
           exit (1);
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
           cerr << "Proxy Pull Supplier already connected!" 
                << endl;
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE Exception "
                << "connecting Pull Consumer! Retrying..." 
                << endl;
           continue;
        }
     }
     cout << "Connected Pull Consumer." << endl;

     // Pull data.
     CORBA::Any *data;
     CORBA::ULong l = 0;
     for (int i=0; (discnum == 0) || (i < discnum); i++)
     {
        if(trymode)
        {
           try {
               CORBA::Boolean has_event;
               data = proxy_supplier->try_pull(has_event);
               cout << "Consumer: try_pull() called. Data : " << flush;
               if (has_event)
               {
                  l = 0;
                  *data >>= l;
                  cout << l << endl;
                  delete data;
               }
               else
               {
                  cout << "None" << endl;
               }
           }
           catch (CosEventComm::Disconnected& ex) {
              cout << endl;
              cerr << "Failed. Caught Disconnected Exception !" << endl;
           }
           catch (CORBA::COMM_FAILURE& ex) {
              cout << endl;
              cerr << "Failed. Caught COMM_FAILURE Exception !" << endl;
           }
           catch (CORBA::Exception& ex) {
              cout << endl;
              cerr<<"CORBA exception, unable to try_pull()"
#ifdef HAVE_OMNIORB4
                  <<": "<<ex._name()
#endif
                  << endl;
           }
        }
        else
        {
           try {
               cout << "Pull Consumer: pull() called. ";
               cout.flush();
               data = proxy_supplier->pull();
               l = 0;
               *data >>= l;
               cout << "Data : " << l << endl;
               delete data;
           }
           catch(CORBA::TRANSIENT&) {
              cout << "caught TRANSIENT." << endl;
              omni_thread::sleep(1);
           }
           catch (CosEventComm::Disconnected& ex) {
              cout << endl;
              cerr << "Failed. Caught Disconnected exception!" << endl;
              exit(1);
           }
           catch (CORBA::COMM_FAILURE& ex) {
              cout << endl;
              cerr << "Failed. Caught COMM_FAILURE exception!" << endl;
              exit(1);
           }
           catch (CORBA::SystemException& ex) {
              cout << endl;
              cerr<<"System exception, unable to pull()";
#ifdef HAVE_OMNIORB4
              cerr<<": "<<ex._name();
              if(ex.NP_minorString())
                  cerr<<" ("<<ex.NP_minorString()<<")";
#endif
              cerr<< endl;
              exit(1);
           }
           catch (CORBA::Exception& ex) {
              cout << endl;
              cerr<<"CORBA exception, unable to pull()"
#ifdef HAVE_OMNIORB4
                  <<": "<<ex._name()
#endif
                  << endl;
              exit(1);
           }
        }
     }

     // Disconnect - retrying on Comms Failure.
     while (1)
     {
        try {
           proxy_supplier->disconnect_pull_supplier();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
           cerr << "Caught COMM_FAILURE exception "
                << "disconnecting Pull Consumer! Retrying..." 
                << endl;
           continue;
        }
     }
     cout << "Disconnected Pull Consumer." << endl;

     // Yawn
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
"\nCreate a PullConsumer to receive events from a channel.\n"
"syntax: "<<(argc?argv[0]:"pullcons")<<" OPTIONS [CHANNEL_URI]\n"
"\n"
"CHANNEL_URI: The event channel may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -t       enable try_pull mode\n"
" -r       connect using a nil reference\n"
" -d NUM   disconnect after receiving NUM events   [0 - never disconnect]\n"
" -s SECS  sleep SECS seconds after disconnecting  [0]\n"
" -n NAME  channel name (if URI is not specified)  [\"EventChannel\"]\n"
" -h       display this help text\n" << endl;
}
