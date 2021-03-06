#include "stdafx.h"
#include "SearchFile.h"

void SearchFile::countNTFSDisk()
{
	char diskName[] = ("A:\\");
	for (int i = 0; i < 26; i++)	//固定26毕竟字母只有26个
	{
		diskName[0] = 'A' + i;
		char szVolumeNameBuf[MAX_PATH] = { 0 };
		DWORD dwVolumeSerialNum;
		DWORD dwMaxComponentLength;
		DWORD dwSysFlags;
		char szFileSystemBuf[MAX_PATH] = { 0 };
		DWORD dwFileSystemBuf = MAX_PATH;
		GetVolumeInformationA(diskName,	//根目录名称
			szVolumeNameBuf,			//卷名称
			MAX_PATH,					//卷名称长度
			&dwVolumeSerialNum,			//磁盘序列号
			&dwMaxComponentLength,		//磁盘最大支持的文件名称长度
			&dwSysFlags,				//磁盘属性
			szFileSystemBuf,			//磁盘类型
			MAX_PATH					//磁盘类型长度
		);
		if (!memcmp("NTFS", szFileSystemBuf, sizeof("NTFS")))
		{
			//记录
			nameOfDisks.push_back('A' + i);
		}
	}
}

bool SearchFile::createUSN(char volumeName)
{
	DWORD outLen;
	//创建USN文件
	CREATE_USN_JOURNAL_DATA createDataOfUsn;	//microsoft doc.
	createDataOfUsn.AllocationDelta = 0;
	createDataOfUsn.MaximumSize = 0;
	bool ifCreteUJD = DeviceIoControl(
		hDisk[volumeName - 'A'],											//“文件”句柄
		FSCTL_CREATE_USN_JOURNAL,						//IO类型
		&createDataOfUsn,								//输入结构
		sizeof(createDataOfUsn),						//长度
		NULL,											//输出
		0,												//长度
		&outLen,										//输出长度
		NULL											//异步结构
	);
	return ifCreteUJD;
}

bool SearchFile::closeUSN(char volumeName)
{
	DWORD outLen;
	//删除USN
	DELETE_USN_JOURNAL_DATA delUSNData;
	delUSNData.UsnJournalID = usnJournalData[volumeName - 'A'].UsnJournalID;
	delUSNData.DeleteFlags = USN_DELETE_FLAG_DELETE;
	bool ifCloseUJD = DeviceIoControl(hDisk[volumeName - 'A'], FSCTL_DELETE_USN_JOURNAL, &delUSNData, sizeof(delUSNData), NULL, 0, &outLen, NULL);
	return false;
}

bool SearchFile::openDisk(char volumeName)
{
	//打开文件
	CString diskName("\\\\.\\A:");
	diskName.SetAt(4, volumeName);//-> \\.\A: A在第5个
	//共享读写才可以保证打开，不被占用FILE_SHAGE_WRITE
	hDisk[volumeName - 'A'] = CreateFile(diskName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
	return hDisk[volumeName - 'A'] != INVALID_HANDLE_VALUE;
}

bool SearchFile::closeDisk(char volumeName)
{
	bool ifClosed = CloseHandle(hDisk[volumeName - 'A']);
	return ifClosed;
}

bool SearchFile::getUSNInfo(char volumeName)
{
	DWORD outLen;
	//获取USN的Info
	bool ifGetUjd = DeviceIoControl(hDisk[volumeName - 'A'], FSCTL_QUERY_USN_JOURNAL, NULL, 0, &usnJournalData[volumeName - 'A'], sizeof(usnJournalData[volumeName - 'A']), &outLen, NULL);
	return ifGetUjd;
}

void SearchFile::searchFileOfDisk(char volumeName)
{
	DWORD outLen;
	//读取USN
	MFT_ENUM_DATA mftEnumData;
	//匹配数据版本
	mftEnumData.MinMajorVersion = usnJournalData[volumeName - 'A'].MinSupportedMajorVersion;
	mftEnumData.MaxMajorVersion = usnJournalData[volumeName - 'A'].MinSupportedMajorVersion;
	mftEnumData.StartFileReferenceNumber = 0;
	mftEnumData.LowUsn = 0;
	mftEnumData.HighUsn = usnJournalData[volumeName - 'A'].NextUsn;
	const int bufLen = 0x70000;
	BYTE buffer[bufLen];
	PUSN_RECORD record;
	FileIden fileIden;
	//磁盘根目录
	//记录文件个数
	fileCount[volumeName - 'A']++;
	//处理文件名称和parentFileReferenceNum
	fileIden.fileNmae = CString(volumeName);
	fileIden.parentFileReferenceNum = 0;
	//第一个hash
	fileMap[volumeName - 'A'].insert(std::make_pair(0, fileIden));
	//第二个hash
	fileMap2[volumeName - 'A'].insert(std::make_pair(CString(volumeName), 0));
	//加入set
	fileNameSet[volumeName - 'A'].insert(CString(volumeName));
	while (DeviceIoControl(hDisk[volumeName - 'A'], FSCTL_ENUM_USN_DATA, &mftEnumData, sizeof(mftEnumData), &buffer, bufLen, &outLen, NULL))
	{
		DWORD pageLen = outLen;
		//开始获取USN记录
		record = (PUSN_RECORD)(buffer + sizeof(USN));//buffer使用BYTE 单字节 便于增加长度
		pageLen -= sizeof(USN);
		while (pageLen > 0)
		{
			CString fileName(record->FileName, record->FileNameLength / 2);
			//记录文件个数
			fileCount[volumeName - 'A']++;
			//处理文件名称和parentFileReferenceNum
			fileIden.fileNmae = fileName;
			fileIden.parentFileReferenceNum = record->ParentFileReferenceNumber;
			//第一个hash
			fileMap[volumeName - 'A'].insert(std::make_pair(record->FileReferenceNumber, fileIden));
			//第二个hash
			fileMap2[volumeName - 'A'].insert(std::make_pair(fileName, record->FileReferenceNumber));
			//加入set
			fileNameSet[volumeName - 'A'].insert(fileName);
			//下一个USN记录
			pageLen -= record->RecordLength;
			record = (USN_RECORD*)((BYTE*)record + record->RecordLength);
		}
		//设置下一次开始测位置
		mftEnumData.StartFileReferenceNumber = *(USN*)buffer;
	}
}

std::vector<FindResult>& SearchFile::findFileFromFullName(CString name)
{
	fullFindResult.clear();
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		size_t len = fileMap2[nameOfDisks[i] - 'A'].count(name);
		if (len)
		{
			hashFN2RN::iterator it = fileMap2[nameOfDisks[i] - 'A'].find(name);
			//使用count做条件比使用CString比较更加快速查找到结束位置
			for (int j = 0; j < len; ++j)
			{
				fullFindResult.push_back(FindResult(it->second, nameOfDisks[i]));
				it++;
			}
		}
	}
	return fullFindResult;
}

SearchFile::SearchFile()
{
	for (int i = 0; i < 26; i++)
	{
		fileCount[i] = 0;
	}
}

SearchFile::~SearchFile()
{
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		char diskNmae = nameOfDisks[i];
		bool ifcloseUSN = closeUSN(diskNmae);
		bool ifcloseDisk = closeDisk(diskNmae);
	}
}

void SearchFile::setNTFSDisk(std::vector<char> list)
{
	nameOfDisks = list;
}

std::vector<FindResult>& SearchFile::getFindResult()
{
	return findResult;
}

void SearchFile::searchAllFile()
{
	if (!nameOfDisks.size())
		countNTFSDisk();
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		char diskNmae = nameOfDisks[i];
		bool ifOpenDisk = openDisk(diskNmae);
		bool ifcreateUSN = createUSN(diskNmae);
		bool ifgetUSNInfo = getUSNInfo(diskNmae);
		searchFileOfDisk(diskNmae);
	}
}

CString SearchFile::getPath(unsigned long long rn, char volumeName)
{
	CString path;
	if (rn != 0)
	{
		size_t len = fileMap[volumeName - 'A'].count(rn);
		if (len != 0)
		{
			hashRN2FI::iterator it = fileMap[volumeName - 'A'].find(rn);
			path = it->second.fileNmae;
			it = fileMap[volumeName - 'A'].find(it->second.parentFileReferenceNum);
			while (it != fileMap[volumeName - 'A'].end())
			{
				path = it->second.fileNmae + CString("\\") + path;
				it = fileMap[volumeName - 'A'].find(it->second.parentFileReferenceNum);
			}
			path = CString(volumeName) + CString(":\\") + path;
		}
		else return CString("*");//不存在文件
	}
	else path = CString(volumeName) + CString(":\\");
	return path;
}

std::vector<FindResult>& SearchFile::findFileFromName(CString name)
{
	//结果查找标记
	bool ifRes = false;
	bool ifWithWildcard = (name.Find(CString("*")) != -1 || name.Find(CString("?")) != -1);
	std::vector<CString> nameList;
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		for (std::set<CString>::iterator it = fileNameSet[nameOfDisks[i] - 'A'].begin();
			it != fileNameSet[nameOfDisks[i] - 'A'].end();
			it++
			)
		{
			//匹配是否成立
			if (!ifWithWildcard)//没有通配符
			{
				if (it->Find(name) != -1)
					nameList.push_back(*it);
			}
			else
			{
				//查找
				if (matchWithWildCard(name, *it))
					nameList.push_back(*it);
			}
		}
		for (int k = 0; k < nameList.size(); k++)
		{
			size_t len = fileMap2[nameOfDisks[i] - 'A'].count(nameList[k]);
			if (len)
			{
				hashFN2RN::iterator it = fileMap2[nameOfDisks[i] - 'A'].find(nameList[k]);
				//使用count做条件比使用CString比较更加快速查找到结束位置
				for (int j = 0; j < len; ++j)
				{
					findResult.push_back(FindResult(it->second, nameOfDisks[i]));
					it++;
				}
			}
		}
	}
	return findResult;
}

void SearchFile::resetFind()
{
	findResult.clear();
}

FileInfo SearchFile::getFileInfo(FindResult res)
{
	FileInfo info;
	size_t len = fileMap[res.volumeName - 'A'].count(res.rn);
	if (len > 0)
	{
		info.fileName = fileMap[res.volumeName - 'A'].find(res.rn)->second.fileNmae;
		info.path = getPath(res.rn, res.volumeName);
	}
	else
	{
		info.fileName = info.path = "*";
	}
	return info;
}

void SearchFile::upDataFile(LPARAM lParam, WCHAR * path, WCHAR * path2)
{
	switch (lParam)
	{
	case SHCNE_MKDIR:	//创建目录
	case SHCNE_CREATE:	//创建文件
		for (int i = 0; i < nameOfDisks.size(); i++)
		{
			char diskNmae = nameOfDisks[i];
			if (diskNmae == path[0])
			{
				//伪造文件记录
				CString str(path);
				CStringArray strArray;
				splitCString(str, CString("\\"), strArray);
				DWORDLONG parentFileRn = fileMap2[diskNmae - 'A'].find(strArray.GetAt(strArray.GetSize() - 2))->second;
				DWORDLONG fakeRn = 0;
				int count;
				do
				{
					fakeRn++;
					count = (int)fileMap[diskNmae - 'A'].count(fakeRn);
				} while (count != 0);
				FileIden fakeIden;
				fakeIden.parentFileReferenceNum = parentFileRn;
				fakeIden.fileNmae = strArray.GetAt(strArray.GetSize() - 1);
				//查找是否存在
				bool isIn = false;
				findFileFromFullName(fakeIden.fileNmae);
				for (int i = 0; i < fullFindResult.size(); i++)
				{
					if (getFileInfo(fullFindResult[i]).path == str)
					{
						isIn = true;
						break;
					}
				}
				if (!isIn)
				{
					//记录文件个数
					fileCount[diskNmae - 'A']++;
					//第一个hash
					fileMap[diskNmae - 'A'].insert(std::make_pair(fakeRn, fakeIden));
					//第二个hash
					fileMap2[diskNmae - 'A'].insert(std::make_pair(fakeIden.fileNmae, fakeRn));
					//加入set
					fileNameSet[diskNmae - 'A'].insert(fakeIden.fileNmae);
				}
			}
		}
		break;
	case SHCNE_RMDIR:	//删除目录
	case SHCNE_DELETE:	//删除文件
		for (int i = 0; i < nameOfDisks.size(); i++)
		{
			char diskNmae = nameOfDisks[i];
			if (diskNmae == path[0])
			{
				//伪造文件记录
				CString str(path);
				CStringArray strArray;
				splitCString(str, CString("\\"), strArray);
				//找到对应文件
				bool isIn = false;
				FileInfo info;
				unsigned long long delRn;
				findFileFromFullName(strArray.GetAt(strArray.GetSize() - 1));
				for (int i = 0; i < fullFindResult.size(); i++)
				{
					info = getFileInfo(fullFindResult[i]);
					if (info.path == str)
					{
						isIn = true;
						delRn = fullFindResult[i].rn;
						break;
					}
				}
				if (isIn)
				{
					//del
					fileCount[diskNmae - 'A']--;
					fileMap[diskNmae - 'A'].erase(delRn);
					size_t len = fileMap2[diskNmae - 'A'].count(strArray.GetAt(strArray.GetSize() - 1));
					if (len)
					{
						hashFN2RN::iterator it = fileMap2[nameOfDisks[i] - 'A'].find(strArray.GetAt(strArray.GetSize() - 1));
						//使用count做条件比使用CString比较更加快速查找到结束位置
						for (int j = 0; j < len; ++j)
						{
							FindResult res = FindResult(it->second, nameOfDisks[i]);
							if (res.rn == delRn)
							{
								fileMap2[diskNmae - 'A'].erase(it);
							}
							it++;
						}
					}
				}
			}
		}
		break;
	case SHCNE_RENAMEFOLDER:	//文件夹重命名
	case SHCNE_RENAMEITEM:	//文件重命名
		for (int i = 0; i < nameOfDisks.size(); i++)
		{
			char diskNmae = nameOfDisks[i];
			if (diskNmae == path[0])
			{
				CString str(path), str2(path2);
				CStringArray strArray, strArray2;
				splitCString(str, CString("\\"), strArray);
				splitCString(str2, CString("\\"), strArray2);
				//查找是否存在
				bool isIn = false;
				FileInfo info;
				unsigned long long renameRn;
				findFileFromFullName(strArray.GetAt(strArray.GetSize() - 1));
				for (int i = 0; i < fullFindResult.size(); i++)
				{
					info = getFileInfo(fullFindResult[i]);
					if (info.path == str)
					{
						isIn = true;
						renameRn = fullFindResult[i].rn;
						break;
					}
				}
				if (isIn)
				{
					FileIden newIden = fileMap[diskNmae - 'A'].find(renameRn)->second;
					newIden.fileNmae = strArray2.GetAt(strArray2.GetSize() - 1);
					//删除原有记录
					fileMap[diskNmae - 'A'].erase(renameRn);
					size_t len = fileMap2[diskNmae - 'A'].count(strArray.GetAt(strArray.GetSize() - 1));
					if (len)
					{
						hashFN2RN::iterator it = fileMap2[nameOfDisks[i] - 'A'].find(strArray.GetAt(strArray.GetSize() - 1));
						//使用count做条件比使用CString比较更加快速查找到结束位置
						for (int j = 0; j < len; ++j)
						{
							FindResult res = FindResult(it->second, nameOfDisks[i]);
							if (res.rn == renameRn)
							{
								fileMap2[diskNmae - 'A'].erase(it);
							}
							it++;
						}
					}
					//加入新记录
					//第一个hash
					fileMap[diskNmae - 'A'].insert(std::make_pair(renameRn, newIden));
					//第二个hash
					fileMap2[diskNmae - 'A'].insert(std::make_pair(newIden.fileNmae, renameRn));
					//加入set
					fileNameSet[diskNmae - 'A'].insert(newIden.fileNmae);
				}
			}
		}
		break;
	}
}
