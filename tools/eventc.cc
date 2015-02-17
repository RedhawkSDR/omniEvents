// -*- Mode: C++; -*-
//                            Package   : omniEvents
// eventc.cc                  Created   : 1/4/98
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
//    Client to the event channel factory. Requests creation of an event
//    channel and registers it with the Naming service.
//
//   Modified by REDHAWK (United States Government) - 2015

/*
  $Log: eventc.cc,v $
  Revision 1.4.2.2  2005/04/27 20:49:32  alextingle
  Merge across changes from HEAD branch (see CHANGES_262. Change version number ready for release 2.6.2.

  Revision 1.4.2.1  2004/11/01 12:27:12  alextingle
  New EventChannel parameter `PullRetryPeriod_ms' supercedes `PullRetryPeriod'.
  The new parameter is set by `eventc -R'. The old parameter and its `-r'
  option still work as before, for backwards compatibility.

  Revision 1.4  2004/10/08 09:06:19  alextingle
  More robust exception minor code handling.

  Revision 1.3  2004/08/06 16:16:40  alextingle
  Simplified call to ORB_init().

  Revision 1.2  2004/08/04 21:52:52  alextingle
  'n' & 'N' options now take a full name path. No more 'k' or 'K' options.

  Revision 1.1  2004/05/31 10:29:37  alextingle
  New 'tools' directory. Contains useful command line tools that previously lived in the 'examples' directory.

  Revision 1.14  2004/05/28 10:38:23  alextingle
  Now uses new omniEvents.idl header. Properly this time!

  Revision 1.13  2004/05/28 10:16:54  alextingle
  Now uses new omniEvents.idl header.

  Revision 1.12  2004/04/30 17:48:02  alextingle
  New 'real time push' feature: -t option.

  Revision 1.11  2004/04/21 10:23:46  alextingle
  If there is no Naming Service, eventc only issues a warning unless it is needed to find the factory, or options -n or -k are set.

  Revision 1.10  2004/04/20 16:51:59  alextingle
  All examples updated for latest version on omniEvents. Server may now be
  specified as a 'corbaloc' string or IOR, instead of as naming service id/kind.

  Revision 1.9  2004/03/28 00:58:05  alextingle
  New options. -c sets CyclePeriod_ns. -i sets the Channel's InsName.

  Revision 1.8  2004/03/26 16:06:30  alextingle
  Added verbose (-v) option that prints the new channel's IOR to standard out.

  Revision 1.7  2004/02/20 17:41:40  alextingle
  Moved 'endl;' to the actual end!!

  Revision 1.6  2004/02/20 14:01:54  alextingle
  New param: -p sets MaxNumProxies for omniEvents 2.5+.
  No longer sends parameters that are not explicitly set on
  the command line. This leaves the server to decide upon
  default values.

  Revision 1.5  2004/02/04 22:29:55  alextingle
  Reworked all C++ examples.
  Removed catch(...) as it tends to make it harder to see what's going on.
  Now uses POA instead of BOA.
  Uses omniORB4's Exception name probing.
  No longer uses 'naming.h/cc' utility code.

  Revision 1.4  2003/12/21 11:12:01  alextingle
  Most exceptions are now caught by a unified catch block.

  Revision 1.3  2003/11/03 22:21:21  alextingle
  Removed all platform specific switches. Now uses autoconf, config.h.
  Removed stub header in order to allow makefile dependency checking to work
  correctly.

  Revision 1.1.1.1.2.1  2002/09/28 22:20:51  shamus13
  Added ifdefs to enable omniEvents to compile
  with both omniORB3 and omniORB4. If __OMNIORB4__
  is defined during compilation, omniORB4 headers
  and command line option syntax is used, otherwise
  fall back to omniORB3 style.

  Revision 1.1.1.1  2002/09/25 19:00:25  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.6  2000/09/05 01:05:38  naderp
  Added MaxQueueLength QOS.

  Revision 1.5  2000/08/30 04:39:20  naderp
  Port to omniORB 3.0.1.

  Revision 1.4  2000/03/16 05:34:30  naderp
  Added stdlib.h for solaris getopt()

  Revision 1.3  2000/03/16 02:44:13  naderp
  Added iostream and signal headers.

  Revision 1.2  2000/03/06 13:23:50  naderp
  Using util getRootNamingContext function.
  Using stub headers.

  Revision 1.1  1999/11/01 20:37:42  naderp
  Updated usage statement.

Revision 1.0  99/11/01  17:05:13  17:05:13  naderp (Paul Nader)
omniEvents 2.0.
Added -m switch to support MaxEventsPerConsumer criteria.

Revision 0.6  99/08/27  11:48:22  11:48:22  naderp (Paul Nader)
Partitioned EventChannelFactory_i from CosEvent_i.

Revision 0.5  99/05/10  11:27:50  11:27:50  naderp (Paul Nader)
Initialised rootContext.

Revision 0.4  99/04/23  16:02:22  16:02:22  naderp (Paul Nader)
gcc port.

Revision 0.3  99/04/23  09:32:58  09:32:58  naderp (Paul Nader)
Windows Port.

Revision 0.2  99/04/21  18:06:23  18:06:23  naderp (Paul Nader)
*** empty log message ***

Revision 0.1.1.1  98/11/27  17:01:51  17:01:51  naderp (Paul Nader)
Enclosed supports call in try block to avoid core dump.
Added information messages for exceptions.

Revision 0.1  98/11/25  14:07:22  14:07:22  naderp (Paul Nader)
Initial Revision

*/

//
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

#include "omniEvents.hh"
#include "naming.h"

static void usage(int argc, char **argv);
static void appendCriterion(   CosLifeCycle::Criteria&,const char*,const char*);
static void appendCriterionStr(CosLifeCycle::Criteria&,const char*,const char*);

int
main(int argc, char **argv)
{
  int result =1;

  //
  // Start orb.
  CORBA::ORB_var orb = CORBA::ORB_init(argc,argv);

  // Process Options
  bool verbose =false;
  bool needNameService =false;
  const char* channelName ="EventChannel";
  const char* factoryName ="EventChannelFactory";
  CosLifeCycle::Criteria criteria;

  int c;
  while ((c = getopt(argc,argv,"n:N:m:c:i:p:q:R:r:t:vh")) != EOF)
  {
     switch (c)
     {
     case 'n':
       channelName=optarg;
       needNameService=true;
       break;

     case 'N':
       factoryName=optarg;
       break;

     case 'm': // OLD OPTION
       appendCriterion(criteria,"MaxEventsPerConsumer",optarg);
       break;

     case 'c':
       appendCriterion(criteria,"CyclePeriod_ns",optarg);
       break;

     case 'i':
       appendCriterionStr(criteria,"InsName",optarg);
       break;

     case 'p':
       appendCriterion(criteria,"MaxNumProxies",optarg);
       break;

     case 'q':
       appendCriterion(criteria,"MaxQueueLength",optarg);
       break;

     case 'R':
       appendCriterion(criteria,"PullRetryPeriod_ms",optarg);
       break;

     case 'r': // This option is deprecated in favour of -R:
       appendCriterion(criteria,"PullRetryPeriod",optarg);
       break;

     case 't':
       appendCriterionStr(criteria,"FilterId",optarg);
       break;

     case 'v':
       verbose=true;
       break;

     case 'h':
       usage(argc,argv);
       exit(0);

     default :
       usage(argc,argv);
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
    }

    action="narrow object reference to event channel factory";
    omniEvents::EventChannelFactory_var factory =
      omniEvents::EventChannelFactory::_narrow(obj);
    if(CORBA::is_nil(factory))
    {
       cerr << "Failed to narrow Event Channel Factory reference." << endl;
       exit(1);
    }

    // Check that the factory is of the right type
    action="check factory supports EventChannel object interface";
    CosLifeCycle::Key key;
    key.length (1);
    key[0].id = CORBA::string_dup("EventChannel");
    key[0].kind = CORBA::string_dup("object interface");

    if(!factory->supports(key))
    {
      cerr << "Factory does not support Event Channel Interface! [\""
           << factoryName << "\"]" << endl;
      exit(1);
    }

    //
    // Create Event Channel Object.
    action="create EventChannel object";
    CORBA::Object_var channelObj =factory->create_object(key, criteria);
    if (CORBA::is_nil(channelObj))
    {
       cerr << "Channel Factory returned nil reference! [\""
            << channelName << "\"]" << endl;
       exit(1);
    }

    // Narrow object returned to an Event Channel
    CosEventChannelAdmin::EventChannel_var channel =
      CosEventChannelAdmin::EventChannel::_narrow(channelObj);
    if (CORBA::is_nil(channel))
    {
       cerr << "Failed to narrow Event Channel! [\""
            << channelName << "\"]" << endl;
       exit(1);
    }

    // Print the new EventChannel's IOR to standard output.
    if(verbose)
    {
      CORBA::String_var sior =orb->object_to_string(channel);
      cout<<sior.in()<<endl;
    }

    //
    // Register event channel with naming service
    if(!CORBA::is_nil(rootContext))
    {
      CosNaming::Name name =str2name(channelName);
      try{
        action="register (bind) EventChannel with the naming service";
        rootContext->bind(name,channel.in());
      }
      catch(CosNaming::NamingContext::AlreadyBound& ex) {
        action="register (rebind) EventChannel with the naming service";
        rootContext->rebind(name,channel.in());
      }
    }

    //
    // Clean up nicely.
    action="destroy orb";
    orb->destroy();

    //
    // If we get here, then everything has worked OK.
    result=0;

  }
  catch (CosLifeCycle::NoFactory& ex) /* create_object() */ {
     cerr<<"Failed to create Event Channel: NoFactory"
       " (interface not supported) "<<endl;
  }
  catch (CosLifeCycle::CannotMeetCriteria& ex) /* create_object() */ {
     cerr<<"Failed to create Event Channel: CannotMeetCriteria "<<endl;
  }
  catch (CosLifeCycle::InvalidCriteria& ex) /* create_object() */ {
     cerr<<"Failed to create Event Channel: InvalidCriteria "<<endl;
  }
  catch (CORBA::COMM_FAILURE& ex) {
     cerr<<"System exception, unable to "<<action<<": COMM_FAILURE"<<endl;
  }
  catch (CORBA::SystemException& ex) {
     cerr<<"System exception, unable to "<<action;
#if defined(HAVE_OMNIORB4)
     cerr<<" "<<ex._name();
     if(ex.NP_minorString())
         cerr<<" ("<<ex.NP_minorString()<<")";
#endif
     cerr<<endl;
  }
  catch (CORBA::Exception& ex) {
     cerr<<"CORBA exception, unable to "<<action
#ifdef HAVE_OMNIORB4
         <<": "<<ex._name()
#endif
         << endl;
  }
  catch (omniORB::fatalException& ex) {
     cerr<<"Fatal Exception, unable to "<<action<<endl;
  }

  return result;
}

static void
usage(int argc, char **argv)
{
  cerr<<
"\nCreate an EventChannel and register it in the naming service.\n"
"syntax: "<<(argc?argv[0]:"eventc")<<" OPTIONS [FACTORY_URI]\n"
"\n"
"FACTORY_URI: The factory may be specified as a URI.\n"
" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
" For example: corbaloc::localhost:11169/omniEvents\n"
"\n"
"OPTIONS:                                         DEFAULT:\n"
" -n channel name                                  [\"EventChannel\"]\n"
" -N factory name (if URI is not specified)        [\"EventChannelFactory\"]\n"
" -c override default CyclePeriod_ns of new channel (nanoseconds)\n"
" -i set the InsName of new channel, to enable access via corbaloc\n"
" -p override default MaxNumProxies of new channel\n"
" -q override default MaxQueueLength of new channel\n"
" -R override default PullRetryPeriod_ms for new channel (milliseconds)\n"
" -t set an event type filter, FilterId=<RepositoryId>\n"
" -v print the IOR of the new EventChannel to standard output.\n"
" -h display this help text\n"
"OLD OPTIONS: (only used by omniEvents v2.4 and earlier)\n"
" -m override default MaxEventsPerConsumer for new channel\n" << endl;
}

static void appendCriterion(
  CosLifeCycle::Criteria& criteria,
  const char* name,
  const char* value
)
{
  CORBA::ULong criteriaLen =criteria.length();
  ++criteriaLen;
  criteria.length(criteriaLen);
  criteria[criteriaLen-1].name=CORBA::string_dup(name);
  criteria[criteriaLen-1].value<<=CORBA::ULong(atol(value));
}

static void appendCriterionStr(
  CosLifeCycle::Criteria& criteria,
  const char* name,
  const char* value
)
{
  CORBA::ULong criteriaLen =criteria.length();
  ++criteriaLen;
  criteria.length(criteriaLen);
  criteria[criteriaLen-1].name=CORBA::string_dup(name);
  criteria[criteriaLen-1].value<<=value;
}
