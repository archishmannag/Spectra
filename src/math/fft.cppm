module;
#include <algorithm>
#include <complex>
#include <concepts>
#include <numbers>
#include <stdexcept>
#include <valarray>
#include <vector>
export module fft;

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
    auto fft(std::valarray<std::complex<T>> &input) -> void
    {
        auto constexpr pi_val = std::numbers::pi_v<T>;
        T constexpr real = 1.0;
        const size_t size = input.size();
        if (size <= 1)
        {
            return;
        }

        // divide
        std::valarray<std::complex<T>> even = input[std::slice(0, size / 2, 2)];
        std::valarray<std::complex<T>> odd = input[std::slice(1, size / 2, 2)];

        // conquer
        fft(even);
        fft(odd);

        // combine
        for (size_t k = 0; k < size / 2; ++k)
        {
            // Polar rotates the vector, product scales it.
            std::complex<T> vector = std::operator*(std::polar(real, -2 * pi_val * k / size), odd[k]);
            input[k] = std::operator+(even[k], vector);
            input[k + (size / 2)] = std::operator-(even[k], vector);
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
    auto ifft(std::valarray<std::complex<T>> &input) -> void
    {
        // conjugate the complex numbers
        input = input.apply(std::conj);

        // forward fft
        fft(input);

        // conjugate the complex numbers again
        input = input.apply(std::conj);

        // scale the numbers
        input /= input.size();
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
        std::ranges::transform(input, std::begin(temp), [](T val)
                               { return std::complex<T>(val, 0); });
        fft(temp);
        std::vector<std::complex<T>> output(std::begin(temp), std::end(temp));
        return output;
    }

    template <std::floating_point T = float>
    auto ifft(const std::vector<std::complex<T>> &input) -> std::vector<T>
    {
        if (input.empty())
        {
            throw std::invalid_argument("Input vector is empty");
        }
        std::valarray<std::complex<T>> temp(input.data(), input.size());
        fft(temp);
        std::vector<T> output(input.size());
        std::ranges::transform(std::begin(temp), std::end(temp), std::begin(output), [](std::complex<T> val)
                               { return val.real(); });
        return output;
    }
} // namespace math
