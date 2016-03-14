
#include <iostream>
#include <stack>
#include <cuda_runtime_api.h>
#include <stdio.h>
#include "Complex.cu"
#include <GL/freeglut.h>

#define WINDOW_DIM 512
#define NUM_THREADS 32 

using namespace std;

Complex minC(-2.0, -1.2);
Complex maxC(1.0, 1.8);
Complex* dev_minC;
Complex* dev_maxC;
Complex* dev_c; 
int* dev_iteration; 

const int maxIt = 2000;

Complex* c = new Complex[WINDOW_DIM * WINDOW_DIM];
int iteration[WINDOW_DIM * WINDOW_DIM];  

struct Position 
{
  Position() : x(0), y(0) {}
  float x, y; 
};
Position start, end; 

struct Memory 
{
  Memory(float a, float b, float c, float d)
    : minC_r(a), minC_i(b), maxC_r(c), maxC_i(d) {}
  float minC_r, minC_i, maxC_r, maxC_i;
};
stack<Memory> memory_stack;

class RGB
{
public:
  RGB()
    : r(0), g(0), b(0) {}
  RGB(double r0, double g0, double b0)
    : r(r0), g(g0), b(b0) {}
public:
  double r;
  double g;
  double b;
};

RGB* colors = NULL;

void InitializeColors()
{
  colors = new RGB[maxIt + 1];
  for (int i = 0; i < maxIt; ++i)
    {
      if (i < 5)
        { 
          colors[i] = RGB(1, 1, 1);
        }
      else
        {
          colors[i] = RGB(drand48(), drand48(), drand48());
        }
    }
  colors[maxIt] = RGB(); 
}

__global__ void ComputeIteration(Complex* dev_minC, Complex* dev_maxC, int* dev_iteration, Complex* dev_c) 
{
  int id = threadIdx.x + blockIdx.x * blockDim.x; 
  int i = id / WINDOW_DIM;   
  int j = id % WINDOW_DIM;
  
  double dX = dev_maxC->r - dev_minC->r;
  double dY = dev_maxC->i - dev_minC->i;
  double nX = (double) i / WINDOW_DIM;
  double nY = (double) j / WINDOW_DIM;
  
  dev_c[id].r = dev_minC->r + nX * dX;
  dev_c[id].i = dev_minC->i + nY * dY;

  Complex Z;
  Z.r = dev_c[id].r;
  Z.i = dev_c[id].i;
  dev_iteration[id] = 0;
      
  while(dev_iteration[id] < 2000 && Z.magnitude2() < 4.0)
  {
    dev_iteration[id]++;
    Z = (Z*Z) + dev_c[id];
  }
}

void ComputeMBSet() 
{
  cudaMalloc((void**)&dev_iteration, WINDOW_DIM * WINDOW_DIM * sizeof(int));
  cudaMalloc((void**)&dev_minC, sizeof(Complex));
  cudaMalloc((void**)&dev_maxC, sizeof(Complex));
  cudaMalloc((void**)&dev_c, WINDOW_DIM * WINDOW_DIM * sizeof(Complex));
  cudaMemcpy(dev_minC, &minC, sizeof(Complex), cudaMemcpyHostToDevice);
  cudaMemcpy(dev_maxC, &maxC, sizeof(Complex), cudaMemcpyHostToDevice);
  cudaMemcpy(dev_iteration, iteration, WINDOW_DIM * WINDOW_DIM * sizeof(int), cudaMemcpyHostToDevice);  
  cudaMemcpy(dev_c, c, WINDOW_DIM * WINDOW_DIM * sizeof(Complex), cudaMemcpyHostToDevice);  
  
  ComputeIteration<<< WINDOW_DIM * WINDOW_DIM / NUM_THREADS, NUM_THREADS >>>(dev_minC, dev_maxC, dev_iteration, dev_c);

  cudaMemcpy(iteration, dev_iteration, WINDOW_DIM * WINDOW_DIM * sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(c, dev_c, WINDOW_DIM * WINDOW_DIM * sizeof(Complex), cudaMemcpyDeviceToHost);  
}

void display(void)  
{
  glClearColor(0.0, 0.0, 0.0, 0.0); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  
  glBegin(GL_POINTS);
  for(int i = 0; i < WINDOW_DIM; i++)
  {
    for(int j = 0; j < WINDOW_DIM; j++)
    {
      glColor3f(colors[iteration[i*WINDOW_DIM + j]].r, colors[iteration[i*WINDOW_DIM + j]].g, colors[iteration[i*WINDOW_DIM + j]].b);
      glVertex2d(i, j);
    }
  }
  glEnd();

  glutSwapBuffers();
}

void keyboard (unsigned char key, int x, int y) 
{
  if(key == 'q') 
  { 
    exit(0);  
  }

  if(key == 'b')
  {
    if(memory_stack.size() > 0)
    {
      Memory temp = memory_stack.top();  
      memory_stack.pop();          
      minC.r = temp.minC_r;
      minC.i = temp.minC_i;
      maxC.r = temp.maxC_r;
      maxC.i = temp.maxC_i;
      ComputeMBSet();   
      glutPostRedisplay();
    }
    else
      cout<<"You cannot go back any more!"<<endl;
  }
}

void mouse(int button, int state, int x, int y) 
{
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
  {
    start.x = x;
    start.y = y;
  }

  if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)                      
  {
    memory_stack.push(Memory(minC.r, minC.i, maxC.r, maxC.i));   
    
    double dx = abs(x - start.x);
    double dy = abs(y - start.y);
    double ds = dx > dy ? dy : dx;

    end.x = x > start.x ? start.x + ds : start.x - ds;
    end.y = y > start.y ? start.y + ds : start.y - ds;

    int min_i = min(start.x, end.x), min_j = min(start.y, end.y);
    minC.r = c[min_i*WINDOW_DIM + min_j].r;
    minC.i = c[min_i*WINDOW_DIM + min_j].i;
    
    int max_i = max(start.x, end.x), max_j = max(start.y, end.y);
    maxC.r = c[max_i*WINDOW_DIM + max_j].r;
    maxC.i = c[max_i*WINDOW_DIM + max_j].i;

    ComputeMBSet();  
    glutPostRedisplay(); 
  }
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(WINDOW_DIM, WINDOW_DIM);
  glutCreateWindow("MBSet");

  glViewport(0, 0, WINDOW_DIM, WINDOW_DIM);                                            
  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity();

  gluOrtho2D(0, WINDOW_DIM, WINDOW_DIM, 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();  
  
  InitializeColors();
  ComputeMBSet();
  
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc (keyboard);
  glutMouseFunc(mouse);

  glutMainLoop();
  return 0;
}
