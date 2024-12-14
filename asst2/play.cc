#include<iostream>
#include<mutex>
#include<thread>


std::mutex mtx;

void thread_func(int a) {
    mtx.lock();
    if(a == 1) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    std::cout << "Thread " << a << " is running" << std::endl;
    mtx.unlock();
}

int main() {
    std::thread t1(thread_func, 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread t2(thread_func, 2);
    t1.join();
    t2.join();
}