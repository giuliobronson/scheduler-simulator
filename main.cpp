#include "scheduler.h"

#include <thread>

int main() {
   Scheduler* s = new Scheduler(10,15);

   Process p1 = Process(s, 2, 1);
   Process p2 = Process(s, 5, 2);

   std::thread thread1(p1);
   std::thread thread2(p2);

   thread1.join();
   thread2.join();

   return 0;
}