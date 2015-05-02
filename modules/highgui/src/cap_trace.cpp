#ifdef HAVE_TRACE

#include "precomp.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// For cvtColor
#include "opencv2/imgproc/imgproc.hpp"

#define PATH_PHYSICAL_ADDRESS "/dev/shm/camera_buffer_pa"
#define PATH_DEV_MEM "/dev/mem"

#define FRAME_W (1024)
#define FRAME_H (576)
#define FRAME_D (2)

#define FRAME_SIZE (FRAME_W * FRAME_H * FRAME_D)

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

    // Cache converted frame
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

    close();

    frameSize = FRAME_SIZE;

    // There is only one camera
    if (index) {
        return false;
    }

    // Open file with physical address (updated each frame)
    pCaptureBufferPhysicalAddressFile = fopen(PATH_PHYSICAL_ADDRESS, "rb");
    if (NULL == pCaptureBufferPhysicalAddressFile) {
        return false;
    }

    devMemFd = ::open(PATH_DEV_MEM, O_RDWR|O_SYNC);
    if (devMemFd < 0) {
        return false;
    }

    return true;
}

void CvCaptureCAM_Trace::close()
{
    // Unmap the buffer
    if (pMemVirtAddressMapped) {
        munmap((void *)pMemVirtAddressMapped, sizeMapped);
    }

    // Close /dev/shm/camera_buffer_pa
    fclose(pCaptureBufferPhysicalAddressFile);

    // Close /dev/mem
    ::close(devMemFd);

    pMemVirtAddressMapped = NULL;
    sizeMapped = 0;
    pMemVirtAddressBuffer = NULL;
    frameSize = 0;
    pCaptureBufferPhysicalAddressFile = NULL;
    devMemFd = -1;
    physAddress = 0;;

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
        munmap((void *)pMemVirtAddressMapped, sizeMapped);

        pageOffset = physAddress & getpagesize();
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
            return false;
        }

        pMemVirtAddressBuffer = (void *)((int)pMemVirtAddressMapped + pageOffset);
        return true;
    }
    
    return false;
}


IplImage* CvCaptureCAM_Trace::retrieveFrame(int)
{
    // Convert from 16-bit YUYV to BGR24
    if (pMemVirtAddressBuffer) {
        // Create matrix container for raw frame buffer
        cv::Mat matYuvColor = cv::Mat(FRAME_H, FRAME_W, CV_8UC2, pMemVirtAddressBuffer);
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
    case CV_CAP_PROP_FRAME_WIDTH:
        return FRAME_W;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return FRAME_H;
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
