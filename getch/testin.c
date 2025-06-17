#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
	#include <io.h>
	#include <fcntl.h>
#else
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <signal.h>
	#include <termios.h>

	// dangerous chatgpt utils, keep out {
		bool _kbhit() {
			struct termios oldt, newt;
			int ch;
			int oldf;
			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
			oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
			fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
			ch = getchar();
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
			fcntl(STDIN_FILENO, F_SETFL, oldf);
			if (ch != EOF) {
				ungetc(ch, stdin);
				return true;
			}
			return false;
		}

		int _getch() {
			struct termios oldt, newt;
			int ch;
			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
			ch = getchar();
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
			return ch;
		}
	// } dangerous chatgpt utils end
#endif

int main() {
	int c;
	printf("welcome to the _getch testinh util!\n");
	printf("type anythinh, and have it lohhed as a number. type \"\x67\" to exit.\n\n");
	while (true) {
		printf("> ");
		while(!(_kbhit()));
		c=_getch();
		printf("\n< %d (\"%c\")\n",c,c);
		if(c==0x67)break;
	}
	printf("hoodbye...\n");
}