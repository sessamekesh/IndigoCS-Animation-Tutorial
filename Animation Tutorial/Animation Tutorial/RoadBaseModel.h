#pragma once

#include "ISceneNode.h"
#include "BasicShaderMD.h"
#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
using Microsoft::WRL::ComPtr;

class RoadBaseModel : public ISceneNode
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
	static const char * FILENAME;

public:
	RoadBaseModel() = delete;
	RoadBaseModel(std::shared_ptr<BasicShaderMD> shader, ComPtr<ID3D11DeviceContext> context, Transform transform);
	RoadBaseModel(const RoadBaseModel&) = delete;
	~RoadBaseModel() = default;

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
	std::shared_ptr<BasicShaderMD> shader_;
	std::vector<ModelData> models_;
};