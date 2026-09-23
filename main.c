#define RLSW_IMPLEMENTATION
#include "raylib.h"
#include "rlights.h"
#include "rlgl.h"
#include <raymath.h>

#include <stdio.h>

// --------------- PORQUERIAS PARA WINDOWS ---------------

#if defined(_WIN32)
// To avoid conflicting windows.h symbols with raylib, some flags are defined
// WARNING: Those flags avoid inclusion of some Win32 headers that could be required
// by user at some point and won't be included...
//-------------------------------------------------------------------------------------

// If defined, the following flags inhibit definition of the indicated items.
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
//#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

// Type required before windows.h inclusion
typedef struct tagMSG *LPMSG;

#include <winsock2.h>
#include <windows.h>

// Type required by some unused function...
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>

// Some required types defined for MSVC/TinyC compiler
#if defined(_MSC_VER) || defined(__TINYC__)
    #include "propidl.h"
#endif
#endif

// graphics config
#define GLSL_VERSION 330
#define TARGET_FPS 30

#define CAMERA_FOV 65

static Color COLOR_BG = {255,255,255,255};
static Color COLOR_FG = {128,128,128,255};
static Color COLOR_HL = ORANGE;

//----------------------------------------------------------------------------------
// UTILIDADES
//----------------------------------------------------------------------------------
float mapear(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(int argc, char** argv)
{
    int WINDOW_WIDTH = 200;
    int WINDOW_HEIGHT = 200;

    const int screenWidth = WINDOW_WIDTH;
    const int screenHeight = WINDOW_HEIGHT;

    Camera3D camera;

    camera.position = (Vector3){20.0f, 10.5f, 20.0f};
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = CAMERA_FOV;
    camera.projection = CAMERA_PERSPECTIVE;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"cad-renderer");
    SetTargetFPS(TARGET_FPS);

    //SetWindowPosition(768/2,1024/2);

    RenderTexture2D backgroundTexture = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture2D modelTexture = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture2D foregroundTexture = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture2D shaderTexture = LoadRenderTexture(screenWidth, screenHeight);

    Shader shader = LoadShader(TextFormat("src/shaders/glsl%i/base-new.vs", GLSL_VERSION), TextFormat("src/shaders/glsl%i/fragment-new.fs", GLSL_VERSION));
 
    Vector4 normalizedFGColor = ColorNormalize(COLOR_FG);
    Vector4 normalizedBGColor = ColorNormalize(COLOR_BG);
    float edgeColor[3] = {normalizedFGColor.x, normalizedFGColor.y, normalizedFGColor.z};
    float backgroundColor[3] = {normalizedBGColor.x, normalizedBGColor.y, normalizedBGColor.z};
    float shaderResolution[2] = {300, 300};
    
    int edgeColorLoc = GetShaderLocation(shader, "edgeColor");
    int backgroundColorLoc = GetShaderLocation(shader, "backgroundColor");
    int resolutionLoc = GetShaderLocation(shader, "resolution");
    
    SetShaderValue(shader, edgeColorLoc, edgeColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, backgroundColorLoc, backgroundColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, resolutionLoc, shaderResolution, SHADER_UNIFORM_VEC2);

    int matNormalLoc = GetShaderLocation(shader, "matNormal");
    int renderModeLoc = GetShaderLocation(shader, "renderMode");

    Model model = LoadModel("src/models/model.obj");
    // assimp export model.stl model.obj
    Shader defaultShader = model.materials[0].shader;

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        SetShaderValue(shader, resolutionLoc, shaderResolution, SHADER_UNIFORM_VEC2);

         BeginTextureMode(backgroundTexture);
            ClearBackground(COLOR_BG);
            BeginMode3D(camera);
                DrawGrid(10,1.0f);
                model.materials[0].shader = defaultShader;
                DrawModel(model, Vector3Zero(), 1.0f, (Color){0,0,0,0});
            EndMode3D();
        EndTextureMode();

        BeginTextureMode(modelTexture);
            //ClearBackground((Color){0,0,0,0});
            ClearBackground(COLOR_BG);
            BeginMode3D(camera);
                model.materials[0].shader = shader;
                int modeGeometry = 1;
                SetShaderValue(shader, renderModeLoc, &modeGeometry, SHADER_UNIFORM_INT);
                DrawModel(model, Vector3Zero(), 1.0f, COLOR_BG);
            EndMode3D();
        EndTextureMode();

        BeginTextureMode(foregroundTexture);
            ClearBackground((Color){0,0,0,0});
            BeginMode3D(camera);
                DrawGrid(20,4.0f);
                model.materials[0].shader = defaultShader;
                DrawModel(model, Vector3Zero(), 1.0f, (Color){0,0,0,0});
                // DrawModelWires(model,Vector3Zero(),1.0f,COLOR_FG);
            EndMode3D();
        EndTextureMode();
        
        BeginTextureMode(shaderTexture);
            //DrawTexture(backgroundTexture.texture,0,0,WHITE);
            BeginShaderMode(shader);
                int modePostProcess = 2;
                SetShaderValue(shader, renderModeLoc, &modePostProcess, SHADER_UNIFORM_INT);
                DrawTexture(modelTexture.texture,0,0,WHITE);
            EndShaderMode();
            DrawTexture(foregroundTexture.texture,0,0,WHITE);
        EndTextureMode();

        BeginDrawing();
            DrawTexture(shaderTexture.texture,0,0,WHITE);
        EndDrawing();

    }

    Image outputImage = LoadImageFromTexture(shaderTexture.texture);
    ExportImage(outputImage, "output.png");

    UnloadShader(shader);

    return EXIT_SUCCESS;
}