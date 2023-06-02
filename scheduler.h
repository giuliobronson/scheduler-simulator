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

   bool operator<(const Process& other) const {
      return priority < other.priority;
   }

   void operator()();
};

int Process::s_id = 0;

class Queue {
public:
   virtual Process front() = 0;

   virtual void push(const Process& p) = 0;

   virtual void pop() = 0;

   virtual bool empty() = 0;
};

class RoundRobin : public Queue {
private:
   std::queue<Process> q;

public:
   Process front() {
      return q.front();
   }

   void push(const Process& p) {
      q.push(p);
   }

   void pop() {
      q.pop();
   }

   bool empty() {
      return q.empty();
   }
};

class FCFS : public Queue {
private:
   std::priority_queue<Process> pq;

public:
   Process front() {
      return pq.top();
   }

   void push(const Process& p) {
      pq.push(p);
   }

   void pop() {
      pq.pop();
   }

   bool empty() {
      return pq.empty();
   }
};

class Scheduler {
private:
   std::vector<Queue*> queues;
   Process *front, *curr;
   bool idle;
   std::mutex mutex;
   std::condition_variable cv;

public:
   Scheduler() : idle(true) {
      queues.push_back(new RoundRobin());
      queues.push_back(new RoundRobin());
      queues.push_back(new FCFS());
   }

   void request_cpu(Process* p) {
      std::unique_lock<std::mutex> lock(mutex);
      enqueue_process(p);
      schedule_process();
      while(!idle && p->getPID() != front->getPID()) 
         cv.wait(lock);
      curr = p; curr->toggleState();
      idle = false;
   }
   
   void release_cpu() {
      std::unique_lock<std::mutex> lock(mutex);
      std::cout << 1;
      curr->toggleState();
      dequeue_process();
      schedule_process();
      idle = true;
      cv.notify_all();
   }
   
   void schedule_process() {
      for(auto queue : queues) {
         if(!queue->empty()) {
            Process p = queue->front();
            front = &p;
            break;
         }
      }
      if(!front || !curr) return;
      if(front->getPID() != curr->getPID() && curr->getState()) {
         preempt();
         curr = front;
      }
   }

   void enqueue_process(Process* p) {
      queues[p->getPriority()]->push(*p);
   }   
   
   void dequeue_process() {
      queues[curr->getPriority()]->pop();
      curr->changePriority();
   }
   
   void preempt() {
      curr->toggleState();
      idle = true;
      request_cpu(curr);
      cv.notify_all();
   }
};

void Process::operator()() {
   s->request_cpu(this);
   time_t start = time(0);
   int dt = time(0) - start;
   while (dt < burst && state) {
      std::cout << "Process #" << pid << " executing at time " << dt << std::endl;
      dt = time(0) - start;
   }
   s->release_cpu();
}

#endif
