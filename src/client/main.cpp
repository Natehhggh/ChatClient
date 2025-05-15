#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <enet/enet.h>


int main(int argc, char ** argv){
    if(enet_initialize() != 0){
        fprintf(stderr,  "An error occured initialized enet\n");
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize);

    ENetHost *client;
    //num outgoing connections, number channels, bandwith
    client = enet_host_create(NULL,1, 1, 0, 0);

    if(client == NULL){
        fprintf(stderr,  "An error occured creating enet host\n");
        return EXIT_FAILURE;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;


    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;

    peer = enet_host_connect(client, &address, 1, 0);

    if(peer == NULL){
        fprintf(stderr,  "No available peers for initiating connection");
        return EXIT_FAILURE;
    }

    //this can be looped to attempt to reconnect, or to return to menu in a game or something
    //for now we're exiting with success
    if(enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT){
        puts("Connected to localhost::7777 successful");
    }else{
        enet_peer_reset(peer);
        puts("Connection to localhost::7777 failed");
        return EXIT_SUCCESS;
    }


    //Game Loop Start
    while(enet_host_service(client, &event, 1000) > 0){
        switch(event.type){
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %x:%u on channel %u.\n",
                        event.packet -> dataLength,
                        event.packet -> data,
                        //event.peer -> data, changing to show IP for local connectionc
                        event.peer->address.host,
                        event.peer->address.port,
                        event.channelID);
                break;

        }
    }
    //Game Loop End


    enet_peer_disconnect(peer, 0);

    while(enet_host_service(client, &event, 3000) > 0){
        switch(event.type){
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                puts("Disconnected successfully");
                break;

        }
    }

    return EXIT_SUCCESS;

}
