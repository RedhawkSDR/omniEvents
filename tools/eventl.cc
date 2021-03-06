/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 * Description:
 *     list all event channels managed by the omniEvents service
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

#ifdef HAVE_STDLIB_H
#  include <stdlib.h> // exit()
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include <stdio.h>
#include "omniEvents.hh"
#include "naming.h"

static void usage(int argc, char **argv);

int
main(int argc, char **argv)
{
  int result =1;
  bool needNameService =false;
  const char* factoryName ="EventChannelFactory";
  bool details=false;
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

  while ((c = getopt(argc,argv,"dh")) != EOF)
  {
    switch (c)
      {
      case 'd':
        details = true;
        break;

      case 'h': usage(argc,argv);
        exit(0);

      default : usage(argc,argv);
        exit(-1);
      }
  }

 // Need the naming service to find the factory if there is no URI argument.
  needNameService=(needNameService || optind>=argc);

  //
  // Use one big try...catch block.
  // 'action' variable keeps track of what we're doing.
  const char* action ="start";
  try
  {
    CORBA::Object_var obj;
    //
    // Get Name Service root context.(we can carry on without it though)
    CosNaming::NamingContext_var rootContext=CosNaming::NamingContext::_nil();
    try {
      action="resolve initial reference 'NameService'";
      obj=orb->resolve_initial_references("NameService");
      rootContext=CosNaming::NamingContext::_narrow(obj);
      if(CORBA::is_nil(rootContext))
          throw CORBA::OBJECT_NOT_EXIST();
      cerr<<"Resolved Naming service..." << std::endl;
    }
    catch (CORBA::Exception& ex) {
       if(needNameService)
           throw;
       else
           cerr<<"Warning - failed to "<<action<<"."<<endl;
    }

    //
    // Obtain reference to the Event Channel Factory implementation.
    // (from command-line argument or from the Naming Service).
    if(optind<argc)
    {
      action="convert URI from command line into object reference";
      obj=orb->string_to_object(argv[optind]);
    }
    else
    {
      action="find Event Channel Factory in naming service";
      obj=rootContext->resolve(str2name(factoryName));
      if(CORBA::is_nil(obj)) {
        std::cout << "NamingService failed to find EventChannelFactory...." <<std::endl;
        exit(1);
      }
    }

    action="narrow object reference to event channel factory";
    omniEvents::EventChannelFactoryExt_var factory =
      omniEvents::EventChannelFactoryExt::_narrow(obj);
    if(CORBA::is_nil(factory))
    {
      cerr << "Failed to narrow Event Channel Factory reference." << endl;
      exit(1);
    }

    cerr<<"Resolved EventChannelFactoryExt..." << std::endl;

    try {
        omniEvents::EventChannelInfoIterator_var eiter;
        omniEvents::EventChannelInfoList_var     elist;
        cerr<<"Grabbing list of channels..." << std::endl;
        factory->list_channels(0, elist, eiter );
        omniEvents::EventChannelInfo_var einfo;
        if ( !CORBA::is_nil(eiter) ) {
          while ( eiter->next_one( einfo ) == true ) {
            std::cout << "CHANNEL : "<< einfo->channel_name ;
            if ( einfo->has_mapper == false ) std::cout << " (unmapped) ";
            std::cout << std::endl;

            if ( details )  {
              std::cout << "  has mapper: "<<einfo->has_mapper << std::endl;
              std::cout << "  pull_retry_period_ms: "<<einfo->pull_retry_period << std::endl;
              std::cout << "  max_queue_length: "<<einfo->max_queue_length<< std::endl;
              std::cout << "  max_num_proxies: "<<einfo->max_num_proxies<< std::endl;
              std::cout << "  cycle_period_ns: "<<einfo->cycle_period << std::endl;
              std::cout << "  filter id = " << einfo->filter_id << std::endl;
            }
          }
        }
      }
      catch(...) {
        std::cerr << " Listing of event channels failed..." << std::endl;
      }

    //

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
"\nList All EventChannel.\n"
"syntax: "<<(argc?argv[0]:"eventl")<<" OPTIONS [FACTORY_URI]\n"
"\n"
"FACTORY_URI: The factory may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
" For example: corbaloc::localhost:11169/omniEvents\n"
"OPTIONS:                                         DEFAULT:\n"
" -d       provide detailed channel information\n"
" -h       display this help text\n" << endl;
}

