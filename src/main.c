#include <raylib.h>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define KEYBIND_LEFT KEY_LEFT
#define KEYBIND_RIGHT KEY_RIGHT
#define KEYBIND_UP KEY_UP
#define KEYBIND_DOWN KEY_DOWN

#define KEYBIND_LEFT_ALT KEY_A
#define KEYBIND_RIGHT_ALT KEY_D
#define KEYBIND_UP_ALT KEY_W
#define KEYBIND_DOWN_ALT KEY_S

#define INPUT_BUFFER_SIZE 4

#define DEAD_TEXT "YOU DIED! FINAL SCORE: %i"
#define DEAD_TEXT_SIZE 32
#define DEAD_COLOR_TEXT WHITE

#define BUTTON_SIZE 128

#define BORDER_SIZE 4
#define GRID_SIZE 16
#define TOTAL_GRID GRID_SIZE*GRID_SIZE
#define FRAMES_PER_TICK 10

#define COLOR_BACKGROUND BLACK
#define COLOR_BORDER WHITE
#define COLOR_SNAKE DARKGREEN
#define COLOR_SNAKE_HEAD GREEN
#define COLOR_APPLE RED

#if defined(PLATFORM_DESKTOP)
#undef BORDER_SIZE
#define BORDER_SIZE 0
#endif

typedef enum {
    SCREEN_GAME,
    SCREEN_DEAD
} Screen;

typedef enum {
    INPUT_NULL,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_UP,
    INPUT_DOWN
} InputBuffer;

typedef struct Position
{
    int x;
    int y;
} Position;

typedef struct InputState
{
    bool left;
    bool right;
    bool up;
    bool down;
} InputState;

typedef struct GameState
{
    unsigned long frame;
    int score;
    Position snake[TOTAL_GRID];
    Position apple;
    Position direction;
} GameState;

#define NULL_POSITION (Position){-1, -1}
#define ZERO_POSITION (Position){0, 0}
#define ZERO_INPUT (InputState){0, 0, 0 ,0};

bool PositionEquals(Position pos1, Position pos2)
{
    return pos1.x == pos2.x && pos1.y == pos2.y;
}

Position PositionAdd(Position pos1, Position pos2)
{
    return (Position){pos1.x + pos2.x, pos1.y + pos2.y};
}

Position PositionSubtract(Position pos1, Position pos2)
{
    return (Position){pos1.x - pos2.x, pos1.y - pos2.y};
}

bool PositionInBounds(Position pos)
{
    return pos.x >= 0 && pos.x < GRID_SIZE && pos.y >= 0 && pos.y < GRID_SIZE;
}

bool CollidesWithSnake(Position snake[], Position pos, bool ignoreHead)
{
    for (int i = ignoreHead; i < TOTAL_GRID; ++i)
    {
        if (PositionEquals(snake[i], NULL_POSITION)) return false;
        else if (PositionEquals(snake[i], pos)) return true;
    }

    return false;
}

Position GetNextApplePosition(Position snake[])
{
    Position apple = NULL_POSITION;
    while (PositionEquals(apple, NULL_POSITION) || CollidesWithSnake(snake, apple, false))
    {
        apple = (Position){GetRandomValue(0, GRID_SIZE-1), GetRandomValue(0, GRID_SIZE-1)};
    }

    return apple;
}

GameState NewGameState(void)
{
    GameState state;
    
    for (int i = 0; i < TOTAL_GRID; ++i)
    {
        state.snake[i] = NULL_POSITION;
    }

    state.score = 0;
    state.snake[0] = (Position){GRID_SIZE/2, GRID_SIZE/2};
    state.apple = GetNextApplePosition(state.snake);
    state.direction = ZERO_POSITION;

    return state;
}

void DrawSquare(Position pos, Color color)
{
    Vector2 gridSize = (Vector2){WINDOW_WIDTH-(BORDER_SIZE*2), WINDOW_HEIGHT-(BORDER_SIZE*2)};
    DrawRectangleRec((Rectangle){
        gridSize.x / GRID_SIZE * pos.x + BORDER_SIZE,
        gridSize.y / GRID_SIZE * pos.y + BORDER_SIZE,
        gridSize.x / GRID_SIZE,
        gridSize.y / GRID_SIZE
        },
        color
    );
}

InputState GetKeyboardInputState(void)
{
    InputState state;

    state.left = IsKeyPressed(KEYBIND_LEFT) || IsKeyPressed(KEYBIND_LEFT_ALT);
    state.right = IsKeyPressed(KEYBIND_RIGHT) || IsKeyPressed(KEYBIND_RIGHT_ALT);
    state.up = IsKeyPressed(KEYBIND_UP) || IsKeyPressed(KEYBIND_UP_ALT);
    state.down = IsKeyPressed(KEYBIND_DOWN) || IsKeyPressed(KEYBIND_DOWN_ALT);

    return state;
}

void TryDirection(GameState* state, Position attempt)
{
    if (PositionEquals(state->snake[1], NULL_POSITION))
    {
        state->direction = attempt;
        return;
    }

    if (PositionEquals(PositionAdd(state->direction, attempt), ZERO_POSITION))
        return;

    state->direction = attempt;
}

void TickSnake(GameState* state, Screen* screen)
{
    // Move snake
    Position prevPosition = PositionAdd(state->snake[0], state->direction);
    for (int i = 0; i < TOTAL_GRID; ++i)
    {
        if (PositionEquals(state->snake[i], NULL_POSITION))
        {
            break;
        }

        Position temp = state->snake[i];
        state->snake[i] = prevPosition;
        prevPosition = temp;
    }

    // Eat apple
    if (PositionEquals(state->snake[0], state->apple))
    {
        TraceLog(LOG_INFO, "%i", ++state->score);
        state->apple = GetNextApplePosition(state->snake);

        for (int i = 0; i < TOTAL_GRID; ++i)
        {
            if (PositionEquals(state->snake[i], NULL_POSITION))
            {
                state->snake[i] = prevPosition;
                break;
            }
        }
    }    

    // Check if dead
    if (!PositionInBounds(state->snake[0]) || CollidesWithSnake(state->snake, state->snake[0], true))
    {
        TraceLog(LOG_INFO, "Fail");
        (*screen) = SCREEN_DEAD;
    }
}

void UpdateInput(GameState* state)
{
    InputState inputState = GetKeyboardInputState();

    if (inputState.left)
        TryDirection(state, (Position){-1, 0});
    else if (inputState.right)
        TryDirection(state, (Position){1, 0});
    else if (inputState.up)
        TryDirection(state, (Position){0, -1});
    else if (inputState.down)
        TryDirection(state, (Position){0, 1});
}

void DrawScreenGame(GameState* state, Screen* screen)
{
    ++state->frame;
    DrawSquare(state->apple, RED);
    
    for (int i = 0; i < TOTAL_GRID; ++i)
    {
        if (!PositionEquals(state->snake[i], NULL_POSITION))
        {
            DrawSquare(state->snake[i], (i == 0 ? COLOR_SNAKE_HEAD : COLOR_SNAKE));
        }
        else break;
    }

    UpdateInput(state);

    if (state->frame % FRAMES_PER_TICK == 0)
    {
        TickSnake(state, screen);
    }
}

void DrawScreenDead(GameState* state, Screen* screen)
{
    const char* finalText = TextFormat(DEAD_TEXT, state->score);
    Vector2 textSize = MeasureTextEx(GetFontDefault(), finalText, DEAD_TEXT_SIZE, 0);
    DrawTextEx(GetFontDefault(), finalText, (Vector2){(WINDOW_WIDTH - textSize.x) / 2, (WINDOW_HEIGHT - textSize.y) / 2}, DEAD_TEXT_SIZE, 0, DEAD_COLOR_TEXT);

    if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
    {
        (*screen) = SCREEN_GAME;
        (*state) = NewGameState();
    }
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake");
    SetTargetFPS(60);

    Screen currentScreen = SCREEN_GAME;
    GameState gameState = NewGameState();

    while (!WindowShouldClose())
    {
        BeginDrawing();

        // Border for non desktop platforms
        ClearBackground(COLOR_BORDER);
        DrawRectangle(BORDER_SIZE, BORDER_SIZE, WINDOW_WIDTH-(BORDER_SIZE*2), WINDOW_HEIGHT-(BORDER_SIZE*2), COLOR_BACKGROUND);

        switch (currentScreen)
        {
            case SCREEN_GAME: DrawScreenGame(&gameState, &currentScreen); break;
            case SCREEN_DEAD: DrawScreenDead(&gameState, &currentScreen); break;
            default: break;
        }

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}