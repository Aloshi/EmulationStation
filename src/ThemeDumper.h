#pragma once

#include "ThemeData.h"

// Should be able to:
// 1. Take a list of "tests" to run.
//   a. Each test works by calling setTheme on a theme with a "fake" theme that records all getElement() access.
// 2. Results can be output to a text file, OR compared with a loaded theme to create warnings about potentially unused elements.

class ThemeDumper
{

};
