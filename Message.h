#include <string>
#include <iostream>
namespace FIX{
    struct Tag{
        enum{
            BeginString = 8,
            BodyLength = 9,
            MsgType = 35,
            SenderCompId = 49,
            TargetCompId = 56,
            HeartbeatInterval = 108,

        };
    };
    struct MsgType{

        enum{
            Logon = 'A',
            Hearbeat = '0',
            NewOrder = 'D',
        };
    };

}