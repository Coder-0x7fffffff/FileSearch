#include "stdafx.h"
#include "oper.h"

void splitCString(CString strSrc, CString strGap, CStringArray & strResult)
{
	int pos = strSrc.Find(strGap);
	CString strLeft = TEXT("");
	while (0 <= pos)
	{
		strLeft = strSrc.Left(pos);
		if (!strLeft.IsEmpty())
		{
			strResult.Add(strLeft);
		}

		strSrc = strSrc.Right(strSrc.GetLength() - pos - strGap.GetLength());
		pos = strSrc.Find(strGap);
	}
	if (!strSrc.IsEmpty())
	{
		strResult.Add(strSrc);
	}
}
