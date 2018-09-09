#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <string>

// used for encrypting plain text, primarily passwords
class Encryptor {
  public:
  Encryptor() { }
  virtual ~Encryptor() { }
  // encryption based on RSA256 algorithm (?)
  std::string& encrypt(const std::string& inString);
  // adding salt to encrypted passwords
  std::string& salt(const std::string& inString);
};

#endif	// ENCRYPTOR_H
