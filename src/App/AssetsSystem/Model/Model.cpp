#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Math/Vector2.hpp>
#include <Ailurus/Math/Vector3.hpp>

namespace Ailurus
{
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
		std::vector<uint8_t> meshVertexData;
		meshVertexData.resize(layout.GetStride());

		auto pFloatArray = reinterpret_cast<float*>(meshVertexData.data());
		size_t writeIndex = 0;

		auto WriteData = [&pFloatArray, &writeIndex](float value) {
			pFloatArray[writeIndex] = value;
			writeIndex++;
		};

		for (auto i = 0; i < pAssimpMesh->mNumVertices; i++)
		{
			size_t texCoordIndex = 0;
			size_t vertexColorIndex = 0;
			for (const auto attr : layout.GetAttributes())
			{
				switch (attr)
				{
					case AttributeType::Position:
						WriteData(pAssimpMesh->mVertices[i].x);
						WriteData(pAssimpMesh->mVertices[i].y);
						WriteData(pAssimpMesh->mVertices[i].z);
						break;
					case AttributeType::Color:
						WriteData(pAssimpMesh->mColors[vertexColorIndex][i].r);
						WriteData(pAssimpMesh->mColors[vertexColorIndex][i].g);
						WriteData(pAssimpMesh->mColors[vertexColorIndex][i].b);
						WriteData(pAssimpMesh->mColors[vertexColorIndex][i].a);
						vertexColorIndex++;
						break;
					case AttributeType::Normal:
						WriteData(pAssimpMesh->mNormals[i].x);
						WriteData(pAssimpMesh->mNormals[i].y);
						WriteData(pAssimpMesh->mNormals[i].z);
						break;
					case AttributeType::TexCoord:
						WriteData(pAssimpMesh->mTextureCoords[texCoordIndex][i].x);
						WriteData(pAssimpMesh->mTextureCoords[texCoordIndex][i].y);
						texCoordIndex++;
						break;
					case AttributeType::Tangent:
						WriteData(pAssimpMesh->mTangents[i].x);
						WriteData(pAssimpMesh->mTangents[i].y);
						WriteData(pAssimpMesh->mTangents[i].z);
						break;
					case AttributeType::Bitangent:
						WriteData(pAssimpMesh->mBitangents[i].x);
						WriteData(pAssimpMesh->mBitangents[i].y);
						WriteData(pAssimpMesh->mBitangents[i].z);
						break;
				}
			}
		}

		return meshVertexData;
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

		std::vector<uint8_t> meshIndexData;
		meshIndexData.resize(pAssimpMesh->mNumFaces * 3 * indexSizeInByte);
		size_t writeIndex = 0;

		auto WriteData = [&meshIndexData, &writeIndex, &indexFormat](uint64_t value) {
			uint8_t* pData = meshIndexData.data();
			switch (indexFormat)
			{
				case IndexBufferFormat::UInt16:
				{
					auto pIndexData = reinterpret_cast<uint16_t*>(pData);
					pIndexData[writeIndex] = value;
					break;
				}
				case IndexBufferFormat::UInt32:
				{
					auto pIndexData = reinterpret_cast<uint32_t*>(pData);
					pIndexData[writeIndex] = value;
					break;
				}
			}
			writeIndex++;
		};

		for (auto i = 0; i < pAssimpMesh->mNumFaces; i++)
		{
			const aiFace& face = pAssimpMesh->mFaces[i];

			if (face.mNumIndices != 3)
			{
				Logger::LogError("Only support triangle mesh, mesh index count = {}", face.mNumIndices);
				continue;
			}

			for (auto j = 0; j < face.mNumIndices; j++)
				WriteData(face.mIndices[j]);
		}

		return meshIndexData;
	}

	static std::unique_ptr<Mesh> GenerateMesh(const aiMesh* pAssimpMesh)
	{
		VertexAttributeDescription layout = ReadLayout(pAssimpMesh);
		std::vector<uint8_t> vertexData = ReadVertex(pAssimpMesh, layout);
		IndexBufferFormat indexFormat = ReadIndexFormat(pAssimpMesh);
		std::vector<uint8_t> indexData = ReadIndex(pAssimpMesh, indexFormat);

		return std::make_unique<Mesh>(vertexData.data(), vertexData.size(), layout,
			indexFormat, indexData.data(), indexData.size());
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

		const aiScene* pAssimpScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!pAssimpScene || pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pAssimpScene->mRootNode)
		{
			Logger::LogError("Failed to load mesh from path: {}\n\t Error: {}", path, importer.GetErrorString());
			return false;
		}

		AssimpProcessNode(pAssimpScene->mRootNode, pAssimpScene, _meshes);

		return true;
	}
} // namespace Ailurus