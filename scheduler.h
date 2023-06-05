#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <limits>
#include <iostream>

const int inf = std::numeric_limits<int>::max();
const int io_time = 20;

class Scheduler;

class Process {
private:
   Scheduler* s;
   int pid;
   int priority;
   bool state;
   int exec_time, remainder;
   int time_slice;
   int burst;
   int io_op; 

protected:
   static int s_id;
   static int clock;

public:
   Process(Scheduler* s, int burst, int io_op) : s(s), priority(0), state(false), remainder(burst),   
    time_slice(0), burst(burst), io_op(io_op) {
      this->pid = s_id++;
      this->exec_time = (this->io_op * 2 - 1) * this->burst;
   }

   int getPID() {
      return this->pid;
   }

   void changePriority() {
      this->priority = (this->priority + 1) % 3;
   }
   
   int getPriority() {
      return this->priority;
   }
   
   void toggleState() {
      this->state = !this->state;
   }
   
   bool getState() {
      return this->state;
   }

   void setTimeSlice(int quantum) {
      if(this->remainder == 0) this->remainder = burst;
      this->time_slice = std::min(quantum, this->remainder);
   }

   int getExecTime() {
      return this->exec_time;
   }

   void consumeTime(int time) {
      this->remainder -= time;
      this->exec_time -= time;
   }

   void operator()();
};

int Process::s_id = 0;
int Process::clock = 0;

class IODevice {
private:
   std::queue<Process> processes;
   bool idle;
   std::mutex mutex;
   std::condition_variable cv;

public:
   IODevice() : idle(true) {};

   void requestIO(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      processes.push(p);
      Process& front = processes.front();
      while(!idle || p.getPID() != front.getPID()) 
         cv.wait(lock);
      processes.pop();
      idle = false; 
      std::cout << "Process #" << p.getPID() << " request I/O" <<  std::endl;
      time_t start = time(0); int dt = 0;
      while(dt < io_time) 
         dt = time(0) - start;
      std::cout << "Process #" << p.getPID() << " release I/O" <<  std::endl;
      releaseIO();
      if(p.getExecTime() > 0) p();
   }

   void releaseIO() {
      std::unique_lock<std::mutex> lock(mutex);
      idle = true;
      cv.notify_all();
   }
};

class Scheduler {
private:
   std::vector<std::queue<Process>*> queues;
   IODevice* io;
   Process *front, *curr;
   std::mutex mutex;
   std::condition_variable cv;
   int quantum[3] = { 10, 15, inf };
   bool idle;

public:
   Scheduler() : front(nullptr), curr(nullptr), idle(true) {
      io = new IODevice();
      queues.push_back(new std::queue<Process>);
      queues.push_back(new std::queue<Process>);
      queues.push_back(new std::queue<Process>);
   }

   void requestCPU(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      enqueueProcess(p);
      scheduleProcess();
      while(!idle || p.getPID() != front->getPID()) 
         cv.wait(lock);
      curr = &p; curr->toggleState(); 
      idle = false; 
   }
   
   void releaseCPU() {
      std::unique_lock<std::mutex> lock(mutex);
      Process& p = *curr;
      curr->toggleState();
      dequeueProcess();
      scheduleProcess(); 
      idle = true;
      cv.notify_all();
      // if(p.getPriority() == 3) io->requestIO(p);
   }
   
   void scheduleProcess() {
      for(auto queue : queues) {
         if(!queue->empty()) {
            Process& p = queue->front();
            front = &p;
            break;
         }
      }
      if(!front || !curr) return;
      if(front->getPID() != curr->getPID() && curr->getState()) 
         curr->toggleState();
   }

   void enqueueProcess(Process& p) {
      queues[p.getPriority()]->push(p);
      p.setTimeSlice(quantum[p.getPriority()]);
   }   
   
   void dequeueProcess() {
      queues[curr->getPriority()]->pop(); // TODO: Adicionar lógica para colocar na fila de I/O
      curr->changePriority();
   }
   
   void preempt(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      idle = true;
      while(!idle || p.getPID() != front->getPID()) 
         cv.wait(lock);
      curr = &p; curr->toggleState(); 
      idle = false; 
   }
};

void Process::operator()() {
   s->requestCPU(*this);
   while(true) {
      std::cout << "Process #" << pid << " started execution at time " << clock <<  std::endl;
      time_t start = time(0); int dt = 0;
      while(dt < time_slice && state) 
         dt = time(0) - start;
      clock += dt; consumeTime(dt);
      std::cout << "Process #" << pid << " ended execution at time   " << clock << std::endl;
      if(state) break;
      s->preempt(*this);
   }
   s->releaseCPU();
   if(exec_time > 0) (*this)();
}

#endif
