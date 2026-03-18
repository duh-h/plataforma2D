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
#define MAX_PEDRAS 4

// --- Estruturas ---

typedef enum
{
    TIPO_CHAO,
    TIPO_PLATAFORMA_PEDRA,
    TIPO_PLATAFORMA,
    TIPO_TRIANGULO_PRA_CIMA,
    TIPO_PAREDE
} ItemType;

typedef struct
{
    Rectangle rect;
    float speed;
    bool active;
} FallingBlock;

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

typedef enum GameScreen
{
    TELA_INICIAL,
    JOGO
} GameScreen;

// --- Declaração de Funções ---
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, FallingBlock *pedras, float delta);
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
    //  Camera2D auste

    InitWindow(screenWidth, screenHeight, "Raylib - Plataforma2D");

    InitAudioDevice();

    Music music = LoadMusicStream("musics/pink_-_glorytothemachine.mp3");
    Music musicMenu = LoadMusicStream("musics/sybil_system_-_glorytothemachine.mp3");

    PlayMusicStream(musicMenu);

    float timePlayed = 0.5f; // Time played normalized [0.0f..1.0f]

    float pan = 0.5f; // Default audio pan center [-1.0f..1.0f]
    SetMusicPan(music, pan);

    float volume = 1.0f; // Default audio volume [0.0f..1.0f]
    SetMusicVolume(music, volume);

    GameScreen telaAtual = TELA_INICIAL;

    // Configuração do Jogador e Carregamento de Textura
    Player player = {0};
    player.position = (Vector2){400, 280};
    player.speed = 0;
    player.canJump = false;

    Texture2D background = LoadTexture("PixelFantasy_Caves_1.0/background4a.png");
    Texture2D backgroundFundo1 = LoadTexture("PixelFantasy_Caves_1.0/background3.png");
    Texture2D backgroundFundo2 = LoadTexture("PixelFantasy_Caves_1.0/background2.png");
    Texture2D backgroundFundo3 = LoadTexture("PixelFantasy_Caves_1.0/background1.png");
    Texture2D fundoMenu = LoadTexture("fundoTelaInicial.png");

    Texture2D plataforma = LoadTexture("Assets 1024 Cave/Cave - Floor.png");
    Texture2D plataformaTriangulo = LoadTexture("Assets 1024 Cave/Cave - SmallRocks.png");
    Texture2D plataformaTrianguloCaindo = LoadTexture("Assets 1024 Cave/Cave - SmallRocks.png");
    Texture2D plataformaPedra = LoadTexture("Assets 1024 Cave/Cave - Platforms.png");
    Texture2D plataformaChao = LoadTexture("Assets 1024 Cave/Cave - Platforms.png");

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

    FallingBlock pedras[MAX_PEDRAS];

    
    for (int i = 0; i < MAX_PEDRAS; i++)
    {
        pedras[i].active = true;
        pedras[i].speed = (float)GetRandomValue(200, 400);
        pedras[i].rect = (Rectangle){(float)GetRandomValue(200, 18000), (float)GetRandomValue(-500, -100), 16, 35};
    }
    

    EnvItem envItems[600];
    int contador = 0;
    float refX = 0.0f;

    // --- SEÇÃO 1: A BASE SEGURA (Chão Inicial) ---
    envItems[contador++] = (EnvItem){{-50, 0, 50, 800}, 1, BLANK, TIPO_PAREDE};
    envItems[contador++] = (EnvItem){{refX, 400, 1000, 400}, 1, DARKGRAY, TIPO_CHAO};
    refX += 1000.0f;

    // --- SEÇÃO 2: A ESCADA (Aquecimento) ---
    refX += 120.0f;
    for (int i = 0; i < 4; i++)
    {
        float alturaY = (i % 2 == 0) ? 340.0f : 280.0f;
        envItems[contador++] = (EnvItem){{refX, alturaY, 100, 10}, 1, GRAY, TIPO_PLATAFORMA};

        refX += 190.0f;
        envItems[contador++] = (EnvItem){{refX, alturaY - 20, 30, 30}, 1, DARKGRAY, TIPO_PLATAFORMA_PEDRA};
        refX += 150.0f;
    }

    // --- SEÇÃO 3: O GRANDE CAMPO MINADO (Mais longo e muito mais cruel) ---
    refX += 120.0f;

    // Aumentamos o chão de 2500 para 4000 pixels!
    envItems[contador++] = (EnvItem){{refX, 400, 4000, 400}, 1, DARKGRAY, TIPO_CHAO};
    float posDesafio = refX + 250.0f;

    // Agora são 11 desafios seguidos, com espinhos em TODO LUGAR
    for (int i = 0; i < 11; i++)
    {
        // 4 espinhos no chão formando um fosso mortal
        envItems[contador++] = (EnvItem){{posDesafio, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};
        envItems[contador++] = (EnvItem){{posDesafio + 20, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};
        envItems[contador++] = (EnvItem){{posDesafio + 40, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};
        envItems[contador++] = (EnvItem){{posDesafio + 60, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};

        // Plataforma para pular o fosso (agora mais baixa: Y=340, pulo fácil)
        envItems[contador++] = (EnvItem){{posDesafio - 30, 340, 120, 10}, 1, GRAY, TIPO_PLATAFORMA};

        // ESPINHO EM CIMA DA PLATAFORMA! (O jogador tem que pular rápido)
        envItems[contador++] = (EnvItem){{posDesafio + 20, 305, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};

        // Bloco flutuante para aterrissar depois da plataforma
        envItems[contador++] = (EnvItem){{posDesafio + 120, 270, 30, 30}, 1, DARKGRAY, TIPO_PLATAFORMA_PEDRA};

        // ESPINHO LOGO DEPOIS DO BLOCO (Para evitar que o jogador caia direto)
        envItems[contador++] = (EnvItem){{posDesafio + 160, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};

        posDesafio += 360.0f; // Espaçamento
    }
    refX += 4000.0f;

    // --- SEÇÃO 4: PARKOUR NO VAZIO (Fase expandida) ---
    refX += 160.0f;
    float alturaParkour = 350.0f;

    for (int i = 0; i < 12; i++)
    {
        envItems[contador++] = (EnvItem){{refX, alturaParkour, 30, 30}, 1, DARKGRAY, TIPO_PLATAFORMA_PEDRA};

        // Colocamos um espinho em cima de CADA pedra ímpar para o jogador ter que desviar no ar

        refX += 140.0f;

        if (i % 2 == 0)
            alturaParkour -= 40.0f;
        else
            alturaParkour += 20.0f;
    }

    // --- SEÇÃO 5: ZIGUEZAGUE DAS PLATAFORMAS ---
    refX += 60.0f;
    envItems[contador++] = (EnvItem){{refX, 400, 800, 400}, 1, DARKGRAY, TIPO_CHAO};
    refX += 800.0f + 40.0f;

    // Aumentado para 8 plataformas flutuantes
    for (int i = 0; i < 8; i++)
    {
        envItems[contador++] = (EnvItem){{refX, 320, 100, 10}, 1, GRAY, TIPO_PLATAFORMA};
        envItems[contador++] = (EnvItem){{refX + 40, 285, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};
        refX += 200.0f;
    }

    // --- SEÇÃO 6: NOVO DESAFIO! (O Corredor dos Saltos Curtos) ---
    refX += 100.0f;
    envItems[contador++] = (EnvItem){{refX, 400, 3000, 400}, 1, DARKGRAY, TIPO_CHAO};

    float posCorredor = refX + 150.0f;

    // O jogador corre no chão plano, mas tem que pular de bloquinho em bloquinho no chão entre vários espinhos
    for (int i = 0; i < 15; i++)
    {
        // Bloco no chão para o jogador pisar (Y=370, já que o chão é 400 e o bloco tem 30)
        envItems[contador++] = (EnvItem){{posCorredor, 370, 30, 30}, 1, DARKGRAY, TIPO_PLATAFORMA_PEDRA};

        // Espinhos imprensando o bloco
        envItems[contador++] = (EnvItem){{posCorredor + 40, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};
        envItems[contador++] = (EnvItem){{posCorredor + 60, 365, 16, 35}, 0, RED, TIPO_TRIANGULO_PRA_CIMA};

        posCorredor += 180.0f;
    }
    refX += 3000.0f;

    // --- SEÇÃO 7: CHÃO FINAL E LINHA DE CHEGADA ---
    refX += 100.0f;
    envItems[contador++] = (EnvItem){{refX, 400, 1000, 400}, 1, DARKGRAY, TIPO_CHAO};
    refX += 1000.0f;
    envItems[contador++] = (EnvItem){{refX, -200, 50, 800}, 1, BLANK, TIPO_PAREDE};

    // Atualiza a quantidade real de blocos gerados
    int envItemsLength = contador;

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

    SetTargetFPS(60);

    // Loop Principal
    while (!WindowShouldClose())
    {

        float deltaTime = GetFrameTime();

        if (telaAtual == TELA_INICIAL)
        {
            UpdateMusicStream(musicMenu);

            float btnWidth = 200.0f;
            float btnHeight = 60.0f;
            float btnX = screenWidth / 2.0f - (btnWidth / 2.0f);
            float btnY = screenHeight / 2.0f - 50.0f;

            Rectangle btnJogar = {btnX, btnY, btnWidth, btnHeight};

            Vector2 mousePos = GetMousePosition();

            if (CheckCollisionPointRec(mousePos, btnJogar))
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {

                    telaAtual = JOGO;
                    StopMusicStream(musicMenu);
                    PlayMusicStream(music);
                }
            }
        }
        else if (telaAtual == JOGO)
        {
            
            UpdateMusicStream(music);
            // Física Pedra caindo
            UpdatePlayer(&player, envItems, envItemsLength, pedras, deltaTime);
            for (int i = 0; i < MAX_PEDRAS; i++)
            {
                if (pedras[i].active)
                {
                    
                    pedras[i].rect.y += pedras[i].speed * deltaTime;

                    // Se a pedra cair na tela e passar muito do jogador (errou o alvo)
                    if (pedras[i].rect.y > player.position.y + 600)
                    {
                        // Joga a pedra lá pro alto
                        pedras[i].rect.y = player.position.y - (float)GetRandomValue(600, 800);
                        
                        // Persegue o X atual do jogador!
                        pedras[i].rect.x = player.position.x + (float)GetRandomValue(0, 240);
                    }
                }
            }
            
        }
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

        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight / 0.65f);

        timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);

        if (timePlayed > 1.0f)
            timePlayed = 1.0f;

        //  Desenho
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        ClearBackground(DARKGRAY);

        if (telaAtual == TELA_INICIAL)
        {
            DrawTexturePro(fundoMenu,
                           (Rectangle){0, 0, (float)fundoMenu.width, (float)fundoMenu.height},
                           (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                           (Vector2){0, 0}, 0.0f, WHITE);

            float btnWidth = 200.0f;
            float btnHeight = 60.0f;

            float btnX = screenWidth / 2.0f - (btnWidth / 2.0f);
            float btnY = screenHeight / 2.0f - 50.0f; // Subimos o botão

            Rectangle btnJogar = {btnX, btnY, btnWidth, btnHeight};

            Vector2 mousePos = GetMousePosition();
            bool isHovering = CheckCollisionPointRec(mousePos, btnJogar);

            // --- Desenho ---

            // Opacidade: Usamos Fade() para definir o nível de transparência (0.0f a 1.0f)
            // 0.7f significa 70% opaco, 30% transparente
            Color corFundo = isHovering ? Fade(LIGHTGRAY, 0.7f) : Fade(GRAY, 0.7f);

            DrawRectangleRec(btnJogar, corFundo);
            DrawRectangleLinesEx(btnJogar, 4.0f, BLACK);

            // --- Texto (ajustado para caber no botão menor) ---
            int fontSize = 30; // Diminuímos a fonte também para combinar
            int textWidth = MeasureText("JOGAR", fontSize);

            // Centraliza o texto dentro do botão
            float textX = btnX + (btnWidth / 2.0f) - (textWidth / 2.0f);
            float textY = btnY + (btnHeight / 2.0f) - (fontSize / 2.0f); // Centralização vertical

            DrawText("JOGAR", (int)textX, (int)textY, fontSize, BLACK);
        }
        else if (telaAtual == JOGO)
        {

            // DrawTexture(backgroundFundo3, 0, 0, WHITE);

            float screenW = (float)GetScreenWidth();
            float screenH = (float)GetScreenHeight();

            Rectangle dest = {0, 0, screenW, screenH};

            Rectangle src3 = {camera.target.x * 0.05f, 0, (float)backgroundFundo3.width, (float)backgroundFundo3.height};
            DrawTexturePro(backgroundFundo3, src3, dest, (Vector2){0, 0}, 0.0f, WHITE);

            Rectangle src2 = {camera.target.x * 0.15f, 0, (float)backgroundFundo2.width, (float)backgroundFundo2.height};
            DrawTexturePro(backgroundFundo2, src2, dest, (Vector2){0, 0}, 0.0f, WHITE);

            Rectangle src1 = {camera.target.x * 0.30f, 0, (float)backgroundFundo1.width, (float)backgroundFundo1.height};
            DrawTexturePro(backgroundFundo1, src1, dest, (Vector2){0, 0}, 0.0f, WHITE);

            Rectangle src0 = {camera.target.x * 0.40f, 0, (float)backgroundFundo1.width, (float)backgroundFundo1.height};
            DrawTexturePro(background, src0, dest, (Vector2){0, 0}, 0.0f, WHITE);

            BeginMode2D(camera);

            // Desenha o cenario

            // Desenha pedra caindo
            for (int i = 0; i < MAX_PEDRAS; i++)
            {
                if (pedras[i].active)
                {
                    float margemX = 1824.0f;
                    float margemY = 60.0f;

                    Rectangle recortePedraCaindo = {
                        0.0f + margemX,
                        0.0f + margemY,
                        175.0f,
                        260.0f};

                    Rectangle destRec = {
                        pedras[i].rect.x - 5,
                        pedras[i].rect.y + 5,
                        pedras[i].rect.width + 10,
                        pedras[i].rect.height + 2};

                    Vector2 origin = {0.0f, 0.0f};

                    DrawTexturePro(plataformaTrianguloCaindo, recortePedraCaindo, destRec, origin, 0.0f, WHITE);
                    // DrawRectangleRec(pedra.rect, RED);
                }
            }
            for (int i = 0; i < envItemsLength; i++)
            {
                EnvItem *ei = envItems + i;

                if (ei->type == TIPO_PLATAFORMA)
                {
                    /*
                    Explicaçao do recorte
                    A imagem fonte esta em 2048x2048
                    Localize a figura desejada e pegar desde o ponto zero ate a figura total
                    Ex: um recorte de uma pedra no meio contendo ela esta 1024x1024
                        se pedra tem tamanho 100x100
                        margemX = 1024-100 = 924
                        margemY = 924
                        h = 100
                        w = 100
                    */

                    float margemX = 60.0f;
                    float margemY = 0.0f;

                    Rectangle recortePlataforma = {
                        0.0f + margemX,
                        0.0f + margemY,
                        430.0f,
                        136.0f};

                    Rectangle destRec = {ei->rect.x - 20, ei->rect.y - 25, ei->rect.width + 40, ei->rect.height + 30};

                    Vector2 origin = {0.0f, 0.0f};

                    DrawTexturePro(plataforma, recortePlataforma, destRec, origin, 0.0f, WHITE);

                    // DrawTextureRec(plataforma, recortePlataforma ,(Vector2){ei->rect.x-18, ei->rect.y}, WHITE);
                }
                else if (ei->type == TIPO_PLATAFORMA_PEDRA)
                {

                    float margemX = 216.0f;
                    float margemY = 216.0f;

                    // 1090x 430 / 920 x 300

                    Rectangle recortePedra = {
                        0.0f + margemX,
                        0.0f + margemY,
                        196.0f,
                        196.0f};

                    Rectangle destRec = {ei->rect.x - 5, ei->rect.y - 2, ei->rect.width + 10, ei->rect.height + 2};

                    Vector2 origin = {0.0f, 0.0f};

                    DrawTexturePro(plataformaPedra, recortePedra, destRec, origin, 0.0f, WHITE);
                }
                else if (ei->type == TIPO_TRIANGULO_PRA_CIMA)
                {

                    float margemX = 1600.0f;
                    float margemY = 1150.0f;

                    Rectangle recortePedra = {
                        0.0f + margemX,
                        0.0f + margemY,
                        200.0f,
                        250.0f};

                    Rectangle destRec = {ei->rect.x - 5, ei->rect.y - 2, ei->rect.width + 10, ei->rect.height + 2};

                    Vector2 origin = {0.0f, 0.0f};

                    DrawTexturePro(plataformaTriangulo, recortePedra, destRec, origin, 0.0f, WHITE);
                }
                else if (ei->type == TIPO_CHAO)
                {

                    // 390x190
                    // 624x212

                    float margemX = 234.0f;
                    float margemY = 24.0f;

                    Rectangle recortePedra = {
                        0.0f + margemX,
                        0.0f + margemY,
                        390.0f,
                        190.0f};

                    Rectangle destRec = {ei->rect.x - 5, ei->rect.y - 2, ei->rect.width + 10, ei->rect.height + 2};

                    Vector2 origin = {0.0f, 0.0f};

                    DrawTexturePro(plataformaChao, recortePedra, destRec, origin, 0.0f, WHITE);
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
        }
        EndDrawing();
    }

    // Descarregar textura

    UnloadTexture(background);
    UnloadTexture(backgroundFundo1);
    UnloadTexture(backgroundFundo2);
    UnloadTexture(backgroundFundo3);
    UnloadTexture(fundoMenu);

    UnloadTexture(plataforma);
    UnloadTexture(plataformaTrianguloCaindo);
    UnloadTexture(plataformaPedra);
    UnloadTexture(plataformaChao);
    UnloadTexture(plataformaTriangulo);

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
    UnloadMusicStream(music);
    UnloadMusicStream(musicMenu); // Unload music stream buffers from RAM

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

// Funções Auxiliares (Física e Câmera)

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, FallingBlock *pedras, float delta)
{
    // Movimentação do Jogador
    if (IsKeyDown(KEY_LEFT))
    {
        player->position.x -= PLAYER_HOR_SPD * delta;
        player->isFacingRight = false;
    }
    if (IsKeyDown(KEY_RIGHT))
    {
        player->position.x += PLAYER_HOR_SPD * delta;
        player->isFacingRight = true;
    }
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    Rectangle playerCorpo = {player->position.x - 15, player->position.y - 55, 30, 50};

    // --- LOOP EXCLUSIVO DAS PEDRAS (Vai apenas até MAX_PEDRAS) ---
    for (int i = 0; i < MAX_PEDRAS; i++)
    {
        if (pedras[i].active && CheckCollisionRecs(playerCorpo, pedras[i].rect))
        {
            player->position = (Vector2){400, 280}; // Morreu pelas pedras
            player->speed = 0;
            pedras[i].rect.y = -100;
        }
    }

    // --- LOOP EXCLUSIVO DO CENÁRIO (Vai até envItemsLength) ---
    bool hitObstacle = false;
    float larguraColisao = 30.0f;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);

        // Morte instantânea por espinho
        if (ei->type == TIPO_TRIANGULO_PRA_CIMA && CheckCollisionRecs(playerCorpo, ei->rect))
        {
            player->position = (Vector2){400, 280}; // Morreu pelo espinho
            player->speed = 0;
            return; // Sai da função imediatamente
        }

        if (!ei->blocking)
            continue;

        // Pisar no bloco (Gravidade e chão)
        if (ei->rect.x <= (p->x + larguraColisao / 2) &&
            ei->rect.x + ei->rect.width >= (p->x - larguraColisao / 2) &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed * delta)
        {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y;
        }

        // Colisão com as paredes laterais dos blocos
        if (CheckCollisionRecs(playerCorpo, ei->rect))
        {
            if (ei->rect.y < player->position.y - 15 && ei->rect.height > 15)
            {
                if (IsKeyDown(KEY_RIGHT))
                    player->position.x = ei->rect.x - 16;
                else if (IsKeyDown(KEY_LEFT))
                    player->position.x = ei->rect.x + ei->rect.width + 16;
            }
        }
    }

    // Aplica a gravidade se estiver caindo (não encostou em obstáculo)
    if (!hitObstacle)
    {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    }
    else
    {
        player->canJump = true;
    }
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