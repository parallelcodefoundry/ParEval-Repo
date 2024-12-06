```markdown
# XSBench: Monte Carlo Reactor Analysis

## Overview
XSBench is a high-performance Python library for Monte Carlo reactor analysis. It provides a flexible and customizable framework for simulating various reactor systems.

## Performance Abstraction
XSBench abstracts away the performance details of underlying hardware, allowing users to easily switch between different execution models (e.g., OpenMP offload).

## Usage
1. Compile XSBench with OpenMP support using `gcc -fopenmp xsbench.c -o xsbench`.
2. Run `python` to execute a simulation.

## Citing XSBench
Papers citing the XSBench program in general should refer to:

[J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto. https://www.mcs.anl.gov/papers/P5064-0114.pdf](https://www.mcs.anl.gov/papers/P5064-0114.pdf)

Bibtex Entry:

```bibtex
@inproceedings{Tramm:wy,
author = {Tramm, John R and Siegel, Andrew R and Islam, Tanzima and Schulz, Martin},
title = {{XSBench} - The Development and Verification of a Performance Abstraction for {M}onte {C}arlo Reactor Analysis},
booktitle = {{PHYSOR} 2014 - The Role of Reactor Physics toward a Sustainable Future},
address = {Kyoto},
year = 2014,
url = "https://www.mcs.anl.gov/papers/P5064-0114.pdf"
}
```

## Development Team
Authored and maintained by John Tramm ([@jtramm](https://github.com/jtramm)) with help from Ron Rahaman, Amanda Lund, and other [contributors](https://github.com/ANL-CESAR/XSBench/graphs/contributors).

## License
XSBench is released under the MIT License.

## Contributing
Contributions are welcome. Please submit pull requests or issues via [GitHub](https://github.com/ANL-CESAR/XSBench/issues).

## Performance Abstraction Configuration
To enable OpenMP offload, set the `SIMULATION_METHOD` configuration option to 1:

```python
Inputs().simulation_method = 1
```

This will execute the simulation using OpenMP offload.
```markdown

# XSBench: Monte Carlo Reactor Analysis

## Overview
XSBench is a high-performance Python library for Monte Carlo reactor analysis. It provides a flexible and customizable framework for simulating various reactor systems.

## Performance Abstraction
XSBench abstracts away the performance details of underlying hardware, allowing users to easily switch between different execution models (e.g., OpenMP offload).

## Usage
1. Compile XSBench with OpenMP support using `gcc -fopenmp xsbench.c -o xsbench`.
2. Run `python` to execute a simulation.

## Citing XSBench
Papers citing the XSBench program in general should refer to:

[J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto. https://www.mcs.anl.gov/papers/P5064-0114.pdf](https://www.mcs.anl.gov/papers/P5064-0114.pdf)

Bibtex Entry:

```bibtex
@inproceedings{Tramm:wy,
author = {Tramm, John R and Siegel, Andrew R and Islam, Tanzima and Schulz, Martin},
title = {{XSBench} - The Development and Verification of a Performance Abstraction for {M}onte {C}arlo Reactor Analysis},
booktitle = {{PHYSOR} 2014 - The Role of Reactor Physics toward a Sustainable Future},
address = {Kyoto},
year = 2014,
url = "https://www.mcs.anl.gov/papers/P5064-0114.pdf"
}
```

## Development Team
Authored and maintained by John Tramm ([@jtramm](https://github.com/jtramm)) with help from Ron Rahaman, Amanda Lund, and other [contributors](https://github.com/ANL-CESAR/XSBench/graphs/contributors).

## License
XSBench is released under the MIT License.

## Contributing
Contributions are welcome. Please submit pull requests or issues via [GitHub](https://github.com/ANL-CESAR/XSBench/issues).

## Performance Abstraction Configuration
To enable OpenMP offload, set the `SIMULATION_METHOD` configuration option to 1:

```python
Inputs().simulation_method = 1
```

This will execute the simulation using OpenMP offload.

## OpenMP Offload Example
```python
import xsbench

# Set simulation method to 1 for OpenMP offload
Inputs().simulation_method = 1

# Create a reactor object with some settings
reactor = xsbench.Reactor(
    name='Test Reactor',
    fuel_type='Uranium-235',
    coolant='Water'
)

# Initialize the reactor
reactor.init()

# Run the simulation
reactor.run()
```

This code will execute the simulation using OpenMP offload.