#include "raylib.h"
#include <stdlib.h>
#include <math.h>

// Interpolación lineal simple
static float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}


#define SCREEN_W  960
#define SCREEN_H  720

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

#define MAX_SEGMENTS 64
#define MAX_BANANAS 16
#define MAX_PLATFORMS 16

Segment segments[MAX_SEGMENTS];
int segCount = 3;

Banana bananas[MAX_BANANAS];
int bananaCount = 0;

Platform platforms[MAX_PLATFORMS];
int platformCount = 0;

Vector2 vel = {0, 0};
float gravity = 900.0f;
bool onGround = false;

Color BG_COLOR = {96, 200, 189, 255};

void InitLevel(void);
void ResetGame(void);
void SpawnBanana(float x, float y);
void SpawnPlatform(float x, float y, float w, float h);
bool CheckCollisionSegmentPlatform(const Segment *s, const Platform *p);
void DrawBanana(const Banana *b);


int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Snakebird - Demo raylib");
    SetTargetFPS(60);

    ResetGame();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float move = 0;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) move += 1;
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) move -= 1;

        float speed = 250.0f;
        segments[0].pos.x += move * speed * dt;

        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE)) && onGround) {
            vel.y = -500.0f;
            onGround = false;
        }

        vel.y += gravity * dt;
        segments[0].pos.y += vel.y * dt;

        onGround = false;
        for (int i = 0; i < platformCount; i++) {
            if (CheckCollisionSegmentPlatform(&segments[0], &platforms[i])) {
                segments[0].pos.y = platforms[i].rect.y - segments[0].size;
                vel.y = 0;
                onGround = true;
            }
        }

        if (segments[0].pos.x < 0) segments[0].pos.x = 0;
        if (segments[0].pos.x > SCREEN_W - segments[0].size) segments[0].pos.x = SCREEN_W - segments[0].size;
        if (segments[0].pos.y > SCREEN_H) ResetGame();

        for (int i = 1; i < segCount; i++) {
            Vector2 target = segments[i - 1].pos;
            float followSpeed = 10.0f;
            segments[i].pos.x = Lerp(segments[i].pos.x, target.x - (segments[i].size * i * 0.1f), followSpeed * dt);
            segments[i].pos.y = Lerp(segments[i].pos.y, target.y + (segments[i].size * 0.2f), followSpeed * dt);
        }

        for (int b = 0; b < bananaCount; b++) {
            if (!bananas[b].alive) continue;
            Rectangle headRect = { segments[0].pos.x, segments[0].pos.y, segments[0].size, segments[0].size };
            if (CheckCollisionCircleRec(bananas[b].pos, bananas[b].radius, headRect)) {
                bananas[b].alive = false;
                if (segCount < MAX_SEGMENTS) {
                    segments[segCount].pos = (Vector2){ segments[segCount - 1].pos.x - 20, segments[segCount - 1].pos.y };
                    segments[segCount].size = segments[0].size;
                    segCount++;
                }
            }
        }

        if (IsKeyPressed(KEY_R)) ResetGame();

        BeginDrawing();
            ClearBackground(BG_COLOR);

            // Dibujar plataformas como nubes
            for (int i = 0; i < platformCount; i++) {
                Rectangle r = platforms[i].rect;
                int parts = (int)(r.width / (r.height));
                if (parts < 1) parts = 1;
                float step = r.width / parts;
                for (int p = 0; p < parts; p++) {
                    float cx = r.x + p * step + step * 0.5f;
                    float cy = r.y + r.height * 0.5f;
                    DrawCircle((int)cx, (int)cy, r.height * 0.45f, WHITE);
                }
            }

            for (int b = 0; b < bananaCount; b++) {
                if (bananas[b].alive) DrawBanana(&bananas[b]);
            }

            for (int i = segCount - 1; i >= 0; i--) {
                Segment *s = &segments[i];
                if (i == 0) {
                    DrawRectangle((int)s->pos.x, (int)s->pos.y, (int)s->size, (int)s->size, (Color){240,120,40,255});
                    DrawRectangle((int)(s->pos.x + s->size * 0.6f), (int)(s->pos.y + s->size * 0.1f),
                                  (int)(s->size * 0.25f), (int)(s->size * 0.25f), (Color){255,200,70,255});
                    DrawCircle((int)(s->pos.x + s->size * 0.25f), (int)(s->pos.y + s->size * 0.3f), s->size * 0.12f, WHITE);
                    DrawCircle((int)(s->pos.x + s->size * 0.25f), (int)(s->pos.y + s->size * 0.3f), s->size * 0.06f, BLACK);
                    DrawTriangle(
                        (Vector2){s->pos.x + s->size * 0.7f, s->pos.y + s->size * 0.4f},
                        (Vector2){s->pos.x + s->size * 0.95f, s->pos.y + s->size * 0.5f},
                        (Vector2){s->pos.x + s->size * 0.7f, s->pos.y + s->size * 0.6f},
                        (Color){255,215,0,255}
                    );
                } else {
                    DrawRectangle((int)s->pos.x, (int)s->pos.y, (int)s->size, (int)s->size, (Color){255 - i*6, 160 + i*2, 50 + i*2, 255});
                }
            }

            DrawText(TextFormat("Bananas: %d/%d  Segments: %d", bananaCount, bananaCount, segCount), 10, 10, 20, BLACK);
            DrawText("A/D = mover | W/Space = saltar | R = reiniciar", 10, SCREEN_H - 28, 16, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void ResetGame(void) {
    segCount = 3;
    float startX = 200;
    float startY = 380;
    float size = 48;
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        segments[i].size = size;
        segments[i].pos.x = startX - i * (size + 2);
        segments[i].pos.y = startY;
    }
    vel = (Vector2){0, 0};
    onGround = false;

    platformCount = 0;
    SpawnPlatform(120, 420, 300, 44);
    SpawnPlatform(520, 540, 240, 44);
    SpawnPlatform(300, 300, 160, 40);
    SpawnPlatform(720, 360, 160, 40);

    bananaCount = 0;
    SpawnBanana(180, 320);
    SpawnBanana(300, 260);
    SpawnBanana(520, 480);
    SpawnBanana(760, 320);
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

bool CheckCollisionSegmentPlatform(const Segment *s, const Platform *p) {
    Rectangle segRec = { s->pos.x, s->pos.y, s->size, s->size };
    Rectangle platRec = p->rect;
    if (CheckCollisionRecs(segRec, platRec)) {
        float segBottom = s->pos.y + s->size;
        float platTop = platRec.y;
        if (segBottom > platTop && segBottom < platTop + 16) {
            return true;
        }
    }
    return false;
}

void DrawBanana(const Banana *b) {
    Vector2 p = b->pos;
    float r = b->radius;
    DrawCircle((int)p.x, (int)p.y, r, (Color){255,215,0,255});
    DrawCircleLines((int)p.x, (int)p.y, r, (Color){200,150,0,255});
}
