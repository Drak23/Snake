#include "raylib.h"
#include <stdlib.h>
#include <math.h>

#define SCREEN_W 960
#define SCREEN_H 720
#define MAX_SEGMENTS 64
#define MAX_BANANAS 16
#define MAX_PLATFORMS 16

typedef struct {
    Vector2 pos;
    float size;
} Segment;

typedef struct {
    Vector2 pos;
    float radius;
    bool alive;
} Banana;

typedef struct {
    Rectangle rect;
} Platform;

// Globales
Segment segments[MAX_SEGMENTS];
int segCount = 3;

Banana bananas[MAX_BANANAS];
int bananaCount = 0;

Platform platforms[MAX_PLATFORMS];
int platformCount = 0;

Vector2 vel = {0, 0};
float gravity = 900.0f;
bool onGround = false;
Color BG_COLOR = {125, 200, 255, 255};

// --- Prototipos
static float Lerp(float a, float b, float t);
void ResetGame(void);
void SpawnBanana(float x, float y);
void SpawnPlatform(float x, float y, float w, float h);
bool CheckCollisionSegmentPlatform(const Segment *s, const Platform *p);
void DrawBanana(const Banana *b);
void DrawBirdSegment(const Segment *s, int i);
void SpawnNewBanana(void);

// --- Funciones principales

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "SnakeBird - Raylib Demo Mejorada");
    SetTargetFPS(60);

    ResetGame();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float moveX = 0;

        // Movimiento lateral
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveX += 1;
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) moveX -= 1;

        // Salto
        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE)) && onGround) {
            vel.y = -500.0f;
            onGround = false;
        }

        float speed = 250.0f;
        segments[0].pos.x += moveX * speed * dt;

        // Gravedad
        vel.y += gravity * dt;
        segments[0].pos.y += vel.y * dt;

        // Colisiones con plataformas
        onGround = false;
        for (int i = 0; i < platformCount; i++) {
            if (CheckCollisionSegmentPlatform(&segments[0], &platforms[i])) {
                segments[0].pos.y = platforms[i].rect.y - segments[0].size;
                vel.y = 0;
                onGround = true;
            }
        }

        // Límites de pantalla
        if (segments[0].pos.x < 0) segments[0].pos.x = 0;
        if (segments[0].pos.x > SCREEN_W - segments[0].size) segments[0].pos.x = SCREEN_W - segments[0].size;
        if (segments[0].pos.y > SCREEN_H) ResetGame();

        // Movimiento de segmentos
        for (int i = 1; i < segCount; i++) {
            Vector2 target = segments[i - 1].pos;
            float followSpeed = 10.0f;
            segments[i].pos.x = Lerp(segments[i].pos.x, target.x - (segments[i].size * 0.5f), followSpeed * dt);
            segments[i].pos.y = Lerp(segments[i].pos.y, target.y + (segments[i].size * 0.2f), followSpeed * dt);
        }

        // Comer bananas
        for (int b = 0; b < bananaCount; b++) {
            if (!bananas[b].alive) continue;
            Rectangle headRect = {segments[0].pos.x, segments[0].pos.y, segments[0].size, segments[0].size};
            if (CheckCollisionCircleRec(bananas[b].pos, bananas[b].radius, headRect)) {
                bananas[b].alive = false;
                if (segCount < MAX_SEGMENTS) {
                    segments[segCount].pos = segments[segCount - 1].pos;
                    segments[segCount].size = segments[0].size;
                    segCount++;
                }
                SpawnNewBanana();
            }
        }

        if (IsKeyPressed(KEY_R)) ResetGame();

        // --- DIBUJO ---
        BeginDrawing();
        ClearBackground(BG_COLOR);

        // Dibujar plataformas como plantas
        for (int i = 0; i < platformCount; i++) {
            Rectangle r = platforms[i].rect;
            DrawRectangle((int)r.x, (int)r.y, (int)r.width, (int)r.height, (Color){60, 180, 75, 255}); // verde base
            for (int j = 0; j < (int)(r.width / 20); j++) {
                float cx = r.x + j * 20 + 10;
                DrawTriangle(
                    (Vector2){cx, r.y},
                    (Vector2){cx - 10, r.y - 20},
                    (Vector2){cx + 10, r.y - 20},
                    (Color){50, 200, 50, 255}
                );
            }
        }

        // Dibujar bananas
        for (int b = 0; b < bananaCount; b++) {
            if (bananas[b].alive) DrawBanana(&bananas[b]);
        }

        // Dibujar ave segmentada
        for (int i = segCount - 1; i >= 0; i--) DrawBirdSegment(&segments[i], i);

        DrawText(TextFormat("Bananas comidas: %d", segCount - 3), 10, 10, 20, BLACK);
        DrawText("← → para moverte | ↑ / Espacio para saltar | R para reiniciar", 10, SCREEN_H - 30, 18, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// --- Funciones auxiliares ---

static float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void ResetGame(void) {
    segCount = 3;
    float startX = 200;
    float startY = 380;
    float size = 42;
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        segments[i].size = size;
        segments[i].pos.x = startX - i * (size + 2);
        segments[i].pos.y = startY;
    }
    vel = (Vector2){0, 0};
    onGround = false;

    // Plataformas tipo plantas (bien distribuidas)
    platformCount = 0;
    SpawnPlatform(100, 600, 250, 40);
    SpawnPlatform(420, 480, 220, 40);
    SpawnPlatform(700, 360, 200, 40);
    SpawnPlatform(500, 250, 200, 40);
    SpawnPlatform(220, 150, 160, 40);

    // Bananas iniciales
    bananaCount = 0;
    SpawnBanana(180, 520);
    SpawnBanana(520, 420);
    SpawnBanana(720, 300);
    SpawnBanana(550, 180);
}

void SpawnPlatform(float x, float y, float w, float h) {
    if (platformCount >= MAX_PLATFORMS) return;
    platforms[platformCount++].rect = (Rectangle){x, y, w, h};
}

void SpawnBanana(float x, float y) {
    if (bananaCount >= MAX_BANANAS) return;
    bananas[bananaCount].pos = (Vector2){x, y};
    bananas[bananaCount].radius = 14.0f;
    bananas[bananaCount].alive = true;
    bananaCount++;
}

void SpawnNewBanana(void) {
    if (bananaCount >= MAX_BANANAS) return;
    float x = GetRandomValue(100, SCREEN_W - 100);
    float y = GetRandomValue(100, SCREEN_H - 300);
    SpawnBanana(x, y);
}

bool CheckCollisionSegmentPlatform(const Segment *s, const Platform *p) {
    Rectangle segRec = {s->pos.x, s->pos.y, s->size, s->size};
    return CheckCollisionRecs(segRec, p->rect);
}

void DrawBanana(const Banana *b) {
    Vector2 p = b->pos;
    float r = b->radius;
    DrawCircle((int)p.x, (int)p.y, r, (Color){255, 215, 0, 255});
    DrawCircleLines((int)p.x, (int)p.y, r, (Color){200, 150, 0, 255});
    DrawRectangle((int)p.x - 2, (int)p.y - (int)r - 6, 4, 8, (Color){90, 60, 0, 255});
}

void DrawBirdSegment(const Segment *s, int i) {
    Color bodyColor = (i == 0) ? (Color){255, 180, 70, 255} : (Color){250 - i * 3, 160 + i, 60 + i * 2, 255};
    DrawCircle((int)(s->pos.x + s->size / 2), (int)(s->pos.y + s->size / 2), s->size / 2, bodyColor);

    if (i == 0) {
        // Pico
        DrawTriangle(
            (Vector2){s->pos.x + s->size * 0.9f, s->pos.y + s->size * 0.45f},
            (Vector2){s->pos.x + s->size * 1.1f, s->pos.y + s->size * 0.5f},
            (Vector2){s->pos.x + s->size * 0.9f, s->pos.y + s->size * 0.55f},
            ORANGE
        );
        // Ojo
        DrawCircle((int)(s->pos.x + s->size * 0.3f), (int)(s->pos.y + s->size * 0.3f), s->size * 0.1f, WHITE);
        DrawCircle((int)(s->pos.x + s->size * 0.3f), (int)(s->pos.y + s->size * 0.3f), s->size * 0.05f, BLACK);
    }
}
