from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class AudioVisualiser(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("freetype/2.13.3")
        self.requires("glfw/3.4")
        self.requires("miniaudio/0.11.21")
        self.requires("libsndfile/1.2.2")
        self.requires("glew/2.2.0")
        self.requires("glm/1.0.1")
        self.requires("utfcpp/4.0.5")
        self.requires("opengl/system")
        self.requires("catch2/3.11.0")

    def configure(self):
        self.options["miniaudio/*"].header_only = False
        self.options["catch2/*"].console_width = 150

    def layout(self):
        cmake_layout(self)
        self.folders.build = "build"
        self.folders.generators = "conan/" + str(self.settings.build_type)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.build()
