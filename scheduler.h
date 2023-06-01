#include <thread>
#include <chrono>
#include <condition_variable>
#include "queue.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

class Scheduler {
  private:
    Process *front;
    RoundRobin *rr1, *rr2;
    Queue *fcfs;
    bool idle;
    std::mutex mutex;
    std::condition_variable cv;

  public:
    Scheduler(RoundRobin *rr1, RoundRobin *rr2, Queue *fcfs) : front(nullptr), rr1(rr1), rr2(rr2),
                                                               fcfs(fcfs), idle(true) {}

    void request_cpu(Process *p) {
      std::unique_lock<std::mutex> lock(mutex);
      enqueue(p);
      schedule();
      while (!this->idle && p != front) cv.wait(lock);
      this->idle = false;
      front->execute();
    }

    void release_cpu() {
      std::unique_lock<std::mutex> lock(mutex);
      this->idle = true;
      cv.notify_all();
      dequeue();
      schedule();
    }

    void schedule() {
    }

    void enqueue(Process *p) {
    }

    void dequeue() {
    }

    void preempt() {
    }
};

#endif