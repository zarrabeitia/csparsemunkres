from distutils.core import setup, Extension

module1 = Extension('csparsemunkres',
                    sources = ['pythonmodule.cpp', 'matrix.cpp', 'munkres.cpp'])

setup (name = 'csparsemunkres',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])


