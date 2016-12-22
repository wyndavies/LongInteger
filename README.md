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
