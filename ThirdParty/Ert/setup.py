import os
import subprocess

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        cmake_args = [] # Fill in extra stuff we may need
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir]
        cfg = 'Debug' if self.debug else 'Release'
        cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
        build_args = ['--config', cfg, '--', '-j2']
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        env = os.environ.copy()
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

setup(
    name='statoil_libecl',
    version='0.1.1',
    author_email='chandan.nath@gmail.com',
    description='libecl',
    long_description=open("README.md", "r").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/statoil/libecl",
    license="GNU General Public License, Version 3, 29 June 2007",
    packages=find_packages(where='python', exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
    package_dir={'': 'python'},
    ext_package='ecl',
    ext_modules=[CMakeExtension('libecl')],
    cmdclass=dict(build_ext=CMakeBuild),
    install_requires=[
        'cwrap',
        'numpy',
        'pandas',
    ],
)
