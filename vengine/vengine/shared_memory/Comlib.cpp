#include "pch.h"
#include "Comlib.h"

Comlib::Comlib(LPCWSTR bufferName, size_t bufferSize, ProcessType type)
{
    this->type = type;
    sharedMemory = new(__FILE__, __LINE__) Memory(bufferName, bufferSize);
    messageData = sharedMemory->GetMemoryBuffer();
    mutex = new(__FILE__, __LINE__) Mutex(L"MutexMap");

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
                message = new(__FILE__, __LINE__) char[msgLength];
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