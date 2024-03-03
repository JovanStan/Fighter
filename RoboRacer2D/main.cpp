
#include "stdafx.h"
#include <Windows.h>
#include "Sprite.h"

#include "Input.h"

#include <stdio.h>
#include <time.h>
#include "fmod.hpp"
#include<iostream>
#include<fstream>
#include <string>
#include<thread>
using namespace std;
#define MAX_LOADSTRING 100
#define LVL_NUM lvl1Num//(currentLevel == LevelState::GS_Level1) ? lvl1Num : lvl2Num

enum GameState
{
	GS_Splash,
	GS_Loading,
	GS_Menu,
	GS_Credits,
	GS_Running,
	GS_NextLevel,
	GS_Paused,
	GS_GameOver,
	GS_HighScore
};

enum LevelState {
	GS_Level1,
	GS_Level2
};

int previousTime;
int refreshMills = 30; // refresh interval in milliseconds

float p_deltaTime = 0.0;

bool fullscreen = false;
GLfloat screen_height;
GLfloat screen_width;

//Game variables
Sprite* knight_left;
Sprite* knight_right;
Sprite* knight_right_strip;
Sprite* knight_left_strip;
Sprite* background;
Sprite* player;
Sprite* arrow;

Sprite** orc;
static int lvl1Num = 4;
static int lvl2Num = 5;
static int levelNumber = 1;

static int orcSpeed = -150;
static int orcsKilled = 0;

Sprite* goblin2;

static int goblinLives = 2;
static int goblinSpawnX = 1000;

Sprite* pickupArrow;
float pickupSpawnTimer;
float pickupSpawnThreshold;

static int ammo = 0;


Sprite* pauseButton;
Sprite* resumeButton;

Sprite* splashScreen;
Sprite* menuScreen;
Sprite* creditsScreen;
Sprite* nextLevelScreen;
Sprite* continueButton;
Sprite* gameOverScreen;
Sprite* replayButton;
Sprite* highScoreScreen;

Sprite* playButton;
Sprite* creditsButton;
Sprite* exitButton;
Sprite* menuButton;
Sprite* highScoreButton;

Input* inputManager;

GameState m_gameState;
LevelState currentLevel;

float uiTimer;
const float UI_THRESHOLD = 0.01f;


float splashDisplayTimer;
float splashDisplayThreshold;

float levelTimer;
float levelMaxTime;

FMOD::System* audiomgr;
FMOD::Channel* chMovement;
FMOD::Sound* sfxFloop;
FMOD::Sound* sfxButton;
FMOD::Sound* sfxBoing;
FMOD::Sound* sfxArrow;
FMOD::Sound* musBackground;

ifstream file;

bool InitFmod()
{
	FMOD_RESULT result;
	result = FMOD::System_Create(&audiomgr);
	if (result != FMOD_OK)
	{
		return false;
	}
	result = audiomgr->init(50, FMOD_INIT_NORMAL, NULL);
	if (result != FMOD_OK)
	{
		return false;
	}
	return true;
}

const bool LoadAudio()
{
	FMOD_RESULT result;
	result = audiomgr->createSound("resources/floop.wav", FMOD_DEFAULT, 0, &sfxFloop);
	result = audiomgr->createSound("resources/button.wav", FMOD_DEFAULT, 0, &sfxButton);
	result = audiomgr->createSound("resources/boing.wav", FMOD_DEFAULT, 0, &sfxBoing);
	result = audiomgr->createSound("resources/arrow.wav", FMOD_DEFAULT, 0, &sfxArrow);

	result = audiomgr->createSound("resources/lotrBackground.mp3", FMOD_LOOP_NORMAL | FMOD_2D, 0, &musBackground);
	FMOD::Channel* channel;
	result = audiomgr->playSound(musBackground, 0, false, &channel);
	return true;
}

void DrawText(char* string, float x, float y, const float r, const float g, const float b)  // BItmap Font
{
	glColor3f(r, g, b);
	glLineWidth(4.0);
	char* c;
	glRasterPos3f(x, y, 0.0);
	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
}

void DrawStrokeText(char* string, int x, int y, const float r, const float g, const float b) // Stroke Font
{
	char* c;

	glColor3f(r, g, b);
	glPushMatrix();
	glTranslatef(x, y + 8, 0.0);
	glLineWidth(2.0);
	glScalef(0.14f, -0.12f, 0.0);

	for (c = string; *c != '\0'; c++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	}
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
}

void DrawCredits()
{
	float startX = 325.0f;
	float startY = 250.0f;
	float spaceY = 30.0f;
	DrawText("Game Director/Programmer", startX - 50, startY, 1, 0.0f, 1.0f);
	DrawText("Jovan Stankovic", startX, startY + spaceY, 0.0f, 1, 1.0f);
	DrawText("3D Artist", startX + 30, startY + 2 * spaceY + 10, 1, 0.0f, 1.0f);
	DrawText("Jovan Stankovic", startX, startY + spaceY + 70, 0.0f, 1, 1.0f);
}

void DrawScore()
{
	char orcs[50];
	sprintf_s(orcs, 50, "Orcs killed: %i", orcsKilled);
	DrawText(orcs, 350.0f, 25.0f, 0.0f, 0.0f, 1.0f);

	char levelAmmo[50];
	sprintf_s(levelAmmo, 50, "Ammo: %i", ammo);
	DrawText(levelAmmo, 350.0f, 50.0f, 0.0f, 0.0f, 1.0f);
}

void DrawHighScore()
{
	float startX = 325.0f;
	float startY = 250.0f;
	float spaceY = 30.0f;
	file.open("highScore.txt", ifstream::in);
	char output[100];
	if (file.is_open()) {
		while (!file.eof()) {
			file >> output;
			if (output == "\n") {

			}
			DrawText(output, startX, startY + spaceY, 0.0f, 0.0f, 1.0f);
		}
	}
	file.close();
}

void DrawStats()
{
	char orcs[50];
	char levelAmmo[50];
	sprintf_s(orcs, 50, "Orcs killed: %i", player->GetValue());
	DrawText(orcs, 325.0f, 270.0f, 0.0f, 0.0f, 1.0f);
}

void LoadSplash()
{
	m_gameState = GameState::GS_Splash;
	splashScreen = new Sprite(1);
	splashScreen->SetFrameSize(800.0f, 600.0f);
	splashScreen->SetNumberOfFrames(1);
	splashScreen->AddTexture("resources/splashLogo.png", false);
	splashScreen->IsActive(true);
	splashScreen->IsVisible(true);
}

const bool LoadTextures()
{
	background = new Sprite(1);
	background->SetFrameSize(1877.0f, 600.0f);
	background->SetNumberOfFrames(1);
	background->AddTexture("resources/background.png", false);

	knight_right = new Sprite(2);
	knight_right->SetFrameSize(100.0f, 125.0f);
	knight_right->SetNumberOfFrames(2);
	knight_right->SetPosition(0, screen_height - 130.0f);
	knight_right->AddTexture("resources/knightRight1.png");
	knight_right->AddTexture("resources/knightRight2.png");
	//knight_right->AddTexture("resources/knightRight3.png");
	//knight_right->AddTexture("resources/knightRight4.png");

	knight_left = new Sprite(2);
	knight_left->SetFrameSize(100.0f, 125.0f);
	knight_left->SetNumberOfFrames(2);
	knight_left->SetPosition(0, screen_height - 130.0f);
	knight_left->AddTexture("resources/knightLeft1.png");
	knight_left->AddTexture("resources/knightLeft2.png");
	//knight_left->AddTexture("resources/knightLeft3.png");
	//knight_left->AddTexture("resources/knightLeft4.png");

	knight_right_strip = new Sprite(1);
	knight_right_strip->SetFrameSize(125.0f, 100.0f);
	knight_right_strip->SetNumberOfFrames(4);
	knight_right_strip->SetPosition(0, screen_height - 130.0f);
	knight_right_strip->AddTexture("resources/knightRight.png");

	knight_left_strip = new Sprite(1);
	knight_left_strip->SetFrameSize(125.0f, 100.0f);
	knight_left_strip->SetNumberOfFrames(4);
	knight_right_strip->SetPosition(0, screen_height - 130.0f);
	knight_left_strip->AddTexture("resources/knightLeft.png");


	goblin2 = new Sprite(1);
	goblin2->SetFrameSize(200.0f,150.0f);
	goblin2->SetNumberOfFrames(1);
	goblin2->SetPosition(goblinSpawnX, 450);
	goblin2->AddTexture("resources/goblin2.png");

	goblin2->IsActive(true);
	goblin2->IsVisible(true);
	goblin2->IsCollideable(true);
	goblin2->SetVelocity(-50);

	arrow = new Sprite(1);
	arrow->SetFrameSize(50, 30);
	arrow->SetNumberOfFrames(1);
	arrow->SetPosition(100, screen_height - 80);
	arrow->AddTexture("resources/arrow2.png");

	arrow->IsVisible(false);
	arrow->IsActive(false);

	pickupArrow = new Sprite(1);
	pickupArrow->SetFrameSize(50.0f, 70.0f);
	pickupArrow->SetNumberOfFrames(1);
	pickupArrow->AddTexture("resources/pickupArrow.png");
	pickupArrow->IsVisible(false);
	pickupArrow->IsActive(false);
	pickupArrow->SetValue(1);
	pickupArrow->IsCollideable(true);


	background->IsVisible(true);
	background->IsActive(true);
	background->SetVelocity(-50.0f);

	knight_right->IsActive(true);
	knight_right->IsVisible(true);

	player = knight_right;
	player->IsActive(true);
	player->IsVisible(true);



	pauseButton = new Sprite(1);
	pauseButton->SetFrameSize(75.0f, 38.0f);
	pauseButton->SetNumberOfFrames(1);
	pauseButton->SetPosition(5.0f, 5.0f);
	pauseButton->AddTexture("resources/pauseButton.png");
	pauseButton->IsVisible(true);
	pauseButton->IsActive(true);
	inputManager->AddUiElement(pauseButton);

	resumeButton = new Sprite(1);
	resumeButton->SetFrameSize(75.0f, 38.0f);
	resumeButton->SetNumberOfFrames(1);
	resumeButton->SetPosition(80.0f, 5.0f);
	resumeButton->AddTexture("resources/resumeButton.png");
	inputManager->AddUiElement(resumeButton);

	Sprite::Rect collision;
	collision.left = 34.0f;
	collision.right = -10.0f;
	collision.top = 0.0f;
	collision.bottom = 0.0f;
	knight_left->SetCollisionRectOffset(collision);
	knight_right->SetCollisionRectOffset(collision);

	knight_left->IsCollideable(true);
	knight_right->IsCollideable(true);
	arrow->IsCollideable(true);


	Sprite::Point center;
	float radius;
	center.x = knight_right->GetSize().width / 2.0f;
	center.y = knight_right->GetSize().height / 2.0f;
	radius = (center.x + center.y) / 2.0f;
	knight_right->SetCenter(center);
	knight_right->SetRadius(radius);
	knight_left->SetCenter(center);
	knight_left->SetRadius(radius);
	center.x = pickupArrow->GetSize().width / 2.0f;
	float yOffset = (pickupArrow->GetSize().height / 4.0f) * 3.0f;
	center.y = yOffset;
	pickupArrow->SetCenter(center);
	radius = pickupArrow->GetSize().width / 2.0f;
	pickupArrow->SetRadius(radius);

	menuScreen = new Sprite(1);
	menuScreen->SetFrameSize(800.0f, 600.0f);
	menuScreen->SetNumberOfFrames(1);
	menuScreen->AddTexture("resources/mainmenu.png", false);
	menuScreen->IsActive(true);
	menuScreen->IsVisible(true);

	highScoreScreen = new Sprite(1);
	highScoreScreen->SetFrameSize(800.0f, 600.0f);
	highScoreScreen->SetNumberOfFrames(1);
	highScoreScreen->AddTexture("resources/highScoreScreen.png", false);
	highScoreScreen->IsActive(true);
	highScoreScreen->IsVisible(true);

	playButton = new Sprite(1);
	playButton->SetFrameSize(75.0f, 38.0f);
	playButton->SetNumberOfFrames(1);
	playButton->SetPosition(350.0f, 200.0f);
	playButton->AddTexture("resources/playButton.png");
	playButton->IsVisible(true);
	playButton->IsActive(false);
	inputManager->AddUiElement(playButton);



	creditsButton = new Sprite(1);
	creditsButton->SetFrameSize(75.0f, 38.0f);
	creditsButton->SetNumberOfFrames(1);
	creditsButton->SetPosition(350.0f, 250.0f);
	creditsButton->AddTexture("resources/creditsButton.png");
	creditsButton->IsVisible(true);
	creditsButton->IsActive(false);
	inputManager->AddUiElement(creditsButton);

	highScoreButton = new Sprite(1);
	highScoreButton->SetFrameSize(75.0f, 38.0f);
	highScoreButton->SetNumberOfFrames(1);
	highScoreButton->SetPosition(350.0f, 300.0f);
	highScoreButton->AddTexture("resources/highScoreButton.png");
	highScoreButton->IsVisible(true);
	highScoreButton->IsActive(false);
	inputManager->AddUiElement(highScoreButton);

	exitButton = new Sprite(1);
	exitButton->SetFrameSize(75.0f, 38.0f);
	exitButton->SetNumberOfFrames(1);
	exitButton->SetPosition(350.0f, 350.0f);
	exitButton->AddTexture("resources/exitButton.png");
	exitButton->IsVisible(true);
	exitButton->IsActive(false);
	inputManager->AddUiElement(exitButton);

	// credits screen
	creditsScreen = new Sprite(1);
	creditsScreen->SetFrameSize(800.0f, 600.0f);
	creditsScreen->SetNumberOfFrames(1);
	creditsScreen->AddTexture("resources/credits.png", false);
	creditsScreen->IsActive(false);
	creditsScreen->IsVisible(true);

	menuButton = new Sprite(1);
	menuButton->SetFrameSize(75.0f, 38.0f);
	menuButton->SetNumberOfFrames(1);
	menuButton->SetPosition(350.0f, 400.0f);
	menuButton->AddTexture("resources/menuButton.png");
	menuButton->IsVisible(true);
	menuButton->IsActive(false);
	inputManager->AddUiElement(menuButton);

	nextLevelScreen = new Sprite(1);
	nextLevelScreen->SetFrameSize(800.0f, 600.0f);
	nextLevelScreen->SetNumberOfFrames(1);
	nextLevelScreen->AddTexture("resources/levelCompleted.png", false);
	nextLevelScreen->IsActive(true);
	nextLevelScreen->IsVisible(true);

	continueButton = new Sprite(1);
	continueButton->SetFrameSize(75.0f, 38.0f);
	continueButton->SetNumberOfFrames(1);
	continueButton->SetPosition(350.0f, 300.0f);
	continueButton->AddTexture("resources/continueButton.png");
	continueButton->IsVisible(true);
	continueButton->IsActive(false);
	inputManager->AddUiElement(continueButton);

	gameOverScreen = new Sprite(1);
	gameOverScreen->SetFrameSize(800.0f, 600.0f);
	gameOverScreen->SetNumberOfFrames(1);
	gameOverScreen->AddTexture("resources/gameOver.png", false);
	gameOverScreen->IsActive(true);
	gameOverScreen->IsVisible(true);

	replayButton = new Sprite(1);
	replayButton->SetFrameSize(75.0f, 38.0f);
	replayButton->SetNumberOfFrames(1);
	replayButton->SetPosition(350.0f, 300.0f);
	replayButton->AddTexture("resources/replayButton.png");
	replayButton->IsVisible(true);
	replayButton->IsActive(false);
	inputManager->AddUiElement(replayButton);

	return true;
}

void SpawnPickup(float p_DeltaTime)
{
	if (pickupArrow->IsVisible() == false)
	{
		pickupSpawnTimer += p_DeltaTime;
		if (pickupSpawnTimer > pickupSpawnThreshold)
		{
			float marginX = pickupArrow->GetSize().width;
			float marginY = pickupArrow->GetSize().height;
			float spawnX = (rand() % (int)(screen_width - (marginX * 2))) + marginX;
			float spawnY = screen_height - ((rand() % (int)(player->GetSize().height - (marginY * 1.5))) + marginY);
			pickupArrow->SetPosition(spawnX, spawnY);
			pickupArrow->IsVisible(true);
			pickupArrow->IsActive(true);
			pickupSpawnTimer = 0.0f;
		}
	}
}

void OrcInit(int num) {

	orc = (Sprite**)malloc(sizeof(Sprite) * num);

	for (int i = 0; i < num; i++) {
		orc[i] = new Sprite(1);
		orc[i]->SetFrameSize(190.0f, 160.0f);
		orc[i]->SetNumberOfFrames(4);
		orc[i]->SetPosition(0, screen_height - 130.0f);
		orc[i]->AddTexture("resources/orcStrip.png");

		orc[i]->IsActive(true);
		orc[i]->IsVisible(true);
		orc[i]->IsCollideable(true);
		orc[i]->SetPosition(screen_width * i + 600, 450);
		orc[i]->SetVelocity(orcSpeed);
	}
}



void CheckCollisions()
{

	for (int i = 0; i < LVL_NUM; i++) {
		if (player->IntersectsRect(orc[i])) {
			orc[i]->IsVisible(false);
			orc[i]->IsActive(false);
			
			m_gameState = GameState::GS_GameOver;
		}
	}

	if (player->IntersectsRect(goblin2)) {
		goblin2->IsVisible(false);
		goblin2->IsActive(false);
		m_gameState = GameState::GS_GameOver;
	}

 	for (int i = 0; i < LVL_NUM; i++) {
		if (arrow->IntersectsRect(orc[i])) {
			orc[i]->IsVisible(false);
			orc[i]->IsActive(false);
			arrow->IsVisible(false);
			arrow->IsVisible(false);
			arrow->SetPosition(player->GetPosition().x, player->GetPosition().y + 20);
			orcsKilled++;
			player->SetValue(orcsKilled);
		}
	} 	

	if (arrow->IntersectsRect(goblin2)) {
		arrow->IsActive(false);
		arrow->IsVisible(false);
		arrow->SetPosition(player->GetPosition().x, player->GetPosition().y + 20);
		goblinLives--;
		if (goblinLives == 0)
		{
			goblin2->IsActive(false);
			goblin2->IsVisible(false);
			orcsKilled += 2;
			player->SetValue(orcsKilled);
		}
	}

	if (player->IntersectsRect(pickupArrow)) {
		pickupArrow->IsVisible(false);
		pickupArrow->IsActive(false);
		player->SetValue(player->GetValue() + pickupArrow->GetValue());
		pickupSpawnTimer = 0.0f;
		ammo += 2;
		//pickupsReceived++;
	}
		
}

void CheckBackground()
{
	float leftThreshold = 0.0f;
	float rightThreshold = -(background->GetSize().width - screen_width);
	if (background->GetPosition().x > 0)
	{
		background->SetPosition(0.0f, background->GetPosition().y);
	}
	else if (background->GetPosition().x < rightThreshold)
	{
		background->SetPosition(rightThreshold, background->GetPosition().y);
	}
}

void CheckBoundaries(Sprite* p_sprite)
{
	Sprite::Rect check = p_sprite->GetCollisionRect();
	float offset;
	float x;
	float y;
	if (check.left < 0.0f)
	{
		p_sprite->SetVelocity(0.0f);
		offset = check.left;
		x = p_sprite->GetPosition().x - offset;
		y = p_sprite->GetPosition().y;
		p_sprite->SetPosition(x, y);
	}
	else if (check.right > screen_width)
	{
		p_sprite->SetVelocity(0.0f);
		offset = screen_width - check.right;
		x = p_sprite->GetPosition().x + offset;
		y = p_sprite->GetPosition().y;
		p_sprite->SetPosition(x, y);
	}
	if (check.top < 0.0f)
	{
		p_sprite->SetVelocity(0.0f);
		offset = check.top;
		y = p_sprite->GetPosition().y - offset;
		x = p_sprite->GetPosition().x;
		p_sprite->SetPosition(x, y);
	}
	else if (check.bottom > screen_height)
	{
		p_sprite->SetVelocity(0.0f);
		offset = screen_height - check.bottom;
		y = p_sprite->GetPosition().y + offset;
		x = p_sprite->GetPosition().x;
		p_sprite->SetPosition(x, y);
	}
}

void RestartGame()
{
	player->SetValue(0);
	knight_right->SetValue(0);
	knight_left->SetValue(0);
	splashDisplayTimer = 0.0f;
	splashDisplayThreshold = 5.0f;
	background->SetVelocity(0.0f);
	knight_left->SetPosition(screen_width / 2.0f - 50.0f, screen_height - 130.0f);
	knight_left->IsVisible(false);
	knight_right->SetPosition(screen_width / 2.0f - 50.0f, screen_height - 130.0f);
	player = knight_right;
	player->IsActive(true);
	player->IsVisible(true);
	player->SetVelocity(0.0f);

	delete goblin2;
	delete orc;
	delete arrow;
	OrcInit(LVL_NUM);
}

void ProcessInput()
{
	Input::Command command = inputManager->GetCommand();
	switch (m_gameState)
	{
	case GameState::GS_Splash:
	case GameState::GS_Loading:
	{
		return;
	}
	break;
	case GameState::GS_Menu:
	case GameState::GS_Credits:
	case GameState::GS_Paused:
	case GameState::GS_NextLevel:
	case GameState::GS_GameOver:
	case GameState::GS_HighScore:
	{
		command = Input::Command::CM_UI;
	}
	break;
	case GameState::GS_Running:
	{
	}
	break;
	}

	uiTimer += p_deltaTime;
	if (uiTimer > UI_THRESHOLD)
	{
		uiTimer = 0.0f;
		switch (command)
		{
		case Input::Command::CM_STOP:
		{
			player->SetVelocity(0.0f);
			background->SetVelocity(0.0f);
			chMovement->setPaused(true);
		}
		break;

		case Input::Command::CM_LEFT:
		{
			if (player == knight_right)
			{
				knight_right->IsActive(false);
				knight_right->IsVisible(false);
				knight_left->SetPosition(knight_right->GetPosition());
				knight_left->SetValue(knight_right->GetValue());

			}	
				player = knight_left;
				player->IsActive(true);
				player->IsVisible(true);
				player->SetVelocity(-100.0f);
				background->SetVelocity(50.0f);
				chMovement->setPaused(false);
		}
		break;

		case Input::Command::CM_RIGHT:
		{
			if (player == knight_left)
			{
				knight_left->IsActive(false);
				knight_left->IsVisible(false);
				knight_right->SetPosition(knight_left->GetPosition());
				knight_right->SetValue(knight_left->GetValue());
			}
			player = knight_right;
			player->IsActive(true);
			player->IsVisible(true);
			player->SetVelocity(100.0f);
			background->SetVelocity(-50.0f);
			chMovement->setPaused(false);
		}
		break;

		case Input::Command::CM_MOVE:
		{
			// ispaljuje strele samo kad je okrenut na desno
			if (player == knight_right) {
				if (ammo <= 0) {
					arrow->IsActive(false);
					arrow->IsVisible(false);
					ammo = 0;
				}
				else {
					arrow->IsActive(true);
					arrow->IsVisible(true);
					ammo--;
				}
			}
		}
		break;

		case Input::Command::CM_QUIT:
		{
			PostQuitMessage(0);
		}
		break;

		case Input::Command::CM_UI:
		{
			FMOD::Channel* channel;
			if (pauseButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				pauseButton->IsClicked(false);
				pauseButton->IsVisible(false);
				pauseButton->IsActive(false);

				resumeButton->IsVisible(true);
				resumeButton->IsActive(true);
				m_gameState = GS_Paused;
			}

			if (resumeButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				resumeButton->IsClicked(false);
				resumeButton->IsVisible(false);
				resumeButton->IsActive(false);

				pauseButton->IsVisible(true);
				pauseButton->IsActive(true);
				m_gameState = GS_Running;
			}
			if (playButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				playButton->IsClicked(false);
				exitButton->IsActive(false);
				playButton->IsActive(false);
				creditsButton->IsActive(false);
				highScoreButton->IsActive(false);
				m_gameState = GameState::GS_Running;
				currentLevel = LevelState::GS_Level1;
			}
			if (creditsButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				creditsButton->IsClicked(false);
				exitButton->IsActive(false);
				playButton->IsActive(false);
				creditsButton->IsActive(false);
				highScoreButton->IsActive(false);
				m_gameState = GameState::GS_Credits;
			}
			if (exitButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				playButton->IsClicked(false);
				exitButton->IsActive(false);
				playButton->IsActive(false);
				creditsButton->IsActive(false);
				PostQuitMessage(0);
			}
			if (menuButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				menuButton->IsClicked(false);
				menuButton->IsActive(false);
				m_gameState = GameState::GS_Menu;

			}
			if (continueButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				continueButton->IsClicked(false);
				continueButton->IsActive(false);

				// orc spawn control
				lvl1Num++;
				orcSpeed += -50;
				levelNumber++;
				OrcInit(LVL_NUM);

				// knight spawn
				knight_right->SetPosition(0.0f, screen_height - 130.0f);
				player = knight_right;
				player->IsActive(true);
				player->IsVisible(true);
				player->SetVelocity(0.0f);

				// goblin spawn
				goblin2->IsActive(true);
				goblin2->IsVisible(true);
				goblin2->IsCollideable(true);
				goblin2->SetVelocity(-75);
				goblinLives = 2;
				goblinSpawnX += 200;

				ammo += 4;

				m_gameState = GameState::GS_Running;
				currentLevel = LevelState::GS_Level2;				
				
			}
			if (replayButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				replayButton->IsClicked(false);
				replayButton->IsActive(false);
				exitButton->IsActive(false);

				knight_right->SetPosition(0.0f, screen_height - 130.0f);
				player = knight_right;
				player->IsActive(true);
				player->IsVisible(true);
				player->SetVelocity(0.0f);


				OrcInit(LVL_NUM);


				pickupSpawnTimer = 0;
				pickupSpawnThreshold = 5.0f;
				pickupArrow->IsVisible(false);


				goblin2->IsActive(true);
				goblin2->IsVisible(true);
				goblin2->IsCollideable(true);
				goblin2->SetVelocity(-75);
				goblinLives = 2;


				orcsKilled = 0;
				ammo = 6;
				m_gameState = GameState::GS_Running;
			}

			if (highScoreButton->IsClicked())
			{
				audiomgr->playSound(sfxButton, 0, false, &channel);
				highScoreButton->IsClicked(false);
				highScoreButton->IsActive(false);
				exitButton->IsActive(false);
				playButton->IsActive(false);
				creditsButton->IsActive(false);
				m_gameState = GameState::GS_HighScore;
			}
		}
		break;
		}
	}
}

void Update()
{
	if (arrow->IsActive() && arrow->IsVisible()) {
		arrow->MoveArrow(Sprite::SpriteState::MOVE);
	}
	player->SetVelocity(0);
	background->SetVelocity(0);

	

	switch (m_gameState)
	{
	case GameState::GS_Splash:
	case GameState::GS_Loading:
	{
		splashScreen->Update(p_deltaTime);
		splashDisplayTimer += p_deltaTime;
		if (splashDisplayTimer > splashDisplayThreshold)
		{
			m_gameState = GameState::GS_Menu;
		}
	}
	break;
	case GameState::GS_Menu:
	{
		menuScreen->Update(p_deltaTime);
		playButton->IsActive(true);
		creditsButton->IsActive(true);
		exitButton->IsActive(true);
		highScoreButton->IsActive(true);
		playButton->Update(p_deltaTime);
		creditsButton->Update(p_deltaTime);
		highScoreButton->Update(p_deltaTime);
		exitButton->Update(p_deltaTime);
		ProcessInput();
	}
	break;

	case GameState::GS_Credits:
	{
		creditsScreen->Update(p_deltaTime);
		menuButton->IsActive(true);
		menuButton->Update(p_deltaTime);
		ProcessInput();
	}
	break;

	case GameState::GS_HighScore:
	{
		FMOD::Channel* channel;
		highScoreScreen->Update(p_deltaTime);
		menuButton->IsActive(true);
		menuButton->Update(p_deltaTime);
		ProcessInput();
	}
	break;

	case GameState::GS_Running:
	{
		ProcessInput();
		CheckBoundaries(player);
		CheckBackground();

		background->Update(p_deltaTime);
		knight_left->Update(p_deltaTime);
		knight_right->Update(p_deltaTime);
		knight_left_strip->Update(p_deltaTime);
		knight_right_strip->Update(p_deltaTime);
		pauseButton->Update(p_deltaTime);
		resumeButton->Update(p_deltaTime);
		exitButton->IsActive(false);
		arrow->Update(p_deltaTime);
		goblin2->Update(p_deltaTime);
		pickupArrow->Update(p_deltaTime);
		SpawnPickup(p_deltaTime);
		for (int i = 0; i < LVL_NUM; i++)
		{
			orc[i]->Update(p_deltaTime);
		} 
		CheckCollisions();
		if (orcsKilled == lvl1Num + 2)
		{
			orcsKilled = 0;
			m_gameState = GameState::GS_NextLevel;
		//	currentLevel = LevelState::GS_Level2;
		} 
	}
	break;
	case GameState::GS_Paused:
	{
		ProcessInput();
	}
	break;
	case GameState::GS_NextLevel:
	{
		nextLevelScreen->Update(p_deltaTime);
		continueButton->IsActive(true);
		continueButton->Update(p_deltaTime);
		exitButton->IsActive(true);
		exitButton->Update(p_deltaTime);
		char levelNum[50];
		sprintf_s(levelNum, 50, "Level: %i completed", levelNumber);
		DrawText(levelNum, 300.0f, 225.0f, 0.0f, 0.0f, 1.0f);
		ProcessInput();
	}
	break;
	case GameState::GS_GameOver:
	{
		gameOverScreen->Update(p_deltaTime);
		replayButton->IsActive(true);
		replayButton->Update(p_deltaTime);
		exitButton->IsActive(true);
		exitButton->Update(p_deltaTime);
		ProcessInput();
	}
	break;
	}

	inputManager->SetCommand(Input::Command::CM_INVALID);
}

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch (m_gameState)
	{
	case GameState::GS_Splash:
	case GameState::GS_Loading:
	{
		splashScreen->Render();
	}
	break;
	case GameState::GS_Menu:
	{
		menuScreen->Render();
		playButton->Render();
		creditsButton->Render();
		highScoreButton->Render();
		exitButton->Render();
	}
	break;

	case GameState::GS_HighScore:
	{
		highScoreScreen->Render();
		menuButton->Render();
		DrawHighScore();
	}
	break;

	case GameState::GS_Credits:
	{
		creditsScreen->Render();
		menuButton->Render();
		DrawCredits();
	}
	break;
	case GameState::GS_Running:
	case GameState::GS_Paused:
	{
		background->Render();
		knight_left->Render();
		knight_right->Render();
		knight_left_strip->Render();
		knight_right_strip->Render();
		pauseButton->Render();
		resumeButton->Render();
		arrow->Render();
		goblin2->Render();
		pickupArrow->Render();
		for (int i = 0; i < LVL_NUM; i++)
		{
			orc[i]->Render();
		} 
		DrawScore();
	}
	break;
	case GameState::GS_NextLevel:
	{
		nextLevelScreen->Render();
		DrawStats();
		continueButton->Render();
		exitButton->Render();
		char levelNum[50];
		sprintf_s(levelNum, 50, "Level: %i completed", levelNumber);
		DrawText(levelNum, 300.0f, 225.0f, 0.0f, 0.0f, 1.0f);
	}
	break;
	case GameState::GS_GameOver:
	{
		gameOverScreen->Render();
		replayButton->Render();
		exitButton->Render();
		DrawStats();
	}
	break;
	}

	glutSwapBuffers();
}

void StartGame()
{
	LoadSplash();

	pickupSpawnTimer = 0;
	pickupSpawnThreshold = 3.0f;

	inputManager = new Input();

	uiTimer = 0.0f;
	srand(time(NULL));

	splashDisplayTimer = 0.0f;
	splashDisplayThreshold = 5.0f;

	currentLevel = LevelState::GS_Level1;

	ammo = 6;

	OrcInit(lvl1Num);
}

void GameLoop()
{
	if (m_gameState == GameState::GS_Splash)
	{
		InitFmod();
		LoadAudio();
		LoadTextures();
		m_gameState = GameState::GS_Loading;
	}

	Update();
	Render();
}

void EndGame()
{
	delete knight_left;
	delete knight_right;
	delete knight_right_strip;
	delete knight_left_strip;
	delete background;
	delete pauseButton;
	delete resumeButton;
	delete splashScreen;
	delete menuScreen;
	delete creditsScreen;
	delete playButton;
	delete creditsButton;
	delete exitButton;
	delete menuButton;
	delete nextLevelScreen;
	delete continueButton;
	delete gameOverScreen;
	delete replayButton;
	delete highScoreButton;
	delete highScoreScreen;
	delete orc;
	delete arrow;
	delete goblin2;
	delete pickupArrow;

	delete inputManager;

	sfxFloop->release();
	sfxButton->release();
	musBackground->release();
	audiomgr->release();
	sfxBoing->release();
}

void reshape(const GLsizei p_width, const GLsizei p_height)
{
	screen_height = (GLfloat)p_height;
	screen_width = (GLfloat)p_width;

	GLsizei h = p_height;
	GLsizei w = p_width;
	if (h == 0) h = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void initGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_TEXTURE_2D);
	//glShadeModel(GL_SMOOTH);
	glShadeModel(GL_FLAT);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_DEPTH_TEST);

	previousTime = glutGet(GLUT_ELAPSED_TIME);
}

void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(refreshMills, timer, 0);
}

void display()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	p_deltaTime = (float)(currentTime - previousTime) / 1000;
	previousTime = currentTime;

	GameLoop();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:     // ESC key
		exit(0);
		break;
	default:
		inputManager->keyboardKey(key, x, y);
	}
}

void specialKeys(int key, int x, int y)
{
	inputManager->keyboardSpec(key, x, y);
}

void mouse(int button, int state, int x, int y)
{
	inputManager->mouse(button, state, x, y);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);  // Enable double buffered mode

	screen_height = (GLfloat)600;
	screen_width = (GLfloat)800;

	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Fighter");

	// Callback Functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, timer, 0);


	// Keyboard and Mouse
	glutSpecialFunc(specialKeys);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	// Initialise GL and Game
	initGL();
	StartGame();

	// Game Loop
	glutMainLoop();

	EndGame();

	return 0;
}