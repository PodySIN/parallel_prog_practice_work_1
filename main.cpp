#include <iostream>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string.h>
#include <vector>

namespace hvostov {
  struct data_t {
    double r;
    size_t tests;
    size_t seed;
  };

  bool isInside(double x, double y, double r)
  {
    return (x * x + y * y) <= (r * r);
  }

  size_t calc(double r, size_t tests, size_t seed)
  {
    size_t count = 0;
    std::mt19937 gen(seed);
    std::uniform_real_distribution< double > dist(-r, r);
    for (size_t i = 0; i < tests; i++) {
      count += isInside(dist(gen), dist(gen), r);
    }
    return count;
  }

  void* calc_wrapper(void* data)
  {
    data_t* d = static_cast< data_t* >(data);
    size_t answer = calc(d->r, d->tests, d->seed);
    return reinterpret_cast< void* >(answer);
  }

  double area(double r, size_t threads, size_t tests)
  {
    size_t test_per_thread = tests / threads;
    size_t remainders = tests % threads;
    std::vector< pthread_t > v(threads);
    std::vector< data_t > d(threads);
    for (size_t i = 0; i < threads; i++) {
      size_t thread_tests = test_per_thread + (i < remainders ? 1 : 0);
      d[i] = {r, thread_tests, i};
      int err = pthread_create(&v[i], nullptr, calc_wrapper, &d[i]);
      if (err) {
        throw std::runtime_error(strerror(err));
      }
    }
    size_t count = 0;
    for (size_t i = 0; i < threads; i++) {
      void* res = nullptr;
      int err = pthread_join(v[i], &res);
      if (err) {
        throw std::runtime_error(strerror(err));
      }
      count += reinterpret_cast< size_t >(res);
    }
    double answer = (r * r * 4) * ((count * 1.0) / (tests * 1.0));
    return answer;
  }

}

int main(int argc, char** argv)
{
  if (argc != 4) {
    std::cerr << "Expect 3 args\n";
    return 1;
  }
  double r;
  size_t threads, tests;
  try {
    r = std::stod(argv[1]);
    threads = std::stoull(argv[2]);
    tests = std::stoull(argv[3]);
  } catch (...) {
    std::cerr << "Wrong input\n";
    return 1;
  }

  if (r <= 0 || threads == 0 || tests == 0) {
    std::cerr << "All arguments must be >= 0 (r > 0)\n";
    return 1;
  }

  double area = 0.0;
  try {
    area = hvostov::area(r, threads, tests);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  std::cout << "Area: " << area << '\n';
}
