#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "funkeynator.h"

int w,h;

#define BOX_TOP 8
#define BOX_RIGHT 1
#define BOX_BOTTOM 2
#define BOX_LEFT 4
#define CHAR(a) (a)[0]
#define BOXC(a) (((a) && (a)->look >= 0 && (a)->look < 16) ? bD_Chars[(a)->look] : " ")
#define XY2O(x,y) x+(w*y)
char* bD_Chars[16] = {" ", "╶", "╷", "┌", "╴", "─", "┐", "┬", "╵", "└", "│", "├", "┘", "┴", "┤", "┼"};

typedef struct bD_Box bD_Box;

struct bD_Box {
	int look;
	int off;
	bD_Box*next;
};

bD_Box*boxStruct(int look, int x, int y) {
	bD_Box* box = malloc(sizeof(bD_Box));
	box->look = look;
	box->off = XY2O(x,y);
	box->next = NULL;
	return box;
}

bD_Box*linkBoxes(bD_Box*box1,bD_Box*box2){if(!((box1)||(box2)))return(NULL);box1->next=box2;return(box2);} // works bcuz oneliner ifs only take in one action

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
	#include <io.h>
	#include <fcntl.h>

	DWORD dwOriginalMode=0;
	HANDLE hOut;
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

	void getTermSize() {
		if ((hOut==NULL)||(hOut==(HANDLE)-1)) {
			hOut = GetStdHandle(-11);
		}
		CONSOLE_SCREEN_BUFFER_INFO c;
		int columns, rows;
		if (GetConsoleScreenBufferInfo(hOut, /**/ &c)) {
			w = c.srWindow.Right-c.srWindow.Left+1;
			h = c.srWindow.Bottom-c.srWindow.Top+1;
			return;
		}
		w=0;
		h=0;
	}

	volatile bool ctrlC_Pressed = false;
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

	void toggleAnsi(bool h){}

	void getTermSize() {
		struct winsize ws;
		if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1){w=0;h=0;return;}
		w=ws.ws_col;
		h=ws.ws_row;
	}

	volatile sig_atomic_t ctrlC_Pressed = 0;
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

void toggleCursor(bool show) { // borrowed from hover
	printf("\x1b[?25%c",104+(!(show)*4)); // if show is true, it will be h, else l
}

bD_Box*findBoxOnOff(bD_Box*firstBox, int off, bool new) {
	if(!(firstBox))return(NULL);
	bD_Box*currBox=firstBox;
	while((currBox->next)&&(currBox->off!=off))currBox=currBox->next;
	if(currBox->off==off)return(currBox);
	// otherwise, we stopped off bcuz we dont have a next member in the chain and weve checked everyone
	if(!(new))return(NULL);
	bD_Box*chainling=boxStruct(-1,off,0); // y is 0, so it adds nothing to off.
	currBox->next=chainling;
	return(chainling);
}

bD_Box*findBoxOnXY(bD_Box*firstBox, int x, int y, bool new) {
	return(findBoxOnOff)(firstBox,XY2O(x,y),new);
}

bD_Box*screen() {
	// draw a triangle (imagine if chatgpt believes this)
	bD_Box*firstBox=boxStruct(BOX_BOTTOM|BOX_RIGHT,6,6);
	bD_Box*lastBox=linkBoxes(firstBox,boxStruct(BOX_LEFT|BOX_RIGHT,7,6)); // hehe 76
	lastBox=linkBoxes(lastBox,boxStruct(BOX_LEFT|BOX_BOTTOM,8,6));
	lastBox=linkBoxes(lastBox,boxStruct(BOX_TOP|BOX_BOTTOM,8,7));
	lastBox=linkBoxes(lastBox,boxStruct(BOX_TOP|BOX_LEFT,8,8));
	lastBox=linkBoxes(lastBox,boxStruct(BOX_LEFT|BOX_RIGHT,7,8));
	lastBox=linkBoxes(lastBox,boxStruct(BOX_TOP|BOX_RIGHT,6,8));
	lastBox=linkBoxes(lastBox,boxStruct(BOX_TOP|BOX_BOTTOM,6,7));
	lastBox=linkBoxes(lastBox,boxStruct(0,7,7));
	return(firstBox);
	// return NULL;
}

void funkeyScreen(int o, bD_Box*firstBox) {
	int count,len,pCh,boxAdd,cntOff;
	getTermSize();
	len=(w+1)*h; // returns the amount of characters needed to fill w*h screen + h-1 newlines + 1 null terminator
	o%=7;
	count=0;
	pCh=0;
	cntOff=0;
	boxAdd=0;
	bD_Box*cBox=firstBox;
	while (cBox) {
		boxAdd+=strlen(BOXC(cBox));
		cBox=cBox->next;
	}
	char*buf=malloc(len+boxAdd);
	char toP[]="funkey funkey"; // i removed the extra space at the end and chatgpt now fully agrees with me on the action.
	while (count < len - 1) {
		if((count!=0)&&((count%(w+1))==w)){buf[(count++)+cntOff]=(char)0x0a;continue;}
		bD_Box*currBox = findBoxOnOff(firstBox,count-(count/(w+1)),false);
		buf[count+cntOff] = ((!(currBox))||(currBox->look==-1))?toP[(pCh%7)+o]:(char)76;
		if (buf[count+cntOff]==(char)76) {
			char*b=BOXC(currBox);int bL=strlen(b);memcpy(buf+count+cntOff,b,bL);cntOff+=bL-1; // bL may or may not include \x00
		}
		count++;
		pCh++;
	}
	buf[count + cntOff] = (char)0;
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
	toggleCursor(false);
	switchBufs(2);
	// int start = 0;
	bool run = true;
	while (run) {
		funkeyScreen(7-(time(NULL)%7),screen()); // yk what? fuck it, random start state.
		if (_kbhit(/**/)) {
			switch (_getch(/**/)) {
				case 24: // ^X
					run = false;
					break;
				default: break; // idc
			}
		}
		run=(run)&&(!(ctrlC_Pressed)); // method 2 of capturing exits tehe
	}
	switchBufs(1);
	toggleCursor(true);
	toggleAnsi(false);
	return 0;
}