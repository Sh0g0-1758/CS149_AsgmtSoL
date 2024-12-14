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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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

void TaskSystemParallelThreadPoolSleeping::async_check() {
    std::cout << "Checking for async" << std::endl;
    auto it = waiting_tasks.begin();
    while (it != waiting_tasks.end()) {
        bool ready = true;
        for (auto dep_task : std::get<2>(it->second)) {
            if (std::find(completed_tasks.begin(), completed_tasks.end(), dep_task) == completed_tasks.end()) {
                ready = false;
                break;
            }
        }
        if (ready) {
            running_tasks[it->first] = std::get<1>(it->second);
            task_mtx.lock();
            for (int i = 0; i < std::get<1>(it->second); i++) {
                task_queue.push({it->first, {std::get<0>(it->second), {i, std::get<1>(it->second)}}});
            }
            task_queue_size += std::get<1>(it->second);
            task_mtx.unlock();
            task_cv.notify_all();
            it = waiting_tasks.erase(it);
        } else {
            ++it;
        }
    }
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
        auto [task_id, mix] = task_queue.front();
        task_queue.pop();
        lock.unlock();
        task_cv.notify_all();
        mix.first->runTask(mix.second.first, mix.second.second);
        task_queue_size--;
        if(all_tasks.empty()) return;
        async_mutex.lock();
        running_tasks[task_id]--;
        if(running_tasks[task_id] == 0) {
            completed_tasks.push_back(task_id);
            running_tasks.erase(task_id);
            if(waiting_tasks.size() != 0) async_check();
        }
        async_mutex.unlock();
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
        task_queue.push({0, {runnable,{i, num_total_tasks}}});
    }
    task_queue_size += num_total_tasks;
    task_mtx.unlock();
    task_cv.notify_all();
    while(task_queue_size != 0) {}
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {

    TaskID taskid = currID;
    currID++;
    all_tasks.push_back(taskid);
    async_mutex.lock();
    for(auto dep_task: deps) {
        if(std::find(completed_tasks.begin(), completed_tasks.end(), dep_task) != completed_tasks.end()) {
            continue;
        } else {
            waiting_tasks[taskid] = {runnable, num_total_tasks, deps};
            async_mutex.unlock();
            return taskid;
        }
    }
    running_tasks[taskid] = num_total_tasks;
    async_mutex.unlock();
    task_mtx.lock();
    for (int i = 0; i < num_total_tasks; i++) {
        task_queue.push({taskid, {runnable,{i, num_total_tasks}}});
    }
    task_queue_size += num_total_tasks;
    task_mtx.unlock();
    task_cv.notify_all();
    return taskid;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    bool keep_running = true;
    while(keep_running) {
        bool all_completed = true;
        async_mutex.lock();
        for(auto task: all_tasks) {
            if(std::find(completed_tasks.begin(), completed_tasks.end(), task) == completed_tasks.end()) {
                all_completed = false;
                break;
            }
        }
        async_mutex.unlock();
        if(all_completed) keep_running = false;
    }
    return;
}
