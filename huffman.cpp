/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2015  Adán G. Medrano-Chávez ryuuba@gmail.com
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#include "huffman.h"

bool Huffman::loadFile()
{
   char c;
   Code code;
   std::ifstream ifs(filename.c_str(), std::ifstream::in);
   std::cout << "Opening " << filename << std::endl;
   if(ifs.is_open())
   {
      while (ifs.good()) 
      {
         c=ifs.get();
         if(c == EOF) break;
         text.push(c);
         if(characterMap.find(c)==characterMap.end())
         {
            characterMap[c]=1;
            codeMap[c] = code;
         }
         else
            characterMap[c]++;
      }
      ifs.close();
      return true;
   }
   else
   {
      std::cerr << "File " << filename << ' '
                << "couldn't be opened while trying load it" << std::endl; 
      return false;
   }
}

void Huffman::fillQueue()
{
   HuffmanNode node;
   for(auto& pair : characterMap)
   {
      node.first=pair.first;
      node.second=pair.second;
      huffmanQueue.push(node);      
   }
   characterMap.clear();
}

void Huffman::computeHuffmanMap()
{
   HuffmanNode left, right, intermediate;
   while(huffmanQueue.size()>1)
   {
      left=huffmanQueue.top(); huffmanQueue.pop();
      right=huffmanQueue.top(); huffmanQueue.pop();
      for(char& k : left.first)
         codeMap[k].push_back(false);
      for(char& k : right.first)
         codeMap[k].push_back(true);
      intermediate.first=left.first+right.first;
      intermediate.second=left.second+right.second;
      huffmanQueue.push(intermediate);
   }
}

void Huffman::printHuffmanMap()
{
   std::cout << "Huffman code" << std::endl;
   for(auto& pair : codeMap)
      std::cout << pair.first << ' ' << pair.second.size() << std::endl;
}

bool Huffman::writeHuffmanMap()
{
   std::string tname = filename + ".har";
   std::ofstream ofs(tname.c_str(), std::ofstream::out);
   Packet packet;
   if(ofs.is_open())
   {
      ofs << text.size() << ' ' << codeMap.size();
      std::cout << "Text size: " << text.size() << ' '
      << "Map length: " << codeMap.size() << std::endl;
      for(auto& charCode : codeMap)
      {
         ofs << charCode.first //Imprime un caracter 'x'
                //imprime el tamaño del codigo del caracter 'x'
             << static_cast<unsigned char>(charCode.second.size());
         std::cout << charCode.first  << ' ' << charCode.second.size() << ' ';
         packet.clear(); //Elimina todos los bytes de un paquete
         //Convierte un codigo de longitud variable a un paquete de bytes
         toPacket(charCode.second, packet);
         //Imprime en el flujo de salida el conjunto de bytes correspondientes
         //a un codigo
         for(Byte& byte : packet)
         {
            ofs << toChar(byte);
            std::cout << byte;
         }
         std::cout << std::endl;
      }
      ofs.close();
      return true;
   }
   else
   {
      std::cerr << "Huffman Map couldn't be written"<<std::endl;
      return false;
   }
}

bool Huffman::encode()
{
   int bit=8;
   Byte byte;
   byte.reset();
   std::string tname = filename + ".har";
   //Abre un flujo de salida en modo append (escribe al final de un archivo)
   std::ofstream ofs(tname.c_str(), std::ofstream::app);
   if(ofs.is_open())
   {
      char c;
      while(!text.empty())
      {
         c=text.front();
         //Los codigos en codeMap estan invertidos
         for(int i = codeMap[c].size() - 1; i >= 0; i--)
         {
            //Accedes al bit i del codigo variable del caracter c
            codeMap[c].at(i) ? byte[--bit]=true : byte[--bit]=false;
            if(bit==0)
            {
               ofs << toChar(byte);
               //std::cout << byte << ' ';
               bit=8;
               byte.reset();
            }
         }
         text.pop();
      }
      if(bit>0)
         ofs << toChar(byte);
      //std::cout << byte << std::endl;
      ofs.close();
      return true;
   }
   else
   {
      std::cerr << "File couldn't be opened" <<std::endl;
      return false;
   }
}

unsigned char Huffman::toChar(Byte& bin) const
{
   return static_cast<unsigned char>(bin.to_ulong());
}

Huffman::Packet& Huffman::toPacket(Code& code, Packet& packet) const
{
   unsigned char c;
   int bit = 8, i;
   Byte byte;
   byte.reset();
   for(i = code.size() -1; i >= 0; i--)
   {
      code[i] ? byte[--bit] = true : byte[--bit] = false;
      if(bit == 0)
      {
         packet.push_back(byte);
         byte.reset();
         bit = 8;
      }
   }
   if(bit < 8)
      packet.push_back(byte);
   return packet;
}

Huffman::Code& Huffman::toCode(Packet& packet, Code& code, int codeLength) const
{
   int i, j, bits, length = codeLength;
   for(Byte& byte : packet)
   {
         length >= 8 ? bits = 0 : bits = 8-length%8;
         for(j = 7; j >= bits; j--)
         {
            byte[j] ? code.push_back(true) : code.push_back(false);
            byte[j] ? std::cout << '1' : std::cout << '0';
         }
         length = length - 8;
   }
   return code;
}

bool Huffman::compress(std::string& name)
{
   filename = name;
   if(loadFile())
   {
      fillQueue();
      computeHuffmanMap();
      if(writeHuffmanMap()) 
         if(encode())
            return true;
   }
   return false;
}

bool Huffman::decompress(std::string& name, int l)
{
   filename = name;
   limit = l;
   if(loadCodedFile())
   {
      std::cout << "File  " << name << " was loaded" << std::endl;
      if(decode())
         return true;
   }
   return false;
}

bool Huffman::loadCodedFile()
{
   char character, codePart, codeLength;
   unsigned codeMapLength, codePartLength;
   Code code;
   Packet packet;
   std::cout << "Opening " << filename << std::endl;
   std::ifstream ifs(filename.c_str(), std::ifstream::in);
   if(ifs.is_open())
   {
      ifs >> textSize;
      ifs >> codeMapLength;
      std::cout << "Text size: " << textSize << std::endl;
      std::cout << "Codemap length " << int(codeMapLength) << std::endl;
      for(int i = 0; i < codeMapLength; i++)
      {
         ifs >> std::noskipws; //no skip white spaces
         ifs >> character;
         ifs >> codeLength;
         std::cout << character << ' ' << int(codeLength) << ' ';
         for(int j = 0; j < ceil(double(codeLength)/8); j++)
         {
            unsigned char c;   
            ifs >> c;
            Byte byte(c);
            packet.push_back(byte);
         }
         toCode(packet, code, codeLength);
         std::cout << std::endl;
         decodeMap[code] = character;
         code.clear();
         packet.clear();
      }
      while(ifs.good())
         codedText.push(ifs.get());
      return true;
   }
   else
   {
      std::cerr << "File " << filename 
                << " couldn't be opened while trying to load it" << std::endl;
      return false;
   }
}

bool Huffman::decode()
{
   Code code;
   int bit = 8, numOfDecodedCharacters = 0;
   while(numOfDecodedCharacters < textSize)
   {         
      do
      {
         Byte byte(codedText.front());
         byte[--bit] ? code.push_back(true) : code.push_back(false);
         //code.back() ?  std::cout<<'1' : std::cout <<'0';
         if(bit == 0)
         {
            codedText.pop();
            bit = 8;
         }         
      }while (decodeMap.find(code) == decodeMap.end());
      //std::cout << ' ' << std::endl;
      //std::cout << decodeMap[code];
      text.push(decodeMap[code]);
      numOfDecodedCharacters++;
      code.clear();
   }
   std::cout << std::endl;
   printText();
   return false;
}

void Huffman::printText()
{
   while(!text.empty() && limit > 0)
   {
      std::cout << text.front();
      text.pop();
      limit--;
   }
   std::cout << std::endl;
}
