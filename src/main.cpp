/******************************************************************************

Author: Cameron Chrobocinski
Date of Development: Summer 2019
Purpose: Personal Project

I developed this code while working on a project for TechSource Inc. that
required me to research Monte Carlo integration. The purpose of this code was to
help me get a better understanding of the processes necessary for Monte Carlo
integration, and was never meant to be viewed or used professionally.

*******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

int main()
{
    int size, dim;  //both hold user input

    cout << "Number of points needed from this LHC simulation: ";
    cin >> size;
    cout << "Number of dimensions in the 'Hypercube': ";
    cin >> dim;
    
    const int SIZE = size, DIMENSION = dim; //constants that hold user input
    float ratio[DIMENSION]; //holds the scale of each dimension
    float dimScale[DIMENSION][2];   //holds the lower and upper bounds of each dimension
    char choice;    //holds user input
    bool flop[DIMENSION];   //1 = dimension can use floating points, 0 = dimension uses integers only
    bool valid;  //keeps track of do-while validity
    
    do{ //do-while sets dimensional values for "flop"
        valid = true;
        
        cout << "Set random floating-point use for each dimension.\n" <<
                "0. No dimensions add random floating point values < 1\n" <<
                "1. All dimensions add random floating-point values < 1\n" <<
                "2. Customize each dimension's individual use\n";
        cin >> choice;
        
        switch(choice){
            case '0':   //case 0 removes any randomness in the output
                for(int i = 0; i < DIMENSION; i++){ //for loop sets all values of "flop" to 0
                    flop[i] = 0;
                }
                break;
            case '1':   //case 1 allows for randomness in output
                for(int i = 0; i < DIMENSION; i++){ //for loop sets all values of "flop" to 1
                    flop[i] = 1;
                }
                break;
            case '2':   //case 2 allows the user to pick which dimensions don't allow for randomness in output
                char choice2;
                bool valid2;
                do{ //do-while allows for individual customization of dimensional flop use
                    valid2 = true;
                    
                    for(int i = 0; i < DIMENSION; i++){ //for loop asks for and sets "flop" values
                        cout << "Allow dimension " << i + 1 << " to add random floating-point\n" <<
                                "numbers < 1? 0 = NO, 1 = YES\n";
                        cin >> choice2;
                        
                        switch(choice2){
                            case '0':   //case 0 removes randomness in current dimension's output
                                flop[i] = 0;
                                break;
                            case '1':   //case 1 allows for randomness in current dimension's output
                                flop[i] = 1;
                                break;
                            default:    //default triggers while condition
                                valid2 = false;
                                break;
                        }
                    }
                }while(!valid2);
                break;
            default:    //default triggers while condition
                valid = false;
                break;
        }
    }while(!valid);
    
    do{ //do-while sets scale of each dimension
        valid = true;
        
        cout << "Alter scale of any dimension(s)?\n" <<
                "0. No\n" <<
                "1. Yes\n";
        cin >> choice;
        
        switch(choice){
            case '0':   //0 sets a 1:1 scale
                for(int i = 0; i < DIMENSION; i++){
                    dimScale[i][0] = 0;
                    dimScale[i][1] = SIZE;
                }
                break;
            case '1':   //1 allows the user to manually input bounds for each dimension
                for(int i = 0; i < DIMENSION; i++){
                    cout << "Lower bound of dimension " << i + 1 << ": ";
                    cin >> dimScale[i][0];
                    
                    cout << "Upper bound of dimension " << i + 1 << ": ";
                    cin >> dimScale[i][1];
                }
                break;
            default:    //default repeats the process
                valid = false;
                break;
        }
    }while(!valid);
    
    float points[SIZE][DIMENSION];  //stores coordinates
    srand(time(NULL));  //seeds random generator
    
    for(int j = 0; j < DIMENSION; j++){ //for loop populates points array
        int range[SIZE];    //"range" is the range of values 0 to SIZE
        //the formula in the following statement is as follows:
        //possible range of values / SIZE
        ratio[j] = (dimScale[j][1] - dimScale[j][0]) / static_cast<float>(SIZE);
        
        for(int k = 0; k < SIZE; k++){  //for loop populates range array
            range[k] = k;
        }
        
        for(int i = 0; i < SIZE; i++){  //for loop generates random unique selection from range
            int temp = rand() % SIZE;   //temporary var to generate random index in range
            float decimal = ((rand() % 100) / (100.0 * (1 / ratio[j]))) * static_cast<float>(flop[j]); //holds random addition to value
            
            while(range[temp] == -1){   //while loop ensures unique selection
                temp = rand() % SIZE;
            }
            
            
            points[i][j] = range[temp] + decimal;    //assigns current index of "points" the value of "range[temp]" plus a random decimal value
            points[i][j] *= ratio[j];   //adjust value of "points" for range of possible values
            points[i][j] += dimScale[j][0];     //adjust value of "points" for starting point of possible values
            range[temp] = -1;   //selected index is marked as selected
        }
    }
    
    cout << "\nCoordinates\n" <<
              "-----------\n";
    
    for(int i = 0; i < SIZE; i++){  //for loop prints coordinates
        for(int j = 0; j < DIMENSION; j++){
            //the following line is legacy code, left behind for reference
            //cout << points[i][j] * ratio[j] + dimScale[j][0] << " ";
            cout << points[i][j] << " ";
        }
        cout << endl;
    }
    
    for(int i = 0; i < DIMENSION; i++){
        float mean = 0;  //"mean" holds average of a given dimension
        float variance = 0;   //"variance" holds the variance thereof
        float stdDev = 0; //"stdDev" holds the standard deviation thereof
        
        cout << "\nAnalysis of dimension " << i + 1 << endl <<
                "-----------------------\n";
        
        for(int j = 0; j < SIZE; j++){  //for loop adds all values of a dimension to "mean" var
            mean += points[j][i];
        }
        mean /= SIZE;   //divides value of "mean" by SIZE
        cout << "Mean: " << mean << endl;
        
        for(int j = 0; j < SIZE; j++){  //for loop adds the value of (value-of-current-dimensional-index - mean) for each value in a dimension
            variance += points[j][i] - mean;
        }
        variance *= variance;   //squares value of "variance"
        variance /= SIZE - 1;    //divides value of "variance" by SIZE - 1 (I'm using the sample variance formula here)
        cout << "Variance: " << variance << endl;
        
        stdDev = sqrt(variance);    //gives "stdDev" the value of the squared root of "variance"
        cout << "Standard Deviation: " << stdDev << endl;
    }

    return 0;
}




