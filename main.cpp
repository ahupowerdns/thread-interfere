#include <iostream>
#include <stdint.h>
using std::cout;
using std::endl;
using std::pair;
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "atomic_counter.hpp"
typedef pair<unsigned int, unsigned int> range_t;

AtomicCounter  __attribute__((aligned(32)))  g_countAtomic;
uint32_t g_count;

void* doCountNaive(void *p)
{
    range_t *range = (range_t*)p;
    
    for(unsigned int n = range->first ; n != range->second; ++n)
    {
        if(!(n%2)) 
            ++g_count;
    }
    return 0; 
}

void* doCountAtomic(void *p)
{
    range_t *range = (range_t*)p;
    
    for(unsigned int n = range->first ; n != range->second; ++n)
    {
        if(!(n%2))
            ++g_countAtomic;
    }
    return 0; 
}

void* doCountNoShare(void *p)
{
    range_t *range = (range_t*)p;
    uint32_t count = 0;
    for(unsigned int n = range->first ; n != range->second; ++n)
    {
        if(!(n%2))
            ++count;
    }
    return (void*)(uint64_t)count; 
}

uint32_t volatile __attribute__((aligned(64))) g_count1;
uint32_t volatile g_count2;

void* doCountFalseShare1(void *p)
{
    range_t *range = (range_t*)p;
    for(unsigned int n = range->first ; n != range->second; ++n)
    {
        if(!(n%2))
            ++g_count1;
    }
    return 0;
}

void* doCountFalseShare2(void *p)
{
    range_t *range = (range_t*)p;
    for(unsigned int n = range->first ; n != range->second; ++n)
    {
        if(!(n%2))
            ++g_count2;
    }
    return 0;
}


class StopWatch
{
public:
    explicit StopWatch(const char* what) : d_what(what)
    {
        gettimeofday(&d_start, 0);
    }
    ~StopWatch() 
    {
        struct timeval now;
        gettimeofday(&now, 0);
        
        double diff = (now.tv_sec - d_start.tv_sec) + (now.tv_usec - d_start.tv_usec)/1000000.0;
        cout << "'"<<d_what<< "' run took " << diff <<" seconds"<<endl;
    }
private:
    struct timeval d_start;
    std::string d_what;
};

 
int main (int argc, const char * argv[])
{
    range_t* range = new range_t(0,1000000000U);
    {
        StopWatch sw("Single threaded count");
        doCountNaive(range);
    }
    cout<<"Single threaded count is: "<<g_count<<endl;

    g_count = 0;
    
    range_t* range1 = new range_t(0U, 500000000U), *range2=new range_t(500000000U, 1000000000U);
    void* res1, *res2;
    
    pthread_t tid1, tid2;
    
    {
        StopWatch sw("Naive count");
        pthread_create(&tid1, 0, doCountNaive, (void*)range1);
        pthread_create(&tid2, 0, doCountNaive, (void*)range2);
        pthread_join(tid1, &res1);
        pthread_join(tid2, &res2);
    }
    cout<<"Naive count is: "<<g_count<<endl;
    
    {
        StopWatch sw("Atomic count");
        pthread_create(&tid1, 0, doCountAtomic, (void*)range1);
        pthread_create(&tid2, 0, doCountAtomic, (void*)range2);
        pthread_join(tid1, &res1);
        pthread_join(tid2, &res2);
    }
    cout<<"Atomic count is: " << g_countAtomic << endl;
   
    {
        StopWatch sw("NoShare count");
        pthread_create(&tid1, 0, doCountNoShare, (void*)range1);
        pthread_create(&tid2, 0, doCountNoShare, (void*)range2);
        pthread_join(tid1, &res1);
        pthread_join(tid2, &res2);
        
    }
    cout<<"NoShare count is: " << (uint64_t)res1 + (uint64_t)res2 << endl;
    
    {
        StopWatch sw("False share count");
        pthread_create(&tid1, 0, doCountFalseShare1, (void*)range1);
        pthread_create(&tid2, 0, doCountFalseShare2, (void*)range2);
        pthread_join(tid1, &res1);
        pthread_join(tid2, &res2);
    }
    cout<<"False share count is: " << g_count1 + g_count2 << endl;
    cout<< (char*)&g_count2 - (char*)&g_count1 << endl;
    cout << ((uint64_t)(char*)&g_count1) %64 <<endl;
    return 0;
}

