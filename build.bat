@echo off
gcc funkeynator.c -o funkeynator. && (xxd -i funkeynator > funkeynator.h) && gcc funkey.c -o funkey && (del funkeynator & del funkeynator.h)