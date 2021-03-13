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

#ifndef _AUDIO_HW_H_
#define _AUDIO_HW_H_

#include <audio_utils/resampler.h>
#include <hardware/audio.h>
#include <cutils/list.h>
#include <sound/asound.h>
#include <tinyalsa/asoundlib.h>

/* ALSA cards for AML */
#define CARD_AMLOGIC_BOARD 0
#define CARD_AMLOGIC_DEFAULT CARD_AMLOGIC_BOARD
/* ALSA ports for AML */
#if (ENABLE_HUITONG == 0)
#define PORT_MM 1
#endif

#include "audio_hwsync.h"
#include "audio_post_process.h"
#include "aml_hw_mixer.h"
#include "../amlogic_AQ_tools/audio_eq_drc_compensation.h"
#include "aml_dcv_dec_api.h"
#include "aml_dca_dec_api.h"
#include "aml_audio_types_def.h"
#include "audio_format_parse.h"
#include "aml_alsa_mixer.h"
#ifndef MS12_V24_ENABLE
#include "../libms12/include/aml_audio_ms12.h"
#else
#include "../libms12_v24/include/aml_audio_ms12.h"
#endif
//#include "aml_audio_mixer.h"
#include "audio_port.h"
#include "aml_audio_ease.h"
#include "aml_malloc_debug.h"

/* number of frames per period */
/*
 * change DEFAULT_PERIOD_SIZE from 1024 to 512 for passing CTS
 * test case test4_1MeasurePeakRms(android.media.cts.VisualizerTest)
 */
#define DEFAULT_PLAYBACK_PERIOD_SIZE 512//1024
#define DEFAULT_CAPTURE_PERIOD_SIZE  1024
#define DEFAULT_PLAYBACK_PERIOD_CNT 6

//#define DEFAULT_PERIOD_SIZE 512

/* number of ICE61937 format frames per period */
#define DEFAULT_IEC_SIZE 6144

/* number of periods for low power playback */
#define PLAYBACK_PERIOD_COUNT 4
/* number of periods for capture */
#define CAPTURE_PERIOD_COUNT 4

/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 5000

#define RESAMPLER_BUFFER_FRAMES (DEFAULT_PLAYBACK_PERIOD_SIZE * 6)
#define RESAMPLER_BUFFER_SIZE (4 * RESAMPLER_BUFFER_FRAMES)

static unsigned int DEFAULT_OUT_SAMPLING_RATE = 48000;

/* sampling rate when using MM low power port */
#define MM_LOW_POWER_SAMPLING_RATE 44100
/* sampling rate when using MM full power port */
#define MM_FULL_POWER_SAMPLING_RATE 48000
/* sampling rate when using VX port for narrow band */
#define VX_NB_SAMPLING_RATE 8000

#define AUDIO_PARAMETER_STREAM_EQ "audioeffect_eq"
#define AUDIO_PARAMETER_STREAM_SRS "audioeffect_srs_param"
#define AUDIO_PARAMETER_STREAM_SRS_GAIN "audioeffect_srs_gain"
#define AUDIO_PARAMETER_STREAM_SRS_SWITCH "audioeffect_srs_switch"

/* Get a new HW synchronization source identifier.
 * Return a valid source (positive integer) or AUDIO_HW_SYNC_INVALID if an error occurs
 * or no HW sync is available. */
#define AUDIO_PARAMETER_HW_AV_SYNC "hw_av_sync"

#define AUDIO_PARAMETER_HW_AV_EAC3_SYNC "HwAvSyncEAC3Supported"

#define SYS_NODE_EARC           "/sys/class/extcon/earcrx/state"

#define DDP_FRAME_SIZE      (768)
#define EAC3_MULTIPLIER     (4) //EAC3 bitstream in IEC61937
#define HBR_MULTIPLIER      (16) //MAT or DTSHD bitstream in IEC61937
#define JITTER_DURATION_MS  (3)
#define FLOAT_ZERO              (0.000001)


/*the same as "AUDIO HAL FORMAT" in kernel*/
enum audio_hal_format {
    TYPE_PCM = 0,
    TYPE_DTS_EXPRESS = 1,
    TYPE_AC3 = 2,
    TYPE_DTS = 3,
    TYPE_EAC3 = 4,
    TYPE_DTS_HD = 5 ,
    TYPE_MULTI_PCM = 6,
    TYPE_TRUE_HD = 7,
    TYPE_DTS_HD_MA = 8,//should not used after we unify DTS-HD&DTS-HD MA
    TYPE_PCM_HIGH_SR = 9,
    TYPE_AC4 = 10,
    TYPE_MAT = 11,
    TYPE_DDP_ATMOS = 12,
    TYPE_TRUE_HD_ATMOS = 13,
    TYPE_MAT_ATMOS = 14,
    TYPE_AC4_ATMOS = 15,
};

#define FRAMESIZE_16BIT_STEREO 4
#define FRAMESIZE_32BIT_STEREO 8
#define FRAMESIZE_32BIT_3ch 12
#define FRAMESIZE_32BIT_5ch 20
#define FRAMESIZE_32BIT_8ch 32

/* copy from VTS */
/* hardware/interfaces/audio/core/all-versions/default/include/core/default/Util.h */
/*
    0            ->         Result::OK
    -EINVAL      ->         Result::INVALID_ARGUMENTS
    -ENODATA     ->         Result::INVALID_STATE
    -ENODEV      ->         Result::NOT_INITIALIZED
    -ENOSYS      ->         Result::NOT_SUPPORTED
*/
enum Result {
    OK,
    NOT_INITIALIZED,
    INVALID_ARGUMENTS,
    INVALID_STATE,
    NOT_SUPPORTED,
    RESULT_TOO_BIG
};

#define AML_HAL_MIXER_BUF_SIZE  64*1024

#define SYSTEM_APP_SOUND_MIXING_ON 1
#define SYSTEM_APP_SOUND_MIXING_OFF 0
struct aml_hal_mixer {
    unsigned char start_buf[AML_HAL_MIXER_BUF_SIZE];
    unsigned int wp;
    unsigned int rp;
    unsigned int buf_size;
    /* flag to check if need cache some data before write to mix */
    unsigned char need_cache_flag;
    pthread_mutex_t lock;
};

/**
 *  Audio Format Description of CEC Short Audio Descriptor
 *  CEA-861-D: Table 37. RequestShortAudioDescriptorAction.java
 */
typedef enum {
    AML_HDMI_FORMAT_RESERVED1             = 0x0,    // SAD_CODEC_RESERVED1
    AML_HDMI_FORMAT_LPCM                  = 0x1,    // SAD_CODEC_LPCM
    AML_HDMI_FORMAT_AC3                   = 0x2,    // SAD_CODEC_AC3
    AML_HDMI_FORMAT_MPEG1                 = 0x3,    // SAD_CODEC_MPEG1
    AML_HDMI_FORMAT_MP3                   = 0x4,    // SAD_CODEC_MP3
    AML_HDMI_FORMAT_MPEG2MC               = 0x5,    // SAD_CODEC_MPEG2MC
    AML_HDMI_FORMAT_AAC                   = 0x6,    // SAD_CODEC_AAC
    AML_HDMI_FORMAT_DTS                   = 0x7,    // SAD_CODEC_DTS
    AML_HDMI_FORMAT_ATRAC                 = 0x8,    // SAD_CODEC_ATRAC
    AML_HDMI_FORMAT_OBA                   = 0x9,    // SAD_CODEC_OBA
    AML_HDMI_FORMAT_DDP                   = 0xA,    // SAD_CODEC_DDP
    AML_HDMI_FORMAT_DTSHD                 = 0xB,    // SAD_CODEC_DTSHD
    AML_HDMI_FORMAT_MAT                   = 0xC,    // SAD_CODEC_MAT
    AML_HDMI_FORMAT_DST                   = 0xD,    // SAD_CODEC_DST
    AML_HDMI_FORMAT_WMAPRO                = 0xE,    // SAD_CODEC_WMAPRO
    AML_HDMI_FORMAT_RESERVED2             = 0xF,    // SAD_CODEC_RESERVED2
} AML_HDMI_FORMAT_E;

struct format_desc {
    AML_HDMI_FORMAT_E fmt;
    bool is_support;
    unsigned int max_channels;
    /*
     * bit:    6     5     4    3    2    1    0
     * rate: 192  176.4   96  88.2  48  44.1   32
     */
    unsigned int sample_rate_mask;
    unsigned int max_bit_rate;
    /* only used by dd+ format */
    bool   atmos_supported;
};

struct aml_arc_hdmi_desc {
    struct format_desc pcm_fmt;
    struct format_desc dts_fmt;
    struct format_desc dtshd_fmt;
    struct format_desc dd_fmt;
    struct format_desc ddp_fmt;
    struct format_desc mat_fmt;
};

enum patch_src_assortion {
    SRC_DTV                     = 0,
    SRC_ATV                     = 1,
    SRC_LINEIN                  = 2,
    SRC_HDMIIN                  = 3,
    SRC_SPDIFIN                 = 4,
    SRC_REMOTE_SUBMIXIN         = 5,
    SRC_WIRED_HEADSETIN         = 6,
    SRC_BUILTIN_MIC             = 7,
    SRC_ECHO_REFERENCE          = 8,
    SRC_ARCIN                   = 9,
    SRC_OTHER                   = 10,
    SRC_INVAL                   = 11,
};

typedef enum {
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_UNKNOWN = 0,
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_C,/**< Center */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_MONO = TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_C,
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_R,/**< Left and right speakers */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_STEREO = TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_R,
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R,/**< Left, center and right speakers */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_R_S,/**< Left, right and surround speakers */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_S,/**< Left,center right and surround speakers */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_R_SL_RS,/**< Left, right, surround left and surround right */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_SL_SR,/**< Left, center, right, surround left and surround right */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_SL_SR_LFE,/**< Left, center, right, surround left, surround right and lfe*/
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_5_1 = TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_SL_SR_LFE,
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_SL_SR_RL_RR_LFE, /**< Left, center, right, surround left, surround right, rear left, rear right and lfe */
    TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_7_1 = TIF_HAL_PLAYBACK_AUDIO_SOURCE_CHANNEL_CONFIGURATION_L_C_R_SL_SR_RL_RR_LFE
} TIF_HAL_Playback_AudioSourceChannelConfiguration_t;
enum OUT_PORT {
    OUTPORT_SPEAKER             = 0,
    OUTPORT_HDMI_ARC            = 1,
    OUTPORT_HDMI                = 2,
    OUTPORT_SPDIF               = 3,
    OUTPORT_AUX_LINE            = 4,
    OUTPORT_HEADPHONE           = 5,
    OUTPORT_REMOTE_SUBMIX       = 6,
    OUTPORT_A2DP                = 7,
    OUTPORT_BT_SCO              = 8,
    OUTPORT_BT_SCO_HEADSET      = 9,
    OUTPORT_MAX                 = 10,
};

enum IN_PORT {
    INPORT_TUNER                = 0,
    INPORT_HDMIIN               = 1,
    INPORT_SPDIF                = 2,
    INPORT_LINEIN               = 3,
    INPORT_REMOTE_SUBMIXIN      = 4,
    INPORT_WIRED_HEADSETIN      = 5,
    INPORT_BUILTIN_MIC          = 6,
    INPORT_ECHO_REFERENCE       = 7,
    INPORT_ARCIN                = 8,
    INPORT_MAX                  = 9,
};

struct audio_patch_set {
    struct listnode list;
    struct audio_patch audio_patch;
};

typedef enum stream_usecase {
    STREAM_PCM_NORMAL       = 0,
    STREAM_PCM_DIRECT       = 1,
    STREAM_PCM_HWSYNC       = 2,
    STREAM_RAW_DIRECT       = 3,
    STREAM_RAW_HWSYNC       = 4,
    STREAM_PCM_PATCH        = 5,
    STREAM_RAW_PATCH        = 6,
    STREAM_PCM_MMAP         = 7,

    STREAM_USECASE_MAX      = 8,
} stream_usecase_t;

typedef enum alsa_device {
    I2S_DEVICE = 0,
    DIGITAL_DEVICE, /*for spdifa*/
    TDM_DEVICE,
    EARC_DEVICE,
	DIGITAL_DEVICE2, /*for spdifb*/
    ALSA_DEVICE_CNT
} alsa_device_t;

enum stream_status {
    STREAM_STANDBY = 0,
    STREAM_HW_WRITING,
    STREAM_MIXING,
    STREAM_PAUSED
};

#if defined(IS_ATOM_PROJECT)
typedef enum atom_stream_type {
    STREAM_ANDROID = 0,
    STREAM_HDMI,
    STREAM_OPTAUX
} atom_stream_type_t;
#endif

typedef union {
    unsigned long long timeStamp;
    unsigned char tsB[8];
} aec_timestamp;

struct aml_audio_mixer;

struct aml_bt_output {
    bool active;
    struct pcm *pcm_bt;
    char *bt_out_buffer;
    size_t bt_out_frames;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *resampler_buffer;
    size_t resampler_buffer_size_in_frames;
    size_t resampler_in_frames;
};
#define MAX_STREAM_NUM   5
#define HDMI_ARC_MAX_FORMAT  20
struct aml_audio_device {
    struct audio_hw_device hw_device;
    /* see note below on mutex acquisition order */
    pthread_mutex_t lock;
    pthread_mutex_t pcm_write_lock;
    /*
    if dolby ms12 is enabled, the ms12 thread will change the
    stream information depending on the main input format.
    we need protect the risk and this case.
    */
    pthread_mutex_t trans_lock;
    int mode;
    audio_devices_t in_device;
    audio_devices_t out_device;
    int in_call;
    struct aml_stream_in *active_input;
    struct aml_stream_out *active_output[MAX_STREAM_NUM];
    unsigned char active_output_count;
    bool mic_mute;
    bool speaker_mute;
    unsigned int card;
    struct audio_route *ar;
    struct echo_reference_itfe *echo_reference;
    bool low_power;
    struct aml_stream_out *hwsync_output;
    struct aml_hal_mixer hal_mixer;
    struct pcm *pcm;
    struct aml_bt_output bt_output;
    bool pcm_paused;
    unsigned hdmi_arc_ad[HDMI_ARC_MAX_FORMAT];
    bool hi_pcm_mode;
    bool audio_patching;
    /* audio configuration for dolby HDMI/SPDIF output */
    int hdmi_format;
    int pre_hdmi_format;
    int spdif_format;
    bool spdif_enable;
    int hdmi_is_pth_active;
    int disable_pcm_mixing;
    /* mute/unmute for vchip  lock control */
    bool parental_control_av_mute;
    int routing;
    struct audio_config output_config;
    /* The HDMI ARC capability info currently set. */
    struct aml_arc_hdmi_desc hdmi_descs;
    /* Save the HDMI ARC actual capability info. */
    struct aml_arc_hdmi_desc hdmi_arc_capability_desc;
    int arc_hdmi_updated;
    int a2dp_active;
    int a2dp_updated;
    void * a2dp_hal;
    int hdmi_format_updated;
    struct aml_native_postprocess native_postprocess;
    /* used only for real TV source */
    enum patch_src_assortion patch_src;
    /* for port config infos */
    float sink_gain[OUTPORT_MAX];
    float speaker_volume;
    enum OUT_PORT active_outport;
    float src_gain[INPORT_MAX];
    enum IN_PORT active_inport;
    /* message to handle usecase changes */
    bool usecase_changed;
    uint32_t usecase_masks;
    int usecase_cnt[STREAM_USECASE_MAX];
    struct aml_stream_out *active_outputs[STREAM_USECASE_MAX];
    pthread_mutex_t patch_lock;
    struct aml_audio_patch *audio_patch;
    /* indicates atv to mixer patch, no need HAL patching  */
    bool tuner2mix_patch;
    /* Now only two pcm handle supported: I2S, SPDIF */
    pthread_mutex_t alsa_pcm_lock;
    struct pcm *pcm_handle[ALSA_DEVICE_CNT];
    int pcm_refs[ALSA_DEVICE_CNT];
    bool is_paused[ALSA_DEVICE_CNT];
    struct aml_hw_mixer hw_mixer;
    audio_format_t sink_format;
    audio_format_t optical_format;
    audio_format_t sink_capability;
    volatile int32_t next_unique_ID;
    /* list head for audio_patch */
    struct listnode patch_list;

    bool dual_spdifenc_inited;
    bool dual_decoder_support;

    /* Dolby MS12 lib variable start */
    struct dolby_ms12_desc ms12;
    bool dolby_ms12_status;
    struct pcm_config ms12_config;
    int mixing_level;
    int advol_level;
    bool ad_switch_enable;
    bool associate_audio_mixing_enable;
    bool need_reset_for_dual_decoder;
    uint64_t a2dp_no_reconfig_ms12;
    /* Dolby MS12 lib variable end */

    /*used for ac3 eac3 decoder*/
    struct dolby_ddp_dec ddp;
    /**
     * enum eDolbyLibType
     * DolbyDcvLib  = dcv dec lib   , libHwAudio_dcvdec.so
     * DolbyMS12Lib = dolby MS12 lib, libdolbyms12.so
     */
    int dolby_lib_type;
    int dolby_lib_type_last;
    int dolby_decode_enable;   /*it can decode dolby, not passthrough lib*/
    int dts_decode_enable;
    /*used for dts decoder*/
    struct dca_dts_dec dts_hd;
    bool bHDMIARCon;
    bool bHDMIConnected;
    bool bHDMIConnected_update;

    /**
     * buffer pointer whose data output to headphone
     * buffer size equal to efect_buf_size
     */
    void *spk_output_buf;
    void *effect_buf;
    size_t effect_buf_size;
    size_t spk_tuning_lvl;
    /* ringbuffer for tuning latency total buf size */
    size_t spk_tuning_buf_size;
    ring_buffer_t spk_tuning_rbuf;
    bool mix_init_flag;
    struct eq_drc_data eq_data;
    /*used for high pricision A/V from amlogic amadec decoder*/
    unsigned first_apts;
    /*
    first apts flag for alsa hardware prepare,true,need set apts to hw.
    by default it is false as we do not need set the first apts in normal use case.
    */
    bool first_apts_flag;
    size_t frame_trigger_thred;
    struct aml_audio_parser *aml_parser;
    int continuous_audio_mode;
    int continuous_audio_mode_default;
    int delay_disable_continuous;
    bool atoms_lock_flag;
    bool need_remove_conti_mode;
    int  exiting_ms12;
    bool doing_reinit_ms12;    /*we are doing reinit ms12*/
    struct timespec ms12_exiting_start;
    int debug_flag;
    int dcvlib_bypass_enable;
    int dtslib_bypass_enable;
    float dts_post_gain;
    bool spdif_encoder_init_flag;
    /*atsc has video in program*/
    bool is_has_video;
    struct aml_stream_out *ms12_out;
    struct timespec mute_start_ts;
	int spdif_fmt_hw;
    bool ms12_ott_enable;
    bool ms12_main1_dolby_dummy;
    /*amlogic soft ware noise gate fot analog TV source*/
    void* aml_ng_handle;
    int aml_ng_enable;
    float aml_ng_level;
    int source_mute;
    int aml_ng_attack_time;
    int aml_ng_release_time;
    int system_app_mixing_status;
    int audio_type;
    struct aml_mixer_handle alsa_mixer;
    struct subMixing *sm;
    struct aml_audio_mixer *audio_mixer;
    bool is_TV;
    bool is_STB;
    bool useSubMix;
    //int cnt_stream_using_mixer;
    int tsync_fd;
    bool raw_to_pcm_flag;
    bool is_netflix;
    int dtv_aformat;
    unsigned int dtv_i2s_clock;
    unsigned int dtv_spidif_clock;
    unsigned int dtv_droppcm_size;
    int need_reset_ringbuffer;
    unsigned int tv_mute;
    int sub_apid;
    int sub_afmt;
    int pid;
    int demux_id;
    bool compensate_video_enable;
    int patch_start;
    int mute_start;
    aml_audio_ease_t  *audio_ease;
    /*four variable used for when audio discontinue and underrun,
      whether mute output*/
    int discontinue_mute_flag;
    int audio_discontinue;
    int no_underrun_count;
    int no_underrun_max;
    int   dap_bypass_enable;
    float dap_bypassgain;
    int start_mute_flag;
    int start_mute_count;
    int start_mute_max;
    int underrun_mute_flag;
    int ad_start_enable;
    int count;
    int sound_track_mode;
    void *alsa_handle[ALSA_DEVICE_CNT];
    int FactoryChannelReverse;
    bool dual_spdif_support; /*1 means supports spdif_a & spdif_b & spdif interface*/
    bool ms12_force_ddp_out; /*1 force ms12 output ddp*/
    uint64_t  sys_audio_frame_written;
    void* hw_mediasync;
    struct aec_t *aec;
    bool bt_wbs;
    int security_mem_level;

    /* display audio format on UI, both streaming and hdmiin*/
    audio_format_t hal_internal_format;
    bool is_dolby_atmos;
    int update_type;
};

struct meta_data {
    uint32_t frame_size;
    uint64_t pts;
    uint64_t payload_offset;
};

struct meta_data_list {
    struct listnode list;
    struct meta_data mdata;
};

typedef struct aml_stream_config {
    struct audio_config config;
} aml_stream_config_t;

typedef struct aml_device_config {
    uint32_t device_port;

} aml_device_config_t;

struct aml_stream_out {
    struct audio_stream_out stream;
    /* see note below on mutex acquisition order */
    pthread_mutex_t lock;
    struct audio_config audioCfg;
    /* config which set to ALSA device */
    struct pcm_config config;
    audio_format_t    alsa_output_format;
    /* channel mask exposed to AudioFlinger. */
    audio_channel_mask_t hal_channel_mask;
    /* format mask exposed to AudioFlinger. */
    audio_format_t hal_format;
    /* samplerate exposed to AudioFlinger. */
    unsigned int hal_rate;
    unsigned int hal_ch;
    unsigned int hal_frame_size;
    unsigned int rate_convert;
    audio_output_flags_t flags;
    audio_devices_t out_device;
    struct pcm *pcm;
    struct resampler_itfe *resampler;
    char *buffer;
    size_t buffer_frames;
    bool standby;
    struct aml_audio_device *dev;
    int write_threshold;
    bool low_power;
    unsigned multich;
    int codec_type;
    uint64_t frame_write_sum;
    uint64_t frame_skip_sum;
    uint64_t last_frames_postion;
    uint64_t spdif_enc_init_frame_write_sum;
    int skip_frame;
    int32_t *tmp_buffer_8ch;
    size_t tmp_buffer_8ch_size;
    int is_tv_platform;
    void *audioeffect_tmp_buffer;
    bool pause_status;
    bool hw_sync_mode;
    int  tsync_status;
    float volume_l;
    float volume_r;
    float last_volume_l;
    float last_volume_r;
    int last_codec_type;
    /**
     * as raw audio framesize  is 1 computed by audio_stream_out_frame_size
     * we need divide more when we got 61937 audio package
     */
    int raw_61937_frame_size;
    /* recorded for wraparound print info */
    unsigned last_dsp_frame;
    audio_hwsync_t *hwsync;
    struct timespec timestamp;
    struct timespec lasttimestamp;
    stream_usecase_t usecase;
    uint32_t dev_usecase_masks;
    /**
     * flag indicates that this stream need do mixing
     * int is_in_mixing: 1;
     * Normal pcm may not hold alsa pcm device.
     */
    int is_normal_pcm;
    unsigned int card;
    alsa_device_t device;
    ssize_t (*write)(struct audio_stream_out *stream, const void *buffer, size_t bytes);
    enum stream_status status;
    audio_format_t hal_internal_format;
    bool dual_output_flag;
    uint64_t input_bytes_size;
    uint64_t continuous_audio_offset;
    bool hwsync_pcm_config;
    bool hwsync_raw_config;
    bool direct_raw_config;
    bool is_device_differ_with_ms12;
    uint64_t total_write_size;
    int  ddp_frame_size;
    int dropped_size;
    unsigned long long mute_bytes;
    bool is_get_mute_bytes;
    bool normal_pcm_mixing_config;
    uint32_t latency_frames;
    unsigned int inputPortID;
    int exiting;
    pthread_mutex_t cond_lock;
    pthread_cond_t cond;
    struct hw_avsync_header_extractor *hwsync_extractor;
    struct listnode mdata_list;
    pthread_mutex_t mdata_lock;
    bool first_pts_set;
    bool need_first_sync;
    uint64_t last_pts;
    uint64_t last_payload_offset;
    struct audio_config out_cfg;
    int debug_stream;
    uint64_t us_used_last_write;
    bool offload_mute;
    bool need_convert;
    size_t last_playload_used;
    int ddp_frame_nblks;
    uint64_t total_ddp_frame_nblks;
    int framevalid_flag;
    bool bypass_submix;
    int need_drop_size;
    int position_update;
    bool spdifenc_init;
    void *spdifenc_handle;
    bool dual_spdif;
    int codec_type2;  /*used for dual bitstream output*/
    struct pcm *pcm2; /*used for dual bitstream output*/
    int pcm2_mute_cnt;
    bool tv_src_stream;
    unsigned int write_func;
    uint64_t  last_frame_reported;
    struct timespec  last_timestamp_reported;
    void    *pstMmapAudioParam;    // aml_mmap_audio_param_st (aml_mmap_audio.h)
    bool ac3_parser_init;
    void * ac3_parser_handle;
    struct resample_para aml_resample;
    unsigned char *resample_outbuf;
    bool restore_hdmitx_selection;
    bool restore_continuous;
    bool continuous_mode_check;
    void * ac4_parser_handle;
    int64_t last_mmap_nano_second;
    int32_t last_mmap_position;
    void *spdifout_handle;
    uint64_t main_input_ns;
    bool is_sink_format_prepared;
    bool is_ms12_main_decoder;
    aml_dec_config_t  dec_config;               /*store the decode config*/
    aml_dec_t *aml_dec;                        /*store the decoder handle*/
    int ad_substream_supported;
};

typedef ssize_t (*write_func)(struct audio_stream_out *stream, const void *buffer, size_t bytes);

#define MAX_PREPROCESSORS 3 /* maximum one AGC + one NS + one AEC per input stream */
struct aml_stream_in {
    struct audio_stream_in stream;
#if defined(ENABLE_HBG_PATCH)
    int hbg_channel;
#endif
    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct pcm_config config;
    int pcm_block_mode;         /*0 open with non block mode, 1 open with block mode*/
    struct pcm *pcm;
    unsigned int device;
    audio_channel_mask_t hal_channel_mask;
    audio_format_t hal_format;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t frames_in;
    unsigned int requested_rate;
    uint32_t main_channels;
    bool standby;
    int source;
    struct echo_reference_itfe *echo_reference;
    bool need_echo_reference;
    effect_handle_t preprocessors[MAX_PREPROCESSORS];
    int num_preprocessors;
    int16_t *proc_buf;
    size_t proc_buf_size;
    size_t proc_frames_in;
    int16_t *ref_buf;
    size_t ref_buf_size;
    size_t ref_frames_in;
    int read_status;
    /* HW parser audio format */
    int spdif_fmt_hw;
    /* SW parser audio format */
    audio_format_t spdif_fmt_sw;
    struct timespec mute_start_ts;
    int mute_flag;
    int mute_log_cntr;
    bool first_buffer_discard;
    struct aml_audio_device *dev;
    void *input_tmp_buffer;
    size_t input_tmp_buffer_size;
    void *tmp_buffer_8ch;
    size_t tmp_buffer_8ch_size;
#if defined(IS_ATOM_PROJECT)
    int ref_count;
    void *aux_buf;
    size_t aux_buf_size;
    size_t aux_buf_write_bytes;
    void *mic_buf;
    size_t mic_buf_size;
    //void *tmp_buffer_8ch;
    //size_t tmp_buffer_8ch_size;
    pthread_mutex_t aux_mic_mutex;
    pthread_cond_t aux_mic_cond;
#endif
    unsigned int frames_read;
    uint64_t timestamp_nsec;
};
typedef  int (*do_standby_func)(struct aml_stream_out *out);
typedef  int (*do_startup_func)(struct aml_stream_out *out);

inline int continous_mode(struct aml_audio_device *adev)
{
    return adev->continuous_audio_mode;
}
inline bool direct_continous(struct audio_stream_out *stream)
{
    struct aml_stream_out *out = (struct aml_stream_out *)stream;
    struct aml_audio_device *adev = out->dev;
    if ((out->flags & AUDIO_OUTPUT_FLAG_DIRECT) && adev->continuous_audio_mode) {
        return true;
    } else {
        return false;
    }
}
inline bool primary_continous(struct audio_stream_out *stream)
{
    struct aml_stream_out *out = (struct aml_stream_out *)stream;
    struct aml_audio_device *adev = out->dev;
    if ((out->flags & AUDIO_OUTPUT_FLAG_PRIMARY) && adev->continuous_audio_mode) {
        return true;
    } else {
        return false;
    }
}
/* called when adev locked */
inline int dolby_stream_active(struct aml_audio_device *adev)
{
    int i = 0;
    int is_dolby = 0;
    struct aml_stream_out *out = NULL;
    for (i = 0 ; i < STREAM_USECASE_MAX; i++) {
        out = adev->active_outputs[i];
        if (out && (out->hal_internal_format == AUDIO_FORMAT_AC3
            || out->hal_internal_format == AUDIO_FORMAT_E_AC3
            || out->hal_internal_format == AUDIO_FORMAT_DOLBY_TRUEHD
            || out->hal_internal_format == AUDIO_FORMAT_AC4
            || out->hal_internal_format == AUDIO_FORMAT_MAT)) {
            is_dolby = 1;
            break;
        }
    }
    return is_dolby;
}

/* called when adev locked */
inline int dts_stream_active(struct aml_audio_device *adev)
{
    int i = 0;
    int is_dts = 0;
    struct aml_stream_out *out = NULL;
    for (i = 0 ; i < STREAM_USECASE_MAX; i++) {
        out = adev->active_outputs[i];
        if (out && (out->hal_internal_format == AUDIO_FORMAT_DTS
            || out->hal_internal_format == AUDIO_FORMAT_DTS_HD)) {
            is_dts = 1;
            break;
        }
    }
    return is_dts;
}


/* called when adev locked */
inline int hwsync_lpcm_active(struct aml_audio_device *adev)
{
    int i = 0;
    int is_hwsync_lpcm = 0;
    struct aml_stream_out *out = NULL;
    for (i = 0 ; i < STREAM_USECASE_MAX; i++) {
        out = adev->active_outputs[i];
        if (out && audio_is_linear_pcm(out->hal_internal_format) && (out->flags & AUDIO_OUTPUT_FLAG_HW_AV_SYNC)) {
            is_hwsync_lpcm = 1;
            break;
        }
    }
    return is_hwsync_lpcm;
}

inline struct aml_stream_out *direct_active(struct aml_audio_device *adev)
{
    int i = 0;
    struct aml_stream_out *out = NULL;
    for (i = 0 ; i < STREAM_USECASE_MAX; i++) {
        out = adev->active_outputs[i];
        if (out && (out->flags & AUDIO_OUTPUT_FLAG_DIRECT)) {
            return out;
        }
    }
    return NULL;
}

/*
 *@brief get_output_format get the output format always return the "sink_format" of adev
 */
audio_format_t get_output_format(struct audio_stream_out *stream);
void *audio_patch_output_threadloop(void *data);

/*
 *@brief audio_hal_data_processing
 * format:
 *    if pcm-16bits-stereo, add audio effect process, and mapping to 8ch
 *    if raw data, packet it to IEC61937 format with spdif encoder
 *    if IEC61937 format, write them to hardware
 * return
 *    0, success
 *    -1, fail
 */
ssize_t audio_hal_data_processing(struct audio_stream_out *stream
                                    , const void *input_buffer
                                    , size_t input_buffer_bytes
                                    , void **output_buffer
                                    , size_t *output_buffer_bytes
                                    , audio_format_t output_format);

/*
 *@brief hw_write the api to write the data to audio hardware
 */
ssize_t hw_write(struct audio_stream_out *stream
                    , const void *buffer
                    , size_t bytes
                    , audio_format_t output_format);

int do_output_standby_l(struct audio_stream *stream);

ssize_t out_write_new(struct audio_stream_out *stream,
                      const void *buffer,
                      size_t bytes);
int out_standby_new(struct audio_stream *stream);
ssize_t mixer_aux_buffer_write(struct audio_stream_out *stream, const void *buffer,
                               size_t bytes);
int dsp_process_output(struct aml_audio_device *adev, void *in_buffer,
                       size_t bytes);
int release_patch_l(struct aml_audio_device *adev);
enum hwsync_status check_hwsync_status (uint apts_gap);
void config_output(struct audio_stream_out *stream, bool reset_decoder);
int start_ease_in(struct aml_audio_device *adev);
int start_ease_out(struct aml_audio_device *adev);
int out_standby_direct (struct audio_stream *stream);

void *adev_get_handle();
audio_format_t get_non_ms12_output_format(audio_format_t src_format, struct aml_audio_device *aml_dev);

/* 'bytes' are the number of bytes written to audio FIFO, for which 'timestamp' is valid.
 * 'available' is the number of frames available to read (for input) or yet to be played
 * (for output) frames in the PCM buffer.
 * timestamp and available are updated by pcm_get_htimestamp(), so they use the same
 * datatypes as the corresponding arguments to that function. */
struct aec_info {
    struct timespec timestamp;
    uint64_t timestamp_usec;
    unsigned int available;
    size_t bytes;
};
/* Capture codec parameters */
/* Set up a capture period of 32 ms:
 * CAPTURE_PERIOD = PERIOD_SIZE / SAMPLE_RATE, so (32e-3) = PERIOD_SIZE / (16e3)
 * => PERIOD_SIZE = 512 frames, where each "frame" consists of 1 sample of every channel (here, 2ch) */
#define CAPTURE_PERIOD_MULTIPLIER 16
#define CAPTURE_PERIOD_SIZE (CODEC_BASE_FRAME_COUNT * CAPTURE_PERIOD_MULTIPLIER)
#define CAPTURE_PERIOD_COUNT 4
#define CAPTURE_PERIOD_START_THRESHOLD 0
#define CAPTURE_CODEC_SAMPLING_RATE 16000

#ifdef ENABLE_AEC_HAL
#define NUM_AEC_REFERENCE_CHANNELS 1
#endif
#ifdef ENABLE_AEC_APP
/* App AEC uses 2-channel reference */
#define NUM_AEC_REFERENCE_CHANNELS 2
#endif /* #ifdef ENABLE_AEC_FUNC */
#define CODEC_BASE_FRAME_COUNT 32
#define PLAYBACK_PERIOD_MULTIPLIER 32  /* 21 ms */
#define PLAYBACK_PERIOD_SIZE (CODEC_BASE_FRAME_COUNT * PLAYBACK_PERIOD_MULTIPLIER)
#define CHANNEL_STEREO 2
#define PLAYBACK_CODEC_SAMPLING_RATE 48000

#endif
