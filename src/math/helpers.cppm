module;
#include <cmath>
#include <numbers>
#include <ranges>
export module math:helpers;

import glm;

export namespace math::helpers
{
    template <typename R>
    concept floating_point_range = std::ranges::range<R> && std::is_floating_point_v<std::ranges::range_value_t<R>>;

    constexpr auto hanning_window(floating_point_range auto &range) -> void
    {
        using T = std::ranges::range_value_t<decltype(range)>;

        const auto size = static_cast<double>(std::ranges::size(range));
        constexpr T half = T(0.5);
        constexpr T one = T(1);
        constexpr T two_pi = T(2) * std::numbers::pi_v<T>;

        for (auto i = 0U; i < size; ++i)
        {
            range[i] *= static_cast<T>(half * (one - std::cos((two_pi * static_cast<T>(i)) / (size - one))));
        }
    }

    /**
     * Converts HSV color values to RGBA color values.
     *
     * Hue: 0-360
     *
     * Saturation: 0-1
     *
     * Value: 0-1
     *
     * Alpha: Preset to 1.0 (fully opaque)
     *
     * @param hsv
     * @return vec4 RGBA color values.
     */
    auto hsv_to_rgba(const glm::vec3 &hsv) -> glm::vec4
    {
        float hue = std::fmod(hsv.x, 360.F);
        float saturation = hsv.y;
        float value = hsv.z;

        int hue_segment = static_cast<int>(hue / 60.0F) % 6;
        float fractional_hue = (hue / 60.0F) - static_cast<float>(hue_segment);
        float primary_value = value * (1.0F - saturation);
        float secondary_value = value * (1.0F - fractional_hue * saturation);
        float tertiary_value = value * (1.0F - (1.0F - fractional_hue) * saturation);

        switch (hue_segment)
        {
        case 0:
            return { value, tertiary_value, primary_value, 1.0F };
        case 1:
            return { secondary_value, value, primary_value, 1.0F };
        case 2:
            return { primary_value, value, tertiary_value, 1.0F };
        case 3:
            return { primary_value, secondary_value, value, 1.0F };
        case 4:
            return { tertiary_value, primary_value, value, 1.0F };
        case 5:
            return { value, primary_value, secondary_value, 1.0F };
        default:
            return { 0.0F, 0.0F, 0.0F, 1.0F }; // Fallback case
        }
    }

    constexpr auto operator""_percent(long double value) -> float
    {
        return static_cast<float>(value / 100.0L);
    }
} // namespace math::helpers
