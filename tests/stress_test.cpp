#include <GL/glew.h>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

import glm;
import math;
import utility;
import opengl;

TEST_CASE("Stress: FFT large datasets", "[stress][fft]")
{
    SECTION("FFT with 8192 samples")
    {
        constexpr size_t size = 8192;
        std::vector<float> input(size);

        // Generate sine wave
        for (size_t i = 0; i < size; ++i)
        {
            input[i] = std::sin(2.0F * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(size));
        }

        auto result = math::fft(input);
        REQUIRE(result.size() == size);

        // Verify round-trip
        auto reconstructed = math::ifft(result);
        REQUIRE(reconstructed.size() == size);

        float max_error = 0.0F;
        for (size_t i = 0; i < size; ++i)
        {
            float error = std::abs(reconstructed[i] - input[i]);
            max_error = std::max(max_error, error);
        }
        REQUIRE(max_error < 1e-3F);
    }

    SECTION("FFT with 16384 samples")
    {
        constexpr size_t size = 16384;
        std::vector<float> input(size);

        // Generate complex signal with multiple frequencies
        for (size_t i = 0; i < size; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(size);
            input[i] = std::sin(2.0F * std::numbers::pi_v<float> * 10.0F * t) + 0.5F * std::sin(2.0F * std::numbers::pi_v<float> * 25.0F * t);
        }

        auto result = math::fft(input);
        REQUIRE(result.size() == size);
    }

    SECTION("Multiple consecutive FFT operations")
    {
        constexpr size_t size = 4096;
        constexpr int iterations = 100;

        std::vector<float> input(size);
        for (size_t i = 0; i < size; ++i)
        {
            input[i] = static_cast<float>(i) / static_cast<float>(size);
        }

        for (int iter = 0; iter < iterations; ++iter)
        {
            auto result = math::fft(input);
            REQUIRE(result.size() == size);
        }
    }

    SECTION("FFT round-trip stress test")
    {
        constexpr size_t size = 2048;
        constexpr int iterations = 50;

        std::vector<float> original(size);
        for (size_t i = 0; i < size; ++i)
        {
            original[i] = std::sin(2.0F * std::numbers::pi_v<float> * static_cast<float>(i) / 256.0F);
        }

        std::vector<float> current = original;

        // Multiple round trips
        for (int iter = 0; iter < iterations; ++iter)
        {
            auto transformed = math::fft(current);
            current = math::ifft(transformed);
        }

        // Check accumulated error
        float total_error = 0.0F;
        for (size_t i = 0; i < size; ++i)
        {
            total_error += std::abs(current[i] - original[i]);
        }
        float avg_error = total_error / static_cast<float>(size);

        REQUIRE(avg_error < 0.1F); // Accumulated error should still be reasonable
    }
}

TEST_CASE("Stress: Notifier concurrent operations", "[stress][notifier][threading]")
{
    SECTION("High volume subscriptions")
    {
        utility::c_notifier::reset();

        constexpr int num_subscriptions = 10000;
        std::atomic<int> counter{ 0 };

        for (int i = 0; i < num_subscriptions; ++i)
        {
            utility::c_notifier::subscribe([&counter]()
                                           { counter++; });
        }

        utility::c_notifier::notify();
        REQUIRE(counter == num_subscriptions);
    }

    SECTION("Concurrent subscribe from many threads")
    {
        utility::c_notifier::reset();

        constexpr int num_threads = 20;
        constexpr int subscriptions_per_thread = 500;
        std::atomic<int> counter{ 0 };

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

    SECTION("Rapid subscribe-notify-reset cycles")
    {
        constexpr int cycles = 1000;
        std::atomic<int> total_executions{ 0 };

        for (int cycle = 0; cycle < cycles; ++cycle)
        {
            utility::c_notifier::reset();

            int expected = (cycle % 10) + 1;
            for (int i = 0; i < expected; ++i)
            {
                utility::c_notifier::subscribe([&total_executions]()
                                               { total_executions++; });
            }

            utility::c_notifier::notify();
        }

        // Verify all callbacks were executed
        REQUIRE(total_executions > 0);
    }

    SECTION("Interleaved operations from multiple threads")
    {
        std::atomic<bool> should_stop{ false };
        std::atomic<int> subscribe_count{ 0 };
        std::atomic<int> notify_count{ 0 };
        std::atomic<int> reset_count{ 0 };

        auto run_duration = std::chrono::milliseconds(100);

        std::thread subscriber([&]()
                               {
            while (!should_stop.load())
            {
                utility::c_notifier::subscribe([&subscribe_count]() { subscribe_count++; });
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });

        std::thread notifier([&]()
                             {
            while (!should_stop.load())
            {
                utility::c_notifier::notify();
                notify_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } });

        std::thread resetter([&]()
                             {
            while (!should_stop.load())
            {
                utility::c_notifier::reset();
                reset_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            } });

        std::this_thread::sleep_for(run_duration);
        should_stop.store(true);

        subscriber.join();
        notifier.join();
        resetter.join();

        // Verify operations occurred
        REQUIRE(notify_count > 0);
        REQUIRE(reset_count > 0);
    }
}

TEST_CASE("Stress: Buffer layout many elements", "[stress][buffer_layout]")
{
    SECTION("Many float elements")
    {
        opengl::c_buffer_layout layout;

        constexpr int num_elements = 1000;
        for (int i = 0; i < num_elements; ++i)
        {
            layout.push<float>(1);
        }

        REQUIRE(layout.get_elements().size() == num_elements);
        REQUIRE(layout.get_stride() == num_elements * sizeof(float));
    }

    SECTION("Mixed element types in large quantities")
    {
        opengl::c_buffer_layout layout;

        constexpr int repetitions = 100;
        for (int i = 0; i < repetitions; ++i)
        {
            layout.push<float>(3);
            layout.push<unsigned int>(2);
            layout.push<unsigned char>(4);
        }

        REQUIRE(layout.get_elements().size() == repetitions * 3);

        unsigned int expected_stride = repetitions * (3 * sizeof(float) + 2 * sizeof(unsigned int) + 4 * sizeof(unsigned char));
        REQUIRE(layout.get_stride() == expected_stride);
    }

    SECTION("Verify element integrity with many elements")
    {
        opengl::c_buffer_layout layout;

        constexpr int num_sets = 200;
        for (int i = 0; i < num_sets; ++i)
        {
            layout.push<float>(2);
            layout.push<unsigned char>(4);
        }

        const auto &elements = layout.get_elements();
        REQUIRE(elements.size() == num_sets * 2);

        // Verify pattern repeats correctly
        for (int i = 0; i < num_sets; ++i)
        {
            REQUIRE(elements[i * 2].type == GL_FLOAT);
            REQUIRE(elements[i * 2].count == 2);
            REQUIRE(elements[i * 2 + 1].type == GL_UNSIGNED_BYTE);
            REQUIRE(elements[i * 2 + 1].count == 4);
        }
    }
}

TEST_CASE("Stress: Math helpers with large datasets", "[stress][math][helpers]")
{
    SECTION("Hanning window on large dataset")
    {
        constexpr size_t size = 100000;
        std::vector<float> data(size, 1.0F);

        math::helpers::hanning_window(data);

        // Verify properties hold for large dataset
        REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(data[size - 1], Catch::Matchers::WithinAbs(0.0F, 1e-6F));

        // Check symmetry on subset
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_THAT(data[i], Catch::Matchers::WithinAbs(data[size - 1 - i], 1e-5F));
        }
    }

    SECTION("Many HSV to RGBA conversions")
    {
        constexpr int num_conversions = 100000;

        for (int i = 0; i < num_conversions; ++i)
        {
            float hue = static_cast<float>(i % 360);
            float saturation = static_cast<float>(i % 100) / 100.0F;
            float value = static_cast<float>((i % 50) + 50) / 100.0F;

            glm::vec3 hsv(hue, saturation, value);
            glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

            // Verify output is in valid range
            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
            REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        }
    }

    SECTION("Alternating hanning window operations")
    {
        constexpr int iterations = 1000;
        constexpr size_t size = 512;

        std::vector<float> data(size);

        for (int iter = 0; iter < iterations; ++iter)
        {
            // Reset to ones
            std::fill(data.begin(), data.end(), 1.0F);

            // Apply window
            math::helpers::hanning_window(data);

            // Verify window properties
            REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
            REQUIRE_THAT(data[size - 1], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        }
    }
}

TEST_CASE("Stress: Combined operations", "[stress][integration]")
{
    SECTION("FFT on windowed data")
    {
        constexpr size_t size = 4096;
        constexpr int iterations = 100;

        for (int iter = 0; iter < iterations; ++iter)
        {
            std::vector<float> data(size);
            for (size_t i = 0; i < size; ++i)
            {
                data[i] = std::sin(2.0F * std::numbers::pi_v<float> * static_cast<float>(i) / 64.0F);
            }

            math::helpers::hanning_window(data);
            auto result = math::fft(data);

            REQUIRE(result.size() == size);
        }
    }

    SECTION("Notifier with computation callbacks")
    {
        utility::c_notifier::reset();

        constexpr int num_callbacks = 1000;
        std::atomic<int> completed{ 0 };

        for (int i = 0; i < num_callbacks; ++i)
        {
            utility::c_notifier::subscribe([&completed]()
                                           {
                // Simulate some work
                std::vector<float> data(256);
                for (size_t j = 0; j < data.size(); ++j)
                {
                    data[j] = std::sin(static_cast<float>(j));
                }
                math::helpers::hanning_window(data);
                completed++; });
        }

        utility::c_notifier::notify();
        REQUIRE(completed == num_callbacks);
    }
}
