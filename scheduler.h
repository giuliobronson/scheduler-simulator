#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <limits>

class Scheduler;

class Process {
private:
   Scheduler* s;
   const int burst;
   int pid;
   int priority;
   bool state;
   int io_op; 
   int total_time;

protected:
   static int s_id;
   static int clock;

public:
   Process(Scheduler* s, int burst, int io_op) : s(s), burst(burst), io_op(io_op), priority(0), 
   state(false), total_time(0) {
      this->pid = ++s_id;
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
   
   int getPriority() {
      return this->priority;
   }

   int getTimeSlice() {
    int timeSlice;
    int quantum = s->getQuantum(priority);
    timeSlice = burst > quantum ? quantum : burst;
    return timeSlice;
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
   int quantumList[3];

public:
   Scheduler(int firstQuantum, int secondQuantum) : front(nullptr), curr(nullptr), idle(true) {
      queues.push_back(new std::queue<Process>); // Round Robin (quantum=10)
      queues.push_back(new std::queue<Process>); // Round Robin (quantum=15)
      queues.push_back(new std::queue<Process>); // FCFS
      queues.push_back(new std::queue<Process>); // IO Device (exec_time = 20)
      int infin = std::numeric_limits<int>::max();
      quantumList[3] = (firstQuantum, secondQuantum, infin);
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

   int getQuantum(int priority) {
    return quantumList[priority];
   }
};

void Process::operator()() {
   s->request_cpu(*this);
   int timeSlice = getTimeSlice();
   while(true) {
      std::cout << "Process #" << pid << " started execution at time " << clock <<  std::endl;
      time_t start = time(0); int dt = 0;
      while(dt < timeSlice && state) 
         dt = time(0) - start;
      total_time += dt;
      clock += dt;
      std::cout << "Process #" << pid << " ended execution at time " << clock << std::endl;
      if(total_time == burst) {
        if(io_op) {
          // Adicionar à fila do dispositivo
          // request_io()
          io_op--;
          total_time = 0;
        } 
        else break;
      }
      else
        s->preempt(*this);
   }
   s->release_cpu();
}

#endif

// 1ª OPÇÃO
  // O processo é finalizado no primeiro quantum
  // release_cpu()
    // Retirar o processo da fila atual
    // Alocar próximo processo à CPU

// 2ª OPÇÃO
  // O processo não é finalizado no primeiro quantum
  // ?????
    // Retirar processo da fila atual
    // Realocar na fila seguinte
  // schedule_process()
    // Marcar o processo como FRONT
  // request_cpu()
    // Alocar o processo à CPU
  // O processo é finalizado
  // release_cpu()
    // Retirar o processo da fila atual
    // Alocar próximo processo à CPU

// 3ª OPÇÃO
  // O processo possui uma operação de I/O
  // O precesso segue a 1ª ou a 2ª OPÇÃO 
  // ???????
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

