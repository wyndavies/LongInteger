# LongInteger

This started out as part of a completely different piece of work. I ended up needing to do calculations with a number bigger than MAX_INT,
so I created a wrapped class to hold this.
Over time I've gradually expanded this class to have more and more functionality.

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
