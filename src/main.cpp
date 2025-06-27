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
#include <random>
#include <chrono>

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

std::pair<double, double> parseBounds(const std::string& input) {
    auto bounds = split(input, ":");
    if (bounds.size() != 2) 
        throw std::invalid_argument("Invalid base-scale format, use lower:upper");

    return {std::stof(bounds[0]), std::stof(bounds[1])};
}

std::vector<std::tuple<int, double, double>> parseOverrides(const std::string& input) {
    std::vector<std::tuple<int, double, double>> result;
    if (input.empty()) 
        return result;

    auto entries = split(input, ",");
    for (const auto& entry : entries) {
        auto parts = split(entry, ":");
        if (parts.size() != 3) 
            throw std::invalid_argument("Invalid scale override format, use dim:lower:upper");
            
        int dim = std::stoi(parts[0]);
        double low = std::stof(parts[1]);
        double high = std::stof(parts[2]);
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

bool randomIsValid(const std::vector<std::string> random, const int NUMBER_OF_DIMENSIONS) {
    // input validation for randomization
    if (random.size() > 1 || (random[0] != "true" && random[0] != "false")) {    
        
        // input validation that there are not more dimensions identified for randomization than there are dimensions
        if (random.size() > NUMBER_OF_DIMENSIONS){
            throw std::invalid_argument("Invalid input. Too many dimensions randomized.");
            return false;
        }

        for (int i = 0; i < random.size(); i++){
            int randomAsInt;
    
            // input validation that each dimension is an integer
            try{
                randomAsInt = std::stoi(random[i]);
            }catch(std::invalid_argument& e){
                throw std::invalid_argument("Invalid input. Dimension " + random[i] + " is not an integer.");
                return false;
            }

            // input validation that there is no randomization for a dimension that does not exist
            bool randomGreaterThanDimensionsCount = randomAsInt >= NUMBER_OF_DIMENSIONS;
            bool randomLessThanZero = randomAsInt < 0;
            if (randomGreaterThanDimensionsCount || randomLessThanZero){
                throw std::invalid_argument("Invalid input. Dimension " + random[i] + " does not exist.");
                return false;
            }

            // input validation that each dimension is unique
            for (int j = 0; j < i; j++){
                if (random[i] == random[j]){
                    throw std::invalid_argument("Invalid input. Dimension " + random[i] + " is duplicated.");
                    return false;
                }
            }
        }
    }

    return true;
}

bool outfileIsValid(const std::string outDir) {
    // input validation that the file path is not empty
    if (outDir.length() == 0) {
        throw std::invalid_argument("Invalid input. File path is empty.");
        return false;
    }

    // input validation that the file path is valid
    std::ofstream out;
    try {
        out.open(outDir, std::ios::out | std::ios::trunc);
    } catch (std::ofstream::failure& e) {
        throw std::invalid_argument("Invalid input. File path is invalid.");
        return false;
    }

    // input validation that the file path is writable
    if (!out.is_open()) {
        throw std::invalid_argument("Invalid input. File path is not writable.");
        return false;
    }

    out.close();

    return true;
}

bool headingsAreValid(const std::vector<std::string> headings, const int NUMBER_OF_DIMENSIONS) {  
    // input validation for the headings
    if (headings.size() == 0 && headings.size() != NUMBER_OF_DIMENSIONS) {
        throw std::invalid_argument("Invalid number of headings. Received " + std::to_string(headings.size()) + ", expected " + std::to_string(NUMBER_OF_DIMENSIONS) + "\n");
        return false;
    }

    return true;
}

int findPrecision(const double ratio) {
    int precision = 0;

    while (ratio * pow(10.0, precision) < 100.0) {
        precision++;
    }

    return precision;
}

int main(int argc, char *argv[])
{
    // letters used: hndrbsoc
    const std::string OPTION_NUMBER = "number";
    const std::string OPTION_DIMENSIONS = "dimensions";
    const std::string OPTION_RANDOM = "random";
    const std::string OPTION_BASE_SCALE = "base-scale";
    const std::string OPTION_SCALES = "scales";
    const std::string OPTION_OUT_PATH = "out-path";
    const std::string OPTION_HEADINGS = "column-headings";

    const std::string RANDOM_TRUE = "true";
    const std::string RANDOM_FALSE = "false";
    const std::string OUT_PATH_DEFAULT = "lhc.csv";
    const std::string BASE_SCALE_DEFAULT = "0:1";
    const std::string RANDOM_DEFAULT = RANDOM_FALSE;

    cxxopts::Options options("lhc", "Latin Hypercube generator");

    options.add_options()
        (optionKeyFormatter(OPTION_NUMBER), "Required. Positive integer. The number of points to generate.", cxxopts::value<long>())
        (optionKeyFormatter(OPTION_DIMENSIONS), "Required. Positive integer. The number of dimensions in each point.", cxxopts::value<int>())
        (optionKeyFormatter(OPTION_RANDOM), "Optional. Select randomness: '" + RANDOM_FALSE + "' = none, '" + RANDOM_TRUE + "' = all, or a comma-separated list of dimension indices. This option will add a small amount of random variance to each point in each selected dimension", cxxopts::value<std::string>()->default_value(RANDOM_DEFAULT))
        (optionKeyFormatter(OPTION_BASE_SCALE), "Optional. A pair of floating-point values. Default scale for all dimensions in the form lower:upper", cxxopts::value<std::string>()->default_value(BASE_SCALE_DEFAULT))
        (optionKeyFormatter(OPTION_SCALES), "Optional. Comma-separated dimension:lower:upper overrides", cxxopts::value<std::string>())
        (optionKeyFormatter(OPTION_OUT_PATH), "Optional. File path for CSV output", cxxopts::value<std::string>()->default_value(OUT_PATH_DEFAULT))
        (optionKeyFormatter(OPTION_HEADINGS), "Optional. Column names for CSV output", cxxopts::value<std::string>())
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        std::cout << "NOTE: Please be aware that generating a large number of points (i.e. over five million) may take a long time and be resource intensive." << std::endl;
        return 0;
    }

    if (result.count(OPTION_NUMBER) == 0 || result.count(OPTION_DIMENSIONS) == 0) {
        throw std::invalid_argument("Missing required arguments");
        return 1;
    }

    long NUMBER_OF_POINTS = result[OPTION_NUMBER].as<long>();
    int NUMBER_OF_DIMENSIONS = result[OPTION_DIMENSIONS].as<int>();
    std::vector<std::string> random = split(result[OPTION_RANDOM].as<std::string>(), ",");
    std::pair<double, double> baseScale = parseBounds(result[OPTION_BASE_SCALE].as<std::string>());

    if (NUMBER_OF_POINTS <= 0) {
        throw std::invalid_argument("Number of points must be greater than 0");
        return 1;
    }

    if (NUMBER_OF_POINTS > std::mt19937::max()) {
        throw std::invalid_argument("Number of points must be less than " + std::to_string(std::mt19937::max()));
        return 1;
    }

    if (NUMBER_OF_DIMENSIONS <= 0) {
        throw std::invalid_argument("Number of dimensions must be greater than 0");
        return 1;
    }

    if (baseScale.first > baseScale.second) {
        throw std::invalid_argument("Base Scale: Lower bound must be less than upper bound");
        return 1;
    }

    double ratio[NUMBER_OF_DIMENSIONS];              // holds the scale of each dimension
    int precision[NUMBER_OF_DIMENSIONS];             // holds the precision of each dimension
    double dimensionScales[NUMBER_OF_DIMENSIONS][2]; // holds the lower and upper bounds of each dimension
    bool valid;                                      // keeps track of do-while validity

    // check if random is valid
    if (!randomIsValid(random, NUMBER_OF_DIMENSIONS)) {
        return 1;
    }

    // check if outDir is valid
    std::string outDir = result[OPTION_OUT_PATH].as<std::string>();
    if (!outfileIsValid(outDir)) {
        return 1;
    }
    std::ofstream out;
    out.open(outDir, std::ios::out | std::ios::trunc);

    // check if headings are valid
    std::vector<std::string> headings;
    if (result.count(OPTION_HEADINGS)) {
        headings = escapeHeadings(
            split(
                result[OPTION_HEADINGS].as<std::string>(), 
                ","
            )
        );
    } else {
        for (int dimensionIndex = 0; dimensionIndex < NUMBER_OF_DIMENSIONS; ++dimensionIndex) {
            headings.push_back("dim" + std::to_string(dimensionIndex));
        }
    }
    headingsAreValid(
        headings,
        NUMBER_OF_DIMENSIONS
    );
    
    // set the base scale for each dimension
    for (int dimensionIndex = 0; dimensionIndex < NUMBER_OF_DIMENSIONS; ++dimensionIndex) {
        dimensionScales[dimensionIndex][0] = baseScale.first;
        dimensionScales[dimensionIndex][1] = baseScale.second;
    }

    std::cout << "Generating " << NUMBER_OF_POINTS << " points in " << NUMBER_OF_DIMENSIONS << " dimensions.\n";

    std::cout << "Random selection: ";
    for (int i = 0; i < random.size(); i++){
        std::cout << random[i] << " ";
    }
    std::cout << "\n";

    std::cout << "File output path: " << outDir << "\n";

    std::cout << "Headings: ";
    for (const std::string h : headings) {
        std::cout << h << " ";
    }
    std::cout << "\n";

    std::cout << "Base scale: " << baseScale.first << ":" << baseScale.second << "\n";

    // set the lower and upper bounds for each customized dimension
    if (result.count(OPTION_SCALES)) {
        for (const auto& [dimensionIndex, low, high] : parseOverrides(result[OPTION_SCALES].as<std::string>())) {
            // input validation that the dimension index is valid
            if (dimensionIndex < 0 || dimensionIndex >= NUMBER_OF_DIMENSIONS) {
                throw std::invalid_argument("Invalid dimension index in --scale: " + std::to_string(dimensionIndex));
                return 1;
            }

            // input validation that the lower bound is less than the upper bound
            if (low > high) {
                throw std::invalid_argument("Invalid scale in --scale: " + std::to_string(low) + ":" + std::to_string(high));
                return 1;
            }

            // set the lower and upper bounds
            dimensionScales[dimensionIndex][0] = low;
            dimensionScales[dimensionIndex][1] = high;

            std::cout << "Dimension " << dimensionIndex << " scale: " << low << ":" << high << "\n";
        }
    }

    std::vector<std::vector<double>> points(NUMBER_OF_POINTS, std::vector<double>(NUMBER_OF_DIMENSIONS));   //stores coordinates
    
    std::seed_seq seed{ static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };  // seeds random generator
    std::mt19937 generator (seed);  // create random number generator

    std::cout << "Generating points...\n";
    try {
        // for loop populates points array
        for(long dimensionIndex = 0; dimensionIndex < NUMBER_OF_DIMENSIONS; dimensionIndex++){
            // the range of values 0 to SIZE 
            std::vector<long> range(NUMBER_OF_POINTS);
            
            // for loop populates range array
            for(long rangeIndex = 0; rangeIndex < NUMBER_OF_POINTS; rangeIndex++){  
                range[rangeIndex] = rangeIndex;
            }
            
            double upperBound = dimensionScales[dimensionIndex][1];
            double lowerBound = dimensionScales[dimensionIndex][0];
            ratio[dimensionIndex] = (upperBound - lowerBound) / NUMBER_OF_POINTS;
            precision[dimensionIndex] = findPrecision(ratio[dimensionIndex]);
            
            // for loop generates random unique selection from range
            for(long pointIndex = 0; pointIndex < NUMBER_OF_POINTS; pointIndex++){  
                // temporary var to generate random index in range
                long temp = generator() % NUMBER_OF_POINTS;   
                double decimal = (double)(generator() % 100) / 100.0; //holds random addition to value
                
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
                    temp = generator() % NUMBER_OF_POINTS;
                }
                
                points[pointIndex][dimensionIndex] = temp + decimal;     // assigns current index of "points" the value of "range[temp]" plus a random decimal value
                points[pointIndex][dimensionIndex] *= ratio[dimensionIndex];    // adjust value of "points" for range of possible values
                points[pointIndex][dimensionIndex] += lowerBound;               // adjust value of "points" for starting point of possible values
                range[temp] = -1;                                               // selected index is marked as selected
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // export headings and data to csv
    std::cout << "Writing to " << result[OPTION_OUT_PATH].as<std::string>() << "..." << std::endl;
    for (const std::string h : headings) {
        out << h;
        if (h != headings.back()) {
            out << ",";
        }
    }
    for (long pointIndex = 0; pointIndex < NUMBER_OF_POINTS; pointIndex++) {        
        out << std::endl;
        for (int dimensionIndex = 0; dimensionIndex < NUMBER_OF_DIMENSIONS; dimensionIndex++) {
            out << std::fixed << std::setprecision(precision[dimensionIndex]) << points[pointIndex][dimensionIndex];
            if (dimensionIndex < NUMBER_OF_DIMENSIONS - 1) {
                out << ",";
            }
        }
    }
    out.close();

    std::cout << "Done!" << std::endl;

    return 0;
}
