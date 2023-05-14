#include "tasksys.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <iostream>
#include <atomic>

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

/* run 구현, (IRunnaable)runable과 num_total_tasks를 받아서
*  num_total_tasks 만큼 runTask를 진행
*/
void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) { 
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

   
    this->num_threads = num_threads;


}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::runThreadParallelSpawn(IRunnable* runnable, std::atomic_int& current_task, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // for을 사용하면 순차적으로 되지만 while을 사용하면 병렬처리가 됨, for도 병렬처리를 지원하는데 왜 이런 문제가 발생했지?
    /*
    for(int i = current_task; i < num_total_tasks; i++){
        runnable->runTask(i, num_total_tasks);
    }*/
    

    // 총 작업 수만큼 반복됨 (0~128번 각 스레드마다)
    
    int i;
    while ((i = current_task++) < num_total_tasks) {
        runnable->runTask(i, num_total_tasks);
    }
    
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    /*
    std::vector<std::thread> threads;
    std::atomic_int current_task(0); // 공통적으로 접근되어서 사용되니 atomic으로
    
    for (int i = 0; i < this->num_threads; i++) {
        threads.push_back(std::thread(&TaskSystemParallelSpawn::runThreadParallelSpawn, this, runnable, std::ref(current_task), num_total_tasks));
        // thread.push_back(std::thread(function)); 다음과 같이 스레드를 추가, 여기서 필요한 인자도 넣는 형식으로 진행
    }
    for (int i = 0; i < this->num_threads; i++) {
        threads[i].join();
    }
    */ 
    

    // 이 코드의 경우 389.440 ms로 오히려 느려짐 = atomic variable을 사용하지 않아서 serial하게 실행되는 것이 원인 같음
    // atomic variable을 사용해도 속도가 변하지 않음 -> runThread 부분에서 serial하게 동작하는 원인이 있는 것 같음
    // thread 개수만큼 스레드를 만들고 진행

    
    std::thread my_thread[this->num_threads];
    std::atomic_int current_task(0);
    for (int i = 0; i < this->num_threads; i++) {
        my_thread[i] = std::thread(&TaskSystemParallelSpawn::runThreadParallelSpawn, this, runnable, std::ref(current_task), num_total_tasks);
    }

    for (int i = 0; i < this->num_threads; i++) {
        my_thread[i].join();
    }
    
    
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

/*
* 1. thread pool로 사용할 수 있는 work thread pool을 생성
* 2. thread pool에서 큐에 task을 추가
* 3. thread가 큐에서 task를 가져와서 처리하는데 이때 mutex lock을 걸음
* 4. task가 끝나면 새 task를 가져옴
* 5. 위 과정을 반복

*/


const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}
void TaskSystemParallelThreadPoolSpinning::runThreadParallelThreadPoolSpinning(int thread_number) {
    

    while (!thread_state->done_flag) {// 스레드가 끝나지 않았다면 = 진행중이라면
        thread_state->main_mutex->lock(); // 뮤텍스 락을 걸고
        int i = thread_state->current_task; // i를 현재 task로 바꿈
        int total_task = thread_state->num_total_tasks;// 총 task를 num task로 바꿈

        if (total_task == 0 || i >= total_task) {// task가 완료되었다면
            thread_state->main_mutex->unlock();// 락을 풀음
            // 완료되었는지 while로 continue해서 확인
            continue;
        }

        thread_state->current_task++; //  다 끝나지 않았다면 이어서 진행, 새 task를 가져오고
        thread_state->main_mutex->unlock();// lock을 풀고
        thread_state->runnable->runTask(i, total_task); // 작업을 할당
        thread_state->task_done_mutex->lock(); // 완료된 작업 수를 수정할 때 다른 스레드들이 접근하지 못하게
        thread_state->task_done++;// 완료된 작업 수 증가 시키고
        thread_state->task_done_mutex->unlock(); // 다시 lock을 풀음
     }
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    // 여기서 지정된 수의 스레드를 생성
    this->thread_state = new ThreadState();// 각 state를 설정해주는 객체를 만들고
    this->num_threads = num_threads;// 스레드 개수를 설정
    for (int i = 0; i < num_threads; i++) { 
        this->pool.push_back(std::thread(&TaskSystemParallelThreadPoolSpinning::runThreadParallelThreadPoolSpinning, this, i));
    }
}


// 위에서 pool에 전부 push한 경우 실행이 끝날 때까지 대기
TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    this->thread_state->done_flag = true;
    for (int i = 0; i < this->num_threads; i++) {
        this->pool[i].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // std::cout << "start\n";
    // state 초기화
    thread_state->main_mutex->lock();
    thread_state->current_task = 0;
    thread_state->runnable = runnable;
    thread_state->num_total_tasks = num_total_tasks;
    thread_state->done_flag = false;
    thread_state->task_done = 0;
    thread_state->main_mutex->unlock();
    bool done_work = false;
    // 모든 작업이 완료되었는지 확인
    while (!done_work) {
        thread_state->task_done_mutex->lock();
        done_work = thread_state->task_done >= thread_state->num_total_tasks;
        thread_state->task_done_mutex->unlock();
    }
}


TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
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

void TaskSystemParallelThreadPoolSleeping::runThreadParallelThreadPoolSleeping(int thread_number) {
    while (!thread_state->done_flag) {
        std::unique_lock<std::mutex> main_lock(*thread_state->main_mutex);
        // sleep thread worker when there is no work to execute
        while ((thread_state->num_total_tasks == 0 || thread_state->current_task >= thread_state->num_total_tasks) &&
            !thread_state->done_flag) {
            thread_state->main_cr->wait(main_lock);
        }
        int i = thread_state->current_task;
        int total_task = thread_state->num_total_tasks;
        // keep looping if current task is done
        if (total_task == 0 || i >= total_task) {
            main_lock.unlock();
            continue;
        }
        thread_state->current_task++;
        main_lock.unlock();
        thread_state->runnable->runTask(i, total_task);
        // increment task done counter
        std::unique_lock<std::mutex> task_done_lock(*thread_state->task_done_mutex);
        thread_state->task_done++;
        task_done_lock.unlock();
        // have one thread designated as the waker or wake when tasks are finished
        if (thread_number == 0 || thread_state->task_done >= thread_state->num_total_tasks) {
            thread_state->task_done_cr->notify_one();
        }
    }
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->thread_state = new ThreadState();
    this->num_threads = num_threads;
    for (int i = 0; i < num_threads; i++) {
        pool.push_back(std::thread(&TaskSystemParallelThreadPoolSleeping::runThreadParallelThreadPoolSleeping, this, i));
    }


}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    thread_state->done_flag = true;
    thread_state->main_cr->notify_all();
    for (int i = 0; i < this->num_threads; i++) {
        pool[i].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    thread_state->main_mutex->lock();
    thread_state->current_task = 0;
    thread_state->runnable = runnable;
    thread_state->num_total_tasks = num_total_tasks;
    thread_state->done_flag = false;
    thread_state->task_done = 0;
    thread_state->main_mutex->unlock();
    thread_state->main_cr->notify_all();
    bool done_work = false;
    // loop to check if all works are done
    while (!done_work) {
        std::unique_lock<std::mutex> task_done_lock(*thread_state->task_done_mutex);
        if (thread_state->task_done < thread_state->num_total_tasks) {
            thread_state->main_cr->notify_all();
        }
        // sleep main thread when all tasks are not done
        while (thread_state->task_done < thread_state->num_total_tasks) {
            thread_state->task_done_cr->wait(task_done_lock);
        }
        done_work = thread_state->task_done >= thread_state->num_total_tasks;
        task_done_lock.unlock();
    }
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
