#pragma once
#include <windows.h>
#include <string>


// window size in pixels
#define WINDOWSIZE_X 1000
#define WINDOWSIZE_Y 800
// game board size in tiles
#define MAX_X 64
#define MAX_Y 48
// tile size in pixels
#define TILESIZE 15
// menu event IDs
#define ID_FILE_NEWGAME 9001
#define ID_FILE_EXIT 9002
#define ID_LEVEL_1 9003


class Drawing
{
	static RECT statusArea; // status text area
	static RECT gameArea; // main game area

	static const HBRUSH clearBrush; // main window color
	static const HBRUSH backgroundBrush; // background color
	static const HBRUSH currentPlayersBrush; // main player's color
	static const HBRUSH otherPlayersBrush; // other player's color

	static const HFONT font; // font for writing status informations - scores, etc.

public:
	static void Init(HWND hWnd);
	static void SetBG(HWND hWnd);
	static void ClearScreen(HWND hWnd);
	static void DrawSquare(HWND hWnd, int xGrid, int yGrid, int choice);
	static void DrawStatus(HWND hWnd, std::string status);
};