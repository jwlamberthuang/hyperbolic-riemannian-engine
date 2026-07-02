#include <iostream>
#include <stdexcept>

void run_trajectory_store_tests();

int main() {
    try {
        std::cout << "[Test Runner] Running validation suites..." << std::endl;
        run_trajectory_store_tests();
        std::cout << "[Test Runner] All validation suites passed." << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[Test Runner] Failure: " << error.what() << std::endl;
        return 1;
    }
}
