#ifndef COMMANDS_H
#define COMMANDS_H

/*
 * Define command codes for exchanging instructions between
 * the server and clients.
 */
enum Commands {
  cSignup,	// the sequence of signup strings will follow
  cLogin,	// the sequence of login strings will follow
  cFindUser,	// username will follow
  cTextUser,	// username and text will follow
  cCallUser,	// username will follow
  cLogout,	// user logged out
  cQuit,	// user quit the program
  cUsername,	// the following string will be the username
  cPassword,	// the following string will be the encrypted password
  cName,	// the following string will be user's name
  cContactUsername
};

#endif	// COMMANDS_H
