#ifndef COLORS_H
#define COLORS_H
#include <string>
// Ctrl[attribute;foreground;backgroundm
// \x1b 0,1,...   30+color   40+color
const std::string BLACK("\x1b[0;30;48m");
const std::string RED("\x1b[0;31;48m");
const std::string GREEN("\x1b[0;32;48m");
const std::string YELLOW("\x1b[0;33;48m");
const std::string BLUE("\x1b[0;34;48m");
const std::string MAGENTA("\x1b[0;35;48m");
const std::string CYAN("\x1b[0;36;48m");
const std::string WHITE("\x1b[0;37;48m");

const std::string BOLDBLACK("\x1b[1;30;48m");
const std::string BOLDRED("\x1b[1;31;48m");
const std::string BOLDGREEN("\x1b[1;32;48m");
const std::string BOLDYELLOW("\x1b[1;33;48m");
const std::string BOLDBLUE("\x1b[1;34;48m");
const std::string BOLDMAGENTA("\x1b[1;35;48m");
const std::string BOLDCYAN("\x1b[1;36;48m");

const std::string DEFAULT_COLOR("\x1b[0m");
const std::string HOME_CURSOR("\x1b[0;0H");
const std::string CLEAR_SCREEN("\x1b[2J");
#endif
