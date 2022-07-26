//libraries used
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

//for the text
#include <string>
#include <sstream>


//https://stackoverflow.com/questions/21007329/what-is-an-sdl-renderer#:~:text=screen%2C%20borders%20etc.-,SDL_Renderer,functions%20tied%20to%20the%20SDL_Renderer
//renderer is the struct that holds the data of the things to be rendered to the screen
SDL_Renderer* renderer;
//SDL_Window is the struct that holds all info about the Window itself: size, position, full screen, borders etc (also holds the renderer)
SDL_Window* window;
//pointer to a custom font that is used for the score
TTF_Font* font;
//struct that represents a colour using RGB as well as the alpha value(how transparent)
//represents the colour of the ball and the walls
SDL_Color objectColour;



//~~~IDENTIFIERS~~~~
//identifier for the width and height of the sdl_window display
#define WIDTH 720
#define HEIGHT 720
//identifier for the font size to be 32 pixels
#define FONT_SIZE 28
#define BALL_SPEED 16
#define BALLSIZE 16
#define PADDLESPEED 18
#define RIGHTPADDLESPEED 8
#define PI 3.14159265358979323846



//Gamestate to see if running or not
bool gameRunning;

//~~~Limit Fps~~~~
//The max FPS that is to be obtained
const int FPS = 60;
//The time between frames which represents the number of ticks between frames that need to pass until a new frame can be drawn
const int frameDelay = 1000.0f / FPS;
//number of milliseconds since SDL library initialization by using getTicks() 
Uint32 frameStart;
//the frame time between the last frameStart call and the most recent getTicks() call after all events and actions are finished
int frameTime; //used to determine game time and how long the game has been running for


//~~~Game Objects Attributes~~~
//Create a rectangle that represents the left and right paddle, the ball, and the score
SDL_Rect l_paddle, r_paddle, ball, score_board;
//velocity of the ball
float velX, velY;
//scores for both the left and right side
int l_score, r_score;
//display the score on the screen
std::string score;
//determine who's turn it is to start the game (true is right paddle, false is left)
bool turn;

//fixes the to_string bug 
namespace patch
{
	template < typename T > std::string to_string(const T& n)
	{
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}


//Resets the objects and determines who hits first
void serve() {
	//reset the paddles to their starting position
	l_paddle.y = r_paddle.y = (HEIGHT / 2) - (l_paddle.h / 2);

	//if right paddle's turn, set up for the right
	if (turn) {
		//set the ball to be closer to the right paddle
		ball.x = l_paddle.x + (l_paddle.w * 4);
		//send ball right slowly to the paddle
		velX = BALL_SPEED / 2;
	}
	else {
		//set the ball to be closer to the left paddle
		ball.x = r_paddle.x - (r_paddle.w * 4);
		//send the ball left to the left paddle
		velX = -1*(BALL_SPEED / 2);
	}
	//set the y velocity to not change so the ball moves in a straight line
	velY = 0;
	//set the ball position to be approximately in the center of the screen
	ball.y = HEIGHT / 2 - (BALLSIZE / 2);
	//after each turn, switch servers
	turn = !turn;
}

/*
* Display each players score onto the screen
* 
* @params	{string}		Text to display on the screen
* @params	{int}			x and y position that determines where the score is printed
* 
* @returns	text on the screen that corresponds to the scoreboard
*/
void write(std::string text, int x, int y) {
	//sdl_surface pointer that is used to create an sdl texture
	SDL_Surface* surface;
	//represents the scoreboard texture
	SDL_Texture* texture;

	//create a read only string that corresponds to the text to be displayed on the screen
	const char* t = text.c_str();

	/*
	* create an SDL_surface that contains pixel information of the text to be sent to a texture
	* assumes that TTF was initialized correctly (done in the main function)
	* 
	* @params {TTF_Font}		the font to render with.
	* @params {const char *}	text to render, in Latin1 encoding
	* @params {SDL_Color}		the foreground color for the text.
	* 
	* @returns	Returns a new 8-bit, palletized surface, or NULL if there was an error.
	* 
	*/
	surface = TTF_RenderText_Solid(font, t, objectColour);
	//create a texture from the surface to print onto the renderer
	texture = SDL_CreateTextureFromSurface(renderer, surface);


	//set the width and height of the scoreboard
	score_board.w = surface->w;
	score_board.h = surface->h;
	//set the x and y where the score_board is going to be placed on the screen
	score_board.x = x - score_board.w;
	score_board.y = y - score_board.h;
	//frees the memory of the surface
	SDL_FreeSurface(surface);
	//Copy the entire texture to score_board and place it on the renderer to display on the screen.
	SDL_RenderCopy(renderer, texture, NULL, &score_board);
	//free the memory of the texture
	SDL_DestroyTexture(texture);
}


void update() {

	//uses built in SDL collision detection to see if two rectangles intersect (ball and paddle)
	//https://gamedev.stackexchange.com/questions/4253/in-pong-how-do-you-calculate-the-balls-direction-when-it-bounces-off-the-paddl
	if (SDL_HasIntersection(&ball, &r_paddle)) {
		double rel = (r_paddle.y + (r_paddle.h / 2)) - (ball.y + (BALLSIZE / 2));
		double norm = rel / (r_paddle.h / 2);
		double bounce = norm * (5 * PI / 12);
		velX = -BALL_SPEED * cos(bounce);
		velY = BALL_SPEED * -sin(bounce);
	}
	
	//uses built in SDL collision detection to see if two rectangles intersect (ball and paddle)
	if(SDL_HasIntersection(&ball, &l_paddle)) {
		double rel = (l_paddle.y + (l_paddle.h / 2)) - (ball.y + (BALLSIZE / 2));
		double norm = rel / (l_paddle.h / 2);
		double bounce = norm * (5 * PI / 12);
		velX = BALL_SPEED * cos(bounce);
		velY = BALL_SPEED * -sin(bounce);
	}


	//if the position of the ball is at the bottom half of the paddle , move the right paddle upwards in order for it to bounce back to the user
	if (ball.y > r_paddle.y + (r_paddle.h / 2)) { //follows the trajectory of the ball and mimics it
		r_paddle.y += RIGHTPADDLESPEED;
	}

	//if the position of the ball is at the top half of the paddle , move the paddle downwards in order for it to bounce back to the user
	if (ball.y < r_paddle.y + (r_paddle.h / 2)) { //follows the trajectory of the ball and mimics it
		r_paddle.y -= RIGHTPADDLESPEED;
	}


	//right paddle scores
	if (ball.x <= 0) { 
		//increase score of right paddle
		r_score++;
		//the left paddle serves
		//turn = false;
		//reset the game to initial game state
		serve(); 

	}
	//left paddle scores
	if (ball.x + BALLSIZE >= WIDTH){
		//increase score of left paddle
		l_score++; 
		//the right paddle serves
		//turn = true;
		//reset the game to initial game state
		serve(); 
	}

	//If the ball touches the roof or floor, flip the direction it is going 
	if (ball.y <= 0 || ball.y + BALLSIZE >= HEIGHT) {
		velY = -velY;
	}

	//increase the position of the ball every frame
	ball.x += velX;
	ball.y += velY;

	//prints the score to the center of the screen
	score = std::to_string(l_score) + " | " + std::to_string(r_score);


	//check left paddle boundaries
	if (l_paddle.y < 0) { //if left paddle goes past top of the screen, set it to the top
		l_paddle.y = 0;
	}
	//if the left paddle goes below the screen, set it to the bottom
	if (l_paddle.y + l_paddle.h > HEIGHT) { //checks the bottom of the rectangle and ensures the bottom will always stop at the edge of the screen
		l_paddle.y = HEIGHT - l_paddle.h;
	}


	//check right paddle boundaries
	if (r_paddle.y < 0){ //if right paddle goes past top of the screen, set it to the top
		r_paddle.y = 0; 
	}
	//if the right paddle goes below the screen, set it to the bottom
	if (r_paddle.y + r_paddle.h > HEIGHT) { //checks the bottom of the rectangle and ensures the bottom will always stop at the edge of the screen
		r_paddle.y = HEIGHT - r_paddle.h;
	}




}

//user inputs that moves the paddles up and down
void input() {
	//create event handler that handles different type of events with name 'event'
	//events are inputs information on current things in the game (mouse input, controller, etc)
	//read events from event queue or place events on event queue
	SDL_Event event;

	//Get a snapshot of the current state of the keyboard.
	//Returns a pointer to an array of key states.

	/*
	* Gets a snapshot of the current keyboard state (pressed or unpressed). The current state is return as a pointer to an array.
	* The size of this array is stored in 'numkeys'. The array is indexed by the SDLK_* symbols. 
	* A value of 1 means the key is pressed and a value of 0 means its not. 
	* The pointer returned is a pointer to an internal SDL array and should not be freed by the caller.
	* 
	* @params {int*}	If non-NULL, receives the length of the returned array and places it into 'numkeys'
	*					'numkeys' corresponds to an integer pointer that has the the length of the returned array
	*					
	* 
	* @returns			Returns a pointer to an array of key states and assigns the length of the array to an integer 'numkeys'
	*/
	const Uint8* keystates = SDL_GetKeyboardState(NULL);

	//polls all the events and sees if a correct input is pressed
	//not using scancode because we want it to represent specific keys that the keyboard switch represents
	while (SDL_PollEvent(&event)) {

		//If forced quit, quit and clean up
		if (event.type == SDL_QUIT) {
			gameRunning = false;
		}

		//if the keyboard is pressed, check which one
		if (event.type == SDL_KEYDOWN) {
			//key.keysym.syms is used for checking for certain key presses
			//SDLK_# represents a certain key on the keyboard
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE: //close the game when escape is pressed
				gameRunning = false;
				break;
			case SDLK_w:
				//move the player up
				l_paddle.y -= PADDLESPEED;
				break;
			case SDLK_s:
				//move the player down
				l_paddle.y += PADDLESPEED;
				break;
			default:
				break;
			}
		}
	}
}

//renders out the game onto the screen
void render() {
	//The colour that is set after you clear the screen (black as default)
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255);
	//Clears the rendering target with the draw color set above
	//Sets the entire screen to be black / background
	SDL_RenderClear(renderer);

	//set the game object colours to be this white
	SDL_SetRenderDrawColor(renderer, objectColour.r, objectColour.g, objectColour.b, 255);

	//fill the paddles and balls to be white
	SDL_RenderFillRect(renderer, &l_paddle);
	SDL_RenderFillRect(renderer, &r_paddle);
	SDL_RenderFillRect(renderer, &ball);



	//draw the scoreboard onto the screen in the center of the renderer
	write(score, WIDTH / 2 + FONT_SIZE, FONT_SIZE * 2);


	SDL_RenderPresent(renderer);
}


int main(int argc, char* argv[]) {
	//check to see if SDL is able to initialize with all flags (audio, keyboard+mouse input, etc)
	//returns 0 if success, negative number is SDL cannot be initialized
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "SDL_Init failed" << std::endl;
		return false;
	}

	/*
	* check to see if a default renderer and window is able to be created
	* 
	* @params {int}				width and height of the potential window to display to the user
	* @params {Unit32}			flags used to create the windows (fullscreen, windows, etc)
	* @params {SDL_Window**}	reference pointer to an sdl window used to fill and display to the screen
	* @params {SDL_Renderer**}	reference pointer to an sdl renderer used to contain the rendering state
	* 
	* @returns	0 if success, negative number if either a window or renderer is unable to be created
	*/
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
		std::cout << "SDL_CreateWindowAndRenderer() failed)" << std::endl;
		return false;
	}
	//check to see if you are able to use True Type fonts to create images with different fonts
	//creates textures from font text
	if (TTF_Init() < 0) {
		std::cout << "TTF_Inidt() failed" << std::endl;
		return false;
	}
	//assign the ttf_font pointer variable to a custom font with the given font and size
	font = TTF_OpenFont("minecraft-font.ttf", FONT_SIZE);
	
	//Colours for the walls and the balls
	objectColour.r = objectColour.g = objectColour.b = 255;
	//set the score for each player to be 0
	l_score = r_score = 0;


	//set the size of the paddle to take 1/4 of the screen
	l_paddle.h = HEIGHT / 4;
	//the thickness of the paddle
	l_paddle.w = 12;
	//set the starting x position of the left paddle to be 32 pixels to the right
	l_paddle.x = 32; 
	//set the starting y position of the left paddle to be approximately in the center of the screen
	l_paddle.y = (HEIGHT / 2) - (l_paddle.h / 2);


	//set the size of the right paddle to be the same as the left
	//set the starting y position of the right paddle
	r_paddle = l_paddle;
	//set the starting x position of the right paddle
	r_paddle.x = WIDTH - r_paddle.w - 32;

	//set the size of the ball
	ball.w = ball.h = BALLSIZE;





	//set the game state to be running
	gameRunning = true;

	//determines who serves the ball
	serve();

	//game loop
	while (gameRunning) {
		//number of milliseconds since SDL library initialization per call
		frameStart = SDL_GetTicks();

		//place all game updates / rendering here in between the frames
		
		//represents where the logic of the game will be
		update();
		//checks the user input
		input();
		//renders out the aftermath of the input and update function onto the screen
		render();




		//number of seconds it took to go through all events, updating renderer, and rendering to the window
		frameTime = SDL_GetTicks() - frameStart;
		//delay frames to ensure consistency / reach the VSYNC goal of 60FPS
		if (frameDelay > frameTime) {  
			//wait a specified # of milliseconds before returning
			// if the speed it took to go through all the events and rendering is faster than the frameDelay, wait the difference of times
			// in order to draw onto the screen after the correct time interval
			SDL_Delay(frameDelay - frameTime);
		}

	}
	//Free the memory used by the font variable, and free font itself as well.
	TTF_CloseFont(font); //Do not use font after this without loading a new font to it.
	//destroys the renderer and frees memory
	SDL_DestroyRenderer(renderer);
	//destroys the window and frees memory
	SDL_DestroyWindow(window);
	//Destroys and clears memory for all the initialized subsystems (sound, input, etc)
	SDL_Quit();

	return 0;

}

