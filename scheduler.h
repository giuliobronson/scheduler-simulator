#include <thread>
#include <mutex>
#include <condition_variable>
#include "queue.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

class Scheduler {
  private:
    std::condition_variable cv;
    RoundRobin *rr1, *rr2;
    Queue *fcfs;

  public:
    Scheduler(RoundRobin *rr1, RoundRobin *rr2, Queue *fcfs): rr1(rr1), rr2(rr2), fcfs(fcfs) {}

    void request_cpu_time(Process *p) {
      this->rr1->push(p);
    }

    void release_cpu_time() {

    }
};

#endif