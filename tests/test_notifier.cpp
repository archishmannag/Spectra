#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

import utility;

TEST_CASE("Notifier: Basic functionality", "[utility][notifier][unit]")
{
    SECTION("Subscribe and notify single callback")
    {
        utility::c_notifier::reset();

        bool called = false;
        utility::c_notifier::subscribe([&called]()
                                       { called = true; });

        REQUIRE_FALSE(called);
        utility::c_notifier::notify();
        REQUIRE(called);
    }

    SECTION("Multiple subscriptions are all notified")
    {
        utility::c_notifier::reset();

        int counter = 0;
        utility::c_notifier::subscribe([&counter]()
                                       { counter++; });
        utility::c_notifier::subscribe([&counter]()
                                       { counter++; });
        utility::c_notifier::subscribe([&counter]()
                                       { counter++; });

        REQUIRE(counter == 0);
        utility::c_notifier::notify();
        REQUIRE(counter == 3);
    }

    SECTION("Notify without subscriptions does nothing")
    {
        utility::c_notifier::reset();
        REQUIRE_NOTHROW(utility::c_notifier::notify());
    }

    SECTION("Subscribe after notify executes immediately")
    {
        utility::c_notifier::reset();
        utility::c_notifier::notify();

        bool called = false;
        utility::c_notifier::subscribe([&called]()
                                       { called = true; });
        REQUIRE(called);
    }

    SECTION("Reset clears notification state")
    {
        utility::c_notifier::reset();
        utility::c_notifier::notify();

        bool called1 = false;
        utility::c_notifier::subscribe([&called1]()
                                       { called1 = true; });
        REQUIRE(called1);

        utility::c_notifier::reset();

        bool called2 = false;
        utility::c_notifier::subscribe([&called2]()
                                       { called2 = true; });
        REQUIRE_FALSE(called2); // Should not execute immediately after reset
    }

    SECTION("Reset clears pending subscriptions")
    {
        utility::c_notifier::reset();

        bool called = false;
        utility::c_notifier::subscribe([&called]()
                                       { called = true; });

        utility::c_notifier::reset();
        utility::c_notifier::notify();

        REQUIRE_FALSE(called); // Callback was cleared by reset
    }
}

TEST_CASE("Notifier: Order of execution", "[utility][notifier][unit]")
{
    SECTION("Callbacks execute in subscription order")
    {
        utility::c_notifier::reset();

        std::vector<int> execution_order;

        utility::c_notifier::subscribe([&execution_order]()
                                       { execution_order.push_back(1); });
        utility::c_notifier::subscribe([&execution_order]()
                                       { execution_order.push_back(2); });
        utility::c_notifier::subscribe([&execution_order]()
                                       { execution_order.push_back(3); });
        utility::c_notifier::subscribe([&execution_order]()
                                       { execution_order.push_back(4); });

        utility::c_notifier::notify();

        REQUIRE(execution_order.size() == 4);
        REQUIRE(execution_order[0] == 1);
        REQUIRE(execution_order[1] == 2);
        REQUIRE(execution_order[2] == 3);
        REQUIRE(execution_order[3] == 4);
    }
}

TEST_CASE("Notifier: Multiple notify cycles", "[utility][notifier][unit]")
{
    SECTION("Can notify multiple times")
    {
        utility::c_notifier::reset();

        int count1 = 0;
        utility::c_notifier::subscribe([&count1]()
                                       { count1++; });
        utility::c_notifier::notify();
        REQUIRE(count1 == 1);

        int count2 = 0;
        utility::c_notifier::subscribe([&count2]()
                                       { count2++; });
        REQUIRE(count2 == 1); // Executes immediately since already notified

        utility::c_notifier::reset();

        int count3 = 0;
        utility::c_notifier::subscribe([&count3]()
                                       { count3++; });
        REQUIRE(count3 == 0); // Does not execute immediately after reset
        utility::c_notifier::notify();
        REQUIRE(count3 == 1);
    }
}

TEST_CASE("Notifier: Thread safety", "[utility][notifier][unit][threading]")
{
    SECTION("Concurrent subscriptions")
    {
        utility::c_notifier::reset();

        std::atomic<int> counter{ 0 };
        constexpr int num_threads = 10;
        constexpr int subscriptions_per_thread = 10;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t)
        {
            threads.emplace_back([&counter, subscriptions_per_thread]()
                                 {
                for (int i = 0; i < subscriptions_per_thread; ++i)
                {
                    utility::c_notifier::subscribe([&counter]() { counter++; });
                } });
        }

        for (auto &thread : threads)
        {
            thread.join();
        }

        utility::c_notifier::notify();
        REQUIRE(counter == num_threads * subscriptions_per_thread);
    }

    SECTION("Concurrent notify and subscribe")
    {
        utility::c_notifier::reset();

        std::atomic<int> subscribe_counter{ 0 };
        std::atomic<int> notify_counter{ 0 };
        std::atomic<bool> should_stop{ false };

        // Thread that subscribes
        std::thread subscribe_thread([&]()
                                     {
            for (int i = 0; i < 100; ++i)
            {
                utility::c_notifier::subscribe([&subscribe_counter]() { subscribe_counter++; });
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });

        // Thread that notifies
        std::thread notify_thread([&]()
                                  {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            utility::c_notifier::notify();
            notify_counter++; });

        subscribe_thread.join();
        notify_thread.join();

        // All callbacks should have been executed
        REQUIRE(subscribe_counter > 0);
        REQUIRE(notify_counter == 1);
    }

    SECTION("Concurrent reset operations")
    {
        utility::c_notifier::reset();

        std::atomic<int> counter{ 0 };

        std::thread subscribe_thread([&counter]()
                                     {
            for (int i = 0; i < 50; ++i)
            {
                utility::c_notifier::subscribe([&counter]() { counter++; });
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });

        std::thread reset_thread([]()
                                 {
            for (int i = 0; i < 5; ++i)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                utility::c_notifier::reset();
            } });

        subscribe_thread.join();
        reset_thread.join();

        // This should not crash or cause undefined behavior
        REQUIRE_NOTHROW(utility::c_notifier::notify());
    }
}

TEST_CASE("Notifier: Edge cases", "[utility][notifier][unit]")
{
    SECTION("Empty lambda subscription")
    {
        utility::c_notifier::reset();
        utility::c_notifier::subscribe([]() {});
        REQUIRE_NOTHROW(utility::c_notifier::notify());
    }

    SECTION("Lambda with captured values")
    {
        utility::c_notifier::reset();

        std::vector<int> values;
        int x = 42;
        std::string str = "test";

        utility::c_notifier::subscribe([&values, x, str]()
                                       {
            values.push_back(x);
            values.push_back(static_cast<int>(str.length())); });

        utility::c_notifier::notify();

        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == 42);
        REQUIRE(values[1] == 4);
    }

    SECTION("Complex lambda operations")
    {
        utility::c_notifier::reset();

        std::vector<int> results;

        utility::c_notifier::subscribe([&results]()
                                       {
            for (int i = 0; i < 5; ++i)
            {
                results.push_back(i * i);
            } });

        utility::c_notifier::notify();

        REQUIRE(results.size() == 5);
        REQUIRE(results[0] == 0);
        REQUIRE(results[1] == 1);
        REQUIRE(results[2] == 4);
        REQUIRE(results[3] == 9);
        REQUIRE(results[4] == 16);
    }

    SECTION("Multiple resets in sequence")
    {
        utility::c_notifier::reset();
        utility::c_notifier::reset();
        utility::c_notifier::reset();

        bool called = false;
        utility::c_notifier::subscribe([&called]()
                                       { called = true; });

        REQUIRE_FALSE(called);
        utility::c_notifier::notify();
        REQUIRE(called);
    }

    SECTION("Multiple notifies without subscriptions")
    {
        utility::c_notifier::reset();
        REQUIRE_NOTHROW(utility::c_notifier::notify());
        REQUIRE_NOTHROW(utility::c_notifier::notify());
        REQUIRE_NOTHROW(utility::c_notifier::notify());
    }
}

TEST_CASE("Notifier: State transitions", "[utility][notifier][unit]")
{
    SECTION("State: Not notified -> Notified")
    {
        utility::c_notifier::reset();

        bool before_notify = false;
        utility::c_notifier::subscribe([&before_notify]()
                                       { before_notify = true; });
        REQUIRE_FALSE(before_notify);

        utility::c_notifier::notify();
        REQUIRE(before_notify);

        bool after_notify = false;
        utility::c_notifier::subscribe([&after_notify]()
                                       { after_notify = true; });
        REQUIRE(after_notify); // Executes immediately
    }

    SECTION("State: Notified -> Reset -> Not notified")
    {
        utility::c_notifier::reset();
        utility::c_notifier::notify();

        bool immediate = false;
        utility::c_notifier::subscribe([&immediate]()
                                       { immediate = true; });
        REQUIRE(immediate);

        utility::c_notifier::reset();

        bool deferred = false;
        utility::c_notifier::subscribe([&deferred]()
                                       { deferred = true; });
        REQUIRE_FALSE(deferred);
    }
}
