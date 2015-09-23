
#ifdef HAVE_FFTW

#include "fftw3-ocvext.h"

using namespace cv;


//==============================================================================
//					Matrix Dump routines
//==============================================================================
//
//  These routines dump a matix to a file as text.  They are used for debugging.
//  If the specified filename is NULL, the text will be written to stdout.
//

#define MAX_DATA_DUMP			100											// Maximum number of data elements that will be dumped, 0==all 
#define MAX_DATA_REACHED(c)	   ((MAX_DATA_DUMP!=0) && ((c)>=MAX_DATA_DUMP))	


//------------------------
// OpenCV matrices:
//------------------------

//enum { CV_8U=0, CV_8S=1, CV_16U=2, CV_16S=3, CV_32S=4, CV_32F=5, CV_64F=6 };
//
//CV_8U - 8-bit unsigned integers ( 0..255 )
//CV_8S - 8-bit signed integers ( -128..127 )
//CV_16U - 16-bit unsigned integers ( 0..65535 )
//CV_16S - 16-bit signed integers ( -32768..32767 )
//CV_32S - 32-bit signed integers ( -2147483648..2147483647 )
//CV_32F - 32-bit floating-point numbers ( -FLT_MAX..FLT_MAX, INF, NAN )
//CV_64F - 64-bit floating-point numbers ( -DBL_MAX..DBL_MAX, INF, NAN )

char const * OCV_TypeToText( int Type )
{
	if (Type == CV_8U)	return ("CV_8U"); 		// 0
	if (Type == CV_8S)	return ("CV_8S"); 		// 1
	if (Type == CV_16U)	return ("CV_16U"); 		// 2 
	if (Type == CV_16S)	return ("CV_16S"); 		// 3
	if (Type == CV_32S)	return ("CV_32S"); 		// 4
	if (Type == CV_32F)	return ("CV_32F"); 		// 5
	if (Type == CV_64F)	return ("CV_64F"); 		// 6
	
	return ("<undefined>");
}

int OCV_TextToType( char * Text )
{
	if (strcmp( Text, "CV_8U" ) == 0)	return (CV_8U); 		// 0
	if (strcmp( Text, "CV_8S" ) == 0)	return (CV_8S); 		// 1
	if (strcmp( Text, "CV_16U") == 0)	return (CV_16U); 		// 2 
	if (strcmp( Text, "CV_16S") == 0)	return (CV_16S); 		// 3
	if (strcmp( Text, "CV_32S") == 0)	return (CV_32S); 		// 4
	if (strcmp( Text, "CV_32F") == 0)	return (CV_32F); 		// 5
	if (strcmp( Text, "CV_64F") == 0)	return (CV_64F); 		// 6
	
	return (-1);
}


bool DumpMatix_OCV( cv::Mat & mat, const char * Title, char * FileName )
{
	bool bRet= false;
	
	FILE * fp = (FileName != NULL) ? fopen( FileName, "w" ) : stdout;
	
	if (fp!=NULL)
	{
		if (Title != NULL)
		{
			fprintf( fp, "\n" );
			fprintf( fp, "\n%s", Title );
			fprintf( fp, "\n" );
		}
	
		fprintf( fp, "\nrows=%d", mat.rows );
		fprintf( fp, "\ncols=%d", mat.cols );
		fprintf( fp, "\ndims=%d", mat.dims );
		fprintf( fp, "\nchannels()=%d", mat.channels() );
		fprintf( fp, "\ntype()=%d   (%s)", mat.type(), OCV_TypeToText(mat.type()) );
		fprintf( fp, "\nelemSize()=%d", mat.elemSize() );
		fprintf( fp, "\nelemSize1()=%d", mat.elemSize1() );
		fprintf( fp, "\ndepth()=%d   (%s)", mat.depth(), OCV_TypeToText(mat.depth()) );
		fprintf( fp, "\nempty()=%c", mat.empty() ? 'T' : 'F' );
		fprintf( fp, "\nisContinuous()=%c", mat.isContinuous() ? 'T' : 'F' );
		fprintf( fp, "\nisSubmatrix()=%c", mat.isSubmatrix() ? 'T' : 'F' );

		for (int i=0; i<mat.dims; i++)
		{
			fprintf( fp, "\nsize[%d]=%d", i, mat.size[i] );
		}
		
		for (int i=0; i<mat.dims; i++)
		{
			fprintf( fp, "\nstep[%d]=%d", i, mat.step[i] );
		}
		
		for (int i=0; i<mat.dims; i++)
		{
			fprintf( fp, "\nstep1(%d)=%d", i, mat.step1(i) );
		}
		
		
		fprintf( fp, "\ndata=" );
		int mat_type= mat.type();
		int outcnt=0;
		
		if (mat.dims == 1)
		{
			for (int i= 0; i< mat.size[0]; i++)
			{
				if (MAX_DATA_REACHED(outcnt)) break;	// stop if max reached

				if (mat_type == CV_8U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned char >(i) );
				if (mat_type == CV_8S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   char >(i) );
				if (mat_type == CV_16U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned short>(i) );
				if (mat_type == CV_16S)	fprintf( fp, "%d,",  (signed int)   mat.at<unsigned short>(i) );
				if (mat_type == CV_32S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   int  >(i) );
				if (mat_type == CV_32F)	fprintf( fp, "%f,",  (float)		mat.at<float         >(i) );
				if (mat_type == CV_64F)	fprintf( fp, "%lf,", (double)       mat.at<double        >(i) );
				
				outcnt++;
			}
		}
		
		if (mat.dims == 2)
		{
			for (int i= 0; i< mat.size[0]; i++)
			for (int j= 0; j< mat.size[1]; j++)
			{
				if (MAX_DATA_REACHED(outcnt)) break;	// stop if max reached

				if (mat_type == CV_8U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned char >(i,j) );
				if (mat_type == CV_8S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   char >(i,j) );
				if (mat_type == CV_16U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned short>(i,j) );
				if (mat_type == CV_16S)	fprintf( fp, "%d,",  (signed int)   mat.at<unsigned short>(i,j) );
				if (mat_type == CV_32S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   int  >(i,j) );
				if (mat_type == CV_32F)	fprintf( fp, "%f,",  (float)		mat.at<float         >(i,j) );
				if (mat_type == CV_64F)	fprintf( fp, "%lf,", (double)       mat.at<double        >(i,j) );
				
				outcnt++;
			}
		}

		if (mat.dims == 3)
		{
			for (int i= 0; i< mat.size[0]; i++)
			for (int j= 0; j< mat.size[1]; j++)
			for (int k= 0; k< mat.size[2]; k++)
			{
				if (MAX_DATA_REACHED(outcnt)) break;	// stop if max reached

				if (mat_type == CV_8U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned char >(i,j,k) );
				if (mat_type == CV_8S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   char >(i,j,k) );
				if (mat_type == CV_16U)	fprintf( fp, "%u,",  (unsigned int) mat.at<unsigned short>(i,j,k) );
				if (mat_type == CV_16S)	fprintf( fp, "%d,",  (signed int)   mat.at<unsigned short>(i,j,k) );
				if (mat_type == CV_32S)	fprintf( fp, "%d,",  (signed int)   mat.at<signed   int  >(i,j,k) );
				if (mat_type == CV_32F)	fprintf( fp, "%f,",  (float)		mat.at<float         >(i,j,k) );
				if (mat_type == CV_64F)	fprintf( fp, "%lf,", (double)       mat.at<double        >(i,j,k) );
				
				outcnt++;
			}
		}
		

		#if (1)
		fprintf( fp, "\nraw =" );
		
		float * pf = (float *) mat.data;
		
		for (int i=0; i< (mat.rows*mat.cols*mat.channels()); i++)
		{
			if (MAX_DATA_REACHED(i)) break;	// stop if max reached

			fprintf( fp, "%f, ",  (float)  pf[ i ] );	// raw dump
		} 
		#endif



		fprintf( fp, "\n" );
		if (fp!=stdout)
		{	
			fclose (fp);	
		}

		bRet= true;
	}
	
	return (bRet);
}

bool DumpMatix_OCV( cv::Mat & mat, char * FileName )
{
	return (DumpMatix_OCV( mat, NULL, FileName ));
}

//------------------------
// FFTW matrices:
//------------------------
	

char const * FFTW_MatTypeToText( int Type )
{
	if (Type == FFTW_MAT_TYPE_REAL)		return ("Real"); 		// 1
	if (Type == FFTW_MAT_TYPE_COMPLEX)	return ("Complex"); 	// 2
	
	return ("<undefined>");
}

int FFTW_TextToMatType( char * Text )
{
	if (strcmp( Text, "Real" ) == 0)	return (FFTW_MAT_TYPE_REAL); 		// 1
	if (strcmp( Text, "Complex" ) == 0)	return (FFTW_MAT_TYPE_COMPLEX); 	// 2

	return (FFTW_MAT_TYPE_UNKNOWN);
}
	

char const * FFTW_DataTypeToText( int Type )
{
	if (Type == FFTW_DATA_TYPE_FLOAT)	return ("float"); 		// 1
	if (Type == FFTW_DATA_TYPE_DOUBLE)	return ("double"); 		// 2
	
	return ("<undefined>");
}

int FFTW_TextToDataType( char * Text )
{
	if (strcmp( Text, "float" ) == 0)	return (FFTW_DATA_TYPE_FLOAT); 		// 1
	if (strcmp( Text, "double" ) == 0)	return (FFTW_DATA_TYPE_DOUBLE); 	// 2

	return (FFTW_DATA_TYPE_UNKNOWN);
}


bool DumpMatix_FFTWF( fftwf_complex * mat,  char * FileName, int Rows, int Cols )
{
	bool bRet= false;

	FILE * fp = (FileName != NULL) ? fopen( FileName, "w" ) : stdout;

	if (fp!=NULL)
	{
		fprintf( fp, "\n" );

		fprintf( fp, "\nrows=%d", Rows );
		fprintf( fp, "\ncols=%d", Cols );
		fprintf( fp, "\ndims=%d", 2 );

		fprintf( fp, "\ndata=" );
		
		for (int r=0; r<Rows; r++)
		{
			for (int c=0; c<Cols; c++)
			{
				int i= (r*Cols) + c;

				if (MAX_DATA_REACHED(i*2)) break;	// stop if max reached
			
				fprintf( fp, "%f, ",  (float)  mat[ i ][0] );			// real part
				fprintf( fp, "%f, ",  (float)  mat[ i ][1] );			// imaginary part
			}
		} 
	

		#if (0)
		fprintf( fp, "\nraw =" );
		
		float * pf = (float *) mat;
		
		for (int i=0; i< (Rows*Cols*2); i++)
		{
			if (MAX_DATA_REACHED(i)) break;				// stop if max reached
			fprintf( fp, "%f, ",  (float)  pf[ i ] );	// raw dump
		} 
		#endif


		if (fp!=stdout)
		{	
			fclose (fp);	
		}

		bRet= true;
	}
	
	return (bRet);
}


bool DumpMatix_FFTWF( float * mat,  char * FileName, int Rows, int Cols )
{
	bool bRet= false;

	FILE * fp = (FileName != NULL) ? fopen( FileName, "w" ) : stdout;

	if (fp!=NULL)
	{
		fprintf( fp, "\n" );

		fprintf( fp, "\nrows=%d", Rows );
		fprintf( fp, "\ncols=%d", Cols );
		fprintf( fp, "\ndims=%d", 2 );

		fprintf( fp, "\ndata=" );
		
		for (int r=0; r<Rows; r++)
		{
			for (int c=0; c<Cols; c++)
			{
				int i= (r*Cols) + c;
	
				if (MAX_DATA_REACHED(i)) break;				// stop if max reached
				
				fprintf( fp, "%f, ",  (float)  mat[ i ] );	// real part
			}
		} 
	

		if (fp!=stdout)
		{	
			fclose (fp);	
		}

		bRet= true;
	}
	
	return (bRet);
}

bool DumpMatix_FFTWF( FFTWMatrix &FFTW,  char * FileName )
{
	bool bRet= false;

	FILE * fp = (FileName != NULL) ? fopen( FileName, "w" ) : stdout;

	if (fp!=NULL)
	{
		fprintf( fp, "\nRows        = %d", FFTW.Rows );
		fprintf( fp, "\nCols        = %d", FFTW.Cols );
		fprintf( fp, "\nMatType     = %d   (%s)", FFTW.MatType,  FFTW_MatTypeToText(FFTW.MatType) );
		fprintf( fp, "\nDataType    = %d   (%s)", FFTW.DataType, FFTW_DataTypeToText(FFTW.DataType) );
		fprintf( fp, "\nData.Size   = %d", FFTW.Data.Size);
		fprintf( fp, "\nData.Void   = %p", FFTW.Data.Void );
	
		if (FFTW.Data.Void != NULL)
		{
			if (FFTW.MatType == FFTW_MAT_TYPE_REAL)
			{	
				fprintf( fp, "\nData=" );
			
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					int i= (r*FFTW.Cols) + c;
	
					if (MAX_DATA_REACHED(i)) break;							// stop if max reached

					fprintf( fp, "%f, ",  (float)  FFTW.Data.Real[ i ] );			// real part
				}
			}
			
			if (FFTW.MatType == FFTW_MAT_TYPE_COMPLEX)
			{		
				fprintf( fp, "\nData=" );
		
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					int i= (r*FFTW.Cols) + c;

					if (MAX_DATA_REACHED(i*2)) break;							// stop if max reached

					fprintf( fp, "%f, ",  (float)  FFTW.Data.Complex[ i ][0] );	// real part
					fprintf( fp, "%f, ",  (float)  FFTW.Data.Complex[ i ][1] );	// imaginary part
				}

			} 
		}
		
		if (fp!=stdout)
		{	
			fclose (fp);	
		}

		bRet= true;
	}
	
	return (bRet);
}




//==============================================================================
//					FFTWMatrix routines
//==============================================================================
//
// These functions operate on an FFTWMatrix type var
 

bool FFTWMatrix_CreateFromOCV( cv::Mat &OCV, FFTWMatrix &FFTW, bool bForceComplex )
{
	memset( &FFTW, 0, sizeof(FFTW) );
	
	if 	((OCV.dims == 2) &&				// For now, we only habdle 2d matrices
		 (OCV.depth() == CV_32F))		// For now, we only handle floats.
	{
		FFTW.Rows= OCV.rows;
		FFTW.Cols= OCV.cols;

		if (OCV.channels() == 1) 
		{
			FFTW.MatType  = FFTW_MAT_TYPE_REAL;
		 	FFTW.DataType = FFTW_DATA_TYPE_FLOAT;
		 	FFTW.Data.Size = FFTW.Rows * FFTW.Cols * sizeof(float) * 2;  // * 2 in case we use it as complex  
		}
		
		if ((OCV.channels() == 2) || (bForceComplex))
		{
			FFTW.MatType  = FFTW_MAT_TYPE_COMPLEX;
		 	FFTW.DataType = FFTW_DATA_TYPE_FLOAT;
		 	FFTW.Data.Size = FFTW.Rows * FFTW.Cols * sizeof(float) * 2;
		}
		
		if (FFTW.Data.Size > 0)
		{
			FFTW.Data.Void= fftwf_malloc( FFTW.Data.Size );
		}
	
	}
	
	return (FFTW.Data.Void != NULL);
}  


void  FFTWMatrix_Destroy( FFTWMatrix &FFTW )
{
	if (FFTW.Data.Void != NULL) 
	{
		fftwf_free(FFTW.Data.Void);
	}
	
	memset( &FFTW, 0, sizeof(FFTW) );
}


bool FFTWMatrix_CopyDataFromOCV( cv::Mat &OCV, FFTWMatrix &FFTW )
{
	bool bCopied= false;
	
	if (FFTW.Data.Void != NULL) 
	{
		if ((FFTW.Rows == OCV.rows)  					&&	// for now, we only do idenically sized matrices
			(FFTW.Cols == OCV.cols)  					&&
			(FFTW.DataType  == FFTW_DATA_TYPE_FLOAT)  	&&	// For now, we only handle floats.
		    (OCV.depth()    == CV_32F))	
		{
		
			if ((OCV.channels() == 1)  	&&
				(FFTW.MatType == FFTW_MAT_TYPE_REAL))
			{
				// Copy Real to real
				
				// grossly inefficient way, but.........
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					FFTW.Data.Real[ (r*FFTW.Cols) + c ] = OCV.at<float>(r,c); 
				}

				bCopied= true;

			}			

			if ((OCV.channels() == 1)  	&&
				(FFTW.MatType == FFTW_MAT_TYPE_COMPLEX))
			{
				// Copy Real to complex
				
				// grossly inefficient way, but for now ......
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					FFTW.Data.Complex[ (r*FFTW.Cols) + c ][0] = OCV.at<float>(r,c);
					FFTW.Data.Complex[ (r*FFTW.Cols) + c ][1] = 0.0;
				}

				bCopied= true;
			}			

			if ((OCV.channels() == 2)  	&&
				(FFTW.MatType == FFTW_MAT_TYPE_REAL))
			{
				// Copy Complex to real
				
				// grossly inefficient way, but for now ......
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					FFTW.Data.Real[ (r*FFTW.Cols) + c ] = OCV.at<Vec2f>(r,c)[0]; 
				}

				bCopied= true;
			}			

			if ((OCV.channels() == 2)  	&&
				(FFTW.MatType == FFTW_MAT_TYPE_COMPLEX))
			{
				// Copy Complex to Complex
				
				// grossly inefficient way, but for now ......
				for (int r=0; r<FFTW.Rows; r++)
				for (int c=0; c<FFTW.Cols; c++)
				{
					FFTW.Data.Complex[ (r*FFTW.Cols) + c ][0] = OCV.at<Vec2f>(r,c)[0]; 
					FFTW.Data.Complex[ (r*FFTW.Cols) + c ][1] = OCV.at<Vec2f>(r,c)[1]; 
				}

				bCopied= true;
			}			

		}	
	
	}
	
	return (bCopied);
}


//==============================================================================
//					Matrix conversion routines
//==============================================================================
//

void fftwf_HCtoFC( fftwf_complex * in, fftwf_complex * out, const int rows, const int cols )
{
	// Converts a half complex array to a full complex array

	if ((rows>0)  && (cols>0))
	{
		// Make a copy if the input if source and target are the same
		 
		fftwf_complex * incopy = NULL;
			
		if (in==out)
		{
			incopy= (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * rows* cols);
			if (incopy == NULL)	return;  
			memcpy( incopy, in, sizeof(fftwf_complex) * rows * cols );
			in= incopy;
		}		

		// Create some defiens for easy reference 

		#define _in(r,c)   in[  ((r)*cols)  + (c) ]
		#define _out(r,c)  out[ ((r)*cols)  + (c) ]
		#define _hin(r,c)  in[  ((r)*hcols) + (c) ]
		#define _hout(r,c) out[ ((r)*hcols) + (c) ]

		int hcols= (cols/2)+1;		// hcols is the number of colums of actual data in the half complex array.

		// copy the first half of each rowinbound row to its destination 
		
		for (int m=rows-1; m>=0; m--)
		{
			memcpy( &_out(m,0), &_hin(m,0), hcols * sizeof(_out(m,0)) );
		}
		
		// copy the implied conjugates of the first row
		
		for (int colin=1, colout=(cols-1);  colout>=hcols; colin++, colout--)
		{
			_out(0,colout)[0]=  _out(0,colin)[0];
			_out(0,colout)[1]= -_out(0,colin)[1];
		}
		
		// copy the implied conjugates of the remaining rows
		
		for (int rowin=1, rowout=(rows-1);  rowout>0;     rowin++, rowout--)
		for (int colin=1, colout=(cols-1);  colout>=hcols; colin++, colout--)
		{
			_out(rowout,colout)[0]=  _out(rowin,colin)[0];
			_out(rowout,colout)[1]= -_out(rowin,colin)[1];
		}

		// cleanup

		if (incopy != NULL) fftwf_free(incopy); 

		#undef _in
		#undef _out
		#undef _hin
		#undef _hout
		
	}

}

void fftwf_FCtoCCS( fftwf_complex * in, float  * out, const int rows, const int cols )
{
	// Converts a full complex array to a packed CCS array
	// This CCS packed format matches OpenCV's format
		
	if ((rows>0)  && (cols>0))
	{

		// Make a copy if the input if source and target are the same
		 
		fftwf_complex * incopy = NULL;
			
		if ((void *) in == (void*) out)
		{
			incopy= (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * rows* cols);
			if (incopy == NULL)	return;  
			memcpy( incopy, in, sizeof(fftwf_complex) * rows * cols );
			in= incopy;
		}		

		// Create some defines for easy reference 
	
		#define _in(r,c)   in[  ((r)*cols) + (c) ]
		#define _out(r,c)  out[ ((r)*cols) + (c) ]

		#define _hin(r,c)  in[  ((r)*hcols) + (c) ]
		#define _hout(r,c) out[ ((r)*hcols) + (c) ]

		int hcols= (cols/2)+1;		// hcols is the number of colums of actual data in the half complex array.


		// Do the first element of each row (first column) since it is "special"
		
		_out(0,0)=  _in(0,0)[0];
		
		for (int m=1; m<rows; m++)
		{
			_out(m,0)=  _in((m+1)/2,0)[(m+1)%2];
		}

		// Do middle elements of each row
		
		for (int m=0; m<rows; m++)
		{
			// if num cols is even, don't do last row 
			memcpy( &_out(m,1), &_in(m,1)[0], ((cols-2) + (cols%2)) * sizeof(_out(m,1)) );
		}

		// If num cols is even, do last element of each row (last column) since it is "special"

		if ((cols%2) == 0)
		{
			_out(0,cols-1)=  _in(0,cols/2)[0];
			for (int m=1; m<rows; m++)
			{
				_out(m,cols-1)=  _in((m+1)/2,cols/2)[(m+1)%2];
			}
		}
		
		// cleanup
		
		if (incopy != NULL) fftwf_free(incopy); 

		#undef _in
		#undef _out
	}
}


void fftwf_HCtoCCS( fftwf_complex * in, float * out, const int rows, const int cols )
{
	// Converts a half complex array to a packed CCS array
	// This CCS packed format matches OpenCV's format
		
	if ((rows>0)  && (cols>0))
	{

		// Allocate some memory to receive ti full complex array
		 
		fftwf_complex * fcin;
			
		fcin= (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * rows* cols);
		if (fcin == NULL)	return;  

		// Create a full complex array from the half complex array
		 
		fftwf_HCtoFC( in, fcin, rows,  cols );
		
		// Convert to CCS packed format
		
		fftwf_FCtoCCS( fcin, out, rows, cols );
		
		// cleanup
		
		fftwf_free(fcin); 
	}
}

//==============================================================================
//					FFTW "plan" manager
//==============================================================================


#define USE_FFTW_PLAN_MANAGER	1

#if (USE_FFTW_PLAN_MANAGER)

#define	MAX_PLANS		50			//TBD: create an LRU eviction policy to discard old plans

typedef struct
{
	// Each "plan" will vary based on this data, so it will make up our index into the list

	int			_rows;
	int			_cols;
	void * 		_srcbuf;
	void * 		_tgtbuf;
	void *		_func;

	fftwf_plan 		plan;
	
	MultiTypeBuffer	tmpbuf1; 	// will be NULL if _srcbuf is not NULL
	MultiTypeBuffer	tmpbuf2; 	// will be NULL if _tgtbuf is not NULL	 			

	MultiTypePtr	srcbuf; 	
	MultiTypePtr	tgtbuf; 	

	uint32_t		use_cntr;	// "timestamp" this plan was last used

} fftw_plan_info_t;



class fftw_plan_manager
{
	public:
	
	fftw_plan_info_t	plist[MAX_PLANS];		// the list of available plans
	int 				plist_cnt;				// the size of the list
	
	uint32_t			use_cntr;

	fftw_plan_manager();
	~fftw_plan_manager();

	bool AllocatePlan( int index, int rows, int cols, void * srcbuf, void * tgtbuf, void * func );
	void DeallocatePlan( int index  );
	
	int  SearchList( int rows, int cols, void * srcbuf, void * tgtbuf, void * func );
	int  SearchList_LRU();
	
	fftw_plan_info_t * GetPlanInfo( int rows, int cols,  void * srcbuf, void * tgtbuf, void * func );

};
	
	
fftw_plan_manager::fftw_plan_manager()
{
	memset( plist, 0, sizeof(plist) );
	plist_cnt= 0;
	use_cntr= 0;
}

fftw_plan_manager::~fftw_plan_manager()
{
	for (int i=0; i< plist_cnt; i++)
	{
		DeallocatePlan( i );
	}
	plist_cnt= 0;
}


void fftw_plan_manager::DeallocatePlan( int index )
{
	fftwf_destroy_plan(plist[index].plan);

	plist[index].tmpbuf1.Deallocate();
	plist[index].tmpbuf2.Deallocate();

	memset( &plist[index], 0, sizeof(plist[index]) );
}


bool fftw_plan_manager::AllocatePlan( int index, int rows, int cols, void * srcbuf, void * tgtbuf, void * func )
{
	
		// Save the index values
	
	plist[index]._rows= rows;			
	plist[index]._cols= cols;
	plist[index]._srcbuf= srcbuf;
	plist[index]._tgtbuf= tgtbuf;
	plist[index]._func= func;

		// allocate buffers as needed 

	if (srcbuf == NULL)
	{
		plist[index].tmpbuf1.Allocate( sizeof(fftwf_complex) * rows * cols );  // the largest it can be
		plist[index].srcbuf.Void= plist[index].tmpbuf1.Void;
	} 
	else
	{
		plist[index].srcbuf.Void= srcbuf;
	}
	
	if (tgtbuf == NULL)
	{
		plist[index].tmpbuf2.Allocate( sizeof(fftwf_complex) * rows * cols );  // the largest it can be
		plist[index].tgtbuf.Void= plist[index].tmpbuf2.Void;
	} 
	else
	{
		plist[index].tgtbuf.Void= tgtbuf;
	}

	if ((plist[index].srcbuf.Void == NULL) || (plist[index].tgtbuf.Void == NULL))
	{
		// couldn't allocate buffers, clean up and return
		plist[index].tmpbuf1.Deallocate();
		plist[index].tmpbuf2.Deallocate();
		memset( &plist[index], 0, sizeof(plist[index]) );
		return (false);
	}

		// create the plan
		//
		// Note:  Using FFTW_MEASURE instead of FFTW_ESTIMATE produces faster DFT calculations, but there is significant overhead 
		//        in generating a plan with FFTW_MEASURE. If we get the cycle counters going then we can maybe turn it back on.
	
	if (func == (void *) fftwf_plan_dft_r2c_2d)
	{
		plist[index].plan = fftwf_plan_dft_r2c_2d( rows, cols, plist[index].srcbuf.Float, plist[index].tgtbuf.Complex, FFTW_ESTIMATE);
		//plist[index].plan = fftwf_plan_dft_r2c_2d( rows, cols, plist[index].srcbuf.Float, plist[index].tgtbuf.Complex, FFTW_MEASURE);
	}

	else 
	if (func == (void *) fftwf_plan_dft_2d)
	{
		plist[index].plan = fftwf_plan_dft_2d( rows, cols, plist[index].srcbuf.Complex, plist[index].tgtbuf.Complex, FFTW_FORWARD, FFTW_ESTIMATE);
		//plist[index].plan = fftwf_plan_dft_2d( rows, cols, plist[index].srcbuf.Complex, plist[index].tgtbuf.Complex, FFTW_FORWARD, FFTW_MEASURE);
	}

	else 
	{
		// invlaid function passed , clean up and return
		plist[index].tmpbuf1.Deallocate();
		plist[index].tmpbuf2.Deallocate();
		memset( &plist[index], 0, sizeof(plist[index]) );
		return (false);
	}

		// All done.
		
	return (true);
}



int fftw_plan_manager::SearchList( int rows, int cols,  void * srcbuf, void * tgtbuf, void * func )
{

	for (int i= 0; i<plist_cnt; i++)
	{
		if ((plist[i]._rows   == rows)   &&
			(plist[i]._cols   == cols)   &&
			(plist[i]._srcbuf == srcbuf) &&
			(plist[i]._tgtbuf == tgtbuf) &&
			(plist[i]._func   == func))
		{
			return (i);		// found it
		}
	}

	return (-1);   // did not find it
}


int fftw_plan_manager::SearchList_LRU()
{
	int 		LRU_index= -1;
	uint32_t	LRU;
	
	if (plist_cnt > 0)
	{
		LRU_index= 0;
		LRU= plist[0].use_cntr;
		
		for (int i= 1; i<plist_cnt; i++)
		{
			if (plist[i].use_cntr < LRU)
			{  
				LRU_index= i;
				LRU= plist[i].use_cntr;
			}
		}
	}
	
	return (LRU_index);  
}

 
fftw_plan_info_t * fftw_plan_manager::GetPlanInfo( int rows, int cols,  void * srcbuf, void * tgtbuf, void * func )
{

	//printf( "\r\nSearch list for (%d ,%d, %p, %p, %p )", rows, cols,  srcbuf, tgtbuf, func ); 

		// Search for and existing matching entry in our list
		
	int index= SearchList( rows, cols,  srcbuf, tgtbuf, func );
	
	if (index >= 0)
	{
		//printf( "\r\nFound plan (%d ,%d, %p, %p, %p ) at index %d ", rows, cols,  srcbuf, tgtbuf, func, index ); 
		plist[index].use_cntr= ++use_cntr;
		return (&plist[index]);		// found it
	}

		// Not found. Get an open slot to add it

	if (plist_cnt < MAX_PLANS)
	{
		index= plist_cnt;
	}
	else
	{
		index=  SearchList_LRU();
		if (index != -1)
		{
			//printf( "\r\nDeallocating LRU plan at index %d ", index );
			DeallocatePlan( index );
		}
		else
		{
			printf( "\r\nError: MAX_PLANS reached & LRU not found (%d) ", MAX_PLANS );
			return (NULL);
		} 
	}
	

		// Allocte the new plan

	if (AllocatePlan( index, rows, cols, srcbuf, tgtbuf, func ))
	{
		//printf( "\r\nAdding plan (%d ,%d, %p, %p, %p ) at index %d ", rows, cols,  srcbuf, tgtbuf, func, index ); 
		if (index==plist_cnt)
		{	
			plist_cnt++;
		}
		plist[index].use_cntr= ++use_cntr;
		return (&plist[index]);		
	}
	else
	{
		printf( "\r\nError: Could not Allocate plan (%d ,%d, %p, %p, %p ) ", rows, cols,  srcbuf, tgtbuf, func ); 
	}
	
	return (NULL);
}

#endif


//==============================================================================
//					OpenCV replacement routines
//==============================================================================

#if (USE_FFTW_PLAN_MANAGER)
fftw_plan_manager PlanManager;
#endif


bool fftw_CanProcessOCVdft( cv::InputArray _src0, cv::OutputArray _dst, int flags, int nonzero_rows )
{
	Mat src = _src0.getMat();
	
	if ((src.rows >= 4)				&&		// these conditions guarantee it is a normal 2d nontrivial contiguous matrix of type float
		(src.cols >= 4)				&&
		(src.dims == 2)				&&
		(src.depth() == CV_32F)  	&&
		(!src.empty())				&&
		(src.isContinuous())		&&
		(!src.isSubmatrix()))
	{
	
		if ((flags == 0) || (flags ==DFT_COMPLEX_OUTPUT))  
		{
			if (nonzero_rows == 0)
			{
				if ((src.channels() == 1) ||		// it is a 2 dimensional array of real numbers
				    (src.channels() == 2))			// it is a 2 dimensional array of complex
				{
					return (true);					
				}
			}
		}
	}
	
	return (false);
}

bool fftw_ProcessOCVdft( cv::InputArray _src0, cv::OutputArray _dst, int flags, int nonzero_rows )
{
	bool bHandled= false;

	if (fftw_CanProcessOCVdft( _src0, _dst, flags, nonzero_rows ))
	{
		Mat src = _src0.getMat();
				
		//DumpMatix_OCV( src, "src",  NULL );	
	
		//-------------------------------------------------------------------------------------------
		// Handle real input, CCS complex output
		//
		//		cv::dft( cvmat_in, cvmat_out, 0, 0 );
		// 
		//		p = fftwf_plan_dft_r2c_2d( Rows, Cols, fftwf_in_r, fftwf_out, FFTW_ESTIMATE);
		//		fftwf_execute(p); 
		//		fftwf_HCtoCCS( fftwf_out, fftwf_out, Rows, Cols );
		//-------------------------------------------------------------------------------------------
		
		if ((flags == 0) && (src.channels() == 1))  
		{
		  	_dst.create( src.size(), CV_32F );
		  	Mat dst = _dst.getMat();
	
			if ((dst.size() == src.size()) && dst.isContinuous())
			{

				#if (USE_FFTW_PLAN_MANAGER) 				

					// Note: fftw has issues directly reading/writing the opencv matrix when the "plan" is stored and reused.  The 
					//       problem seems to be that the buffers get destroyed by opencv and fftw seg faults , even though we don't
					//       use the plan that accesses those buffers. So we pass  NULL here to the plan manager for both src and tgt buffers.
					//		 That way the plan manager will create private buffers for the calculations. This incurs extra time copying to/from 
					//		 the buffers, but is still worth it as the additional overhead of the memcpy is small compared to the overall savings in 
					//       in using fftw over OpenCV,
					//
					//		 TBD: fftw supplies "New-array Execute Functions" which allow you to run a plan on different src/tgt buffers.
					//            Instead of calling "fftwf_execute(plan)" you call "fftw_execute_dft(plan,inbuf,outbuf)".  If it works, then
					//            it would elimanate the need for the memcpy prior to executing the plan, but not after.  I have not had time 
					//			  to test this function, so I do not implement it here. Proceed with caution if you do. -BFM

					fftw_plan_info_t * ppi = PlanManager.GetPlanInfo( src.rows, src.cols, NULL, NULL, (void *) fftwf_plan_dft_r2c_2d );				
				
					if (ppi != NULL)
					{
						memcpy( ppi->srcbuf.Real, src.data, src.rows * src.cols * sizeof(float));
						fftwf_execute(ppi->plan); 
						fftwf_HCtoCCS( ppi->tgtbuf.Complex, (float *) dst.data, dst.rows, dst.cols );
						bHandled= true;
					}

				#else
				
					fftwf_complex *  tmp = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * src.rows * src.cols);
					fftwf_plan p = fftwf_plan_dft_r2c_2d( src.rows, src.cols, (float *) src.data, (fftwf_complex *) tmp, FFTW_ESTIMATE);
					fftwf_execute(p); 
					fftwf_HCtoCCS( (fftwf_complex *) tmp, (float *) dst.data, dst.rows, dst.cols );
					fftwf_destroy_plan(p);
					fftwf_free(tmp); 
					bHandled= true;

				#endif
			}
				
			if (!bHandled)
			{
				dst.release();		// cleanup on failure ??   May noyt be necessary
			}
		}
 		
		//-------------------------------------------------------------------------------------------
		// Handle real input, full complex output
		//
		//		cv::dft( cvmat_in, cvmat_out, DFT_COMPLEX_OUTPUT, 0 );
		// 
		//		p = fftwf_plan_dft_r2c_2d( Rows, Cols, fftwf_in_r, fftwf_out, FFTW_ESTIMATE);
		//		fftwf_execute(p); 
		//		fftwf_HCtoFC( fftwf_out, fftwf_out, Rows, Cols );
		//-------------------------------------------------------------------------------------------

		else if ((flags == DFT_COMPLEX_OUTPUT) && (src.channels() == 1))  
		{
		  	_dst.create( src.size(), CV_MAKETYPE(CV_32F, 2) );
		  	Mat dst = _dst.getMat();

			if ((dst.size() == src.size()) && dst.isContinuous())
			{
				#if (USE_FFTW_PLAN_MANAGER) 				

					// See note above:
					fftw_plan_info_t * ppi = PlanManager.GetPlanInfo( src.rows, src.cols, NULL, NULL, (void *) fftwf_plan_dft_r2c_2d );				
				
					if (ppi != NULL)
					{
						memcpy( ppi->srcbuf.Real, src.data, src.rows * src.cols * sizeof(float));
						fftwf_execute(ppi->plan); 
						fftwf_HCtoFC( ppi->tgtbuf.Complex, (fftwf_complex *) dst.data, dst.rows, dst.cols );
						bHandled= true;
					}

				#else

					fftwf_plan p = fftwf_plan_dft_r2c_2d( src.rows, src.cols, (float *) src.data, (fftwf_complex *) dst.data, FFTW_ESTIMATE);
					fftwf_execute(p); 
					fftwf_HCtoFC( (fftwf_complex *) dst.data, (fftwf_complex *) dst.data, dst.rows, dst.cols );
					fftwf_destroy_plan(p);
					bHandled= true;
				
				#endif
				
			}

			if (!bHandled)
			{
				dst.release();
			}
		}

		//-------------------------------------------------------------------------------------------
		// Handle complex input, CCS complex output
		//
		//		cv::dft( cvmat_in, cvmat_out, 0, 0 );
		//
		//		p = fftwf_plan_dft_2d( Rows, Cols, fftwf_in, fftwf_out, FFTW_FORWARD, FFTW_ESTIMATE);
		//		fftwf_execute(p); 
		//		fftwf_FCtoCCS( fftwf_complex_buf_in, fftwf_real_buf_out, Rows, Cols );  // Complex bux 
		//
		//-------------------------------------------------------------------------------------------

		else if ((flags == 0) && (src.channels() == 2))  
		{
		  	_dst.create( src.size(), CV_32F );
		  	Mat dst = _dst.getMat();

			if ((dst.size() == src.size()) && dst.isContinuous())
			{
				#if (USE_FFTW_PLAN_MANAGER) 				

					// See note above:
					fftw_plan_info_t * ppi = PlanManager.GetPlanInfo( src.rows, src.cols, NULL, NULL, (void *) fftwf_plan_dft_2d );				
				
					if (ppi != NULL)
					{
						memcpy( ppi->srcbuf.Complex, src.data, src.rows * src.cols * sizeof(fftwf_complex));
						fftwf_execute(ppi->plan); 
						fftwf_FCtoCCS( ppi->tgtbuf.Complex, (float *) dst.data, dst.rows, dst.cols );
						bHandled= true;
					}

				#else

					fftwf_plan p = fftwf_plan_dft_2d( src.rows, src.cols, (fftwf_complex *) src.data, (fftwf_complex *) dst.data, FFTW_FORWARD, FFTW_ESTIMATE);
					fftwf_execute(p); 
					fftwf_FCtoCCS( (fftwf_complex *) dst.data, (float *) dst.data, dst.rows, dst.cols );
					fftwf_destroy_plan(p);
					bHandled= true;
				
				#endif
			}

			if (!bHandled)
			{
				dst.release();
			}
		}

		//-------------------------------------------------------------------------------------------
		// Handle complex input, full complex output
		//
		//		cv::dft( cvmat_in, cvmat_out, DFT_COMPLEX_OUTPUT, 0 );
		//
		//		p = fftwf_plan_dft_2d( Rows, Cols, fftwf_in, fftwf_out, FFTW_FORWARD, FFTW_ESTIMATE);
		//		fftwf_execute(p); 
		//-------------------------------------------------------------------------------------------


		else if ((flags == DFT_COMPLEX_OUTPUT) && (src.channels() == 2))  
		{
		  	_dst.create( src.size(), CV_MAKETYPE(CV_32F, 2) );
		  	Mat dst = _dst.getMat();
	
			if ((dst.size() == src.size()) && dst.isContinuous())
			{
				#if (USE_FFTW_PLAN_MANAGER) 				

					// See note above:
					fftw_plan_info_t * ppi = PlanManager.GetPlanInfo( src.rows, src.cols, NULL, NULL, (void *) fftwf_plan_dft_2d );				
				
					if (ppi != NULL)
					{
						memcpy( ppi->srcbuf.Real, src.data, src.rows * src.cols * sizeof(fftwf_complex));
						fftwf_execute(ppi->plan); 
						memcpy( (fftwf_complex *) dst.data, ppi->tgtbuf.Complex, dst.rows * dst.cols * sizeof(fftwf_complex));
						bHandled= true;
					}

				#else

					fftwf_plan p = fftwf_plan_dft_2d( src.rows, src.cols, (fftwf_complex *) src.data, (fftwf_complex *) dst.data, FFTW_FORWARD, FFTW_ESTIMATE);
					fftwf_execute(p); 
					fftwf_destroy_plan(p);
					bHandled= true;
				
				#endif
			}

			if (!bHandled)
			{
				dst.release();
			}
		}
		
	}
	
	//printf( " bhandled = %c", (bHandled ? 'T':'F' ));
	

	return (bHandled);
}

//============================================================================================
//			On/off switch
//============================================================================================


static bool fftw_enabled= true;

bool is_enabled_fftw_dft()
{
	return (fftw_enabled);
}

bool enable_fftw_dft( bool bEnable )
{
	bool was_enabled= fftw_enabled;
	fftw_enabled= bEnable;
	return (was_enabled);
}


#endif
