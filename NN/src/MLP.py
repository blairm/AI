import math
import random


def neuronOutputCountMLP( layerNeuronCount=[] ) -> int:
    result:int = 0

    for i in layerNeuronCount:
        result += i

    return result

def feedForwardMLP( layerNeuronCount=[], weights=[], inputs=[], neuronOutput=[] ) -> float:
    neuronIndex:int = 0
    layerCount:int = len( layerNeuronCount )

    for i in inputs:
        neuronOutput[ neuronIndex ] = i
        neuronIndex += 1

    for i in range( 1, layerCount ):
        startNeuronIndex:int = neuronIndex - layerNeuronCount[ i - 1 ]

        for j in range( layerNeuronCount[ i ] ):
            weightIndex:int = j * ( layerNeuronCount[ i - 1 ] + 1 )

            neuronOutput[ neuronIndex ] = weights[ weightIndex ][ i - 1 ]
            for k in range( 1, layerNeuronCount[ i - 1 ] + 1 ):
                neuronOutput[ neuronIndex ] += weights[ weightIndex + k ][ i - 1 ] * neuronOutput[ startNeuronIndex + k - 1 ]
            neuronOutput[ neuronIndex ] = math.tanh( neuronOutput[ neuronIndex ] )

            neuronIndex += 1

    return neuronOutput[ neuronIndex - 1 ]

def lo_feedForwardMLP( lo_layerNeuronCount=[], weights=[], inputs=[], *args ) -> float:
    layerNeuronCount = []
    for i in range( len( lo_layerNeuronCount ) ):
        layerNeuronCount.append( int( lo_layerNeuronCount[ i ][ 0 ] ) )

    neuronOutputCount:int   = neuronOutputCountMLP( layerNeuronCount )
    neuronOutput            = list( range( neuronOutputCount ) )

    result:float = feedForwardMLP( layerNeuronCount, weights, inputs[ 0 ], neuronOutput )
    return result

def lo_learnMLP( lo_layerNeuronCount=[], lo_weights=[], inputs=[], lo_outputs=[], maxCount:int=0, learningRate:float=0, decay:float=0, threshold:float=0, *args ) -> []:
    layerCount = len( lo_layerNeuronCount )
    layerNeuronCount = []
    for i in range( layerCount ):
        layerNeuronCount.append( int( lo_layerNeuronCount[ i ][ 0 ] ) )

    weights = []
    for weightsRow in lo_weights:
        weights.append( list( weightsRow ) )

    outputs = []
    for i in range( len( lo_outputs ) ):
        outputs.append( int( lo_outputs[ i ][ 0 ] ) )

    neuronOutputCount:int   = neuronOutputCountMLP( layerNeuronCount )
    neuronOutput            = list( range( neuronOutputCount ) )
    errorCount:int          = neuronOutputCount - layerNeuronCount[ 0 ]
    error                   = list( range( errorCount ) )

    rowCount:int            = len( outputs )

    for count in range( maxCount ):
        currPattern:int = random.randint( 0, rowCount - 1 )

        output:float = feedForwardMLP( layerNeuronCount, weights, inputs[ currPattern ], neuronOutput )
        if abs( outputs[ currPattern ] - output ) > threshold:
            #error in output layer
            errorIndex:int = errorCount - 1;
            error[ errorIndex ] = ( outputs[ currPattern ] - output ) * ( 1 - output * output )
            errorIndex -= 1

            #error in other layers
            for i in range( layerCount - 2, 0, -1 ):
                neuronCount:int = layerNeuronCount[ i ]
                outputStartIndex:int = errorIndex + 1

                for j in range( neuronCount - 1, -1, -1 ):
                    error[ errorIndex ] = 0

                    for k in range( layerNeuronCount[ i + 1 ] ):
                        weightIndex:int     = ( k * ( neuronCount + 1 ) ) + j + 1
                        weight:float        = weights[ weightIndex ][ i ]
                        error[ errorIndex ] += error[ outputStartIndex + k ] * weight

                    error[ errorIndex ]     /= float( layerNeuronCount[ i + 1 ] )

                    currNeuronOuput:float   = neuronOutput[ outputStartIndex + layerNeuronCount[ 0 ] - neuronCount + j ]
                    error[ errorIndex ]     *= 1 - currNeuronOuput * currNeuronOuput

                    errorIndex -= 1

            errorIndex = 0

            neuronIndex:int = 0
            for i in range( 1, layerCount ):
                for j in range( layerNeuronCount[ i ] ):
                    weightIndex:int = j * ( layerNeuronCount[ i - 1 ] + 1 )

                    weights[ weightIndex ][ i - 1 ] += learningRate * error[ errorIndex ]
                    weights[ weightIndex ][ i - 1 ] *= 1 - decay
                    for k in range( 1, layerNeuronCount[ i - 1 ] + 1 ):
                        weights[ weightIndex + k ][ i - 1 ] += learningRate * error[ errorIndex ] * neuronOutput[ neuronIndex + k - 1 ]
                        weights[ weightIndex + k ][ i - 1 ] *= 1 - decay

                    errorIndex += 1

                neuronIndex += layerNeuronCount[ i - 1 ]

        done:bool = True
        for i in range( rowCount ):
            output:float = feedForwardMLP( layerNeuronCount, weights, inputs[ i ], neuronOutput )
            if abs( outputs[ i ] - output ) > threshold:
                done = False
                break

        if done:
            break

    return weights