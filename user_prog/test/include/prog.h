#ifndef PROGRAM_H
#define PROGRAM_H

#include <Hx86/stdint.h>
#include <Hx86/utils/string.h>

/**
 * @typedef constructor
 * @brief Defines a pointer to a function with no arguments and no return value.
 *
 * This is used to reference global constructors during initialization.
 */
typedef void (*constructor)();

/**
 * @brief External declaration for the start and end of the constructors section.
 *
 * These symbols are defined by the linker and mark the range of global constructors to call during
 * initialization.
 */
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

/**
 * @brief Calls all global constructors in the range defined by `start_ctors` and `end_ctors`.
 *
 * This function is called during kernel initialization to ensure all static/global objects are
 * properly constructed.
 */
extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();  // Call each constructor in the range.
    }
}

// Forward declaration of Calculator class
class Calculator {
public:
    Calculator();
    ~Calculator();

    void onPressNum(uint32_t num);
    void onPressFunc(char func);
    void evaluate();
    void clearCalculator();

private:
    Window* mainWindow;
    Label* screen;
    Button* btn_0;
    Button* btn_1;
    Button* btn_2;
    Button* btn_3;
    Button* btn_4;
    Button* btn_5;
    Button* btn_6;
    Button* btn_7;
    Button* btn_8;
    Button* btn_9;
    Button* btn_dot;
    Button* btn_plus;
    Button* btn_minus;
    Button* btn_multiplication;
    Button* btn_division;
    Button* btn_solve;
    Button* btn_clear;

    char input[64] = {0};
    int inputIndex = 0;
    double currentValue = 0;
    char lastOperator = 0;
    bool newInput = true;
    bool hasDecimal = false;
    bool hasResult = false;
};

#endif  // PROGRAM_H
