#include "Diameter.h"

typedef unsigned short WORD;
typedef unsigned int   DWORD;

const unsigned short WORD_LENGTH  = 16;
const unsigned short DWORD_LENGTH = 32;

const WORD word_masks[]   = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
                              0x001F, 0x003F, 0x007F, 0x00FF,
                              0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                              0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

DWORD extractDword(char* buffer, int startWord, int startBit, int len)
{
   DWORD value = 0, value1 = 0;

   if (startBit+len <= WORD_LENGTH)
   {
      startWord = startWord * 2;
      unsigned char HIBYTE = (unsigned char) buffer[startWord];
      unsigned char LOBYTE = (unsigned char) buffer[startWord+1];
      WORD word = (WORD) (HIBYTE << 8) | LOBYTE;
      value =  value | (word >> (WORD_LENGTH - startBit - len)) & word_masks[len];
   }
   else
   {
      int lenLeft = len;
      startWord =  startWord * 2;
      int  wordOffset = startWord;
      WORD curWord;
      bool firstWord = true;

      while (lenLeft >= WORD_LENGTH || firstWord)
      {
         unsigned char HIBYTE = (unsigned char) buffer[wordOffset];
         unsigned char LOBYTE = (unsigned char) buffer[wordOffset+1];
         curWord = (WORD) (HIBYTE << 8) | LOBYTE;
         wordOffset = wordOffset + 2;
         if (firstWord)
         {
            value = curWord & word_masks[16 - startBit];

            firstWord = false;
            lenLeft -= WORD_LENGTH - startBit;
         }
         else
         {
            value = (value << WORD_LENGTH) | curWord;
            lenLeft -= WORD_LENGTH;
         }
      }

      if (lenLeft > 0)
      {
         unsigned char HIBYTE = (unsigned char) buffer[wordOffset];
         unsigned char LOBYTE = (unsigned char) buffer[wordOffset+1];
         curWord = (WORD) (HIBYTE << 8) | LOBYTE;
         value = (value << lenLeft) | (curWord >> (WORD_LENGTH - lenLeft));
      }
   }

   return value;
}

Diameter::Diameter(m_Packet *pkt)
{
    unsigned int  avpCode;
    unsigned int avpLength;
    char *dMsg = pkt->pay_load;
    int caplen = pkt->getCapLen();
    
    msgLength = extractDword(dMsg, 0, 8, 24);
    commandFlag = extractDword(dMsg, 2, 0, 8);
    cc = extractDword(dMsg, 2, 8, 24);
    appId = extractDword(dMsg, 4, 0, 32);
    hopIdentifier = extractDword(dMsg, 6, 0, 32);

    int msgRemaining = msgLength - 20;
    int avpStartWord = 10;

     while(msgRemaining > 0)
     { 
        if(caplen < (avpStartWord + 32))
             break; 
         avpCode = extractDword(dMsg, avpStartWord, 0, 32);
         avpLength = extractDword(dMsg, avpStartWord + 2, 8, 24);

         if(avpLength <= 0)
             break;

         switch (avpCode)
         {
             case 264:
                 //originHost = new char[(avpLength - 4)];
                 //originHost = memcpy(originHost, (void *)(dMsg + ((avpStartWord + 4)*2)), (avpLength-8));
                 break;

             case 268:
                 resCode = extractDword(dMsg, avpStartWord + 4, 0, (avpLength - 8) * 8);
                 break;

             case 416:
                 reqType = extractDword(dMsg, avpStartWord + 4, 0, (avpLength - 8) * 8);
                 break;
         }

         int roundoff =0;
         int reminder = avpLength%4;
         if(reminder)
             roundoff = 4-reminder;

         msgRemaining -= avpLength + roundoff;
         avpStartWord += (avpLength+roundoff)/2;
     }

     request = commandFlag & 0x80;
    if(request)
    {
       request = 1;
    }
    else
    {
       request = 0;
    }
}


void Diameter::printPkt()
{
    std::cout << "Abhinay:: msgLength is:" << msgLength << std::endl;
    std::cout << "Abhinay:: commandFlag is:" << commandFlag << std::endl;
    std::cout << "Abhinay:: cc is:" << cc << std::endl;
    std::cout << "Abhinay:: appId is:" << appId << std::endl;
    std::cout << "Abhinay:: hopIdentifier is:" << hopIdentifier << std::endl;
    //std::cout << "Abhinay:: originHost is:" << originHost << std::endl;
    std::cout << "Abhinay:: resCode is:" << resCode << std::endl;
    std::cout << "Abhinay:: ReqType is:" << reqType << std::endl;
    std::cout << "Abhinay:: Timestamp is:" << timeStamp << std::endl;
}
