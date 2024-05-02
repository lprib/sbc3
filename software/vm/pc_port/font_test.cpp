#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
   // Initialization
   //--------------------------------------------------------------------------------------
   const int screenWidth = 800;
   const int screenHeight = 63 * 6;

   InitWindow(
      screenWidth,
      screenHeight,
      "raylib [text] example - raylib fonts"
   );

   // NOTE: Textures MUST be loaded after Window initialization (OpenGL context
   // is required)
   Font font = {0};

   font = LoadFont("font2.png");

   const char* messages[8] = {
      "the QUICK brown FOX jumped OVER the LAZY dog",
      "THE quick BROWN fox JUMPED over THE lazy DOG",
      "123 234 345 456 567 678 789 890",
      "!@# @#$ #$% $%^ %^& ^&* &*( *() ()_ )_+ -=",
      "[]\\{}|;':\",./<>?`~",
      "file.read(reinterpret_cast<char*>(magic), sizeof(magic));",
      "Platform& operator=(const Platform&) = delete;",
      "for (int i = 0; i < 8; i++)"
   };

   int spacing = 2;

   //  Color colors[8] = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE,
   //  LIME, GOLD, RED };
   Color colors[8];
   for(int i = 0; i < 8; ++i) {
      colors[i] = DARKPURPLE;
   }

   SetTargetFPS(60); // Set our game to run at 60 frames-per-second
   //--------------------------------------------------------------------------------------

   // Main game loop
   while(!WindowShouldClose()) // Detect window close button or ESC key
   {
      // Update
      //----------------------------------------------------------------------------------
      // TODO: Update your variables here
      //----------------------------------------------------------------------------------

      // Draw
      //----------------------------------------------------------------------------------
      BeginDrawing();

      ClearBackground(RAYWHITE);

      int size = 3;
      int linespace = 3;
      for(int i = 0; i < 8; i++) {
         DrawTextEx(
            font,
            messages[i],
            {10.0f, (float)((8 + linespace) * size * i + 10)},
            font.baseSize * size,
            spacing,
            colors[i]
         );
      }

      EndDrawing();
      //----------------------------------------------------------------------------------
   }

   // De-Initialization
   //--------------------------------------------------------------------------------------

   // Fonts unloading
   UnloadFont(font);

   CloseWindow(); // Close window and OpenGL context
   //--------------------------------------------------------------------------------------

   return 0;
}