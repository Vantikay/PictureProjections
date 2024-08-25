#include "raylib.h"
#include "raymath.h"
#include <exception>
using namespace std;

// Light data
typedef struct {
    int type;
    int enabled;
    Vector3 position;
    Vector3 target;
    Color color;
    float attenuation;
} Light;

// Light type
typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1
} LightType;

const int MAX_SHADERS = 8;

Vector4 getCol1(Matrix m) {
    return { m.m0, m.m1, m.m2, m.m3 };
}

Vector4 getCol2(Matrix m) {
    return { m.m4, m.m5, m.m6, m.m7 };
}

Vector4 getCol3(Matrix m) {
    return { m.m8, m.m9, m.m10, m.m11 };
}

Vector4 getCol4(Matrix m) {
    return { m.m12, m.m13, m.m14, m.m15 };
}

void RenderPortal(RenderTexture texture, Camera3D camera, Camera3D portalCamera, Vector3 portalPosition, Model model, Shader portalShader) {

    BeginTextureMode(texture);

        ClearBackground(RAYWHITE);

        Vector3 cameraOffset = Vector3Subtract(camera.position, portalPosition);
        portalCamera.position = Vector3Add(portalCamera.position, cameraOffset);
        Vector3 cameraDirection = Vector3Subtract(camera.target, camera.position);
        portalCamera.target = Vector3Add(portalCamera.position, cameraDirection);

        Matrix lookAtMatrix = MatrixLookAt(camera.position, camera.target, camera.up);

        SetShaderValueMatrix(portalShader, GetShaderLocation(portalShader, "lookAtMatrix"), lookAtMatrix);


        BeginMode3D(portalCamera);

            DrawModel(model, { 0.0f, 0.0f, 0.0f }, 10.0f, WHITE);

        EndMode3D();

    EndTextureMode();
}


int main(void)
{
    int screenWidth = 1500;
    int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Picture Projections Prototype");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = { 0.0f, 2.0f, 4.0f };
    camera.target = { 0.0f, 2.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();
    SetTargetFPS(60);

    Model gallery = LoadModel("../resources/gallery2.m3d");

    Model island = LoadModel("../resources/island2.m3d");
    island.transform = { 1.0f, 0.0f, 0.0f, 3.0f, 
                         0.0f, 1.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 1.0f, 0.0f, 
                         0.0f, 0.0f, 0.0f, 1.0f}; // adds 3 to x position

    Shader standardShader = LoadShader("../resources/shaders/lighting.vs", "../resources/shaders/lighting.fs");
    standardShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(standardShader, "viewPos");

    // currently only need this shader for adjusting texture coords
    Shader portalShader = LoadShader("../resources/shaders/lighting.vs", "../resources/shaders/portal.fs");
    portalShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(portalShader, "viewPos");

    
    Shader* shaders = new Shader[MAX_SHADERS];
    shaders[0] = standardShader;
    shaders[1] = portalShader;

    Light light;
    light.enabled = 1;
    light.type = LIGHT_DIRECTIONAL;
    light.position = { 7.0f, 10.0f, 5.0f };
    light.target = { 0.0f, 0.0f, 0.0f };
    light.color = GetColor(0xFFFFAAFF);


    // Send to shader light enabled state and type
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "lights[0].enabled"), &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "lights[0].type"), &light.type, SHADER_UNIFORM_INT);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "lights[0].enabled"), &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "lights[0].type"), &light.type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "lights[0].position"), position, SHADER_UNIFORM_VEC3);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "lights[0].position"), position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "lights[0].target"), target, SHADER_UNIFORM_VEC3);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "lights[0].target"), target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    float color[4] = { (float)light.color.r / (float)255, (float)light.color.g / (float)255,
                       (float)light.color.b / (float)255, (float)light.color.a / (float)255 };
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "lights[0].color"), color, SHADER_UNIFORM_VEC4);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "lights[0].color"), color, SHADER_UNIFORM_VEC4);

    float ambientColor[4] = { 0.8f, 1.0f, 1.0f, 1.0f };
    SetShaderValue(standardShader, GetShaderLocation(standardShader, "ambient"), ambientColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "ambient"), ambientColor, SHADER_UNIFORM_VEC4);

    Image transparentImage = GenImageColor(16, 16, GetColor(0x00000000));
    Texture2D transparentTexture = LoadTextureFromImage(transparentImage);
    RenderTexture islandPortalTexture = LoadRenderTexture(screenWidth * 2, screenHeight * 2);
    for (int i = 0; i < gallery.materialCount; i++) {
        gallery.materials[i].shader = standardShader;
        gallery.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = transparentTexture; // to make texture not effect render
    }
    gallery.materials[4].shader = portalShader;
    gallery.materials[4].maps[MATERIAL_MAP_DIFFUSE].texture = islandPortalTexture.texture;
    gallery.materials[4].maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00000000);
    
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "screenWidth"), &screenWidth, SHADER_UNIFORM_INT);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "screenHeight"), &screenHeight, SHADER_UNIFORM_INT);

    for (int i = 0; i < island.materialCount; i++) {
        island.materials[i].shader = standardShader;
        island.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = transparentTexture;
    }

    Mesh* islandPortalMeshPtr = 0;
    for (int i = 0; i < gallery.meshCount; i++) {
        if (gallery.meshMaterial[i] == 4) {
            islandPortalMeshPtr = &gallery.meshes[i];
            break;
        }
    }

    Model cubeModel = LoadModelFromMesh(GenMeshCube(5.0f, 5.0f, 5.0f));
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00000000);
    Image checkImage = GenImageChecked(256, 256, 64, 64, BLUE, GREEN);
    Texture2D checkTexture = LoadTextureFromImage(checkImage);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = checkTexture;
    cubeModel.materials[0].shader = standardShader;

    Camera3D islandCamera = camera;
    islandCamera.position = { 21.0f, 2.0f, 0.0f };
    islandCamera.target = { 25.0f, 2.0f, 0.0f };

    bool isPortal = false;
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "isPortal"), &isPortal, SHADER_UNIFORM_INT);
    SetShaderValue(portalShader, GetShaderLocation(portalShader, "fovy"), &camera.fovy, SHADER_UNIFORM_FLOAT);

    Matrix lookAtMatrix;

    Camera3D portalView = camera;
    portalView.position = { 2.75f, 2.0f, -4.0f };
    portalView.target = { 6.75f, 2.0f, -4.0f };


    // get average position to find portal center
    Vector3 islandPortalPosition = { 0.0f, 0.0f, 0.0f };
    float galleryScale = 10.0f;
    float* islandPortalVertices = islandPortalMeshPtr->vertices;
    for (int i = 0; i < 3 * islandPortalMeshPtr->vertexCount; i += 3) {
        islandPortalPosition.x += islandPortalVertices[i];
        islandPortalPosition.y += islandPortalVertices[i + 1];
        islandPortalPosition.z += islandPortalVertices[i + 2];
    }
    float portalScalar = galleryScale / islandPortalMeshPtr->vertexCount;
    islandPortalPosition.x *= portalScalar;
    islandPortalPosition.y *= portalScalar;
    islandPortalPosition.z *= portalScalar;

    int portalViewMoveSteps = 10;
    bool movingToPortalView = false;
    Vector3 portalViewMoveStartPosition;
    Vector3 portalViewMoveStartTarget;


    // render once to get picture
    RenderPortal(islandPortalTexture, portalView, islandCamera, islandPortalPosition, island, portalShader);

    bool renderedFirstPortal = false;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {


        

        // Camera PRO usage example (EXPERIMENTAL)
        // This new camera function allows custom movement/rotation values to be directly provided
        // as input parameters, with this approach, rcamera module is internally independent of raylib inputs
        if (!movingToPortalView) {
            UpdateCameraPro(&camera,
                {
                    (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 0.1f -      // Move forward-backward
                    (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,
                    (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f -   // Move right-left
                    (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
                    0.0f                                                // Move up-down
                },
            {
                GetMouseDelta().x * 0.05f,                            // Rotation: yaw
                GetMouseDelta().y * 0.05f,                            // Rotation: pitch
                0.0f                                                // Rotation: roll
            },
                GetMouseWheelMove() * 2.0f);
        }
                                   // Move to target (zoom)

        if (!movingToPortalView && IsKeyPressed(KEY_SPACE)) {
            islandCamera = camera;
            // Vector3 portalCameraOffset = { 5.0f, 0.0f, 0.0f };
            // islandCamera.position = Vector3Add(islandCamera.position, portalCameraOffset);
        }

        if (!movingToPortalView && IsKeyPressed(KEY_P)) {
            isPortal = !isPortal;
        }

        if (!movingToPortalView && IsKeyPressed(KEY_I)) {
            portalViewMoveStartPosition = camera.position;
            portalViewMoveStartTarget = camera.target;
            movingToPortalView = true;
        }
        
        if (movingToPortalView) {
            Vector3 positionStep = Vector3Subtract(portalView.position, portalViewMoveStartPosition);
            positionStep = Vector3Scale(positionStep, 1.0f / portalViewMoveSteps);
            camera.position = Vector3Add(camera.position, positionStep);
            
            Vector3 targetStep = Vector3Subtract(portalView.target, portalViewMoveStartTarget);
            targetStep = Vector3Scale(targetStep, 1.0f / portalViewMoveSteps);
            camera.target = Vector3Add(camera.target, targetStep);

            if (Vector3Distance(camera.position, portalView.position) < 0.001f) {
                camera.position = portalView.position;
                movingToPortalView = false;
            }
        }

        if (IsKeyPressed(KEY_Y)) {
            float cameraDistance = Vector3Length(Vector3Subtract(camera.position, islandPortalPosition));
            TraceLog(LOG_INFO, TextFormat("Distance of main camera to portal: %f", cameraDistance));
            TraceLog(LOG_INFO, TextFormat("Portal Position: (%f, %f, %f)", islandPortalPosition.x, islandPortalPosition.y, islandPortalPosition.z));
            TraceLog(LOG_INFO, TextFormat("Main Camera Position: (%f, %f, %f)", camera.position.x, camera.position.y, camera.position.z));
            TraceLog(LOG_INFO, TextFormat("Main Camera Target: (%f, %f, %f)", camera.target.x, camera.target.y, camera.target.z));
            TraceLog(LOG_INFO, TextFormat("Portal View Camera Position: (%f, %f, %f)", portalView.position.x, portalView.position.y, portalView.position.z));
            TraceLog(LOG_INFO, TextFormat("Portal View Camera Target: (%f, %f, %f)", portalView.target.x, portalView.target.y, portalView.target.z));
            TraceLog(LOG_INFO, TextFormat("Island Camera Position: (%f, %f, %f)", islandCamera.position.x, islandCamera.position.y, islandCamera.position.z));
            TraceLog(LOG_INFO, TextFormat("Island Camera Target: (%f, %f, %f)", islandCamera.target.x, islandCamera.target.y, islandCamera.target.z));

        }

        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(standardShader, standardShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        
        if (isPortal) {
            RenderPortal(islandPortalTexture, camera, islandCamera, islandPortalPosition, island, portalShader);
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
       
                    DrawModel(gallery, { 0.0f, 0.0f, 0.0f }, galleryScale, WHITE);
                    DrawModel(island, { 0.0f, 0.0f, 0.0f }, 10.0f, WHITE);
                    // DrawModel(cubeModel, { 10.0f, 0.0f, 10.0f }, 1.0f, WHITE);

            EndMode3D();
            /*
            DrawTexturePro(islandPortalTexture.texture, 
                            { 0.0f, 0.0f, (float)screenWidth, -(float)screenHeight }, 
                            { 0.0f, 0.0f, screenWidth * 0.2f, screenHeight * 0.2f }, 
                            { 0.0f, 0.0f }, 
                            0.0f, 
                            WHITE);
            */

        EndDrawing();
    }

    UnloadModel(gallery);
    UnloadModel(island);
    UnloadModel(cubeModel);
    UnloadShader(standardShader);
    delete[] shaders;
    UnloadShader(portalShader);
    UnloadTexture(checkTexture);
    UnloadTexture(transparentTexture);
    UnloadRenderTexture(islandPortalTexture);
    CloseWindow(); 


    return 0;
}

