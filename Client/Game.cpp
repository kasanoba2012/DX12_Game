#include "pch.h"
#include "Game.h"
#include "Engine.h"
#include "SceneManager.h"

void Game::CreateGraphicsShader(const WindowInfo& info)
{
	GEngine->CreateGraphicsShader(info);	

	GET_SINGLE(SceneManager)->LoadScene(L"TestScene");
}

void Game::Update()
{
	GEngine->Update();
}
 