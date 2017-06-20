# LongInteger

This started out as part of a completely different piece of work. I ended up needing to do calculations with a number bigger than MAX_INT,
so I created a wrapped class to hold this.
Over time I've gradually expanded this class to have more and more functionality. I've also amended it to run on Linux (tested on Ubuntu).

Internally it holds the values as a byte array. A UINT is used to index the array, which gives it a size limit of 4,294,967,296 digits. The largest number it can hold should be around 1.09e+2466, which is big enough for most purposes.

It supports:
- Addition
- Subtraction
- Multiplication (uses Karatsuba algorithm. Toom-Cook3 algorithm. Multi-threading.)
- Division (Burnikel/Ziegler algorithm)
- Factorials (uses an adaption of Roman Pasachnik's Prime Swing algorithm)
- Bitwise operations
- Powers
- Square Root (uses Babylonian algorithm - performance seems okayish)
- Donald Knuth's up-arrow function (hyper-exponentials)
- Comparisons
- operator overloads for everything I could think of

To get the code to run on Ubuntu you need the following files:
GeneralIntWrapper.h
GeneralIntWrapper.hpp
LongInteger.h
LongInteger.cpp
MyHardwareInfo.h
MyHardwareInfo.cpp
QueueOfThreads.h
QueueOfThreads.hpp
ReceiveUpdateClass.h
ReceiveUpdateClass.hpp
PrimeSieve.h
PrimeSieve.cpp
PrimeSwing.h
PrimeSwing.cpp
UtilityFunctions.h
UtilityFunctions.cpp
UtilityTypes.h
main.cpp (optional - you can design your own)


If you are using a current version of GCC (tested with GCC 5.4 on Ubuntu) you can build the code using the following options:
-std=c++14 -pthread
In NetBeans or Eclipse you can add these options by going to Project Properties -> Build -> C++ Compiler -> Additional Options

I've added a piece of code to the top of LongIntegers.h that adds a definition for 'make_unique<>' if it detects that the compiler doesn't support C++14, but does support C++11.
This turned out to be all that was needed to allow the code to build on older compilers that only have C++11 support.
Options for compiler with C++11 support:
-std=c++11 -pthread

From the command line:
g++ -std=c++11 -pthread  main.cpp LongInteger.cpp MyHardwareInfo.cpp UtilityFunctions.cpp PrimeSieve.cpp PrimeSwing.cpp -o LongIntegerTest.exe

The code will not work if the compiler does not support at least C++11.

The C++11 code has been tested using GCC on a variety of Linux platforms. On Solaris I couldn't get an up-to-date version of GCC working, so I tried Solaris Studio. This builds the code, but errors when it is running, claiming a pure virtual method has been called (but rather nicely doesn't point out what that method actually is). After leaping through a huge number of hoops I finally managed to get GCC 6.3 to build on Solaris and the code runs fine when built from the command line.

How to get GCC 6.3 built on Solaris:
I reckon this is useful info as it took quite some time to find the correct process. Final method was taken from StackOverflow, but I've forgotten the link.
First go to PackageManager and install the standard GCC option, which will probably be 5.4.
Download and extract GCC 6.3 source code into a folder somewhere. I used /opt/gcc/
Run the following:
cd /opt/gcc
./contrib/download_prerequisites
cd ..
mkdir objdir
cd objdir
$PWD/../gcc/configure --prefix=/opt/gcc 
gmake
gmake install

Note that 'gmake' will probably take several hours. Don't use the -j option to run multiple jobs as this ends up erroring.

After 'gmake install' has finished add /opt/gcc/bin to the PATH variable in the .profile file in your home folder. Put it before /usr/bin so it doesn't read the older version of GCC you used to build the 6.3 version.


Testing on OpenSUSE produced an odd outcome. It errors when I try to build from the command line (the errors are claiming all the standard library functions are missing, which is usually a sign that it doesn't support C++ 11), but when I build it from NetBeans - using GCC (and it is definately pointing at the same version) - it works just fine. So that is a puzzle. My command line options work on other brands of Linux so I dunno what is different.


ToDo:
- Work out the exact point to switch to the BZ division algorithm
- Work out the best stepping size to use when multiplying the divisor (when dividing a very large number by a number less than half
    the size, it is quicker to multiply the divisor by a large factor, do the division and then multiply the quotient and divide the
    modulus, adding the remainder to the new quotient and using the new modulus as the final one. This sounds like a lot more work - and
    it is - but it is quicker. But is it quicker to multiply/divide the initial result by the whole offset amount, or use a smaller amount
    in a loop? Testing doesn't yet show up a clear answer - i.e. the timings are all over the place.
- Find out why ToomCook3's performance is as expected in Debug builds, but is frankly bizarre in Release builds (slower than Long
Multiplication except in the bands between multiples of 350 & 400 - i.e. 350-400 digits, 700-800, 1050-1200, etc - where it is much
faster)
- Do more testings of the Karatsuba algorithm in Release builds. Rechecking it (during the TC3 tests) shows it seems much less efficient than Long Division. Either that, or Long Division gets much more efficient in Release builds. The crossover point seems to shift from 40-50 digits up to nearly 1000 digits in Release builds.
- Find a quicker power algorithm (a quicker way was found, but it wasn't a huge improvement)
- Find a quicker way to convert from Base 256 to decimal
- Make the classes more generic so as to lower the interdependencies.

Done:
- Added multithreading to ToomCook3 and Karatsuba algorithms. Can't see any way to add it to the Burnikel-Ziegler division algorithm as each step requires the output of the immediately preceeding step.
- Made the QueueOfThreads class a template.
- Replace LongIntWrapper with template class GeneralIntWrapper.
- Tidy up multithreading for ToomCook3 algorithm.
- Integrate ToomCook3 into the standard multiplication algorithm
- Add factorial algorithm
- Add square root function
- Add square function (front-end to power algorithm which does pow(2))
- Removed tests for UINTs being less than zero (legacy of when I was using ints for indexing)
- Integrate the BZ division algorithm into the division code, switching to the appropriate algorithm based on the size of the numbers.
