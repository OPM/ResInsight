from setuptools import setup, find_packages

with open('README.rst') as f:
    readme = f.read()

with open('LICENSE') as f:
	license = f.read()

setup(
    name='rips',
    version='@RESINSIGHT_MAJOR_VERSION@.@RESINSIGHT_MINOR_VERSION@.@RESINSIGHT_PATCH_VERSION@',
    description='Python Interface for ResInsight',
    long_description=readme,
    author='Ceetron Solutions',
    author_email='info@ceetronsolutions.com',
    url='http://www.resinsight.org',
    license=license,
    packages=find_packages(exclude=('tests', 'docs', '__pycache', 'examples'))
)