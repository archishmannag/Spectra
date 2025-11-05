module;
#include <miniaudio.h>
#include <sndfile.hh>

#include <filesystem>
#include <string>
#include <vector>
export module music:track;

namespace
{
    struct s_ma_snd_data_source
    {
        explicit s_ma_snd_data_source(const std::filesystem::path &path);
        ~s_ma_snd_data_source();

        // Explicitly enable move semantics, disable copy semantics
        s_ma_snd_data_source(s_ma_snd_data_source &&) = default;
        auto operator=(s_ma_snd_data_source &&) -> s_ma_snd_data_source & = default;

        static auto read_pcm_frames(ma_data_source *data_source, void *out_frames, ma_uint64 frame_count, ma_uint64 *frames_read) -> ma_result;
        static auto seek_pcm_frames(ma_data_source *data_source, ma_uint64 frame_index) -> ma_result;
        static auto get_data_format(ma_data_source *data_source, ma_format *format, ma_uint32 *channels, ma_uint32 *sample_rate, ma_channel *channel_map, std::size_t channel_map_size) -> ma_result;
        static auto get_cursor(ma_data_source *data_source, ma_uint64 *cursor) -> ma_result;
        static auto get_length(ma_data_source *data_source, ma_uint64 *length) -> ma_result;
        static auto set_looping(ma_data_source *pDataSource, ma_bool32 isLooping) -> ma_result;

        static constexpr ma_data_source_vtable s_table{
            .onRead = read_pcm_frames,
            .onSeek = seek_pcm_frames,
            .onGetDataFormat = get_data_format,
            .onGetCursor = get_cursor,
            .onGetLength = get_length,
            .onSetLooping = set_looping,
            .flags = 0
        };

    private:
        ma_data_source_base m_base{};
        SndfileHandle m_sndfile;
        ma_linear_resampler m_resampler{};
        ma_channel_converter m_channel_converter{};
        bool m_looping{ false };
        ma_uint64 m_length{ 0 };
        ma_uint64 m_cursor{ 0 };
        ma_uint32 m_output_channels{ 2 };
        ma_uint32 m_output_sample_rate{ 44100 };
    };
} // namespace

export namespace music
{
    class c_track
    {
    public:
        explicit c_track(int track_id, const std::filesystem::path &path);

        c_track(c_track &&other) noexcept;
        auto operator=(c_track &&other) noexcept -> c_track &;

        [[nodiscard]] auto get_track_id() const -> int;

        auto play() -> void;
        auto pause() -> void;
        [[nodiscard]] auto is_playing() const -> bool;

        auto seek(std::uint64_t frame_index) -> void;
        auto set_looping(bool is_looping) -> void;
        [[nodiscard]] auto get_cursor_frame() -> std::uint64_t;
        [[nodiscard]] auto is_looping() const -> bool;
        [[nodiscard]] auto get_total_frames() -> std::uint64_t;

        [[nodiscard]] auto get_filename() const -> std::string;
        [[nodiscard]] auto data_ptr() -> ma_data_source *;

    private:
        int m_track_id{ 0 };
        s_ma_snd_data_source m_snd_data_source;
        std::string m_filename;
        bool m_is_playing{ false };
        bool m_is_looping{ false };
    };
} // namespace music

// Implementation
namespace
{
    auto s_ma_snd_data_source::set_looping(ma_data_source *pDataSource, ma_bool32 isLooping) -> ma_result
    {
        auto *self = reinterpret_cast<s_ma_snd_data_source *>(pDataSource);
        self->m_looping = (isLooping != 0);
        return MA_SUCCESS;
    }

    auto s_ma_snd_data_source::get_length(ma_data_source *data_source, ma_uint64 *length) -> ma_result
    {
        auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
        *length = self->m_length;
        return MA_SUCCESS;
    }

    auto s_ma_snd_data_source::get_cursor(ma_data_source *data_source, ma_uint64 *cursor) -> ma_result
    {
        auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
        *cursor = self->m_cursor;
        return MA_SUCCESS;
    }

    auto s_ma_snd_data_source::get_data_format(ma_data_source *data_source, ma_format *format, ma_uint32 *channels, ma_uint32 *sample_rate, ma_channel *channel_map, std::size_t channel_map_size) -> ma_result
    {
        auto *self = reinterpret_cast<s_ma_snd_data_source *>(data_source);
        *format = ma_format_f32;
        *channels = self->m_output_channels;
        *sample_rate = self->m_output_sample_rate;
        ma_channel_map_init_standard(ma_standard_channel_map_default, channel_map, channel_map_size, *channels);
        return MA_SUCCESS;
    }

    auto s_ma_snd_data_source::seek_pcm_frames(ma_data_source *data_source, ma_uint64 frame_index) -> ma_result
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

    auto s_ma_snd_data_source::read_pcm_frames(ma_data_source *data_source, void *out_frames, ma_uint64 frame_count, ma_uint64 *frames_read) -> ma_result
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
            auto frames = self->m_sndfile.readf(temp_buffer.data() + (frames_read_total * static_cast<std::size_t>(self->m_sndfile.channels())), static_cast<sf_count_t>(re_size - frames_read_total));
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

    s_ma_snd_data_source::~s_ma_snd_data_source()
    {
        ma_channel_converter_uninit(&m_channel_converter, nullptr);
        ma_linear_resampler_uninit(&m_resampler, nullptr);
        ma_data_source_uninit(&m_base);
    }

    s_ma_snd_data_source::s_ma_snd_data_source(const std::filesystem::path &path)
        : m_sndfile(path.string())
    {
        // Throw if file failed to open (e.g., file not found or unsupported format).
        if (not m_sndfile or sf_error(m_sndfile.rawHandle()) != SF_ERR_NO_ERROR)
        {
            throw std::runtime_error("Failed to open audio file: " + path.string() + ". Error: " + sf_strerror(nullptr));
        }

        // Initialize base with our vtable of custom functions.
        ma_data_source_config base_config = ma_data_source_config_init();
        base_config.vtable = &s_table;
        ma_data_source_init(&base_config, &m_base);

        // Initialize resampler
        ma_linear_resampler_config resampler_config = ma_linear_resampler_config_init(ma_format_f32, static_cast<ma_uint32>(m_sndfile.channels()), static_cast<ma_uint32>(m_sndfile.samplerate()), m_output_sample_rate);
        ma_linear_resampler_init(&resampler_config, nullptr, &m_resampler);

        // Initialize channel converter
        ma_channel_converter_config channel_config = ma_channel_converter_config_init(ma_format_f32, static_cast<ma_uint32>(m_sndfile.channels()), nullptr, m_output_channels, nullptr, ma_channel_mix_mode_default);
        ma_channel_converter_init(&channel_config, nullptr, &m_channel_converter);

        m_length = static_cast<ma_uint64>(m_sndfile.frames()) * m_output_sample_rate / static_cast<ma_uint64>(m_sndfile.samplerate());
    }
} // namespace

namespace music
{
    c_track::c_track(int track_id, const std::filesystem::path &path)
        : m_track_id(track_id), m_snd_data_source(path), m_filename(path.filename().string())
    {
    }

    c_track::c_track(c_track &&other) noexcept
        : m_track_id(other.m_track_id),
          m_snd_data_source(std::move(other.m_snd_data_source)),
          m_filename(std::move(other.m_filename)),
          m_is_playing(other.m_is_playing),
          m_is_looping(other.m_is_looping)
    {
        other.m_track_id = 0;
        other.m_is_playing = false;
        other.m_is_looping = false;
    }

    auto c_track::operator=(c_track &&other) noexcept -> c_track &
    {
        if (this != &other)
        {
            m_track_id = other.m_track_id;
            m_snd_data_source = std::move(other.m_snd_data_source);
            m_filename = std::move(other.m_filename);
            m_is_playing = other.m_is_playing;
            m_is_looping = other.m_is_looping;

            other.m_track_id = 0;
            other.m_is_playing = false;
            other.m_is_looping = false;
        }
        return *this;
    }

    auto c_track::get_track_id() const -> int
    {
        return m_track_id;
    }

    auto c_track::play() -> void
    {
        m_is_playing = true;
    }

    auto c_track::pause() -> void
    {
        m_is_playing = false;
    }

    auto c_track::is_playing() const -> bool
    {
        return m_is_playing;
    }

    auto c_track::seek(std::uint64_t frame_index) -> void
    {
        ma_data_source_seek_to_pcm_frame(&m_snd_data_source, frame_index);
    }

    auto c_track::get_cursor_frame() -> std::uint64_t
    {
        ma_uint64 cursor = 0;
        ma_data_source_get_cursor_in_pcm_frames(&m_snd_data_source, &cursor);
        return cursor;
    }

    auto c_track::set_looping(bool is_looping) -> void
    {
        m_is_looping = is_looping;
        ma_data_source_set_looping(&m_snd_data_source, is_looping ? MA_TRUE : MA_FALSE);
    }

    auto c_track::is_looping() const -> bool
    {
        return m_is_looping;
    }

    auto c_track::get_total_frames() -> std::uint64_t
    {
        ma_uint64 length = 0;
        ma_data_source_get_length_in_pcm_frames(&m_snd_data_source, &length);
        return length;
    }

    auto c_track::get_filename() const -> std::string
    {
        return m_filename;
    }

    auto c_track::data_ptr() -> ma_data_source *
    {
        return reinterpret_cast<ma_data_source *>(&m_snd_data_source);
    }
} // namespace music
