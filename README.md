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

-----------------
How to install GCC 6.3 on Solaris.<br>
The GCC builds available via the package manager are rather old and don't properly support C++14.<br>
Warning - this requires over 10Gb free working space during the build, though most of this can be removed afterwards.<br>
So to get GCC 6.3 running, first install any old GCC from the package manager so you have a C and C++ compiler and linker.
Extract gcc source code into a folder somewhere where you have full access permissions. Example assumes you are using /opt/gcc/
Then type the following at a terminal:<br>
cd /opt/gcc<br>
./contrib/download_prerequisites<br>
cd ..<br>
mkdir objdir<br>
cd objdir<br>
$PWD/../gcc/configure --prefix=/opt/gcc <br>
gmake (and wait for several days as it compiles)<br>
gmake install<br>

Afterwards the objdir directory can be deleted as can all directories bar 6 in the gcc directory.
The 6 sub-directories to keep are:<br>
gcc/bin<br>
gcc/include<br>
gcc/lib<br>
gcc/lib32<br>
gcc/lib64<br>
gcc/libexec<br>

Remove the older version of GCC via the package manager.
Include the gcc/bin path in your working path and you will have access to a compiler that can build C++14 code.<br>
Test this by typing 'gcc --version' at the console.

For other platforms it should be the same process.

Differences for Ubuntu:<br>
sudo apt-get install gcc-multilib (if Ubuntu complains about no 32-bit libraries when using configure)<br>
'make' instead of 'gmake'<br>
Then...<br>
Edit .bashrc in the home directory and add these 2 lines to the end:<br>
LD_LIBRARY_PATH=/opt/gcc/lib:/opt/gcc/lib32:/opt/gcc/lib64:/lib:/usr/lib:/usr/local/lib<br>
export LD_LIBRARY_PATH<br>

For some reason on Solaris it worked fine without any configuration changes and seemed to be getting the right libraries.
----------------



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
