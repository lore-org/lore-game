#include <execution>
#include <vector>

void func() {
    std::vector<int> v = {};
    std::sort(std::execution::par, v.begin(), v.end());
    std::sort(std::execution::par_unseq, v.begin(), v.end());
}