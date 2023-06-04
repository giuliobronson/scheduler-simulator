#include "scheduler.h"

#include <thread>

std::vector<std::thread> threads;
std::vector<Process> processes;

int main() {
   Scheduler* s = new Scheduler();

   int n, burst, io_op;
   std::cin >> n;
   for(int i = 0; i < n; i++) {
      std::cin >> burst >> io_op;
      processes.push_back(Process(s, burst, io_op));
   } 

   for (auto& process : processes) {
      threads.push_back(std::thread(process));
   }
   for (auto& thread : threads) {
      thread.join();
   }

   // Process p1 = Process(s, 2, 1);
   // Process p2 = Process(s, 5, 2);

   // std::thread thread1(p1);
   // std::thread thread2(p2);

   // thread1.join();
   // thread2.join();

   return 0;
}