#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>
#include <cstring>

enum MessageType {
    mSignup,         // sign up a new user if username is free to use
    mAddUser,        // add the new user to the vector of existing users
    mLogin,          // login a user if the username/password combination is correct
    mLogout,         // logout a user
    mQuit,           // user quit the program
    mFindUser,       // find another user to send the contact request
    mContactRequest, // send the contact request (and response to it?)
    mSendContact,    // send the client-distributed process a contact
    mUsernameStatus, // return status on the username query for new users
    mSignupName,     // send the name of the user to be signed up
    mPassword,       // send the password of the user to be signed up
    mText,           // text message to another user
    mVoice           // voice message to another user
};

#endif // MESSAGE_H
