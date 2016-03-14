// RSA Assignment for ECE4122/6122 Fall 2015

#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "RSA_Algorithm.h"

using namespace std;

// Implement the RSA_Algorithm methods here

// Constructor
RSA_Algorithm::RSA_Algorithm()
  : rng(gmp_randinit_default)
{
  // get a random seed for the random number generator
  int dr = open("/dev/random", O_RDONLY);
  if (dr < 0)
    {
      cout << "Can't open /dev/random, exiting" << endl;
      exit(0);
    }
  unsigned long drValue;
  read(dr, (char*)&drValue, sizeof(drValue));
  //cout << "drValue " << drValue << endl;
  rng.seed(drValue);
// No need to init n, d, or e.
}

// Fill in the remainder of the RSA_Algorithm methods
void RSA_Algorithm::GenerateRandomKeyPair(size_t sz)
{
  mpz_class p = rng.get_z_bits(sz);
  while (true){
    int ret = mpz_probab_prime_p(p.get_mpz_t(),100);
    if (ret > 0) break;
    else p = rng.get_z_bits(sz);
  } 
  
  mpz_class q = rng.get_z_bits(sz);
  while (true){
    int ret = mpz_probab_prime_p(q.get_mpz_t(),100);
    if (ret > 0) break;
    else q = rng.get_z_bits(sz);
  } 
  
  n = p*q;
  mpz_class phi_n = (p-1)*(q-1);

  while(true){
    d = rng.get_z_bits(2*sz);
    if (d >= phi_n) continue;
    mpz_class rop;
    mpz_gcd(rop.get_mpz_t(), d.get_mpz_t(), phi_n.get_mpz_t());
    if (rop == 1) break;
  }
  
  mpz_invert(e.get_mpz_t(), d.get_mpz_t(), phi_n.get_mpz_t());
  PrintNDE();
}

mpz_class RSA_Algorithm::Encrypt(mpz_class M)
{
  mpz_class C;
  mpz_powm(C.get_mpz_t(), M.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
  PrintC(C);
  return C;
}

mpz_class RSA_Algorithm::Decrypt(mpz_class C)
{
  mpz_class D;
  mpz_powm(D.get_mpz_t(), C.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
  return D;
}


void RSA_Algorithm::PrintND()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << endl;
}

void RSA_Algorithm::PrintNE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " e " << e << endl;
}

void RSA_Algorithm::PrintNDE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << " e " << e << endl;
}

void RSA_Algorithm::PrintM(mpz_class M)
{ // Do not change this, right format for the grading script
  cout << "M " << M << endl;
}

void RSA_Algorithm::PrintC(mpz_class C)
{ // Do not change this, right format for the grading script
  cout << "C " << C << endl;
}




