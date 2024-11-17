#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cstdlib>
#include <fstream> // Include this header for file operations

class MathUtils
{
public:
    // Corrected method signature and fixed parameter names
    static float getMax(float i1, float i2)
    {
        return (i1 > i2) ? i1 : i2; // Use the correct variable names
    }
    static float getMin(float i1, float i2)
    {
        return (i1 < i2) ? i1 : i2; // Use the correct variable names
    }

private:
    // Typically, static utility classes do not need constructors or destructors
    MathUtils() = delete; // Delete default constructor to prevent instantiation
};

#endif // MATHUTILS_H
