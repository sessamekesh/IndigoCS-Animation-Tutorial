#include "MixamoCharacter.h"
#include "Logger.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/config.h>
#include <sstream>
#include <queue>

#ifndef VALIDATE
#define VALIDATE(hr, msg) if (FAILED(hr)) { Logger::Log(msg); return false; }
#endif

const char * MixamoCharacter::MODEL_FILENAME = "../../assets/Beta.fbx";
const char * MixamoCharacter::ANIMATION_FILENAME = "../../assets/samba_dancing.fbx";

MixamoCharacter::MixamoCharacter(std::shared_ptr<ShaderPNS4_MD1> shader, ComPtr<ID3D11DeviceContext> context, Transform transform)
	: ISceneNode(transform)
	, renderContext_(context)
	, shader_(shader)
	, models_()
{}

std::future<bool> MixamoCharacter::Initialize(ComPtr<ID3D11Device> device)
{
	return std::async(std::launch::async, [this, device] {
		return InitVertexAndIndexBuffers(device);
	});
}

void MixamoCharacter::SetTransform(Transform transform)
{
	transform_ = transform;
}

bool MixamoCharacter::Update(float dt)
{
	return true;
}

bool MixamoCharacter::Render()
{
	std::uint32_t stride = sizeof(ShaderPNS4_MD1::Vertex);
	std::uint32_t offset = 0u;

	bool isValid = true;

	for (ModelData model : models_)
	//ModelData model = models_[1u];
	{
		renderContext_->IASetVertexBuffers(0, 1, model.VertexBuffer.GetAddressOf(), &stride, &offset);
		renderContext_->IASetIndexBuffer(model.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		renderContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		shader_->SetModelMatrix((transform_ * model.Transform).GetTransformMatrix());
		shader_->SetObjectMaterial(model.Material);
		isValid &= shader_->Render(renderContext_, model.NumIndices);
	}

	return isValid;
}

bool MixamoCharacter::InitVertexAndIndexBuffers(ComPtr<ID3D11Device> device)
{
	Logger::Log("Loading mixamo character");

	std::uint32_t nFaces = 0u;

	auto mp = std::async(std::launch::async, [] { return aiImportFile(MixamoCharacter::MODEL_FILENAME, aiProcessPreset_TargetRealtime_MaxQuality); });
	auto ap = std::async(std::launch::async, [] { return aiImportFile(MixamoCharacter::ANIMATION_FILENAME, aiProcessPreset_TargetRealtime_MaxQuality); });

	const aiScene* mixamoModel = mp.get();
	const aiScene* animation = ap.get();
	if (!mixamoModel || !animation)
	{
		Logger::Log("Failed to load mixamo character model!");
		return false;
	}

	// Create each model, each of which should have a different material for use
	models_.reserve(mixamoModel->mNumMeshes);

	// Block to introduce scope of the vector
	{
		// Using a vector to prevent frequent memory allocations and frees between models in the mesh
		std::vector<ShaderPNS4_MD1::Vertex> vertices;
		std::vector<std::uint32_t> indices;
		for (std::uint32_t meshIdx = 0u; meshIdx < mixamoModel->mNumMeshes; meshIdx++)
		{
			auto mesh = mixamoModel->mMeshes[meshIdx];

			vertices.clear();
			indices.clear();
			vertices.reserve(mixamoModel->mMeshes[meshIdx]->mNumVertices);
			indices.reserve(mixamoModel->mMeshes[meshIdx]->mNumFaces * 3u);
			nFaces += mixamoModel->mMeshes[meshIdx]->mNumFaces;

			if (!mixamoModel->mMeshes[meshIdx]->mNormals)
			{
				Logger::Log("Could not find normals for mesh (mixamo)");
				return false;
			}

			if (mixamoModel->mMeshes[meshIdx]->mNumFaces == 0u)
			{
				Logger::Log("Mesh (mixamo model) does not contain any faces");
				continue;
			}

			for (std::uint32_t vertIdx = 0u; vertIdx < mixamoModel->mMeshes[meshIdx]->mNumVertices; vertIdx++)
			{
				auto v = mixamoModel->mMeshes[meshIdx]->mVertices[vertIdx];
				auto n = mixamoModel->mMeshes[meshIdx]->mNormals[vertIdx];

				ShaderPNS4_MD1::Vertex toAdd;
				toAdd.Position = Vec4(v.x, v.y, v.z, 1.f);
				toAdd.Normal = Vec4(n.x, n.y, n.z, 0.f);

				vertices.push_back(toAdd);
			}

			std::vector<std::string> boneNames;
			std::vector<Transform> boneOffsets;
			boneNames.reserve(mixamoModel->mMeshes[meshIdx]->mNumBones);
			boneOffsets.reserve(mixamoModel->mMeshes[meshIdx]->mNumBones);

			for (std::uint32_t boneIdx = 0u; boneIdx < mixamoModel->mMeshes[meshIdx]->mNumBones; boneIdx++)
			{
				auto bone = mixamoModel->mMeshes[meshIdx]->mBones[boneIdx];
				boneNames.push_back(std::string(bone->mName.C_Str()));
				boneOffsets.push_back(Transform::FromTransformMatrix(
					Matrix(
						bone->mOffsetMatrix.a1, bone->mOffsetMatrix.a2, bone->mOffsetMatrix.a3, bone->mOffsetMatrix.a4,
						bone->mOffsetMatrix.b1, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.b4,
						bone->mOffsetMatrix.c1, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.c4,
						bone->mOffsetMatrix.d1, bone->mOffsetMatrix.d2, bone->mOffsetMatrix.d3, bone->mOffsetMatrix.d4
						)));
			}

			unsigned int k = 0u;
			for (std::uint32_t faceIdx = 0u; faceIdx < mixamoModel->mMeshes[meshIdx]->mNumFaces; faceIdx++)
			{
#if defined(DEBUG) | defined(_DEBUG)
				if (mixamoModel->mMeshes[meshIdx]->mFaces[faceIdx].mNumIndices != 3u)
				{
					Logger::Log("Mesh (mixamo model) does not contain triangulated faces");
					return false;
				}
#endif

				indices.push_back(mixamoModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[0u]);
				indices.push_back(mixamoModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[1u]);
				indices.push_back(mixamoModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[2u]);
			}

			// Vertices and indices are loaded, create buffers
			D3D11_BUFFER_DESC vbDesc = { 0 };
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbDesc.ByteWidth = sizeof(ShaderPNS4_MD1::Vertex) * (UINT)vertices.size();
			vbDesc.CPUAccessFlags = 0x00;
			vbDesc.MiscFlags = 0x00;
			vbDesc.StructureByteStride = 0x00;
			vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_BUFFER_DESC ibDesc = { 0 };
			ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibDesc.ByteWidth = sizeof(std::uint32_t) * (UINT)indices.size();
			ibDesc.CPUAccessFlags = 0x00;
			ibDesc.MiscFlags = 0x00;
			ibDesc.StructureByteStride = 0x00;
			ibDesc.Usage = D3D11_USAGE_IMMUTABLE;

			// Vector elements must be stored contiguously. As per the C++11 standard,
			//  they don't necessarily have to be stored as an array, but the identity
			//  &v[n] = &v[0] + n for all 0 <= n < v.size() must hold true.
			// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#69
			D3D11_SUBRESOURCE_DATA vertexData = { 0 };
			D3D11_SUBRESOURCE_DATA indexData = { 0 };
			vertexData.pSysMem = &vertices[0];
			indexData.pSysMem = &indices[0];

			HRESULT hr = { 0 };
			ModelData nextModel;
			hr = device->CreateBuffer(&vbDesc, &vertexData, &(nextModel.VertexBuffer));
			VALIDATE(hr, "Failed to create vertex buffer (road model)");

			hr = device->CreateBuffer(&ibDesc, &indexData, &nextModel.IndexBuffer);
			VALIDATE(hr, "Failed to create index buffer (road model)");

			nextModel.NumIndices = (std::uint32_t)indices.size();

			// Material
			auto foo = mixamoModel->mMaterials[mixamoModel->mMeshes[meshIdx]->mMaterialIndex];
			aiColor4D diffuseColor;
			aiColor4D ambientColor;
			aiColor4D specularColor;
			float shininess;

			aiGetMaterialColor(foo, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
			aiGetMaterialColor(foo, AI_MATKEY_COLOR_DIFFUSE, &ambientColor);
			aiGetMaterialColor(foo, AI_MATKEY_COLOR_SPECULAR, &specularColor);
			aiGetMaterialFloat(foo, AI_MATKEY_SHININESS, &shininess);

			nextModel.Material.AmbientColor = { ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };
			nextModel.Material.DiffuseColor = { diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
			nextModel.Material.SpecularColor = { specularColor.r, specularColor.g, specularColor.b, specularColor.a };
			nextModel.Material.SpecularColor.w = shininess;

			// Transform
			nextModel.Transform = Transform(); // Default identity transformation. We'll try this out.

			models_.push_back(nextModel); // Lol, add the new model to the list!
		}
	}

	aiReleaseImport(mixamoModel);
	aiReleaseImport(animation);

	std::stringstream ss;
	ss << "The mixamo model has " << nFaces << " faces. Crazy, right?";
	Logger::Log(ss.str());

	return true;
}