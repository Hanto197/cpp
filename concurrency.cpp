#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
using namespace std;

int counter = 0;

// atomic is a lock-free synchronization mechanism used for shared data access in multithreading making it faster than a mutex
// It ensures that operations on a variable happen atomically, meaning they can't be interrupted by another thread
// It is used instead of mutex to avoid race conditions, It is useful for simple data types like int, bool, pointer, etc
atomic<int> new_counter(0);

mutex mute, mtx, mtx1, mtx2;

void test(int num){
    cout << "Arguement passed in:" << num << endl;
}

void increment(){
    for(int i=0; i<1000000; i++){

        // solution to avoid race condition where two threads changes counter at the same which causes discrepancies
        // lock gaurd locks the critical section which can only be accessed by one thread at a time 
        // and unlocks the mtx when the thread goes out of scope
        lock_guard<mutex> lock(mtx); 
        // anything below the lock in scope is called critical seection
        counter++; 

        // unique lock allows manual unlocking and if not unlocked, it unlocks automatically when thread goes out of scope
        unique_lock<mutex> locky(mute); 
        locky.unlock();

        // deferred locking with unique lock
        unique_lock<mutex> lucky(mute, defer_lock); 
        lucky.lock();

        // atomic
        new_counter++;

    }
}

// the below two functions cause a dead lock because th1 locks mtx1 and th2 locks mtx2 
// but after that th1 waits to lock mtx2 and th2 waits to lock mtx1 which makes them wait forever causing deadlock

void thread1(){
    lock_guard<mutex> lock1(mtx1);
    this_thread::sleep_for(chrono::milliseconds(100)); // Simulate delay
    lock_guard<mutex> lock2(mtx1);
}

void thread2(){
    lock_guard<mutex> lock2(mtx2);
    this_thread::sleep_for(chrono::milliseconds(100));
    lock_guard<mutex> lock1(mtx1);
}

// solution for deadlock using scoped lock

void thread1(){
    scoped_lock lock(mtx1, mtx2);  // Locks both mutexes at once
}

void thread2(){
    scoped_lock lock(mtx1, mtx2); // no risk of dead lock
}

int main() {
    thread t(&test, 100); // starts executing thread parallely
    t.join();

    thread t1(increment);
    thread t2(increment); 

    t1.join(); // waits until t1 thread finishes execution
    t2.join(); 

    thread th1(thread1); 
    thread th2(thread2); 

    th1.join(); 
    th2.join(); 

    // No need to manually join, Thread automatically joins when `t` goes out of scope and it is available from C++20
    // jthread jt(increment);

	return 0;
}
