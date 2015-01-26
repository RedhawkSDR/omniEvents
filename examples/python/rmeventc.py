#                            Package   : omniEvents
# rmeventc.py                Created   : 20/04/2004
#                            Author    : Alex Tingle
#
#    Copyright (C) 2004 Alex Tingle.
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

"""Client to the event channel factory. Destroys the named EventChannel."""

import sys
import getopt
import CORBA
import CosNaming
import EventChannelAdmin
import CosLifeCycle
import CosEventChannelAdmin
from naming import *

def main():
  result=1

  #
  # Start orb.
  orb=CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

  # Process Options
  ecName=str2name("EventChannel")

  try:
    opts,args=getopt.getopt(sys.argv[1:],"n:h")
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(-1)
  for option, optarg in opts:
    if option=='-n':
      ecName=str2name(optarg)
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
    # Obtain reference to the Event Channel.
    # (from command-line argument or from the Naming Service).
    if len(args):
      action="convert URI from command line into object reference"
      obj=orb.string_to_object(args[0])
    else:
      #
      # Get Name Service root context.
      action="resolve initial reference 'NameService'"
      obj=orb.resolve_initial_references("NameService")
      rootContext=obj._narrow(CosNaming.NamingContext)
      if rootContext is None:
        raise CORBA.OBJECT_NOT_EXIST(0,CORBA.COMPLETED_NO)

      #
      # Obtain reference to the Event Channel.
      action="find Event Channel in naming service"
      obj=rootContext.resolve(ecName)

      #
      # Unbind the Channel's reference in the naming service.
      action="unbind Event Channel from naming service"
      rootContext.unbind(ecName)

    action="narrow object reference to event channel"
    channel=obj._narrow(CosEventChannelAdmin.EventChannel)
    if channel is None:
      raise CORBA.OBJECT_NOT_EXIST(0,CORBA.COMPLETED_NO)
    
    #
    # Destroy the EventChannel.
    action="destroy Event Channel"
    channel.destroy()

    #
    # Clean up nicely.
    action="destroy orb"
    orb.destroy()

    #
    # If we get here, then everything has worked OK.
    result=0

  except CORBA.ORB.InvalidName, ex: # resolve_initial_references()
     sys.stderr.write("Failed to %s. ORB::InvalidName\n"%action)
  except CosNaming.NamingContext.InvalidName, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::InvalidName\n"%action)
  except CosNaming.NamingContext.NotFound, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::NotFound\n"%action)
  except CosNaming.NamingContext.CannotProceed, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::CannotProceed\n"%action)
  except CORBA.TRANSIENT, ex:
     sys.stderr.write("Failed to %s. TRANSIENT\n"%action)
  except CORBA.OBJECT_NOT_EXIST, ex:
     sys.stderr.write("Failed to %s. OBJECT_NOT_EXIST\n"%action)
  except CORBA.COMM_FAILURE, ex:
     sys.stderr.write("Failed to %s. COMM_FAILURE\n"%action)
  except CORBA.SystemException, ex:
     sys.stderr.write("System exception, unable to %s.\n"%action)
  except CORBA.Exception, ex:
     sys.stderr.write("CORBA exception, unable to %s.\n"%action)

  sys.exit(result)
#end main


def usage():
  print """
Destroy an EventChannel.
syntax: python rmeventc.py OPTIONS [CHANNEL_URI]

CHANNEL_URI: The event channel may be specified as a URI.
 This may be an IOR, or a corbaloc::: or corbaname::: URI.

OPTIONS:                                         DEFAULT:
 -n NAME  channel name (if URI is not specified)  ["EventChannel"]
 -h       display this help text
"""


################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  main()
