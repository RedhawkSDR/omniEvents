//                            Package   : omniEvents
//   channel.cc               Created   : 2005/04/23
//                            Author    : Alex Tingle
//
//    Copyright (C) 2005 Alex Tingle
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
//    Demonstates how to make a standalone EventChannel in your own
//    application, using libomniEvents.
//	


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <signal.h>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include <omniEvents/EventChannel.h>

/** Signal handler. */
void myShutdown(int signum)
{
  OmniEvents::Orb::inst().shutdown(signum);
}

int main(int argc, char **argv)
{
  //
  // Start orb.
  CORBA::ORB_var orb = CORBA::ORB_init(argc,argv);

  const char* action=""; // Use this variable to help report errors.
  try {

    action="initialise OmniEvents::Orb";
    // Your code MUST include these two lines.
    OmniEvents::Orb::inst()._orb=orb;
    OmniEvents::Orb::inst().resolveInitialReferences();

    action="activate the RootPOA's POAManager";
    // You MUST activate the RootPOA's POAManager. You can do this yourself
    // in the normal way, or you can use the reference that OmniEvents::Orb
    // has resolved for you.
    PortableServer::POAManager_var pman;
    pman=OmniEvents::Orb::inst()._RootPOA->the_POAManager();
    pman->activate();

    action="create EventChannel servant";
    // The constructor just allocates memory.
    OmniEvents::EventChannel_i* channelSrv =new OmniEvents::EventChannel_i();

    action="activate EventChannel servant";
    // activate() creates & activates the EventChannel's POA and CORBA objects.
    channelSrv->activate("MyChannel");

    // From this point, clients may invoke EventChannel operations.
    
    action="obtain an object reference to the EventChannel";
    CosEventChannelAdmin::EventChannel_var channelRef =channelSrv->_this();

    // The user interface of this example is simple: The EventChannel's IOR
    // is dumped to the standard output stream.
    action="stringify the EventChannel reference";
    CORBA::String_var sior =orb->object_to_string(channelRef.in());
    cout<<sior.in()<<endl;

    action="set signal handlers";
    ::signal(SIGINT , ::myShutdown);
    ::signal(SIGTERM, ::myShutdown);
 
    action="collect orphan requests";
    // You MUST call this method, it processes orphan (asynchronous) method
    // calls made by the EventChannel.
    // You can safely call it instead of CORBA::ORB::run(). If you do not
    // want to park the main thread, then you must create a new thread for this
    // method.
    OmniEvents::Orb::inst().run();

    // OmniEvents::Orb::shutdown() has been called by the myShutdown() signal
    // handler. (The user pressed Ctrl-C or killed the process.)

    // In order to make run() return, you MUST call OmniEvents::Orb::shutdown().

    action="destroy orb";
    orb->destroy();

  }
  catch(CORBA::SystemException& ex) {
     cerr<<"Failed to "<<action<<".";
#if defined(HAVE_OMNIORB4)
     cerr<<" "<<ex._name();
     if(ex.NP_minorString())
         cerr<<" ("<<ex.NP_minorString()<<")";
#endif
     cerr<<endl;
     ::exit(1);
  }
  catch(CORBA::Exception& ex) {
     cerr<<"Failed to "<<action<<"."
#if defined(HAVE_OMNIORB4)
       " "<<ex._name()
#endif
       <<endl;
     ::exit(1);
  }

  return 0;
}
