#include <core/process.h>

ProcessManager* ProcessManager::activeInstance = nullptr;

Process::Process(GlobalDescriptorTable *gdt, Paging* pager, void (*entrypoint)(void*), void* arg)
{
    this->pager = pager;
    this->pid = ProcessManager::activeInstance ? ProcessManager::activeInstance->generatePID() : 0;
    this->gdt = gdt;

    // Initialize main thread
    AddThread(entrypoint, arg);

    // Map kernel memory
    pager->map_kernel(process_page_directory);
}

uint32_t Process::AddThread(void (*entrypoint)(void*), void* arg)
{
    if (threadsList.GetSize() >= MAX_THREADS)
        return -1; // Error: No available thread slots

    Thread* tmp = new Thread(this, this->gdt, entrypoint, arg);
    tmp->tid = generateTID();

    threadsList.PushBack(tmp);

   // DEBUG_LOG("Thread created for process : %d with TID :%d, entrypoint : 0x%x", this->pid, this->threads[threadCount]->tid, entrypoint);
    return tmp->getTID();
}

Thread* Process::ScheduleThread()
{
    if(threadsList.GetSize() == 0)
        return nullptr;
        
    Thread* currentThread = threadsList.PopFront();
    threadsList.PushBack(currentThread);
    //DEBUG_LOG("Scheduling TID : %d, in PID : %d", currentThread->getTID(), this->pid);
    return threadsList.GetFront();
}

Thread* Process::getCurrentThread()
{
    if(threadsList.GetSize() == 0)
        return nullptr;
    return threadsList.GetFront();
}

Process::~Process()
{
}




ProcessManager::ProcessManager(Paging* pager)
{
    this->pager = pager;
    activeInstance = this;
}

ProcessManager::~ProcessManager()
{
}

bool ProcessManager::AddProcess(Process* process)
{
    if (processList.GetSize() >= MAX_PROCESSES)
        return false;
    processList.PushBack(process);
    return true;
}

bool ProcessManager::RemoveProcess(Process* process)
{
    // Remove the process if it exists in the list
    if(processList.Remove([process](Process* pr) { return pr == process; })){
        currentProcess = 0;
        return true;
    } else {
        return false;
    }
}

void ProcessManager::mapKernel(Process* process)
{
    memset(process->process_page_directory, 0, sizeof(this->pager->KernelPageDirectory));
    memcpy(process->process_page_directory, this->pager->KernelPageDirectory, sizeof(this->pager->KernelPageDirectory));
}

void ProcessManager::makeKernel(Process* process)
{
    process->threadsList.GetFront()->cpustate->cs = GlobalDescriptorTable::activeInstance->CodeSegmentSelector();
    process->type = KERNEL;
}

Process* ProcessManager::getCurrentProcess()
{
    return currentProcess;
}

uint32_t ProcessManager::getCurrentPID()
{
    return  getCurrentProcess()->getPID();
}

Process* ProcessManager::getProcessByPID(uint32_t PID)
{
    return processList.Find([PID](Process* pr) {
        return pr && pr->pid == PID;
    });
}

CPUState* ProcessManager::Schedule(CPUState* cpustate)
{
    if (processList.GetSize() <= 0)
        return cpustate;

    if (currentProcess){
        // Save the state of current thread
        if(Thread* currentThread = getCurrentProcess()->getCurrentThread()) {
            currentThread->cpustate = cpustate;
        }
        // Move to next process
        Process* tmp = processList.PopFront();
        if (!tmp) return cpustate;
        processList.PushBack(tmp);
    }

    currentProcess = processList.GetFront();

    if (!currentProcess || !currentProcess->process_page_directory) {
        DEBUG_LOG("ERROR: Invalid process %d", currentProcess->pid);
        return cpustate;
    }

    // Switch address space
    pager->switch_page_directory(currentProcess->process_page_directory);
    
    // Schedule thread from the new process
    if(Thread* nextThread = currentProcess->ScheduleThread()) {
        return nextThread->cpustate;
    }
    
    return cpustate;
}
