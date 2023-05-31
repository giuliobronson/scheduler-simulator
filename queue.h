#include <iostream>
#include <queue>

#ifndef QUEUE_H
#define QUEUE_H


class Process {
  private: 
    int pid;
    int exec_time; // Quanto do processo foi executado
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
      std::cout << this->pid << ": Executing..." << std::endl;
    }

    void stop() {

    }
};

class Queue {
  private:
    std::queue<Process*> q;

  public:
    void push(Process *p) {
      q.push(p);
    }

    void pop() {
      q.pop();
    }
};

class RoundRobin: public Queue {
  private:
    int quantum;

  public:
    RoundRobin(int quantum): quantum(quantum) {}
};

int Process::s_id = 0;

#endif