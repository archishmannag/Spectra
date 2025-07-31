module;
#include <miniaudio.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <vector>
export module music:audio;
import :track;

export namespace music
{
    class c_audio_manager
    {
    public:
        c_audio_manager();
        ~c_audio_manager();

        void add_track(std::shared_ptr<c_track> &track);

    private:
        mutable std::mutex m_mutex;
        ma_context m_context{};
        ma_device m_device{};
        std::vector<std::weak_ptr<c_track>> m_tracks;

        static constexpr void s_callback_fn(ma_device *device, void *output, const void *input, ma_uint32 frame_count);
    };
} // namespace music

// Implementation
namespace music
{
    c_audio_manager::c_audio_manager()
    {
        ma_context_config context_config = ma_context_config_init();
        auto result = ma_context_init(nullptr, 0, &context_config, &m_context);
        if (result != MA_SUCCESS)
        {
            std::println(std::cerr, "Failed to initialize audio context: {}", ma_result_description(result));
            return;
        }
        ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
        device_config.playback.format = ma_format_f32;
        device_config.playback.channels = 2;
        device_config.sampleRate = 44100;
        device_config.dataCallback = s_callback_fn;
        device_config.pUserData = this;

        result = ma_device_init(&m_context, &device_config, &m_device);
        if (result != MA_SUCCESS)
        {
            std::println(std::cerr, "Failed to initialize audio device: {}", ma_result_description(result));
            ma_context_uninit(&m_context);
            return;
        }
        result = ma_device_start(&m_device);
        if (result != MA_SUCCESS)
        {
            std::println(std::cerr, "Failed to start audio device: {}", ma_result_description(result));
            ma_device_uninit(&m_device);
            ma_context_uninit(&m_context);
            return;
        }
    }

    c_audio_manager::~c_audio_manager()
    {
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
        m_tracks.clear();
    }

    void c_audio_manager::add_track(std::shared_ptr<c_track> &track)
    {
        std::lock_guard lock(m_mutex);
        m_tracks.push_back(track);
    }

    constexpr void c_audio_manager::s_callback_fn(ma_device *device, void *output, const void * /*input*/, ma_uint32 frame_count)
    {
        auto *audio_manager = reinterpret_cast<c_audio_manager *>(device->pUserData);
        auto *output_samples = reinterpret_cast<float *>(output);
        std::fill(output_samples, output_samples + static_cast<std::size_t>(frame_count * device->playback.channels), 0.F);
        std::lock_guard lock(audio_manager->m_mutex);
        for (const auto &track_ptr_weak : audio_manager->m_tracks)
        {
            auto track_ptr = track_ptr_weak.lock();
            if (!track_ptr)
            {
                continue;
            }
            if (track_ptr->is_playing())
            {
                std::vector<float> temp(static_cast<std::size_t>(frame_count * device->playback.channels));
                ma_uint64 frames_read = 0;
                ma_data_source_read_pcm_frames(track_ptr->data_ptr(), temp.data(), frame_count, &frames_read);
                std::transform(output_samples, output_samples + static_cast<std::size_t>(frame_count * device->playback.channels), temp.begin(), output_samples, std::plus<float>{});
            }
        }
    }

} // namespace music
