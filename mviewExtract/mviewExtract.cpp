// mviewExtract.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

//戻り値: 文字バイト数（終端NULL除く）
int freadNullStr(FILE *fp, char *buf, int maxBuf)
{
  int i=0;
  char c = 0;

  if(buf==NULL)
  {
    while(1)
    {
      c = fgetc(fp);
      if(c==EOF || c==0)return i;
      i++;
    }
  }

  for(;i<maxBuf-1;i++)
  {
    c = fgetc(fp);
    if(c==EOF)return i;
    if(c==0)
    {
      buf[i] = 0;
      return i;
    }
    buf[i] = c;
  }
  buf[maxBuf-1]=0;
  return maxBuf-1;
}

BYTE *buf = NULL;
const unsigned int bufSize = 1024*16;

bool FILEtoFILE(FILE *dst, FILE *src, unsigned int size)
{
  if(buf==NULL)return false;
  unsigned int nokori = size;

  while(1)
  {
    size_t numRead = fread(buf, 1, (nokori>bufSize)?bufSize:nokori, src);
    if(numRead==0)return false;
    if(fwrite(buf, 1, numRead, dst)!=numRead)return false;
    if(nokori<numRead)return false;
    nokori -= numRead;
    if(nokori<=0)return true;
  }
  return false;
}

void SanitizePath(char *path)
{
  if(path[0]=='\\'||path[0]=='/')path[0]='_';
  char *found = NULL;
  while(1)
  {
    found = strstr(path, "..");
    if(found==NULL)break;
    found[0] = '_';
    found[1] = '_';
  }
  found = NULL;
  while(1)
  {
    found = strstr(path, ":");
    if(found==NULL)break;
    found[0] = '_';
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
  char *path = NULL, *pathDir = NULL;
  FILE *fpMview = NULL;
  FILE *fpDst = NULL;

  DWORD flag, sizeData, sizeDataDecompressed;

  if(argc<2)goto err;
  fpMview = _wfopen(argv[1], L"rb");
  if(fpMview==NULL)goto err;

  path = (char *)malloc(4096);
  if(path==NULL)goto err;
  pathDir = (char *)malloc(4096);
  if(pathDir==NULL)goto err;

  buf = (BYTE *)malloc(bufSize);
  if(buf==NULL)goto err;

  while(1)
  {
    flag=0;
    sizeData=0;
    sizeDataDecompressed=0;

    int numPath = freadNullStr(fpMview, path, 4096);
    if(numPath==0)goto err;

    SanitizePath(path);

    freadNullStr(fpMview, NULL, 0);
    if(fread(&flag, 1, 4, fpMview)!=4)goto err;
    if(fread(&sizeData, 1, 4, fpMview)!=4)goto err;
    if(fread(&sizeDataDecompressed, 1, 4, fpMview)!=4)goto err;
    
    if(flag & 0x1)printf("\"%s\"  flag=%u (compressed!), size=%u, decompressedSize=%u\n", path, flag, sizeData, sizeDataDecompressed);
    else          printf("\"%s\"  flag=%u, size=%u\n", path, flag, sizeData);

    if(PathCombineA(path, "out\\", path)==NULL)goto err;
    strcpy_s(pathDir, 4096, path);

    PathRemoveFileSpecA(pathDir);
    PathAddBackslashA(pathDir);
    BOOL bRet = MakeSureDirectoryPathExists(pathDir);

    fpDst = fopen(path, "wb");
    if(fpDst==NULL)goto err;

    if(!FILEtoFILE(fpDst, fpMview, sizeData))goto err;

    fclose(fpDst);
  }
  
  if(buf)free(buf);
  if(path)free(path);
  if(pathDir)free(pathDir);
  if(fpMview)fclose(fpMview);
  if(fpDst)fclose(fpDst);
	return 0;

err:
  if(buf)free(buf);
  if(path)free(path);
  if(pathDir)free(pathDir);
  if(fpMview)fclose(fpMview);
  if(fpDst)fclose(fpDst);
  return 0;
}

