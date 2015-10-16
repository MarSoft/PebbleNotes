"""
pypebbleapi
------------
Pebble-api for python.
Library to ease the access to the Pebble Timeline and the creation of Pins.
"""

from setuptools import setup, find_packages


setup(
    name='pypebbleapi',
    version='1.0.0',
    url='https://github.com/youtux/pypebbleapi',
    license='MIT',
    author='Alessio Bogon',
    author_email='youtux@gmail.com',
    description='Pebble-api for python.',
    long_description=__doc__,
    packages=find_packages(),
    install_requires=['requests>=2.5.1,<3', 'six~=1.9', 'Cerberus~=0.9'],
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ]
)
