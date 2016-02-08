#include <iostream>
#include <string> 
#include <vector>
#include <unistd.h>
#include <cstdlib>


void allamem( void );
void delemem( void );
void allamem_2( void );
void delemem_2( void );
class inti;
std::vector<inti *> intmem; 
std::vector<inti *> intmem_2;

 
class garbagecollector {

private :

   std::vector<void*> garbage; //TODO map with time
   std::vector<void*> arrayGarbage; //TODO map with time
   static garbagecollector * singleINST;
   static int GarbageCount ; 
   static int ArrGarbageCount  ;
  

private:
   garbagecollector(){
      GarbageCount = 0 ;
      ArrGarbageCount = 0; 
       // TODO timer to clear 15 mins older than current time
  }

public: 
  ~garbagecollector(); 

   static garbagecollector* getGarhandler(){
       if(!singleINST)
           singleINST = new garbagecollector(); 

       return (singleINST); 
   }


   void addTogarbage(void * ptr){
      garbage.push_back(ptr);
      GarbageCount++;
   }

   void addToArrGarbage(void * ptr){
      arrayGarbage.push_back(ptr);
      ArrGarbageCount++;
   }
  
   void * getaplaceholder() {
       void * ret = garbage.back();
       garbage.pop_back();
       GarbageCount--;
       return ret;
   }
   void * getarrayplaceholder() {
       void * ret = arrayGarbage.back();
       arrayGarbage.pop_back();
       ArrGarbageCount--;
       return ret;
   }

   void clearoldgarbage(){}// TODO with timer

   static inline int getGarbCount(){return GarbageCount;}
   static inline int getArrGarbCount() {return ArrGarbageCount;} 
};
int garbagecollector::GarbageCount = 0;
int garbagecollector::ArrGarbageCount = 0;
garbagecollector* garbagecollector::singleINST = NULL;


class inti {
private :

  int * actptr; 

public : 

  inti(){
    actptr = new int ();
 }

  ~inti (){
     if(actptr)
     delete actptr; 
       }
 
  void callConst()
  {
     ::inti();
   } 

  void calldes()
  {
    if(actptr)
     delete  actptr;
       
   } 
 inline void * operator new(size_t size ) throw (std::bad_alloc); 
 inline void * operator new[](size_t size) throw (std::bad_alloc);
 inline void operator delete  ( void* ptr );
 inline void operator delete[]( void * ptr, std::size_t sz);
};


int main(int count , char ** argv)
{
 int summa;
  while (1)
  {
   allamem(); 
   // std::cout << "Out of allocation\n" << std::endl; 
     usleep(100);
   delemem_2();
   allamem_2();
   usleep(100);
   delemem(); 
  }
  return 0; 
}

void allamem( void )
{
  for ( int i = 0 ; i < 10000; i++)
  {
   inti * temp = new inti[1000]; 
   intmem.push_back(temp);  
  }
}

void delemem( void )
{
  // if(intmem.size() <= 0)
   //return; 
   std::vector<inti *>::iterator iter, end;
   for(iter = intmem.begin(), end = intmem.end() ; iter != end; ++iter) {
    delete [](*iter);}
   intmem.clear();
}


void allamem_2( void )
{
  for ( int i = 0 ; i < 10000; i++)
  {
   inti * temp = new inti (); 
   intmem_2.push_back(temp);  
  }
}

void delemem_2( void )
{
   std::vector<inti *>::iterator iter, end;
 //  std::cout << "Size of vector " << intmem_2.size() << std::endl ; 
 //  if(intmem_2.size() <= 0)
 //  return; 

   for(iter = intmem_2.begin(), end = intmem_2.end() ; iter != end; ++iter) {
   // std::cout << "deleteing" << std::endl ;  
    delete (*iter);}
   intmem_2.clear();
}

inline void * inti::operator new(size_t size) throw (std::bad_alloc) 
{
 void *p;

 if( garbagecollector::getGarbCount() == 0)
 {
	 while ((p = malloc(sizeof (inti))) == NULL)
		 if (p == NULL)
		 {       // report no memory
			 static const std::bad_alloc nomem;
			 throw(nomem);
		 }
 }
 else
 {
	 inti *ptr;
	 ptr = (inti*)garbagecollector::getGarhandler()->getaplaceholder();
	 if(ptr)
	 {
		 p = realloc(ptr, sizeof(inti));
		 if(!p)
		 {
		      static const std::bad_alloc nomem;
                      throw(nomem); 
		 }       
	 }else
	 {
	          static const std::bad_alloc nomem;
                  throw(nomem);	              
	 }  
 }
    inti *tmp = (inti *)p; 
    //tmp->actptr = new int [10];
    tmp->callConst();
  return (tmp); 
}

inline void inti::operator delete(void *ptr)
{ 
  if(ptr)
  garbagecollector::getGarhandler()->addTogarbage(ptr);
  //TODO call destructor
}


inline void * inti::operator new[](size_t size) throw (std::bad_alloc)
{
   void *p;
   void *firstEl;
   if(garbagecollector::getArrGarbCount() == 0)
   {
    while ((p = malloc(size * sizeof(inti))) == NULL)
    if (p == NULL)
     {       // report no memory
        static const std::bad_alloc nomem;
        throw(nomem);
     }
   }
     else
   {
         inti *ptr;
         ptr = (inti*)garbagecollector::getGarhandler()->getarrayplaceholder();
         if(ptr)
         {
                 p = realloc(ptr,size * sizeof(inti));
                 if(!p)
                 {
                      static const std::bad_alloc nomem;
                      throw(nomem);
                 }
         }else
         {
                  static const std::bad_alloc nomem;
                  throw(nomem);
         }
   }

    firstEl = p ;
   
    for (int i = 0; i < size; i++)
    {
       p+=(i * sizeof(inti));
       inti *tmp = (inti *)(p);
       //tmp->actptr = new int [10];
       tmp->callConst();
    }
    //std::cout << "Allocation finished\n" << std::endl; 
      inti *tmp = (inti *) firstEl;    
      return (tmp);
}

inline void inti::operator delete [](void *ptr, std::size_t sz)
{
  if(ptr)
  garbagecollector::getGarhandler()->addToArrGarbage(ptr);
  //TODO call destructor sz/(sizeof(inti)) times by moving the ptr
  //std::cout << "Delete called for array size = " << sz/(sizeof(inti)) << std::endl ;
}
