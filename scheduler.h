#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <limits>
#include <thread>
#include <algorithm>

class Scheduler;

class Process {
private:
   Scheduler* s;
   const int burst;
   int pid;
   int priority;
   bool state;
   int io_op; 
   int total_cpu_time;

protected:
   static int s_id;

public:
   Process(Scheduler* s, int burst, int io_op) : s(s), burst(burst), io_op(io_op), priority(0), 
   state(false), total_cpu_time(0) {
      this->pid = s_id++;
   }

   int getPID() {
      return this->pid;
   }

   void changePriority() {
      this->priority = (this->priority + 1) % 4;
   }

   void sendToIO() {
      this->priority = 3;
   }

   void setEndPriority() {
      this->priority = -1;
   }

   int getPriority() {
      return this->priority;
   }

   int getTotalTime() {
    return total_cpu_time;
   }

   int getBurst() {
    return burst;
   }

   int getIOOp() {
    return io_op;
   }
   
   void toggleState() {
      this->state = !this->state;
   }

   void setTotalTime(int time) {
      this->total_cpu_time = time;
   }

   void decrementIOOP() {
      this->io_op--;
   }
   
   bool getState() {
      return this->state;
   }

   int getTimeSlice();

   void operator()();

   void io_operation();
};

int Process::s_id = 0;

class IODevice {
private:
std::queue<Process> queue;
bool idleIO;
std::mutex mutexIO;
std::condition_variable cvIO;
Process *frontIO;

public:
IODevice(): frontIO(nullptr), idleIO(true) {}
void request_io(Process& p) {
      std::unique_lock<std::mutex> lock(mutexIO);
      queue.push(p);
      Process& pAux = queue.front();
      frontIO = &pAux;
      while(!idleIO || p.getPID() != frontIO->getPID()) 
         cvIO.wait(lock);
      idleIO = false;
      exec_io_op(p);
   }

   void exec_io_op(Process& p) {
      time_t start = time(0)*1000;
      int dt = 0;
      std::cout <<"I/O #P" << p.getPID() << " " << "Start of IO operation" << std::endl;
      while(dt < 20)
        dt = time(0)*1000 - start;
      std::cout <<"I/O #P" << p.getPID() << " " << "End of IO operation"  << std::endl;
      release_io(p);
   }
   
   void release_io(Process& p) {
      queue.pop(); 
      p.decrementIOOP();
      p.changePriority();
      Process& pAux = queue.front();
      frontIO = &pAux;
      idleIO = true;
      cvIO.notify_all();
   }

};

class Scheduler {
private:
   std::vector<std::queue<Process>*> queues;
   IODevice *deviceIO;
   Process *frontCPU, *currCPU;
   bool idleCPU, idleIO;
   std::mutex mutexCPU;
   std::condition_variable cvCPU;
   int quantumList[3];

   protected:
   static time_t start_clock;

public:
   Scheduler(int firstQuantum, int secondQuantum) : frontCPU(nullptr), currCPU(nullptr), 
    idleCPU(true) {
      queues.push_back(new std::queue<Process>); // Round Robin (quantum=10)
      queues.push_back(new std::queue<Process>); // Round Robin (quantum=15)
      queues.push_back(new std::queue<Process>); // FCFS
      int infin = std::numeric_limits<int>::max();
      quantumList[0] = firstQuantum;
      quantumList[1] = secondQuantum;
      quantumList[2] = infin;
      deviceIO = new IODevice;
   }

   IODevice* getDeviceIO() {
    return deviceIO;
   }

   int getClock() {
      return time(0)*1000 - start_clock;
   }

   void request_cpu(Process& p) {
      std::unique_lock<std::mutex> lock(mutexCPU);
      enqueue_process(p);
      schedule_process();
      while(!idleCPU || p.getPID() != frontCPU->getPID()) 
         cvCPU.wait(lock);
      currCPU = &p; currCPU->toggleState(); 
      idleCPU = false; 
   }

   void release_cpu(Process& p) {
      std::unique_lock<std::mutex> lock(mutexCPU);
      currCPU->toggleState();
      dequeue_process(p);
      schedule_process(); 
      idleCPU = true;
      cvCPU.notify_all();
   }
   
   void schedule_process() {
      for(auto queue : queues) {
         if(!queue->empty()) {
            Process& p = queue->front();
            frontCPU = &p;
            break;
         }
      }
      if(!frontCPU || !currCPU) return;
      if(frontCPU->getPID() != currCPU->getPID() && currCPU->getState()) 
         currCPU->toggleState();
   }

   void enqueue_process(Process& p) {
      queues[p.getPriority()]->push(p);
   }   
   
   void dequeue_process(Process& p) {
      queues[p.getPriority()]->pop(); 
      // Decrementar a prioridade
      if(p.getTotalTime() < p.getBurst())
        p.changePriority();
      // Alocar na fila do dispositivo I/O
      else if(p.getIOOp()) {
        p.sendToIO();
        p.setTotalTime(0);
      }
      // Altera a prioridade para -1 para indicar o término do processo
      else p.setEndPriority();
   }
   
   void preempt(Process& p) {
      std::unique_lock<std::mutex> lock(mutexCPU);
      idleCPU = true;
      while(!idleCPU || p.getPID() != frontCPU->getPID()) 
         cvCPU.wait(lock);
      currCPU = &p; currCPU->toggleState(); 
      idleCPU = false; 
   }

   int getQuantum(int priority) {
    return quantumList[priority];
   }
};

time_t Scheduler::start_clock = time(0)*1000;


int Process::getTimeSlice() {
    int timeSlice;
    int quantum = s->getQuantum(priority);
    timeSlice = std::min(quantum, burst-total_cpu_time);
    return timeSlice;
   }

void Process::operator()() {
   s->request_cpu(*this);
   while(true) {
      int timeSlice = getTimeSlice();
      std::cout << "CPU #P" << pid << " started execution at time " << s->getClock() <<  std::endl;
      time_t start = time(0)*1000; int dt = 0;
      while(dt < timeSlice && state) {
         dt = time(0)*1000 - start;
      }
      total_cpu_time += dt;
      std::cout << "CPU #P" << pid << " ended execution at time " << s->getClock() << std::endl;
      if(!state)
        s->preempt(*this);
      else
        s->release_cpu(*this);
      if(priority == 3) {
        IODevice *aux = s->getDeviceIO();
        aux->request_io(*this);
      }

      if(priority != -1)
        s->request_cpu(*this);
      else break;
   }
}

#endif

// 1ª OPÇÃO
  // O processo é finalizado no primeiro quantum
  // release_cpu()
    // Retirar o processo da fila atual
    // Alocar próximo processo à CPU

// 2ª OPÇÃO
  // O processo não é finalizado no primeiro quantum
  // release_cpu()
    // Retirar processo da fila atual
    // Realocar na fila seguinte
  // schedule_process()
    // Marcar o processo como FRONTCPU
  // request_cpu()
    // Alocar o processo à CPU
  // O processo é finalizado
  // release_cpu()
    // Retirar o processo da fila atual
    // Alocar próximo processo à CPU

// 3ª OPÇÃO
  // O processo possui uma operação de I/O
  // O precesso segue a 1ª ou a 2ª OPÇÃO 
  // release_cpu()
    // Retirar o processo da fila atual
    // Realocar na fila de I/O
  // request_io()
    // Requisitar o uso do dispositivo I/O
  // A operação de I/O é finalizada
  // realease_io()
    // Retirar o processo da fila de I/O
    // Alocar próximo processo ao dispositivo de I/O
    // Realocar na 1ª fila 
  //... 

