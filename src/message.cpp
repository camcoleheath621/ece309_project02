#include "core/message.h"

// default constructor for empty message
Message::Message() {        

    role_ = Role::System;   // set role_ to the System option
    content_ = "";          // empty string for content_
}

// constructor with parameters
Message::Message(Role role, std::string content) {  
    role_ = role;           // set role_ to the given role
    content_ = content;     // set content_ to the given content
}

// getter for role_ so that it can be accessed without modifying the object
Role Message::role() const noexcept {  
    return role_;           // return the role_
}

// getter for content_ so that it can be accessed without modifying the object
const std::string& Message::content() const noexcept {  
    return content_;        // return the content_
}
