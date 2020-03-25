# ResInsight Scripting API - rips

## Please note
The folder **rips** is a local copy for testing purposes. We need to be able to communicate the generated files to readthedocs in some way.

## How to generate documentation locally
- Install Python 3.x
- Install dependencies for rips `pip install grpcio grpcio-tools protobuf`
- Install the documentation system `pip install sphinx`
- Install dependencies for sphinx `pip install m2r sphinx_rtd_theme`
- Open command line in folder **docs**
- Execute `make html`
- Open the generated documentation in a browser `build/html/index.html`
