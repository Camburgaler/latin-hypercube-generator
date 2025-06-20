/******************************************************************************

Author: Cameron Chrobocinski
Date of Development: Summer 2019
Purpose: Personal Project

I developed this code while working on a project for TechSource Inc. that
required me to research Monte Carlo integration. The purpose of this code was to
help me get a better understanding of the processes necessary for Monte Carlo
integration, and was never meant to be viewed or used professionally.

Date: Summer 2025

Updated code to convert cin to cxxopts.
Added functionality to output to CSV.
Added ability to toggle verbose console logging.

*******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "cxxopts.hpp"
#include <string>
#include <vector>
#include <regex>
#include <functional>
#include <fstream>

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;

    do {
        pos = str.find(delimiter, prev); // get position of delimiter
        if (pos == std::string::npos)
            pos = str.length();
        std::string token = str.substr(prev, pos-prev);

        if (!token.empty()) 
            tokens.push_back(token);
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

std::pair<float, float> parseBounds(const std::string& input) {
    auto bounds = split(input, ":");
    if (bounds.size() != 2) 
        throw std::invalid_argument("Invalid base-scale format, use lower:upper");

    return {std::stof(bounds[0]), std::stof(bounds[1])};
}

std::vector<std::tuple<int, float, float>> parseOverrides(const std::string& input) {
    std::vector<std::tuple<int, float, float>> result;
    if (input.empty()) 
        return result;

    auto entries = split(input, ",");
    for (const auto& entry : entries) {
        auto parts = split(entry, ":");
        if (parts.size() != 3) 
            throw std::invalid_argument("Invalid scale override format, use dim:lower:upper");
            
        int dim = std::stoi(parts[0]);
        float low = std::stof(parts[1]);
        float high = std::stof(parts[2]);
        result.emplace_back(dim, low, high);
    }

    return result;
}

std::string optionKeyFormatter(const std::string& key) {
    if (key.size() > 0) {
        return key.substr(0, 1) + "," + key;
    } else {
        throw std::invalid_argument("Invalid option name");
    }
}

std::string sanitizeHeading(const std::string& header) {
    std::string sanitized = "";
    for (char c : header) {
        if (std::isalnum(c) || c == '_' || c == ' ') {
            sanitized += c;
        }
    }
    return sanitized;
}

std::vector<std::string> escapeHeadings(const std::vector<std::string> headings) {
    std::vector<std::string> escaped = headings;
    for (int i = 0; i < headings.size(); i++) {
        std::string h = sanitizeHeading(headings[i]);
        if (h.find_first_of(",\"\n\r") != std::string::npos) {
            std::string escapedHeading = h;
            std::string quoted = "\"";
            for (char c : escapedHeading) {
                if (c == '"') quoted += "\"\"";
                else quoted += c;
            }
            quoted += "\"";
            escaped[i] = quoted;
        }
    }
    return escaped;
}

template <class T>
bool vectorContains(const std::vector<T>& haystack, const T& needle) {
    return std::find(haystack.begin(), haystack.end(), needle) != haystack.end();
}

int main(int argc, char *argv[])
{
    // letters used: hndrbsfocv
    const std::string OPTION_NUMBER = "number";
    const std::string OPTION_DIMENSIONS = "dimensions";
    const std::string OPTION_RANDOM = "random";
    const std::string OPTION_BASE_SCALE = "base-scale";
    const std::string OPTION_SCALES = "scales";
    const std::string OPTION_FILE_OUTPUT = "file-output";
    const std::string OPTION_OUT_PATH = "out-path";
    const std::string OPTION_HEADINGS = "column-headings";
    const std::string OPTION_VERBOSE = "verbose";

    const std::string RANDOM_TRUE = "true";
    const std::string RANDOM_FALSE = "false";
    const std::string OUT_PATH_DEFAULT = "lhc.csv";

    cxxopts::Options options("lhc", "Latin Hypercube generator");

    options.add_options()
        (optionKeyFormatter(OPTION_NUMBER), "Required. Number of points", cxxopts::value<int>()->default_value("1000"))
        (optionKeyFormatter(OPTION_DIMENSIONS), "Required. Number of dimensions", cxxopts::value<int>()->default_value("1"))
        (optionKeyFormatter(OPTION_RANDOM), "Optional. Select randomness: '" + RANDOM_FALSE + "' = none, '" + RANDOM_TRUE + "' = all, or a comma-separated list of dimension indices", cxxopts::value<std::string>()->default_value("false"))
        (optionKeyFormatter(OPTION_BASE_SCALE), "Optional. Default scale for all dimensions in the form lower:upper", cxxopts::value<std::string>()->default_value("0:1"))
        (optionKeyFormatter(OPTION_SCALES), "Optional. Comma-separated dimension:lower:upper overrides", cxxopts::value<std::string>())
        (optionKeyFormatter(OPTION_FILE_OUTPUT), "Optional. Flag to toggle CSV file output")
        (optionKeyFormatter(OPTION_OUT_PATH), "Optional. File path for CSV output", cxxopts::value<std::string>()->default_value("lhc.csv"))
        (optionKeyFormatter(OPTION_HEADINGS), "Optional. Column names for CSV output (only alphanumeric and underscore characters)", cxxopts::value<std::string>())
        (optionKeyFormatter(OPTION_VERBOSE), "Optional. Flag to toggle verbose console logging")
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count(OPTION_NUMBER) == 0 || result.count(OPTION_DIMENSIONS) == 0) {
        throw std::invalid_argument("Missing required arguments");
        return 1;
    }

    int NUMBER_OF_POINTS = result[OPTION_NUMBER].as<int>();
    int NUMBER_OF_DIMENSIONS = result[OPTION_DIMENSIONS].as<int>();
    std::vector<std::string> random = split(result[OPTION_RANDOM].as<std::string>(), ",");
    std::pair<float, float> baseScale = parseBounds(result[OPTION_BASE_SCALE].as<std::string>());
    
    float ratio[NUMBER_OF_DIMENSIONS];              // holds the scale of each dimension
    float dimensionScales[NUMBER_OF_DIMENSIONS][2]; // holds the lower and upper bounds of each dimension
    bool valid;                                     // keeps track of do-while validity

    std::cout << "Generating " << NUMBER_OF_POINTS << " points in " << NUMBER_OF_DIMENSIONS << " dimensions.\n";

    // input validation for randomization
    if (random.size() > 1 || (random[0] != RANDOM_TRUE && random[0] != RANDOM_FALSE)) {    
        
        // input validation that there are not more dimensions identified for randomization than there are dimensions
        if (random.size() > NUMBER_OF_DIMENSIONS){
            throw std::invalid_argument("Invalid input. Too many dimensions randomized.");
            return 1;
        }

        for (int i = 0; i < random.size(); i++){
            int randomAsInt;
    
            // input validation that each dimension is an integer
            try{
                randomAsInt = std::stoi(random[i]);
            }catch(std::invalid_argument& e){
                throw std::invalid_argument("Invalid input. Dimension " + random[i] + " is not an integer.");
                return 1;
            }

            // input validation that there is no randomization for a dimension that does not exist
            bool randomGreaterThanDimensionsCount = randomAsInt >= NUMBER_OF_DIMENSIONS;
            bool randomLessThanZero = randomAsInt < 0;
            if (randomGreaterThanDimensionsCount || randomLessThanZero){
                throw std::invalid_argument("Invalid input. Dimension " + random[i] + " does not exist.");
                return 1;
            }

            // input validation that each dimension is unique
            for (int j = 0; j < i; j++){
                if (random[i] == random[j]){
                    throw std::invalid_argument("Invalid input. Dimension " + random[i] + " is duplicated.");
                    return 1;
                }
            }
        }
    }
    std::cout << "Random selection: ";
    for (int i = 0; i < random.size(); i++){
        std::cout << random[i] << " ";
    }
    std::cout << "\n";

    // input validation for file output
    std::ofstream out;
    std::vector<std::string> headings;
    if (result.count(OPTION_FILE_OUTPUT)) {
        std::string outDir = OUT_PATH_DEFAULT;
        if (result.count(OPTION_OUT_PATH)) {
            outDir = result[OPTION_OUT_PATH].as<std::string>();
        }


        // input validation that the file path is valid
        try {
            out.open(outDir, std::ios::out | std::ios::trunc);
        } catch (std::ofstream::failure& e) {
            throw std::invalid_argument("Invalid input. File path is invalid.");
            return 1;
        }

        // input validation that the file path is writable
        if (!out.is_open()) {
            throw std::invalid_argument("Invalid input. File path is not writable.");
            return 1;
        }

        std::cout << "File output path: " << outDir << "\n";

        // input validation for the headings
        if (result.count(OPTION_HEADINGS)) {
            headings = escapeHeadings(split(result[OPTION_HEADINGS].as<std::string>(), ","));
            if (headings.size() != NUMBER_OF_DIMENSIONS) {
                throw std::invalid_argument("Invalid number of headings. Received " + std::to_string(headings.size()) + ", expected " + std::to_string(NUMBER_OF_DIMENSIONS) + "\n");
                return 1;
            }
        } else {
            for (int i = 0; i < NUMBER_OF_DIMENSIONS; ++i) {
                headings.push_back("dim" + std::to_string(i));
            }
        }

        std::cout << "Headings: ";
        for (int i = 0; i < headings.size(); i++){
            std::cout << headings[i] << " ";
        }
        std::cout << "\n";
    }
    
    // set the base scale for each dimension
    for (int i = 0; i < NUMBER_OF_DIMENSIONS; ++i) {
        dimensionScales[i][0] = baseScale.first;
        dimensionScales[i][1] = baseScale.second;
    }

    std::cout << "Base scale: " << baseScale.first << ":" << baseScale.second << "\n";

    // set the lower and upper bounds for each customized dimension
    if (result.count(OPTION_SCALES)) {
        for (const auto& [i, low, high] : parseOverrides(result[OPTION_SCALES].as<std::string>())) {
            // input validation that the dimension index is valid
            if (i < 0 || i >= NUMBER_OF_DIMENSIONS) {
                throw std::invalid_argument("Invalid dimension index in --scale: " + std::to_string(i));
                return 1;
            }

            // set the lower and upper bounds
            dimensionScales[i][0] = low;
            dimensionScales[i][1] = high;

            std::cout << "Dimension " << i << " scale: " << low << ":" << high << "\n";
        }
    }
    
    float points[NUMBER_OF_POINTS][NUMBER_OF_DIMENSIONS];   //stores coordinates
    srand(time(NULL));                                      //seeds random generator
    
    // for loop populates points array
    for(int dimensionIndex = 0; dimensionIndex < NUMBER_OF_DIMENSIONS; dimensionIndex++){
        // the range of values 0 to SIZE 
        int range[NUMBER_OF_POINTS];    
        
        float upperBound = dimensionScales[dimensionIndex][1];
        float lowerBound = dimensionScales[dimensionIndex][0];
        ratio[dimensionIndex] = (upperBound - lowerBound) / static_cast<float>(NUMBER_OF_POINTS);
        
        // for loop populates range array
        for(int k = 0; k < NUMBER_OF_POINTS; k++){  
            range[k] = k;
        }
        
        // for loop generates random unique selection from range
        for(int pointIndex = 0; pointIndex < NUMBER_OF_POINTS; pointIndex++){  
            // temporary var to generate random index in range
            int temp = rand() % NUMBER_OF_POINTS;   
            float dimensionRatio = ratio[dimensionIndex];
            float inverseDimensionRatio = 1 / dimensionRatio;
            float decimal = (rand() % 100) / (100.0 * inverseDimensionRatio); //holds random addition to value
            
            if (
                (
                    random.size() > 1
                    && !vectorContains(random, std::to_string(dimensionIndex))
                ) 
                || random[0] == RANDOM_FALSE
            ) {
                decimal = 0;
            }
            
            // while loop ensures unique selection
            while(range[temp] == -1){   
                temp = rand() % NUMBER_OF_POINTS;
            }
            
            
            points[pointIndex][dimensionIndex] = range[temp] + decimal; // assigns current index of "points" the value of "range[temp]" plus a random decimal value
            points[pointIndex][dimensionIndex] *= dimensionRatio;       // adjust value of "points" for range of possible values
            points[pointIndex][dimensionIndex] += lowerBound;           // adjust value of "points" for starting point of possible values
            range[temp] = -1;                                           // selected index is marked as selected
        }
    }
    
    if (result.count(OPTION_VERBOSE)) {
        std::cout << "\nCoordinates\n" <<
                "-----------\n";
        
        // for loop prints coordinates
        for(int i = 0; i < NUMBER_OF_POINTS; i++){  
            for(int j = 0; j < NUMBER_OF_DIMENSIONS; j++){
                std::cout << points[i][j] << " ";
            }
            std::cout << std::endl;
        }
        
        for(int i = 0; i < NUMBER_OF_DIMENSIONS; i++){
            float mean = 0;     // "mean" holds average of a given dimension
            float variance = 0; // "variance" holds the variance thereof
            float stdDev = 0;   // "stdDev" holds the standard deviation thereof
            
            std::cout << "\nAnalysis of dimension " << i + 1 << std::endl <<
                    "-----------------------\n";
            
            // for loop adds all values of a dimension to "mean" var
            for(int j = 0; j < NUMBER_OF_POINTS; j++){  
                mean += points[j][i];
            }
            mean /= NUMBER_OF_POINTS; // divides value of "mean" by SIZE
            std::cout << "Mean: " << mean << std::endl;
            
            // for loop adds the value of (value-of-current-dimensional-index - mean) for each value in a dimension
            for(int j = 0; j < NUMBER_OF_POINTS; j++){  
                variance += points[j][i] - mean;
            }
            variance *= variance;   // squares value of "variance"
            variance /= NUMBER_OF_POINTS - 1; // divides value of "variance" by SIZE - 1 (I'm using the sample variance formula here)
            std::cout << "Variance: " << variance << std::endl;
            
            stdDev = sqrt(variance);    // gives "stdDev" the value of the squared root of "variance"
            std::cout << "Standard Deviation: " << stdDev << std::endl;
        }
    }

    // export headings and data to csv
    if (result.count(OPTION_FILE_OUTPUT)) {
        std::cout << "Writing to " << result[OPTION_OUT_PATH].as<std::string>() << std::endl;
        for (const std::string h : headings) {
            out << h;
            if (h != headings.back()) {
                out << ",";
            }
        }
        out << std::endl;
        for (int i = 0; i < NUMBER_OF_POINTS; i++) {
            for (int j = 0; j < NUMBER_OF_DIMENSIONS; j++) {
                out << points[i][j];
                if (j < NUMBER_OF_DIMENSIONS - 1) {
                    out << ",";
                }
            }
            out << std::endl;
        }
    }

    return 0;
}
