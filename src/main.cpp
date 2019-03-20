#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <cstdint>
#include <entt.hpp>
#include <iostream>
#include <fstream>

#define NUM_EVENTS 2

struct R{
    float x;
};

struct W1{
    float x; 
};


struct W2{
    float x;
};

void systemw1(entt::DefaultRegistry &registery){
    auto view = registery.view<R, W1>();

    for(auto entity : view){
        auto &r = view.get<R>(entity);
        auto &w1 = view.get<W1>(entity);
        w1.x = r.x;
    }
}

void systemw2(entt::DefaultRegistry &registery){
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
    entt::DefaultRegistry registry;
    for(std::uint64_t i = 0; i < 10000L; i++) {
        registry.create();
    }
}

entt::DefaultRegistry test_entt_10000_with_components () {
    entt::DefaultRegistry registry;
    for(std::uint64_t i = 0; i < 10000L; i++) {
        auto entity = registry.create();
        registry.assign<R>(entity);
        registry.assign<W1>(entity);
        registry.assign<W2>(entity);
    }
    return registry;
}

//g++ -I../lib/entt-2.7.3/src/entt/ main.cpp -lpapi
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

    auto reg = test_entt_10000_with_components();

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

    auto reg1 = test_entt_10000_with_components();

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

    //ramping up performance test
    std::ofstream rampup("../data/rampup.dat");
    entt::DefaultRegistry reg2;
    for(int64_t i = 0; i <= 10000; i = i+1000){
        reg2 = test_entt_10000_with_components();
        PAPI_start_counters(Events, NUM_EVENTS);
        PAPI_read_counters(start, NUM_EVENTS);
        systemw1(reg1);
        systemw2(reg1);
        PAPI_stop_counters(stop, NUM_EVENTS);
        printf("%llu stop - %llu start\n", stop[0], start[0]);
        // printf("%llu stop - %llu start\n", stop[1], start[1]);
        rampup << i << " " << stop[0] - start[0] << " " << stop[1] - start[1] <<std::endl;
    }
    rampup.close();
        
}