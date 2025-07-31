module;
#include <miniaudio.h>
#include <sndfile.hh>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
export module music:track;

namespace
{
    struct s_ma_snd_data_source
    {
        ma_data_source_base base{};
        SndfileHandle m_sndfile;
        ma_linear_resampler m_resampler{};
        ma_channel_converter m_channel_converter{};
        bool m_looping{ false };
        ma_uint64 m_length{ 0 };
        ma_uint64 m_cursor{ 0 };
        ma_uint32 m_output_channels{ 2 };
        ma_uint32 m_output_sample_rate{ 44100 };

        explicit s_ma_snd_data_source(const std::filesystem::path &path)
            : m_sndfile(path.string())
        {
            // Initiqlize base with our vtable of custom functions.
            ma_data_source_config base_config = ma_data_source_config_init();
            base_config.vtable = &s_table;
            ma_data_source_init(&base_config, &base);

            // Initialize resampler  to convert to `44.1` kHz
            ma_linear_resampler_config resampler_config = ma_linear_resampler_config_init(ma_format_f32, static_cast<ma_uint32>(m_sndfile.channels()), static_cast<ma_uint32>(m_sndfile.samplerate()), m_output_sample_rate);
            ma_linear_resampler_init(&resampler_config, nullptr, &m_resampler);

            // Initialize channel converter to convert to stereo
            ma_channel_converter_config channel_config = ma_channel_converter_config_init(ma_format_f32, static_cast<ma_uint32>(m_sndfile.channels()), nullptr, m_output_channels, nullptr, ma_channel_mix_mode_default);
            ma_channel_converter_init(&channel_config, nullptr, &m_channel_converter);

            m_length = static_cast<ma_uint64>(m_sndfile.frames()) * m_output_sample_rate / static_cast<ma_uint64>(m_sndfile.samplerate());
        }

        ~s_ma_snd_data_source()
        {
            ma_channel_converter_uninit(&m_channel_converter, nullptr);
            ma_linear_resampler_uninit(&m_resampler, nullptr);
            ma_data_source_uninit(&base);
        }

        static ma_result read_pcm_frames(ma_data_source *data_source, void *out_frames, ma_uint64 frame_count, ma_uint64 *frames_read)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
            auto *out = reinterpret_cast<float *>(out_frames);
            std::size_t frames_read_total = 0;
            bool looped = false;
            ma_uint64 re_size = 0;
            ma_linear_resampler_get_required_input_frame_count(&self->m_resampler, frame_count, &re_size);

            std::vector<float> temp_buffer(re_size * static_cast<std::size_t>(self->m_sndfile.channels()), 0.F);
            while (frames_read_total < re_size)
            {
                auto frames = self->m_sndfile.readf(temp_buffer.data() + (frames_read_total * static_cast<std::size_t>(self->m_sndfile.channels())),
                                                    static_cast<sf_count_t>(frame_count - frames_read_total));
                if (frames <= 0)
                {
                    if (looped)
                    {
                        break;
                    }
                    if (self->m_looping)
                    {
                        self->m_sndfile.seek(0, SEEK_SET);
                        looped = true;
                    }
                    else
                    {
                        break;
                    }
                }
                frames_read_total += static_cast<std::size_t>(frames);
            }

            // Resample to sample rate of 44.1kHz
            std::vector<float> resampled_buffer(frame_count * static_cast<std::size_t>(self->m_sndfile.channels()), 0.F);
            ma_uint64 resampled_frames = resampled_buffer.size();
            ma_linear_resampler_process_pcm_frames(&self->m_resampler, temp_buffer.data(), &re_size, resampled_buffer.data(), &resampled_frames);

            // Perform channel conversion to stereo
            ma_channel_converter_process_pcm_frames(&self->m_channel_converter, out, resampled_buffer.data(), frame_count);

            *frames_read = frame_count;
            self->m_cursor += frame_count;
            if (self->m_cursor >= self->m_length)
            {
                if (self->m_looping)
                {
                    self->m_cursor %= self->m_length;
                }
                else
                {
                    self->m_cursor = self->m_length;
                }
            }
            return MA_SUCCESS;
        }

        static ma_result seek_pcm_frames(ma_data_source *data_source, ma_uint64 frame_index)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
            auto input_seek_index = frame_index * static_cast<ma_uint64>(self->m_sndfile.samplerate()) / self->m_output_sample_rate;
            if (self->m_sndfile.seek(static_cast<sf_count_t>(input_seek_index), SEEK_SET) < 0)
            {
                return MA_ERROR;
            }
            ma_linear_resampler_reset(&self->m_resampler);
            self->m_cursor = frame_index;
            return MA_SUCCESS;
        }

        static ma_result get_data_format(ma_data_source *data_source, ma_format *format, ma_uint32 *channels, ma_uint32 *sample_rate, ma_channel *channel_map, std::size_t channel_map_size)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
            *format = ma_format_f32;
            *channels = self->m_output_channels;
            *sample_rate = self->m_output_sample_rate;
            ma_channel_map_init_standard(ma_standard_channel_map_default, channel_map, channel_map_size, *channels);
            return MA_SUCCESS;
        }

        static ma_result get_cursor(ma_data_source *data_source, ma_uint64 *cursor)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
            *cursor = self->m_cursor;
            return MA_SUCCESS;
        }

        static ma_result get_length(ma_data_source *data_source, ma_uint64 *length)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
            *length = self->m_length;
            return MA_SUCCESS;
        }

        static ma_result set_looping(ma_data_source *pDataSource, ma_bool32 isLooping)
        {
            auto *self = reinterpret_cast<s_ma_snd_data_source *>(pDataSource);
            self->m_looping = (isLooping != 0);
            return MA_SUCCESS;
        }

        static constexpr ma_data_source_vtable s_table{
            .onRead = read_pcm_frames,
            .onSeek = seek_pcm_frames,
            .onGetDataFormat = get_data_format,
            .onGetCursor = get_cursor,
            .onGetLength = get_length,
            .onSetLooping = set_looping,
            .flags = 0
        };
    };
} // namespace

export namespace music
{
    class c_track : public std::enable_shared_from_this<c_track>
    {
    public:
        void play_file(const std::filesystem::path &path);
        void play();
        void pause();
        [[nodiscard]] bool is_playing() const;

        void seek(std::uint64_t frame_index) const;
        [[nodiscard]] std::uint64_t get_cursor_frame() const;
        void set_looping(bool is_looping);
        bool is_looping() const;
        [[nodiscard]] std::uint64_t get_length() const;

        [[nodiscard]] std::string get_filename() const;
        [[nodiscard]] ma_data_source *data_ptr() const;

    private:
        std::unique_ptr<s_ma_snd_data_source> m_snd_data_source;
        std::string m_filename;
        bool m_is_playing{ false };
        bool m_is_looping{ false };
    };
} // namespace music

namespace music
{
    void c_track::play()
    {
        m_is_playing = true;
    }

    void c_track::pause()
    {
        m_is_playing = false;
    }

    bool c_track::is_playing() const
    {
        return m_is_playing;
    }

    void c_track::play_file(const std::filesystem::path &path)
    {
        m_filename = path.filename().string();
        m_snd_data_source = std::make_unique<s_ma_snd_data_source>(path);
    }

    void c_track::seek(std::uint64_t frame_index) const
    {
        if (m_snd_data_source)
        {
            ma_data_source_seek_pcm_frames(m_snd_data_source.get(), frame_index, nullptr);
        }
    }

    std::uint64_t c_track::get_cursor_frame() const
    {
        ma_uint64 cursor = 0;
        if (m_snd_data_source)
        {
            ma_data_source_get_cursor_in_pcm_frames(m_snd_data_source.get(), &cursor);
        }
        return cursor;
    }

    void c_track::set_looping(bool is_looping)
    {
        m_is_looping = is_looping;
        if (m_snd_data_source)
        {
            ma_data_source_set_looping(m_snd_data_source.get(), is_looping ? MA_TRUE : MA_FALSE);
        }
    }

    bool c_track::is_looping() const
    {
        return m_is_looping;
    }

    std::uint64_t c_track::get_length() const
    {
        ma_uint64 length = 0;
        if (m_snd_data_source)
        {
            ma_data_source_get_length_in_pcm_frames(m_snd_data_source.get(), &length);
        }
        return length;
    }

    std::string c_track::get_filename() const
    {
        return m_filename;
    }

    ma_data_source *c_track::data_ptr() const
    {
        return reinterpret_cast<ma_data_source *>(m_snd_data_source.get());
    }
} // namespace music
