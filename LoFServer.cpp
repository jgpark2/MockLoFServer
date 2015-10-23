#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <unordered_map>
#include "39dll.h"
#include "player.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "wininet.lib")

using namespace std;


const enum MSG {NONE, LOGIN, ENTER, LEAVE, CHAT};

const enum ITEM_REF {
    I_TYPE=0;
    I_PHY=1;
    I_NIM=2;
    I_MEN=3;
    I_HP=4;
    I_MP=5;
    I_DEFENSE=6;
    I_SPEED=7;
    I_DAMAGE=8;
    I_MAGIC=9;
    I_COST=10;
    I_PROFIT=11;
}

const enum PARTY_REF {
    MAX_PARTY_NUM=6;
    NO_PARTY=-1;
    PARTY_ERROR=0;
    PARTY_CREATE=1;
    PARTY_UPDATE=2;
    PARTY_LEAVE=3;
    PARTY_INVITE=4;
    PARTY_INVITE_ERROR=5;
    PARTY_INVITE_REJECT=6;
    PARTY_INVITE_ACCEPT=7;
    PARTY_JOIN_ERROR=8;
    PARTY_EXP=9;
}

const int MAX_PLAYERS=32;

unordered_map<Player*> players;
vector<Party> party_list;
bool running=false;

void sendAll(Player* player) {
    for(auto it : players)
        if((it->second) != player)
            sendmessage((it->second)->sock, "", 0, 0);
}

void caseLogin() {
    int id=readbyte(0);
    Player player = players.get(id);
    player->name = readstring(0);
    cout << "Player: " << player->name << " joined." << endl;
    clearbuffer(0);
        writebyte(3,0);
        writebyte(id,0);

    /*const char* name_1=player->name.c_str();
    char* name=const_cast<char*>(name_1);*/
    //^ string to char* conversion... change it to:   &(player->name)[0]
        writestring(&(player->name)[0],0);
        sendAll(player);

    for(auto it : players)
        if((it->second) != player) {
            clearbuffer(0);
                writebyte(3,0);
                writebyte((it->second)->id,0);
                writestring(&(it->second->name)[0],0);
                sendmessage(it->second->sock, "", 0, 0);
        }
                
}

void caseEnter() {
    int id=readbyte(0);
    Player player = players.get(id);

    player->x = readshort(0);
    player->y = readshort(0);
    player->direction = readshort(0);

    clearbuffer(0);
        writebyte(5,0);
        writebyte(id,0);
        writeshort(player->x,0);
        writeshort(player->y,0);
        writeshort(player->direction,0);
        sendAll(player);
}

void caseLeave() {
    int id=readbyte(0);
    Player player = players.get(id);

    cout << player->name << " has left." << endl;

    clearbuffer(0);
        writebyte(6,0);
        writebyte(id,0);
        writestring(&(player->name)[0],0);
        sendAll(player);

    closesock(player->sock);
}

void caseChat() {
    string message=readstring(0);
    cout << message << endl;
    clearbuffer(0);
        writebyte(8,0);
        writestring(&message[0],0);
        sendAll(nullptr);
}

void handleMSG(MSG msgID){
    /*
    --obj_login event user 0-- In my case, this should include the 100-line event user 0 code on obj_player too
    messageid = readbyte() //Check the ID of the incoming message

    switch(messageid) //Depending on the ID we will have to execute a different script, we use a switch-case structure for this
    {
        case MSG_ACCOUNT_CREATE: //Player wants to create an account
        case_msg_account_create()
        break;

        case MSG_LOGIN: //Player tries to login
        case_msg_login()
        break;

        case MSG_ENTER: //Player entered the game room
        case_msg_enter()
        break;
        
        case MSG_ACC_ENTER: //Player logged in with his account (not with character yet)
        case_msg_account_enter()
        break;
        
        case MSG_CHAR_CREATE: //Player tries to create new char
        case_msg_char_create()
        break;
        
        case MSG_CHAR_DELETE: //Player tries to delete a char
        case_msg_char_delete()
        break;
        
        case MSG_CHAR_LOGIN: //Player tries to login in to game with char
        case_msg_char_login()
        break;
    }
    */

    switch(msgID) {
        case LOGIN:
            caseLogin();
            break;

        case ENTER: //Relaying requested user data.
            caseEnter();
            break;

        case LEAVE: //Someone left.
            caseLeave();
            break;

        case CHAT: // The chat
            caseChat();
            break;

        default:
            break;
    }
}

void end() {
    dllFree();
    closesock(listen);
}

int main()
{
    int serverport = 10800;

    dllInit();

    scr_server_init() //Initialize the engine
    scr_vars_init('stagbeetle') //Intialize import global config variables
    init_PQ_rooms();

    //_addline('Created Accounts: '+string(global._accounts))
    //_addline('Created Characters: '+string(global._characters))

    auto buff_buffer_ind = 0;
    auto buff_buffer_max = 32;
    int buff_buffer[13+1][buff_buffer_max]; //13= original buff info slots, +1 for chance slot, ...
    //..as this needs to be stored unlike other buff calculations, where chance is given and thrown out
    for(auto buff_it : buff_buffer)
        for(auto buff_info_it : *buff_it)
            *buff_info_it = 0;
    /*for (int i = 0; i < buff_buffer_max; i += 1) {
        for (int j=0; j<13+1; j+=1)
            buff_buffer[i,j]=0;
    }*/

    /* --obj_controller user event 10, aka when you start the server from console--
    start=true;

    listen = tcplisten(global._port,10,true); // Open a connection for players

    if(!listen) 
    {
    show_message("Listening Socket could not be created!#*sob*");
    game_end();
    exit; //Game end doesn't stop the whole script so call exit!
    }
    alarm[0] = room_speed //Reset the Traffic-Calculation each second

    init_monsters()
    init_cashshopitem()

    instance_create(224,8,obj_startwar_btn)
    instance_create(224,32,obj_forcelogout_btn)
    //instance_create(334, 8, obj_sendchanges_btn)

    global._inputshow=false
    _text=""*/
    running = true;

    auto listen = tcplisten(serverport, 2, 1);
	if (listen <= 0) {
		cout << "Failed to listen on port " << serverport << endl;
        end();
		return 0;
	}
	cout << "Server listening on port " << serverport << endl;

	init_monsters();
    init_cashshopitem();
    
	while(true)
	{
        if (!running)
            break;

        Sleep(5); // Pause for a bit, without this your CPU would max out (1ms)
        //where is this Sleep fn? sleep in c++ goes by seconds, usleep is unsupported and harder to workaround in windows


        /*
        --obj_controller step events--
        if !start exit;

        client = tcpaccept(listen, true) //Accept incoming connections in non-blocking mode
        if(client <= 0)exit;    //Exit this code if no client wants to connect

        //If a client wants to connect we use the setnagle function to enable a smooth connection
        setnagle(client,true)

        //We create a new instance of obj_login that will manage all incoming messages
        //from the new client that just connected to our server

        new_client = instance_create(8,56,obj_login)    //Create the new instance
        new_client.socket = client                      //Give the new instance a socket id that it will use to communicate with the client
        sound_play(sound0);
        _addline("Connection from 'iphere'...");
        */

	    
        // Accept New Players
	    auto newSocket = tcpaccept(listen, true); //Accept incoming connections in non-blocking mode
	    if(newSocket > 0)
        {
            //If a client wants to connect we use the setnagle function to enable a smooth connection
            setnagle(newSocket,true);

            if (players.size()>=MAX_PLAYERS) {
                clearbuffer(0);
                writebyte(2, 0);
                writebyte(i, 0);//?
                sendmessage(newSocket, "", 0, 0);
                cout << tcpip(newSocket) << " ignored due to max connection size " << MAX_PLAYERS << endl;
            }
            else {
                Player* player = new Player();
                player->sock = newSocket;
                clearbuffer(0);
                writebyte(1, 0);
                writebyte(player->id, 0);//?
                sendmessage(player->sock, "", 0, 0);
                cout << "Connection from " << tcpip(player->sock) << " ..." << endl;
                break;
            }
        }


        /*
        --obj_login step event--
        if(!tcpconnected(socket))
        scr_objlogindestroy()

        _ip=iptouint(tcpip(socket))
        //We check the size of the incoming message to display it on the server screen
        size = receivemessage(socket);

        //If the size is <0 we didn't receive any messages so we don't do anything
        if(size < 0) 
        break;

        //If the size of the message we received is 0 the client disconnected so we delete the instance to break the connection
        if(size == 0)
        {
            scr_objlogindestroy()
            break;
        }

        bufferdecrypt(global._buffpass) //Decrypt the Buffer
        temp_size = buffsize()          //Check the size of the message that just came in
        global._download_sec += temp_size   //Refresh the amount of received traffic for the current second to draw it on the server screen
        global._download_total += temp_size //Refresh the amount of total received traffic to draw it on the server screen
        event_user(0) //Switch to the user_defined event0 to check the message we just got
        */
        // INCOMING CLIENT MESSAGES (Messages Sent From Client)
        for(auto it : players)
		{
            Player player = it->second;  //This whole section should be a method inside Player :x

            auto size = receivemessage(player->sock, 0, 0); // Messages coming from clients

            if(!tcpconnected(player->socket) || size==0) { // Yessss finally short circuits
                scr_objlogindestroy();
                continue;
            }

            if(size > 0) {
                auto msgID = (MSG)(int)readbyte(0);
                handleMSG(msgID);
            }
        }
	}

	clearbuffer(0);
	writebyte(7, 0);

    for(auto it : players) {
        sendmessage(it->second->sock, "", 0, 0);
    }

    return 0;
}
