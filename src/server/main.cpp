#include <cstdlib>
#include <cstdio>
#include <enet/types.h>
#include <iostream>
#include <enet/enet.h>
#include <map>
#include <cstring>

struct ClientData{
    int id;
    char* username;
};


//TODO: I dont want a map, and these to be pointers
std::map<int, ClientData> ClientMap;


void SendPacket(ENetPeer* peer, const char* data){
    ENetPacket *packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void BroadcastPacket(ENetHost* server, const char* data){
    ENetPacket *packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, packet);

}

void ParseData(ENetHost *server, int id, char *data){
    std:: cout << "Parsing: " << data << "\n";
    int data_type;
    sscanf(data, "%d|", &data_type);

    switch(data_type){
        case 1:
            {
                char *msg;
                msg = data + 2;
                char send_data[1024] = {'\0'};
                sprintf(send_data, "1|%d|%s", id, msg);
                BroadcastPacket(server, send_data);

                break;
            }
            break;

        case 2:{

            char *username = data + 2;
            //sscanf(data,"2|%s", username);
            ClientMap[id].username = strdup(username);            

            char send_data[1024] = {'\0'};
            snprintf(send_data, sizeof(send_data), "2|%d|%s", id, username);
            std::cout << "Send: " << send_data << "\n";
            //I dont think this needs to be broadcast
            BroadcastPacket(server, send_data);
            break;
        }


    }
}

int main(int argc, char ** argv){

    if(enet_initialize() != 0){
        fprintf(stderr,  "An error occured initialized enet\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost *server;
    ENetEvent event;

    address.host = ENET_HOST_ANY;
    address.port = 7777;


    server = enet_host_create(&address, 32, 1, 0, 0);

    if(server == NULL){
        fprintf(stderr,  "An error occured creating enet server host\n");
        return EXIT_FAILURE;
    }

    //TODO: Incrementing isn't really ideal, but it works for now
    int nextClientId = 0;

    //Game loop start
    while(true){


        while(enet_host_service(server, &event, 1000) > 0){
            switch(event.type){
            case ENET_EVENT_TYPE_CONNECT:
            {
                printf("A new client connected from %x:%u.\n",
                        event.peer -> address.host,
                        event.peer -> address.port);

                //Why are we broadcasting every existing user every user when 1 connects, and not send the new one who's new and broadcast his ID
                for(auto const & x: ClientMap){
                    char send_data[1024] = {'\0'};
                    sprintf(send_data, "2|%d|%s", x.first, x.second.username);
                    printf("Send: %s \n",send_data);
                    BroadcastPacket(server, send_data);
                }
                ClientMap[nextClientId] = (ClientData){.id = nextClientId};
                //if the peer can hold data, why do we have the map?
                event.peer->data = (void*)&ClientMap[nextClientId];

                char data_to_send[126] = {'\0'};
                sprintf(data_to_send, "3|%d", nextClientId);
                SendPacket(event.peer, data_to_send);
                nextClientId++;
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %x:%u on channel %u.\n",
                        event.packet -> dataLength,
                        event.packet -> data,
                        //event.peer -> data, changing to show IP for local connectionc
                        event.peer->address.host,
                        event.peer->address.port,
                        event.channelID);

                ParseData(server, static_cast<ClientData*>(event.peer->data)->id, (char*)event.packet->data);
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                printf("%x:%u disconnected.\n",
                        event.peer -> address.host,
                        event.peer -> address.port);
                char disconnected_data[126] = {'\0'};
                sprintf(disconnected_data, "4|%d", static_cast<ClientData*>(event.peer->data)->id);
                BroadcastPacket(server, disconnected_data);

                event.peer->data = NULL;
                break;
            }

            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }

    }
    //Game loop end


    enet_host_destroy(server);

    return EXIT_SUCCESS;


}
