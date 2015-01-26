import sys,time,CORBA
import CosEventComm
import CosEventChannelAdmin
from actions import *
import servant

def test(channel):
    # Prepare
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    consumer_i=servant.PushConsumer_i()
    supplier_i=servant.PushSupplier_i()
    supplier_i.connect(channel)
    consumerAdmin=for_consumers(channel)

    test="PPshS-2" # ----------------------------------------------------------
    proxyPushSupplier=obtain_push_supplier(consumerAdmin)
    try:
      connect_push_consumer(proxyPushSupplier,CosEventComm.PushConsumer._nil)
      failed(test+": no exception")
    except CORBA.BAD_PARAM, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="PPshS-3" # ----------------------------------------------------------
    connect_push_consumer(proxyPushSupplier,consumer_i._this())
    try:
      connect_push_consumer(proxyPushSupplier,consumer_i._this())
      failed(test+": no exception")
    except CosEventChannelAdmin.AlreadyConnected, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="PPshS-works" # ----------------------------------------------------------
    push(supplier_i.pushConsumer,CORBA.Any(CORBA.TC_string,test))
    result=consumer_i.receive(time.time()+timeout)
    if(result):
      if(result.value(CORBA.TC_string)==test):
        passed(test)
      else:
        failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
    else:
      failed(test+": timed out")

    test="PPshS-1" # ----------------------------------------------------------
    disconnect_push_supplier(proxyPushSupplier)
    time.sleep(1)
    if(consumer_i.disconnected):
      passed(test+", PshS-1")
    else:
      failed(test)

    test="PshS-2" # ----------------------------------------------------------
    try:
      disconnect_push_supplier(proxyPushSupplier)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test+": got exception %s"%ex.__class__.__name__)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)
#end test


################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
  poa = orb.resolve_initial_references("RootPOA")
  poaManager = poa._get_the_POAManager()
  poaManager.activate()
  obj = orb.resolve_initial_references("Channel")
  channel = obj._narrow(CosEventChannelAdmin.EventChannel)
  
  test(channel)
  time.sleep(3)
##
