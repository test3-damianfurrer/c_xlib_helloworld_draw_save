all: cat.xpm
	gcc -o demo demo.c -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -lXpm && ./demo
cat.xpm:
	convert cat.png cat.xpm
