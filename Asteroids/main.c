#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "./constants.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int windowWidth = defaultWindowWidth;
int windowHeight = defaultWindowHeight;

int isRunning = false;
int lastFrameTime = 0;
float deltaTime;

int difficulty = 0;

asteroid asteroids[48]; //up to 12 total starting asteroids (level 8)
boolet bullets[48];

player ship;

vector2 mousePos;

//Calculate vertex positions from relative position, global center position, and rotation
shape calcVertecies(shape vertecies, float r, vector2 center, float size) {
	shape out;

	out.numVertecies = vertecies.numVertecies;

	for (int i = 0; i < vertecies.numVertecies; i++) {
		out.vertecies[i].x = playerSize * size * cos(r) * vertecies.vertecies[i].x + playerSize * size * sin(r) * vertecies.vertecies[i].y + center.x;
		out.vertecies[i].y = playerSize * size * -sin(r) * vertecies.vertecies[i].x + playerSize * size * cos(r) * vertecies.vertecies[i].y + center.y;
	}

	return out;
}

//Setup window and it's parameters
int initializeWindow(void) {
	//Initializes SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	//Window variable setup
	window = SDL_CreateWindow(
		"Asteroids", //Window Title
		SDL_WINDOWPOS_CENTERED, //Xpos
		SDL_WINDOWPOS_CENTERED, //Ypos
		defaultWindowWidth, //Width
		defaultWindowHeight, //Height
		SDL_WINDOW_ALLOW_HIGHDPI //Flags
	);

	//Set window to fullscreen
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	//Get fullscreen size and set windowWidth/Height to it
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	windowWidth = DM.w;
	windowHeight = DM.h;

	//Checks if window initialized correctly
if (!window) {
	fprintf(stderr, "Error creating SDL Window.\n");
	return false;
}

//Renderer variable setup 
renderer = SDL_CreateRenderer(window, -1, 0);

//Checks if renderer initialized correctly
if (!renderer) {
	fprintf(stderr, "Error creating SDL Renderer.\n");
	return false;
}
}

//Sets up the scene
void setup() {
	//Player variable setup
	ship.pos.x = windowWidth / 2;
	ship.pos.y = windowHeight / 2;
	ship.vel.x = 0;
	ship.vel.y = 0;
	ship.shapeType.numVertecies = 4;
	for (int i = 0; i < 4; i++) {
		ship.shapeType.vertecies[i] = shipVertecies[i];
	}
	ship.r = 180;

	//Setup Asteroids
	for (int i = 0; i < (difficulty + 4)*4; i += 4) {
		asteroids[i].alive = true;
		asteroids[i].pos.x = rand() % windowWidth;
		asteroids[i].pos.y = rand() % windowHeight;
		asteroids[i].r = rand() % 360;
		switch (rand()%3) {
		case 0: 
			asteroids[i].shapeType.numVertecies = 11;
			for (int j = 0; j < 11; j++) {
				asteroids[i].shapeType.vertecies[j] = asteroid1Vertecies[j];
			}
			break;
		case 1:
			asteroids[i].shapeType.numVertecies = 12;
			for (int j = 0; j < 12; j++) {
				asteroids[i].shapeType.vertecies[j] = asteroid2Vertecies[j];
			}
			break;
		case 2:
			asteroids[i].shapeType.numVertecies = 12;
			for (int j = 0; j < 12; j++) {
				asteroids[i].shapeType.vertecies[j] = asteroid3Vertecies[j];
			}
			break;
		}
		asteroids[i].size = 3;
		asteroids[i].vel.x = (rand() % (2 * maxAsteroidSpeed)) - maxAsteroidSpeed;
		asteroids[i].vel.y = (rand() % (2 * maxAsteroidSpeed)) - maxAsteroidSpeed;
	}
}

//Takes and processes inputs
int processInput() {
	//Init events
	SDL_Event event;
	SDL_PollEvent(&event);

	//Check if certain keys are pressed
	switch (event.type) {
		//If Exit or Alt+F4 button is pressed
	case SDL_QUIT:
		return false;
		break;
		//If key is pressed
	case SDL_KEYDOWN:
		//If key pressed is Esc
		if (event.key.keysym.sym == SDLK_ESCAPE) {
			return false;
		}
	}

	//If a key was pressed
	if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
		//Adjust the input variable
		switch (event.key.keysym.sym) {
		//case SDLK_w: ship.isBurn = true; break;
		case SDLK_UP: ship.isBurn = true; break;
		//case SDLK_a: ship.dr += 1; break;
		case SDLK_LEFT: ship.dr += 1; break;
		//case SDLK_d: ship.dr -= 1; break;
		case SDLK_RIGHT: ship.dr -= 1; break;
		}
	}
	//If a key was released
	else if (event.type == SDL_KEYUP && event.key.repeat == 0) {
		//Adjust the input variable
		switch (event.key.keysym.sym) {
		//case SDLK_w: ship.isBurn = false; break;
		case SDLK_UP: ship.isBurn = false; break;
		//case SDLK_a: ship.dr -= 1; break;
		case SDLK_LEFT: ship.dr -= 1; break;
		//case SDLK_d: ship.dr += 1; break;
		case SDLK_RIGHT: ship.dr += 1; break;
		}
	}
}

//Update objects every frame
void update() {
	//Wait an amount of time until target FPS reached

	deltaTime = lastFrameTime;
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), lastFrameTime + targetFrameTime));
	lastFrameTime = SDL_GetTicks();
	deltaTime = (lastFrameTime - deltaTime)/16;

	//update player rotation
	ship.r += ship.dr * playerRotationSpeed * deltaTime;
	if (ship.r >= 360) {
		ship.r -= 360;
	}
	else if (ship.r < 0) {
		ship.r += 360;
	}

	//forwards force
	if (ship.isBurn && sqrt(pow(ship.vel.x,2)+pow(ship.vel.y,2)) <= maxSpeed ) {
		ship.vel.x += accelFactor * sin(ship.r * 3.14 / 180) * deltaTime;
		ship.vel.y += accelFactor * cos(ship.r * 3.14 / 180) * deltaTime;
	}

	ship.vel.x *= decelFactor;
	ship.vel.y *= decelFactor;

	if (sqrt(pow(ship.vel.x, 2) + pow(ship.vel.y, 2)) < 0.1) {
		ship.vel.x = 0;
		ship.vel.y = 0;
	}

	//Teleport asteroid to opposite side of screen if offscreen
	for (int i = 0; i < 48; i++) {

		if (asteroids[i].pos.x > windowWidth + 40) {
			asteroids[i].pos.x = 0 - 40;
		}
		else if (asteroids[i].pos.x < 0 - 40) {
			asteroids[i].pos.x = windowWidth + 40;
		}

		if (asteroids[i].pos.y > windowHeight + 40) {
			asteroids[i].pos.y = 0 - 40;
		}
		else if (asteroids[i].pos.y < 0 - 40) {
			asteroids[i].pos.y = windowHeight + 40;
		}
	}

	if (ship.pos.x > windowWidth + 40) {
		ship.pos.x = 0 - 40;
	}
	else if (ship.pos.x < 0 - 40) {
		ship.pos.x = windowWidth + 40;
	}

	if (ship.pos.y > windowHeight + 40) {
		ship.pos.y = 0 - 40;
	}
	else if (ship.pos.y < 0 - 40) {
		ship.pos.y = windowHeight + 40;
	}

	////update positions
	ship.pos.x += ship.vel.x * deltaTime;
	ship.pos.y += ship.vel.y * deltaTime;

	for (int i = 0; i < 48; i++) {
		asteroids[i].pos.x += asteroids[i].vel.x * deltaTime;
		asteroids[i].pos.y += asteroids[i].vel.y * deltaTime;
	}
}

//Tell renderer to show objects on screen
void render() {
	//Draw background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	//Clear Screen
	SDL_RenderClear(renderer);

	//Loop through every asteroid
	for (int i = 0; i < 48; i++) {
		//calculate vertecies
		asteroids[i].vertecies = calcVertecies(asteroids[i].shapeType, asteroids[i].r * 3.1415 / 180, asteroids[i].pos, asteroids[i].size/2);

		//set color to white
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		//draw lines
		if (asteroids[i].alive) {
			for (int j = 0; j < asteroids[i].vertecies.numVertecies - 1; j++) {
				SDL_RenderDrawLine(renderer, asteroids[i].vertecies.vertecies[j].x, asteroids[i].vertecies.vertecies[j].y, asteroids[i].vertecies.vertecies[j + 1].x, asteroids[i].vertecies.vertecies[j + 1].y);
			}
			SDL_RenderDrawLine(renderer, asteroids[i].vertecies.vertecies[asteroids[i].vertecies.numVertecies - 1].x, asteroids[i].vertecies.vertecies[asteroids[i].vertecies.numVertecies - 1].y, asteroids[i].vertecies.vertecies[0].x, asteroids[i].vertecies.vertecies[0].y);
		}
	}

	//Do same for player
	ship.vertecies = calcVertecies(ship.shapeType, ship.r * 3.14/180, ship.pos, 1);

	//set color to white
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	//draw lines
	for (int j = 0; j < ship.vertecies.numVertecies - 1; j++) {
		SDL_RenderDrawLine(renderer, ship.vertecies.vertecies[j].x, ship.vertecies.vertecies[j].y, ship.vertecies.vertecies[j + 1].x, ship.vertecies.vertecies[j + 1].y);
	}
	SDL_RenderDrawLine(renderer, ship.vertecies.vertecies[ship.vertecies.numVertecies - 1].x, ship.vertecies.vertecies[ship.vertecies.numVertecies - 1].y, ship.vertecies.vertecies[0].x, ship.vertecies.vertecies[0].y);

	//Swap buffer frame for current frame (Draws frame)
	SDL_RenderPresent(renderer);
}

//Uninitializes everything in reverse order it initialized it
void destroyWindow() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

//Main function
int main(int argc, char* argv[]) {
	//initialize RNG
	srand(time(NULL));

	//Set isRunning variable to if window initizalized
	int isRunning = initializeWindow();

	//Set up scene
	setup();

	//Main Game Loop
	while (isRunning) {
		isRunning = processInput();
		update();
		render();
	}

	//If isRunning is false, or loop breaks suddenly, destroy the window and uninitialize everything
	destroyWindow();
}