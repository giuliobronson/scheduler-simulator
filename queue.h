#include <iostream>
#include <queue>

#ifndef QUEUE_H
#define QUEUE_H

enum class State {Q0, Q1, Q2};


class Process {
  private: 
    int pid;
    State state;
    int burst; // Tempo de burst de CPU
    int io_op; // Número de operações de I/O

  protected:
    static int s_id;

  public:
    Process(int burst, int io_op): burst(burst), io_op(io_op) {
      this->pid = ++s_id;
      this->state = State::Q0;
    }

    void execute() {
      std::cout << "Process #" << this->pid << std::endl;
    }
};

class Queue {
  private:
    std::queue<Process*> q; 

  public:
    Process *front() {
      return this->q.front();
    }

    void push(Process *p) {
      this->q.push(p);
    }

    void pop() {
      this->q.pop();
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