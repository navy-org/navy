#include <bootstrap.proto>
#include <hello.proto>
#include <logging>
#include <navy>
#include <port>
#include <result>

#define MSG "What's up? I'm the client."

Res main([[gnu::unused]] int argc, [[gnu::unused]] char const *argv[])
{
    log$("Sup from the client!");

    IpcPort hello = try$(bootstrap_lookup("org.navy.hello"));
    log$("Service found, sending message...");

    HelloGreeting greeting = {
        .msg = MSG,
    };

    send$(hello, greeting);

    return ok$();
}
