#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Math/Vector2.hpp>
#include <Ailurus/Math/Vector3.hpp>
#include <Ailurus/Assert.h>

namespace Ailurus
{
	template <typename T>
	static void WriteBuffer(std::vector<uint8_t>& buffer, size_t* offset, const T& value)
	{
		ASSERT_MSG(offset + sizeof(T) <= buffer.size(), "Write buffer oversize");
		std::memcpy(buffer.data() + *offset, &value, sizeof(T));
		*offset += sizeof(T);
	}

	static VertexAttributeDescription ReadLayout(const aiMesh* pAssimpMesh)
	{
		std::vector<AttributeType> vertexAttrVec;

		if (pAssimpMesh->HasPositions())
			vertexAttrVec.push_back(AttributeType::Position);

		if (pAssimpMesh->HasVertexColors(0))
			vertexAttrVec.push_back(AttributeType::Color);

		if (pAssimpMesh->HasNormals())
			vertexAttrVec.push_back(AttributeType::Normal);

		if (pAssimpMesh->HasTextureCoords(0))
			vertexAttrVec.push_back(AttributeType::TexCoord);

		if (pAssimpMesh->HasTangentsAndBitangents())
		{
			vertexAttrVec.push_back(AttributeType::Tangent);
			vertexAttrVec.push_back(AttributeType::Bitangent);
		}

		return VertexAttributeDescription(vertexAttrVec);
	}

	static std::vector<uint8_t> ReadVertex(const aiMesh* pAssimpMesh, const VertexAttributeDescription& layout)
	{
		const auto vertexSize = layout.GetStride();
		const auto vertexNum = pAssimpMesh->mNumVertices;
		const auto dataBufferSizeInBytes = vertexSize * vertexNum;

		std::vector<uint8_t> data;
		data.resize(dataBufferSizeInBytes);

		size_t offset = 0;
		for (auto i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			size_t texCoordIndex = 0;
			size_t vertexColorIndex = 0;
			for (const auto attr : layout.GetAttributes())
			{
				switch (attr)
				{
					case AttributeType::Position:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mVertices[i].x);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mVertices[i].y);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mVertices[i].z);
						break;
					case AttributeType::Color:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mColors[vertexColorIndex][i].r);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mColors[vertexColorIndex][i].g);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mColors[vertexColorIndex][i].b);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mColors[vertexColorIndex][i].a);
						vertexColorIndex++;
						break;
					case AttributeType::Normal:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mNormals[i].x);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mNormals[i].y);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mNormals[i].z);
						break;
					case AttributeType::TexCoord:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mTextureCoords[texCoordIndex][i].x);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mTextureCoords[texCoordIndex][i].y);
						texCoordIndex++;
						break;
					case AttributeType::Tangent:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mTangents[i].x);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mTangents[i].y);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mTangents[i].z);
						break;
					case AttributeType::Bitangent:
						WriteBuffer<float>(data, &offset, pAssimpMesh->mBitangents[i].x);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mBitangents[i].y);
						WriteBuffer<float>(data, &offset, pAssimpMesh->mBitangents[i].z);
						break;
				}
			}
		}

		return data;
	}

	static IndexBufferFormat ReadIndexFormat(const aiMesh* pAssimpMesh)
	{
		return pAssimpMesh->mNumFaces * 3 < std::numeric_limits<uint16_t>::max()
			? IndexBufferFormat::UInt16
			: IndexBufferFormat::UInt32;
	}

	static std::vector<uint8_t> ReadIndex(const aiMesh* pAssimpMesh, IndexBufferFormat indexFormat)
	{
		const auto indexSizeInByte = VertexAttributeDescription::SizeOf(indexFormat);
		const auto faceNum = pAssimpMesh->mNumFaces;
		constexpr auto faceVertexNum = 3;
		const auto dataBufferSizeInBytes = faceVertexNum * faceNum * indexSizeInByte;

		std::vector<uint8_t> meshIndexData;
		meshIndexData.resize(dataBufferSizeInBytes);

		size_t writeOffset = 0;
		for (auto i = 0; i < pAssimpMesh->mNumFaces; i++)
		{
			const aiFace& face = pAssimpMesh->mFaces[i];

			if (face.mNumIndices != 3)
			{
				Logger::LogError("Only support triangle mesh, mesh index count = {}", face.mNumIndices);
				continue;
			}

			if (indexFormat == IndexBufferFormat::UInt16)
			{
				for (auto j = 0; j < face.mNumIndices; j++)
					WriteBuffer<uint16_t>(meshIndexData, &writeOffset, face.mIndices[j]);
			}
			else if (indexFormat == IndexBufferFormat::UInt32)
			{
				for (auto j = 0; j < face.mNumIndices; j++)
					WriteBuffer<uint32_t>(meshIndexData, &writeOffset, face.mIndices[j]);
			}
		}

		return meshIndexData;
	}

	static std::unique_ptr<Mesh> GenerateMesh(const aiMesh* pAssimpMesh)
	{
		VertexAttributeDescription layout = ReadLayout(pAssimpMesh);
		std::vector<uint8_t> vertexData = ReadVertex(pAssimpMesh, layout);

		if (pAssimpMesh->HasFaces())
		{
			IndexBufferFormat indexFormat = ReadIndexFormat(pAssimpMesh);
			std::vector<uint8_t> indexData = ReadIndex(pAssimpMesh, indexFormat);
			return std::make_unique<Mesh>(vertexData.data(), vertexData.size(), layout,
				indexFormat, indexData.data(), indexData.size());
		}

		return std::make_unique<Mesh>(vertexData.data(), vertexData.size(), layout);
	}

	static void AssimpProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene,
		std::vector<std::unique_ptr<Mesh>>& resultMeshVec)
	{
		resultMeshVec.clear();

		for (unsigned int i = 0; i < pAssimpNode->mNumMeshes; i++)
		{
			const aiMesh* pAssimpMesh = pAssimpScene->mMeshes[pAssimpNode->mMeshes[i]];
			resultMeshVec.push_back(std::move(GenerateMesh(pAssimpMesh)));
		}

		for (auto i = 0; i < pAssimpNode->mNumChildren; i++)
			AssimpProcessNode(pAssimpNode->mChildren[i], pAssimpScene, resultMeshVec);
	}

	bool Model::LoadFromFile(const std::string& path)
	{
		Assimp::Importer importer;

		constexpr auto importFlags =
			aiProcess_Triangulate
			| aiProcess_FlipUVs
			// | aiProcess_CalcTangentSpace // Auto generated tangent & bitangent
			| aiProcess_SortByPType;

		const aiScene* pAssimpScene = importer.ReadFile(path, importFlags);
		if (!pAssimpScene || pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pAssimpScene->mRootNode)
		{
			Logger::LogError("Failed to load mesh from path: {}\n\t Error: {}", path, importer.GetErrorString());
			return false;
		}

		AssimpProcessNode(pAssimpScene->mRootNode, pAssimpScene, _meshes);

		return true;
	}
} // namespace Ailurus