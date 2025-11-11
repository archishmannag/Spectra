module;
#include <algorithm>
#include <complex>
#include <concepts>
#include <numbers>
#include <stdexcept>
#include <valarray>
#include <vector>
export module math:fft;

using namespace std::literals::complex_literals;

// Implementation
namespace math
{
    /**
     * @brief Fast Fourier Transform
     *
     * NOTE: Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#C++
     *
     * @param x input data
     * @return transformed data
     */
    template <std::floating_point T = float>
    auto fft_impl(std::valarray<std::complex<T>> &input) -> void
    {
        static constexpr auto pi_val = std::numbers::pi_v<T>;
        const std::size_t size = input.size();
        if (size <= 1)
        {
            return;
        }

        // divide
        std::valarray<std::complex<T>> even = input[std::slice(0, size / 2, 2)];
        std::valarray<std::complex<T>> odd = input[std::slice(1, size / 2, 2)];

        // conquer
        fft_impl(even);
        fft_impl(odd);

        // combine
        for (std::size_t k = 0; k < size / 2; ++k)
        {
            // Polar rotates the vector, product scales it.
            std::complex<T> vector = std::polar(T{ 1 }, -T{ 2 } * pi_val * static_cast<T>(k) / static_cast<T>(size)) * odd[k];
            input[k] = even[k] + vector;
            input[k + (size / 2)] = even[k] - vector;
        }
    }

    /**
     * @brief Inverse Fast Fourier Transform
     *
     * NOTE: Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#C++
     *
     * @param x input data
     * @return transformed data
     */
    template <std::floating_point T = float>
    auto ifft_impl(std::valarray<std::complex<T>> &input) -> void
    {
        // conjugate the complex numbers
        input = input.apply(std::conj);

        // forward fft
        fft_impl(input);

        // conjugate the complex numbers again
        input = input.apply(std::conj);

        // scale the numbers
        input /= static_cast<T>(input.size());
    }
} // namespace math

export namespace math
{
    template <std::floating_point T = float>
    auto fft(const std::vector<T> &input) -> std::vector<std::complex<T>>
    {
        if (input.empty())
        {
            throw std::invalid_argument("Input vector is empty");
        }
        std::valarray<std::complex<T>> temp(input.size());
        std::ranges::transform(input, std::begin(temp), [](const T &val)
                               { return std::complex<T>(val, T{ 0 }); });
        fft_impl(temp);
        return std::vector<std::complex<T>>(std::from_range, temp);
    }

    template <std::floating_point T = float>
    auto ifft(const std::vector<std::complex<T>> &input) -> std::vector<T>
    {
        if (input.empty())
        {
            throw std::invalid_argument("Input vector is empty");
        }
        std::valarray<std::complex<T>> temp(input.data(), input.size());
        ifft_impl(temp);
        std::vector<T> output(input.size());
        std::ranges::transform(temp, std::begin(output), [](const std::complex<T> &val)
                               { return val.real(); });
        return output;
    }
} // namespace math
