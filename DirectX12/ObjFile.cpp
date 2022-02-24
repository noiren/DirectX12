#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <codecvt> 
#include <cstdio>
#include "DirectGraphics.h"
#include "Utility.h"
#include "ObjFile.h"

bool ObjFile::Load(const char* file_name)
{
	char file_path[256];
	std::vector<std::string> out_material_list;
	int len = (int)strlen(file_name);
	int path_tail_point = 0;

	for (int i = len - 1; i >= 0; i--)
	{
		if (file_name[i] == '/')
		{
			path_tail_point = i;
			break;
		}
	}

	strncpy_s(file_path, file_name, path_tail_point + 1);

	if (CreateMesh(out_material_list, file_path, file_name) == false)
	{
		return false;
	}

	if (LoadMaterialFile(out_material_list, file_path) == false)
	{
		return false;
	}

	return true;
}

bool ObjFile::CreateMesh(std::vector<std::string>& out_material_list, const char* file_path, const char* file_name)
{
	FILE* fp = nullptr;
	fopen_s(&fp, file_name, "r");

	if (fp == nullptr)
	{
		return false;
	}

	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::map<std::string, int> index_list;
	std::string current_mat_name = "";

	const int LineBufferLength = 1024;
	char buffer[LineBufferLength];

	while (fgets(buffer, LineBufferLength, fp) != nullptr)
	{
		// コメントは無視
		if (buffer[0] == '#')
		{
			continue;
		}

		char* parse_point = strchr(buffer, ' ');
		if (parse_point == nullptr)
		{
			continue;
		}

		Replace('\n', '\0', buffer);

		// 頂点関連
		if (buffer[0] == 'v')
		{
			// 頂点座標
			if (buffer[1] == ' ')
			{
				ParseVKeywordTag(vertices, &parse_point[1]);
			}
			// 法線座標
			else if (buffer[1] == 'n')
			{
				ParseVKeywordTag(normals, &parse_point[1]);
			}
		}
		// 面情報
		else if (buffer[0] == 'f')
		{
			ParseFKeywordTag(m_Vertices, index_list, current_mat_name, vertices, normals, &parse_point[1]);
		}
		else if (strstr(buffer, "mtllib") == buffer)
		{
			Replace('\n', '\0', buffer);
			// マテリアルファイル名を保存
			out_material_list.push_back(&buffer[strlen("mtllib") + 1]);
		}
		else if (strstr(buffer, "usemtl") == buffer)
		{
			Replace('\n', '\0', buffer);
			// 所属マテリアル名の切り替え
			current_mat_name = &buffer[strlen("usemtl") + 1];
		}
	}

	fclose(fp);

	return true;
}

void ObjFile::ParseVKeywordTag(std::vector<Vector3>& data, char* buff)
{
	std::vector<std::string> split_strings;
	Split(' ', buff, split_strings);

	int count = 0;
	float values[3] = { 0.0f };

	for (std::string str : split_strings)
	{
		values[count] = (float)atof(str.c_str());
		count++;
	}

	data.push_back(Vector3(values[0], values[1], values[2]));
}

void ObjFile::ParseFKeywordTag(std::vector<CustomVertex> & out_custom_vertices, std::map<std::string, int> & index_list, std::string current_material, std::vector<Vector3> & vertices, std::vector<Vector3> & normals, char* buffer)
{
	int count = 0;
	int vertex_info[3] =
	{
		-1, -1, -1,
	};
	std::vector<std::string> space_split;

	Split(' ', buffer, space_split);

	for (int i = 0; i < space_split.size(); i++)
	{
		CustomVertex vertex;
		ParseSlashKeywordTag(vertex_info, (char*)space_split[i].c_str());

		for (int i = 0; i < 3; i++)
		{
			if (vertex_info[i] == -1)
			{
				continue;
			}

			int id = vertex_info[i];

			switch (i)
			{
			case 0:
				vertex.Position = vertices[id];
				break;
			case 2:
				vertex.Normal = normals[id];
				break;
			}
		}

		// 最適化
#define OPTIMIZATION (0)
#if OPTIMIZATION
		std::string key = "";

		for (int i = 0; i < 3; i++)
		{
			std::ostringstream sout;
			sout << std::setfill('0') << std::setw(5) << vertex_info[i];
			key += sout.str();
		}

		if (index_list.count(key) > 0)
		{
			m_Indices[current_material].push_back(index_list[key]);
		}
		else
		{
			// 頂点バッファリストに追加
			out_custom_vertices.push_back(vertex);
			m_Indices[current_material].push_back(out_custom_vertices.size() - 1);
			index_list[key] = out_custom_vertices.size() - 1;
		}
#else
		// 頂点バッファリストに追加
		out_custom_vertices.push_back(vertex);
		m_Indices[current_material].push_back((UINT)out_custom_vertices.size() - 1);

#endif
	}
}

void ObjFile::ParseSlashKeywordTag(int* list, char* buffer)
{
	int counter = 0;
	std::vector<std::string> slash_split;
	Split('/', buffer, slash_split);

	for (std::string str : slash_split)
	{
		if (str.size() > 0)
		{
			list[counter] = atoi(str.c_str()) - 1;
		}
		counter++;

	}
}

bool ObjFile::LoadMaterialFile(std::vector<std::string> file_list, std::string file_path)
{
	char buffer[1024];

	for (auto mat_file_name : file_list)
	{
		FILE* fp = nullptr;
		std::string name = mat_file_name;
		//name += mat_file_name;

		fopen_s(&fp, name.c_str(), "r");

		if (fp == nullptr)
		{
			return false;
		}
		std::string current_material_name = "";

		while (fgets(buffer, 1024, fp) != nullptr)
		{
			// マテリアルグループ名
			if (strstr(buffer, "newmtl") == buffer)
			{
				Replace('\n', '\0', buffer);
				current_material_name = &buffer[strlen("newmtl") + 1];
			}
			// Ambientカラー
			else if (strstr(buffer, "Ka") == buffer)
			{
				Replace('\n', '\0', buffer);
				std::vector<std::string> split;
				Split(' ', &buffer[strlen("Ka") + 1], split);

				for (int i = 0; i < split.size(); i++)
				{
					m_Materials[current_material_name].Ambient[i] = (float)atof(split[i].c_str());
				}
			}
			// Diffuseカラー
			else if (strstr(buffer, "Kd") == buffer)
			{
				Replace('\n', '\0', buffer);
				std::vector<std::string> split;
				Split(' ', &buffer[strlen("Kd") + 1], split);

				for (int i = 0; i < split.size(); i++)
				{
					m_Materials[current_material_name].Diffuse[i] = (float)atof(split[i].c_str());
				}
			}
		}

		fclose(fp);
	}

	return true;

}