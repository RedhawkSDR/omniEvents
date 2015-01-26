// -*- Mode: C++; -*-
//                            Package   : omniEvents
//  omniEventsLog.cc          Created   : 1/10/99
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

/*
  $Log: omniEventsLog.cc,v $
  Revision 1.20.2.3  2005/05/10 14:28:11  alextingle
  Updated copyrights to 2005.

  Revision 1.20.2.2  2005/05/04 23:06:02  alextingle
  Fixed reference leak. Releases ref to factory servant.

  Revision 1.20.2.1  2004/11/16 21:46:11  alextingle
  Made several methods virtual to allow users of libomniEvents to override
  the default persistency behaviour. (Dirk O. Siebnich)

  Revision 1.20  2004/09/25 23:12:28  alextingle
  New method: Orb::reportObjectFailure() - flags unexpected failures at a higher
  priority than normal non-fatal exceptions.

  New macro: NP_MINORSTRING() - a safe interface to
  CORBA::SystemException::NP_minorString() that returns "??" when there is no
  mapping for the exception's minor code.

  Revision 1.19  2004/07/26 16:27:08  alextingle
  Support for NT service on windows: main() moved into daemon.cc.
  New (laxer) start up syntax. Port is now set with -p (not -s). There is no
  special cold start mode.
  More flexible naming service name option -N. (No more -K option).

  Revision 1.18  2004/07/16 08:45:46  alextingle
  New macro: IF_OMNIORB4(). Fixes warnings on AIX xlC_r.

  Revision 1.17  2004/07/15 16:18:45  alextingle
  Fixed casting warnings on Tru64.

  Revision 1.16  2004/07/06 12:46:34  alextingle
  Moved default macros into defaults.h

  Revision 1.15  2004/07/06 10:59:39  alextingle
  Tightened privileges on created files.

  Revision 1.14  2004/07/05 13:52:37  alextingle
  Improved iostream portability (again).

  Revision 1.13  2004/07/02 15:20:39  alextingle
  Added daemonization, syslog & pidfile support on Unix.
  Corrected trace levels for consistency with omniORB.

  Revision 1.12  2004/05/14 14:44:48  alextingle
  Explicitly cast 'flags' to type ios::openmode for platforms where this type is an enum.

  Revision 1.11  2004/04/20 20:23:03  alextingle
  Cross-platform friendly use of std:ifstream.

  Revision 1.10  2004/04/20 17:16:16  alextingle
  Corrected openOfstream() arg name/comments.

  Revision 1.9  2004/03/30 17:31:00  alextingle
  Added exception handlers to thread methods.

  Revision 1.8  2004/03/28 01:03:58  alextingle
  Refactored class omniEventsLog to allow for more EventChannelFactory parameters.\nNew omniEvents params: -v, -a (alternate endPoint).

  Revision 1.7  2004/03/23 13:29:34  alextingle
  Fixed bizarre compilation errors on Sun's CC.

  Revision 1.6  2004/01/11 16:57:26  alextingle
  New persistancy log file format, implemented by PersistNode.h/cc. The new format enables new nodes to be added and old ones erased by appending a single line to the file, rather than by re-persisting the whole application. This is much more efficient when lots of proxies are being created all at once. It's also a much simpler solution, with far fewer lines of code.

  Revision 1.5  2003/12/21 16:17:19  alextingle
  Added OmniEvents namespace. Now uses POA implementations.

  Revision 1.4  2003/11/14 14:05:21  alextingle
  New output() members functions. Eliminates the need for friend ostream
  functions that are problematic on earlier versions of Microsoft
  VisualC++. Improved helpfulness of usage and error messages.

  Revision 1.3  2003/11/03 22:41:00  alextingle
  Oops! Removed excess log comments.

  Revision 1.2  2003/11/03 22:38:39  alextingle
  Removed many platform specific switches, alas many remain. Now uses
    autoconf,config.h wherever possible.
  Removed local definition of strdup() - omniEvents.cc manages to get
    along without it, so it can't be needed here.
  Moved convoluted code to obtain hostname into its own header file:
    gethostname.h
  Moved code that constructs logfile names into its own private method,
    omniEventsLog::initializeFileNames()
  Added explicit initialisation for all `class omniEventsLog' members.
  Moved code that opens file stream into its own private method,
    omniEventsLog::openOfstream()

  Revision 1.1.1.1  2002/09/25 19:00:35  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.8  2000/09/26 09:20:26  naderp
  STL Default parameters rework.
  Configurable checkpoint period.

  Revision 1.7  2000/09/04 05:05:22  naderp
  Fixed problem with triggering multiple checkpoints.

  Revision 1.6  2000/08/30 04:38:24  naderp
  Fix to prevent recorder thread from terminating.

  Revision 1.5  2000/08/30 00:57:29  naderp
  Fixed broken persist method exiting after first checkpoint.

  Revision 1.4  2000/03/06 13:19:58  naderp
  Moved port from global to factory persistency data.

  Revision 1.3  2000/03/06 04:15:09  naderp
  Removed internal dependency between factory and Naming Service.

  Revision 1.2  2000/03/02 04:20:09  naderp
  Initialising factory refernce in init().

  Revision 1.1  2000/03/02 02:00:25  naderp
  Re-open active logfile during re-start.

  Revision 1.0  1999/11/01 17:04:08  naderp
  Initial revision

*/

#include "omniEventsLog.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#if defined(__VMS) && __CRTL_VER < 70000000
#  include <omniVMS/unlink.hxx>
#endif

#ifdef __WIN32__
#  include <io.h>
#  include <winbase.h>
#  define stat(x,y) _stat(x,y)
#  define unlink(x) _unlink(x)
#  define STRUCT_STAT struct _stat
#else
#  define STRUCT_STAT struct stat
#endif // __WIN32__

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_LIBC_H
#  include <libc.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif

#include <errno.h>
#include <time.h>
#include <assert.h>
#include "gethostname.h"

#include "EventChannelFactory.h"
#include "Orb.h"
#include "defaults.h"

//
// Set flags for use in calls to omniEventsLog::openOfstream()
//

#if defined(HAVE_FSTREAM_OPEN)
#  define FLAG_TRUNCATE ios::trunc
#  define FLAG_APPEND   ios::app
#  define FLAG_SYNC     0
#elif defined(HAVE_FSTREAM_ATTACH)
#  if defined(__WIN32__)
#    define FLAG_SYNC 0
#  elif defined(O_SYNC)
#    define FLAG_SYNC O_SYNC
#  else
#    define FLAG_SYNC O_FSYNC // FreeBSD 3.2 does not have O_SYNC???
#  endif
#  define FLAG_TRUNCATE O_CREAT|O_TRUNC
#  define FLAG_APPEND   O_APPEND
#else
#  error "Can't open a file without ofstream::open() or ofstream::attach()"
#endif

//
// Append ';' to VMS filenames to force the latest version.
//

#ifdef __VMS
#  define VMS_SEMICOLON ";"
#else
#  define VMS_SEMICOLON
#endif

extern int yyparse();
extern int yydebug;
extern FILE *yyin;

namespace OmniEvents {

/** This class can be used to generate timestamps.  The t() method normally
 * returns a timestamp string, but if the same timestamp (to the nearest
 * second) would be returned as last time then an empty string is returned
 * instead.
 */
class timestamp
{
  char str[29];
public:
  timestamp(void)
  {
    str[0] = '[';
    str[1] = str[28] = '\0';
  }
  const char* t(void)
  {
    time_t t =time(NULL);
    char*  p =ctime(&t);
    if(strncmp(p, &str[1], 24) == 0)
        return "";
    strncpy(&str[1], p, 24);
    str[25] = ']';
    str[26] = ' ';
    str[27] = ' ';
    return str;
  }
};

timestamp ts;

//------------------------------------------------------------------------
//           omniEvents Log Implementation
//------------------------------------------------------------------------

omniEventsLog *omniEventsLog::theLog = NULL;

omniEventsLog::omniEventsLog(const char* logdir) :
  _logstream(),
  _activeFilename(NULL),
  _backupFilename(NULL),
  _checkpointFilename(NULL),
  _workerThread(NULL),
  _factory(NULL),
  _checkpointNeeded(true),
  _lock()
{
  omniEventsLog::theLog = this;
  initializeFileNames(logdir);
}


omniEventsLog::~omniEventsLog()
{
  DB(20, "omniEventsLog::~omniEventsLog()");
/*
  if(NULL != _workerThread)
  {
    _workerThread->join(0);
    _workerThread = NULL;
  }
*/
  if(NULL != _factory)
  {
    _factory->_remove_ref();
    _factory = NULL;
  }
  omniEventsLog::theLog = NULL;
}


bool omniEventsLog::fileExists(const char* filename) const
{
  STRUCT_STAT sb;
  return(::stat(filename,&sb) == 0);
}


PersistNode* omniEventsLog::bootstrap(int port, const char* endPointNoListen)
{
  //
  // Construct a new initialState, from the arguments.
  PersistNode* initialState=new PersistNode();
  PersistNode* ecf =initialState->addnode("ecf");
  ecf->addattr("port",port);
  if(endPointNoListen && endPointNoListen[0])
      ecf->addattr(string("endPointNoListen=")+endPointNoListen);
  return initialState;
} // bootstrap()


PersistNode* omniEventsLog::parse()
{
  //
  // Restart - parse log file.
  ifstream persiststream(_activeFilename);
  if(!persiststream)
  {
    cerr << "Error: cannot read database file '"
         << _activeFilename << "'." << endl;
    if( fileExists(_backupFilename) ) 
    {
      cerr <<
        " Backup file '" << _backupFilename << "' exists.\n"
        " Either rename it to '" << _activeFilename << "' to\n"
        " to recover the server's state, or delete it to create a new\n"
        " database file." << endl;
    }
    exit(1);
  }
  PersistNode* initialState=new PersistNode(persiststream);
  persiststream.close();
  
  //
  // Check that the file contains a valid EventChannelFactory.
  const char* errorStr =NULL;
  PersistNode* ecf=initialState->child("ecf");
  if(!ecf)
      errorStr="Can't find EventChannelFactory.";
  else if(ecf->attrLong("port",-1)<=0)
      errorStr="EventChannelFactory is not assigned a valid port.";

  if(errorStr)
  {
    cerr<<"Error parsing database '"<<_activeFilename<<"'.\n"
        <<errorStr<<" Try deleting the file (and any backup)."<<endl;
    exit(1);
  }

  return initialState;
} // parse()


void omniEventsLog::incarnateFactory(PersistNode* initialState)
{
  assert(initialState!=NULL);

  //
  // Open the logstream (The EventChannelFactory might want to write to it).
  try
  {
    openOfstream(_logstream,_activeFilename,FLAG_APPEND);
  }
  catch (IOError& ex)
  {
    cerr << "Error: cannot "
         << (fileExists(_activeFilename)?"write to":"create new")
         << " database file '" << _activeFilename
         << "': " << strerror(errno) << endl;
    cerr << "\nUse option '-l' or set the environment variable "
         << OMNIEVENTS_LOGDIR_ENV_VAR
         << "\nto specify the directory where the files are kept.\n"
         << endl;
    _logstream.close();
    unlink(_activeFilename);
    exit(1);
  }

  //
  // Recreate the persisted factory.
  PersistNode* ecf=initialState->child("ecf");
  assert(ecf!=NULL);
  _factory =new EventChannelFactory_i(*ecf);
  CORBA::Object_var obj;
  assert(!CORBA::is_nil(obj = _factory->_this()));
} // incarnateFactory


void omniEventsLog::runWorker()
{
  assert(_factory!=NULL);

  _workerThread=new omniEventsLogWorker(
      this,
      &omniEventsLog::checkpoint, // member function pointer
      omni_thread::PRIORITY_NORMAL
    );
}


void omniEventsLog::output(ostream& os)
{
  _factory->output(os);
  os<<endl;
}


void omniEventsLog::checkpoint(void)
{
  int idle_time_btw_chkpt;
  static int firstCheckPoint = 1;
  char *itbc = getenv("OMNIEVENTS_ITBC");
  if (itbc == NULL || sscanf(itbc,"%d",&idle_time_btw_chkpt) != 1)
  {
    idle_time_btw_chkpt=OMNIEVENTS_LOG_CHECKPOINT_PERIOD;
  }

  omni_mutex mutex;
  omni_condition cond(&mutex);

  mutex.lock();
  while (1) {

    // Take an initial checkpoint the first time. All subsequent
    // checkpoints are conditionally tested on whether they are
    // needed or not.

    if (! firstCheckPoint)
    {
       unsigned long s, n;
       omni_thread::get_time(&s, &n, idle_time_btw_chkpt);
       cond.timedwait(s,n);

       _lock.lock();
       if(!_checkpointNeeded)
       {
          _lock.unlock();
          continue;
       }
    }
    else
    {
       _lock.lock();
       firstCheckPoint = 0;
    }
  
    DB(1,ts.t() << "Checkpointing Phase 1: Prepare.")
  
    ofstream ckpf;
    int fd = -1;
  
    try
    {
      try
      {
        openOfstream(ckpf,_checkpointFilename,FLAG_TRUNCATE|FLAG_SYNC,&fd);
      }
      catch(IOError& ex)
      {
        DB(0,ts.t() << "Error: cannot open checkpoint file '"
             << _checkpointFilename << "' for writing.")
        throw;
      }
  
      output(ckpf);
  
      ckpf.close();
      if(!ckpf)
          throw IOError();
  
  // a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
      if(close(fd) < 0)
          throw IOError();
#endif
  
    }
    catch(IOError& ex)
    {
      DB(0,ts.t()<<"I/O error writing checkpoint file: "<<strerror(errno)
           <<"\nAbandoning checkpoint")
      ckpf.close();
  // a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
      close(fd);
#endif
      unlink(_checkpointFilename);
      _lock.unlock();
      continue;
    }
  
    //
    // Now commit the checkpoint to become the active log.
    //
  
    DB(1,ts.t() << "Checkpointing Phase 2: Commit.")

  // a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
    close(_logstream.rdbuf()->fd());
#endif
  
    _logstream.close();
  
    unlink(_backupFilename);
  
#if defined(__WIN32__)
    if(rename(_activeFilename, _backupFilename) != 0)
#elif defined(__VMS)
    if(rename(_activeFilename, _backupFilename) < 0)
#else
    if(link(_activeFilename,_backupFilename) < 0)
#endif
    {
      // Failure here leaves old active and checkpoint file.
      DB(0,ts.t() << "Error: failed to link backup file '"
           << _backupFilename << "' to old log file '"
           << _activeFilename << "'.")
      exit(1);
    }
  
#if !defined( __VMS) && !defined(__WIN32__)
    if(unlink(_activeFilename) < 0)
    {
      // Failure here leaves active and backup pointing to the same (old) file.
      DB(0,ts.t() << "Error: failed to unlink old log file '"
           << _activeFilename << "': " << strerror(errno))
      exit(1);
    }
#endif
  
#if defined(__WIN32__)
    if(rename(_checkpointFilename,_activeFilename) != 0)
#elif defined(__VMS)
    if(rename(_checkpointFilename,_activeFilename) < 0)
#else
    if(link(_checkpointFilename,_activeFilename) < 0)
#endif
    {
      // Failure here leaves no active but backup points to the old file.
      DB(0,ts.t() << "Error: failed to link log file '" << _activeFilename
           << "' to checkpoint file '" << _checkpointFilename << "'.")
      exit(1);
    }
  
#if !defined( __VMS) && !defined(__WIN32__)
    if (unlink(_checkpointFilename) < 0)
    {
      // Failure here leaves active and checkpoint pointing to the same file.
      DB(0,ts.t() << "Error: failed to unlink checkpoint file '"
           << _checkpointFilename << "'.")
      exit(1);
    }
#endif
  
    try
    {
      openOfstream(_logstream,_activeFilename,FLAG_APPEND|FLAG_SYNC,&fd);
    }
    catch (IOError& ex)
    {
      DB(0,ts.t() << "Error: cannot open new log file '" << _activeFilename
           << "' for writing.")
      exit(1);
    }
  
    DB(1,ts.t() << "Checkpointing completed.")
  
    _checkpointNeeded=false;
    _lock.unlock();
  }
  mutex.unlock();
} // checkpoint


/** Sets the values of 'active', 'backup' and 'checkpoint' file names.
 * The files are placed in the log file directory, that is specified by:
 *
 * - command line parameter -l,  OR ELSE
 * - OMNIEVENTS_LOGDIR_ENV_VAR env. var. (default `OMNIEVENTS_LOGDIR'), OR ELSE
 * - OMNIEVENTS_LOG_DEFAULT_LOCATION (set in defaults.h),  OR ELSE
 * - Unix: /var/lib/omniEvents,  Win32: C:\TEMP,  VMS: []
 *
 * The current hostname is incorporated into the file names.
 */
void omniEventsLog::initializeFileNames(const char* logdir)
{
  if(!logdir)
      logdir=getenv(OMNIEVENTS_LOGDIR_ENV_VAR);
  if(!logdir)
      logdir=OMNIEVENTS_LOG_DEFAULT_LOCATION;

  const char* logname ="omnievents-";
  char hostname[MAXHOSTNAMELEN];
  if (0!=gethostname(hostname,MAXHOSTNAMELEN))
  {
    cerr << "Error: cannot get the name of this host." << endl;
    exit(1);
  }
  const char* sep ="";

#if defined(__WIN32__)
  sep="\\";
#elif defined(__VMS)
  char last( logdir[strlen(logdir)-1] );
  if (last != ':' && last != ']')
  {
    cerr << "Error: " << OMNIEVENTS_LOGDIR_ENV_VAR << " (" << logdir
         << ") is not a directory name." << endl;
    exit(1);
  }
#else // Unix
  if (logdir[0] != '/')
  {
    cerr << "Error: " << OMNIEVENTS_LOGDIR_ENV_VAR << " (" << logdir
         << ") is not an absolute path name." << endl;
    exit(1);
  }
  if (logdir[strlen(logdir)-1] != '/')
      sep="/";
#endif

  // VMS_SEMICOLON specifies latest version of the file on VMS
  // (essentially, we're saying we don't want to use VMS file versioning).

  setFilename(_activeFilename,logdir,sep,logname,hostname,".log" VMS_SEMICOLON);
  setFilename(_backupFilename,logdir,sep,logname,hostname,".bak" VMS_SEMICOLON);
  setFilename(
          _checkpointFilename,logdir,sep,logname,hostname,".ckp" VMS_SEMICOLON);
}


/** Helper function that sets the value of the first parameter to the
 * concatenation of all the subsequent parameters.
 */
void omniEventsLog::setFilename(
  char*& filename,     const char* logdir,   const char* sep,
  const char* logname, const char* hostname, const char* ext)
{
  size_t len=1+
    strlen(logdir)+strlen(sep)+strlen(logname)+strlen(hostname)+strlen(ext);
  filename=new char[len];
  sprintf(filename,"%s%s%s%s%s",logdir,sep,logname,hostname,ext);
}


/** Helper method that opens an output file stream using whatever method
 * is available. Available flags are:
 *
 * - FLAG_TRUNCATE
 * - FLAG_APPEND
 * - FLAG_SYNC
 *
 * @param s  reference to the ofstream object.
 * @param filename  the name of the file to open.
 * @param flags  see description for available mode flags.
 * @param fd  reference to a file descriptor. If used, this parameter is set
 *            to the fd that was opened, if any.
 */
void omniEventsLog::openOfstream(
  ofstream& s, const char* filename, int flags, int* fd)
{
#if defined(HAVE_FSTREAM_OPEN)
#  ifdef HAVE_STD_IOSTREAM
      ios::openmode openmodeflags =ios::out|ios::openmode(flags);
#  else
      int           openmodeflags =ios::out|flags;
#  endif

#  ifdef FSTREAM_OPEN_PROT
      s.open(filename,openmodeflags,0644);
#  else
      s.open(filename,openmodeflags);
#  endif
      if (!s)
          throw IOError();

#elif defined(HAVE_FSTREAM_ATTACH)
#  ifdef __WIN32__
      int localFd = _open(filename, O_WRONLY | flags, _S_IWRITE);
#  else
      int localFd = open(filename, O_WRONLY | flags, 0644);
#  endif /* __WIN32__ */
      if (localFd < 0)
          throw IOError();
      if(fd)
          (*fd)=localFd;
      s.attach(localFd);
#endif
}


//------------------------------------------------------------------------
//           OmniEvents Log Worker Implementation
//------------------------------------------------------------------------
omniEventsLogWorker::omniEventsLogWorker(
  omniEventsLog* object,
  Method         method,
  priority_t     priority
):omni_thread(NULL,priority)
{
  DB(15, "omniEventsLogWorker::omniEventsLogWorker()");

  _method=method;
  _object=object;

  start_undetached();
}


void* omniEventsLogWorker::run_undetached(void *)
{
  try {
    DB(15, "omniEventsLogWorker : run_undetached Start");
    (_object->*_method)();
    DB(15, "omniEventsLogWorker : run_undetached End");
  }
  catch (CORBA::SystemException& ex) {
    DB(0,"omniEventsLogWorker killed by CORBA system exception"
       IF_OMNIORB4(": "<<ex._name()<<" ("<<NP_MINORSTRING(ex)<<")") ".")
  }
  catch (CORBA::Exception& ex) {
    DB(0,"omniEventsLogWorker killed by CORBA exception"
       IF_OMNIORB4(": "<<ex._name()<<) ".")
  }
  catch(...) {
    DB(0,"omniEventsLogWorker killed by unknown exception.")
  }
  return NULL;
}

omniEventsLogWorker::~omniEventsLogWorker()
{
  DB(20, "omniEventsLogWorker::~omniEventsLogWorker()");
}


}; // end namespace OmniEvents
