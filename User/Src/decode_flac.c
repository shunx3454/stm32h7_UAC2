/* example_c_decode_file - Simple FLAC file decoder using libFLAC
 * Copyright (C) 2007-2009  Josh Coalson
 * Copyright (C) 2011-2016  Xiph.Org Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * This example shows how to use libFLAC to decode a FLAC file to a WAVE
 * file.  It only supports 16-bit stereo files.
 *
 * Complete API documentation can be found at:
 *   http://xiph.org/flac/api/
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "FLAC/stream_decoder.h"
#include "ff.h"
#include "share/compat.h"
#include <stdio.h>
#include <inttypes.h>

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
                                                     const FLAC__int32 *const buffer[], void *client_data);
static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata,
                              void *client_data);
static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status,
                           void *client_data);
static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
                                                   size_t *bytes, void *client_data);

static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;

struct udata {
  FIL *inf;
  FIL *outf;
};

static FLAC__bool write_little_endian_uint16(FIL *f, FLAC__uint16 x) {
  return f_putc(x, f) != EOF && f_putc(x >> 8, f) != EOF;
}

static FLAC__bool write_little_endian_int16(FIL *f, FLAC__int16 x) {
  return write_little_endian_uint16(f, (FLAC__uint16)x);
}

static FLAC__bool write_little_endian_uint32(FIL *f, FLAC__uint32 x) {
  return f_putc(x, f) != EOF && f_putc(x >> 8, f) != EOF && f_putc(x >> 16, f) != EOF && f_putc(x >> 24, f) != EOF;
}

int decode_flac(const char *flacname, const char *pcmname) {
  FLAC__bool ok = true;
  FLAC__StreamDecoder *decoder = 0;
  FLAC__StreamDecoderInitStatus init_status;
  struct udata udata;
  int errorcode = 0;
  FRESULT res;

  do {
    udata.inf = mmalloc(sizeof(FIL));
    udata.outf = mmalloc(sizeof(FIL));
    if (udata.inf == NULL || udata.outf == NULL) {
      errorcode = 1;
      break;
    }

    res = f_open(udata.inf, flacname, FA_READ);
    if (res) {
      errorcode = 2;
      break;
    }
    res = f_open(udata.outf, pcmname, FA_CREATE_ALWAYS | FA_WRITE);
    if (res) {
      f_close(udata.inf);
      errorcode = 3;
      break;
    }

    if ((decoder = FLAC__stream_decoder_new()) == NULL) {
      errorcode = 4;
      break;
    }

    (void)FLAC__stream_decoder_set_md5_checking(decoder, true);

    init_status = FLAC__stream_decoder_init_stream(decoder, read_callback, NULL, NULL, NULL, NULL, write_callback,
                                                   metadata_callback, error_callback, &udata

    );
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
      printf("ERROR: initializing decoder: %s\r\n", FLAC__StreamDecoderInitStatusString[init_status]);
      ok = false;
      errorcode = 5;
      break;
    }

    if (ok) {
      ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
      printf("decoding: %s\r\n", ok ? "succeeded" : "FAILED");
      printf("   state: %s\r\n", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
    }

    FLAC__stream_decoder_delete(decoder);

  } while (0);
  if (errorcode)
    printf("decode flac errorcode: %d\r\n", errorcode);

  f_close(udata.inf);
  f_close(udata.outf);
  mfree(udata.inf);
  mfree(udata.outf);
  return errorcode;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
                                              const FLAC__int32 *const buffer[], void *client_data) {
  struct udata *pudata = (struct udata *)client_data;
  const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps / 8));
  size_t i;
  UINT nbw;

  (void)decoder;

  if (total_samples == 0) {
    printf("ERROR: this example only works for FLAC files that have a total_samples count in STREAMINFO\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (channels != 2 || bps != 16) {
    printf("ERROR: this example only supports 16bit stereo streams\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (frame->header.channels != 2) {
    printf("ERROR: This frame contains %u channels (should be 2)\n", frame->header.channels);
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (buffer[0] == NULL) {
    printf("ERROR: buffer [0] is NULL\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (buffer[1] == NULL) {
    printf("ERROR: buffer [1] is NULL\n");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }

  /* write WAVE header before we write the first frame */
  // if (frame->header.number.sample_number == 0) {
  //   if (fwrite("RIFF", 1, 4, f) < 4 || !write_little_endian_uint32(f, total_size + 36) ||
  //       fwrite("WAVEfmt ", 1, 8, f) < 8 || !write_little_endian_uint32(f, 16) || !write_little_endian_uint16(f, 1) ||
  //       !write_little_endian_uint16(f, (FLAC__uint16)channels) || !write_little_endian_uint32(f, sample_rate) ||
  //       !write_little_endian_uint32(f, sample_rate * channels * (bps / 8)) ||
  //       !write_little_endian_uint16(f, (FLAC__uint16)(channels * (bps / 8))) || /* block align */
  //       !write_little_endian_uint16(f, (FLAC__uint16)bps) || fwrite("data", 1, 4, f) < 4 ||
  //       !write_little_endian_uint32(f, total_size)) {
  //     fprintf(stderr, "ERROR: write error\n");
  //     return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  //   }
  // }

  /* write decoded PCM samples */
  //for (i = 0; i < frame->header.blocksize; i++) {
  //  if (!write_little_endian_int16(pudata->outf, (FLAC__int16)buffer[0][i]) || /* left channel */
  //      !write_little_endian_int16(pudata->outf, (FLAC__int16)buffer[1][i])    /* right channel */
  //  ) {
  //    printf("ERROR: write error\n");
  //    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  //  }
  //}

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
  (void)decoder, (void)client_data;

  /* print some stats */
  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    /* save for later */
    total_samples = metadata->data.stream_info.total_samples;
    sample_rate = metadata->data.stream_info.sample_rate;
    channels = metadata->data.stream_info.channels;
    bps = metadata->data.stream_info.bits_per_sample;

    printf("sample rate    : %u Hz\r\n", sample_rate);
    printf("channels       : %u\r\n", channels);
    printf("bits per sample: %u\r\n", bps);
    printf("total samples  : %" PRIu64 "\r\n", total_samples);
  }
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
  (void)decoder, (void)client_data;

  printf("Got error callback: %s\r\n", FLAC__StreamDecoderErrorStatusString[status]);
}

static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
                                                   size_t *bytes, void *client_data) {
  struct udata *pudata = (struct udata *)client_data;
  (void)decoder;
  size_t read_size = *bytes;
  if (f_read(pudata->inf, buffer, read_size, bytes) != FR_OK) {
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
  }
  if (*bytes == 0) {
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
  } else {
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
}