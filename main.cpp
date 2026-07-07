/* Rosa Knowles 
 * Created 7/6/2026
 * Last Edited 7/7/2026
 * Snake game written in Raylib
 * Compilation command bc i'm too lazy to write a makefile: 
    * g++ main.cpp -o main -I C:\raylib\raylib\src -O1 -Wall -std=c++23 -Wno-missing-braces -L ./lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
 */


#include "raylib.h"
#include "raymath.h"
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <vector>
#include <cmath>

#define TILE_SIZE 16
#define NUM_TILES 13
#define START_X 6
#define START_Y 6
#define SCALAR 2
#define SNAKE_SPEED 4
#define BG_COLOR (Color){38, 35, 25, 255}
#define HEAD_COLOR (Color){16, 145, 67, 255}
#define START_MSG "Press The Arrow Keys\nor WASD to Start."
#define GAME_OVER_MSG_A "GAME OVER."
#define GAME_OVER_MSG_B "Press Any Key to Play Again."


// struct that stores a pair of bytes 
// useful for storing coordinate pairs 
struct BytePair
{
    int8_t x;
    int8_t y;

    // adds the struct to another struct
    // mutates the struct that this is called on
    void add(BytePair & b)
    {
        x += b.x;
        y += b.y;
    }
    // same as the previous function, but subtracts
    void subtract(BytePair & b)
    {
        x -= b.x;
        y -= b.y;
    }
};

// enum that defines objects on the game board
enum BoardElements: int8_t
{
    EMPTY,
    HEAD,
    BODY,
    APPLE
};

// enum that defines game state
enum GameState: int8_t
{
    NOT_PLAYING,
    PLAYING,
    GAME_OVER
};

// function that places an apple on the board 
void place_apple(BoardElements board[NUM_TILES][NUM_TILES])
{
    using namespace std;

    // vector of available positions 
    vector<BytePair> positions;
    for (int8_t i = 0; i < NUM_TILES; ++i)
    {
        for (int8_t j = 0; j < NUM_TILES; ++j)
        {
            if (board[i][j] == EMPTY)
                positions.push_back((BytePair){i,j});
        }
    }

    // pick a random available position, and place an apple there 
    BytePair apple_pos = positions.at(rand() % positions.size());
    board[apple_pos.x][apple_pos.y] = APPLE;
}

// function that empties the board 
void empty_board(BoardElements board[NUM_TILES][NUM_TILES])
{
    for (int8_t i = 0; i < NUM_TILES; ++i)
    {
        for (int8_t j = 0; j < NUM_TILES; ++j)
        {
            board[i][j] = EMPTY;
        }
    }
}

// function that clears the tail
void clear_tail(BoardElements board[NUM_TILES][NUM_TILES])
{
    for (int8_t i = 0; i < NUM_TILES; ++i)
    {
        for (int8_t j = 0; j < NUM_TILES; ++j)
        {
            if (board[i][j] == BODY) board[i][j] = EMPTY;
        }
    }
}

// function that initializes/resets the board
void set_board(BoardElements board[NUM_TILES][NUM_TILES], BytePair * head, std::vector<BytePair> * snake_body,
                uint16_t * frame_counter, BytePair * velocity, GameState * game_state, int32_t * score, int32_t * high_score)
{
    using namespace std;

    // fill board with empty tiles
    empty_board(board);

    // place snake in center of board
    board[START_X][START_Y] = HEAD;
    // store position of head
    *head = {START_X, START_Y};
    // store positions of the body
    *snake_body = {};

    // store number of frames that have passed since last movement
    *frame_counter = 0;

    // vector that stores the movement direction of the snake 
    *velocity = {0,0};
    // stores game state 
    *game_state = NOT_PLAYING;
    *score = 0;
    *high_score = max(*score, *high_score);

    // place apple on board 
    place_apple(board);
}

int main(int argc, char ** argv)
{
    using namespace std;

    // calculate screen dimensions
    const size_t SCREEN_WIDTH = TILE_SIZE * NUM_TILES * SCALAR;
    const size_t SCREEN_HEIGHT = (TILE_SIZE * NUM_TILES * SCALAR) + (2 * TILE_SIZE * SCALAR);

    // initialize window and related devices 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");
    InitAudioDevice();
    // SetTargetFPS(60);
    int16_t framerate = GetMonitorRefreshRate(GetCurrentMonitor());
    SetTargetFPS(framerate);
    // uncomment if you don't want the escape key to kill the game
    // SetExitKey(KEY_NULL);

    // set random seed 
    srand(time(NULL));

    // vector that stores the size of the individual board tiles
    Vector2 tile_size_vec = {TILE_SIZE * SCALAR, TILE_SIZE * SCALAR};

    // initialize matrix that stores board elements 
    BoardElements board[NUM_TILES][NUM_TILES];

    // initialize game variables 
    BytePair head, velocity;
    vector<BytePair> snake_body;
    uint16_t frame_counter;
    GameState game_state;
    int32_t score, high_score;

    // initialize board
    set_board(board, &head, &snake_body, &frame_counter, &velocity, &game_state, &score, &high_score);

    while(!WindowShouldClose())
    {
        if (game_state != GAME_OVER)
        {
            // handle changing direction
            // also sets game state to playing if it is currently set as not playing
            if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && board[head.x][head.y - 1] != BODY)
            {
                velocity = {0,-1};
                game_state = (game_state == GAME_OVER) ? GAME_OVER : PLAYING;
            }
            else if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && board[head.x][head.y + 1] != BODY)
            {
                velocity = {0, 1};
                game_state = (game_state == GAME_OVER) ? GAME_OVER : PLAYING;
            }
            else if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && board[head.x - 1][head.y] != BODY)
            {
                velocity = {-1, 0};
                game_state = (game_state == GAME_OVER) ? GAME_OVER : PLAYING;
            }
            else if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && board[head.x + 1][head.y] != BODY)
            {
                velocity = {1, 0};
                game_state = (game_state == GAME_OVER) ? GAME_OVER : PLAYING;
            }


            // move snake 
            // only moves if a specific number of frames have passed
            if (frame_counter >= framerate / SNAKE_SPEED)
            {
                // gonna need to check if should be tail
                board[head.x][head.y] = EMPTY;
                head.add(velocity);
                
                // handle collision 
                // apple eaten
                if (board[head.x][head.y] == APPLE)
                {
                    // we only need to place the apple, 
                    // the apple gets automatically erased 
                    // when the head moves
                    place_apple(board);
                    // extend tail
                    BytePair tail_addition = {head.x,head.y};
                    tail_addition.subtract(velocity);
                    snake_body.push_back(tail_addition);
                    // place additional tail piece on board 
                    board[tail_addition.x][tail_addition.y] = BODY;
                    // update score 
                    score++;
                    high_score = max(score, high_score);
                }
                // hit edge of board or yourself
                else if (head.x < 0 || head.x >= NUM_TILES || head.y < 0 || head.y >= NUM_TILES || board[head.x][head.y] == BODY)
                {
                    // set game state to game over
                    game_state = GAME_OVER;
                }
                // move tail if no collision detected
                else 
                {
                    size_t tail_len = snake_body.size();

                    // has each tail position replace the tail position ahead of it,
                    // and the tail closest to the head replaces the position of the head 
                    for (size_t i = 0; i < tail_len; ++i)
                    {
                        if (i == tail_len - 1)
                        {
                            snake_body.at(i).x = head.x - velocity.x;
                            snake_body.at(i).y = head.y - velocity.y;
                        }
                        else 
                        {
                            snake_body.at(i).x = snake_body.at(i + 1).x;
                            snake_body.at(i).y = snake_body.at(i + 1).y;
                        }
                    }

                    // place tail on board
                    clear_tail(board);
                    for (const auto & b : snake_body)
                    {
                        board[b.x][b.y] = BODY;
                    }
                }

                board[head.x][head.y] = HEAD;

                frame_counter = 0;
            }
        }
        // game state is game over 
        else 
        {
            if (GetKeyPressed() != 0)
                set_board(board, &head, &snake_body, &frame_counter, &velocity, &game_state, &score, &high_score);
        }

        BeginDrawing();
            if (game_state != GAME_OVER)
            {
                ClearBackground(BG_COLOR);

                // draw score bg 
                DrawRectangle(0, 0, SCREEN_WIDTH, TILE_SIZE * 2 * SCALAR, BLACK);

                for (int8_t i = 0; i < NUM_TILES; ++i)
                {
                    for (int8_t j = 0; j < NUM_TILES; ++j)
                    {
                        if (board[i][j] != EMPTY)
                        {
                            Color rendercolor = BLANK;

                            // pick rectangle color based on tile id
                            switch (board[i][j])
                            {
                                case HEAD:
                                    rendercolor = HEAD_COLOR;
                                    break;
                                case BODY:
                                    rendercolor = GREEN;
                                    break;
                                case APPLE:
                                    rendercolor = RED;
                                    break;
                                // shouldn't ever be reached lol
                                case EMPTY:
                                    rendercolor = BLANK;
                                    break;
                            }

                            // render rectangle!
                            DrawRectangleV({(float)i * TILE_SIZE * SCALAR, ((float)j * TILE_SIZE * SCALAR) + (2 * SCALAR * TILE_SIZE)}, 
                                tile_size_vec, rendercolor);
                        }
                    }
                }

                // render starting text if not playing
                if (game_state == NOT_PLAYING)
                {
                    int32_t text_size = MeasureText(START_MSG, 20);
                    DrawTextEx((Font){0}, START_MSG, 
                        {((float)SCREEN_WIDTH - text_size) / 2, (((float)SCREEN_HEIGHT) - 130) / 2}, 20, 20 * 0.1, RAYWHITE);
                }
            }
            // game over state
            else
            {
                ClearBackground(BLACK);
                // display game over msg
                int32_t text_size = MeasureText(GAME_OVER_MSG_A, 40);
                DrawTextEx((Font){0}, GAME_OVER_MSG_A,
                    {((float)SCREEN_WIDTH - text_size) / 2, (((float)SCREEN_HEIGHT) - 130) / 2}, 40, 40 * 0.1, RED);
                text_size = MeasureText(GAME_OVER_MSG_B, 20);
                DrawTextEx((Font){0}, GAME_OVER_MSG_B,
                    {((float)SCREEN_WIDTH - text_size) / 2, (((float)SCREEN_HEIGHT) - 50) / 2}, 20, 20 * 0.1, RED);
            }

            // score and high score should always be rendered, regardless of game state
            DrawTextEx((Font){0}, TextFormat("Score: %03i\nHigh Score: %03i\n", score, high_score), {5,5}, 20, 20 * 0.1, RAYWHITE);

        EndDrawing();

        frame_counter++;
    }

    // kill window and related devices 
    CloseAudioDevice();
    CloseWindow();
}