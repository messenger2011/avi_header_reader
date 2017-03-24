#include <stdio.h>
#include "avi.h"
#include <string.h>

#define PAD_EVEN(x) ( ((x)+1) & ~1 )

#define strncasecmp(x,y,z) _tcsnicmp(x,y,z)

//static size_t avi_read(int fd, char *buf, size_t len)
//{
//	size_t n = 0;
//	size_t r = 0;
//
//	while (r < len) {
//		n = read(fd, buf + r, len - r);
//
//		if (n <= 0)
//			return r;
//		r += n;
//	}
//
//	return r;
//}

long AVI_video_frames(avi *AVI)
{
	return AVI->video_frames;
}
int  AVI_video_width(avi *AVI)
{
	return AVI->width;
}
int  AVI_video_height(avi *AVI)
{
	return AVI->height;
}
double AVI_frame_rate(avi *AVI)
{
	return AVI->fps;
}
char* AVI_video_compressor(avi *AVI)
{
	return AVI->compressor2;
}

static size_t avi_read(FILE * fp, char *buf, size_t len)
{
	size_t n = 0;
	size_t r = 0;

	n = fread(buf, sizeof(char), len, fp);

	return n;
}


/* Copy n into dst as a 4 byte, little endian number.
Should also work on big endian machines */

static void long2str(unsigned char *dst, int n)
{
	dst[0] = (n) & 0xff;
	dst[1] = (n >> 8) & 0xff;
	dst[2] = (n >> 16) & 0xff;
	dst[3] = (n >> 24) & 0xff;
}

/* Convert a string of 4 or 2 bytes to a number,
also working on big endian machines */

static unsigned long str2ulong(unsigned char *str)
{
	return (str[0] | (str[1] << 8) | (str[2] << 16) | (str[3] << 24));
}
static unsigned long str2ushort(unsigned char *str)
{
	return (str[0] | (str[1] << 8));
}

static int avi_add_index_entry(avi *AVI, unsigned char *tag, long flags, unsigned long pos, unsigned long len)
{
	void *ptr;

	if (AVI->n_idx >= AVI->max_idx) {
		ptr = realloc((void *)AVI->idx, (AVI->max_idx + 4096) * 16);

		if (ptr == 0) {
			return -1;
		}
		AVI->max_idx += 4096;
		AVI->idx = (unsigned char((*)[16])) ptr;
	}

	/* Add index entry */

	//   fprintf(stderr, "INDEX %s %ld %lu %lu\n", tag, flags, pos, len);

	memcpy(AVI->idx[AVI->n_idx], tag, 4);
	long2str(AVI->idx[AVI->n_idx] + 4, flags);
	long2str(AVI->idx[AVI->n_idx] + 8, pos);
	long2str(AVI->idx[AVI->n_idx] + 12, len);

	/* Update counter */

	AVI->n_idx++;

	if (len > AVI->max_len) AVI->max_len = len;

	return 0;
}

void main()
{
	AVI_open_input_file("D:\\Downloads\\samples\\flame.avi", 1);
	getchar();
}

avi *AVI_open_input_file(char *filename, int getIndex) {
	avi * AVI = NULL;
	AVI = (avi *)malloc(sizeof(avi));
	if (AVI == NULL)
	{
		free(AVI);
		return 0;
	}
	memset((void *)AVI, 0, sizeof(avi));
	//
	FILE * fp;
	AVI->fp = fopen(filename, "rb");

	if (!AVI->fp)
	{
		free(AVI);
		return 0;
	}

	avi_parse_input_file(AVI, getIndex);
}

int avi_parse_input_file(avi *AVI, int getIndex)
{
	long i, n, rate, scale, idx_type;
	unsigned char * hdrl_data;
	long header_offset = 0, hdrl_len = 0;
	long nvi, ioff;
	int j;
	int lasttag = 0;
	int vids_strh_seen = 0;
	int vids_strf_seen = 0;
	int auds_strh_seen = 0;
	//  int auds_strf_seen = 0;
	int num_stream = 0;
	char data[256];

	if (avi_read(AVI->fp, data, 12) != 12)
		return 0;
	if (strncasecmp(data, "RIFF", 4) != 0 ||
		strncasecmp(data + 8, "AVI ", 4) != 0) return 0;

	/* Go through the AVI file and extract the header list,
	the start position of the 'movi' list and an optionally
	present idx1 tag */

	hdrl_data = 0;

	while (1)
	{
		if (avi_read(AVI->fp, data, 8) != 8)
			break; /* We assume it's EOF */

		n = str2ulong((unsigned char *)data + 4);
		n = PAD_EVEN(n);

		if (strncasecmp(data, "LIST", 4) == 0)
		{
			if (avi_read(AVI->fp, data, 4) != 4)
				return 0;
			n -= 4;
			if (strncasecmp(data, "hdrl", 4) == 0)
			{
				hdrl_len = n;
				hdrl_data = (unsigned char *)malloc(n);
				if (hdrl_data == 0)
					return;

				// offset of header

				// header_offset = fseek(AVI->fp, 0, SEEK_CUR);
				header_offset = ftell(AVI->fp);

				if (avi_read(AVI->fp, (char *)hdrl_data, n) != n)
					return 0;
			}
			else if (strncasecmp(data, "movi", 4) == 0)
			{
				// AVI->movi_start = fseek(AVI->fp, 0, SEEK_CUR);
				AVI->movi_start = ftell(AVI->fp);
				fseek(AVI->fp, n, SEEK_CUR);
			}
			else
				fseek(AVI->fp, n, SEEK_CUR);
		}
		else if (strncasecmp(data, "idx1", 4) == 0)
		{
			/* n must be a multiple of 16, but the reading does not
			break if this is not the case */

			AVI->n_idx = AVI->max_idx = n / 16;
			AVI->idx = (unsigned  char((*)[16]))
				malloc(n);
			if (AVI->idx == 0)
				return 0;
			if (avi_read(AVI->fp, (char *)AVI->idx, n) != n)
				return 0;
		}
		else
			fseek(AVI->fp, n, SEEK_CUR);
	}

	if (!hdrl_data)
		return 0;
	if (!AVI->movi_start)
		return 0;

	/* Interpret the header list */

	for (i = 0; i < hdrl_len;)
	{
		/* List tags are completly ignored */

		if (strncasecmp((char *)hdrl_data + i, "LIST", 4) == 0) { i += 12; continue; }

		n = str2ulong(hdrl_data + i + 4);
		n = PAD_EVEN(n);

		/* Interpret the tag and its args */

		if (strncasecmp((char *)hdrl_data + i, "strh", 4) == 0)
		{
			i += 8;
			if (strncasecmp((char *)hdrl_data + i, "vids", 4) == 0 && !vids_strh_seen)
			{
				memcpy(AVI->compressor, hdrl_data + i + 4, 4);
				AVI->compressor[4] = 0;

				// ThOe
				AVI->v_codech_off = header_offset + i + 4;

				scale = str2ulong((unsigned char *)hdrl_data + i + 20);
				rate = str2ulong(hdrl_data + i + 24);
				if (scale != 0) AVI->fps = (double)rate / (double)scale;
				AVI->video_frames = str2ulong(hdrl_data + i + 32);
				AVI->video_strn = num_stream;
				AVI->max_len = 0;
				vids_strh_seen = 1;
				lasttag = 1; /* vids */
			}
			else if (strncasecmp((char *)hdrl_data + i, "auds", 4) == 0 && !auds_strh_seen)
			{

			}
		}
		else if (strncasecmp((char *)hdrl_data + i, "strf", 4) == 0)
		{
			i += 8;
			if (lasttag == 1)
			{
				AVI->width = str2ulong(hdrl_data + i + 4);
				AVI->height = str2ulong(hdrl_data + i + 8);
				vids_strf_seen = 1;
				//ThOe
				AVI->v_codecf_off = header_offset + i + 16;

				memcpy(AVI->compressor2, hdrl_data + i + 16, 4);
				AVI->compressor2[4] = 0;

			}
			lasttag = 0;
		}
		else
		{
			i += 8;
			lasttag = 0;
		}

		i += n;
	}

	free(hdrl_data);

	if (!vids_strh_seen || !vids_strf_seen)
		return 0;

	AVI->video_tag[0] = AVI->video_strn / 10 + '0';
	AVI->video_tag[1] = AVI->video_strn % 10 + '0';
	AVI->video_tag[2] = 'd';
	AVI->video_tag[3] = 'b';

	fseek(AVI->fp, AVI->movi_start, SEEK_SET);

	/* get index if wanted */

	if (!getIndex)
		return(0);

	/* if the file has an idx1, check if this is relative
	to the start of the file or to the start of the movi list */

	idx_type = 0;

	if (AVI->idx)
	{
		long pos, len;

		/* Search the first videoframe in the idx1 and look where
		it is in the file */

		for (i = 0; i < AVI->n_idx; i++)
			if (strncasecmp((char *)AVI->idx[i], (char *)AVI->video_tag, 3) == 0) break;
		if (i >= AVI->n_idx) return 0;

		pos = str2ulong(AVI->idx[i] + 8);
		len = str2ulong(AVI->idx[i] + 12);

		fseek(AVI->fp, pos, SEEK_SET);
		if (avi_read(AVI->fp, data, 8) != 8) return 0;
		if (strncasecmp((char *)data, (char *)AVI->idx[i], 4) == 0 &&
			str2ulong((unsigned char *)data + 4) == len)
		{
			idx_type = 1; /* Index from start of file */
		}
		else
		{
			fseek(AVI->fp, pos + AVI->movi_start - 4, SEEK_SET);
			if (avi_read(AVI->fp, data, 8) != 8) return 0;
			if (strncasecmp((char *)data, (char *)AVI->idx[i], 4) == 0 && str2ulong((unsigned char *)data + 4) == len)
			{
				idx_type = 2; /* Index from start of movi list */
			}
		}
		/* idx_type remains 0 if neither of the two tests above succeeds */
	}

	if (idx_type == 0)
	{
		/* we must search through the file to get the index */

		fseek(AVI->fp, AVI->movi_start, SEEK_SET);

		AVI->n_idx = 0;

		while (1)
		{
			if (avi_read(AVI->fp, data, 8) != 8) break;
			n = str2ulong((unsigned char *)data + 4);

			/* The movi list may contain sub-lists, ignore them */

			if (strncasecmp(data, "LIST", 4) == 0)
			{
				fseek(AVI->fp, 4, SEEK_CUR);
				continue;
			}

			/* Check if we got a tag ##db, ##dc or ##wb */

			if (((data[2] == 'd' || data[2] == 'D') &&
				(data[3] == 'b' || data[3] == 'B' || data[3] == 'c' || data[3] == 'C'))
				|| ((data[2] == 'w' || data[2] == 'W') &&
				(data[3] == 'b' || data[3] == 'B')))
			{
				avi_add_index_entry(AVI, (unsigned char *)data, 0, fseek(AVI->fp, 0, SEEK_CUR) - 8, n);
			}

			fseek(AVI->fp, PAD_EVEN(n), SEEK_CUR);
		}
		idx_type = 1;
	}

	/* Now generate the video index and audio index arrays */

	nvi = 0;
	for (i = 0; i < AVI->n_idx; i++) {
		if (strncasecmp((char *)AVI->idx[i], (char *)AVI->video_tag, 3) == 0) nvi++;
	}

	AVI->video_frames = nvi;

	//   fprintf(stderr, "chunks = %ld %d %s\n", AVI->track[0].audio_chunks, AVI->anum, AVI->track[0].audio_tag);

	CHUNK * chunk;

	if (AVI->video_frames == 0)
		return 0;
	AVI->video_index = (video_index_entry *)malloc(nvi * sizeof(video_index_entry));
	chunk = (CHUNK *)malloc(nvi * sizeof(CHUNK));
	if (AVI->video_index == 0 || chunk == 0)
		return 0;

	nvi = 0;

	ioff = idx_type == 1 ? 8 : AVI->movi_start + 4;

	for (i = 0; i < AVI->n_idx; i++) {

		//video
		if (strncasecmp((char *)AVI->idx[i], (char *)AVI->video_tag, 3) == 0) {
			AVI->video_index[nvi].key = str2ulong(AVI->idx[i] + 4);
			AVI->video_index[nvi].pos = str2ulong(AVI->idx[i] + 8) + ioff;
			AVI->video_index[nvi].len = str2ulong(AVI->idx[i] + 12);
			/*chunk[nvi].data = (unsigned char *)malloc(chunk[nvi].dwSize * sizeof(unsigned char *));
			fseek(AVI->fp, AVI->video_index[nvi].pos, SEEK_SET);
			fread(chunk[nvi].data, sizeof(chunk[nvi].data), 1, AVI->fp);*/
			fpp * f = get_frame(AVI, AVI->video_index[nvi].key, 0, 0, 0);
			nvi++;
		}
	}

	/* Reposition the file */

	fseek(AVI->fp, AVI->movi_start, SEEK_SET);
	AVI->video_pos = 0;

	return(0);
}

fpp * get_frame(avi * cell_file, int frame_num, int cropped, int scaled, int converted)
{
	// variable
	int dummy;
	int width = AVI_video_width(cell_file);
	int height = AVI_video_height(cell_file);
	int status;

	// There are 600 frames in this file (i.e. frame_num = 600 causes an error)
	AVI_set_video_position(cell_file, frame_num);

	//Read in the frame from the AVI
	char* image_buf = (char*)malloc(width * height * sizeof(char));
	status = AVI_read_frame(cell_file, image_buf, &dummy);
	if (status == -1) {
		exit(-1);
	}

	// The image is read in upside-down, so we need to flip it
	fpp* image_chopped;
	image_chopped = chop_flip_image(image_buf,
		height,
		width,
		cropped,
		scaled,
		converted);

	// free image buffer
	free(image_buf);

	// return
	return image_chopped;
}

fpp* chop_flip_image(char *image,
	int height,
	int width,
	int cropped,
	int scaled,
	int converted) {

	// fixed dimensions for cropping or not cropping, square vertices starting from initial point in top left corner going down and right
	int top;
	int bottom;
	int left;
	int right;
	if (cropped == 1) {
		top = 0;
		bottom = 0;
		left = 0;
		right = 0;
	}
	else {
		top = 0;
		bottom = height - 1;
		left = 0;
		right = width - 1;
	}

	// dimensions of new cropped image
	int height_new = bottom - top + 1;
	int width_new = right - left + 1;

	// counters
	int i, j;

	// allocate memory for cropped/flipped frame
	fpp* result = (fpp *)malloc(height_new * width_new * sizeof(fpp));

	// crop/flip and scale frame
	fpp temp;
	if (scaled) {
		fpp scale = 1.0 / 255.0;
		for (i = 0; i < height_new; i++) {				// rows
			for (j = 0; j < width_new; j++) {			// colums
				temp = (fpp)image[((height - 1 - (i + top)) * width) + (j + left)] * scale;
				if (temp < 0) {
					result[i*width_new + j] = temp + 256;
				}
				else {
					result[i*width_new + j] = temp;
				}
			}
		}
	}
	else {
		for (i = 0; i < height_new; i++) {				// rows
			for (j = 0; j < width_new; j++) {			// colums
				temp = (fpp)image[((height - 1 - (i + top)) * width) + (j + left)];
				if (temp < 0) {
					result[i*width_new + j] = temp + 256;
				}
				else {
					result[i*width_new + j] = temp;
				}
			}
		}
	}

	// convert storage method (from row-major to column-major)
	fpp* result_converted = (fpp *)malloc(height_new * width_new * sizeof(fpp));
	if (converted == 1) {
		for (i = 0; i < width_new; i++) {				// rows
			for (j = 0; j < height_new; j++) {			// colums
				result_converted[i*height_new + j] = result[j*width_new + i];
			}
		}
	}
	else {
		result_converted = result;
	}
	free(result);

	// return
	return result_converted;
}

long AVI_read_frame(avi *AVI, char *vidbuf, int *keyframe)
{
	long n;
	if (AVI->video_pos < 0 || AVI->video_pos >= AVI->video_frames) return -1;
	n = AVI->video_index[AVI->video_pos].len;
	*keyframe = (AVI->video_index[AVI->video_pos].key == 0x10) ? 1 : 0;
	fseek(AVI->fp, AVI->video_index[AVI->video_pos].pos, SEEK_SET);
	if (avi_read(AVI->fp, vidbuf, n) != n)
	{
		return -1;
	}
	AVI->video_pos++;
	return n;
}

int AVI_set_video_position(avi *AVI, long frame)
{
	if (frame < 0) frame = 0;
	AVI->video_pos = frame;
	return 0;
}