//Quick and dirty Tetris using ASCII assets

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
using namespace std;

//strings for block assets
wstring blocks[7];

//height and width of grid
int gridHeight = 18;
int gridWidth = 12;
unsigned char *grid = nullptr;

//height and width of window
const int windowHeight = 44;
const int windowWidth = 80;
const int xOffset = 30;
const int yOffset = 2;

std::chrono::milliseconds tickInterval = 50ms;


void initBlocks()
{
	//create block assets as strings
	blocks[0].append(L"..X...X...X...X.");
	blocks[1].append(L"..X..XX...X.....");
	blocks[2].append(L".....XX..XX.....");
	blocks[3].append(L"..X..XX..X......");
	blocks[4].append(L".X...XX...X.....");
	blocks[5].append(L".X...X...XX.....");
	blocks[6].append(L"..X...X..XX.....");
}

void initGrid()
{
	//setup the grid boundaries 
	// 9 = boundary, 0 = empty space
	grid = new unsigned char[gridWidth * gridHeight];
	for (int i = 0; i < gridWidth; i++)
	{
		for (int j = 0; j < gridHeight; j++)
		{
			grid[j*gridWidth + i] = (i == 0 || i == gridWidth - 1 || j == gridHeight - 1) ? 9 : 0;
		}
	}
}

int rotateBlock(int x, int y, int r)
{
	//logic to rotate blocks in place
	int i = 0;
	switch (r % 4)
	{
	case 0: //0 degrees
		i = y * 4 + x;
		break;
		//0 1 2 3
		//4 5 6 7
		//8 9 10 11
		//12 13 14 15

	case 1://90 degrees
		i = 12 + y - (4 * x);
		break;
		//12 8 4 0
		//13 9 5 1
		//14 10 6 2
		//15 11 7 3
	case 2://180 degrees
		i = 15 - (4 * y) - x;
		break;
		//15 14 13 12
		//11 10 9 8
		//7 6 5 4
		//3 2 1 0
	case 3://270 degrees
		i = 3 - y + (4 * x);
		break;
		//3 7 11 15
		//2 6 10 14
		//1 5 9 13
		//0 4 8 12
	}
	return i;
}

bool checkBlockCollision(int block, int rotation, int x, int y)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			//get block and grid indices
			int pieceIndex = rotateBlock(i, j, rotation);
			int gridIndex = (y + j) * gridWidth + (x + i);

			if (x + i >= 0 && x + i < gridWidth)
			{
				if (y + j >= 0 && y + j < gridHeight)
				{
					if (blocks[block][pieceIndex] == L'X' && grid[gridIndex] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

int main()
{
	wchar_t *window = new wchar_t[windowWidth*windowHeight];
	for (int i = 0; i < windowWidth*windowHeight; i++)
		window[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	initBlocks();
	initGrid();

	bool isGameOver = false;
	int currentBlock = rand()%7;
	int currentRotation = 0;
	int currentX = gridWidth / 2;
	int currentY = 0;

	int fallingSpeed = 40; //ticks required per fall
	int fallTickCounter = 0;
	int blockCount = 0;
	int score = 0;

	bool forcePieceDown = false;
	bool isRotateHeld = false;
	bool keys[4];
	vector<int> lines;

	while (!isGameOver)
	{
		// Update Tick
		this_thread::sleep_for(tickInterval);
		fallTickCounter++;
		if (fallTickCounter == fallingSpeed)
		{
			forcePieceDown = true;
		}

		//Receive Input
		for (int i = 0; i < 4; i++)
		{														//R    L    D Z
			keys[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[i]))) != 0;
		}

		//Game Logic

		currentX += (keys[0] && checkBlockCollision(currentBlock, currentRotation, currentX + 1, currentY)) ? 1 : 0;
		currentX += (keys[1] && checkBlockCollision(currentBlock, currentRotation, currentX - 1, currentY)) ? -1 : 0;
		currentY += (keys[2] && checkBlockCollision(currentBlock, currentRotation, currentX, currentY + 1)) ? 1 : 0;
		
		if(keys[3])
		{ 
			currentRotation += (!isRotateHeld && checkBlockCollision(currentBlock, currentRotation + 1, currentX, currentY)) ? 1 : 0;
			isRotateHeld = true;
		}
		else
		{
			isRotateHeld = false;
		}
		

		if (forcePieceDown)
		{
			//Update Difficulty
			blockCount++;
			if (blockCount % 50 == 0)
				if (fallingSpeed >= 10)
					fallingSpeed --;
			
			if (checkBlockCollision(currentBlock, currentRotation, currentX, currentY + 1))
			{
				currentY++;
			}
			else
			{
				//lock the current block 
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						if (blocks[currentBlock][rotateBlock(i, j, currentRotation)] == L'X')
						{
							grid[(currentY + j) * gridWidth + (currentX + i)] = currentBlock + 1;
						}
					}
				}

				//check for complete lines
				for (int j = 0; j < 4; j++)
				{
					if (currentY + j < gridHeight - 1)
					{
						bool line = true;
						for (int i = 1; i < gridWidth - 1; i++)
						{
							line &= (grid[(currentY + j) * gridWidth + i]) != 0;
						}

						if (line)
						{
							//there is a line
							for (int i = 1; i < gridWidth - 1; i++)
								grid[(currentY + j) * gridWidth + i] = 8;
							lines.push_back(currentY + j);
						}
					}
				}
				score += 25;
				if (!lines.empty())
					score += (1 << lines.size()) * 100;//give them lots of points for getting multiple points!

				//choose next block
				currentBlock = rand() % 7;
				currentRotation = 0;
				currentX = gridWidth / 2;
				currentY = 0;

				//if the grid is full
				isGameOver = !checkBlockCollision(currentBlock, currentRotation, currentX, currentY);
			}
			fallTickCounter = 0;
		}

		//Draw Grid
		for (int i = 0; i < gridWidth; i++)
		{
			for (int j = 0; j < gridHeight; j++)
			{
				window[(j + yOffset) * windowWidth + (i + xOffset)] = L" ABCDEFG-%"[grid[j*gridWidth + i]];
			}
		}
		//Draw current piece
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (blocks[currentBlock][rotateBlock(i, j, currentRotation)] == L'X')
				{
					window[(currentY + j + yOffset) * windowWidth + (currentX + i + xOffset)] = currentBlock + 65;
				}
			}
		}
		if (!lines.empty())
		{
			WriteConsoleOutputCharacter(console, window, windowHeight* windowWidth, { 0,0 }, &bytesWritten);
			this_thread::sleep_for(500ms); // animate the delete a bit
			for (auto &line : lines)
			{
				for (int i = 1; i < gridWidth - 1; i++)
				{
					for (int j = line; j > 0; j--)
					{
						grid[j * gridWidth + i] = grid[(j - 1) * gridWidth + i];
					}
					grid[i] = 0;
				}
			}
			lines.clear();
		}

		swprintf_s(&window[2 * windowWidth + gridWidth], 16, L"Score: %8d", score);

		WriteConsoleOutputCharacter(console, window, windowHeight * windowWidth, { 0,0 }, &bytesWritten);
	}
	
	//Game Over
	CloseHandle(console);
	cout << "Game Over!" << endl << "Score: " << score << endl;
	system("pause");

	
}



