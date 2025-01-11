#pragma once

#include <iostream>
#include <stdexcept>

void format_error_to(std::ostream& stream, std::exception const& error, bool use_color = true);
