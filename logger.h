#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <chrono>
#include <iostream>
#include <string>

typedef class Logger {
  public:
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::system_clock::now(); // √
    void log_info(const std::string &msgString) {
        std::cout << elapsed_time() << " info: " << msgString << std::flush;
    }

    void log_error(const std::string &msgString) {
        std::cerr << "\x1b[31m" << elapsed_time() << " error: " << msgString
                  << "\x1b[0m" << std::flush;
    }

    void log_prompt(const std::string &msgString) {
        std::cout << "\x1b[32m" << elapsed_time() << " prompt: " << msgString
                  << "\x1b[0m" << std::flush;
    }

  private:
    std::string elapsed_time() const noexcept {
        std::chrono::high_resolution_clock::duration elapsed_seconds =
            std::chrono::system_clock::now() - start;
        // 转换为秒
        return "[" +
               std::to_string(
                   (float)(std::chrono::duration_cast<std::chrono::nanoseconds>(
                               elapsed_seconds)
                               .count()) /
                   1000000000.0f) +
               "]";
    }
} Logger;
#endif