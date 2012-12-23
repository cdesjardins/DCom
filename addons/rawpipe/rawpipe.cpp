// rawpipe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <afxwin.h>
#include <stdlib.h>
#include "nbnpapi.h"

#define RAWPIPE_CHUNK_SIZE  512
int main(int argc, char* argv[])
{
    CNonBlockingNamedPipe pipe;
    char *pData;
    int nSize;
    FILE *f;
    int nRet;
    int nIndex;
    if (argc == 3)
    {
        f = fopen(argv[1], "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            nSize = ftell(f) + 1;
            fseek(f, 0, SEEK_SET);
            pData = (char*)malloc(nSize);
            if (pData)
            {
                nRet = fread(pData, sizeof(char), nSize, f);
                fclose(f);
                if (pipe.Open(argv[2]) == true)
                {
                    for (nIndex = 0; nIndex < nSize; nIndex += RAWPIPE_CHUNK_SIZE)
                    {
                        if (pipe.Write(&(pData[nIndex]), RAWPIPE_CHUNK_SIZE, 1000) != RAWPIPE_CHUNK_SIZE)
                        {
                            AfxMessageBox("Write failure.");
                            break;
                        }
                    }
                }
                else
                {
                    AfxMessageBox("Unable to open named pipe.");
                }
                free(pData);
            }
            else
            {
                AfxMessageBox("Unable to allocate memory.");
            }
        }
        else
        {
            char szBuf[64];
            sprintf(szBuf, "Unable to open file %s.", argv[1]); 
            AfxMessageBox(szBuf);
        }
    }
    else
    {
        AfxMessageBox("Filename parameter required.");
    }

	return 0;
}

