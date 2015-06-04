#ifdef HAVE_TRACE

#include "precomp.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

#define LIBYUV

#ifdef LIBYUV
#include "libyuv.h"
#else
// For cvtColor
#include "opencv2/imgproc.hpp"
#endif

#define PATH_PHYSICAL_ADDRESS "/dev/shm/camera_buffer_pa"
#define PATH_DEV_MEM "/dev/mem"

// define COPY_ON_GRAB to immediately create a copy of the YUV frame before the
// later color space conversion in retrieve(). If the buffer isn't stable 
// for the interval between a grab and retrieve this is necessary
#define COPY_ON_GRAB

// Capture video frames from Trace video camera

class CvCaptureCAM_Trace : public CvCapture
{
    public:
        CvCaptureCAM_Trace() { init(); }
        virtual ~CvCaptureCAM_Trace() { close(); }

        virtual bool open( int index );
        virtual void close();

        virtual double getProperty(int);
        virtual bool setProperty(int, double);
        virtual bool grabFrame();
        virtual IplImage* retrieveFrame(int);
        virtual int getCaptureDomain() { return CV_CAP_TRACE; } // Return the type of the capture object: CV_CAP_VFW, etc...

    protected:
        void init();

        struct frameInternal
        {
            IplImage *retrieveFrame()
            {
                if (matFrame.empty())
                    return NULL;
                iplFrame = IplImage(matFrame);
                return &iplFrame;
            }
            cv::Mat matFrame;
            private:
            IplImage iplFrame;
        };

    private:
        void *pMemVirtAddressMapped;
        int sizeMapped;
        void *pMemVirtAddressBuffer;
        int frameSize;
        int frameWidth, frameHeight, frameDepth;
        FILE *pCaptureBufferPhysicalAddressFile;
        int devMemFd;
        uint32_t physAddress;

        // Frame counters and timestamp
        int frameNumber;
        struct timeval tsBegin;
        struct timeval tsCurrent;

        // Cache converted frame
#ifdef COPY_ON_GRAB
        uint8_t *pBufferYuv;
#endif
#ifdef LIBYUV
        uint8_t *pBufferARGB;
#endif
        frameInternal frameBgr;

};

void CvCaptureCAM_Trace::init()
{
    pMemVirtAddressMapped = NULL;
    sizeMapped = 0;
    pMemVirtAddressBuffer = NULL;
    frameSize = 0;
    pCaptureBufferPhysicalAddressFile = NULL;
    devMemFd = -1;
    physAddress = 0;;
#ifdef COPY_ON_GRAB
    pBufferYuv = NULL;
#endif
#ifdef LIBYUV
    pBufferARGB = NULL;
#endif
}

// Initialize camera input
bool CvCaptureCAM_Trace::open( int index )
{

    // There is only one camera
    if (index) {
        return false;
    }

    close();

    // Open file with physical address (updated each frame)
    // Fail if this doesn't exist, we assume it will be available at startup
    pCaptureBufferPhysicalAddressFile = fopen(PATH_PHYSICAL_ADDRESS, "r");
    if (NULL == pCaptureBufferPhysicalAddressFile) {
        return false;
    }

    // Read a first frame buffer address to get the size
    fseek(pCaptureBufferPhysicalAddressFile, 0, SEEK_SET);
    fflush(pCaptureBufferPhysicalAddressFile);
    uint32_t discard;
    int paramsFound = fscanf(pCaptureBufferPhysicalAddressFile, "%08x %d %d %d",
            &discard,
            &frameWidth,
            &frameHeight,
            &frameDepth);
    if (4 == paramsFound) {
        frameSize = frameWidth * frameHeight * frameDepth;
    }

    devMemFd = ::open(PATH_DEV_MEM, O_RDWR|O_SYNC);
    if (devMemFd < 0) {
        fclose(pCaptureBufferPhysicalAddressFile);
        pCaptureBufferPhysicalAddressFile = 0;
        return false;
    }

#ifdef COPY_ON_GRAB
    // Allocate a temporary YUV copy buffer
    if (frameSize) {
        pBufferYuv = (uint8_t *)malloc(frameSize);
        if (NULL == pBufferYuv) {
            fclose(pCaptureBufferPhysicalAddressFile);
            pCaptureBufferPhysicalAddressFile = 0;
            ::close(devMemFd);
            devMemFd = -1;
            return false;
        }
    }
#endif
#ifdef LIBYUV
    // Allocate a temporary ARGB buffer
    if (frameSize) {
        pBufferARGB = (uint8_t *)malloc(frameWidth*frameHeight*4);
        if (NULL == pBufferARGB) {
            fclose(pCaptureBufferPhysicalAddressFile);
            pCaptureBufferPhysicalAddressFile = 0;
            ::close(devMemFd);
            devMemFd = -1;
            return false;
        }
    }
#endif

    frameNumber = 0;
    gettimeofday(&tsBegin, NULL); 

    return true;
}

void CvCaptureCAM_Trace::close()
{
    // Unmap the buffer
    if (pMemVirtAddressMapped) {
        munmap((void *)pMemVirtAddressMapped, sizeMapped);
        pMemVirtAddressMapped = NULL;
    }

    // Close /dev/shm/camera_buffer_pa
    if (pCaptureBufferPhysicalAddressFile) {
        fclose(pCaptureBufferPhysicalAddressFile);
        pCaptureBufferPhysicalAddressFile = NULL;
    }

    // Close /dev/mem
    if (devMemFd >= 0) {
        ::close(devMemFd);
        devMemFd = -1;
    }

#ifdef COPY_ON_GRAB
    if (pBufferYuv) {
        free(pBufferYuv);
        pBufferYuv = NULL;
    }
#endif
#ifdef LIBYUV
    if (pBufferARGB) {
        free(pBufferARGB);
        pBufferARGB = NULL;
    }
#endif

    sizeMapped = 0;
    pMemVirtAddressBuffer = NULL;
    frameSize = 0;
    physAddress = 0;

}

// Grab the frame buffer pointer
bool CvCaptureCAM_Trace::grabFrame()
{

    uint32_t pageAlignedAddress;
    off_t pageOffset;
    int frameSizeNew;

    if (pCaptureBufferPhysicalAddressFile) {

        // Format is "buffer_pa width height byte_depth format timestamp"
        // e.g. "bb1c3600 1024 576 2 YUYV422I_YUYV 1234"
        // multiply width*height*byte_depth to get buffer size
        fseek(pCaptureBufferPhysicalAddressFile, 0, SEEK_SET);
        fflush(pCaptureBufferPhysicalAddressFile);
        int paramsFound = fscanf(pCaptureBufferPhysicalAddressFile, "%08x %d %d %d",
                &physAddress,
                &frameWidth,
                &frameHeight,
                &frameDepth);

        printf("Got pa=%08x, width=%d, height=%d, depth=%d\n", physAddress, frameWidth, frameHeight, frameDepth);

        if (paramsFound != 4) {
            return false;
        }

        frameSizeNew = frameWidth * frameHeight * frameDepth;

#ifdef COPY_ON_GRAB
        // If the frame size changed we need to reallocate our buffer
        if (frameSizeNew != frameSize) {
            if (frameSize) {
                pBufferYuv = (uint8_t *)realloc(pBufferYuv, frameSize);
                if (NULL == pBufferYuv) {
                    return false;
                }
            }
        }
#endif
#ifdef LIBYUV
        // If the frame size changed we need to reallocate our buffer
        if (frameSizeNew != frameSize) {
            if (frameSize) {
                pBufferARGB = (uint8_t *)realloc(pBufferARGB, frameWidth*frameHeight*4);
                if (NULL == pBufferARGB) {
                    return false;
                }
            }
        }
#endif

        frameSize = frameSizeNew;

        // unmap existing buffer
        if (pMemVirtAddressMapped) {
            munmap((void *)pMemVirtAddressMapped, sizeMapped);
        }

        pageOffset = physAddress & (getpagesize()-1);
        pageAlignedAddress = physAddress - pageOffset;
        sizeMapped = frameSize + pageOffset;

        pMemVirtAddressMapped = mmap(
                (void *)pageAlignedAddress,
                (size_t)sizeMapped,
                PROT_READ,
                MAP_SHARED,
                (int)devMemFd,
                (off_t)pageAlignedAddress
                );

        if (NULL == pMemVirtAddressMapped) {
            sizeMapped = 0;
            pMemVirtAddressBuffer = NULL;
            return false;
        }

        pMemVirtAddressBuffer = (void *)((int)pMemVirtAddressMapped + pageOffset);

#ifdef COPY_ON_GRAB
        // Copy entire YUV frame here, in practice the frame buffers update
        // to quickly and we see interleaved frames otherwise
        memcpy(pBufferYuv, pMemVirtAddressBuffer, frameSize);
#endif

        gettimeofday(&tsCurrent, NULL); 
        frameNumber++;

        return true;
    }

    return false;
}

IplImage* CvCaptureCAM_Trace::retrieveFrame(int)
{
    // Convert from 16-bit YUYV to BGR24
    if (pMemVirtAddressBuffer) {
        // Create matrix container for raw frame buffer
#if 0
        struct timeval tsStart, tsEnd, tsElap;
        gettimeofday(&tsStart, NULL); 
#endif

#ifndef LIBYUV
#ifdef COPY_ON_GRAB
        cv::Mat matYuvColor = cv::Mat(frameHeight, frameWidth, CV_8UC2, pBufferYuv);
#else
        cv::Mat matYuvColor = cv::Mat(frameHeight, frameWidth, CV_8UC2, pMemVirtAddressBuffer);
#endif
        // Perform the colorspace conversion
        cv::cvtColor(matYuvColor, frameBgr.matFrame, CV_YUV2BGR_YUYV);
#else
#ifdef COPY_ON_GRAB
        libyuv::YUY2ToARGB(pBufferYuv, frameWidth*frameDepth, pBufferARGB, frameWidth*4, frameWidth, frameHeight);
#else
        libyuv::YUY2ToARGB(pMemVirtAddressBuffer, 0, pBufferARGB, 0, frameWidth, frameHeight);
#endif
        frameBgr.matFrame = cv::Mat(frameHeight, frameWidth, CV_8UC4, (void*)pBufferARGB); 
#endif

#if 0
        gettimeofday(&tsEnd, NULL); 
        timersub(&tsEnd, &tsStart, &tsElap);
        printf("Conversion took %f ms\n", 1000 * tsElap.tv_sec + ((double) tsElap.tv_usec) / 1000);
#endif

        return frameBgr.retrieveFrame();
    } else {
        return NULL;
    }
}

double CvCaptureCAM_Trace::getProperty( int property_id )
{
    switch( property_id )
    {
        case CV_CAP_PROP_POS_MSEC:
            if (0 == frameNumber) {
                return 0;
            } else {
                struct timeval tsElapsed;
                timersub(&tsCurrent, &tsBegin, &tsElapsed);
                return 1000 * tsElapsed.tv_sec + ((double) tsElapsed.tv_usec) / 1000;
            }
            break;
        case CV_CAP_PROP_POS_FRAMES:
            return frameNumber;
            break;
        case CV_CAP_PROP_FRAME_WIDTH:
            return frameWidth;
            break;
        case CV_CAP_PROP_FRAME_HEIGHT:
            return frameHeight;
            break;
    }
    return 0;
}

bool CvCaptureCAM_Trace::setProperty( int, double )
{
    return false;
}


CvCapture* cvCreateCameraCapture_Trace( int index )
{
    CvCaptureCAM_Trace* capture = new CvCaptureCAM_Trace;

    if( capture->open( index ))
        return capture;

    delete capture;
    return 0;
}

#endif // HAVE_TRACE
