#include "Comlib.h"

Comlib::Comlib(LPCWSTR bufferName, size_t bufferSize, ProcessType type)
{
    this->type = type;
    sharedMemory = new Memory(bufferName, bufferSize);
    messageData = sharedMemory->GetMemoryBuffer();
    mutex = new Mutex(L"MutexMap");

    head = sharedMemory->GetControlBuffer();
    tail = head + 1;
    freeMemory = tail + 1;

    if (type == Producer)
    {
        std::cout << "Producer activated" << std::endl;

        *head = 0;
        *tail = 0;
        *freeMemory = bufferSize;
    }
    else if (type == Consumer)
    {
        std::cout << "Consumer activated" << std::endl;

        *tail = 0;
    }
}

Comlib::~Comlib()
{
    if (sharedMemory)
        delete sharedMemory;
    if (mutex)
        delete mutex;
}

//bool Comlib::Send(char* message, SectionHeader* secHeader)
//{
//    mutex->Lock();
//    bool result = false;
//
//    //save size of free memory in variable
//    size_t memoryLeft = sharedMemory->GetBufferSize() - *head;
//
//    //is the size of the message plus message header larger than size of free memory
//    if (secHeader->messageLength + sizeof(SectionHeader) >= memoryLeft)
//    {
//        //has consumer read the data at the start of the buffer
//        if (*tail != 0)
//        {
//            //Set messageID to 0 to indicate we start at the beginning of the buffer
//            secHeader->messageID = 0;
//
//            //copy data from mgsHeader to messageData at the position of head, so we don't overwrite anything
//            memcpy(messageData + *head, secHeader, sizeof(SectionHeader));
//
//            //reduce free memory
//            *freeMemory -= (secHeader->messageLength + sizeof(SectionHeader));
//            //set head to beginning of buffer
//            *head = 0;
//
//            mutex->Unlock();
//            result = false;
//        }
//        //if not, do nothing and wait for consumer to stop being lazy
//        else
//        {
//            mutex->Unlock();
//            result = false;
//        }
//    }
//    else if (secHeader->messageLength + sizeof(SectionHeader) < *freeMemory - 1)
//    {
//        secHeader->messageID = 1;
//
//        //Write data in msgHeader to messageData offset by *head
//        memcpy(messageData + *head, secHeader, sizeof(SectionHeader));
//        //Write data in message to messageData offset by *head and the header we just wrote
//        memcpy(messageData + *head + sizeof(SectionHeader), message, secHeader->messageLength);
//
//        //subtract size of message + messageHeader from size of freememory
//        *freeMemory -= (secHeader->messageLength + sizeof(SectionHeader));
//        //set new head position to previous head position plus the data we just wrote. 
//        //Modulus to wrap around to the beginning if needed
//        *head = (*head + secHeader->messageLength + sizeof(SectionHeader)) % sharedMemory->GetBufferSize();
//
//        mutex->Unlock();
//        //message has been sent!
//        result = true;
//    }
//    else
//    {
//        mutex->Unlock();
//        result = false;
//    }
//
//    return result;
//}

bool Comlib::Recieve(char*& message, MessageHeader*& secHeader)
{
    mutex->Lock();
    bool result = false;
    size_t msgLength = 0;

    if (*freeMemory < sharedMemory->GetBufferSize())
    {
        //if free memory is less than total size it means head has started writing data
        if (*head != *tail)
        {
            //save messageheader data at *tail position in header
			secHeader = ((MessageHeader*)&messageData[*tail]);
            msgLength = secHeader->messageLength;

            if (secHeader->messageType == 0)
            {
                //message type 0 means buffer restart
				*freeMemory += (msgLength + sizeof(MessageHeader));
                //move tail position to beginning
                *tail = 0;

                mutex->Unlock();
                result = false;
            }
            else
            {
                message = new char[msgLength];
                //message ID != 0 means there's a message to read
                //copy data from messageData (at tail position plus size of the header previously read) to message variable
				memcpy(message, &messageData[*tail + sizeof(MessageHeader)], msgLength);

                //update tail position by adding message length and size of header, and modulus to 
                //wrap around to beignning if needed
				*tail = (*tail + msgLength + sizeof(MessageHeader)) % sharedMemory->GetBufferSize();
				*freeMemory += (msgLength + sizeof(MessageHeader));

                mutex->Unlock();
                //message has been read!
                result = true;

            }

        }
        else
        {
            //All data read, unlock and wait for more data
            mutex->Unlock();
            result = false;
        }
    }
    else
    {
        //if free memory is not less than total size it means head has not written any data to the buffer
        //noting to read - unlock and wait for head to stop being lazy
        mutex->Unlock();
        result = false;
    }


    return result;
}