#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../circuits/order_finding.h"

#include "../utils/utils.h"
#include "../utils/logger.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

int ITERATIONS = 1;

int execute_shor(int64 N, Logger *logger) {
    
    char buffer[255];
    sprintf(buffer, "\n\n---------------- STARTING SHOR WITH N = %ld ----------------\n\n", N);
    logger_message(logger, "SHOR", buffer);

    if(N%2==0) {
        sprintf(buffer, "%ld = %ld * %ld\n", N, N/2, (int64)2);
        logger_message(logger, "SHOR", buffer);
        return EXIT_SUCCESS;
    }

    while(true) { // Will exit with returns
        int64 a = rand()%(N-1)+1;
        if(gcd(a, N) != 1) {
            //printf("%ld = %ld * %ld\n", N, (int64)gcd(a, N), N/gcd(a, N));
            continue;
        }

        //a is now a coprime with N
        logger_message(logger, "SHOR", "STARTING ORDER FINDING\n");
        int64 r = order_finding(a, N, ITERATIONS, logger);

        sprintf(buffer, "ORDER FINDING : r = %ld\n", r);
        logger_message(logger, "SHOR", buffer);
        if(r%2==1) {
            logger_message(logger, "SHOR", "R IS NOT PAIR, RESTARTING\n\n");
            continue;
        }

        int64 ar2 = fexp_mod(a, r/2, N);
        int64 g = gcd(ar2 + 1, N);
        if(g != 1 && g != N) {

            sprintf(buffer, "%ld = %ld * %ld\n", N, g, N/g);
            logger_message(logger, "SHOR", buffer);
            return EXIT_SUCCESS;
        }
        else {
            sprintf(buffer, "TRIVIAL G (%ld) FOUND, RESTARTING\n\n", g);
            logger_message(logger, "SHOR", buffer);
        }
    }
}

int main(/*int argc, char *argv[]*/) {
    srand(time(NULL));
    
    int threads = omp_get_max_threads();
    omp_set_num_threads(threads);
    ITERATIONS = threads;

    //if(argc < 2) {fprintf(stderr, "./shor <N>"); return EXIT_FAILURE;}
    //int64 N = atoll(argv[1]);

    Logger *logger = logger_create("Shor execution");

    int MAX = 55;
    double valeur[55] = {
        15, 21, 33, 35, 39, 45, 51, 55, 57, 63, 
        65, 69, 75, 85, 87, 91, 93, 95, 99, 105, 
        111, 115, 117, 119, 123, 129, 133, 135, 141, 183, 
        205, 273, 319, 411, 469, 525, 603, 615, 621, 623, 
        629, 655, 665, 693, 725, 731, 747, 749, 759, 775, 
        791, 813, 831, 843, 849
    };
    for(int i = 0; i < MAX; i++) {
        int64 N = valeur[i];
        if(execute_shor(N, logger) == EXIT_FAILURE) return EXIT_FAILURE;
    }

    logger_free(logger);

    return 1;
}
