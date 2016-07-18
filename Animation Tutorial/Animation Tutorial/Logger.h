#pragma once

#include <mutex>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <future>

class Logger
{
public:
	template <class MsgType>
	static void Log(MsgType message)
	{
		std::async(std::launch::async, [&message] {
			std::lock_guard<std::mutex> lock(write_lock_);
			auto dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - appStart_).count() / 1000000.f;
			std::cout << "(" << std::fixed << std::setprecision(6) << dur << ") " << message << std::endl;
		});
	}

private:
	static std::mutex write_lock_;
	static std::chrono::high_resolution_clock::time_point appStart_;
};