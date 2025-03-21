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

        std::vector<float> get_samples(int fps) const;

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

    std::vector<float> c_track::get_samples(int fps = 60) const
    {
        // NOTE: From sf::Music's private function TimeToSamples
        auto offset = (m_sound.getPlayingOffset().asMicroseconds() * m_sample_rate * m_channels + 50'000) / 1'000'000;

        std::size_t size = m_sample_rate /* * m_channels */ / fps;
        size = std::min(size, static_cast<std::size_t>(m_buffer.getSampleCount() - offset));
        std::vector<float> samples(size);
        if (size)
        {
            // std::copy_n(m_buffer.getSamples() + offset, size, samples.begin());
            for (auto i = 0; i < size; ++i)
            {
                samples[i] = m_buffer.getSamples()[offset + (m_channels * i)];
            }
        }
        return samples;
    }
} // namespace music
