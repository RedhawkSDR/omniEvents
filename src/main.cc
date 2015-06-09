//                            Package   : omniEvents
//  main.cc                   Created   : 2004/08/01
//                            Author    : Alex Tingle.
//
//    Copyright (C) 1998 Paul Nader, 2004-2005 Alex Tingle.
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
//    Event Services Channel Factory implementation. The factory registers
//    itself with the naming service. Clients wishing to create event
//    channels can either use the factory by resolving its name with the
//    naming service or create in-process channels.
//
//    Modified by REDHAWK (United States Government) - 2015

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

#include "main.h"
#include "omniEvents.h"
#include "naming.h"
#include "omniEventsLog.h"
#include "EventChannelFactory.h"
#include "Orb.h"
#include "daemon.h"
#include "version.h"
#include "IteratorSupport.h"

#if defined(HAVE_SIGNAL_H) && defined(HAVE_SIGSET)
#  include <signal.h>
#  define SIGSET(sig,func) ::sigset(sig,func)
#elif defined(HAVE_SIGNAL_H)
#  include <signal.h>
#  define SIGSET(sig,func) ::signal(sig,func)
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/internal/orbOptions.h>
#endif

#include <cstdlib>
#include <stdio.h> // for sprintf
#include <stdlib.h>

int main(int argc, char** argv)
{
  OmniEvents::Daemon daemon(argc,argv);

#ifdef HAVE_OMNIORB4
  try
  {
    // Duplicate argv & argc.
    int    originalArgc =argc;
    char** originalArgv =new char*[originalArgc];
    for(int i=0; i<originalArgc; ++i)
        originalArgv[i]=strdup(argv[i]);

    // Remove ORB arguments from argc & argv.
    try {
      omni::orbOptions::singleton().extractInitOptions(argc,argv);
    }
    catch(...) {
      argc=originalArgc;
      argv=originalArgv;
    }
#endif

  using namespace OmniEvents;

  //
  // Process Options
  const char* endPointNoListen =NULL;
  int         port             =0;
  const char* logDir           =NULL;
  const char* factoryName      ="EventChannelFactory";
  bool        verbose          =false;

  int c;
  while ((c = getopt(argc,argv,"O:a:p:l:P:N:dft:vVh")) != EOF)
  {
     switch (c)
     {
        case 'O': break; // Helps protect us from -ORB arguments.

     // Initialisation options (only useful on first run)
        case 'a': endPointNoListen=optarg;
                  break;

        case 'p': port=atoi(optarg);
                  if (port <= 0)
                  {
                     cerr<<"\nError: port must be a positive integer"<<endl;
                     usage(argc,argv);
                  }
                  break;

     // Other options
        case 'l': logDir=optarg;
                  break;

        case 'P': daemon.pidfile(optarg);
                  break;

        case 'N': factoryName=optarg;
                  break;

        case 'd': cerr<<"Option '-d' is deprecated. Use '-f' instead."<<endl;
                  daemon.foreground(true);
                  break;

        case 'f': daemon.foreground(true);
                  break;

     // Informational options
        case 't': daemon.tracefile(optarg);
                  break;

        case 'v': verbose=true;
                  break;

        case 'V': cout<<OmniEvents::version()<<endl;
                  ::exit(0);

        case 'h':
        default : usage(argc,argv);
                  break;
     }
  }

  //
  // Create database instance.
  omniEventsLog logfile(logDir);
  PersistNode* initialState =NULL;
  if(logfile.fileExists(logfile.activeFilename()))
  {
    // Read library file.
    initialState=logfile.parse();
    // Check for incompatibilities between options and the file.
    if(port && port!=initialState->child("ecf")->attrLong("port"))
    {
      cerr<<
        "Error: Option '-p "<<port<<"' conflicts with value '"<<
        initialState->child("ecf")->attrLong("port")<<"'\n stored in"
        " database file '"<<logfile.activeFilename()<<"'.\n"
        " Either delete the file to clear the database, or do not use the"
        " '-p' option."<<endl;
      exit(1);
    }
    if(endPointNoListen && string(endPointNoListen)!=
         initialState->child("ecf")->attrString("endPointNoListen"))
    {
      cerr<<
        "Error: Option '-a "<<endPointNoListen<<"' conflicts with value '"<<
        initialState->child("ecf")->attrString("endPointNoListen")<<"'\n"
        " stored in database file '"<<logfile.activeFilename()<<"'.\n"
        " Either delete the file to clear the database, or do not use the"
        " '-a' option."<<endl;
      exit(1);
    }
  }
  else if(logfile.fileExists(logfile.backupFilename()))
  {
    // Quit with an error.
    cerr <<
      "Error: backup file '" << logfile.backupFilename() << "' exists.\n"
      " Rename it to '" << logfile.activeFilename() << "'\n"
      " to recover the server's state, or delete it to create a new\n"
      " database file." << endl;
    exit(1);
  }
  else
  {
    // Create initial state without a library file.
    initialState=logfile.bootstrap(port?port:11169,endPointNoListen);
  }
  port=initialState->child("ecf")->attrLong("port",port);
  string endPoint2=initialState->child("ecf")->attrString("endPointNoListen");

  //
  // Daemonise
  daemon.daemonize();

  //
  // Initialise orb & POAs.
#ifdef HAVE_OMNIORB4
  char endPoint[64];
  sprintf(endPoint,"giop:::%d",port);
  if(endPoint2.empty())
  {
    const char* opts[][2] ={ {"endPoint",endPoint}, {0,0} };
    Orb::inst()._orb=CORBA::ORB_init(originalArgc,originalArgv,"omniORB4",opts);
  }
  else
  {
    const char* opts[][2] ={
      {"endPoint",endPoint},
      {"endPointNoListen",endPoint2.c_str()},
      {0,0} };
    Orb::inst()._orb=CORBA::ORB_init(originalArgc,originalArgv,"omniORB4",opts);
  }
#else
  insertArgs(argc, argv, 1, 2);
  argv[1] = strdup("-ORBpoa_iiop_port");
  argv[2] = new char[32 + 1];
  sprintf(argv[2], "%d", port);
  Orb::inst()._orb=CORBA::ORB_init(argc,argv);
#endif
  Orb::inst().resolveInitialReferences();
  {
    PortableServer::POAManager_var pman;
    pman=Orb::inst()._RootPOA->the_POAManager();
    pman->activate();
    pman=Orb::inst()._omniINSPOA->the_POAManager();
    pman->activate();
  }

  //
  // If omniEvents is restarting then the omniEventsLog object
  // will take care of creating the factory and any subordinate
  // event channels, proxies, etc under it.
  logfile.incarnateFactory(initialState);
  delete initialState; // Tidy up.
  initialState=NULL;
  PortableServer::POA_ptr iteratorGC;
  {
    /// create Iterator GC support...
    iteratorGC = createGCPOA( Orb::inst()._RootPOA.in(), "Iterators" );
  }

  {
    //
    // Register factory with the Naming Service.
    omniEvents::EventChannelFactory_var factory( logfile.factory()->_this() );
    bindName2Object(
      Orb::inst()._NameService.in(),
      str2name(factoryName),
      factory.in()
    );

    //
    // Print the factory IOR.
    if(verbose)
    {
      DB(1,"Starting omniEvents on port "<<port)
      if(!endPoint2.empty())
          DB(1,"Alternate endPoint "<<endPoint2.c_str())
      CORBA::String_var iorstr =
        Orb::inst()._orb->object_to_string(factory.in());
      DB(1,iorstr.in())
    }
  } // factory reference is released.

#ifdef HAVE_SIGNAL_H
  SIGSET(SIGINT , ::OmniEvents_Orb_shutdown);
  SIGSET(SIGTERM, ::OmniEvents_Orb_shutdown);
#  ifdef SIGUSR1
  SIGSET(SIGUSR1, ::OmniEvents_Orb_bumpTraceLevel);
#  endif
#  ifdef SIGPIPE
  SIGSET(SIGPIPE, SIG_IGN); // Ignore broken pipes
#  endif
#endif

  daemon.runningOk();

  //
  // Start the background tasks.
  logfile.runWorker(); // Logfile's worker thread.
  Orb::inst().run();   // Use the main thread to collect orphaned responses.

  DB(1,"Shutdown requested.")
  Orb::inst()._orb->shutdown(1); // Synchronous shutdown
  Orb::inst()._orb->destroy(); // clean up

  return 0; // Delete any pidfile & exit.

#ifdef HAVE_OMNIORB4
  }
  catch (CORBA::SystemException& ex) {
    DB(0,"System exception: "<<ex._name()<<" ("<<NP_MINORSTRING(ex)<<")")
  }
  catch (CORBA::Exception& ex) {
    DB(0,"CORBA exception: "<<ex._name())
  }
  return 1;
#endif
} // end main()


//
// Signal handlers.
//

extern "C"
{
  void OmniEvents_Orb_shutdown(int signum)
  {
    OmniEvents::Orb::inst().shutdown(signum);
  }

  void OmniEvents_Orb_bumpTraceLevel(int signum)
  {
    omniORB::traceLevel=(omniORB::traceLevel+5)%45;
    DB(0,"TRACE LEVEL BUMPED TO "<<omniORB::traceLevel<<" BY SIGNAL "<<signum)
  }
}
