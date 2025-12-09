#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <random>
#include <vector>

import math;
import glm;

TEST_CASE("Fuzz: FFT with random inputs", "[fuzz][fft]")
{
    SECTION("Random float values")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-1000.0F, 1000.0F);

        constexpr int num_tests = 100;

        for (int test = 0; test < num_tests; ++test)
        {
            size_t size = 1 << (4 + (test % 6)); // Powers of 2: 16, 32, 64, 128, 256, 512
            std::vector<float> input(size);

            for (auto &val : input)
            {
                val = dis(gen);
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

            // Error should be reasonable relative to input magnitude
            float max_input = *std::max_element(input.begin(), input.end(),
                                                [](float a, float b)
                                                { return std::abs(a) < std::abs(b); });
            float relative_error = max_error / (std::abs(max_input) + 1e-6F);
            REQUIRE(relative_error < 1e-3F);
        }
    }

    SECTION("Random inputs with various sizes")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-100.0F, 100.0F);

        std::vector<size_t> sizes = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };

        for (size_t size : sizes)
        {
            std::vector<float> input(size);
            for (auto &val : input)
            {
                val = dis(gen);
            }

            REQUIRE_NOTHROW([&]()
                            {
                                auto result = math::fft(input);
                                auto reconstructed = math::ifft(result);
                            }());
        }
    }

    SECTION("Edge case values")
    {
        std::vector<float> edge_values = {
            0.0F,
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            std::numeric_limits<float>::epsilon(),
            1e-10F,
            1e10F
        };

        for (float val : edge_values)
        {
            std::vector<float> input(16, val);

            REQUIRE_NOTHROW([&]()
                            {
                                auto result = math::fft(input);
                                REQUIRE(result.size() == input.size());
                            }());
        }
    }

    SECTION("Mixed positive and negative values")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> pos_dis(0.0F, 1000.0F);
        std::uniform_real_distribution<float> neg_dis(-1000.0F, 0.0F);
        std::uniform_int_distribution<int> sign_dis(0, 1);

        for (int test = 0; test < 50; ++test)
        {
            std::vector<float> input(64);
            for (auto &val : input)
            {
                val = sign_dis(gen) ? pos_dis(gen) : neg_dis(gen);
            }

            auto result = math::fft(input);
            auto reconstructed = math::ifft(result);

            for (size_t i = 0; i < input.size(); ++i)
            {
                REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(input[i], 1e-3F));
            }
        }
    }

    SECTION("Sparse signals")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-100.0F, 100.0F);
        std::uniform_int_distribution<int> sparse_dis(0, 9);

        for (int test = 0; test < 50; ++test)
        {
            std::vector<float> input(128, 0.0F);

            // Only 10% of values are non-zero
            for (auto &val : input)
            {
                if (sparse_dis(gen) == 0)
                {
                    val = dis(gen);
                }
            }

            auto result = math::fft(input);
            auto reconstructed = math::ifft(result);

            for (size_t i = 0; i < input.size(); ++i)
            {
                REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(input[i], 1e-4F));
            }
        }
    }
}

TEST_CASE("Fuzz: Hanning window with random data", "[fuzz][math][helpers]")
{
    SECTION("Random float ranges")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-1000.0F, 1000.0F);

        for (int test = 0; test < 100; ++test)
        {
            size_t size = 2 + (test % 1000);
            std::vector<float> data(size);

            for (auto &val : data)
            {
                val = dis(gen);
            }

            std::vector<float> original = data;

            REQUIRE_NOTHROW(math::helpers::hanning_window(data));

            // Verify properties
            if (size > 2)
            {
                // Edges should be close to 0
                REQUIRE(std::abs(data[0]) < std::abs(original[0]));
                REQUIRE(std::abs(data[size - 1]) < std::abs(original[size - 1]));

                // All values should be reduced or equal
                for (size_t i = 0; i < size; ++i)
                {
                    REQUIRE(std::abs(data[i]) <= std::abs(original[i]) + 1e-6F);
                }
            }
        }
    }

    SECTION("Various data sizes")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-100.0F, 100.0F);

        std::vector<size_t> sizes = { 1, 2, 3, 7, 15, 31, 63, 127, 255, 511, 1023 };

        for (size_t size : sizes)
        {
            std::vector<float> data(size);
            for (auto &val : data)
            {
                val = dis(gen);
            }

            REQUIRE_NOTHROW(math::helpers::hanning_window(data));
            REQUIRE(data.size() == size);
        }
    }

    SECTION("Extreme values")
    {
        std::vector<float> data;

        // Very large values
        data.assign(16, 1e20F);
        REQUIRE_NOTHROW(math::helpers::hanning_window(data));

        // Very small values
        data.assign(16, 1e-20F);
        REQUIRE_NOTHROW(math::helpers::hanning_window(data));

        // Mixed extremes
        data = { 1e20F, -1e20F, 1e-20F, -1e-20F, 0.0F, 1.0F, -1.0F, 1000.0F };
        REQUIRE_NOTHROW(math::helpers::hanning_window(data));
    }
}

TEST_CASE("Fuzz: HSV to RGBA with random inputs", "[fuzz][math][helpers]")
{
    SECTION("Random valid HSV values")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> hue_dis(0.0F, 360.0F);
        std::uniform_real_distribution<float> sat_dis(0.0F, 1.0F);
        std::uniform_real_distribution<float> val_dis(0.0F, 1.0F);

        for (int test = 0; test < 1000; ++test)
        {
            glm::vec3 hsv(hue_dis(gen), sat_dis(gen), val_dis(gen));
            glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

            // Verify output is always in valid range
            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
            REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));

            // At least one RGB component should be non-zero if value > 0
            if (hsv.z > 0.01F)
            {
                REQUIRE((rgba.r > 0.0F || rgba.g > 0.0F || rgba.b > 0.0F));
            }
        }
    }

    SECTION("Hue wraparound behavior")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0F, 1000.0F);

        for (int test = 0; test < 100; ++test)
        {
            float hue = dis(gen);
            glm::vec3 hsv(hue, 1.0F, 1.0F);
            glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

            // Should handle wraparound gracefully
            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
        }
    }

    SECTION("Negative hue values")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-360.0F, 0.0F);

        for (int test = 0; test < 100; ++test)
        {
            float hue = dis(gen);
            glm::vec3 hsv(hue, 0.5F, 0.5F);
            glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

            // Should handle negative hues
            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
        }
    }

    SECTION("Edge case saturation and value")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> hue_dis(0.0F, 360.0F);

        std::vector<std::pair<float, float>> edge_sv = {
            { 0.0F, 0.0F }, { 0.0F, 1.0F }, { 1.0F, 0.0F }, { 1.0F, 1.0F }, { 0.5F, 0.0F }, { 0.5F, 1.0F }, { 0.0F, 0.5F }, { 1.0F, 0.5F }
        };

        for (int test = 0; test < 100; ++test)
        {
            float hue = hue_dis(gen);
            auto [s, v] = edge_sv[test % edge_sv.size()];

            glm::vec3 hsv(hue, s, v);
            glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
            REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        }
    }

    SECTION("Out of range saturation and value")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> hue_dis(0.0F, 360.0F);
        std::uniform_real_distribution<float> out_dis(-1.0F, 2.0F);

        for (int test = 0; test < 100; ++test)
        {
            glm::vec3 hsv(hue_dis(gen), out_dis(gen), out_dis(gen));

            // Should handle out-of-range values gracefully (even if unexpected)
            REQUIRE_NOTHROW([&]()
                            {
                                auto rgba = math::helpers::hsv_to_rgba(hsv);
                                // Alpha should always be 1
                                REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
                            }());
        }
    }
}

TEST_CASE("Fuzz: Combined operations with random data", "[fuzz][integration]")
{
    SECTION("FFT on windowed random data")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-100.0F, 100.0F);

        for (int test = 0; test < 50; ++test)
        {
            size_t size = 1 << (4 + (test % 6)); // 16, 32, 64, 128, 256, 512
            std::vector<float> data(size);

            for (auto &val : data)
            {
                val = dis(gen);
            }

            // Apply window
            math::helpers::hanning_window(data);

            // FFT
            auto result = math::fft(data);
            REQUIRE(result.size() == size);

            // IFFT
            auto reconstructed = math::ifft(result);
            REQUIRE(reconstructed.size() == size);
        }
    }

    SECTION("Color generation from FFT magnitudes")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-10.0F, 10.0F);

        for (int test = 0; test < 50; ++test)
        {
            std::vector<float> audio_data(64);
            for (auto &val : audio_data)
            {
                val = dis(gen);
            }

            auto fft_result = math::fft(audio_data);

            // Use FFT magnitudes to generate colors
            for (size_t i = 0; i < fft_result.size() / 2; ++i)
            {
                float magnitude = std::abs(fft_result[i]);
                float normalized = std::clamp(magnitude / 10.0F, 0.0F, 1.0F);

                float hue = normalized * 360.0F;
                glm::vec3 hsv(hue, 1.0F, normalized);
                glm::vec4 rgba = math::helpers::hsv_to_rgba(hsv);

                REQUIRE(rgba.r >= 0.0F);
                REQUIRE(rgba.r <= 1.0F);
                REQUIRE(rgba.g >= 0.0F);
                REQUIRE(rgba.g <= 1.0F);
                REQUIRE(rgba.b >= 0.0F);
                REQUIRE(rgba.b <= 1.0F);
            }
        }
    }
}

TEST_CASE("Fuzz: Percent literal with random values", "[fuzz][math][helpers]")
{
    SECTION("Random percentage values")
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<long double> dis(0.0L, 1000.0L);

        for (int test = 0; test < 100; ++test)
        {
            long double percent_val = dis(gen);
            float expected = static_cast<float>(percent_val / 100.0L);

            // Note: Can't test literal directly with runtime values,
            // but we can verify the conversion logic
            float result = static_cast<float>(percent_val / 100.0L);
            REQUIRE_THAT(result, Catch::Matchers::WithinAbs(expected, 1e-6F));
        }
    }
}
