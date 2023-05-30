#include<queue>

#ifndef PROCESS_H
#define PROCESS_H

class Process {
  private: 
    int pid;

  protected:
    static int s_id;

  public:
    Process() {
      this->pid = ++s_id;
    }

    int getPID() {
      return this->pid;
    }
};

template<typename T>
class Queue {
  private:
    queue<T> q;
    
};

int Process::s_id = 0;

#endif