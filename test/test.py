import sys,time
import CORBA
import CosEventChannelAdmin
import testproxypushconsumer
import testproxypullsupplier
import testproxypullconsumer
import testproxypushsupplier

def main():
  orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
  poa = orb.resolve_initial_references("RootPOA")
  poaManager = poa._get_the_POAManager()
  poaManager.activate()
  obj = orb.resolve_initial_references("Channel")
  channel = obj._narrow(CosEventChannelAdmin.EventChannel)
  
  testproxypushconsumer.test(channel)
  testproxypullsupplier.test(channel)
  testproxypullconsumer.test(channel)
  testproxypushsupplier.test(channel)
  time.sleep(3)

################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  main()
