#include "RoadBaseModel.h"
#include "Logger.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#ifndef VALIDATE
#define VALIDATE(hr, msg) if (FAILED(hr)) { Logger::Log(msg); return false; }
#endif

const char * RoadBaseModel::FILENAME = "../../assets/Road.fbx";

RoadBaseModel::RoadBaseModel(std::shared_ptr<BasicShaderMD> shader, ComPtr<ID3D11DeviceContext> context, Transform transform)
	: ISceneNode(transform)
	, renderContext_(context)
	, shader_(shader)
	, models_()
{}

std::future<bool> RoadBaseModel::Initialize(ComPtr<ID3D11Device> device)
{
	return std::async(std::launch::async, [this, device] {
		return InitVertexAndIndexBuffers(device);
	});
}

void RoadBaseModel::SetTransform(Transform transform)
{
	transform_ = transform;
}

bool RoadBaseModel::Update(float dt)
{
	// No animation in this model, so just return true
	return true;
}

bool RoadBaseModel::Render()
{
	std::uint32_t stride = sizeof(BasicShaderMD::Vertex);
	std::uint32_t offset = 0u;
	
	bool isValid = true;

	for (ModelData model : models_)
	{
		// NEXT TIME: Optimize this by passing to a manager to render all
		//  things at once that require the same shader and bindings.
		// Try to minimize graphics card data binding changes, etc.
		renderContext_->IASetVertexBuffers(0, 1, model.VertexBuffer.GetAddressOf(), &stride, &offset);
		renderContext_->IASetIndexBuffer(model.IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		renderContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		shader_->SetModelMatrix((transform_ * model.Transform).GetTransformMatrix());
		shader_->SetObjectMaterial(model.Material);
		isValid &= shader_->Render(renderContext_, model.NumIndices);
	}

	return isValid;
}

bool RoadBaseModel::InitVertexAndIndexBuffers(ComPtr<ID3D11Device> device)
{
	Logger::Log("Loading road base model");
	const aiScene* roadBaseModel = aiImportFile(RoadBaseModel::FILENAME, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!roadBaseModel)
	{
		Logger::Log("Failed to load road model!");
		return false;
	}

	// Create each model, each of which should have different materials for use
	models_.reserve(roadBaseModel->mNumMeshes);

	// Block to introduce scope to the vector
	{
		// Using a vector to prevent frequent memory allocations and frees between
		//  models in the mesh.
		std::vector<BasicShaderMD::Vertex> vertices;
		std::vector<std::uint16_t> indices;
		for (std::uint8_t meshIdx = 0u; meshIdx < roadBaseModel->mNumMeshes; meshIdx++)
		{
			vertices.clear();
			indices.clear();
			vertices.reserve(roadBaseModel->mMeshes[meshIdx]->mNumVertices);
			indices.reserve(roadBaseModel->mMeshes[meshIdx]->mNumFaces * 3u);

			if (!roadBaseModel->mMeshes[meshIdx]->mNormals)
			{
				Logger::Log("Could not find normals for mesh (road model)");
				return false;
			}

			if (roadBaseModel->mMeshes[meshIdx]->mNumFaces == 0u)
			{
				Logger::Log("Mesh (road model) does not contain any faces");
				continue;
			}

			//for (std::uint16_t vertIdx = 0u; vertIdx < roadBaseModel->mMeshes[meshIdx]->mNumVertices; vertIdx++)
			//{
			//	auto v = roadBaseModel->mMeshes[meshIdx]->mVertices[vertIdx];
			//	auto n = roadBaseModel->mMeshes[meshIdx]->mNormals[vertIdx];

			//	vertices.push_back(BasicShaderMD::Vertex(Vec4(v.x, v.y, v.z, 1.f), Vec4(n.x, n.y, n.z, 0.f)));
			//}

			unsigned int k = 0u;
			for (std::uint16_t faceIdx = 0u; faceIdx < roadBaseModel->mMeshes[meshIdx]->mNumFaces; faceIdx++)
			{
#if defined(DEBUG) | defined(_DEBUG)
				if (roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mNumIndices != 3u)
				{
					Logger::Log("Mesh (road model) does not contain triangulated faces");
					return false;
				}
#endif
				auto v1 = roadBaseModel->mMeshes[meshIdx]->mVertices[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[0u]];
				auto v2 = roadBaseModel->mMeshes[meshIdx]->mVertices[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[1u]];
				auto v3  = roadBaseModel->mMeshes[meshIdx]->mVertices[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[2u]];
				auto n = Vec3::Cross(Vec3(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z), Vec3(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z)).Normal();
				//auto n = roadBaseModel->mMeshes[meshIdx]->mNormals[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[0u]];
				vertices.push_back(BasicShaderMD::Vertex(Vec4(v1.x, v1.y, v1.z, 1.f), Vec4(n.x, n.y, n.z, 0.f)));

				//n = roadBaseModel->mMeshes[meshIdx]->mNormals[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[1u]];
				vertices.push_back(BasicShaderMD::Vertex(Vec4(v2.x, v2.y, v2.z, 1.f), Vec4(n.x, n.y, n.z, 0.f)));

				//n = roadBaseModel->mMeshes[meshIdx]->mNormals[roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[2u]];
				vertices.push_back(BasicShaderMD::Vertex(Vec4(v3.x, v3.y, v3.z, 1.f), Vec4(n.x, n.y, n.z, 0.f)));

				//indices.push_back(roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[0u]);
				//indices.push_back(roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[1u]);
				//indices.push_back(roadBaseModel->mMeshes[meshIdx]->mFaces[faceIdx].mIndices[2u]);
				indices.push_back(k++);
				indices.push_back(k++);
				indices.push_back(k++);
			}

			// Vertices and indices are loaded, create buffers
			D3D11_BUFFER_DESC vbDesc = { 0 };
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbDesc.ByteWidth = sizeof(BasicShaderMD::Vertex) * (UINT)vertices.size();
			vbDesc.CPUAccessFlags = 0x00;
			vbDesc.MiscFlags = 0x00;
			vbDesc.StructureByteStride = 0x00;
			vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_BUFFER_DESC ibDesc = { 0 };
			ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibDesc.ByteWidth = sizeof(std::uint16_t) * (UINT)indices.size();
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
			auto foo = roadBaseModel->mMaterials[roadBaseModel->mMeshes[meshIdx]->mMaterialIndex];
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

			// Lol, add the new model to the list!
			models_.push_back(nextModel);
		}
	}

	aiReleaseImport(roadBaseModel);

	return true;
}