#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846
#define DEG_TO_RAD(angle_in_degrees) ((angle_in_degrees) * M_PI / 180.0)


double factorial(int n);
double pow(double base, int exp);
double sqrt(double x);
double normalize_angle(double x);
double cos(double angle);
double sin(double angle);
double tan(double angle);

#endif // MATH_H
