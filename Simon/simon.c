#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "ansi_escapes.h"
#include "windowsConsoleInteraction.h"

#include "bass.h"
#include <conio.h>
#pragma comment(lib, "bass.lib")

DWORD green_button, red_button, yellow_button, blue_button, wrong;

extern EVENT eventMain(VOID);

extern void setupConsole(void);
extern void restoreConsoleMode(void);
extern void restoreConsole(void);
extern void getCursorPosition(int* row, int* col);

typedef unsigned char sshort;

typedef enum ColorButtons {
	NO_COLOR = 0,
	GREEN = 1,
	RED,
	YELLOW,
	BLUE
}Color;

void delay(int milliseconds)
{
	long pause;
	clock_t now, then;

	pause = milliseconds * (CLOCKS_PER_SEC / 1000);
	now = then = clock();
	while ((now - then) < pause)
		now = clock();
}

void display(int button, int screenSize[2])
{
	setupConsole();

	moveTo(0, 0);

	for (int row = 0; row <= (screenSize[0] / 2) - 2; row++)
	{
		if (button == GREEN)
			setBackgroundColorRGB(0, 150, 0); // bright green
		else
			setBackgroundColorRGB(15, 60, 15); // shadow green
		for (int greenCol = 0; greenCol <= (screenSize[1] / 2) - 1; greenCol++)
			printf(" ");

		if (button == RED)
			setBackgroundColorRGB(150, 30, 40); // bright red
		else
			setBackgroundColorRGB(70, 30, 30); // shadow red
		for (int rednCol = 0; rednCol <= (screenSize[1] / 2) - 1; rednCol++)
			printf(" ");

		printf("\n");

	}

	if (screenSize[0] % 2 != 0)
		printf("\n");

	for (int row = 0; row <= (screenSize[0] / 2) - 2; row++)
	{
		if (button == YELLOW)
			setBackgroundColorRGB(255, 255, 0); // bright yellow
		else
			setBackgroundColorRGB(50, 50, 0); // shadow yellow
		for (int greenCol = 0; greenCol <= (screenSize[1] / 2) - 1; greenCol++)
			printf(" ");


		if (screenSize[1] % 2 != 0)
			printf(" ");

		if (button == BLUE)
			setBackgroundColorRGB(40, 30, 150); // bright blue
		else
			setBackgroundColorRGB(15, 15, 60); // shadow blue
		for (int rednCol = 0; rednCol <= (screenSize[1] / 2) - 1; rednCol++)
			printf(" ");
	}
	restoreConsole();
	printf("\n");
}

int coordinatesToButton(COORD click, int windowSize[2])
{
	if (click.Y < windowSize[0] / 2)
	{
		if (click.X < windowSize[1] / 2)
			return GREEN;
		if (click.X > windowSize[1] / 2)
			return RED;
	}

	if (click.Y > windowSize[0] / 2)
	{
		if (click.X < windowSize[1] / 2)
			return YELLOW;
		if (click.X > windowSize[1] / 2)
			return BLUE;
	}
	return 0;
}

void rng(sshort* sequence, int level)
{
	sequence[level - 1] = 1 + rand() % 4;
	sequence[level] = 0;
}

void playSFX(int button)
{
	switch (button)
	{
	case 1:
		BASS_ChannelPlay(green_button, FALSE);
		break;
	case 2:
		BASS_ChannelPlay(red_button, FALSE);
		break;
	case 3:
		BASS_ChannelPlay(yellow_button, FALSE);
		break;
	case 4:
		BASS_ChannelPlay(blue_button, FALSE);
		break;
	default:
		break;
	}
}

// display error messages
void Error(const char* text)
{
	printf("Error(%d): %s\n", BASS_ErrorGetCode(), text);
	BASS_Free();
	exit(0);
}

int main(int argc, char** argv)
{

	int level = 1;
	int windowSize[2];
	sshort sequence[4000];
	sshort input[4000];
	COORD mouseCoord;
	EVENT retEvent;
	char trashcan[10];
	bool mistake = false;

	srand(time(NULL));
	
	int device = -1;

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		printf("An incorrect version of BASS was loaded");
		return;
	}

	if (!BASS_Init(device, 44100, 0, 0, NULL))
		Error("Can't initialize device");

	// try streaming the file/url

	green_button = BASS_StreamCreateFile(FALSE, "green_button.wav", 0, 0, 0);
	red_button = BASS_StreamCreateFile(FALSE, "red_button.wav", 0, 0, 0);
	yellow_button = BASS_StreamCreateFile(FALSE, "yellow_button.wav", 0, 0, 0);
	blue_button = BASS_StreamCreateFile(FALSE, "blue_button.wav", 0, 0, 0);
	wrong = BASS_StreamCreateFile(FALSE, "wrong.wav", 0, 0, 0);

	//BASS_Free();

	printf("Starting Simon. Do you wish to:\n(P) play now\nor \n(T)see the tutorial?\n");
	fgets(trashcan, 5, stdin);

	if (trashcan[0] == 'T' || trashcan[0] == 't')
	{
		//teach how to play
	}

	if (trashcan[0] == 'P' || trashcan[0] == 'p')
	{
		for (bool replay = true; replay == true;)
		{
			level = 1;
			while (mistake == false)
			{
				setupConsole();
				moveTo(999, 999);
				getCursorPosition(&windowSize[0], &windowSize[1]);
				clearScreenToTop();
				restoreConsoleMode();
				display(NO_COLOR, windowSize);

				rng(sequence, level);

				for (int count = 0; count < level; count++)
				{
					delay(500);
					display(sequence[count], windowSize);
					playSFX(sequence[count]);
					delay(510);
					display(NO_COLOR, windowSize);
				}

				for (int count = 0; count < level; count++)
				{
					display(NO_COLOR, windowSize);
					while (true)
					{
						retEvent.event.mouseEvent = 0xc00;
						retEvent = eventMain();
						mouseCoord = retEvent.event.mouseCoord;
						if (retEvent.event.mouseEvent == FROM_LEFT_1ST_BUTTON_PRESSED)
							break;
					}

					input[count] = coordinatesToButton(mouseCoord, windowSize);

					display(input[count], windowSize);
					playSFX(sequence[count]);

					if (sequence[count] != input[count])
					{
						mistake = true;
						break;
					}
				}

				if (mistake == true)
				{
					//play error
					BASS_ChannelPlay(wrong, FALSE);
					puts("Not right! Try again? (Y/N)");
					fgets(trashcan, 5, stdin);
					if (trashcan[0] == 'n' || trashcan[0] == 'N')
						replay = false;
				}
				level++;
			}
		}
	}

	return 0;
}