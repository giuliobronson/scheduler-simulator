#include <iostream>
#include <queue>
#include <time.h>

#ifndef QUEUE_H
#define QUEUE_H


class Process {
  private: 
    int pid;
    int burst; // Tempo de burst de CPU
    int io_op; // Número de operações de I/O

  protected:
    static int s_id;

  public:
    Process(int burst, int io_op): burst(burst), io_op(io_op) {
      this->pid = ++s_id;
    }

    int getPID() {
      return this->pid;
    }

    int execute() {
      time_t start, end; start = time(0);
      while(true) {
        if(time(0) - start == 1) {
          std::cout << this->pid << ": Executing..." << std::endl;
          start += 1;
        }
      }
    }

    void stop() {

    }
};

class Queue {
  private:
    std::queue<Process> q;

  public:
    void push(Process p) {
      q.push(p);
    }

    void pop() {
      q.pop();
    }
};

int Process::s_id = 0;

#endif