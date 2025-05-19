#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Ailurus/Application/AssetsSystem/AssetsSystem.h"
#include "Ailurus/Application/AssetsSystem/Mesh.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Math/Vector3.hpp"

namespace Ailurus
{

	AssetsManager::~AssetsManager()
	{
	}

	AssetsManager::AssetsManager()
	{
	}

	struct Vertex {
		Vector3f Position;
		Vector3f Normal;
		Vector2f TexCoords;
		Vector3f Tangent;
		Vector3f Bitangent;
	};

	struct Texture {
		unsigned int id;
		std::string type;
		std::string path;
	};

	static Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			if (mesh->mTextureCoords[0])
			{
				vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.TexCoords = Vector2f::Zero;
			}

			// u
			vertex.Tangent.x = mesh->mTangents[i].x;
			vertex.Tangent.y = mesh->mTangents[i].y;
			vertex.Tangent.z = mesh->mTangents[i].z;

			// v
			vertex.Bitangent.x = mesh->mBitangents[i].x;
			vertex.Bitangent.y = mesh->mBitangents[i].y;
			vertex.Bitangent.z = mesh->mBitangents[i].z;

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
	}

	void processNode(aiNode *node, const aiScene *scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	template <>
	AssetReference<Mesh> AssetsManager::LoadAsset<Mesh>(const std::string& path)
	{
		auto itr = _meshMap.find(path);
		if (itr != _meshMap.end())
			return AssetReference(reinterpret_cast<Mesh*>(itr->second.get()));

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Logger::LogError("Failed to load mesh from path: {}\n\t Error: {}", path, importer.GetErrorString());
			return AssetReference<Mesh>(nullptr);
		}


	}
} // namespace Ailurus