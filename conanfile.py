from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class AudioVisualiser(ConanFile):
    name = "AudioVisualiser"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("glew/2.2.0")
        self.requires("glfw/3.4")

    def layout(self):
        cmake_layout(self)
        self.folders.build = "build"
        self.folders.generators = "conan/debug" if self.settings.build_type == "Debug" else "conan/release"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def test(self):
        cmake = CMake(self)
        cmake.test()