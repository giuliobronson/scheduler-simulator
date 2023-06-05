#include "scheduler.h"

#include <thread>

int main() {
   Scheduler* s = new Scheduler(10,15);

  //  Process p1 = Process(s, 40, 1);
  //  Process p2 = Process(s, 15, 2);


   Process p1 = Process(s, 8, 3);
   Process p2 = Process(s, 40, 1);
   Process p3 = Process(s, 10, 2);
   Process p4 = Process(s, 30, 1);
   

   std::thread thread1(p1);
   std::thread thread2(p2);
   std::thread thread3(p3);
   std::thread thread4(p4);

   thread1.join();
   thread2.join();

   return 0;
}