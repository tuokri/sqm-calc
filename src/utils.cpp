/** \file utils.cpp
 *  \brief utils implementation file.
 */

#include "utils.hpp"

std::istream& get_user_input(
    const std::ostream& os, std::istream& is,
    const std::string& prompt, std::string& buffer)
{
    std::cout << prompt << std::endl;
    return std::getline(is, buffer);
}
