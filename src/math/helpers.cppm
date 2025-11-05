module;
#include <cmath>
#include <ranges>
export module math:helpers;

export namespace math::helpers
{
    constexpr auto hanning_window(std::ranges::range auto &range) -> void
    {
        const auto size = static_cast<double>(std::ranges::size(range));
        for (auto i = 0U; i < size; ++i)
        {
            range[i] *= 0.5 * (1 - std::cos((2 * std::numbers::pi * i) / (size - 1)));
        }
    }
} // namespace math::helpers
