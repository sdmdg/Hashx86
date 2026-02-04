#ifndef MATH_H
#define MATH_H

#define PI 3.14159265358979323846
#define DEG2RAD(x) ((x) * PI / 180.0)

// Trigonometry
double sin(double x);
double cos(double x);
double tan(double x);

// Basic Math
double sqrt(double x);
double abs(double x);
double pow(double base, int exp);
int floor(double x);
int ceil(double x);

#endif
