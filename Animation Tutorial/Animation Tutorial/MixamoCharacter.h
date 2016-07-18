#pragma once

#include "ISceneNode.h"
#include "ShaderPNS4_MD1.h"
#include <wrl.h>
#include <string>
#include <memory>
#include <map>
#include <vector>
using Microsoft::WRL::ComPtr;

class MixamoCharacter : public ISceneNode
{
protected:
	struct ModelData
	{
	public:
		std::uint32_t NumIndices;
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
		Material Material;
		Transform Transform;

		ModelData()
			: NumIndices(0u)
			, VertexBuffer(nullptr)
			, IndexBuffer(nullptr)
			, Material(Material::BasicGray)
			, Transform()
		{}
	};

protected:
	static const char * MODEL_FILENAME;
	static const char * ANIMATION_FILENAME;

public:
	MixamoCharacter() = delete;
	~MixamoCharacter() = default;
	MixamoCharacter(const MixamoCharacter&) = delete;
	MixamoCharacter(std::shared_ptr<ShaderPNS4_MD1> shader, ComPtr<ID3D11DeviceContext> context, Transform transform);

	std::future<bool> Initialize(ComPtr<ID3D11Device> device);
	void SetTransform(Transform transform);

public:
	// Inherited via ISceneNode
	virtual bool Update(float dt) override;
	virtual bool Render() override;

private:
	bool InitVertexAndIndexBuffers(ComPtr<ID3D11Device> device);

private:
	ComPtr<ID3D11DeviceContext> renderContext_;
	std::shared_ptr<ShaderPNS4_MD1> shader_;
	std::vector<ModelData> models_;
};