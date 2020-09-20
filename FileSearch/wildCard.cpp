#include "stdafx.h"
#include "wildCard.h"

bool matchWithWildCard(CString str, CString matchStr)
{
	//动态规划通配符字符串匹配
	int len1 = str.GetLength();
	int len2 = matchStr.GetLength();
	str = CString(" ") + str;;
	matchStr = CString(" ") + matchStr;
	bool **ifMatch = new bool*[len1+1];//是否匹配二维数组
	for (int i = 0; i < len1 + 1; ++i)
	{
		ifMatch[i] = new bool[len2 + 1];
		memset(ifMatch[i], 0, (len2 + 1)*sizeof(bool));
	}
	ifMatch[0][0] = true;
	for (int i = 1; str[i] == '*'; i++)
		ifMatch[i][0] = true;
	for (int i = 1; str[i]; ++i)
	{
		for (int j = 1; matchStr[j]; ++j)
		{
			if (str[i] == '?')
				ifMatch[i][j] = ifMatch[i-1][j-1];
			else if (str[i] == '*')
				ifMatch[i][j] = ifMatch[i][j - 1] || ifMatch[i - 1][j] || ifMatch[i - 1][j - 1];
			else ifMatch[i][j] = ifMatch[i - 1][j - 1] && str[i] == matchStr[j];
		}
	}
	bool match = ifMatch[len1][len2];
	for (int i = 0; i < len1 + 1; ++i)
	{
		delete ifMatch[i];
	}
	delete ifMatch;
	return match;
}
