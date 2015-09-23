
#pragma once

#ifdef HAVE_FFTW

//==============================================================================
//
// fftw3-ocvext.h
//
// fftw3-ocvext provides extended functionality to fftw3 for the purpose of replacing
// OpenCV DFT() functionality with FFTW version 3 equivalent. 
//
//
// These two calls produce the same output:
//
//		cv::dft( cvmat_in, cvmat_out, DFT_COMPLEX_OUTPUT, 0 );
//
//		p = fftwf_plan_dft_2d( Rows, Cols, fftwf_in, fftwf_out, FFTW_FORWARD, FFTW_ESTIMATE);
//		fftwf_execute(p); 
//
//
// and these two: 
//
//		cv::dft( cvmat_in, cvmat_out, 0, 0 );
//
//		p = fftwf_plan_dft_2d( Rows, Cols, fftwf_in, fftwf_out, FFTW_FORWARD, FFTW_ESTIMATE);
//		fftwf_execute(p); 
//		fftwf_FCtoCCS( fftwf_complex_buf_in, fftwf_real_buf_out, Rows, Cols );  // Complex bux 
//
//
// and these two: 
//
//		cv::dft( cvmat_in, cvmat_out, DFT_COMPLEX_OUTPUT, 0 );
// 
//		p = fftwf_plan_dft_r2c_2d( Rows, Cols, fftwf_in_r, fftwf_out, FFTW_ESTIMATE);
//		fftwf_execute(p); 
//		fftwf_HCtoFC( fftwf_out, fftwf_out, Rows, Cols );
//
//
// and these two: 
//
//		cv::dft( cvmat_in, cvmat_out, 0, 0 );
// 
//		p = fftwf_plan_dft_r2c_2d( Rows, Cols, fftwf_in_r, fftwf_out, FFTW_ESTIMATE);
//		fftwf_execute(p); 
//		fftwf_HCtoCCS( fftwf_out, fftwf_out, Rows, Cols );
//
//
// They are the only cases we will handle
//
//==============================================================================


#include "fftw3.h"


//==============================================================================
//						FFTWMatrix
//==============================================================================
//
// FFTWMatrix is a container for an fftw array.  
// It's purpose is to store attributes about a given fftw matriw . Such attributes 
// include number of rows, number of columns, whether the matrix contains  real or 
// complex data, and the base data type of the elements (double or float).
//
//==============================================================================


#define	FFTW_MAT_TYPE_UNKNOWN	0
#define	FFTW_MAT_TYPE_REAL		1
#define	FFTW_MAT_TYPE_COMPLEX	2

#define	FFTW_DATA_TYPE_UNKNOWN	0
#define	FFTW_DATA_TYPE_FLOAT	1
#define	FFTW_DATA_TYPE_DOUBLE	2



#define MultiTypePtrList	void 		  *	Void;		\
							float         * Real;		\
							fftwf_complex * Complex;	\
							unsigned char * Byte;		\
							unsigned int  * UInt;		\
							int           * Int;		\
							float 	      * Float;		\
							double		  * Double	




typedef union
{
	MultiTypePtrList;
	
} MultiTypePtr;				// various was to access the data



class MultiTypeBuffer
{
	public:
	
	int 			Size;		// sizeof of the buffer in bytes
	
	union						// various was to access the data
	{
		MultiTypePtrList;
	}; 

	MultiTypeBuffer()
	{
		Size= 0;	
		Void= NULL;
	}

	~MultiTypeBuffer()
	{
		Deallocate();
	}
	
	bool Allocate( int bytes )
	{
		Deallocate();

		Void= fftwf_malloc(bytes);
		if (Void != NULL)
		{
			Size= bytes;
		}
		return (Void != NULL);
	}
	
	void Deallocate()
	{
		if (Void != NULL)
		{
			 fftwf_free( Void );
		}
		Size= 0;	
		Void= NULL;
	}		

	
};

typedef struct
{
	int 	Rows;
	int 	Cols;
	int 	MatType;
	int 	DataType;

	MultiTypeBuffer	Data;
	
} FFTWMatrix;



//==============================================================================
//					Matrix Dump routines
//==============================================================================

extern char const * OCV_TypeToText( int Type );
extern int OCV_TextToType( char * Text );

extern bool DumpMatix_OCV( cv::Mat & mat,  char * FileName );
extern bool DumpMatix_OCV( cv::Mat & mat,  const char * Title, char * FileName );
extern bool DumpMatix_FFTWF( fftwf_complex * mat,  char * FileName, int Rows, int Cols );
extern bool DumpMatix_FFTWF( float * mat,  char * FileName, int Rows, int Cols );
extern bool DumpMatix_FFTWF( FFTWMatrix &FFTW,  char * FileName );


//==============================================================================
//					FFTWMatrix routines
//==============================================================================

extern char const * FFTW_MatTypeToText( int Type );
extern int          FFTW_TextToMatType( char * Text );
extern char const * FFTW_DataTypeToText( int Type );
extern int          FFTW_TextToDataType( char * Text );

extern bool FFTWMatrix_CreateFromOCV( cv::Mat &OCV, FFTWMatrix &FFTW, bool bForceComplex );
extern bool FFTWMatrix_CopyDataFromOCV( cv::Mat &OCV, FFTWMatrix &FFTW );
extern void FFTWMatrix_Destroy( FFTWMatrix &FFTW );


//==============================================================================
//					Matrix Conversion routines
//==============================================================================

CV_EXPORTS_W void fftwf_HCtoFC( fftwf_complex * in, fftwf_complex * out, const int rows, const int cols );
CV_EXPORTS_W void fftwf_FCtoCCS( fftwf_complex * in, float  * out, const int rows, const int cols );
CV_EXPORTS_W void fftwf_HCtoCCS( fftwf_complex * in, float * out, const int rows, const int cols );


//==============================================================================
//					OpenCV replacement routines
//==============================================================================

extern bool fftw_CanProcessOCVdft( cv::InputArray _src0, cv::OutputArray _dst, int flags, int nonzero_rows );
extern bool fftw_ProcessOCVdft( cv::InputArray _src0, cv::OutputArray _dst, int flags, int nonzero_rows );


CV_EXPORTS_W bool is_enabled_fftw_dft();
CV_EXPORTS_W bool enable_fftw_dft( bool bEnable );


#endif

