#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include <thread>

/* Implementation of class "MessageQueue" */

/* */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
	std::unique_lock<std::mutex> ul(_mutex);
	while(_queue.empty()) _condition.wait(ul);

	// pull them from the queue
	T msg = std::move(_queue.front());
	_queue.pop_front();
	return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	std::lock_guard<std::mutex> guard(_mutex);
	_queue.clear();
	_queue.emplace_back(msg);
	_condition.notify_all();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (_msg.receive() == TrafficLightPhase::green) break;
	}
	return ;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class (trafficObject).
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.


	// init stop watch
	auto t0 = std::chrono::high_resolution_clock::now();

	int randomCycleDuration;

	while (true){
		auto t1 = std::chrono::high_resolution_clock ::now();
		auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count(); // time passed since t0=last update
		// cycle duration should be between 4-6 sec, 4000-6000 ms
		randomCycleDuration = int(double(rand())/RAND_MAX * 2000) + 4000;
		if (timeSinceLastUpdate >= randomCycleDuration){
			// toggle the current phase of the traffic light and send to the message queue
			if (_currentPhase==TrafficLightPhase::red) _currentPhase = TrafficLightPhase::green;
			else _currentPhase = TrafficLightPhase::red;
			_msg.send(std::move(_currentPhase));
			// reset stop watch
			t0 = std::chrono::high_resolution_clock::now();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1) );
	}

}

