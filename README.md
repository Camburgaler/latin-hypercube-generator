# Latin Hypercube Point Generator

## Description

An excerpt from the [Wikipedia article on latin hypercube sampling](https://en.wikipedia.org/wiki/Latin_hypercube_sampling#cite_note-C3M-1): "Latin hypercube sampling (LHS) is a statistical method for generating a near-random sample of parameter values from a multidimensional distribution. The sampling method is often used to construct computer experiments or for Monte Carlo integration." This project was written during a summer internship that I did for TechSource, Inc., an LANL-adjacent company in Los Alamos, New Mexico. I needed to better understand Monte Carlo integration, and this helped to facilitate that. I plan to bring this to a more accessible format at some point.

## Features

-   Generates LHC samples
-   Support for an arbitrary number of dimensions
-   Configurable range for each dimension
-   Toggleable random variance
-   Export data to CSV

## Compilation

```bash
g++ -static -I include -g -o lhc src/main.cpp
```

## Usage

```
  lhc [OPTION...]

  -n, --number arg           Required. Positive integer. The number of
                             points to generate.
  -d, --dimensions arg       Required. Positive integer. The number of
                             dimensions in each point.
  -r, --random arg           Optional. Select randomness: 'false' = none,
                             'true' = all, or a comma-separated list of
                             dimension indices. This option will add a
                             small amount of random variance to each point
                             in each selected dimension (default: false)
  -b, --base-scale arg       Optional. A pair of floating-point values.
                             Default scale for all dimensions in the form
                             lower:upper (default: 0:1)
  -s, --scales arg           Optional. Comma-separated
                             dimension:lower:upper overrides
  -o, --out-path arg         Optional. File path for CSV output (default:
                             lhc.csv)
  -c, --column-headings arg  Optional. Column names for CSV output
  -h, --help                 Print help

NOTE: Please be aware that generating a large number of points (i.e. over five million) may take a long time and be resource intensive.
```

## Example Output

### Console

```bash
$ ./lhc -n 1000 -d 5 -r true -f -s 0:0:1000,3:0:10,4:-1000:1000
Generating 1000 points in 5 dimensions.
Random selection: true
File output path: lhc.csv
Headings: dim0 dim1 dim2 dim3 dim4
Base scale: 0:1
Dimension 0 scale: 0:1000
Dimension 3 scale: 0:10
Dimension 4 scale: -1000:1000
Generating points...
Writing to lhc.csv...
Done!
```

### CSV Excerpt

```csv
dim0,dim1,dim2,dim3,dim4
866.32,0.83296,0.33675,3.9225,-509.06
562.26,0.87963,0.97232,1.0555,692.00
209.95,0.28945,0.61518,6.4720,-763.54
185.47,0.05392,0.41277,8.6201,-306.46
150.80,0.04556,0.46169,3.5289,-417.46
753.36,0.85700,0.56880,0.5137,394.72
887.80,0.49907,0.30354,4.8745,54.14
181.29,0.99033,0.60394,5.5204,567.56
912.32,0.67701,0.24658,7.2106,-48.10
8.00,0.81029,0.98483,8.6182,162.24
984.08,0.43974,0.19252,6.8308,752.72
490.18,0.31467,0.13611,5.9179,841.76
550.03,0.32800,0.46703,7.5362,-142.66
914.39,0.57556,0.47453,3.4694,647.90
111.88,0.78515,0.48942,4.0961,209.90
200.63,0.72052,0.78757,7.1754,357.82
468.87,0.95007,0.47064,0.1125,-479.08
704.23,0.73242,0.32050,0.1872,-350.08
759.44,0.17712,0.35910,4.7921,125.52
624.04,0.06657,0.11461,6.9688,191.80
...
```

## Planned Improvements

-   Add customization option for the amount of random variance.

## Credits

Thanks to [C++ Options](https://github.com/jarro2783/cxxopts) for the command-line parsing library.
