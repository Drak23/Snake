#include "raylib.h"
#include "raymath.h"   
#include <stdlib.h>
#include <time.h>


#define TILE_SIZE 40
#define GRID_WIDTH 20
#define GRID_HEIGHT 15
#define MAX_BODY 50
#define FRUIT_COUNT 3

typedef struct {
    Vector2 pos;
} Segment;

typedef struct {
    Segment body[MAX_BODY];
    int length;
} Snakebird;

typedef struct {
    Vector2 pos;
    bool active;
} Fruit;

typedef struct {
    Rectangle rect;
} Platform;

// ------------------------------------------------------------
// FUNCIONES AUXILIARES
// ------------------------------------------------------------

bool IsOnGround(Snakebird *snake, Platform *platforms, int platformCount) {
    for (int i = 0; i < snake->length; i++) {
        Vector2 below = (Vector2){snake->body[i].pos.x, snake->body[i].pos.y + 1};
        // revisa suelo de plataformas
        for (int p = 0; p < platformCount; p++) {
            Rectangle pr = platforms[p].rect;
            if (below.x * TILE_SIZE >= pr.x &&
                below.x * TILE_SIZE < pr.x + pr.width &&
                below.y * TILE_SIZE >= pr.y &&
                below.y * TILE_SIZE < pr.y + pr.height) {
                return true;
            }
        }
        // revisa si hay cuerpo debajo
        for (int j = 0; j < snake->length; j++) {
            if (i != j && snake->body[j].pos.x == below.x && snake->body[j].pos.y == below.y)
                return true;
        }
    }
    return false;
}

bool CheckCollisionWithPlatforms(Vector2 next, Platform *platforms, int count) {
    for (int i = 0; i < count; i++) {
        Rectangle r = platforms[i].rect;
        if (next.x * TILE_SIZE >= r.x && next.x * TILE_SIZE < r.x + r.width &&
            next.y * TILE_SIZE >= r.y && next.y * TILE_SIZE < r.y + r.height) {
            return true;
        }
    }
    return false;
}

bool CheckCollisionWithBody(Vector2 next, Snakebird *snake) {
    for (int i = 0; i < snake->length; i++) {
        if (snake->body[i].pos.x == next.x && snake->body[i].pos.y == next.y) return true;
    }
    return false;
}

// ------------------------------------------------------------
// JUEGO PRINCIPAL
// ------------------------------------------------------------
int main() {
    InitWindow(GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE, "Snakebird - Raylib Version");
    SetTargetFPS(10);
    srand(time(NULL));

    // Snakebird inicial
    Snakebird snake = {0};
    snake.length = 3;
    snake.body[0].pos = (Vector2){5, 5};
    snake.body[1].pos = (Vector2){5, 6};
    snake.body[2].pos = (Vector2){5, 7};

    // Plataformas (diseño original)
    Platform platforms[3];
    platforms[0].rect = (Rectangle){0, GRID_HEIGHT * TILE_SIZE - 40, GRID_WIDTH * TILE_SIZE, 40};
    platforms[1].rect = (Rectangle){200, GRID_HEIGHT * TILE_SIZE - 160, 200, 40};
    platforms[2].rect = (Rectangle){480, GRID_HEIGHT * TILE_SIZE - 280, 160, 40};
    int platformCount = 3;

    // Frutas aleatorias
    Fruit fruits[FRUIT_COUNT];
    for (int i = 0; i < FRUIT_COUNT; i++) {
        fruits[i].pos = (Vector2){rand() % GRID_WIDTH, rand() % (GRID_HEIGHT - 3)};
        fruits[i].active = true;
    }

    float gravityTimer = 0.0f;
    float gravityDelay = 0.3f;

    while (!WindowShouldClose()) {
        // ---------------------------
        // INPUT: Movimiento paso a paso
        // ---------------------------
        Vector2 direction = {0, 0};
        if (IsKeyPressed(KEY_RIGHT)) direction = (Vector2){1, 0};
        if (IsKeyPressed(KEY_LEFT)) direction = (Vector2){-1, 0};
        if (IsKeyPressed(KEY_UP)) direction = (Vector2){0, -1};

        if (direction.x != 0 || direction.y != 0) {
            Vector2 nextHead = Vector2Add(snake.body[0].pos, direction);
            if (!CheckCollisionWithPlatforms(nextHead, platforms, platformCount) &&
                !CheckCollisionWithBody(nextHead, &snake)) {
                // Mueve cuerpo
                for (int i = snake.length - 1; i > 0; i--) {
                    snake.body[i].pos = snake.body[i - 1].pos;
                }
                snake.body[0].pos = nextHead;
            }
        }

        // ---------------------------
        // FÍSICA: Gravedad
        // ---------------------------
        gravityTimer += GetFrameTime();
        if (gravityTimer >= gravityDelay) {
            gravityTimer = 0;
            if (!IsOnGround(&snake, platforms, platformCount)) {
                for (int i = 0; i < snake.length; i++) {
                    snake.body[i].pos.y += 1;
                }
            }
        }

        // ---------------------------
        // COLISIÓN CON FRUTAS
        // ---------------------------
        for (int i = 0; i < FRUIT_COUNT; i++) {
            if (fruits[i].active &&
                fruits[i].pos.x == snake.body[0].pos.x &&
                fruits[i].pos.y == snake.body[0].pos.y) {
                fruits[i].active = false;
                if (snake.length < MAX_BODY) {
                    snake.body[snake.length].pos = snake.body[snake.length - 1].pos;
                    snake.length++;
                }
            }
        }

        // ---------------------------
        // DIBUJO
        // ---------------------------
        BeginDrawing();
        ClearBackground((Color){120, 170, 255, 255}); // Fondo azul claro

        // plataformas
        for (int i = 0; i < platformCount; i++) {
            DrawRectangleRec(platforms[i].rect, (Color){60, 180, 75, 255});
        }

        // frutas
        for (int i = 0; i < FRUIT_COUNT; i++) {
            if (fruits[i].active)
                DrawCircle(fruits[i].pos.x * TILE_SIZE + TILE_SIZE / 2,
                           fruits[i].pos.y * TILE_SIZE + TILE_SIZE / 2,
                           TILE_SIZE / 4, YELLOW);
        }

        // snakebird
        for (int i = snake.length - 1; i >= 0; i--) {
            DrawRectangleV(
                (Vector2){snake.body[i].pos.x * TILE_SIZE, snake.body[i].pos.y * TILE_SIZE},
                (Vector2){TILE_SIZE, TILE_SIZE},
                i == 0 ? (Color){255, 100, 100, 255} : (Color){255, 160, 160, 255});
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
