/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "types.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct SimpleNN
{
    f32*    weights;
    s32     count;
};

struct RbfNN
{
    SimpleNN    simpleNN;
    f32**       centres;
    f32*        centresArray;
};

struct MlpNN
{
    f32**   weights;
    s32*    layerNeuronCount;
    s32     layerCount;
};

struct NNData
{
    f32**   input;
    f32*    inputArray;
    f32*    output;
    s32     rowCount;
    s32     columnCount;
};


s32 MAX_COUNT           = 999999;
f32 OUTPUT_THRESHOLD    = 0.1f;

bool PERCEPTRON         = true;
bool LMS                = true;
bool RBF                = true;
bool MLP                = true;


void buildRBFNN( RbfNN* rbfNN,
                 SimpleNN simpleNN,
                 s32 centreDimCount )
{
    rbfNN->simpleNN     = simpleNN;

    s32 centreCount     = simpleNN.count - 1;
    rbfNN->centresArray = new f32[ centreCount * centreDimCount ];
    rbfNN->centres      = new f32*[ centreCount ];

    for( s32 i = 0 ; i < centreCount; ++i )
        rbfNN->centres[ i ] = &rbfNN->centresArray[ i * centreDimCount ];
}

void buildNNData( NNData* nnData,
                  s32 rowCount,
                  s32 columnCount )
{
    nnData->inputArray  = new f32[ rowCount * columnCount ];
    nnData->input       = new f32*[ rowCount ];
    for( s32 i = 0; i < rowCount; ++i )
        nnData->input[ i ] = &nnData->inputArray[ i * columnCount ];

    nnData->output        = new f32[ rowCount ];

    nnData->rowCount      = rowCount;
    nnData->columnCount   = columnCount;
}

f32 feedForwardPerceptron( SimpleNN* simpleNN,
                           f32* data )
{
    //bias
    f32 sum = simpleNN->weights[ 0 ];
    for( s32 i = 1; i < simpleNN->count; ++i )
        sum += simpleNN->weights[ i ] * data[ i - 1 ];

    if( sum > 0 )
        return 1.0f;
    else
        return -1.0f;
}

bool learnPerceptron( SimpleNN* simpleNN,
                      NNData* data,
                      f32 learningRate )
{
    s32 weightCount = simpleNN->count;
    ASSERT( weightCount == data->columnCount + 1 );

    s32 rowCount    = data->rowCount;

    for( s32 count = 0; count < MAX_COUNT; ++count )
    {
        s32 currpattern = rand() % rowCount;

        f32 output = feedForwardPerceptron( simpleNN, data->input[ currpattern ] );
        if( output != data->output[ currpattern ] )
        {
            f32 inputMultiplier = learningRate * ( data->output[ currpattern ] - output );

            //bias
            simpleNN->weights[ 0 ] += inputMultiplier;
            for( s32 i = 1; i < weightCount; ++i )
                simpleNN->weights[ i ] += inputMultiplier * data->input[ currpattern ] [ i - 1 ];
        }

        bool done = true;
        for( s32 i = 0; i < rowCount; ++i )
        {
            f32 output = feedForwardPerceptron( simpleNN, data->input[ i ] );
            if( output != data->output[ i ] )
            {
                done = false;
                break;
            }
        }

        if( done )
            return true;
    }

    return false;
}

void verifyTrainingPerceptron( SimpleNN* simpleNN,
                               NNData* data,
                               f32 learningRate )
{
    memset( simpleNN->weights, 0, simpleNN->count * sizeof( f32 ) );

    if( learnPerceptron( simpleNN, data, learningRate ) )
    {
        for( s32 i = 0; i < data->rowCount; ++i )
        {
            f32 output = feedForwardPerceptron( simpleNN, data->input[ i ] );
            if( data->output[ i ] != output )
            {
                printf( "Failed!\n" );
                return;
            }
        }

        printf( "Successfully trained: %f, %f, %f\n",
                simpleNN->weights[ 0 ],
                simpleNN->weights[ 1 ],
                simpleNN->weights[ 2 ] );
    }
    else
    {
        printf( "Failed!\n" );
    }
}

f32 feedForwardLMS( SimpleNN* simpleNN,
                    f32* data )
{
    //bias
    f32 sum = simpleNN->weights[ 0 ];
    for( s32 i = 1; i < simpleNN->count; ++i )
        sum += simpleNN->weights[ i ] * data[ i - 1 ];

    f32 result = tanhf( sum );
    return result;
}

bool learnLMS( SimpleNN* simpleNN,
               NNData* data,
               f32 learningRate )
{
    s32 weightCount = simpleNN->count;
    ASSERT( weightCount == data->columnCount + 1 );

    s32 rowCount    = data->rowCount;

    for( s32 count = 0; count < MAX_COUNT; ++count )
    {
        s32 currpattern = rand() % rowCount;

        f32 output = feedForwardLMS( simpleNN, data->input[ currpattern ] );
        if( fabs( data->output[ currpattern ] - output ) > OUTPUT_THRESHOLD )
        {
            f32 inputMultiplier = learningRate * ( data->output[ currpattern ] - output ) * ( 1.0f - output * output );

            //bias
            simpleNN->weights[ 0 ] += inputMultiplier;
            for( s32 i = 1; i < weightCount; ++i )
                simpleNN->weights[ i ] += inputMultiplier * data->input[ currpattern ] [ i - 1 ];
        }

        bool done = true;
        for( s32 i = 0; i < rowCount; ++i )
        {
            f32 output = feedForwardLMS( simpleNN, data->input[ i ] );
            if( fabs( data->output[ i ] - output ) > OUTPUT_THRESHOLD )
            {
                done = false;
                break;
            }
        }

        if( done )
            return true;
    }

    return false;
}

void verifyTrainingLMS( SimpleNN* simpleNN,
                        NNData* data,
                        f32 learningRate )
{
    memset( simpleNN->weights, 0, simpleNN->count * sizeof( f32 ) );

    if( learnLMS( simpleNN, data, learningRate ) )
    {
        for( s32 i = 0; i < data->rowCount; ++i )
        {
            f32 output = feedForwardLMS( simpleNN, data->input[ i ] );
            if( fabs( data->output[ i ] - feedForwardLMS( simpleNN, data->input[ i ] ) ) > OUTPUT_THRESHOLD )
            {
                printf( "Failed!\n" );
                return;
            }
        }

        printf( "Successfully trained: %f, %f, %f\n",
                simpleNN->weights[ 0 ],
                simpleNN->weights[ 1 ],
                simpleNN->weights[ 2 ] );
    }
    else
    {
        printf( "Failed!\n" );
    }
}

void calculateRBFInput( RbfNN* rbfNN,
                        f32* data,
                        s32 dataCount,
                        f32* rbfInput )
{
    s32 inputCount = rbfNN->simpleNN.count - 1;
    for( s32 i = 0; i < inputCount; ++i )
    {
        rbfInput[ i ] = 0.0f;
        for( s32 j = 0; j < dataCount; ++j )
        {
            f32 result = data[ j ] - rbfNN->centres[ i ][ j ];
            result *= result;
            rbfInput[ i ] += result;
        }

        rbfInput[ i ] = expf( -rbfInput[ i ] );
    }
}

f32 feedForwardRBF( RbfNN* rbfNN,
                    f32* data,
                    s32 dataCount )
{
    s32 inputCount  = rbfNN->simpleNN.count - 1;
    f32* newData    = new f32[ inputCount ];
    calculateRBFInput( rbfNN, data, dataCount, newData );
    f32 result      = feedForwardLMS( &rbfNN->simpleNN, newData );

    delete [] newData;

    return result;
}

bool learnRBF( RbfNN* rbfNN,
               NNData* data,
               f32 learningRate )
{
    SimpleNN* simpleNN  = &rbfNN->simpleNN;
    s32 weightCount     = simpleNN->count;

    s32 rowCount        = data->rowCount;
    s32 columnCount     = data->columnCount;

    f32* newData        = new f32[ weightCount - 1 ];

    bool done           = true;

    for( s32 count = 0; count < MAX_COUNT; ++count )
    {
        s32 currpattern = rand() % rowCount;

        f32 output = feedForwardRBF( rbfNN, data->input[ currpattern ], columnCount );
        if( fabs( data->output[ currpattern ] - output ) > OUTPUT_THRESHOLD )
        {
            f32 inputMultiplier = learningRate * ( data->output[ currpattern ] - output ) * ( 1.0f - output * output );

            calculateRBFInput( rbfNN, data->input[ currpattern ], columnCount, newData );

            //bias
            simpleNN->weights[ 0 ] += inputMultiplier;
            for( s32 i = 1; i < weightCount; ++i )
                simpleNN->weights[ i ] += inputMultiplier * newData[ i - 1 ];
        }

        done = true;
        for( s32 i = 0; i < rowCount; ++i )
        {
            f32 output = feedForwardRBF( rbfNN, data->input[ i ], columnCount );
            if( fabs( data->output[ i ] - output ) > OUTPUT_THRESHOLD )
            {
                done = false;
                break;
            }
        }

        if( done )
            break;
    }

    delete [] newData;

    return done;
}

void verifyTrainingRBF( RbfNN* rbfNN,
                        NNData* data,
                        f32 learningRate )
{
    SimpleNN* simpleNN = &rbfNN->simpleNN;
    memset( simpleNN->weights, 0, simpleNN->count * sizeof( f32 ) );

    if( learnRBF( rbfNN, data, learningRate ) )
    {
        for( s32 i = 0; i < data->rowCount; ++i )
        {
            f32 output = feedForwardRBF( rbfNN, data->input[ i ], data->columnCount );
            if( fabs( data->output[ i ] - output ) > OUTPUT_THRESHOLD )
            {
                printf( "Failed!\n" );
                return;
            }
        }

        printf( "Successfully trained: %f, %f, %f\n",
                simpleNN->weights[ 0 ],
                simpleNN->weights[ 1 ],
                simpleNN->weights[ 2 ] );
    }
    else
    {
        printf( "Failed!\n" );
    }
}

s32 neuronOutputCountMLP( MlpNN* mlpNN )
{
    s32 result = 0;

    for( s32 i = 0; i < mlpNN->layerCount; ++i )
        result += mlpNN->layerNeuronCount[ i ];

    return result;
}

f32 feedForwardMLP( MlpNN* mlpNN,
                    f32* data,
                    f32* neuronOutput )
{
    s32 neuronIndex = 0;

    for( s32 i = 0; i < mlpNN->layerNeuronCount[ 0 ]; ++i, ++neuronIndex )
        neuronOutput[ neuronIndex ] = data[ i ];

    for( s32 i = 1; i < mlpNN->layerCount; ++i )
    {
        s32 startNeuronIndex = neuronIndex - mlpNN->layerNeuronCount[ i - 1 ];
        for( s32 j = 0; j < mlpNN->layerNeuronCount[ i ]; ++j, ++neuronIndex )
        {
            s32 weightIndex = j * ( mlpNN->layerNeuronCount[ i - 1 ] + 1 );

            neuronOutput[ neuronIndex ] = mlpNN->weights[ i - 1 ][ weightIndex ];
            for( s32 k = 1; k < mlpNN->layerNeuronCount[ i - 1 ] + 1; ++k )
                neuronOutput[ neuronIndex ] += mlpNN->weights[ i - 1 ][ weightIndex + k ] * neuronOutput[ startNeuronIndex + k - 1 ];
            neuronOutput[ neuronIndex ] = tanhf( neuronOutput[ neuronIndex ] );
        }
    }

    f32 result = neuronOutput[ neuronIndex - 1 ];
    return result;
}

bool learnMLP( MlpNN* mlpNN,
               NNData* data,
               f32 learningRate,
               f32 decay )
{
    s32 rowCount            = data->rowCount;

    s32 neuronOutputCount   = neuronOutputCountMLP( mlpNN );
    f32* neuronOutput       = new f32[ neuronOutputCount ];
    s32 errorCount          = neuronOutputCount - mlpNN->layerNeuronCount[ 0 ];
    f32* error              = new f32[ errorCount ];

    bool done               = false;

    for( s32 count = 0; count < MAX_COUNT; ++count )
    {
        s32 currpattern = rand() % rowCount;

        f32 output = feedForwardMLP( mlpNN, data->input[ currpattern ], neuronOutput );
        if( fabs( data->output[ currpattern ] - output ) > OUTPUT_THRESHOLD )
        {
            //error in output layer
            s32 errorIndex = errorCount - 1;
            error[ errorIndex ] = ( data->output[ currpattern ] - output ) * ( 1 - output * output );
            --errorIndex;

            //error in other layers
            for( s32 i = mlpNN->layerCount - 2; i >= 1; --i )
            {
                s32 layerNeuronCount = mlpNN->layerNeuronCount[ i ];
                s32 outputStartIndex = errorIndex + 1;
                for( s32 j = layerNeuronCount - 1; j >= 0; --j, --errorIndex )
                {
                    error[ errorIndex ] = 0;
                    for( s32 k = 0; k < mlpNN->layerNeuronCount[ i + 1 ]; ++k )
                    {
                        s32 weightIndex     = ( k * ( layerNeuronCount + 1 ) ) + j + 1;
                        f32 weight          = mlpNN->weights[ i ][ weightIndex ];
                        error[ errorIndex ] += error[ outputStartIndex + k ] * weight;
                    }

                    error[ errorIndex ]     /= ( f32 ) mlpNN->layerNeuronCount[ i + 1 ];

                    f32 currNeuronOutput    = neuronOutput[ outputStartIndex + mlpNN->layerNeuronCount[ 0 ] - layerNeuronCount + j ];
                    error[ errorIndex ]     *= 1 - currNeuronOutput * currNeuronOutput;
                }
            }

            errorIndex = 0;

            s32 neuronIndex = 0;
            for( s32 i = 1; i < mlpNN->layerCount; ++i, neuronIndex += mlpNN->layerNeuronCount[ i - 2 ] )
            {
                for( s32 j = 0; j < mlpNN->layerNeuronCount[ i ]; ++j, ++errorIndex )
                {
                    s32 weightIndex = j * ( mlpNN->layerNeuronCount[ i - 1 ] + 1 );

                    mlpNN->weights[ i - 1 ][ weightIndex ] += learningRate * error[ errorIndex ];
                    mlpNN->weights[ i - 1 ][ weightIndex ] *= 1 - decay;
                    for( s32 k = 1; k < mlpNN->layerNeuronCount[ i - 1 ] + 1; ++k )
                    {
                        mlpNN->weights[ i - 1 ][ weightIndex + k ] += learningRate * error[ errorIndex ] * neuronOutput[ neuronIndex + k - 1 ];
                        mlpNN->weights[ i - 1 ][ weightIndex + k ] *= 1 - decay;
                    }
                }
            }
        }

        done = true;
        for( s32 i = 0; i < rowCount; ++i )
        {
            f32 output = feedForwardMLP( mlpNN, data->input[ i ], neuronOutput );
            if( fabs( data->output[ i ] - output ) > OUTPUT_THRESHOLD )
            {
                done = false;
                break;
            }
        }

        if( done )
            break;
    }

    delete [] neuronOutput;
    delete [] error;

    return done;
}

void verifyTrainingMLP( MlpNN* mlpNN,
                        NNData* data,
                        f32 learningRate,
                        f32 decay )
{
    for( s32 i = 0; i < mlpNN->layerCount - 1; ++i )
    {
        for( s32 j = 0; j < mlpNN->layerNeuronCount[ i + 1 ] * ( mlpNN->layerNeuronCount[ i ] + 1 ); ++j )
            mlpNN->weights[ i ][ j ] = ( f32 ) ( ( rand() % 200 ) - 100 ) / 100.0f;
    }

    if( learnMLP( mlpNN, data, learningRate, decay ) )
    {
        f32* neuronOutput = new f32[ neuronOutputCountMLP( mlpNN ) ];
        for( s32 i = 0; i < data->rowCount; ++i )
        {
            f32 output = feedForwardMLP( mlpNN, data->input[ i ], neuronOutput );
            if( fabs( data->output[ i ] - output ) > OUTPUT_THRESHOLD )
            {
                printf( "Failed!\n" );
                return;
            }
        }
        delete [] neuronOutput;

        printf( "Successfully trained\n" );

        for( s32 i = 0; i < mlpNN->layerCount - 1; ++i )
        {
            printf( "Layer: %d\n", i + 1 );

            for( s32 j = 0; j < mlpNN->layerNeuronCount[ i + 1 ]; ++j )
            {
                printf( "Neuron: %d", j );

                s32 startIndex = j * ( mlpNN->layerNeuronCount[ i ] + 1 );
                for( s32 k = 0; k < mlpNN->layerNeuronCount[ i ] + 1; ++k )
                    printf( ", %f", mlpNN->weights[ i ][ startIndex + k ] );

                printf( "\n" );
            }
        }
    }
    else
    {
        printf( "Failed!\n" );
    }
}

int main( int argc,
          char** argv )
{
    s32 inputDim = 2;

    NNData andTable = {};
    buildNNData( &andTable, 4, inputDim );
    andTable.input[ 0 ][ 0 ] =  1;      andTable.input[ 0 ][ 1 ] =  1;      andTable.output[ 0 ] =  1;
    andTable.input[ 1 ][ 0 ] =  1;      andTable.input[ 1 ][ 1 ] = -1;      andTable.output[ 1 ] = -1;
    andTable.input[ 2 ][ 0 ] = -1;      andTable.input[ 2 ][ 1 ] =  1;      andTable.output[ 2 ] = -1;
    andTable.input[ 3 ][ 0 ] = -1;      andTable.input[ 3 ][ 1 ] = -1;      andTable.output[ 3 ] = -1;

    NNData orTable = {};
    buildNNData( &orTable, 4, inputDim );
    orTable.input[ 0 ][ 0 ] =  1;       orTable.input[ 0 ][ 1 ] =  1;       orTable.output[ 0 ] =  1;
    orTable.input[ 1 ][ 0 ] =  1;       orTable.input[ 1 ][ 1 ] = -1;       orTable.output[ 1 ] =  1;
    orTable.input[ 2 ][ 0 ] = -1;       orTable.input[ 2 ][ 1 ] =  1;       orTable.output[ 2 ] =  1;
    orTable.input[ 3 ][ 0 ] = -1;       orTable.input[ 3 ][ 1 ] = -1;       orTable.output[ 3 ] = -1;

    NNData xorTable = {};
    buildNNData( &xorTable, 4, inputDim );
    xorTable.input[ 0 ][ 0 ] =  1;      xorTable.input[ 0 ][ 1 ] =  1;      xorTable.output[ 0 ] = -1;
    xorTable.input[ 1 ][ 0 ] =  1;      xorTable.input[ 1 ][ 1 ] = -1;      xorTable.output[ 1 ] =  1;
    xorTable.input[ 2 ][ 0 ] = -1;      xorTable.input[ 2 ][ 1 ] =  1;      xorTable.output[ 2 ] =  1;
    xorTable.input[ 3 ][ 0 ] = -1;      xorTable.input[ 3 ][ 1 ] = -1;      xorTable.output[ 3 ] = -1;

    NNData nutsTable = {};
    buildNNData( &nutsTable, 6, inputDim );
    nutsTable.input[ 0 ][ 0 ] = 2.2f;   nutsTable.input[ 0 ][ 1 ] = 1.4f;   nutsTable.output[ 0 ] =  1;
    nutsTable.input[ 1 ][ 0 ] = 1.5f;   nutsTable.input[ 1 ][ 1 ] = 1.0f;   nutsTable.output[ 1 ] =  1;
    nutsTable.input[ 2 ][ 0 ] = 0.6f;   nutsTable.input[ 2 ][ 1 ] = 0.5f;   nutsTable.output[ 2 ] =  1;
    nutsTable.input[ 3 ][ 0 ] = 2.3f;   nutsTable.input[ 3 ][ 1 ] = 2.0f;   nutsTable.output[ 3 ] = -1;
    nutsTable.input[ 4 ][ 0 ] = 1.3f;   nutsTable.input[ 4 ][ 1 ] = 1.5f;   nutsTable.output[ 4 ] = -1;
    nutsTable.input[ 5 ][ 0 ] = 0.3f;   nutsTable.input[ 5 ][ 1 ] = 1.0f;   nutsTable.output[ 5 ] = -1;

    if( PERCEPTRON )
    {
        printf( "Perceptron\n" );
        s32 weightCount     = inputDim + 1;
        SimpleNN simpleNN   = { new f32[ weightCount ], weightCount };
        f32 learningRate    = 0.1f;

        printf( "And Truth Table:\n" );
        verifyTrainingPerceptron( &simpleNN, &andTable, learningRate );
        printf( "\n" );

        printf( "Or Truth Table:\n" );
        verifyTrainingPerceptron( &simpleNN, &orTable, learningRate );
        printf( "\n" );

        printf( "Xor Truth Table:\n" );
        verifyTrainingPerceptron( &simpleNN, &xorTable, learningRate );
        printf( "\n" );

        printf( "Nuts Truth Table:\n" );
        verifyTrainingPerceptron( &simpleNN, &nutsTable, learningRate );
        printf( "\n\n" );
    }

    if( LMS )
    {
        printf( "LMS\n" );
        s32 weightCount     = inputDim + 1;
        SimpleNN simpleNN   = { new f32[ weightCount ], weightCount };
        f32 learningRate    = 0.1f;

        printf( "And Truth Table:\n" );
        verifyTrainingLMS( &simpleNN, &andTable, learningRate );
        printf( "\n" );

        printf( "Or Truth Table:\n" );
        verifyTrainingLMS( &simpleNN, &orTable, learningRate );
        printf( "\n" );

        printf( "Xor Truth Table:\n" );
        verifyTrainingLMS( &simpleNN, &xorTable, learningRate );
        printf( "\n" );

        printf( "Nuts Truth Table:\n" );
        verifyTrainingLMS( &simpleNN, &nutsTable, learningRate );
        printf( "\n\n" );
    }

    if( RBF )
    {
        printf( "RBF\n" );
        s32 centreCount = 2;

        s32 weightCount = centreCount + 1;
        SimpleNN simpleNN = { new f32[ weightCount ], weightCount };
        RbfNN rbfNN = {};
        buildRBFNN( &rbfNN, simpleNN, inputDim );

        f32 learningRate = 0.01f;

        printf( "And Truth Table:\n" );
        rbfNN.centres[ 0 ][ 0 ] = 1;
        rbfNN.centres[ 0 ][ 1 ] = 1;

        rbfNN.centres[ 1 ][ 0 ] = 1;
        rbfNN.centres[ 1 ][ 1 ] = 1;

        verifyTrainingRBF( &rbfNN, &andTable, learningRate );
        printf( "\n" );

        printf( "Or Truth Table:\n" );
        rbfNN.centres[ 0 ][ 0 ] = -1;
        rbfNN.centres[ 0 ][ 1 ] = -1;

        rbfNN.centres[ 1 ][ 0 ] = -1;
        rbfNN.centres[ 1 ][ 1 ] = -1;

        verifyTrainingRBF( &rbfNN, &orTable, learningRate );
        printf( "\n" );

        printf( "Xor Truth Table:\n" );
        rbfNN.centres[ 0 ][ 0 ] = -1;
        rbfNN.centres[ 0 ][ 1 ] = -1;

        rbfNN.centres[ 1 ][ 0 ] = 1;
        rbfNN.centres[ 1 ][ 1 ] = 1;

        verifyTrainingRBF( &rbfNN, &xorTable, learningRate );
        printf( "\n" );

        printf( "Nuts Truth Table:\n" );
        rbfNN.centres[ 0 ][ 0 ] = 0.5f;
        rbfNN.centres[ 0 ][ 1 ] = 1.5f;

        rbfNN.centres[ 1 ][ 0 ] = 1.5f;
        rbfNN.centres[ 1 ][ 1 ] = 0.5f;

        verifyTrainingRBF( &rbfNN, &nutsTable, learningRate );
        printf( "\n\n" );
    }

    if( MLP )
    {
        printf( "MLP\n" );
        s32 layerCount              = 3;
        ASSERT( layerCount > 2 );
        MlpNN mlpNN                 = { new f32*[ layerCount - 1 ], new s32[ layerCount ], layerCount };
        mlpNN.layerNeuronCount[ 0 ] = inputDim;
        mlpNN.layerNeuronCount[ 1 ] = 2;
        mlpNN.layerNeuronCount[ 2 ] = 1;
        ASSERT( mlpNN.layerNeuronCount[ layerCount - 1 ] == 1 );

        for( s32 i = 0; i < layerCount - 1; ++i )
            mlpNN.weights[ i ] = new f32[ mlpNN.layerNeuronCount[ i + 1 ] * ( mlpNN.layerNeuronCount[ i ] + 1 ) ];

        f32 learningRate            = 0.15f;
        f32 decay                   = 0.00005f;

        printf( "And Truth Table:\n" );
        verifyTrainingMLP( &mlpNN, &andTable, learningRate, decay );
        printf( "\n" );

        printf( "Or Truth Table:\n" );
        verifyTrainingMLP( &mlpNN, &orTable, learningRate, decay );
        printf( "\n" );

        printf( "Xor Truth Table:\n" );
        verifyTrainingMLP( &mlpNN, &xorTable, learningRate, decay );
        printf( "\n" );

        printf( "Nuts Truth Table:\n" );
        verifyTrainingMLP( &mlpNN, &nutsTable, learningRate, decay );
        printf( "\n\n" );
    }

    system( "pause" );
    return 0;
}