// KBHIT.CPP

#include "kbhit.h"


#if defined (__unix) || defined (__unix__)
#include <unistd.h> // read()
#elif defined (_WIN32)
#include <conio.h>
#endif

keyboard::keyboard()
{
#if defined (__unix) || defined (__unix__)
	tcgetattr(0,&initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	new_settings.c_lflag &= ~ISIG;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_settings);
#endif
	peek_character=-1;
}

keyboard::~keyboard()
{
#if defined (__unix) || defined (__unix__)
    tcsetattr(0, TCSANOW, &initial_settings);
#endif
}

int keyboard::kb_hit()
{
#if defined (__unix) || defined (__unix__)
	unsigned char ch;
	int nread;
	if (peek_character != -1) return 1;
	new_settings.c_cc[VMIN]=0;
	tcsetattr(0, TCSANOW, &new_settings);
	nread = read(0,&ch,1);
	new_settings.c_cc[VMIN]=1;
	tcsetattr(0, TCSANOW, &new_settings);
	if (nread == 1) 
	{
		peek_character = ch;
        	return 1;
	}
	return 0;
#elif defined (_WIN32)
	return kbhit();
#endif
}

int keyboard::kb_getch()
{
#if defined (__unix) || defined (__unix__)
	char ch;
 	if (peek_character != -1) {
        	ch = peek_character;
        	peek_character = -1;
	} else {
		if (!read(0,&ch,1)) return 0;
	}
	return ch;
#elif defined (_WIN32)
	if (kbhit())
		return getch();
	else
		return 0;
#endif
}

