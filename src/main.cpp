#include <iostream>
#include <print>

import gui;

auto main() -> int
{
    try
    {
        gui::init();
        {
            gui::c_window window(1700, 600, "Spectra");
            window.show();
        }
        gui::terminate();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::println(std::cerr, "Exception caught: {}", e.what());
        return -1;
    }
}
