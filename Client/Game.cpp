#include "pch.h"
#include "Game.h"
#include "Engine.h"

shared_ptr<Mesh> mesh = make_shared<Mesh>();
shared_ptr<Shader> shader = make_shared<Shader>();

void Game::Init(const WindowInfo& info)
{
	GEngine->Init(info);
	
	// �ﰢ�� ���� �����
	vector<Vertex> vec(3);
	vec[0].pos = Vec3(0, 0.5f, 0.5f);
	vec[0].color = Vec4(1.f, 0, 0, 1.f);
	vec[1].pos = Vec3(0.5f, -0.5f, 0.5f);
	vec[1].color = Vec4(0, 1, 0, 1.f);
	vec[2].pos = Vec3(-0.5f, -0.5f, 0.5f);
	vec[2].color = Vec4(0, 0, 1, 1.f);

	// �ش� �ﰢ�� ������ �޽� init�� ���ؼ� GPU �޸������� ����
	mesh->Init(vec);

	// ���̴� ������ �о ��� ���̴� ó������ ������
	shader->Init(L"..\\Resources\\Shader\\default.hlsli");

	GEngine->GetCmdQueue()->WaitSync();
}

void Game::Update()
{
	GEngine->RenderBegin();

	// ���̴� ������ �о �׸��׷��޶�� ��� �׸��� �غ�
	shader->Update();

	{
		Transform t;
		t.offset = Vec4(0.5f, 0.f, 0.f, 0.f);
		mesh->SetTransform(t);

		// ������ �׷��޶���� Ȯ���ϰ� ��ü �׷��޶�� ��ɾ� ����
		mesh->Render();
	}

	{
		Transform t;
		t.offset = Vec4(0.f, 0.5f, 0.f, 0.f);
		mesh->SetTransform(t);

		// ������ �׷��޶���� Ȯ���ϰ� ��ü �׷��޶�� ��ɾ� ����
		mesh->Render();
	}
		
	mesh->Render();

	GEngine->RenderEnd();
}
