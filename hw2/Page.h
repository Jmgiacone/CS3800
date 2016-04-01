//
// Created by jordan on 3/29/16.
//

#ifndef PAGE_H
#define PAGE_H


class Page
{
  private:
    const int ownerProgram, logicalPageNum;
    bool useBit;
    unsigned long long timestamp;
  public:
    Page(int owner, int logicalPage, unsigned long long stamp) : ownerProgram(owner), logicalPageNum(logicalPage), useBit(false), timestamp(stamp) {}
    int getOwnerProgram() {return ownerProgram;}
    int getLogicalPageNum() {return logicalPageNum;}
    bool getUseBit() {return useBit;}
    unsigned long long getTimestamp() {return timestamp;}
    void setUseBit(bool bit) {useBit = bit;}
    void setTimestamp(unsigned long long stamp) {timestamp = stamp;}

};


#endif
