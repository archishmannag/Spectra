#include <GL/glew.h>
#include <catch2/catch_test_macros.hpp>

import opengl;

TEST_CASE("BufferLayout: Construction and initialization", "[opengl][buffer_layout][unit]")
{
    SECTION("Default construction creates empty layout")
    {
        opengl::c_buffer_layout layout;
        REQUIRE(layout.get_stride() == 0);
        REQUIRE(layout.get_elements().empty());
    }
}

TEST_CASE("BufferLayout: Push float elements", "[opengl][buffer_layout][unit]")
{
    SECTION("Push single float")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(1);

        REQUIRE(layout.get_stride() == sizeof(float));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_FLOAT);
        REQUIRE(element.count == 1);
        REQUIRE(element.normalized == GL_FALSE);
    }

    SECTION("Push multiple floats")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(3);

        REQUIRE(layout.get_stride() == 3 * sizeof(float));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_FLOAT);
        REQUIRE(element.count == 3);
        REQUIRE(element.normalized == GL_FALSE);
    }

    SECTION("Push multiple float elements sequentially")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(2);
        layout.push<float>(3);

        REQUIRE(layout.get_stride() == 5 * sizeof(float));
        REQUIRE(layout.get_elements().size() == 2);

        REQUIRE(layout.get_elements()[0].count == 2);
        REQUIRE(layout.get_elements()[1].count == 3);
    }
}

TEST_CASE("BufferLayout: Push unsigned int elements", "[opengl][buffer_layout][unit]")
{
    SECTION("Push single unsigned int")
    {
        opengl::c_buffer_layout layout;
        layout.push<unsigned int>(1);

        REQUIRE(layout.get_stride() == sizeof(unsigned int));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_UNSIGNED_INT);
        REQUIRE(element.count == 1);
        REQUIRE(element.normalized == GL_FALSE);
    }

    SECTION("Push multiple unsigned ints")
    {
        opengl::c_buffer_layout layout;
        layout.push<unsigned int>(4);

        REQUIRE(layout.get_stride() == 4 * sizeof(unsigned int));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_UNSIGNED_INT);
        REQUIRE(element.count == 4);
    }
}

TEST_CASE("BufferLayout: Push unsigned char elements", "[opengl][buffer_layout][unit]")
{
    SECTION("Push single unsigned char")
    {
        opengl::c_buffer_layout layout;
        layout.push<unsigned char>(1);

        REQUIRE(layout.get_stride() == sizeof(unsigned char));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_UNSIGNED_BYTE);
        REQUIRE(element.count == 1);
        REQUIRE(element.normalized == GL_TRUE); // unsigned char is normalized
    }

    SECTION("Push multiple unsigned chars")
    {
        opengl::c_buffer_layout layout;
        layout.push<unsigned char>(4);

        REQUIRE(layout.get_stride() == 4 * sizeof(unsigned char));
        REQUIRE(layout.get_elements().size() == 1);

        const auto &element = layout.get_elements()[0];
        REQUIRE(element.type == GL_UNSIGNED_BYTE);
        REQUIRE(element.count == 4);
        REQUIRE(element.normalized == GL_TRUE);
    }
}

TEST_CASE("BufferLayout: Mixed element types", "[opengl][buffer_layout][unit]")
{
    SECTION("Float, unsigned int, and unsigned char combination")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(3);         // position (x, y, z)
        layout.push<float>(2);         // texture coords (u, v)
        layout.push<unsigned char>(4); // color (r, g, b, a)

        unsigned int expected_stride = 3 * sizeof(float) + 2 * sizeof(float) + 4 * sizeof(unsigned char);

        REQUIRE(layout.get_stride() == expected_stride);
        REQUIRE(layout.get_elements().size() == 3);

        REQUIRE(layout.get_elements()[0].type == GL_FLOAT);
        REQUIRE(layout.get_elements()[0].count == 3);

        REQUIRE(layout.get_elements()[1].type == GL_FLOAT);
        REQUIRE(layout.get_elements()[1].count == 2);

        REQUIRE(layout.get_elements()[2].type == GL_UNSIGNED_BYTE);
        REQUIRE(layout.get_elements()[2].count == 4);
    }

    SECTION("Complex vertex layout")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(3);         // position
        layout.push<float>(3);         // normal
        layout.push<float>(2);         // UV
        layout.push<unsigned int>(1);  // texture ID
        layout.push<unsigned char>(4); // color

        REQUIRE(layout.get_elements().size() == 5);

        unsigned int expected_stride = 3 * sizeof(float) +        // position
                                       3 * sizeof(float) +        // normal
                                       2 * sizeof(float) +        // UV
                                       1 * sizeof(unsigned int) + // texture ID
                                       4 * sizeof(unsigned char); // color

        REQUIRE(layout.get_stride() == expected_stride);
    }
}

TEST_CASE("BufferLayout: Stride calculations", "[opengl][buffer_layout][unit]")
{
    SECTION("Empty layout has zero stride")
    {
        opengl::c_buffer_layout layout;
        REQUIRE(layout.get_stride() == 0);
    }

    SECTION("Single element stride")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(1);
        REQUIRE(layout.get_stride() == sizeof(float));
    }

    SECTION("Cumulative stride calculation")
    {
        opengl::c_buffer_layout layout;
        REQUIRE(layout.get_stride() == 0);

        layout.push<float>(2);
        REQUIRE(layout.get_stride() == 2 * sizeof(float));

        layout.push<unsigned int>(1);
        REQUIRE(layout.get_stride() == 2 * sizeof(float) + sizeof(unsigned int));

        layout.push<unsigned char>(4);
        REQUIRE(layout.get_stride() == 2 * sizeof(float) + sizeof(unsigned int) + 4 * sizeof(unsigned char));
    }
}

TEST_CASE("BufferLayout: Element properties", "[opengl][buffer_layout][unit]")
{
    SECTION("Element order is preserved")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(1);
        layout.push<unsigned int>(2);
        layout.push<unsigned char>(3);
        layout.push<float>(4);

        const auto &elements = layout.get_elements();
        REQUIRE(elements.size() == 4);

        REQUIRE(elements[0].type == GL_FLOAT);
        REQUIRE(elements[0].count == 1);

        REQUIRE(elements[1].type == GL_UNSIGNED_INT);
        REQUIRE(elements[1].count == 2);

        REQUIRE(elements[2].type == GL_UNSIGNED_BYTE);
        REQUIRE(elements[2].count == 3);

        REQUIRE(elements[3].type == GL_FLOAT);
        REQUIRE(elements[3].count == 4);
    }

    SECTION("Normalization flags are correct")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(1);
        layout.push<unsigned int>(1);
        layout.push<unsigned char>(1);

        const auto &elements = layout.get_elements();

        // Float and unsigned int should not be normalized
        REQUIRE(elements[0].normalized == GL_FALSE);
        REQUIRE(elements[1].normalized == GL_FALSE);

        // Unsigned char should be normalized
        REQUIRE(elements[2].normalized == GL_TRUE);
    }
}

TEST_CASE("BufferLayout: Type size helper", "[opengl][buffer_layout][unit]")
{
    SECTION("Get size of GL_FLOAT")
    {
        auto size = opengl::s_vertex_buffer_element::get_size_of_type(GL_FLOAT);
        REQUIRE(size == sizeof(GLfloat));
    }

    SECTION("Get size of GL_UNSIGNED_INT")
    {
        auto size = opengl::s_vertex_buffer_element::get_size_of_type(GL_UNSIGNED_INT);
        REQUIRE(size == sizeof(GLuint));
    }

    SECTION("Get size of GL_UNSIGNED_BYTE")
    {
        auto size = opengl::s_vertex_buffer_element::get_size_of_type(GL_UNSIGNED_BYTE);
        REQUIRE(size == sizeof(GLubyte));
    }

    SECTION("Get size of invalid type returns 0")
    {
        auto size = opengl::s_vertex_buffer_element::get_size_of_type(0xFFFF);
        REQUIRE(size == 0);
    }
}

TEST_CASE("BufferLayout: Typical usage patterns", "[opengl][buffer_layout][unit]")
{
    SECTION("Simple 2D position and color layout")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(2);         // x, y
        layout.push<unsigned char>(4); // r, g, b, a

        REQUIRE(layout.get_elements().size() == 2);
        REQUIRE(layout.get_stride() == 2 * sizeof(float) + 4 * sizeof(unsigned char));
    }

    SECTION("3D textured model layout")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(3); // position (x, y, z)
        layout.push<float>(3); // normal (nx, ny, nz)
        layout.push<float>(2); // texture coords (u, v)

        REQUIRE(layout.get_elements().size() == 3);
        REQUIRE(layout.get_stride() == 8 * sizeof(float));
    }

    SECTION("Instanced rendering layout")
    {
        opengl::c_buffer_layout layout;
        layout.push<float>(3);  // position
        layout.push<float>(4);  // color
        layout.push<float>(16); // model matrix (4x4)

        REQUIRE(layout.get_elements().size() == 3);
        REQUIRE(layout.get_stride() == 23 * sizeof(float));
    }
}
