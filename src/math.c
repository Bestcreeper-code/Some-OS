#include "headers/math.h"


double factorial(int n) {
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}


double pow(double base, int exp) {
    double result = 1.0;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

double sqrt(double x) {
    double result;
    __asm__ (
        "fldl %1\n"    // Load double x onto FPU stack
        "fsqrt\n"      // Compute square root of st(0)
        "fstpl %0\n"   // Store the result from st(0) into result
        : "=m" (result)   // output
        : "m" (x)         // input
    );
    return result;
}

// Normalize angle to range [-PI, PI] for better convergence
double normalize_angle(double x) {
    while (x > M_PI) x -= 2 * M_PI;
    while (x < -M_PI) x += 2 * M_PI;
    return x;
}

// Cosine using Taylor series expansion
double cos(double x) {
    x = normalize_angle(x);
    double sum = 0.0;
    int terms = 10;

    for (int n = 0; n < terms; ++n) {
        double term = pow(-1, n) * pow(x, 2 * n) / factorial(2 * n);
        sum += term;
    }
    return sum;
}

// Sine using Taylor series expansion
double sin(double x) {
    x = normalize_angle(x);
    double sum = 0.0;
    int terms = 10;

    for (int n = 0; n < terms; ++n) {
        double term = pow(-1, n) * pow(x, 2 * n + 1) / factorial(2 * n + 1);
        sum += term;
    }
    return sum;
}

// Tangent as sin(x)/cos(x)
double tan(double x) {
    double cos_val = cos(x);
    if (cos_val == 0.0) {
        // Handle division by zero (tangent undefined)
        // Here just return large number (or could return NAN)
        return 1e10; 
    }
    return sin(x) / cos_val;
}
