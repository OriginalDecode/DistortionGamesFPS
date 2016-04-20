#include "GameWrapper.h"
#include "GameEnum.h"

#include <Engine.h>
#include <DL_Debug.h>


#include <ModelLoader.h>
#include <ModelProxy.h>

#include <Scene.h>
#include <DeferredRenderer.h>
#include <Instance.h>
#include <InputWrapper.h>
#include <Camera.h>

#include <XMLReader.h>
#include <MathHelper.h>
#include <PointLight.h>

#include <ClientLevel.h>
#include <ClientLevelFactory.h>

#include <Cursor.h>
#include <PostMaster.h>
#include <AudioInterface.h>
#include <ClientNetworkManager.h>

GameWrapper::GameWrapper(float aHeight, float aWidth, ID3D11Device* aDevice, ID3D11DeviceContext* aContext)
{
	DL_Debug::Debug::Create();
	Prism::Engine::CreateOcculus(aHeight, aWidth, aDevice, aContext);
}


GameWrapper::~GameWrapper()
{
	Prism::Audio::AudioInterface::Destroy();
	PostMaster::Destroy();
	ClientNetworkManager::Destroy();
	Prism::Engine::Destroy();
	DL_Debug::Debug::Destroy();
}

void GameWrapper::SetContext(ID3D11DeviceContext* aContext)
{
	Prism::Engine::GetInstance()->SetContext(aContext);
}

void GameWrapper::SetDevice(ID3D11Device* aDevice)
{
	Prism::Engine::GetInstance()->SetDevice(aDevice);
}

void GameWrapper::SetWindowSize(const CU::Vector2<float>& aWindowSize)
{
	Prism::Engine::GetInstance()->SetWindowSize(aWindowSize);
}

void GameWrapper::Init()
{
	CU::InputWrapper::Create(GetActiveWindow(), GetModuleHandle(NULL), DISCL_NONEXCLUSIVE
		| DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	Prism::Audio::AudioInterface::CreateInstance();
	PostMaster::Create();
	ClientNetworkManager::Create();

	Prism::ModelLoader::GetInstance()->Pause();
	myScene = new Prism::Scene();
	myRenderer = new Prism::DeferredRenderer();
	myCamera = new Prism::Camera(myPlayerMatrix);
	myScene->SetCamera(*myCamera);

	myModels.Init(8);
	Prism::Model* model = new Prism::Model();
	model->InitCube(1.f, 1.f, 1.f, CU::Vector4<float>(1.f, 1.f, 1.f, 1.f));
	myModels.Add(model);

	Prism::Model* model1 = new Prism::Model();
	model1->InitCube(1.f, 1.f, 1.f, CU::Vector4<float>(0.f, 1.f, 0.f, 1.f));
	myModels.Add(model1);

	Prism::Model* model2 = new Prism::Model();
	model2->InitCube(1.f, 1.f, 1.f, CU::Vector4<float>(0.f, 0.f, 1.f, 1.f));
	myModels.Add(model2);

	myOrientations.Init(8);
	myOrientations.Add(CU::Matrix44<float>());
	myOrientations.Add(CU::Matrix44<float>());
	myOrientations.Add(CU::Matrix44<float>());
	myOrientations.Add(CU::Matrix44<float>());

	myOrientations[0].SetPos(CU::Vector3<float>(1.5f, 0, 0));
	myOrientations[1].SetPos(CU::Vector3<float>(0, 1.5f, 0));
	myOrientations[2].SetPos(CU::Vector3<float>(0, 0, 1.5f));
	myOrientations[3].SetPos(CU::Vector3<float>(0, 0, -1.f));

	Prism::ModelProxy* test = Prism::ModelLoader::GetInstance()->LoadModel("Data/Resource/Model/Pickups/Healthpack/SM_health_pack.fbx", "Data/Resource/Shader/S_effect_pbl_deferred.fx");

	Prism::ModelProxy* test2 = Prism::ModelLoader::GetInstance()->LoadModelAnimated("Data/Resource/Model/Enemy/SK_cyborg_3_idle.fbx", "Data/Resource/Shader/S_effect_pbl_animated.fx");
	myAnimation = new Prism::Instance(*test2, myOrientations[3]);

	myScene->AddInstance(new Prism::Instance(*test, myOrientations[0]), eObjectRoomType::ALWAYS_RENDER);
	myScene->AddInstance(new Prism::Instance(*test, myOrientations[1]), eObjectRoomType::ALWAYS_RENDER);
	myScene->AddInstance(new Prism::Instance(*test, myOrientations[2]), eObjectRoomType::ALWAYS_RENDER);
	myScene->AddInstance(myAnimation, eObjectRoomType::ALWAYS_RENDER);

	LoadGym();
	
	myRenderer->SetCubeMap("level_01");
	Prism::ModelLoader::GetInstance()->UnPause();
	myCursor = new GUI::Cursor(Prism::Engine::GetInstance()->GetWindowSizeInt());;

	myLevelFactory = new ClientLevelFactory("Data/Level/LI_level.xml");

	eStateStatus status = eStateStatus::eKeepState;
	myLevel = static_cast<ClientLevel*>(myLevelFactory->LoadLevel(0, myCursor, status));
	myLevel->SetCamera(myCamera);
}

void GameWrapper::Update(float aDelta, const CU::Matrix44<float>& aView, const CU::Matrix44<float>& aProjection, const CU::Matrix44<float>& aViewProjection)
{
	myCamera->SetOrientation(aView);
	myCamera->SetProjection(aProjection);
	myCamera->SetViewProjection(aViewProjection);
	myCamera->Update(aDelta);

	myOrientations[0] *= CU::Matrix44<float>::CreateRotateAroundX(aDelta);

	myAnimation->Update(aDelta);
}

void GameWrapper::Render(const DirectX::XMMATRIX& aViewProjection, ID3D11RenderTargetView* aRenderTarget, ID3D11DepthStencilView* aDepthStencil)
{
	for (int i = 0; i < myModels.Size(); ++i)
	{
	//	myModels[i]->RenderOcculus(myOrientations[i], ConvertMatrix(aViewProjection));
	}

	//myRenderer->Render(myScene, aRenderTarget, aDepthStencil);
	myLevel->Render(aRenderTarget, aDepthStencil);
}

CU::Matrix44<float> GameWrapper::ConvertMatrix(const DirectX::XMMATRIX& aMatrix)
{
	CU::Matrix44<float> toReturn;

	toReturn.myMatrix[0] = -aMatrix.r[0].m128_f32[0];
	toReturn.myMatrix[1] = -aMatrix.r[0].m128_f32[1];
	toReturn.myMatrix[2] = -aMatrix.r[0].m128_f32[2];
	toReturn.myMatrix[3] = -aMatrix.r[0].m128_f32[3];
	toReturn.myMatrix[4] = aMatrix.r[1].m128_f32[0];
	toReturn.myMatrix[5] = aMatrix.r[1].m128_f32[1];
	toReturn.myMatrix[6] = aMatrix.r[1].m128_f32[2];
	toReturn.myMatrix[7] = aMatrix.r[1].m128_f32[3];
	toReturn.myMatrix[8] = aMatrix.r[2].m128_f32[0];
	toReturn.myMatrix[9] = aMatrix.r[2].m128_f32[1];
	toReturn.myMatrix[10] = aMatrix.r[2].m128_f32[2];
	toReturn.myMatrix[11] = aMatrix.r[2].m128_f32[3];
	toReturn.myMatrix[12] = aMatrix.r[3].m128_f32[0];
	toReturn.myMatrix[13] = aMatrix.r[3].m128_f32[1];
	toReturn.myMatrix[14] = aMatrix.r[3].m128_f32[2];
	toReturn.myMatrix[15] = aMatrix.r[3].m128_f32[3];

	return toReturn;
}

void GameWrapper::LoadGym()
{
	XMLReader reader;
	reader.OpenDocument("Data/Level/Level_01/level_01_gymbana.xml");

	tinyxml2::XMLElement* root = reader.ForceFindFirstChild("root");
	tinyxml2::XMLElement* scene = reader.ForceFindFirstChild(root, "scene");

	tinyxml2::XMLElement* prop = reader.ForceFindFirstChild(scene, "prop");
	for (; prop != nullptr; prop = reader.FindNextElement(prop, "prop"))
	{
		std::string type;
		reader.ForceReadAttribute(prop, "propType", type);

		CU::Vector3<float> position;
		CU::Vector3<float> rotation;
		CU::Matrix44<float> orientation;

		if (type.find("floor") != type.npos || type.find("wall") != type.npos || type.find("ceil") != type.npos)
		{
			reader.ForceReadAttribute(reader.ForceFindFirstChild(prop, "position"), "X", "Y", "Z", position);
			reader.ForceReadAttribute(reader.ForceFindFirstChild(prop, "rotation"), "X", "Y", "Z", rotation);

			rotation.x = CU::Math::DegreeToRad(rotation.x);
			rotation.y = CU::Math::DegreeToRad(rotation.y);
			rotation.z = CU::Math::DegreeToRad(rotation.z);

			
			orientation = orientation * CU::Matrix44f::CreateRotateAroundZ(rotation.z);
			orientation = orientation * CU::Matrix44f::CreateRotateAroundX(rotation.x);
			orientation = orientation * CU::Matrix44f::CreateRotateAroundY(rotation.y);
			orientation.SetPos(position);
			myOrientations.Add(orientation);
		}

		if (type.find("floor") != type.npos)
		{
			std::string model = "Data/Resource/Model/Modular_set/Dev_set/Floors/SM_dev_floor_200_x_200.fbx";
			if (type.find("400_400") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Floors/SM_dev_floor_400_400.fbx";
			}

			Prism::Instance* floor = new Prism::Instance(*Prism::ModelLoader::GetInstance()->LoadModel(model, "Data/Resource/Shader/S_effect_pbl_deferred.fx"), myOrientations.GetLast());
			myScene->AddInstance(floor, eObjectRoomType::ALWAYS_RENDER);
		}
		else if (type.find("wall") != type.npos)
		{
			std::string model = "Data/Resource/Model/Modular_set/Dev_set/Walls/SM_dev_wall_100_x_400_L.fbx";
			if (type.find("SM_dev_wall_100_x_400_R") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Walls/SM_dev_wall_100_x_400_R.fbx";
			}
			else if (type.find("SM_dev_wall_200_x_300") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Walls/SM_dev_wall_200_x_300.fbx";
			}
			else if (type.find("SM_dev_wall_200_x_400") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Walls/SM_dev_wall_200_x_400.fbx";
			}
			else if (type.find("SM_dev_wall_400_x_400") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Walls/SM_dev_wall_400_x_400.fbx";
			}

			Prism::Instance* wall = new Prism::Instance(*Prism::ModelLoader::GetInstance()->LoadModel(model, "Data/Resource/Shader/S_effect_pbl_deferred.fx"), myOrientations.GetLast());
			myScene->AddInstance(wall, eObjectRoomType::ALWAYS_RENDER);
		}
		else if (type.find("ceil") != type.npos)
		{
			std::string model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_100_200_b_.fbx";
			if (type.find("dev_ceiling_100_200_l_") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_100_200_l_.fbx";
			}
			else if (type.find("dev_ceiling_100_200_r_") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_100_200_r_.fbx";
			}
			else if (type.find("dev_ceiling_100_200_t_") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_100_200_t_.fbx";
			}
			else if (type.find("dev_ceiling_200_x_200") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_200_x_200.fbx";
			}
			else if (type.find("dev_ceiling_400_400") != type.npos)
			{
				model = "Data/Resource/Model/Modular_set/Dev_set/Ceilings/SM_dev_ceiling_400_400.fbx";
			}

			Prism::Instance* ceiling = new Prism::Instance(*Prism::ModelLoader::GetInstance()->LoadModel(model, "Data/Resource/Shader/S_effect_pbl_deferred.fx"), myOrientations.GetLast());
			myScene->AddInstance(ceiling, eObjectRoomType::ALWAYS_RENDER);
		}
	}

	tinyxml2::XMLElement* pointlight = reader.FindFirstChild(scene, "pointlight");
	for (; pointlight != nullptr; pointlight = reader.FindNextElement(pointlight, "pointlight"))
	{
		CU::Vector3<float> position;
		CU::Vector4<float> color;
		float range;

		reader.ForceReadAttribute(reader.ForceFindFirstChild(pointlight, "position"), "X", "Y", "Z", position);
		reader.ForceReadAttribute(reader.ForceFindFirstChild(pointlight, "color"), "R", "G", "B", "A", color);
		reader.ForceReadAttribute(reader.ForceFindFirstChild(pointlight, "range"), "value", range);

		Prism::PointLight* light = new Prism::PointLight(0, false);
		light->SetPosition(position);
		light->SetColor(color);
		light->SetRange(range);
		light->Update();
		myScene->AddLight(light);
	}

	reader.CloseDocument();
}
