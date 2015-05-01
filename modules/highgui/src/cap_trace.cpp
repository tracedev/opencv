#ifdef HAVE_TRACE

#include "precomp.hpp"

#define PATH_PHYSICAL_ADDRESS "/dev/shm/camera_buffer_pa"
#define PATH_DEV_MEM "/dev/mem"

// Capture video frames from Trace video camera

class CvCaptureCAM_Trace : public CvCapture
{
public:
    CvCaptureCAM_Trace() { init(); }
    virtual ~CvCaptureCAM_Trace() { close(); }

    virtual bool open( int index );
    virtual void close();

    virtual double getProperty(int);
    virtual bool setProperty(int, double) { return false; }
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);
    virtual int getCaptureDomain() { return CV_CAP_TRACE; } // Return the type of the capture object: CV_CAP_VFW, etc...

protected:
    void init();

    int frame_w = 1024;
    int frame_h = 576;
    int frame_d = 2;

private:
    volatile void *pMemVirtAddressMapped;
    int sizeMapped;
    volatile void *pMemVirtAddressBuffer;
    int frameSize;
    FILE *pCaptureBufferPhysicalAddressFile;
    fd devMemFd;
    uint32_t physAddress;

    // Cache converted frames
    OutputMap frameColor;
    
};


void CvCaptureCAM_Trace::init()
{
    pMemVirtAddressMapped = NULL;
    sizeMapped = 0;
    pMemVirtAddressBuffer = NULL;
    frameSize = 0;
    pCaptureBufferPhysicalFile = NULL;
    devMemFd = -1;
    physAddress = 0;;
}

// Initialize camera input
bool CvCaptureCAM_Trace::open( int index )
{

    close();

    frameSize = frame_w * frame_h * frame_d;

    // There is only one camera
    if (index) {
        return false;
    }

    // Open file with physical address (updated each frame)
    pCaptureBufferPhysicalFile = fopen(PATH_PHYSICAL_ADDRESS, "rb");
    if (NULL == pCaptureBufferPhysicalFile) {
        return false;
    }

    devMemFd = open(PATH_DEV_MEM, O_RDWR|O_SYNC);
    if (devMemFd < 0) {
        return false;
    }

    return true;
}

void CvCaptureCAM_Trace::close( CvCaptureCAM_Trace* capture )
{
    // Unmap the buffer
    if (pMemVirtAddressMapped) {
        munmap((void *)pMemVirtAddressMapped, sizeMapped);
    }

    // Close /dev/shm/camera_buffer_pa
    fclose(pCaptureBufferPhysicalAddressFile);

    // Close /dev/mem
    close(devMemFd);

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

        pageOffset = physAddress & MMAP_MEM_PAGEALIGN;
        pageAlignedAddress = physAddr - pageOffset;
        sizeMapped = frameSize + pageOffset;

        pMemVirtAddressMapped = mmap(
            (void *)pageAlignedAddress,
            (size_t)sizeMapped,
            (int)devMemFd,
            (off_t)pageAlignedAddress
        );

        if (NULL == pMemVirtAddressMapped) {
            sizeMapped = 0;    
            return false;
        }

        pMemVirtualAddressBuffer = (int)((int)pMemVirtAddressMapped + pageOffset);
        return true;
    }
    
    return false;
}


IplImage* CvCaptureCAM_Trace::retrieveFrame(int)
{
    // Convert from 16-bit YUYV to BGR24
    if (pMemVirtualAddressBuffer) {
        // Create matrix container for raw frame buffer
        cv::Mat YUV_frame = cv:Mat(frame_w, frame_h, CV_8UC2, pMemVirtualAddressBuffer);
        cvtColor(YUV_frame, frameColor.res, CV_YUV2BGR_YUYV);

        return frameColor.getIplImagePtr();
    } else {
        return NULL;
    }
}

double CvCaptureCAM_Trace::getProperty( int property_id )
{
    switch( property_id )
    {
    case CV_CAP_PROP_FRAME_WIDTH:
        return frame_w;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return frame_h;
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
