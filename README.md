# LongInteger

This started out as part of a completely different piece of work. I ended up needing to do calculations with a number bigger than MAX_INT,
so I created a wrapped class to hold this.
Over time I've gradually expanded this class to have more and more functionality.

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
