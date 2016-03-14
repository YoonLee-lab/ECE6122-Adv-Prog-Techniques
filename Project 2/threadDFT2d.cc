// Threaded two-dimensional Discrete FFT transform
// YOUR NAME HERE
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>
#include <pthread.h>

#include "Complex.h"
#include "InputImage.h"


using namespace std;

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.
Complex* ImageData;
int ImageWidth;
int ImageHeight;
int N;
int nThreads = 16;
int cnt;
int* judge = new int[nThreads];
//bool localSense[16];
//bool globalSense;

pthread_mutex_t cntMutex;

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = N; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

// GRAD Students implement the following 2 functions.
// Undergrads can use the built-in barriers in pthreads.

int JudgeSum()
{
  int sum = 0;
  for( int i = 0; i < nThreads; i++)
    sum += judge[i];
  return sum;
}

void SetPosition(int i)
{
  judge[i] = 1;
}

void MyBarrier_Init()
{
  for( int i = 0; i < nThreads; i++)
    judge[i] = 0;
}

void MyBarrier()
{
  while(JudgeSum() != nThreads ){}
}

void Transpose(Complex* h, int width, int height)
{
  for( int row = 0; row < height; ++row )
    for( int col = 0; col < width; ++col )
      if( col > row)
      {
        Complex temp = h[row * width + col];
        h[row * width + col] = h[col * width + row];
        h[col * width + row] = temp;
      }
}

void Transform1D(Complex* h, int N)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  
  // Reorder the initial h array.
  
  Complex* reorder = new Complex[N]();
  for( int i = 0; i < N; ++i ) reorder[ReverseBits(i)] = h[i];
  for( int i = 0; i < N; ++i ) h[i] = reorder[i];
  delete[] reorder;
  
  // Precompute the Wn values.
  
  Complex* W = new Complex[N]();
  
  for( int i = 0; i < N/2; ++i )
  {
    W[i] = Complex(cos(2*M_PI*i/N),-sin(2*M_PI*i/N));
    W[i+N/2] = Complex(-1) * W[i];
  }
  
  // Do the transformation
  int subLen = 2;
  int subNum = N/2;
  while(subLen <= N)
  {
    Complex* temp = new Complex[N]();
    for( int i = 0; i < subNum; ++i )
      for( int j = 0; j < subLen; ++j )
        temp[i*subLen + j] = temp[i*subLen + j] + h[i*subLen + j%(subLen/2)] + h[i*subLen + j%(subLen/2) + subLen/2] * W[N*j/subLen];
    for( int j = 0; j < N; ++j )
      h[j] = temp[j];
    subLen *= 2;
    subNum /= 2;
    delete[] temp;
  }
  delete[] W;
}

void Transform1DInverse(Complex* H, int N)
{

  Complex* reorder = new Complex[N]();
  for( int i = 0; i < N; ++i ) reorder[ReverseBits(i)] = H[i];
  for( int i = 0; i < N; ++i ) H[i] = reorder[i];
  delete[] reorder;
  
  // Precompute the Wn values.
  
  Complex* W = new Complex[N]();
  
  for( int i = 0; i < N/2; ++i )
  {
    W[i] = Complex(cos(2*M_PI*i/N),sin(2*M_PI*i/N));
    W[i+N/2] = Complex(-1) * W[i];
  }
  
  // Do the transformation
  int subLen = 2;
  int subNum = N/2;
  while(subLen <= N)
  {
    Complex* temp = new Complex[N]();
    for( int i = 0; i < subNum; ++i )
      for( int j = 0; j < subLen; ++j )
        temp[i*subLen + j] = temp[i*subLen + j] + H[i*subLen + j%(subLen/2)] + H[i*subLen + j%(subLen/2) + subLen/2] * W[N*j/subLen];
    for( int j = 0; j < N; ++j )
      H[j] = temp[j];
    subLen *= 2;
    subNum /= 2;
    delete[] temp;
  }
  for( int i = 0; i < N; ++i )
  {
    H[i] =  H[i]*Complex(1.0/N);
    if( H[i].Mag().real < 1e-10 ) H[i] = Complex(0);
  }
      
  delete[] W;

}

void* Transform2DThread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active cnt and signal main if all complete
  unsigned long myId = (unsigned long) v;
  
  int rowsPerThread = ImageHeight/nThreads;
  int startingRow = myId * rowsPerThread;

  for( int r = 0; r < rowsPerThread; ++r )
  {
    Transform1D(ImageData + (startingRow + r) * ImageWidth, ImageWidth);
  }

  SetPosition(myId);
  return 0;
}

void* Transform2DInverseThread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active cnt and signal main if all complete
  unsigned long myId = (unsigned long) v;
  
  int rowsPerThread = ImageHeight/nThreads;
  int startingRow = myId * rowsPerThread;

  for( int r = 0; r < rowsPerThread; ++r )
  {
    Transform1DInverse(ImageData + (startingRow + r) * ImageWidth, ImageWidth);
  }

  SetPosition(myId);
  return 0;
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.

  // Create the helper object for reading the image
  InputImage image(inputFN);
  // Create the global pointer to the image array data
  ImageData = image.GetImageData();
  ImageWidth = image.GetWidth();
  ImageHeight = image.GetHeight();
  N = ImageWidth;

  pthread_mutex_init(&cntMutex,0);

  // Create 16 threads
  for( int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt,0,Transform2DThread, (void*)i);
  }

  // Wait for all threads complete

  MyBarrier();

  Transpose(ImageData, ImageWidth, ImageHeight);

  MyBarrier_Init();
  
  for( int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt,0,Transform2DThread, (void*)i);
  }

  // Wait for all threads complete

  MyBarrier();

  Transpose(ImageData, ImageWidth, ImageHeight);

  // Write the transformed data
  string fn1("Tower-DFT2D.txt");
  image.SaveImageData(fn1.c_str(), ImageData, ImageWidth, ImageHeight);

  MyBarrier_Init();

  // Do the 2D transform inverse here!!!

  // Create 16 threads
  for( int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt,0,Transform2DInverseThread, (void*)i);
  }

  // Wait for all threads complete

  MyBarrier();

  Transpose(ImageData, ImageWidth, ImageHeight);

  MyBarrier_Init();
  
  for( int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt,0,Transform2DInverseThread, (void*)i);
  }

  // Wait for all threads complete

  MyBarrier();

  Transpose(ImageData, ImageWidth, ImageHeight);

  // Write the transformed data
  string fn2("MyAfterInverse.txt");
  image.SaveImageData(fn2.c_str(), ImageData, ImageWidth, ImageHeight);

}


int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
