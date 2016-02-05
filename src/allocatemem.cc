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

 
class carbagecollector {

private :

   std::vector<void*> carbage; //TODO map with time
   std::vector<void*> arrayCarbage; //TODO map with time
   static carbagecollector * singleINST;
   static int CarbageCount ; 
   static int ArrCarbageCount  ;
  

private:
   carbagecollector(){
      CarbageCount = 0 ;
      ArrCarbageCount = 0; 
       // TODO timer to clear 15 mins older than current time
  }

public: 
  ~carbagecollector(); 

   static carbagecollector* getCarhandler(){
       if(!singleINST)
           singleINST = new carbagecollector(); 

       return (singleINST); 
   }


   void addTocarbage(void * ptr){
      carbage.push_back(ptr);
      CarbageCount++;
   }

   void addToArrCarbage(void * ptr){
      arrayCarbage.push_back(ptr);
      ArrCarbageCount++;
   }
  
   void * getaplaceholder() {
       void * ret = carbage.back();
       carbage.pop_back();
       CarbageCount--;
       return ret;
   }
   void * getarrayplaceholder() {
       void * ret = arrayCarbage.back();
       arrayCarbage.pop_back();
       ArrCarbageCount--;
       return ret;
   }

   void clearoldcarbage(){}// TODO with timer

   static inline int getCarbCount(){return CarbageCount;}
   static inline int getArrCarbCount() {return ArrCarbageCount;} 
};

//carbagecollector::CarbageCount = 0;
//carbagecollector::ArrCarbageCount = 0;

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
 //inline void operator delete[]( void * ptr);
};


int main(int count , char ** argv)
{
 int summa;
  while (1)
  {
   allamem(); 
   // std::cout << "Out of allocation\n" << std::endl; 
    // sleep(9);
   delemem_2();
   allamem_2();
  //   sleep(9);
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

 if( carbagecollector::getCarbCount() == 0)
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
	 ptr = (inti*)carbagecollector::getCarhandler()->getaplaceholder();
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
  carbagecollector::getCarhandler()->addTocarbage(ptr);
  //TODO call destructor
}


inline void * inti::operator new[](size_t size) throw (std::bad_alloc)
{
   void *p;
   void *firstEl;

   while ((p = malloc(size * sizeof(inti))) == NULL)
    if (p == NULL)
     {       // report no memory
        static const std::bad_alloc nomem;
        throw(nomem);
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


