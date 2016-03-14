// Distributed two-dimensional Discrete FFT transform
// Yujia Liu
// ECE8893 Project 1


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;

void Transform1D(Complex* h, int w, Complex* H)
{
  for ( int k = 0; k < w; ++k )
    for ( int n = 0; n < w; ++n )
      H[k] = H[k] + Complex(cos(2*M_PI * n * k / w), -sin(2*M_PI * n * k / w)) * h[n];
}

void Transform1DInverse(Complex* H, int w, Complex* h)
{
  for ( int n = 0; n < w; ++n )
    for ( int k = 0; k < w; ++k )
      h[n] = h[n] + Complex(1.0/w) * Complex(cos(2*M_PI * n * k / w), sin(2*M_PI * n * k / w)) * H[k];
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

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  // 4) Obtain a pointer to the Complex 1d array of input data
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  // 6) Send the resultant transformed values to the appropriate
  //    other processors for the next phase.
  // 6a) To send and receive columns, you might need a separate
  //     Complex array of the correct size.
  // 7) Receive messages from other processes to collect your columns
  // 8) When all columns received, do the 1D transforms on the columns
  // 9) Send final answers to CPU 0 (unless you are CPU 0)
  //   9a) If you are CPU 0, collect all values from other processors
  //       and print out with SaveImageData().
  
  InputImage image(inputFN);  // Create the helper object for reading the image

  int width = image.GetWidth();
  int height = image.GetHeight();
  
  int nCPUs, rank;

  MPI_Comm_size(MPI_COMM_WORLD, &nCPUs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int rowsPerCPU = height/nCPUs;
  Complex* I;
  if( rank == 0)
    I = image.GetImageData();

  Complex* h = new Complex[width*rowsPerCPU]();
  Complex* H = new Complex[width*rowsPerCPU]();

  MPI_Status* pStat = new MPI_Status[nCPUs];
  MPI_Request* pReq = new MPI_Request[nCPUs];

  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Isend(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pReq[cpu]);
      cout << "CPU " << rank << " queued send" << endl;
    }
  }
    
  int startRow = rank * rowsPerCPU;
  MPI_Recv(h, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pStat[rank]);
  cout << "CPU " << rank << " queued recv" << endl; 

  for ( int row = 0; row < rowsPerCPU; ++row )
  {
    Complex* currenth = h + row * width;
    Complex* currentH = H + row * width;
    Transform1D(currenth, width, currentH);
  }

  MPI_Isend(H, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pReq[rank]);
  cout << "CPU " << rank << " queued send" << endl;

  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Recv(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pStat[cpu]);
      cout << "CPU " << cpu << " queued recv" << endl; 
    }
    cout << "CPU " << rank << " queued all recv" << endl;
  
    string fn1("myafter1d.txt");
    image.SaveImageData(fn1.c_str(), I, width, height); 
    Transpose(I, width, height);
  }

  int temp = height;
  height = width;
  width = temp;

  rowsPerCPU = height/nCPUs;
  
  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Isend(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pReq[cpu]);
      cout << "CPU " << rank << " queued send" << endl;
    }
  }

  Complex* hh = new Complex[rowsPerCPU * width];
  Complex* HH = new Complex[rowsPerCPU * width];

  startRow = rank * rowsPerCPU;
  MPI_Recv(hh, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pStat[rank]);
  cout << "CPU " << rank << " queued recv" << endl; 

  for ( int row = 0; row < rowsPerCPU; ++row )
  {
    Complex* currenth = hh + row * width;
    Complex* currentH = HH + row * width;
    Transform1D(currenth, width, currentH);
  }

  MPI_Isend(HH, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pReq[rank]);
  cout << "CPU " << rank << " queued send" << endl;

  if( rank == 0 )
  { 
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Recv(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pStat[cpu]);
      cout << "CPU " << cpu << " queued recv" << endl; 
    }
    cout << "CPU " << rank << " queued all recv" << endl;
  
    Transpose(I, width, height);

    string fn2("myafter2d.txt");
    image.SaveImageData(fn2.c_str(), I, width, height);
  }

  temp = height;
  height = width;
  width = temp;

  rowsPerCPU = height/nCPUs;

/* Transform Inverse */

  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Isend(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pReq[cpu]);
      cout << "CPU " << rank << " queued send" << endl;
    }
  }

  Complex* hhh = new Complex[rowsPerCPU * width];
  Complex* HHH = new Complex[rowsPerCPU * width];

  startRow = rank * rowsPerCPU;
  MPI_Recv(HHH, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pStat[rank]);
  cout << "CPU " << rank << " queued recv" << endl; 

  for ( int row = 0; row < rowsPerCPU; ++row )
  {
    Complex* currentH = HHH + row * width;
    Complex* currenth = hhh + row * width;
    Transform1DInverse(currentH, width, currenth);
  }

  MPI_Isend(hhh, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pReq[rank]);
  cout << "CPU " << rank << " queued send" << endl;

  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Recv(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pStat[cpu]);
      cout << "CPU " << cpu << " queued recv" << endl; 
    }
    cout << "CPU " << rank << " queued all recv" << endl;
  
    Transpose(I, width, height);
  }

  temp = height;
  height = width;
  width = temp;

  rowsPerCPU = height/nCPUs;

  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Isend(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pReq[cpu]);
      cout << "CPU " << rank << " queued send" << endl;
    }
  }

  Complex* hhhh = new Complex[rowsPerCPU * width];
  Complex* HHHH = new Complex[rowsPerCPU * width];

  MPI_Recv(HHHH, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pStat[rank]);
  cout << "CPU " << rank << " queued recv" << endl; 

  for ( int row = 0; row < rowsPerCPU; ++row )
  {
    Complex* currentH = HHHH + row * width;
    Complex* currenth = hhhh + row * width;
    Transform1DInverse(currentH, width, currenth);
  }

  MPI_Isend(hhhh, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, 0, startRow, MPI_COMM_WORLD, &pReq[rank]);
  cout << "CPU " << rank << " queued send" << endl;
  
  if( rank == 0 )
  {
    for( int cpu = 0; cpu < nCPUs; ++cpu)
    {
      int startRow = cpu * rowsPerCPU;
      MPI_Recv(I + startRow * width, rowsPerCPU * width * sizeof(Complex), MPI_CHAR, cpu, startRow, MPI_COMM_WORLD, &pStat[cpu]);
      cout << "CPU " << cpu << " queued recv" << endl; 
    }
    cout << "CPU " << rank << " queued all recv" << endl;
   
    Transpose(I, width, height);
    temp = height;
    height = width;
    width = temp;

    string fn3("myafter2dInverse.txt");
    image.SaveImageData(fn3.c_str(), I, width, height);
  }

  delete [] H;
  delete [] h;
  delete [] HH;
  delete [] hh;
  delete [] HHH;
  delete [] hhh;
  delete [] HHHH;
  delete [] hhhh;
  delete [] pStat;
  delete [] pReq;

}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  MPI_Init(&argc, &argv);

  Transform2D(fn.c_str()); // Perform the transform.
    
  // Finalize MPI here
  MPI_Finalize();
}  
  

  
