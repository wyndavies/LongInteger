# LongInteger

This started out as part of a completely different piece of work. I ended up needing to do calculations with a number bigger than MAX_INT,
so I created a wrapped class to hold this.
Over time I've gradually expanded this class to have more and more functionality. I've also amended it to run on Linux (tested on Ubuntu).

Internally it holds the values as a byte array. A UINT is used to index the array, which gives it a size limit of 4,294,967,296 digits. The largest number it can hold should be around 1.09e+2466, which is big enough for most purposes.

It supports:
- Addition
- Subtraction
- Multiplication (uses Karatsuba algorithm. Toom-Cook3 algorithm in testing)
- Division (Burnikel/Ziegler algorithm being developed)
- Bitwise operations
- Powers
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
main.cpp (optional - you can design your own)


If you are using a current version of GCC (tested with GCC 5.4 on Ubuntu) you can build the code using the following options:
-std=c++14 -pthread
In NetBeans or Eclipse you can add these options by going to Project Properties -> Build -> C++ Compiler -> Additional Options

If you are using an older version of GCC or a compiler that doesn't have C++14 support, but does have C++11 support then you need to set slightly different options. There is a small piece of code at the top of LongIntegers.h that adds a definition for make_unique<> if it detects that the compiler is running in C++11 mode.
Options for compiler with C++11 support:
-std=c++11 -pthread

The C++11 code has been tested using GCC on a variety of Linux platforms. On Solaris I couldn't get an up-to-date version of GCC working, so I used Solaris Studio. This builds the code, but errors when it is running, claiming a pure virtual method has been called (but rather nicely doesn't point out what that method actually is).


ToDo:
- Remove some remaining tests for UINTs being less than zero (legacy of when I was using ints for indexing)
- Finish off the BZ division algorithm and integrate it into the standard division call
    - Work out the exact point to switch between algorithms
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
