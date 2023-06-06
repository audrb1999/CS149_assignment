#include <stdio.h>
#include <thread>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    int width;
    int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStart(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.



 for (int i = args->threadId; i < args->height; i = i + args->numThreads) {
     // x0 = -2, x1 = 1, y0 = -1, y1 = 1
     // threadid는 0과 1
     // 높이는 1200
     // 

      mandelbrotSerial(
        args->x0, args->y0, args->x1, args->y1,
        args->width, args->height,
        i, 1,
        args->maxIterations,
        args->output
      );

      //startrow = i
      //numrows = 1
    
    }
  
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads, // 스레드 개수
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;// 맥스 스레드

    if (numThreads > MAX_THREADS)// 맥스 스레드보다 현재 스레드가 더 많으면
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);// 에러 발생
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];// 32개의 스레드를 만듬
    WorkerArgs args[MAX_THREADS];// 실행에 필요한 구조체 32개 만듬

    for (int i=0; i<numThreads; i++) {// 스레드 개수 만큼 반복
      
        // TODO FOR CS149 STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread

        // 스레드 개수 만큼 해당 구조체에 값들을 저장
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i; // 스레드 id도 저장(스레드 개수로 생각하면 될 듯)
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);// std 스레드로 반복 실행
    }
    
    workerThreadStart(&args[0]);

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();// 스레드 종료까지 기달림
    }
}

