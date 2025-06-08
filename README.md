# Latin Hypercube Point Generator

## Description

An excerpt from the [Wikipedia article on latin hypercube sampling](https://en.wikipedia.org/wiki/Latin_hypercube_sampling#cite_note-C3M-1): "Latin hypercube sampling (LHS) is a statistical method for generating a near-random sample of parameter values from a multidimensional distribution. The sampling method is often used to construct computer experiments or for Monte Carlo integration. " This project was written during a summer internship that I did for TechSource, Inc., an LANL-adjacent company in Los Alamos, New Mexico. I needed to better understand Monte Carlo integration, and this helped to facilitate that. I plan to bring this to a more accessible format at some point.

## Features

-   Generates LHC samples for arbitrary dimensions
-   Configurable range and floating-point inclusion
-   Basic statistical analysis (mean, variance, std. dev)

## Compilation

```bash
g++ -I include -g -o lhc src/main.cpp
```

## Usage

```bash
  lhc [OPTION...]

  -n, --number arg      Number of points (default: 1000)
  -d, --dimensions arg  Number of dimensions (default: 1)
  -r, --random arg      Select randomness: 'false' = none, 'true' = all, or
                        a comma-separated list of dimension indices
                        (default: false)
  -b, --base-scale arg  Default scale for all dimensions in the form
                        lower:upper (default: 0:1)
  -s, --scales arg      Comma-separated dimension:lower:upper overrides
  -h, --help            Print help
```

## Example Output

```bash
$ ./lhc -n 1000 -d 5 -r true -b 0:1000
...
713.8 730.56 925.16 903.16 546.68
413.84 909.48 725.99 691.76 346.68
780.88 236 261.83 1005.2 236.16
774.16 640.44 759.92 575.28 250.8
1014.96 1062 751.46 832.08 510.28
722.48 547.24 180.57 247.6 284.56
188.64 921.48 77.11 729.32 717.16
129.96 873.36 451.59 185.04 397.76
303.72 310.12 364.68 315.72 292.12
78.16 797.68 202.49 218.76 871.6
1057.16 549.36 321.99 503.32 902.64
284.52 624.96 264.4 378.28 392.16
130.16 646.84 13.59 313.08 307.64
759.52 1077.32 1105.38 78.08 1078.6
802.52 990.44 632.29 694.96 560.04
1038.96 472.08 593.79 748.68 430.04
816.04 655.64 1081.89 880.48 342.72
460.6 920.48 207.51 1001.92 418.44
960.88 320.2 916.8 935.12 860.6
431.84 179.28 889.94 481.52 768.48
565.24 101.12 156.03 618.6 1063.8

Analysis of dimension 1
-----------------------
Mean: 598.487
Variance: 5.12713e-05
Standard Deviation: 0.0071604

Analysis of dimension 2
-----------------------
Mean: 619.756
Variance: 0.000131175
Standard Deviation: 0.0114532

Analysis of dimension 3
-----------------------
Mean: 578.979
Variance: 4.21367e-05
Standard Deviation: 0.00649128

Analysis of dimension 4
-----------------------
Mean: 544.027
Variance: 4.81939e-07
Standard Deviation: 0.000694219

Analysis of dimension 5
-----------------------
Mean: 606.795
Variance: 0.000149843
Standard Deviation: 0.012241
```

## Planned Improvements

-   Export data to CSV.
-   Add customization option for the amount of random variance.

## Credits

Thanks to [C++ Options](https://github.com/jarro2783/cxxopts) for the command-line parsing library.
