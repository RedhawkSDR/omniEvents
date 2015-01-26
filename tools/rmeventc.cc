//                            Package   : omniEvents
// rmeventc.cc                Created   : 2003/12/21
//                            Author    : Alex Tingle
//
//    Copyright (C) 2003 Alex Tingle
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
//    Destroys the named EventChannel.
//	

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

#ifdef HAVE_STDLIB_H
#  include <stdlib.h> // exit()
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include <cstdio>

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include "CosEventChannelAdmin.hh"
#include "naming.h"

static void usage(int argc, char **argv);

int
main(int argc, char **argv)
{
  int result =1;

  //
  // Start orb.
#if defined(HAVE_OMNIORB4)
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB4");
#else
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB3");
#endif

  // Process Options
  int c;

  CosNaming::Name ecName =str2name("EventChannel");

  while ((c = getopt(argc,argv,"n:h")) != EOF)
  {
     switch (c)
     {
        case 'n': ecName=str2name(optarg);
                  break;

        case 'h': usage(argc,argv);
                  exit(0);

        default : usage(argc,argv);
                  exit(-1);
     }
  }

  //
  // Use one big try...catch block.
  // 'action' variable keeps track of what we're doing.
  const char* action ="start";
  try
  {
    CORBA::Object_var obj;
    
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
      //
      // Get Name Service root context.
      action="resolve initial reference 'NameService'";
      obj=orb->resolve_initial_references("NameService");
      CosNaming::NamingContext_var rootContext=
        CosNaming::NamingContext::_narrow(obj);
      if(CORBA::is_nil(rootContext))
          throw CORBA::OBJECT_NOT_EXIST();

      //
      // Obtain reference to the Event Channel.
      action="find Event Channel in naming service";
      obj=rootContext->resolve(ecName);

      //
      // Unbind the Channel's reference in the naming service.
      action="unbind Event Channel from naming service";
      rootContext->unbind(ecName);
    }
    
    action="narrow object reference to event channel";
    CosEventChannelAdmin::EventChannel_var channel =
      CosEventChannelAdmin::EventChannel::_narrow(obj);
    if(CORBA::is_nil(channel))
        throw CORBA::OBJECT_NOT_EXIST();
    
    //
    // Destroy the EventChannel.
    action="destroy Event Channel";
    channel->destroy();

    //
    // Clean up nicely.
    action="destroy orb";
    orb->destroy();

    //
    // If we get here, then everything has worked OK.
    result=0;

  }
  catch(CORBA::ORB::InvalidName& ex) { // resolve_initial_references
     cerr<<"Failed to "<<action<<". ORB::InvalidName"<<endl;
  }
  catch(CosNaming::NamingContext::InvalidName& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::InvalidName"<<endl;
  }
  catch(CosNaming::NamingContext::NotFound& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::NotFound"<<endl;
  }
  catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
     cerr<<"Failed to "<<action<<". NamingContext::CannotProceed"<<endl;
  }
  catch(CORBA::TRANSIENT& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". TRANSIENT"<<endl;
  }
  catch(CORBA::OBJECT_NOT_EXIST& ex) { // _narrow()
     cerr<<"Failed to "<<action<<". OBJECT_NOT_EXIST"<<endl;
  }
  catch(CORBA::SystemException& ex) {
     cerr<<"Failed to "<<action<<".";
#if defined(HAVE_OMNIORB4)
     cerr<<" "<<ex._name();
     if(ex.NP_minorString())
         cerr<<" ("<<ex.NP_minorString()<<")";
#endif
     cerr<<endl;
  }
  catch(CORBA::Exception& ex) {
     cerr<<"Failed to "<<action<<"."
#if defined(HAVE_OMNIORB4)
       " "<<ex._name()
#endif
       <<endl;
  }

  return result;
}

static void
usage(int argc, char **argv)
{
  cerr<<
"\nDestroy an EventChannel.\n"
"syntax: "<<(argc?argv[0]:"rmeventc")<<" OPTIONS [CHANNEL_URI]\n"
"\n"
"CHANNEL_URI: The event channel may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -n NAME  channel name (if URI is not specified)  [\"EventChannel\"]\n"
" -h       display this help text\n" << endl;
}

