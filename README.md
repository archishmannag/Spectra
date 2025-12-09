# Spectra

A real-time audio visualizer built with C++ and OpenGL.

![License](https://img.shields.io/badge/license-MIT-blue.svg) 
![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg) 
![OpenGL](https://img.shields.io/badge/OpenGL-4.2+-green.svg) 
![Build System](https://img.shields.io/badge/build-CMake%20%2B%20Conan-orange.svg) 

## 🎵 Features

### Current Implementation

- **Dynamic Waveform Visualization**: Real-time frequency spectrum display with logarithmic frequency scaling
- **Cross-platform Support**: Built with portable libraries (GLFW, OpenGL)

### Key Capabilities

- **Audio Formats**: Support for multiple audio formats via libsndfile
- **Interactive Controls**:
  - `Space` - Play/Pause audio
  - `Escape` - Exit application
- **Real-time Processing**: Low-latency audio analysis and visualization
- **Memory Safety**: Built with address sanitizers and proper RAII patterns

## 🚀 Installation

### Prerequisites

- **Compiler**: Clang 15+ or GCC 12+ (C++23 modules support required)
- **Build Tools**: CMake 3.28+, Ninja
- **Package Manager**: [Conan 2.0+](https://conan.io/)
- **Graphics**: OpenGL 4.2+ compatible GPU and drivers

### Dependencies

The project uses Conan for dependency management. All dependencies are automatically handled:

```python
# Core dependencies (from conanfile.py)
freetype/2.13.3      # Font rendering
glfw/3.4             # Cross-platform windowing
miniaudio/0.11.21    # Audio processing
libsndfile/1.2.2     # Audio file I/O
glew/2.2.0          # OpenGL extension loading
glm/1.0.1           # Mathematics for graphics
utfcpp/4.0.5        # UTF-8 string handling
opengl/system       # OpenGL system integration
```

### Build Instructions

1. **Clone the repository**:

   ```bash
   git clone <repository-url>
   cd audio-visualiser
   ```

2. **Install dependencies with Conan**:

   ```bash
   conan install . --build=missing -s build_type=Release
   ```

3. **Build the project**:

   ```bash
   conan build .
   ```

4. **Run the visualizer**:
   ```bash
   ./bin/AudioVisualiser
   ```

### Development Build

For development with debug symbols and sanitizers:

```bash
conan install . --build=missing -s build_type=Debug
conan build .
```

## 🎮 Usage

### Basic Controls

| Key      | Action            |
| -------- | ----------------- |
| `Space`  | Toggle play/pause |
| `Escape` | Exit application  |

### Audio Loading

Currently, the visualizer supports drag-and-drop audio file loading. Future versions will support:

- File browser integration
- Playlist management

## 🎯 Implementation Plan

### Phase 1: Core Foundation ✅

- [x] Basic audio playback system
- [x] FFT implementation for frequency analysis
- [x] OpenGL rendering pipeline
- [x] Real-time waveform visualization
- [x] Basic window management and controls
- [x] **Audio File Support**: Drag-and-drop loading of audio files

### Phase 2: Enhanced Features (Planned)

- [ ] **Multiple Visualization Modes**:
  - Spectrum analyzer
  - Oscilloscope view
  - 3D frequency landscapes
- [ ] **Interactive Controls**:
  - Volume control
  - Frequency range selection
  - Color customization
- [ ] **Performance Optimization**:
  - GPU-based FFT computation
  - Multithreaded audio processing

### Phase 3: Advanced Features (Future)

- [ ] **Real-time Effects**: Audio filtering and effects
- [ ] **Recording Capability**: Export visualizations as video
- [ ] **Plugin System**: Support for custom visualization plugins
- [ ] **MIDI Integration**: Musical instrument input support
- [ ] **VR/AR Support**: Immersive 3D visualizations

## 🤝 Contributing

We welcome contributions! Please feel free to:

1. **Report Issues**: Use GitHub issues for bug reports
2. **Submit Pull Requests**: Follow the existing code style
3. **Suggest Features**: Open discussions for new ideas
4. **Improve Documentation**: Help make the project more accessible

### Development Guidelines

- Follow C++23 best practices
- Use modules for new components
- Include appropriate error handling
- Add tests for new functionality
- Update documentation

## 🐛 Troubleshooting

### Common Issues

**Build Errors**:

- Ensure C++23 compiler support
- Verify Conan profile matches your system
- Check OpenGL driver compatibility

**Audio Issues**:

- Verify system audio device accessibility
- Check miniaudio backend compatibility
- Ensure proper permissions for audio access

**Graphics Issues**:

- Update graphics drivers
- Verify OpenGL 4.2+ support
- Check window manager compatibility

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **FFT Implementation**: Based on algorithms from [Rosetta Code](https://rosettacode.org/wiki/Fast_Fourier_transform#C++)
- **Audio Processing**: Powered by [miniaudio](https://miniaud.io/)
- **Graphics**: Built with modern OpenGL and [GLFW](https://www.glfw.org/)
- **Mathematics**: Enhanced by [GLM](https://github.com/g-truc/glm)

_Spectra - Where sound becomes sight_ 🎵✨
