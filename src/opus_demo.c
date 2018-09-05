/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "opus.h"
#include "debug.h"
#include "opus_types.h"
#include "opus_private.h"
#include "opus_multistream.h"

#include "stdio.h" 
#include "windows.h"
#include"io.h"

void xtc_decode(char* inFile, char *outFile) {
	FILE *fin, *fout;
	char opusData[640];
	opus_int16 *pcmData[640];
	char opusHeader[9];

	OpusDecoder *dec = NULL;
	int channels = 1;
	int err;

	fin = fopen(inFile, "rb");
	if (!fin)
	{
		fprintf(stderr, "Could not open input file %s\n", inFile);
		return EXIT_FAILURE;
	}

	int readCount = fread(opusHeader, 1, sizeof(opusHeader), fin);
	if (memcmp(opusHeader, "!Opus.", 6) != 0) {
		fprintf(stderr, "Not opus file:%s\n",inFile);
		return;
	}

	strcpy(outFile, inFile);
	strcat(outFile, ".pcm");
	fout = fopen(outFile, "wb+");
	if (!fout)
	{
		fprintf(stderr, "Could not open output file %s\n", outFile);
		fclose(fin);
		return EXIT_FAILURE;
	}

	short sampling_rate = *(short*)(opusHeader + 7);
	dec = opus_decoder_create(sampling_rate, channels, &err);

	if (err != OPUS_OK)
	{
		fprintf(stderr, "Cannot create decoder: %s\n", opus_strerror(err));
		fclose(fin);
		fclose(fout);
		return EXIT_FAILURE;
	}

	do
	{
		int readCount = fread(opusHeader, 1, 1, fin);
		if (readCount <= 0)
		{
			break;
		}
		unsigned char length = opusHeader[0];
		if (length <= 0)
		{
			break;
		}
		readCount = fread(opusData, 1, length, fin);

		int decodeCount = opus_decode(dec, opusData, length, pcmData, sampling_rate*0.02, 0);
		if (decodeCount <= 0) {
			break;
		}
		fwrite(pcmData, decodeCount * sizeof(opus_int16), 1, fout);
	} while (1);

	fprintf(stdout, "decode result = %s,sample rate %d\n", outFile, sampling_rate);

	fclose(fin);
	fclose(fout);
	return EXIT_SUCCESS;
}

int find_decode()
{
	char cwd[256];
	char filePath[sizeof(cwd)];
	char outFilePath[sizeof(cwd)];
	_getcwd(cwd, sizeof(cwd));
	strcpy(filePath,cwd);

	strcat(filePath, "\\*.*");
	struct _finddata_t files;
	int File_Handle;
	int i = 0;
	File_Handle = _findfirst(filePath, &files);
	if (File_Handle == -1)
	{
		printf("error\n");
		return 0;
	}

	do
	{
		strcpy(filePath, cwd);
		strcat(filePath, "/");
		strcat(filePath, files.name);

		strcpy(outFilePath, filePath);
		strcat(outFilePath,".pcm");
		if (files.attrib & _A_SUBDIR)
		{
			continue;
		}
		if (strstr(files.name, ".exe")) {
			continue;
		}
		if (strstr(files.name, ".pcm")) {
			continue;
		}
		xtc_decode(filePath, outFilePath);
	} while (0 == _findnext(File_Handle, &files));
	_findclose(File_Handle);
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout;

	if (argc == 1){	
		find_decode();
	}
	else if(argc == 2){
		char *inFile = argv[1];
		char outFile[256];
		strcpy(outFile, inFile);
		strcat(outFile, ".pcm");
		xtc_decode(inFile, outFile);
	}

	return EXIT_SUCCESS;
}
