
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define 	AVIF_COPYRIGHTED   0x00020000
#define 	AVIF_HASINDEX   0x00000010
#define 	AVIF_ISINTERLEAVED   0x00000100
#define 	AVIF_MUSTUSEINDEX   0x00000020
#define 	AVIF_TRUSTCKTYPE   0x00000800
#define 	AVIF_WASCAPTUREFILE   0x00010000

typedef struct {
	int ckid;
	int dwFlags;
	int dwChunkOffset;
	int dwChunkLength;
} AVIINDEXENTRY;

typedef struct {
	int dwFourCC;
	unsigned int dwSize;
	char * data; // contains headers or video/audio data; dwSize
} CHUNK;

typedef struct {
	int dwList;
	int dwSize;
	int dwFourCC;
	unsigned char * data[]; // contains Lists and Chunks; dwSize - 4
} LIST;

typedef struct tagRECT {
	long left;
	long top;
	long right;
	long bottom;
} RECT;

typedef struct
{
	long dwMicroSecPerFrame; // frame display rate (or 0)
	long dwMaxBytesPerSec; // max. transfer rate
	long dwPaddingGranularity; // pad to multiples of this
							   // size;
	long dwFlags; // the ever-present flags
	long dwTotalFrames; // # frames in file
	long dwInitialFrames;
	long dwStreams;
	long dwSuggestedBufferSize;
	long dwWidth;
	long dwHeight;
	long dwReserved[4];
} MainAVIHeader;

typedef struct {
	char *  fccType[4];
	char * fccHandler[4];
	int dwFlags;
	unsigned short wPriority;
	unsigned short wLanguage;
	int dwInitialFrames;
	int dwScale;
	int dwRate; /* dwRate / dwScale == samples/second */
	int dwStart;
	int dwLength; /* In units above... */
	int dwSuggestedBufferSize;
	int dwQuality;
	int dwSampleSize;
	RECT rcFrame;
} AVIStreamHeader;

typedef struct
{
	unsigned long key;
	unsigned long pos;
	unsigned long len;
} video_index_entry;

typedef struct
{
	FILE* fp;

	long width;
	long height;
	double fps;
	long video_strn;
	long video_frames;
	char video_tag[4];
	long video_pos;

	unsigned long max_len;

	unsigned long pos;        /* position in file */
	long   n_idx;             /* number of index entries actually filled */
	long   max_idx;           /* number of index entries actually allocated */

	long  v_codech_off;       /* absolut offset of video codec (strh) info */
	long  v_codecf_off;       /* absolut offset of video codec (strf) info */

	unsigned char(*idx)[16]; /* index entries (AVI idx1 tag) */
	video_index_entry *video_index;
	char compressor[8];
	char compressor2[8];
	unsigned long last_pos;          /* Position of last frame written */
	unsigned long last_len;          /* Length of last frame written */
	int must_use_index;              /* Flag if frames are duplicated */
	unsigned long   movi_start;
} avi;

#define fpp float

avi *AVI_open_input_file(char *filename, int getIndex);
int avi_parse_input_file(avi *AVI, int getIndex);
long AVI_read_frame(avi *AVI, char *vidbuf, int *keyframe);
fpp* chop_flip_image(char *image,
	int height,
	int width,
	int cropped,
	int scaled,
	int converted);

fpp* get_frame(avi * cell_file,
	int frame_num,
	int cropped,
	int scaled,
	int converted);
int AVI_set_video_position(avi *AVI, long frame);