#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include <thread>

template<typename T>
T MessageQueue<T>::receive() {
	std::unique_lock <std::mutex> ul(_mutex);
	while (_queue.empty()) _condition.wait(ul);

	// pull them from the queue
	T msg = std::move(_queue.front());
	_queue.pop_front();
	return msg;
}

template<typename T>
void MessageQueue<T>::send(T &&msg) {
	std::lock_guard <std::mutex> guard(_mutex);
	_queue.clear();
	_queue.emplace_back(msg);
	_condition.notify_one();
}


TrafficLight::TrafficLight() {
	_currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (_msg.receive() == TrafficLightPhase::green) break;
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase() {
	return _currentPhase;
}

// virtual function which is executed in a thread
void TrafficLight::simulate() {
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


void TrafficLight::cycleThroughPhases() {
	// init stop watch
	auto t0 = std::chrono::high_resolution_clock::now();
	int randomCycleDuration;
	// keep running during the whole process since the start of the simulation
	while (true) {
		auto t1 = std::chrono::high_resolution_clock::now();
		auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
				t1 - t0).count(); // time passed since t0=last update
		// cycle duration should be between 4-6 sec, 4000-6000 ms
		randomCycleDuration = int(double(rand()) / RAND_MAX * 2000) + 4000;

		// toggle the traffic light if it's been long enough since the last toggle
		if (timeSinceLastUpdate >= randomCycleDuration) {
			// toggle the current phase of the traffic light and send to the message queue
			if (_currentPhase == TrafficLightPhase::red) {
				_currentPhase = TrafficLightPhase::green;
				std::cout << "Light " << _id << " switched from red to green" << std::endl;
			} else {
				_currentPhase = TrafficLightPhase::red;
				std::cout << "Light " << _id << " switched from green to red" << std::endl;
			}
			_msg.send(std::move(_currentPhase));
			// reset stop watch
			t0 = std::chrono::high_resolution_clock::now();
		}

		// sleep at every iteration to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

