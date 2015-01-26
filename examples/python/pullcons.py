#                            Package   : omniEvents
# pullcons.py                Created   : 16/11/2003
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

"""Pull Model consumer implementation."""

import os,sys
import time
import threading
import signal
import getopt
import CORBA
import CosNaming
import CosEventComm__POA,CosEventComm
import CosEventChannelAdmin
from naming import *


class Consumer_i(CosEventComm__POA.PullConsumer):
  def disconnect_pull_consumer(self):
      print "Pull Consumer: disconnected."
#end class Consumer_i


def main():
  orb=CORBA.ORB_init(sys.argv,CORBA.ORB_ID)

  testloop    = TestLoop()
  channelName = "EventChannel"

  # Process Options
  try:
    opts,args=getopt.getopt(sys.argv[1:],"d:trs:n:h")
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(-1)
  for option, optarg in opts:
    if option=='-d':
      testloop.discnum = int(optarg)
    elif option=='-t':
      testloop.trymode = 1
    elif option=='-r':
      testloop.refnil = 1
    elif option=='-s':
      testloop.sleepInterval = int(optarg)
    elif option=='-n':
      channelName = optarg
    elif option=='-h':
      usage()
      sys.exit(0)
    else:
      usage()
      sys.exit(-1)

  # Ignore broken pipes
  if signal.__dict__.has_key('SIGPIPE'):
    signal.signal(signal.SIGPIPE, signal.SIG_IGN)

  action="start" # Use this variable to help report errors.
  try:

    if not testloop.refnil:
      action="resolve initial reference 'RootPOA'"
      poa=orb.resolve_initial_references("RootPOA")

      action="activate the RootPOA's POAManager"
      poaManager=poa._get_the_POAManager()
      poaManager.activate()

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
      obj=rootContext.resolve(str2name(channelName))

    action="narrow object reference to event channel"
    channel=obj._narrow(CosEventChannelAdmin.EventChannel)
    if channel is None:
      raise CORBA.OBJECT_NOT_EXIST(0,CORBA.COMPLETED_NO)

  except CORBA.ORB.InvalidName, ex: # resolve_initial_references()
     sys.stderr.write("Failed to %s. ORB::InvalidName\n"%action)
     sys.exit(1)
  except CosNaming.NamingContext.InvalidName, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::InvalidName\n"%action)
     sys.exit(1)
  except CosNaming.NamingContext.NotFound, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::NotFound\n"%action)
     sys.exit(1)
  except CosNaming.NamingContext.CannotProceed, ex: # resolve()
     sys.stderr.write("Failed to %s. NamingContext::CannotProceed\n"%action)
     sys.exit(1)
  except CORBA.TRANSIENT, ex:
     sys.stderr.write("Failed to %s. TRANSIENT\n"%action)
     sys.exit(1)
  except CORBA.OBJECT_NOT_EXIST, ex:
     sys.stderr.write("Failed to %s. OBJECT_NOT_EXIST\n"%action)
     sys.exit(1)
  except CORBA.COMM_FAILURE, ex:
     sys.stderr.write("Failed to %s. COMM_FAILURE\n"%action)
     sys.exit(1)
  except CORBA.SystemException, ex:
     sys.stderr.write("System exception, unable to %s.\n"%action)
     sys.exit(1)
  except CORBA.Exception, ex:
     sys.stderr.write("CORBA exception, unable to %s.\n"%action)
     sys.exit(1)

  #
  # Get Consumer Admin interface - retrying on Comms Failure.
  while(1):
    try:
        testloop.consumer_admin=channel.for_consumers()
        if testloop.consumer_admin is None:
          sys.stderr.write("Event Channel returned nil Consumer Admin!\n")
          sys.exit(1)
        break
    except CORBA.COMM_FAILURE, ex:
      sys.stderr.write("Caught COMM_FAILURE exception "+ \
        "obtaining Consumer Admin! Retrying...\n")
      time.sleep(1)
  print "Obtained ConsumerAdmin."

  # Perform connection tests in a new thread.
  # Do this so that the main thread can respond to keyboard interrupts.
  testloop.start()

  # Make sure that the main thread gets some time every 200ms, so that it
  # can respond to keyboard interrupts.
  try:
    while(1):
      time.sleep(1)
  except:
    os._exit(0) # Kills all known threads, dead!
#end main()


class TestLoop(threading.Thread):
  def __init__(self):
      threading.Thread.__init__(self)
      self.discnum=0
      self.trymode=0
      self.refnil=0
      self.sleepInterval=0
      self.consumer_admin=None
  
  def run(self):
      if self.refnil:
        cptr=None
      else:
        consumer=Consumer_i()
        cptr=consumer._this() # SIDE EFFECT: Activates object in POA

      while (1):
        #
        # Get proxy supplier - retrying on Comms Failure.
        while (1):
          try:
            proxy_supplier = self.consumer_admin.obtain_pull_supplier ()
            if proxy_supplier is None:
              sys.stderr.write("Consumer Admin returned nil Proxy Supplier!\n")
              sys.exit(1)
            break
          except CORBA.COMM_FAILURE, ex:
            sys.stderr.write("Caught COMM_FAILURE exception "+ \
              "obtaining Proxy Pull Supplier! Retrying...\n")
            time.sleep(1)
        print "Obtained ProxyPullSupplier."

        #
        # Connect Pull Consumer - retrying on Comms Failure.
        while (1):
          try:
              proxy_supplier.connect_pull_consumer(cptr)
              break
          except CORBA.BAD_PARAM, ex:
            sys.stderr.write( \
              'Caught BAD_PARAM exception connecting Pull Consumer!')
            sys.exit(1)
          except CosEventChannelAdmin.AlreadyConnected, ex:
            sys.stderr.write('Proxy Pull Supplier already connected!')
            sys.exit(1)
          except CORBA.COMM_FAILURE, ex:
            sys.stderr.write("Caught COMM_FAILURE exception " +\
              "connecting Pull Consumer! Retrying...")
            time.sleep(1)
        print "Connected Pull Consumer."

        #
        # Pull data.
        i=0
        while((self.discnum == 0) or (i < self.discnum)):
          try:
              if self.trymode:
                data,has_event = proxy_supplier.try_pull()
                if has_event:
                  l=data.value(CORBA.TC_ulong)
                  print "Pull Consumer: try_pull() called. Data :",l
                else:
                  print "Pull Consumer: try_pull() called. Data : None"
                  time.sleep(1)
              else:
                sys.stdout.write("Pull Consumer: pull() called. ")
                data=proxy_supplier.pull()
                l=data.value(CORBA.TC_ulong)
                print "Data :",l
          except CORBA.TRANSIENT, ex: 
              print "caught TRANSIENT."
              time.sleep(1)
          except CosEventComm.Disconnected, ex: 
              print "\nFailed. Caught Disconnected exception!"
              sys.exit(1)
          except CORBA.COMM_FAILURE, ex: 
              print "\nFailed. Caught COMM_FAILURE exception!"
              sys.exit(1)
          i=i+1


        # Disconnect - retrying on Comms Failure.
        while (1):
          try:
              proxy_supplier.disconnect_pull_supplier()
              break
          except CORBA.COMM_FAILURE, ex: 
            sys.stderr.write("Caught COMM_FAILURE exception " +\
              "connecting Pull Consumer! Retrying...")
            time.sleep(1)
        print "Disconnected Pull Consumer."

        # Yawn
        print "Sleeping",self.sleepInterval,"seconds."
        time.sleep(self.sleepInterval)
#end class TestLoop


def usage():
  print """
Create a PullConsumer to receive events from a channel.
syntax: python pullcons.py OPTIONS [CHANNEL_URI]

CHANNEL_URI: The event channel may be specified as a URI.
 This may be an IOR, or a corbaloc::: or corbaname::: URI.

OPTIONS:                                         DEFAULT:
 -t       enable try_pull mode
 -r       connect using a nil reference
 -d NUM   disconnect after receiving NUM events   [0 - never disconnect]
 -s SECS  sleep SECS seconds after disconnecting  [0]
 -n NAME  channel name (if URI is not specified)  ["EventChannel"]
 -h       display this help text
"""

################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  main()
