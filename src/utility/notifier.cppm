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
        bool notified = false;
        {
            std::scoped_lock lock(s_mutex);
            notified = s_is_notified;
        }
        if (notified)
        {
            callback();
        }
        else
        {
            std::scoped_lock lock(s_mutex);
            s_subscribers.push(std::move(callback));
        }
    }

    void c_notifier::notify()
    {
        std::queue<std::function<void()>> current;
        {
            std::scoped_lock lock(s_mutex);
            std::swap(current, s_subscribers);
            s_is_notified = true;
        }
        while (not current.empty())
        {
            current.front()();
            current.pop();
        }
    }
    void c_notifier::reset() noexcept
    {
        std::lock_guard lock(s_mutex);
        while (not s_subscribers.empty())
        {
            s_subscribers.pop();
        }
        s_is_notified = false;
    }
} // namespace utility
