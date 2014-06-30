from distutils.core import setup, Extension

module1 = Extension('csparsemunkres',
                    sources = ['pythonmodule.cpp', 'matrix.cpp', 'munkres.cpp'])

setup (name = 'csparsemunkres',
        version = '1.0',
        description = 'kuhn-munkres algorithm for the Assignment Problem with an incomplete cost matrix',
        license = "GPLv2",
        author = "Luis A. Zarrabeitia",
        author_email = "zarrabeitia at gmail dot com",
        ext_modules = [module1],
        classifiers = [
            'Intended Audience :: Developers',
            'Intended Audience :: Science/Research',
            'License :: OSI Approved :: GPLv2',
            'Operating System :: OS Independent',
            'Programming Language :: Python',
            'Topic :: Scientific/Engineering :: Mathematics',
            'Topic :: Software Development :: Libraries :: Python Modules'
        ]
)


