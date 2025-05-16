#include <cstdio>
#include <cstdlib>
#include <enet/enet.h>
#include "chat_screen.hpp"
#include <cstring>
#include <ncurses.h>
#include <pthread.h>
#include <map>

struct ClientData{
    int id;
    char* username;
};

static ChatScreen screen;
static int ClientId = -1;


std::map<int,ClientData> ClientMap;

void SendPacket(ENetPeer* peer, const char* data){
    ENetPacket * packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void ParseData(char *data){
    //std:: cout << "Parsing: " << data << "\n";
    int data_type;
    int id;
    sscanf(data, "%d|%d", &data_type, &id);

    switch(data_type){
        case 1:
            if(id != ClientId){
                char* msg;
                //Temp, ids can be more than 1 digit
                msg = data + 4;
                screen.PostMessage(ClientMap[id].username, msg);
            }
            break;

        case 2:
        {
            if(id != ClientId){
                char* username;
                //Temp, ids can be more than 1 digit
                username = data + 4;
                ClientMap[id]= (ClientData){.id = id, .username = strdup(username)};
            }

            break;
        }


        case 3:
            ClientId = id;
            break;


    }
}
void* MsgLoop(void* client){
    while(true){
        ENetEvent event;
        while(enet_host_service((ENetHost*)client, &event, 0) > 0){
            switch(event.type){
                case ENET_EVENT_TYPE_RECEIVE:
                    ParseData((char*)event.packet->data);
                    enet_packet_destroy(event.packet);
                    break;

            }
        }
    }
}


int main(int argc, char ** argv){

    char username [80];
    printf("Enter Username: ");
    scanf("%s", username);

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

    char str_data[80] = "2|";
    strcat(str_data, username);
    SendPacket(peer, str_data);

    screen.Init();

    //Create thread for recieving data
    pthread_t thread;
    pthread_create(&thread, NULL, MsgLoop, client);

    //Game Loop Start
    

    bool running = true;
    while(running){
        std::string msg = screen.CheckBoxInput();

        screen.PostMessage(username, msg.c_str());
        
        std::string messageData = "1|";
        messageData += msg;


        SendPacket(peer, messageData.c_str());

        if(msg == "/exit"){
            running = false;
        }

    }


    pthread_join(thread,NULL);

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

            case ENET_EVENT_TYPE_CONNECT:
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_NONE:
                enet_packet_destroy(event.packet);
                break;

        }
    }


    return EXIT_SUCCESS;

}
