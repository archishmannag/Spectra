add_executable(audio_visualizer_tests)

target_sources(audio_visualizer_tests
   PRIVATE
)

target_link_libraries(audio_visualizer_tests
    PRIVATE
    visualizer_lib
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(audio_visualizer_tests)
