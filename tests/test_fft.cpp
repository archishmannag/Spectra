#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

import math;

using namespace std::complex_literals;

TEST_CASE("FFT: Basic functionality", "[fft][unit]")
{
    SECTION("FFT of empty vector throws exception")
    {
        std::vector<float> empty_input;
        REQUIRE_THROWS_AS(math::fft(empty_input), std::invalid_argument);
    }

    SECTION("FFT of single element")
    {
        std::vector<float> input = { 1.0F };
        auto result = math::fft(input);
        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result[0].real(), Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(result[0].imag(), Catch::Matchers::WithinAbs(0.0F, 1e-6F));
    }

    SECTION("FFT of two elements")
    {
        std::vector<float> input = { 1.0F, 2.0F };
        auto result = math::fft(input);
        REQUIRE(result.size() == 2);
        REQUIRE_THAT(result[0].real(), Catch::Matchers::WithinAbs(3.0F, 1e-6F));
        REQUIRE_THAT(result[0].imag(), Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(result[1].real(), Catch::Matchers::WithinAbs(-1.0F, 1e-6F));
        REQUIRE_THAT(result[1].imag(), Catch::Matchers::WithinAbs(0.0F, 1e-6F));
    }

    SECTION("FFT of constant signal")
    {
        std::vector<float> input(8, 1.0F);
        auto result = math::fft(input);
        REQUIRE(result.size() == 8);
        // DC component should be 8, all others should be ~0
        REQUIRE_THAT(result[0].real(), Catch::Matchers::WithinAbs(8.0F, 1e-5F));
        REQUIRE_THAT(result[0].imag(), Catch::Matchers::WithinAbs(0.0F, 1e-5F));
        for (size_t i = 1; i < result.size(); ++i)
        {
            REQUIRE_THAT(std::abs(result[i]), Catch::Matchers::WithinAbs(0.0F, 1e-5F));
        }
    }

    SECTION("FFT of sine wave")
    {
        // Create a simple sine wave at frequency 1 (one cycle over 8 samples)
        std::vector<float> input(8);
        constexpr float two_pi = 2.0F * std::numbers::pi_v<float>;
        for (size_t i = 0; i < 8; ++i)
        {
            input[i] = std::sin(two_pi * static_cast<float>(i) / 8.0F);
        }

        auto result = math::fft(input);
        REQUIRE(result.size() == 8);

        // For a pure sine wave, we expect peaks at frequencies 1 and 7 (symmetric)
        REQUIRE(std::abs(result[1]) > 2.0F); // Should have significant magnitude
        REQUIRE(std::abs(result[7]) > 2.0F); // Mirror frequency
    }

    SECTION("FFT power of 2 sizes")
    {
        for (size_t size : { 2, 4, 8, 16, 32, 64, 128, 256 })
        {
            std::vector<float> input(size, 1.0F);
            auto result = math::fft(input);
            REQUIRE(result.size() == size);
            REQUIRE_THAT(result[0].real(), Catch::Matchers::WithinAbs(static_cast<float>(size), 1e-4F));
        }
    }
}

TEST_CASE("IFFT: Basic functionality", "[fft][unit]")
{
    SECTION("IFFT of empty vector throws exception")
    {
        std::vector<std::complex<float>> empty_input;
        REQUIRE_THROWS_AS(math::ifft(empty_input), std::invalid_argument);
    }

    SECTION("IFFT of single element")
    {
        std::vector<std::complex<float>> input = { 1.0F + 0.0iF };
        auto result = math::ifft(input);
        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("IFFT of two elements")
    {
        std::vector<std::complex<float>> input = { 3.0F + 0.0iF, -1.0F + 0.0iF };
        auto result = math::ifft(input);
        REQUIRE(result.size() == 2);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(result[1], Catch::Matchers::WithinAbs(2.0F, 1e-6F));
    }
}

TEST_CASE("FFT/IFFT: Round-trip consistency", "[fft][unit]")
{
    SECTION("Round-trip with simple signal")
    {
        std::vector<float> original = { 1.0F, 2.0F, 3.0F, 4.0F };
        auto transformed = math::fft(original);
        auto reconstructed = math::ifft(transformed);

        REQUIRE(reconstructed.size() == original.size());
        for (size_t i = 0; i < original.size(); ++i)
        {
            REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(original[i], 1e-5F));
        }
    }

    SECTION("Round-trip with larger signal")
    {
        std::vector<float> original(64);
        for (size_t i = 0; i < original.size(); ++i)
        {
            original[i] = std::sin(2.0F * std::numbers::pi_v<float> * static_cast<float>(i) / 64.0F);
        }

        auto transformed = math::fft(original);
        auto reconstructed = math::ifft(transformed);

        REQUIRE(reconstructed.size() == original.size());
        for (size_t i = 0; i < original.size(); ++i)
        {
            REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(original[i], 1e-4F));
        }
    }

    SECTION("Round-trip with random-like data")
    {
        std::vector<float> original = {
            1.5F, -2.3F, 4.7F, -0.8F,
            3.2F, -1.1F, 0.9F, 2.6F
        };

        auto transformed = math::fft(original);
        auto reconstructed = math::ifft(transformed);

        REQUIRE(reconstructed.size() == original.size());
        for (size_t i = 0; i < original.size(); ++i)
        {
            REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(original[i], 1e-5F));
        }
    }
}

TEST_CASE("FFT: Double precision", "[fft][unit]")
{
    SECTION("FFT with double precision")
    {
        std::vector<double> input = { 1.0, 2.0, 3.0, 4.0 };
        auto result = math::fft(input);
        REQUIRE(result.size() == 4);

        // Verify it's actually using double precision
        REQUIRE_THAT(result[0].real(), Catch::Matchers::WithinAbs(10.0, 1e-12));
    }

    SECTION("Round-trip with double precision")
    {
        std::vector<double> original = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
        auto transformed = math::fft(original);
        auto reconstructed = math::ifft(transformed);

        REQUIRE(reconstructed.size() == original.size());
        for (size_t i = 0; i < original.size(); ++i)
        {
            REQUIRE_THAT(reconstructed[i], Catch::Matchers::WithinAbs(original[i], 1e-10));
        }
    }
}

TEST_CASE("FFT: Parseval's theorem", "[fft][unit]")
{
    SECTION("Energy conservation in frequency domain")
    {
        std::vector<float> input = { 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F };

        // Calculate energy in time domain
        float time_energy = 0.0F;
        for (const auto &val : input)
        {
            time_energy += val * val;
        }

        // Calculate energy in frequency domain
        auto transformed = math::fft(input);
        float freq_energy = 0.0F;
        for (const auto &val : transformed)
        {
            freq_energy += std::norm(val);
        }
        freq_energy /= static_cast<float>(input.size());

        // Energies should be equal (within numerical precision)
        REQUIRE_THAT(freq_energy, Catch::Matchers::WithinAbs(time_energy, 1e-3F));
    }
}

TEST_CASE("FFT: Symmetry properties", "[fft][unit]")
{
    SECTION("Real input produces conjugate symmetric output")
    {
        std::vector<float> input = { 1.0F, 2.0F, 3.0F, 4.0F, 4.0F, 3.0F, 2.0F, 1.0F };
        auto result = math::fft(input);

        size_t N = result.size();
        // Check conjugate symmetry: X[k] = conj(X[N-k])
        for (size_t k = 1; k < N / 2; ++k)
        {
            REQUIRE_THAT(result[k].real(), Catch::Matchers::WithinAbs(result[N - k].real(), 1e-5F));
            REQUIRE_THAT(result[k].imag(), Catch::Matchers::WithinAbs(-result[N - k].imag(), 1e-5F));
        }
    }
}
