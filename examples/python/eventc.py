#                            Package   : omniEvents
# eventc.py                  Created   : 16/11/2003
#                            Author    : Alex Tingle
#
#    Copyright (C) 2003-2004 Alex Tingle.
#
#    This file is part of the omniEvents application.
#
#    omniEvents is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    omniEvents is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

"""Client to the event channel factory. Requests creation of an event
channel and registers it with the Naming service."""

import sys
import getopt
import CORBA
import CosNaming
import EventChannelAdmin
import CosLifeCycle
import CosEventChannelAdmin
from naming import *

def makeNVP(name,value):
  if(type(value)==type('')):
    return CosLifeCycle.NameValuePair(name,CORBA.Any(CORBA.TC_string,value))
  elif(type(value)==type(0)):
    return CosLifeCycle.NameValuePair(name,CORBA.Any(CORBA.TC_ulong,value))
  else:
    sys.stderr.write("Unexpected type.\n")
    sys.exit(1)
#end makeNVP


def main():
  #
  # Start orb.
  orb=CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

  # Process Options
  verbose=0
  needNameService=0
  channelName="EventChannel"
  factoryName="EventChannelFactory"
  criteria=[]

  try:
    opts,args=getopt.getopt(sys.argv[1:],"n:N:m:c:i:p:q:R:r:vh")
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(-1)
  for option, optarg in opts:
    if option=='-n':
      channelName=optarg
      needNameService=1
    elif option=='-N':
      factoryName=optarg
    elif option=='-m': ## OLD OPTION
      criteria.append(makeNVP("MaxEventsPerConsumer",int(optarg)))
    elif option=='-c':
      criteria.append(makeNVP("CyclePeriod_ns",int(optarg)))
    elif option=='-i':
      criteria.append(makeNVP("InsName",optarg)) # string param
    elif option=='-p':
      criteria.append(makeNVP("MaxNumProxies",int(optarg)))
    elif option=='-q':
      criteria.append(makeNVP("MaxQueueLength",int(optarg)))
    elif option=='-R':
      criteria.append(makeNVP("PullRetryPeriod_ms",int(optarg)))
    elif option=='-r': ## This option is deprecated in favour of -R:
      criteria.append(makeNVP("PullRetryPeriod",int(optarg)))
    elif option=='-v':
      verbose=1
    elif option=='-h':
      usage()
      sys.exit(0)
    else:
      usage()
      sys.exit(-1)
      
  # Use one big try...catch block.
  # 'action' variable keeps track of what we're doing.
  action="start"
  try:

    #
    # Get Name Service root context.
    rootContext=None
    try:
      action="resolve initial reference 'NameService'"
      obj=orb.resolve_initial_references("NameService")
      rootContext=obj._narrow(CosNaming.NamingContext)
      if rootContext is None:
        raise CORBA.OBJECT_NOT_EXIST(0,CORBA.COMPLETED_NO)
    except CORBA.Exception, ex:
        if needNameService:
          raise ex
        else:
          sys.stderr.write("Warning - failed to %s\n"%action)

    #
    # Obtain reference to the Event Channel Factory implementation.
    # (from command-line argument or from the Naming Service).
    if len(args):
      action="convert URI from command line into object reference"
      obj=orb.string_to_object(args[0])
    else:
      action="find Event Channel Factory in naming service"
      obj=rootContext.resolve(str2name(factoryName))

    action="narrow object reference to event channel factory"
    factory=obj._narrow(EventChannelAdmin.EventChannelFactory)
    if factory is None:
      sys.stderr.write("Failed to narrow Event Channel Factory reference.\n")
      sys.exit(1)


    # Check that the factory is of the right type
    action="check factory supports EventChannel object interface"
    key=[CosNaming.NameComponent("EventChannel","object interface")]
    if not factory.supports(key):
      sys.stderr.write('Factory does not support Event Channel Interface!'+ \
        ' [%s]\n'%factoryName)
      sys.exit(1)

    #
    # Create Event Channel Object.
    action="create EventChannel object"
    channelObj=factory.create_object(key,criteria)
    if channelObj is None:
      sys.stderr.write('Channel Factory returned nil reference!'+ \
        ' [%s]\n'%channelName)
      sys.exit(1)

    # Print the new EventChannel's IOR to standard output.
    if verbose:
      print orb.object_to_string(channelObj)

    # Narrow object returned to an Event Channel
    channel=channelObj._narrow(CosEventChannelAdmin.EventChannel)
    if channel is None:
      sys.stderr.write('Failed to narrow Event Channel!'+ \
        ' [%s]\n'%channelName)
      sys.exit(1)

    #
    # Register event channel with naming service
    if rootContext is not None:
      name=str2name(channelName)
      try:
          action="register (bind) EventChannel with the naming service"
          rootContext.bind(name,channel)
      except CosNaming.NamingContext.AlreadyBound, ex:
        action="register (rebind) EventChannel with the naming service"
        rootContext.rebind(name,channel)

    #
    # Clean up nicely.
    action="destroy orb"
    orb.destroy()

    #
    # If we get here, then everything has worked OK.
    sys.exit(0)

  except CosLifeCycle.NoFactory, ex: # create_object()
     sys.stderr.write("Failed to create Event Channel: NoFactory"+
       " (interface not supported) \n")
  except CosLifeCycle.CannotMeetCriteria, ex: # create_object()
     sys.stderr.write("Failed to create Event Channel: CannotMeetCriteria \n")
  except CosLifeCycle.InvalidCriteria, ex: # create_object()
     sys.stderr.write("Failed to create Event Channel: InvalidCriteria \n")
  except CORBA.COMM_FAILURE, ex:
     sys.stderr.write("System exception, unable to %s: COMM_FAILURE\n"%action)
  except CORBA.SystemException, ex:
     sys.stderr.write("System exception, unable to %s.\n"%action)
  except CORBA.Exception, ex:
     sys.stderr.write("CORBA exception, unable to %s.\n"%action)
  sys.exit(1)
#end main


def usage():
  sys.stderr.write("""
Create an EventChannel and register it in the naming service.
syntax: eventc OPTIONS [FACTORY_URI]

FACTORY_URI: The factory may be specified as a URI.
 This may be an IOR, or a corbaloc::: or corbaname::: URI.
 For example: corbaloc::localhost:11169/omniEvents

OPTIONS:                                         DEFAULT:
 -n channel name                                  ["EventChannel"]
 -N factory name (if URI is not specified)        ["EventChannelFactory"]
 -c override default CyclePeriod_ns of new channel (nanoseconds)
 -i set the InsName of new channel, to enable access via corbaloc
 -p override default MaxNumProxies of new channel
 -q override default MaxQueueLength of new channel
 -R override default PullRetryPeriod_ms for new channel (milliseconds)
 -v print the IOR of the new EventChannel to standard output.
 -h display this help text
OLD OPTIONS: (only used by omniEvents v2.4 and earlier)
 -m override default MaxEventsPerConsumer for new channel

""")


################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  main()
