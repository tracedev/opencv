#ifdef HAVE_TRACE

#include "precomp.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

// For cvtColor
#include "opencv2/imgproc/imgproc.hpp"

#define PATH_PHYSICAL_ADDRESS "/dev/shm/camera_buffer_pa"
#define PATH_DEV_MEM "/dev/mem"

#define FRAME_W (1024)
#define FRAME_H (576)
#define FRAME_D (2)

#define FRAME_SIZE (FRAME_W * FRAME_H * FRAME_D)

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
}

// Initialize camera input
bool CvCaptureCAM_Trace::open( int index )
{

    // There is only one camera
    if (index) {
        return false;
    }

    close();

    // RCB TODO, once available in the physical address file, extract
    // the frame size here
    frameSize = FRAME_SIZE;

    // Open file with physical address (updated each frame)
    pCaptureBufferPhysicalAddressFile = fopen(PATH_PHYSICAL_ADDRESS, "rb");
    if (NULL == pCaptureBufferPhysicalAddressFile) {
        return false;
    }

    devMemFd = ::open(PATH_DEV_MEM, O_RDWR|O_SYNC);
    if (devMemFd < 0) {
        fclose(pCaptureBufferPhysicalAddressFile);
        pCaptureBufferPhysicalAddressFile = 0;
        return false;
    }

#ifdef COPY_ON_GRAB
    // Allocate a temporary YUV copy buffer
    pBufferYuv = (uint8_t *)malloc(frameSize);
    if (NULL == pBufferYuv) {
        fclose(pCaptureBufferPhysicalAddressFile);
        pCaptureBufferPhysicalAddressFile = 0;
        ::close(devMemFd);
        devMemFd = -1;
        return false;
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
    size_t bytesRead;

    if (pCaptureBufferPhysicalAddressFile) {

        fflush(pCaptureBufferPhysicalAddressFile);
        fseek(pCaptureBufferPhysicalAddressFile, 0, SEEK_SET);
        bytesRead = fread(&physAddress, 1, sizeof(int), pCaptureBufferPhysicalAddressFile);

        if (sizeof(int) != bytesRead) {
            return false;
        }

        // unmap existing buffer
        if (pMemVirtAddressMapped) {
            munmap((void *)pMemVirtAddressMapped, sizeMapped);
        }

	//printf("physAddress=%08x\n", physAddress);

        pageOffset = physAddress & (getpagesize()-1);
	//printf("pageSize=%08x\n", getpagesize());
	//printf("pageOffset=%08x\n", (int)pageOffset);
        pageAlignedAddress = physAddress - pageOffset;
	//printf("pageAlignedAddres=%08x\n", pageAlignedAddress);
        sizeMapped = frameSize + pageOffset;
	//printf("size=%d, sizeMapped=%d\n", frameSize, sizeMapped);

        pMemVirtAddressMapped = mmap(
            (void *)pageAlignedAddress,
            (size_t)sizeMapped,
            PROT_READ,
            MAP_SHARED,
            (int)devMemFd,
            (off_t)pageAlignedAddress
        );
	//printf("pMemVirtAddressMapped=%p\n", pMemVirtAddressMapped);

        if (NULL == pMemVirtAddressMapped) {
            sizeMapped = 0;
            pMemVirtAddressBuffer = NULL;
            return false;
        }

        pMemVirtAddressBuffer = (void *)((int)pMemVirtAddressMapped + pageOffset);
	//printf("pMemVirtAddressBuffer=%p\n", pMemVirtAddressBuffer);

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
#ifdef COPY_ON_GRAB
	//printf("Converting color of cached YUV frame %p", pBufferYuv);
        cv::Mat matYuvColor = cv::Mat(FRAME_H, FRAME_W, CV_8UC2, pBufferYuv);
#else
	//printf("Converting color of pMemVirtAddressBuffer=%p\n", pMemVirtAddressBuffer);
        cv::Mat matYuvColor = cv::Mat(FRAME_H, FRAME_W, CV_8UC2, pMemVirtAddressBuffer);
#endif
        // Perform the colorspace conversion
        cv::cvtColor(matYuvColor, frameBgr.matFrame, CV_YUV2BGR_YUYV);

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
        return FRAME_W;
        break;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return FRAME_H;
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
