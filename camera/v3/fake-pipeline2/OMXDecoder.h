#ifndef OMXDECODER
#define OMXDECODER
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <utils/Log.h>
//#include <media/openmax/OMX_Component.h>
#include <OMX_Component.h>
#include <OMX_IVCommon.h>
#include <OMX_Core.h>
#include <OMX_Index.h>

//#include <binder/IPCThreadState.h>
#include <utils/Errors.h>
#include <utils/Thread.h>
#include <utils/Timers.h>
#include <utils/Mutex.h>
#include <utils/List.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <sys/wait.h>
#include <dlfcn.h>
#ifdef GE2D_ENABLE
#include "ge2d_stream.h"
#endif
#include <nativebase/nativebase.h>
//#include <android/native_window.h>
//#include <gui/Surface.h>
//#include <HardwareAPI.h>


using namespace android;
#define TempBufferNum   (3)
#define MAX_POLLING_COUNT (100)
#define MAX_CONTINUE_VSYNC_FAIL_COUNT (45) // 30fps, about 1.5s
#define ROUND_16(X)     ((X + 0xF) & (~0xF))
#define ROUND_32(X)     ((X + 0x1F) & (~0x1F))
#define YUV_SIZE(W, H)   ((W) * (H) * 3 >> 1)
#define ZTE_BUF_ADDR_ALIGNMENT_VALUE 512
#define OMX_IndexVendorZteOmxDecNormalYUVMode 0x7F10000C
#define LOG_LINE(fmt, ...) ALOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
typedef OMX_ERRORTYPE (*InitFunc)();
typedef OMX_ERRORTYPE (*DeinitFunc)();
typedef OMX_ERRORTYPE (*GetHandleFunc)(OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);
typedef OMX_ERRORTYPE (*FreeHandleFunc)(OMX_HANDLETYPE *);

struct GrallocBufInfo {
    uint32_t width;
    uint32_t height;
    int format;
    uint32_t stride;
};

class OMXDecoder
{
public:
    enum Decoder_Type{
        DEC_NONE,
        DEC_MJPEG,
        DEC_H264,
    };

public:
    bool mTimeOut;
public:
    OMXDecoder();
    OMXDecoder(bool useDMABuffer, bool keepOriginalSize);
    ~OMXDecoder();
    bool setParameters(uint32_t in_width, uint32_t in_height,
                               uint32_t out_width, uint32_t out_height,
                               uint32_t out_buffer_count);
    bool initialize(const char* name);
    bool prepareBuffers();
    void start();
    void freeBuffers();
    void deinitialize();
    //void saveNativeBufferHdr(void *buffer, int index, int bufferNum, struct GrallocBufInfo info, bool status);//omx zero-copy
    OMX_BUFFERHEADERTYPE* dequeueInputBuffer();
    void queueInputBuffer(OMX_BUFFERHEADERTYPE* pBufferHdr);
    //ANativeWindowBuffer * dequeueOutputBuffer();
    //native_handle_t * dequeueOutputBuffer();
    OMX_BUFFERHEADERTYPE* dequeueOutputBuffer();
    //void releaseOutputBuffer(ANativeWindowBuffer * pBufferHdr);
    //void releaseOutputBuffer(native_handle_t * pBufferHdr);
    void releaseOutputBuffer(OMX_BUFFERHEADERTYPE* pBufferHdr);
    bool hasReadyOutputBuffer();

    template<class T> void InitOMXParams(T *params);

    OMX_ERRORTYPE OnEvent(
            OMX_IN OMX_EVENTTYPE eEvent,
            OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2,
            OMX_IN OMX_PTR pEventData);

    OMX_ERRORTYPE emptyBufferDone(OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);
    OMX_ERRORTYPE fillBufferDone(OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_ERRORTYPE OnEvent(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent,
            OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2,
            OMX_IN OMX_PTR pEventData);

    static OMX_ERRORTYPE OnEmptyBufferDone(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_ERRORTYPE OnFillBufferDone(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_CALLBACKTYPE kCallbacks;
    int Decode(uint8_t*src, size_t src_size,
            int dst_fd,uint8_t *dst_buf,
            size_t src_w, size_t src_h,
            size_t dst_w, size_t dst_h);

    size_t outputWidth() {
        return mOutWidth;
    }
    size_t outputHeight() {
        return mOutHeight;
    }
private:
    int decoderType;
    int mWaitVsyncDuration;

    OMX_ERRORTYPE WaitForState(OMX_HANDLETYPE hComponent, OMX_STATETYPE eTestState, OMX_STATETYPE eTestState2);
    OMX_U32 mInWidth;
    OMX_U32 mInHeight;
    OMX_U32 mOutWidth;
    OMX_U32 mOutHeight;
    int mFormat;
    uint32_t mStride;
    int mIonFd;
    bool mUseDMABuffer;
    bool mKeepOriginalSize;
    OMX_PARAM_PORTDEFINITIONTYPE mVideoInputPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE mVideoOutputPortParam;
    OMX_BUFFERHEADERTYPE mInOutPutBufferParam;
    OMX_HANDLETYPE mVDecoderHandle;
    Mutex mInputBufferLock;
    List<OMX_BUFFERHEADERTYPE*> mListOfInputBufferHeader;
    Mutex mOutputBufferLock;
    List<OMX_BUFFERHEADERTYPE*> mListOfOutputBufferHeader;

    struct out_buffer_t {
        int index;
        int fd;
        int mIonFd;
        void *fd_ptr;
        struct ion_handle *ion_hnd;
        OMX_BUFFERHEADERTYPE * pBuffer;
    };

    struct out_buffer_t *mOutBuffer;
    int mNoFreeFlag;
    OMX_BUFFERHEADERTYPE **mppBuffer;

    //native buffer
    uint32_t mOutBufferCount;

    struct out_buffer_native {
        //ANativeWindowBuffer *mOutputNativeBuffer;
        native_handle_t* handle;
        OMX_BUFFERHEADERTYPE *pBuffer;
        bool isQueued;
    };

    struct out_buffer_native *mOutBufferNative;

    void* mLibHandle;
    InitFunc mInit;
    DeinitFunc mDeinit;
    GetHandleFunc mGetHandle;
    FreeHandleFunc mFreeHandle;
    OMX_STRING mDecoderComponentName;
    OMX_VERSIONTYPE mSpecVersion;
    int mDequeueFailNum;
    int mContinuousVsyncFailNum;
    uint8_t* mTempFrame[TempBufferNum];
    int mUvmFd;
    OMX_TICKS timeStamp = 0;
    Mutex mOMXControlMutex;
    Condition mOMXVSync;

#ifdef GE2D_ENABLE
    ge2dTransform* mGE2D;
#endif

private:
    void QueueBuffer(uint8_t* src, size_t size);
    int DequeueBuffer(int dst_fd ,uint8_t* dst_buf,
                      size_t src_w, size_t src_h,
                      size_t dst_w, size_t dst_h);

    bool normal_buffer_init(int buffer_size);
    int ion_alloc_buffer(int ion_fd, size_t size, int* pShareFd, unsigned int flag, unsigned int alloc_hmask);
    bool ion_buffer_init();
    bool uvm_buffer_init();
    void free_ion_buffer();
    void free_normal_buffer();
    void free_uvm_buffer();
    bool do_buffer_init();
    void do_buffer_free();
    void SetOutputBuffer(int share_fd, uint8_t* addr);
    bool OMXWaitForVSync(nsecs_t reltime);
};
#endif
