#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include "funkeynator.h"

#define CHAR(a) (a)[0]

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
	#include <io.h>
	#include <fcntl.h>

	DWORD dwOriginalMode=0;
	HANDLE hOut;
	volatile bool ctrlC_Pressed = false;
	void toggleAnsi(bool enabld) {
		if (enabld) {
			hOut=GetStdHandle(-11);
			if(hOut==(HANDLE)-1)return;
			if(!(GetConsoleMode(hOut,&dwOriginalMode)))return;
			DWORD new=dwOriginalMode|4;
			if((dwOriginalMode!=new)&&(!(SetConsoleMode(hOut,new))))return;
			return;
		}
		SetConsoleMode(hOut,dwOriginalMode);
	}

	void getTermSize(int*w,int*h) {
		if ((hOut==NULL)||(hOut==(HANDLE)-1)) {
			hOut = GetStdHandle(-11);
		}
		CONSOLE_SCREEN_BUFFER_INFO c;
		int columns, rows;
		if (GetConsoleScreenBufferInfo(hOut, &c)) {
			*w = c.srWindow.Right-c.srWindow.Left+1;
			*h = c.srWindow.Bottom-c.srWindow.Top+1;
			return;
		}
		*w=0;
		*h=0;
	}

	BOOL WINAPI __ctrlC_Handler(DWORD e) {
		ctrlC_Pressed=(e==CTRL_C_EVENT);
		return(ctrlC_Pressed);
	}

	void setupCtrlC() {
		SetConsoleCtrlHandler(__ctrlC_Handler, TRUE);
	}
#else
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <signal.h>
	#include <termios.h>

	#define _isatty isatty
	#define _fileno fileno

	volatile sig_atomic_t ctrlC_Pressed = 0;

	void toggleAnsi(bool h){}

	void getTermSize(int*w,int*h) {
		struct winsize ws;
		if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1){*w=0;*h=0;return;}
		*w=ws.ws_col;
		*h=ws.ws_row;
	}

	void __ctrlC_Handler(int e) {
		ctrlC_Pressed=(e==SIGINT);
		// return(ctrlC_Pressed);
	}

	void setupCtrlC() {
		int count = 1;
		while (count < NSIG) {
			signal(count, __ctrlC_Handler);
			count++;
		}
	}

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

void switchBufs(int b) {
	switch (b) {
		case 1:
			printf("\x1b[2J\x1b[H\x1b[?1049l");
			break;
		case 2:
			printf("\x1b[?1049h\x1b[2J\x1b[H");
			break;
		default: break;
	}
}

void funkeyScreen(int o) {
	int w,h,count,len,pCh;
	getTermSize(&w,&h);
	len=(w+1)*h;
	o%=7;
	count=0;
	pCh=0;
	char*buf=malloc(len);
	char toP[]="funkey funkey"; // i removed the extra space at the end and chatgpt now fully agrees with me on the action.
	while (count < len - 1) {
		if((count!=0)&&((count%(w+1))==w)){buf[count++]=(char)0x0a;continue;}
		buf[count++]=toP[(pCh++%7)+o];
	}
	buf[len - 1] = (char)0;
	printf("\x1b[0;0H%s\x1b[76C", buf); // make sure cursor is moved all the way to the end hehe
	free(buf);
}

int main() {
	if (!(_isatty(_fileno(stdout)))) {
		#ifdef _WIN32
			_setmode(_fileno(stdout), _O_BINARY); // stupid windows
		#endif
		fwrite(funkeynator,1,funkeynator_len,stdout);
		return 0; // hehe lol
	}
	toggleAnsi(true);
	setupCtrlC();
	switchBufs(2);
	int start = time(NULL);
	int last = -1;
	bool run = true;
	while (run) {
		int t = 6-((time(NULL)-start)%7);
		if(t!=last){funkeyScreen(t);last=t;}
		if (_kbhit()) {
			switch (_getch()) {
				// case 3: // ^C
				case 24: // ^X
					run = false;
					break;
				default: break; // idc
			}
		}
		run=(run)&&(!(ctrlC_Pressed)); // method 2 of capturing exits tehe
	}
	switchBufs(1);
	toggleAnsi(false);
	return 0;
}