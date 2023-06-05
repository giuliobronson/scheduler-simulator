#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <limits>
#include <chrono>
#include <iostream>

long long getCurrentTime() {
    auto current_time = std::chrono::system_clock::now();
    auto duration = current_time.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count() / 100; // Ajuste para simulação de tempo
}

const int inf = std::numeric_limits<int>::max();

class Scheduler;

/*
* Classe Process simula uma processo 
* que requisita tempo de CPU
*/
class Process {
private:
   Scheduler* s;
   int pid;
   int priority;
   bool state; // true se processo está tivo
   int exec_time; // Tempo de execução total
   int remainder; // Restante de tempo no burst após início de execução
   int time_slice; // Parcela de tempo designada pelo escalanador ao processo
   int burst;
   int io_op; 

protected:
   static int s_id;

public:
   Process(Scheduler* s, int burst, int io_op) : s(s), priority(0), state(false), remainder(burst),   
    time_slice(0), burst(burst), io_op(io_op) {
      this->pid = s_id++;
      this->exec_time = (this->io_op + 1) * this->burst;
   }

   int getPID() {
      return this->pid;
   }

   void changePriority() {
      if(this->remainder <= 0) this->priority = -1;
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

   int getBurstTime() {
      return this->remainder;
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

/*
* Classe IODevice simula a fila de processos
* esperando operações de I/O em um dispositivo
*/
class IODevice {
private:
   const int io_time = 20; // Tempo de I/O
   std::queue<Process> processes;
   Process *front;
   std::mutex mutex;
   std::condition_variable cv;
   bool idle; // true se dispositivo está ocioso

public:
   IODevice() : front(nullptr), idle(true) {};

   void requestIO(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      processes.push(p);
      Process& tmp = processes.front();
      front = &tmp;
      while(!idle || p.getPID() != front->getPID()) 
         cv.wait(lock);
      processes.pop();
      tmp = processes.front();
      front = &tmp;
      idle = false;  

      // Executa tarefa
      std::cout << "Process #" << p.getPID() << " request I/O" <<  std::endl;
      long long start = getCurrentTime(); long long dt = 1;
      while(dt < io_time) 
         dt = getCurrentTime() - start;
      std::cout << "Process #" << p.getPID() << " release I/O" <<  std::endl;

      // Libera fila de I/O
      idle = true;
      cv.notify_all();
   }
};

class Scheduler {
private:
   std::vector<std::queue<Process>*> queues; // Filas do escalonador
   IODevice *io;
   Process *front, *curr; // Processos do início da fila e o que está executando 
   std::mutex mutex;
   std::condition_variable cv;
   int quantum[3] = { 10, 15, inf }; // Quantum de cada fila
   bool idle;

protected:
   static long long clock;

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
      queues[curr->getPriority()]->pop(); 
      curr->changePriority();
   }
   
   void preempt(Process& p) {
      std::unique_lock<std::mutex> lock(mutex);
      idle = true;
      cv.notify_all();
      while(!idle || p.getPID() != front->getPID())
         cv.wait(lock);
      curr = &p; curr->toggleState(); 
      idle = false; 
   }

   void startClock() {
      this->clock = getCurrentTime();
   }

   long long getClock() {
      return getCurrentTime() - clock;
   }

   IODevice* getIODevice() {
      return io;
   }
};

long long Scheduler::clock = getCurrentTime();

void Process::operator()() {
   s->requestCPU(*this);
   while(true) {
      std::cout << "Process #" << pid << " started execution at time " << s->getClock() <<  std::endl;
      long long start = getCurrentTime(); long long dt = 1;
      while(dt < time_slice && state) 
         dt = getCurrentTime() - start;
      consumeTime(dt);
      std::cout << "Process #" << pid << " ended execution at time   " << s->getClock() << std::endl;
      if(state) break;
      setTimeSlice(time_slice - dt);
      s->preempt(*this);
   }
   s->releaseCPU();
   if(this->getBurstTime() <= 0 && exec_time > 0) {
      IODevice *io = s->getIODevice();
      io->requestIO(*this);
   }
   if(exec_time > 0) (*this)();
}

#endif
