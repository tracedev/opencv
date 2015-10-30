#ifdef HAVE_TRACE

#include "precomp.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

//#define TIMING_MEASUREMENTS

// For cvtColor
#include "opencv2/imgproc.hpp"

#define PATH_PHYSICAL_ADDRESS "/dev/shm/camera_buffer_pa"
#define PATH_DEV_MEM "/dev/cachedmem"

// define COPY_ON_GRAB to immediately create a copy of the YUV frame before the
// later color space conversion in retrieve(). If the buffer isn't stable 
// for the interval between a grab and retrieve this is necessary
#define COPY_ON_GRAB

// Capture video frames from Trace video camera

//======================================================
//		Virtual Buffer manager
//======================================================

#define USE_VM_BUFFER_MANAGER	

#ifdef USE_VM_BUFFER_MANAGER

//#define dbgprintf(x)  { printf x;   fflush( stdout ); }
#define dbgprintf(x)

typedef struct 
{
	void * PhysicalAddress;
	void * VirtualAddress;
	size_t Size;
		
} vm_mapping;


class vmBufferManager
{
	private:
	
	vm_mapping	vmBuffers[15];
	int			vmBufferCnt;
	int			vmBufferMax;
	
	int 		fd_devmem;

	public:

	vmBufferManager()
	{
		memset( vmBuffers, 0, sizeof(vmBuffers) );
		vmBufferCnt= 0;
		vmBufferMax= sizeof(vmBuffers)/sizeof(vmBuffers[0]);
		
		fd_devmem = ::open(PATH_DEV_MEM, O_RDWR|O_SYNC);
    	if (fd_devmem < 0) 
    	{
    		dbgprintf(( "\n cap_trace::vmBufferManager:   open(PATH_DEV_MEM, O_RDWR|O_SYNC) failed" ));  	
    	}
	}

	~vmBufferManager()
	{
		ReleaseVmMappings();
	   	if (fd_devmem < 0) 
    	{
    		::close(fd_devmem);
    	}

	}

	void ReleaseVmMappings()
	{
		for (int i=0; i<vmBufferCnt; i++)
		{
			if (vmBuffers[i].VirtualAddress != NULL)
			{
				munmap( vmBuffers[i].VirtualAddress, vmBuffers[i].Size );
			} 
		}
		memset( vmBuffers, 0, sizeof(vmBuffers) );
		vmBufferCnt= 0;
	}


	void * MakeVmMapping( void * DesiredPhysicalAddress, size_t  DesiredSize, void ** MappedPhysicalAddress, size_t  * MappedSize )
	{
	  	if (fd_devmem < 0)
	  	{
	  		return (NULL);
	  	}
   
     	uint32_t	PageOffset = ((uint32_t) DesiredPhysicalAddress) & (getpagesize()-1);
        void *		PageAlignedAddress = (void *) (((uint32_t) DesiredPhysicalAddress) - PageOffset);
        uint32_t 	PageAlignedSize = DesiredSize + PageOffset;

		void * 		MappedVirtualAddress;

        dbgprintf(( "\n cap_trace::MakeVmMapping:   DesiredPhysicalAddress = %p ", DesiredPhysicalAddress ));
        dbgprintf(( "\n cap_trace::MakeVmMapping:   DesiredSize            = %d ", DesiredSize ));  			
        dbgprintf(( "\n cap_trace::MakeVmMapping:   PageOffset             = %d ", PageOffset ));  			
        dbgprintf(( "\n cap_trace::MakeVmMapping:   PageAlignedAddress     = %p ", PageAlignedAddress ));  	
        dbgprintf(( "\n cap_trace::MakeVmMapping:   PageAlignedSize        = %d ", PageAlignedSize ));  		
   
        MappedVirtualAddress= mmap(	PageAlignedAddress,
                					PageAlignedSize,

					                PROT_READ,
									//PROT_READ|PROT_WRITE|PROT_EXEC,

									MAP_SHARED,
									//MAP_SHARED | MAP_ANONYMOUS,
									//MAP_PRIVATE,
									//MAP_PRIVATE | MAP_ANONYMOUS,
									//MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
									//MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED,

  						            (int)fd_devmem,
  						            (off_t)PageAlignedAddress );
  									//(int) -1,
									//(off_t) 0 );
 
 
       	if (MappedVirtualAddress != MAP_FAILED) 
       	{
        	dbgprintf(( "\n cap_trace::vmBufferManager:   mmap() success  pa=%p, size= %d, va = %p ", PageAlignedAddress, PageAlignedSize, MappedVirtualAddress )); 
		    
		    *MappedPhysicalAddress= PageAlignedAddress;
		    *MappedSize = PageAlignedSize;
		    return (MappedVirtualAddress);
       	}
       	
        dbgprintf(( "\n cap_trace::vmBufferManager:   mmap() FAILED !!  pa=%p, size= %d ", PageAlignedAddress, PageAlignedSize ));  

        *MappedPhysicalAddress= NULL;
        *MappedSize = 0;
        return (NULL);
	}
	
	
	void * GetVmMapping( void * PhysicalAddress, size_t Size )
	{
			// First try to use an existing mapping

        dbgprintf(( "\n cap_trace::GetVmMapping:  Entry, pa=%p,  size = %d ", PhysicalAddress, Size ));  
		
		for (int i=0; i<vmBufferCnt; i++)
		{
		
			uint32_t	vmbuf_pa_start   = (uint32_t) vmBuffers[i].PhysicalAddress;
			uint32_t	vmbuf_pa_end     = vmbuf_pa_start + vmBuffers[i].Size;
			uint32_t	desired_pa_start = (uint32_t) PhysicalAddress;
			uint32_t	desired_pa_end   = desired_pa_start + Size;
			
			
			if  ((vmbuf_pa_start <= desired_pa_start) &&
			 	 (vmbuf_pa_end   >= desired_pa_end) )
			{
			 	  
				// it fits, just return the corresponding part of this one
				
				dbgprintf(( "\n cap_trace::vmBufferManager: Found match for pa=%p, size= %d at entry [%d]  ",  PhysicalAddress, Size, i ));

				return (void *) (((uint32_t) vmBuffers[i].VirtualAddress) + (desired_pa_start - vmbuf_pa_start));
			}
		}
		
			// No existing mapping found, create a new one
			
		if (vmBufferCnt < vmBufferMax)
		{
			void * MappedPhysicalAddress;
			void * MappedVirtualAddress;
			size_t MappedSize;
			 
			MappedVirtualAddress = MakeVmMapping( PhysicalAddress, Size, &MappedPhysicalAddress, &MappedSize );
			
			if (MappedVirtualAddress != NULL)
			{
				vmBuffers[vmBufferCnt].PhysicalAddress = MappedPhysicalAddress;
				vmBuffers[vmBufferCnt].VirtualAddress  = MappedVirtualAddress;
				vmBuffers[vmBufferCnt].Size            = MappedSize;
				vmBufferCnt++;

        		dbgprintf(( "\n cap_trace::vmBufferManager: Added entry [%d],  pa=%p, va= %p, size= %d  ", vmBufferCnt-1, MappedPhysicalAddress, MappedVirtualAddress, MappedSize ));
        		
        		return (MappedVirtualAddress);        		
			}
		}
		else
		{
			dbgprintf(( "\n cap_trace::vmBufferManager:  Max buffers reached (%d) \n", vmBufferMax ));  
		}
		
		return (NULL);
	}

};
#endif

//======================================================
//		NEON Memcpy
//======================================================

#ifdef CV_NEON 

__attribute__ ((noinline))  void memcpy_neon( void * tgt, const void * src, size_t  bytes );

__attribute__ ((noinline))  
void memcpy_neon( void * tgt, const void * src, size_t  bytes )
{
	
	// Buffers are word aligned, handle with neon

	volatile register uint32_t tmp;

    asm volatile
    (
     "1:      			  \n"
     "cmp  %2,#0x80		  \n"		// 128 byte chunk (loop)
     "blt 	2f			  \n"
     "vldm %1!,{d0-d15}   \n"
     "vstm %0!,{d0-d15}   \n"
     "sub  %2,%2,#0x80    \n"
     "b    1b    		  \n"
     "2:      			  \n"

     "cmp  %2,#0x40		  \n"		// 64 byte chunk
     "ittt	 ge	  		  \n"		
     "vldmge %1!,{d0-d7}  \n"
     "vstmge %0!,{d0-d7}  \n"
     "subge  %2,%2,#0x40  \n"
 
     "cmp  %2,#0x20		  \n"		// 32 byte chunk
     "ittt	 ge	  		  \n"		
     "vldmge %1!,{d0-d3}  \n"
     "vstmge %0!,{d0-d3}  \n"
     "subge  %2,%2,#0x20  \n"
 
     "cmp  %2,#0x10		  \n"		// 16 byte chunk
     "ittt	 ge	  		  \n"		
     "vldmge %1!,{d0-d1}  \n"
     "vstmge %0!,{d0-d1}  \n"
     "subge  %2,%2,#0x10  \n"
     
     "cmp  %2,#0x8		  \n"		// 8 byte chunk
     "ittt	 ge	  		  \n"	
     "vldmge %1!,{d0}     \n"
     "vstmge %0!,{d0}     \n"
     "subge  %2,%2,#0x8   \n"
 
     "cmp  %2,#0x4		  \n"		// 4 byte chunk
     "itt	 ge	  		  \n"
     "ldrge  %3,[%1]	  \n"
     "strge  %3,[%0]	  \n"
     "ittt	 ge	  		  \n"
	 "addge  %0,%0,#0x4   \n"	
	 "addge  %1,%1,#0x4   \n"	
     "subge  %2,%2,#0x4   \n"

     "cmp  %2,#0x2		  \n"		// 2 byte chunk
     "itt	 ge	  		  \n"
     "ldrhge  %3,[%1]	  \n"
     "strhge  %3,[%0]	  \n"
     "ittt	 ge	  		  \n"
	 "addge  %0,%0,#0x2   \n"	
	 "addge  %1,%1,#0x2   \n"	
     "subge  %2,%2,#0x2   \n"
	  
     "cmp  %2,#0x1		  \n"		// 1 byte chunk
     "itt	  ge		  \n"
     "ldrbge  %3,[%1]	  \n"
     "strbge  %3,[%0]	  \n"
   //"subge  %2,%2,#0x1   \n"
 
	  : "+r"(tgt),      // %0
	 	"+r"(src),     	// %1
	  	"+r"(bytes),    // %2
	  	"+r"(tmp)       // %3
    );
}

#endif

void * fastMemcpy( void *dest, const void *src, size_t n );

void * fastMemcpy( void *dest, const void *src, size_t n )
{

#ifdef CV_NEON 
	// Neon has problems with buffers that are not word aligned, 
	// so we use normal memcpy() in those cases.  Note that we could
	// check to see if the buffers can be word aligned by processing  
	// some leading bytes, but since this routine will almost always
	// be called with aligned buffers anyway we don't bother.
	
	if ( ((((uint32_t) dest) % 4) == 0) &&
	     ((((uint32_t) src)  % 4) == 0) )
	{
		memcpy_neon( dest, src, n );
		return (dest);
    }
#endif

    // default = use normal memcpy; 
    
    return (memcpy( dest, src, n ));
}

//======================================================
//	
//======================================================

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
        int fourcc;
        int frameWidth, frameHeight, frameDepth;
        FILE *pCaptureBufferPhysicalAddressFile;
        int devMemFd;
        uint32_t physAddress;

        // Frame counters and timestamp
        int frameNumber;
        struct timeval tsBegin;
        struct timeval tsCurrent;


#ifdef USE_VM_BUFFER_MANAGER
		vmBufferManager	vmBufMgr;
#endif

        // Cache converted frame
#ifdef COPY_ON_GRAB
        uint8_t *pBufferYuv;
#endif
        frameInternal returnFrame;

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
    fourcc =  CV_FOURCC_MACRO('A', 'R', 'G', 'B');
#ifdef COPY_ON_GRAB
    pBufferYuv = NULL;
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

#ifdef USE_VM_BUFFER_MANAGER
	vmBufMgr.ReleaseVmMappings();
#endif


#ifdef COPY_ON_GRAB
    // Allocate a temporary YUV copy buffer
    if (frameSize) {
        pBufferYuv = (uint8_t *)cv::fastMalloc(frameSize);
        if (NULL == pBufferYuv) {
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

#ifdef USE_VM_BUFFER_MANAGER
	vmBufMgr.ReleaseVmMappings();
#endif

#ifdef COPY_ON_GRAB
    if (pBufferYuv) {
        cv::fastFree(pBufferYuv);
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

        //printf("Got pa=%08x, width=%d, height=%d, depth=%d\n", physAddress, frameWidth, frameHeight, frameDepth);

        if (paramsFound != 4) {
            return false;
        }

        frameSizeNew = frameWidth * frameHeight * frameDepth;

#ifdef COPY_ON_GRAB
        // If the frame size changed we need to reallocate our buffer
        if (frameSizeNew != frameSize) {
            if (frameSize) {
                if (NULL != pBufferYuv) {
                	cv::fastFree(pBufferYuv);
                }
                pBufferYuv = (uint8_t *) cv::fastMalloc(frameSize);
                if (NULL == pBufferYuv) {
                    return false;
                }
            }
        }
#endif

        frameSize = frameSizeNew;


#ifdef USE_VM_BUFFER_MANAGER

		pMemVirtAddressBuffer= vmBufMgr.GetVmMapping( (void *) physAddress, (size_t) frameSize );
		if (pMemVirtAddressBuffer == NULL)
		{
			return false;
		}
#else

		// unmap existing buffer
        if (pMemVirtAddressMapped) {
            munmap((void *)pMemVirtAddressMapped, sizeMapped);
        }

		uint32_t pageAlignedAddress;
		off_t pageOffset;

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

        if (MAP_FAILED == pMemVirtAddressMapped) {
            pMemVirtAddressMapped= NULL;
            sizeMapped = 0;
            pMemVirtAddressBuffer = NULL;
            return false;
        }

        pMemVirtAddressBuffer = (void *)((uint32_t)pMemVirtAddressMapped + pageOffset);

#endif

#ifdef COPY_ON_GRAB
        // Copy entire YUV frame here, in practice the frame buffers update
        // too quickly and we see interleaved frames otherwise

        fastMemcpy( pBufferYuv, pMemVirtAddressBuffer, frameSize );
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

#ifdef TIMING_MEASUREMENTS
        struct timeval tsStart, tsEnd, tsElap;
        gettimeofday(&tsStart, NULL); 
#endif

        // Create matrix container for raw frame buffer
#ifdef COPY_ON_GRAB
        cv::Mat matYuvColor = cv::Mat(frameHeight, frameWidth, CV_8UC2, pBufferYuv);
#else
        cv::Mat matYuvColor = cv::Mat(frameHeight, frameWidth, CV_8UC2, pMemVirtAddressBuffer);
#endif

        // Perform the colorspace conversion
        switch (fourcc)
        {
            case CV_FOURCC_MACRO('Y', 'U', 'Y', '2'):
                returnFrame.matFrame = matYuvColor;
                break;
            case CV_FOURCC_MACRO('A', 'R', 'G', 'B'):
                cv::cvtColor(matYuvColor, returnFrame.matFrame, CV_YUV2BGRA_YUY2);
                break;
            case CV_FOURCC_MACRO('2', '4', 'B', 'G'):
                cv::cvtColor(matYuvColor, returnFrame.matFrame, CV_YUV2BGR_YUY2);
                break;
            case CV_FOURCC_MACRO('G', 'R', 'A', 'Y'):
                cv::cvtColor(matYuvColor, returnFrame.matFrame, CV_YUV2GRAY_YUY2);
                break;
        }

#ifdef TIMING_MEASUREMENTS
        gettimeofday(&tsEnd, NULL); 
        timersub(&tsEnd, &tsStart, &tsElap);
        printf("%f ms\n", 1000 * tsElap.tv_sec + ((double) tsElap.tv_usec) / 1000);
#endif

        return returnFrame.retrieveFrame();
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
        case CV_CAP_PROP_FOURCC:
            return fourcc;
            break;
    }
    return 0;
}

bool CvCaptureCAM_Trace::setProperty( int property_id, double property_value)
{
    int numSupportedFourcc = 4;
    int supportedFourcc[] = { CV_FOURCC_MACRO('Y', 'U', 'Y', '2'),
        CV_FOURCC_MACRO('A', 'R', 'G', 'B'),
        CV_FOURCC_MACRO('2', '4', 'B', 'G'),
        CV_FOURCC_MACRO('G', 'R', 'A', 'Y')
     };
    switch( property_id )
    {
        case CV_CAP_PROP_FOURCC:
            for (int i = 0; i < numSupportedFourcc; i++)
            {
                if ((int)property_value == supportedFourcc[i]) {
                    fourcc = (int) property_value;
                    return true;
                }
            }
            break;
    }
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
