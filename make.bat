del *.gb
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -c -o main.o main.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o alienship_attack.gb main.o
del *.o
del *.map
del *.sym
del *.lst
del *.sav