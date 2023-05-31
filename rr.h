#include "queue.h"

#ifndef RR_H
#define RR_H

class RoundRobin: public Queue {
  private:
    int quantum;

  public:
    RoundRobin(int quantum): quantum(quantum) {}
};

#endif