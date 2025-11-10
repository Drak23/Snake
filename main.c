#include "raylib.h"
#include "raymath.h"  
#include <stdbool.h>
#include <time.h>

#define TILE_SIZE 40
#define MAX_BODY 50
#define MAX_PLATFORMS 6
#define MAX_FRUITS 5

// ----------------------------------------------------
// Estructuras
// ----------------------------------------------------
typedef struct Segment 
{
    Vector2 pos;
} Segment;

typedef struct Snake 
{
    Segment body[MAX_BODY];
    int length;
} Snake;

typedef struct Platform 
{
    Rectangle rect;
} Platform;

typedef struct Fruit 
{
    Vector2 pos;
    bool active;
} Fruit;

// ----------------------------------------------------
// Funciones auxiliares
// ----------------------------------------------------
static Vector2 Vec2Add(Vector2 a, Vector2 b)
{
    Vector2 r = { a.x + b.x, a.y + b.y };
    return r;
}

static bool Vec2Equals(Vector2 a, Vector2 b)
{
    return ((int)a.x == (int)b.x && (int)a.y == (int)b.y);
}

bool CheckCollisionWithPlatforms(Vector2 pos, Platform platforms[], int count)
{
    Rectangle box = { pos.x * TILE_SIZE, pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
    for (int i = 0; i < count; i++) 
    {
        if (CheckCollisionRecs(box, platforms[i].rect)) return true;
    }
    return false;
}

bool CheckCollisionWithBody(Vector2 pos, Snake *snake)
{
    for (int i = 0; i < snake->length; i++) 
    {
        if (Vec2Equals(pos, snake->body[i].pos)) return true;
    }
    return false;
}

bool IsSupported(Vector2 pos, Platform platforms[], int count)
{
    Vector2 below = { pos.x, pos.y + 1 };
    return CheckCollisionWithPlatforms(below, platforms, count);
}

bool HasBodyBelow(Vector2 pos, Snake *snake)
{
    Vector2 below = { pos.x, pos.y + 1 };
    for (int i = 0; i < snake->length; i++) 
    {
        if (Vec2Equals(below, snake->body[i].pos)) return true;
    }
    return false;
}

void SpawnFruits(Fruit fruits[], int maxFruits)
{
    for (int i = 0; i < maxFruits; i++) 
    {
        fruits[i].pos = (Vector2){ GetRandomValue(1, 18), GetRandomValue(1, 10) };
        fruits[i].active = true;
    }
}

// ----------------------------------------------------
// MAIN
// ----------------------------------------------------
int main(void)
{
    InitWindow(800, 600, "Snakebird - Subir y bajar con cabeza y cola afectadas por gravedad");
    SetTargetFPS(10);

    Snake snake = {0};
    snake.length = 4;
    snake.body[0].pos = (Vector2){5, 5};
    snake.body[1].pos = (Vector2){5, 6};
    snake.body[2].pos = (Vector2){5, 7};
    snake.body[3].pos = (Vector2){5, 8};

    Platform platforms[MAX_PLATFORMS] = 
    {
        {{0, 560, 800, 40}},      // suelo
        {{200, 480, 160, 40}},
        {{400, 400, 160, 40}},
        {{600, 320, 160, 40}},
        {{0, 240, 160, 40}},
        {{300, 160, 200, 40}},
    };

    Fruit fruits[MAX_FRUITS] = {0};
    SpawnFruits(fruits, MAX_FRUITS);

    while (!WindowShouldClose())
    {
        // ------------------------------------------
        // INPUT: Movimiento paso a paso
        // ------------------------------------------
        Vector2 direction = {0, 0};
        if (IsKeyPressed(KEY_RIGHT)) direction = (Vector2){1, 0};
        if (IsKeyPressed(KEY_LEFT))  direction = (Vector2){-1, 0};
        if (IsKeyPressed(KEY_UP))    direction = (Vector2){0, -1};
        if (IsKeyPressed(KEY_DOWN))  direction = (Vector2){0, 1};

        Vector2 nextHead = Vec2Add(snake.body[0].pos, direction);

        // Lógica para subir
        if (direction.y == -1)
        {
            Vector2 above = { snake.body[0].pos.x, snake.body[0].pos.y - 1 };
            // Subir si no hay bloque encima
            if (CheckCollisionWithPlatforms(above, platforms, MAX_PLATFORMS))
                direction = (Vector2){0, 0};
            else
                nextHead = above;
        }

        // Lógica para bajar 
        if (direction.y == 1)
        {
            Vector2 below = { snake.body[0].pos.x, snake.body[0].pos.y + 1 };
            bool hasGround = CheckCollisionWithPlatforms(below, platforms, MAX_PLATFORMS);
            bool hasBody = HasBodyBelow(snake.body[0].pos, &snake);

            if (hasGround || hasBody)
                direction = (Vector2){0, 0}; // no puede bajar
            else
                nextHead = below;
        }

        // Movimiento horizontal y vertical permitido
        if ((direction.x != 0 || direction.y != 0) &&
            !CheckCollisionWithPlatforms(nextHead, platforms, MAX_PLATFORMS) &&
            !CheckCollisionWithBody(nextHead, &snake))
        {
            for (int i = snake.length - 1; i > 0; i--) 
            {
                snake.body[i].pos = snake.body[i - 1].pos;
            }
            snake.body[0].pos = nextHead;
        }

        // ------------------------------------------
        // GRAVEDAD
        // ------------------------------------------
        bool headSupported = IsSupported(snake.body[0].pos, platforms, MAX_PLATFORMS) || HasBodyBelow(snake.body[0].pos, &snake);
        bool tailSupported = IsSupported(snake.body[snake.length - 1].pos, platforms, MAX_PLATFORMS);

        if (!headSupported && !tailSupported)
        {
            bool blocked = false;
            for (int i = 0; i < snake.length; i++) 
            {
                Vector2 below = { snake.body[i].pos.x, snake.body[i].pos.y + 1 };
                if (CheckCollisionWithPlatforms(below, platforms, MAX_PLATFORMS)) 
                {
                    blocked = true;
                    break;
                }
            }

            if (!blocked) 
            {
                for (int i = 0; i < snake.length; i++) 
                {
                    snake.body[i].pos.y += 1;
                }
            }
        }

        // ------------------------------------------
        // FRUTAS
        // ------------------------------------------
        for (int i = 0; i < MAX_FRUITS; i++) 
        {
            if (fruits[i].active && Vec2Equals(fruits[i].pos, snake.body[0].pos)) 
            {
                fruits[i].active = false;
                if (snake.length < MAX_BODY) 
                {
                    snake.body[snake.length].pos = snake.body[snake.length - 1].pos;
                    snake.length++;
                }
            }
        }

        // ------------------------------------------
        // DIBUJO
        // ------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < MAX_PLATFORMS; i++)
            DrawRectangleRec(platforms[i].rect, GRAY);

        for (int i = 0; i < MAX_FRUITS; i++)
            if (fruits[i].active)
                DrawCircle(fruits[i].pos.x * TILE_SIZE + TILE_SIZE / 2,
                           fruits[i].pos.y * TILE_SIZE + TILE_SIZE / 2,
                           TILE_SIZE / 3, PURPLE);

        for (int i = 0; i < snake.length; i++) 
        {
            Color c = (i == 0) ? RED : (i == snake.length - 1 ? BLUE : ORANGE);
            DrawRectangle(snake.body[i].pos.x * TILE_SIZE,
                          snake.body[i].pos.y * TILE_SIZE,
                          TILE_SIZE, TILE_SIZE, c);
        }

        DrawText("Subir(^) | Bajar() | Lados(<-, ->)", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
