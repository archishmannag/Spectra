module;
#include <cmath>
#include <complex>
#include <numbers>
#include <valarray>
export module fft;

export namespace math
{
    /**
     * @brief Fast Fourier Transform
     *
     * NOTE: Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#C++
     *
     * @param x input data
     */
    void fft(std::valarray<std::complex<double>> &input)
    {
        // DFT
        unsigned int size = input.size();
        unsigned int k_val = size;
        unsigned int n_val{};
        double theta_t = std::numbers::pi_v<double> / size;
        std::complex<double> phi_t = std::complex(cos(theta_t), -sin(theta_t));
        std::complex<double> t_val;
        while (k_val > 1)
        {
            n_val = k_val;
            k_val >>= 1;
            phi_t = phi_t * phi_t;
            t_val = 1.0L;
            for (unsigned int outer = 0; outer < k_val; outer++)
            {
                for (unsigned int inner = outer; inner < size; inner += n_val)
                {
                    unsigned int index = inner + k_val;
                    std::complex<double> val = input[inner] - input[index];
                    input[inner] += input[index];
                    input[index] = val * t_val;
                }
                t_val *= phi_t;
            }
        }
        // Decimate
        auto bits = static_cast<unsigned int>(std::log2(size));
        for (unsigned int a_val = 0; a_val < size; a_val++)
        {
            unsigned int b_val = a_val;
            // Reverse bits
            b_val = (((b_val & 0xaaaaaaaa) >> 1) | ((b_val & 0x55555555) << 1));
            b_val = (((b_val & 0xcccccccc) >> 2) | ((b_val & 0x33333333) << 2));
            b_val = (((b_val & 0xf0f0f0f0) >> 4) | ((b_val & 0x0f0f0f0f) << 4));
            b_val = (((b_val & 0xff00ff00) >> 8) | ((b_val & 0x00ff00ff) << 8));
            b_val = ((b_val >> 16) | (b_val << 16)) >> (32 - bits);
            if (b_val > a_val)
            {
                std::complex<double> temp = input[a_val];
                input[a_val] = input[b_val];
                input[b_val] = temp;
            }
        }
    }

    /**
     * @brief Inverse Fast Fourier Transform
     *
     * NOTE: Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#C++
     *
     * @param x input data
     */
    void inv_fft(std::valarray<std::complex<double>> &input)
    {
        // conjugate the complex numbers
        input = input.apply(std::conj);

        // forward fft
        fft(input);

        // conjugate the complex numbers again
        input = input.apply(std::conj);

        // scale the numbers
        input /= static_cast<double>(input.size());
    }
} // namespace math
