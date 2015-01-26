//                            Package   : omniEvents
// eventf.cc                  Created   : 2004-05-30
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle
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

static void usage(int argc, char **argv);
static CosEventChannelAdmin::EventChannel_ptr getChannel(const char* sior);

CORBA::ORB_ptr orb;

int
main(int argc, char **argv)
{
  int result =1;

  //
  // Start orb.
#if defined(HAVE_OMNIORB4)
  orb=CORBA::ORB_init(argc,argv,"omniORB4");
#else
  orb=CORBA::ORB_init(argc,argv,"omniORB3");
#endif

  // Process Options
  int c;

  while((c = getopt(argc,argv,"h")) != EOF)
  {
    switch (c)
    {
      case 'h': usage(argc,argv);
                exit(0);

      default : usage(argc,argv);
                exit(-1);
    }
  }

  if(optind!=argc-2)
  {
    usage(argc,argv);
    exit(-1);
  }

  //
  // Use one big try...catch block.
  // 'action' variable keeps track of what we're doing.
  const char* action ="start";
  try
  {
    using namespace CosEventChannelAdmin;

    action="convert URI into reference to source channel";
    EventChannel_var from_channel =getChannel(argv[optind]);

    action="convert URI into reference to destination channel";
    EventChannel_var to_channel   =getChannel(argv[optind+1]);

    action="obtain ConsumerAdmin";
    ConsumerAdmin_var cadmin =from_channel->for_consumers();

    action="obtain ProxyPushSupplier";
    ProxyPushSupplier_var supplier =cadmin->obtain_push_supplier();

    action="obtain SupplierAdmin";
    SupplierAdmin_var sadmin =to_channel->for_suppliers();

    action="obtain ProxyPushConsumer";
    ProxyPushConsumer_var consumer =sadmin->obtain_push_consumer();

    action="connect PushConsumer";
    consumer->connect_push_supplier(supplier.in());

    action="connect PushSupplier";
    supplier->connect_push_consumer(consumer.in());

    //
    // Clean up nicely.
    action="destroy orb";
    orb->destroy();

    //
    // If we get here, then everything has worked OK.
    result=0;

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
"\nConnect (federate) two event channels.\n"
"syntax: "<<(argc?argv[0]:"eventf")<<" OPTIONS [FROM_CHANNEL] [TO_CHANNEL]\n"
"\n"
"FROM/TO_CHANNEL: The event channels must be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
"\n"
"OPTIONS:\n"
" -h  display this help text\n" << endl;
}


//
// Obtain object reference to EventChannel
static CosEventChannelAdmin::EventChannel_ptr
getChannel(const char* sior)
{
  // convert URI from command line into object reference";
  CORBA::Object_var obj =orb->string_to_object(sior);

  // narrow object reference to event channel";
  CosEventChannelAdmin::EventChannel_var channel =
    CosEventChannelAdmin::EventChannel::_narrow(obj);
  if(CORBA::is_nil(channel))
      throw CORBA::OBJECT_NOT_EXIST();
  
  return channel._retn();
}
