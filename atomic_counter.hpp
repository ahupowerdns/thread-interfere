//
//  Header.h
//  Hello World
//
//  Created by bert on 1/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Hello_World_Header_h
#define Hello_World_Header_h


// lifted from boost, with thanks
class AtomicCounter
{
public:
    
    explicit AtomicCounter( unsigned int v = 0) : value_( v ) {}
    
    unsigned int operator++()
    {
        return atomic_exchange_and_add( &value_, +1 ) + 1;
    }
    
    unsigned int operator--()
    {
        return atomic_exchange_and_add( &value_, -1 ) - 1;
    }
    
    operator unsigned int() const
    {
        return atomic_exchange_and_add( &value_, 0);
    }
    
private:
    AtomicCounter(AtomicCounter const &);
    AtomicCounter &operator=(AtomicCounter const &);
    
    mutable unsigned int value_;
    
    // the below is necessary because __sync_fetch_and_add is not universally available on i386.. I 3> RHEL5. 
#if defined( __GNUC__ ) && ( defined( __i386__ ) || defined( __x86_64__ ) )
    static int atomic_exchange_and_add( unsigned int * pw, int dv )
    {
        // int r = *pw;
        // *pw += dv;
        // return r;
        
        int r;
        
        __asm__ __volatile__
        (
         "lock\n\t"
         "xadd %1, %0":
         "+m"( *pw ), "=r"( r ): // outputs (%0, %1)
         "1"( dv ): // inputs (%2 == %1)
         "memory", "cc" // clobbers
         );
        
        return r;
    }
#else 
    static int atomic_exchange_and_add( unsigned int * pw, int dv )
    {
        return __sync_fetch_and_add(pw, dv);
    }
#endif
};



#endif
