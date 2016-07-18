#include "Logger.h"

std::mutex Logger::write_lock_;
std::chrono::high_resolution_clock::time_point Logger::appStart_ = std::chrono::high_resolution_clock::now();