#include <condition_variable>
#include <mutex>

enum signal_condition {
    on_thread_exit,
};

class semaphore {
  public:
    semaphore(int value = 0) : value(value) {}
    void wait();
    void signal();
    void signal(signal_condition);

  private:
    int value;
    std::mutex m;
    std::condition_variable_any cv;

    semaphore(const semaphore& orig) = delete;
    const semaphore& operator=(const semaphore& rhs) const = delete;
};