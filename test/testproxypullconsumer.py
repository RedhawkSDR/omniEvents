import sys,time,CORBA
import CosEventComm
import CosEventChannelAdmin
from actions import *
import servant

def test(channel):
    # Prepare
    assert(channel._is_a(CosEventChannelAdmin._tc_EventChannel.id()))
    consumer_i=servant.PullConsumer_i()
    consumer_i.connect(channel)
    supplier_i=servant.PullSupplier_i()
    supplierAdmin=for_suppliers(channel)
    supplier_i.event=CORBA.Any(CORBA.TC_string,"PPulC-works")

    test="PPulC-2" # ----------------------------------------------------------
    proxyPullConsumer=obtain_pull_consumer(supplierAdmin)
    try:
      connect_pull_supplier(proxyPullConsumer,CosEventComm.PullSupplier._nil)
      failed(test+": no exception")
    except CORBA.BAD_PARAM, ex:
      passed(test)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="PPulC-3" # ----------------------------------------------------------
    connect_pull_supplier(proxyPullConsumer,supplier_i._this())
    try:
      connect_pull_supplier(proxyPullConsumer,supplier_i._this())
      failed(test+": no exception")
    except CosEventChannelAdmin.AlreadyConnected, ex:
      passed(test)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

    test="PPulC-works" # ----------------------------------------------------------
    result,hasEvent =try_pull(consumer_i.pullSupplier,time.time()+timeout)
    if(hasEvent):
      if(result.value(CORBA.TC_string)==test):
        passed(test)
      else:
        failed(test+": wrong value ("+result.value(CORBA.TC_string)+")")
    else:
      failed(test+": timed out")

    test="PulC-2" # ----------------------------------------------------------
    disconnect_pull_consumer(proxyPullConsumer)
    time.sleep(2)
    if(supplier_i.disconnected):
      passed(test)
    else:
      failed(test)

    test="PulC-3" # ----------------------------------------------------------
    try:
      disconnect_pull_consumer(proxyPullConsumer)
      failed(test+": no exception")
    except CORBA.OBJECT_NOT_EXIST, ex:
      passed(test)
    except CORBA.Exception, ex:
      failed(test+": wrong exception: %s"%ex.__class__.__name__)

#end test() -------------------------------------------------------------------


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
