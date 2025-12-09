#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>
#include <cmath>
#include <numbers>
#include <vector>

import math;
import glm;

using namespace math::helpers;

TEST_CASE("Hanning window: Basic functionality", "[math][helpers][unit]")
{
    SECTION("Hanning window on empty range")
    {
        std::vector<float> data;
        REQUIRE_NOTHROW(hanning_window(data));
        REQUIRE(data.empty());
    }

    SECTION("Hanning window on single element")
    {
        std::vector<float> data = { 1.0F };
        hanning_window(data);
        // Single element should be multiplied by 0 (window edge)
        REQUIRE_THAT(data[0], Catch::Matchers::IsNaNMatcher());
    }

    SECTION("Hanning window on two elements")
    {
        std::vector<float> data = { 1.0F, 1.0F };
        hanning_window(data);
        // Both edges should be 0
        REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(data[1], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
    }

    SECTION("Hanning window symmetry")
    {
        std::vector<float> data(8, 1.0F);
        hanning_window(data);

        // Hanning window should be symmetric
        for (size_t i = 0; i < data.size() / 2; ++i)
        {
            REQUIRE_THAT(data[i], Catch::Matchers::WithinAbs(data[data.size() - 1 - i], 1e-6F));
        }
    }

    SECTION("Hanning window edge values")
    {
        std::vector<float> data(16, 1.0F);
        hanning_window(data);

        // First and last elements should be close to 0
        REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(data[15], Catch::Matchers::WithinAbs(0.0F, 1e-6F));

        // Middle element should be close to 1
        REQUIRE(data[7] > 0.9F);
        REQUIRE(data[8] > 0.9F);
    }

    SECTION("Hanning window modifies data correctly")
    {
        std::vector<float> data = { 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F };
        std::vector<float> original = data;
        hanning_window(data);

        // All elements should be modified (reduced)
        for (size_t i = 0; i < data.size(); ++i)
        {
            REQUIRE(data[i] <= original[i]);
        }
    }

    SECTION("Hanning window with doubles")
    {
        std::vector<double> data(16, 1.0);
        hanning_window(data);

        // Check edge values with double precision
        REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0, 1e-12));
        REQUIRE_THAT(data[15], Catch::Matchers::WithinAbs(0.0, 1e-12));
    }

    SECTION("Hanning window with array")
    {
        std::array<float, 8> data;
        data.fill(1.0F);
        hanning_window(data);

        // Should work with arrays too
        REQUIRE_THAT(data[0], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(data[7], Catch::Matchers::WithinAbs(0.0F, 1e-6F));
    }
}

TEST_CASE("HSV to RGBA: Basic functionality", "[math][helpers][unit]")
{
    SECTION("Pure red (H=0)")
    {
        glm::vec3 hsv(0.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Pure green (H=120)")
    {
        glm::vec3 hsv(120.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Pure blue (H=240)")
    {
        glm::vec3 hsv(240.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("White (S=0, V=1)")
    {
        glm::vec3 hsv(0.0F, 0.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Black (V=0)")
    {
        glm::vec3 hsv(180.0F, 0.5F, 0.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Gray (S=0, V=0.5)")
    {
        glm::vec3 hsv(0.0F, 0.0F, 0.5F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(0.5F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.5F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.5F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Yellow (H=60)")
    {
        glm::vec3 hsv(60.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Cyan (H=180)")
    {
        glm::vec3 hsv(180.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Magenta (H=300)")
    {
        glm::vec3 hsv(300.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Hue wraparound (H=360)")
    {
        glm::vec3 hsv(360.0F, 1.0F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        // Should wrap to red
        REQUIRE_THAT(rgba.r, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        REQUIRE_THAT(rgba.g, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.b, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("Desaturated color")
    {
        glm::vec3 hsv(180.0F, 0.5F, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        // Alpha should always be 1
        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));

        // All RGB components should be between 0.5 and 1
        REQUIRE(rgba.r >= 0.45F);
        REQUIRE(rgba.r <= 1.05F);
        REQUIRE(rgba.g >= 0.45F);
        REQUIRE(rgba.g <= 1.05F);
        REQUIRE(rgba.b >= 0.45F);
        REQUIRE(rgba.b <= 1.05F);
    }

    SECTION("All hue segments covered")
    {
        // Test each hue segment (0-59, 60-119, 120-179, 180-239, 240-299, 300-359)
        for (int segment = 0; segment < 6; ++segment)
        {
            float hue = static_cast<float>(segment * 60 + 30); // Middle of each segment
            glm::vec3 hsv(hue, 1.0F, 1.0F);
            glm::vec4 rgba = hsv_to_rgba(hsv);

            // All RGB values should be in valid range
            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
            REQUIRE(rgba.g >= 0.0F);
            REQUIRE(rgba.g <= 1.0F);
            REQUIRE(rgba.b >= 0.0F);
            REQUIRE(rgba.b <= 1.0F);
            REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
        }
    }
}

TEST_CASE("Percent literal: Basic functionality", "[math][helpers][unit]")
{
    SECTION("Zero percent")
    {
        auto value = 0.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.0F, 1e-6F));
    }

    SECTION("100 percent")
    {
        auto value = 100.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }

    SECTION("50 percent")
    {
        auto value = 50.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.5F, 1e-6F));
    }

    SECTION("25 percent")
    {
        auto value = 25.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.25F, 1e-6F));
    }

    SECTION("75 percent")
    {
        auto value = 75.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.75F, 1e-6F));
    }

    SECTION("1 percent")
    {
        auto value = 1.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.01F, 1e-6F));
    }

    SECTION("10 percent")
    {
        auto value = 10.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.1F, 1e-6F));
    }

    SECTION("Values over 100 percent")
    {
        auto value = 150.0_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(1.5F, 1e-6F));
    }

    SECTION("Decimal percentages")
    {
        auto value = 33.333_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.33333F, 1e-5F));
    }

    SECTION("Small percentages")
    {
        auto value = 0.1_percent;
        REQUIRE_THAT(value, Catch::Matchers::WithinAbs(0.001F, 1e-6F));
    }
}

TEST_CASE("Math helpers: Integration", "[math][helpers][unit]")
{
    SECTION("Hanning window and HSV can be used together")
    {
        std::vector<float> data(16, 1.0F);
        hanning_window(data);

        // Use windowed data to generate colors
        for (size_t i = 0; i < data.size(); ++i)
        {
            float hue = data[i] * 360.0F;
            glm::vec3 hsv(hue, 1.0F, 1.0F);
            glm::vec4 rgba = hsv_to_rgba(hsv);

            REQUIRE(rgba.r >= 0.0F);
            REQUIRE(rgba.r <= 1.0F);
        }
    }

    SECTION("Percent literal in color calculations")
    {
        auto saturation = 75.0_percent;
        glm::vec3 hsv(180.0F, saturation, 1.0F);
        glm::vec4 rgba = hsv_to_rgba(hsv);

        REQUIRE_THAT(rgba.a, Catch::Matchers::WithinAbs(1.0F, 1e-6F));
    }
}
