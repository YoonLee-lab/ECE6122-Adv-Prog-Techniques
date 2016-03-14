// ECE4122/6122 RSA Encryption/Decryption assignment
// Fall Semester 2015

#include <iostream>
#include "RSA_Algorithm.h"

using namespace std;

int main()
{
  // Instantiate the one and only RSA_Algorithm object
  RSA_Algorithm RSA;
  
  // Loop from sz = 32 to 1024 inclusive
  // for each size choose 10 different key pairs
  // For each key pair choose 10 differnt plaintext 
  // messages making sure it is smaller than n.
  // If not smaller then n then choose another
  // For eacm message encrypt it using the public key (n,e).
  // After encryption, decrypt the ciphertext using the private
  // key (n,d) and verify it matches the original message.

  // your code here
  size_t sz = 32;
  for (sz = 32; sz <= 1024; sz *= 2)
  {
    for (int i = 0; i < 10; ++i)
    {
      RSA.GenerateRandomKeyPair(sz);

      for (int j = 0; j < 10; ++j)
      {
        gmp_randclass r(gmp_randinit_default);
        r.seed(i*j+j);
        mpz_class M = r.get_z_bits(sz*2);
        while(true){
          if (M > RSA.n) M = r.get_z_bits(sz*2);
          else break;
        }
        RSA.PrintM(M);
        
        mpz_class C = RSA.Encrypt(M);
        mpz_class D = RSA.Decrypt(C);
        if (M == D) cout << "true" << endl;
        else cout << "false" << endl;
      }
    }
  }
}
  
