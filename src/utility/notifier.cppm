module;
#include <functional>
#include <mutex>
#include <queue>
#include <utility>
export module utility:notifier;

export namespace utility
{
    class c_notifier
    {
    public:
        static void subscribe(std::function<void()> callback);
        static void notify();
        static void reset() noexcept;

    private:
        inline static std::mutex s_mutex;
        inline static std::queue<std::function<void()>> s_subscribers;
        inline static bool s_is_notified;
    };
} // namespace utility

// Implementation
namespace utility
{
    void c_notifier::subscribe(std::function<void()> callback)
    {
        std::lock_guard lock(s_mutex);
        if (s_is_notified)
        {
            callback();
            return;
        }
        s_subscribers.push(std::move(callback));
    }

    void c_notifier::notify()
    {
        std::lock_guard lock(s_mutex);
        s_is_notified = true;
        while (not s_subscribers.empty())
        {
            s_subscribers.front()();
            s_subscribers.pop();
        }
    }
    void c_notifier::reset() noexcept
    {
        while (not s_subscribers.empty())
        {
            s_subscribers.pop();
        }
        s_is_notified = false;
    }
} // namespace utility
