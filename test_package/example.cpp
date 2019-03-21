#include <iostream>
#include <boost/scope_guard.hpp>

int main() {
    boost::scope_guard_success say = [] { std::cout << "Dunnit" << std::endl; };
}
