#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>
#include <debug.h>
#include <core/paging.h>
#include <core/memory.h>
#include <core/thread.h>
#include <gui/eventHandler.h>
#include <utils/linkedList.h>

#define MAX_THREADS 16 // Maximum threads per process
#define MAX_PROCESSES 256 // Maximum number of processes

typedef enum {
    KERNEL = 0x0,
    USER = 0x1,
} PROCESS_TYPE;

struct HeapData {
    uint32_t heap_start;
    uint32_t heap_end;
};

class Process
{
friend class ProcessManager;
private:
    uint32_t pid;
    Paging* pager;
    LinkedList<Thread*> threadsList;
    uint32_t tidCounter = 0;
    
public:
    uint32_t process_page_directory[1024] __attribute__((aligned(4096)));
    PROCESS_TYPE type = KERNEL;
    
    Process(Paging* pager, void (*entrypoint)(void*), void* arg);
    uint32_t AddThread(void (*entrypoint)(void*), void* arg);
    Thread* ScheduleThread();
    Thread* getCurrentThread();
    ~Process();
    uint32_t getPID() { return pid; }
    uint32_t generateTID() { return tidCounter++; }
    LinkedList<Event*> eventQueue;
    HeapData heapData;
};

class ProcessManager
{
private:
    
    LinkedList<Process*> processList;
    uint32_t pidCounter = 0;
    Paging* pager;

public:
    Process* currentProcess = 0;

    static ProcessManager* activeInstance;

    ProcessManager(Paging* pager);
    ~ProcessManager();
    bool AddProcess(Process* process);
    bool RemoveProcess(Process* process);
    void mapKernel(Process* process);
    void makeKernel(Process* process);
    Process* getCurrentProcess();
    uint32_t getCurrentPID();
    Process* getProcessByPID(uint32_t PID);
    CPUState* Schedule(CPUState* cpustate);

    uint32_t generatePID() { return pidCounter++; }
};


#endif // PROCESS_H
