#ifndef PROCESS_TYPES_H
#define PROCESS_TYPES_H

#include <core/memory.h>
#include <types.h>
#include <utils/linkedList.h>

enum ThreadState {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_TERMINATED
};

// --------------------------------------------------------------------------
// CONTEXT (FIXED for pusha/iret)
// --------------------------------------------------------------------------
struct CPUState {
    // General Purpose Registers
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;

    // Segment Registers
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    // Interrupt Information
    uint32_t error;

    // Return State
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

class Process;  // Forward declaration

struct ProcessControlBlock;  // Forward declaration

struct HeapSegment {
    uint32_t startAddress;
    uint32_t endAddress;
    uint32_t maxAddress;
};

struct ThreadControlBlock {
    uint32_t tid;
    uint32_t pid;
    ThreadState state;

    uint8_t* stack;
    CPUState* context;
    ProcessControlBlock* parent;
    uint32_t wakeTime;
};

struct ProcessControlBlock {
    uint32_t pid;

    uint32_t* page_directory;

    LinkedList<ThreadControlBlock*> threads;
    bool isKernelProcess;
    HeapSegment heap;
};

#endif  // PROCESS_TYPES_H
