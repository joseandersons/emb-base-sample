#ifndef DISPLAY_ANSI_H
#define DISPLAY_ANSI_H

#ifdef CONFIG_DISPLAY_ANSI_COLORS
    #define ANSI_RED    "\x1b[31m"
    #define ANSI_GREEN  "\x1b[32m"
    #define ANSI_YELLOW "\x1b[33m"
    #define ANSI_RESET  "\x1b[0m"
#else
    #define ANSI_RED    ""
    #define ANSI_GREEN  ""
    #define ANSI_YELLOW ""
    #define ANSI_RESET  ""
#endif

#ifdef CONFIG_DISPLAY_ANSI_CLEAR_ON_UPDATE
    #define ANSI_CLEAR  "\x1b[2J"
    #define ANSI_HOME   "\x1b[H"
#else
    #define ANSI_CLEAR  ""
    #define ANSI_HOME   ""
#endif

#endif
