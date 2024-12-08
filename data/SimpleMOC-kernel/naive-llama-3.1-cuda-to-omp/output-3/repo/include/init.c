#include <omp.h>

// ... (other includes and definitions)

Table buildExponentialTable( void )
{
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;   

    // compute number of arry values
    //int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );
    int N = 353; 

    // compute spacing
    float dx = maxVal / (float) N;

#pragma omp target data map(to:table.values[:2*N]) map(from:dx,maxVal,N)
    {
        for( int n = 0; n < N; n++ )
        {
            // compute slope and y-intercept for ( 1 - exp(-x) )
            float exponential = exp( - n * dx );
            table.values[ 2*n ] = - exponential;
            table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
        }

        // assign data to table
        table.dx = dx;
        table.maxVal = maxVal - table.dx;
        table.N = N;
    }

    return table;
}

void __ompCheckError( const char *file, const int line )
{
#ifdef OPENMP_ERROR_CHECK
    omp_err_t err = omp_get_error();
    if ( omp_no_error != err )
    {
        fprintf( stderr, "ompCheckError() failed at %s:%i : %s\n",
                 file, line, omp_strerror(err) );
        exit( -1 );
    }
 
    // More careful checking. However, this will affect performance.
    // Comment away if needed.
    err = omp_get_last_status();
    if(omp_success != err)
    {
        fprintf(stderr, "ompCheckError() with sync failed at %s:%i : %s\n",
                file, line, omp_strerror(err) );
        exit(-1);
    }
#endif
 
    return;
}