#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>
#include <cstring>

enum MessageType {
    mSignup,                 // sign up a new user if username is free to use
    mAddUser,                // add the new user to the vector of existing users
    mLogin,                  // login a user if the username/password combination is correct
    mLogout,                 // logout a user
    mQuit,                   // user quit the program
    mFindUser,               // find another user to send the contact request
    mContactRequest,         // send the contact request (and response to it?)
    mEstablishedContact,     // send an established contact to a user
    mSentContactRequest,     // send user information for a sent contact request
    mReceivedContactRequest, // send user information for a received contact request
    mUsernameStatus,         // return status on the username query for new users
    mSignupName,             // send the name of the user to be signed up
    mPassword,               // send the password of the user to be signed up
    mText,                   // text message to another user
    mCall,                   // request/receive a voice call with another user
    mCallRequest,            // respond to a request for a voice call from another user
    mCallResponse,           // transmit a response to a request for a voice call (DBG: too many call message types?)
    mVoice                   // voice message to another user
};

#endif // MESSAGE_H
