// -*- Mode: C++; -*-
//                            Package   : omniEvents
// defaults.h	              Created   : 1/10/99
//                            Author    : Paul Nader (pwn)
//
//    Copyright (C) 1998 Paul Nader, 2004 Alex Tingle.
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
// Contains definitions of default values.

/*
  $Log: defaults.h,v $
  Revision 1.7.2.2  2005/05/10 14:28:10  alextingle
  Updated copyrights to 2005.

  Revision 1.7.2.1  2004/11/01 12:27:12  alextingle
  New EventChannel parameter `PullRetryPeriod_ms' supercedes `PullRetryPeriod'.
  The new parameter is set by `eventc -R'. The old parameter and its `-r'
  option still work as before, for backwards compatibility.

  Revision 1.7  2004/07/26 20:50:39  alextingle
  Version --> 2.5.4

  Revision 1.6  2004/07/06 12:46:34  alextingle
  Moved default macros into defaults.h

  Revision 1.5  2004/05/28 10:11:37  alextingle
  Added a comment.

  Revision 1.4  2004/04/19 22:04:29  alextingle
  Corrected default CyclePeriod to 0.1s

  Revision 1.3  2004/03/28 01:01:21  alextingle
  New QoS parameters: CyclePeriod_ns and InsName.

  Revision 1.2  2004/01/11 16:57:26  alextingle
  New persistancy log file format, implemented by PersistNode.h/cc. The new format enables new nodes to be added and old ones erased by appending a single line to the file, rather than by re-persisting the whole application. This is much more efficient when lots of proxies are being created all at once. It's also a much simpler solution, with far fewer lines of code.

  Revision 1.1  2003/12/21 16:19:49  alextingle
  Moved into 'src' directory as part of the change to POA implementation.

  Revision 1.2  2003/11/03 22:36:48  alextingle
  Updated License to GNU Lesser General Public v2.1

  Revision 1.1.1.1  2002/09/25 19:00:32  shamus13
  Import of OmniEvents source tree from release 2.1.1

  Revision 1.1  2000/09/05 01:07:40  naderp
  Added MaxQueueLength QOS.

  Revision 1.0  1999/11/01 16:48:08  naderp
  Initial revision

*/

#ifndef _DEFAULTS_H_
#define _DEFAULTS_H_

#define PULL_RETRY_PERIOD_MS 1000 ///< 1 second
#define MAX_QUEUE_LENGTH     1023
#define MAX_NUM_PROXIES      1024 ///< Only limits number of ProxyPullSuppliers.
#define CYCLE_PERIOD_NS      100000000 ///< Delay between cycles. (0.1 second)

/** Define OMNIEVENTS_LOG_DEFAULT_LOCATION to specify the default location where
 * the omniEvents server executable places its persistency log file. The default
 * location is C:\TEMP for Win32 and /var/lib/omniEvents for UNIX systems. The
 * location can be overridden at runtime using the OMNIEVENTS_LOGDIR environment
 * variable.
 * Value must be an absolute pathname.
 */
#if defined(__WIN32__)
#  define OMNIEVENTS_LOG_DEFAULT_LOCATION "C:\\omniEvents"
#elif defined(__VMS) // What is a good default for VMS?
#  define OMNIEVENTS_LOG_DEFAULT_LOCATION "[]"
#else // Unix
#  define OMNIEVENTS_LOG_DEFAULT_LOCATION "/var/lib/omniEvents"
#endif

/** Define OMNIEVENTS_LOGDIR_ENV_VAR to specify the environment variable that
 * users may set to override the OMNIEVENTS_LOG_DEFAULT_LOCATION. The default
 * environment variable name is `OMNIEVENTS_LOGDIR'.
 */
#define OMNIEVENTS_LOGDIR_ENV_VAR "OMNIEVENTS_LOGDIR"

/** Define OMNIEVENTS_LOG_CHECKPOINT_PERIOD to specify the number of seconds
 * the omniEvents server executable should wait before persistency log check-
 * points. The default value is 900 (ie 15 minutes). You should adjust this
 * value depending on how often operations involving state changes are performed.
 * Operations of this type include creating event channels and connecting or
 * disconnecting clients. Lowering the period reduces the maximum size of the
 * logfile.
 * Value is in seconds.
 */
#define OMNIEVENTS_LOG_CHECKPOINT_PERIOD (15*60)

#endif // _DEFAULTS_H_
