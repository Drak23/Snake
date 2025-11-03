/*******************************************************************************************
*
*   raylib [core] example - Basic 3d example
*
*   Welcome to raylib!
*
*   To compile example, just press F5.
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 cubePosition = { 0 };
Vector2 ballPosition;
Vector2 ballSpeed;

Vector2 Player1Position;
Vector2 Player1Size;
Vector2 Player2Position;
Vector2 Player2Size;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    ballPosition.x = 400;
    ballPosition.y = 225;
    
    ballSpeed.x =1;
    ballSpeed.y =1;

    Player1Position.x = 50;
    Player1Position.y = 225;
    Player1Size.x = 15;
    Player1Size.y = 60;

    Player2Position.x = screenWidth-50;
    Player2Position.y = 225;
    Player2Size.x = 15;
    Player2Size.y = 60;


    InitWindow(screenWidth, screenHeight, "raylib");

    camera.position = (Vector3){ 10.0f, 10.0f, 8.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    //--------------------------------------------------------------------------------------

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateCamera(&camera, CAMERA_ORBITAL);
    //----------------------------------------------------------------------------------

    if (IsKeyDown(KEY_W)) Player1Position.y -= 2.0f;
    if (IsKeyDown(KEY_S)) Player1Position.y += 2.0f;
    if (IsKeyDown(KEY_UP)) Player2Position.y -= 2.0f;
    if (IsKeyDown(KEY_DOWN)) Player2Position.y += 2.0f;

    ballPosition.x += ballSpeed.x;
    ballPosition.y += ballSpeed.y;

    if ((ballPosition.x >= (GetScreenWidth() - 15)) || (ballPosition.x <= 15)) ballSpeed.x *= -1.0f;
    if ((ballPosition.y >= (GetScreenHeight() - 15)) || (ballPosition.y <= 15)) ballSpeed.y *= -1.05f;

    if (Player1Position.x < 0) Player1Position.x = 0;
    else if (Player1Position.x >= GetScreenWidth()) Player1Position.x = GetScreenWidth() - 1;
    if (Player1Position.y < 0) Player1Position.y = 0;
    else if (Player1Position.y >= GetScreenHeight() ) Player1Position.y = GetScreenHeight() - 1;

    if (Player2Position.x < 0) Player2Position.x = 0;
    else if (Player2Position.x >= GetScreenWidth()) Player2Position.x = GetScreenWidth() - 1;
    if (Player2Position.y < 0) Player2Position.y = 0;
    else if (Player2Position.y >= GetScreenHeight()) Player2Position.y = GetScreenHeight() - 1;

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);

        DrawCircleV(ballPosition, 15, RED);
        DrawRectangleV(Player1Position, Player1Size, BLUE);
        DrawRectangleV(Player2Position, Player2Size, GREEN);

        //DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
