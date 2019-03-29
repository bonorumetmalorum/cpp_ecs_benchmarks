#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <cstdint>
#include <entt.hpp>
#include <iostream>
#include <fstream>

#define NUM_EVENTS 2

//i have 4mb l3 cache, 2 x 256 kb l2 cache and 2x 32 Kb l1 data cache
// http://www.cpu-world.com/CPUs/Core_i7/Intel-Core%20i7-3667U%20Mobile%20processor.html

struct R{
    float x = 100.0; //32 bits
};

struct W1{
    float x = 100.0;
};


struct W2{
    float x = 100.0;
};

struct W3{
    float x = 100.0;
};

void systemw1(entt::Registry<u_int64_t> &registery){
    auto view = registery.view<R, W1>();

    for(auto entity : view){
        auto &r = view.get<R>(entity);
        auto &w1 = view.get<W1>(entity);
        w1.x = r.x;
    }
}

void systemw2(entt::Registry<u_int64_t> &registery){
    auto view = registery.view<R, W2>();

    for(auto entity : view){
        auto &r = view.get<R>(entity);
        auto &w2 = view.get<W2>(entity);
        w2.x = r.x;
    }
}

void handle_error (int retval)
{
     printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
     exit(1);
}

void test_entt_10000_empty () {
    entt::Registry<u_int64_t> registry;
    for(std::uint64_t i = 0; i < 10000L; i++) {
        registry.create();
    }
}

entt::Registry<u_int64_t> test_entt_with_components(long num_entities)
{
    entt::Registry<u_int64_t> registry;
    for(std::uint64_t i = 0; i < num_entities; i++) {
        auto entity = registry.create();
        registry.assign<R>(entity);
        registry.assign<W1>(entity);
        registry.assign<W2>(entity);
        
    }
    return registry;
}


void add_entities(entt::Registry<uint64_t> &reg, long noe) {
    for(std::uint64_t i = 0; i < noe; i++){
        auto entt = reg.create();
        reg.assign<R>(entt);
        reg.assign<W1>(entt);
        reg.assign<W2>(entt);
        reg.assign<W3>(entt);
    }
}

//g++ -O0-I../lib/entt-2.7.3/src/entt/ main.cpp -lpapi
int main(int argc, char *argv[]){

    //create event set
    int Events[NUM_EVENTS] = {PAPI_L1_DCM, PAPI_L2_DCM};
    //before operation
    long long start[NUM_EVENTS] = {0,0};
    //after operation
    long long stop[NUM_EVENTS] = {0,0};
    //result = stop[i] - start[i]
    long long values[NUM_EVENTS] = {0,0};
    int num_hwcntrs = 0;

    //Initialize the PAPI library and get the number of counters available
    if ((num_hwcntrs = PAPI_num_counters()) <= PAPI_OK){
        handle_error(1);
    }

    std::cout << "papi init ok" << std::endl;

    //need to make a tests for 10000 entities
    std::cout << "Constructing 10000 entities" << std::endl;

    PAPI_start_counters(Events, NUM_EVENTS); //start counters

    PAPI_read_counters(start, NUM_EVENTS);

    test_entt_10000_empty();

    PAPI_stop_counters(stop, NUM_EVENTS);

    std::ofstream empty("../data/10000empty.dat");

    for(std::uint8_t i = 0; i < NUM_EVENTS; i++){
        printf("%llu stop - %llu start\n", stop[i], start[i]);
        values[i] = stop[i] - start[i];
    }

    empty << "0 " << "\"L1 data cache miss\" " << values[0] << std::endl;
    empty << "1 " << "\"L2 data cache miss\" " << values[1] << std::endl;
    empty.close();

    printf("%llu L1 cache misses and %llu L2 cache misses\n", values[0], values[1]);


    std::cout << "Constructing 10000 entities with components" << std::endl;

    std::ofstream withcomponents("../data/withcomponents.dat");

    PAPI_start_counters(Events, NUM_EVENTS);

    PAPI_read_counters(start, NUM_EVENTS);

    auto reg = test_entt_with_components(10000L);

    PAPI_stop_counters(stop, NUM_EVENTS);

    for(std::uint8_t i = 0; i < NUM_EVENTS; i++){
        printf("%llu stop - %llu start\n", stop[i], start[i]);
        values[i] = stop[i] - start[i];
    }

    withcomponents << "0 " << "\"L1 data cache miss\" " << values[0] << std::endl;
    withcomponents << "1 " << "\"L2 data cache miss\" " << values[1] << std::endl;
    withcomponents.close();

    printf("%llu L1 cache misses and %llu L2 cache misses\n", values[0], values[1]);

    std::cout << "Constructing 10000 entities and updating sequentially" << std::endl;

    std::ofstream update("../data/update.dat");

    auto reg1 = test_entt_with_components(10000L);

    PAPI_start_counters(Events, NUM_EVENTS);

    PAPI_read_counters(start, NUM_EVENTS);
    
    systemw1(reg1);
    systemw2(reg1);

    PAPI_stop_counters(stop, NUM_EVENTS);
   
    for(std::uint8_t i = 0; i < NUM_EVENTS; i++){
        printf("%llu stop - %llu start\n", stop[i], start[i]);
        values[i] = stop[i] - start[i];
    }

    update << "0 " << "\"L1 data cache miss\" " << values[0] << std::endl;
    update << "1 " << "\"L2 data cache miss\" " << values[1] << std::endl;
    update.close();

    printf("%llu L1 cache misses and %llu L2 cache misses\n", values[0], values[1]);

    //cpu cycles vs creating entities
    int EventsClock[1] = {PAPI_REF_CYC};
    long long stopRamp[1] = {0};
    std::ofstream cyclesvscreation("../data/cyclesvscreation.dat");
    long long NUM_ENT = 10000;
    long STEP = 100;
    entt::Registry<uint64_t> reg2;
    printf("%lli", NUM_ENT);
    for(uint64_t i = 0; i <= NUM_ENT; i = i+STEP){
        PAPI_start_counters(EventsClock, 1);
        add_entities(reg2, STEP);
        PAPI_stop_counters(stopRamp, 1);
        printf("%llu cpu clock\n", stopRamp[0]);
        cyclesvscreation << i << " " << stopRamp[0] <<std::endl;
    }
    cyclesvscreation.close();
    
    //cpu cycles vs increasing reads
    stopRamp[1] = {0};
    std::ofstream cyclesvsreads("../data/cyclesvsreads.dat");
    long long NUM_ENTreads = 10000;
    long STEPreads = 1;
    entt::Registry<uint64_t> reg3 = test_entt_with_components(NUM_ENT);
    auto view = reg3.view<R>();
    for(uint64_t i = 0; i < NUM_ENTreads; i = i+STEPreads){
        PAPI_start_counters(EventsClock, 1);
        auto &var = view.get(i);
        PAPI_stop_counters(stopRamp, 1);
        printf("%llu stop\n", stopRamp[0]);
        cyclesvsreads << i << " " << stopRamp[0] <<std::endl;
    }
    cyclesvsreads.close();

    //cpu cycles vs increasing writes
    stopRamp[1] = {0};
    std::ofstream cyclesvswrites("../data/cyclesvswrites.dat");
    long long NUM_ENTwrites = 10000;
    long STEPwrites = 1;
    entt::Registry<uint64_t> reg4 = test_entt_with_components(NUM_ENT);
    auto view1 = reg4.view<W1>();
    for(uint64_t i = 0; i < NUM_ENTwrites; i = i+STEPwrites){
        PAPI_start_counters(EventsClock, 1);
        view1.each([](auto entity, W1 &w1){w1.x += 100;});
        PAPI_stop_counters(stopRamp, 1);
        printf("%llu stop\n", stopRamp[0]);
        cyclesvswrites << i << " " << stopRamp[0] <<std::endl;
    }
    cyclesvswrites.close();     
}