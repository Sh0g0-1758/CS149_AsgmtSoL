#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void do_something(int i, int num_total_tasks, IRunnable* runnable) {
    runnable->runTask(i, num_total_tasks);
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::thread threads[num_total_tasks];
    for (int i = 0; i < num_total_tasks; i++) {
        threads[i] = std::thread(do_something, i, num_total_tasks, runnable);
    }
    for (int i = 0; i < num_total_tasks; i++) {
        threads[i].join();
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

void TaskSystemParallelThreadPoolSpinning::spinning_thread_function() {
    while (true) {        
        task_mtx.lock();
        if(task_queue.empty()) {
            task_mtx.unlock();
            if(stop) break;
            continue;
        }
        auto [runnable, mix] = task_queue.front();
        task_queue.pop();
        task_mtx.unlock();
        runnable->runTask(mix.first, mix.second);
        task_queue_size--;
    }
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    for (int i = 0; i < num_threads; i++) {
        ThreadPool.emplace_back(std::thread([this]() { this -> spinning_thread_function(); }));
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    stop = true;
    for(size_t i = 0; i < ThreadPool.size(); i++) {
        ThreadPool[i].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    task_mtx.lock();
    for (int i = 0; i < num_total_tasks; i++) {
        task_queue.push({runnable,{i, num_total_tasks}});
    }
    task_queue_size += num_total_tasks;
    task_mtx.unlock();
    while(task_queue_size != 0) {}
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

void TaskSystemParallelThreadPoolSleeping::sleeping_thread_function() {
    while (true) {
        std::unique_lock<std::mutex> lock(task_mtx);
        if(task_queue.empty()) task_cv.wait(lock);
        if(task_queue.empty()) {
            lock.unlock();
            task_cv.notify_all();
            if(stop) break;
            continue;
        }
        auto [runnable, mix] = task_queue.front();
        task_queue.pop();
        lock.unlock();
        task_cv.notify_all();
        runnable->runTask(mix.first, mix.second);
        task_queue_size--;
    }
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    for (int i = 0; i < num_threads; i++) {
        ThreadPool.emplace_back(std::thread([this]() { this -> sleeping_thread_function(); }));
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    task_cv.notify_all();
    stop = true;
    for(size_t i = 0; i < ThreadPool.size(); i++) {
        ThreadPool[i].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    task_mtx.lock();
    for (int i = 0; i < num_total_tasks; i++) {
        task_queue.push({runnable,{i, num_total_tasks}});
    }
    task_queue_size += num_total_tasks;
    task_mtx.unlock();
    task_cv.notify_all();
    while(task_queue_size != 0) {}
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
