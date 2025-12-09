add_executable(audio_visualizer_tests)

target_sources(audio_visualizer_tests
    PRIVATE
    fft_test.cpp
    math_helpers_test.cpp
    buffer_layout_test.cpp
    notifier_test.cpp
    stress_test.cpp
    fuzz_test.cpp
)

target_link_libraries(audio_visualizer_tests
    PRIVATE
    visualizer_lib
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(audio_visualizer_tests)
