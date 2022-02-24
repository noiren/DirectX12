#pragma once

#include <vector>
#include <map>
#include <d3d11.h>
#include "DirectGraphics.h"
#include "GraphicsUtitlity.h"

class DirectGraphics;

class ObjFile
{
public:
	/*コンストラクタ*/
	ObjFile()
	{
		m_Vertices.clear();
	}

	/*デストラクタ*/
	~ObjFile()
	{

	}

	bool Load(const char* file_name);

private:
	bool CreateMesh(std::vector<std::string>& out_material_list, const char* file_path, const char* file_name);

	void ParseVKeywordTag(std::vector<Vector3>& out_vertices, char* buff);

	void ParseFKeywordTag(std::vector<CustomVertex>& out_custom_vertices, std::map<std::string, int>& index_list, std::string current_material_name, std::vector<Vector3>& vertices, std::vector<Vector3>& normals, char* buffer);

	void ParseSlashKeywordTag(int* list, char* buffer);

	bool LoadMaterialFile(std::vector<std::string> file_list, std::string file_path);

private:
	std::vector<CustomVertex> m_Vertices;		//!< @brief 頂点バッファ
	std::map <std::string, std::vector<UINT>> m_Indices;//!< @brief インデックスバッファ
	std::map<std::string, ObjMaterial> m_Materials;

};