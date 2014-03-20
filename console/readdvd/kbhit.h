// KBHIT.H

#ifndef KBHIT_H
#define KBHIT_H

#if defined (__unix) || defined (__unix__)
#include <termios.h>
#endif

class keyboard
{
public:

      keyboard();
    ~keyboard();
    int kb_hit();
    int kb_getch();

private:

#if defined (__unix) || defined (__unix__)
    struct termios initial_settings, new_settings;
#endif
    int peek_character;

};

#endif

