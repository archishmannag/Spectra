module;
#include <SFML/Audio.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <vector>
export module music;

export namespace music
{
    class c_track
    {
    public:
        explicit c_track(const std::filesystem::path &path);

        void play();
        void pause();
        bool is_playing() const;

        std::vector<float> get_samples(int fps = 60);

    private:
        sf::SoundBuffer m_buffer;
        sf::Sound m_sound;
        unsigned int m_sample_rate;
        unsigned int m_channels;
    };
} // namespace music

namespace music
{
    c_track::c_track(const std::filesystem::path &path)
    {
        m_buffer.loadFromFile(path.string());
        m_sound.setBuffer(m_buffer);

        m_sample_rate = m_buffer.getSampleRate();
        m_channels = m_buffer.getChannelCount();
    }

    void c_track::play()
    {
        m_sound.play();
    }

    void c_track::pause()
    {
        m_sound.pause();
    }

    bool c_track::is_playing() const
    {
        return m_sound.getStatus() == sf::Sound::Playing;
    }

    std::vector<float> c_track::get_samples(int fps)
    {
        // Calculate the sample offset based on current playback position
        auto offset_microseconds = m_sound.getPlayingOffset().asMicroseconds();
        offset_microseconds = std::max<std::int64_t>(offset_microseconds, 0);

        auto offset = static_cast<std::size_t>((static_cast<int64_t>(offset_microseconds) * m_sample_rate * m_channels) / 1'000'000);

        // Calculate how many samples we need for the current frame
        std::size_t samples_needed = m_sample_rate / fps;

        // Make sure we don't try to read past the end of the buffer
        std::size_t samples_available = (m_buffer.getSampleCount() > offset) ? (m_buffer.getSampleCount() - offset) / m_channels : 0;

        std::size_t size = std::min(samples_needed, samples_available);

        std::vector<float> samples(size);

        // Only copy samples if we actually have data to copy
        if (size > 0)
        {
            const std::int16_t *buffer_samples = m_buffer.getSamples();
            for (std::size_t i = 0; i < size; ++i)
            {
                // Only take the first channel's sample from each frame
                samples[i] = buffer_samples[offset + (m_channels * i)];
            }
        }
        else
        {
            m_sound.stop();
        }

        return samples;
    }
} // namespace music
