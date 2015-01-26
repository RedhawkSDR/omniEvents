//                            Package   : omniEvents
// Eventc.java                Created   : 2004/03/10
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle.
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

import gnu.getopt.Getopt;
import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.*;
import org.omg.CosLifeCycle.*;
import org.omg.CosEventComm.*;
import org.omg.CosEventChannelAdmin.*;
import net.sourceforge.omniorb.EventChannelAdmin.*;

public class Eventc
{
  static ORB orb;

  public static void main(String args[])
  {
    //
    // Start orb.
    orb=ORB.init(args, null);
    // Must strip out ORB arguments manually in Java,
    // since ORB.init() doesn't do it for us.
    args=stripOrbArgs(args);

    // Process Options
    boolean verbose =false;
    String channelName ="EventChannel";
    String channelKind ="EventChannel";
    String factoryName ="EventChannelFactory";
    String factoryKind ="EventChannelFactory";
    NVP[] criteria =new NVP[0];

    Getopt g =new Getopt("eventc",args,"n:k:N:K:m:c:i:p:q:R:r:vh");
    int c;
    while ((c = g.getopt()) != -1)
    {
      switch (c)
      {
      case 'n':
        channelName=g.getOptarg();
        break;

      case 'k':
        channelKind=g.getOptarg();
        break;

      case 'N':
        factoryName=g.getOptarg();
        break;

      case 'K':
        factoryKind=g.getOptarg();
        break;

      case 'm':  // OLD OPTION
        criteria=appendCriterion(criteria,"MaxEventsPerConsumer",g.getOptarg());
        break;

      case 'c':
        criteria=appendCriterion(criteria,"CyclePeriod_ns",g.getOptarg());
        break;

      case 'i':
        criteria=appendCriterionStr(criteria,"InsName",g.getOptarg());
        break;

      case 'p':
        criteria=appendCriterion(criteria,"MaxNumProxies",g.getOptarg());
        break;

      case 'q':
        criteria=appendCriterion(criteria,"MaxQueueLength",g.getOptarg());
        break;

      case 'R':
        criteria=appendCriterion(criteria,"PullRetryPeriod_ms",g.getOptarg());
        break;

      case 'r': // This option is deprecated in favour of -R:
        criteria=appendCriterion(criteria,"PullRetryPeriod",g.getOptarg());
        break;

      case 'v':
        verbose=true;
        break;

      case 'h':
        usage();
        System.exit(0);
 
      default :
        usage();
        System.exit(-1);
      }
    }

    //
    // Use one big try...catch block.
    // 'action' variable keeps track of what we're doing.
    String action ="start";
    try
    {

      //
      // Get Name Service root context.
      action="resolve initial reference 'NameService'";
      org.omg.CORBA.Object obj =orb.resolve_initial_references("NameService");
      NamingContext rootContext=NamingContextHelper.narrow(obj);

      //
      // Obtain reference to the Event Channel Factory implementation.
      // (from command-line argument or from the Naming Service).
      if(g.getOptind()<args.length)
      {
        action="convert URI from command line into object reference";
        obj=orb.string_to_object(args[g.getOptind()]);
      }
      else
      {
        if(rootContext==null)
            throw new OBJECT_NOT_EXIST();

        action="find Event Channel Factory in naming service";
        NameComponent name[] ={ new NameComponent(factoryName,factoryKind) };

        obj=rootContext.resolve(name);
      }

      action="narrow object reference to event channel factory";
      EventChannelFactory factory =EventChannelFactoryHelper.narrow(obj);
      if(factory==null)
      {
        System.err.println("Failed to narrow Event Channel Factory reference.");
        System.exit(1);
      }

      // Check that the factory is of the right type
      action="check factory supports EventChannel object interface";
      NameComponent key[] ={
        new NameComponent("EventChannel","object interface")
      };

      if(!factory._supports(key))
      {
        System.err.println("Factory does not support Event Channel Interface!"
          +" [\""+factoryName+"\", \""+factoryKind+"\"]"
        );
        System.exit(1);
      }

      //
      // Create Event Channel Object.
      action="create EventChannel object";
      org.omg.CORBA.Object channelObj =factory.create_object(key, criteria);
      if(channelObj==null)
      {
        System.err.println("Channel Factory returned nil reference! [\""
          +channelName+"\", \""+channelKind+"\"]"
        );
        System.exit(1);
      }

      // Narrow object returned to an Event Channel
      EventChannel channel =EventChannelHelper.narrow(channelObj);
      if(channel==null)
      {
        System.err.println("Failed to narrow Event Channel ! [\""
          +channelName+"\", \""+channelKind+"\"]"
        );
        System.exit(1);
      }

      // Print the new EventChannel's IOR to standard output.
      if(verbose)
      {
        System.out.println(orb.object_to_string(channel));
      }

      //
      // Register event channel with naming service
      if(rootContext!=null)
      {
        NameComponent name[] ={ new NameComponent(channelName,channelKind) };

        try{
          action="register (bind) EventChannel with the naming service";
          rootContext.bind(name,channel);
        }
        catch(AlreadyBound ex) {
          action="register (rebind) EventChannel with the naming service";
          rootContext.rebind(name,channel);
        }
      }

      //
      // Clean up nicely.
      action="destroy orb";
      orb.destroy();

      //
      // If we get here, then everything has worked OK.
    }
    catch (NoFactory ex) /* create_object() */ {
      System.err.println("Failed to create Event Channel: NoFactory"
        +" (interface not supported) ");
    }
    catch (CannotMeetCriteria ex) /* create_object() */ {
      System.err.println("Failed to create Event Channel: CannotMeetCriteria ");
    }
    catch (InvalidCriteria ex) /* create_object() */ {
      System.err.println("Failed to create Event Channel: InvalidCriteria ");
    }
    catch (COMM_FAILURE ex) {
      System.err.println("System exception, unable to "+action+": COMM_FAILURE");
    }
    catch (SystemException ex) {
      System.err.println("System exception, unable to "+action);
    }
    catch (UserException ex) {
      System.err.println("CORBA exception, unable to "+action);
    }
    catch (java.lang.Exception ex) {
      System.err.println("Java exception, unable to "+action);
      ex.printStackTrace();
    }
  } // end main


  static String[]
  stripOrbArgs(String[] args)
  {
    int len =0;
    for(int i=0; i<args.length; ++i)
    {
      if(args[i].startsWith("-ORB"))
          ++i; // Skip this arg AND the next one.
      else
          args[len++]=args[i]; // keep this arg.
    }
    String[] result =new String[len];
    System.arraycopy(args,0,result,0,len);
    return result;
  }


  static void
  usage()
  {
    System.err.println(
 "\nCreate an EventChannel and register it in the naming service.\n"
+"syntax: java Eventc OPTIONS [FACTORY_URI]\n"
+"\n"
+"FACTORY_URI: The factory may be specified as a URI.\n"
+" This may be an IOR, or a corbaloc::: or corbaname::: URI.\n"
+" For example: corbaloc::localhost:11169/omniEvents\n"
+"\n"
+"OPTIONS:                                         DEFAULT:\n"
+" -n channel name                                  [\"EventChannel\"]\n"
+" -k channel kind                                  [\"EventChannel\"]\n"
+" -N factory name (if URI is not specified)        [\"EventChannelFactory\"]\n"
+" -K factory kind (if URI is not specified)        [\"EventChannelFactory\"]\n"
+" -c override default CyclePeriod_ns of new channel (nanoseconds)\n"
+" -i set the InsName of new channel, to enable access via corbaloc\n"
+" -p override default MaxNumProxies of new channel\n"
+" -q override default MaxQueueLength of new channel\n"
+" -R override default PullRetryPeriod_ms for new channel (milliseconds)\n"
+" -v print the IOR of the new EventChannel to standard output.\n"
+" -h display this help text\n"
+"OLD OPTIONS: (only used by omniEvents v2.4 and earlier)\n"
+" -m override default MaxEventsPerConsumer for new channel\n");
  }

  static NVP[] appendCriterion(NVP[] criteria, String name, String value)
  {
    Integer valueAsInteger =new Integer(value);
    NVP criterion =new NVP();
    criterion.name=name;
    criterion.value=orb.create_any();
    criterion.value.insert_ulong(valueAsInteger.intValue());
    
    // Append criterion to criteria array.
    NVP[] result =new NVP[1+criteria.length];
    System.arraycopy(criteria,0,result,0,criteria.length);
    result[criteria.length]=criterion;
    return result;
  }

  static NVP[] appendCriterionStr(NVP[] criteria, String name, String value)
  {
    NVP criterion =new NVP();
    criterion.name=name;
    criterion.value=orb.create_any();
    criterion.value.insert_string(value);
    
    // Append criterion to criteria array.
    NVP[] result =new NVP[1+criteria.length];
    System.arraycopy(criteria,0,result,0,criteria.length);
    result[criteria.length]=criterion;
    return result;
  }
} // end class Eventc
