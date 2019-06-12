from setuptools import setup, find_packages

with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
	license = f.read()

RIPS_DIST_VERSION = '8'
	
setup(
    name='rips',
    version='@RESINSIGHT_MAJOR_VERSION@.@RESINSIGHT_MINOR_VERSION@.@RESINSIGHT_PATCH_VERSION@.' + RIPS_DIST_VERSION,
    description='Python Interface for ResInsight',
    long_description=readme,
    author='Ceetron Solutions',
    author_email='info@ceetronsolutions.com',
    url='http://www.resinsight.org',
    license=license,
	include_package_data=True,
    packages=['rips', 'rips.generated', 'rips.examples', 'rips.tests']
)