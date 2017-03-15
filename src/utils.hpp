/** \file utils.hpp
 *  \brief Utility functions and global variables.
 */

#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include <iostream>
#include <string>

/** \name _ANSI_COLORS_
 *  \brief ANSI codes for styled terminal output.
 */
//@{
const std::string _RED_ = "\033[31m";
const std::string _GRN_ = "\033[32m";
const std::string _YEL_ = "\033[33m";
const std::string _BLU_ = "\033[34m";
const std::string _END_ = "\033[0m";
//@}

/** \brief Function for prompting user input and reading it to desired string.
 *  \param os Reference to std::ostream (where the prompt message is written to).
 *  \param is Reference to std::istream (where user input is read from).
 *  \param prompt String to write to os.
 *  \param buffer Target for input.
 *  \return Input stream reference (parameter is).
 */
std::istream& get_user_input(
    const std::ostream& os, std::istream& is,
    const std::string& prompt, std::string& buffer);

#endif // UTILS_HPP_INCLUDED
