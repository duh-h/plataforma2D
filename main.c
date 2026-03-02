#include "raylib.h"
#include "raymath.h"

// --- TODO: ---
// fazer pacote de textura
// remover ajustes manuais
// procurar e carregar texturas de cenario

// --- Definições de Física ---
#define G 750
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f

// --- Estruturas ---

typedef enum
{
    TIPO_CHAO,
    TIPO_PLATAFORMA,
    TIPO_PAREDE
} ItemType;

typedef struct Player
{
    Vector2 position;
    float speed;
    bool canJump;

    Texture2D texture;
    Rectangle frameRec;
    int currentFrame;
    int framesCounter;
    int framesSpeed;
    bool isFacingRight;
} Player;

typedef struct EnvItem
{
    Rectangle rect;
    int blocking;
    Color color;
    ItemType type;

} EnvItem;

// --- Declaração de Funções ---
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

int main(void)
{
    // Inicialização
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Raylib - Plataforma2D");

    // Configuração do Jogador e Carregamento de Textura
    Player player = {0};
    player.position = (Vector2){400, 280};
    player.speed = 0;
    player.canJump = false;

    Texture2D background = LoadTexture("Assets 1024 Cave/Square - Black.jpg");

    Texture2D plataforma = LoadTexture("Assets 1024 Cave/Cave - Floor.png");

    Texture2D pinkRun = LoadTexture("1 Pink_Monster/Pink_Monster_Run_6.png");
    Texture2D pinkIdle = LoadTexture("1 Pink_Monster/Pink_Monster_Idle_4.png");
    Texture2D pinkJump = LoadTexture("1 Pink_Monster/Pink_Monster_Jump_8.png");

    Texture2D owletRun = LoadTexture("2 Owlet_Monster/Owlet_Monster_Run_6.png");
    Texture2D owletIdle = LoadTexture("2 Owlet_Monster/Owlet_Monster_Idle_4.png");
    Texture2D owletJump = LoadTexture("2 Owlet_Monster/Owlet_Monster_Jump_8.png");

    Texture2D dudeRun = LoadTexture("3 Dude_Monster/Dude_Monster_Run_6.png");
    Texture2D dudeIdle = LoadTexture("3 Dude_Monster/Dude_Monster_Idle_4.png");
    Texture2D dudeJump = LoadTexture("3 Dude_Monster/Dude_Monster_Jump_8.png");

    Texture2D texRun = pinkRun;
    Texture2D texIdle = pinkIdle;
    Texture2D texJump = pinkJump;

    player.texture = texIdle;
    player.currentFrame = 0;
    player.framesCounter = 0;
    player.framesSpeed = 10;

    // Configuracoes de recorte (1/6 da largura da sprite) (Ajustado manualmente mais abaixo)
    player.frameRec = (Rectangle){0.0f, 0.0f, (float)player.texture.width / 6, (float)player.texture.height};

    // Itens do Cenário
    EnvItem envItems[42] = {
        // {x(maior mais a esquerda),y(menor mais a cima),largura,altura}
        {{0, 0, 1000, 400}, 0, LIGHTGRAY},               // tela
        {{0, -100, 30, 600}, 1, GRAY},                   // parede
        {{0, 400, 2000, 200}, 1, GRAY},                  // chao
        {{300, 250, 400, 10}, 1, GRAY},                  // plataforma
        {{250, 325, 100, 10}, 1, GRAY, TIPO_PLATAFORMA}, // plataforma
        {{650, 325, 100, 10}, 1, GRAY, TIPO_PLATAFORMA}, // plataforma
        {{170, 370, 30, 30}, 1, DARKGRAY}};              // bloco
    int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

    for (int i = 7; i < 42; i++)
    {
        if (i <= 22)
        {
            if (i % 2 == 0)
            {
                float novaplataformaX = 650 + ((i - 6) * 200);

                envItems[i] = (EnvItem){{novaplataformaX, 325, 100, 10}, 1, GRAY, TIPO_PLATAFORMA};
            }
            else
            {

                float novapedraX = 675 + ((i - 6) * 200);

                envItems[i] = (EnvItem){{novapedraX, 370, 30, 30}, 1, DARKGRAY};
            };
        }
        else
        {

            float novapedraX = 675 + ((i - 6) * 200);

            envItems[i] = (EnvItem){{novapedraX, 370, 30, 30}, 1, DARKGRAY};
        }
    }

    // Configuração da Câmera
    Camera2D camera = {0};
    camera.target = player.position;
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Array de ponteiros para funções de câmera (Mantido do original)
    void (*cameraUpdaters[])(Camera2D *, Player *, EnvItem *, int, float, int, int) = {
        UpdateCameraCenter,
        UpdateCameraCenterInsideMap,
        UpdateCameraCenterSmoothFollow,
        UpdateCameraEvenOutOnLanding,
        UpdateCameraPlayerBoundsPush};
    int cameraOption = 2;
    int cameraUpdatersLength = sizeof(cameraUpdaters) / sizeof(cameraUpdaters[0]);

    // char *cameraDescriptions[] = {
    //   "null"};

    SetTargetFPS(60);

    // Loop Principal
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // Atualiza Física
        UpdatePlayer(&player, envItems, envItemsLength, deltaTime);

        // Atualizando valores de divisao para diferetes sprites
        int divisor = 4; // Valor inicial (sprite idle)

        if (player.texture.id == texRun.id)
        {
            divisor = 6;
        }
        else if (player.texture.id == texJump.id)
        {
            divisor = 8;
        }
        else
        {
            divisor = 4; // Idle denovo
        }

        // Ordem de preferencia dos movomentos + interacoes com teclas precionadas ()
        if (!player.canJump) // pulo
        {
            if (player.texture.id != texJump.id)
            {
                player.texture = texJump;
                player.currentFrame = 0;
                player.framesCounter = 0;
            }

            player.framesCounter++;
            if (player.framesCounter >= (60 / player.framesSpeed))
            {
                player.framesCounter = 0;
                player.currentFrame++;
                if (player.currentFrame > 7)
                    player.currentFrame = 0;
            }
        }
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) // corrida
        {
            if (player.texture.id != texRun.id)
            {
                player.texture = texRun;
                player.currentFrame = 0;
                player.framesCounter = 0;
            }

            player.framesCounter++;
            if (player.framesCounter >= (60 / player.framesSpeed))
            {
                player.framesCounter = 0;
                player.currentFrame++;
                if (player.currentFrame > 5)
                    player.currentFrame = 0;
            }
        }
        else // parado
        {
            if (player.texture.id != texIdle.id)
            {
                player.texture = texIdle;
                player.currentFrame = 0;
                player.framesCounter = 0;
            }

            player.framesCounter++;
            if (player.framesCounter >= (60 / player.framesSpeed))
            {
                player.framesCounter = 0;
                player.currentFrame++;
                if (player.currentFrame > 3)
                    player.currentFrame = 0;
            }
        }

        // Atualizando valores de divisao para diferetes sprites
        divisor = 4;
        if (player.texture.id == texRun.id)
            divisor = 6;
        else if (player.texture.id == texJump.id)
            divisor = 8;

        // Isso garante que a matemática sempre use o divisor e a textura corretos do momento
        float larguraFrame = (float)player.texture.width / divisor;
        player.frameRec.x = (float)player.currentFrame * larguraFrame;
        player.frameRec.height = (float)player.texture.height;

        // Atualizar a largura aqui também (evita bugs de flip)
        if (player.isFacingRight)
            player.frameRec.width = larguraFrame;
        else
            player.frameRec.width = -larguraFrame;

        // Ajustes de zoom e camera
        camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
        if (camera.zoom > 3.0f)
            camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f)
            camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2){400, 280};
        }
        if (IsKeyPressed(KEY_E))
        {
            if (texIdle.id == pinkIdle.id)
            {
                texRun = owletRun;
                texIdle = owletIdle;
                texJump = owletJump;
            }
            else if (texIdle.id == owletIdle.id)
            {
                texRun = dudeRun;
                texIdle = dudeIdle;
                texJump = dudeJump;
            }
            else if (texIdle.id == dudeIdle.id)
            {
                texRun = pinkRun;
                texIdle = pinkIdle;
                texJump = pinkJump;
            }
        }
        if (IsKeyPressed(KEY_C))
            cameraOption = (cameraOption + 1) % cameraUpdatersLength;

        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

        //  Desenho
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        ClearBackground(SKYBLUE);
        // DrawTexture(background, 0, 0, WHITE);
        // DrawTextureEx(background, (Vector2){0, 0}, 0.0f, 2.0f, WHITE);

        BeginMode2D(camera);

        // Desenha o cenario
        for (int i = 0; i < envItemsLength; i++)
        {
            EnvItem *ei = envItems + i;

            if (ei->type == TIPO_PLATAFORMA)
            {

                float margemX = 60.0f;
                float margemY = 0.0f;

                
                Rectangle recortePlataforma = {
                    0.0f + margemX,
                    0.0f + margemY,
                    430.0f,
                    136.0f
                };

                Rectangle destRec = {ei->rect.x-20, ei->rect.y-25, ei->rect.width+40, ei->rect.height+30};

                Vector2 origin = {0.0f, 0.0f};

                DrawTexturePro(plataforma, recortePlataforma, destRec, origin, 0.0f, WHITE);

                // DrawTextureRec(plataforma, recortePlataforma ,(Vector2){ei->rect.x-18, ei->rect.y}, WHITE);
            }
            else
            {

                DrawRectangleRec(ei->rect, ei->color);
            }
        }
        // DEBUG: Desenha o retângulo de colisão para saber onde esta a física
        // Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40.0f, 40.0f };
        // DrawRectangleRec(playerRect, Fade(RED, 0.5f));

        // player.position é o centro-baixo do player
        Vector2 drawPos = {player.position.x - 30.0f, player.position.y - 75.0f}; // 30.0f e 75.0f sao ajustes manuais

        // Correcao porque o sprite tem um pouco de espaço vazio embaixo
        drawPos.y += 15;

        // Correcao para certos sprites
        if (player.texture.id == texJump.id)
        {
            drawPos.x += 0.0f;
        }
        // Funcao para sprite (modificada para almentar o tamanho dos sprites)
        DrawTexturePro(
            player.texture,
            player.frameRec,
            (Rectangle){drawPos.x, drawPos.y, 60.0f, 60.0f},
            (Vector2){0, 0},
            0.0f,
            WHITE);

        EndMode2D();

        // UI
        DrawText("Controls:", 20, 20, 10, BLACK);
        DrawText("Controls: E muda personagem", 40, 40, 10, DARKGRAY);
        DrawText("Controls R reseta", 40, 60, 10, DARKGRAY);
        DrawText(" C pra mudar a camera mode", 40, 120, 10, DARKGRAY);
        DrawText("camera mode:", 20, 140, 10, BLACK);
        // DrawText(cameraDescriptions[cameraOption], 40, 160, 10, DARKGRAY);

        EndDrawing();
    }

    // Descarregar textura

    UnloadTexture(background);
    UnloadTexture(plataforma);

    UnloadTexture(player.texture);

    UnloadTexture(pinkRun);
    UnloadTexture(pinkIdle);
    UnloadTexture(pinkJump);

    UnloadTexture(owletRun);
    UnloadTexture(owletIdle);
    UnloadTexture(owletJump);

    UnloadTexture(dudeRun);
    UnloadTexture(dudeIdle);
    UnloadTexture(dudeJump);
    CloseWindow();

    return 0;
}

// Funções Auxiliares (Física e Câmera)

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta)
{
    if (IsKeyDown(KEY_LEFT))
    {
        player->position.x -= PLAYER_HOR_SPD * delta;
        player->isFacingRight = false; // Define que está olhando para a esquerda
    }
    if (IsKeyDown(KEY_RIGHT))
    {
        player->position.x += PLAYER_HOR_SPD * delta;
        player->isFacingRight = true; // Define que está olhando para a direita
    }
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    float larguraColisao = 40.0f;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);

        if (ei->blocking &&
            // Verifica se a borda DIREITA do player passou do início do bloco
            ei->rect.x <= (p->x + larguraColisao / 2) &&
            // Verifica se a borda ESQUERDA do player ainda está antes do fim do bloco
            ei->rect.x + ei->rect.width >= (p->x - larguraColisao / 2) &&

            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed * delta)
        {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y;
            break;
        }
    }
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        if (!ei->blocking)
            continue;

        // Retângulo de colisão do corpo
        Rectangle playerRect = {player->position.x - 15, player->position.y - 55, 30, 50};

        if (CheckCollisionRecs(playerRect, ei->rect))
        {
            // Ignora o chão onde o pé está pisando
            if (ei->rect.y < player->position.y - 15)
            {

                if (ei->rect.height > 15)
                {
                    // Removemos a regra do speed
                    if (IsKeyDown(KEY_RIGHT))
                        player->position.x = ei->rect.x - 16;
                    else if (IsKeyDown(KEY_LEFT))
                        player->position.x = ei->rect.x + ei->rect.width + 16;
                }
            }
        }
    }
    if (!hitObstacle)
    {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    }
    else
        player->canJump = true;
}

// funções de câmera (apenas leem a posição do jogador)
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->offset = (Vector2){width / 2.0f, height / 2.0f};
    camera->target = player->position;
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){width / 2.0f, height / 2.0f};
    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){maxX, maxY}, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){minX, minY}, *camera);

    if (max.x < width)
        camera->offset.x = width - (max.x - (float)width / 2);
    if (max.y < height)
        camera->offset.y = height - (max.y - (float)height / 2);
    if (min.x > 0)
        camera->offset.x = (float)width / 2 - min.x;
    if (min.y > 0)
        camera->offset.y = (float)height / 2 - min.y;
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static float minSpeed = 30;
    static float minEffectLength = 10;
    static float fractionSpeed = 0.8f;

    camera->offset = (Vector2){width / 2.0f, height / 2.0f};
    Vector2 diff = Vector2Subtract(player->position, camera->target);
    float length = Vector2Length(diff);

    if (length > minEffectLength)
    {
        float speed = fmaxf(fractionSpeed * length, minSpeed);
        camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed * delta / length));
    }
}

void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static float evenOutSpeed = 700;
    static int eveningOut = false;
    static float evenOutTarget;

    camera->offset = (Vector2){width / 2.0f, height / 2.0f};
    camera->target.x = player->position.x;

    if (eveningOut)
    {
        if (evenOutTarget > camera->target.y)
        {
            camera->target.y += evenOutSpeed * delta;

            if (camera->target.y > evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
        else
        {
            camera->target.y -= evenOutSpeed * delta;

            if (camera->target.y < evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
    }
    else
    {
        if (player->canJump && (player->speed == 0) && (player->position.y != camera->target.y))
        {
            eveningOut = 1;
            evenOutTarget = player->position.y;
        }
    }
}

void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static Vector2 bbox = {0.2f, 0.2f};

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height}, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){(1 + bbox.x) * 0.5f * width, (1 + bbox.y) * 0.5f * height}, *camera);
    camera->offset = (Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height};

    if (player->position.x < bboxWorldMin.x)
        camera->target.x = player->position.x;
    if (player->position.y < bboxWorldMin.y)
        camera->target.y = player->position.y;
    if (player->position.x > bboxWorldMax.x)
        camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x);
    if (player->position.y > bboxWorldMax.y)
        camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}