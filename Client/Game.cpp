#include "pch.h"
#include "Game.h"
#include "Engine.h"

shared_ptr<Mesh> mesh = make_shared<Mesh>();
shared_ptr<Shader> shader = make_shared<Shader>();

void Game::Init(const WindowInfo& info)
{
	GEngine->Init(info);
	
	// 삼각형 정보 만들기
	vector<Vertex> vec(3);
	vec[0].pos = Vec3(0, 0.5f, 0.5f);
	vec[0].color = Vec4(1.f, 0, 0, 1.f);
	vec[1].pos = Vec3(0.5f, -0.5f, 0.5f);
	vec[1].color = Vec4(0, 1, 0, 1.f);
	vec[2].pos = Vec3(-0.5f, -0.5f, 0.5f);
	vec[2].color = Vec4(0, 0, 1, 1.f);

	// 해당 삼각형 정보를 메쉬 init를 통해서 GPU 메모리쪽으로 전달
	mesh->Init(vec);

	// 쉐이더 파일을 읽어서 어떻게 쉐이더 처리할지 컴파일
	shader->Init(L"..\\Resources\\Shader\\default.hlsli");

	GEngine->GetCmdQueue()->WaitSync();
}

void Game::Update()
{
	GEngine->RenderBegin();

	// 쉐이더 파일을 읽어서 그림그려달라는 대로 그리기 준비
	shader->Update();

	{
		Transform t;
		t.offset = Vec4(0.5f, 0.f, 0.f, 0.f);
		mesh->SetTransform(t);

		// 무엇을 그려달라는지 확인하고 물체 그려달라는 명령어 전달
		mesh->Render();
	}

	{
		Transform t;
		t.offset = Vec4(0.f, 0.5f, 0.f, 0.f);
		mesh->SetTransform(t);

		// 무엇을 그려달라는지 확인하고 물체 그려달라는 명령어 전달
		mesh->Render();
	}
		
	mesh->Render();

	GEngine->RenderEnd();
}
