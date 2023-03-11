#include "../src/peer_connection.h"

class ObserverExt
{
public:
    static void OnConnectionChange(void* ctx, PeerConnectionState state)
    {
        
    }
};

int main()
{
    Events events;
    events.on_connection_change = &ObserverExt::OnConnectionChange;
    
    auto peer = rtc_create_peer_connection(nullptr, &events, nullptr);
    if (peer)
    {
        return -1;
    }
    
    
    
    return 0;
}
