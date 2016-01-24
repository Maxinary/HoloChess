#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <SDL\SDL.h>
#include <string>
#include <SDL\SDL_image.h>
#undef main

SDL_Window* mainWindow;
SDL_Renderer* boardR;

int distance(int x1, int y1, int x2, int y2) {
	return std::abs(std::sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}

void fatalError(std::string fatal) {
	std::cout << "A fatal error has occurred: " + fatal + "\n\nPress enter to exit";
	int a;
	std::cin >> a;
	SDL_Quit();
	exit(-1);
}

//ALGORTIHM TAKEN FROM http://content.gpwiki.org/index.php/SDL:Tutorials:Drawing_and_Filling_Circles
void draw_circle(SDL_Renderer* renderer, int n_cx, int n_cy, int radius)
{
	// if the first pixel in the screen is represented by (0,0) (which is in sdl)
	// remember that the beginning of the circle is not in the middle of the pixel
	// but to the left-top from it:

	double error = (double)-radius;
	double x = (double)radius - 0.5;
	double y = (double)0.5;
	double cx = n_cx - 0.5;
	double cy = n_cy - 0.5;

	while (x >= y){
		SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy + y));
		SDL_RenderDrawPoint(renderer, (int)(cx + y), (int)(cy + x));

		if (x != 0){
			SDL_RenderDrawPoint(renderer, (int)(cx - x), (int)(cy + y));
			SDL_RenderDrawPoint(renderer, (int)(cx + y), (int)(cy - x));
		}

		if (y != 0){
			SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy - y));
			SDL_RenderDrawPoint(renderer, (int)(cx - y), (int)(cy + x));
		}

		if (x != 0 && y != 0){
			SDL_RenderDrawPoint(renderer, (int)(cx - x), (int)(cy - y));
			SDL_RenderDrawPoint(renderer, (int)(cx - y), (int)(cy - x));
		}

		error += y;
		++y;
		error += y;

		if (error >= 0){
			--x;
			error -= x;
			error -= x;
		}
	}
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer* rend) {
	SDL_Surface* s = IMG_Load(path.c_str());
	if (s == NULL)
	{
		std::cout << SDL_GetError()<<std::endl;
		fatalError("Failed to load PNG image! ");
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(rend, s);
	SDL_FreeSurface(s);
	return t;
}

int modulo(int number, int mod) {//mod which suits my needs
	if (number >= 0) {
		return number%mod;
	}
	else {
		return number+mod*(1+(int) number/mod);
	}
}

int main(int argc, char* argv) {
	//SETUP
	
	//SDL initialization
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fatalError("Could not init SDL");
	}

	//window creation
	int size = 501;
	mainWindow = SDL_CreateWindow("Memes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size, size, SDL_WINDOW_SHOWN);
	if (mainWindow == NULL) {
		fatalError("window was not initialized");
	}
	boardR = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	
	//textures
	SDL_Texture* player = loadTexture("player.png",boardR);
	SDL_Texture* king = loadTexture("king.png", boardR);

	//board setup
	int centerRadius = size/8;
	int secondRadius = centerRadius + size*3/16;
	int firstRadius = secondRadius + size*3/16;
	int board[3][12];
	int center;
	
	board[0][0] = 1;
	board[0][1] = 1;
	board[0][2] = 1;
	board[0][3] = 2;
	board[0][4] = 1;
	board[0][5] = 1;

	board[0][6] = -1;
	board[0][7] = -1;
	board[0][8] = -1;
	board[0][9] = -2;
	board[0][10] = -1;
	board[0][11] = -1;

	for (int j = 1; j < 3; j++) {
		for (int i = 0; i < 12; i++) {
			board[j][i] = 0;
		}
	}
	

	Uint8 playerColors[2][3];
	playerColors[0][0] = 255;
	playerColors[0][1] = 0;
	playerColors[0][2] = 0;

	playerColors[1][0] = 0;
	playerColors[1][1] = 0;
	playerColors[1][2] = 255;

	SDL_Event e;
	bool change = true;
	SDL_Rect playerRect;
	playerRect.h = size / 16;
	playerRect.w = size / 16;

	int playerPos[] = { (firstRadius + secondRadius) / 2 , (secondRadius + centerRadius) / 2 , 0};
	int selected[2] = { -1, 0 };
	float turn = 1;//-1 if blue, 1 if red
	int previous[2];
	int oldBoard[3][12];
	int win = 0;
	bool exit = false;
	while(!exit) {
		if (change) {//draw stuff
			change = false;
			//clear board
			SDL_SetRenderDrawColor(boardR, 0, 0, 0, 255);
			SDL_RenderClear(boardR);
			SDL_SetRenderDrawColor(boardR, 255, 255, 255, 255);
			
			//draw outline
			SDL_SetRenderDrawColor(boardR, 255, 255, 255, 255);
			draw_circle(boardR, size / 2, size / 2, centerRadius);
			draw_circle(boardR, size / 2, size / 2, firstRadius);
			draw_circle(boardR, size / 2, size / 2, secondRadius);
			for (int i = 0; i < 12; i++) {
				SDL_RenderDrawLine(boardR,
					size / 2 + centerRadius*cos(M_PI*i / 6),
					size / 2 + centerRadius*sin(M_PI*i / 6),
					size / 2 + firstRadius*cos(M_PI*i / 6),
					size / 2 + firstRadius*sin(M_PI*i / 6)
					);
			}

			//draw background
			if (win == 0) {
				if (turn == -1) {
					SDL_SetRenderDrawColor(boardR, 0, 0, 25, 255);
				}
				else if (turn == 1) {
					SDL_SetRenderDrawColor(boardR, 25, 0, 0, 255);
				}
			}
			else {
				exit = true;
				if (win == 1) {
					SDL_SetRenderDrawColor(boardR, 255, 0, 0, 255);
				}
				else if (win == -1) {
					SDL_SetRenderDrawColor(boardR, 0, 0, 255, 255);
				}
				else if (win == 2) {
					SDL_SetRenderDrawColor(boardR, 150, 150, 150, 255);
				}
			}
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					if (distance(size / 2, size / 2, i, j)>firstRadius) {
						SDL_RenderDrawPoint(boardR, i, j);
					}
				}
			}

			SDL_SetRenderDrawColor(boardR, 90, 90, 90, 255);

			//draw selected sector
			{
				double angle;
				double dist;
				if (selected[0] == 0) {
					for (int i = 1; i < size; i++) {
						for (int j = 1; j < size; j++) {
							angle = atan2(size / 2 - j, size / 2 - i);
							dist = distance(size/2,size/2,j,i);
							if (
								(angle < M_PI*(selected[1] + 1) / 6 - M_PI && angle > M_PI*selected[1] / 6 - M_PI)
								&&
								(dist<firstRadius && dist>secondRadius)//distance correct
								) {
								SDL_RenderDrawPoint(boardR, i, j);
							}
						}
					}
				}
				else if (selected[0] == 1) {
					for (int i = 1; i < size; i++) {
						for (int j = 1; j < size; j++) {
							angle = atan2(size / 2 - j, size / 2 - i);
							dist = distance(size / 2, size / 2, j, i);
							if (
								(angle < M_PI*(selected[1] + 1) / 6 - M_PI && angle > M_PI*selected[1] / 6 - M_PI)
								&&
								(dist<secondRadius && dist>centerRadius)//distance correct
								) {
								SDL_RenderDrawPoint(boardR, i, j);
							}
						}
					}
				}
				else if (selected[0] == 2) {
					for (int i = 1; i < size; i++) {
						for (int j = 1; j < size; j++) {
							dist = distance(size / 2, size / 2, j, i);
							if (dist<centerRadius) {
								SDL_RenderDrawPoint(boardR, i, j);
							}
						}
					}
				}
			}
			//draw players
			for (int j = 0; j < 3;j++){	
				for (int i = 0; i < sizeof(board[0]) / sizeof(int); i++) {
					if (board[j][i] != 0) {
						playerRect.x = size / 2 - playerRect.w / 2 + playerPos[j]*cos(M_PI*i / 6 + M_PI / 12);
						playerRect.y = size / 2 - playerRect.h / 2 + playerPos[j] *sin(M_PI*i / 6 + M_PI / 12);
						if (board[j][i] == 1) {
							SDL_SetTextureColorMod(player, playerColors[0][0], playerColors[0][1], playerColors[0][2]);
							SDL_RenderCopy(boardR, player, NULL, &playerRect);
						}
						else if (board[j][i] == -1) {
							SDL_SetTextureColorMod(player, playerColors[1][0], playerColors[1][1], playerColors[1][2]);
							SDL_RenderCopy(boardR, player, NULL, &playerRect);
						}
						else if (board[j][i] == 2) {
							SDL_SetTextureColorMod(king, playerColors[0][0], playerColors[0][1], playerColors[0][2]);
							SDL_RenderCopy(boardR, king, NULL, &playerRect);
						}
						else if (board[j][i] == -2) {
							SDL_SetTextureColorMod(king, playerColors[1][0], playerColors[1][1], playerColors[1][2]);
							SDL_RenderCopy(boardR, king, NULL, &playerRect);
						}
					}
				}
			}

			//render board
			SDL_RenderPresent(boardR);
			if (win != 0) {
				SDL_Delay(2000);
			}
		}

		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				exit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				change = true;
				//transfer
				for (int i = 0; i < sizeof(selected)/sizeof(int); i++) {
					previous[i] = selected[i];
				}

				//set new position
				selected[1] = atan2(size / 2 - e.motion.y, size / 2 - e.motion.x) * 6 / M_PI + 6;
				int dist = distance(size / 2, size / 2, e.motion.x, e.motion.y);
				if (dist < centerRadius) {
					selected[0] = 2;
					selected[1] = 0;
				}
				else if (dist < secondRadius) {
					selected[0] = 1;
				}
				else if (dist < firstRadius) {
					selected[0] = 0;
				}
				
				
				//movement
				std::cout << board[selected[0]][selected[1]]<<std::endl;
				if (selected[0] != -1 && previous[0] != -1 && board[selected[0]][selected[1]] == 0 && (//must be able to move
					((selected[0] == previous[0] && ((abs(selected[1] - previous[1]) == 1) || abs(selected[1] - previous[1]) == 11))) ||//horizontal movement
					(abs(selected[0] - previous[0]) == 1 && selected[1] == previous[1] || ((selected[0] == 2) != (previous[0] == 2)))//vertical
					)
					) {
						board[selected[0]][selected[1]] = board[previous[0]][previous[1]];
						board[previous[0]][previous[1]] = 0;

						//capturing

						//copy old board
						for (int i = 0; i < 3; i++) {
							for (int j = 0; j < 12; j++) {
								oldBoard[i][j] = board[i][j];
							}
						}

						//horizontal capture
						for (int i = 0; i < 2; i++) {
							for (int j = 0; j < 12; j++) {
								if (oldBoard[i][(j + 2) % 12] != 0 && (((double)oldBoard[i][j]) / oldBoard[i][(j + 2) % 12] > 0 && ((double)oldBoard[i][(j + 1) % 12]) / oldBoard[i][(j + 2) % 12] < 0)) {//capture between
									if (abs(oldBoard[i][(j + 1) % 12]) == 2) {
										if (win != 0 && win != oldBoard[i][(j + 1) % 12] / (-2)) {
											win = 2;
										}
										else {
											win = oldBoard[i][(j + 1) % 12] / (-2);
										}
									}
									board[i][(j + 1) % 12] = 0;
								}
							}
						}

						//vertical capture

						for (int i = 0; i < 6; i++) {
							if (oldBoard[1][(i + 6) % 12] && (((double)oldBoard[1][i]) / oldBoard[1][(i + 6) % 12] > 0 && ((double)oldBoard[2][0]) / oldBoard[1][(i + 6) % 12] < 0)) {
								//capturing the center
								if (abs(oldBoard[2][0]) == 2) {
									if (win != 0 && win != oldBoard[2][0] / (-2)) {
										win = 2;
									}
									else {
										win = oldBoard[2][0] / (-2);
									}
								}
								board[2][0] = 0;
							}
						}

						for (int i = 0; i < 12; i++) {
							//capturing the second orbit
							if (oldBoard[2][0] != 0 && (((double)oldBoard[0][i]) / oldBoard[2][0] > 0 && ((double)oldBoard[1][i])/oldBoard[2][0] < 0)) {
								if (abs(oldBoard[1][i]) == 2) {
									if (win != 0 && win != oldBoard[1][i] / (-2)) {
										win = 2;
									}
									else {
										win = oldBoard[1][i] / (-2);
									}
								}
								board[1][i] = 0;
							}
						}

						turn *= -1;
						selected[0] = -1;
				}
				else {
					//final positioning
					if (board[selected[0]][selected[1]] / turn <= 0) {//make sure you click the correct color
						selected[0] = -1;
					}
				}
				break;
			}
		}
	}
	SDL_Quit();
	return 1;
}