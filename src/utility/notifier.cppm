module;
#include <functional>
#include <utility>
#include <vector>
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
        inline static std::vector<std::function<void()>> s_subscribers;
        inline static bool s_is_notified;
    };
} // namespace utility

// Implementation
namespace utility
{
    void c_notifier::subscribe(std::function<void()> callback)
    {
        if (s_is_notified)
        {
            callback();
            return;
        }
        s_subscribers.push_back(std::move(callback));
    }

    void c_notifier::notify()
    {
        s_is_notified = true;
        for (const auto &subscriber : s_subscribers)
        {
            subscriber();
        }
        s_subscribers.clear();
    }
    void c_notifier::reset() noexcept
    {
        s_subscribers.clear();
        s_is_notified = false;
    }
} // namespace utility
