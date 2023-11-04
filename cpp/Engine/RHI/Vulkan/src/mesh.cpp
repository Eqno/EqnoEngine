#include "../include/mesh.h"

void Mesh::InitMesh(const Device& device,
	const Render& render,
	const Pipeline& pipeline) {
	texture.CreateTextureImage(device, data, render);
	texture.CreateTextureImageView(device.GetLogical());
	texture.CreateTextureSampler(device);

	buffer.CreateVertexBuffer(device, data, render);
	buffer.CreateIndexBuffer(device, data, render);

	descriptor.CreateUniformBuffers(device);
	descriptor.CreateDescriptorPool(device.GetLogical());
	descriptor.CreateDescriptorSets(device.GetLogical(),
		pipeline.GetDescriptorSetLayout(),
		texture);
}

void Mesh::DestroyMesh(const VkDevice& device) const {
	descriptor.Destroy(device);
	texture.Destroy(device);
	buffer.CleanupBuffers(device);
}
