all: 
	gcc main.c -o main -Wformat=0 && sudo ./main

rm:
	rm main