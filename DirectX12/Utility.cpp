#include "Utility.h"

void Split(char split_char, char* buffer, std::vector<std::string>& out)
{
	int count = 0;
	if (buffer == nullptr)
	{
		return;
	}

	int start_point = 0;
	
	// お尻まで見る
	while (buffer[count] != '\0')
	{
		if (buffer[count] == split_char)
		{
			if (start_point != count)
			{
				char split_str[256] = { 0 };
				strncpy_s(split_str, 256, &buffer[start_point], count - start_point);
				out.emplace_back(split_str);
			}
			else
			{
				out.emplace_back(""); //連続して一致した場合その間にはなにもないよね
			}
			start_point = count + 1;
		}
		count++;
	}

	// 一つも一致しなかった場合はそのまま吐き出そう
	if (start_point != count)
	{
		char split_str[256] = { 0 };
		strncpy_s(split_str, 256, &buffer[start_point], count - start_point);
		out.emplace_back(split_str);
	}
}

void Replace(char search_char, char replace_char, char* buffer)
{
	int len = strlen(buffer);

	for (int i = 0; i < len; ++i)
	{
		if (buffer[i] == search_char)
		{
			buffer[i] = replace_char;
		}
	}
}