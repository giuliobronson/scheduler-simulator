#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

class Scheduler;

class Process {
private:
   Scheduler* s;
   int pid;
   int priority;
   bool state;
   int burst;
   int io_op; 

protected:
   static int s_id;
   static int clock;

public:
   Process(Scheduler* s, int burst, int io_op) : s(s), burst(burst), io_op(io_op), priority(0), 
   state(false) {
      this->pid = ++s_id;
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

   void operator()();
};

int Process::s_id = 0;
int Process::clock = 0;

class Scheduler {
private:
   std::vector<std::queue<Process>*> queues;
   Process *front, *curr;
   bool idle;
   std::mutex mutex;
   std::condition_variable cv;

public:
   Scheduler() : front(nullptr), curr(nullptr), idle(true) {
      queues.push_back(new std::queue<Process>);
      queues.push_back(new std::queue<Process>);
      queues.push_back(new std::queue<Process>);
   }

   void request_cpu(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      enqueue_process(p);
      schedule_process();
      while(!idle || p.getPID() != front->getPID()) 
         cv.wait(lock);
      curr = &p; curr->toggleState(); 
      idle = false; 
   }
   
   void release_cpu() {
      std::unique_lock<std::mutex> lock(mutex);
      curr->toggleState();
      dequeue_process();
      schedule_process(); 
      idle = true;
      cv.notify_all();
   }
   
   void schedule_process() {
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

   void enqueue_process(Process& p) {
      queues[p.getPriority()]->push(p);
   }   
   
   void dequeue_process() {
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
   s->request_cpu(*this);
   while(true) {
      std::cout << "Process #" << pid << " started execution at time " << clock <<  std::endl;
      time_t start = time(0); int dt = 0;
      while(dt < burst && state) 
         dt = time(0) - start;
      clock += dt;
      std::cout << "Process #" << pid << " ended execution at time " << clock << std::endl;
      if(state) break;
      s->preempt(*this);
   }
   s->release_cpu();
}

#endif
