BUILD INSTRUCTIONS FOR WINDOWS

1. Prerequisites

 o Microsoft Visual C++ compiler. (Tested with version 6.0)
   The environment variables for command-line compiling must be set up.
   You can test this by trying to compile `hello.cc' (in this directory)
   with the command:
   
   cl -TP -GX -MD hello.cc

 o omniORB4. Get it from http://omniorb.sourceforge.net/
   You should set up your PATH environment to include:
   <omniORB Top-Level Directory>\bin\x86_win32
   Test this by checking that this command prints out the omniidl help:
   
   omniidl -u

 o A fairly recent version of GNU make for Windows (3.78.1 or above).
   Download it from http://unxutils.sourceforge.net/ or Google for `gnu make
   windows'. The `make.exe' also needs to be in the PATH. For example, you
   could copy it into your C:\winnt directory. The following command should
   show version text:

   make --help

2. Preparation

 o Copy the file `config.h' from this directory to:
   <omniEvents Top-Level Directory>\src\config.h

 o Copy the file `config.mk' from this directory to:
   <omniEvents Top-Level Directory>\config.mk

 o Edit `config.mk' to set the values of variables:
   OMNIORB_BASE    - full path to omniORB top-level directory.
   OMNIORB_LIBS    - libraries provided by omniORB.
   OMNIEVENTS_BASE - full path to omniEvents top-level directory.

3. Compile

   Open a command window, and `cd' to the omniEvents top-level directory.
   The following command builds omniEvents:
   
   make

4. Use it

   See the notes for Windows Users in the main README file for important
   guidelines on how to use omniEvents on Windows.

<end>   