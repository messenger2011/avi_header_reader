#include <stdio.h>
#include "avi.h"

void read_avi(char * file_path)
{
	FILE *fp;
	fp = fopen(file_path, "rb");

	if (fp != NULL)
	{
		MainAVIHeader main_avi_header;
		AVIStreamHeader avi_stream_header;
		//
		fseek(fp, 32, SEEK_SET);
		fread(&main_avi_header, sizeof(MainAVIHeader), 1, fp);
		//
		//fseek(fp, 108, SEEK_SET);
		//fread(&avi_stream_header, sizeof(AVIStreamHeader), 1, fp);
		//
		printf("%d\n", main_avi_header.dwTotalFrames);
	}
}