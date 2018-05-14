/*
 * Copyright (C) 2017 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DOLBY_MS12_C_H_
#define _DOLBY_MS12_C_H_



#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


//get the handle of dlopen "/system/lib/libdolbyms12.so"
int get_libdolbyms12_handle(void);

/*@@
    @brief dolby ms12 self cleanup
*/
void dolby_ms12_self_cleanup(void);

/*@@
    @brief get dolby ms12 output max size(bytes)
*/
int get_dolby_ms12_output_max_size(void);
/*@@
    @brief Get the handle "dolby_mS12_pointer" as dolby ms12 with config argv[][] and argc

    @int argc
    @char **argv
*/
void * dolby_ms12_init(int argc, char **argv);

//release dolby ms12
void dolby_ms12_release(void *dolby_mS12_pointer);

/*@@
    @brief Input main[dolby/he-aac/pcm]
    @if single input as pcm, use this api
    @if dual input such as "multi-pcm + stereo&16bites pcm", we choose multi-pcm through this api

    @void *dolby_mS12_pointer //dolby ms12 handle
    @const void *audio_stream_out_buffer //main input buffer address
    @size_t audio_stream_out_buffer_size //main input buffer size
    @int audio_stream_out_format //main format
    @int audio_stream_out_channel_num //main channel num
    @int audio_stream_out_sample_rate //main sample rate
*/
int dolby_ms12_input_main(void *dolby_mS12_pointer
    , const void *input_main_buffer
    , size_t audio_stream_out_buffer_size
    , int audio_stream_out_format
    , int audio_stream_out_channel_num
    , int audio_stream_out_sample_rate);


/*@@
    @brief input associate data, format contains [dolby/he-aac], and here the input main format is same as the associate format

    @void *dolby_mS12_pointer //dolby ms12 handle
    @const void *audio_stream_out_buffer //associate input buffer address
    @size_t audio_stream_out_buffer_size //associate input buffer size
    @int audio_stream_out_format //associate format
    @int audio_stream_out_channel_num //associate channel num
    @int audio_stream_out_sample_rate //associate sample rate
*/
int dolby_ms12_input_associate(void *dolby_mS12_pointer
    , const void *audio_stream_out_buffer
    , size_t audio_stream_out_buffer_size
    , int audio_stream_out_format
    , int audio_stream_out_channel_num
    , int audio_stream_out_sample_rate
    );


/*@@
    @brief Input system data, format is stereo-16bits PCM.
    @when input dual stream, such as "dolby + pcm" or "multi-pcm + stereo&16bits-pcm"

    @void *dolby_mS12_pointer //dolby ms12 handle
    @const void *audio_stream_out_buffer //system input buffer address
    @size_t audio_stream_out_buffer_size //system input buffer size
    @int audio_stream_out_format //system format
    @int audio_stream_out_channel_num //system channel num
    @int audio_stream_out_sample_rate //system sample rate
*/
int dolby_ms12_input_system(void *dolby_mS12_pointer
    , const void *audio_stream_out_buffer
    , size_t audio_stream_out_buffer_size
    , int audio_stream_out_format
    , int audio_stream_out_channel_num
    , int audio_stream_out_sample_rate);


#ifdef REPLACE_OUTPUT_BUFFER_WITH_CALLBACK
int dolby_ms12_register_pcm_callback(void *callback, void *priv_data);

int dolby_ms12_register_bitstream_callback(void *callback, void *priv_data);
#else
/*@@
    @brief Get output data

    @void *dolby_mS12_pointer //dolby ms12 handle
    @const void *ms12_out_buffer //output buffer address
    @size_t request_out_buffer_size //request data size
*/
int dolby_ms12_output(void *dolby_mS12_pointer
    , const void *ms12_out_buffer
    , size_t request_out_buffer_size
    );
#endif

/*@@
    @brief get all the runtime config params, as the style of "int argc, char **argv"

    @void *dolby_mS12_pointer //dolby ms12 handle
    @int argc
    @char **argv
*/
int dolby_ms12_update_runtime_params(void *dolby_mS12_pointer, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif //end of _DOLBY_MS12_C_H_