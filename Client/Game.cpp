#include "pch.h"
#include "Game.h"
#include "Engine.h"
#include "Material.h"

#include "GameObject.h"
#include "MeshRenderer.h"

//shared_ptr<Mesh> mesh = make_shared<Mesh>();
// 메쉬 생성 대신 게임오브젝트를 생성해서 게임오브젝트에서 메쉬 생성을 한다.
shared_ptr<GameObject> gameObject = make_shared<GameObject>();

void Game::Init(const WindowInfo& info)
{
	GEngine->Init(info);

	// 삼각형 정보 만들기
	//vector<Vertex> vec(3);
	//vec[0].pos = Vec3(0, 0.5f, 0.5f);
	//vec[0].color = Vec4(1.f, 0, 0, 1.f);
	//vec[1].pos = Vec3(0.5f, -0.5f, 0.5f);
	//vec[1].color = Vec4(0, 1, 0, 1.f);
	//vec[2].pos = Vec3(-0.5f, -0.5f, 0.5f);
	//vec[2].color = Vec4(0, 0, 1, 1.f);

	// 사각형 만들기
	vector<Vertex> vec(4);
	vec[0].pos = Vec3(-0.5f, 0.5f, 0.5f);
	vec[0].color = Vec4(1.f, 0.f, 0.f, 1.f);
	vec[0].uv = Vec2(0.f, 0.f);
	vec[1].pos = Vec3(0.5f, 0.5f, 0.5f);
	vec[1].color = Vec4(0.f, 1.f, 0.f, 1.f);
	vec[1].uv = Vec2(1.f, 0.f);
	vec[2].pos = Vec3(0.5f, -0.5f, 0.5f);
	vec[2].color = Vec4(0.f, 0.f, 1.f, 1.f);
	vec[2].uv = Vec2(1.f, 1.f);
	vec[3].pos = Vec3(-0.5f, -0.5f, 0.5f);
	vec[3].color = Vec4(0.f, 1.f, 0.f, 1.f);
	vec[3].uv = Vec2(0.f, 1.f);

	vector<uint32> indexVec;
	{
		indexVec.push_back(0);
		indexVec.push_back(1);
		indexVec.push_back(2);
	}

	{
		indexVec.push_back(0);
		indexVec.push_back(2);
		indexVec.push_back(3);
	}

	// 게임 오브젝트를 통한 메쉬 생성 테스트
	gameObject->Init(); // Transform

	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	{
		shared_ptr<Mesh> mesh = make_shared<Mesh>();
		mesh->Init(vec, indexVec);
		meshRenderer->SetMesh(mesh);
	}

	{
		// 쉐이더 파일을 읽어서 어떻게 쉐이더 처리할지 컴파일
		shared_ptr<Shader> shader = make_shared<Shader>();
		shared_ptr<Texture> texture = make_shared<Texture>();
		shader->Init(L"..\\Resources\\Shader\\default.hlsli");
		texture->Init(L"..\\Resources\\Texture\\Lucian.jpg");

		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(shader);
		// 메터리얼 옵션 변경
		material->SetFloat(0, 0.3f);
		material->SetFloat(1, 0.4f);
		material->SetFloat(2, 0.3f);
		material->SetTexture(0, texture);
		meshRenderer->SetMaterial(material);
	}

	// 게임 오브젝트에 메쉬 붙히기
	gameObject->AddComponent(meshRenderer);

	GEngine->GetCmdQueue()->WaitSync();
}

void Game::Update()
{
	// input, timer Update 함수
	GEngine->Update();

	GEngine->RenderBegin();

	gameObject->Update();

	GEngine->RenderEnd();
}
