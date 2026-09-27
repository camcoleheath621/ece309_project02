// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>
#include <string>   
#include <utility>  // used for std::move
#include <fstream>  // used for Test 12 transcript file

int main() {
    

    // Test 1: Empty Conversation Bounds **PASSED**
    //
    // verify that an empty conversation can handle an invalid
    // index safely and remains empty after accessing it
    {
        Conversation empty;                     // create conversation

        assert(empty.size()==0);                // verify conversation is empty
        assert(empty.begin() == empty.end());   

        const Message& invalid = empty.at(0);   // create variable for message at index 0

        assert(invalid.role() == Role::System); // verify system role
        assert(empty.size() == 0);              // verify empty conversation has no messages

    }

    // Test 2: System Message Ordering **PASSED**
    //
    // verify system message is first and all messages
    // stay in the correct role and order
    {
        Conversation ordering;                  // create conversation

        // create mock conversation
        ordering.append(Message(Role::System, "Be concise"));
        ordering.append(Message(Role::User, "Hello"));
        ordering.append(Message(Role::Assistant, "Hi!"));
        ordering.append(Message(Role::User, "How are you doing?"));
        ordering.append(Message(Role::Assistant, "Well, how are you doing?"));
        
        // verify all messages were stored
        assert(ordering.size() == 5); 

        // verify all messages have correct roles and indices
        assert(ordering.at(0).role() == Role::System);
        assert(ordering.at(0).content() == "Be concise");

        assert(ordering.at(1).role() == Role::User);
        assert(ordering.at(1).content() == "Hello");

        assert(ordering.at(2).role() == Role::Assistant);
        assert(ordering.at(2).content() == "Hi!");

        assert(ordering.at(3).role() == Role::User);
        assert(ordering.at(3).content() == "How are you doing?");

        assert(ordering.at(4).role() == Role::Assistant);
        assert(ordering.at(4).content() == "Well, how are you doing?");
    
    }

    // Test 3: Rule of Five: Copy Constructor **PASSED**
    //
    // verify the copy constructor makes a deep copy with identical
    // messages that have a different memory location
    {
        Conversation copy;          // create conversation

        // create mock conversation
        copy.append(Message(Role::User,"Hello"));
        copy.append(Message(Role::Assistant,"Hi!"));
        copy.append(Message(Role::User,"Does my harness obey the Rule of Five"));
        copy.append(Message(Role::Assistant,"I hope so!"));

        Conversation copy2(copy);   // use copy constructor 

        assert(copy.size() == copy2.size());    // verify both conversations are the same size
        
        // verify deep copy by going through every message in both conversations
        for (std::size_t i = 0; i < copy.size(); i++) {             
            assert(copy.at(i).role() == copy2.at(i).role());        // verify roles
            assert(copy.at(i).content() == copy2.at(i).content());  // verify messages
        }

        assert(copy.begin() != copy2.begin()); // verify the two conversations use different memory locations

    }

    // Test 4: Rule of Five: Move Constructor **PASSED**
    //
    // verify the move constructor steals the original memory location
    // and leaves the original conversation empty
    {
        Conversation move;      // create conversation

        // create mock conversation
        move.append(Message(Role::User,"Hello"));
        move.append(Message(Role::Assistant,"Hi!"));
        move.append(Message(Role::User,"Does my harness obey the Rule of Five"));
        move.append(Message(Role::Assistant,"I hope so!"));

        // save original memory location before moving
        const Message* move_og_ptr = move.begin(); 

        // make copy of original for comparison
        Conversation expected(move); 

        // use move constructor to transfer ownership
        Conversation move2(std::move(move));    // used #include<utility>

        // verify size of moved conversation
        assert(expected.size() == move2.size());
        
        // verify that every message and role matches original conversation
        for (std::size_t i = 0; i < expected.size(); i++) {
            assert(expected.at(i).role() == move2.at(i).role());
            assert(expected.at(i).content() == move2.at(i).content());   
        }

        // verify move2 stole original memory location
        assert(move_og_ptr == move2.begin()); 

        // verify original conversation is now empty
        assert(move.size()==0);
        assert(move.begin() == move.end());
    }

    // Test 5: Growth Behavior **PASSED**
    //
    // verify the memory grows by doubling when full
    // while still preserving all stored messages
    {
        Conversation growth; // create conversation

        const Message* growth00_ptr = growth.begin(); // save original memory location

        // Message 1 at index [0]
        growth.append(Message(Role::User, "Hello"));    // capacity goes from 0 to 1
        const Message* growth01_ptr = growth.begin();   // save new memory location
        assert(growth.begin() != growth00_ptr);         // verify reallocation

        // Message 2 at index [1]
        growth.append(Message(Role::Assistant, "Hi!")); // capacity goes from 1 to 2
        const Message* growth02_ptr = growth.begin();   // save new memory location
        assert(growth.begin() != growth01_ptr);         // verify reallocation

        // Message 3 at index [2]
        growth.append(Message(Role::User, "How are you doing?")); // capacity goes from 2 to 4
        const Message* growth03_ptr = growth.begin();             // save new memory location
        assert(growth.begin() != growth02_ptr);                   // verify reallocation

        // Message 4 at index [3]
        growth.append(Message(Role::Assistant, "Well, how are you doing?")); // capacity stays 4
        const Message* growth04_ptr = growth.begin();                        // save new memory location
        assert(growth.begin() == growth03_ptr);                              // verify no reallocation
        
        // Message 5 at index [4]
        growth.append(Message(Role::User, "I'm doing good, what is your growth behavior?")); // capacity goes from 4 to 8
        const Message* growth05_ptr = growth.begin();               // save new memory location
        assert(growth.begin() != growth04_ptr);                     // verify reallocation

        // Message 6 at index [5]
        growth.append(Message(Role::Assistant, "My growth behavior is O(1)")); // capacity stays 8
        const Message* growth06_ptr = growth.begin();               // save new memory location
        assert(growth.begin() == growth05_ptr);                     // verify no reallocation

        // Message 7 at index [6]
        growth.append(Message(Role::User, "Why is append amortized O(1)?")); // capacity stays 8
        const Message* growth07_ptr = growth.begin();               // save new memory location
        assert(growth.begin() == growth06_ptr);                     // verify no reallocation

        //Message 8 at index [7]
        growth.append(Message(Role::Assistant, "Because the array doubles its capacity when full, so reallocations happen less often.")); // capacity stays 8
        const Message* growth08_ptr = growth.begin();               // save new memory location
        assert(growth.begin() == growth07_ptr);                     // verify no reallocation
        
        // Message 9 at index [8]
        growth.append(Message(Role::User, "Thank you!")); // capacity goes from 8 to 16
        const Message* growth09_ptr = growth.begin();               // save new memory location
        assert(growth.begin() != growth08_ptr);                     // verify reallocation

        // Message 10 at index [9]
        growth.append(Message(Role::Assistant, "Of course, have a good day!")); // capacity stays 16
        assert(growth.begin() == growth09_ptr);                     // verify no reallocation

        // verify all messages were stored
        assert(growth.size()== 10);

        Role expected_roles[10] = 
        {
        Role::User,         // index[0], line 1
        Role::Assistant,    // index[1], line 2
        Role::User,         // index[2], line 3
        Role::Assistant,    // index[3], line 4
        Role::User,         // index[4], line 5
        Role::Assistant,    // index[5], line 6
        Role::User,         // index[6], line 7
        Role::Assistant,    // index[7], line 8
        Role::User,         // index[8], line 9
        Role::Assistant     // index[9], line 10
        };

        std::string expected_content[10] = 
        {
        "Hello",                                            // index[0], line 1
        "Hi!",                                              // index[1], line 2
        "How are you doing?",                               // index[2], line 3
        "Well, how are you doing?",                         // index[3], line 4
        "I'm doing good, what is your growth behavior?",    // index[4], line 5
        "My growth behavior is O(1)",                       // index[5], line 6
        "Why is append amortized O(1)?",                    // index[6], line 7
        "Because the array doubles its capacity when full, so reallocations happen less often.", // index[7], line 8
        "Thank you!",                                       // index[8], line 9        
        "Of course, have a good day!"                       // index[9], line 10
        };

        // go through all messages to see if they were preserved during reallocation
        for (std::size_t i = 0; i < growth.size(); i++) {
            assert(growth.at(i).role() == expected_roles[i]);       // verify roles
            assert(growth.at(i).content() == expected_content[i]);  // verify messages
        }
    }

    // Test 6: Sentinel Scanner: Clean Text **PASSED**
    //
    // verify normal text without a sentinel is returned correctly
    // and does not falsely trigger the sentinel
    {
        SentinelScanner scanner("<|end_conversation|>"); // create scanner with stop sentinel

        SentinelScanner::Out result = scanner.feed("Hello there");      // feed scanner normal text with no sentinel
        assert(result.sentinel_found == false);                         // verify no sentinel was falsely detected

        SentinelScanner::Out flushed = scanner.flush();                 // release any text held in pending_

        assert(result.safe_text + flushed.safe_text == "Hello there");  // verify that all original text has returned
    }

    // Test 7: Sentinel Scanner: Split Sentinel **PASSED**
    //
    // verify that our sentinelscanner correctly flags a sentinel 
    // when feed multiple partial pieces across every boundary
    {
        std::string sentinel = "<|end_conversation|>";      // define stop sentinel
        std::string text = "Goodbye" + sentinel;            // combine safe text with sentinel

        // test all possible locations where the input could be split
        for(std::size_t i = 0; i <= text.size(); i++) {
            SentinelScanner split(sentinel); // create a new scanner for each split

            SentinelScanner::Out result_split01 = split.feed(text.substr(0,i));   // feed first part
            SentinelScanner::Out result_split02 = split.feed(text.substr(i));    // feed remaining part

            assert((result_split01.sentinel_found || result_split02.sentinel_found) == true);   // verify sentinel was found
            assert((result_split01.safe_text + result_split02.safe_text) == "Goodbye");        // verify only safe text was returned
        }
    }

    // Test 8: Sentinel Scanner: False Alarms **PASSED**
    //
    // verify that the sentinelscanner does not mistake
    // similar text for the actual sentinel
    {
        SentinelScanner false_alarm("<|end_conversation|>"); // create false_alarm with stop sentinel

        SentinelScanner::Out result_fa_01 = false_alarm.feed("See you later <|end");    // feed false_alarm normal text + partial sentinel
        SentinelScanner::Out result_fa_02 = false_alarm.feed("_discussion|>");          // feed false_alarm normal text after partial sentinel

        assert((result_fa_01.sentinel_found || result_fa_02.sentinel_found) == false);  // verify sentinel was not found

        SentinelScanner::Out flushed_fa = false_alarm.flush();  // release remaining text in pending_

        assert(result_fa_01.safe_text + result_fa_02.safe_text + flushed_fa.safe_text == "See you later <|end_discussion|>"); // verify that all original text has returned

    }

    // Test 9: Sentinel Scanner: Bounded Memory **PASSED**
    //
    // verify that pending_ never exceeds sentinel.size() - 1
    // when feeding a large adversarial stream
    {
        std::string sentinel_bm = "<|end_conversation|>";   // stop sentinel used for bounded_memory
        SentinelScanner bounded_memory(sentinel_bm);        // create bounded_memory
        
        std::string adversarial;                    // adversarial input
        std::string repeat = "<|end_";              // repeatedly resembles the start of the sentinel

        std::size_t four_mb = 4 * 1024 * 1024;      // 4MB target stream size
        std::size_t total_fed = 0;                  // total chars sent to bounded_memory
        std::size_t total_safe = 0;                 // total chars released as safe text
        
        // Build a 4MB adversarial string
        while(adversarial.size() < four_mb) {       
            adversarial += repeat;
        }

        adversarial.resize(four_mb);    // resize to exactly 4MB

        // Feed stream one char at a time
        for(std::size_t i = 0; i < adversarial.size(); i++) {

            std::string one_char(1, adversarial[i]);    // turn single char into single char string

            SentinelScanner::Out result_bm = bounded_memory.feed(one_char); // feed bounded_memory the one char string
            
            total_fed += 1;                                         // keep track of all chars entering bounded_memory
            total_safe += result_bm.safe_text.size();               // count chars released as safe text

            std::size_t pending_amount = total_fed - total_safe;    // calculate pending_ manually without accessing private

            assert(pending_amount <= (sentinel_bm.size() - 1));     // verify that pending_ never exceeds sentinel length -1

        }

    }

    // Test 10: Harness: Turn Limit **PASSED**
    //
    // verify that the harness stops after the turn limit
    // has been reached
    {
        // Fake User Input
        class TestInput : public InputSource {   
        public: 
            std::string read_line() override {
                if (count_ == 0) {          // first line
                    count_++;               // move to next fake input
                    return "Hello";         // fake input "Hello"
                }
                
            
                if (count_ == 1) {          // second line
                    count_++;               // move to next fake input
                    return "How are you";   // fake input "How are you"
                }

                eof_ = true;                // end of fake input
                return "";
            }

            bool is_eof() const override {  // see if there is any input left
                return eof_;
            }

        private :
            int count_ = 0;                 // initilize count_ at 0
            bool eof_ = false;              // initilize eof_ for false
        };

        // Fake Output Terminal
        class TestOutput : public OutputSink {
        public:
            void write(std::string_view text) override {    // calls harness to print something
                output_ += text;                            // save the printed text
            }
        
        private:
            std::string output_;                            // store everything harness prints
        };

        // only allow two conversation turns
        HarnessConfig cfg;
        cfg.max_turns = 2;

        // create fake model using the greeting script
        auto model_tl = std::make_unique<ScriptedModelClient>("scripts/greeting.script");

        // copy the script's system message into the harness config
        cfg.system_message = model_tl->system_message();

        // transfer ownership
        Harness harness_tl(std::move(model_tl), cfg); // uses <utility>

        TestInput input_tl;     // create our own fake input
        TestOutput output_tl;   // create our own fake output
        
        // run the conversation and save why it stopped
        StopReason result_tl = harness_tl.run(input_tl, output_tl); 

        // verify it stopped because it reached two turns
        assert(result_tl.kind == StopReason::Kind::TurnLimit);

    }

    // Test 11: Harness: Sentinel Halt **PASSED**
    //
    // verify that the harness stops with the sentinel
    // when the model produces the stop sentinel
    {
        // Fake User Input
        class TestInput : public InputSource {   
        public: 
            std::string read_line() override {
                if (count_ == 0) {          // first line
                    count_++;               // move to next fake input
                    return "Hello";         // fake input "Hello"
                }
                
            
                if (count_ == 1) {          // second line
                    count_++;               // move to next fake input
                    return "How are you";   // fake input "How are you"
                }

                if (count_ == 2) {          // third line
                    count_++;               // move to next fake input
                    return "Goodbye";       // fake input "Goodbye"
                }

                eof_ = true;                // end of fake input
                return "";
            }

            bool is_eof() const override {  // see if there is any input left
                return eof_;
            }

        private :
            int count_ = 0;                 // initilize count_ at 0
            bool eof_ = false;              // initilize eof_ for false
        };

        // Fake Output Terminal
        class TestOutput : public OutputSink {
        public:
            void write(std::string_view text) override {    // calls harness to print something
                output_ += text;                            // save the printed text
            }
        
        private:
            std::string output_;                            // store everything harness prints
        };

        // allow a higher turn limit
        HarnessConfig cfg;
        cfg.max_turns = 10;

        // create fake model using the greeting script
        auto model_t2 = std::make_unique<ScriptedModelClient>("scripts/greeting.script");

        // copy the script's system message into the harness config
        cfg.system_message = model_t2->system_message();

        // transfer ownership
        Harness harness_t2(std::move(model_t2), cfg); // uses <utility>

        TestInput input_t2;     // create our own fake input
        TestOutput output_t2;   // create our own fake output
        
        // run the conversation and save why it stopped
        StopReason result_t2 = harness_t2.run(input_t2, output_t2); 

        // verify it stopped because model produced the sentinel
        assert(result_t2.kind == StopReason::Kind::Sentinel);
    }

    // Test 12: Transcript Round Trip **PASSED**
    //
    // verify ReplayModelClient reproduces the same messages 
    // in a mock conversation using the transcript format     
    {
        Conversation mock;   // create conversation

        // create mock conversation messages
        mock.append(Message(Role::System, "Be concise."));
        mock.append(Message(Role::User, "Hello"));
        mock.append(Message(Role::Assistant, "Hi there!"));
        mock.append(Message(Role::User, "Goodbye"));
        mock.append(Message(Role::Assistant, "Goodbye!"));

        // open transcript file for saving
        std::ofstream file("tests/p2/test_transcript.txt"); // used #include <fstream>

        // verify the file opened correctly
        assert(file.is_open());   

        bool first = true;  // format helper

        // save every message using the same format that's in main.cpp
        for (const Message* m = mock.begin(); m != mock.end(); ++m) {

            // if this is not the first message use seperate message blocks
            if (!first) {
                file << "---\n";   
            }

            // after the first message beings, no other message can be first
            else {
                first = false;
            }
            
            // save the correct role name
            if (m->role() == Role::System) {
                file << "role: system\n";
            }
            
            else if (m->role() == Role::User) {
                file << "role: user\n";
            }
            
            else {
                file << "role: assistant\n";
            }

            // save file content
            file << m->content() << "\n";   
        }

        // finish saving before replaying the conversation
        file.close();   

        // load the transcript we just saved
        ReplayModelClient replay("tests/p2/test_transcript.txt");

        // create the replay conversation
        Conversation replay_conversation;   

        // replay the two assistant messages
        Message replay_01 = replay.generate(replay_conversation);
        Message replay_02 = replay.generate(replay_conversation);

        // verify saved system message was loaded correctly
        assert(replay.system_message() == mock.at(0).content());

        // verify replayed roles
        assert(replay_01.role() == Role::Assistant);
        assert(replay_02.role() == Role::Assistant);

        // verify replayed messages match the original messages
        assert(replay_01.content() == mock.at(2).content());
        assert(replay_02.content() == mock.at(4).content());
    }

    return 0;
}
