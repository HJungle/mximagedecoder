// MXDemo.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "windows.h"
#include <io.h>
#include <direct.h>  
#include <iostream>
#include <string>
#include <vector>
#include "../MXImageDecoder/imximagedecoder.h"


#define TEST_C_API 0

using namespace std;
using namespace mx;

void DecodeFiles(const vector<string>& files);
void OnResult(const DecodeInfo *info);
int UTF8ToGBK(const char * lpUTF8Str, char * lpGBKStr, int nGBKStrLen);

void GetFiles(const string& path, vector<string>& files)
{
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;  
	string p;  
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)  
	{  
		do  
		{  
			//�����Ŀ¼,����֮  
			//�������,�����б�  
			if((fileinfo.attrib &  _A_SUBDIR))  
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)  
					GetFiles( p.assign(path).append("\\").append(fileinfo.name), files );  
			}  
			else  
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
			}  
		}while(_findnext(hFile, &fileinfo)  == 0);

		_findclose(hFile);  
	}  
} 

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hnst = LoadLibrary(L"MXImageDecoder");//�õ���̬���ӿ�ľ�� 

	char *buffer; 
	if ((buffer = _getcwd(NULL, 0)) == NULL)
	{
		perror("getcwd error");  

		return 0;
	}

#ifdef _DEBUG
	string path = "C:\\Users\\mx-home\\Desktop\\MXImageDecoder\\MXImageDecoder\\Debug";
#else
	string path = buffer;
	//string path = "C:\\Users\\mx-home\\Desktop\\MXImageDecoder\\MXImageDecoder\\Release";
#endif
	path += "\\img";
	cout << "ͼƬĿ¼��" << path << endl;

	vector<string> files;
	GetFiles(path, files);
	cout << "�ļ�������" << files.size() << endl;

	if(files.size())
		DecodeFiles(files);

	getchar();

	return 0;
}

void DecodeFiles(const vector<string>& files)
{
	HINSTANCE hnst = LoadLibrary(L"MXImageDecoder");//�õ���̬���ӿ�ľ�� 

	if (hnst == NULL)
	{
		cout << "����MXImageDecoderʧ�ܣ�" << GetLastError() << endl;

		return ;
	}
#if TEST_C_API
	typedef DecodeResult(__cdecl *decoderfunction)(const char *, DecodeInfo **);
	typedef void(__cdecl *destroyfunction)(DecodeInfo *);

	decoderfunction dec = (decoderfunction)GetProcAddress(hnst, "DecodeImageFile");//�õ���̬���ӿ���add���������ĵ�ַ 
	destroyfunction des = (destroyfunction)GetProcAddress(hnst, "DestoryDecodeInfo");

	if (!dec || !des)
	{
		cout << "��ú�����ַʧ��!" << endl;

		return ;
	}
	
#else

	AllocDecoderFunction allocfunc = (AllocDecoderFunction)GetProcAddress(hnst, "AllocDecoder");
	DestroyDecoderFunction destroyfunc = (DestroyDecoderFunction)GetProcAddress(hnst, "DestroyDecoder");

	if (!allocfunc || !destroyfunc)
	{
		cout << "��ú�����ַʧ��!" << endl;

		return ;
	}

	IMXImageDecoder *decoder = NULL;
	allocfunc(&decoder);

	if (!decoder)
	{
		cout << "��ý�����ʧ��!" << endl;

		return;
	}
#endif

	int failcount = 0;
	for (int i = 0; i < files.size(); i++)
	{
		cout << "�����ļ���" << files[i] << endl;

#if TEST_C_API

		DecodeInfo *info = NULL;
		DecodeResult dr = dec(files[i].c_str(), &info);

		if (dr == DR_OK && info)
		{
			OnResult(info);
		}
#else
		if (decoder->DecodeFile(files[i]) == DR_OK)
		{
			const DecodeInfo *info = decoder->GetResult();

			if (info)
				OnResult(info);
			else
			{
				failcount++;
				cout << "����ʧ��!" << endl;
			}
		}
#endif

		cout << "-----------------------------------" << endl;
	}

	cout << "���������\nһ����" << files.size()
		<< "���ɹ���" << files.size() - failcount
		<< "��ʧ�ܣ�" << failcount
		<< "��ʧ�ܱȣ�%" << (failcount / (float)files.size()) * 100 << endl;



#if !TEST_C_API
	destroyfunc(decoder);
#endif

	FreeLibrary(hnst);//�ͷŶ�̬���ӿ� 
}

void OnResult(const DecodeInfo *info)
{
	cout << "����������" << info->count << endl;

	if (info->count)
	{
		char szResult[1024] = {0};
		for (int i = 0; i < info->count; i++)
		{
			DataInfo data = info->data[i];

			UTF8ToGBK(data.data, szResult, 1024);
			cout << "�������ݡ�" << i + 1 << "����" << szResult << endl;
		}
	}

}

int UTF8ToGBK(const char * lpUTF8Str, char * lpGBKStr, int nGBKStrLen)
{
	if (!lpUTF8Str)  //���UTF8�ַ���ΪNULL������˳�
		return 0;

	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;

	nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL);  //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��
	lpUnicodeStr = new WCHAR[nRetLen + 1];  //ΪUnicode�ַ����ռ�
	nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, lpUnicodeStr, nRetLen);  //ת����Unicode����
	if (!nRetLen)  //ת��ʧ��������˳�
		return 0;

	nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL);  //��ȡת����GBK���������Ҫ���ַ��ռ䳤��

	if (!lpGBKStr)  //���������Ϊ���򷵻�ת������Ҫ�Ŀռ��С
	{
		if (lpUnicodeStr)
			delete[]lpUnicodeStr;
		return nRetLen;
	}

	if (nGBKStrLen < nRetLen)  //���������������Ȳ������˳�
	{
		if (lpUnicodeStr)
			delete[]lpUnicodeStr;
		return 0;
	}

	nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, (char *)lpGBKStr, nRetLen, NULL, NULL);  //ת����GBK����

	if (lpUnicodeStr)
		delete[]lpUnicodeStr;

	return nRetLen;
}
