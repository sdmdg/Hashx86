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
 * These symbols are defined by the linker and mark the range of global constructors to call during initialization.
 */
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

/**
 * @brief Calls all global constructors in the range defined by `start_ctors` and `end_ctors`.
 * 
 * This function is called during kernel initialization to ensure all static/global objects are properly constructed.
 */
extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i) (); // Call each constructor in the range.
    }
}



#endif // PROGRAM_H