#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <thread>
#include <chrono>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    // Code taken from Udacity lesson material
    std::unique_lock<std::mutex> uLock(_msgQueueMtx);
    _cond.wait(uLock, [this] {return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // Code taken from Udacity lesson material
    std::lock_guard<std::mutex> uLock(_msgQueueMtx);

    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {

}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        TrafficLightPhase rcvPhase = _msgQueue.receive();
        if (rcvPhase == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called.
    // To do this, use the thread queue in the base class.

    // TrafficLight is a child of class TrafficObject, so it inherits TrafficObject's std::vector<std::thread> threads.
    // I only need to start one thread, using cycleThroughPhases() and add it to that vector of threads
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    while(true) {
        //get a cycle duration
        int cycle_time = (std::rand() % 3) + 4;

        // wait for the cycle duration
        std::this_thread::sleep_for(std::chrono::seconds(cycle_time));
        
        // toggle the light
        switch(_currentPhase) {
            case TrafficLightPhase::red:
                _currentPhase = TrafficLightPhase::green;
                //std::cout << "toggling to green\n";
                break;
            case TrafficLightPhase::green:
                _currentPhase = TrafficLightPhase::red;
                //std::cout << "toggling to red\n";
                break;
            default:
                std::cout << "Traffic light phase is not a valid color!\n";
        }
        
        // send an update method to the message queue using move semantics
        _msgQueue.send(std::move(_currentPhase));

        // sleep for 1ms between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

