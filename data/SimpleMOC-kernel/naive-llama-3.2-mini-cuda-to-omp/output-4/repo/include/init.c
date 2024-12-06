#include <omp.h>

// Function to build a table of exponential values for linear interpolation
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

    #pragma omp parallel for reduction(+:table.values[2*N])
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

    return table;
}