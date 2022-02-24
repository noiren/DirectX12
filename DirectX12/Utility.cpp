#include "Utility.h"

void Split(char split_char, char* buffer, std::vector<std::string>& out)
{
	int count = 0;
	if (buffer == nullptr)
	{
		return;
	}

	int start_point = 0;
	
	// ‚¨K‚Ü‚ÅŒ©‚é
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
				out.emplace_back(""); //˜A‘±‚µ‚Äˆê’v‚µ‚½ê‡‚»‚ÌŠÔ‚É‚Í‚È‚É‚à‚È‚¢‚æ‚Ë
			}
			start_point = count + 1;
		}
		count++;
	}

	// ˆê‚Â‚àˆê’v‚µ‚È‚©‚Á‚½ê‡‚Í‚»‚Ì‚Ü‚Ü“f‚«o‚»‚¤
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